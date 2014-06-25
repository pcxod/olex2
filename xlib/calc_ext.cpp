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
#include "mat_id.h"
//.............................................................................
CalculatedVars::Object::Object(const CalculatedVars::Object &o,
  CalculatedVars &parent)
{
  atoms.Clear();
  name = o.name;
  type = o.type;
  atoms.SetCapacity(o.atoms.Count());
  for (size_t i = 0; i < o.atoms.Count(); i++) {
    atoms.Add(new TGroupCAtom(
      parent.rm.aunit.FindCAtomById(o.atoms[i].GetAtom()->GetId()),
      parent.GetEqiv(*o.atoms[i].GetMatrix())));
  }
}
//.............................................................................
olxstr CalculatedVars::Object::GetQualifiedName() const {
  olxstr rv;
  if (type == cv_ot_centroid) {
    rv = "centroid";
  }
  else if (type == cv_ot_line) {
    rv = "line";
  }
  else {
    rv = "plane";
  }
  return rv << '.' << name;
}
//.............................................................................
void CalculatedVars::Object::ToDataItem(TDataItem &i_, bool use_id) const {
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
}
//.............................................................................
void CalculatedVars::Object::FromDataItem(const TDataItem &di,
  CalculatedVars &parent, bool use_id)
{
  if (di.GetName().Equals("plane")) {
    type = cv_ot_plane;
  }
  else if (di.GetName().Equals("line")) {
    type = cv_ot_line;
  }
  else if (di.GetName().Equals("centroid")) {
    type = cv_ot_centroid;
  }
  else {
    throw TInvalidArgumentException(__OlxSourceInfo, "object type");
  }
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
    if (atoms.GetLast().GetAtom() == NULL) {
      throw TInvalidArgumentException(__OlxSourceInfo, "atom reference");
    }
  }
  name = di.GetFieldByName("name");
}
//.............................................................................
ConstPtrList<const TSAtom> CalculatedVars::Object::GetAtoms() const {
  TSAtomCPList rv;
  if (atoms.IsEmpty()) return rv;
  TAsymmUnit &au = *atoms[0].GetAtom()->GetParent();
  for (size_t i = 0; i < atoms.Count(); i++) {
    if (atoms[i].GetAtom()->IsDeleted()) continue;
    TSAtom *a = new TSAtom(NULL);
    a->CAtom(*atoms[i].GetAtom());
    a->_SetMatrix(atoms[i].GetMatrix());
    a->ccrd() = *atoms[i].GetMatrix()*atoms[i].GetAtom()->ccrd();
    a->crd() = au.Orthogonalise(a->ccrd());
    rv.Add(a);
  }
  return rv;
}
//.............................................................................
bool CalculatedVars::Object::IsValid() const {
  size_t ac = 0;
  for (size_t i = 0; i < atoms.Count(); i++) {
    if (!atoms[i].GetAtom()->IsDeleted())
      ac++;
  }
  if (type == cv_ot_centroid) {
    return ac > 0;
  }
  else if (type == cv_ot_line) {
    return ac > 1;
  }
  else {
    return ac > 2;
  }
}
//.............................................................................
//.............................................................................
//.............................................................................
void CalculatedVars::Var::ToDataItem(TDataItem &i_) const {
  TDataItem &i = i_.AddItem("var");
  i.AddField("name", name);
  olxstr value;
  if (type == cv_vt_distance) {
    value = "dist";
  }
  else {
    value = "angl";
  }
  i.AddField("value", value.stream(' ') << r1.GetQualifiedName() <<
    r2.GetQualifiedName());
}
//.............................................................................
olx_pair_t<olxstr, olxstr> CalculatedVars::Var::parseObject(const olxstr on) {
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
  Var *rv = new Var(v.name,
    ObjectRef(*parent.objects[v.r1.object.GetQualifiedName()], v.r1.prop),
    ObjectRef(*parent.objects[v.r2.object.GetQualifiedName()], v.r2.prop));
  rv->type = v.type;
  return rv;
}
//.............................................................................
CalculatedVars::Var *CalculatedVars::Var::FromDataItem(const TDataItem &i,
  CalculatedVars &parent)
{
  TStrList vt(i.GetFieldByName("value"), ' ');
  if (vt.Count() != 3) {
    throw TInvalidArgumentException(__OlxSourceInfo, "value");
  }
  olx_pair_t<olxstr, olxstr> o1 = parseObject(vt[1]),
    o2 = parseObject(vt[2]);
  Var *v = new Var(i.GetFieldByName("name"),
    ObjectRef(*parent.objects[o1.a], o1.b),
    ObjectRef(*parent.objects[o2.a], o2.b));
  if (vt[0].Equals("dist"))
    v->type = cv_vt_distance;
  else
    v->type = cv_vt_angle;
  return v;
}
//.............................................................................
TEValueD CalculatedVars::Var::Calculate(class VcoVContainer &vcov) const {
  TSAtomCPList l1 = r1.object.GetAtoms(),
    l2 = r2.object.GetAtoms();
  TEValueD rv;
  if (type == cv_vt_distance) {
    if (r1.object.type == cv_ot_centroid && r2.object.type == cv_ot_centroid) {
      rv = vcov.CalcC2CDistance(l1, l2);
    }
    if (r1.object.type == cv_ot_plane && r2.object.type == cv_ot_plane) {
      if (r1.prop.Equalsi('c') && r1.prop.Equalsi('c')) {
        rv = vcov.CalcC2CDistance(l1, l2);
      }
      else if ((r1.prop.Equalsi('c') && r2.prop.IsEmpty()) ||
        (r2.prop.Equalsi('c') && r1.prop.IsEmpty()))
      {
        if (r1.prop.IsEmpty())
          rv = vcov.CalcP2CDistance(l1, l2);
        else
          rv = vcov.CalcP2CDistance(l2, l1);
      }
    }
  }
  else {
    if (r1.object.type == cv_ot_plane && r2.object.type == cv_ot_plane) {
      if (r1.prop.Equalsi('n') && r1.prop.Equalsi('n')) {
        rv = vcov.CalcP2PAngle(l1, l2);
      }
      else if (r1.prop.IsEmpty() && r2.prop.IsEmpty())
      {
        rv = vcov.CalcP2PAngle(l1, l2);
        rv.V() = 180 - rv.GetV();
      }
    }
  }
  l1.DeleteItems();
  l2.DeleteItems();
  return rv;
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
    Object *o = new Object(*cv.objects.GetValue(i), *this);
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
  for (size_t i = 0; i < objects.Count(); i++) {
    delete objects.GetValue(i);
  }
  for (size_t i = 0; i < eqivs.Count(); i++) {
    delete eqivs.GetValue(i);
  }
  vars.Clear();
  objects.Clear();
  eqivs.Clear();
}
//.............................................................................
void CalculatedVars::CalcAll() const {
  TXApp &app = TXApp::GetInstance();
  VcoVContainer vcovc(app.XFile().GetAsymmUnit());
  try {
    olxstr src_mat = app.InitVcoV(vcovc);
    app.NewLogEntry() << "Using " << src_mat << " matrix for the calculation";
  }
  catch (TExceptionBase& e) {
    throw TFunctionFailedException(__OlxSourceInfo, e,
      "could not initialise ");
  }
  for (size_t i = 0; i < vars.Count(); i++) {
    TOlxVars::SetVar(olxstr("olex2.calculated.") << vars.GetKey(i),
      vars.GetValue(i)->Calculate(vcovc).ToString());
  }
}
//.............................................................................
void CalculatedVars::ToDataItem(TDataItem &di, bool use_id) const {
  olxstr equivs_str;
  for (size_t i = 0; i < eqivs.Count(); i++) {
    equivs_str << ',' << full_smatd_id<>::get(*eqivs.GetValue(i));
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
  TStrList et(di.GetFieldByName("equivs"), ',');
  for (size_t i = 0; i < et.Count(); i++) {
    uint64_t id = et[i].RadUInt<uint64_t>();
    eqivs.Add(id, new smatd(full_smatd_id<>::get(id)));
  }
  TDataItem &os = di.GetItemByName("objects");
  for (size_t i = 0; i < os.ItemCount(); i++) {
    Object *o = new Object;
    o->FromDataItem(os.GetItemByIndex(i), *this, use_id);
    Object *& ao = objects.Add(o->GetQualifiedName(), o);
    if (ao != o) {
      delete ao;
      ao = o;
    }
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
