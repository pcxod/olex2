//---------------------------------------------------------------------------//
// namespace TXClasses: TLattice
// (c) Oleg V. Dolomanov, 2004
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
int TLattice_SortAtomsById(const TSAtom* a1, const TSAtom* a2)  {
  return olx_cmp(a1->CAtom().GetId(), a2->CAtom().GetId());
}
int TLattice_AtomsSortByDistance(const TSAtom* A1, const TSAtom* A2)  {
  const double d = A1->crd().QLength() - A2->crd().QLength();
  return (d < 0 ? -1 : ((d > 0 ) ? 1 : 0));
}
//---------------------------------------------------------------------------
// TLattice function bodies
//---------------------------------------------------------------------------
TLattice::TLattice(ASObjectProvider& ObjectProvider) :
  OnStructureGrow(Actions.New("STRGEN")),
  OnStructureUniq(Actions.New("STRUNIQ")),
  OnDisassemble(Actions.New("DISASSEBLE")),
  OnAtomsDeleted(Actions.New("ATOMSDELETE")),
  Objects(ObjectProvider)
{
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
  delete &Objects;
}
//..............................................................................
void TLattice::ClearAtoms()  {
  if( !Objects.atoms.IsEmpty() )  {
    OnAtomsDeleted.Enter(this);
    Objects.atoms.Clear();
    OnAtomsDeleted.Exit(this);
  }
}
//..............................................................................
void TLattice::ClearFragments()  {
  for( size_t i=0; i < Fragments.Count(); i++ )
    delete Fragments[i];
  Fragments.Clear();
}
//..............................................................................
void TLattice::ClearMatrices()  {
  for( size_t i=0; i < Matrices.Count(); i++ )
    delete Matrices[i];
  Matrices.Clear();
}
//..............................................................................
void TLattice::Clear(bool ClearUnitCell)  {
  ClearAtoms();
  ClearBonds();
  ClearFragments();
  ClearMatrices();
  ClearPlanes();
  if( ClearUnitCell )  {
    GetUnitCell().Clear();
    GetAsymmUnit().Clear();
  }
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
          vec3d rs = au.Orthogonalise(m * cnt);
          if( center.QDistanceTo(rs) > qrad )  continue;
          Result.Add(new smatd(m))->SetId((uint8_t)i, j, k, l);  // set Tag to identify the matrix (and ellipsoids) in the UnitCell
        }
  }
  const size_t res_cnt = Result.Count();
  for( size_t i=0; i < mstart; i++ )  {  // check if all matrices are uniq
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
  const size_t ac = Objects.atoms.Count();
  if( ocrd != NULL )  {
    for( size_t i=0; i < ac; i++ )  {
      TSAtom& sa = Objects.atoms[i];
      (*ocrd)[i] = sa.crd();
      GetAsymmUnit().CellToCartesian(sa.ccrd(), sa.crd());
      if( !sa.CAtom().IsAvailable() )
        dac++;
    }
  }
  else  {
    for( size_t i=0; i < ac; i++ )  {
      if( !Objects.atoms[i].CAtom().IsAvailable() )
        dac++;
    }
  }
  TSAtomPList atoms(ac-dac);
  dac = 0;
  for( size_t i=0; i < ac; i++ )  {
    TSAtom& sa = Objects.atoms[i];
    sa.ClearNodes();
    sa.ClearBonds();
    if( !sa.CAtom().IsAvailable() )  {
      dac++;
      sa.SetNetwork(*Network);
    }
    else
      atoms[i-dac] = &sa;
  }
  BuildAtomRegistry();
  Network->Disassemble(Objects, Fragments);
  dac = 0;
  for( size_t i=0; i < ac; i++ )  {
    TSAtom& sa = Objects.atoms[i];
    if( sa.IsDeleted() )
      dac++;
    else  {
      if( ocrd != NULL )
        sa.crd() = (*ocrd)[i];
    }
  }
  if( dac != 0 )  {
    OnAtomsDeleted.Enter(this);
    for( size_t i=0; i < ac; i++ )  {
      if( Objects.atoms[i].IsDeleted() )
        Objects.atoms.Null(i);
    }
    Objects.atoms.Pack();
    OnAtomsDeleted.Exit(this);
  }
  TNetPList::QuickSorter.SortSF(Fragments, CompareFragmentsBySize);
  for( size_t i=0; i < Fragments.Count(); i++ )  {
    for( size_t j=0; j < Fragments[i]->NodeCount(); j++ )
      Fragments[i]->Node(j).CAtom().SetFragmentId((uint32_t)i);
  }
}
//..............................................................................
void TLattice::BuildPlanes()  {
  ClearPlanes();
  for( size_t i=0; i < PlaneDefs.Count(); i++ )  {
    TSPlane::Def& pd = PlaneDefs[i];
    for( size_t j=0; j < Matrices.Count(); j++ )  {
      TSPlane* p = pd.FromAtomRegistry(Objects, i, Network, *Matrices[j]);
      if( p != NULL ) {
        bool uniq = true;
        for( size_t k=0; k < Objects.planes.Count()-1; k++ )  {
          if( Objects.planes[k].GetCenter().QDistanceTo(p->GetCenter()) < 1e-6 )  {
            uniq = false;
            break;
          }
        }
        if( !uniq )
          Objects.planes.DeleteLast();
      }
    }
  }
}
//..............................................................................
void TLattice::InitBody()  {
  OnDisassemble.Enter(this);
  if( !ApplyGrowInfo() )  {
    // create identity matrix
    Matrices.Add(new smatd(GetUnitCell().GetMatrix(0)))->SetId(0);
    ClearPlanes();
    Objects.atoms.IncCapacity(GetAsymmUnit().AtomCount());
    for( size_t i=0; i < GetAsymmUnit().AtomCount(); i++ )    {
      TCAtom& CA = GetAsymmUnit().GetAtom(i);
      if( CA.IsDeleted() )  continue;
      GenerateAtom(CA, *Matrices[0]);
    }
  }
  GenerateBondsAndFragments(NULL);
  BuildPlanes();
  OnDisassemble.Exit(this);
}
//..............................................................................
void TLattice::Init()  {
  Clear(false);
  GetUnitCell().ClearEllipsoids();
  GetUnitCell().InitMatrices();
  GetAsymmUnit().GetRefMod()->UpdateUsedSymm(GetUnitCell());
  GetUnitCell().FindSymmEq(); // find and remove
  InitBody();
}
//..............................................................................
void TLattice::Uniq(bool remEqv)  {
  OnStructureUniq.Enter(this);
  Clear(false);
  ClearMatrices();
  GetUnitCell().UpdateEllipsoids();  // if new atoms are created...
  GetUnitCell().FindSymmEq(); // find and remove
  InitBody();
  OnStructureUniq.Exit(this);
}
//..............................................................................
void TLattice::GenerateWholeContent(TCAtomPList* Template)  {
  OnStructureGrow.Enter(this);
  Generate(Template, false);
  OnStructureGrow.Exit(this);
}
//..............................................................................
void TLattice::Generate(TCAtomPList* Template, bool ClearCont)  {
  if( ClearCont && Template != NULL ) 
    ClearAtoms();
  else  {
    const size_t ac = Objects.atoms.Count();
    size_t da = 0;
    for( size_t i=0; i < ac; i++ )  {  // restore atom coordinates
      TSAtom& sa = Objects.atoms[i];
      if( sa.IsDeleted() )
        da++;
      else
        GetAsymmUnit().CellToCartesian(sa.ccrd(), sa.crd());
    }
    if( da != 0 )  {
      const size_t ac = Objects.atoms.Count();
      OnAtomsDeleted.Enter(this);
      for( size_t i=0; i < ac; i++ )  {  // restore atom coordinates
        if( Objects.atoms[i].IsDeleted() )
          Objects.atoms.Null(i);
      }
      Objects.atoms.Pack();
      OnAtomsDeleted.Exit(this);
    }
  }

  TAsymmUnit& au = GetAsymmUnit();
  Objects.atoms.IncCapacity(Matrices.Count()*au.AtomCount());
  for( size_t i=0; i < Matrices.Count(); i++ )  {
    smatd* M = Matrices[i];
    for( size_t j=0; j < au.AtomCount(); j++ )  {
      if( !au.GetAtom(j).IsAvailable() )  continue;
      GenerateAtom(au.GetAtom(j), *Matrices[i]);
    }
  }
  Disassemble();
}
//..............................................................................
void TLattice::GenerateCell()  {
  ClearAtoms();
  ClearMatrices();
  OnStructureGrow.Enter(this);
  const TUnitCell& uc = GetUnitCell();
  TAsymmUnit& au = GetAsymmUnit();
  for( size_t i=0; i < uc.MatrixCount(); i++ )  {
    const smatd& m = uc.GetMatrix(i);
    for( size_t j=0; j < au.AtomCount(); j++ )  {
      TCAtom& ca = au.GetAtom(j);
      if( !ca.IsAvailable() )  continue;
      TSAtom& sa = Objects.atoms.New(Network);
      sa.CAtom(ca);
      sa.ccrd() = m*ca.ccrd();
      const vec3i t = -sa.ccrd().Floor<int>();
      sa.ccrd() += t;
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
      au.CellToCartesian(sa.ccrd(), sa.crd());
      sa.SetEllipsoid(&GetUnitCell().GetEllipsoid(m.GetContainerId(), ca.GetId()));
      sa.AddMatrix(lm);
    }
  }
  Disassemble();
  OnStructureGrow.Exit(this);
}
//..............................................................................
void TLattice::GenerateBox(const mat3d& norms, const vec3d& size, const vec3d& center,
                           bool clear_content)
{
  OnStructureGrow.Enter(this);
  if( clear_content )  {
    ClearAtoms();
    ClearMatrices();
  }
  const TUnitCell& uc = GetUnitCell();
  TAsymmUnit& au = GetAsymmUnit();
  for( size_t i=0; i < uc.MatrixCount(); i++ )  {
    const smatd& m = uc.GetMatrix(i);
    for( int di = -3; di <= 3; di++ )  {
      for( int dj = -3; dj <= 3; dj++ )  {
        for( int dk = -3; dk <= 3; dk++ )  {
          const vec3d t(di, dj, dk);
          const uint32_t m_id = smatd::GenerateId((uint8_t)i, t);
          smatd* lm = NULL;
          bool matrix_created = false;
          for( size_t j=0; j < Matrices.Count(); j++ )  {
            if( Matrices[j]->GetId() == m_id )  {
              lm = Matrices[j];
              break;
            }
          }
          if( lm == NULL )  {
            lm = new smatd(m);
            lm->t += t;
            lm->SetRawId(m_id);
            matrix_created = true;
          }
          for( size_t j=0; j < au.AtomCount(); j++ )  {
            TCAtom& ca = au.GetAtom(j);
            if( ca.IsDeleted() )  continue;
            vec3d p = m*ca.ccrd() + t;
            const vec3d ccrd = p;
            const vec3f c = norms*(au.CellToCartesian(p) - center);
            if( olx_abs(c[0]) > size[0] || olx_abs(c[1]) > size[1] || olx_abs(c[2]) > size[2] )
              continue;
            GenerateAtom(ca, *lm);
            if( matrix_created )  {
              Matrices.Add(lm);
              matrix_created = false;
            }
          }
          if( matrix_created )
            delete lm;
        }
      }
    }
  }
  Disassemble();
  OnStructureGrow.Exit(this);
}
//..............................................................................
void TLattice::Generate(const vec3d& MFrom, const vec3d& MTo, TCAtomPList* Template, bool ClearCont)  {
  OnStructureGrow.Enter(this);
  vec3d VTo(olx_round(MTo[0]+1), olx_round(MTo[1]+1), olx_round(MTo[2]+1));
  vec3d VFrom(olx_round(MFrom[0]-1), olx_round(MFrom[1]-1), olx_round(MFrom[2]-1));
  if( ClearCont )  {
    ClearAtoms();
    ClearMatrices();
  }
  GenerateMatrices(VFrom, VTo, MFrom, MTo);
  Generate(Template, ClearCont);
  OnStructureGrow.Exit(this);
}
//..............................................................................
void TLattice::Generate(const vec3d& center, double rad, TCAtomPList* Template, bool ClearCont)  {
  OnStructureGrow.Enter(this);
  if( ClearCont )  {
    ClearAtoms();
    ClearMatrices();
  }
  GenerateMatrices(Matrices, center, rad);
  Generate(Template, ClearCont);
  OnStructureGrow.Exit(this);
}
//..............................................................................
SortedObjectList<smatd, smatd::ContainerIdComparator>
  TLattice::GetFragmentGrowMatrices(const TCAtomPList& l) const
{
  SortedObjectList<smatd, smatd::ContainerIdComparator> res;
  smatd_list all; // we need a non-sorted list to implement 'recursion'
  const TUnitCell& uc = GetUnitCell();
  res.Add(uc.GetMatrix(0));
  all.AddNew(uc.GetMatrix(0));
  for( size_t i=0; i < all.Count(); i++ )  {
    for( size_t j=0; j < l.Count(); j++ )  {
      TCAtom& a = *l[j];
      for( size_t k=0; k < a.AttachedSiteCount(); k++ )  {
        smatd m = uc.MulMatrix(a.GetAttachedSite(k).matrix, all[i]);
        if( res.AddUnique(m) )
          all.AddNew(m);  // recursion
      }
    }
  }
  return res;
}
//..............................................................................
void TLattice::GetGrowMatrices(smatd_list& res) const {
  const TUnitCell& uc = GetUnitCell();
  const size_t ac = Objects.atoms.Count();
  for( size_t i=0; i < ac; i++ )  {
    TSAtom& sa = Objects.atoms[i];
    if( sa.IsGrown() || !sa.IsAvailable() || !sa.CAtom().IsAvailable() )  continue;
    const TCAtom& ca = sa.CAtom();
    for( size_t j=0; j < ca.AttachedSiteCount(); j++ )  {
      const TCAtom::Site& site = ca.GetAttachedSite(j);
      if( !site.atom->IsAvailable() )  continue;
      const smatd m = uc.MulMatrix(site.matrix, sa.GetMatrix(0));
      bool found = false;
      for( size_t l=0; l < MatrixCount(); l++ )  {
        if( Matrices[l]->GetId() == m.GetId() )  {
          found = true;  
          break;
        }
      }
      if( !found && res.IndexOf(m) == InvalidIndex )
        res.AddCCopy(m);
    }
  }
}
//..............................................................................
void TLattice::DoGrow(const TSAtomPList& atoms, bool GrowShell, TCAtomPList* Template)  {
  RestoreCoordinates();
  // all matrices after MatrixCount are new and has to be used for generation
  const size_t currentCount = MatrixCount();
  const TUnitCell& uc = GetUnitCell();
  OnStructureGrow.Enter(this);
  if( GrowShell )  {
    for( size_t i=0; i < atoms.Count(); i++ )  {
      TSAtom* SA = atoms[i];
      const TCAtom& CA = SA->CAtom();
      for( size_t j=0; j < CA.AttachedSiteCount(); j++ )  {
        const TCAtom::Site& site = CA.GetAttachedSite(j);
        if( !site.atom->IsAvailable() )  continue;
        const smatd m = uc.MulMatrix(site.matrix, atoms[i]->GetMatrix(0));
        smatd *mp;
        bool found = false;
        size_t l; // need to use it later
        for( l=0; l < MatrixCount(); l++ )  {
          if( Matrices[l]->GetId() == m.GetId() )  {
            found = true;  
            mp = Matrices[l];
            break;
          }
        }
        if( !found )
          mp = Matrices.Add(new smatd(m));
        if( Objects.atomRegistry.Find(TSAtom::Ref(site.atom->GetId(), m.GetId())) == NULL )
          GenerateAtom(*site.atom, *mp);
      }
    }
  }
  else  {
  // the fragmens to grow by a particular matrix
    TTypeList<TIntList> Fragments2Grow;
    for( size_t i=0; i < atoms.Count(); i++ )  {
      TSAtom* SA = atoms[i];
      const TCAtom& CA = SA->CAtom();
      for( size_t j=0; j < CA.AttachedSiteCount(); j++ )  {
        const TCAtom::Site& site = CA.GetAttachedSite(j);
        if( !site.atom->IsAvailable() )  continue;
        const smatd m = uc.MulMatrix(site.matrix, atoms[i]->GetMatrix(0));
        bool found = false;
        size_t l; // need to use it later
        for( l=0; l < MatrixCount(); l++ )  {
          if( Matrices[l]->GetId() == m.GetId() )  {
            found = true;  
            break;
          }
        }
        if( !found )  {
          Matrices.Add(new smatd(m));
          Fragments2Grow.Add(new TIntList).Add(site.atom->GetFragmentId());
        }
        else  {
          if( l >= currentCount )  {
            TIntList& ToGrow = Fragments2Grow[l-currentCount];
            if( ToGrow.IndexOf(site.atom->GetFragmentId()) == InvalidIndex )
              ToGrow.Add(site.atom->GetFragmentId());
          }
        }
      }
    }
    for( size_t i = currentCount; i < MatrixCount(); i++ )  {
      smatd* M = Matrices[i];
      TIntList& ToGrow = Fragments2Grow[i-currentCount];
      for( size_t j=0; j < GetAsymmUnit().AtomCount(); j++ )  {
        TCAtom& ca = GetAsymmUnit().GetAtom(j);
        if( ca.IsAvailable()&& ToGrow.IndexOf(ca.GetFragmentId()) != InvalidIndex )
          GenerateAtom(ca, *M);
      }
    }
  }
  RestoreCoordinates();
  Disassemble();
  OnStructureGrow.Exit(this);
}
//..............................................................................
void TLattice::GrowFragments(bool GrowShells, TCAtomPList* Template)  {
  TSAtomPList TmpAtoms;
  const size_t ac = Objects.atoms.Count();
  for( size_t i=0; i < ac; i++ )  {
    TSAtom& A = Objects.atoms[i];
    if( A.IsDeleted() || !A.CAtom().IsAvailable() )  
      continue;
    for( size_t j=0; j < A.NodeCount(); j++ )  {
      if( A.Node(j).IsDeleted() )
        A.NullNode(j);
    }
    A.PackNodes();
    if( !A.IsGrown() )
      TmpAtoms.Add(A);
  }
  if( !TmpAtoms.IsEmpty() )
    GrowAtoms(TmpAtoms, GrowShells, Template);
}
//..............................................................................
void TLattice::GrowAtoms(const TSAtomPList& atoms, bool GrowShells, TCAtomPList* Template)  {
  if( atoms.IsEmpty() )  return;
/* restore atom centres if were changed by some other procedure */
  DoGrow(atoms, GrowShells, Template);
}
//..............................................................................
void TLattice::GrowAtom(TSAtom& Atom, bool GrowShells, TCAtomPList* Template)  {
  if( Atom.IsGrown() )  return;
  TSAtomPList atoms;
/* restore atom centres if were changed by some other procedure */
  atoms.Add(&Atom);
  DoGrow(atoms, GrowShells, Template);
}
//..............................................................................
void TLattice::GrowFragment(uint32_t FragId, const smatd& transform)  {
  smatd *M = NULL;
  // check if the matix is unique
  bool found = false;
  for( size_t i=0; i < Matrices.Count(); i++ )  {
    if( Matrices[i]->GetId() == transform.GetId() )  {
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
    if( ca.IsAvailable() && ca.GetFragmentId() == FragId )
      GenerateAtom(ca, *M);
  }
  RestoreCoordinates();
  Disassemble();
  OnStructureGrow.Exit(this);
}
//..............................................................................
void TLattice::GrowAtoms(const TCAtomPList& atoms, const smatd_list& matrices)  {
  if( atoms.IsEmpty() )  return;
  smatd_plist addedMatrices;
  // check if the matices is unique
  for( size_t i=0; i < matrices.Count(); i++ )  {
    bool found = false;
    for( size_t j=0; j < Matrices.Count(); j++ )  {
      if( Matrices[j]->GetId() == matrices[i].GetId() )  {
        found = true;
        addedMatrices.Add(Matrices[j]);
        break;
      }
    }
    if( !found )
      addedMatrices.Add(Matrices.Add(new smatd(matrices[i])));
  }
  if( addedMatrices.IsEmpty() )  return;
  OnStructureGrow.Enter(this);
  Objects.atoms.IncCapacity(atoms.Count()*addedMatrices.Count());
  for( size_t i=0; i < addedMatrices.Count(); i++ )  {
    for( size_t j=0; j < atoms.Count(); j++ )  {
      if( !atoms[j]->IsDeleted() && atoms[j]->IsAvailable() )
        GenerateAtom(*atoms[j], *addedMatrices[i]);
    }
  }
  RestoreCoordinates();
  Disassemble();
  OnStructureGrow.Exit(this);
}
//..............................................................................
void TLattice::GrowAtom(TCAtom& atom, const smatd& matrix)  {
  // check if the matices is unique
  if( GetAtomRegistry().Find(TSAtom::Ref(atom.GetId(), matrix.GetId())) != NULL )
    return;
  smatd* m = NULL;
  bool found = false;
  for( size_t i=0; i < Matrices.Count(); i++ )  {
    if( Matrices[i]->GetId() == matrix.GetId() )  {
      m = Matrices[i];
      found = true;
      break;
    }
  }
  if( !found )
    m = Matrices.Add(new smatd(matrix));
  OnStructureGrow.Enter(this);
  GenerateAtom(atom, *m);
  RestoreCoordinates();
  Disassemble();
  OnStructureGrow.Exit(this);
}
//..............................................................................
TSAtom& TLattice::GenerateAtom(TCAtom& a, smatd& symop, TNetwork* net)  {
  TSAtom& SA = Objects.atoms.New(net == NULL ? Network : net);
  SA.CAtom(a);
  SA.AddMatrix(&symop);
  SA.ccrd() = symop * SA.ccrd();
  GetAsymmUnit().CellToCartesian(SA.ccrd(), SA.crd());
  SA.SetEllipsoid(&GetUnitCell().GetEllipsoid(symop.GetContainerId(), SA.CAtom().GetId()));
  return SA;
}
//..............................................................................
void TLattice::Grow(const smatd& transform)  {
  smatd *M = NULL;
  // check if the matix is unique
  bool found = false;
  for( size_t i=0; i < Matrices.Count(); i++ )  {
    if( Matrices[i]->GetId() == transform.GetId() )  {
      M = Matrices[i];
      found = true;
      break;
    }
  }
  if( !found )
    M = Matrices.Add(new smatd(transform));
  OnStructureGrow.Enter(this);
  TAsymmUnit& au = GetAsymmUnit();
  const size_t ac = au.AtomCount();
  Objects.atoms.IncCapacity(ac);
  for( size_t i=0; i < ac; i++ )  {
    TCAtom& ca = au.GetAtom(i);
    if( ca.IsAvailable() )
      GenerateAtom(ca, *M);
  }
  RestoreCoordinates();
  Disassemble();
  OnStructureGrow.Exit(this);
}
//..............................................................................
void TLattice::RestoreAtom(const TSAtom::FullRef& id)  {
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
  }
  TSAtom& sa = GenerateAtom(GetAsymmUnit().GetAtom(id.catom_id), *matr);
  sa.CAtom().SetDeleted(false);
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
      }
      sa.AddMatrix(matr);
    }
  }
}
//..............................................................................
TSAtom* TLattice::FindSAtom(const olxstr& Label) const {
  const size_t ac = Objects.atoms.Count();
  for( size_t i=0; i < ac; i++ )
    if( Label.Equalsi(Objects.atoms[i].GetLabel()) )  
      return &Objects.atoms[i];
  return NULL;
}
//..............................................................................
TSAtom* TLattice::FindSAtom(const TCAtom& ca) const {
  const size_t ac = Objects.atoms.Count();
  for( size_t i=0; i < Objects.atoms.Count(); i++ )
    if( ca.GetId() == Objects.atoms[i].CAtom().GetId() )  
      return &Objects.atoms[i];
  return NULL;
}
//..............................................................................
TSAtom* TLattice::NewCentroid(const TSAtomPList& Atoms)  {
  vec3d cc, ce;
  double aan = 0;
  for( size_t i=0; i < Atoms.Count(); i++ )  {
    cc += Atoms[i]->ccrd()*Atoms[i]->CAtom().GetChemOccu();
    ce += vec3d::Qrt(Atoms[i]->CAtom().ccrdEsd())*Atoms[i]->CAtom().GetOccu();
    aan += Atoms[i]->CAtom().GetChemOccu();
  }
  if( aan == 0 )  return NULL;
  ce.Sqrt();
  ce /= aan;
  cc /= aan;
  try  {
    TCAtom& CCent = AsymmUnit->NewCentroid(cc);
    GetUnitCell().AddEllipsoid();
    TSAtom& c = GenerateAtom(CCent, *Matrices[0]);
    CCent.ccrdEsd() = ce;
    GetUnitCell().AddEllipsoid();
    return &c;
  }
  catch(const TExceptionBase& exc)  {
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }
}
//..............................................................................
TSAtom* TLattice::NewAtom(const vec3d& center)  {
  TCAtom* ca = NULL;
  try  { ca = &AsymmUnit->NewAtom();  }
  catch(const TExceptionBase& exc)  {
    throw TFunctionFailedException(__OlxSourceInfo, exc.Replicate());
  }
  ca->ccrd() = center;
  GetUnitCell().AddEllipsoid();
  TSAtom& sa = GenerateAtom(*ca, *Matrices[0]);
  return &sa;
}
//..............................................................................
TSPlanePList TLattice::NewPlane(const TSAtomPList& Atoms, double weightExtent, bool regular)  {
  TSPlane* Plane = TmpPlane(Atoms, weightExtent);
  TSPlanePList rv;
  if( Plane != NULL)  {
    Plane->SetRegular(regular);
    TSPlane::Def pd = Plane->GetDef();
    bool found = false;
    for( size_t i=0; i < PlaneDefs.Count(); i++ )  {
      if( PlaneDefs[i] == pd )  {
        found = true;
        break;
      }
    }
    if( !found )  {
      PlaneDefs.AddCCopy(pd);
      if( IsGenerated() )  {
        delete Plane;
        for( size_t i=0; i < Matrices.Count(); i++ )  {
          TSPlane* p = pd.FromAtomRegistry(Objects, PlaneDefs.Count()-1, Network, *Matrices[i]);
          if( p != NULL )  {
            bool uniq = true;
            for( size_t j=0; j < Objects.planes.Count()-1; j++ )  {
              if( Objects.planes[j].GetCenter().QDistanceTo(p->GetCenter()) < 1e-6 )  {
                uniq = false;
                break;
              }
            }
            if( !uniq )
              Objects.planes.DeleteLast();
            else
              rv.Add(p);
          }
        }
      }
      else  {
        Objects.planes.Attach(*Plane);
        rv.Add(Plane);
        Plane->_SetDefId(PlaneDefs.Count()-1);
      }
    }
    else
      delete Plane;
  }
  return rv;
}
//..............................................................................
TSPlane* TLattice::TmpPlane(const TSAtomPList& atoms, double weightExtent)  {
  if( atoms.Count() < 3 )  return NULL;
  //TODO: need to consider occupancy for disordered groups ...
  TTypeList<AnAssociation2<TSAtom*, double> > Points;
  Points.SetCapacity(atoms.Count());
  if( weightExtent != 0 )  {
    double swg = 0;
    for( size_t i=0; i < atoms.Count(); i++ )  {
      const double wght = pow(atoms[i]->GetType().z, weightExtent);
      Points.AddNew(atoms[i], wght);
      swg += wght;
    }
    // normalise the sum of weights to atoms.Count()...
    const double m = atoms.Count()/swg;
    for( size_t i=0; i < Points.Count(); i++ )
      Points[i].B() *= m;
  }
  else  {
    for( size_t i=0; i < atoms.Count(); i++ )
      Points.AddNew(atoms[i], 1);
  }

  Objects.planes.New(Network).Init(Points);
  return &Objects.planes.Detach(Objects.planes.Count()-1);
}
//..............................................................................
void TLattice::UpdatePlaneDefinitions()  {
  PlaneDefs.ForEach(ACollectionItem::TagSetter<>(0));
  for( size_t i=0; i < Objects.planes.Count(); i++ )  {
    TSPlane& sp = Objects.planes[i];
    if( sp.IsDeleted() || sp.GetDefId() >= PlaneDefs.Count() )  // would be odd
      continue;
    PlaneDefs[sp.GetDefId()].IncTag();
  }
  TSizeList ids(PlaneDefs.Count());
  size_t id=0;
  for( size_t i=0; i < PlaneDefs.Count(); i++ )  {
    if( PlaneDefs[i].GetTag() != 0 )
      ids[i] = id++;
    else
      PlaneDefs.NullItem(i);
  }
  for( size_t i=0; i < Objects.planes.Count(); i++ )  {
    TSPlane& sp = Objects.planes[i];
    if( sp.IsDeleted() || sp.GetDefId() >= PlaneDefs.Count() )  // would be odd
      continue;
    sp._SetDefId(ids[sp.GetDefId()]);
  }
  PlaneDefs.Pack();
}
//..............................................................................
void TLattice::UpdateAsymmUnit()  {
  if( Objects.atoms.IsEmpty() )  return;
  const size_t ac = GetAsymmUnit().AtomCount();
  TArrayList<TSAtomPList> AUAtoms(ac);
  TSizeList del_cnt(ac);
  for( size_t i=0; i < ac; i++ )
    del_cnt[i] = 0;
  const size_t lat_ac = Objects.atoms.Count();
  for( size_t i=0; i < lat_ac; i++ )  {
    TSAtom& sa = Objects.atoms[i];
    if( sa.IsDeleted() )  {
      del_cnt[sa.CAtom().GetId()]++;
      continue;
    }
    AUAtoms[sa.CAtom().GetId()].Add(sa);
  }
  for( size_t i=0; i < ac; i++ )  {  // create lists to store atom groups
    TSAtomPList& l = AUAtoms[i];
    if( del_cnt[i] == 0 && (l.Count() > 1) )  continue;  // nothing to do
    TCAtom& ca = AsymmUnit->GetAtom(i);
    if( l.IsEmpty() )  {  // all atoms are deleted or none generated
      if( !ca.IsDeleted() && ca.IsAvailable() )
        ca.SetDeleted(del_cnt[i] != 0);
      continue;
    }
    else if( l.Count() == 1 )  {  // special case...
      if( l[0]->IsAUAtom() )  continue;
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
void TLattice::MoveFragment(const vec3d& to, TSAtom& fragAtom)  {
  if( IsGenerated() )  {
    TBasicApp::NewLogEntry(logError) << "Cannot perform this operation on grown structure";
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
    TBasicApp::NewLogEntry(logInfo) << "Could not find closest matrix";
}
//..............................................................................
void TLattice::MoveFragment(TSAtom& to, TSAtom& fragAtom)  {
  if( IsGenerated() )  {
    TBasicApp::NewLogEntry(logError) << "Cannot perform this operation on grown structure";
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
      for( size_t i=0; i < Objects.atoms.Count(); i++ )  {
        TSAtom& sa = Objects.atoms[i];
        if( sa.CAtom().GetFragmentId() == fragId )  {
          sa.CAtom().ccrd() = *m * sa.CAtom().ccrd();
          if( sa.CAtom().GetEllipsoid() != NULL ) 
            *sa.CAtom().GetEllipsoid() = GetUnitCell().GetEllipsoid(m->GetContainerId(), sa.CAtom().GetId());
        }
      }
    }
    delete m;
    GetUnitCell().UpdateEllipsoids();
    Uniq();
  }
  else
    TBasicApp::NewLogEntry(logInfo) << "Could not find closest matrix";
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
    OnStructureGrow.Enter(this);
    Matrices.Add(m);
    for( size_t i=0; i < fragAtom.GetNetwork().NodeCount(); i++ )  {
      TSAtom& SA = fragAtom.GetNetwork().Node(i);
      if( SA.IsDeleted() )  continue;
      GenerateAtom(SA.CAtom(), *m, &SA.GetNetwork());
    }
    Disassemble();
    OnStructureGrow.Exit(this);
  }
  else
    TBasicApp::NewLogEntry(logInfo) << "Could not find closest matrix";
}
//..............................................................................
void TLattice::MoveFragmentG(TSAtom& to, TSAtom& fragAtom)  {
  smatd* m = GetUnitCell().GetClosest(to.ccrd(), fragAtom.ccrd(), true);
  if( m != NULL )  {
/* restore atom centres if were changed by some other procedure */
    RestoreCoordinates();
    OnStructureGrow.Enter(this);
    Matrices.Add(m);
    TSAtomPList atoms;
    if( to.CAtom().GetFragmentId() == fragAtom.CAtom().GetFragmentId() )
      atoms.Add(&fragAtom);
    else  // copy whole fragment then
      for( size_t i=0; i < fragAtom.GetNetwork().NodeCount(); i++ )
        atoms.Add(&fragAtom.GetNetwork().Node(i));

    for( size_t i=0; i < atoms.Count(); i++ )  {
      TSAtom* SA = atoms.GetItem(i);
      if( SA->IsDeleted() )  continue;
      GenerateAtom(SA->CAtom(), *m);
    }
    Disassemble();
    OnStructureGrow.Exit(this);
  }
  else
    TBasicApp::NewLogEntry(logInfo) << "Could not find closest matrix";
}
//..............................................................................
void TLattice::MoveToCenter()  {
  if( IsGenerated() )  {
    TBasicApp::NewLogEntry(logError) << "Please note that asymetric unit will not be updated: "
      "the structure is grown";
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
    if( IsGenerated() )  {
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
  if( !IsGenerated() )  {  
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
  if( IsGenerated() || Fragments.Count() < 2 )  return;
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
        if( frag->Node(k).CAtom().IsAttachedTo(fa.CAtom()) )  {
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
template <int run>
size_t TLattice_CompaqAll_Process(TUnitCell& uc, TCAtom& ca, const smatd& matr)  {
  if( run == 1 && ca.GetType() == iQPeakZ )  return 0;
  size_t cnt = 0;
  for( size_t i=0; i < ca.AttachedSiteCount(); i++ )  {
    TCAtom::Site& site = ca.GetAttachedSite(i);
    if( site.atom->GetTag() != 0  )  continue;
    if( !matr.IsFirst() )  {
      cnt++;
      site.matrix = uc.MulMatrix(site.matrix, matr);
    }
    else if( site.atom->GetFragmentId() == ca.GetFragmentId() )  {
      TPSTypeList<double, TCAtom*> sorted_al;
      const TAsymmUnit& au = *ca.GetParent();
      for( size_t j=0; j < site.atom->AttachedSiteCount(); j++ )  {
        TCAtom::Site& st = site.atom->GetAttachedSite(j);
        if( st.atom->GetTag() != 0 || !st.matrix.IsFirst() )  continue;
        sorted_al.Add(au.Orthogonalise(ca.ccrd()-st.atom->ccrd()).Length(), st.atom);
      }
      if( sorted_al.IsEmpty() || sorted_al.GetObject(0) != &ca )
        continue;
    }
    site.atom->SetTag(1);
    site.atom->SetFragmentId(ca.GetFragmentId());
    site.atom->ccrd() = site.matrix*site.atom->ccrd();
    if( site.atom->GetEllipsoid() != NULL )
      *site.atom->GetEllipsoid() = uc.GetEllipsoid(site.matrix.GetContainerId(), site.atom->GetId());
    if( site.atom->IsAvailable() )
      cnt += TLattice_CompaqAll_Process<run>(uc, *site.atom, site.matrix);
  }
  return cnt;
}
void TLattice::CompaqAll()  {
  if( IsGenerated() || Fragments.Count() < 2 )  return;
  TUnitCell& uc = GetUnitCell();
  GetAsymmUnit().GetAtoms().ForEach(ACollectionItem::TagSetter<>(0));
  size_t cnt = 0;
  for( size_t i=0; i < Objects.atoms.Count(); i++ )  {
    TSAtom& sa = Objects.atoms[i];
    if( sa.CAtom().GetTag() != 0 || !sa.CAtom().IsAvailable() )
      continue;
    cnt += TLattice_CompaqAll_Process<1>(uc, sa.CAtom(), uc.GetMatrix(0));
  }
  // prcess Q-peaks
  for( size_t i=0; i < Objects.atoms.Count(); i++ )  {
    TSAtom& sa = Objects.atoms[i];
    if( sa.CAtom().GetTag() != 0 || !sa.CAtom().IsAvailable() )
      continue;
    cnt += TLattice_CompaqAll_Process<2>(uc, sa.CAtom(), uc.GetMatrix(0));
  }
  OnStructureUniq.Enter(this);
  TActionQueueLock __queuelock(&OnStructureUniq);
  Init();
  if( cnt != 0 )
    MoveToCenter();
  __queuelock.Unlock();
  OnStructureUniq.Exit(this);
}
//..............................................................................
void TLattice::CompaqClosest()  {
  if( IsGenerated() || Fragments.Count() < 2 )  return;
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
void TLattice::CompaqType(short type)  {
  if( IsGenerated() )  return;
  const size_t ac = Objects.atoms.Count();
  const TAsymmUnit& au = GetAsymmUnit();
  for( size_t i=0; i < ac; i++ )  {
    TSAtom& sa = Objects.atoms[i];
    if( sa.GetType() != type )  continue;
    smatd* transform = NULL;
    double minQD = 1000;
    for( size_t j=0; j < ac; j++ )  {
      TSAtom& sb = Objects.atoms[j];
      if( sb.GetType() == type || sb.GetType() == iQPeakZ )  continue;
      double qd = 0;
      smatd* m = GetUnitCell().GetClosest(sb.ccrd(), sa.ccrd(), true, &qd);
      if( qd < minQD )  {
        if( transform != NULL )
          delete transform;
        transform = m;
        minQD = qd;
      }
      else if( m != NULL )
        delete m;
    }
    if( transform == NULL )  continue;
    sa.ccrd() = sa.CAtom().ccrd() = (*transform * sa.ccrd());
    au.CellToCartesian(sa.CAtom().ccrd(), sa.crd());
    if( sa.CAtom().GetEllipsoid() != NULL )
      *sa.CAtom().GetEllipsoid() =
        GetUnitCell().GetEllipsoid(transform->GetContainerId(), sa.CAtom().GetId());
    delete transform;
  }
  OnStructureUniq.Enter(this);
  Init();
  OnStructureUniq.Exit(this);
  //RestoreADPs(false);
  //UpdateConnectivity();
}
//..............................................................................
void TLattice::TransformFragments(const TSAtomPList& fragAtoms, const smatd& transform)  {
  if( IsGenerated() )  {
    TBasicApp::NewLogEntry(logError) << "Cannot perform this operation on grown structure";
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
  UpdateAsymmUnit();
  Disassemble(false);
}
//..............................................................................
void TLattice::UpdateConnectivityInfo()  {
  GetAsymmUnit()._UpdateConnInfo();
  GetUnitCell().FindSymmEq();
  UpdateAsymmUnit();
  Disassemble(false);
}
//..............................................................................
void TLattice::Disassemble(bool create_planes)  {
  if( Objects.atoms.IsEmpty() )  return;
  OnDisassemble.Enter(this);
  // clear bonds & fargments
  ClearBonds();
  ClearFragments();
  TArrayList<vec3d> ocrd(Objects.atoms.Count());
  GenerateBondsAndFragments(&ocrd);
  if( create_planes )
    BuildPlanes();
  OnDisassemble.Exit(this);
}
//..............................................................................
void TLattice::RestoreCoordinates()  {
  const size_t ac = Objects.atoms.Count();
  for( size_t i=0; i < ac; i++ )  {
    TSAtom& sa = Objects.atoms[i];
    GetAsymmUnit().CellToCartesian(sa.ccrd(), sa.crd());
  }
}
//..............................................................................
bool TLattice::_AnalyseAtomHAdd(AConstraintGenerator& cg, TSAtom& atom,
  TSAtomPList& ProcessingAtoms, int part, TCAtomPList* generated)
{
  if( ProcessingAtoms.IndexOf(atom) != InvalidIndex ||
      (atom.CAtom().IsHAttached() && part == DefNoPart) )
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
      ProcessingAtoms.Remove(atom);
      for( size_t i=0; i < parts.Count(); i++ )  {
        _AnalyseAtomHAdd(cg, atom, ProcessingAtoms, parts[i], &gen_atoms.AddNew());
        TCAtomPList& gen = gen_atoms.GetLast();
        for( size_t j=0; j < gen.Count(); j++ )  {
          gen[j]->SetPart(parts[i]);
          rm->Vars.SetParam(*gen[j], catom_var_name_Sof, occu[i]);
        }
      }
      cg.AnalyseMultipart(AE, gen_atoms);
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
        TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": XCH";
        cg.FixAtom(AE, fgCH1, h_elm, NULL, generated);
      }
      else  {
        if( d > 1.35 )  {
          TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": XCH3";
          cg.FixAtom(AE, fgCH3, h_elm, NULL, generated);
        }
        else  {
          if( d < 1.25 )  {
            if( NAE.Count() > 1 ) {
              TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": X=CH2";
              cg.FixAtom(AE, fgCH2, h_elm, NULL, generated);
            }
            else
              TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": possibly X=CH2";
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
        TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": XYCH2";
        cg.FixAtom(AE, fgCH2, h_elm, NULL, generated);
      }
      else  {
        if( (d1 < db || d2 < db) && v < 160 )  {
          TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": X(Y=C)H";
          cg.FixAtom(AE, fgCH1, h_elm, NULL, generated);
        }
      }
    }
    else if( AE.Count() == 3 )  {
      double v = olx_tetrahedron_volume( atom.crd(), AE.GetCrd(0), AE.GetCrd(1), AE.GetCrd(2) );
      if( v > 0.3 )  {
        TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": XYZCH";
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
        TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": R5CH";
        cg.FixAtom(AE, fgBH1, h_elm, NULL, generated);
      }
    }
  }
  else if( atom.GetType() == iNitrogenZ )  {  // nitrogen
    if( AE.Count() == 1 )  {
      double d = AE.GetCrd(0).DistanceTo(atom.crd());
      if( d > 1.35 )  {
        if( d > 1.44 )  {
          TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": XNH3";
          cg.FixAtom(AE, fgNH3, h_elm, NULL, generated);
        }
        else  {
          TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": XNH2";
          cg.FixAtom(AE, fgNH2, h_elm, NULL, generated);
        }
      }
      else  if( d > 1.2 )  {  //else nitrile
        // have to check if double bond
        TSAtom* A = FindSAtom(AE.GetCAtom(0));
        TAtomEnvi NAE;
        if( A == 0 )
          throw TFunctionFailedException(__OlxSourceInfo, olxstr("Could not locate atom ") << AE.GetLabel(0) );

        UnitCell->GetAtomEnviList(*A, NAE, false, part);
        NAE.Exclude(atom.CAtom());

        if( A->GetType() == iCarbonZ && NAE.Count() > 1 )  {
          vec3d a = NAE.GetCrd(0);
          a -= NAE.GetBase().crd();
          vec3d b = AE.GetBase().crd();
          b -= NAE.GetBase().crd();

          d = a.CAngle(b);
          d = acos(d)*180/M_PI;
          if( d > 115 && d < 130 )  {
            TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": X=NH2";
            cg.FixAtom(AE, fgNH2, h_elm, &NAE, generated);
          }
        }
        else  {
          TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": X=NH";
          cg.FixAtom(AE, fgNH1, h_elm, NULL, generated);
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
      double d1 = AE.GetCrd(0).DistanceTo(atom.crd());
      double d2 = AE.GetCrd(1).DistanceTo(atom.crd());
      if( d1 > 1.72 || d2 > 1.72 )  {  // coordination?
        if( v > 165 )  // skip ..
          TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": RN->M";
        else if( (d1 < 1.5 && d1 > 1.35) || (d2 < 1.5 && d2 > 1.35) )  {
          TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": RNH(2)M";
          cg.FixAtom(AE, fgNH2, h_elm, NULL, generated);
        }
        else if( d1 > 1.72 && d2 > 1.72 )  {
          TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": XX'NH";
          cg.FixAtom(AE, fgNH1, h_elm, NULL, generated);
        }
      }
      else if( v < 120 && d1 > 1.45 && d2 > 1.45 )  {
        TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": R2NH2+";
        cg.FixAtom(AE, fgNH2, h_elm, NULL, generated);
      }
      else if( v < 120 && (d1 < 1.3 || d2 < 1.3) )
        ;
      else  {
        if( (d1+d2) > 2.70 )  {
          TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": XYNH";
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
          TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": R2HN->M";
          cg.FixAtom(AE, fgNH1, h_elm, NULL, generated);
        }
        else  {
          TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": R3NH+";
          cg.FixAtom(AE, fgNH1, h_elm, NULL, generated);
        }
      }
    }
  }
  if( atom.GetType() == iOxygenZ )  {  // oxygen
    if( AE.IsEmpty() )  {
      TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": OH2";
      TAtomEnvi pivoting;
      UnitCell->GetAtomPossibleHBonds(AE, pivoting);
      RemoveNonHBonding( pivoting );
      cg.FixAtom(AE, fgOH2, h_elm, &pivoting, generated);
    }
    else if( AE.Count() == 1 )  {
      const double d = AE.GetCrd(0).DistanceTo(atom.crd());
      if( d > 1.3 )   {  // otherwise a doubl bond
        if( AE.GetType(0) == iChlorineZ )
          ;
        else  if( AE.GetType(0) == iCarbonZ )  {  // carbon
          TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": COH";
          TAtomEnvi pivoting;
          UnitCell->GetAtomPossibleHBonds(AE, pivoting);
          RemoveNonHBonding(pivoting);
          cg.FixAtom(AE, fgOH1, h_elm, &pivoting, generated);
        }
        else  if( AE.GetType(0) == iSulphurZ )  {
          if( d > 1.48 )  {
            TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": SOH";
            cg.FixAtom(AE, fgOH1, h_elm, NULL, generated);
          }
        }
        else  if( AE.GetType(0) == iPhosphorusZ )  {
          if( d > 1.54 )  {
            TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": POH";
            TAtomEnvi pivoting;
            UnitCell->GetAtomPossibleHBonds(AE, pivoting);
            RemoveNonHBonding(pivoting);
            cg.FixAtom(AE, fgOH1, h_elm, &pivoting, generated);
          }
        }
        else  if( AE.GetType(0) == iSiliconZ )  {
          if( d > 1.6 )  {
            TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": SiOH";
            TAtomEnvi pivoting;
            UnitCell->GetAtomPossibleHBonds(AE, pivoting);
            RemoveNonHBonding(pivoting);
            cg.FixAtom(AE, fgOH1, h_elm, &pivoting, generated);
          }
        }
        else  if( AE.GetType(0) == iBoronZ )  {
          if( d < 1.38 )  {
            TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": B(III)OH";
            TAtomEnvi pivoting;
            UnitCell->GetAtomPossibleHBonds(AE, pivoting);
            RemoveNonHBonding(pivoting);
            cg.FixAtom(AE, fgOH1, h_elm, &pivoting, generated);
          }
        }
        else  if( AE.GetType(0) == iNitrogenZ )  {
          if( d > 1.37 )  {
            TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": NOH";
            TAtomEnvi pivoting;
            UnitCell->GetAtomPossibleHBonds(AE, pivoting);
            RemoveNonHBonding(pivoting);
            cg.FixAtom(AE, fgOH1, h_elm, &pivoting, generated);
          }
        }
        else if( d > 1.8 )  {  // coordination bond?
          TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": possibly M-OH2";
          cg.FixAtom(AE, fgOH2, h_elm, NULL, generated);
        }
      }
    }
    else if( AE.Count() == 2 )  {
      const double d1 = AE.GetCrd(0).DistanceTo(atom.crd());
      const double d2 = AE.GetCrd(1).DistanceTo(atom.crd());
      if( (d1 > 1.8 && d2 < 1.8 && d2 > 1.38) || (d2 > 1.8 && d1 < 1.8 && d1 > 1.38) )  {
        TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": possibly M-O(H)R";
        cg.FixAtom(AE, fgOH1, h_elm, NULL, generated);
      }
    }
  }
  else if( atom.GetType() == iBoronZ )  {  // boron
    if( AE.Count() == 3 )  {
      const vec3d cnt = AE.GetBase().crd();
      const double v = olx_tetrahedron_volume( 
        cnt, 
        (AE.GetCrd(0)-cnt).Normalise() + cnt, 
        (AE.GetCrd(1)-cnt).Normalise() + cnt, 
        (AE.GetCrd(2)-cnt).Normalise() + cnt);
      if( v > 0.1 )  {
        TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": XYZBH";
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
        TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": X4BH";
        cg.FixAtom(AE, fgBH1, h_elm, NULL, generated);
      }
    }
    else if( AE.Count() == 5 )  {
      TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": X5BH";
      cg.FixAtom(AE, fgBH1, h_elm, NULL, generated);
    }
  }
  else if( atom.GetType() == iSiliconZ )  {
    if( AE.Count() == 3 )  {
      const vec3d cnt = AE.GetBase().crd();
      const double v = olx_tetrahedron_volume( 
        cnt, 
        (AE.GetCrd(0)-cnt).Normalise() + cnt, 
        (AE.GetCrd(1)-cnt).Normalise() + cnt, 
        (AE.GetCrd(2)-cnt).Normalise() + cnt);
      if( v > 0.1 )  {
        TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": XYZSiH";
        cg.FixAtom(AE, fgSiH1, h_elm, NULL, generated);
      }
    }
    else if( AE.Count() == 2 )  {  // no validation yet...
      TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": XYSiH2";
      cg.FixAtom(AE, fgSiH2, h_elm, NULL, generated);
    }
  }
  else if( atom.GetType() == iSulphurZ )  {
    if( AE.Count() == 1 && AE.GetType(0) == iCarbonZ )  {
      double d = AE.GetCrd(0).DistanceTo(atom.crd());
      if( d > 1.72 )  {
        TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": CSH";
        cg.FixAtom(AE, fgSH1, h_elm, NULL, generated);
      }
    }
  }
  ProcessingAtoms.Delete(ProcessingAtoms.IndexOf(&atom));
  return true;
}
//..............................................................................
void TLattice::_ProcessRingHAdd(AConstraintGenerator& cg, const ElementPList& rcont, const TSAtomPList& atoms) {
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
          try  {
            const double v = olx_tetrahedron_volume( 
              cnt, 
              (AE.GetCrd(0)-cnt).Normalise() + cnt, 
              (AE.GetCrd(1)-cnt).Normalise() + cnt, 
              (AE.GetCrd(2)-cnt).Normalise() + cnt);
            if( v < 0.1 )  continue;  // coordination or substituted
          }
          catch(...)  {  continue;  }
        }
        for( size_t k=0; k < AE.Count(); k++ )  {
          if( (AE.GetCrd(k) - rings[i][j]->crd()).QLength() > 4.0 )
            AE.Delete(k--);
        }
        if( AE.Count() == 2 && rings[i][j]->GetType() == iCarbonZ && atoms.IndexOf(AE.GetBase()) != InvalidIndex )  {
          TBasicApp::NewLogEntry(logInfo) << rings[i][j]->GetLabel() << ": X(Y=C)H (ring)";
          cg.FixAtom(AE, fgCH1, h_elm);
          rings[i][j]->CAtom().SetHAttached(true);
        }
      }
    }
  }
}
//..............................................................................
void TLattice::AnalyseHAdd(AConstraintGenerator& cg, const TSAtomPList& atoms)  {
  if( IsGenerated() )  {
    TBasicApp::NewLogEntry(logError) << "Hadd is not applicable to grown structure";
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
  _ProcessRingHAdd(cg, rcont, atoms); // Cp
  rcont.Add(rcont[0]);
  _ProcessRingHAdd(cg, rcont, atoms); // Ph
  rcont.GetLast() = CTypes[1];
  _ProcessRingHAdd(cg, rcont, atoms); // Py

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
    for( size_t j=0; j < atoms[i]->CAtom().AttachedSiteCount(); j++ )  {
      if( atoms[i]->CAtom().GetAttachedAtom(j).GetType() == iHydrogenZ &&
        !atoms[i]->CAtom().GetAttachedAtom(j).IsDeleted() )
      {
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
        const double d = AE.GetCrd(0).DistanceTo(SA->crd());
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
    for( size_t i=0; i < Envi.Count(); i++ )  {
      double d = Envi.GetBase().crd().DistanceTo(Envi.GetCrd(i));
      if( Envi.GetMatrix(i).IsFirst() && // prioritise sligtly longer intramolecular bonds
        Envi.GetBase().CAtom().GetFragmentId() == Envi.GetCAtom(i).GetFragmentId() )
      {
        d -= 0.15;
      }
      hits.Add(d, &Envi.GetCAtom(i));
    }

    while( hits.Count() > 1 &&
      ((hits.GetKey(hits.Count()-1) - hits.GetKey(0)) > 0.05) )  {
      Envi.Exclude(*hits.GetObject(hits.Count()-1));
      hits.Delete(hits.Count()-1);
    }
  }
  // all similar length  .... Q peaks might help :)
  if( Envi.Count() > 1 )  {
    TPSTypeList<double, TCAtom*> hits;
    AE.Clear();
    UnitCell->GetAtomQEnviList(Envi.GetBase(), AE);
    for( size_t i=0; i < AE.Count(); i++ )  {
//      v1 = AE.GetCrd(i);
//      v1 -= Envi.GetBase()->crd();
      const double d = Envi.GetBase().crd().DistanceTo(AE.GetCrd(i));
      if( d < 0.7 || d > 1.3 )
        AE.Exclude(AE.GetCAtom(i--));
    }
    if( AE.IsEmpty() || AE.Count() > 1 )  return;
    vec3d vec1 = AE.GetCrd(0) - Envi.GetBase().crd();
    for( size_t i=0; i < Envi.Count(); i++ )  {
      vec3d vec2 = Envi.GetCrd(i) - Envi.GetBase().crd();
      hits.Add(olx_abs(-1 + vec2.CAngle(vec1)), &Envi.GetCAtom(i));
    }
    while( hits.Count() > 1 )  {
      Envi.Exclude( *hits.GetObject( hits.Count() - 1 ) );
      hits.Delete(hits.Count() - 1);
    }
  }

}
//..............................................................................
void TLattice::SetAnis(const TCAtomPList& atoms, bool anis)  {
  if( !anis )  {
    for( size_t i=0; i < atoms.Count(); i++ )  {
      if( olx_is_valid_index(atoms[i]->GetEllpId()) )  {
         GetAsymmUnit().NullEllp(atoms[i]->GetEllpId());
         atoms[i]->AssignEllp(NULL);
      }
    }
    GetAsymmUnit().PackEllps();
  }
  else  {
    evecd ee(6);
    for( size_t i=0; i < atoms.Count(); i++ )  {
      if( atoms[i]->GetEllipsoid() == NULL)  {
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
  for( size_t i=0; i < Objects.bonds.Count(); i++ )  {
    if( Objects.bonds[i].IsDeleted() )  continue;
    Objects.bonds[i].SetTag(sbond_tag++);
  }
  // initialise atom tags
  size_t satom_tag = 0;
  for( size_t i=0; i < Objects.atoms.Count(); i++ )  {
    if( Objects.atoms[i].IsDeleted() )  continue;
    Objects.atoms[i].SetTag(satom_tag++);
  }
  // initialise fragment tags
  size_t frag_tag = 0;
  Network->SetTag(-1);
  for( size_t i=0; i < Fragments.Count(); i++ )
    Fragments[i]->SetTag(frag_tag++);
  // save satoms - only the original CAtom Tag and the generating matrix tag
  TDataItem& atoms = item.AddItem("Atoms");
  for( size_t i=0; i < Objects.atoms.Count(); i++ )  {
    if( !Objects.atoms[i].IsDeleted() )
      Objects.atoms[i].ToDataItem(atoms.AddItem("Atom"));
  }
  // save bonds
  TDataItem& bonds = item.AddItem("Bonds");
  for( size_t i=0; i < Objects.bonds.Count(); i++ )  {
    if( !Objects.bonds[i].IsDeleted() )
      Objects.bonds[i].ToDataItem(bonds.AddItem("Bond"));
  }
  // save fragments
  TDataItem& frags = item.AddItem("Fragments");
  for( size_t i=0; i < Fragments.Count(); i++ )
    Fragments[i]->ToDataItem(frags.AddItem("Fragment"));
  // restore original matrix tags 
  for( size_t i=0; i < mat_c; i++ )
    Matrices[i]->SetRawId(m_tags[i]);
  // save planes
  TSPlanePList valid_planes;
  for( size_t i=0; i < Objects.planes.Count(); i++ )  {
    if( Objects.planes[i].IsDeleted() ) continue;
    size_t p_ac = 0;  
    for( size_t j=0; j < Objects.planes[i].Count(); j++ ) 
      if( Objects.planes[i].GetAtom(j).IsAvailable() )
        p_ac++;
    if( p_ac >= 3 ) // a plane must contain at least three atoms
      valid_planes.Add(Objects.planes[i]);
    else
      Objects.planes[i].SetDeleted(true);
  }
  TDataItem& planes = item.AddItem("Planes");
  for( size_t i=0; i < valid_planes.Count(); i++ )
    valid_planes[i]->ToDataItem(planes.AddItem("Plane"));
}
//..............................................................................
void TLattice::FromDataItem(TDataItem& item)  {
  TActionQueueLock ql(&OnAtomsDeleted);
  Clear(true);
  Delta = item.GetRequiredField("delta").ToDouble();
  DeltaI = item.GetRequiredField("deltai").ToDouble();
  GetAsymmUnit().FromDataItem(item.FindRequiredItem("AUnit"));
  GetUnitCell().InitMatrices();
  const TDataItem& mat = item.FindRequiredItem("Matrices");
  Matrices.SetCapacity(mat.ItemCount());
  for( size_t i=0; i < mat.ItemCount(); i++ )  {
    smatd* m = new smatd;
    TSymmParser::SymmToMatrix(mat.GetItem(i).GetValue(), *m);
    GetUnitCell().InitMatrixId(*Matrices.Add(m));
    m->SetRawId(mat.GetItem(i).GetRequiredField("id").ToUInt());
  }
  // precreate fragments
  const TDataItem& frags = item.FindRequiredItem("Fragments");
  Fragments.SetCapacity(frags.ItemCount());
  for( size_t i=0; i < frags.ItemCount(); i++ )
    Fragments.Add(new TNetwork(this, NULL));
  // precreate bonds
  const TDataItem& bonds = item.FindRequiredItem("Bonds");
  Objects.bonds.IncCapacity(bonds.ItemCount());
  for( size_t i=0; i < bonds.ItemCount(); i++ )
    Objects.bonds.New(NULL);
  // precreate and load atoms
  const TDataItem& atoms = item.FindRequiredItem("Atoms");
  Objects.atoms.IncCapacity(atoms.ItemCount());
  for( size_t i=0; i < atoms.ItemCount(); i++ )
    Objects.atoms.New(NULL);
  for( size_t i=0; i < atoms.ItemCount(); i++ )
    Objects.atoms[i].FromDataItem(atoms.GetItem(i), *this);
  // load bonds
  for( size_t i=0; i < bonds.ItemCount(); i++ )
    Objects.bonds[i].FromDataItem(bonds.GetItem(i), *this);
  // load fragments
  for( size_t i=0; i < frags.ItemCount(); i++ )
    Fragments[i]->FromDataItem(frags.GetItem(i));
  TDataItem& planes = item.FindRequiredItem("Planes");
  for( size_t i=0; i < planes.ItemCount(); i++ )  {
    TSPlane& p = Objects.planes.New(Network);
    p.FromDataItem(planes.GetItem(i));
    TSPlane::Def def = p.GetDef();
    size_t di = InvalidIndex;
    for( size_t j=0; j < PlaneDefs.Count(); j++ )  {
      if( PlaneDefs[j] == def )  {
        di = j;
        break;
      }
    }
    if( di == InvalidIndex )  {
      p._SetDefId(PlaneDefs.Count());
      PlaneDefs.AddNew(def);
    }
    else
      p._SetDefId(di);
  }
  GetAsymmUnit()._UpdateConnInfo();
  GetUnitCell().FindSymmEq();
  for( size_t i=0; i < GetAsymmUnit().AtomCount(); i++ )
    GetAsymmUnit().GetAtom(i).SetDeleted(false);
  BuildAtomRegistry();
}
//..............................................................................
void TLattice::SetGrowInfo(GrowInfo* grow_info)  {
  if( _GrowInfo != NULL )
    delete _GrowInfo;
  _GrowInfo = grow_info;
}
//..............................................................................
TLattice::GrowInfo* TLattice::GetGrowInfo() const {
  if( !IsGenerated() )  return NULL;
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

  gi.info.SetCount(au.AtomCount());
  const size_t ac = Objects.atoms.Count();
  for( size_t i=0; i < ac; i++ )  {
    TSAtom& sa = Objects.atoms[i];
    TIndexList& mi = gi.info[sa.CAtom().GetId()];
    const size_t mi_cnt = mi.Count();
    mi.SetCount(mi_cnt + sa.MatrixCount()+1);
    mi[mi_cnt] = -(int)sa.MatrixCount(); // separator field
    for( size_t j=1; j <= sa.MatrixCount(); j++ )
      mi[mi_cnt+j] = sa.GetMatrix(j-1).GetId();
  }
  // restore matrix tags
  for( size_t i=0; i < mtags.Count(); i++ )
    Matrices[i]->SetRawId(mtags[i]);
  return &gi;
}
//..............................................................................
bool TLattice::ApplyGrowInfo()  {
  TAsymmUnit& au = GetAsymmUnit();
  if( _GrowInfo == NULL || !Objects.atoms.IsEmpty() || !Matrices.IsEmpty() || 
    GetUnitCell().MatrixCount() != _GrowInfo->unc_matrix_count )
  {
    if( _GrowInfo != NULL )  {
      delete _GrowInfo;
      _GrowInfo = NULL;
    }
    return false;
  }
  Matrices.Assign(_GrowInfo->matrices);
  _GrowInfo->matrices.Clear();
  Objects.atoms.IncCapacity(au.AtomCount()*Matrices.Count());
  for( size_t i=0; i < au.AtomCount(); i++ )    {
    TCAtom& ca = GetAsymmUnit().GetAtom(i);
    // we still need masked and detached atoms here
    if( ca.IsDeleted() )  continue;
    if( i >= _GrowInfo->info.Count() )  {  // create just with I matrix
      GenerateAtom(ca, *Matrices[0]);
      continue;
    }
    const TIndexList& mi = _GrowInfo->info[i];
    for( size_t j=0; j < mi.Count(); j++ )  {
      if( mi[j] < 0 )  {
        const size_t matr_cnt = olx_abs(mi[j]),
          matr_start = j+1; 
        TSAtom& a = GenerateAtom(ca, *Matrices[mi[matr_start]]);
        for( size_t k=matr_start+1; k < matr_start+matr_cnt; k++, j++ )
          a.AddMatrix(Matrices[mi[k]]);
      }
    }
  }
  delete _GrowInfo;
  _GrowInfo = NULL;
  return true;
}
//..............................................................................
void TLattice::_CreateFrags(TCAtom& start, TCAtomPList& dest)  {
  start.SetTag(1);
  dest.Add(start);
  for( size_t i=0; i < start.AttachedSiteCount(); i++ )  {
    const TCAtom::Site& site = start.GetAttachedSite(i);
    if( site.atom->GetTag() != 0 )  continue;
    _CreateFrags(*site.atom, dest);
  }
}
//..............................................................................
olxstr TLattice::CalcMoiety() const {
  const TAsymmUnit& au = GetAsymmUnit();
  TTypeList<TCAtomPList> cfrags;
  for( size_t i=0; i < au.AtomCount(); i++ )  {
    if( au.GetAtom(i).IsDeleted() || au.GetAtom(i).GetType() == iQPeakZ )
      au.GetAtom(i).SetTag(1);  // ignore
    else
      au.GetAtom(i).SetTag(0); // unprocessed
  }
  for( size_t i=0; i < au.AtomCount(); i++ )  {
    if( au.GetAtom(i).GetTag() == 0 )
      _CreateFrags(au.GetAtom(i), cfrags.AddNew());
  }
  // multiplicity,content, reference fragment index
  TTypeList<AnAssociation3<double,ContentList, size_t> > frags;
  for( size_t i=0; i < cfrags.Count(); i++ )  {
    ElementDict _cld;
    for( size_t j=0; j < cfrags[i].Count(); j++ )
      _cld.Add(&cfrags[i][j]->GetType(), 0) += cfrags[i][j]->GetOccu();
    ContentList cl(_cld.Count());
    for( size_t j=0; j < _cld.Count(); j++ )
      cl.Set(j, new ElementCount(*_cld.GetKey(j), _cld.GetValue(j)));
    XElementLib::SortContentList(cl);
    bool uniq = true;
    double wght=0, overall_occu = 0;
    for( size_t j=0; j < cfrags[i].Count(); j++ )  {
      const double occu = cfrags[i][j]->GetOccu();
      if( overall_occu == 0 )
        overall_occu = occu;
      else if( overall_occu != -1 && olx_abs(overall_occu-occu) > 0.01 )  {
        overall_occu = -1;
        break;
      }
    }
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
        frags[j].A() += cl[0].count/frags[j].GetB()[0].count;
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
  // apply Z multiplier...
  const double zp_mult = (double)GetUnitCell().MatrixCount()/olx_max(au.GetZ(), 1);
  if( zp_mult != 1 )  {
    for( size_t i=0; i < frags.Count(); i++ )  {
      const TCAtomPList& l = cfrags[frags[i].GetC()];
      const size_t generators = GetFragmentGrowMatrices(l).Count();
      const int gd = int(generators == 0 ? 1 : generators);
      frags[i].A() *= zp_mult/gd;
      for( size_t j=0; j < frags[i].GetB().Count(); j++ )
        frags[i].B()[j].count *= gd;
    }
  }
  olxstr rv;
  for( size_t i=0; i < frags.Count(); i++ )  {
    if( !rv.IsEmpty() )  rv << ", ";
    if( frags[i].GetA() != 1 )
      rv << olx_round(frags[i].GetA(), 100) << '(';
    for( size_t j=0; j < frags[i].GetB().Count(); j++ )  {
      rv << frags[i].GetB()[j].element.symbol;
      if( frags[i].GetB()[j].count != 1 )
        rv << olx_round(frags[i].GetB()[j].count, 100);
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
  const size_t ac = Objects.atoms.Count();
  for( size_t i=0; i < ac; i++ )  {
    TSAtom& sa = Objects.atoms[i];
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
void TLattice::BuildAtomRegistry()  {
  if( Matrices.IsEmpty() )  return;
  TUnitCell& uc = GetUnitCell();
  vec3i mind(100,100,100), maxd(-100,-100,-100);
  for( size_t i=0; i < Matrices.Count(); i++ )
    vec3i::UpdateMinMax(Matrices[i]->GetT(Matrices[i]->GetId()), mind, maxd);
  maxd[0] += 1;  maxd[1] += 1;  maxd[2] += 1;
  AtomRegistry::RegistryType& registry = Objects.atomRegistry.Init(mind, maxd);
  size_t cnt=0;
  const size_t ac = Objects.atoms.Count();
  for( size_t i=0; i < ac; i++ )  {
    TSAtom* sa = &Objects.atoms[i];
    if( !sa->IsAvailable() || sa->CAtom().IsMasked() )  continue;
    for( size_t mi=0; mi < sa->MatrixCount(); mi++ )  {
      const smatd& matr = sa->GetMatrix(mi);
      const vec3i t = smatd::GetT(matr.GetId());
      TArrayList<TSAtomPList*>* aum_slice = registry.Value(t);
      for( size_t j=0; j < sa->CAtom().EquivCount(); j++ )  {
        const smatd m = uc.MulMatrix(sa->CAtom().GetEquiv(j), matr);
        TSAtom* sa1 = Objects.atomRegistry.Find(TSAtom::Ref(sa->CAtom().GetId(), m.GetId()));
        if( sa1 != NULL && sa1 != sa && !sa1->IsDeleted() )  {
          sa1->AddMatrices(*sa);
          sa->SetDeleted(true);
          sa = sa1;
          break;
        }
      }
      if( aum_slice == NULL )  {
        const size_t matr_cnt = GetUnitCell().MatrixCount();
        aum_slice = (registry.Value(t) = new TArrayList<TSAtomPList*>(matr_cnt));
        for( size_t j=0; j < matr_cnt; j++ )
          (*aum_slice)[j] = NULL;
      }
      TSAtomPList* au_slice = (*aum_slice)[matr.GetContainerId()];
      if( au_slice == NULL )  {
        const size_t atom_cnt = GetAsymmUnit().AtomCount();
        au_slice = ((*aum_slice)[matr.GetContainerId()] = new TSAtomPList(atom_cnt));
        for( size_t j=0; j < atom_cnt; j++)
          (*au_slice)[j] = NULL;
      }
      if( (*au_slice)[sa->CAtom().GetId()] != NULL && (*au_slice)[sa->CAtom().GetId()] != sa )
        (*au_slice)[sa->CAtom().GetId()]->SetDeleted(true);
      (*au_slice)[sa->CAtom().GetId()] = sa;
    }
  }
}
//..............................................................................
void TLattice::AddLatticeContent(const TLattice& latt)  {
  if( latt.IsGenerated() )
    throw TInvalidArgumentException(__OlxSourceInfo, "cannot adopt grown structure");
  TSAtomPList new_atoms;
  TSBondPList new_bonds;
  for( size_t i=0; i < latt.Objects.atoms.Count(); i++ )  {
    const TSAtom& src_a = latt.Objects.atoms[i];
    TCAtom& ca = GetAsymmUnit().NewAtom();
    GetAsymmUnit().CartesianToCell(ca.ccrd() = src_a.crd());
    ca.SetType(src_a.GetType());
    ca.SetLabel(src_a.GetLabel(), false);
    TSAtom* sa = new_atoms.Add(Objects.atoms.New(Network));
    sa->CAtom(ca);
    sa->crd() = GetAsymmUnit().Orthogonalise(sa->ccrd());
    sa->AddMatrix(Matrices[0]);
  }
  for( size_t i=0; i < latt.Objects.bonds.Count(); i++ )  {
    const TSBond& src_b = latt.Objects.bonds[i];
    TSBond* sb = new_bonds.Add(Objects.bonds.New(Network));
    sb->SetA(*new_atoms[src_b.A().GetOwnerId()]);
    sb->SetB(*new_atoms[src_b.B().GetOwnerId()]);
  }
  for( size_t i=0; i < latt.Objects.atoms.Count(); i++ )  {
    const TSAtom& src_a = latt.Objects.atoms[i];
    TSAtom& sa = *new_atoms[i];
    for( size_t j=0; j < src_a.NodeCount(); j++ )
      sa.AddNode(*new_atoms[src_a.Node(j).GetOwnerId()]);
    for( size_t j=0; j < src_a.BondCount(); j++ )
      sa.AddBond(*new_bonds[src_a.Bond(j).GetOwnerId()]);
  }
  for( size_t i=0; i < latt.FragmentCount(); i++ )  {
    const TNetwork& src_n = latt.GetFragment(i);
    TNetwork& net = *Fragments.Add(new TNetwork(this, Network));
    net.SetOwnerId(Fragments.Count()-1);
    for( size_t j=0; j < src_n.NodeCount(); j++ )  {
      TSAtom& a = *new_atoms[src_n.Node(j).GetOwnerId()];
      net.AddNode(a);
      a.SetNetwork(net);
    }
    for( size_t j=0; j < src_n.BondCount(); j++ )  {
      TSBond& b = *new_bonds[src_n.Bond(j).GetOwnerId()];
      net.AddBond(b);
      b.SetNetwork(net);
    }
  }
  GetUnitCell().UpdateEllipsoids();
  RestoreADPs(false);
}
//..............................................................................
void TLattice::SetDelta(double v)  {
  if( Delta != v )  {
    Delta = v;
    UpdateConnectivity();
  }
}
//..............................................................................
void TLattice::SetDeltaI(double v)  {
  if( DeltaI != v )  {
    DeltaI = v;
    GetUnitCell().FindSymmEq();
    UpdateConnectivity();
  }
}
//..............................................................................
//..............................................................................
//..............................................................................
void TLattice::LibGetFragmentCount(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal(olxstr(FragmentCount()));
}
//..............................................................................
void TLattice::LibGetMoiety(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal(CalcMoiety());
}
//..............................................................................
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
  TLibrary* lib = new TLibrary(name.IsEmpty() ? olxstr("latt") : name);
  lib->RegisterFunction<TLattice>( new TFunction<TLattice>(this,  &TLattice::LibGetFragmentCount, "GetFragmentCount", fpNone,
"Returns number of fragments in the lattice") );
  lib->RegisterFunction<TLattice>( new TFunction<TLattice>(this,  &TLattice::LibGetFragmentAtoms, "GetFragmentAtoms", fpOne,
"Returns a comma separated list of atoms in specified fragment") );
  lib->RegisterFunction<TLattice>( new TFunction<TLattice>(this,  &TLattice::LibGetMoiety, "GetMoiety", fpNone,
"Returns moelcular moiety") );
  return lib;
}
