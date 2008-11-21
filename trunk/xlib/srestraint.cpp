#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "srestraint.h"
#include "asymmunit.h"

TSimpleRestraint::TSimpleRestraint(TSRestraintList* parent, const short listType)  {
  Value = 0;
  Esd = Esd1 = 0;
  ListType = listType;
  Parent = parent;
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
  const smatd* tm = NULL;
  if( ma != NULL )  {
    if( !ma->r.IsI() || ma->t.QLength() != 0 ) 
      tm = &aa.GetParent()->AddUsedSymm( *ma );
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
        InvolvedAtoms[i].GetAtom()->GetParent()->RemUsedSymm( *InvolvedAtoms[i].GetMatrix() );
      InvolvedAtoms.NullItem(i);
      if( ListType == rltBonds )  {
        int ei = i + ((i%2)==0 ? 1 : -1);
        if( InvolvedAtoms[ei].GetMatrix() != NULL )
          InvolvedAtoms[ei].GetAtom()->GetParent()->RemUsedSymm( *InvolvedAtoms[ei].GetMatrix() );
        InvolvedAtoms.NullItem(ei);
      }
      else if( ListType == rltAngles )  {
        if( (i%3) == 0 )  {
          if( InvolvedAtoms[i+1].GetMatrix() != NULL )
            InvolvedAtoms[i+1].GetAtom()->GetParent()->RemUsedSymm( *InvolvedAtoms[i+1].GetMatrix() );
          InvolvedAtoms.NullItem(i+2);
          if( InvolvedAtoms[i+2].GetMatrix() != NULL )
            InvolvedAtoms[i+2].GetAtom()->GetParent()->RemUsedSymm( *InvolvedAtoms[i+2].GetMatrix() );
          InvolvedAtoms.NullItem(i+2);
        }
        else if( (i%3) == 1 )  {
          if( InvolvedAtoms[i-1].GetMatrix() != NULL )
            InvolvedAtoms[i-1].GetAtom()->GetParent()->RemUsedSymm( *InvolvedAtoms[i-1].GetMatrix() );
          InvolvedAtoms.NullItem(i+1);
          if( InvolvedAtoms[i+1].GetMatrix() != NULL )
            InvolvedAtoms[i+1].GetAtom()->GetParent()->RemUsedSymm( *InvolvedAtoms[i+1].GetMatrix() );
          InvolvedAtoms.NullItem(i+1);
        }
        else if( (i%3) == 2 )  {
          if( InvolvedAtoms[i-1].GetMatrix() != NULL )
            InvolvedAtoms[i-1].GetAtom()->GetParent()->RemUsedSymm( *InvolvedAtoms[i-1].GetMatrix() );
          InvolvedAtoms.NullItem(i-2);
          if( InvolvedAtoms[i-2].GetMatrix() != NULL )
            InvolvedAtoms[i-2].GetAtom()->GetParent()->RemUsedSymm( *InvolvedAtoms[i-2].GetMatrix() );
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
    InvolvedAtoms[i].GetAtom()->GetParent()->RemUsedSymm( *InvolvedAtoms[i].GetMatrix() );
  InvolvedAtoms.Delete(i);
}
//..............................................................................
void TSimpleRestraint::Delete()  {
  for( int i=0; i < InvolvedAtoms.Count(); i++ )  {
    if( InvolvedAtoms[i].GetMatrix() != NULL )
      InvolvedAtoms[i].GetAtom()->GetParent()->RemUsedSymm( *InvolvedAtoms[i].GetMatrix() );
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
void TSimpleRestraint::Assign(TAsymmUnit& tau, const TSimpleRestraint& sr)  {
  Clear();
  ListType = sr.GetListType();
  Value = sr.Value;
  Esd = sr.Esd;
  Esd1 = sr.Esd1;
  AllNonHAtoms = sr.AllNonHAtoms;

  if( sr.AtomCount() == 0 )  return;

  TAsymmUnit * au = sr.GetAtom(0).GetAtom()->GetParent();

  if( au == &tau )  {
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
void TSimpleRestraint::FromDataItem(TDataItem& item) {
  throw TNotImplementedException(__OlxSourceInfo);
}
//..............................................................................
//..............................................................................
//..............................................................................
//..............................................................................
//..............................................................................
//..............................................................................
void TSRestraintList::Assign(TAsymmUnit& au, const TSRestraintList& rl)  {
  if( rl.GetRestraintListType() != RestraintListType )
    throw TInvalidArgumentException(__OlxSourceInfo, "list type mismatch");

  Clear();
  for(int i=0; i < rl.Count(); i++)  {
    AddNew().Assign(au, rl.Restraints[i]);
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
  olxstr list_type;
  switch( RestraintListType )  {
    case rltNone:
      list_type = "none";
      break;
    case rltAtoms:
      list_type = "atoms";
      break;
    case rltBonds:
      list_type = "bonds";
      break;
    case rltAngles:
      list_type = "angles";
      break;
    case rltGroup:
      list_type = "group";
      break;
  }
  item.AddField("list_type", list_type);
  int rs_id = 0;
  for( int i=0; i < Restraints.Count(); i++ )  {
    if( !Restraints[i].IsAllNonHAtoms() && Restraints[i].AtomCount() == 0 )  continue;
    Restraints[i].ToDataItem( item.AddItem(rs_id++) );
  }
}
//..............................................................................
void TSRestraintList::FromDataItem(TDataItem& item) {
  throw TNotImplementedException(__OlxSourceInfo);
}
//..............................................................................



#ifdef __BORLANDC__
  #pragma package(smart_init)
#endif
