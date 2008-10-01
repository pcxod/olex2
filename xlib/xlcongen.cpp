#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "xlcongen.h"
#include "unitcell.h"

//..............................................................................
TXlConGen::TXlConGen(TIns* ins )  {
  InsFile = ins;
}
//..............................................................................
bool TXlConGen::FixParam(const short paramMask, TStrList& res, const TCAtomPList& atoms, const TFixedValueList& values)  {
  throw TNotImplementedException( __OlxSourceInfo );
}
//..............................................................................
bool TXlConGen::FixAtom( TAtomEnvi& envi, const short Group, const TBasicAtomInfo& atomType, TAtomEnvi* pivoting, TCAtomPList* generated)  {
  try  {
    TAsymmUnit& au = *envi.GetBase().CAtom().GetParent();
    TSimpleRestraint* sr;
    TCAtomPList CreatedAtoms;
    TAtomEnvi NEnvi;
    GenerateAtom( CreatedAtoms, envi, Group, atomType, pivoting);

    short afix = 0;
    switch( Group )  {
      case fgCH3:
        if( envi.Count() == 1 )  {
          //TSAtom *SA = envi.GetBase()->Network()->GetLattice()->FindSAtom( envi.GetLabel(0) );
          //envi.GetBase()->Network()->GetLattice()->GetUnitCell()->GetAtomEnviList(*SA, NEnvi);
          if( NEnvi.Count() > 2 )  // TODO: makes sense?
            afix = 137;
          else
            afix = 137;
        }
        break;
      case fgCH2:
        if( envi.Count() == 2 )
          afix = 23;
        else if( envi.Count() == 1 )
          afix = 93;
        break;
      case fgCH1:
        GenerateAtom( CreatedAtoms, envi, Group, atomType);
        if( envi.Count() == 3 )
          afix = 13;
        else if( envi.Count() == 2 )
          afix = 43;
        else if( envi.Count() == 1 )
          afix = 163;
        break;
      case fgSiH1:
        if( NEnvi.Count() == 3 )
          afix = 13;
        break;
      case fgOH3:
        break;
      case fgOH2:
        if( CreatedAtoms.Count() == 2 )  {
          sr = &au.RestrainedDistances().AddNew();
          sr->SetEsd(0.02);
          sr->SetValue(0.85);
          sr->AddAtomPair(envi.GetBase().CAtom(), NULL, *CreatedAtoms[0], NULL);
          sr->AddAtomPair(envi.GetBase().CAtom(), NULL, *CreatedAtoms[1], NULL);

          sr = &au.RestrainedAngles().AddNew();
          sr->SetEsd(0.04);
          sr->SetValue(1.39);
          sr->AddAtomPair(*CreatedAtoms[1], NULL, *CreatedAtoms[0], NULL);

          if( envi.Count() == 1 )  {
            sr = &au.SimilarDistances().AddNew();
            sr->SetEsd(0.02);
            sr->AddAtomPair(envi.GetCAtom(0), NULL, *CreatedAtoms[0], NULL);
            sr->AddAtomPair(envi.GetCAtom(0), NULL, *CreatedAtoms[1], NULL);
          }
        }
        break;
      case fgSH1:
      case fgOH1:
        if( envi.Count() == 1 )
          afix = 147;
        break;
      case fgNH4:
        break;
      case fgNH3:
        break;
      case fgNH2:
        if( envi.Count() == 1 )  {
          if( pivoting != NULL )
            afix = 93;
          else  {
            if( CreatedAtoms.Count() == 2 )  {
              sr = &au.RestrainedDistances().AddNew();
              sr->SetEsd(0.02);
              sr->SetValue(0.86);
              sr->AddAtomPair(envi.GetBase().CAtom(), NULL, *CreatedAtoms[0], NULL);
              sr->AddAtomPair(envi.GetBase().CAtom(), NULL, *CreatedAtoms[1], NULL);

              sr = &au.RestrainedAngles().AddNew();
              sr->SetEsd(0.04);
              sr->SetValue(1.40);
              sr->AddAtomPair(*CreatedAtoms[1], NULL, *CreatedAtoms[0], NULL);

              if( envi.Count() == 1 )  {
                sr = &au.SimilarDistances().AddNew();
                sr->SetEsd(0.02);
                sr->AddAtomPair(envi.GetCAtom(0), NULL, *CreatedAtoms[0], NULL);
                sr->AddAtomPair(envi.GetCAtom(0), NULL, *CreatedAtoms[1], NULL);
              }
            }
          }
        }
        else if( envi.Count() == 2 )
          afix = 23;
        break;
      case fgNH1:
        if( envi.Count() == 3 )
          afix = 13;
        else if( envi.Count() == 2 )
          afix = 43;
        break;
      case fgBH1:
        if( envi.Count() == 4 ||  envi.Count() == 5 )
          afix = 153;
        break;
    }
    if( afix != 0 )  {
      TAfixGroup& ag = au.GetAfixGroups().New( &envi.GetBase().CAtom(), afix );
      for( int i=0; i < CreatedAtoms.Count(); i++ )
        ag.AddDependent(*CreatedAtoms[i]);
    }
    for( int i=0; i < CreatedAtoms.Count(); i++ )  {
      CreatedAtoms[i]->SetPart( envi.GetBase().CAtom().GetPart() );
      if( envi.GetBase().GetAtomInfo() == iOxygenIndex ||
          envi.GetBase().GetAtomInfo() == iNitrogenIndex )
        CreatedAtoms[i]->SetUisoVar( -1.5 );
      else
        CreatedAtoms[i]->SetUisoVar( -1.2 );
      CreatedAtoms[i]->SetUiso( 4*caDefIso*caDefIso );
      CreatedAtoms[i]->SetOccpVar( envi.GetBase().CAtom().GetOccpVar() );
      CreatedAtoms[i]->SetOccp( envi.GetBase().CAtom().GetOccp() );
      if( generated != NULL )
        generated->Add(CreatedAtoms[i]);
    }

    if( CreatedAtoms.IsEmpty() )  // nothing inserted
      return false;
  }
  catch( TExceptionBase& )  {}
  return false;
}
//..............................................................................

