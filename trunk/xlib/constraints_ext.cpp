#include "refmodel.h"
#include "constraints_ext.h"
#include "atomref.h"
#include "pers_util.h"

void rotated_adp_constraint::FromToks(const TStrList& toks, RefinementModel& rm,
    TTypeList<rotated_adp_constraint>& out)
{
  if( toks.Count() < 4 )  return;
  TCAtom *src = rm.aunit.FindCAtom(toks[0]),
    *dest = rm.aunit.FindCAtom(toks[1]);
  if( src == NULL || dest == NULL )  return;
  adirection& dir = rm.DirectionById(toks[2]);
  bool refine_angle = false;
  const double angle = toks[3].ToDouble();
  if( toks.Count() > 4 )
    refine_angle = toks[4].ToBool();
  out.Add(new rotated_adp_constraint(*src, *dest, dir, angle, refine_angle));
}
//...................................................................................
rotated_adp_constraint* rotated_adp_constraint::Copy(
  RefinementModel& rm, const rotated_adp_constraint& c)
{
  TCAtom* ref = rm.aunit.FindCAtomById(c.source.GetId());
  TCAtom* atom = rm.aunit.FindCAtomById(c.destination.GetId());
  if( ref == NULL || atom == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "asymmetric units do not match");
  return new rotated_adp_constraint(*ref,
    *atom, rm.DirectionById(c.dir.id), c.angle, c.refine_angle);
}
//...................................................................................
#ifndef _NO_PYTHON
PyObject* rotated_adp_constraint::PyExport() const {
  return Py_BuildValue("i,i,O,d,b", source.GetTag(), destination.GetTag(),
    PythonExt::BuildString(dir.id), angle, refine_angle
  );
}
#endif
//...................................................................................
olxstr rotated_adp_constraint::ToInsStr(const RefinementModel& rm) const {
  return olxstr("REM", 64).stream(' ') << GetName() << source.GetLabel()
    << destination.GetLabel() << dir.id << angle << refine_angle;
}
//...................................................................................
const olxstr& rotated_adp_constraint::GetName()  {
  static olxstr name("olex2.constraint.rotated_adp");
  return name;
}
//...................................................................................
void rotated_adp_constraint::ToDataItem(TDataItem& di) const {
  di.AddField("source", source.GetTag()).
    AddField("destination", destination.GetTag()).
    AddField("angle", angle).
    AddField("refine_angle", refine_angle);
  dir.ToDataItem(di.AddItem("dir"));
}
//...................................................................................
rotated_adp_constraint* rotated_adp_constraint::FromDataItem(
  const TDataItem& di, const RefinementModel& rm)
{
  return new rotated_adp_constraint(
    rm.aunit.GetAtom(di.GetRequiredField("source").ToSizeT()),
    rm.aunit.GetAtom(di.GetRequiredField("destination").ToSizeT()),
      *adirection::FromDataItem(di.FindRequiredItem("dir"), rm),
      di.GetRequiredField("angle").ToDouble(),
      di.GetRequiredField("refine_angle").ToBool()
    );
}
//...................................................................................
void rotated_adp_constraint::UpdateParams(const TStrList& toks)  {
  if( toks.Count() != 1 )
    throw TInvalidArgumentException(__OlxSourceInfo, "argument number");
  angle = toks[0].ToDouble();
}
//...................................................................................
//...................................................................................
//...................................................................................
olxstr adirection::type_names[] = {"static", "vector", "normal"};
const olxstr& adirection::GetName()  {
  static olxstr name("olex2.direction");
  return name;
}
//...................................................................................
const olxstr &adirection::EncodeType(uint16_t type)  {
  if( type == direction_static )
    return type_names[0];
  else if( type == direction_vector )
    return type_names[1];
  else if( type == direction_normal )
    return type_names[2];
  throw TInvalidArgumentException(__OlxSourceInfo, olxstr("type=") << type);
}
//...................................................................................
uint16_t adirection::DecodeType(const olxstr &type)  {
  if( type.Equalsi(type_names[0]) )
    return direction_static;
  else if( type.Equalsi(type_names[1]) )
    return direction_vector;
  else if( type.Equalsi(type_names[2]) )
    return direction_normal;
  throw TInvalidArgumentException(__OlxSourceInfo, olxstr("type=") << type);
}
//...................................................................................
void adirection::FromToks(const TStrList& toks, RefinementModel& rm,
    TTypeList<adirection>& out)
{
  if( toks.Count() < 4 )  return;
  try  {
    uint16_t type = adirection::DecodeType(toks[0]);
    if( type == direction_static )  {
      if( toks.Count() < 5 )  {
        TBasicApp::NewLogEntry(logError) << "Too few parameters for static direction";
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
  catch( const TExceptionBase& ex )  {
    TBasicApp::NewLogEntry(logException) << ex.GetException()->GetError();
  }
}
//...................................................................................
adirection* direction::DoCopy(RefinementModel& rm) const {
  TCAtomGroup agroup;
  agroup.SetCapacity(atoms.Count());
  for( size_t i=0; i < atoms.Count(); i++ )  {
    TCAtom* a = rm.aunit.FindCAtomById(atoms[i].GetAtom()->GetId());
    if( a == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "asymmetric units do not match");
    const smatd *m=NULL;
    if( atoms[i].GetMatrix() != NULL )
      m = &rm.AddUsedSymm(*atoms[i].GetMatrix());
    agroup.Add(new TGroupCAtom(a, m));
  }
  return new direction(id, agroup, type);
}
//...................................................................................
#ifndef _NO_PYTHON
PyObject *static_direction::PyExport() const {
  return Py_BuildValue("s, s, (f,f,f)",
    PythonExt::BuildString(adirection::EncodeType(direction_static)),
    PythonExt::BuildString(id),
    direction_static, value[0], value[1], value[2]);
}
PyObject *direction::PyExport() const {
  size_t sz = 0;
  for( size_t i=0; i < atoms.Count(); i++ )  {
    if( atoms[i].GetAtom()->IsDeleted() )  continue;
    sz++;
  }
  PyObject *rv = PyTuple_New(sz+2);
  PyTuple_SetItem(rv, 0, PythonExt::BuildString(adirection::EncodeType(type)));
  PyTuple_SetItem(rv, 1, PythonExt::BuildString(id));
  sz = 0;
  for( size_t i=0; i < atoms.Count(); i++ )  {
    if( atoms[i].GetAtom()->IsDeleted() )  continue;
    PyTuple_SetItem(rv, ++sz+1, Py_BuildValue("(i,i)",
      atoms[i].GetAtom()->GetTag(),
      atoms[i].GetMatrix() == NULL ? -1 : atoms[i].GetMatrix()->GetId()));
  }
  return rv;  
}
#endif
//...................................................................................
olxstr static_direction::ToInsStr(const RefinementModel& rm) const {
  return olxstr("REM", 64).stream(' ') << GetName() << adirection::EncodeType(direction_static)
    << id << value[0] << value[1] << value[2];
}
//...................................................................................
olxstr direction::ToInsStr(const RefinementModel& rm) const {
  olxstr rv("REM", 64);
  rv.stream(' ') << GetName() << adirection::EncodeType(type) << id;
  for( size_t i=0; i < atoms.Count(); i++ )  {
    if( atoms[i].GetAtom()->IsDeleted() )  continue;
    rv << ' ' << atoms[i].GetFullLabel(rm);
  }
  return rv;
}
//...................................................................................
void static_direction::ToDataItem(TDataItem& di) const {
  di.AddField("type", adirection::EncodeType(direction_static))
    .AddField("id", id)
    .AddField("value", PersUtil::VecToStr(value));
}
//...................................................................................
void direction::ToDataItem(TDataItem& di) const {
  di.AddField("type", adirection::EncodeType(type)).
    AddField("id", id);
  for( size_t i=0; i < atoms.Count(); i++ )  {
    if( atoms[i].GetAtom()->IsDeleted() )  continue;
    atoms[i].ToDataItem(di.AddItem("atom"));
  }
}
//...................................................................................
adirection* adirection::FromDataItem(const TDataItem& di,
  const RefinementModel& rm)
{
  uint16_t type = adirection::DecodeType(di.GetRequiredField("type"));
  if( type == direction_static )
    return static_direction().CreateFromDataItem(di, rm);
  else
    return direction().CreateFromDataItem(di, rm);
}
//...................................................................................
adirection* static_direction::CreateFromDataItem(const TDataItem& di,
  const RefinementModel& rm) const
{
  return new static_direction(di.GetRequiredField("id"),
    PersUtil::FloatVecFromStr(di.GetRequiredField("value")));
}
//...................................................................................
adirection* direction::CreateFromDataItem(const TDataItem& di,
  const RefinementModel& rm) const
{
  uint16_t type = adirection::DecodeType(di.GetRequiredField("type"));
  TCAtomGroup agroup;
  agroup.SetCapacity(di.ItemCount());
  for( size_t i=0; i < di.ItemCount(); i++ )
    agroup.Add(TGroupCAtom().FromDataItem(di.GetItem(i), rm));
  return new direction(di.GetRequiredField("id"), agroup, type);
}
//...................................................................................
vec3d direction::get() const {
  return vec3d(0,0,0);
}
//...................................................................................
