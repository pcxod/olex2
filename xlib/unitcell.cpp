//---------------------------------------------------------------------------//
// namespace TXClasses: crystallographic core
// TUnitCell: a collection of matrices and ellipoids
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include <stdlib.h>

#include "unitcell.h"
#include "evpoint.h"
#include "asymmunit.h"
#include "lattice.h"

#include "catom.h"
#include "ellipsoid.h"
#include "network.h"

#include "bapp.h"
#include "log.h"

#include "emath.h"

#include "olxmps.h"
#include "arrays.h"
#undef GetObject
//---------------------------------------------------------------------------
// TUnitCell function bodies
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
TUnitCell::TUnitCell(TLattice *L)  {  Lattice = L;  }
//..............................................................................
TUnitCell::~TUnitCell()  {
  Clear();
}
//..............................................................................
void TUnitCell::ClearEllipsoids()  {
  for( size_t i=0; i < Ellipsoids.Count(); i++ )  {
    for( size_t j=0; j < Ellipsoids[i].Count(); j++ )
      if( Ellipsoids[i][j] != NULL )
        delete Ellipsoids[i][j];
  }
  Ellipsoids.Clear();
}
//..............................................................................
void TUnitCell::AddEllipsoid()  {
  Ellipsoids.SetCount( Ellipsoids.Count() + 1);
  for( size_t j=0; j < Matrices.Count(); j++ )
    Ellipsoids.Last().Add( NULL );
}
//..............................................................................
void TUnitCell::Clear()  {
  Matrices.Clear();
  ClearEllipsoids();
}
//..............................................................................
void TUnitCell::InitMatrixId(smatd& m) const {
  for( size_t i=0; i < Matrices.Count(); i++ )  {
    if( Matrices[i].r == m.r )  {
      const vec3d dt = m.t-Matrices[i].t;
      const int8_t ta = (int8_t)(m.t[0]-Matrices[i].t[0]);
      const int8_t tb = (int8_t)(m.t[1]-Matrices[i].t[1]);
      const int8_t tc = (int8_t)(m.t[2]-Matrices[i].t[2]);
      if( olx_abs(dt[0]-ta) < 1e-6 && olx_abs(dt[1]-tb) < 1e-6 && olx_abs(dt[2]-tc) < 1e-6 )  {
        m.SetId((uint8_t)i, ta, tb, tc);
        return;
      }
    }
  }
  throw TInvalidArgumentException(__OlxSourceInfo, "could not locate matrix");
}
//..............................................................................
double TUnitCell::CalcVolume()  const  {
  TAsymmUnit& au = GetLattice().GetAsymmUnit();
  static const double k = M_PI/180;
  vec3d ang(au.Angles()[0].GetV()*k, au.Angles()[1].GetV()*k, au.Angles()[2].GetV()*k);
  vec3d ax(au.Axes()[0].GetV(), au.Axes()[1].GetV(), au.Axes()[2].GetV());
  vec3d cs(cos(ang[0]), cos(ang[1]), cos(ang[2]) );
  return ax.Prod()*sqrt(1-cs.QLength() + 2*cs.Prod());
}
//..............................................................................
TEValue<double> TUnitCell::CalcVolumeEx()  const  {
  TAsymmUnit& au = GetLattice().GetAsymmUnit();
  static const double k = M_PI/180;
  vec3d ang(au.Angles()[0].GetV()*k, au.Angles()[1].GetV()*k, au.Angles()[2].GetV()*k);
  vec3d ange(au.Angles()[0].GetE()*k, au.Angles()[1].GetE()*k, au.Angles()[2].GetE()*k);
  vec3d ax(au.Axes()[0].GetV(), au.Axes()[1].GetV(), au.Axes()[2].GetV());
  vec3d axe(au.Axes()[0].GetE(), au.Axes()[1].GetE(), au.Axes()[2].GetE());
  vec3d cs(cos(ang[0]), cos(ang[1]), cos(ang[2]) );
  vec3d ss(sin(ang[0]), sin(ang[1]), sin(ang[2]) );
  double t = sqrt(1-cs.QLength() + 2*cs.Prod());
  double r = ax.Prod();
  double v = r*t;
  double esd = sqrt( olx_sqr(ax[1]*ax[2]*t*axe[0]) +  
    olx_sqr(ax[0]*ax[2]*t*axe[1]) +
    olx_sqr(ax[0]*ax[1]*t*axe[2]) +
    olx_sqr(r/t*ss[0]*(cs[0]-cs[1]*cs[2])*ange[0]) +
    olx_sqr(r/t*ss[1]*(cs[1]-cs[0]*cs[2])*ange[1]) +
    olx_sqr(r/t*ss[2]*(cs[2]-cs[0]*cs[1])*ange[2]) 
    );
  return  TEValue<double>(v, esd);
}
//..............................................................................
size_t TUnitCell::GetMatrixMultiplier(short Latt)  {
  size_t count = 0;
  switch( abs(Latt) )  {
    case 1: count = 1;  break;
    case 2: count = 2;  break;  // Body Centered (I)
    case 3: count = 3;  break;  // R Centered
    case 4: count = 4;  break;  // Face Centered (F)
    case 5: count = 2;  break;  // A Centered (A)
    case 6: count = 2;  break;  // B Centered (B)
    case 7: count = 2;  break;  // C Centered (C);
    default:
      throw TInvalidArgumentException(__OlxSourceInfo, "LATT");
  }
  if( Latt > 0 )  count *= 2;
  return count;
}
//..............................................................................
void  TUnitCell::InitMatrices()  {
  Matrices.Clear();
  GenerateMatrices(Matrices, GetLattice().GetAsymmUnit(), GetLattice().GetAsymmUnit().GetLatt() );
  const size_t mc = Matrices.Count();
  for( size_t i=0; i < mc; i++ )
    Matrices[i].SetId((uint8_t)i);
  UpdateEllipsoids();
}
//..............................................................................
void TUnitCell::GenerateMatrices(smatd_list& out, const TAsymmUnit& au, short lat)  {
  out.SetCapacity( GetMatrixMultiplier(au.GetLatt())*au.MatrixCount());
  out.AddNew().r.I();
  // check if the E matrix is in the list
  for( size_t i=0;  i < au.MatrixCount(); i++ )  {
    const smatd& m = au.GetMatrix(i);
    if( m.r.IsI() )  continue;  // will need to insert the identity matrix at position 0
    out.AddNew( m );
  }

  for( size_t i=0; i < out.Count(); i++ )  {
    const smatd& m = out[i];
    switch( abs(lat) )  {
      case 1: break;
      case 2:      // Body Centered (I)
        out.InsertCCopy(++i, m).t += vec3d(0.5, 0.5, 0.5);
        break;
      case 3:      // R Centered
        out.InsertCCopy(++i, m).t += vec3d(2./3, 1./3, 1./3);
        out.InsertCCopy(++i, m).t += vec3d(1./3, 2./3, 2./3);
        break;
      case 4:      // Face Centered (F)
        out.InsertCCopy(++i, m).t += vec3d(0, 0.5, 0.5);
        out.InsertCCopy(++i, m).t += vec3d(0.5, 0, 0.5);
        out.InsertCCopy(++i, m).t += vec3d(0.5, 0.5, 0);
        break;
      case 5:      // A Centered (A)
        out.InsertCCopy(++i, m).t += vec3d(0, 0.5, 0.5);
        break;
      case 6:      // B Centered (B)
        out.InsertCCopy(++i, m).t += vec3d(0.5, 0, 0.5);
        break;
      case 7:      // C Centered (C);
        out.InsertCCopy(++i, m).t += vec3d(0.5, 0.5, 0);
        break;
      default:
        throw TInvalidArgumentException(__OlxSourceInfo, "LATT");
    }
  }
  if( au.GetLatt() > 0 )  {
    for( size_t i=0; i < out.Count(); i++ )  {
      const smatd& m = out[i];
      out.InsertCCopy(++i, m) *= -1;
    }
  }
}
//..............................................................................
void TUnitCell::UpdateEllipsoids()  {
  const TAsymmUnit& au = GetLattice().GetAsymmUnit();
  const size_t ac = au.AtomCount();
  const size_t mc = Matrices.Count();

  const mat3d abc2xyz(mat3d::Transpose(au.GetCellToCartesian())),
              xyz2abc(mat3d::Transpose(au.GetCartesianToCell()));

  ClearEllipsoids();
  Ellipsoids.SetCount(ac);
  for( size_t i=0; i < ac; i++ )  {
    const TCAtom& A1 = au.GetAtom(i);
    Ellipsoids[i].SetCount(mc);
    for( size_t j=0; j < mc; j++ )  {
      if( olx_is_valid_index(A1.GetEllpId()) )  {
        TEllipsoid* E = new TEllipsoid;
        E->SetId(j*ac+A1.GetId());
        *E = *A1.GetEllipsoid();
        E->MultMatrix(abc2xyz*Matrices[j].r*xyz2abc);
        Ellipsoids[i][j] = E;
      }
      else
        Ellipsoids[i][j] = NULL;
    }
  }
}
//..............................................................................
const TEllipsoid& TUnitCell::GetEllipsoid(size_t MatrixId, size_t AUId) const  {
  return *Ellipsoids[AUId][MatrixId];
}
//..............................................................................
TUnitCell::TSearchSymmEqTask::TSearchSymmEqTask(TPtrList<TCAtom>& atoms,
  const smatd_list& matrices, double tol) :
  Atoms(atoms), Matrices(matrices), tolerance(tol)
{
  AU = atoms[0]->GetParent();
  Latt = &AU->GetLattice();
}
//..............................................................................
void TUnitCell::TSearchSymmEqTask::Run(size_t ind) const {
  if( Atoms[ind]->IsDeleted() )  return;
  const size_t ac = Atoms.Count();
  const size_t mc = Matrices.Count();
  for( size_t i=ind; i < ac; i++ )  {
    if( Atoms[i]->IsDeleted() )  continue;
    for( size_t j=0; j < mc; j++ )  {
      vec3d v = Atoms[ind]->ccrd() - Matrices[j] * Atoms[i]->ccrd();
      const int iLx = olx_round(v[0]);  v[0] -= iLx;
      const int iLy = olx_round(v[1]);  v[1] -= iLy;
      const int iLz = olx_round(v[2]);  v[2] -= iLz;
      if( j == 0 && (iLx|iLy|iLz) == 0 )  {  // I
        if( ind == i || Atoms[i]->GetFragmentId() == Atoms[ind]->GetFragmentId() )  continue;
        AU->CellToCartesian(v);
        const double d = v.Length();
        if( d < tolerance )  {
          if( Atoms[ind]->GetAtomInfo() == iQPeakIndex )  {
            Atoms[ind]->SetDeleted(true);
            break;
          }
          Atoms[i]->SetDeleted(true);
          continue;
        }
        if( Latt->GetNetwork().HBondExists(*Atoms[ind], *Atoms[i], Matrices[j], v.Length()) )  {
          Atoms[ind]->AttachAtomI(Atoms[i]);
          Atoms[i]->AttachAtomI(Atoms[ind]);
        }
        continue;
      }

      AU->CellToCartesian(v);
      const double Dis = v.Length();
      if( (j != 0) && (Dis < tolerance) )  {
        if( i == ind )  {
          Atoms[ind]->SetDegeneracy(Atoms[ind]->GetDegeneracy() + 1);
          continue;
        }
        if( Atoms[i]->GetExyzGroup() != NULL && Atoms[i]->GetExyzGroup() == Atoms[ind]->GetExyzGroup() ) 
          continue;  
        if( Atoms[ind]->GetAtomInfo() == iQPeakIndex )  {
          Atoms[ind]->SetDeleted(true);
          break;
        }
        Atoms[i]->SetDeleted(true);
      }
      else  {
        if( Latt->GetNetwork().CBondExists(*Atoms[ind], *Atoms[i], Matrices[j], Dis) )  {
          Atoms[ind]->SetGrowable(true);
          if( Atoms[ind]->IsAttachedTo(*Atoms[i]) )  
            continue;
          Atoms[ind]->AttachAtom(Atoms[i]);
          if( i != ind )  {
            Atoms[i]->SetGrowable(true);
            Atoms[i]->AttachAtom(Atoms[ind]);
          }
        }
        else if( Latt->GetNetwork().HBondExists(*Atoms[ind], *Atoms[i], Matrices[j], Dis) )  {
          if( Atoms[ind]->IsAttachedToI(*Atoms[i]) )
            continue;
          Atoms[ind]->AttachAtomI(Atoms[i]);
          if( i != ind )
            Atoms[i]->AttachAtomI(Atoms[ind]);
        }
      }
    }
  }
}
//..............................................................................
void TUnitCell::FindSymmEq(double tol) const  {
  TStrList report;
  TCAtomPList ACA;
  // sorting the content of the asymmetric unit in order to improve the algorithm
  // which is a bit sluggish for a big system; however it will not be ever used
  // in the grow-fuse (XP) cycle: a save and load operations will be used in that
  // case
  ACA.SetCapacity( GetLattice().GetAsymmUnit().AtomCount() );
  for( size_t i=0; i < GetLattice().GetAsymmUnit().AtomCount(); i++ )  {
    TCAtom& A1 = GetLattice().GetAsymmUnit().GetAtom(i);
    if( A1.IsDeleted() )  continue;
    ACA.Add(A1)->SetTag(0);  
    A1.ClearAttachedAtoms();
    A1.SetDegeneracy(1);
  }
  // searching for symmetrical equivalents; the search could be optimised by
  // removing the translational equivalents in the firts order; however the task is not
  // very common, so it should be OK. (An identity (E) matrix is in the list
  // so translational equivalents will be removed too
  if( ACA.IsEmpty() )  return;
  TSearchSymmEqTask searchTask(ACA, Matrices, tol);
  TListIteratorManager<TSearchSymmEqTask> searchm(searchTask, ACA.Count(), tQuadraticTask, 1000);
}
//..............................................................................
smatd* TUnitCell::GetClosest(const vec3d& to, const vec3d& from, bool ConsiderOriginal, double* dist) const  {
  const smatd* minMatr = NULL;
  vec3d V1;
  int minix, miniy, miniz;
  double minD=10000;
  if( ConsiderOriginal )  {
    V1 = from-to;
    minD = GetLattice().GetAsymmUnit().CellToCartesian(V1).QLength();
    if( dist != NULL )
      *dist = minD;
  }
  for( size_t i=0; i < Matrices.Count(); i++ )  {
    const smatd& matr = Matrices[i];
    V1 = matr * from - to;
    const int ix = olx_round(V1[0]);  V1[0] -= (ix);  // find closest distance
    const int iy = olx_round(V1[1]);  V1[1] -= (iy);
    const int iz = olx_round(V1[2]);  V1[2] -= (iz);
    // check for identity matrix
    if( i == 0 && (ix|iy|iz) == 0 )  continue;
    GetLattice().GetAsymmUnit().CellToCartesian(V1);
    const double D = V1.QLength();
    if( D < minD )  {
      minD = D;
      minMatr = &matr;
      minix = ix;  miniy = iy;  miniz = iz;
    }
    else  {
      if( D == minD && minMatr == NULL )  {
        minMatr = &matr;
        minix = ix;  miniy = iy;  miniz = iz;
      }
    }
  }
  if( minMatr != NULL)  {
    smatd* retVal = new smatd(*minMatr);
    retVal->t[0] -= minix;
    retVal->t[1] -= miniy;
    retVal->t[2] -= miniz;
    retVal->SetId(minMatr->GetContainerId(), -minix, -miniy, -miniz);
    if( dist != NULL )
      *dist = minD;
    return retVal;
  }
  return NULL;
}
//..............................................................................
double TUnitCell::FindClosestDistance(const class TCAtom& a_from, const TCAtom& a_to) const  {
  vec3d V1, from(a_from.ccrd()), to(a_to.ccrd());
  V1 = from-to;
  GetLattice().GetAsymmUnit().CellToCartesian(V1);
  double minD = V1.QLength();
  for( size_t i=0; i < MatrixCount(); i++ )  {
    const smatd& matr = GetMatrix(i);
    V1 = matr * from - to;
    V1[0] -= olx_round(V1[0]);  // find closest distance
    V1[1] -= olx_round(V1[1]);
    V1[2] -= olx_round(V1[2]);
    GetLattice().GetAsymmUnit().CellToCartesian(V1);
    double D = V1.QLength();
    if( D < minD )  
      minD = D;
  }
  return sqrt(minD);
}
//..............................................................................
smatd_list* TUnitCell::GetBinding(const TCAtom& toA, const TCAtom& fromA,
    const vec3d& to, const vec3d& from, bool IncludeI, bool IncludeHBonds) const  {
  smatd_list* retVal = new smatd_list;
  smatd Im;
  Im.I().SetId(0);

  for( size_t i=0; i < MatrixCount(); i++ )  {
    const smatd& matr = GetMatrix(i);
    vec3d V1 = matr * from - to;
    const int ix = olx_round(V1[0]);  V1[0] -= (ix);
    const int iy = olx_round(V1[1]);  V1[1] -= (iy);
    const int iz = olx_round(V1[2]);  V1[2] -= (iz);
    // check for identity matrix
    if( !IncludeI && i == 0 &&  (ix|iy|iz) == 0 )  continue;
    GetLattice().GetAsymmUnit().CellToCartesian(V1);
    const double qD = V1.QLength();
    if( GetLattice().GetNetwork().CBondExistsQ(toA, fromA, matr, qD) )  {
      smatd& newMatr = retVal->AddNew(matr);
      newMatr.t[0] -= ix;
      newMatr.t[1] -= iy;
      newMatr.t[2] -= iz;
      newMatr.SetId(matr.GetContainerId(), -ix, -iy, -iz);
    }
    else if( IncludeHBonds )  {
      if( GetLattice().GetNetwork().HBondExistsQ(toA, fromA, matr, qD) )  {
        smatd& newMatr = retVal->AddNew(matr);
        newMatr.t[0] -= ix;
        newMatr.t[1] -= iy;
        newMatr.t[2] -= iz;
        newMatr.SetId(matr.GetContainerId(), -ix, -iy, -iz);
      }
    }
  }
  for( int i=-1; i <= 1; i++ )  {
    for( int j=-1; j <= 1; j++ ) {
      for( int k=-1; k <= 1; k++ )  {
        if( (i|j|k) == 0 )  continue;
        vec3d V1(from[0] + i - to[0], from[1] + j - to[1], from[2] + k - to[2]);
        Im.t = V1;
        GetLattice().GetAsymmUnit().CellToCartesian(V1);
        const double qD = V1.QLength();
        if( GetLattice().GetNetwork().CBondExistsQ(toA, fromA, Im, qD) )  {
          smatd& retMatr = retVal->AddNew().I();
          retMatr.t[0] += i;
          retMatr.t[1] += j;
          retMatr.t[2] += k;
          retMatr.SetId(0, i, j, k);
        }
        else if( IncludeHBonds )  {
          if( GetLattice().GetNetwork().HBondExistsQ(toA, fromA, Im, qD) )  {
            smatd& retMatr = retVal->AddNew().I();
            retMatr.t[0] += i;
            retMatr.t[1] += j;
            retMatr.t[2] += k;
            retMatr.SetId(0, i, j, k);
          }
        }
      }
    }
  }
  return retVal;
}
//..............................................................................
smatd_list* TUnitCell::GetInRange(const vec3d& to, const vec3d& from, double R, bool IncludeI) const  {
  smatd_list* retVal = new smatd_list;
  R *= R;
  for( size_t i=0; i < MatrixCount(); i++ )  {
    const smatd& matr = GetMatrix(i);
    vec3d V1 = matr * from;
    V1 -= to;
    int ix = olx_round(V1[0]);  V1[0] -= ix;
    int iy = olx_round(V1[1]);  V1[1] -= iy;
    int iz = olx_round(V1[2]);  V1[2] -= iz;
    // check for identity matrix
    if( !IncludeI && i == 0 && (ix|iy|iz) == 0 )  continue;
    GetLattice().GetAsymmUnit().CellToCartesian(V1);
    if( V1.QLength() < R )  {
      smatd& retMatr = retVal->AddNew(matr);
      retMatr.t[0] -= ix;
      retMatr.t[1] -= iy;
      retMatr.t[2] -= iz;
      retMatr.SetId(matr.GetContainerId(), -ix, -iy, -iz);
    }
  }
  for( int i=-1; i <= 1; i++ )  {
    for( int j=-1; j <= 1; j++ ) {
      for( int k=-1; k <= 1; k++ )  {
        if( (i|j|k) == 0 )  continue;
        vec3d V1(from[0] + i - to[0], from[1] + j - to[1], from[2] + k - to[2]);
        GetLattice().GetAsymmUnit().CellToCartesian(V1);
        if( V1.QLength() < R )  {
          smatd& retMatr = retVal->AddNew().I();
          retMatr.t[0] += i;
          retMatr.t[1] += j;
          retMatr.t[2] += k;
          retMatr.SetId(0, i, j, k);
        }
      }
    }
  }
  return retVal;
}
//..............................................................................
smatd_list* TUnitCell::GetInRangeEx(const vec3d& to, const vec3d& from, 
                                       double R, bool IncludeI, const smatd_list& ToSkip) const  {
  smatd_list* retVal = new smatd_list;
  vec3d V2;
  R *= R;
  for( size_t i=0; i < MatrixCount(); i++ )  {
    const smatd& matr = GetMatrix(i);
    vec3f V1 = matr * from - to;
    const int ix = olx_round(V1[0]);  V1[0] -= ix;
    const int iy = olx_round(V1[1]);  V1[1] -= iy;
    const int iz = olx_round(V1[2]);  V1[2] -= iz;
    for( int ii=-1; ii <= 1; ii++ )  {
      for( int ij=-1; ij <= 1; ij++ )  {
        for( int ik=-1; ik <= 1; ik++ )  {
          if( !IncludeI && i == 0 && ((ix-ii)|(iy-ij)|(iz-ik)) == 0 )
            continue;
          V2 = V1;
          V2[0] += ii;  V2[1] += ij;  V2[2] += ik;
          GetLattice().GetAsymmUnit().CellToCartesian(V2);
          if( V2.QLength() < R )  {
            smatd* retMatr = new smatd(matr);
            retMatr->t[0] -= (ix-ii);
            retMatr->t[1] -= (iy-ij);
            retMatr->t[2] -= (iz-ik);
            if( ToSkip.IndexOf( *retMatr ) != InvalidIndex )  {
              delete retMatr;
              continue;
            }
            retMatr->SetId(matr.GetContainerId(), ii-ix, ij-iy, ik-iz);
            retVal->Add(retMatr);
          }
        }
      }
    }
  }
  return retVal;
}
//..............................................................................
void TUnitCell::_FindInRange(const vec3d& to, double R, 
                            TTypeList< AnAssociation3<TCAtom const*,smatd, vec3d> >& res) const {
  const TAsymmUnit& au = GetLattice().GetAsymmUnit();
  vec3d V1;
  R *= R;
  const size_t ac = au.AtomCount();
  for( size_t i=0; i < ac; i++ )  {
    const TCAtom& a = au.GetAtom(i);
    if( a.IsDeleted() )  continue;
    for( size_t j=0; j < MatrixCount(); j++ )  {
      const smatd& matr = GetMatrix(j);
      V1 = matr * a.ccrd() - to;
      const int ix = olx_round(V1[0]);  V1[0] -= ix;  // find closest distance
      const int iy = olx_round(V1[1]);  V1[1] -= iy;
      const int iz = olx_round(V1[2]);  V1[2] -= iz;
      au.CellToCartesian(V1);
      const double D = V1.QLength();
      //if( D < R && D != 0 )  {
      if( D < R )  {
        smatd& m = res.AddNew(&a, matr, V1).B();
        m.t[0] -= ix;
        m.t[1] -= iy;
        m.t[2] -= iz;
        m.SetId(matr.GetContainerId(), -ix, -iy, -iz);
      }
    }
    for( int ii=-1; ii <= 1; ii++ )  {
      for( int ij=-1; ij <= 1; ij++ ) {
        for( int ik=-1; ik <= 1; ik++ )  {
          if( (ii|ij|ik) == 0 )  continue;
          V1[0] = a.ccrd()[0] + ii;
          V1[1] = a.ccrd()[1] + ij;
          V1[2] = a.ccrd()[2] + ik;
          V1 -= to;
          au.CellToCartesian(V1);
          const double D = V1.QLength();
          if( D < R && D > 0.0001 )  {
            smatd& m = res.AddNew(&a).B().I();
            m.t[0] += ii;
            m.t[1] += ij;
            m.t[2] += ik;
            m.SetId(0, ii, ik, ik);
            res.Last().C() = V1;
          }
        }
      }
    }
  }
}
//..............................................................................
void TUnitCell::GetAtomEnviList(TSAtom& atom, TAtomEnvi& envi, bool IncludeQ, int part)  const {
  const TAsymmUnit& au = GetLattice().GetAsymmUnit();
  envi.SetBase(atom);

  smatd I;
  I.I().SetId(0);

  for( size_t i=0; i < atom.NodeCount(); i++ )  {
    TSAtom& a = atom.Node(i);
    if( a.IsDeleted() ) continue;
    if( !IncludeQ && a.GetAtomInfo() == iQPeakIndex )  continue;
    if( part == DefNoPart || (a.CAtom().GetPart() == 0 || a.CAtom().GetPart() == part) )  {
      if( TNetwork::HaveSharedMatrix(atom, a) )  // put only the 'uniq' entries
        envi.Add( a.CAtom(), I, a.crd() );
    }
  }
  for( size_t i=0; i < atom.CAtom().AttachedAtomCount(); i++ )  {
    TCAtom& A = atom.CAtom().GetAttachedAtom(i);
    if( A.IsDeleted() || (!IncludeQ && A.GetAtomInfo() == iQPeakIndex) )  continue;
    if( A.GetPart() < 0 || atom.CAtom().GetPart() < 0 )
      continue;

    smatd* m = GetClosest(atom.ccrd(), A.ccrd(), false);
    if( m == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "Could not find symmetry generated atom");
    vec3d v = *m * A.ccrd();
    au.CellToCartesian(v);
    // make sure that atoms on center of symmetry are not counted twice
    bool Add = true;
    for( size_t j=0; j < envi.Count(); j++ )  {
      if( envi.GetCAtom(j) == A && envi.GetCrd(j).QDistanceTo(v) < 0.001 )  {
        Add = false;
        break;
      }
    }
    if( Add )  {
      if( part == DefNoPart || (A.GetPart() == 0 || A.GetPart() == part) )
        envi.Add(A, *m, v);
    }
    delete m;
  }
  smatd_list ml;
  BondInfoList toCreate, toDelete;
  ConnInfo::Compile(atom.CAtom(), toCreate, toDelete, ml);
  /* process own connectivity info. according to latest changes, olex2 sort */
  for( size_t i=0; i < toDelete.Count(); i++ )  {
    if( toDelete[i].matr == NULL )  {
      for( size_t j=0; j < envi.Count(); j++ )  {
        if( envi.GetCAtom(j) == toDelete[i].to )
          envi.Delete(j--);
      }
    }
    else {
      for( size_t j=0; j < envi.Count(); j++ )  {
        if( envi.GetCAtom(j) == toDelete[i].to )  {
          if( envi.GetMatrix(j).EqualExt(*toDelete[i].matr) )  {
            envi.Delete(j);
            break;
          }
        }
      }
    }
  }
  for( size_t i=0; i < toCreate.Count(); i++ )  {
    bool found = false;
    for( size_t j=0; j < envi.Count(); j++ )  {
      if( envi.GetCAtom(j) == toCreate[i].to )  {
        if( toCreate[i].matr == NULL )  {
          if( envi.GetMatrix(j).IsI() )  {
            found = true;
            break;
          }
        }
        else if( envi.GetMatrix(j).EqualExt(*toCreate[i].matr) )  {
          found = true;
          break;
        }
      }
    }
    if( !found )  {
      if( toCreate[i].matr == NULL )  {
        vec3d crd = toCreate[i].to.ccrd();
        au.CellToCartesian(crd);
        if( crd.QDistanceTo(atom.crd()) > 0.001 )
          envi.Add(toCreate[i].to, I, crd);
      }
      else  {
        vec3d crd = (*toCreate[i].matr)*toCreate[i].to.ccrd();
        au.CellToCartesian(crd);
        if( crd.QDistanceTo(atom.crd()) > 0.001 )
          envi.Add(toCreate[i].to, *toCreate[i].matr, crd);
      }
    }
  }
  // finally, CONN
  uint16_t maxb = (uint16_t)olx_abs(atom.CAtom().GetConnInfo().maxBonds);
  if( envi.Count() > maxb )  {
    envi.SortByDistance();
    while( envi.Count() > maxb )
      envi.Delete(envi.Count()-1);
  }
}
//..............................................................................
void TUnitCell::GetAtomQEnviList(TSAtom& atom, TAtomEnvi& envi)  {
  if( atom.IsGrown() )
    throw TFunctionFailedException(__OlxSourceInfo, "Not implementd for grown structre");

  envi.SetBase( atom );
  smatd I;
  I.I().SetId(0);

  for( size_t i=0; i < atom.NodeCount(); i++ )  {
    TSAtom& A = atom.Node(i);
    if( A.IsDeleted() ) continue;
    if( A.GetAtomInfo() == iQPeakIndex && TNetwork::HaveSharedMatrix(A, atom) )
      envi.Add(A.CAtom(), I, A.crd());
  }
  for( size_t i=0; i < atom.CAtom().AttachedAtomCount(); i++ )  {
    TCAtom& A = atom.CAtom().GetAttachedAtom(i);
    if( A.IsDeleted() || A.GetAtomInfo() != iQPeakIndex )  continue;

    smatd* m = GetClosest(atom.ccrd(), A.ccrd(), false);
    if( m == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "Could not find symmetry generated atom");
    vec3d v = *m * A.ccrd();
    GetLattice().GetAsymmUnit().CellToCartesian(v);
    // make sure that atoms on center of symmetry are not counted twice
    bool Add = true;
    for( size_t j=0; j < envi.Count(); j++ )  {
      if( envi.GetCAtom(j) == A && envi.GetCrd(j) == v )  {
        Add = false;
        break;
      }
    }
    if( Add )
      envi.Add( A, *m, v );
    delete m;
  }
}
//..............................................................................
void TUnitCell::GetAtomPossibleHBonds(const TAtomEnvi& ae, TAtomEnvi& envi)  {
  envi.SetBase( ae.GetBase() );

  TAsymmUnit& au = GetLattice().GetAsymmUnit();

  const size_t ac = au.AtomCount();
  for( size_t i=0; i < ac; i++ )  {
    TCAtom& A = au.GetAtom(i);
    if( A.GetAtomInfo() == iQPeakIndex || A.IsDeleted() )  continue;

    bool considerI =  (A != ae.GetBase().CAtom());
    // O and N and S for a while
    if( !(A.GetAtomInfo() == iOxygenIndex ||
          A.GetAtomInfo() == iNitrogenIndex ||
          A.GetAtomInfo() == iSulphurIndex) )  continue;

    smatd_list& ms = *GetInRange( ae.GetBase().ccrd(), A.ccrd(), 2.9, considerI );

    for( size_t j=0; j < ms.Count(); j++ )  {
      vec3d v = ms[j] * A.ccrd();
      au.CellToCartesian(v);
      const double qd = v.QDistanceTo( ae.GetBase().crd() );
      if(  qd < 2*2 || qd > 2.9*2.9 )  continue;
      if( ae.Count() == 1 )  {
        vec3d v1 = ae.GetCrd(0) - ae.GetBase().crd();
        vec3d v2 = v - ae.GetBase().crd();
        // 89 - 130 degrees
        const double ca = v1.CAngle(v2);
        if( ca > 0.01 || ca < -0.64 )  continue;
      }
//      else
//        continue;

      // make sure that atoms on center of symmetry are not counted twice
      bool Add = true;
      for( size_t k=0; k < envi.Count(); k++ )  {
        if( envi.GetCAtom(k) == A && envi.GetCrd(k) == v )  {
          Add = false;
          break;
        }
      }
      if( Add )
        envi.Add( A, ms[j], v );
    }
    delete &ms;
  }
}
//..............................................................................
TCAtom* TUnitCell::FindOverlappingAtom(const vec3d& pos, double delta) const  {
  TTypeList< AnAssociation3<TCAtom const*,smatd,vec3d> > res;
  _FindInRange(pos, delta, res);
  if( res.IsEmpty() )
    return NULL;
  if( res.Count() == 1 )
    return const_cast<TCAtom*>(res[0].A());
  else  {
    const TAsymmUnit& au = GetLattice().GetAsymmUnit();
    vec3d cpos(pos);
    au.CellToCartesian(cpos);
    double minQD = 1000;
    size_t ri = InvalidIndex;
    for( size_t i=0; i < res.Count(); i++ )  {
      const double qd = res[i].GetC().QDistanceTo(cpos);
      if( qd < minQD )  {
        ri = i;
        minQD = qd;
      }
    }
    if( ri == InvalidIndex )
      throw TFunctionFailedException(__OlxSourceInfo, "assert here");
    return const_cast<TCAtom*>(res[ri].A());
  }
}
//..............................................................................
TCAtom* TUnitCell::FindCAtom(const vec3d& center) const  {
  TTypeList< AnAssociation3<const TCAtom*, smatd, vec3d> > res;
  _FindInRange(center, 0.1, res);
  if( res.IsEmpty() )
    return NULL;
  if( res.Count() == 1 )
    return const_cast<TCAtom*>(res[0].A());
  throw TFunctionFailedException(__OlxSourceInfo, "assert, too many atoms returned");
}
//..............................................................................
void TUnitCell::BuildStructureMap( TArray3D<short>& map, double delta, short val, 
                                  size_t* structurePoints, TPSTypeList<TBasicAtomInfo*, double>* radii,
                                  const TCAtomPList* _template )  {

  TBasicApp::GetLog() << "Building structure map...\n";
  TTypeList< AnAssociation3<vec3d,TCAtom*, double> > allAtoms;
  GenereteAtomCoordinates( allAtoms, true, _template );
  
  const size_t da = map.Length1(),
               db = map.Length2(),
               dc = map.Length3();
  const size_t map_dim[] = {da, db, dc};
  map.FastInitWith(0);
  // expand atom list to +/-1/3 of UC
  ExpandAtomCoordinates(allAtoms, 1./2);
  // precalculate the sphere/ellipsoid etc coordinates for all distinct scatterers
  const TAsymmUnit& au = GetLattice().GetAsymmUnit();

  TPSTypeList<int, double > scatterers;
  for( size_t i=0; i < au.AtomCount(); i++ )  {
    if( au.GetAtom(i).IsDeleted() )  continue;
    size_t ind = scatterers.IndexOfComparable( au.GetAtom(i).GetAtomInfo().GetIndex() );
    if( ind != InvalidIndex )  continue;
    double r = au.GetAtom(i).GetAtomInfo().GetRad2() + delta;
    if( radii != NULL )  {
      size_t b_i = radii->IndexOfComparable( &au.GetAtom(i).GetAtomInfo() );
      if( b_i != InvalidIndex )
        r = radii->GetObject(b_i) + delta;
    }
    scatterers.Add(au.GetAtom(i).GetAtomInfo().GetIndex(), r);
  }
  for( size_t i=0; i < allAtoms.Count(); i++ )  {
    const double sr = scatterers[ allAtoms[i].GetB()->GetAtomInfo().GetIndex() ];
    allAtoms[i].C() = sr*sr;
    au.CellToCartesian(allAtoms[i].A());
  }
  const size_t ac = allAtoms.Count();
  for( size_t j = 0; j < da; j ++ )  {
    const double dx = (double)j/da;
    if( (j%5) == 0 )  {
      TBasicApp::GetLog() << '\r' << j*100/da << '%';
      TBasicApp::GetInstance().Update();
    }
    for( size_t k = 0; k < db; k ++ )  {
      const double dy = (double)k/db;
      for( size_t l = 0; l < dc; l ++ )  {
        vec3d p(dx, dy, (double)l/dc);
        au.CellToCartesian(p);
        for( size_t i=0; i < ac; i++ )  {
          if( p.QDistanceTo( allAtoms[i].GetA() ) <= allAtoms[i].GetC() )  {  
            map.Data[j][k][l] = val;
            break;
          }
        }
      }
    }
  }
  if( structurePoints != NULL )  {
    size_t sp = 0;
    for( size_t i=0; i < da; i++ )  {
      for( size_t j=0; j < db; j++ )
        for( size_t k=0; k < dc; k++ )
          if( map.Data[i][j][k] == val )
            sp ++;
    }
    *structurePoints = sp;
  }
  TBasicApp::GetLog() << '\r' << "Done\n";
  TBasicApp::GetInstance().Update();
}
void TUnitCell::BuildStructureMapEx( TArray3D<short>& map, double resolution, double delta, short val, 
                                  size_t* structurePoints, TPSTypeList<TBasicAtomInfo*, 
                                  double>* radii, const TCAtomPList* _template)  {

  throw TNotImplementedException(__OlxSourceInfo);
  //TTypeList< AnAssociation2<vec3d,TCAtom*> > allAtoms;
  //vec3d center;
  //GenereteAtomCoordinates( allAtoms, true, _template );
  //
  //const size_t da = map.Length1(),
  //          db = map.Length2(),
  //          dc = map.Length3();
  //vec3i dim(da, db, dc);
  //// angstrem per pixel
  //double capp = Lattice->GetAsymmUnit().Axes()[2].GetV()/dc,
  //       bapp = Lattice->GetAsymmUnit().Axes()[1].GetV()/db,
  //       aapp = Lattice->GetAsymmUnit().Axes()[0].GetV()/da;
  //map.FastInitWith(0);
  //// precalculate the sphere/ellipsoid etc coordinates for all distinct scatterers
  //const TAsymmUnit& au = GetLattice().GetAsymmUnit();
  //TPSTypeList<int, TArray3D<bool>* > scatterers;
  //for( size_t i=0; i < au.AtomCount(); i++ )  {
  //  if( au.GetAtom(i).IsDeleted() )  continue;
  //  size_t ind = scatterers.IndexOfComparable( au.GetAtom(i).GetAtomInfo().GetIndex() );
  //  if( ind != InvalidIndex )  continue;
  //  double r = au.GetAtom(i).GetAtomInfo().GetRad2() + delta;
  //  if( radii != NULL )  {
  //    size_t b_i = radii->IndexOfComparable( &au.GetAtom(i).GetAtomInfo() );
  //    if( b_i != InvalidIndex )
  //      r = radii->GetObject(b_i) + delta;
  //  }
  //  const double sr = r*r;
  //  const int ard = olx_round(r/resolution)*3;
  //  TArray3D<bool>* spm = new TArray3D<bool>(-ard, ard, -ard, ard, -ard, ard);
  //  for( int x=-ard; x <= ard; x ++ )  {
  //    for( int y=-ard; y <= ard; y ++ )  {
  //      for( int z=-ard; z <= ard; z ++ )  {
  //        vec3d v((double)x/da, (double)y/db, (double)z/dc);
  //        au.CellToCartesian(v);
  //        spm->Data[x+ard][y+ard][z+ard] = (v.QLength() < sr);
  //      }
  //    }
  //  }
  //  scatterers.Add(au.GetAtom(i).GetAtomInfo().GetIndex(), spm);
  //}

  //for( size_t i=0; i < allAtoms.Count(); i++ )  {
  //  TArray3D<bool>* spm = scatterers[ allAtoms[i].GetB()->GetAtomInfo().GetIndex() ];
  //  center = allAtoms[i].GetA()*dim;
  //  const int ard = spm->Length1()/2;
  //  for( int j = -ard; j < ard; j ++ )  {
  //    for( int k = -ard; k < ard; k ++ )  {
  //      for( int l = -ard; l < ard; l ++ )  {
  //        if( !spm->Data[j+ard][k+ard][l+ard] )  continue;
  //        vec3i crd( olx_round(center[0]+j), olx_round(center[1]+k), olx_round(center[2]+l));
  //        for( size_t m=0; m < 3; m++ )  {
  //          if( crd[m] < 0 )        crd[m] += dim[m];
  //          if( crd[m] >= dim[m] )  crd[m] -= dim[m];
  //        }
  //        map.Data[crd[0]][crd[1]][crd[2]] = val;
  //      }
  //    }
  //  }
  //}
  //if( structurePoints != NULL )  {
  //  long sp = 0;
  //  size_t mapX = map.Length1(), mapY = map.Length2(), mapZ = map.Length3();
  //  for( size_t i=0; i < mapX; i++ )
  //    for( size_t j=0; j < mapY; j++ )
  //      for( size_t k=0; k < mapZ; k++ )
  //        if( map.Data[i][j][k] == val )  {
  //          sp ++;
  //        }
  //  *structurePoints = sp;
  //}

  //for( size_t i=0; i < scatterers.Count(); i++ )
  //  delete scatterers.Object(i);
}
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
void TUnitCell::LibVolumeEx(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal(CalcVolumeEx().ToString());
}
//..............................................................................
void TUnitCell::LibCellEx(const TStrObjList& Params, TMacroError& E)  {
  if( Params[0].Equalsi('a') )
    E.SetRetVal( Lattice->GetAsymmUnit().Axes()[0].ToString() );
  else if( Params[0].Equalsi('b') )
    E.SetRetVal( Lattice->GetAsymmUnit().Axes()[1].ToString() );
  else if( Params[0].Equalsi('c') )
    E.SetRetVal( Lattice->GetAsymmUnit().Axes()[2].ToString() );
  else if( Params[0].Equalsi("alpha") )
    E.SetRetVal( Lattice->GetAsymmUnit().Angles()[0].ToString() );
  else if( Params[0].Equalsi("beta") )
    E.SetRetVal( Lattice->GetAsymmUnit().Angles()[1].ToString() );
  else if( Params[0].Equalsi("gamma") )
    E.SetRetVal( Lattice->GetAsymmUnit().Angles()[2].ToString() );
  else
    E.ProcessingError(__OlxSrcInfo, "invalid argument");
}
//..............................................................................
void TUnitCell::LibMatrixCount(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal(Matrices.Count());
}
//..............................................................................
class TLibrary* TUnitCell::ExportLibrary(const olxstr& name)  {
  TLibrary* lib = new TLibrary( name.IsEmpty() ? olxstr("uc") : name );
  lib->RegisterFunction<TUnitCell>( new TFunction<TUnitCell>(this,  &TUnitCell::LibVolumeEx, "VolumeEx", fpNone,
"Returns unit cell volume with esd") );
  lib->RegisterFunction<TUnitCell>( new TFunction<TUnitCell>(this,  &TUnitCell::LibCellEx, "CellEx", fpOne,
"Returns unit cell side/angle with esd") );
  lib->RegisterFunction<TUnitCell>( new TFunction<TUnitCell>(this,  &TUnitCell::LibMatrixCount, "MatrixCount", fpNone,
"Returns the number of matrices in the unit cell") );
  return lib;
}
//..............................................................................

