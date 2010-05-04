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
  return olx_cmp_size_t(n2->NodeCount(), n1->NodeCount());
}
int TLattice_SortAtomsById(const TSAtom* a1, const TSAtom* a2)  {
  return olx_cmp_size_t(a1->CAtom().GetId(), a2->CAtom().GetId());
}
int TLattice_AtomsSortByDistance(const TSAtom* A1, const TSAtom* A2)  {
  const double d = A1->crd().QLength() - A2->crd().QLength();
  return (d < 0 ? -1 : ((d > 0 ) ? 1 : 0));
}
//---------------------------------------------------------------------------
// TLattice function bodies
//---------------------------------------------------------------------------
TLattice::TLattice() :
  OnStructureGrow(Actions.New("STRGEN")),
  OnStructureUniq(Actions.New("STRUNIQ")),
  OnDisassemble(Actions.New("DISASSEBLE"))
{
  Generated = false;
  AsymmUnit = new TAsymmUnit(this);
  UnitCell = new TUnitCell(this);
  Network = new TNetwork(this, NULL);
  Delta = 0.5f;
  DeltaI = 1.2f;
  _GrowInfo = NULL;
}
//..............................................................................
TLattice::~TLattice()  {
  Clear(true);
  delete UnitCell;
  delete AsymmUnit;
  delete Network;
  if( _GrowInfo != NULL )
    delete _GrowInfo;
}
//..............................................................................
void  TLattice::ClearAtoms()  {
  for( size_t i=0; i < Atoms.Count(); i++ )
    delete Atoms[i];
  Atoms.Clear();
}
//..............................................................................
void  TLattice::ClearBonds()  {
  for( size_t i=0; i < Bonds.Count(); i++ )
    delete Bonds[i];
  Bonds.Clear();
}
//..............................................................................
void  TLattice::ClearFragments()  {
  for( size_t i=0; i < Fragments.Count(); i++ )
    delete Fragments[i];
  Fragments.Clear();
}
//..............................................................................
void  TLattice::ClearMatrices()  {
  for( size_t i=0; i < Matrices.Count(); i++ )
    delete Matrices[i];
  Matrices.Clear();
}
//..............................................................................
void  TLattice::ClearPlanes()  {
  for( size_t i=0; i < Planes.Count(); i++ )
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
  B->SetLattId( Bonds.Count() );
  Bonds.Add( B );
}
//..............................................................................
void TLattice::AddSAtom(TSAtom *A)  {
  A->SetLattId( Atoms.Count() );
  Atoms.Add(A);
}
//..............................................................................
void TLattice::AddSPlane(TSPlane *P)  {
  P->SetLattId( Planes.Count() );
  Planes.Add(P);
}
//..............................................................................
size_t TLattice::GenerateMatrices(const vec3d& VFrom, const vec3d& VTo,
  const vec3d& MFrom, const vec3d& MTo)  {
  GenerateMatrices(Matrices, VFrom, VTo, MFrom, MTo);
  return MatrixCount();
}
//..............................................................................
size_t TLattice::GenerateMatrices(smatd_plist& Result,
     const vec3d& VFrom, const vec3d& VTo,
     const vec3d& MFrom, const vec3d& MTo)  
{
  const size_t mstart = Result.Count();
  const vec3d Center = GetAsymmUnit().GetOCenter(true, false);

  Result.SetCapacity( (int)(GetUnitCell().MatrixCount()*
                              (olx_abs(VTo[0]-VFrom[0])+1)*
                              (olx_abs(VTo[1]-VFrom[1])+1)*
                              (olx_abs(VTo[2]-VFrom[2])+1)) );

  const size_t uc_mc = GetUnitCell().MatrixCount();
  for( size_t i=0; i < uc_mc; i++ )  {
    for( int j=(int)VFrom[0]; j <= (int)VTo[0]; j++ )
      for( int k=(int)VFrom[1]; k <= (int)VTo[1]; k++ )
      for( int l=(int)VFrom[2]; l <= (int)VTo[2]; l++ )  {
        smatd tmp_mat = GetUnitCell().GetMatrix(i);
        tmp_mat.t[0] += j;  tmp_mat.t[1] += k;  tmp_mat.t[2] += l;
        if( !vec3d::IsInRangeInc(tmp_mat * Center, MFrom, MTo) )
          continue;
        Result.Add(new smatd(tmp_mat))->SetId((uint8_t)i, j, k, l);  // set Tag to identify the matrix (and ellipsoids) in the UnitCell
      }
  }

// Analysis of matrixes: check if the center of gravity of the asymmetric unit
// is inside the generation volume
  for( size_t i=mstart; i < Result.Count(); i++ )  {
    if( !vec3d::IsInRangeInc(*Result[i]* Center, MFrom, MTo) )  {
      delete Result[i];
      Result[i] = NULL;
    }
  }
  Result.Pack();
  const size_t res_cnt = Result.Count();
  for( size_t i=0; i < mstart; i++ )  {  // check if al matrices are uniq
    smatd* M = Result[i];
    if( M == NULL )  continue;
    for( size_t j=mstart; j < res_cnt; j++ )  {
      smatd* M1 = Result[j];
      if( M1 == NULL )  continue;
      if( M1->GetId() == M->GetId() )  {
        delete M1;
        Result[j] = NULL;
        break;
      }
    }
  }
  Result.Pack();
  if( Result.IsEmpty() )
    Result.Add(new smatd)->I().SetId(0);
  return Result.Count();
}
//..............................................................................
size_t TLattice::GenerateMatrices(smatd_plist& Result, const vec3d& center, double rad)  {
  const TAsymmUnit& au = GetAsymmUnit();
  const vec3d cnt = au.GetOCenter(true, false);
  const size_t uc_mc = GetUnitCell().MatrixCount();
  const double qrad = rad*rad;
  const size_t mstart = Result.Count();
  for( size_t i=0; i < uc_mc; i++ )  {
    for( int j=-5; j <= 5; j++ )
      for( int k=-5; k <= 5; k++ )
        for( int l=-5; l <= 5; l++ )  {
          smatd m = GetUnitCell().GetMatrix(i);
          m.t[0] += j;  m.t[1] += k;  m.t[2] += l;
          vec3d rs = m * cnt;
          au.CellToCartesian(rs);
          if( center.QDistanceTo(rs) > qrad )  continue;
          Result.Add(new smatd(m))->SetId((uint8_t)i, j, k, l);  // set Tag to identify the matrix (and ellipsoids) in the UnitCell
        }
  }
  const size_t res_cnt = Result.Count();
  for( size_t i=0; i < mstart; i++ )  {  // check if al matrices are uniq
    smatd* m = Result[i];
    if( m == NULL )  continue;
    for( size_t j=mstart; j < res_cnt; j++ )  {
      smatd* m1 = Result[j];
      if( m1 == NULL )  continue;
      if( m1->GetId() == m->GetId() )  {
        delete m1;
        Result[j] = NULL;
        break;
      }
    }
  }
  Result.Pack();
  if( Result.IsEmpty() )
    Result.Add(new smatd )->I().SetId(0);
  return Result.Count();
}
//..............................................................................
void TLattice::GenerateBondsAndFragments(TArrayList<vec3d> *ocrd)  {
  // treat detached and the rest of atoms separately
  size_t dac = 0;
  const size_t ac = Atoms.Count();
  if( ocrd != NULL )  {
    for( size_t i=0; i < ac; i++ )  {
      (*ocrd)[i] = Atoms[i]->crd();
      GetAsymmUnit().CellToCartesian(Atoms[i]->ccrd(), Atoms[i]->crd());
      if( !Atoms[i]->CAtom().IsAvailable() )
        dac++;
    }
  }
  else  {
    for( size_t i=0; i < ac; i++ )  {
      if( !Atoms[i]->CAtom().IsAvailable() )
        dac++;
    }
  }
  TSAtomPList atoms(ac-dac);
  dac = 0;
  for( size_t i=0; i < ac; i++ )  {
    if( !Atoms[i]->CAtom().IsAvailable() )  {
      dac++;
      Atoms[i]->SetNetwork(*Network);
    }
    else
      atoms[i-dac] = Atoms[i];
  }
  Network->Disassemble(atoms, Fragments, Bonds);
  dac = 0;
  for( size_t i=0; i < ac; i++ )  {
    if( Atoms[i]->IsDeleted() )  {
      delete Atoms[i];
      Atoms[i] = NULL;
      dac++;
      continue;
    }
    if( ocrd != NULL )
      Atoms[i]->crd() = (*ocrd)[i];
    Atoms[i]->SetLattId(i-dac);
  }
  if( dac != 0 )
    Atoms.Pack();
}
//..............................................................................
void TLattice::InitBody()  {
  if( !ApplyGrowInfo() )  {
    // create identity matrix
    Matrices.Add(new smatd(GetUnitCell().GetMatrix(0)))->SetId(0);
    ListAsymmUnit(Atoms, NULL, true);
  }
  OnDisassemble.Enter(this);
  GenerateBondsAndFragments(NULL);
  Fragments.QuickSorter.SortSF(Fragments, TLattice_SortFragments);
  TNetPList::QuickSorter.SortSF(Fragments, CompareFragmentsBySize);
  // precalculate memory usage
  size_t bondCnt = Bonds.Count();
  for( size_t i=0; i < Fragments.Count(); i++ )
    bondCnt += Fragments[i]->BondCount();
  Bonds.SetCapacity(bondCnt + 1);
  // end
  for( size_t i=0; i < Fragments.Count(); i++ )  {
    TNetwork* Frag = Fragments[i];
    for( size_t j=0; j < Frag->BondCount(); j++ )
      AddSBond(&Frag->Bond(j));
    for( size_t j=0; j < Frag->NodeCount(); j++ )  {
      TSAtom& SAtom = Frag->Node(j);
      TCAtom& CAtom = SAtom.CAtom();
      CAtom.SetFragmentId((uint32_t)i);
    }
  }
  TSAtomPList::QuickSorter.SortSF(Atoms, TLattice_SortAtomsById);
  OnDisassemble.Exit(this);
}
void TLattice::Init()  {
  Clear(false);
  GetUnitCell().ClearEllipsoids();
  GetUnitCell().InitMatrices();
  Generated = false;
  GetUnitCell().FindSymmEq(0.1); // find and remove
  InitBody();
}
//..............................................................................
void  TLattice::Uniq(bool remEqv)  {
  OnStructureUniq.Enter(this);
  Clear(false);
  ClearMatrices();
  GetUnitCell().UpdateEllipsoids();  // if new atoms are created...
  GetUnitCell().FindSymmEq(0.1); // find and remove
  InitBody();
  Generated = false;
  OnStructureUniq.Exit(this);
}
//..............................................................................
void TLattice::GenerateAtoms(const TSAtomPList& atoms, TSAtomPList& result, const smatd_plist& matrices)  {
  if( atoms.IsEmpty() )  return;
  result.SetCapacity(result.Count() + matrices.Count()*atoms.Count());
  for( size_t i=0; i < matrices.Count(); i++ )  {
    smatd* M = matrices[i];
    for( size_t j=0; j < atoms.Count(); j++ )  {
      if( atoms[j]->IsDeleted() )  continue;
      TSAtom* A = new TSAtom( Network );
      A->CAtom( atoms[j]->CAtom() );
      A->ccrd() = *M * A->ccrd();
      GetAsymmUnit().CellToCartesian( A->ccrd(), A->crd() );
      A->SetEllipsoid(&GetUnitCell().GetEllipsoid(M->GetContainerId(), atoms[j]->CAtom().GetId()));
      A->AddMatrix(M);
      result.Add(A);
    }
  }
}
//..............................................................................
void TLattice::GenerateWholeContent(TCAtomPList* Template)  {
  OnStructureGrow.Enter(this);
  Generated = false; // force the procedure
  Generate(Template, false, true);
  OnStructureGrow.Exit(this);
}
//..............................................................................
void  TLattice::Generate(TCAtomPList* Template, bool ClearCont, bool IncludeQ)  {
  if( ClearCont && Template)  {
    ClearAtoms();
  }
  else  {
    for( size_t i=0; i < Atoms.Count(); i++ )  {  // restore atom coordinates
      TSAtom* A = Atoms[i];
      if( A->IsDeleted() )  {
        delete A;
        Atoms[i] = NULL;
        continue;
      }
      GetAsymmUnit().CellToCartesian(A->ccrd(), A->crd());
    }
  }
  Atoms.Pack();
  TSAtomPList AtomsList;
  ListAsymmUnit(AtomsList, Template, IncludeQ);
  GenerateAtoms(AtomsList, Atoms, Matrices);
  for( size_t i=0; i < AtomsList.Count(); i++ )
    delete AtomsList[i];

  Disassemble();
  Generated = true;
}
//..............................................................................
void TLattice::GenerateCell(bool includeQ)  {
  ClearAtoms();
  ClearMatrices();
  OnStructureGrow.Enter(this);
  const TUnitCell& uc = GetUnitCell();
  TAsymmUnit& au = GetAsymmUnit();
  for( size_t i=0; i < uc.MatrixCount(); i++ )  {
    const smatd& m = uc.GetMatrix(i);
    for( size_t j=0; j < au.AtomCount(); j++ )  {
      TCAtom& ca = au.GetAtom(j);
      if( ca.IsDeleted() )  continue;
      TSAtom* sa = new TSAtom(Network);
      sa->CAtom(ca);
      sa->ccrd() = m*ca.ccrd();
      vec3i t;
      for( int k=0; k < 3; k++ )  {
        while( sa->ccrd()[k] < 0 )  {
          sa->ccrd()[k] += 1;
          t[k] += 1;
        }
        while( sa->ccrd()[k] >= 1 )  {
          sa->ccrd()[k] -= 1;
          t[k] -= 1;
        }
      }
      const uint32_t m_id = smatd::GenerateId((uint8_t)i, t);
      smatd* lm = NULL;
      for( size_t k=0; k < Matrices.Count(); k++ )  {
        if( Matrices[k]->GetId() == m_id )  {
          lm = Matrices[k];
          break;
        }
      }
      if( lm == NULL )  {
        lm = Matrices.Add(new smatd(m));
        lm->t += t;
        lm->SetRawId(m_id);
      }
      au.CellToCartesian(sa->ccrd(), sa->crd());
      sa->SetEllipsoid(&GetUnitCell().GetEllipsoid(m.GetContainerId(), ca.GetId()));
      sa->AddMatrix(lm);
      AddSAtom(sa);
    }
  }
  Atoms.QuickSorter.SortSF(Atoms, TLattice_AtomsSortByDistance );
  const size_t lc = Atoms.Count();
  float* distances = new float[lc+1];
  for( size_t i=0; i < lc; i++ )
    distances[i] = (float)Atoms[i]->crd().QLength();    
  for( size_t i=0; i < lc; i++ )  {
    if( Atoms[i] == NULL )  continue;
    for( size_t j=i+1; j < lc; j++ )  {
      if( Atoms[j] == NULL )  continue;
      if( (distances[j] - distances[i]) > 0.1 )  break;
      const double qd = Atoms[i]->crd().QDistanceTo(Atoms[j]->crd());
      if( qd < 0.00001 )  {
        Atoms[i]->AddMatrices(Atoms[j]);
        delete Atoms[j];
        Atoms[j] = NULL;
        continue;
      }
    }
  }
  delete [] distances;
  Atoms.Pack();

  Disassemble();
  Generated = true;
  OnStructureGrow.Exit(this);
}
//..............................................................................
void TLattice::Generate(const vec3d& MFrom, const vec3d& MTo, TCAtomPList* Template,
       bool ClearCont, bool IncludeQ)  
{
  vec3d VTo(olx_round(MTo[0]+1), olx_round(MTo[1]+1), olx_round(MTo[2]+1));
  vec3d VFrom(olx_round(MFrom[0]-1), olx_round(MFrom[1]-1), olx_round(MFrom[2]-1));
  if( ClearCont )  {
    ClearAtoms();
    ClearMatrices();
  }
  GenerateMatrices(VFrom, VTo, MFrom, MTo);
  OnStructureGrow.Enter(this);
  Generate(Template, ClearCont, IncludeQ);
  OnStructureGrow.Exit(this);
}
//..............................................................................
void TLattice::Generate(const vec3d& center, double rad, TCAtomPList* Template,
       bool ClearCont, bool IncludeQ)
{
  if( ClearCont )  {
    ClearAtoms();
    ClearMatrices();
  }
  GenerateMatrices(Matrices, center, rad);
  OnStructureGrow.Enter(this);
  Generate(Template, ClearCont, IncludeQ);
  OnStructureGrow.Exit(this);
}
//..............................................................................
bool TLattice::IsExpandable(TSAtom& A) const {
  return (A.CAtom().IsGrowable() && !A.IsGrown());
}
//..............................................................................
void TLattice::GetGrowMatrices(smatd_list& res) const {
  for( size_t i=0; i < Atoms.Count(); i++ )  {
    if( Atoms[i]->IsGrown() || !Atoms[i]->IsAvailable() )  continue;
    const TCAtom& ca = Atoms[i]->CAtom();
    for( size_t j=0; j < ca.AttachedAtomCount(); j++ )  {
      const TCAtom& ca1 = ca.GetAttachedAtom(j);
      if( !ca1.IsAvailable() )  continue;
      smatd_list* BindingMatrices = GetUnitCell().GetBinding(ca, ca1, Atoms[i]->ccrd(), ca1.ccrd(), false, false);
      for( size_t k=0; k < BindingMatrices->Count(); k++ )  {
        const smatd& M = BindingMatrices->Item(k);
        bool found = false;
        for( size_t l=0; l < MatrixCount(); l++ )  {
          if( *Matrices[l] == M )  {
            found = true;  
            break;
          }
        }
        if( !found && res.IndexOf(M) == InvalidIndex )
          res.AddCCopy(M);
      }
      delete BindingMatrices;
    }
  }
}
//..............................................................................
void TLattice::DoGrow(const TSAtomPList& atoms, bool GrowShell, TCAtomPList* Template)  {
  // all matrices after MatrixCount are new and has to be used for generation
  size_t currentCount = MatrixCount();
  // the fragmens to grow by a particular matrix
  TTypeList<TIntList> Fragments2Grow;
  OnStructureGrow.Enter(this);
  for( size_t i=0; i < atoms.Count(); i++ )  {
    TSAtom* SA = atoms[i];
    SA->SetGrown(true);
    const TCAtom& CA = SA->CAtom();
    for(size_t j=0; j < CA.AttachedAtomCount(); j++ )  {
      const TCAtom& CA1 = CA.GetAttachedAtom(j);
      if( !CA1.IsAvailable() )  
        continue;
      smatd_list* BindingMatrices = GetUnitCell().GetBinding(CA, CA1, SA->ccrd(), CA1.ccrd(), false, false);
      for( size_t k=0; k < BindingMatrices->Count(); k++ )  {
        const smatd& M = BindingMatrices->Item(k);
        bool found = false;
        size_t l; // need to use it later
        for( l=0; l < MatrixCount(); l++ )  {
          if( *Matrices[l] == M )  {
            found = true;  
            break;
          }
        }
        if( !found )  {
          Matrices.Add( new smatd(M) );
          Fragments2Grow.Add( new TIntList ).Add( CA1.GetFragmentId() );
        }
        else  {
          if( l >= currentCount )  {
            TIntList* ToGrow = &Fragments2Grow[l-currentCount];
            if( ToGrow->IndexOf(CA1.GetFragmentId()) == InvalidIndex )
              ToGrow->Add( CA1.GetFragmentId() );
          }
        }
      }
      delete BindingMatrices;
    }
  }
  for( size_t i = currentCount; i < MatrixCount(); i++ )  {
    smatd* M = Matrices[i];
    TIntList& ToGrow = Fragments2Grow[i-currentCount];
    for( size_t j=0; j < GetAsymmUnit().AtomCount(); j++ )  {
      TCAtom& ca = GetAsymmUnit().GetAtom(j);
      for( size_t k=0; k < ToGrow.Count(); k++ )  {
        if( !ca.IsAvailable() )  continue;
        if( ca.GetFragmentId() == ToGrow[k] )  {
          TSAtom* SA = new TSAtom( Network );
          SA->CAtom( ca );
          SA->AddMatrix(M);
          SA->ccrd() = *M * SA->ccrd();
          SA->SetEllipsoid(&GetUnitCell().GetEllipsoid(M->GetContainerId(), SA->CAtom().GetId()));
          AddSAtom(SA);
        }
      }
    }
  }

  RestoreCoordinates();
  Disassemble();

  Generated = true;
  OnStructureGrow.Exit(this);
}
//..............................................................................
void TLattice::GrowFragments(bool GrowShells, TCAtomPList* Template)  {
  TSAtomPList TmpAtoms;
  for( size_t i=0; i < Atoms.Count(); i++ )  {
    TSAtom* A = Atoms[i];
    if( A->IsDeleted() || !A->CAtom().IsAvailable() )  
      continue;
    for( size_t j=0; j < A->NodeCount(); j++ )  {
      if( A->Node(j).IsDeleted() )
        A->NullNode(j);
    }
    A->PackNodes();
    if( IsExpandable(*A) )
      TmpAtoms.Add(A);
  }
  if( !TmpAtoms.IsEmpty() )
    GrowAtoms(TmpAtoms, GrowShells, Template);
}
//..............................................................................
void TLattice::GrowAtoms(const TSAtomPList& atoms, bool GrowShells, TCAtomPList* Template)  {
  if( atoms.IsEmpty() )  return;
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
void TLattice::GrowAtom(uint32_t FragId, const smatd& transform)  {
  smatd *M = NULL;
  // check if the matix is unique
  bool found = false;
  for( size_t i=0; i < Matrices.Count(); i++ )  {
    if( *Matrices[i] == transform )  {
      M = Matrices[i];
      found = true;
      break;
    }
  }
  if( !found )
    M = Matrices.Add(new smatd(transform));

  OnStructureGrow.Enter(this);

  for( size_t i=0; i < GetAsymmUnit().AtomCount(); i++ )  {
    TCAtom& ca = GetAsymmUnit().GetAtom(i);
    if( !ca.IsAvailable() )  continue;
    if( ca.GetFragmentId() == FragId )  {
      TSAtom* SA = new TSAtom(Network);
      SA->CAtom(ca);
      SA->AddMatrix(M);
      SA->ccrd() = (*M) * SA->ccrd();
      SA->SetEllipsoid(&GetUnitCell().GetEllipsoid(M->GetContainerId(), SA->CAtom().GetId()));
      AddSAtom(SA);
    }
  }
  RestoreCoordinates();
  Disassemble();

  Generated = true;
  OnStructureGrow.Exit(this);
}
//..............................................................................
void TLattice::GrowAtoms(const TSAtomPList& atoms, const smatd_list& matrices)  {
  smatd *M = NULL;
  smatd_plist addedMatrices;
  // check if the matices is unique
  for( size_t i=0; i < matrices.Count(); i++ )  {
    bool found = false;
    for( size_t j=0; j < Matrices.Count(); j++ )  {
      if( *Matrices[j] == matrices[i] )  {
        found = true;
        break;
      }
    }
    if( !found )  {
      M = new smatd(matrices[i]);
      Matrices.Add(M);
      addedMatrices.Add(M);
      this->GetUnitCell().InitMatrixId(*M);
    }
  }

  OnStructureGrow.Enter(this);
  Atoms.SetCapacity( Atoms.Count() + atoms.Count()*addedMatrices.Count() );
  for( size_t i=0; i < addedMatrices.Count(); i++ )  {
    for( size_t j=0; j < atoms.Count(); j++ )  {
      if( atoms[j]->IsDeleted() || !atoms[j]->CAtom().IsAvailable() )  continue;
      TSAtom* SA = new TSAtom( Network );
      SA->CAtom(atoms[j]->CAtom());
      SA->AddMatrix(addedMatrices[i]);
      SA->ccrd() = *addedMatrices[i] * SA->ccrd();
      SA->SetEllipsoid(&GetUnitCell().GetEllipsoid(M->GetContainerId(), SA->CAtom().GetId()));
      AddSAtom(SA);
    }
  }
  RestoreCoordinates();
  Disassemble();

  Generated = true;
  OnStructureGrow.Exit(this);
}
//..............................................................................
void TLattice::Grow(const smatd& transform)  {
  smatd *M = NULL;
  // check if the matix is unique
  bool found = false;
  for( size_t i=0; i < Matrices.Count(); i++ )  {
    if( *Matrices[i] == transform )  {
      M = Matrices[i];
      found = true;
      break;
    }
  }
  if( !found )  {
    M = new smatd(transform);
    Matrices.Add(M);
  }

  OnStructureGrow.Enter(this);
  TAsymmUnit& au = GetAsymmUnit();
  const size_t ac = au.AtomCount();
  Atoms.SetCapacity(Atoms.Count() + ac);
  for( size_t i=0; i < ac; i++ )  {
    TCAtom& ca = au.GetAtom(i);
    if( ca.IsAvailable() )  continue;
    TSAtom* SA = new TSAtom(Network);
    SA->CAtom(ca);
    SA->AddMatrix(M);
    SA->ccrd() = (*M) * SA->ccrd();
    SA->SetEllipsoid(&GetUnitCell().GetEllipsoid(M->GetContainerId(), SA->CAtom().GetId()));
    AddSAtom(SA);
  }
  RestoreCoordinates();
  Disassemble();

  Generated = true;
  OnStructureGrow.Exit(this);
}
//..............................................................................
void TLattice::RestoreAtom(const TSAtom::Ref& id)  {
  if( smatd::GetContainerId(id.matrix_id) >= GetUnitCell().MatrixCount() )
    throw TInvalidArgumentException(__OlxSourceInfo, "matrix ID");
  if( id.catom_id >= GetAsymmUnit().AtomCount() )
    throw TInvalidArgumentException(__OlxSourceInfo, "catom ID");
  smatd* matr = NULL;
  for( size_t i=0; i < Matrices.Count(); i++ )  {
    if( Matrices[i]->GetId() == id.matrix_id )  {
      matr = Matrices[i];
      break;
    }
  }
  if( matr == NULL )  {
    matr = Matrices.Add( new smatd(smatd::FromId(id.matrix_id, 
      GetUnitCell().GetMatrix(smatd::GetContainerId(id.matrix_id)))) );
    Generated = true;
  }
  TSAtom* sa = new TSAtom(Network);
  sa->CAtom( GetAsymmUnit().GetAtom(id.catom_id) );
  sa->CAtom().SetDeleted(false);
  sa->ccrd() = (*matr) * sa->ccrd();
  sa->AddMatrix(matr);
  GetAsymmUnit().CellToCartesian(sa->ccrd(), sa->crd());
  sa->SetEllipsoid(&GetUnitCell().GetEllipsoid(matr->GetContainerId(), id.catom_id));
  AddSAtom(sa);
  if( id.matrices != NULL )  {
    for( size_t i=0; i < id.matrices->Count(); i++ )  {
      if( smatd::GetContainerId((*id.matrices)[i]) >= GetUnitCell().MatrixCount() )
        throw TInvalidArgumentException(__OlxSourceInfo, "matrix ID");
      matr = NULL;
      for( size_t j=0; j < Matrices.Count(); j++ )  {
        if( Matrices[j]->GetId() == (*id.matrices)[i] )  {
          matr = Matrices[j];
          break;
        }
      }
      if( matr == NULL )  {
        matr = Matrices.Add(new smatd(smatd::FromId((*id.matrices)[i],
          GetUnitCell().GetMatrix(smatd::GetContainerId((*id.matrices)[i])))));
        Generated = true;
      }
      sa->AddMatrix(matr);
    }
  }
}
//..............................................................................
TSAtom* TLattice::FindSAtom(const olxstr& Label) const {
  for( size_t i=0; i < Atoms.Count(); i++ )
    if( Label.Equalsi( Atoms[i]->GetLabel()) )  
      return Atoms[i];
  return NULL;
}
//..............................................................................
TSAtom* TLattice::FindSAtom(const TCAtom& ca) const {
  for( size_t i=0; i < Atoms.Count(); i++ )
    if( ca.GetId() == Atoms[i]->CAtom().GetId() )  
      return Atoms[i];
  return NULL;
}
//..............................................................................
TSAtom* TLattice::NewCentroid(const TSAtomPList& Atoms)  {
  vec3d cc, ce;
  double aan = 0;
  for( size_t i=0; i < Atoms.Count(); i++ )  {
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
  TSAtom* Centroid = new TSAtom( Network );
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
  //TODO: need to consider occupancy for disordered groups ...
  TTypeList<AnAssociation2<TSAtom*, double> > Points;
  Points.SetCapacity(atoms.Count());
  if( weightExtent != 0 )  {
    double swg = 0;
    for( size_t i=0; i < atoms.Count(); i++ )  {
      double wght = pow(atoms[i]->GetType().GetMr(), (double)weightExtent);
      Points.AddNew(atoms[i], wght);
      swg += wght*wght;
    }
    for( size_t i=0; i < Points.Count(); i++ )
      Points[i].B() /= swg;
  }
  else  {
    for( size_t i=0; i < atoms.Count(); i++ )
      Points.AddNew(atoms[i], 1);
  }

  TSPlane* Plane = new TSPlane(Network);
  Plane->Init(Points);
  return Plane;
}
//..............................................................................
void TLattice::UpdateAsymmUnit()  {
  if( Atoms.IsEmpty() )  return;
  const size_t ac = GetAsymmUnit().AtomCount();
  TArrayList<TSAtomPList> AUAtoms(ac);
  TSizeList del_cnt(ac);
  for( size_t i=0; i < ac; i++ )
    del_cnt[i] = 0;
  const size_t lat_ac = Atoms.Count();
  for( size_t i=0; i < lat_ac; i++ )  {
    if( Atoms[i]->IsDeleted() )  {
      del_cnt[Atoms[i]->CAtom().GetId()]++;
      continue;
    }
    AUAtoms[Atoms[i]->CAtom().GetId()].Add(Atoms[i]);
  }
  for( size_t i=0; i < ac; i++ )  {  // create lists to store atom groups
    TSAtomPList& l = AUAtoms[i];
    if( del_cnt[i] == 0 && (l.Count() != 1) )  continue;  // nothing to do
    TCAtom& ca = AsymmUnit->GetAtom(i);
    if( l.IsEmpty() )  {  // all atoms are deleted
      if( !ca.IsDeleted() )
        ca.SetDeleted(ca.IsAvailable());
      continue;
    }
    else if( l.Count() == 1 )  {  // special case...
      if( l[0]->GetMatrix(0).IsFirst() )  continue;
      if( l[0]->GetEllipsoid() )
        ca.UpdateEllp(*l[0]->GetEllipsoid());
      ca.ccrd() = l[0]->ccrd();
      continue;
    }
    // find the original atom, or symmetry equivalent if removed
    TSAtom* OA = NULL;
    const size_t lst_c = l.Count();
    for( size_t j=0; j < lst_c; j++ )  {
      TSAtom* A = l[j];
      const size_t am_c = A->MatrixCount();
      for( size_t k=0; k < am_c; k++ )  {
        const smatd& m = A->GetMatrix(k);
        if( m.IsFirst() )  {  // the original atom
          OA = A;  
          break; 
        }
        if( OA != NULL )  break;
      }
    }
    if( OA == NULL )
      OA = l[0];
    ca.SetDeleted(false);
    if( OA->GetEllipsoid() )
      ca.UpdateEllp(*OA->GetEllipsoid());
    ca.ccrd() = OA->ccrd();
  }
}
//..............................................................................
void TLattice::ListAsymmUnit(TSAtomPList& L, TCAtomPList* Template, bool IncludeQ)  {
  ClearPlanes();
  if( Template != NULL )  {
    L.SetCapacity( L.Count() + Template->Count() );
    for( size_t i=0; i < Template->Count(); i++ )  {
      if( (*Template)[i]->IsDeleted() )  continue;
      TSAtom* A = L.Add(new TSAtom(Network));
      A->CAtom( *Template->Item(i) );
      A->SetEllipsoid(&GetUnitCell().GetEllipsoid(0, Template->Item(i)->GetId())); // ellipsoid for the identity matrix
      A->AddMatrix(Matrices[0]);
      A->SetLattId(L.Count()-1);
    }
  }
  else  {
    L.SetCapacity(L.Count() + GetAsymmUnit().AtomCount());
    for( size_t i=0; i < GetAsymmUnit().AtomCount(); i++ )    {
      TCAtom& CA = GetAsymmUnit().GetAtom(i);
      if( CA.IsDeleted() )  continue;
      if( !IncludeQ && CA.GetType() == iQPeakZ )  continue;
      TSAtom* A = L.Add(new TSAtom(Network));
      A->CAtom(CA);
      A->SetEllipsoid(&GetUnitCell().GetEllipsoid(0, CA.GetId())); // ellipsoid for the identity matrix
      A->AddMatrix(Matrices[0]);
      A->SetLattId(L.Count()-1);
    }
  }
}
//..............................................................................
void TLattice::MoveFragment(const vec3d& to, TSAtom& fragAtom)  {
  if( Generated )  {
    TBasicApp::GetLog().Error("Cannot perform this operation on grown structure");
    return;
  }
  smatd* m = GetUnitCell().GetClosest(to, fragAtom.ccrd(), true);
  if( m != NULL )  {
    for( size_t i=0; i < fragAtom.GetNetwork().NodeCount(); i++ )  {
      TSAtom& SA = fragAtom.GetNetwork().Node(i);
      SA.CAtom().ccrd() = *m * SA.CAtom().ccrd();
      if( SA.CAtom().GetEllipsoid() != NULL )
        *SA.CAtom().GetEllipsoid() = GetUnitCell().GetEllipsoid(m->GetContainerId(), SA.CAtom().GetId());
    }
    delete m;
    GetUnitCell().UpdateEllipsoids();
    Uniq();
  }
  else
    TBasicApp::GetLog().Info("Could not find closest matrix");
}
//..............................................................................
void TLattice::MoveFragment(TSAtom& to, TSAtom& fragAtom)  {
  if( Generated )  {
    TBasicApp::GetLog().Error("Cannot perform this operation on grown structure");
    return;
  }
  smatd* m = GetUnitCell().GetClosest(to.CAtom(), fragAtom.CAtom(), true);  
  if( m != NULL )  {
    if( to.CAtom().GetFragmentId() == fragAtom.CAtom().GetFragmentId() )  {
      fragAtom.CAtom().ccrd() = *m * fragAtom.CAtom().ccrd();
      if( fragAtom.CAtom().GetEllipsoid() != NULL )
        *fragAtom.CAtom().GetEllipsoid() = GetUnitCell().GetEllipsoid(m->GetContainerId(), fragAtom.CAtom().GetId());
    }
    else  {  // move whole fragment then
      uint32_t fragId = fragAtom.CAtom().GetFragmentId();
      for( size_t i=0; i < Atoms.Count(); i++ )  {
        TSAtom* SA = Atoms[i];
        if( SA->CAtom().GetFragmentId() == fragId )  {
          SA->CAtom().ccrd() = *m * SA->CAtom().ccrd();
          if( SA->CAtom().GetEllipsoid() != NULL ) 
            *SA->CAtom().GetEllipsoid() = GetUnitCell().GetEllipsoid(m->GetContainerId(), SA->CAtom().GetId());
        }
      }
    }
    delete m;
    GetUnitCell().UpdateEllipsoids();
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
    OnStructureGrow.Enter(this);
    Matrices.Add(m);
    for( size_t i=0; i < fragAtom.GetNetwork().NodeCount(); i++ )  {
      TSAtom& SA = fragAtom.GetNetwork().Node(i);
      if( SA.IsDeleted() )  continue;
      TSAtom* atom = new TSAtom( &SA.GetNetwork() );
      atom->CAtom( SA.CAtom() );
      atom->AddMatrix(m);
      atom->ccrd() = SA.ccrd();
      atom->ccrd() = (*m) * atom->ccrd();
      GetAsymmUnit().CellToCartesian(atom->ccrd(), atom->crd());
      atom->SetEllipsoid(&GetUnitCell().GetEllipsoid(m->GetContainerId(), atom->CAtom().GetId()));
      AddSAtom(atom);
    }
    Disassemble();
    OnStructureGrow.Exit(this);
  }
  else
    TBasicApp::GetLog().Info("Could not find closest matrix");
}
//..............................................................................
void TLattice::MoveFragmentG(TSAtom& to, TSAtom& fragAtom)  {
  smatd* m = GetUnitCell().GetClosest(to.ccrd(), fragAtom.ccrd(), true);
  if( m != NULL )  {
/* restore atom centres if were changed by some other procedure */
    RestoreCoordinates();
    Generated = true;
    OnStructureGrow.Enter(this);
    Matrices.Add(m);
    TSAtomPList atoms;
    if( to.CAtom().GetFragmentId() == fragAtom.CAtom().GetFragmentId() )
      atoms.Add(&fragAtom);
    else  // copy whole fragment then
      for( size_t i=0; i < fragAtom.GetNetwork().NodeCount(); i++ )
        atoms.Add(&fragAtom.GetNetwork().Node(i));

    for( size_t i=0; i < atoms.Count(); i++ )  {
      TSAtom* SA = atoms.Item(i);
      if( SA->IsDeleted() )  continue;
      TSAtom* atom = new TSAtom(&SA->GetNetwork());
      atom->CAtom(SA->CAtom());
      atom->AddMatrix(m);
      atom->ccrd() = SA->ccrd();
      atom->ccrd() = (*m) * atom->ccrd();
      GetAsymmUnit().CellToCartesian(atom->ccrd(), atom->crd());
      atom->SetEllipsoid(&GetUnitCell().GetEllipsoid(m->GetContainerId(), atom->CAtom().GetId()));
      AddSAtom(atom);
    }
    Disassemble();
    OnStructureGrow.Exit(this);
  }
  else
    TBasicApp::GetLog().Info("Could not find closest matrix");
}
//..............................................................................
void TLattice::MoveToCenter()  {
  if( Generated )  {
    TBasicApp::GetLog().Error("Please note that asymetric unit will not be updated: the structure is grown");
    OnStructureGrow.Enter(this);
  }
  for( size_t i=0; i < Fragments.Count(); i++ )  {
    TNetwork* frag = Fragments[i];
    vec3d molCenter;
    size_t ac = 0;
    for( size_t j=0; j < frag->NodeCount(); j++ )  {
      if( frag->Node(j).IsDeleted() )  continue;
      molCenter += frag->Node(j).ccrd();
      ac++;
    }
    if( ac == 0 )  continue;
    molCenter /= ac;
    smatd* m = GetUnitCell().GetClosest(vec3d(0.5, 0.5, 0.5), molCenter, true);
    if( m == NULL )  continue;
    if( Generated )  {
      for( size_t j=0; j < frag->NodeCount(); j++ )  {
        TSAtom& SA = frag->Node(j);
        SA.ccrd() = *m * SA.ccrd();
        SA.SetEllipsoid(&GetUnitCell().GetEllipsoid(m->GetContainerId(), SA.CAtom().GetId()));
      }
    }
    else  {
      for( size_t j=0; j < frag->NodeCount(); j++ )  {
        TSAtom& SA = frag->Node(j);
        SA.CAtom().ccrd() = *m * SA.CAtom().ccrd();
        if( SA.CAtom().GetEllipsoid() != NULL ) 
          *SA.CAtom().GetEllipsoid() = GetUnitCell().GetEllipsoid(m->GetContainerId(), SA.CAtom().GetId());
      }
    }
    delete m;
  }
  if( !Generated )  {  
    OnStructureUniq.Enter(this);
    Init();
    OnStructureUniq.Exit(this);
  }
  else  {
    RestoreCoordinates();
    Disassemble();
    OnStructureGrow.Exit(this);
  }
}
//..............................................................................
void TLattice::Compaq()  {
  if( Generated || Fragments.Count() < 2 )  return;
  TNetwork* frag = Fragments[0];
  vec3d acenter;
  size_t ac = 0;
  for( size_t i=0; i < frag->NodeCount(); i++ )  {
    if( frag->Node(i).IsDeleted() )  continue;
    acenter += frag->Node(i).ccrd();
    ac++;
  }
  if( ac == 0 )  return;
  acenter /= ac;
  for( size_t i=1; i < Fragments.Count(); i++ )  {
    frag = Fragments[i];
    smatd* m = NULL;

    for( size_t j=0; j < Fragments[0]->NodeCount(); j++ )  {
      TSAtom& fa = Fragments[0]->Node(j);
      for( size_t k=0; k < frag->NodeCount(); k++ )  {
        if( frag->Node(k).CAtom().IsAttachedTo( fa.CAtom() ) )  {
          m = GetUnitCell().GetClosest(fa.ccrd(), frag->Node(k).ccrd(), true);
          if( m != NULL )  break;
        }
      }
      if( m != NULL )  break;
    }
    if( m == NULL )  {
      vec3d molCenter;
      ac = 0;
      for( size_t j=0; j < frag->NodeCount(); j++ )  {
        if( frag->Node(j).IsDeleted() )  continue;
        molCenter += frag->Node(j).ccrd();
        ac++;
      }
      if( ac == 0 )  continue;
      molCenter /= ac;
      m = GetUnitCell().GetClosest(acenter, molCenter, true);
    }
    if( m != NULL )  {
      for( size_t j=0; j < frag->NodeCount(); j++ )  {
        TSAtom& SA = frag->Node(j);
        if( SA.IsDeleted() )  continue;
        SA.CAtom().ccrd() = *m * SA.CAtom().ccrd();
        if( SA.CAtom().GetEllipsoid() != NULL ) 
          *SA.CAtom().GetEllipsoid() = GetUnitCell().GetEllipsoid(m->GetContainerId(), SA.CAtom().GetId());
      }
      delete m;
    }
  }
  OnStructureUniq.Enter(this);
  Init();
  OnStructureUniq.Exit(this);
}
//..............................................................................
void TLattice::CompaqAll()  {
  if( Generated || Fragments.Count() < 2 )  return;
  OnStructureUniq.Enter(this);
  OnDisassemble.SetEnabled(false);
  bool changes = true;
  size_t itr = 0;
  while( changes )  {
    changes = false;
    for( size_t i=0; i < Fragments.Count(); i++ )  {
      for( size_t j=i+1; j < Fragments.Count(); j++ )  {
        smatd* m = NULL;
        for( size_t k=0; k < Fragments[i]->NodeCount(); k++ )  {
          const TSAtom& fa = Fragments[i]->Node(k);
          for( size_t l=0; l < Fragments[j]->NodeCount(); l++ )  {
            if( Fragments[j]->Node(l).CAtom().IsAttachedTo(fa.CAtom()) )  {
              m = GetUnitCell().GetClosest(fa.CAtom(), Fragments[j]->Node(l).CAtom(), false);
              if( m != NULL )
                break;
            }
          }
          if( m != NULL )  break;
        }
        if( m == NULL )  continue;
        changes = true;
        for( size_t k=0; k < Fragments[j]->NodeCount(); k++ )  {
          TSAtom& SA = Fragments[j]->Node(k);
          if( SA.IsDeleted() )  continue;
          SA.CAtom().ccrd() = *m * SA.CAtom().ccrd();
          if( SA.CAtom().GetEllipsoid() != NULL )
            *SA.CAtom().GetEllipsoid() = GetUnitCell().GetEllipsoid(m->GetContainerId(), SA.CAtom().GetId());
        }
        delete m;
      }
      Init();
      if( ++itr > 100 )  {
        TBasicApp::GetLog().Error(olxstr("The procedure:\n")  << __OlxSourceInfo <<
          "\nhas failed due to large number of iterations\nPlease contact the developers team");
        changes = false;
        break;
      }
    }
  }
  OnDisassemble.SetEnabled(true);
  Init();
  OnStructureUniq.Exit(this);
}
//..............................................................................
void TLattice::CompaqClosest()  {
  if( Generated || Fragments.Count() < 2 )  return;
  const size_t fr_cnt = Fragments.Count();
  TDoubleList vminQD(fr_cnt);
  for( size_t i = 0; i < fr_cnt; i++ )
    vminQD[i] = 100000;
  for( size_t fi = 0; fi < fr_cnt; fi++ )  {
    TNetwork* neta = Fragments[fi];
    const size_t neta_cnt = neta->NodeCount();
    for( size_t i=fi+1; i < fr_cnt; i++ )  {
      //for( size_t j=0; j < fr_cnt; j++ )
      //  TBasicApp::GetLog() << olxstr::FormatFloat(3, vminQD[j]) << ' ';
      //TBasicApp::GetLog() << '\n';
      TNetwork* netb = Fragments[i];
      const size_t netb_cnt = netb->NodeCount();
      double minQD = 100000;
      smatd* transform = NULL;
      for( size_t j=0; j < neta_cnt; j++ )  {
        const vec3d& crda = neta->Node(j).CAtom().ccrd();
        for( size_t k=0; k < netb_cnt; k++ )  {
          const vec3d& crdb = netb->Node(k).CAtom().ccrd();
          double qd = 0;
          smatd* m = GetUnitCell().GetClosest(crda, crdb, true, &qd);
          if( m == NULL )  {
            if( qd < minQD )  { // still remember the minimal distance
              minQD = qd;
              if( transform != NULL )  {  // reset the transform as well
                delete transform;
                transform = NULL;
              }
            }
            continue;
          }
          if( qd < minQD )  {
            minQD = qd;
            if( transform != NULL )
              delete transform;
            transform = m;
          }
          else
            delete m;
        }
      }
      if( vminQD[i] <= minQD )  {
        if( transform )
          delete transform;
        continue;
      }
      vminQD[i] = minQD;
      if( transform == NULL )  continue;
      for( size_t k=0; k < netb_cnt; k++ )  {
        TSAtom& fb = netb->Node(k);
        if( fb.IsDeleted() )  continue;
        fb.CAtom().ccrd() = *transform * fb.CAtom().ccrd();
        if( fb.CAtom().GetEllipsoid() != NULL )
          *fb.CAtom().GetEllipsoid() = GetUnitCell().GetEllipsoid(transform->GetContainerId(), fb.CAtom().GetId());
      }
      // this needs to be done if any one fragment is transformed multiple times...
      GetUnitCell().UpdateEllipsoids();
      delete transform;
    }
  }
  OnStructureUniq.Enter(this);
  Init();
  OnStructureUniq.Exit(this);
}
//..............................................................................
void TLattice::CompaqQ()  {
  if( Generated )  return;
  OnStructureUniq.Enter(this);
  const size_t ac = Atoms.Count();
  for( size_t i=0; i < ac; i++ )  {
    if( Atoms[i]->GetType() != iQPeakZ )  continue;
    const vec3d& crda = Atoms[i]->ccrd();
    smatd* transform = NULL;
    double minQD = 1000;
    for( size_t j=0; j < ac; j++ )  {
      if( Atoms[j]->GetType() == iQPeakZ )  continue;
      double qd = 0;
      smatd* m = GetUnitCell().GetClosest(Atoms[j]->ccrd(), crda, true, &qd);
      if( qd < minQD )  {
        if( transform != NULL )  delete transform;
        transform = m;
        minQD = qd;
      }
      else if( m != NULL )
        delete m;
    }
    if( transform == NULL )  continue;
    Atoms[i]->CAtom().ccrd() = *transform * Atoms[i]->CAtom().ccrd();
    delete transform;
  }
  Init();
  OnStructureUniq.Exit(this);
}
//..............................................................................
void TLattice::TransformFragments(const TSAtomPList& fragAtoms, const smatd& transform)  {
  if( Generated )  {
    TBasicApp::GetLog().Error("Cannot perform this operation on grown structure");
    return;
  }
  // transform may come with random Tag, so need to process ADP's manually - cannot pick from UC
  const mat3d abc2xyz(mat3d::Transpose(GetAsymmUnit().GetCellToCartesian())),
              xyz2abc(mat3d::Transpose(GetAsymmUnit().GetCartesianToCell()));
  const mat3d etm = abc2xyz*transform.r*xyz2abc;
  for( size_t i=0; i < fragAtoms.Count(); i++ )
    fragAtoms[i]->GetNetwork().SetTag(i);

  for( size_t i=0; i < fragAtoms.Count(); i++ )  {
    if( fragAtoms[i]->GetNetwork().GetTag() == i )  {
      for( size_t j=0; j < fragAtoms[i]->GetNetwork().NodeCount(); j++ )  {
        TSAtom& SA = fragAtoms[i]->GetNetwork().Node(j);
        SA.CAtom().ccrd() = transform * SA.CAtom().ccrd();
        if( SA.CAtom().GetEllipsoid() != NULL ) 
          SA.CAtom().GetEllipsoid()->MultMatrix(etm);
      }
    }
  }
  OnStructureUniq.Enter(this);
  Init();
  OnStructureUniq.Exit(this);
}
//..............................................................................
void TLattice::UpdateConnectivity()  {
  Disassemble();
  UpdateAsymmUnit();
}
//..............................................................................
void TLattice::Disassemble()  {
  OnDisassemble.Enter(this);
  // clear bonds & fargments
  ClearBonds();
  ClearFragments();
  TArrayList<vec3d> ocrd(Atoms.Count());
  GenerateBondsAndFragments(&ocrd);
  Fragments.QuickSorter.SortSF(Fragments, TLattice_SortFragments);
  // precalculate memory usage
  size_t bondCnt = Bonds.Count();
  for( size_t i=0; i < Fragments.Count(); i++ )
    bondCnt += Fragments[i]->BondCount();
  Bonds.SetCapacity( bondCnt );
  // end

  for( size_t i=0; i < Fragments.Count(); i++ )  {
    TNetwork* Frag = Fragments[i];
    for( size_t j=0; j < Frag->BondCount(); j++ )
      AddSBond( &Frag->Bond(j) );
  }
  //TSAtomPList::QuickSorter.SortSF(Atoms, TLattice_SortAtomsById);
  OnDisassemble.Exit(this);
}
//..............................................................................
void TLattice::RestoreCoordinates()  {
  const size_t ac = Atoms.Count();
  for( size_t i=0; i < ac; i++ )
    GetAsymmUnit().CellToCartesian(Atoms[i]->ccrd(), Atoms[i]->crd());
}
//..............................................................................
bool TLattice::_AnalyseAtomHAdd(AConstraintGenerator& cg, TSAtom& atom, TSAtomPList& ProcessingAtoms, int part, TCAtomPList* generated)  {
  if( ProcessingAtoms.IndexOf(atom) != InvalidIndex || (atom.CAtom().IsHAttached() && part == DefNoPart) )
    return false;
  ProcessingAtoms.Add(atom);

  cm_Element& h_elm = XElementLib::GetByIndex(iHydrogenIndex);
  TAtomEnvi AE;
  UnitCell->GetAtomEnviList(atom, AE, false, part);
  if( part == DefNoPart )  {  // check for disorder
    TIntList parts;
    TDoubleList occu;
    RefinementModel* rm = GetAsymmUnit().GetRefMod();
    for( size_t i=0; i < AE.Count(); i++ )  {
      if( AE.GetCAtom(i).GetPart() != 0 && AE.GetCAtom(i).GetPart() != AE.GetBase().CAtom().GetPart() ) 
        if( parts.IndexOf(AE.GetCAtom(i).GetPart()) == InvalidIndex )  {
          parts.Add(AE.GetCAtom(i).GetPart());
          occu.Add(rm->Vars.GetParam(AE.GetCAtom(i), catom_var_name_Sof));
        }
    }
    if( !parts.IsEmpty() )  {  // here we go..
      TTypeList<TCAtomPList> gen_atoms;
      ProcessingAtoms.Remove(&atom);
      for( size_t i=0; i < parts.Count(); i++ )  {
        _AnalyseAtomHAdd(cg, atom, ProcessingAtoms, parts[i], &gen_atoms.AddNew());
        TCAtomPList& gen = gen_atoms.Last();
        for( size_t j=0; j < gen.Count(); j++ )  {
          gen[j]->SetPart(parts[i]);
          rm->Vars.SetParam(*gen[j], catom_var_name_Sof, occu[i] );
        }
      }
      cg.AnalyseMultipart(gen_atoms);
      return false;
    }
  }
  if( atom.GetType() == iCarbonZ )  {
    if( AE.Count() == 1 )  {
      // check acetilene
      double d = AE.GetCrd(0).DistanceTo(atom.crd());
      TSAtom* A = FindSAtom(AE.GetCAtom(0));
      TAtomEnvi NAE;
      if( A == 0 )
        throw TFunctionFailedException(__OlxSourceInfo, olxstr("Could not locate atom ") << AE.GetLabel(0) );

      UnitCell->GetAtomEnviList(*A, NAE, false, part);
      if( A->GetType() == iCarbonZ && NAE.Count() == 2 && d < 1.2)  {
        TBasicApp::GetLog().Info(olxstr(atom.GetLabel()) << ": XCH");
        cg.FixAtom(AE, fgCH1, h_elm, NULL, generated);
      }
      else  {
        if( d > 1.35 )  {
          TBasicApp::GetLog().Info(olxstr(atom.GetLabel()) << ": XCH3");
          cg.FixAtom(AE, fgCH3, h_elm, NULL, generated);
        }
        else  {
          if( d < 1.25 )  {
            if( NAE.Count() > 1 ) {
              TBasicApp::GetLog().Info(olxstr(atom.GetLabel()) << ": X=CH2");
              cg.FixAtom(AE, fgCH2, h_elm, NULL, generated);
            }
            else
              TBasicApp::GetLog().Info(olxstr(atom.GetLabel()) << ": possibly X=CH2");
          }
        }
      }
    }
    else  if( AE.Count() == 2 )  {
      const double db = 1.41;
      vec3d a = AE.GetCrd(0);
        a -= atom.crd();
      vec3d b = AE.GetCrd(1);
        b -= atom.crd();
      double v = a.CAngle(b);
      v = acos(v)*180/M_PI;
      double d1 = AE.GetCrd(0).DistanceTo( atom.crd() );
      double d2 = AE.GetCrd(1).DistanceTo( atom.crd() );
      if(  d1 > db && d2 > db && v < 125 )  {
        TBasicApp::GetLog().Info(olxstr(atom.GetLabel()) << ": XYCH2");
        cg.FixAtom(AE, fgCH2, h_elm, NULL, generated);
      }
      else  {
        if( (d1 < db || d2 < db) && v < 160 )  {
          TBasicApp::GetLog().Info(olxstr(atom.GetLabel()) << ": X(Y=C)H");
          cg.FixAtom(AE, fgCH1, h_elm, NULL, generated);
        }
      }
    }
    else if( AE.Count() == 3 )  {
      double v = TetrahedronVolume( atom.crd(), AE.GetCrd(0), AE.GetCrd(1), AE.GetCrd(2) );
      if( v > 0.3 )  {
        TBasicApp::GetLog().Info(olxstr(atom.GetLabel()) << ": XYZCH");
        cg.FixAtom(AE, fgCH1, h_elm, NULL, generated);
      }
    }
    else if( AE.Count() == 5 )  {  // carboranes ...
      //check
      bool proceed = false;
      for( size_t j=0; j < AE.Count(); j++ )
        if( AE.GetType(j) == iBoronZ )  {
          proceed = true;  break;
        }
      if( proceed )  {
        TBasicApp::GetLog().Info(olxstr(atom.GetLabel()) << ": R5CH");
        cg.FixAtom(AE, fgBH1, h_elm, NULL, generated);
      }
    }
  }
  else if( atom.GetType() == iNitrogenZ )  {  // nitrogen
    if( AE.Count() == 1 )  {
      double d = AE.GetCrd(0).DistanceTo( atom.crd() );
      if( d > 1.35 )  {
        TBasicApp::GetLog().Info(olxstr(atom.GetLabel()) << ": XNH2");
        cg.FixAtom(AE, fgNH2, h_elm, NULL, generated);
      }
      else  {
        if( d > 1.2 )  {  //else nitrile
          // have to check if double bond
          TSAtom* A = FindSAtom(AE.GetCAtom(0));
          TAtomEnvi NAE;
          if( A == 0 )
            throw TFunctionFailedException(__OlxSourceInfo, olxstr("Could not locate atom ") << AE.GetLabel(0) );

          UnitCell->GetAtomEnviList(*A, NAE, false, part);
          NAE.Exclude( atom.CAtom() );

          if( A->GetType() == iCarbonZ && NAE.Count() > 1 )  {
            vec3d a = NAE.GetCrd(0);
              a -= NAE.GetBase().crd();
            vec3d b = AE.GetBase().crd();
              b -= NAE.GetBase().crd();

            d = a.CAngle(b);
            d = acos(d)*180/M_PI;
            if( d > 115 && d < 130 )  {
              TBasicApp::GetLog().Info(olxstr(atom.GetLabel()) << ": X=NH2");
              cg.FixAtom(AE, fgNH2, h_elm, &NAE, generated);
            }
          }
          else  {
            TBasicApp::GetLog().Info(olxstr(atom.GetLabel()) << ": X=NH");
            cg.FixAtom(AE, fgNH1, h_elm, NULL, generated);
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
        if( v > 165 )  // skip ..
          TBasicApp::GetLog().Info(olxstr(atom.GetLabel()) << ": RN->M");
        else if( (d1 < 1.5 && d1 > 1.35) || (d2 < 1.5 && d2 > 1.35) )  {
          TBasicApp::GetLog().Info(olxstr(atom.GetLabel()) << ": RNH(2)M");
          cg.FixAtom(AE, fgNH2, h_elm, NULL, generated);
        }
        else if( d1 > 1.72 && d2 > 1.72 )  {
          TBasicApp::GetLog().Info(olxstr(atom.GetLabel()) << ": XX'NH");
          cg.FixAtom(AE, fgNH1, h_elm, NULL, generated);
        }
      }
      else if( v < 120 && d1 > 1.45 && d2 > 1.45 )  {
        TBasicApp::GetLog().Info(olxstr(atom.GetLabel()) << ": R2NH2+");
        cg.FixAtom(AE, fgNH2, h_elm, NULL, generated);
      }
      else if( v < 120 && (d1 < 1.3 || d2 < 1.3) )
        ;
      else  {
        if( (d1+d2) > 2.70 )  {
          TBasicApp::GetLog().Info(olxstr(atom.GetLabel()) << ": XYNH");
          cg.FixAtom(AE, fgNH1, h_elm, NULL, generated);
        }
      }
    }
    else if( AE.Count() == 3 )  {
    // remove coordination bond ...
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
          TBasicApp::GetLog().Info(olxstr(atom.GetLabel()) << ": R2HN->M");
          cg.FixAtom(AE, fgNH1, h_elm, NULL, generated);
        }
        else  {
          TBasicApp::GetLog().Info(olxstr(atom.GetLabel()) << ": R3NH+");
          cg.FixAtom(AE, fgNH1, h_elm, NULL, generated);
        }
      }
    }
  }
  if( atom.GetType() == iOxygenZ )  {  // oxygen
    if( AE.IsEmpty() )  {
      TBasicApp::GetLog().Info(olxstr(atom.GetLabel()) << ": OH2");
      TAtomEnvi pivoting;
      UnitCell->GetAtomPossibleHBonds(AE, pivoting);
      RemoveNonHBonding( pivoting );
      if( pivoting.Count() > 0 )
        if( !_AnalyseAtomHAdd( cg, *FindSAtom(pivoting.GetCAtom(0)), ProcessingAtoms) )
          pivoting.Clear();
      cg.FixAtom(AE, fgOH2, h_elm, &pivoting, generated);
    }
    else if( AE.Count() == 1 )  {
      double d = AE.GetCrd(0).DistanceTo( atom.crd() );
      if( d > 1.3 )   {  // otherwise a doubl bond
        if( AE.GetType(0) == iChlorineZ )
          ;
        else  if( AE.GetType(0) == iCarbonZ )  {  // carbon
          TBasicApp::GetLog().Info(olxstr(atom.GetLabel()) << ": COH");
          TAtomEnvi pivoting;
          UnitCell->GetAtomPossibleHBonds(AE, pivoting);
          RemoveNonHBonding( pivoting );
          if( pivoting.Count() > 0 )
            if( !_AnalyseAtomHAdd( cg, *FindSAtom(pivoting.GetCAtom(0)), ProcessingAtoms) )
              pivoting.Clear();
          cg.FixAtom(AE, fgOH1, h_elm, &pivoting, generated);
        }
        else  if( AE.GetType(0) == iSulphurZ )  {
          if( d > 1.48 )  {
            TBasicApp::GetLog().Info(olxstr(atom.GetLabel()) << ": SOH");
            cg.FixAtom(AE, fgOH1, h_elm, NULL, generated);
          }
        }
        else  if( AE.GetType(0) == iPhosphorusZ )  {
          if( d > 1.54 )  {
            TBasicApp::GetLog().Info(olxstr(atom.GetLabel()) << ": POH");
            TAtomEnvi pivoting;
            UnitCell->GetAtomPossibleHBonds(AE, pivoting);
            RemoveNonHBonding( pivoting );
            if( pivoting.Count() > 0 )
              if( !_AnalyseAtomHAdd( cg, *FindSAtom(pivoting.GetCAtom(0)), ProcessingAtoms) )
                pivoting.Clear();
            cg.FixAtom(AE, fgOH1, h_elm, &pivoting, generated);
          }
        }
        else  if( AE.GetType(0) == iSiliconZ )  {
          if( d > 1.6 )  {
            TBasicApp::GetLog().Info(olxstr(atom.GetLabel()) << ": SiOH");
            TAtomEnvi pivoting;
            UnitCell->GetAtomPossibleHBonds(AE, pivoting);
            RemoveNonHBonding( pivoting );
            if( pivoting.Count() > 0 )
              if( !_AnalyseAtomHAdd( cg, *FindSAtom(pivoting.GetCAtom(0)), ProcessingAtoms) )
                pivoting.Clear();
            cg.FixAtom(AE, fgOH1, h_elm, &pivoting, generated);
          }
        }
        else  if( AE.GetType(0) == iBoronZ )  {
          if( d < 1.38 )  {
            TBasicApp::GetLog().Info(olxstr(atom.GetLabel()) << ": B(III)OH");
            TAtomEnvi pivoting;
            UnitCell->GetAtomPossibleHBonds(AE, pivoting);
            RemoveNonHBonding( pivoting );
            if( pivoting.Count() > 0 )
              if( !_AnalyseAtomHAdd( cg, *FindSAtom(pivoting.GetCAtom(0)), ProcessingAtoms) )
                pivoting.Clear();
            cg.FixAtom(AE, fgOH1, h_elm, &pivoting, generated);
          }
        }
        else  if( AE.GetType(0) == iNitrogenZ )  {
          if( d > 1.37 )  {
            TBasicApp::GetLog().Info(olxstr(atom.GetLabel()) << ": NOH");
            TAtomEnvi pivoting;
            UnitCell->GetAtomPossibleHBonds(AE, pivoting);
            RemoveNonHBonding(pivoting);
            if( pivoting.Count() > 0 )
              if( !_AnalyseAtomHAdd( cg, *FindSAtom(pivoting.GetCAtom(0)), ProcessingAtoms) )
                pivoting.Clear();
            cg.FixAtom(AE, fgOH1, h_elm, &pivoting, generated);
          }
        }
        else if( d > 1.8 )  {  // coordination bond?
          TBasicApp::GetLog().Info(olxstr(atom.GetLabel()) << ": possibly M-OH2");
          cg.FixAtom(AE, fgOH2, h_elm, NULL, generated);
        }
      }
    }
  }
  else if( atom.GetType() == iBoronZ )  {  // boron
    if( AE.Count() == 3 )  {
      const vec3d cnt = AE.GetBase().crd();
      const double v = TetrahedronVolume( 
        cnt, 
        (AE.GetCrd(0)-cnt).Normalise() + cnt, 
        (AE.GetCrd(1)-cnt).Normalise() + cnt, 
        (AE.GetCrd(2)-cnt).Normalise() + cnt);
      if( v > 0.1 )  {
        TBasicApp::GetLog().Info(olxstr(atom.GetLabel()) << ": XYZBH");
        cg.FixAtom(AE, fgBH1, h_elm, NULL, generated);
      }
    }
    else if( AE.Count() == 4 )  {
      vec3d a, b;
      double sumAng = 0;
      for( size_t i=0; i < AE.Count(); i++ )  {
        a = AE.GetCrd(i);
        a -= atom.crd();
        for( size_t j=i+1; j < AE.Count(); j++ )  {
          b = AE.GetCrd(j);
          b -= atom.crd();
          double ca = b.CAngle(a);
          if( ca < -1 )  ca = -1;
          if( ca > 1 )   ca = 1;
          sumAng += acos(ca);
        }
      }
      if( sumAng*180/M_PI > 700 )  {   //!! not sure it works, lol
        TBasicApp::GetLog().Info(olxstr(atom.GetLabel()) << ": X4BH");
        cg.FixAtom(AE, fgBH1, h_elm, NULL, generated);
      }
    }
    else if( AE.Count() == 5 )  {
      TBasicApp::GetLog().Info(olxstr(atom.GetLabel()) << ": X5BH");
      cg.FixAtom(AE, fgBH1, h_elm, NULL, generated);
    }
  }
  else if( atom.GetType() == iSiliconZ )  {
    if( AE.Count() == 3 )  {
      const vec3d cnt = AE.GetBase().crd();
      const double v = TetrahedronVolume( 
        cnt, 
        (AE.GetCrd(0)-cnt).Normalise() + cnt, 
        (AE.GetCrd(1)-cnt).Normalise() + cnt, 
        (AE.GetCrd(2)-cnt).Normalise() + cnt);
      if( v > 0.1 )  {
        TBasicApp::GetLog().Info(olxstr(atom.GetLabel()) << ": XYZSiH");
        cg.FixAtom(AE, fgSiH1, h_elm, NULL, generated);
      }
    }
    else if( AE.Count() == 2 )  {  // no validation yet...
      TBasicApp::GetLog().Info(olxstr(atom.GetLabel()) << ": XYSiH2");
      cg.FixAtom(AE, fgSiH2, h_elm, NULL, generated);
    }
  }
  else if( atom.GetType() == iSulphurZ )  {
    if( AE.Count() == 1 && AE.GetType(0) == iCarbonZ )  {
      double d = AE.GetCrd(0).DistanceTo(atom.crd());
      if( d > 1.72 )  {
        TBasicApp::GetLog().Info(olxstr(atom.GetLabel()) << ": CSH");
        cg.FixAtom(AE, fgSH1, h_elm, NULL, generated);
      }
    }
  }
  ProcessingAtoms.Delete( ProcessingAtoms.IndexOf(&atom) );
  return true;
}
//..............................................................................
void TLattice::_ProcessRingHAdd(AConstraintGenerator& cg, const ElementPList& rcont) {
  TTypeList<TSAtomPList> rings;
  cm_Element& h_elm = XElementLib::GetByIndex(iHydrogenIndex);
  for( size_t i=0; i < FragmentCount(); i++ )
    GetFragment(i).FindRings(rcont, rings);
  TAtomEnvi AE;
  for( size_t i=0; i < rings.Count(); i++ )  {
    double rms = TSPlane::CalcRMS( rings[i] );
    if( rms < 0.05 && TNetwork::IsRingRegular( rings[i]) )  {
      for( size_t j=0; j < rings[i].Count(); j++ )  {
        AE.Clear();
        UnitCell->GetAtomEnviList(*rings[i][j], AE);
        if( AE.Count() == 3 )  {
          const vec3d cnt = AE.GetBase().crd();
          double v = TetrahedronVolume( 
            cnt, 
            (AE.GetCrd(0)-cnt).Normalise() + cnt, 
            (AE.GetCrd(1)-cnt).Normalise() + cnt, 
            (AE.GetCrd(2)-cnt).Normalise() + cnt);
          if( v < 0.1 )  continue;  // coordination or substituted
        }
        for( size_t k=0; k < AE.Count(); k++ )  {
          if( (AE.GetCrd(k) - rings[i][j]->crd()).QLength() > 4.0 )
            AE.Delete(k--);
        }
        if( AE.Count() == 2 && rings[i][j]->GetType() == iCarbonZ )  {
          TBasicApp::GetLog().Info(olxstr(rings[i][j]->GetLabel()) << ": X(Y=C)H (ring)");
          cg.FixAtom(AE, fgCH1, h_elm);
          rings[i][j]->CAtom().SetHAttached(true);
        }
      }
    }
  }
}
//..............................................................................
void TLattice::AnalyseHAdd(AConstraintGenerator& cg, const TSAtomPList& atoms)  {
  if( Generated )  {
    TBasicApp::GetLog().Error("Hadd is not applicable to grown structure");
    return;
  }
  ElementPList CTypes;
  CTypes.Add(XElementLib::FindBySymbol("C"));
  CTypes.Add(XElementLib::FindBySymbol("N"));
  CTypes.Add(XElementLib::FindBySymbol("O"));
  CTypes.Add(XElementLib::FindBySymbol("B"));
  CTypes.Add(XElementLib::FindBySymbol("Si"));
  CTypes.Add(XElementLib::FindBySymbol("S"));
  TSAtomPList ProcessingAtoms;

  for( size_t i=0; i < atoms.Count(); i++ )
    atoms[i]->CAtom().SetHAttached(false);

  // treat rings
  ElementPList rcont;
  rcont.Add(CTypes[0]);
  for( size_t i=0; i < 4; i++ )  
    rcont.Add(rcont[0]);
  _ProcessRingHAdd(cg, rcont); // Cp
  rcont.Add(rcont[0]);
  _ProcessRingHAdd(cg, rcont); // Ph
  rcont.Last() = CTypes[1];
  _ProcessRingHAdd(cg, rcont); // Py

  for( size_t i=0; i < atoms.Count(); i++ )  {
    if( atoms[i]->IsDeleted() || !atoms[i]->CAtom().IsAvailable() )  continue;

    bool consider = false;
    for( size_t j=0; j < CTypes.Count(); j++ )  {
      if( atoms[i]->GetType() == *CTypes[j] )  {
        consider = true;
        break;
      }
    }
    if( !consider )  continue;
    for( size_t j=0; j < atoms[i]->NodeCount(); j++ )  {
      TSAtom& A = atoms[i]->Node(j);
      if( A.IsDeleted() )  continue;
      if( A.GetType() == iHydrogenZ ) {
        consider = false;
        break;
      }
    }
    for( size_t j=0; j < atoms[i]->CAtom().AttachedAtomCount(); j++ )  {
      if( atoms[i]->CAtom().GetAttachedAtom(j).GetType() == iHydrogenZ &&
        !atoms[i]->CAtom().GetAttachedAtom(j).IsDeleted() )  {
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
  for( size_t i=0; i < Envi.Count(); i++ )  {
    TSAtom* SA = FindSAtom(Envi.GetCAtom(i));
    AE.Clear();
    UnitCell->GetAtomEnviList(*SA, AE);
    if( SA->GetType() == iOxygenZ )  {
    /* this case needs an investigation, but currently the same atom cannot be a pivoting one ...*/
      if( SA->CAtom() == Envi.GetBase().CAtom() )  {
        Envi.Exclude(SA->CAtom());
        continue;
      }
      if( AE.IsEmpty() )  {
        if( SA->CAtom() != Envi.GetBase().CAtom() )  // it is a symmetrical equivalent
          Envi.Exclude( SA->CAtom() );
      }
      else if( AE.Count() == 1 )  {
        double d = AE.GetCrd(0).DistanceTo( SA->crd() );
        if( d > 1.8 )  // coordination bond?
          Envi.Exclude(SA->CAtom());
      }
      else if( AE.Count() == 2 )  {  // not much can be done here ... needs thinking
        //Envi.Exclude( SA->CAtom() );
        // commented 17.03.08, just trying to what the shortest distance will give
      }
      else if( AE.Count() == 3 )  {  // coordinated water molecule
        Envi.Exclude(SA->CAtom());
      }
    }
    else if( SA->GetType() == iNitrogenZ )  {
      if( AE.Count() > 3 )
          Envi.Exclude(SA->CAtom());
    }
  }
  // choose the shortest bond ...
  if( Envi.Count() > 1 )  {
    TPSTypeList<double, TCAtom*> hits;

    for( size_t i=0; i < Envi.Count(); i++ )
      hits.Add(Envi.GetBase().crd().DistanceTo(Envi.GetCrd(i)), &Envi.GetCAtom(i));

    while( hits.Count() > 1 &&
      ((hits.GetComparable(hits.Count()-1) - hits.GetComparable(0)) > 0.05) )  {
      Envi.Exclude(*hits.GetObject(hits.Count()-1));
      hits.Remove(hits.Count()-1);
    }
  }
  // all similar length  .... Q peaks might help :)
  if( Envi.Count() > 1 )  {
    TPSTypeList<double, TCAtom*> hits;
    vec3d vec1, vec2;
    AE.Clear();
    UnitCell->GetAtomQEnviList( Envi.GetBase(), AE );
    for( size_t i=0; i < AE.Count(); i++ )  {
//      v1 = AE.GetCrd(i);
//      v1 -= Envi.GetBase()->crd();
      double d = Envi.GetBase().crd().DistanceTo( AE.GetCrd(i) );

      if( d < 0.7 || d > 1.3 )  {
        AE.Exclude(AE.GetCAtom(i));
        i--;
      }
    }
    if( AE.IsEmpty() || AE.Count() > 1 )  return;
    vec1 = AE.GetCrd(0);
    vec1 -= Envi.GetBase().crd();
    for( size_t i=0; i < Envi.Count(); i++ )  {
      vec2 = Envi.GetCrd(i);
      vec2 -= Envi.GetBase().crd();
      hits.Add(olx_abs(-1 + vec2.CAngle(vec1)), &Envi.GetCAtom(i));
    }
    while( hits.Count() > 1 )  {
      Envi.Exclude( *hits.GetObject( hits.Count() - 1 ) );
      hits.Remove(hits.Count() - 1);
    }
  }

}
//..............................................................................
void TLattice::SetAnis( const TCAtomPList& atoms, bool anis )  {
  if( !anis )  {
    for( size_t i=0; i < atoms.Count(); i++ )  {
      if( olx_is_valid_index(atoms[i]->GetEllpId()) )  {
         GetAsymmUnit().NullEllp( atoms[i]->GetEllpId() );
         atoms[i]->AssignEllp(NULL);
      }
    }
    GetAsymmUnit().PackEllps();
  }
  else  {
    evecd ee(6);
    for( size_t i=0; i < atoms.Count(); i++ )  {
      if( atoms[i]->GetEllipsoid() == NULL )  {
        ee[0] = ee[1] = ee[2] = atoms[i]->GetUiso();
        atoms[i]->UpdateEllp(ee);
      }
    }
  }
  OnStructureUniq.Enter(this);
  Init();
  OnStructureUniq.Exit(this);
}
//..............................................................................
void TLattice::ToDataItem(TDataItem& item) const  {
  item.AddField("delta", Delta);
  item.AddField("deltai", DeltaI);
  item.AddField("grown", Generated);
  GetAsymmUnit().ToDataItem(item.AddItem("AUnit"));
  TDataItem& mat = item.AddItem("Matrices");
  const size_t mat_c = Matrices.Count();
  // save matrices, change matrix tags to the position in the list and remember old tags
  TArrayList<uint32_t> m_tags(mat_c);
  for( size_t i=0; i < mat_c; i++ )  {
    mat.AddItem(i, TSymmParser::MatrixToSymmEx(*Matrices[i])).AddField("id", Matrices[i]->GetId());
    m_tags[i] = Matrices[i]->GetId();
    Matrices[i]->SetRawId((uint32_t)i);
  }
  // initialise bond tags
  size_t sbond_tag = 0;
  for( size_t i=0; i < Bonds.Count(); i++ )  {
    if( Bonds[i]->IsDeleted() )  continue;
    Bonds[i]->SetTag(sbond_tag++);
  }
  // initialise atom tags
  size_t satom_tag = 0;
  for( size_t i=0; i < Atoms.Count(); i++ )  {
    if( Atoms[i]->IsDeleted() )  continue;
    Atoms[i]->SetTag(satom_tag++);
  }
  // initialise fragment tags
  size_t frag_tag = 0;
  for( size_t i=0; i < Fragments.Count(); i++ )  {
    //if( Fragments[i]->NodeCount() == 0 )  continue;
    Fragments[i]->SetTag(frag_tag++);
  }
  // save satoms - only the original CAtom Tag and the generating matrix tag
  TDataItem& atoms = item.AddItem("Atoms");
  for( size_t i=0; i < Atoms.Count(); i++ )  {
    if( Atoms[i]->IsDeleted() )  continue;
    Atoms[i]->ToDataItem(atoms.AddItem("Atom"));
  }
  // save bonds
  TDataItem& bonds = item.AddItem("Bonds");
  for( size_t i=0; i < Bonds.Count(); i++ )  {
    if( Bonds[i]->IsDeleted() )  continue;
    Bonds[i]->ToDataItem(bonds.AddItem("Bond"));
  }
  // save fragments
  TDataItem& frags = item.AddItem("Fragments");
  for( size_t i=0; i < Fragments.Count(); i++ )  {
    //if( Fragments[i]->NodeCount() == 0 )  continue;
    Fragments[i]->ToDataItem(frags.AddItem("Fragment"));
  }
  // restore original matrix tags 
  for( size_t i=0; i < mat_c; i++ )
    Matrices[i]->SetRawId(m_tags[i]);
  // save planes
  TSPlanePList valid_planes;
  for( size_t i=0; i < Planes.Count(); i++ )  {
    if( Planes[i]->IsDeleted() ) continue;
    size_t p_ac = 0;  
    for( size_t j=0; j < Planes[i]->Count(); j++ ) 
      if( Planes[i]->Atom(j).IsAvailable() )
        p_ac++;
    if( p_ac >= 3 ) // a plane must contain at least three atoms
      valid_planes.Add(Planes[i]);
    else
      Planes[i]->SetDeleted(true);
  }
  TDataItem& planes = item.AddItem("Planes");
  for( size_t i=0; i < valid_planes.Count(); i++ )
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
  for( size_t i=0; i < mat.ItemCount(); i++ )  {
    smatd* m = new smatd;
    TSymmParser::SymmToMatrix(mat.GetItem(i).GetValue(), *m);
    Matrices.Add(m);
    m->SetRawId(mat.GetItem(i).GetRequiredField("id").ToUInt() );
  }
  // precreate fragments
  const TDataItem& frags = item.FindRequiredItem("Fragments");
  Fragments.SetCapacity( frags.ItemCount() );
  for( size_t i=0; i < frags.ItemCount(); i++ )
    Fragments.Add(new TNetwork(this, NULL));
  // precreate bonds
  const TDataItem& bonds = item.FindRequiredItem("Bonds");
  Bonds.SetCapacity(bonds.ItemCount());
  for( size_t i=0; i < bonds.ItemCount(); i++ )
    Bonds.Add(new TSBond(NULL))->SetLattId(i);
  // precreate and load atoms
  const TDataItem& atoms = item.FindRequiredItem("Atoms");
  Atoms.SetCapacity(atoms.ItemCount());
  for( size_t i=0; i < atoms.ItemCount(); i++ )
    Atoms.Add( new TSAtom(NULL) )->SetLattId(i);
  for( size_t i=0; i < atoms.ItemCount(); i++ )
    Atoms[i]->FromDataItem(atoms.GetItem(i), *this);
  // load bonds
  for( size_t i=0; i < bonds.ItemCount(); i++ )
    Bonds[i]->FromDataItem(bonds.GetItem(i), Fragments);
  // load fragments
  for( size_t i=0; i < frags.ItemCount(); i++ )
    Fragments[i]->FromDataItem(frags.GetItem(i));
  GetUnitCell().InitMatrices();
  //int eqc = GetUnitCell().FindSymmEq(0.1, true, false, false); // find and not remove
  //GetAsymmUnit().SetContainsEquivalents( eqc != 0 );
  //Disassemble();
  TDataItem& planes = item.FindRequiredItem("Planes");
  for( size_t i=0; i < planes.ItemCount(); i++ )
    Planes.Add(new TSPlane(Network))->FromDataItem(planes.GetItem(i));
}
//..............................................................................
void TLattice::SetGrowInfo(GrowInfo* grow_info)  {
  if( _GrowInfo != NULL )
    delete _GrowInfo;
  _GrowInfo = grow_info;
}
//..............................................................................
TLattice::GrowInfo* TLattice::GetGrowInfo() const  {
  if( !Generated )
    return NULL;
  const TAsymmUnit& au = GetAsymmUnit();
  GrowInfo& gi = *(new GrowInfo);
  gi.matrices.SetCount( Matrices.Count() );
  gi.unc_matrix_count = GetUnitCell().MatrixCount();
  // save matrix tags and init gi.matrices
  TArrayList<uint32_t> mtags(Matrices.Count());
  for( size_t i=0; i < Matrices.Count(); i++ )  {
    mtags[i] = Matrices[i]->GetId();
    (gi.matrices[i] = new smatd(*Matrices[i]))->SetRawId(mtags[i]);
    Matrices[i]->SetRawId((uint32_t)i);
  }

  gi.info.SetCount( au.AtomCount() );
  for( size_t i=0; i < Atoms.Count(); i++ )  {
    TIndexList& mi = gi.info[ Atoms[i]->CAtom().GetId() ];
    const size_t mi_cnt = mi.Count();
    mi.SetCount(mi_cnt + Atoms[i]->MatrixCount()+1);
    mi[mi_cnt] = -(int)Atoms[i]->MatrixCount(); // separator field
    for( size_t j=1; j <= Atoms[i]->MatrixCount(); j++ )
      mi[mi_cnt+j] = Atoms[i]->GetMatrix(j-1).GetId();
  }
  // restore matrix tags
  for( size_t i=0; i < mtags.Count(); i++ )
    Matrices[i]->SetRawId(mtags[i]);
  return &gi;
}
//..............................................................................
bool TLattice::ApplyGrowInfo()  {
  TAsymmUnit& au = GetAsymmUnit();
  if( _GrowInfo == NULL || !Atoms.IsEmpty() || !Matrices.IsEmpty() || 
    GetUnitCell().MatrixCount() != _GrowInfo->unc_matrix_count ||
    au.AtomCount() < _GrowInfo->info.Count() )  // let's be picky
  {
    if( _GrowInfo != NULL )  {
      delete _GrowInfo;
      _GrowInfo = NULL;
    }
    return false;
  }
  Matrices.Assign( _GrowInfo->matrices );
  _GrowInfo->matrices.Clear();
  Atoms.SetCapacity(au.AtomCount()*Matrices.Count());
  for( size_t i=0; i < au.AtomCount(); i++ )    {
    TCAtom& ca = GetAsymmUnit().GetAtom(i);
    // we still need masked and detached atoms here
    if( ca.IsDeleted() )  continue;
    if( i >= _GrowInfo->info.Count() )  {  // create just with I matrix
      TSAtom* a = Atoms.Add( new TSAtom(Network) );
      a->CAtom(ca);
      a->SetEllipsoid( &GetUnitCell().GetEllipsoid(0, ca.GetId()) ); // ellipsoid for the identity matrix
      a->SetLattId( Atoms.Count() - 1 );
      au.CellToCartesian(a->ccrd(), a->crd());
      a->AddMatrix( Matrices[0] );
      continue;
    }
    const TIndexList& mi = _GrowInfo->info[i];
    for( size_t j=0; j < mi.Count(); j++ )  {
      if( mi[j] < 0 )  {
        const size_t matr_cnt = olx_abs(mi[j]),
          matr_start = j+1;       
        TSAtom* a = Atoms.Add( new TSAtom(Network) );
        a->CAtom(ca);
        a->SetEllipsoid(&GetUnitCell().GetEllipsoid(Matrices[mi[matr_start]]->GetContainerId(), ca.GetId())); // ellipsoid for the matrix
        a->SetLattId( Atoms.Count() - 1 );
        a->ccrd() = (*Matrices[mi[matr_start]]) * ca.ccrd();
        au.CellToCartesian(a->ccrd(), a->crd());
        for( size_t k=matr_start; k < matr_start+matr_cnt; k++, j++ )
          a->AddMatrix( Matrices[mi[k]] );
      }
    }
  }
  Generated = true;
  delete _GrowInfo;
  _GrowInfo = NULL;
  return true;
}
//..............................................................................
olxstr TLattice::CalcMoiety() const {
  /* There is a need to find out if fragments can be grow and if they are polymeric,
  a simple test for polymers would be to compare the atom's degenerocity is smaller than
  the number of potential uniq directions in which the atom can grow. It is more complicated
  if the atoms is not on a special position, because it can be either polymeric or form
  dimers/trimers etc... */
  TLattice latt;
  latt.AsymmUnit->SetRefMod(AsymmUnit->GetRefMod());
  latt.AsymmUnit->Assign(GetAsymmUnit());
  latt.AsymmUnit->_UpdateConnInfo();
  latt.AsymmUnit->DetachAtomType(iQPeakZ, true);
  latt.Init();
  latt.CompaqAll();
  latt.Fragments.QuickSorter.SortSF(latt.Fragments, TLattice_SortFragments);
  // multiplicity,content, reference fragment index
  TTypeList<AnAssociation3<double,ContentList, size_t> > frags;
  TArrayList<vec3d> centres(latt.FragmentCount());
  for( size_t i=0; i < latt.FragmentCount(); i++ )  {
    ContentList cl = latt.GetFragment(i).GetContentList();
    if( cl.IsEmpty())  continue;
    XElementLib::SortContentList(cl);
    bool uniq = true;
    double wght=0, overall_occu = 0;
    for( size_t j=0; j < latt.GetFragment(i).NodeCount(); j++ )  {
      TSAtom& nd = latt.GetFragment(i).Node(j);
      if( nd.IsDeleted() || nd.GetType() == iQPeakZ )  continue;
      centres[i] += nd.crd();
      wght += 1;
      const double occu = nd.CAtom().GetOccu()*nd.CAtom().GetDegeneracy();
      if( overall_occu == 0 )
        overall_occu = occu;
      else if( overall_occu != -1 && olx_abs(overall_occu-occu) > 0.01 )
        overall_occu = -1;
    }
    centres[i] /= wght;
    for( size_t j=0; j < frags.Count(); j++ )  {
      if( frags[j].GetB().Count() != cl.Count() )  continue;
      bool equals = true;
      if( frags[j].GetB()[0].element != cl[0].element )
        equals = false;
      else  {
        for( size_t k=1; k < cl.Count(); k++ )  {
          if( frags[j].GetB()[k].element != cl[k].element || 
            olx_abs((frags[j].GetB()[k].count/frags[j].GetB()[0].count)-(cl[k].count/cl[0].count)) > 0.01 )
          {
            equals = false;
            break;
          }
        }
      }
      if( equals )  {
        // consider special case of nearby and/or overlapping fragments, compare rations, not counts...
        if( centres[i].QDistanceTo(centres[frags[j].GetC()]) < 4 )  {    // just sum up the values
          if( olx_abs(overall_occu) == 1 )  {
            for( size_t k=0; k < cl.Count(); k++ )
              frags[j].B()[k].count += cl[k].count;
          }
          else
            frags[j].A() += cl[0].count/frags[j].GetB()[0].count;
        }
        else  {  // just increment the count
          frags[j].A() += cl[0].count/frags[j].GetB()[0].count;
        }
        uniq = false;
        break;
      }
    }
    if( uniq )  {
      if( olx_abs(overall_occu) == 1 )
        frags.AddNew(1, cl, i);
      else  {  // apply overal atom occupancy
        for( size_t j=0; j < cl.Count(); j++ )
          cl[j].count /= overall_occu;
        frags.AddNew(overall_occu, cl, i);
      }
    }
  }
  olxstr rv;
  for( size_t i=0; i < frags.Count(); i++ )  {
    if( !rv.IsEmpty() )  rv << ", ";
    if( frags[i].GetA() != 1 )
      rv << frags[i].GetA() << '(';
    for( size_t j=0; j < frags[i].GetB().Count(); j++ )  {
      rv << frags[i].GetB()[j].element.symbol;
      if( frags[i].GetB()[j].count != 1 )
        rv << frags[i].GetB()[j].count;
      if( (j+1) < frags[i].GetB().Count() )
        rv << ' ';
    }
    if( frags[i].GetA() != 1 )
      rv << ')';
  }
  return rv;
}
//..............................................................................
void TLattice::RestoreADPs(bool restoreCoordinates)  {
  TUnitCell& uc = GetUnitCell();
  const TAsymmUnit& au = GetAsymmUnit();
  uc.UpdateEllipsoids();
  for( size_t i=0; i < AtomCount(); i++ )  {
    TSAtom& sa = GetAtom(i);
    if( restoreCoordinates )
      au.CellToCartesian(sa.ccrd(), sa.crd());
    if( sa.CAtom().GetEllipsoid() != NULL )
      sa.SetEllipsoid(&uc.GetEllipsoid(sa.GetMatrix(0).GetContainerId(), sa.CAtom().GetId()));
  }
  for( size_t i=0; i < uc.EllpCount(); i++ )  {
    TEllipsoid* elp = uc.GetEllp(i);
    if( elp != NULL )  
      elp->SetTag(0);
  }
}
//..............................................................................
//..............................................................................
//..............................................................................
void TLattice::LibGetFragmentCount(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal( olxstr(FragmentCount()) );
}
void TLattice::LibGetFragmentAtoms(const TStrObjList& Params, TMacroError& E)  {
  size_t index = Params[0].ToSizeT();
  if( index >= FragmentCount() )
    throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, FragmentCount());
  olxstr rv;
  for( size_t i=0; i < Fragments[index]->NodeCount(); i++ )  {
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
