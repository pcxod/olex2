#include "refmodel.h"
#include "constraints_ext.h"
#include "atomref.h"
//#include "catomlist.h"

void shared_rotated_adp_constraint::FromToks(const TStrList& toks, RefinementModel& rm,
    TTypeList<shared_rotated_adp_constraint>& out)
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
    out.Add(new shared_rotated_adp_constraint(*ref,
      *agroup[0].GetAtom(), agroup[1], agroup[2], toks[i+3].ToDouble()));
  }
}
//...................................................................................
shared_rotated_adp_constraint* shared_rotated_adp_constraint::Copy(
  RefinementModel& rm, const shared_rotated_adp_constraint& c)
{
  TCAtom* ref = rm.aunit.FindCAtomById(c.reference.GetId());
  TCAtom* atom = rm.aunit.FindCAtomById(c.atom.GetId());
  TCAtom* from = rm.aunit.FindCAtomById(c.dir_from.GetAtom()->GetId());
  TCAtom* to = rm.aunit.FindCAtomById(c.dir_to.GetAtom()->GetId());
  if( ref == NULL || atom == NULL || from== NULL || to == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "asymmetric units do not match");
  const smatd *f_m = NULL, *t_m = NULL;
  if( c.dir_from.GetMatrix() != NULL )
    f_m = &rm.AddUsedSymm(*c.dir_from.GetMatrix());
  if( c.dir_to.GetMatrix() != NULL )
    t_m = &rm.AddUsedSymm(*c.dir_to.GetMatrix());
  return new shared_rotated_adp_constraint(*ref,
      *atom, TGroupCAtom(from, f_m), TGroupCAtom(to, t_m),
      c.angle);
}
//...................................................................................
#ifndef _NO_PYTHON
PyObject* shared_rotated_adp_constraint::PyExport() const {
  return Py_BuildValue("i,i,(i,i),(i,i),d", reference.GetTag(), atom.GetTag(),
    dir_from.GetAtom()->GetTag(), dir_from.GetMatrix() == NULL ? -1 : dir_from.GetMatrix()->GetId(),
    dir_to.GetAtom()->GetTag(), dir_to.GetMatrix() == NULL ? -1 : dir_to.GetMatrix()->GetId(),
    angle
  );
}
#endif
//...................................................................................
olxstr shared_rotated_adp_constraint::ToInsStr(const RefinementModel& rm) const {
  return olxstr("REM olex2.shared_rotated_adp ") << reference.GetLabel()
    << ' ' << atom.GetLabel() << ' ' << dir_from.GetFullLabel(rm)
    << ' ' << dir_to.GetFullLabel(rm) << ' ' << angle;
}
//...................................................................................
