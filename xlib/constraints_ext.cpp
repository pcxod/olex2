/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "refmodel.h"
#include "constraints_ext.h"
#include "atomref.h"
#include "pers_util.h"
#include "index_range.h"
#include "analysis.h"

void rotated_adp_constraint::FromToks(const TStrList& toks, RefinementModel& rm,
  TTypeList<rotated_adp_constraint>& out)
{
  if (toks.Count() < 4) {
    return;
  }
  TCAtom* src = rm.aunit.FindCAtom(toks[0]),
    * dest = rm.aunit.FindCAtom(toks[1]);
  if (src == 0 || dest == 0) {
    return;
  }
  adirection& dir = rm.DirectionById(toks[2]);
  bool refine_angle = false;
  const double angle = toks[3].ToDouble();
  if (toks.Count() > 4) {
    refine_angle = toks[4].ToBool();
  }
  out.Add(new rotated_adp_constraint(*src, *dest, dir, angle, refine_angle));
}
//.............................................................................
rotated_adp_constraint* rotated_adp_constraint::Copy(
  RefinementModel& rm, const rotated_adp_constraint& c)
{
  TCAtom* ref = rm.aunit.FindCAtomById(c.source.GetId());
  TCAtom* atom = rm.aunit.FindCAtomById(c.destination.GetId());
  if (ref == 0 || atom == 0) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "asymmetric units do not match");
  }
  return new rotated_adp_constraint(*ref,
    *atom, rm.DirectionById(c.dir.id), c.angle, c.refine_angle);
}
//.............................................................................
#ifdef _PYTHON
PyObject* rotated_adp_constraint::PyExport() const {
  return Py_BuildValue("i,i,O,d,b", source.GetTag(), destination.GetTag(),
    PythonExt::BuildString(dir.id), angle, refine_angle
  );
}
#endif
//.............................................................................
olxstr rotated_adp_constraint::ToInsStr(const RefinementModel& rm) const {
  return olxstr("", 64).stream(' ') << GetName() << source.GetResiLabel(true)
    << destination.GetResiLabel(true) << dir.id << angle << refine_angle;
}
//.............................................................................
const olxstr& rotated_adp_constraint::GetName()  {
  static olxstr name("olex2.constraint.rotated_adp");
  return name;
}
//.............................................................................
void rotated_adp_constraint::ToDataItem(TDataItem& di) const {
  di.AddField("source", source.GetTag())
    .AddField("destination", destination.GetTag())
    .AddField("angle", angle)
    .AddField("refine_angle", refine_angle)
    .AddField("dir_id", dir.id);
}
//.............................................................................
rotated_adp_constraint* rotated_adp_constraint::FromDataItem(
  const TDataItem& di, const RefinementModel& rm)
{
  return new rotated_adp_constraint(
    rm.aunit.GetAtom(di.GetFieldByName("source").ToSizeT()),
    rm.aunit.GetAtom(di.GetFieldByName("destination").ToSizeT()),
      rm.DirectionById(di.GetFieldByName("dir_id")),
      di.GetFieldByName("angle").ToDouble(),
      di.GetFieldByName("refine_angle").ToBool()
    );
}
//.............................................................................
void rotated_adp_constraint::UpdateParams(const TStrList& toks)  {
  if (toks.Count() != 1) {
    throw TInvalidArgumentException(__OlxSourceInfo, "argument number");
  }
  angle = toks[0].ToDouble();
}
//.............................................................................
olxstr rotated_adp_constraint::Describe() const {
  olxstr rv = "ADP of ";
  return rv << destination.GetLabel() << " is the ADP of " << source.GetLabel() <<
    " rotated " << angle << " degrees around " << dir.Describe();
}
//.............................................................................
//.............................................................................
//.............................................................................
void rotating_adp_constraint::FromToks(const TStrList& toks, RefinementModel& rm,
  TTypeList<rotating_adp_constraint>& out)
{
  if (toks.Count() < 6) {
    return;
  }
  TCAtom* src = rm.aunit.FindCAtom(toks[0]),
    * dest = rm.aunit.FindCAtom(toks[1]);
  if (src == 0 || dest == 0) {
    return;
  }
  double s = toks[2].ToDouble(), 
    a = toks[4].ToDouble(),
    b = toks[5].ToDouble(),
    g = toks[6].ToDouble();
  bool refine_size = toks[3].ToBool(),
    refine_angle = toks[7].ToBool();
  out.Add(new rotating_adp_constraint(*src, *dest, s, refine_size, a, b, g, refine_angle));
}
//.............................................................................
rotating_adp_constraint* rotating_adp_constraint::Copy(
  RefinementModel& rm, const rotating_adp_constraint& c)
{
  TCAtom* ref = rm.aunit.FindCAtomById(c.source.GetId());
  TCAtom* atom = rm.aunit.FindCAtomById(c.destination.GetId());
  if (ref == 0 || atom == 0) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "asymmetric units do not match");
  }
  return new rotating_adp_constraint(*ref,
    *atom, c.size, c.refine_size,
    c.alpha, c.beta, c.gamma, c.refine_angle);
}
//.............................................................................
#ifdef _PYTHON
PyObject* rotating_adp_constraint::PyExport() const {
  return Py_BuildValue("i,i,d,b,d,d,d,b", source.GetTag(), destination.GetTag(),
    size, refine_size, alpha, beta, gamma, refine_angle
  );
}
#endif
//.............................................................................
olxstr rotating_adp_constraint::ToInsStr(const RefinementModel& rm) const {
  return olxstr("", 64).stream(' ') << GetName() << source.GetLabel()
    << destination.GetLabel() << size << refine_size <<
    alpha << beta << gamma << refine_angle;
}
//.............................................................................
const olxstr& rotating_adp_constraint::GetName() {
  static olxstr name("olex2.constraint.rotating_adp");
  return name;
}
//.............................................................................
void rotating_adp_constraint::ToDataItem(TDataItem& di) const {
  di.AddField("source", source.GetTag())
    .AddField("destination", destination.GetTag())
    .AddField("size", size)
    .AddField("refine_size", refine_size)
    .AddField("alpha", size)
    .AddField("beta", size)
    .AddField("gamma", gamma)
    .AddField("refine_angle", refine_angle);
}
//.............................................................................
rotating_adp_constraint* rotating_adp_constraint::FromDataItem(
  const TDataItem& di, const RefinementModel& rm)
{
  return new rotating_adp_constraint(
    rm.aunit.GetAtom(di.GetFieldByName("source").ToSizeT()),
    rm.aunit.GetAtom(di.GetFieldByName("destination").ToSizeT()),
    di.GetFieldByName("size").ToDouble(),
    di.GetFieldByName("refine_size").ToBool(),
    di.GetFieldByName("alpha").ToDouble(),
    di.GetFieldByName("beta").ToDouble(),
    di.GetFieldByName("gamma").ToDouble(),
    di.GetFieldByName("refine_angle").ToBool()
  );
}
//.............................................................................
void rotating_adp_constraint::UpdateParams(const TStrList& toks) {
  if (toks.Count() != 4) {
    throw TInvalidArgumentException(__OlxSourceInfo, "argument number");
  }
  size = toks[0].ToDouble();
  alpha = toks[1].ToDouble();
  beta = toks[2].ToDouble();
  gamma = toks[3].ToDouble();
}
//.............................................................................
olxstr rotating_adp_constraint::Describe() const {
  olxstr rv = "ADP of ";
  return rv << destination.GetLabel() << " is the ADP of " << source.GetLabel() <<
    " scaled by " << size << " and rotated (" <<
    alpha << ", " << beta << ", " << gamma << ")";
}
//.............................................................................
//.............................................................................
//.............................................................................
const olxstr& adirection::GetName()  {
  static olxstr name("olex2.direction");
  return name;
}
//.............................................................................
const olxstr& adirection::EncodeType(uint16_t type) {
  if (type == direction_static) {
    return type_names()[0];
  }
  if (type == direction_vector) {
    return type_names()[1];
  }
  if (type == direction_normal) {
    return type_names()[2];
  }
  if (type == direction_centroid) {
    return type_names()[3];
  }
  throw TInvalidArgumentException(__OlxSourceInfo, olxstr("type=") << type);
}
//.............................................................................
uint16_t adirection::DecodeType(const olxstr& type) {
  if (type.Equalsi(type_names()[0])) {
    return direction_static;
  }
  if (type.Equalsi(type_names()[1])) {
    return direction_vector;
  }
  if (type.Equalsi(type_names()[2])) {
    return direction_normal;
  }
  if (type.Equalsi(type_names()[3])) {
    return direction_centroid;
  }
  throw TInvalidArgumentException(__OlxSourceInfo, olxstr("type=") << type);
}
//.............................................................................
void adirection::FromToks(const TStrList& toks, RefinementModel& rm,
  TTypeList<adirection>& out)
{
  if (toks.Count() < 4) {
    return;
  }
  try {
    uint16_t type = adirection::DecodeType(toks[0]);
    if (type == direction_static) {
      if (toks.Count() < 5) {
        TBasicApp::NewLogEntry(logError) <<
          "Too few parameters for a static direction";
        return;
      }
      out.Add(new static_direction(toks[1],
        vec3d(toks[2].ToDouble(), toks[3].ToDouble(), toks[4].ToDouble())));
      return;
    }
    TAtomReference aref(toks.Text(' ', 2));
    TCAtomGroup agroup;
    size_t atomAGroup;
    aref.Expand(rm, agroup, EmptyString(), atomAGroup);
    out.Add(new direction(toks[1], agroup, type));
  }
  catch (const TExceptionBase& ex) {
    TBasicApp::NewLogEntry(logException) << ex.GetException()->GetError();
  }
}
//.............................................................................
adirection* direction::DoCopy(RefinementModel& rm) const {
  TCAtomGroup agroup;
  agroup.SetCapacity(atoms.Count());
  for (size_t i = 0; i < atoms.Count(); i++) {
    TCAtom* a = rm.aunit.FindCAtomById(atoms[i].GetAtom()->GetId());
    if (a == 0) {
      throw TFunctionFailedException(__OlxSourceInfo, "asymmetric units do not match");
    }
    const smatd* m = 0;
    if (atoms[i].GetMatrix() != 0) {
      m = &rm.AddUsedSymm(*atoms[i].GetMatrix());
    }
    agroup.Add(new TGroupCAtom(a, m));
  }
  return new direction(id, agroup, type);
}
//.............................................................................
olxstr direction::Describe() const {
  olxstr rv;
  if (type == direction_vector) {
    rv = "vector through";
  }
  else if (type == direction_normal) {
    rv = "normal for";
  }
  else if (type == direction_centroid) {
    rv = "centroid for";
  }
  return rv << " [" << olx_analysis::alg::label(atoms, ',') << ']';
}
//.............................................................................
olxstr static_direction::Describe() const {
  olxstr rv = "vector";
  return  rv << " [" << value.ToString() << ']';
}
//.............................................................................
#ifdef _PYTHON
PyObject *static_direction::PyExport() const {
  return Py_BuildValue("s, s, (f,f,f)",
    PythonExt::BuildString(adirection::EncodeType(direction_static)),
    PythonExt::BuildString(id),
    direction_static, value[0], value[1], value[2]);
}
PyObject* direction::PyExport() const {
  size_t sz = 0;
  for (size_t i = 0; i < atoms.Count(); i++) {
    if (atoms[i].GetAtom()->IsDeleted()) {
      continue;
    }
    sz++;
  }
  PyObject* rv = PyTuple_New(sz + 2);
  PyTuple_SetItem(rv, 0, PythonExt::BuildString(adirection::EncodeType(type)));
  PyTuple_SetItem(rv, 1, PythonExt::BuildString(id));
  sz = 0;
  for (size_t i = 0; i < atoms.Count(); i++) {
    if (atoms[i].GetAtom()->IsDeleted()) {
      continue;
    }
    PyTuple_SetItem(rv, ++sz + 1, Py_BuildValue("(i,i)",
      atoms[i].GetAtom()->GetTag(),
      atoms[i].GetMatrix() == 0 ? -1 : atoms[i].GetMatrix()->GetId()));
  }
  return rv;
}
#endif
//.............................................................................
olxstr static_direction::ToInsStr(const RefinementModel& rm) const {
  return olxstr("", 64).stream(' ') << GetName()
    << adirection::EncodeType(direction_static)
    << id << value[0] << value[1] << value[2];
}
//.............................................................................
olxstr direction::ToInsStr(const RefinementModel& rm) const {
  olxstr rv("", 64);
  rv.stream(' ') << GetName() << adirection::EncodeType(type) << id;
  for (size_t i = 0; i < atoms.Count(); i++) {
    if (atoms[i].GetAtom()->IsDeleted()) {
      continue;
    }
    rv << ' ' << atoms[i].GetFullLabel(rm);
  }
  return rv;
}
//.............................................................................
void static_direction::ToDataItem(TDataItem& di) const {
  di.AddField("type", adirection::EncodeType(direction_static))
    .AddField("id", id)
    .AddField("value", PersUtil::VecToStr(value));
}
//.............................................................................
void direction::ToDataItem(TDataItem& di) const {
  di.AddField("type", adirection::EncodeType(type)).
    AddField("id", id);
  for (size_t i = 0; i < atoms.Count(); i++) {
    if (atoms[i].GetAtom()->IsDeleted()) {
      continue;
    }
    atoms[i].ToDataItem(di.AddItem("atom"));
  }
}
//.............................................................................
adirection* adirection::FromDataItem(const TDataItem& di,
  const RefinementModel& rm)
{
  uint16_t type = adirection::DecodeType(di.GetFieldByName("type"));
  if (type == direction_static) {
    return static_direction().CreateFromDataItem(di, rm);
  }
  else {
    return direction(type).CreateFromDataItem(di, rm);
  }
}
//.............................................................................
adirection* static_direction::CreateFromDataItem(const TDataItem& di,
  const RefinementModel& rm) const
{
  return new static_direction(di.GetFieldByName("id"),
    PersUtil::VecFromStr<vec3d>(di.GetFieldByName("value")));
}
//.............................................................................
adirection* direction::CreateFromDataItem(const TDataItem& di,
  const RefinementModel& rm) const
{
  uint16_t type = adirection::DecodeType(di.GetFieldByName("type"));
  TCAtomGroup agroup;
  agroup.SetCapacity(di.ItemCount());
  for (size_t i = 0; i < di.ItemCount(); i++) {
    agroup.Add(new TGroupCAtom(di.GetItemByIndex(i), rm));
  }
  return new direction(di.GetFieldByName("id"), agroup, type);
}
//.............................................................................
vec3d direction::get() const {
  if (atoms.IsEmpty()) {
    throw TFunctionFailedException(__OlxSourceInfo, "empty atom list");
  }
  if (type == direction_vector || type == direction_normal) {
    TAsymmUnit &au = *atoms[0].GetAtom()->GetParent();
    vec3d_list Points;
    Points.SetCapacity(atoms.Count());
    for (size_t i = 0; i < atoms.Count(); i++) {
      Points.AddNew(au.Orthogonalise(atoms[i].ccrd()));
    }
    plane::mean<>::out po = plane::mean<>::calc(Points);
    if (type == direction_vector) {
      return po.normals[2];
    }
    else {
      return po.normals[0];
    }
  }
  else if (type == direction_centroid) {
    TAsymmUnit &au = *atoms[0].GetAtom()->GetParent();
    vec3d cnt;
    for (size_t i = 0; i < atoms.Count(); i++) {
      cnt += au.Orthogonalise(atoms[i].ccrd());
    }
    return cnt / atoms.Count();
  }
  throw TFunctionFailedException(__OlxSourceInfo, "undefined object type");
}
//.............................................................................
//.............................................................................
//.............................................................................
void same_group_constraint::FromToks(const TStrList& toks, RefinementModel& rm,
  TTypeList<same_group_constraint>& out)
{
  if (toks.Count() < 7) {
    return;
  }
  size_t gc = toks[0].ToUInt();
  TAtomReference aref(toks.Text(' ', 1));
  TCAtomGroup agroup;
  size_t atomAGroup;
  aref.Expand(rm, agroup, EmptyString(), atomAGroup);
  if ((agroup.Count() % gc) != 0 || (agroup.Count() / gc < 3)) {
    throw TInvalidArgumentException(__OlxSourceInfo, "atom number");
  }
  same_group_constraint& g = *new same_group_constraint;
  atomAGroup = agroup.Count() / gc;
  for (size_t i = 0, cnt = 0; i < gc; i++) {
    TCAtomPList& l = g.groups.AddNew(atomAGroup);
    for (size_t j = 0; j < atomAGroup; j++, cnt++) {
      l[j] = agroup[cnt].GetAtom();
    }
  }
  out.Add(g);
}
//.............................................................................
same_group_constraint* same_group_constraint::Copy(
  RefinementModel& rm, const same_group_constraint& c)
{
  same_group_constraint& g = *new same_group_constraint;
  for (size_t i = 0; i < c.groups.Count(); i++) {
    TCAtomPList& l = g.groups.AddNew(c.groups[i].Count());
    for (size_t j = 0; j < c.groups[i].Count(); j++) {
      TCAtom* a = rm.aunit.FindCAtomById(c.groups[i][j]->GetId());
      if (a == 0) {
        delete& g;
        throw TFunctionFailedException(__OlxSourceInfo,
          "asymmetric units do not match");
      }
      l[j] = a;
    }
  }
  return &g;
}
//.............................................................................
#ifdef _PYTHON
PyObject* same_group_constraint::PyExport() const {
  PyObject* gs = PyTuple_New(groups.Count());
  for (size_t i = 0; i < groups.Count(); i++) {
    PyObject* g = PyTuple_New(groups[i].Count());
    for (size_t j = 0; j < groups[i].Count(); j++) {
      PyTuple_SetItem(g, j, Py_BuildValue("i", groups[i][j]->GetTag()));
    }
    PyTuple_SetItem(gs, i, g);
  }
  return gs;
}
#endif
//.............................................................................
olxstr same_group_constraint::ToInsStr(const RefinementModel& rm) const {
  olxstr rv;
  rv << GetName() << ' ' << groups.Count();
  for (size_t i = 0; i < groups.Count(); i++) {
    for (size_t j = 0; j < groups[i].Count(); j++) {
      rv << ' ' << groups[i][j]->GetResiLabel();
    }
  }
  return rv;
}
//.............................................................................
const olxstr& same_group_constraint::GetName()  {
  static olxstr name("olex2.constraint.same_group");
  return name;
}
//.............................................................................
void same_group_constraint::ToDataItem(TDataItem& di) const {
  di.SetValue(groups.Count());
  IndexRange::Builder rb;
  for (size_t i = 0; i < groups.Count(); i++) {
    for (size_t j = 0; j < groups[i].Count(); j++) {
      rb << groups[i][j]->GetTag();
    }
  }
  di.AddField("atoms", rb.GetString());
}
//.............................................................................
same_group_constraint* same_group_constraint::FromDataItem(
  const TDataItem& di, const RefinementModel& rm)
{
  size_t n = di.GetValue().ToSizeT();
  same_group_constraint &g = *new same_group_constraint;
  IndexRange::RangeItr ai(di.GetFieldByName("atoms"));
  size_t ag = n/ai.CalcSize();
  for( size_t i=0; i < n; i++ )  {
    TCAtomPList &l = g.groups.AddNew(ag);
    for (size_t j = 0; j < ag; j++) {
      l[j] = &rm.aunit.GetAtom(ai.Next());
    }
  }
  return &g;
}
//.............................................................................
void same_group_constraint::UpdateParams(const TStrList& toks)  {}
//.............................................................................
olxstr same_group_constraint::Describe() const {
  olxstr rv;
  for (size_t i=0; i < groups.Count(); i++) {
    if (!rv.IsEmpty()) {
      rv << ", ";
    }
    rv << '[' << olx_analysis::alg::label(groups[i], ',') << ']';
  }
  return olxstr('{') << rv << '}';
}
//.............................................................................
//.............................................................................
//.............................................................................
olxstr tls_group_constraint::ToInsStr(const RefinementModel& rm) const {
  olxstr_buf rv;
  olxstr ws(' ');
  rv << GetName();
  for (size_t i=0; i < atoms.Count(); i++) {
    rv << ws << atoms[i]->GetResiLabel();
  }
  return olxstr(rv);
}
//.............................................................................
void tls_group_constraint::FromToks(const TStrList& toks, RefinementModel& rm,
  TTypeList<tls_group_constraint>& out)
{
  if (toks.Count() < 4) {
    return;
  }
  TAtomReference aref(toks.Text(' '));
  TCAtomGroup agroup;
  size_t atomAGroup;
  aref.Expand(rm, agroup, EmptyString(), atomAGroup);
  if (agroup.Count() < 3) {
    throw TInvalidArgumentException(__OlxSourceInfo, "atom number");
  }
  TCAtomPList l(agroup, FunctionAccessor::MakeConst(&TGroupCAtom::GetAtom));
  out.Add(new tls_group_constraint(l));
}
//.............................................................................
tls_group_constraint* tls_group_constraint::Copy(
  RefinementModel& rm, const tls_group_constraint& c)
{
  TCAtomPList l(c.atoms.Count());
  for (size_t i=0; i < c.atoms.Count(); i++) {
    TCAtom* a = rm.aunit.FindCAtomById(c.atoms[i]->GetId());
    if (a == 0) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "asymmetric units do not match");
    }
    l[i] = a;
  }
  return new tls_group_constraint(l);
}
//.............................................................................
const olxstr& tls_group_constraint::GetName() {
  static olxstr name("olex2.constraint.tls_group");
  return name;
}
//.............................................................................
olxstr tls_group_constraint::Describe() const {
  return olxstr("TLS [") << olx_analysis::alg::label(atoms, ',') << ']';
}
//.............................................................................
void tls_group_constraint::UpdateParams(const TStrList& toks) {
  if (!toks.IsEmpty()) {
    throw TNotImplementedException(__OlxSourceInfo);
  }
}
//.............................................................................
void tls_group_constraint::ToDataItem(TDataItem& di) const {
  IndexRange::Builder rb;
  for (size_t i=0; i < atoms.Count(); i++) {
    rb << atoms[i]->GetTag();
  }
  di.AddField("atoms", rb.GetString());
}
//.............................................................................
tls_group_constraint* tls_group_constraint::FromDataItem(const TDataItem& di,
  const RefinementModel& rm)
{
  IndexRange::RangeItr ai(di.GetFieldByName("atoms"));
  TCAtomPList l;
  while (ai.HasNext()) {
    l.Add(rm.aunit.GetAtom(ai.Next()));
  }
  if (l.Count() < 3) {
    throw TInvalidArgumentException(__OlxSourceInfo, "atom number");
  }
  return new tls_group_constraint(l);
}
//.............................................................................
#ifdef _PYTHON
PyObject* tls_group_constraint::PyExport() const {
  PyObject *a = PyTuple_New(atoms.Count());
  for (size_t i = 0; i < atoms.Count(); i++) {
    PyTuple_SetItem(a, i, Py_BuildValue("i", atoms[i]->GetTag()));
  }
  return a;
}
#endif

//.............................................................................
//.............................................................................
//.............................................................................
olxstr same_disp_constraint::ToInsStr(const RefinementModel& rm) const {
  olxstr_buf rv;
  olxstr ws(' ');
  rv << GetName();
  for (size_t i = 0; i < atoms.Count(); i++) {
    rv << ws << atoms[i]->GetResiLabel();
  }
  return olxstr(rv);
}
//.............................................................................
void same_disp_constraint::FromToks(const TStrList& toks, RefinementModel& rm,
  TTypeList<same_disp_constraint>& out)
{
  if (toks.Count() < 2) {
    return;
  }
  TAtomReference aref(toks.Text(' '));
  TCAtomGroup agroup;
  size_t atomAGroup;
  aref.Expand(rm, agroup, EmptyString(), atomAGroup);
  if (agroup.Count() < 2) {
    throw TInvalidArgumentException(__OlxSourceInfo, "atom number");
  }
  TCAtomPList l(agroup, FunctionAccessor::MakeConst(&TGroupCAtom::GetAtom));
  out.Add(new same_disp_constraint(l));
}
//.............................................................................
same_disp_constraint* same_disp_constraint::Copy(
  RefinementModel& rm, const same_disp_constraint& c)
{
  TCAtomPList l(c.atoms.Count());
  for (size_t i = 0; i < c.atoms.Count(); i++) {
    TCAtom* a = rm.aunit.FindCAtomById(c.atoms[i]->GetId());
    if (a == 0) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "asymmetric units do not match");
    }
    l[i] = a;
  }
  return new same_disp_constraint(l);
}
//.............................................................................
const olxstr& same_disp_constraint::GetName() {
  static olxstr name("olex2.constraint.same_disp");
  return name;
}
//.............................................................................
olxstr same_disp_constraint::Describe() const {
  return olxstr("Same disp [") << olx_analysis::alg::label(atoms, ',') << ']';
}
//.............................................................................
void same_disp_constraint::UpdateParams(const TStrList& toks) {
  if (!toks.IsEmpty()) {
    throw TNotImplementedException(__OlxSourceInfo);
  }
}
//.............................................................................
void same_disp_constraint::ToDataItem(TDataItem& di) const {
  IndexRange::Builder rb;
  for (size_t i = 0; i < atoms.Count(); i++) {
    rb << atoms[i]->GetTag();
  }
  di.AddField("atoms", rb.GetString());
}
//.............................................................................
same_disp_constraint* same_disp_constraint::FromDataItem(const TDataItem& di,
  const RefinementModel& rm)
{
  IndexRange::RangeItr ai(di.GetFieldByName("atoms"));
  TCAtomPList l;
  while (ai.HasNext()) {
    l.Add(rm.aunit.GetAtom(ai.Next()));
  }
  if (l.Count() < 2) {
    throw TInvalidArgumentException(__OlxSourceInfo, "atom number");
  }
  return new same_disp_constraint(l);
}
//.............................................................................
#ifdef _PYTHON
PyObject* same_disp_constraint::PyExport() const {
  PyObject* a = PyTuple_New(atoms.Count());
  for (size_t i = 0; i < atoms.Count(); i++) {
    PyTuple_SetItem(a, i, Py_BuildValue("i", atoms[i]->GetTag()));
  }
  return a;
}
#endif
//.............................................................................
