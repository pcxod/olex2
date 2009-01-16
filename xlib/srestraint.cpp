#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "srestraint.h"
#include "refmodel.h"

TSimpleRestraint::TSimpleRestraint(TSRestraintList& parent, const short listType) : Parent(parent)  {
  Value = 0;
  Esd = Esd1 = 0;
  ListType = listType;
  AllNonHAtoms = false;
}
//..............................................................................
TSimpleRestraint::~TSimpleRestraint()  {
  Clear();
}
//..............................................................................
void TSimpleRestraint::Clear()  {
  InvolvedAtoms.Clear();
}
//..............................................................................
void TSimpleRestraint::AddAtoms(const TCAtomGroup& atoms)  {
  InvolvedAtoms.SetCapacity( InvolvedAtoms.Count() + atoms.Count() );
  for( int i=0; i < atoms.Count(); i++ )
    InvolvedAtoms.AddNew( atoms[i].GetAtom(), atoms[i].GetMatrix() );
}
//..............................................................................
void TSimpleRestraint::AddAtom(TCAtom& aa, const smatd* ma)  {
  if( aa.GetParent() != &Parent.GetRM().aunit  )
    throw TInvalidArgumentException(__OlxSourceInfo, "mismatching asymmetric unit");
  const smatd* tm = NULL;
  if( ma != NULL )  {
    if( !ma->r.IsI() || ma->t.QLength() != 0 ) 
      tm = &Parent.GetRM().AddUsedSymm( *ma );
  }
  InvolvedAtoms.AddNew( &aa, tm );
}
//..............................................................................
void TSimpleRestraint::AddAtomPair(TCAtom& aa, const smatd* ma,
                                   TCAtom& ab, const smatd* mb)  {
  AddAtom(aa, ma);
  AddAtom(ab, mb);
}
//..............................................................................
void TSimpleRestraint::Validate()  {
  for( int i=0; i < InvolvedAtoms.Count(); i++ )  {
    if( InvolvedAtoms.IsNull(i) )  continue;
    if( InvolvedAtoms[i].GetAtom()->IsDeleted() )  {
      if( InvolvedAtoms[i].GetMatrix() != NULL )
        Parent.GetRM().RemUsedSymm( *InvolvedAtoms[i].GetMatrix() );
      InvolvedAtoms.NullItem(i);
      if( ListType == rltBonds )  {
        int ei = i + ((i%2)==0 ? 1 : -1);
        if( InvolvedAtoms[ei].GetMatrix() != NULL )
          Parent.GetRM().RemUsedSymm( *InvolvedAtoms[ei].GetMatrix() );
        InvolvedAtoms.NullItem(ei);
      }
      else if( ListType == rltAngles )  {
        if( (i%3) == 0 )  {
          if( InvolvedAtoms[i+1].GetMatrix() != NULL )
            Parent.GetRM().RemUsedSymm( *InvolvedAtoms[i+1].GetMatrix() );
          InvolvedAtoms.NullItem(i+2);
          if( InvolvedAtoms[i+2].GetMatrix() != NULL )
            Parent.GetRM().RemUsedSymm( *InvolvedAtoms[i+2].GetMatrix() );
          InvolvedAtoms.NullItem(i+2);
        }
        else if( (i%3) == 1 )  {
          if( InvolvedAtoms[i-1].GetMatrix() != NULL )
            Parent.GetRM().RemUsedSymm( *InvolvedAtoms[i-1].GetMatrix() );
          InvolvedAtoms.NullItem(i+1);
          if( InvolvedAtoms[i+1].GetMatrix() != NULL )
            Parent.GetRM().RemUsedSymm( *InvolvedAtoms[i+1].GetMatrix() );
          InvolvedAtoms.NullItem(i+1);
        }
        else if( (i%3) == 2 )  {
          if( InvolvedAtoms[i-1].GetMatrix() != NULL )
            Parent.GetRM().RemUsedSymm( *InvolvedAtoms[i-1].GetMatrix() );
          InvolvedAtoms.NullItem(i-2);
          if( InvolvedAtoms[i-2].GetMatrix() != NULL )
            Parent.GetRM().RemUsedSymm( *InvolvedAtoms[i-2].GetMatrix() );
          InvolvedAtoms.NullItem(i-2);
        }
      }
    }
  }
  InvolvedAtoms.Pack();
}
//..............................................................................
void TSimpleRestraint::RemoveAtom(int i)  {
  if( InvolvedAtoms[i].GetMatrix() != NULL )
    Parent.GetRM().RemUsedSymm( *InvolvedAtoms[i].GetMatrix() );
  InvolvedAtoms.Delete(i);
}
//..............................................................................
void TSimpleRestraint::Delete()  {
  for( int i=0; i < InvolvedAtoms.Count(); i++ )  {
    if( InvolvedAtoms[i].GetMatrix() != NULL )
      Parent.GetRM().RemUsedSymm( *InvolvedAtoms[i].GetMatrix() );
  }
  InvolvedAtoms.Clear();
}
//..............................................................................
/*
const TSimpleRestraint& TSimpleRestraint::operator = ( const TSimpleRestraint& sr)  {
  Clear();
  for(int i=0; i < sr.InvolvedAtoms.Count(); i+=2 )  {
    AddAtomPair( sr.InvolvedAtoms[i].GetAtom(), sr.InvolvedAtoms[i].GetMatrix(),
                 sr.InvolvedAtoms[i+1].GetAtom(), sr.InvolvedAtoms[i+1].GetMatrix() );
  }
  Value = sr.Value;
  Esd = sr.Esd;
  return sr;
}
*/
//..............................................................................
void TSimpleRestraint::Assign(const TSimpleRestraint& sr)  {
  Clear();
  ListType = sr.GetListType();
  Value = sr.Value;
  Esd = sr.Esd;
  Esd1 = sr.Esd1;
  AllNonHAtoms = sr.AllNonHAtoms;

  if( sr.AtomCount() == 0 )  return;

  TAsymmUnit& au = sr.Parent.GetRM().aunit;
  TAsymmUnit& tau = Parent.GetRM().aunit;

  if( &au == &tau )  {
    for(int i=0; i < sr.InvolvedAtoms.Count(); i++ )
      AddAtom( *sr.InvolvedAtoms[i].GetAtom(), sr.InvolvedAtoms[i].GetMatrix() );
  }
  else  {
    for(int i=0; i < sr.InvolvedAtoms.Count(); i++ )  {
      TCAtom* aa = tau.FindCAtomByLoaderId( sr.InvolvedAtoms[i].GetAtom()->GetLoaderId() );
      if( aa == NULL )
        throw TFunctionFailedException(__OlxSourceInfo, "asymmetric units do not match");
      AddAtom( *aa, sr.InvolvedAtoms[i].GetMatrix() );
    }
  }
}
//..............................................................................
void TSimpleRestraint::Substruct( TSimpleRestraint& sr )  {
  if( sr.GetListType() != ListType )
    throw TInvalidArgumentException(__OlxSourceInfo, "list type mismatch");
  if( ListType == rltAtoms || ListType == rltGroup )  {
    for( int i=0; i < InvolvedAtoms.Count(); i++ )  {
      for( int j=0; j < sr.InvolvedAtoms.Count(); j++ )  {
        if( AtomsEqual(InvolvedAtoms[i].GetAtom(), InvolvedAtoms[i].GetMatrix(),
                       sr.InvolvedAtoms[j].GetAtom(), sr.InvolvedAtoms[j].GetMatrix()) ) {
          InvolvedAtoms.Delete(i);
          break;
        }
      }
    }
  }
  else if( ListType == rltBonds )  {
    for( int i=0; i < InvolvedAtoms.Count(); i+=2 )  {
      for( int j=0; j < sr.InvolvedAtoms.Count(); j+=2 )  {
        if( (AtomsEqual(InvolvedAtoms[i].GetAtom(), InvolvedAtoms[i].GetMatrix(), sr.InvolvedAtoms[j].GetAtom(), sr.InvolvedAtoms[j].GetMatrix()) &&
             AtomsEqual(InvolvedAtoms[i+1].GetAtom(), InvolvedAtoms[i+1].GetMatrix(), sr.InvolvedAtoms[j+1].GetAtom(), sr.InvolvedAtoms[j+1].GetMatrix())) ||
            (AtomsEqual(InvolvedAtoms[i].GetAtom(), InvolvedAtoms[i].GetMatrix(), sr.InvolvedAtoms[j+1].GetAtom(), sr.InvolvedAtoms[j+1].GetMatrix()) &&
             AtomsEqual(InvolvedAtoms[i+1].GetAtom(), InvolvedAtoms[i+1].GetMatrix(), sr.InvolvedAtoms[j].GetAtom(), sr.InvolvedAtoms[j].GetMatrix())) ) {
          InvolvedAtoms.Delete(i);
          InvolvedAtoms.Delete(i+1);
          break;
        }
      }
    }
  }
}
//..............................................................................
void TSimpleRestraint::OnCAtomCrdChange( TCAtom* ca, const smatd& matr )  {
  throw TNotImplementedException(__OlxSourceInfo);
}
//..............................................................................
bool TSimpleRestraint::ContainsAtom(TCAtom* ca) const {
  for( int i=0; i < InvolvedAtoms.Count(); i++ )
    if( InvolvedAtoms[i].GetAtom() == ca )
      return true;
  return false;
}
//..............................................................................
void TSimpleRestraint::ToDataItem(TDataItem& item) const {
  item.AddField("allNonH", AllNonHAtoms);
  item.AddField("esd", Esd);
  item.AddField("esd1", Esd1);
  item.AddField("val", Value);
  TDataItem& atoms = item.AddItem("atoms");
  int atom_id=0;
  for( int i=0; i < InvolvedAtoms.Count(); i++ )  {
    if( InvolvedAtoms[i].GetAtom()->IsDeleted() )  continue;
    TDataItem& atom = atoms.AddItem(atom_id++);
    atom.AddField("atom_id", InvolvedAtoms[i].GetAtom()->GetTag());
    atom.AddField("eqiv_id", InvolvedAtoms[i].GetMatrix() == NULL ? -1 : InvolvedAtoms[i].GetMatrix()->GetTag());
  }
}
//..............................................................................
#ifndef _NO_PYTHON
PyObject* TSimpleRestraint::PyExport(TPtrList<PyObject>& atoms, TPtrList<PyObject>& equiv)  {
  PyObject* main = PyDict_New();
  PyDict_SetItemString(main, "allNonH", Py_BuildValue("b", AllNonHAtoms)  );
  PyDict_SetItemString(main, "esd1", Py_BuildValue("d", Esd) );
  PyDict_SetItemString(main, "esd2", Py_BuildValue("d", Esd1) );
  PyDict_SetItemString(main, "value", Py_BuildValue("d", Value) );

  int atom_cnt=0;
  for( int i=0; i < InvolvedAtoms.Count(); i++ )  {
    if( InvolvedAtoms[i].GetAtom()->IsDeleted() )  continue;
    atom_cnt++;
  }

  PyObject* involved = PyTuple_New(atom_cnt);
  atom_cnt = 0;
  for( int i=0; i < InvolvedAtoms.Count(); i++ )  {
    if( InvolvedAtoms[i].GetAtom()->IsDeleted() )  continue;
    PyObject* eq;
    if( InvolvedAtoms[i].GetMatrix() == NULL )
      eq = Py_None;
    else
      eq = equiv[InvolvedAtoms[i].GetMatrix()->GetTag()];
    Py_IncRef(eq);
    Py_IncRef(atoms[InvolvedAtoms[i].GetAtom()->GetTag()]);
    PyTuple_SetItem(involved, atom_cnt++, 
      Py_BuildValue("OO", atoms[InvolvedAtoms[i].GetAtom()->GetTag()], eq));
  }
  PyDict_SetItemString(main, "atoms", involved);
  return main;
}
#endif//..............................................................................
void TSimpleRestraint::FromDataItem(TDataItem& item) {
  AllNonHAtoms = item.GetRequiredField("allNonH").ToBool();
  Esd = item.GetRequiredField("esd").ToDouble();
  Esd1 = item.GetRequiredField("esd1").ToDouble();
  Value = item.GetRequiredField("val").ToDouble();
  TDataItem& atoms = item.FindRequiredItem("atoms");
  for( int i=0; i < atoms.ItemCount(); i++ )  {
    TDataItem& ai = atoms.GetItem(i);
    int aid = ai.GetRequiredField("atom_id").ToInt();
    int eid = ai.GetRequiredField("eqiv_id").ToInt();
    AddAtom( Parent.GetRM().aunit.GetAtom(aid), eid == -1  ? NULL : &Parent.GetRM().GetUsedSymm(eid));
  }
}
//..............................................................................
//..............................................................................
//..............................................................................
//..............................................................................
//..............................................................................
//..............................................................................
void TSRestraintList::Assign(const TSRestraintList& rl)  {
  if( rl.GetRestraintListType() != RestraintListType )
    throw TInvalidArgumentException(__OlxSourceInfo, "list type mismatch");

  Clear();
  for(int i=0; i < rl.Count(); i++)  {
    AddNew().Assign(rl.Restraints[i]);
  }
}
//..............................................................................
void TSRestraintList::ValidateRestraint( TSimpleRestraint& sr )  {
  if( sr.GetListType() != RestraintListType )
    throw TInvalidArgumentException(__OlxSourceInfo, "list type mismatch");
  int AllAtomsInd = -1;
  for(int i=0; i < Restraints.Count(); i++ )  {
    if( Restraints[i].IsAllNonHAtoms() )  {
      AllAtomsInd = i;
      break;
    }
  }
  if( AllAtomsInd != -1 )  {
    for(int i=0; i < Restraints.Count(); i++ )
      if( i != AllAtomsInd )
        Restraints[i].Delete();
  }
  else  {
    for(int i=0; i < Restraints.Count(); i++ )  {
      if( &Restraints[i] == &sr )
        continue;
      Restraints[i].Substruct(sr);
    }
  }
}
//..............................................................................
void TSRestraintList::OnCAtomCrdChange( TCAtom* ca, const smatd& matr )  {
  throw TNotImplementedException(__OlxSourceInfo);
}
//..............................................................................
void TSRestraintList::Release(TSimpleRestraint& sr)  {
  for(int i=0; i < Restraints.Count(); i++ )  {
    if( &Restraints[i] == &sr )  {
      Restraints.Release(i);
      return;
    }
  }
}
//..............................................................................
void TSRestraintList::ToDataItem(TDataItem& item) const {
  int rs_id = 0;
  for( int i=0; i < Restraints.Count(); i++ )  {
    if( !Restraints[i].IsAllNonHAtoms() && Restraints[i].AtomCount() == 0 )  continue;
    Restraints[i].ToDataItem( item.AddItem(rs_id++) );
  }
}
//..............................................................................
#ifndef _NO_PYTHON
PyObject* TSRestraintList::PyExport(TPtrList<PyObject>& atoms, TPtrList<PyObject>& equiv)  {
  int rs_cnt = 0;
  for( int i=0; i < Restraints.Count(); i++ )  {
    if( !Restraints[i].IsAllNonHAtoms() && Restraints[i].AtomCount() == 0 )  continue;
    rs_cnt++;
  }

  PyObject* main = PyTuple_New( rs_cnt );
  rs_cnt = 0;
  for( int i=0; i < Restraints.Count(); i++ )  {
    if( !Restraints[i].IsAllNonHAtoms() && Restraints[i].AtomCount() == 0 )  continue;
    PyTuple_SetItem(main, rs_cnt++, Restraints[i].PyExport(atoms, equiv) );
  }
  return main;
}
#endif//..............................................................................
void TSRestraintList::FromDataItem(TDataItem& item) {
  for( int i=0; i < item.ItemCount(); i++ )
    AddNew().FromDataItem( item.GetItem(i) );
}
//..............................................................................



#ifdef __BORLANDC__
  #pragma package(smart_init)
#endif
