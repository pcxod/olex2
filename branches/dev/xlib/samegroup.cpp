#include "samegroup.h"
#include "asymmunit.h"
#include "refmodel.h"

void TSameGroup::Assign(TAsymmUnit& tau, const TSameGroup& sg)  {
  Clear();
  if( sg.Count() == 0 )  return;

  TAsymmUnit * au = sg[0].GetParent();

  if( au == &tau )  {
    for(int i=0; i < sg.Count(); i++ )
      Add( const_cast<TCAtom&>(sg[i]) );
  }
  else  {
    for(int i=0; i < sg.Count(); i++ )  {
      TCAtom* aa = tau.FindCAtomById( sg[i].GetId() );
      if( aa == NULL )
        throw TFunctionFailedException(__OlxSourceInfo, "asymmetric units do not match");
      Add( *aa );
    }
  }
  Esd12 = sg.Esd12;  
  Esd13 = sg.Esd13;  
  for( int i=0; i < sg.Dependent.Count(); i++ )
    Dependent.Add( &Parent[ sg.Dependent[i]->Id ] );
}
//..........................................................................................
void TSameGroup::ToDataItem(TDataItem& item) const {
  item.AddField("esd12", Esd12);
  item.AddField("esd13", Esd13);
  int atom_id = 0;
  TDataItem& atoms = item.AddItem("atoms");
  for( int i=0; i < Atoms.Count(); i++ )  {
    if( Atoms[i]->IsDeleted() ) continue;
    atoms.AddItem(atom_id++, Atoms[i]->GetTag() );
  }
  TDataItem& dep = item.AddItem("dependent");
  for( int i=0; i < Dependent.Count(); i++ )  {
    item.AddItem(atom_id++, Dependent[i]->GetId() );
  }
}
//..............................................................................
#ifndef _NO_PYTHON
PyObject* TSameGroup::PyExport(PyObject* main, TPtrList<PyObject>& allGroups, TPtrList<PyObject>& _atoms)  {
  PyDict_SetItemString(main, "esd12", Py_BuildValue("d", Esd12)  );
  PyDict_SetItemString(main, "esd13", Py_BuildValue("d", Esd13)  );
  int atom_cnt = 0;
  for( int i=0; i < Atoms.Count(); i++ )  {
    if( Atoms[i]->IsDeleted() )  continue;
    atom_cnt++;
  }
  PyObject* atoms = PyTuple_New(atom_cnt);
  atom_cnt = 0;
  for( int i=0; i < Atoms.Count(); i++ )  {
    if( Atoms[i]->IsDeleted() )  continue;
    PyTuple_SetItem(atoms, atom_cnt++, _atoms[Atoms[i]->GetTag()] );
  }
  PyDict_SetItemString(main, "atoms", atoms);
  PyObject* dependent = PyTuple_New(Dependent.Count());
  for( int i=0; i < Dependent.Count(); i++ )
    PyTuple_SetItem(dependent, i, allGroups[Dependent[i]->GetTag()] );
  PyDict_SetItemString(main, "dependent", dependent);
  return main;
}
#endif
//..........................................................................................
void TSameGroup::FromDataItem(TDataItem& item) {
  Clear();
  Esd12 = item.GetRequiredField("esd12").ToDouble();
  Esd13 = item.GetRequiredField("esd13").ToDouble();
  TDataItem& atoms = item.FindRequiredItem("atoms");
  for( int i=0; i < atoms.ItemCount(); i++ )
    Atoms.Add( &Parent.RM.aunit.GetAtom(atoms.GetItem(i).GetValue().ToInt()) );
  TDataItem& dep = item.FindRequiredItem("dependent");
  for( int i=0; i < dep.ItemCount(); i++ )
    Dependent.Add( &Parent[dep.GetItem(i).GetValue().ToInt()] );
}
//..........................................................................................
//..........................................................................................
//..........................................................................................
void TSameGroupList::ToDataItem(TDataItem& item) const {
  item.AddField("n", Groups.Count());
  for( int i=0; i < Groups.Count(); i++ )  {
    Groups[i].ToDataItem( item.AddItem(i) );
  }
}
//..............................................................................
#ifndef _NO_PYTHON
PyObject* TSameGroupList::PyExport(TPtrList<PyObject>& _atoms)  {
  PyObject* main = PyTuple_New( Groups.Count() );
  TPtrList<PyObject> allGroups(Groups.Count());
  for( int i=0; i < Groups.Count(); i++ )
    PyTuple_SetItem(main, i, allGroups.Add( PyDict_New() ) );
  for( int i=0; i < Groups.Count(); i++ )
    Groups[i].PyExport(allGroups[i], allGroups, _atoms);
  return main;
}
#endif
//..........................................................................................
void TSameGroupList::FromDataItem(TDataItem& item) {
  Clear();
  int n = item.GetRequiredField("n").ToInt();
  if( n != item.ItemCount() )
    throw TFunctionFailedException(__OlxSourceInfo, "number of groups doe snot match the number of items");
  for( int i=0; i < n; i++ )
    New();
  for( int i=0; i < n; i++ )
    Groups[i].FromDataItem(item.GetItem(i));
}
//..........................................................................................
