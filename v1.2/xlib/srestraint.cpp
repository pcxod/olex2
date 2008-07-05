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
  for( int i=0; i < atoms.Count(); i++ )
    InvolvedAtoms.AddNew( atoms[i].GetAtom(), atoms[i].GetMatrix() );
}
//..............................................................................
void TSimpleRestraint::AddAtom(TCAtom& aa, const TMatrixD* ma)  {
  const TMatrixD* tm = NULL;
  if( ma != NULL )  {
    if( !(ma->Data(0)[0] == 1 && ma->Data(0)[1] == 0 && ma->Data(0)[2] == 0 &&
          ma->Data(1)[0] == 0 && ma->Data(1)[1] == 1 && ma->Data(1)[2] == 0 &&
          ma->Data(2)[0] == 0 && ma->Data(2)[1] == 0 && ma->Data(2)[2] == 1 &&
          ma->Data(0)[3] == 0 && ma->Data(1)[3] == 0 && ma->Data(2)[3] == 0) )  {
      tm = &aa.GetParent()->AddUsedSymm( *ma );
    }
  }
  InvolvedAtoms.AddNew( &aa, tm );
}
//..............................................................................
void TSimpleRestraint::AddAtomPair(TCAtom& aa, const TMatrixD* ma,
                                   TCAtom& ab, const TMatrixD* mb)  {
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
        throw TFunctionFailedException(__OlxSourceInfo, "could not locate atoms");
      AddAtom( *aa, sr.InvolvedAtoms[i].GetMatrix() );
    }
  }
}
//..............................................................................
bool TSimpleRestraint::AtomsEqual(TCAtom* a1, const TMatrixD* m1, TCAtom* a2, const TMatrixD* m2)  {
  if( a1 == a2 )  {
    if( (m1 == NULL && m2 == NULL) ||
        ((m1 != NULL && m2 != NULL)  &&
         (*m1 == *m2) ) )  {
      return true;
    }
  }
  return false;
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
void TSimpleRestraint::OnCAtomCrdChange( TCAtom* ca, const TMatrixD& matr )  {
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
void TSRestraintList::OnCAtomCrdChange( TCAtom* ca, const TMatrixD& matr )  {
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
//..............................................................................



#ifdef __BORLANDC__
  #pragma package(smart_init)
#endif
