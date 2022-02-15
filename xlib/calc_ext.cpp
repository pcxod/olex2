/******************************************************************************
* Copyright (c) 2004-2014 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "calc_ext.h"
#include "xapp.h"
#include "vcov.h"
#include "olxvar.h"
#include "lattice.h"
#include "unitcell.h"
#include "cif.h"
#include "symmparser.h"
#include "mat_id.h"
#include "pers_util.h"
//.............................................................................
CalculatedVars::Object::Object(const CalculatedVars::Object &o,
  CalculatedVars &parent)
  : name(o.name),
  type(o.type),
  group_id(o.group_id),
  obj_id(o.obj_id)
{
  atoms.SetCapacity(o.atoms.Count());
  for (size_t i = 0; i < o.atoms.Count(); i++) {
    atoms.Add(new TGroupCAtom(
      parent.rm.aunit.FindCAtomById(o.atoms[i].GetAtom()->GetId()),
      parent.GetEqiv(*o.atoms[i].GetMatrix())));
  }
}
//.............................................................................
olxstr CalculatedVars::Object::GetTypeName() const {
  if (type == cv_ot_centroid) {
    if (atoms.Count() == 1) {
      return "site";
    }
    return "centroid";
  }
  else if (type == cv_ot_line) {
    return "line";
  }
  return "plane";
}
//.............................................................................
TDataItem& CalculatedVars::Object::ToDataItem(TDataItem &i_, bool use_id) const {
  TDataItem *i;
  if (type == cv_ot_centroid) {
    i = &i_.AddItem("centroid");
  }
  else if (type == cv_ot_line) {
    i = &i_.AddItem("line");
  }
  else {
    i = &i_.AddItem("plane");
  }
  i->AddField("name", name);
  olxstr atoms_str;
  for (size_t j = 0; j < atoms.Count(); j++) {
    if (atoms[j].GetAtom()->IsDeleted()) continue;
    atoms_str << ',';
    if (use_id) {
      atoms_str << atoms[j].GetAtom()->GetTag();
    }
    else {
      atoms_str << atoms[j].GetAtom()->GetResiLabel();
    }
    atoms_str << '-' << atoms[j].GetMatrix()->GetId();
  }
  i->AddField("atoms", atoms_str.SubStringFrom(1));
  return *i;
}
//.............................................................................
uint16_t CalculatedVars::Object::DecodeType(const olxstr &v) {
  if (v.Equals("plane")) {
    return cv_ot_plane;
  }
  else if (v.Equals("line")) {
    return cv_ot_line;
  }
  else if (v.Equals("centroid") || v.Equals("site")) {
    return cv_ot_centroid;
  }
  throw TInvalidArgumentException(__OlxSourceInfo, "object type");
}
//.............................................................................
olx_object_ptr<CalculatedVars::Object>
CalculatedVars::Object::FromDataItem(const TDataItem &di,
  CalculatedVars &parent, bool use_id)
{
  olx_object_ptr<Object> x;
  if (di.GetName().Equals("plane")) {
    x = new Plane();
  }
  else {
    x = new Object(DecodeType(di.GetName()));
  }
  x->FromDataItem_(di, parent, use_id);
  return x;
}
//.............................................................................
void CalculatedVars::Object::FromDataItem_(const TDataItem &di,
  CalculatedVars &parent, bool use_id)
{
  atoms.Clear();
  TStrList at(di.GetFieldByName("atoms"), ',');
  for (size_t i = 0; i < at.Count(); i++) {
    size_t mi = at[i].IndexOf('-');
    if (use_id) {
      atoms.Add(new TGroupCAtom(
        parent.rm.aunit.FindCAtomById(at[i].SubStringTo(mi).ToSizeT()),
        parent.GetEqiv(at[i].SubStringFrom(mi + 1).ToUInt())));
    }
    else {
      atoms.Add(new TGroupCAtom(
        parent.rm.aunit.FindCAtom(at[i].SubStringTo(mi)),
        parent.GetEqiv(at[i].SubStringFrom(mi + 1).ToUInt())));
    }
    if (atoms.GetLast().GetAtom() == 0) {
      throw TInvalidArgumentException(__OlxSourceInfo, "atom reference");
    }
  }
  name = di.GetFieldByName("name");
}
//.............................................................................
ConstPtrList<TSAtom> CalculatedVars::Object::GetAtoms() const {
  TSAtomPList rv;
  if (atoms.IsEmpty()) {
    return rv;
  }
  TAsymmUnit &au = *atoms[0].GetAtom()->GetParent();
  for (size_t i = 0; i < atoms.Count(); i++) {
    if (atoms[i].GetAtom()->IsDeleted()) {
      continue;
    }
    TSAtom *a = new TSAtom(0);
    a->CAtom(*atoms[i].GetAtom());
    a->_SetMatrix(*atoms[i].GetMatrix());
    a->ccrd() = *atoms[i].GetMatrix()*atoms[i].GetAtom()->ccrd();
    a->crd() = au.Orthogonalise(a->ccrd());
    rv.Add(a);
  }
  return rv;
}
//.............................................................................
bool CalculatedVars::Object::IsValid() const {
  for (size_t i = 0; i < atoms.Count(); i++) {
    if (atoms[i].GetAtom()->IsDeleted()) {
      return false;
    }
  }
  return true;
}
//.............................................................................
struct SymmAtom {
  const TCAtom *atom;
  const smatd *symm;
  SymmAtom(const TCAtom* atom, const smatd* symm)
    : atom(atom), symm(symm)
  {}
  int Compare(const SymmAtom& o) const {
    int r = olx_cmp(atom->GetId(), o.atom->GetId());
    if (r == 0) {
      r = olx_cmp(symm->GetId(), o.symm->GetId());
    }
    return r;
  }
};
bool CalculatedVars::Object::SameAtoms(const CalculatedVars::Object& o,
  bool order_matters) const
{
  if (o.atoms.Count() != atoms.Count()) {
    return false;
  }
  if (order_matters) {
    for (size_t i = 0; i < atoms.Count(); i++) {
      if (atoms[i].GetAtom() != o.atoms[i].GetAtom()){
        return false;
      }
      if (atoms[i].GetMatrix() != o.atoms[i].GetMatrix()) {
        return false;
      }
    }
    return true;
  }
  olxset<SymmAtom, TComparableComparator> aset;
  for (size_t i = 0; i < atoms.Count(); i++) {
    aset.Add(SymmAtom(atoms[i].GetAtom(), atoms[i].GetMatrix()));
  }
  for (size_t i = 0; i < atoms.Count(); i++) {
    if (!aset.Contains(SymmAtom(o.atoms[i].GetAtom(), o.atoms[i].GetMatrix()))) {
      return false;
    }
  }
  return true;
}
//.............................................................................
CalculatedVars::Object *CalculatedVars::Object::create(TSAtom &a,
  CalculatedVars &parent)
{
  CalculatedVars::Object *p = new CalculatedVars::Object(cv_ot_centroid);
  p->atoms.Add(new TGroupCAtom(&a.CAtom(), parent.GetEqiv(a.GetMatrix())));
  p->name << a.CAtom().GetResiLabel() << a.GetMatrix().GetId();
  p = &parent.AddObject(p);
  return p;
}
//.............................................................................
CalculatedVars::Object *CalculatedVars::Object::create(TSBond &b,
  CalculatedVars &parent)
{
  CalculatedVars::Object *p = new CalculatedVars::Object(cv_ot_line);
  p->atoms.Add(new TGroupCAtom(&b.A().CAtom(),
    parent.GetEqiv(b.A().GetMatrix())));
  p->atoms.Add(new TGroupCAtom(&b.B().CAtom(),
    parent.GetEqiv(b.B().GetMatrix())));

  p->name << b.A().CAtom().GetResiLabel() << b.B().CAtom().GetResiLabel() <<
    (b.A().GetMatrix().GetId() + b.B().GetMatrix().GetId());

  p = &parent.AddObject(p);
  return p;
}
//.............................................................................
CalculatedVars::Object *CalculatedVars::Object::create(TSPlane &sp,
  CalculatedVars &parent)
{
  CalculatedVars::Plane *p = new CalculatedVars::Plane();
  size_t si = 0;
  for (size_t i = 0; i < sp.Count(); i++) {
    p->atoms.Add(new TGroupCAtom(
      &sp.GetAtom(i).CAtom(),
      parent.GetEqiv(sp.GetAtom(i).GetMatrix())));
    si += sp.GetAtom(i).GetMatrix().GetId();
    p->name << sp.GetAtom(i).CAtom().GetResiLabel();
  }
  p->name << si;
  p->normal = sp.GetNormal();
  return &parent.AddObject(p);
}
//.............................................................................
CalculatedVars::Object* CalculatedVars::Object::Clone(
  CalculatedVars::Object& o, CalculatedVars& parent)
{
  if (o.type == cv_ot_plane) {
    return new Plane(dynamic_cast<Plane&>(o), parent);
  }
  return new Object(o, parent);
}
//.............................................................................
//.............................................................................
TDataItem& CalculatedVars::Plane::ToDataItem(TDataItem &i_, bool use_id) const {
  TDataItem& i = Object::ToDataItem(i_, use_id);
  i.AddField("normal", PersUtil::VecToStr(normal));
  return i;
}
//.............................................................................
void CalculatedVars::Plane::FromDataItem_(const TDataItem &i,
  CalculatedVars &parent, bool use_id)
{
  Object::FromDataItem_(i, parent, use_id);
  normal = PersUtil::VecFromStr<vec3d>(i.GetFieldByName("normal"));
}
//.............................................................................
olxstr CalculatedVars::ObjectRef::GetName() const {
  if (object.type == cv_ot_plane) {
    if (prop == 'n') {
      return "plane normal";
    }
    if (prop == 'c') {
      return "plane centroid";
    }
  }
  else if (object.type == cv_ot_line) {
    if (prop == 'c') {
      return "line center";
    }
  }
  return object.GetTypeName();
}
//.............................................................................
//.............................................................................
//.............................................................................
olxstr CalculatedVars::Var::GetName() const {
  if (type == cv_vt_distance) {
    return "distance";
  }
  if (type == cv_vt_angle) {
    if (refs.Count() == 4) {
      return "dihedral angle";
    }
    return "angle";
  }
  return "shift";
}
//.............................................................................
void CalculatedVars::Var::ToDataItem(TDataItem &i_) const {
  TDataItem &i = i_.AddItem("var");
  i.AddField("name", name);
  olxstr value;
  if (type == cv_vt_distance) {
    value = "distance";
  }
  else if (type == cv_vt_angle) {
    value = "angle";
  }
  else if (type == cv_vt_shift) {
    value = "shift";
  }
  for (size_t i = 0; i < refs.Count(); i++) {
    value << ' ' << refs[i].GetQualifiedName();
  }
  i.AddField("value", value);
}
//.............................................................................
olx_pair_t<olxstr, olxstr> CalculatedVars::Var::parseObject(const olxstr &on) {
  olx_pair_t<olxstr, olxstr> rv(on);
  size_t di = on.IndexOf('.');
  if (di == InvalidIndex) {
    throw TInvalidArgumentException(__OlxSourceInfo, "object name");
  }
  size_t di1 = on.FirstIndexOf('.', di + 1);
  if (di1 != InvalidIndex) {
    rv.b = on.SubStringFrom(di1 + 1);
    rv.a = on.SubStringTo(di1);
  }
  return rv;
}
//.............................................................................
CalculatedVars::Var *CalculatedVars::Var::Clone(const Var &v,
  CalculatedVars &parent)
{
  Var *rv = new Var(v.type, v.name);
  for (size_t i = 0; i < v.refs.Count(); i++) {
    rv->refs.Add(new ObjectRef(
      *parent.objects[v.refs[i].object.GetQualifiedName()], v.refs[i].prop));
  }
  return rv;
}
//.............................................................................
uint16_t CalculatedVars::Var::DecodeType(const olxstr &v) {
  if (v.Equals("distance")) {
    return cv_vt_distance;
  }
  if (v.Equals("angle")) {
    return cv_vt_angle;
  }
  if (v.Equals("shift")) {
    return cv_vt_shift;
  }
  throw TInvalidArgumentException(__OlxSourceInfo, "object type");
}
//.............................................................................
CalculatedVars::Var *CalculatedVars::Var::FromDataItem(const TDataItem &i,
  CalculatedVars &parent)
{
  TStrList vt(i.GetFieldByName("value"), ' ');
  if (vt.Count() < 2) {
    throw TInvalidArgumentException(__OlxSourceInfo, "value");
  }
  Var *v = new Var(DecodeType(vt[0]), i.GetFieldByName("name"));
  for (size_t i = 1; i < vt.Count(); i++) {
    olx_pair_t<olxstr, olxstr> o = parseObject(vt[i]);
    v->refs.Add(new ObjectRef(*parent.objects[o.a], o.b));
  }
  return v;
}
//.............................................................................
bool CalculatedVars::Var::ToCIF(olx_pdict<uint16_t, cif_dp::cetTable*> out)
  const
{
  using namespace cif_dp;
  cetTable& tab = *out[type];
  CifRow& r = tab.AddRow();
  switch (type) {
  case cv_vt_shift:
  case cv_vt_distance:
  {
    r[0] = new cetString(refs[0].object.obj_id);
    r[1] = new cetString(refs[1].object.obj_id);
    r[2] = new cetString(value.ToString());
    r[3] = new cetString(GetName() << ":"
      << refs[0].GetName() << ", " << refs[1].GetName());
    r[4] = new cetString(".");
  }
  break;
  case cv_vt_angle:
  {
    r[0] = new cetString(refs[0].object.obj_id);
    r[1] = new cetString(refs[1].object.obj_id);
    if (refs.Count() < 3) {
      r[2] = new cetString(".");
    }
    else {
      r[2] = new cetString(refs[2].object.obj_id);
    }
    r[3] = new cetString(value.ToString());
    olxstr descr = GetName() << ":"
      << refs[0].GetName() << ", " << refs[1].GetName();
    if (refs.Count() > 2) {
      descr << ", " << refs[2].GetName();
    }
    if (refs.Count() > 3) {
      descr << ", " << refs[3].GetName();
    }
    r[4] = new cetString(descr);
    r[5] = new cetString("yes");
  }
  break;
  }
  return true;
}
//.............................................................................
TEValueD CalculatedVars::Var::Calculate(class VcoVContainer &vcov) const {
  TArrayList<TSAtomCPList> atoms(refs.Count());
  TLinkedList<IOlxObject*, NewCleanup> obj_store;
  for (size_t i = 0; i < refs.Count(); i++) {
    TSAtomPList tmp = refs[i].object.GetAtoms();
    atoms[i] = tmp;
    obj_store.AddAll(tmp);
  }
  TEValueD rv;
  rv.E() = -1;
  if (refs.Count() == 2) {
    ObjectRef &r1 = refs[0], &r2 = refs[1];
    if (type == cv_vt_distance) {
      if (r1.object.type == cv_ot_centroid && r2.object.type == cv_ot_centroid) {
        rv = vcov.CalcC2CDistance(atoms[0], atoms[1]);
      }
      else if (r1.object.type == cv_ot_plane && r2.object.type == cv_ot_plane) {
        if (r1.prop.Equalsi('c') && r2.prop.Equalsi('c')) {
          rv = vcov.CalcC2CDistance(atoms[0], atoms[1]);
        }
        else if ((r1.prop.Equalsi('c') && r2.prop.IsEmpty()) ||
          (r2.prop.Equalsi('c') && r1.prop.IsEmpty()))
        {
          if (r1.prop.IsEmpty()) {
            rv = vcov.CalcP2PCDistance(atoms[1], atoms[0]);
          }
          else {
            rv = vcov.CalcP2PCDistance(atoms[0], atoms[1]);
          }
        }
      }
      else if (r1.object.type == cv_ot_line && r2.object.type == cv_ot_centroid) {
        if (r1.prop.Equalsi('c')) {
          rv = vcov.CalcC2CDistance(atoms[0], atoms[1]);
        }
        else if(atoms[0].Count() == 2 && atoms[1].Count() == 1) {
          rv = vcov.CalcAtomToVectorDistance(*atoms[1][0], *atoms[0][0], *atoms[0][1]);
        }
      }
    }
    else if (type == cv_vt_shift) {
      if (r1.object.type == cv_ot_plane && r2.object.type == cv_ot_plane) {
        if ((r1.prop.Equalsi('c') && r2.prop.IsEmpty()) ||
          (r2.prop.Equalsi('c') && r1.prop.IsEmpty()))
        {
          if (r1.prop.IsEmpty()) {
            rv = vcov.CalcP2PShiftDistance(atoms[1], atoms[0]);
          }
          else {
            rv = vcov.CalcP2PShiftDistance(atoms[0], atoms[1]);
          }
        }
      }
      else if ((r1.object.type == cv_ot_plane && r2.object.type == cv_ot_centroid) ||
        (r2.object.type == cv_ot_plane && r1.object.type == cv_ot_centroid))
      {
        if (r1.object.type == cv_ot_plane) {
          rv = vcov.CalcP2PShiftDistance(atoms[0], atoms[1]);
        }
        else {
          rv = vcov.CalcP2PShiftDistance(atoms[1], atoms[0]);
        }
      }
    }
    else {
      if (r1.object.type == cv_ot_plane && r2.object.type == cv_ot_plane) {
        Plane &p1 = dynamic_cast<CalculatedVars::Plane &>(r1.object),
          &p2 = dynamic_cast<CalculatedVars::Plane &>(r2.object);
        if (r1.prop.Equalsi('n') && r1.prop.Equalsi('n')) {
          rv = vcov.CalcP2PAngle(atoms[0], p1.normal, atoms[1], p2.normal);
        }
        else if (r1.prop.IsEmpty() && r2.prop.IsEmpty()) {
          rv = vcov.CalcP2PAngle(atoms[0], p1.normal, atoms[1], p2.normal);
          rv.V() = 180 - rv.GetV();
        }
      }
      else if ((r1.object.type == cv_ot_plane && r2.object.type == cv_ot_line) ||
        (r2.object.type == cv_ot_plane && r1.object.type == cv_ot_line))
      {
        const ObjectRef &plane = (r1.object.type == cv_ot_plane ? r1 : r2);
        const ObjectRef &line = (r1.object.type == cv_ot_plane ? r2 : r1);
        if (r1.object.type == cv_ot_plane) {
          Plane &p = dynamic_cast<CalculatedVars::Plane &>(r1.object);
          rv = vcov.CalcP2VAngle(atoms[0], p.normal, atoms[1]);
        }
        else {
          Plane &p = dynamic_cast<CalculatedVars::Plane &>(r2.object);
          rv = vcov.CalcP2VAngle(atoms[1], p.normal, atoms[0]);
        }
        if (plane.prop.IsEmpty()) {
          rv.V() = 180 - rv.GetV();
        }
      }
    }
  }
  else if (refs.Count() == 3) {
    if (type == cv_vt_angle &&
      refs[0].object.type == cv_ot_centroid &&
      refs[1].object.type == cv_ot_centroid &&
      refs[2].object.type == cv_ot_centroid)
    {
      rv = vcov.CalcAngle(atoms[0], atoms[1], atoms[2]);
    }
    if (type == cv_vt_angle &&
      refs[0].object.type == cv_ot_plane &&
      refs[1].object.type == cv_ot_plane &&
      refs[2].object.type == cv_ot_plane)
    {
      rv = vcov.CalcAngle(atoms[0], atoms[1], atoms[2]);
    }
  }
  else if (refs.Count() == 4) {
    if (type == cv_vt_angle &&
      refs[0].object.type == cv_ot_centroid &&
      refs[1].object.type == cv_ot_centroid &&
      refs[2].object.type == cv_ot_centroid &&
      refs[3].object.type == cv_ot_centroid)
    {
      rv = vcov.CalcTAngle(*atoms[0][0], *atoms[1][0], *atoms[2][0], *atoms[3][0]);
    }
  }
  return (value = rv);
}
//.............................................................................
//.............................................................................
//.............................................................................
CalculatedVars::CalculatedVars(RefinementModel &rm)
  : rm(rm)
{}
//.............................................................................
void CalculatedVars::Assign(const CalculatedVars &cv) {
  Clear();
  for (size_t i = 0; i < cv.objects.Count(); i++) {
    Object *o = Object::Clone(*cv.objects.GetValue(i), *this);
    objects.Add(o->GetQualifiedName(), o);
  }
  for (size_t i = 0; i < cv.vars.Count(); i++) {
    Var *v = Var::Clone(*cv.vars.GetValue(i), *this);
    vars.Add(v->name, v);
  }
}
//.............................................................................
smatd * CalculatedVars::GetEqiv(const smatd & m) {
  uint64_t id = full_smatd_id<>::get(m);
  size_t ei = eqivs.IndexOf(id);
  if (ei != InvalidIndex) {
    return eqivs.GetValue(ei);
  }
  return eqivs.Add(id, new smatd(m));
}
//.............................................................................
void CalculatedVars::Clear() {
  for (size_t i = 0; i < vars.Count(); i++) {
    delete vars.GetValue(i);
  }
  vars.Clear();
  for (size_t i = 0; i < objects.Count(); i++) {
    delete objects.GetValue(i);
  }
  objects.Clear();
  for (size_t i = 0; i < eqivs.Count(); i++) {
    delete eqivs.GetValue(i);
  }
  eqivs.Clear();
}
//.............................................................................
void CalculatedVars::CalcAll(olx_object_ptr<VcoVContainer> vcovc) const {
  if (vars.IsEmpty()) {
    return;
  }
  if (!vcovc.ok()) {
    TXApp& app = TXApp::GetInstance();
    vcovc = new VcoVContainer(app.XFile().GetAsymmUnit());
    try {
      olxstr src_mat = app.InitVcoV(*vcovc);
      app.NewLogEntry() << "Using " << src_mat << " matrix for the calculation";
    }
    catch (TExceptionBase& e) {
      throw TFunctionFailedException(__OlxSourceInfo, e,
        "could not initialise");
    }
  }
  for (size_t i = 0; i < vars.Count(); i++) {
    olxstr vn = olxstr("olex2.calculated.") << vars.GetKey(i);
    TEValueD v = vars.GetValue(i)->Calculate(vcovc);
    if (v.GetE() < 0) {
      TBasicApp::NewLogEntry(logError) << "Failed to calculate '" << vn << '\'';
    }
    olxstr strv = v.ToString();
    TOlxVars::SetVar(vn, strv);
    TBasicApp::NewLogEntry(logInfo) << vn << '=' << strv;
  }
}
//.............................................................................
void CalculatedVars::ToDataItem(TDataItem &di, bool use_id) const {
  olxstr equivs_str;
  for (size_t i = 0; i < eqivs.Count(); i++) {
    equivs_str << ';' << TSymmParser::MatrixToSymmEx(*eqivs.GetValue(i));
    eqivs.GetValue(i)->SetRawId((uint32_t)i);
  }
  di.AddField("equivs", equivs_str.IsEmpty() ? equivs_str
    : equivs_str.SubStringFrom(1));
  TDataItem &oi = di.AddItem("objects");
  for (size_t i = 0; i < objects.Count(); i++) {
    if (objects.GetValue(i)->GetRefCount() > 0) {
      objects.GetValue(i)->ToDataItem(oi, use_id);
    }
  }
  TDataItem &vi = di.AddItem("vars");
  for (size_t i = 0; i < vars.Count(); i++) {
    vars.GetValue(i)->ToDataItem(vi);
  }
}
//.............................................................................
void CalculatedVars::FromDataItem(const TDataItem &di, bool use_id) {
  Clear();
  TStrList et(di.GetFieldByName("equivs"), ';');
  for (size_t i = 0; i < et.Count(); i++) {
    smatd m = TSymmParser::SymmToMatrix(et[i]);
    eqivs.Add(full_smatd_id<>::get(m), new smatd(m));
  }
  TDataItem &os = di.GetItemByName("objects");
  for (size_t i = 0; i < os.ItemCount(); i++) {
    olx_object_ptr<Object> o = Object::FromDataItem(
      os.GetItemByIndex(i), *this, use_id);
    Object *& ao = objects.Add(o->GetQualifiedName(), &o);
    if (ao != &o) {
      delete ao;
      ao = &o;
    }
    o.release();
  }
  TDataItem &vs = di.GetItemByName("vars");
  for (size_t i = 0; i < vs.ItemCount(); i++) {
    Var *v = Var::FromDataItem(vs.GetItemByIndex(i), *this);
    Var *& av = vars.Add(v->name, v);
    if (v != av) {
      delete av;
      av = v;
    }
  }
}
//.............................................................................
CalculatedVars::Object &CalculatedVars::AddObject(CalculatedVars::Object *o) {
  Object *& ao = objects.Add(o->GetQualifiedName(), o);
  if (ao != o) {
    delete o;
  }
  return *ao;
}
//.............................................................................
CalculatedVars::Var &CalculatedVars::AddVar(Var *v) {
  Var *& av = vars.Add(v->name, v);
  if (v != av) {
    delete av;
    av = v;
  }
  return *av;
}
//.............................................................................
bool CalculatedVars::Validate() const {
  for (size_t i = 0; i < vars.Count(); i++) {
    if (!vars.GetValue(i)->IsValid()) {
      delete vars.GetValue(i);
      vars.Delete(i--);
    }
  }
  for (size_t i = 0; i < objects.Count(); i++) {
    if (objects.GetValue(i)->GetRefCount() == 0) {
      delete objects.GetValue(i);
      objects.Delete(i--);
    }
  }
  for (size_t i = 0; i < eqivs.Count(); i++) {
    eqivs.GetValue(i)->SetRawId(0);
  }
  for (size_t i = 0; i < objects.Count(); i++) {
    Object &o = *objects.GetValue(i);
    for (size_t j = 0; j < o.atoms.Count(); j++) {
      const_cast<smatd *>(o.atoms[j].GetMatrix())->SetRawId(1);
    }
  }
  for (size_t i = 0; i < eqivs.Count(); i++) {
    if (eqivs.GetValue(i)->GetId() == 0) {
      delete eqivs.GetValue(i);
      eqivs.Delete(i--);
    }
  }
  return !vars.IsEmpty();
}
//.............................................................................
struct ObjType {
  size_t idx;
  uint16_t type;
  ObjType(size_t idx, uint16_t type)
    : idx(idx), type(type)
  {}
  int Compare(const ObjType& o) const {
    int r = olx_cmp(idx, o.idx);
    if (r == 0) {
      r = olx_cmp(type, o.type);
    }
    return r;
  }
};
TTypeList<cif_dp::cetTable>::const_list_type
CalculatedVars::ToCIF(const TCif& cif) const
{
  TTypeList<cif_dp::cetTable> rv;
  if (vars.IsEmpty()) {
    return rv;
  }
  for (size_t i = 0; i < eqivs.Count(); i++) {
    TUnitCell::InitMatrixId(cif.GetMatrices(), *eqivs.GetValue(i));
  }
  using namespace cif_dp;
  olx_object_ptr<cetTable> site_defs = new cetTable(
  "_geom_measure_group_id,_geom_measure_site_label,_geom_measure_site_symmetry");
  olx_object_ptr<cetTable> obj_defs = new cetTable(
    "_geom_object_id,_geom_object_measure_group_id,_geom_object_type");

  // build site map
  olxdict<const Object*, size_t, TPointerComparator> site_map;
  TPtrList<const Object> sites;
  for (size_t i = 0; i < objects.Count(); i++) {
    size_t idx = InvalidIndex;
    for (size_t j = 0; j < site_map.Count(); j++) {
      if (site_map.GetKey(j)->SameAtoms(*objects.GetValue(i), false)) {
        idx = site_map.GetValue(j);
        break;
      }
    }
    if (idx == InvalidIndex) {
      size_t id = site_map.Count();
      site_map.Add(objects.GetValue(i), id);
      objects.GetValue(i)->group_id = id;
      sites.Add(objects.GetValue(i));
    }
    else {
      site_map.Add(objects.GetValue(i), idx);
      objects.GetValue(i)->group_id = idx;
    }
  }
  for (size_t i = 0; i < sites.Count(); i++) {
    const Object* obj = sites[i];
    for (size_t j = 0; j < obj->atoms.Count(); j++) {
      CifRow& r = site_defs->AddRow();
      r[0] = new cetString(obj->group_id);
      r[1] = new AtomCifEntry(*obj->atoms[j].GetAtom());
      r[2] = new SymmCifEntry(cif, *obj->atoms[j].GetMatrix());
    }
  }
  // build object map
  olxdict<ObjType, size_t, TComparableComparator> obj_map;
  for (size_t i = 0; i < objects.Count(); i++) {
    ObjType ot(objects.GetValue(i)->group_id, objects.GetValue(i)->type);
    size_t idx = obj_map.IndexOf(ot);
    if (idx == InvalidIndex) {
      size_t id = obj_map.Count();
      obj_map.Add(ot, id);
      objects.GetValue(i)->obj_id = id;
      CifRow& r = obj_defs->AddRow();
      r[0] = new cetString(id);
      r[1] = new cetString(objects.GetValue(i)->group_id);
      r[2] = new cetString(objects.GetValue(i)->GetTypeName());
    }
    else {
      objects.GetValue(i)->obj_id = obj_map.GetValue(idx);
    }
  }

  olx_object_ptr<cetTable> dist(new cetTable(
    "_geom_measure_distance_object_1,_geom_measure_distance_object_2,"
    "_geom_measure_distance,_geom_measure_distance_details,"
    "_geom_measure_distance_publ_flag"));

  olx_object_ptr<cetTable> sdist(new cetTable(
    "_geom_measure_shift_object_1,_geom_measure_shift_object_2,"
    "_geom_measure_shift_distance,_geom_measure_shift_details,"
    "_geom_measure_shift_publ_flag"));

  olx_object_ptr<cetTable> ang(new cetTable(
    "_geom_measure_angle_object_1,_geom_measure_angle_object_2,_geom_measure_angle_object_3,"
    "_geom_measure_angle,_geom_measure_angle_details,"
    "_geom_measure_angle_publ_flag"));

  rv.Add(site_defs.release());
  rv.Add(obj_defs.release());
  olx_pdict<uint16_t, cetTable*> measurements;
  measurements.Add(cv_vt_distance, &dist);
  measurements.Add(cv_vt_shift, &sdist);
  measurements.Add(cv_vt_angle, &ang);
  for (size_t i = 0; i < vars.Count(); i++) {
    if (vars.GetValue(i)->value.GetE() >= 0) {
      vars.GetValue(i)->ToCIF(measurements);
    }
  }
  if (dist->RowCount() != 0) {
    rv.Add(dist.release());
  }
  if (sdist->RowCount() != 0) {
    rv.Add(sdist.release());
  }
  if (ang->RowCount() != 0) {
    rv.Add(ang.release());
  }
  return rv;
}
