#include "xmacro.h"
#include "unitcell.h"
#include "refutil.h"
#include "hkl.h"
#include "utf8file.h"
#include "olxmps.h"
//..............................................................................
struct TestRTask : public TaskBase {
  const TAsymmUnit &au;
  const TUnitCell::SymmSpace &sp;
  const TRefList &refs;
  const TRefPList &testr;
  const evecd &Fsq, &weights;
  const TArray3D<TReflection*> &hkl3d;
  sorted::PrimitiveAssociation<double,
    AnAssociation4<vec3i, double, double,
    AnAssociation3<bool, size_t, double> > > hits;
  olxset<vec3d, TComparableComparator> &uniq_dir;
  double originalR;
  mat3d fm, om, hm;
  TArrayList<TReflection*> mates;
  bool direct, invert;
  double cos_angle;

  TestRTask(const TAsymmUnit &au,
    const TUnitCell::SymmSpace &sp,
    const TRefList &refs,
    const TRefPList &testr,
    const evecd &Fsq, const evecd &weights,
    const TArray3D<TReflection*> &hkl3d,
    olxset<vec3d, TComparableComparator> &uniq_dir,
    double originalR)
    : au(au),
    sp(sp),
    refs(refs),
    testr(testr),
    Fsq(Fsq),
    weights(weights),
    hkl3d(hkl3d),
    uniq_dir(uniq_dir),
    originalR(originalR),
    mates(refs.Count())
  {
    direct = true;
    invert = false;
    cos_angle = -1;
    fm = au.GetCartesianToCell();
    om = au.GetCellToCartesian();
    hm = au.GetHklToCartesian();
  }

  TestRTask(TestRTask &t)
    : au(t.au),
    sp(t.sp),
    refs(t.refs),
    testr(t.testr),
    Fsq(t.Fsq),
    weights(t.weights),
    hkl3d(t.hkl3d),
    uniq_dir(t.uniq_dir),
    originalR(t.originalR),
    mates(refs.Count()),
    direct(t.direct),
    invert(t.invert),
    cos_angle(t.cos_angle),
    fm(t.fm),
    om(t.om),
    hm(t.hm)
  {}

  void Run(size_t h_) {
    int h = (int)h_ - 12;
    const mat3d &dir_tm = direct ? hm : fm;
    for (int k = -12; k <= 12; k++) {
      for (int l = 0; l <= 12; l++)
      {
        if (h == 0 && k == 0 && (l == 0 || l > 1 || l < -1)) {
          continue;
        }
        if (h == 0 && l == 0 && (k > 1 || k < -1)) {
          continue;
        }
        if (k == 0 && l == 0 && (h > 1 || h < -1)) {
          continue;
        }
        vec3d rv = TReflection::ToCart(vec3d(h, k, l), dir_tm).Normalise();
        {
          volatile olx_scope_cs _cs(GetCriticalSection());
          if (!uniq_dir.Add(rv)) {
            continue;
          }
        }
        mat3d rm;
        olx_create_rotation_matrix(rm, rv, cos_angle);
        mat3d hrm = om * rm*fm;
        if (invert) {
          hrm *= -1;
        }
        mates.ForEach(olx_list_init::zero());
        double bup = 0, bdn = 0, ds = 0;
        size_t cnt = 0;
        for (size_t i = 0; i < refs.Count(); i++) {
          vec3d ctr = hrm * vec3d(refs[i].GetHkl());
          vec3i tri = ctr.Round<int>();
          double d = TReflection::ToCart(ctr - tri, hm).QLength();
          if (d > 0.005*0.005) {
            continue;
          }
          ds += d;
          tri = TReflection::Standardise(tri, sp);
          if (!hkl3d.IsInRange(tri)) {
            continue;
          }
          if ((mates[i] = hkl3d(tri)) != 0) {
            double w = 1;// weights[refs[i].GetTag()];
            double x = Fsq[refs[i].GetTag()] - Fsq[mates[i]->GetTag()];
            bup += w * x * (refs[i].GetI() - Fsq[mates[i]->GetTag()]);
            bdn += w * olx_sqr(x);
            if (cnt++ >= refs.Count()/5) {
              break;
            }
          }
        }
        if (cnt < 30) {
          continue;
        }
        double basf = bup / olx_max(1e-3, bdn);
        if (basf <= 0.05 || basf >= 0.95) {
          continue;
        }
        bup = bdn = 0;
        size_t mc = 0;
        for (size_t i = 0; i < testr.Count(); i++) {
          TReflection *mate = mates[testr[i]->GetTag()];
          if (mate == 0) {
            continue;
          }
          mc++;
          double si = testr[i]->GetI();
          double w = weights[testr[i]->GetTag()];
          bup += w * olx_sqr(si -
            basf * (Fsq[testr[i]->GetTag()] - Fsq[mate->GetTag()]) -
            Fsq[mate->GetTag()]);
          bdn += w * olx_sqr(si);
        }
        if (mc < testr.Count() / 4) {
          continue;
        }
        double R = sqrt(bup / (olx_max(1e-3, bdn)));
        if (R < originalR) {
          hits.Add(R / cnt, Association::Create(
            vec3i(h, k, l), basf, cos_angle,
            Association::Create(direct, cnt, sqrt(ds))));
        }
      }
    }
  }
  TestRTask *Replicate() {
    return new TestRTask(*this);
  }
};
//..............................................................................
//..............................................................................
void XLibMacros::macTestR(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  TStopWatch sw(__FUNC__);
  sw.start("Initialising");
  RefUtil::Stats rstat(Options.GetBoolOption('s', true, false));

  TXApp& xapp = TXApp::GetInstance();
  TUnitCell::SymmSpace sp = xapp.XFile().GetUnitCell().GetSymmSpace();
  RefinementModel& rm = xapp.XFile().GetRM();
  if (rm.GetHKLF() != 4) {
    Error.ProcessingError(__OlxSrcInfo, "HKLF4 mode is expected");
    return;
  }

  xapp.NewLogEntry() << "R1 (All, " << rstat.refs.Count() << ") = " <<
    olxstr::FormatFloat(4, rstat.R1);
  xapp.NewLogEntry() << "R1 (I/sig >= 2, " << rstat.partial_R1_cnt << ") = "
    << olxstr::FormatFloat(4, rstat.R1_partial);
  xapp.NewLogEntry() << "wR2 = " << olxstr::FormatFloat(4, rstat.wR2);

  double oR = 0;
  TRefPList testr = rstat.GetNBadRefs(50, &oR);
  for (size_t i = 0; i < olx_min(10, testr.Count()); i++) {
    TReflection& r = *testr[i];
    TBasicApp::NewLogEntry() <<
      olx_print("R %4d %4d %4d %8.2lf %8.2lf %8.2lf = %.2lf",
        r.GetH(), r.GetK(), r.GetL(),
        r.GetI(), r.GetS(), rstat.Fsq[r.GetTag()],
        sqrt(rstat.wsqd[r.GetTag()] * rstat.refs.Count() / rstat.sum_wsqd));

  }
  TBasicApp::NewLogEntry() << "Original wR2=" << olxstr::FormatFloat(3, oR);
  TArray3D<TReflection*> hkl3d(rstat.min_hkl, rstat.max_hkl);
  hkl3d.FastInitWith(0);
  for (size_t i = 0; i < rstat.refs.Count(); i++) {
    hkl3d(rstat.refs[i].GetHkl()) = &rstat.refs[i];
  }
  sorted::PrimitiveAssociation<double,
    AnAssociation4<vec3i, double, double,
    AnAssociation3<bool, size_t, double> > > hits;
  const TAsymmUnit &au = xapp.XFile().GetAsymmUnit();
  mat3d fm = au.GetCartesianToCell(),
    om = au.GetCellToCartesian(),
    hm = au.GetHklToCartesian();
  olxset<vec3d, TComparableComparator> uniq_dir;
  TestRTask task(au, sp, rstat.refs, testr,
    rstat.Fsq, rstat.weights, hkl3d, uniq_dir, oR);
  olx_critical_section cr_s;

  sw.start("Calculating direct lattice directions");
  TListIteratorManager<TestRTask> tasks(task, 24, tLinearTask, 1);
  for (size_t i = 0; i < tasks.Count(); i++) {
    tasks[i].direct = false;
  }
  sw.start("Calculating reciprocal lattice directions");
  tasks.ReRun();
  for (size_t i = 0; i < tasks.Count(); i++) {
    for (size_t j = 0; j < tasks[i].hits.Count(); j++) {
      hits.Add(tasks[i].hits.GetKey(j), tasks[i].hits.GetValue(j));
    }
  }
  if (hits.IsEmpty()) {
    return;
  }
  {
    TTable ho(olx_min(20, hits.Count()), 6);
    ho.ColName(0) = "wR2";
    ho.ColName(1) = "Direction";
    ho.ColName(2) = "BASF";
    ho.ColName(3) = "Angle";
    ho.ColName(4) = "Nref";
    ho.ColName(5) = "D/A";
    for (size_t i = 0; i < olx_min(20, hits.Count()); i++) {
      TStrList &r = ho[i];
      r[0] = olx_print("%.3lf", hits.GetKey(i)*hits.GetValue(i).d.b);
      r[1] = olx_print("%3d%3d%3d /", hits.GetValue(i).a[0],
        hits.GetValue(i).a[1], hits.GetValue(i).a[2]) <<
        (hits.GetValue(0).d.a ? 'd' : 'r');
      r[2] = olx_print("%.3lf", hits.GetValue(i).b);
      r[3] = olx_print("%.3lft", acos(hits.GetValue(i).c) * 180 / M_PI);
      r[4] = olx_print("%z", hits.GetValue(i).d.b);
      r[5] = olx_print("%.3le", hits.GetValue(i).d.c / hits.GetValue(i).d.b);
    }
    TBasicApp::NewLogEntry() << ho.CreateTXTList("Hits", true, false, ' ');
  }
  sw.start("Processing results");
  mat3d rot_m;
  olx_create_rotation_matrix(rot_m,
    TReflection::ToCart(hits.GetValue(0).a,
      hits.GetValue(0).d.a ? hm : fm).Normalise(), hits.GetValue(0).c);

  TBasicApp::NewLogEntry() << acos(hits.GetValue(0).c)*180/M_PI
    << " deg rotation around "
    << (hits.GetValue(0).d.a ? "direct" : "reciprocal") << " lattice vector: "
    << olx_print("%3d %3d %3d",
      hits.GetValue(0).a[0], hits.GetValue(0).a[1], hits.GetValue(0).a[2]);

  rot_m = om * rot_m*fm;
  TBasicApp::NewLogEntry() << "Transformation matrix:";
  for (size_t i = 0; i < 3; i++) {
    for (size_t j = 0; j < 3; j++) {
      if (olx_abs(rot_m[i][j]) < 1e-8) {
        rot_m[i][j] = 0;
      }
    }
    TBasicApp::NewLogEntry() << olx_print("%4.3lft %4.3lft %4.3lft",
      rot_m[i][0], rot_m[i][1], rot_m[i][2]);
  }
  TRefList out;
  for (size_t i = 0; i < rstat.refs.Count(); i++) {
    TReflection &r = *(new TReflection(rstat.refs[i]));
    r.SetBatch(1);
    vec3d tr = rot_m * vec3d(rstat.refs[i].GetHkl());
    vec3i tri = tr.Round<int>();
    if (TReflection::ToCart(tr - tri, hm).QLength() > 0.005*0.005) {
      out.Add(r);
      continue;
    }
    tri = TReflection::Standardise(tri, sp);
    if (!hkl3d.IsInRange(tri) || hkl3d(tri) == 0) {
      out.Add(r);
      continue;
    }
    out.Add(new TReflection(
      tri, rstat.refs[i].GetI(), rstat.refs[i].GetS(), -2)
    );
    out.Add(r);
  }
  olxstr fn = TEFile::ChangeFileExt(xapp.XFile().GetFileName(), "olex2_hklf5.hkl");
  olx_object_ptr<TUtf8File> outs = TUtf8File::Open(fn, "wb", false);
  THklFile::SaveToStream(out, outs());
  outs().Writeln(EmptyString());
  outs().Writeln("TITL Olex2 generated HKLF5 file");
  olxstr t = olx_print("CELL %.5lf %.4lf %.4lf %.4lf %.4lf %.4lf %.4lf",
    xapp.XFile().GetRM().expl.GetRadiation(),
    au.GetAxes()[0], au.GetAxes()[1], au.GetAxes()[2],
    au.GetAngles()[0], au.GetAngles()[1], au.GetAngles()[2]);
  outs().Writeln(t);
  t = olx_print("ZERR %.5lf %.4lf %.4lf %.4lf %.4lf %.4lf %.4lf",
    au.GetZ(),
    au.GetAxisEsds()[0], au.GetAxisEsds()[1], au.GetAxisEsds()[2],
    au.GetAngleEsds()[0], au.GetAngleEsds()[1], au.GetAngleEsds()[2]);
  outs().Writeln(t);
  outs().Writeln(olx_print("BASF %.4lf", 1.0 - hits.GetValue(0).b));
  outs().Writeln(
    olx_print("REM Matrix: %.3lf %.3lft %.3lft %.3lft %.3lft %.3lft %.3lft"
      "%.3lft %.3lft",
      rot_m[0][0], rot_m[0][1], rot_m[0][2],
      rot_m[1][0], rot_m[1][1], rot_m[1][2],
      rot_m[2][0], rot_m[2][1], rot_m[2][2]
    ));
  outs().Writeln("HKLF 5");
}
