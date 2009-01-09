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
#include "symmparser.h"

#undef GetObject
// sorts largest -> smallest
int TLattice_SortFragments(const TNetwork* n1, const TNetwork* n2)  {
  return n2->NodeCount() - n1->NodeCount();
}
//---------------------------------------------------------------------------
// TLattice function bodies
//---------------------------------------------------------------------------
TLattice::TLattice() : AtomsInfo(TAtomsInfo::GetInstance()) {
  Generated = false;

  AsymmUnit   = new TAsymmUnit(this);
  UnitCell    = new TUnitCell(this);
  Network     = new TNetwork(this, NULL);
  Delta    = 0.5f;
  DeltaI    = 1.2f;
  OnStructureGrow = &Actions.NewQueue("STRGEN");
  OnStructureUniq = &Actions.NewQueue("STRUNIQ");
  OnDisassemble= &Actions.NewQueue("DISASSEBLE");
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
int TLattice::GenerateMatrices(const vec3d& VFrom, const vec3d& VTo,
  const vec3d& MFrom, const vec3d& MTo)  {
  GenerateMatrices(Matrices, VFrom, VTo, MFrom, MTo);
  return MatrixCount();
}
//..............................................................................
int TLattice::GenerateMatrices(smatd_plist& Result,
     const vec3d& VFrom, const vec3d& VTo,
     const vec3d& MFrom, const vec3d& MTo)  {
  olxstr Tmp;
  smatd *M, *M1;
  int mstart = Result.Count();
  vec3d Center, C;
  Center = GetAsymmUnit().GetOCenter(true, false);

  Result.SetCapacity( (int)(GetUnitCell().MatrixCount()*
                              (fabs(VTo[0]-VFrom[0])+1)*
                              (fabs(VTo[1]-VFrom[1])+1)*
                              (fabs(VTo[2]-VFrom[2])+1)) );

  for( int i=0; i < GetUnitCell().MatrixCount(); i++ )  {
    for( int j=(int)VFrom[0]; j <= (int)VTo[0]; j++ )
      for( int k=(int)VFrom[1]; k <= (int)VTo[1]; k++ )
      for( int l=(int)VFrom[2]; l <= (int)VTo[2]; l++ )  {
        M = new smatd(GetUnitCell().GetMatrix(i));
        M->SetTag(i);  // set Tag to identify the matrix (and ellipsoids) in the UnitCell
        M->t[0] += j;
        M->t[1] += k;
        M->t[2] += l;
        Result.Add(M);
      }
  }

// Analysis of matrixes: check if the center of gravity of the asymmetric unit
// is inside the generation volume
  for( int i=mstart; i < Result.Count(); i++ )  {
    M = Result[i];
    C = *M * Center;
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
        if( M->t == M1->t )  {
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
  smatd *M = new smatd; // create identity matrix
  *M = GetUnitCell().GetMatrix(0);
  M->SetTag(0);
  Matrices.Add( M );
  ListAsymmUnit(Atoms, NULL, true);

  OnDisassemble->Enter(this);

  if( AtomMask.Count() == Atoms.Count() )  {
    const int ac = Atoms.Count();
    for( int i=0; i < ac; i++ )
      Atoms[i]->SetStandalone(!AtomMask[i]);
    AtomMask.Clear();
  }

  Network->Disassemble(Atoms, Fragments, &Bonds);
  Fragments.QuickSorter.SortSF(Fragments, TLattice_SortFragments);
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
      //if( CAtom.GetAfix() )  {
      //  if( SAtom.NodeCount() == 1 )
      //    CAtom.SetAfixAtomId( SAtom.Node(0).CAtom().GetLoaderId() );
      //  else  {
      //    conIndex = 0;
      //    for( int k=0; k < SAtom.NodeCount(); k++ )  {
      //      if( SAtom.Node(k).GetAtomInfo() != iQPeakIndex )  {
      //        conIndex ++;
      //        CAtom.SetAfixAtomId( SAtom.Node(k).CAtom().GetLoaderId() );
      //      }
      //    }
      //    if( conIndex > 1 )  {
      //      //TBasicApp::GetLog()->CriticalInfo( (olxstr("Bad connectivity for: ") << SAtom->GetLabel()) <<
      //      //  ". Please fix the problem and reload file"
      //      //);
      //      CAtom.SetAfixAtomId(-1);
      //    }
      //  }
      //}
    }
  }
  OnDisassemble->Exit(this);
}
void TLattice::Init()  {
  Clear(false);
  GetUnitCell().ClearEllipsoids();
  GetUnitCell().InitMatrices();
  InitBody();
  TEStrBuffer tmp;
  int eqc = GetUnitCell().FindSymmEq(tmp, 0.1, true, false, false); // find and not remove
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
  OnStructureUniq->Enter(this);

//  UpdateAsymmUnit();  // need to cal it here, the call in ListAsymmUnit
  // will not be executed as Atoms->Count() = 0
  Clear(false);
  ClearMatrices();
  if( GetAsymmUnit().DoesContainEquivalents() && remEqv )  {
    TEStrBuffer Msg;
    GetUnitCell().FindSymmEq(Msg, 0.1, true, false, true);
    //TBasicApp::GetLog()->CriticalInfo(olxstr("Symmetrical counterparts were removed for: ") << Msg.ToString() );
    GetAsymmUnit().SetContainsEquivalents(false);
    Init();
  }
  else  {
    InitBody();
  }
  Generated = false;
  OnStructureUniq->Exit(this);
}
//..............................................................................
void TLattice::GenerateAtoms(const TSAtomPList& atoms, TSAtomPList& result, const smatd_plist& matrices)  {
  if( atoms.IsEmpty() )  return;

//  result->SetCount(mc*ac);
  for(int i=0; i < matrices.Count(); i++ )  {
    smatd* M = matrices[i];
    for(int j=0; j < atoms.Count(); j++ )  {
      if( atoms[j]->IsDeleted() )  continue;
      TSAtom* A = new TSAtom( Network );
      A->CAtom( atoms[j]->CAtom() );
      A->ccrd() = *M * A->ccrd();
      GetAsymmUnit().CellToCartesian( A->ccrd(), A->crd() );
      if( atoms[j]->CAtom().GetLoaderId() != -1 )   // for a centroid or whatsoever == -1
      A->SetEllipsoid( &GetUnitCell().GetEllipsoid(M->GetTag(), atoms[j]->CAtom().GetId()) );
      A->AddMatrix(M);
      result.Add(A);
    }
  }
}
//..............................................................................
void TLattice::GenerateWholeContent(TCAtomPList* Template)  {
  OnStructureGrow->Enter(this);
//  ClearAtoms();
//  ListAsymmUnit(*Atoms(), Template);
  Generated = false; // force the procedure
  Generate(Template, false, true);
  OnStructureGrow->Exit(this);
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
      A->crd() = A->ccrd();
      GetAsymmUnit().CellToCartesian(A->crd());
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
void TLattice::Generate(const vec3d& MFrom, const vec3d& MTo, TCAtomPList* Template,
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
  vec3d VFrom, VTo;

  VTo[0] = Round(MTo[0]+1);     VTo[1] = Round(MTo[1]+1);     VTo[2] = Round(MTo[2]+1);
  VFrom[0] = Round(MFrom[0]-1); VFrom[1] = Round(MFrom[1]-1); VFrom[2] = Round(MFrom[2]-1);

  GenerateMatrices(VFrom, VTo, MFrom, MTo);
  OnStructureGrow->Enter(this);
  Generate(Template, ClearCont, IncludeQ);
  OnStructureGrow->Exit(this);
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
  vec3d V;
  // all matrices after MatrixCount are new and has to be used for generation
  int currentCount = MatrixCount();
  // the fragmens to grow by a particular matrix
  smatd_list *BindingMatrices;

  TIntList* ToGrow;
  TTypeList<TIntList> Fragments2Grow;
  OnStructureGrow->Enter(this);
  for(int i=0; i < atoms.Count(); i++ )  {
    SA = atoms[i];
    SA->SetGrown(true);
    TCAtom& CA = SA->CAtom();
    for(int j=0; j < CA.AttachedAtomCount(); j++ )  {
      TCAtom& CA1 = CA.GetAttachedAtom(j);
      V = CA1.ccrd();
      BindingMatrices = GetUnitCell().GetBinding(CA, CA1, SA->ccrd(), V, false, false);
      if( BindingMatrices->Count() )  {
        for( int k=0; k < BindingMatrices->Count(); k++ )  {
          smatd& M = BindingMatrices->Item(k);
          found = false;
          for( l=0; l < MatrixCount(); l++ )  {
            if( *Matrices[l] == M )  {
              found = true;  break;
            }
          }
          if( !found )  {
            Matrices.Add( new smatd(M) );
            ToGrow = new TIntList;
            ToGrow->Add( CA1.GetFragmentId() );
            Fragments2Grow.Add(ToGrow);
          }
          else  {
            if( l >= currentCount )  {
              ToGrow = &Fragments2Grow[l-currentCount];
              found = false;
              for(int m=0; m < ToGrow->Count(); m++ )  {
                if( ToGrow->Item(m) == CA1.GetFragmentId() )  {
                  found =false;  break;
                }
              }
              if( !found )  
                ToGrow->Add( CA1.GetFragmentId() );
            }
          }
        }
      }
      delete BindingMatrices;
    }
  }
  for(int i = currentCount; i < MatrixCount(); i++ )  {
    smatd* M = Matrices[i];
    ToGrow = &Fragments2Grow[i-currentCount];
    for(int j=0; j < GetAsymmUnit().AtomCount(); j++ )  {
      for(int k=0; k < ToGrow->Count(); k++ )  {
        if( GetAsymmUnit().GetAtom(j).IsDeleted() )  continue;
        if( GetAsymmUnit().GetAtom(j).GetFragmentId() == ToGrow->Item(k) )  {
          SA1 = new TSAtom( Network );
          SA1->CAtom( GetAsymmUnit().GetAtom(j) );
          SA1->AddMatrix(M);
          SA1->ccrd() = *M * SA1->ccrd();
          SA1->SetEllipsoid( &GetUnitCell().GetEllipsoid(M->GetTag(), SA1->CAtom().GetId()) );
          AddSAtom(SA1);
        }
      }
    }
  }

  RestoreCoordinates();
  Disassemble();

  Generated = true;
  OnStructureGrow->Exit(this);
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
void TLattice::GrowAtom(int FragId, const smatd& transform)  {
  smatd *M;
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
    M = new smatd( transform );
    M->SetTag( transform.GetTag() );
    Matrices.Add( M );
  }

  OnStructureGrow->Enter(this);

  for( int i=0; i < GetAsymmUnit().AtomCount(); i++ )  {
    if( GetAsymmUnit().GetAtom(i).IsDeleted() )  continue;
    if( GetAsymmUnit().GetAtom(i).GetFragmentId() == FragId )  {
      TSAtom* SA = new TSAtom( Network );
      SA->CAtom( GetAsymmUnit().GetAtom(i) );
      SA->AddMatrix( M );
      SA->ccrd() = *M * SA->ccrd();
      SA->SetEllipsoid( &GetUnitCell().GetEllipsoid(M->GetTag(), SA->CAtom().GetId()) );
      AddSAtom(SA);
    }
  }
  RestoreCoordinates();
  Disassemble();

  Generated = true;
  OnStructureGrow->Exit(this);
}
//..............................................................................
void TLattice::GrowAtoms(const TSAtomPList& atoms, const smatd_list& matrices)  {
  smatd *M;
  smatd_plist addedMatrices;
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
      M = new smatd( matrices[i] );
      // we do not know abou the origin of this matrix ...
      M->SetTag( 0 );
      Matrices.Add( M );
      addedMatrices.Add( M );
    }
  }

  OnStructureGrow->Enter(this);
  Atoms.SetCapacity( Atoms.Count() + atoms.Count()*addedMatrices.Count() );
  for( int i=0; i < addedMatrices.Count(); i++ )  {
    for( int j=0; j < atoms.Count(); j++ )  {
      if( atoms[j]->IsDeleted() )  continue;
      TSAtom* SA = new TSAtom( Network );
      SA->CAtom( atoms[j]->CAtom() );
      SA->AddMatrix( addedMatrices[i] );
      SA->ccrd() = *addedMatrices[i] * SA->ccrd();
      SA->SetEllipsoid( &GetUnitCell().GetEllipsoid(M->GetTag(), SA->CAtom().GetId()) );
      AddSAtom(SA);
    }
  }
  RestoreCoordinates();
  Disassemble();

  Generated = true;
  OnStructureGrow->Exit(this);
}
//..............................................................................
void TLattice::Grow(const smatd& transform)  {
  smatd *M;
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
    M = new smatd( transform );
    M->SetTag( transform.GetTag() );
    Matrices.Add( M );
  }

  OnStructureGrow->Enter(this);
  Atoms.SetCapacity( Atoms.Count() + GetAsymmUnit().AtomCount() );
  for( int i=0; i < GetAsymmUnit().AtomCount(); i++ )  {
    if( GetAsymmUnit().GetAtom(i).IsDeleted() )  continue;
    TSAtom* SA = new TSAtom( Network );
    SA->CAtom( GetAsymmUnit().GetAtom(i) );
    SA->AddMatrix( M );
    SA->ccrd() = (*M) * SA->ccrd();
    SA->SetEllipsoid( &GetUnitCell().GetEllipsoid(M->GetTag(), SA->CAtom().GetId()) );
    AddSAtom(SA);
  }
  RestoreCoordinates();
  Disassemble();

  Generated = true;
  OnStructureGrow->Exit(this);
}
//..............................................................................
TSAtom* TLattice::FindSAtom(const olxstr& Label) const {
  for( int i =0; i < Atoms.Count(); i++ )
    if( !Label.Comparei( Atoms[i]->GetLabel()) )  
      return Atoms[i];
  return NULL;
}
//..............................................................................
TSAtom* TLattice::FindSAtom(const TCAtom& ca) const {
  for( int i =0; i < Atoms.Count(); i++ )
    if( ca.GetLoaderId() == Atoms[i]->CAtom().GetLoaderId() )  
      return Atoms[i];
  return NULL;
}
//..............................................................................
TSAtom* TLattice::NewCentroid(const TSAtomPList& Atoms)  {
  TSAtom *Centroid;
  vec3d cc, ce;
  double aan = 0;
  for( int i=0; i < Atoms.Count(); i++ )  {
    cc += Atoms[i]->ccrd()*Atoms[i]->CAtom().GetOccu();
    ce += vec3d::Qrt(Atoms[i]->CAtom().ccrdEsd())*Atoms[i]->CAtom().GetOccu();
    aan += Atoms[i]->CAtom().GetOccu();
  }
  if( aan == 0 )  return NULL;
  ce.Sqrt();
  ce /= aan;
  cc /= aan;
  TCAtom *CCent;
  try{ CCent = &AsymmUnit->NewCentroid(cc); }
  catch(const TExceptionBase& exc)  {
    throw TFunctionFailedException(__OlxSourceInfo, exc.Replicate());
  }
  Centroid = new TSAtom( Network );
  Centroid->CAtom(*CCent);
  CCent->ccrdEsd() = ce;
  Centroid->crd() = GetAsymmUnit().CellToCartesian(cc); 
  Centroid->AddMatrix( Matrices[0] );
  AddSAtom(Centroid);
  return Centroid;
}
//..............................................................................
TSAtom* TLattice::NewAtom(const vec3d& center)  {
  TCAtom* ca = NULL;
  try{ ca = &AsymmUnit->NewAtom();  }
  catch(const TExceptionBase& exc)  {
    throw TFunctionFailedException(__OlxSourceInfo, exc.Replicate());
  }
  ca->SetLoaderId( liNewAtom );
  ca->ccrd() = center;
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
  //TODO: need to consider occupancy for disordered groups ...
  TTypeList< AnAssociation2<TSAtom*, double> > Points;
  Points.SetCapacity( atoms.Count() );

  if( weightExtent != 0 )  {
    double swg = 0;
    for(int i=0; i < atoms.Count(); i++ )  {
      double wght = pow(atoms[i]->GetAtomInfo().GetMr(), (double)weightExtent);
      Points.AddNew(atoms[i], wght );
      swg += wght*wght;
    }
    for( int i=0; i < Points.Count(); i++ )
      Points[i].B() /= swg;
  }
  else  {
    for(int i=0; i < atoms.Count(); i++ )
      Points.AddNew(atoms[i], 1 );
  }

  Plane = new TSPlane( Network );
  Plane->Init(Points);
  return Plane;
}
//..............................................................................
void TLattice::UpdateAsymmUnit()  {
  if( Atoms.IsEmpty() )  return;
  GetAsymmUnit().InitAtomIds();
  TTypeList<TSAtomPList> AUAtoms;
  TSAtom* OA;
  const int ac = GetAsymmUnit().AtomCount();
  const int lc = ac + GetAsymmUnit().CentroidCount();
  for( int i=0; i < lc; i++ )  // create lists to store atom groups
    AUAtoms.AddNew();
  const int lat_ac = Atoms.Count();
  for( int i=0; i < lat_ac; i++ )  {
    TSAtom* A = Atoms[i];
    if( A->IsDeleted() )  continue;
    TSAtomPList& l = (A->CAtom().GetLoaderId() == liCentroid) ?  
      AUAtoms[GetAsymmUnit().AtomCount() + A->CAtom().GetId()] :
      AUAtoms[A->CAtom().GetId()];
    l.Add(A);
  }
  for( int i=0; i < lc; i++ )  {  // create lists to store atom groups
    TSAtomPList& l = AUAtoms[i];
    if( l.IsEmpty() )  {  // all atoms are deleted
      if( i >= ac ) AsymmUnit->GetCentroid(i-ac).SetDeleted(true);
      else          AsymmUnit->GetAtom(i).SetDeleted(true);
      continue;
    }
    // find the original atom, or symmetry equivalent if removed
    OA = NULL;
    const int lst_c = l.Count();
    for( int j=0; j < lst_c; j++ )  {
      TSAtom* A = l[j];
      int am_c = A->MatrixCount();
      for( int k=0; k < am_c; k++ )  {
        const smatd& m = A->GetMatrix(k);
        if( m.GetTag() == 0 && m.t.IsNull() )  {  // the original atom
          OA = A;  
          break; 
        }
        if( OA != NULL )  break;
      }
    }
    if( OA == NULL )
      OA = l[0];
    TCAtom& CA = (i >= ac) ? GetAsymmUnit().GetCentroid(i-ac) : GetAsymmUnit().GetAtom(i);
    CA.SetDeleted(false);
    if( OA->GetEllipsoid() )  {
      CA.UpdateEllp(*OA->GetEllipsoid());
    }
    CA.ccrd() = OA->ccrd();
  }
  TEStrBuffer Rep;
  if( GetAsymmUnit().DoesContainEquivalents() )
    AsymmUnit->SetContainsEquivalents( UnitCell->FindSymmEq(Rep, 0.1, false, false, false) != 0 );
}
//..............................................................................
void TLattice::ListAsymmUnit(TSAtomPList& L, TCAtomPList* Template, bool IncludeQ)  {
  ClearPlanes();
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
void TLattice::MoveFragment(const vec3d& to, TSAtom& fragAtom)  {
  if( Generated )  {
    TBasicApp::GetLog().Error("Cannot perform this operation on grown structure");
    return;
  }

  mat3d abc2xyz( GetAsymmUnit().GetCellToCartesian()),
           xyz2abc( GetAsymmUnit().GetCartesianToCell());
  abc2xyz.Transpose();
  xyz2abc.Transpose();

  vec3d from;
  from = fragAtom.ccrd();
  smatd* m = GetUnitCell().GetClosest(to, from, true);
  if( m != NULL )  {
    for(int i=0; i < fragAtom.GetNetwork().NodeCount(); i++ )  {
      TSAtom& SA = fragAtom.GetNetwork().Node(i);
      SA.CAtom().ccrd() = *m * SA.CAtom().ccrd();
      if( SA.CAtom().GetEllipsoid() != NULL )
        SA.CAtom().GetEllipsoid()->MultMatrix( abc2xyz*m->r*xyz2abc );
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

  mat3d abc2xyz( GetAsymmUnit().GetCellToCartesian()),
           xyz2abc( GetAsymmUnit().GetCartesianToCell());
  abc2xyz.Transpose();
  xyz2abc.Transpose();

  smatd* m = GetUnitCell().GetClosest(to.CAtom(), fragAtom.CAtom(), true);
  if( m != NULL )  {
    if( to.CAtom().GetFragmentId() == fragAtom.CAtom().GetFragmentId() )  {
      fragAtom.CAtom().ccrd() = *m * fragAtom.CAtom().ccrd();
      if( fragAtom.CAtom().GetEllipsoid() != NULL )
        fragAtom.CAtom().GetEllipsoid()->MultMatrix(m->r);
    }
    else  {  // move whole fragment then
      int fragId = fragAtom.CAtom().GetFragmentId();
      for(int i=0; i < Atoms.Count(); i++ )  {
        TSAtom* SA = Atoms[i];
        if( SA->CAtom().GetFragmentId() == fragId )  {
          SA->CAtom().ccrd() = *m * SA->CAtom().ccrd();
          if( SA->CAtom().GetEllipsoid() != NULL ) 
            SA->CAtom().GetEllipsoid()->MultMatrix( abc2xyz*m->r*xyz2abc );
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
void TLattice::MoveFragmentG(const vec3d& to, TSAtom& fragAtom)  {
  vec3d from;
  from = fragAtom.ccrd();
  smatd* m = GetUnitCell().GetClosest(to, from, true);
  vec3d offset;
  if( m != NULL )  {
/* restore atom centres if were changed by some other procedure */
    RestoreCoordinates();
    Generated = true;
    OnStructureGrow->Enter(this);
    Matrices.Add(m);
    for(int i=0; i < fragAtom.GetNetwork().NodeCount(); i++ )  {
      TSAtom& SA = fragAtom.GetNetwork().Node(i);
      if( SA.IsDeleted() )  continue;
      TSAtom* atom = new TSAtom( &SA.GetNetwork() );
      atom->CAtom( SA.CAtom() );
      atom->AddMatrix(m);
      atom->ccrd() = SA.ccrd();
      atom->ccrd() = (*m) * atom->ccrd();
      GetAsymmUnit().CellToCartesian(atom->ccrd(), atom->crd());
      atom->SetEllipsoid( &GetUnitCell().GetEllipsoid(m->GetTag(), atom->CAtom().GetId()) );
      AddSAtom(atom);
    }

    Disassemble();

    OnStructureGrow->Exit(this);
  }
  else
  {
    TBasicApp::GetLog().Info("Could not find closest matrix");
  }
}
//..............................................................................
void TLattice::MoveFragmentG(TSAtom& to, TSAtom& fragAtom)  {
  smatd* m = GetUnitCell().GetClosest(to.ccrd(), fragAtom.ccrd(), true);
  vec3d offset;
  if( m != NULL )  {
/* restore atom centres if were changed by some other procedure */
    RestoreCoordinates();
    Generated = true;
    OnStructureGrow->Enter(this);
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
      atom->ccrd() = SA->ccrd();
      atom->ccrd() = (*m) * atom->ccrd();
      GetAsymmUnit().CellToCartesian(atom->ccrd(), atom->crd());
      atom->SetEllipsoid( &GetUnitCell().GetEllipsoid(m->GetTag(), atom->CAtom().GetId()) );
      AddSAtom(atom);
    }

    Disassemble();

    OnStructureGrow->Exit(this);
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
    OnStructureGrow->Enter(this);
  }

  mat3d abc2xyz( GetAsymmUnit().GetCellToCartesian()),
           xyz2abc( GetAsymmUnit().GetCartesianToCell());
  abc2xyz.Transpose();
  xyz2abc.Transpose();

  vec3d molCenter;
  vec3d cnt, err;
  smatd* m;
  for( int i=0; i < Fragments.Count(); i++ )  {
    TNetwork* frag = Fragments[i];
    molCenter.Null();
     for( int j=0; j < frag->NodeCount(); j++ )
       molCenter += frag->Node(j).ccrd();
    if( frag->NodeCount() )  molCenter /= frag->NodeCount();
    m = GetUnitCell().GetClosest(vec3d(0.5, 0.5, 0.5), molCenter, true);
    if( m != NULL )  {
      if( Generated )  {
        for(int j=0; j < frag->NodeCount(); j++ )  {
          TSAtom& SA = frag->Node(j);
          SA.ccrd() = *m * SA.ccrd();
          SA.SetEllipsoid( &GetUnitCell().GetEllipsoid(m->GetTag(), SA.CAtom().GetId()) );
        }
      }
      else  {
        for(int j=0; j < frag->NodeCount(); j++ )  {
          TSAtom& SA = frag->Node(j);
          SA.CAtom().ccrd() = *m * SA.CAtom().ccrd();
          if( SA.CAtom().GetEllipsoid() != NULL ) 
            SA.CAtom().GetEllipsoid()->MultMatrix( abc2xyz*m->r*xyz2abc );
        }
      }
      delete m;
    }
  }
  if( !Generated )  Uniq();
  else  {
    RestoreCoordinates();
    Disassemble();
    OnStructureGrow->Exit(this);
  }
}
//..............................................................................
void TLattice::Compaq()  {
  if( Fragments.Count() < 2 )  return;
  if( Generated )  return;
  //if( Generated )  {
  //  TBasicApp::GetLog().Error("Please note that asymetric unit will not be updated: the structure is grown");
  //  RestoreCoordinates();
  //  OnStructureGrow->Enter(this);
  //}
  vec3d molCenter, acenter;
  smatd* m;

  mat3d abc2xyz( GetAsymmUnit().GetCellToCartesian()),
           xyz2abc( GetAsymmUnit().GetCartesianToCell());
  abc2xyz.Transpose();
  xyz2abc.Transpose();

  molCenter[0] = 0;  molCenter[1] = 0;  molCenter[2] = 0;
  TNetwork* frag = Fragments[0];
  for( int j=0; j < frag->NodeCount(); j++ )
    acenter += frag->Node(j).ccrd();
  acenter /= frag->NodeCount();
  for( int i=1; i < Fragments.Count(); i++ )  {
    frag = Fragments[i];
    m = NULL;

    for(int j=0; j < Fragments[0]->NodeCount(); j++ )  {
      TSAtom& fa = Fragments[0]->Node(j);
      for( int k=0; k < frag->NodeCount(); k++ )  {
        if( frag->Node(k).CAtom().IsAttachedTo( fa.CAtom() ) )  {
          m = GetUnitCell().GetClosest(fa.ccrd(), frag->Node(k).ccrd(), true);
          if( m != NULL )  break;
        }
      }
      if( m != NULL )  break;
    }
    if( m == NULL )  {
      molCenter[0] = 0;  molCenter[1] = 0;  molCenter[2] = 0;
      for( int j=0; j < frag->NodeCount(); j++ )
        molCenter += frag->Node(j).ccrd();
      if( frag->NodeCount() )  molCenter /= frag->NodeCount();
      m = GetUnitCell().GetClosest(acenter, molCenter, true);
    }
    if( m != NULL )  {
      if( !Generated )  {
        for(int j=0; j < frag->NodeCount(); j++ )  {
          TSAtom& SA = frag->Node(j);
          if( SA.IsDeleted() )  continue;
          SA.CAtom().ccrd() = *m * SA.CAtom().ccrd();
          if( SA.CAtom().GetEllipsoid() != NULL ) 
            SA.CAtom().GetEllipsoid()->MultMatrix( abc2xyz*m->r*xyz2abc );
        }
        delete m;
      }
      else  {
        for(int j=0; j < frag->NodeCount(); j++ )  {
          TSAtom& SA = frag->Node(j);
          if( SA.IsDeleted() )  continue;
          SA.ccrd() = *m * SA.ccrd();
          SA.AddMatrix(m);
          GetAsymmUnit().CellToCartesian(SA.ccrd(), SA.crd());
        }
        Matrices.Add(m);
      }
    }
  }
  if( !Generated )  Uniq();
  else  {
    Disassemble();
    OnStructureGrow->Exit(this);
  }
}
//..............................................................................
void TLattice::CompaqAll()  {
  if( Generated )  return;

  if( Fragments.Count() < 2 )  return;

  mat3d abc2xyz( GetAsymmUnit().GetCellToCartesian()),
           xyz2abc( GetAsymmUnit().GetCartesianToCell());
  abc2xyz.Transpose();
  xyz2abc.Transpose();

  smatd* m;
  for( int i=0; i < Fragments.Count(); i++ )  {
    for( int j=i+1; j < Fragments.Count(); j++ )  {
      m = NULL;

      for(int k=0; k < Fragments[i]->NodeCount(); k++ )  {
        TSAtom& fa = Fragments[i]->Node(k);
        for( int l=0; l < Fragments[j]->NodeCount(); l++ )  {
          if( Fragments[j]->Node(l).CAtom().IsAttachedTo( fa.CAtom() ) )  {
            m = GetUnitCell().GetClosest(fa.CAtom().ccrd(), Fragments[j]->Node(l).CAtom().ccrd(), true);
            if( m != NULL )  break;
          }
        }
        if( m != NULL )  break;
      }

      if( m == NULL )  continue;
      for(int k=0; k < Fragments[j]->NodeCount(); k++ )  {
        TSAtom& SA = Fragments[j]->Node(k);
        if( SA.IsDeleted() )  continue;
        SA.CAtom().ccrd() = *m * SA.CAtom().ccrd();
        if( SA.CAtom().GetEllipsoid() != NULL )
          SA.CAtom().GetEllipsoid()->MultMatrix( abc2xyz*m->r*xyz2abc );
      }
      delete m;
    }
  }
  Uniq();
}
//..............................................................................
void TLattice::TransformFragments(const TSAtomPList& fragAtoms, const smatd& transform)  {
  if( Generated )  {
    TBasicApp::GetLog().Error("Cannot perform this operation on grown structure");
    return;
  }

  mat3d abc2xyz( GetAsymmUnit().GetCellToCartesian()),
           xyz2abc( GetAsymmUnit().GetCartesianToCell());
  abc2xyz.Transpose();
  xyz2abc.Transpose();

  for(int i=0; i < fragAtoms.Count(); i++ )
    fragAtoms[i]->GetNetwork().SetTag(i);

  for(int i=0; i < fragAtoms.Count(); i++ )  {
    if( fragAtoms[i]->GetNetwork().GetTag() == i )  {
      for(int j=0; j < fragAtoms[i]->GetNetwork().NodeCount(); j++ )  {
        TSAtom& SA = fragAtoms[i]->GetNetwork().Node(j);
        SA.CAtom().ccrd() *= transform.r;
        SA.CAtom().ccrd() += transform.t;
        if( SA.CAtom().GetEllipsoid() != NULL ) 
          SA.CAtom().GetEllipsoid()->MultMatrix(abc2xyz*transform.r*xyz2abc);
      }
    }
  }
  Uniq();
}
//..............................................................................
void TLattice::Disassemble()  {
  // clear bonds & fargments
  OnDisassemble->Enter(this);
  ClearBonds();
  ClearFragments();

  // find bonds & fragments
  Network->Disassemble(Atoms, Fragments, &Bonds);
  Fragments.QuickSorter.SortSF(Fragments, TLattice_SortFragments);
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
  OnDisassemble->Exit(this);
}
//..............................................................................
void TLattice::RestoreCoordinates()  {
  for( int i=0; i < Atoms.Count(); i++ )  {
    Atoms[i]->crd() = Atoms[i]->ccrd();
    GetAsymmUnit().CellToCartesian(Atoms[i]->ccrd(), Atoms[i]->crd());
  }
}
//..............................................................................
bool TLattice::_AnalyseAtomHAdd(AConstraintGenerator& cg, TSAtom& atom, TSAtomPList& ProcessingAtoms, int part, TCAtomPList* generated)  {

  if( ProcessingAtoms.IndexOf(&atom) != -1 || (atom.CAtom().IsHAttached() && part == -1) )
    return false;
  ProcessingAtoms.Add( &atom );

  TBasicAtomInfo& HAI = AtomsInfo.GetAtomInfo(iHydrogenIndex);
  TAtomEnvi AE;
  UnitCell->GetAtomEnviList(atom, AE, false, part);
  //if( atom.GetAtomInfo() == iCarbonIndex )  { // treat hapta bonds
  //  for( int i=0; i < AE.Count(); i++ )  {
  //    vec3d v( AE.GetCrd(i) - atom.crd());
  //    if( v.Length() > 2.0 )  {
  //      AE.Delete(i);
  //      i--;
  //    }
  //  }
  //}
  if( part == -1 )  {  // check for disorder
    TIntList parts;
    TDoubleList occu;
    RefinementModel* rm = GetAsymmUnit().GetRefMod();
    for( int i=0; i < AE.Count(); i++ )  {
      if( AE.GetCAtom(i).GetPart() != 0 && AE.GetCAtom(i).GetPart() != AE.GetBase().CAtom().GetPart() ) 
        if( parts.IndexOf(AE.GetCAtom(i).GetPart()) == -1 )  {
          parts.Add( AE.GetCAtom(i).GetPart() );
          occu.Add( rm->Vars.GetAtomParam(AE.GetCAtom(i), var_name_Sof) );
        }
    }
    if( !parts.IsEmpty() )  {  // here we go..
      TCAtomPList gen;
      ProcessingAtoms.Remove(&atom);
      for( int i=0; i < parts.Count(); i++ )  {
        _AnalyseAtomHAdd(cg, atom, ProcessingAtoms, parts[i], &gen);
        for( int j=0; j < gen.Count(); j++ )  {
          gen[j]->SetPart( parts[i] );
          rm->Vars.SetAtomParam(*gen[j], var_name_Sof, occu[i] );
        }
        gen.Clear();
      }
      return false;
    }
  }
  if( atom.GetAtomInfo() == iCarbonIndex )  {
    if( AE.Count() == 1 )  {
      // check acetilene
      double d = AE.GetCrd(0).DistanceTo( atom.crd() );
      TSAtom* A = FindSAtom( AE.GetLabel(0) );
      TAtomEnvi NAE;
      if( A == 0 )
        throw TFunctionFailedException(__OlxSourceInfo, olxstr("Could not locate atom ") << AE.GetLabel(0) );

      UnitCell->GetAtomEnviList(*A, NAE, false, part);
      if( A->GetAtomInfo() == iCarbonIndex && NAE.Count() == 2 && d < 1.2)  {
        TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": XCH" );
        cg.FixAtom( AE, fgCH1, HAI, NULL, generated);
      }
      else  {
        if( d > 1.35 )  {
          TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": XCH3" );
          cg.FixAtom( AE, fgCH3, HAI, NULL, generated);
        }
        else  {
          if( d < 1.25 )  {
            if( NAE.Count() > 1 ) {
              TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": X=CH2" );
              cg.FixAtom( AE, fgCH2, HAI, NULL, generated);
            }
            else
              TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": possibly X=CH2" );
          }
        }
      }
    }
    else  if( AE.Count() == 2 )  {
      vec3d a = AE.GetCrd(0);
        a -= atom.crd();
      vec3d b = AE.GetCrd(1);
        b -= atom.crd();
      double v = a.CAngle(b);
      v = acos(v)*180/M_PI;
      double d1 = AE.GetCrd(0).DistanceTo( atom.crd() );
      double d2 = AE.GetCrd(1).DistanceTo( atom.crd() );
      if(  d1 > 1.4 && d2 > 1.4 && v < 120 )  {
        TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": XYCH2" );
        cg.FixAtom( AE, fgCH2, HAI, NULL, generated);
      }
      else  {
        if( (d1 < 1.4 || d2 < 1.4) && v < 160 )  {
          TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": X(Y=C)H" );
          cg.FixAtom( AE, fgCH1, HAI, NULL, generated);
        }
      }
    }
    else if( AE.Count() == 3 )  {
      double v = TetrahedronVolume( atom.crd(), AE.GetCrd(0), AE.GetCrd(1), AE.GetCrd(2) );
      if( v > 0.3 )  {
        TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": XYZCH" );
        cg.FixAtom( AE, fgCH1, HAI, NULL, generated);
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
        cg.FixAtom( AE, fgBH1, HAI, NULL, generated);
      }
    }
  }
  else if( atom.GetAtomInfo() == iNitrogenIndex )  {  // nitrogen
    if( AE.Count() == 1 )  {
      double d = AE.GetCrd(0).DistanceTo( atom.crd() );
      if( d > 1.35 )  {
        TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": XNH2" );
        cg.FixAtom( AE, fgNH2, HAI, NULL, generated);
      }
      else  {
        if( d > 1.2 )  {  //else nitrile
          // have to check if double bond
          TSAtom* A = FindSAtom( AE.GetLabel(0) );
          TAtomEnvi NAE;
          if( A == 0 )
            throw TFunctionFailedException(__OlxSourceInfo, olxstr("Could not locate atom ") << AE.GetLabel(0) );

          UnitCell->GetAtomEnviList(*A, NAE, false, part);
          NAE.Exclude( atom.CAtom() );

          if( A->GetAtomInfo() == iCarbonIndex && NAE.Count() > 1 )  {
            vec3d a = NAE.GetCrd(0);
              a -= NAE.GetBase().crd();
            vec3d b = AE.GetBase().crd();
              b -= NAE.GetBase().crd();

            d = a.CAngle(b);
            d = acos(d)*180/M_PI;
            if( d > 115 && d < 130 )  {
              TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": X=NH2" );
              cg.FixAtom( AE, fgNH2, HAI, &NAE, generated);
            }
          }
          else  {
            TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": X=NH" );
            cg.FixAtom( AE, fgNH1, HAI, NULL, generated);
          }
        }
      }
    }
    else  if( AE.Count() == 2 )  {
      vec3d a = AE.GetCrd(0);
        a -= atom.crd();
      vec3d b = AE.GetCrd(1);
        b -= atom.crd();
      double v = a.CAngle(b);
      v = acos(v)*180/M_PI;
      double d1 = AE.GetCrd(0).DistanceTo( atom.crd() );
      double d2 = AE.GetCrd(1).DistanceTo( atom.crd() );
      if( d1 > 1.72 || d2 > 1.72 )  {  // coordination?
        if( (d1 < 1.5 && d1 > 1.35) || (d2 < 1.5 && d2 > 1.35) )  {
          TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": RNH(2)M" );
          cg.FixAtom( AE, fgNH2, HAI, NULL, generated);
        }
      }
      else if( v < 120 && d1 > 1.45 && d2 > 1.45 )  {
        TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": R2NH2+" );
        cg.FixAtom( AE, fgNH2, HAI, NULL, generated);
      }
      else if( v < 120 || d1 < 1.3 || d2 < 1.3 )
        ;
      else  {
        if( (d1+d2) > 2.73 )  {
          TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": XYNH" );
          cg.FixAtom( AE, fgNH1, HAI, NULL, generated);
        }
      }
    }
    else if( AE.Count() == 3 )  {
    // remove ccordination bond ...
      vec3d a = AE.GetCrd(0);
        a -= atom.crd();
      vec3d b = AE.GetCrd(1);
        b -= atom.crd();
      vec3d c = AE.GetCrd(2);
        c -= atom.crd();
      double v1 = a.CAngle(b);  v1 = acos(v1)*180/M_PI;
      double v2 = a.CAngle(c);  v2 = acos(v2)*180/M_PI;
      double v3 = b.CAngle(c);  v3 = acos(v3)*180/M_PI;
      double d1 = AE.GetCrd(0).DistanceTo( atom.crd() );
      double d2 = AE.GetCrd(1).DistanceTo( atom.crd() );
      double d3 = AE.GetCrd(2).DistanceTo( atom.crd() );
      if( (v1+v2+v3) < 350 && d1 > 1.45 && d2 > 1.45 && d3 > 1.45 )  {
        if( d1 > 1.75 || d2 > 1.75 || d3 > 1.75 )  {
          TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": R2HN->M" );
          cg.FixAtom( AE, fgNH1, HAI, NULL, generated);
        }
        else  {
          TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": R3NH+" );
          cg.FixAtom( AE, fgNH1, HAI, NULL, generated);
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
        if( !_AnalyseAtomHAdd( cg, *FindSAtom(pivoting.GetCAtom(0)), ProcessingAtoms) )
          pivoting.Clear();
      cg.FixAtom( AE, fgOH2, HAI, &pivoting, generated);
    }
    else if( AE.Count() == 1 )  {
      double d = AE.GetCrd(0).DistanceTo( atom.crd() );
      if( d > 1.3 )   {  // otherwise a doubl bond
        if( AE.GetBAI(0) == iChlorineIndex )
          ;
        else  if( AE.GetBAI(0) == iCarbonIndex )  {  // carbon
          TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": COH" );
          TAtomEnvi pivoting;
          UnitCell->GetAtomPossibleHBonds(AE, pivoting);
          RemoveNonHBonding( pivoting );
          if( pivoting.Count() > 0 )
            if( !_AnalyseAtomHAdd( cg, *FindSAtom(pivoting.GetCAtom(0)), ProcessingAtoms) )
              pivoting.Clear();
          cg.FixAtom( AE, fgOH1, HAI, &pivoting, generated);
        }
        else  if( AE.GetBAI(0) == iSulphurIndex )  {
          if( d > 1.48 )  {
            TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": SOH" );
            cg.FixAtom( AE, fgOH1, HAI, NULL, generated);
          }
        }
        else  if( AE.GetBAI(0) == iPhosphorusIndex )  {
          if( d > 1.54 )  {
            TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": POH" );
            TAtomEnvi pivoting;
            UnitCell->GetAtomPossibleHBonds(AE, pivoting);
            RemoveNonHBonding( pivoting );
            if( pivoting.Count() > 0 )
              if( !_AnalyseAtomHAdd( cg, *FindSAtom(pivoting.GetCAtom(0)), ProcessingAtoms) )
                pivoting.Clear();
            cg.FixAtom( AE, fgOH1, HAI, &pivoting, generated);
          }
        }
        else  if( AE.GetBAI(0) == iSiliconIndex )  {
          if( d > 1.6 )  {
            TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": SiOH" );
            TAtomEnvi pivoting;
            UnitCell->GetAtomPossibleHBonds(AE, pivoting);
            RemoveNonHBonding( pivoting );
            if( pivoting.Count() > 0 )
              if( !_AnalyseAtomHAdd( cg, *FindSAtom(pivoting.GetCAtom(0)), ProcessingAtoms) )
                pivoting.Clear();
            cg.FixAtom( AE, fgOH1, HAI, &pivoting, generated);
          }
        }
        else  if( AE.GetBAI(0) == iBoronIndex )  {
          if( d < 1.38 )  {
            TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": B(III)OH" );
            TAtomEnvi pivoting;
            UnitCell->GetAtomPossibleHBonds(AE, pivoting);
            RemoveNonHBonding( pivoting );
            if( pivoting.Count() > 0 )
              if( !_AnalyseAtomHAdd( cg, *FindSAtom(pivoting.GetCAtom(0)), ProcessingAtoms) )
                pivoting.Clear();
            cg.FixAtom( AE, fgOH1, HAI, &pivoting, generated);
          }
        }
        else if( d > 1.8 )  {  // coordination bond?
          TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": possibly M-OH2" );
          cg.FixAtom( AE, fgOH2, HAI, NULL, generated);
        }
      }
    }
  }
  else if( atom.GetAtomInfo() == iBoronIndex )  {  // boron
    if( AE.Count() == 4 )  {
      vec3d a, b;
      double sumAng = 0;
      for( int i=0; i < AE.Count(); i++ )  {
        a = AE.GetCrd(i);
        a -= atom.crd();
        for( int j=i+1; j < AE.Count(); j++ )  {
          b = AE.GetCrd(j);
          b -= atom.crd();
          double ca = b.CAngle(a);
          if( ca < -1 )  ca = -1;
          if( ca > 1 )   ca = 1;
          sumAng += acos(ca);
        }
      }
      if( sumAng*180/M_PI > 700 )  {   //!! not sure it works, lol
        TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": X4BH" );
        cg.FixAtom( AE, fgBH1, HAI, NULL, generated);
      }
    }
    else if( AE.Count() == 5 )  {
      TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": X5BH" );
      cg.FixAtom( AE, fgBH1, HAI, NULL, generated);
    }
  }
  else if( atom.GetAtomInfo() == iSiliconIndex )  {
    if( AE.Count() == 3 )  {
      double v = TetrahedronVolume( atom.crd(), AE.GetCrd(0), AE.GetCrd(1), AE.GetCrd(2) );
      if( v > 0.5 )  {
        TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": XYZSiH" );
        cg.FixAtom( AE, fgSiH1, HAI, NULL, generated);
      }
    }
  }
  else if( atom.GetAtomInfo() == iSulphurIndex )  {
    if( AE.Count() == 1 && AE.GetBAI(0) == iCarbonIndex )  {
      double d = AE.GetCrd(0).DistanceTo( atom.crd() );
      if( d > 1.72 )  {
        TBasicApp::GetLog().Info( olxstr(atom.GetLabel()) << ": CSH" );
        cg.FixAtom( AE, fgSH1, HAI, NULL, generated);
      }
    }
  }
  ProcessingAtoms.Delete( ProcessingAtoms.IndexOf(&atom) );
  return true;
}
//..............................................................................
void TLattice::_ProcessRingHAdd(AConstraintGenerator& cg, const TPtrList<TBasicAtomInfo>& rcont) {
  TTypeList< TSAtomPList > rings;
  TBasicAtomInfo& HAI = AtomsInfo.GetAtomInfo(iHydrogenIndex);
  for( int i=0; i < FragmentCount(); i++ )
    GetFragment(i).FindRings(rcont, rings);
  TAtomEnvi AE;
  for(int i=0; i < rings.Count(); i++ )  {
    double rms = TSPlane::CalcRMS( rings[i] );
    if( rms < 0.05 && TNetwork::IsRingRegular( rings[i]) )  {
      for( int j=0; j < rings[i].Count(); j++ )  {
        AE.Clear();
        UnitCell->GetAtomEnviList(*rings[i][j], AE);
        if( AE.Count() == 3 )  {
          double v = TetrahedronVolume( AE.GetBase().crd(), AE.GetCrd(0), AE.GetCrd(1), AE.GetCrd(2) );
          if( v < 0.05 )  continue;  // coordination
        }
        for( int k=0; k < AE.Count(); k++ )  {
          vec3d v( AE.GetCrd(k) - rings[i][j]->crd());
          if( v.Length() > 2.0 )  {
            AE.Delete(k);
            k--;
          }
        }
        if( AE.Count() == 2 && rings[i][j]->GetAtomInfo() == iCarbonIndex)  {
          TBasicApp::GetLog().Info( olxstr(rings[i][j]->GetLabel()) << ": X(Y=C)H" );
          cg.FixAtom( AE, fgCH1, HAI);
          rings[i][j]->CAtom().SetHAttached(true);
        }
      }
    }
  }
}
//..............................................................................
void TLattice::AnalyseHAdd(AConstraintGenerator& cg, const TSAtomPList& atoms)  {

  TPtrList<TBasicAtomInfo> CTypes;
  CTypes.Add( &AtomsInfo.GetAtomInfo(iCarbonIndex) );
  CTypes.Add( &AtomsInfo.GetAtomInfo(iNitrogenIndex) );
  CTypes.Add( &AtomsInfo.GetAtomInfo(iOxygenIndex) );
  CTypes.Add( &AtomsInfo.GetAtomInfo(iBoronIndex) );
  CTypes.Add( &AtomsInfo.GetAtomInfo(iSiliconIndex) );
  CTypes.Add( &AtomsInfo.GetAtomInfo(iSulphurIndex) );
  TSAtomPList ProcessingAtoms;

  for( int i=0; i < atoms.Count(); i++ )
    atoms[i]->CAtom().SetHAttached(false);

  // treat rings
  TPtrList<TBasicAtomInfo> rcont;
  rcont.Add( &AtomsInfo.GetAtomInfo(iCarbonIndex) );
  for( int i=0; i < 4; i++ )  
    rcont.Add( rcont[0] );
  _ProcessRingHAdd(cg, rcont); // Cp
  rcont.Add( rcont[0] );
  _ProcessRingHAdd(cg, rcont); // Ph
  rcont.Last() = &AtomsInfo.GetAtomInfo(iNitrogenIndex);
  _ProcessRingHAdd(cg, rcont); // Py

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
    if( SA->GetAtomInfo() == AtomsInfo.GetAtomInfo(iOxygenIndex) )  {
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
        double d = AE.GetCrd(0).DistanceTo( SA->crd() );
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
    else if( SA->GetAtomInfo() == AtomsInfo.GetAtomInfo(iNitrogenIndex) )  {
      if( AE.Count() > 3 )
          Envi.Exclude( SA->CAtom() );
    }
  }
  // choose the shortest bond ...
  if( Envi.Count() > 1 )  {
    TPSTypeList<double, TCAtom*> hits;

    for( int i=0; i < Envi.Count(); i++ )
      hits.Add( Envi.GetBase().crd().DistanceTo( Envi.GetCrd(i) ), &Envi.GetCAtom(i) );

    while( hits.Count() > 1 &&
      ((hits.GetComparable(hits.Count()-1) - hits.GetComparable(0)) > 0.1) )  {
      Envi.Exclude( *hits.GetObject(hits.Count()-1) );
      hits.Remove(hits.Count()-1);
    }
  }
  // all similar length  .... Q peaks might help :)
  if( Envi.Count() > 1 )  {
    TPSTypeList<double, TCAtom*> hits;
    vec3d vec1, vec2;
    AE.Clear();
    UnitCell->GetAtomQEnviList( Envi.GetBase(), AE );
    for( int i=0; i < AE.Count(); i++ )  {
//      v1 = AE.GetCrd(i);
//      v1 -= Envi.GetBase()->crd();
      double d = Envi.GetBase().crd().DistanceTo( AE.GetCrd(i) );

      if( d < 0.7 || d > 1.3 )  {
        AE.Exclude( AE.GetCAtom(i) );
        i--;
      }
    }
    if( AE.IsEmpty() || AE.Count() > 1 )  return;
    vec1 = AE.GetCrd(0);
    vec1 -= Envi.GetBase().crd();
    for( int i=0; i < Envi.Count(); i++ )  {
      vec2 = Envi.GetCrd(i);
      vec2 -= Envi.GetBase().crd();
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
      if( atoms[i]->GetEllpId() != -1 )  {
         GetAsymmUnit().NullEllp( atoms[i]->GetEllpId() );
         atoms[i]->AssignEllp( NULL );
      }
    }
    GetAsymmUnit().PackEllps();
  }
  else  {
    evecd ee(6);
    ee[0] = ee[1] = ee[2] = 0.025;
    for( int i=0; i < atoms.Count(); i++ )  {
      if( atoms[i]->GetEllipsoid() == NULL )
         atoms[i]->UpdateEllp( ee );
    }
  }
  OnStructureUniq->Enter(this);
  Init();
  OnStructureUniq->Exit(this);
}
//..............................................................................
void TLattice::ToDataItem(TDataItem& item) const  {
  item.AddField("delta", Delta);
  item.AddField("deltai", DeltaI);
  item.AddField("grown", Generated);
  GetAsymmUnit().ToDataItem(item.AddItem("AUnit"));
  TDataItem& mat = item.AddItem("Matrices");
  const int mat_c = Matrices.Count();
  // save matrices, change matrix tags to the position in the list and remember old tags
  TIntList m_tags(mat_c);
  for( int i=0; i < mat_c; i++ )  {
    mat.AddItem(i, TSymmParser::MatrixToSymmEx(*Matrices[i])).AddField("tag", Matrices[i]->GetTag());
    m_tags[i] = Matrices[i]->GetTag();
    Matrices[i]->SetTag(i);
  }
  // initialise bond tags
  int sbond_tag = 0;
  for( int i=0; i < Bonds.Count(); i++ )  {
    if( Bonds[i]->IsDeleted() )  continue;
    Bonds[i]->SetTag(sbond_tag++);
  }
  // initialise atom tags
  int satom_tag = 0;
  for( int i=0; i < Atoms.Count(); i++ )  {
    if( Atoms[i]->IsDeleted() )  continue;
    Atoms[i]->SetTag(satom_tag++);
  }
  // initialise fragment tags
  int frag_tag = 0;
  for( int i=0; i < Fragments.Count(); i++ )  {
    if( Fragments[i]->NodeCount() == 0 )  continue;
    Fragments[i]->SetTag(frag_tag++);
  }
  // save satoms - only the original CAtom Tag and the generating matrix tag
  TDataItem& atoms = item.AddItem("Atoms");
  for( int i=0; i < Atoms.Count(); i++ )  {
    if( Atoms[i]->IsDeleted() )  continue;
    Atoms[i]->ToDataItem( atoms.AddItem("Atom") );
  }
  // save bonds
  TDataItem& bonds = item.AddItem("Bonds");
  for( int i=0; i < Bonds.Count(); i++ )  {
    if( Bonds[i]->IsDeleted() )  continue;
    Bonds[i]->ToDataItem( bonds.AddItem("Bond") );
  }
  // save fragments
  TDataItem& frags = item.AddItem("Fragments");
  for( int i=0; i < Fragments.Count(); i++ )  {
    if( Fragments[i]->NodeCount() == 0 )  continue;
    Fragments[i]->ToDataItem( frags.AddItem("Fragment") );
  }
  // restore original matrix tags 
  for( int i=0; i < mat_c; i++ )
    Matrices[i]->SetTag(m_tags[i]);
  // save planes
  TSPlanePList valid_planes;
  for( int i=0; i < Planes.Count(); i++ )  {
    if( Planes[i]->IsDeleted() ) continue;
    int p_ac = 0;  
    for( int j=0; j < Planes[i]->Count(); j++ ) 
      if( !Planes[i]->Atom(j).IsDeleted() )
        p_ac++;
    if( p_ac >= 3 ) // a plane must contain at least three atoms
      valid_planes.Add( Planes[i] );
    else
      Planes[i]->SetDeleted(true);
  }
  TDataItem& planes = item.AddItem("Planes");
  for( int i=0; i < valid_planes.Count(); i++ )
    valid_planes[i]->ToDataItem(planes.AddItem("Plane"));  
}
//..............................................................................
void TLattice::FromDataItem(TDataItem& item)  {
  Clear(true);

  Delta = item.GetRequiredField("delta").ToDouble();
  DeltaI = item.GetRequiredField("deltai").ToDouble();
  Generated = item.GetRequiredField("grown").ToBool();
  GetAsymmUnit().FromDataItem( item.FindRequiredItem("AUnit") );
  const TDataItem& mat = item.FindRequiredItem("Matrices");
  Matrices.SetCapacity( mat.ItemCount() );
  for( int i=0; i < mat.ItemCount(); i++ )  {
    smatd* m = new smatd;
    TSymmParser::SymmToMatrix(mat.GetItem(i).GetValue(), *m);
    Matrices.Add(m);
    m->SetTag( mat.GetItem(i).GetRequiredField("tag").ToInt() );
  }
  // precreate fragments
  const TDataItem& frags = item.FindRequiredItem("Fragments");
  Fragments.SetCapacity( frags.ItemCount() );
  for( int i=0; i < frags.ItemCount(); i++ )
    Fragments.Add( new TNetwork(this, NULL) );
  // precreate bonds
  const TDataItem& bonds = item.FindRequiredItem("Bonds");
  Bonds.SetCapacity(bonds.ItemCount());
  for( int i=0; i < bonds.ItemCount(); i++ )
    Bonds.Add( new TSBond(NULL) )->SetLatId(i);
  // precreate and load atoms
  const TDataItem& atoms = item.FindRequiredItem("Atoms");
  Atoms.SetCapacity( atoms.ItemCount() );
  for( int i=0; i < atoms.ItemCount(); i++ )
    Atoms.Add( new TSAtom(NULL) )->SetLatId(i);
  for( int i=0; i < atoms.ItemCount(); i++ )
    Atoms[i]->FromDataItem(atoms.GetItem(i), *this);
  // load bonds
  for( int i=0; i < bonds.ItemCount(); i++ )
    Bonds[i]->FromDataItem(bonds.GetItem(i), Fragments);
  // load fragments
  for( int i=0; i < frags.ItemCount(); i++ )
    Fragments[i]->FromDataItem( frags.GetItem(i) );
  GetUnitCell().InitMatrices();
  TEStrBuffer tmp;
  int eqc = GetUnitCell().FindSymmEq(tmp, 0.1, true, false, false); // find and not remove
  GetAsymmUnit().SetContainsEquivalents( eqc != 0 );
  //Disassemble();
  TDataItem& planes = item.FindRequiredItem("Planes");
  for( int i=0; i < planes.ItemCount(); i++ )
    Planes.Add( new TSPlane(Network) )->FromDataItem(planes.GetItem(i));
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


TLibrary*  TLattice::ExportLibrary(const olxstr& name)  {
  TLibrary* lib = new TLibrary( name.IsEmpty() ? olxstr("latt") : name);
  lib->RegisterFunction<TLattice>( new TFunction<TLattice>(this,  &TLattice::LibGetFragmentCount, "GetFragmentCount", fpNone,
"Returns number of fragments in the lattice") );
  lib->RegisterFunction<TLattice>( new TFunction<TLattice>(this,  &TLattice::LibGetFragmentAtoms, "GetFragmentAtoms", fpOne,
"Returns a comma separated list of atoms in specified fragment") );
  return lib;
}

