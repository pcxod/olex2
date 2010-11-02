#include "exyzgroup.h"
#include "refmodel.h"

void TExyzGroup::Clear()  {  Parent.Delete(Id);  }
//..............................................................................
void TExyzGroup::Assign(const TExyzGroup& ag)  {
  for( size_t i=0; i < ag.Atoms.Count(); i++ )  {
    Atoms.Add(Parent.RM.aunit.FindCAtomById(ag.Atoms[i]->GetId()));
    if( Atoms.GetLast() == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "asymmetric units mismatch");
    Atoms.GetLast()->SetExyzGroup(this);
  }
}
//..............................................................................
void TExyzGroup::ToDataItem(TDataItem& item) const {
  size_t atom_id = 0;
  for( size_t i=0; i < Atoms.Count(); i++ )  {
    if( Atoms[i]->IsDeleted() )  continue;
    item.AddField(olxstr("atom_id_") << atom_id++, Atoms[i]->GetTag());
  }
}
//..............................................................................
#ifndef _NO_PYTHON
PyObject* TExyzGroup::PyExport(TPtrList<PyObject>& atoms)  {
  int atom_cnt = 0;
  for( size_t i=0; i < Atoms.Count(); i++ )  {
    if( Atoms[i]->IsDeleted() )  continue;
    atom_cnt++;
  }
  PyObject* main = PyTuple_New(atom_cnt);
  atom_cnt = 0;
  for( size_t i=0; i < Atoms.Count(); i++ )  {
    if( Atoms[i]->IsDeleted() )  continue;
    PyTuple_SetItem(main, atom_cnt++, Py_BuildValue("i", Atoms[i]->GetTag()));
  }
  return main;
}
#endif
//..............................................................................
void TExyzGroup::FromDataItem(TDataItem& item) {
  for( size_t i=0; i < item.FieldCount(); i++ )
    Atoms.Add(Parent.RM.aunit.GetAtom(item.GetField(i).ToSizeT()));
}
//..............................................................................
//..............................................................................
//..............................................................................
void TExyzGroups::ToDataItem(TDataItem& item) {
  int group_id = 0;
  for( size_t i=0; i < Groups.Count(); i++ )  {
    if( Groups[i].IsEmpty() )  {
      Groups.NullItem(i);
      continue;
    }
    Groups[i].SetId(group_id++);
  }
  Groups.Pack();
  item.AddField("n", Groups.Count());
  for( size_t i=0; i < Groups.Count(); i++ ) 
    Groups[i].ToDataItem(item.AddItem(group_id++));
}
//..............................................................................
#ifndef _NO_PYTHON
PyObject* TExyzGroups::PyExport(TPtrList<PyObject>& atoms)  {
  int group_id = 0;
  for( size_t i=0; i < Groups.Count(); i++ )  {
    if( Groups[i].IsEmpty() )  {
      Groups.NullItem(i);
      continue;
    }
    Groups[i].SetId(group_id++);
  }
  Groups.Pack();

  PyObject* main = PyTuple_New( Groups.Count() );
  for( size_t i=0; i < Groups.Count(); i++ )  {
    PyTuple_SetItem(main, i, Groups[i].PyExport(atoms));
  }
  return main;
}
#endif
//..............................................................................
void TExyzGroups::FromDataItem(TDataItem& item) {
  Clear();
  size_t n = item.GetRequiredField("n").ToSizeT();
  if( n != item.ItemCount() )
    throw TFunctionFailedException(__OlxSourceInfo, "number of items mismatch");
  for( size_t i=0; i < n; i++ )  {
    New().FromDataItem(item.GetItem(i));
  }
}
//..............................................................................

