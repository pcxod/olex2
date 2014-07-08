/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olxs_v_co_v_h
#define __olxs_v_co_v_h
#include "math/align.h"
#include "math/composite.h"
#include "evalue.h"
#include "asymmunit.h"
#include "lattice.h"
#include "bapp.h"
#include "log.h"
#include "symmcon.h"
#undef QLength
BeginXlibNamespace()

const short // constants decribing the stored values
  vcoviX = 0x0001,
  vcoviY = 0x0002,
  vcoviZ = 0x0004,
  vcoviO = 0x0008;
// stores X,Y,Z,SOF for each atom and their correlations
class VcoVMatrix {
  double **data;
  size_t count;
  TTypeList<AnAssociation3<olxstr, short, size_t> > Index;
  olxdict<size_t, size_t, TPrimitiveComparator> AtomIdIndex;
  olxstr_dict<size_t> AtomNameIndex;
  TStrList U_annotations;
  bool diagonal;
protected:
  void Allocate(size_t w, bool diag=false) {
    Clear();
    diagonal = diag;
    if (diagonal) {
      count = 1;
      data = new double*[1];
      data[0] = new double[w];
      memset(data[0], 0, sizeof(double)*w);
    }
    else {
      count = w;
      data = new double*[w];
      for( size_t i=0; i < w; i++ ) { // bottom diagonal agrrangement
        data[i] = new double[i+1];
        memset(data[i], 0, sizeof(double)*(i+1));
      }
    }
  }
  size_t FindAtomIndex(const TCAtom& a) const {
    return AtomIdIndex.Find(a.GetId(), InvalidIndex);
  }
  void UpdateAtomIndex();
public:
  VcoVMatrix();
  ~VcoVMatrix() {  Clear();  }
  void Clear() {
    if( data == NULL )  return;
    Index.Clear();
    for( size_t i=0; i < count; i++ )
      delete [] data[i];
    delete [] data;
    data = NULL;
  }
  double operator () (size_t i, size_t j) const { return Get(i,j); }
  double Get(size_t i, size_t j) const {
    if (diagonal)
      return (i==j) ? data[0][i] : 0;
    return (j <= i ) ? data[i][j] : data[j][i];
  }
  // reads the shelxl VcoV matrix
  void ReadShelxMat(const olxstr& fileName, TAsymmUnit& au);
  // reads the smtbx VcoV matrix
  void ReadSmtbxMat(const olxstr& fileName, TAsymmUnit& au);
  void FromCIF(TAsymmUnit& au);
  // creates matrices AA, AB, ... AX, BA, BB, ... BX, ...
  template <class list> void FindVcoV(const list& atoms, mat3d_list& m) const {
    TSizeList a_indexes;
    TTypeList<TVector3<size_t> > indexes;
    for (size_t i = 0; i < atoms.Count(); i++) {
      if (a_indexes.Add(FindAtomIndex(atoms[i]->CAtom())) == InvalidIndex) {
        SiteSymmCon sc = atoms[i]->CAtom().GetSiteConstraints();
        if (sc.map[6].param != -2 || sc.map[7].param != -2 ||
          sc.map[8].param != -2) // is not constrained?
        {
          TBasicApp::NewLogEntry(logError) << "Unable to located given atom: "
            << atoms[i]->GetLabel();
        }
      }
      indexes.AddNew(InvalidIndex, InvalidIndex, InvalidIndex);
    }
    for (size_t i = 0; i < a_indexes.Count(); i++)  {
      if( a_indexes[i] == InvalidIndex )  continue;
      for( size_t j=a_indexes[i];
          j < Index.Count() && Index[j].GetC() == atoms[i]->CAtom().GetId();
          j++ )
      {
        if( (Index[j].GetB() & vcoviX) != 0 )
          indexes[i][0] = j;
        if( (Index[j].GetB() & vcoviY) != 0 )
          indexes[i][1] = j;
        if( (Index[j].GetB() & vcoviZ) != 0 )
          indexes[i][2] = j;
      }
    }
    m.SetCapacity(olx_sqr(a_indexes.Count()));
    for( size_t i=0; i < a_indexes.Count(); i++ )  {
      for( size_t j=0; j < a_indexes.Count(); j++ )  {
        mat3d& a = m.AddNew();
        for( short k=0; k < 3; k++ )  {
          for( short l=0; l < 3; l++ )  {
            if( indexes[i][k] != InvalidIndex &&
                indexes[j][l] != InvalidIndex )
            {
              a[k][l] = Get(indexes[i][k], indexes[j][l]);
            }
          }
        }
      }
    }
  }
  // for tests
  double Find(const olxstr& atom, const short va, const short vy) const;
  bool IsEmpty() const { return data == NULL; }
};

class VcoVContainer {
  TAsymmUnit& au;
  VcoVMatrix vcov;
  // celle can be extended to matrix once covariances are available
  evecd cell, celle;
public:
  template <class atom_list>
  void ProcessSymmetry(const atom_list& atoms, mat3d_list& ms)  {
    mat3d_alist mt(atoms.Count());
    for( size_t i=0; i < atoms.Count(); i++ )
      mt[i] = mat3d::Transpose(atoms[i]->GetMatrix().r);
    for( size_t i=0; i < atoms.Count(); i++ )  {
      for( size_t j=0; j < atoms.Count(); j++ )  {
        const size_t ind = i*atoms.Count() + j;
        ms[ind] = (mat3d(atoms[i]->GetMatrix().r)*ms[ind])*mt[j];
      }
    }
  }
  // helper functions, matrices are in cartesian frame
  template <class atom_list>
  void GetVcoV(const atom_list& as, mat3d_list& m)  {
    if (GetMatrix().IsEmpty()) {
      size_t sz = olx_sqr(as.Count());
      m.SetCapacity(sz);
      while (m.Count() < sz)
        m.AddNew();
      return;
    }
    vcov.FindVcoV(as, m);
    ProcessSymmetry(as, m);
    mat3d c2f(as[0]->CAtom().GetParent()->GetCellToCartesian());
    mat3d c2f_t(
      mat3d::Transpose(as[0]->CAtom().GetParent()->GetCellToCartesian()));
    for( size_t i=0; i < m.Count(); i++ )
      m[i] = c2f_t*m[i]*c2f;
  }
  // helper functions, matrices are in fractional frame
  template <class atom_list>
  void GetVcoVF(const atom_list& as, mat3d_list& m)  {
    vcov.FindVcoV(as, m);
    ProcessSymmetry(as, m);
  }
protected:
  struct CellEsd {
    VcoVContainer& base;
    vec3d_alist &points, fcrds;
    template <class Evaluator> struct Functor  {
      CellEsd& base;
      const Evaluator& eval;
      Functor(CellEsd& _base, const Evaluator& e) : base(_base), eval(e)  {}
      double calc() const {
        base.base.Orthogonalise(base.fcrds, base.points);
        return eval.calc();
      }
    };
    CellEsd(VcoVContainer& _base, vec3d_alist& _ps)
      : base(_base), points(_ps)  {}
    template <class Evaluator> double DoCalc(const Evaluator& e)  {
      evecd df(6);
      fcrds = base.Fractionalise(points);
      Functor<Evaluator> f(*this, e);
      base.CalcDiff(base.cell, df, f);
      return (base.celle*df).DotProd(df);
    }
  };
  vec3d_alist Fractionalise(const vec3d_alist& l) const {
    vec3d_alist rv(l.Count());
    for( size_t i=0; i < l.Count(); i++ )
      rv[i] = au.Fractionalise(l[i]);
    return rv;
  }
  vec3d_alist& Orthogonalise(const vec3d_alist& l, vec3d_alist& rv) const {
    const double
      cA = cos(cell[3]),
      cB = cos(cell[4]),
      cG = cos(cell[5]), sG = sin(cell[5]),
      m[5] = {
        cell[1]*cG, cell[2]*cB, cell[1]*sG, -cell[2]*(cB*cG-cA)/sG,
        cell[2]*sqrt(1-cA*cA-cB*cB-cG*cG+2*cA*cB*cG)/sG
      };
    for( size_t i=0; i < l.Count(); i++ )  {
      rv[i][0] = l[i][0]*cell[0] + l[i][1]*m[0] + l[i][2]*m[1];
      rv[i][1] = l[i][1]*m[2] + l[i][2]*m[3];
      rv[i][2] = l[i][2]*m[4];
    }
    return rv;
  }
public:
  struct PlaneInfo  {
    double rmsd, d;
    vec3d center, normal;
  };
  template <class VT, class WT>
  static PlaneInfo CalcPlane(const VT& points, const WT& weights, short type=0)  {
    mat3d m, vecs;
    double mass = 0, qmass = 0;
    PlaneInfo rv;
    for( size_t i=0; i < points.Count(); i++ )  {
      rv.center += points[i]*weights[i];
      mass += weights[i];
      qmass += olx_sqr(weights[i]);
    }
    rv.center /= mass;
    for( size_t i=0; i < points.Count(); i++ )  {
      const vec3d t = (points[i] - rv.center)*weights[i];
      m[0][0] += (t[0]*t[0]);
      m[0][1] += (t[0]*t[1]);
      m[0][2] += (t[0]*t[2]);
      m[1][1] += (t[1]*t[1]);
      m[1][2] += (t[1]*t[2]);
      m[2][2] += (t[2]*t[2]);
    } // equ: d = s[0]*x + s[1]*y + s[2]*z
    m[1][0] = m[0][1];
    m[2][0] = m[0][2];
    m[2][1] = m[1][2];
    mat3d::EigenValues(m /= qmass, vecs.I());
    bool swaps = true;
    while( swaps )  {
      swaps = false;
      for( short i=0; i < 2; i++ )  {
        if( m[i][i] > m[i+1][i+1] )  {
          olx_swap(vecs[i], vecs[i+1]);
          olx_swap(m[i][i], m[i+1][i+1]);
          swaps = true;
        }
      }
    }
    rv.d = vecs[type].DotProd(rv.center)/vecs[type].Length();
    rv.normal = vecs[type].Normalise();
    rv.rmsd = (m[type][type] <= 0 ? 0 : sqrt(m[type][type]));
    return rv;
  }
  // plane to plane shift distance
  template <class pT, class cT> struct Centroid2CentriodShiftDistance  {
    const pT& p;
    const cT& c;
    Centroid2CentriodShiftDistance(const pT& _p, const cT& _c) : p(_p), c(_c)  {}
    double calc() const {
      const PlaneInfo pi = p.evaluate();
      const vec3d pc = c.evaluate();
      const double pcd = pc.DotProd(pi.normal) - pi.d;
      const double res = pc.QDistanceTo(pi.center) - pcd*pcd;
      return  res <= 0 ? 0 : sqrt(res);
    }
  };
  // alignment RMSD
  struct AlignmentRMSD  {
    const vec3d_alist& points;
    const TDoubleList& weights;
    AlignmentRMSD(const vec3d_alist& pts, const TDoubleList& wghts)
      : points(pts), weights(wghts)  {}
    double calc() const {
      align::ListsToPairAdaptor<vec3d_alist, TDoubleList> l2p(points, weights);
      align::out ao = align::FindAlignmentQuaternions(l2p);
      return align::CalcRMSD(l2p, ao);
    }
  };
  // plane to atom distance
  template <class planeT, class pointT> struct PlaneToPointDistance  {
    const planeT& plane;
    const pointT& point;
    PlaneToPointDistance(const planeT& _plane, const pointT& _point)
      : plane(_plane), point(_point)  {}
    double calc() const {
      const PlaneInfo pi = plane.evaluate();
      return point.evaluate().DotProd(pi.normal) - pi.d;
    }
  };
  template <class VT, class WT> struct PlaneEvaluator  {
    const VT& values;
    const WT& weights;
    PlaneEvaluator(const VT& _v, const WT& _w) : values(_v), weights(_w)  {}
    PlaneInfo evaluate() const {  return CalcPlane(values, weights);  }
  };
  // plane centroid to plane centroid distance
  template <class VT, class WT> struct CentroidEvaluator  {
    const VT& values;
    const WT& weights;
    CentroidEvaluator(const VT& _v, const WT& _w) : values(_v), weights(_w)  {}
    vec3d evaluate() const {
      double weight = 0;
      vec3d point ;
      for( size_t i=0; i < values.Count(); i++ )  {
        point += values[i];
        weight += weights[i];
      }
      return (point /= weight);
    }
  };
  struct PointProxy  {
    vec3d& point;
    PointProxy(vec3d& v) : point(v)  {}
    const vec3d& evaluate() const {  return point;  }
  };
  struct VectorProxy  {
    vec3d &a, &b;
    VectorProxy(vec3d& _a, vec3d& _b) : a(_a), b(_b)  {}
    vec3d evaluate() const {  return b-a;  }
  };
  template <class VT, class WT> struct PlaneNormalEvaluator  {
    VT values;
    WT weights;
    PlaneNormalEvaluator(const VT& _v, const WT& _w) : values(_v), weights(_w)  {}
    vec3d evaluate() const {  return CalcPlane(values, weights).normal;  }
  };
  template <class VT, class WT> struct LineEvaluator  {
    VT values;
    WT weights;
    LineEvaluator(const VT& _v, const WT& _w)
      : values(_v), weights(_w)
    {
      if (values.Count() < 2)
        throw TInvalidArgumentException(__OlxSourceInfo, "point count");
    }
    vec3d evaluate() const {
      if (values.Count() == 2) {
        return values[1] - values[0];
      }
      return CalcPlane(values, weights, 2).normal;
    }
  };
  template <class VT, class WT> struct Plane  {
    VT values;
    WT weights;
    Plane(const VT& _v, const WT& _w) : values(_v), weights(_w)  {}
    double calc() const {  return CalcPlane(values, weights).rmsd;  }
  };
  // torsion angle
  template <class aT, class bT, class cT, class dT> struct TorsionAngle  {
    aT a;
    bT b;
    cT c;
    dT d;
    TorsionAngle(const aT& _a, const bT& _b, const cT& _c, const dT& _d)
      : a(_a), b(_b), c(_c), d(_d)  {}
    double calc() const {
      return olx_dihedral_angle(a.evaluate(), b.evaluate(), c.evaluate(),
        d.evaluate());
      //return olx_dihedral_angle_signed(a.evaluate(), b.evaluate(),
      //  c.evaluate(), d.evaluate());
    }
    double calc_signed() const {
      return olx_dihedral_angle_signed(a.evaluate(), b.evaluate(),
        c.evaluate(), d.evaluate());
    }
  };
  template <class PT> struct TwistAngle {
    PT a, b;
    TwistAngle(const PT &a_, const PT &b_) : a(a_), b(b_) {}
    double calc() const {
      const PlaneInfo ai = a.evaluate(), bi = b.evaluate();
      return olx_dihedral_angle(ai.center + ai.normal,
        ai.center, bi.center, bi.center + bi.normal);
    }
  };
  template <class PT> struct FoldAngle {
    PT a, b;
    FoldAngle(const PT &a_, const PT &b_) : a(a_), b(b_) {}
    double calc() const {
      const PlaneInfo ai = a.evaluate(), bi = b.evaluate();
      vec3d n_c = (bi.center - ai.center).XProdVec(
        ai.normal + bi.normal).Normalise();
      vec3d p_a = ai.normal - n_c*n_c.DotProd(ai.normal);
      vec3d p_b = bi.normal - n_c*n_c.DotProd(bi.normal);
      return (acos(p_a.CAngle(p_b)) * 180 / M_PI);
    }
  };
  // tetrahedron volume
  template <class aT, class bT, class cT, class dT> struct TetrahedronVolume  {
    aT a;
    bT b;
    cT c;
    dT d;
    TetrahedronVolume(const aT& _a, const bT& _b, const cT& _c, const dT& _d)
      : a(_a), b(_b), c(_c), d(_d)  {}
    double calc() const {
      return olx_tetrahedron_volume(a.evaluate(), b.evaluate(), c.evaluate(),
        d.evaluate());
    }
  };
  // angle between 3 points
  template <class aT, class bT, class cT> struct Angle3  {
    aT a;
    bT b;
    cT c;
    Angle3(const aT& _a, const bT& _b, const cT& _c) : a(_a), b(_b), c(_c)  {}
    double calc() const {
      const vec3d c2 = b.evaluate();
      const double ca = (a.evaluate()-c2).CAngle(c.evaluate()-c2);
      if( olx_abs(ca) >= 1.0-1e-16 )
        return ca < 0 ? 180.0 : 0.0;
      return acos(ca)*180.0/M_PI;
    }
  };
  // angle between 2 vectors
  template <class aT, class bT> struct Angle2  {
    aT a;
    bT b;
    Angle2(const aT& _a, const bT& _b) : a(_a), b(_b)  {}
    double calc() const {
      const double ca = a.evaluate().CAngle(b.evaluate());
      if( olx_abs(ca) >= 1.0-1e-16 )
        return ca < 0 ? 180.0 : 0.0;
      return acos(ca)*180.0/M_PI;
    }
  };
  // point centroid to point distance
  template <class aT, class bT> struct Distance  {
    aT a;
    bT b;
    Distance(const aT& _a, const bT& _b) : a(_a), b(_b)  {}
    double calc() const {  return a.evaluate().DistanceTo(b.evaluate());  }
  };
  // octahedral distortion (in degrees)
  struct OctahedralDistortion  {
    const vec3d_alist& points;
    OctahedralDistortion(const vec3d_alist& _points)
      : points(_points)
    {}
    double calc() const {
      // centroid for first face
      const vec3d c1 = (points[1] + points[3] + points[5])/3;
      vec3d normal = vec3d::Normal(points, 1, 3, 5);
      // centroid for second face
      const vec3d c2 = (points[2] + points[4] + points[6])/3;
      double sum = 0;
      for (int i=1; i < 7; i += 2)  {
        vec3d v1 = points[i].Projection(c1, normal);
        vec3d v2 = points[i+1].Projection(c2, normal);
        sum += olx_abs(M_PI/3-acos(v1.CAngle(v2)));
      }
      return (sum*180/3)/M_PI;
    }
  };
  // octahedral distortion (in degrees), using best plane approach
  struct OctahedralDistortionBP  {
    const vec3d_alist& points;
    vec3d_alist pl;
    TDoubleList weights;
    OctahedralDistortionBP(const vec3d_alist& _points)
      : points(_points), pl(6), weights(6, olx_list_init::value(1.0))
    {}
    double calc() const {
      // translation for first face
      const vec3d c1 = (points[1] + points[3] + points[5])/3;
      // translation for second face
      const vec3d c2 = (points[2] + points[4] + points[6])/3;
      for( short i=0; i < 6; i+=2 )  {
        pl[i] = (points[i+1] - c1);
        pl[i+1] = (points[i+2] - c2);
      }
      const PlaneInfo pi = CalcPlane(pl, weights, 0);
      double sum = 0;
      for( short i=0; i < 6; i+=2 )  {
        const vec3d v1 = pl[i].Projection(pi.center, pi.normal);
        const vec3d v2 = pl[i+1].Projection(pi.center, pi.normal);
        sum += olx_abs(M_PI/3-acos(v1.CAngle(v2)));
      }
      return (sum*180/3)/M_PI;
    }
  };
  // triangle twist (in degrees), using best plane approach
  struct TriangleTwistBP  {
    const vec3d_alist& points;
    vec3d_alist pl;
    TDoubleList weights;
    mutable evecd angles;
    TriangleTwistBP(const vec3d_alist& _points)
      : points(_points), pl(6), weights(6, olx_list_init::value(1.0)),
      angles(3)
    {}
    double calc() const {
      // translation for first face
      const vec3d c1 = (points[0] + points[2] + points[4])/3;
      // translation for second face
      const vec3d c2 = (points[1] + points[3] + points[5])/3;
      for (short i=0; i < 6; i+=2) {
        pl[i] = (points[i] - c1);
        pl[i+1] = (points[i+1] - c2);
      }
      const PlaneInfo pi = CalcPlane(pl, weights, 0);
      for (short i=0; i < 6; i++) {
        pl[i] = pl[i].Projection(pi.center, pi.normal);
      }
      for (short i=0; i < 6; i+=2) {
        angles[i/2] = acos(pl[i].CAngle(pl[i+1]));
      }
      return (olx_sum(angles)*180/3)/M_PI;
    }
  };
protected:
  // helper functions
  template <class VC>
  double CalcEsd(const size_t sz, const mat3d_list& m, const VC& df)  {
    double esd = 0;
    for( size_t i=0; i < sz; i++ )  {
      for( size_t j=0; j < sz; j++ )  {
        const size_t m_ind = i*sz+j;
        for( short k=0; k < 3; k++ )  {
          for( short l=0; l < 3; l++ )  {
            esd += m[m_ind].Get(k,l)*df[i*3+k]*df[j*3+l];
          }
        }
      }
    }
    return esd < 0 ? 0 : esd;
  }
  // helper functions
  template <class list> void AtomsToPoints(const list& atoms, vec3d_alist& r)  {
    r.SetCount(atoms.Count());
    for( size_t i=0; i < atoms.Count(); i++ )
      r[i] = atoms[i]->crd();
  }
  //http://en.wikipedia.org/wiki/Numerical_differentiation
  template <class VecT, class OutVecT, class Evaluator>
  void CalcDiff(VecT& vars, OutVecT& df, const Evaluator& e)  {
    static const double delta=sqrt(2.2e-16);
    for( size_t i=0; i < vars.Count(); i++ )  {
      vars[i] += 2*delta;
      const double v1 = e.calc();
      vars[i] -= delta;
      const double v2 = e.calc();
      vars[i] -= 2*delta;
      const double v3 = e.calc();
      vars[i] -= delta;
      const double v4 = e.calc();
      df[i] = (-v1+8*v2-8*v3+v4)/(12*delta);
      vars[i] += 2*delta;
    }
  }
  struct CalcHelper  {
    VcoVContainer& base;
    vec3d_alist points;
    mat3d_list m;
    template <class list> CalcHelper(VcoVContainer& _base, const list& atoms) :
      base(_base), points(atoms.Count())
    {
      base.GetVcoV(atoms, m);
      base.AtomsToPoints(atoms, points);
    }
    template <class Eval> TEValue<double> DoCalc(const Eval& e)  {
      return base.DoCalcForPoints(points, m, e);
    }
  };
  struct CalcWHelper  {
    VcoVContainer& base;
    vec3d_alist points;
    TDoubleList weights;
    mat3d_list m;
    template <class list> CalcWHelper(VcoVContainer& _base, const list& atms) :
      base(_base), points(atms.Count()),
        weights(atms.Count(), olx_list_init::value(1.0))
    {
      base.GetVcoV(atms, m);
      base.AtomsToPoints(atms, points);
    }
    template <class Eval> TEValue<double> DoCalc(const Eval& e)  {
      return base.DoCalcForPoints(points, m, e);
    }
  };
  template <class list, typename eval>
  TEValue<double> DoCalcForPoints(list& points, const mat3d_list& vcov,
    const eval& e)
  {
    CellEsd ced(*this, points);
    if (GetMatrix().IsEmpty())
      return TEValue<double>(e.calc(), sqrt(ced.DoCalc(e)));
    TDoubleList df(points.Count()*3);
    CompositeVector::CompositeVector_<list> pts(points);
    CalcDiff(pts, df, e);
    return TEValue<double>(e.calc(),
      sqrt(CalcEsd(points.Count(), vcov, df) + ced.DoCalc(e)));
  }
  typedef ConstSlice<vec3d_alist> crd_slice;
  typedef ConstSlice<TDoubleList> weight_slice;
  typedef CentroidEvaluator<crd_slice,weight_slice> cnt_et;
  typedef PointProxy pnt_pt;
  typedef PlaneEvaluator<crd_slice,weight_slice> pln_et;
  typedef PlaneNormalEvaluator<crd_slice,weight_slice> plnn_et;
  typedef LineEvaluator<crd_slice, weight_slice> ln_et;
public:
  VcoVContainer(TAsymmUnit& _au) : au(_au), cell(6), celle(6)  {
    static const double a2r = M_PI/180;
    cell[0] = au.GetAxes()[0];  celle[0] = olx_sqr(au.GetAxisEsds()[0]);
    cell[1] = au.GetAxes()[1];  celle[1] = olx_sqr(au.GetAxisEsds()[1]);
    cell[2] = au.GetAxes()[2];  celle[2] = olx_sqr(au.GetAxisEsds()[2]);
    cell[3] = au.GetAngles()[0]*a2r;
    celle[3] = olx_sqr(au.GetAngleEsds()[0]*a2r);
    cell[4] = au.GetAngles()[1]*a2r;
    celle[4] = olx_sqr(au.GetAngleEsds()[1]*a2r);
    cell[5] = au.GetAngles()[2]*a2r;
    celle[5] = olx_sqr(au.GetAngleEsds()[2]*a2r);
  }
  void ReadShelxMat(const olxstr& fileName) {
    vcov.ReadShelxMat(fileName, au);
  }
  void ReadSmtbxMat(const olxstr& fileName) {
    vcov.ReadSmtbxMat(fileName, au);
  }
  void FromCIF() { vcov.FromCIF(au); }
  // precise calculation
  TEValue<double> CalcDistance(const TSAtom& a1, const TSAtom& a2) {
    TSAtom const* as[] = {&a1,&a2};
    CalcHelper ch(*this, ConstPlainVector<const TSAtom*>(as, 2));
    mat3d vcov = ch.m[0] - ch.m[1] - ch.m[2] + ch.m[3];
    const vec3d v = ch.points[0] - ch.points[1];
    const double val = v.Length();
    double qesd = (vcov*v).DotProd(v)/olx_sqr(val);
    qesd += CellEsd(*this, ch.points).DoCalc(
      Distance<pnt_pt,pnt_pt>(pnt_pt(ch.points[0]), pnt_pt(ch.points[1])));
    return TEValue<double>(val, sqrt(qesd));
  }
  // cartesian centroid
  TEPoint3<double> CalcCentroid(const TSAtomCPList& atoms) {
    CalcHelper ch(*this, atoms);
    mat3d vcov;
    vec3d cnt;
    for( size_t i=0; i < atoms.Count(); i++ )  {
      cnt += atoms[i]->crd();
      for( size_t j=0; j < atoms.Count(); j++ )
        vcov += ch.m[i*atoms.Count()+j];
    }
    vcov *= 1./olx_sqr(atoms.Count());
    cnt /= atoms.Count();
    return TEPoint3<double>(
      TEValueD(cnt[0], sqrt(vcov[0][0])),
      TEValueD(cnt[1], sqrt(vcov[1][1])),
      TEValueD(cnt[2], sqrt(vcov[2][2])));
  }
  // fractional centroid
  TEPoint3<double> CalcCentroidF(const TSAtomCPList& atoms) {
    CalcHelper ch(*this, atoms);
    mat3d vcov;
    vec3d cnt;
    for( size_t i=0; i < atoms.Count(); i++ )  {
      cnt += atoms[i]->ccrd();
      for( size_t j=0; j < atoms.Count(); j++ )
        vcov += ch.m[i*atoms.Count()+j];
    }
    vcov *= 1./olx_sqr(atoms.Count());
    cnt /= atoms.Count();
    return TEPoint3<double>(
      TEValueD(cnt[0], sqrt(vcov[0][0])),
      TEValueD(cnt[1], sqrt(vcov[1][1])),
      TEValueD(cnt[2], sqrt(vcov[2][2])));
  }
  // analytical, http://salilab.org/modeller/8v0/manual/node248.html
  TEValue<double> CalcAngleA(const TSAtom& a1, const TSAtom& a2,
    const TSAtom& a3)
  {
    TSAtom const * as[] = {&a1,&a2,&a3};
    CalcHelper ch(*this, ConstPlainVector<const TSAtom*>(as, 3));
    vec3d ij = (ch.points[0] - ch.points[1]);
    vec3d kj = (ch.points[2] - ch.points[1]);
    vec3d_alist grad(3);
    const double ca = ij.CAngle(kj);
    const double cell_esd = CellEsd(*this, ch.points).DoCalc(
      Angle3<pnt_pt,pnt_pt,pnt_pt>(
        pnt_pt(ch.points[0]), pnt_pt(ch.points[1]),
        pnt_pt(ch.points[2])))/olx_sqr(180/M_PI);
    if( olx_abs(ca) >= 1.0-1e-16 )
      return TEValue<double>(ca < 0 ? 180.0 : 0.0, sqrt(cell_esd)*180/M_PI);
    const double oos = 1./sqrt(1-ca*ca);
    const double ij_l = ij.Length(), kj_l = kj.Length();
    ij.Normalise();
    kj.Normalise();
    grad[0] = (ij*ca - kj)*oos/ij_l;
    grad[2] = (kj*ca - ij)*oos/kj_l;
    grad[1] = -(grad[0] + grad[2]);
    double qesd = CalcEsd(3, ch.m, CompositeVector::Make(grad));
    qesd += cell_esd;
    const double a = acos(ca);
    return TEValue<double>(a,(qesd < 1e-15 ? 0 : sqrt(qesd)))*=180/M_PI;
  }
  TEValue<double> CalcAngle(const TSAtom& a1, const TSAtom& a2,
    const TSAtom& a3)
  {
    TSAtom const * as[] = {&a1,&a2,&a3};
    CalcHelper ch(*this, ConstPlainVector<const TSAtom*>(as, 3));
    return ch.DoCalc(
      Angle3<pnt_pt,pnt_pt,pnt_pt>(pnt_pt(ch.points[0]), pnt_pt(ch.points[1]),
        pnt_pt(ch.points[2])));
  }
  TEValue<double> CalcAngle(const TSAtomCPList& a1, const TSAtomCPList& a2,
    const TSAtomCPList& a3)
  {
    CalcWHelper ch(*this, TSAtomCPList(a1) << a2 << a3);
    return ch.DoCalc(
      Angle3<cnt_et, cnt_et, cnt_et>(
      cnt_et(crd_slice(ch.points, 0, a1.Count()),
        weight_slice(ch.weights, 0, a1.Count())),
      cnt_et(crd_slice(ch.points, a1.Count(), a2.Count()),
        weight_slice(ch.weights, a1.Count(), a2.Count())),
      cnt_et(crd_slice(ch.points, a1.Count()+a2.Count(), a3.Count()),
        weight_slice(ch.weights, a1.Count()+a2.Count(), a3.Count()))
      ));
  }
  // analytical,http://salilab.org/modeller/8v0/manual/node248.html
  TEValue<double> CalcTAngleA(const TSAtom& a1, const TSAtom& a2,
    const TSAtom& a3, const TSAtom& a4)
  {
    TSAtom const * as[] = {&a1,&a2,&a3,&a4};
    CalcHelper ch(*this, ConstPlainVector<const TSAtom*>(as, 4));
    const vec3d ij = a1.crd() - a2.crd();
    const vec3d kj = a3.crd() - a2.crd();
    const vec3d kl = a4.crd() - a3.crd();
    const vec3d mj = ij.XProdVec(kj);
    const vec3d nk = kj.XProdVec(kl);
    const double kj_ql = kj.QLength();
    const double kj_l = sqrt(kj_ql);
    vec3d_alist grad(4);
    grad[0] = mj*(kj_l/mj.QLength());
    grad[3] = -(nk*(kj_l/nk.QLength()));
    grad[1] =
      grad[0]*(ij.DotProd(kj)/kj_ql -1.0) - grad[3]*(kl.DotProd(kj)/kj_ql);
    grad[2] =
      grad[3]*(kl.DotProd(kj)/kj_ql -1.0) - grad[0]*(ij.DotProd(kj)/kj_ql);
    double esd = CalcEsd(4, ch.m, CompositeVector::Make(grad));
    esd += CellEsd(*this, ch.points).DoCalc(
      TorsionAngle<pnt_pt,pnt_pt,pnt_pt,pnt_pt>(
        pnt_pt(ch.points[0]), pnt_pt(ch.points[1]),
        pnt_pt(ch.points[2]), pnt_pt(ch.points[3])))/olx_sqr(180.0/M_PI);
    const double a =
      olx_dihedral_angle_signed(a1.crd(), a2.crd(), a3.crd(), a4.crd());
    return TEValue<double>(a, (esd < 1e-15 ? 0 : sqrt(esd))*180/M_PI);
  }
  // torsion angle
  TEValue<double> CalcTAngle(const TSAtom& a1, const TSAtom& a2,
    const TSAtom& a3, const TSAtom& a4)
  {
    TSAtom const * as[] = {&a1,&a2,&a3,&a4};
    CalcHelper ch(*this, ConstPlainVector<const TSAtom*>(as, 4));
    TorsionAngle<pnt_pt,pnt_pt,pnt_pt,pnt_pt> tha(
      pnt_pt(ch.points[0]), pnt_pt(ch.points[1]), pnt_pt(ch.points[2]),
      pnt_pt(ch.points[3]));
    TEValueD rv = ch.DoCalc(tha);
    rv.V() = tha.calc_signed();
    return rv;
  }
  // bond to bond angle
  TEValue<double> CalcB2BAngle(const TSAtom& a1, const TSAtom& a2,
    const TSAtom& a3, const TSAtom& a4)
  {
    TSAtom const * as[] = {&a1,&a2,&a3,&a4};
    CalcHelper ch(*this, ConstPlainVector<const TSAtom*>(as, 4));
    return ch.DoCalc(
      Angle2<VectorProxy,VectorProxy>(
        VectorProxy(ch.points[0], ch.points[1]),
        VectorProxy(ch.points[2], ch.points[3])));
  }
  // returns rms for the best plane
  TEValue<double> CalcPlane(const TSAtomCPList& atoms) {
    CalcWHelper ch(*this, atoms);
    return ch.DoCalc(Plane<vec3d_alist, TDoubleList>(ch.points, ch.weights));
  }
  // plane to atom distance
  TEValue<double> CalcP2ADistance(const TSAtomCPList& atoms, const TSAtom& a) {
    CalcWHelper ch(*this, TSAtomCPList(atoms) << a);
    return ch.DoCalc(
      PlaneToPointDistance<pln_et,pnt_pt>(
        pln_et(crd_slice(ch.points, 0, atoms.Count()),
          weight_slice(ch.weights, 0, atoms.Count())),
        pnt_pt(ch.points.GetLast())));
  }
  // plane centroid to atom distance
  TEValue<double> CalcPC2ADistance(const TSAtomCPList& plane,
    const TSAtom& a)
  {
    CalcWHelper ch(*this, TSAtomCPList(plane) << a);
    return ch.DoCalc(
      Distance<cnt_et,pnt_pt>(
        cnt_et(crd_slice(ch.points, 0, plane.Count()),
          weight_slice(ch.weights, 0, plane.Count())),
        pnt_pt(ch.points.GetLast())));
  }
  // centroid to centroid
  TEValue<double> CalcC2CDistance(const TSAtomCPList& c1,
    const TSAtomCPList& c2)
  {
    CalcWHelper ch(*this, TSAtomCPList(c1) << c2);
    return ch.DoCalc(
      Distance<cnt_et, cnt_et>(
      cnt_et(crd_slice(ch.points, 0, c1.Count()),
      weight_slice(ch.weights, 0, c1.Count())),
      cnt_et(crd_slice(ch.points, c1.Count(), c2.Count()),
      weight_slice(ch.weights, c1.Count(), c2.Count()))));
  }
  // plane to a vector angle
  TEValue<double> CalcP2VAngle(const TSAtomCPList& plane, const TSAtom& a1,
    const TSAtom& a2)
  {
    CalcWHelper ch(*this, TSAtomCPList(plane) << a1 << a2);
    return ch.DoCalc(
      Angle2<plnn_et, VectorProxy>(
        plnn_et(crd_slice(ch.points, 0, plane.Count()), weight_slice(ch.weights,
          0, plane.Count())),
        VectorProxy(ch.points[plane.Count()], ch.points[plane.Count()+1])));
  }
  TEValue<double> CalcP2VAngle(const TSAtomCPList& plane,
    const TSAtomCPList &ln)
  {
    CalcWHelper ch(*this, TSAtomCPList(plane) << ln);
    return ch.DoCalc(
      Angle2<plnn_et, ln_et>(
      plnn_et(crd_slice(ch.points, 0, plane.Count()),
        weight_slice(ch.weights, 0, plane.Count())),
      ln_et(crd_slice(ch.points, plane.Count(), ln.Count()),
        weight_slice(ch.weights, plane.Count(), ln.Count()))));
  }
  // plane to plane angle
  TEValue<double> CalcP2PAngle(const TSAtomCPList& p1, const TSAtomCPList& p2)
  {
    CalcWHelper ch(*this, TSAtomCPList(p1) << p2);
    return ch.DoCalc(
      Angle2<plnn_et,plnn_et>(
        plnn_et(crd_slice(ch.points, 0, p1.Count()),
          weight_slice(ch.weights, 0, p1.Count())),
        plnn_et(crd_slice(ch.points, p1.Count(), p2.Count()),
          weight_slice(ch.weights, p1.Count(), p2.Count()))));
  }
  // plane to plane twisting angle
  TEValue<double> CalcP2PTAngle(const TSAtomCPList& p1, const TSAtomCPList& p2)
  {
    CalcWHelper ch(*this, TSAtomCPList(p1) << p2);
    return ch.DoCalc(
      TwistAngle<pln_et>(
      pln_et(crd_slice(ch.points, 0, p1.Count()),
        weight_slice(ch.weights, 0, p1.Count())),
      pln_et(crd_slice(ch.points, p1.Count(), p2.Count()),
        weight_slice(ch.weights, p1.Count(), p2.Count()))));
  }
  // plane to plane folding angle
  TEValue<double> CalcP2PFAngle(const TSAtomCPList& p1, const TSAtomCPList& p2)
  {
    CalcWHelper ch(*this, TSAtomCPList(p1) << p2);
    return ch.DoCalc(
      FoldAngle<pln_et>(
      pln_et(crd_slice(ch.points, 0, p1.Count()),
        weight_slice(ch.weights, 0, p1.Count())),
      pln_et(crd_slice(ch.points, p1.Count(), p2.Count()),
        weight_slice(ch.weights, p1.Count(), p2.Count()))));
  }
  //plane centroid to plane centroid distance
  TEValue<double> CalcPC2PCDistance(const TSAtomCPList& p1,
    const TSAtomCPList& p2)
  {
    CalcWHelper ch(*this, TSAtomCPList(p1) << p2);
    return ch.DoCalc(
      Distance<cnt_et, cnt_et>(
        cnt_et(crd_slice(ch.points, 0, p1.Count()), weight_slice(ch.weights,
          0, p1.Count())),
        cnt_et(crd_slice(ch.points, p1.Count(), p2.Count()),
          weight_slice(ch.weights, p1.Count(), p2.Count()))));
  }
  // angle between 3 plane centroids
  TEValue<double> Calc3PCAngle(const TSAtomCPList& p1, const TSAtomCPList& p2,
    const TSAtomCPList& p3)
  {
    CalcWHelper ch(*this, TSAtomCPList(p1) << p2 << p3);
    return ch.DoCalc(
      Angle3<cnt_et, cnt_et, cnt_et>(
        cnt_et(crd_slice(ch.points, 0, p1.Count()),
          weight_slice(ch.weights, 0, p1.Count())),
        cnt_et(crd_slice(ch.points, p1.Count(), p2.Count()),
          weight_slice(ch.weights, p1.Count(), p2.Count())),
        cnt_et(crd_slice(ch.points, p1.Count()+p2.Count(), p3.Count()),
          weight_slice(ch.weights, p1.Count()+p2.Count(), p3.Count()))));
  }
  // angle between plane centroid, atom, plane centroid
  TEValue<double> CalcPCAPCAngle(const TSAtomCPList& p1, const TSAtom &a,
    const TSAtomCPList& p2)
  {
    CalcWHelper ch(*this, TSAtomCPList(p1) << p2 << a);
    return ch.DoCalc(
      Angle3<cnt_et, pnt_pt, cnt_et>(
        cnt_et(crd_slice(ch.points, 0, p1.Count()),
          weight_slice(ch.weights, 0, p1.Count())),
        pnt_pt(ch.points.GetLast()),
        cnt_et(crd_slice(ch.points, p1.Count(), p2.Count()),
          weight_slice(ch.weights, p1.Count(), p2.Count()))));
  }
  //plane to another plane centroid distance
  TEValue<double> CalcP2PCDistance(const TSAtomCPList& p1,
    const TSAtomCPList& p2)
  {
    CalcWHelper ch(*this, TSAtomCPList(p1) << p2);
    return ch.DoCalc(
      PlaneToPointDistance<pln_et, cnt_et>(
        pln_et(crd_slice(ch.points, 0, p1.Count()),
          weight_slice(ch.weights, 0, p1.Count())),
        cnt_et(crd_slice(ch.points, p1.Count(), p2.Count()),
          weight_slice(ch.weights, p1.Count(), p2.Count()))));
  }
  //plane to another plane shift distance
  TEValue<double> CalcP2PShiftDistance(const TSAtomCPList& p1,
    const TSAtomCPList& p2)
  {
    CalcWHelper ch(*this, TSAtomCPList(p1) << p2);
    return ch.DoCalc(
      Centroid2CentriodShiftDistance<pln_et, cnt_et>(
        pln_et(crd_slice(ch.points, 0, p1.Count()),
          weight_slice(ch.weights, 0, p1.Count())),
        cnt_et(crd_slice(ch.points, p1.Count(), p2.Count()),
          weight_slice(ch.weights, p1.Count(), p2.Count()))));
  }
  // tetrahedron volume
  TEValue<double> CalcTetrahedronVolume(const TSAtom& a1, const TSAtom& a2,
    const TSAtom& a3, const TSAtom& a4)
  {
    TSAtom const * as[] = {&a1,&a2,&a3,&a4};
    CalcHelper ch(*this, ConstPlainVector<const TSAtom*>(as, 4));
    return ch.DoCalc(
      TetrahedronVolume<pnt_pt,pnt_pt,pnt_pt,pnt_pt>(
        pnt_pt(ch.points[0]), pnt_pt(ch.points[1]), pnt_pt(ch.points[2]),
        pnt_pt(ch.points[3])));
  }
  // alignment RMSD crds should be prepeared, i.e. inverted
  TEValue<double> CalcAlignmentRMSD(const TSAtomCPList& atoms,
    const vec3d_alist& crds, const TDoubleList& weights)
  {
    CalcHelper ch(*this, atoms);
    return ch.DoCalc(AlignmentRMSD(ch.points, weights));
  }
  /* octahedral distortion, takes {Central Atom, a1, b1, a2, b2, a3, b3},
  returns mean value in degrees. This calculation projects second set of points
  onto the plane defined be the firts set
  */
  TEValue<double> CalcOHDistortion(const TSAtomCPList& atoms)  {
    CalcHelper ch(*this, atoms);
    return ch.DoCalc(OctahedralDistortion(ch.points));
  }
  /* octahedral distortion, takes {Central Atom, a1, b1, a2, b2, a3, b3},
  returns mean value in degrees. This function uses mean plane defined by
  the 6 points arranged around the origin
  */
  TEValue<double> CalcOHDistortionBP(const TSAtomCPList& atoms)  {
    CalcHelper ch(*this, atoms);
    return ch.DoCalc(OctahedralDistortionBP(ch.points));
  }
  /* twist between trianglua planes. This function uses mean plane defined by
  the 6 points arranged around the origin
  */
  TEValue<double> CalcTraingluarTwist(const TSAtomCPList& atoms)  {
    CalcHelper ch(*this, atoms);
    return ch.DoCalc(TriangleTwistBP(ch.points));
  }
  const VcoVMatrix& GetMatrix() const {  return vcov;  }
};

EndXlibNamespace()
#endif
