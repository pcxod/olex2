#include "refmodel.h"
#include "constraints_ext.h"
#include "atomref.h"
//#include "catomlist.h"

void rotated_adp_constraint::FromToks(const TStrList& toks, RefinementModel& rm,
    TTypeList<rotated_adp_constraint>& out)
{
  if( toks.IsEmpty() )  return;
  TCAtom* ref = rm.aunit.FindCAtom(toks[0]);
  if( ref == NULL )  return;
  olxstr resi;
  for( size_t i=1; i < toks.Count(); i += 4 )  {
    if( toks.Count() <= i+3 )  return;
    TAtomReference aref(toks.Text(' ', i, i+3));
    TCAtomGroup agroup;
    size_t atomAGroup;
    try  {  aref.Expand(rm, agroup, resi, atomAGroup);  }
    catch( const TExceptionBase& ex )  {
      TBasicApp::NewLogEntry(logException) << ex.GetException()->GetError();
      continue;
    }
    if( agroup.Count() != 3 )
      continue;
    bool refine_angle = false;
    const double angle = toks[i+3].ToDouble();
    if( i+4 < toks.Count() && toks[i+4].IsBool() )
      refine_angle = toks[4+i++].ToBool();
    out.Add(new rotated_adp_constraint(*ref,
      *agroup[0].GetAtom(), agroup[1], agroup[2], angle, refine_angle));
  }
}
//...................................................................................
rotated_adp_constraint* rotated_adp_constraint::Copy(
  RefinementModel& rm, const rotated_adp_constraint& c)
{
  TCAtom* ref = rm.aunit.FindCAtomById(c.source.GetId());
  TCAtom* atom = rm.aunit.FindCAtomById(c.destination.GetId());
  TCAtom* from = rm.aunit.FindCAtomById(c.dir_from.GetAtom()->GetId());
  TCAtom* to = rm.aunit.FindCAtomById(c.dir_to.GetAtom()->GetId());
  if( ref == NULL || atom == NULL || from== NULL || to == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "asymmetric units do not match");
  const smatd *f_m = NULL, *t_m = NULL;
  if( c.dir_from.GetMatrix() != NULL )
    f_m = &rm.AddUsedSymm(*c.dir_from.GetMatrix());
  if( c.dir_to.GetMatrix() != NULL )
    t_m = &rm.AddUsedSymm(*c.dir_to.GetMatrix());
  return new rotated_adp_constraint(*ref,
      *atom, TGroupCAtom(from, f_m), TGroupCAtom(to, t_m),
      c.angle, c.refine_angle);
}
//...................................................................................
#ifndef _NO_PYTHON
PyObject* rotated_adp_constraint::PyExport() const {
  return Py_BuildValue("i,i,(i,i),(i,i),d,b", source.GetTag(), destination.GetTag(),
    dir_from.GetAtom()->GetTag(), dir_from.GetMatrix() == NULL ? -1 : dir_from.GetMatrix()->GetId(),
    dir_to.GetAtom()->GetTag(), dir_to.GetMatrix() == NULL ? -1 : dir_to.GetMatrix()->GetId(),
    angle, refine_angle
  );
}
#endif
//...................................................................................
olxstr rotated_adp_constraint::ToInsStr(const RefinementModel& rm) const {
  return olxstr("REM", 64).Stream(' ') << GetName() << source.GetLabel()
    << destination.GetLabel() << dir_from.GetFullLabel(rm)
    << dir_to.GetFullLabel(rm) << angle << refine_angle;
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
  dir_from.ToDataItem(di.AddItem("dir_from"));
  dir_to.ToDataItem(di.AddItem("dir_to"));
}
//...................................................................................
rotated_adp_constraint* rotated_adp_constraint::FromDataItem(
  const TDataItem& di, const RefinementModel& rm)
{
  return new rotated_adp_constraint(
    rm.aunit.GetAtom(di.GetRequiredField("source").ToSizeT()),
    rm.aunit.GetAtom(di.GetRequiredField("destination").ToSizeT()),
      TGroupCAtom().FromDataItem(di.FindRequiredItem("dir_from"), rm),
      TGroupCAtom().FromDataItem(di.FindRequiredItem("dir_to"), rm),
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
