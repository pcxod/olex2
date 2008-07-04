//---------------------------------------------------------------------------//
// namespace TXClasses: TLattice
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "lattice.h"
#include "asymmunit.h"
#include "unitcell.h"
#include "network.h"

#include "sbond.h"
#include "splane.h"
#include "ellipsoid.h"

#include "bapp.h"
#include "log.h"

#include "emath.h"
#include "congen.h"

#include "estlist.h"
#include "library.h"

#include "olxmps.h"
#include "estrbuffer.h"

#undef GetObject

//---------------------------------------------------------------------------
// TLattice function bodies
//---------------------------------------------------------------------------
TLattice::TLattice(TAtomsInfo *Info)  {
  AtomsInfo = Info;
  Generated = false;

  AsymmUnit   = new TAsymmUnit(this, Info);
  UnitCell    = new TUnitCell(this);
  Network     = new TNetwork(this, NULL);
  Delta    = 0.5f;
  DeltaI    = 1.2f;
  TBasicApp::GetInstance()->NewActionQueue("STRGEN");
  TBasicApp::GetInstance()->NewActionQueue("STRUNIQ");
  TBasicApp::GetInstance()->NewActionQueue("ATOMADD");
}
//..............................................................................
TLattice::~TLattice()  {
  Clear(true);

  delete UnitCell;
  delete AsymmUnit;
  delete Network;
}
//..............................................................................
void  TLattice::ClearAtoms()  {
  for( int i=0; i < Atoms.Count(); i++ )
    delete Atoms[i];
  Atoms.Clear();
}
//..............................................................................
void  TLattice::ClearBonds()  {
  for( int i=0; i < Bonds.Count(); i++ )
    delete Bonds[i];
  Bonds.Clear();
}
//..............................................................................
void  TLattice::ClearFragments()  {
  for( int i=0; i < Fragments.Count(); i++ )
    delete Fragments[i];
  Fragments.Clear();
}
//..............................................................................
void  TLattice::ClearMatrices()  {
  for( int i=0; i < Matrices.Count(); i++ )
    delete Matrices[i];
  Matrices.Clear();
}
//..............................................................................
void  TLattice::ClearPlanes()  {
  for( int i=0; i < Planes.Count(); i++ )
    delete Planes[i];
  Planes.Clear();
}
//..............................................................................
void  TLattice::Clear(bool ClearUnitCell)  {
  Generated = false;
  if( ClearUnitCell )  {
    GetUnitCell().Clear();
    GetAsymmUnit().Clear();
  }
  ClearAtoms();
  ClearBonds();
  ClearFragments();
  ClearMatrices();
  ClearPlanes();
}
//..............................................................................
void TLattice::AddSBond(TSBond *B)  {
  B->SetLatId( Bonds.Count() );
  Bonds.Add( B );
}
//..............................................................................
void TLattice::AddSAtom(TSAtom *A)  {
  A->SetLatId( Atoms.Count() );
  Atoms.Add(A);
}
//..............................................................................
void TLattice::AddSPlane(TSPlane *P)  {
  P->SetLatId( Planes.Count() );
  Planes.Add(P);
}
//..............................................................................
int TLattice::GenerateMatrices(const TVPointD& VFrom, const TVPointD& VTo,
  const TVPointD& MFrom, const TVPointD& MTo)  {
  GenerateMatrices(Matrices, VFrom, VTo, MFrom, MTo);
  return MatrixCount();
}
//..............................................................................
int  TLattice::GenerateMatrices(TMatrixDPList& Result,
     const TVPointD& VFrom, const TVPointD& VTo,
     const TVPointD& MFrom, const TVPointD& MTo)  {
  olxstr Tmp;
  TMatrixD *M, *M1;
  int mstart = Result.Count();
  TVPointD Center, C;
  Center = GetAsymmUnit().GetOCenter(true, false);

  Result.SetCapacity( (int)(GetUnitCell().MatrixCount()*
                              (fabs(VTo[0]-VFrom[0])+1)*
                              (fabs(VTo[1]-VFrom[1])+1)*
                              (fabs(VTo[2]-VFrom[2])+1)) );

  for( int i=0; i < GetUnitCell().MatrixCount(); i++ )  {
    for( int j=(int)VFrom[0]; j <= (int)VTo[0]; j++ )
      for( int k=(int)VFrom[1]; k <= (int)VTo[1]; k++ )
      for( int l=(int)VFrom[2]; l <= (int)VTo[2]; l++ )  {
        M = new TMatrixD(GetUnitCell().GetMatrix(i));
        M->SetTag(i);  // set Tag to identify the matrix (and ellipsoids) in the UnitCell
        M->Data(0)[3] += j;
        M->Data(1)[3] += k;
        M->Data(2)[3] += l;
        Result.Add(M);
      }
  }

// Analysis of matrixes: check if the center of gravity of the asymmetric unit
// is inside the generation volume
  for( int i=mstart; i < Result.Count(); i++ )  {
    M = Result[i];
    C = *M * Center;
    C[0] += M->Data(0)[3];    C[1] += M->Data(1)[3];    C[2] += M->Data(2)[3];
    if( C[0] > MTo[0] || C[0] < MFrom[0])  {
      Result[i] = NULL;
      delete M;   continue;
    }
    if(  C[1] > MTo[1] || C[1] < MFrom[1] )  {
      Result[i] = NULL;
      delete M;   continue;
    }
    if( C[2] > MTo[2] || C[2] < MFrom[2] )  {
      Result[i] = NULL;
      delete M;   continue;
    }
  }
  Result.Pack();
  for( int i=0; i < mstart; i++ )  {  // check if al matrices are uniq
    M = Result[i];
    for( int j=mstart; j < Result.Count(); j++ )  {
      M1 = Result[j];
      if( !M1 )  continue;
      if( M1->GetTag() == M->GetTag() )  {
        if( M->Data(0)[3] == M1->Data(0)[3] )
          if( M->Data(1)[3] == M1->Data(1)[3] )
            if( M->Data(2)[3] == M1->Data(2)[3] )  {
              delete M1;
              Result.Delete(j);
              break;
            }
      }
    }
  }
  Result.Pack();
  return Result.Count();
}
//..............................................................................
int CompareNets(const TNetwork* N, const TNetwork* N1)  {
  return N1->NodeCount() - N->NodeCount();
}
//..............................................................................
void TLattice::InitBody()  {
  TNetwork *Frag;
  int conIndex;

  TMatrixD *M = new TMatrixD(3, 4); // create identity matrix
  *M = GetUnitCell().GetMatrix(0);
  M->SetTag(0);
  Matrices.Add( M );
  ListAsymmUnit(Atoms, NULL, true);
  Network->Disassemble(Atoms, Fragments, &Bonds);
  for( int i=0; i < Atoms.Count(); i++ )
    Atoms[i]->SetLatId(i);
  TNetPList::QuickSorter.SortSF(Fragments, CompareNets);
  // precalculate memory usage
  int bondCnt = Bonds.Count();
  for(int i=0; i < Fragments.Count(); i++ )
    bondCnt += Fragments[i]->BondCount();
  Bonds.SetCapacity( bondCnt );
  // end
  for(int i=0; i < Fragments.Count(); i++ )  {
    Frag = Fragments[i];
    for( int j=0; j < Frag->BondCount(); j++ )
      AddSBond(&Frag->Bond(j));
    for( int j=0; j < Frag->NodeCount(); j++ )  {
      TSAtom& SAtom = Frag->Node(j);
      TCAtom& CAtom = SAtom.CAtom();
      CAtom.SetFragmentId(i);
      if( CAtom.GetAfix() )  {
        if( SAtom.NodeCount() == 1 )
          CAtom.SetAfixAtomId( SAtom.Node(0).CAtom().GetLoaderId() );
        else  {
          conIndex = 0;
          for( int k=0; k < SAtom.NodeCount(); k++ )  {
            if( SAtom.Node(k).GetAtomInfo() != iQPeakIndex )  {
              conIndex ++;
              CAtom.SetAfixAtomId( SAtom.Node(k).CAtom().GetLoaderId() );
            }
          }
          if( conIndex > 1 )  {
            //TBasicApp::GetLog()->CriticalInfo( (olxstr("Bad connectivity for: ") << SAtom->GetLabel()) <<
            //  ". Please fix the problem and reload file"
            //);
            CAtom.SetAfixAtomId(-1);
          }
        }
      }
    }
  }
}
void TLattice::Init()  {
  Clear(false);
  GetUnitCell().ClearEllipsoids();
  GetUnitCell().InitMatrices();
  InitBody();
  TEStrBuffer tmp;
  int eqc = GetUnitCell().FindSymmEq(tmp, 0.01, true, false, false); // find and not remove
  GetAsymmUnit().SetContainsEquivalents( eqc != 0 );

/*  // decode fixed Uiso parameters
  for( i=0; i < AtomCount(); i++ )
  {
    A = Atom(i);
    CA = A->CAtom();
    if( CA->UisoVar() )
    {
      if( CA->AtomInfo()->Index == iHydrogenIndex ) // "special" treatment for H
      {
        CA->Uiso(4*caDefIso*caDefIso);
      }
      else
      {
        if( A->NodeCount() == 1 )
        {
          A1 = (TSAtom*)A->Node(0);
          if( !A1->CAtom()->UisoVar() )
            CA->Uiso(A1->CAtom()->Uiso()*fabs(CA->UisoVar()));
        }
      }
    }
  }*/
  /*
  for( int i=0; i < Atoms.Count(); i++ )  {
    TCAtom* CA = Atoms[i]->CAtom();
    if( CA->GetAtomInfo()->GetIndex() == iHydrogenIndex ) // "special" treatment for H
      CA->SetUiso(4*caDefIso*caDefIso);
  }
  */
  Generated = false;
}
//..............................................................................
void  TLattice::Uniq(bool remEqv)  {
  TBasicApp::GetInstance()->ActionQueue("STRUNIQ")->Enter(this);

//  UpdateAsymmUnit();  // need to cal it here, the call in ListAsymmUnit
  // will not be executed as Atoms->Count() = 0
  Clear(false);
  ClearMatrices();
  if( GetAsymmUnit().DoesContainEquivalents() && remEqv )  {
    TEStrBuffer Msg;
    GetUnitCell().FindSymmEq(Msg, 0.01, false, false, true);
    //TBasicApp::GetLog()->CriticalInfo(olxstr("Symmetrical counterparts were removed for: ") << Msg.ToString() );
    GetAsymmUnit().SetContainsEquivalents(false);
  }
  InitBody();
  Generated = false;
  TBasicApp::GetInstance()->ActionQueue("STRUNIQ")->Exit(this);
}
//..............................................................................
void TLattice::GenerateAtoms(const TSAtomPList& atoms, TSAtomPList& result, const TMatrixDPList& matrices)  {
  if( !atoms.Count() )  return;

//  result->SetCount(mc*ac);
  for(int i=0; i < matrices.Count(); i++ )  {
    TMatrixD* M = matrices[i];
    for(int j=0; j < atoms.Count(); j++ )  {
      if( atoms[j]->IsDeleted() )  continue;
      TSAtom* A = new TSAtom( Network );
      A->CAtom( atoms[j]->CAtom() );
      A->CCenter() = *M * A->CCenter();
      A->CCenter()[0] += M->Data(0)[3];
      A->CCenter()[1] += M->Data(1)[3];
      A->CCenter()[2] += M->Data(2)[3];
      GetAsymmUnit().CellToCartesian( A->CCenter(), A->Center() );
      if( atoms[j]->CAtom().GetLoaderId() != -1 )   // for a centroid or whatsoever == -1
      A->SetEllipsoid( &GetUnitCell().GetEllipsoid(M->GetTag(), atoms[j]->CAtom().GetId()) );
      A->AddMatrix(M);
      result.Add(A);
    }
  }
}
//..............................................................................
void TLattice::GenerateWholeContent(TCAtomPList* Template)  {
  TBasicApp::GetInstance()->ActionQueue("STRGEN")->Enter(this);
//  ClearAtoms();
//  ListAsymmUnit(*Atoms(), Template);
  Generated = false; // force the procedure
  Generate(Template, false, true);
  TBasicApp::GetInstance()->ActionQueue("STRGEN")->Exit(this);
}
//..............................................................................
void  TLattice::Generate(TCAtomPList* Template, bool ClearCont, bool IncludeQ)  {
/*  if( Generated )
  {
    TBasicApp::GetLog()->Error("TLattice: Structure has been already generated");
    return;
  }*/
  if( ClearCont && Template)  {
    ClearAtoms();
  }
  else  {
    for(int i=0; i < Atoms.Count(); i++ )  {  // restore atom coordinates
      TSAtom* A = Atoms[i];
      if( A->IsDeleted() )  {
        delete A;
        Atoms[i] = NULL;
        continue;
      }
      A->Center() = A->CCenter();
      GetAsymmUnit().CellToCartesian(A->CCenter(), A->Center());
    }
  }
  Atoms.Pack();
  TSAtomPList AtomsList;
  ListAsymmUnit(AtomsList, Template, IncludeQ);
  GenerateAtoms(AtomsList, Atoms, Matrices);
  for( int i=0; i < AtomsList.Count(); i++ )
    delete AtomsList[i];
  Disassemble();
  Generated = true;
}
//..............................................................................
void TLattice::Generate(const TVPointD& MFrom, const TVPointD& MTo, TCAtomPList* Template,
       bool ClearCont, bool IncludeQ)  {
//  if( Generated )
//  {
//    TBasicApp::GetLog()->Error("TLattice: Structure has been already generated");
//    return;
//  }
  if( GetAsymmUnit().DoesContainEquivalents() )  {
    TBasicApp::GetLog().Error("TLattice:: Asymmetric unit contains symmetrical equivalents.");
    return;
  }
  TVPointD VFrom, VTo;

  VTo[0] = Round(MTo[0]+1);     VTo[1] = Round(MTo[1]+1);     VTo[2] = Round(MTo[2]+1);
  VFrom[0] = Round(MFrom[0]-1); VFrom[1] = Round(MFrom[1]-1); VFrom[2] = Round(MFrom[2]-1);

  GenerateMatrices(VFrom, VTo, MFrom, MTo);
  TBasicApp::GetInstance()->ActionQueue("STRGEN")->Enter(this);
  Generate(Template, ClearCont, IncludeQ);
  TBasicApp::GetInstance()->ActionQueue("STRGEN")->Exit(this);
}
//..............................................................................
bool TLattice::IsExpandable(TSAtom& A) const {
  return (A.CAtom().GetCanBeGrown() && !A.IsGrown());
}
//..............................................................................
//..............................................................................
void TLattice::DoGrow(const TSAtomPList& atoms, bool GrowShell, TCAtomPList* Template)  {
  int l;
  TSAtom* SA, *SA1;
  bool found;
  TVPointD V;
  // all matrices after MatrixCount are new and has to be used for generation
  int currentCount = MatrixCount();
  // the fragmens to grow by a particular matrix
  TEList Fragments2Grow;
  TMatrixDList *BindingMatrices;

  TTypeList<int> *ToGrow;
  TBasicApp::GetInstance()->ActionQueue("STRGEN")->Enter(this);
  for(int i=0; i < atoms.Count(); i++ )  {
    SA = atoms[i];
    SA->SetGrown(true);
    TCAtom& CA = SA->CAtom();
    for(int j=0; j < CA.AttachedAtomCount(); j++ )  {
      TCAtom& CA1 = CA.GetAttachedAtom(j);
      V = CA1.CCenter();
      BindingMatrices = GetUnitCell().GetBinding(CA, CA1, SA->CCenter(), V, false, false);
      if( BindingMatrices->Count() )  {
        for( int k=0; k < BindingMatrices->Count(); k++ )  {
          TMatrixD& M = BindingMatrices->Item(k);
          found = false;
          for( l=0; l < MatrixCount(); l++ )  {
            if( *Matrices[l] == M )  {
              found = true;  break;
            }
          }
          if( !found )  {
            Matrices.Add( new TMatrixD(M) );
            ToGrow = new TTypeList<int>;
            ToGrow->AddACopy( CA1.GetFragmentId() );
            Fragments2Grow.Add(ToGrow);
          }
          else  {
            if( l >= currentCount )  {
              ToGrow = (TTypeList<int>*)Fragments2Grow[l-currentCount];
              found = false;
              for(int m=0; m < ToGrow->Count(); m++ )  {
                if( ToGrow->Item(m) == CA1.GetFragmentId() )  {
                  found =false;  break;
                }
              }
              if( !found )  ToGrow->AddACopy( CA1.GetFragmentId() );
            }
          }
        }
      }
      delete BindingMatrices;
    }
  }
  for(int i = currentCount; i < MatrixCount(); i++ )  {
    TMatrixD* M = Matrices[i];
    ToGrow = (TTypeList<int>*)Fragments2Grow[i-currentCount];
    for(int j=0; j < GetAsymmUnit().AtomCount(); j++ )  {
      for(int k=0; k < ToGrow->Count(); k++ )  {
        if( GetAsymmUnit().GetAtom(j).IsDeleted() )  continue;
        if( GetAsymmUnit().GetAtom(j).GetFragmentId() == ToGrow->Item(k) )  {
          SA1 = new TSAtom( Network );
          SA1->CAtom( GetAsymmUnit().GetAtom(j) );
          SA1->AddMatrix(M);
          SA1->CCenter() = *M * SA1->CCenter();
          SA1->CCenter()[0] += M->Data(0)[3];
          SA1->CCenter()[1] += M->Data(1)[3];
          SA1->CCenter()[2] += M->Data(2)[3];
          SA1->SetEllipsoid( &GetUnitCell().GetEllipsoid(M->GetTag(), SA1->CAtom().GetId()) );
          AddSAtom(SA1);
        }
      }
    }
  }
  for(int i=0; i < Fragments2Grow.Count(); i++ )
    delete (TTypeList<int>*)Fragments2Grow[i];

  RestoreCoordinates();
  Disassemble();

  Generated = true;
  TBasicApp::GetInstance()->ActionQueue("STRGEN")->Exit(this);
}
//..............................................................................
void TLattice::GrowFragments(bool GrowShells, TCAtomPList* Template)  {
  TSAtomPList TmpAtoms;
  for( int i=0; i < Atoms.Count(); i++ )  {
    TSAtom* A = Atoms[i];
    if( A->IsDeleted() )  continue;
    for( int j=0; j < A->NodeCount(); j++ )  {
      if( A->Node(j).IsDeleted() )
        A->NullNode(j);
    }
    A->PackNodes();
    if( IsExpandable(*A) )
      TmpAtoms.Add(A);
  }
  if( TmpAtoms.Count() )
    GrowAtoms(TmpAtoms, GrowShells, Template);
}
//..............................................................................
void TLattice::GrowAtoms(const TSAtomPList& atoms, bool GrowShells, TCAtomPList* Template)  {
  if( !atoms.Count() )  return;
/* restore atom centres if were changed by some other procedure */
  RestoreCoordinates();
  DoGrow(atoms, GrowShells, Template);
}
//..............................................................................
void TLattice::GrowAtom(TSAtom& Atom, bool GrowShells, TCAtomPList* Template)  {
  if( !IsExpandable(Atom) )  return;
  TSAtomPList atoms;
/* restore atom centres if were changed by some other procedure */
  RestoreCoordinates();
  Atoms.Add(&Atom);
  DoGrow(atoms, GrowShells, Template);
}
//..............................................................................
void TLattice::GrowAtom(int FragId, const TMatrixD& transform)  {
  TMatrixD *M;
  // check if the matix is unique
  bool found = false;
  for( int i=0; i < Matrices.Count(); i++ )  {
    if( *Matrices[i] == transform )  {
      M = Matrices[i];
      found = true;
      break;
    }
  }
  if( !found )  {
    M = new TMatrixD( transform );
    M->SetTag( transform.GetTag() );
    Matrices.Add( M );
  }

  TBasicApp::GetInstance()->ActionQueue("STRGEN")->Enter(this);

  for( int i=0; i < GetAsymmUnit().AtomCount(); i++ )  {
    if( GetAsymmUnit().GetAtom(i).IsDeleted() )  continue;
    if( GetAsymmUnit().GetAtom(i).GetFragmentId() == FragId )  {
      TSAtom* SA = new TSAtom( Network );
      SA->CAtom( GetAsymmUnit().GetAtom(i) );
      SA->AddMatrix( M );
      SA->CCenter() = *M * SA->CCenter();
      SA->CCenter()[0] += M->Data(0)[3];
      SA->CCenter()[1] += M->Data(1)[3];
      SA->CCenter()[2] += M->Data(2)[3];
      SA->SetEllipsoid( &GetUnitCell().GetEllipsoid(M->GetTag(), SA->CAtom().GetId()) );
      AddSAtom(SA);
    }
  }
  RestoreCoordinates();
  Disassemble();

  Generated = true;
  TBasicApp::GetInstance()->ActionQueue("STRGEN")->Exit(this);
}
//..............................................................................
void TLattice::GrowAtoms(const TSAtomPList& atoms, const TMatrixDList& matrices)  {
  TMatrixD *M;
  TMatrixDPList addedMatrices;
  // check if the matices is unique
  for( int i=0; i < matrices.Count(); i++ )  {
    bool found = false;
    for( int j=0; j < Matrices.Count(); j++ )  {
      if( *Matrices[j] == matrices[i] )  {
        found = true;
        break;
      }
    }
    if( !found )  {
      M = new TMatrixD( matrices[i] );
      // we do not know abou the origin of this matrix ...
      M->SetTag( 0 );
      Matrices.Add( M );
      addedMatrices.Add( M );
    }
  }

  TBasicApp::GetInstance()->ActionQueue("STRGEN")->Enter(this);
  Atoms.SetCapacity( Atoms.Count() + atoms.Count()*addedMatrices.Count() );
  for( int i=0; i < addedMatrices.Count(); i++ )  {
    for( int j=0; j < atoms.Count(); j++ )  {
      if( atoms[j]->IsDeleted() )  continue;
      TSAtom* SA = new TSAtom( Network );
      SA->CAtom( atoms[j]->CAtom() );
      SA->AddMatrix( addedMatrices[i] );
      SA->CCenter() = *addedMatrices[i] * SA->CCenter();
      SA->CCenter()[0] += addedMatrices[i]->Data(0)[3];
      SA->CCenter()[1] += addedMatrices[i]->Data(1)[3];
      SA->CCenter()[2] += addedMatrices[i]->Data(2)[3];
      SA->SetEllipsoid( &GetUnitCell().GetEllipsoid(M->GetTag(), SA->CAtom().GetId()) );
      AddSAtom(SA);
    }
  }
  RestoreCoordinates();
  Disassemble();

  Generated = true;
  TBasicApp::GetInstance()->ActionQueue("STRGEN")->Exit(this);
}
//..............................................................................
void TLattice::Grow(const TMatrixD& transform)  {
  TMatrixD *M;
  // check if the matix is unique
  bool found = false;
  for( int i=0; i < Matrices.Count(); i++ )  {
    if( *Matrices[i] == transform )  {
      M = Matrices[i];
      found = true;
      break;
    }
  }
  if( !found )  {
    M = new TMatrixD( transform );
    M->SetTag( transform.GetTag() );
    Matrices.Add( M );
  }

  TBasicApp::GetInstance()->ActionQueue("STRGEN")->Enter(this);
  Atoms.SetCapacity( Atoms.Count() + GetAsymmUnit().AtomCount() );
  for( int i=0; i < GetAsymmUnit().AtomCount(); i++ )  {
    if( GetAsymmUnit().GetAtom(i).IsDeleted() )  continue;
    TSAtom* SA = new TSAtom( Network );
    SA->CAtom( GetAsymmUnit().GetAtom(i) );
    SA->AddMatrix( M );
    SA->CCenter() = (*M) * SA->CCenter();
    SA->CCenter()[0] += M->Data(0)[3];
    SA->CCenter()[1] += M->Data(1)[3];
    SA->CCenter()[2] += M->Data(2)[3];
    SA->SetEllipsoid( &GetUnitCell().GetEllipsoid(M->GetTag(), SA->CAtom().GetId()) );
    AddSAtom(SA);
  }
  RestoreCoordinates();
  Disassemble();

  Generated = true;
  TBasicApp::GetInstance()->ActionQueue("STRGEN")->Exit(this);
}
//..............................................................................
TSAtom* TLattice::FindSAtom(const olxstr &Label) const {
  for( int i =0; i < Atoms.Count(); i++ )
    if( !Label.Comparei( Atoms[i]->GetLabel()) )  return Atoms[i];
  return NULL;
}
//..............................................................................
TSAtom* TLattice::NewCentroid(const TSAtomPList& Atoms)  {
  TSAtom *Centroid;
  TVPointD C, CC;
  double aan = 0;
  for( int i=0; i < Atoms.Count(); i++ )  {
    C += Atoms[i]->Center()*Atoms[i]->CAtom().GetOccp();
    CC += Atoms[i]->CCenter()*Atoms[i]->CAtom().GetOccp();
    aan += Atoms[i]->CAtom().GetOccp();
  }
  if( aan == 0 )  return NULL;
  C /= aan;
  CC /= aan;
  TCAtom *CCent;
  try{ CCent = &AsymmUnit->NewCentroid(CC); }
  catch(const TExceptionBase& exc)  {
    throw TFunctionFailedException(__OlxSourceInfo, exc.Replicate());
  }
  Centroid = new TSAtom( Network );
  Centroid->CAtom(*CCent);
  Centroid->Center() = C; // if it has been moved
  Centroid->AddMatrix( Matrices[0] );
  AddSAtom(Centroid);
  return Centroid;
}
//..............................................................................
TSAtom* TLattice::NewAtom(const TVPointD& center)  {
  TCAtom* ca = NULL;
  try{ ca = &AsymmUnit->NewAtom();  }
  catch(const TExceptionBase& exc)  {
    throw TFunctionFailedException(__OlxSourceInfo, exc.Replicate());
  }
  ca->SetLoaderId( liNewAtom );
  ca->CCenter() = center;
  TSAtom* a = new TSAtom( Network );
  a->CAtom(*ca);
  a->AddMatrix( Matrices[0] );
  AddSAtom(a);
  GetUnitCell().AddEllipsoid();
  return a;
}
//..............................................................................
TSPlane* TLattice::NewPlane(const TSAtomPList& Atoms, int weightExtent)  {
  TSPlane *Plane = TmpPlane(Atoms, weightExtent);
  if( Plane )  AddSPlane(Plane);
  return Plane;
}
//..............................................................................
TSPlane* TLattice::TmpPlane(const TSAtomPList& atoms, int weightExtent)  {
  if( atoms.Count() < 3 )  return NULL;
  TSPlane *Plane;
  TVectorD Params(3);
  TVPointD Z;
  //TODO: need to consider occupancy for disordered groups ...
  TTypeList< AnAssociation2<TVPointD, double> > Points;
  Points.SetCapacity( atoms.Count() );

  if( weightExtent != 0 )  {
    for(int i=0; i < atoms.Count(); i++ )
      Points.AddNew(atoms[i]->Center(), pow(atoms[i]->GetAtomInfo().GetMr(), (double)weightExtent) );
  }
  else  {
    for(int i=0; i < atoms.Count(); i++ )
      Points.AddNew(atoms[i]->Center(), 1 );
  }

  TSPlane::CalcPlane(Points, Params);
  Plane = new TSPlane( Network );
  // plane equation in this case is: Z = Params[0]*X + Params[1]*Y + Params[2];
  Z[0] = Params[0];
  Z[1] = Params[1];
  Z[2] = -1;
  Plane->D(Params[2]/Z.Length());
  Z.Normalise();
  Plane->Normal() = Z;
  for(int i=0; i < Points.Count(); i++ )
    Plane->AddCrd( Points[i].GetA() );
  return Plane;
}
//..............................................................................
void TLattice::UpdateAsymmUnit()  {
  if( !AtomCount() )  return;
  int ac = GetAsymmUnit().AtomCount();
  GetAsymmUnit().InitAtomIds();
  TEList AUAtoms;
  TEList *AtomG;
  TSAtom* OA;
  for( int i=0; i < GetAsymmUnit().AtomCount(); i++ )  {  // create lists to store atom groups
    AtomG = new TEList;
    AUAtoms.Add(AtomG);
  }
  for( int i=0; i < GetAsymmUnit().CentroidCount(); i++ )  {  // create lists to store atom groups
    AtomG = new TEList;
    AUAtoms.Add(AtomG);
  }
  for( int i=0; i < Atoms.Count(); i++ )  {
    TSAtom* A = Atoms[i];
    if( A->IsDeleted() )  continue;
    if( A->CAtom().GetLoaderId() == liCentroid )  // a centroid
      AtomG = (TEList*)AUAtoms[GetAsymmUnit().AtomCount() + A->CAtom().GetId()];
    else
      AtomG = (TEList*)AUAtoms[A->CAtom().GetId()];
    AtomG->Add(A);
  }
  for( int i=0; i < AUAtoms.Count(); i++ )  {  // create lists to store atom groups
    AtomG = (TEList*)AUAtoms[i];
    if( !AtomG->Count() )  {  // all atoms are deleted
      if( i >= ac ) AsymmUnit->GetCentroid(i-ac).SetDeleted(true);
      else          AsymmUnit->GetAtom(i).SetDeleted(true);
      continue;
    }
    // find the original atom, or symmetry equivalent if removed
    for( int j=0; j < AtomG->Count(); j++ )  {
      TSAtom* A = (TSAtom*)AtomG->Item(j);
      if( A->GetMatrix(0).IsE() )
      { OA = A;  break; } // original atom is not deleted
      OA = A;   // remmebred any not deleted atom
    }
    TCAtom* CA;
    if( i >= ac )  CA = &GetAsymmUnit().GetCentroid(i-ac);
    else           CA = &GetAsymmUnit().GetAtom(i);
    CA->SetDeleted(false);
//    if( CA->LoaderId() == -1 )  continue;
    if( OA->GetEllipsoid() )  {
      CA->UpdateEllp(*OA->GetEllipsoid());
    }
    CA->CCenter() = OA->CCenter();
  }
  for( int i=0; i < AUAtoms.Count(); i++ ) // cleenup memory
    delete (TEList*)AUAtoms[i];
  TEStrBuffer Rep;
  if( GetAsymmUnit().DoesContainEquivalents() )
    AsymmUnit->SetContainsEquivalents( UnitCell->FindSymmEq(Rep, 0.01, false, false, false) != 0 );
}
//..............................................................................
void TLattice::ListAsymmUnit(TSAtomPList& L, TCAtomPList* Template, bool IncludeQ)  {
  if( Template != NULL )  {
    for( int i=0; i < Template->Count(); i++ )  {
      if( Template->Item(i)->IsDeleted() )  continue;
      TSAtom* A = new TSAtom( Network );
      A->CAtom( *Template->Item(i) );
      A->SetEllipsoid( &GetUnitCell().GetEllipsoid(0, Template->Item(i)->GetId()) ); // ellipsoid for the identity matrix
      A->AddMatrix( Matrices[0] );
      A->SetLatId( L.Count() );
      L.Add(A);
    }
  }
  else  {
    for( int i=0; i < GetAsymmUnit().AtomCount(); i++ )    {
      TCAtom& CA = GetAsymmUnit().GetAtom(i);
      if( CA.IsDeleted() )  continue;
      if( !IncludeQ && CA.GetAtomInfo() == iQPeakIndex )  continue;
      TSAtom* A = new TSAtom( Network );
      A->CAtom(CA);
      if( CA.GetLoaderId() >=0 )
        A->SetEllipsoid( &GetUnitCell().GetEllipsoid(0, CA.GetId()) ); // ellipsoid for the identity matrix
      A->AddMatrix( Matrices[0] );
      A->SetLatId( L.Count() );
      L.Add(A);
    }
    //AsymmUnit()->CollapseExyz();
    for( int i=0; i < GetAsymmUnit().CentroidCount(); i++ )  {
      TCAtom& CA = GetAsymmUnit().GetCentroid(i);
      if( CA.IsDeleted() )  continue;
      TSAtom* A = new TSAtom( Network );
      A->CAtom(CA);
      //A->SetEllipsoid( &GetUnitCell().GetEllipsoid(0, CA->GetId() )); // ellipsoid for the identity matrix
      A->AddMatrix( Matrices[0] );
      A->SetLatId( L.Count() );
      L.Add(A);
    }
  }
}
//..............................................................................
void TLattice::MoveFragment(const TVPointD& to, TSAtom& fragAtom)  {
  if( Generated )  {
    TBasicApp::GetLog().Error("Cannot perform this operation on grown structure");
    return;
  }

  TMatrixD abc2xyz( GetAsymmUnit().GetCellToCartesian()),
           xyz2abc( GetAsymmUnit().GetCartesianToCell()), NM(3,3);
  abc2xyz.Transpose();
  xyz2abc.Transpose();

  TVPointD from;
  from = fragAtom.CCenter();
  TMatrixD* m = GetUnitCell().GetClosest(to, from, true);
  TVPointD offset;
  if( m != NULL )  {
    for(int i=0; i < fragAtom.GetNetwork().NodeCount(); i++ )  {
      TSAtom& SA = fragAtom.GetNetwork().Node(i);
      SA.CAtom().CCenter() *= *m;
      offset[0] = m->Data(0)[3];
      offset[1] = m->Data(1)[3];
      offset[2] = m->Data(2)[3];
      SA.CAtom().CCenter() += offset;
      if( SA.CAtom().GetEllipsoid() != NULL )  {
        for( int mk=0; mk < 3; mk++ )
          for( int ml=0; ml < 3; ml++ )
            NM[mk][ml] = m->Data(mk)[ml];
        NM = abc2xyz*NM*xyz2abc;
        SA.CAtom().GetEllipsoid()->MultMatrix(NM);
      }
    }
    delete m;
    Uniq();
  }
  else
  {
    TBasicApp::GetLog().Info("Could not find closest matrix");
  }
}
//..............................................................................
void TLattice::MoveFragment(TSAtom& to, TSAtom& fragAtom)  {
  if( Generated )  {
    TBasicApp::GetLog().Error("Cannot perform this operation on grown structure");
    return;
  }

  TMatrixD abc2xyz( GetAsymmUnit().GetCellToCartesian()),
           xyz2abc( GetAsymmUnit().GetCartesianToCell()), NM(3,3);
  abc2xyz.Transpose();
  xyz2abc.Transpose();

  TMatrixD* m = GetUnitCell().GetClosest(to.CAtom(), fragAtom.CAtom(), true);
  TVPointD offset;
  if( m != NULL )  {
    if( to.CAtom().GetFragmentId() == fragAtom.CAtom().GetFragmentId() )  {
      fragAtom.CAtom().CCenter() *= *m;
      offset[0] = m->Data(0)[3];
      offset[1] = m->Data(1)[3];
      offset[2] = m->Data(2)[3];
      fragAtom.CAtom().CCenter() += offset;
      if( fragAtom.CAtom().GetEllipsoid() != NULL )
        fragAtom.CAtom().GetEllipsoid()->MultMatrix(*m);
    }
    else  {  // move whole fragment then
      int fragId = fragAtom.CAtom().GetFragmentId();
      for(int i=0; i < Atoms.Count(); i++ )  {
        TSAtom* SA = Atoms[i];
        if( SA->CAtom().GetFragmentId() == fragId )  {
          SA->CAtom().CCenter() *= *m;
          offset[0] = m->Data(0)[3];
          offset[1] = m->Data(1)[3];
          offset[2] = m->Data(2)[3];
          SA->CAtom().CCenter() += offset;
          if( SA->CAtom().GetEllipsoid() != NULL )  {
            for( int mk=0; mk < 3; mk++ )
              for( int ml=0; ml < 3; ml++ )
                NM[mk][ml] = m->Data(mk)[ml];
            NM = abc2xyz*NM*xyz2abc;
            SA->CAtom().GetEllipsoid()->MultMatrix(NM);
          }
        }
      }
    }
    delete m;
    Uniq();
  }
  else
    TBasicApp::GetLog().Info("Could not find closest matrix");
}
//..............................................................................
void TLattice::MoveFragmentG(const TVPointD& to, TSAtom& fragAtom)  {
  TVPointD from;
  from = fragAtom.CCenter();
  TMatrixD* m = GetUnitCell().GetClosest(to, from, true);
  TVPointD offset;
  if( m != NULL )  {
/* restore atom centres if were changed by some other procedure */
    RestoreCoordinates();
    Generated = true;
    TBasicApp::GetInstance()->ActionQueue("STRGEN")->Enter(this);
    Matrices.Add(m);
    for(int i=0; i < fragAtom.GetNetwork().NodeCount(); i++ )  {
      TSAtom& SA = fragAtom.GetNetwork().Node(i);
      if( SA.IsDeleted() )  continue;
      TSAtom* atom = new TSAtom( &SA.GetNetwork() );
      atom->CAtom( SA.CAtom() );
      atom->AddMatrix(m);
      atom->CCenter() = SA.CCenter();
      atom->CCenter() = (*m) * atom->CCenter();
      atom->CCenter()[0] += m->Data(0)[3];
      atom->CCenter()[1] += m->Data(1)[3];
      atom->CCenter()[2] += m->Data(2)[3];
      GetAsymmUnit().CellToCartesian(atom->CCenter(), atom->Center());
      atom->SetEllipsoid( &GetUnitCell().GetEllipsoid(m->GetTag(), atom->CAtom().GetId()) );
      AddSAtom(atom);
    }

    Disassemble();

    TBasicApp::GetInstance()->ActionQueue("STRGEN")->Exit(this);
  }
  else
  {
    TBasicApp::GetLog().Info("Could not find closest matrix");
  }
}
//..............................................................................
void TLattice::MoveFragmentG(TSAtom& to, TSAtom& fragAtom)
{
  TMatrixD* m = GetUnitCell().GetClosest(to.CCenter(), fragAtom.CCenter(), true);
  TVPointD offset;
  if( m != NULL )  {
/* restore atom centres if were changed by some other procedure */
    RestoreCoordinates();
    Generated = true;
    TBasicApp::GetInstance()->ActionQueue("STRGEN")->Enter(this);
    Matrices.Add(m);
    TSAtomPList atoms;
    if( to.CAtom().GetFragmentId() == fragAtom.CAtom().GetFragmentId() )
      atoms.Add(&fragAtom);
    else  // move whole fragment then
      for(int i=0; i < fragAtom.GetNetwork().NodeCount(); i++ )
        atoms.Add(&fragAtom.GetNetwork().Node(i));

    for(int i=0; i < atoms.Count(); i++ )  {
      TSAtom* SA = atoms.Item(i);
      if( SA->IsDeleted() )  continue;
      TSAtom* atom = new TSAtom(&SA->GetNetwork());
      atom->CAtom(SA->CAtom());
      atom->AddMatrix(m);
      atom->CCenter() = SA->CCenter();
      atom->CCenter() = (*m) * atom->CCenter();
      atom->CCenter()[0] += m->Data(0)[3];
      atom->CCenter()[1] += m->Data(1)[3];
      atom->CCenter()[2] += m->Data(2)[3];
      GetAsymmUnit().CellToCartesian(atom->CCenter(), atom->Center());
      atom->SetEllipsoid( &GetUnitCell().GetEllipsoid(m->GetTag(), atom->CAtom().GetId()) );
      AddSAtom(atom);
    }

    Disassemble();

    TBasicApp::GetInstance()->ActionQueue("STRGEN")->Exit(this);
  }
  else
  {
    TBasicApp::GetLog().Info("Could not find closest matrix");
  }
}
//..............................................................................
void TLattice::MoveToCenter()  {
  if( Generated )  {
    TBasicApp::GetLog().Error("Please note that asymetric unit will not be updated: the structure is grown");
    TBasicApp::GetInstance()->ActionQueue("STRGEN")->Enter(this);
  }

  TMatrixD abc2xyz( GetAsymmUnit().GetCellToCartesian()),
           xyz2abc( GetAsymmUnit().GetCartesianToCell()), NM(3,3);
  abc2xyz.Transpose();
  xyz2abc.Transpose();

  TVPointD molCenter;
  TVPointD cnt, err;
  TMatrixD* m;
  for( int i=0; i < Fragments.Count(); i++ )  {
    TNetwork* frag = Fragments[i];
    molCenter.Null();
     for( int j=0; j < frag->NodeCount(); j++ )
       molCenter += frag->Node(j).CCenter();
    if( frag->NodeCount() )  molCenter /= frag->NodeCount();
    m = GetUnitCell().GetClosest(TVPointD(0.5, 0.5, 0.5), molCenter, true);
    if( m != NULL )  {
      if( Generated )  {
        for(int j=0; j < frag->NodeCount(); j++ )  {
          TSAtom& SA = frag->Node(j);
          SA.CCenter() = *m * SA.CCenter();
          SA.CCenter()[0] += m->Data(0)[3];  SA.CCenter()[1] += m->Data(1)[3];  SA.CCenter()[2] += m->Data(2)[3];
          SA.SetEllipsoid( &GetUnitCell().GetEllipsoid(m->GetTag(), SA.CAtom().GetId()) );
        }
      }
      else  {
        for(int j=0; j < frag->NodeCount(); j++ )  {
          TSAtom& SA = frag->Node(j);
          cnt = SA.CAtom().CCenter();
          err[0] = SA.CAtom().CCenter()[0].GetE();
          err[1] = SA.CAtom().CCenter()[1].GetE();
          err[2] = SA.CAtom().CCenter()[2].GetE();
          cnt = *m * cnt;
          err = *m * err;
          cnt[0] += m->Data(0)[3];  cnt[1] += m->Data(1)[3];  cnt[2] += m->Data(2)[3];
          SA.CAtom().CCenter() = cnt;
          SA.CAtom().CCenter().Value(0).E() = fabs(err[0]);
          SA.CAtom().CCenter().Value(1).E() = fabs(err[1]);
          SA.CAtom().CCenter().Value(2).E() = fabs(err[2]);
          if( SA.CAtom().GetEllipsoid() != NULL )  {
            for( int mk=0; mk < 3; mk++ )
              for( int ml=0; ml < 3; ml++ )
                NM[mk][ml] = m->Data(mk)[ml];
            NM = abc2xyz*NM*xyz2abc;
            SA.CAtom().GetEllipsoid()->MultMatrix(NM);
          }
        }
      }
      delete m;
    }
  }
  if( !Generated )  Uniq();
  else  {
    RestoreCoordinates();
    Disassemble();
    TBasicApp::GetInstance()->ActionQueue("STRGEN")->Exit(this);
  }
}
//..............................................................................
void TLattice::Compaq()  {
  if( Fragments.Count() < 2 )  return;
  if( Generated )  {
    TBasicApp::GetLog().Error("Please note that asymetric unit will not be updated: the structure is grown");
    RestoreCoordinates();
    TBasicApp::GetInstance()->ActionQueue("STRGEN")->Enter(this);
  }
  TVPointD molCenter, acenter;
  TVPointD offset;
  TMatrixD* m;

  TMatrixD abc2xyz( GetAsymmUnit().GetCellToCartesian()),
           xyz2abc( GetAsymmUnit().GetCartesianToCell()), NM(3,3);
  abc2xyz.Transpose();
  xyz2abc.Transpose();

  molCenter[0] = 0;  molCenter[1] = 0;  molCenter[2] = 0;
  TNetwork* frag = Fragments[0];
  for( int j=0; j < frag->NodeCount(); j++ )
    acenter += frag->Node(j).CCenter();
  acenter /= frag->NodeCount();
  for( int i=1; i < Fragments.Count(); i++ )  {
    frag = Fragments[i];
    m = NULL;

    for(int j=0; j < Fragments[0]->NodeCount(); j++ )  {
      TSAtom& fa = Fragments[0]->Node(j);
      for( int k=0; k < frag->NodeCount(); k++ )  {
        if( frag->Node(k).CAtom().IsAttachedTo( fa.CAtom() ) )  {
          m = GetUnitCell().GetClosest(fa.CCenter(), frag->Node(k).CCenter(), true);
        }
      }
    }
    if( m == NULL )  {
      molCenter[0] = 0;  molCenter[1] = 0;  molCenter[2] = 0;
      for( int j=0; j < frag->NodeCount(); j++ )
        molCenter += frag->Node(j).CCenter();
      if( frag->NodeCount() )  molCenter /= frag->NodeCount();
      m = GetUnitCell().GetClosest(acenter, molCenter, true);
    }
    if( m != NULL )  {
      if( !Generated )  {
        for(int j=0; j < frag->NodeCount(); j++ )  {
          TSAtom& SA = frag->Node(j);
          if( SA.IsDeleted() )  continue;
          SA.CAtom().CCenter() *= *m;
          offset[0] = m->Data(0)[3];
          offset[1] = m->Data(1)[3];
          offset[2] = m->Data(2)[3];
          SA.CAtom().CCenter() += offset;
          if( SA.CAtom().GetEllipsoid() != NULL )  {
            for( int mk=0; mk < 3; mk++ )
              for( int ml=0; ml < 3; ml++ )
                NM[mk][ml] = m->Data(mk)[ml];
            NM = abc2xyz*NM*xyz2abc;
            SA.CAtom().GetEllipsoid()->MultMatrix(NM);
          }
        }
        delete m;
      }
      else  {
        for(int j=0; j < frag->NodeCount(); j++ )  {
          TSAtom& SA = frag->Node(j);
          if( SA.IsDeleted() )  continue;
          SA.CCenter() = (*m) * SA.CCenter();
          SA.CCenter()[0] += m->Data(0)[3];
          SA.CCenter()[1] += m->Data(1)[3];
          SA.CCenter()[2] += m->Data(2)[3];
          SA.AddMatrix(m);
          GetAsymmUnit().CellToCartesian(SA.CCenter(), SA.Center());
        }
        Matrices.Add(m);
      }
    }
  }
  if( !Generated )  Uniq();
  else  {
    Disassemble();
    TBasicApp::GetInstance()->ActionQueue("STRGEN")->Exit(this);
  }
}
//..............................................................................
void TLattice::CompaqAll()  {
  if( Generated )  return;

  if( Fragments.Count() < 2 )  return;

  TMatrixD abc2xyz( GetAsymmUnit().GetCellToCartesian()),
           xyz2abc( GetAsymmUnit().GetCartesianToCell()), NM(3,3);
  abc2xyz.Transpose();
  xyz2abc.Transpose();

  TVPointD offset;
  TMatrixD* m;

  for( int i=0; i < Fragments.Count(); i++ )  {
    for( int j=i+1; j < Fragments.Count(); j++ )  {
      m = NULL;

      for(int k=0; k < Fragments[i]->NodeCount(); k++ )  {
        TSAtom& fa = Fragments[i]->Node(k);
        for( int l=0; l < Fragments[j]->NodeCount(); l++ )  {
          if( Fragments[j]->Node(l).CAtom().IsAttachedTo( fa.CAtom() ) )  {
            m = GetUnitCell().GetClosest(fa.CCenter(), Fragments[j]->Node(l).CCenter(), true);
          }
        }
      }

      if( m == NULL )  continue;
      for(int k=0; k < Fragments[j]->NodeCount(); k++ )  {
        TSAtom& SA = Fragments[j]->Node(k);
        if( SA.IsDeleted() )  continue;
        SA.CAtom().CCenter() *= *m;
        offset[0] = m->Data(0)[3];
        offset[1] = m->Data(1)[3];
        offset[2] = m->Data(2)[3];
        SA.CAtom().CCenter() += offset;
        if( SA.CAtom().GetEllipsoid() != NULL )  {
          for( int mk=0; mk < 3; mk++ )
            for( int ml=0; ml < 3; ml++ )
              NM[mk][ml] = m->Data(mk)[ml];
          NM = abc2xyz*NM*xyz2abc;
          SA.CAtom().GetEllipsoid()->MultMatrix(NM);
        }
      }
      delete m;
    }
  }
  Uniq();
}
//..............................................................................
void TLattice::TransformFragments(const TSAtomPList& fragAtoms, const TMatrixD& transform)  {
  if( Generated )  {
    TBasicApp::GetLog().Error("Cannot perform this operation on grown structure");
    return;
  }

  TMatrixD abc2xyz( GetAsymmUnit().GetCellToCartesian()),
           xyz2abc( GetAsymmUnit().GetCartesianToCell()), NM(3,3);
  abc2xyz.Transpose();
  xyz2abc.Transpose();

  for(int i=0; i < fragAtoms.Count(); i++ )
    fragAtoms[i]->GetNetwork().SetTag(i);

  for(int i=0; i < fragAtoms.Count(); i++ )  {
    if( fragAtoms[i]->GetNetwork().GetTag() == i )  {
      for(int j=0; j < fragAtoms[i]->GetNetwork().NodeCount(); j++ )  {
        TSAtom& SA = fragAtoms[i]->GetNetwork().Node(j);
        SA.CAtom().CCenter() *= transform;
        if( transform.Elements() == 4 )  {
          SA.CAtom().CCenter().Value(0) += transform[0][3];
          SA.CAtom().CCenter().Value(1) += transform[1][3];
          SA.CAtom().CCenter().Value(2) += transform[2][3];
        }
        if( SA.CAtom().GetEllipsoid() != NULL )  {
          for( int mk=0; mk < 3; mk++ )
            for( int ml=0; ml < 3; ml++ )
              NM[mk][ml] = transform[mk][ml];
          NM = abc2xyz*NM*xyz2abc;
          SA.CAtom().GetEllipsoid()->MultMatrix(NM);
        }
      }
    }
  }
  Uniq();
}
//..............................................................................
void TLattice::Disassemble()  {
  // clear bonds & fargments
  ClearBonds();
  ClearFragments();

  // find bonds & fragments
  Network->Disassemble(Atoms, Fragments, &Bonds);
  // restore latId, as some atoms might been removed ny the network
  for( int i=0; i < Atoms.Count(); i++ )
    Atoms[i]->SetLatId(i);

  // precalculate memory usage
  int bondCnt = Bonds.Count();
  for(int i=0; i < Fragments.Count(); i++ )  {
    for( int j=0; j < Fragments[i]->BondCount(); j++ )
      bondCnt ++;
  }
  Bonds.SetCapacity( bondCnt );
  // end

  for( int i=0; i < Fragments.Count(); i++ )  {
    TNetwork* Frag = Fragments[i];
    for( int j=0; j < Frag->BondCount(); j++ )  {
      AddSBond( &Frag->Bond(j) );
    }
    for( int j=0; j < Frag->NodeCount(); j++ )
      Frag->Node(j).CAtom().SetFragmentId(i);
  }
}
//..............................................................................
void TLattice::RestoreCoordinates()  {
  for( int i=0; i < Atoms.Count(); i++ )  {
    Atoms[i]->Center() = Atoms[i]->CCenter();
    GetAsymmUnit().CellToCartesian(Atoms[i]->CCenter(), Atoms[i]->Center());
  }
}
//..............................................................................
bool TLattice::_AnalyseAtomHAdd(AConstraintGenerator& cg, TSAtom& atom, TSAtomPList& ProcessingAtoms)  {

  if( ProcessingAtoms.IndexOf(&atom) != -1 || atom.CAtom().IsHAttached() )
    return false;
  ProcessingAtoms.Add( &atom );

  TBasicAtomInfo& HAI = AtomsInfo->GetAtomInfo(iHydrogenIndex);
  TAtomEnvi AE;
  UnitCell->GetAtomEnviList(atom, AE);
  if( atom.GetAtomInfo() == iCarbonIndex )  {
    if( AE.Count() == 1 )  {
      // check acetilene
      double d = AE.GetCrd(0).DistanceTo( atom.Center() );
      TSAtom* A = FindSAtom( AE.GetLabel(0) );
      TAtomEnvi NAE;
      if( A == 0 )
        throw TFunctionFailedException(__OlxSourceInfo, olxstr("Could not locate atom ") << AE.GetLabel(0) );

      UnitCell->GetAtomEnviList(*A, NAE);
      if( A->GetAtomInfo() == iCarbonIndex && NAE.Count() == 2 && d < 1.2)  {
        TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": XCH" );
        cg.FixAtom( AE, fgCH1, HAI);
      }
      else  {
        if( d > 1.35 )  {
          TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": XCH3" );
          cg.FixAtom( AE, fgCH3, HAI);
        }
        else  {
          if( d < 1.25 )  {
            if( NAE.Count() > 1 ) {
              TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": X=CH2" );
              cg.FixAtom( AE, fgCH2, HAI);
            }
            else
              TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": possibly X=CH2" );
          }
        }
      }
    }
    else  if( AE.Count() == 2 )  {
      TVPointD a = AE.GetCrd(0);
        a -= atom.Center();
      TVPointD b = AE.GetCrd(1);
        b -= atom.Center();
      double v = a.CAngle(b);
      v = acos(v)*180/M_PI;
      double d1 = AE.GetCrd(0).DistanceTo( atom.Center() );
      double d2 = AE.GetCrd(1).DistanceTo( atom.Center() );
      if(  d1 > 1.4 && d2 > 1.4 && v < 120 )  {
        TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": XYCH2" );
        cg.FixAtom( AE, fgCH2, HAI);
      }
      else  {
        if( (d1 < 1.4 || d2 < 1.4) && v < 160 )  {
          TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": X(Y=C)H" );
          cg.FixAtom( AE, fgCH1, HAI);
        }
      }
    }
    else if( AE.Count() == 3 )  {
      double v = TetrahedronVolume<double>( atom.Center(), AE.GetCrd(0), AE.GetCrd(1), AE.GetCrd(2) );
      if( v > 0.5 )  {
        TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": XYZCH" );
        cg.FixAtom( AE, fgCH1, HAI);
      }
    }
    else if( AE.Count() == 5 )  {  // carboranes ...
      //check
      bool proceed = false;
      for( int j=0; j < AE.Count(); j++ )
        if( AE.GetBAI(j) == iBoronIndex )  {
          proceed = true;  break;
        }
      if( proceed )  {
        TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": R5CH" );
        cg.FixAtom( AE, fgBH1, HAI);
      }
    }
  }
  else if( atom.GetAtomInfo() == iNitrogenIndex )  {  // nitrogen
    if( AE.Count() == 1 )  {
      double d = AE.GetCrd(0).DistanceTo( atom.Center() );
      if( d > 1.35 )  {
        TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": XNH2" );
        cg.FixAtom( AE, fgNH2, HAI);
      }
      else  {
        if( d > 1.2 )  {  //else nitrile
          // have to check if double bond
          TSAtom* A = FindSAtom( AE.GetLabel(0) );
          TAtomEnvi NAE;
          if( A == 0 )
            throw TFunctionFailedException(__OlxSourceInfo, olxstr("Could not locate atom ") << AE.GetLabel(0) );

          UnitCell->GetAtomEnviList(*A, NAE);
          NAE.Exclude( atom.CAtom() );

          if( A->GetAtomInfo() == iCarbonIndex && NAE.Count() > 1 )  {
            TVPointD a = NAE.GetCrd(0);
              a -= NAE.GetBase().Center();
            TVPointD b = AE.GetBase().Center();
              b -= NAE.GetBase().Center();

            d = a.CAngle(b);
            d = acos(d)*180/M_PI;
            if( d > 115 && d < 130 )  {
              TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": X=NH2" );
              cg.FixAtom( AE, fgNH2, HAI, &NAE);
            }
          }
          else  {
            TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": X=NH" );
            cg.FixAtom( AE, fgNH1, HAI);
          }
        }
      }
    }
    else  if( AE.Count() == 2 )  {
      TVPointD a = AE.GetCrd(0);
        a -= atom.Center();
      TVPointD b = AE.GetCrd(1);
        b -= atom.Center();
      double v = a.CAngle(b);
      v = acos(v)*180/M_PI;
      double d1 = AE.GetCrd(0).DistanceTo( atom.Center() );
      double d2 = AE.GetCrd(1).DistanceTo( atom.Center() );
      if( d1 > 1.65 || d2 > 1.65 )  {
        ;
      }
      else if( v < 120 && d1 > 1.45 && d2 > 1.45 )  {
        TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": R2NH2+" );
        cg.FixAtom( AE, fgNH2, HAI);
      }
      else if( v < 120 || d1 < 1.3 || d2 < 1.3 )
        ;
      else  {
        TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": XYNH" );
        cg.FixAtom( AE, fgNH1, HAI);
      }
    }
    else  if( AE.Count() == 3 )  {
    // remove ccordination bond ...
      TVPointD a = AE.GetCrd(0);
        a -= atom.Center();
      TVPointD b = AE.GetCrd(1);
        b -= atom.Center();
      TVPointD c = AE.GetCrd(2);
        c -= atom.Center();
      double v1 = a.CAngle(b);  v1 = acos(v1)*180/M_PI;
      double v2 = a.CAngle(c);  v2 = acos(v2)*180/M_PI;
      double v3 = b.CAngle(c);  v3 = acos(v3)*180/M_PI;
      double d1 = AE.GetCrd(0).DistanceTo( atom.Center() );
      double d2 = AE.GetCrd(1).DistanceTo( atom.Center() );
      double d3 = AE.GetCrd(2).DistanceTo( atom.Center() );
      if( (v1+v2+v3) < 350 && d1 > 1.45 && d2 > 1.45 && d3 > 1.45 )  {
        if( d1 > 1.75 || d2 > 1.75 || d3 > 1.75 )  {
          TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": R2HN->M" );
          cg.FixAtom( AE, fgNH1, HAI);
        }
        else  {
          TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": R3NH+" );
          cg.FixAtom( AE, fgNH1, HAI);
        }
      }
    }
  }
  if( atom.GetAtomInfo() == iOxygenIndex )  {  // oxygen
    if( AE.IsEmpty() )  {
      TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": OH2" );
      TAtomEnvi pivoting;
      UnitCell->GetAtomPossibleHBonds(AE, pivoting);
      RemoveNonHBonding( pivoting );
      if( pivoting.Count() > 0 )
        if( !_AnalyseAtomHAdd( cg, *FindSAtom(pivoting.GetLabel(0)), ProcessingAtoms) )
          pivoting.Clear();
      cg.FixAtom( AE, fgOH2, HAI, &pivoting);
    }
    else if( AE.Count() == 1 )  {
      double d = AE.GetCrd(0).DistanceTo( atom.Center() );
      if( d > 1.3 )   {  // otherwise a doubl bond
        if( AE.GetBAI(0) == iChlorineIndex )
          ;
        else  if( AE.GetBAI(0) == iCarbonIndex )  {  // carbon
          TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": COH" );
          TAtomEnvi pivoting;
          UnitCell->GetAtomPossibleHBonds(AE, pivoting);
          RemoveNonHBonding( pivoting );
          if( pivoting.Count() > 0 )
            if( !_AnalyseAtomHAdd( cg, *FindSAtom(pivoting.GetLabel(0)), ProcessingAtoms) )
              pivoting.Clear();
          cg.FixAtom( AE, fgOH1, HAI, &pivoting);
        }
        else  if( AE.GetBAI(0) == iSulphurIndex )  {
          if( d > 1.48 )  {
            TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": SOH" );
            cg.FixAtom( AE, fgOH1, HAI);
          }
        }
        else  if( AE.GetBAI(0) == iPhosphorusIndex )  {
          if( d > 1.54 )  {
            TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": POH" );
            TAtomEnvi pivoting;
            UnitCell->GetAtomPossibleHBonds(AE, pivoting);
            RemoveNonHBonding( pivoting );
            if( pivoting.Count() > 0 )
              if( !_AnalyseAtomHAdd( cg, *FindSAtom(pivoting.GetLabel(0)), ProcessingAtoms) )
                pivoting.Clear();
            cg.FixAtom( AE, fgOH1, HAI, &pivoting);
          }
        }
        else  if( AE.GetBAI(0) == iSiliconIndex )  {
          if( d > 1.6 )  {
            TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": SiOH" );
            TAtomEnvi pivoting;
            UnitCell->GetAtomPossibleHBonds(AE, pivoting);
            RemoveNonHBonding( pivoting );
            if( pivoting.Count() > 0 )
              if( !_AnalyseAtomHAdd( cg, *FindSAtom(pivoting.GetLabel(0)), ProcessingAtoms) )
                pivoting.Clear();
            cg.FixAtom( AE, fgOH1, HAI, &pivoting);
          }
        }
        else  if( AE.GetBAI(0) == iBoronIndex )  {
          if( d < 1.38 )  {
            TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": B(III)OH" );
            TAtomEnvi pivoting;
            UnitCell->GetAtomPossibleHBonds(AE, pivoting);
            RemoveNonHBonding( pivoting );
            if( pivoting.Count() > 0 )
              if( !_AnalyseAtomHAdd( cg, *FindSAtom(pivoting.GetLabel(0)), ProcessingAtoms) )
                pivoting.Clear();
            cg.FixAtom( AE, fgOH1, HAI, &pivoting);
          }
        }
        else if( d > 1.8 )  {  // coordination bond?
          TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": possibly M-OH2" );
          cg.FixAtom( AE, fgOH2, HAI);
        }
      }
    }
  }
  else if( atom.GetAtomInfo() == iBoronIndex )  {  // boron
    if( AE.Count() == 4 )  {
      TVPointD a, b;
      double sumAng = 0;
      for( int i=0; i < AE.Count(); i++ )  {
        a = AE.GetCrd(i);
        a -= atom.Center();
        for( int j=i+1; j < AE.Count(); j++ )  {
          b = AE.GetCrd(j);
          b -= atom.Center();
          double ca = b.CAngle(a);
          if( ca < -1 )  ca = -1;
          if( ca > 1 )   ca = 1;
          sumAng += acos(ca);
        }
      }
      if( sumAng*180/M_PI > 700 )  {   //!! not sure it works, lol
        TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": X4BH" );
        cg.FixAtom( AE, fgBH1, HAI);
      }
    }
    else if( AE.Count() == 5 )  {
      TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": X5BH" );
      cg.FixAtom( AE, fgBH1, HAI);
    }
  }
  ProcessingAtoms.Delete( ProcessingAtoms.IndexOf(&atom) );
  return true;
}
//..............................................................................
void TLattice::AnalyseHAdd(AConstraintGenerator& cg, const TSAtomPList& atoms)  {

  TPtrList<TBasicAtomInfo> CTypes;
  CTypes.Add( &AtomsInfo->GetAtomInfo(iCarbonIndex) );
  CTypes.Add( &AtomsInfo->GetAtomInfo(iNitrogenIndex) );
  CTypes.Add( &AtomsInfo->GetAtomInfo(iOxygenIndex) );
  CTypes.Add( &AtomsInfo->GetAtomInfo(iBoronIndex) );
  TSAtomPList ProcessingAtoms;

  for( int i=0; i < atoms.Count(); i++ )
    atoms[i]->CAtom().SetHAttached(false);

  TBasicAtomInfo& HAI = AtomsInfo->GetAtomInfo(iHydrogenIndex);

  // trean C6 benzene rings, NC5 pyridine
  TTypeList< TSAtomPList > rings;
  TPtrList<TBasicAtomInfo> rcont;
  rcont.Add( &AtomsInfo->GetAtomInfo(iCarbonIndex) );
  for( int i=0; i < 4; i++ )  rcont.Add( rcont[0] );
  rcont.Add( NULL );  // any atom

  for( int i=0; i < FragmentCount(); i++ )
    GetFragment(i).FindRings(rcont, rings);
  TAtomEnvi AE;
  for(int i=0; i < rings.Count(); i++ )  {
    double rms = TSPlane::CalcRMS( rings[i] );
    if( rms < 0.05 && TNetwork::IsRingRegular( rings[i]) )  {
      for( int j=0; j < rings[i].Count(); j++ )  {
        AE.Clear();
        UnitCell->GetAtomEnviList(*rings[i][j], AE);
        if( AE.Count() == 2 && rings[i][j]->GetAtomInfo() == iCarbonIndex)  {
          TBasicApp::GetLog().Info( olxstr(rings[i][j]->GetLabel()) << ": X(Y=C)H" );
          cg.FixAtom( AE, fgCH1, HAI);
          rings[i][j]->CAtom().SetHAttached(true);
        }
      }
    }
  }

  for( int i=0; i < atoms.Count(); i++ )  {
    if( atoms[i]->IsDeleted() )  continue;

    bool consider = false;
    for( int j=0; j < CTypes.Count(); j++ )  {
      if( atoms[i]->GetAtomInfo() == *CTypes[j] )  {
        consider = true;
        break;
      }
    }
    if( !consider )  continue;
    for( int j=0; j < atoms[i]->NodeCount(); j++ )  {
      TSAtom& A = atoms[i]->Node(j);
      if( A.IsDeleted() )  continue;
      if( A.GetAtomInfo() == iHydrogenIndex ) {
        consider = false;
        break;
      }
    }
    for(int j=0; j < atoms[i]->CAtom().AttachedAtomCount(); j++ )  {
      if( atoms[i]->CAtom().GetAttachedAtom(j).GetAtomInfo() == iHydrogenIndex )  {
        consider = false;
        break;
      }
    }
    if( !consider )  continue;
    _AnalyseAtomHAdd(cg, *atoms[i], ProcessingAtoms);
  }
}
//..............................................................................
void TLattice::RemoveNonHBonding(TAtomEnvi& Envi)  {
  TAtomEnvi AE;
  for( int i=0; i < Envi.Count(); i++ )  {
    TSAtom* SA = FindSAtom( Envi.GetLabel(i) );
    AE.Clear();
    UnitCell->GetAtomEnviList(*SA, AE);
    if( SA->GetAtomInfo() == AtomsInfo->GetAtomInfo(iOxygenIndex) )  {
    /* this case needs an investigation, but currently the same atom cannot be a pivoting one ...*/
      if( SA->CAtom() == Envi.GetBase().CAtom() )  {
        Envi.Exclude( SA->CAtom() );
        continue;
      }
      if( AE.IsEmpty() )  {
        if( SA->CAtom() != Envi.GetBase().CAtom() )  // it is a symmetrical equivalent
          Envi.Exclude( SA->CAtom() );
      }
      else if( AE.Count() == 1 )  {
        double d = AE.GetCrd(0).DistanceTo( SA->Center() );
        if( d > 1.8 )  // coordination bond?
          Envi.Exclude( SA->CAtom() );
      }
      else if( AE.Count() == 2 )  {  // not much can be done here ... needs thinking
        //Envi.Exclude( SA->CAtom() );
        // commented 17.03.08, just trying to what the shortest distance will give
      }
      else if( AE.Count() == 3 )  {  // coordinated water molecule
        Envi.Exclude( SA->CAtom() );
      }
    }
    else if( SA->GetAtomInfo() == AtomsInfo->GetAtomInfo(iNitrogenIndex) )  {
      if( AE.Count() > 3 )
          Envi.Exclude( SA->CAtom() );
    }
  }
  // choose the shortest bond ...
  if( Envi.Count() > 1 )  {
    TPSTypeList<double, TCAtom*> hits;

    for( int i=0; i < Envi.Count(); i++ )
      hits.Add( Envi.GetBase().Center().DistanceTo( Envi.GetCrd(i) ), &Envi.GetCAtom(i) );

    while( hits.Count() > 1 &&
      ((hits.GetComparable(hits.Count()-1) - hits.GetComparable(0)) > 0.1) )  {
      Envi.Exclude( *hits.GetObject(hits.Count()-1) );
      hits.Remove(hits.Count()-1);
    }
  }
  // all similar length  .... Q peaks might help :)
  if( Envi.Count() > 1 )  {
    TPSTypeList<double, TCAtom*> hits;
    TVectorD vec1, vec2;
    AE.Clear();
    UnitCell->GetAtomQEnviList( Envi.GetBase(), AE );
    for( int i=0; i < AE.Count(); i++ )  {
//      v1 = AE.GetCrd(i);
//      v1 -= Envi.GetBase()->Center();
      double d = Envi.GetBase().Center().DistanceTo( AE.GetCrd(i) );

      if( d < 0.7 || d > 1.3 )  {
        AE.Exclude( AE.GetCAtom(i) );
        i--;
      }
    }
    if( AE.IsEmpty() || AE.Count() > 1 )  return;
    vec1 = AE.GetCrd(0);
    vec1 -= Envi.GetBase().Center();
    for( int i=0; i < Envi.Count(); i++ )  {
      vec2 = Envi.GetCrd(i);
      vec2 -= Envi.GetBase().Center();
      hits.Add( fabs(-1 + vec2.CAngle(vec1)), &Envi.GetCAtom(i) );
    }
    while( hits.Count() > 1 )  {
      Envi.Exclude( *hits.GetObject( hits.Count() - 1 ) );
      hits.Remove( hits.Count() - 1 );
    }
  }

}
//..............................................................................
void TLattice::SetAnis( const TCAtomPList& atoms, bool anis )  {
  if( !anis )  {
    for( int i=0; i < atoms.Count(); i++ )  {
      if( atoms[i]->GetEllipsoid() != NULL )  {
         GetAsymmUnit().NullEllp( atoms[i]->GetEllipsoid()->GetId() );
         atoms[i]->AssignEllps( NULL );
      }
    }
    GetAsymmUnit().PackEllp();
  }
  else  {
    TVectorD ee(6);
    ee[0] = ee[1] = ee[2] = 0.025;
    for( int i=0; i < atoms.Count(); i++ )  {
      if( atoms[i]->GetEllipsoid() == NULL )
         atoms[i]->UpdateEllp( ee );
    }
  }
  TBasicApp::GetInstance()->ActionQueue("STRUNIQ")->Enter(this);
  GetUnitCell().ClearEllipsoids();
  Init();
  TBasicApp::GetInstance()->ActionQueue("STRUNIQ")->Exit(this);
}
//..............................................................................
//..............................................................................
//..............................................................................
void TLattice::LibGetFragmentCount(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal( olxstr(FragmentCount()) );
}
void TLattice::LibGetFragmentAtoms(const TStrObjList& Params, TMacroError& E)  {
  int index = Params[0].ToInt();
  if( index < 0 || index >= FragmentCount() )
    throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, FragmentCount());
  olxstr rv;
  for( int i=0; i < Fragments[index]->NodeCount(); i++ )  {
    rv << Fragments[index]->Node(i).GetLabel();
    if( (i+1) < Fragments[index]->NodeCount() )
      rv << ',';
  }
  E.SetRetVal( rv );
}
//..............................................................................


class TLibrary*  TLattice::ExportLibrary(const olxstr& name)  {
  TLibrary* lib = new TLibrary( name.IsEmpty() ? olxstr("latt") : name);
  lib->RegisterFunction<TLattice>( new TFunction<TLattice>(this,  &TLattice::LibGetFragmentCount, "GetFragmentCount", fpNone,
"Returns number of fragments in the lattice") );
  lib->RegisterFunction<TLattice>( new TFunction<TLattice>(this,  &TLattice::LibGetFragmentAtoms, "GetFragmentAtoms", fpOne,
"Returns a comma separated list of atoms in specified fragment") );
  return lib;
}

