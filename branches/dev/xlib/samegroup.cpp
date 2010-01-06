#include "samegroup.h"
#include "asymmunit.h"
#include "refmodel.h"

void TSameGroup::Assign(TAsymmUnit& tau, const TSameGroup& sg)  {
  Clear();
  if( sg.Count() == 0 )  return;

  TAsymmUnit * au = sg[0].GetParent();

  if( au == &tau )  {
    for( size_t i=0; i < sg.Count(); i++ )
      Add( const_cast<TCAtom&>(sg[i]) );
  }
  else  {
    for( size_t i=0; i < sg.Count(); i++ )  {
      TCAtom* aa = tau.FindCAtomById( sg[i].GetId() );
      if( aa == NULL )
        throw TFunctionFailedException(__OlxSourceInfo, "asymmetric units do not match");
      Add( *aa );
    }
  }
  Esd12 = sg.Esd12;  
  Esd13 = sg.Esd13;  
  for( size_t i=0; i < sg.Dependent.Count(); i++ )
    Dependent.Add( &Parent[ sg.Dependent[i]->Id ] );
  if( sg.GetParentGroup() != NULL )
    ParentGroup = &Parent[ sg.GetParentGroup()->Id ];
}
//..........................................................................................
TCAtom& TSameGroup::Add(TCAtom& ca)  {  
  ca.SetSameId(Id);
  Atoms.Add(&ca);
  return ca;
}
//..........................................................................................
void TSameGroup::ToDataItem(TDataItem& item) const {
  item.AddField("esd12", Esd12);
  item.AddField("esd13", Esd13);
  int atom_id = 0;
  TDataItem& atoms = item.AddItem("atoms");
  for( size_t i=0; i < Atoms.Count(); i++ )  {
    if( Atoms[i]->IsDeleted() ) continue;
    atoms.AddItem(atom_id++, Atoms[i]->GetTag() );
  }
  TDataItem& dep = item.AddItem("dependent");
  for( size_t i=0; i < Dependent.Count(); i++ )
    item.AddItem(atom_id++, Dependent[i]->GetId() );
  if( ParentGroup != NULL )
    item.AddField("parent", ParentGroup->GetId() );
}
//..............................................................................
#ifndef _NO_PYTHON
PyObject* TSameGroup::PyExport(PyObject* main, TPtrList<PyObject>& allGroups, TPtrList<PyObject>& _atoms)  {
  PyDict_SetItemString(main, "esd12", Py_BuildValue("d", Esd12)  );
  PyDict_SetItemString(main, "esd13", Py_BuildValue("d", Esd13)  );
  int atom_cnt = 0;
  for( size_t i=0; i < Atoms.Count(); i++ )  {
    if( Atoms[i]->IsDeleted() )  continue;
    atom_cnt++;
  }
  PyObject* atoms = PyTuple_New(atom_cnt);
  atom_cnt = 0;
  for( size_t i=0; i < Atoms.Count(); i++ )  {
    if( Atoms[i]->IsDeleted() )  continue;
    PyTuple_SetItem(atoms, atom_cnt++, Py_BuildValue("i", Atoms[i]->GetTag()) );
  }
  PyDict_SetItemString(main, "atoms", atoms);
  PyObject* dependent = PyTuple_New(Dependent.Count());
  for( size_t i=0; i < Dependent.Count(); i++ )
    PyTuple_SetItem(dependent, i, Py_BuildValue("i", Dependent[i]->GetTag()) );
  PyDict_SetItemString(main, "dependent", dependent);
  if( ParentGroup != NULL )
    PyDict_SetItemString(main, "parent", Py_BuildValue("i", ParentGroup->GetTag()));
  return main;
}
#endif
//..........................................................................................
void TSameGroup::FromDataItem(TDataItem& item) {
  Clear();
  Esd12 = item.GetRequiredField("esd12").ToDouble();
  Esd13 = item.GetRequiredField("esd13").ToDouble();
  TDataItem& atoms = item.FindRequiredItem("atoms");
  for( size_t i=0; i < atoms.ItemCount(); i++ )
    Add( Parent.RM.aunit.GetAtom(atoms.GetItem(i).GetValue().ToInt()) );
  TDataItem& dep = item.FindRequiredItem("dependent");
  for( size_t i=0; i < dep.ItemCount(); i++ )
    AddDependent( Parent[dep.GetItem(i).GetValue().ToInt()] );
  const olxstr p_id = item.GetFieldValue("parent");
  if( !p_id.IsEmpty() )
    ParentGroup = &Parent[p_id.ToInt()];
}
//..........................................................................................
//..........................................................................................
//..........................................................................................
void TSameGroupList::Release(TSameGroup& sg)  {
  if( &sg.GetParent() != this )
    throw TInvalidArgumentException(__OlxSourceInfo, "SAME group parent differs");
  Groups.Release(sg.GetId());
  if( sg.GetParentGroup() != NULL )
    sg.GetParentGroup()->RemoveDependent(sg);
  sg.ClearAtomIds();
  for( size_t i=0; i < Groups.Count(); i++ )
    Groups[i].SetId((uint16_t)i);
}
//..........................................................................................
void TSameGroupList::Restore(TSameGroup& sg)  {
  if( &sg.GetParent() != this )
    throw TInvalidArgumentException(__OlxSourceInfo, "SAME group parent differs");
  Groups.Add(sg);
  if( sg.GetParentGroup() != NULL )
    sg.GetParentGroup()->AddDependent(sg);
  sg.SetId( (uint16_t)(Groups.Count()-1) );
}
//..........................................................................................
void TSameGroupList::ToDataItem(TDataItem& item) const {
  item.AddField("n", Groups.Count());
  for( size_t i=0; i < Groups.Count(); i++ ) 
    if( Groups[i].IsValidForSave() )
      Groups[i].ToDataItem( item.AddItem(i) );
}
//..............................................................................
#ifndef _NO_PYTHON
PyObject* TSameGroupList::PyExport(TPtrList<PyObject>& _atoms)  {
  size_t id = 0;
  for( size_t i=0; i < Groups.Count(); i++ )  {
    if( Groups[i].IsValidForSave() )
      Groups[i].SetTag(id++);
  }
  if( id == 0 )
    return Py_None;
  PyObject* main = PyTuple_New( id );
  TPtrList<PyObject> allGroups;
  for( size_t i=0; i < id; i++ )
    PyTuple_SetItem(main, i, allGroups.Add( PyDict_New() ) );
  for( size_t i=0; i < Groups.Count(); i++ ) 
    if( Groups[i].IsValidForSave() )
      Groups[i].PyExport(allGroups[i], allGroups, _atoms);
  return main;
}
#endif
//..........................................................................................
void TSameGroupList::FromDataItem(TDataItem& item) {
  Clear();
  size_t n = item.GetRequiredField("n").ToSizeT();
  if( n != item.ItemCount() )
    throw TFunctionFailedException(__OlxSourceInfo, "number of groups doe snot match the number of items");
  for( size_t i=0; i < n; i++ )
    New();
  for( size_t i=0; i < n; i++ )
    Groups[i].FromDataItem(item.GetItem(i));
}
//..........................................................................................
