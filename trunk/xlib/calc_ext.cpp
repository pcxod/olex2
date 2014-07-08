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
#include "symmparser.h"
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
  for (size_t i = 0; i < atoms.Count(); i++) {
    if (atoms[i].GetAtom()->IsDeleted())
      return false;
  }
  return true;
}
//.............................................................................
CalculatedVars::Object *CalculatedVars::Object::create(TSAtom &a,
  CalculatedVars &parent)
{
  CalculatedVars::Object *p = new CalculatedVars::Object();
  p->atoms.Add(new TGroupCAtom(&a.CAtom(), parent.GetEqiv(a.GetMatrix())));
  p->name << a.CAtom().GetResiLabel() << a.GetMatrix().GetId();
  p->type = cv_ot_centroid;
  p = &parent.AddObject(p);
  return p;
}
//.............................................................................
CalculatedVars::Object *CalculatedVars::Object::create(TSBond &b,
  CalculatedVars &parent)
{
  CalculatedVars::Object *p = new CalculatedVars::Object();
  p->atoms.Add(new TGroupCAtom(&b.A().CAtom(),
    parent.GetEqiv(b.A().GetMatrix())));
  p->atoms.Add(new TGroupCAtom(&b.B().CAtom(),
    parent.GetEqiv(b.B().GetMatrix())));

  p->name << b.A().CAtom().GetResiLabel() << b.B().CAtom().GetResiLabel() <<
    (b.A().GetMatrix().GetId() + b.B().GetMatrix().GetId());

  p->type = cv_ot_line;
  p = &parent.AddObject(p);
  return p;
}
//.............................................................................
CalculatedVars::Object *CalculatedVars::Object::create(TSPlane &sp,
  CalculatedVars &parent)
{
  CalculatedVars::Object *p = new CalculatedVars::Object();
  size_t si = 0;
  for (size_t i = 0; i < sp.Count(); i++) {
    p->atoms.Add(new TGroupCAtom(
      &sp.GetAtom(i).CAtom(),
      parent.GetEqiv(sp.GetAtom(i).GetMatrix())));
    si += sp.GetAtom(i).GetMatrix().GetId();
    p->name << sp.GetAtom(i).CAtom().GetResiLabel();
  }
  p->name << si;
  p->type = cv_ot_plane;
  p = &parent.AddObject(p);
  return p;
}
//.............................................................................
//.............................................................................
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
  else {
    value = "shift";
  }
  for (size_t i = 0; i < refs.Count(); i++) {
    value << ' ' << refs[i].GetQualifiedName();
  }
  i.AddField("value", value);
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
  Var *rv = new Var(v.name);
  rv->type = v.type;
  for (size_t i = 0; i < v.refs.Count(); i++) {
    rv->refs.Add(new ObjectRef(
      *parent.objects[v.refs[i].object.GetQualifiedName()], v.refs[i].prop));
  }
  return rv;
}
//.............................................................................
CalculatedVars::Var *CalculatedVars::Var::FromDataItem(const TDataItem &i,
  CalculatedVars &parent)
{
  TStrList vt(i.GetFieldByName("value"), ' ');
  if (vt.Count() < 2) {
    throw TInvalidArgumentException(__OlxSourceInfo, "value");
  }
  Var *v = new Var(i.GetFieldByName("name"));
  for (size_t i = 1; i < vt.Count(); i++) {
    olx_pair_t<olxstr, olxstr> o = parseObject(vt[i]);
    v->refs.Add(new ObjectRef(*parent.objects[o.a], o.b));
  }
  if (vt[0].Equals("distance"))
    v->type = cv_vt_distance;
  else if (vt[0].Equals("angle"))
    v->type = cv_vt_angle;
  else
    v->type = cv_vt_shift;
  return v;
}
//.............................................................................
TEValueD CalculatedVars::Var::Calculate(class VcoVContainer &vcov) const {
  TArrayList<TSAtomCPList> atoms(refs.Count());
  for (size_t i = 0; i < refs.Count(); i++) {
    atoms[i] = refs[i].object.GetAtoms();
  }
  TEValueD rv;
  rv.E() = -1;
  if (refs.Count() == 2) {
    ObjectRef &r1 = refs[0], &r2 = refs[1];
    if (type == cv_vt_distance) {
      if (r1.object.type == cv_ot_centroid && r2.object.type == cv_ot_centroid) {
        rv = vcov.CalcC2CDistance(atoms[0], atoms[1]);
      }
      if (r1.object.type == cv_ot_plane && r2.object.type == cv_ot_plane) {
        if (r1.prop.Equalsi('c') && r2.prop.Equalsi('c')) {
          rv = vcov.CalcC2CDistance(atoms[0], atoms[1]);
        }
        else if ((r1.prop.Equalsi('c') && r2.prop.IsEmpty()) ||
          (r2.prop.Equalsi('c') && r1.prop.IsEmpty()))
        {
          if (r1.prop.IsEmpty())
            rv = vcov.CalcP2PCDistance(atoms[1], atoms[0]);
          else
            rv = vcov.CalcP2PCDistance(atoms[0], atoms[1]);
        }
      }
    }
    else if (type == cv_vt_shift) {
      if (r1.object.type == cv_ot_plane && r2.object.type == cv_ot_plane) {
        if ((r1.prop.Equalsi('c') && r2.prop.IsEmpty()) ||
          (r2.prop.Equalsi('c') && r1.prop.IsEmpty()))
        {
          if (r1.prop.IsEmpty())
            rv = vcov.CalcP2PShiftDistance(atoms[1], atoms[0]);
          else
            rv = vcov.CalcP2PShiftDistance(atoms[0], atoms[1]);
        }
      }
      else if ((r1.object.type == cv_ot_plane && r2.object.type == cv_ot_centroid) ||
        (r2.object.type == cv_ot_plane && r1.object.type == cv_ot_centroid))
      {
        if (r1.object.type == cv_ot_plane)
          rv = vcov.CalcP2PShiftDistance(atoms[0], atoms[1]);
        else
          rv = vcov.CalcP2PShiftDistance(atoms[1], atoms[0]);
      }
    }
    else {
      if (r1.object.type == cv_ot_plane && r2.object.type == cv_ot_plane) {
        if (r1.prop.Equalsi('n') && r1.prop.Equalsi('n')) {
          rv = vcov.CalcP2PAngle(atoms[0], atoms[1]);
        }
        else if (r1.prop.IsEmpty() && r2.prop.IsEmpty()) {
          rv = vcov.CalcP2PAngle(atoms[0], atoms[1]);
          rv.V() = 180 - rv.GetV();
        }
      }
      else if ((r1.object.type == cv_ot_plane && r2.object.type == cv_ot_line) ||
        (r2.object.type == cv_ot_plane && r1.object.type == cv_ot_line))
      {
        const ObjectRef &plane = (r1.object.type == cv_ot_plane ? r1 : r2);
        const ObjectRef &line = (r1.object.type == cv_ot_plane ? r2 : r1);
        if (r1.object.type == cv_ot_plane)
          rv = vcov.CalcP2VAngle(atoms[0], atoms[1]);
        else
          rv = vcov.CalcP2VAngle(atoms[1], atoms[0]);
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
  }
  for (size_t i = 0; i < atoms.Count(); i++) {
    atoms[i].DeleteItems();
  }
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
    olxstr vn = olxstr("olex2.calculated.") << vars.GetKey(i);
    TEValueD v = vars.GetValue(i)->Calculate(vcovc);
    if (v.GetE() < 0) {
      TBasicApp::NewLogEntry(logError) << "Failed to calculate '" << vn << '\'';
    }
    olxstr strv = v.ToString();
    TOlxVars::SetVar(vn, strv);
    TBasicApp::NewLogEntry() << vn << '=' << strv;
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
