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
  for( int i=0; i < Ellipsoids.Count(); i++ )  {
    for( int j=0; j < Ellipsoids[i].Count(); j++ )
      if( Ellipsoids[i][j] != NULL )
        delete Ellipsoids[i][j];
  }
  Ellipsoids.Clear();
}
//..............................................................................
void TUnitCell::AddEllipsoid()  {
  Ellipsoids.SetCount( Ellipsoids.Count() + 1);
  for( int j=0; j < Matrices.Count(); j++ )
    Ellipsoids.Last().Add( NULL );
}
//..............................................................................
void TUnitCell::Clear()  {
  Matrices.Clear();
  ClearEllipsoids();
}
//..............................................................................
double TUnitCell::CalcVolume()  const  {
  double cosa = cos( GetLattice().GetAsymmUnit().Angles()[0].GetV()*M_PI/180 ),
         cosb = cos( GetLattice().GetAsymmUnit().Angles()[1].GetV()*M_PI/180 ),
         cosg = cos( GetLattice().GetAsymmUnit().Angles()[2].GetV()*M_PI/180 );
  return  GetLattice().GetAsymmUnit().Axes()[0].GetV()*
          GetLattice().GetAsymmUnit().Axes()[1].GetV()*
          GetLattice().GetAsymmUnit().Axes()[2].GetV()*sqrt( (1-cosa*cosa-cosb*cosb-cosg*cosg) + 2*(cosa*cosb*cosg));
}
//..............................................................................
int TUnitCell::GetMatrixMultiplier(short Latt)  {
  int count = 0;
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
  int ac;
  TCAtom *A1;
  vec3d Vec, Vec1, Vec2;
  TEVPointD EP;
  TEStrBuffer Tmp;
  Matrices.Clear();
  // check if the E matrix is in the list
  for( int i=0;  i < GetLattice().GetAsymmUnit().MatrixCount(); i++ )  {
    const smatd& m = GetLattice().GetAsymmUnit().GetMatrix(i);
    if( m.r.IsI() )  continue;  // will need to insert the identity matrix at position 0
    Matrices.AddNew( m );
  }

  smatd* M = new smatd;  // insert the identity matrix at position 0
  M->r.I();
  Matrices.Insert(0, *M);  // I Matrix

  for( int i=0; i < Matrices.Count(); i++ )  {
    smatd& m = Matrices[i];
    switch( abs(GetLattice().GetAsymmUnit().GetLatt()) )  {
      case 1: break;
      case 2:      // Body Centered (I)
        M = new smatd(m);
        M->t[0] += 0.5f;     M->t[1] += 0.5f;     M->t[2] += 0.5f;
        Matrices.Insert(i+1, *M);
        i++;
        break;
      case 3:      // R Centered
        M = new smatd(m);
        M->t[0] += (2./3.);  M->t[1] += (1./3.);  M->t[2] += (1./3.);
        Matrices.Insert(i+1, *M);
        i++;
        M = new smatd(m);
        M->t[0] += (1./3.);  M->t[1] += (2./3.);  M->t[2] += (2./3.);
        Matrices.Insert(i+1, *M);
        i++;
        break;
      case 4:      // Face Centered (F)
        M = new smatd(m);
        M->t[0] += 0.0;    M->t[1] += 0.5;  M->t[2] += 0.5;
        Matrices.Insert(i+1, *M);
        i++;
        M = new smatd(m);
        M->t[0] += 0.5;  M->t[1] += 0;    M->t[2] += 0.5;
        Matrices.Insert(i+1, *M);
        i++;
        M = new smatd(m);
        M->t[0] += 0.5;  M->t[1] += 0.5;  M->t[2] += 0;
        Matrices.Insert(i+1, *M);
        i++;
        break;
      case 5:      // A Centered (A)
        M = new smatd(m);
        M->t[0] += 0;    M->t[1] += 0.5;  M->t[2] += 0.5;
        Matrices.Insert(i+1, *M);
        i++;
        break;
      case 6:      // B Centered (B)
        M = new smatd(m);
        M->t[0] += 0.5;  M->t[1] += 0;    M->t[2] += 0.5;
        Matrices.Insert(i+1, *M);
        i++;
        break;
      case 7:      // C Centered (C);
        M = new smatd(m);
        M->t[0] += 0.5;  M->t[1] += 0.5;  M->t[2] += 0;
        Matrices.Insert(i+1, *M);
        i++;
        break;
      default:
        throw TInvalidArgumentException(__OlxSourceInfo, "LATT");
    }
  }
  if( GetLattice().GetAsymmUnit().GetLatt() > 0 )  {
    for( int i=0; i < Matrices.Count(); i++ )  {
      M = new smatd(Matrices[i]);
      *M *= -1;
      Matrices.Insert(i+1, *M);
      i++;
    }
  }

  ac = GetLattice().GetAsymmUnit().AtomCount();

  mat3d abc2xyz( GetLattice().GetAsymmUnit().GetCellToCartesian()),
           xyz2abc( GetLattice().GetAsymmUnit().GetCartesianToCell());
  abc2xyz.Transpose();
  xyz2abc.Transpose();

  TEllipsoid *E;
  Ellipsoids.Clear();
  Ellipsoids.SetCount(ac);
  for( int i=0; i < ac; i++ )  {
    Ellipsoids[i].SetCapacity(Matrices.Count());
    for( int j=0; j < Matrices.Count(); j++ )
      Ellipsoids[i].Add( NULL );
  }
  for( int i=0; i < ac; i++ )  {
    A1 = &GetLattice().GetAsymmUnit().GetAtom(i);
    for( int j=0; j < Matrices.Count(); j++ )  {
      if( A1->GetEllipsoid() != NULL )  {
        E = new TEllipsoid;
        E->SetId( j*ac+A1->GetId() );
        *E = *A1->GetEllipsoid();
        E->MultMatrix( abc2xyz*Matrices[j].r*xyz2abc );
        Ellipsoids[i][j] = E;
      }
    }
  }
  for( int i=0; i < Matrices.Count(); i++ )
    Matrices[i].SetTag(i);
}
//..............................................................................
const TEllipsoid& TUnitCell::GetEllipsoid(int MatrixId, int AUId) const  {
  return *Ellipsoids[AUId][MatrixId];
}
//..............................................................................
TUnitCell::TSearchSymmEqTask::TSearchSymmEqTask(TPtrList<TCAtom>& atoms,
  const smatd_list& matrices, TStrList& report, double tol, bool initialise) :
                  Atoms(atoms), Matrices(matrices), Report(report), tolerance(tol)  {
  Initialise = initialise;
  AU = atoms[0]->GetParent();
  Latt = &AU->GetLattice();
}
//..............................................................................
void TUnitCell::TSearchSymmEqTask::Run(long ind)  {
  vec3d Vec, Vec1;
  for( int i=ind; i < Atoms.Count(); i++ )  {
    if( Atoms[i]->GetTag() == -1 )  continue;
    for( int j=0; j < Matrices.Count(); j++ )  {
      Vec1 = Matrices[j] * Atoms[i]->ccrd();
      Vec = Atoms[ind]->ccrd();
      Vec -= Vec1;
      int iLx = Round(Vec[0]);  Vec[0] -= iLx;
      int iLy = Round(Vec[1]);  Vec[1] -= iLy;
      int iLz = Round(Vec[2]);  Vec[2] -= iLz;
      // skip I
      if( j == 0 && iLx == 0 && iLy == 0 && iLz == 0 )  {
        if( !Initialise || ind == i )  continue;
        if( Atoms[i]->GetFragmentId() == Atoms[ind]->GetFragmentId() )  continue;
        AU->CellToCartesian(Vec);
        double Dis = Vec.Length();
        if( Latt->GetNetwork().HBondExists(*Atoms[ind], *Atoms[i], Dis) )  {
          Atoms[ind]->AttachAtomI( Atoms[i] );
          Atoms[i]->AttachAtomI( Atoms[ind] );
        }
        continue;
      }

      AU->CellToCartesian(Vec);
      double Dis = Vec.Length();
      if( (j != 0) && (Dis < tolerance) )  {
        if( i == ind )  {
          if( Initialise )  Atoms[ind]->SetDegeneracy( Atoms[ind]->GetDegeneracy() + 1 );
          continue;
        }
        if( Atoms[i]->GetAtomInfo() != Atoms[ind]->GetAtomInfo() ) continue;
        Report.Add( olxstr(Atoms[ind]->Label(), 10) << '-' << Atoms[i]->GetLabel() );
        Atoms[i]->SetTag(-1);
        break;
      }
      else  {
        if( !Initialise )  continue;
        if( Latt->GetNetwork().CBondExists(*Atoms[ind], *Atoms[i], Dis) )  {
          Atoms[ind]->SetCanBeGrown(true);
          if( Atoms[ind]->IsAttachedTo( *Atoms[i] ) )  continue;
          Atoms[ind]->AttachAtom( Atoms[i] );
          if( i != ind )  {
            Atoms[i]->SetCanBeGrown(true);
            Atoms[i]->AttachAtom(Atoms[ind]);
          }
        }
        else if( Latt->GetNetwork().HBondExists(*Atoms[ind], *Atoms[i], Dis) )  {
          Atoms[ind]->AttachAtomI( Atoms[i] );
          if( i != ind )
            Atoms[i]->AttachAtomI( Atoms[ind] );
        }
      }
    }
  }
}
//..............................................................................
int TUnitCell::FindSymmEq(TEStrBuffer &Msg, double tol, bool Initialise, bool remove, bool markDeleted) const  {
  TStrList report;
  TCAtomPList ACA;
  // sorting the content of the asymmetric unit in order to improve the algorithm
  // which is a bit sluggish for a big system; however it will not be ever used
  // in the grow-fuse (XP) cycle: a save and load operations will be used in that
  // case
  ACA.SetCapacity( GetLattice().GetAsymmUnit().AtomCount() );
  for( int i=0; i < GetLattice().GetAsymmUnit().AtomCount(); i++ )  {
    TCAtom& A1 = GetLattice().GetAsymmUnit().GetAtom(i);
    if( A1.IsDeleted() )  continue;
    ACA.Add( &A1 );
    A1.SetTag(0);
    if( Initialise )
      A1.ClearAttachedAtoms();
  }
  // searching for symmetrical equivalents; the search could be optimised by
  // removing the translational equivalents in the firts order; however the task is not
  // very common, so it should be OK. (An identity (E) matrix is in the list
  // so translational equivalents will be removed too
  if( ACA.IsEmpty() )  return 0;
  TSearchSymmEqTask searchTask(ACA, Matrices, report, tol, Initialise);

  TListIteratorManager<TSearchSymmEqTask> searchm(searchTask, ACA.Count(), tQuadraticTask, 1000);

  if( remove )  {
    for( int i=0; i < ACA.Count(); i++ )
      if( ACA[i]->GetTag() == -1 )
        GetLattice().GetAsymmUnit().NullAtom( ACA[i]->GetId() );
    GetLattice().GetAsymmUnit().PackAtoms(); // remove the NULL pointers
  }
  else if( markDeleted )  {
    for( int i=0; i < ACA.Count(); i++ )
      if( ACA[i]->GetTag() == -1 )
        GetLattice().GetAsymmUnit().NullAtom( ACA[i]->GetId() );
    GetLattice().GetAsymmUnit().PackAtoms(); // remove the NULL pointers
  }
  if( report.Count() )  {
    Msg << "Symmetrical equivalents: ";
    Msg << report.Text(';');
  }
  return report.Count();
}
//..............................................................................
smatd* TUnitCell::GetClosest(const TCAtom& to, const TCAtom& atom, bool ConsiderOriginal) const  {
  return GetClosest(to.ccrd(), atom.ccrd(), ConsiderOriginal);
}
//..............................................................................
smatd* TUnitCell::GetClosest(const vec3d& to, const vec3d& from, bool ConsiderOriginal) const  {
  const smatd* minMatr = NULL;
  vec3d V1;
  int minix, miniy, miniz;
  double minD=1000;
  if( ConsiderOriginal )  {
    V1 = from-to;
    GetLattice().GetAsymmUnit().CellToCartesian(V1);
    minD = V1.QLength();
  }
  for( int i=0; i < Matrices.Count(); i++ )  {
    const smatd& matr = Matrices[i];
    V1 = matr * from;
    V1 -= to;
    int ix = Round(V1[0]);  V1[0] -= (ix);  // find closest distance
    int iy = Round(V1[1]);  V1[1] -= (iy);
    int iz = Round(V1[2]);  V1[2] -= (iz);
    // check for identity matrix
    if( !i && !ix && !iy && !iz )  continue;
    GetLattice().GetAsymmUnit().CellToCartesian(V1);
    double D = V1.QLength();
    if( D < minD )  {
      minD = D;
      minMatr = &matr;
      minix = ix;  miniy = iy; miniz = iz;
    }
    else  {
      if( D == minD && minMatr == NULL )  {
        minMatr = &matr;
        minix = ix;  miniy = iy; miniz = iz;
      }
    }
  }
  if( minMatr != NULL)  {
    smatd* retVal = new smatd(*minMatr);
    retVal->t[0] -= minix;
    retVal->t[1] -= miniy;
    retVal->t[2] -= miniz;
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
  for( int i=0; i < MatrixCount(); i++ )  {
    const smatd& matr = GetMatrix(i);
    V1 = matr * from - to;
    V1[0] -= Round(V1[0]);  // find closest distance
    V1[1] -= Round(V1[1]);
    V1[2] -= Round(V1[2]);
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
  smatd* newMatr;
  smatd_list* retVal = new smatd_list;
  vec3d V1;

  for( int i=0; i < MatrixCount(); i++ )  {
    const smatd& matr = GetMatrix(i);
    V1 = matr * from - to;
    int ix = Round(V1[0]);  V1[0] -= (ix);
    int iy = Round(V1[1]);  V1[1] -= (iy);
    int iz = Round(V1[2]);  V1[2] -= (iz);
    // check for identity matrix
    if( !IncludeI )  if( !i && !ix && !iy && !iz )  continue;
    GetLattice().GetAsymmUnit().CellToCartesian(V1);
    double D = V1.Length();
    if( GetLattice().GetNetwork().CBondExists(toA, fromA, D) )  {
      newMatr = new smatd(matr);
      newMatr->t[0] -= ix;
      newMatr->t[1] -= iy;
      newMatr->t[2] -= iz;
      retVal->Add(*newMatr);
    }
    else if( IncludeHBonds )  {
      if( GetLattice().GetNetwork().HBondExists(toA, fromA, D) )  {
        newMatr = new smatd(matr);
        newMatr->t[0] -= ix;
        newMatr->t[1] -= iy;
        newMatr->t[2] -= iz;
        retVal->Add(*newMatr);
      }
    }
  }
  return retVal;
}
//..............................................................................
smatd_list* TUnitCell::GetInRange(const vec3d& to, const vec3d& from, double R, bool IncludeI) const  {
  smatd_list* retVal = new smatd_list;
  smatd* retMatr;
  vec3d V1;
  R *= R;
  for( int i=0; i < MatrixCount(); i++ )  {
    const smatd& matr = GetMatrix(i);
    V1 = matr * from;
    V1 -= to;
    int ix = Round(V1[0]);  V1[0] -= ix;
    int iy = Round(V1[1]);  V1[1] -= iy;
    int iz = Round(V1[2]);  V1[2] -= iz;
    // check for identity matrix
    if( !IncludeI && !i )
      if( !ix && !iy && !iz )  continue;
    GetLattice().GetAsymmUnit().CellToCartesian(V1);
    double D = V1.QLength();
    if( D < R )  {
      retMatr = new smatd(matr);
      retMatr->t[0] -= ix;
      retMatr->t[1] -= iy;
      retMatr->t[2] -= iz;
      retVal->Add(*retMatr);
    }
  }
  return retVal;
}
//..............................................................................
void TUnitCell::FindInRange(const TCAtom& atom, double R, 
                            TArrayList< AnAssociation2<TCAtom const*,vec3d> >& res) const {

  const TAsymmUnit& au = GetLattice().GetAsymmUnit();
  vec3d to( atom.ccrd() );
  vec3d V1;
  R *= R;
  for( int i=0; i < au.AtomCount(); i++ )  {
    const TCAtom& a = au.GetAtom(i);
    if( a.IsDeleted() )  continue;
    for( int j=0; j < MatrixCount(); j++ )  {
      const smatd& matr = GetMatrix(j);
      V1 = matr * a.ccrd() - to;
      V1[0] -= Round(V1[0]);  // find closest distance
      V1[1] -= Round(V1[1]);
      V1[2] -= Round(V1[2]);
      GetLattice().GetAsymmUnit().CellToCartesian(V1);
      double D = V1.QLength();
      if( D < R && D > 0.01 )  {
        res.Add( AnAssociation2<TCAtom const*, vec3d>(&a, V1) );
      }
    }
  }
}
//..............................................................................
smatd_list* TUnitCell::GetInRangeEx(const vec3d& to, const vec3d& from, 
                                       float R, bool IncludeI, const smatd_list& ToSkip) const  {
  smatd_list* retVal = new smatd_list;
  smatd* retMatr;
  vec3d V1, V2;
  R *= R;
  for( int i=0; i < MatrixCount(); i++ )  {
    const smatd& matr = GetMatrix(i);
    V1 = matr * from - to;
    int ix = Round(V1[0]);  V1[0] -= ix;
    int iy = Round(V1[1]);  V1[1] -= iy;
    int iz = Round(V1[2]);  V1[2] -= iz;
    for( int j=0; j < 3; j++ )  {
      for( int k=-1; k <= 1; k++ )  {
        V2 = V1;
        V2[j] += k;
      // check for identity matrix
        if( !IncludeI && !i & !k )
          if( !ix && !iy && !iz )  continue;
        GetLattice().GetAsymmUnit().CellToCartesian(V2);
        double D = V2.QLength();
        if( D < R )  {
          retMatr = new smatd(matr);
          retMatr->t[0] -= ix;
          retMatr->t[1] -= iy;
          retMatr->t[2] -= iz;
          retMatr->t[j] += k;
          if( ToSkip.IndexOf( *retMatr ) != -1 )  {
            delete retMatr;
            continue;
          }
          retVal->Add(*retMatr);
        }
      }
    }
  }
  return retVal;
}
//..............................................................................
void TUnitCell::GetAtomEnviList(TSAtom& atom, TAtomEnvi& envi, bool IncludeQ )  const {
  if( atom.IsGrown() )
    throw TFunctionFailedException(__OlxSourceInfo, "not implementd for grown atoms");

  envi.SetBase( atom );

  smatd I;
  I.r.I();

  for( int i=0; i < atom.NodeCount(); i++ )  {
    TSAtom& A = atom.Node(i);
    if( A.IsDeleted() ) continue;
    if( !IncludeQ && A.GetAtomInfo() == iQPeakIndex )  continue;
    envi.Add( A.CAtom(), I, A.crd() );
  }
  vec3d v;
  for( int i=0; i < atom.CAtom().AttachedAtomCount(); i++ )  {
    TCAtom& A = atom.CAtom().GetAttachedAtom(i);
    if( !IncludeQ && A.GetAtomInfo() == iQPeakIndex )  continue;

    smatd* m = GetClosest( atom.ccrd(), A.ccrd(), false );
    if( m == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "Could not find symmetry generated atom");
    v = *m * A.ccrd();
    this->GetLattice().GetAsymmUnit().CellToCartesian(v);
    // make sure that atoms on center of symmetry are not counted twice
    bool Add = true;
    for( int j=0; j < envi.Count(); j++ )  {
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
void TUnitCell::GetAtomQEnviList(TSAtom& atom, TAtomEnvi& envi)  {
  if( atom.IsGrown() )
    throw TFunctionFailedException(__OlxSourceInfo, "Not implementd for grown structre");

  envi.SetBase( atom );

  smatd I;
  I.r.I();

  for( int i=0; i < atom.NodeCount(); i++ )  {
    TSAtom& A = atom.Node(i);
    if( A.IsDeleted() ) continue;
    if( A.GetAtomInfo() == iQPeakIndex )
      envi.Add( A.CAtom(), I, A.crd() );
  }
  vec3d v;
  for( int i=0; i < atom.CAtom().AttachedAtomCount(); i++ )  {
    TCAtom& A = atom.CAtom().GetAttachedAtom(i);
    if( A.GetAtomInfo() != iQPeakIndex )  continue;

    smatd* m = GetClosest( atom.ccrd(), A.ccrd(), false );
    if( m == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "Could not find symmetry generated atom");
    v = *m * A.ccrd();
    GetLattice().GetAsymmUnit().CellToCartesian(v);
    // make sure that atoms on center of symmetry are not counted twice
    bool Add = true;
    for( int j=0; j < envi.Count(); j++ )  {
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
  if( ae.GetBase().IsGrown() )
    throw TFunctionFailedException(__OlxSourceInfo, "Not implementd for grown structre");

  envi.SetBase( ae.GetBase() );

  smatd I;
  I.r.I();

  TAsymmUnit& au = GetLattice().GetAsymmUnit();

  vec3d v, v1, v2;
  for( int i=0; i < au.AtomCount(); i++ )  {
    TCAtom& A = au.GetAtom(i);
    if( A.GetAtomInfo() == iQPeakIndex || A.IsDeleted() )  continue;

    bool considerI =  (A != ae.GetBase().CAtom());
    // O and N for a while
    if( !( A.GetAtomInfo() == iOxygenIndex ||
           A.GetAtomInfo() == iNitrogenIndex) )  continue;

    smatd_list& ms = *GetInRange( ae.GetBase().ccrd(), A.ccrd(), 2.9, considerI );
    if( &ms == NULL )  continue;

    for( int j=0; j < ms.Count(); j++ )  {
      v = ms[j] * A.ccrd();
      au.CellToCartesian(v);
      double d = v.DistanceTo( ae.GetBase().crd() );
      if(  d < 2 || d > 2.9 )  continue;

      if( ae.Count() == 1 )  {
        v1 = ae.GetCrd(0);  v1 -= ae.GetBase().crd();
        v2 = v;             v2 -= ae.GetBase().crd();
        // 89 - 130 degrees
        d = v1.CAngle(v2);
        if( d > 0.01 || d < -0.64 )  continue;
      }
//      else
//        continue;

      // make sure that atoms on center of symmetry are not counted twice
      bool Add = true;
      for( int k=0; k < envi.Count(); k++ )  {
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
bool TUnitCell::DoesOverlap(const TCAtom& ca, double R) const  {
  vec3d V1, V2;
  TCAtomPList atoms;
  atoms.SetCapacity( GetLattice().GetAsymmUnit().AtomCount() );
  for( int i=0; i < GetLattice().GetAsymmUnit().AtomCount(); i++ )  {
    if( GetLattice().GetAsymmUnit().GetAtom(i).IsDeleted() )  continue;
    atoms.Add( &GetLattice().GetAsymmUnit().GetAtom(i) );
  }

  for( int i=0; i < MatrixCount(); i++ )  {
    const smatd& matr = GetMatrix(i);
    V1 = matr * ca.ccrd();
    for( int j=0; j < atoms.Count(); j++ )  {
      if( atoms[j]->GetLoaderId() == ca.GetLoaderId() )  
        continue;
      V2 = atoms[j]->ccrd() - V1;
      V2[0] -= Round(V2[0]);  // find closest distance
      V2[1] -= Round(V2[1]);
      V2[2] -= Round(V2[2]);
      GetLattice().GetAsymmUnit().CellToCartesian(V2);
      double D = V2.Length();
      if( D < R )  
        return true;
    }
  }
  return false;
}
//..............................................................................
TCAtom* TUnitCell::FindOverlappingAtom(vec3d& pos, double delta) const  {
  vec3d V1, V2, nearest(pos);
  double minD = 1000;
  delta *= delta;
  const TAsymmUnit& au = GetLattice().GetAsymmUnit();

  for( int i=0; i < MatrixCount(); i++ )  {
    const smatd& matr = GetMatrix(i);
    V1 = matr * pos;
    for( int j=0; j < au.AtomCount(); j++ )  {
      const TCAtom& ca = au.GetAtom(j);
      if( ca.IsDeleted() )  continue;
      V2 = ca.ccrd() - V1;
      if( ca.GetAtomInfo() == iQPeakIndex )  {
        for( int k=0; k < 3; k ++ )  { // find closest distance
          V2[k] -= Round(V2[k]);
          if( V2[k] < 0 )     V2[k] += 1;
          if( V2[k] < 0.05 )  V2[k] = 0;
          else if( V2[k] > 0.95 )  V2[k] = 0;
        }
      }
      else  {
        nearest = V1;
        for( int k=0; k < 3; k ++ )  { // find closest distance
          int iv = Round(V2[k]);
          V2[k] -= iv;
          nearest[k] += iv;
          if( V2[k] < 0 )  {     
            V2[k] += 1;
            nearest[k] -= 1;
          }
          if( V2[k] < 0.05 )  {
            V2[k] = 0;
            nearest[k] = ca.ccrd()[k];
          }
          else if( V2[k] > 0.95 )  {
            V2[k] = 0;
            nearest[k] = ca.ccrd()[k];
          }
        }
      }
      au.CellToCartesian(V2);
      double qd = V2.QLength();
	  if( qd < minD && ca.GetAtomInfo() != iQPeakIndex )  {
        minD = qd;
        pos = nearest;
      }
      if( qd < delta )  
        return &au.GetAtom(j);
    }
  }
  return NULL;
}
//..............................................................................
TCAtom* TUnitCell::FindCAtom(const vec3d& center) const  {
  vec3d Vec;
  const TAsymmUnit& au = GetLattice().GetAsymmUnit();
  for( int i=0; i < au.AtomCount(); i++ )  {
    const TCAtom& A = au.GetAtom(i);
    if( A.IsDeleted() )  continue;
    for( int j=0; j < MatrixCount(); j ++ )  {
      const smatd& M = Matrices[j];
      Vec = M * A.ccrd() - center;
      for( int l=0; l < 3; l++ )  {
        Vec[l] -= Round(Vec[l]);
        if( Vec[l] < 0 )  Vec[l] += 1;
        if( Vec[l] < 0.01 )  Vec[l] = 0;
        else if( (1-Vec[l]) < 0.01 )  Vec[l] = 0;
        else if( Vec[l] > 0.99 )  Vec[l] = 0;
      }
      GetLattice().GetAsymmUnit().CellToCartesian(Vec);
      if( Vec.QLength() < 0.01 )
        return &au.GetAtom(i);
    }
  }
  return NULL;
}
//..............................................................................
void TUnitCell::BuildStructureMap( TArray3D<short>& map, double delta, short val, 
                                  long* structurePoints, TPSTypeList<TBasicAtomInfo*, double>* radii)  {

  TTypeList< AnAssociation2<vec3d,TCAtom*> > allAtoms;
  vec3d center, center1;
  GenereteAtomCoordinates( allAtoms, true );
  
  double da = map.Length1(),
         db = map.Length2(),
         dc = map.Length3();
  double dim[] = {da, db, dc};
  // angstrem per pixel
  double capp = Lattice->GetAsymmUnit().Axes()[2].GetV()/dc,
         bapp = Lattice->GetAsymmUnit().Axes()[1].GetV()/db,
         aapp = Lattice->GetAsymmUnit().Axes()[0].GetV()/da;
  // pixel per angstrem
  double cppa = dc/Lattice->GetAsymmUnit().Axes()[2].GetV(),
         bppa = db/Lattice->GetAsymmUnit().Axes()[1].GetV(),
         appa = dc/Lattice->GetAsymmUnit().Axes()[0].GetV();
  double scppa = cppa*cppa, 
         sbppa = bppa*bppa,
         sappa = appa*appa;
  map.FastInitWith(0);
  // precalculate the sphere/ellipsoid etc coordinates for all distinct scatterers
  const TAsymmUnit& au = GetLattice().GetAsymmUnit();
  TPSTypeList<int, TArray3D<bool>* > scatterers;
  for( int i=0; i < au.AtomCount(); i++ )  {
    if( au.GetAtom(i).IsDeleted() )  continue;
    int ind = scatterers.IndexOfComparable( au.GetAtom(i).GetAtomInfo().GetIndex() );
    if( ind != -1 )  continue;
    double r = au.GetAtom(i).GetAtomInfo().GetRad2() + delta;
    if( radii != NULL )  {
      int b_i = radii->IndexOfComparable( &au.GetAtom(i).GetAtomInfo() );
      if( b_i != -1 )
        r = radii->GetObject(b_i) + delta;
    }
    const double sr = r*r;
    TArray3D<bool>* spm = new TArray3D<bool>(0, (int)(r*appa), 0, (int)(r*bppa), 0, (int)(r*cppa));
    for( int x=0; x < spm->Length1(); x ++ )  {
      for( int y=0; y < spm->Length2(); y ++ )  {
        for( int z=0; z < spm->Length3(); z ++ )  {
          int R = Round(x*x/sappa + y*y/sbppa + z*z/scppa);
          spm->Data[x][y][z] = (R < sr);
        }
      }
    }
    scatterers.Add(au.GetAtom(i).GetAtomInfo().GetIndex(), spm);
  }

  for( int i=0; i < allAtoms.Count(); i++ )  {
    TArray3D<bool>* spm = scatterers[ allAtoms[i].GetB()->GetAtomInfo().GetIndex() ];
    for( int j = 0; j < spm->Length1(); j ++ )  {
      for( int k = 0; k < spm->Length2(); k ++ )  {
        for( int l = 0; l < spm->Length3(); l ++ )  {
          if( !spm->Data[j][k][l] )  continue;
          // 1.+++
          center1 = allAtoms[i].GetA();
          center1[0] *= da; center1[1] *= db; center1[2] *= dc;
          center = center1;
          center[0] += j;  center[1] += k;  center[2] += l;
          for( int m=0; m < 3; m++ )  {
            if( center[m] < 0 )        center[m] += dim[m];
            if( center[m] >= dim[m] )  center[m] -= dim[m];
          }
          map.Data[(int)center[0]][(int)center[1]][(int)center[2]] = val;
          // 2.-++
          center = center1;
          center[0] -= j;  center[1] += k;  center[2] += l;
          for( int m=0; m < 3; m++ )  {
            if( center[m] < 0 )        center[m] += dim[m];
            if( center[m] >= dim[m] )  center[m] -= dim[m];
          }
          map.Data[(int)center[0]][(int)center[1]][(int)center[2]] = val;
          // 3.+-+
          center = center1;
          center[0] += j;  center[1] -= k;  center[2] += l;
          for( int m=0; m < 3; m++ )  {
            if( center[m] < 0 )        center[m] += dim[m];
            if( center[m] >= dim[m] )  center[m] -= dim[m];
          }
          map.Data[(int)center[0]][(int)center[1]][(int)center[2]] = val;
          // 4.++-
          center = center1;
          center[0] += j;  center[1] += k;  center[2] -= l;
          for( int m=0; m < 3; m++ )  {
            if( center[m] < 0 )        center[m] += dim[m];
            if( center[m] >= dim[m] )  center[m] -= dim[m];
          }
          map.Data[(int)center[0]][(int)center[1]][(int)center[2]] = val;
          // 5.--+
          center = center1;
          center[0] -= j;  center[1] -= k;  center[2] += l;
          for( int m=0; m < 3; m++ )  {
            if( center[m] < 0 )        center[m] += dim[m];
            if( center[m] >= dim[m] )  center[m] -= dim[m];
          }
          map.Data[(int)center[0]][(int)center[1]][(int)center[2]] = val;
          // 6.-+-
          center = center1;
          center[0] -= j;  center[1] += k;  center[2] -= l;
          for( int m=0; m < 3; m++ )  {
            if( center[m] < 0 )        center[m] += dim[m];
            if( center[m] >= dim[m] )  center[m] -= dim[m];
          }
          map.Data[(int)center[0]][(int)center[1]][(int)center[2]] = val;
          // 7.+--
          center = center1;
          center[0] += j;  center[1] -= k;  center[2] -= l;
          for( int m=0; m < 3; m++ )  {
            if( center[m] < 0 )        center[m] += dim[m];
            if( center[m] >= dim[m] )  center[m] -= dim[m];
          }
          map.Data[(int)center[0]][(int)center[1]][(int)center[2]] = val;
          // 8.---
          center = center1;
          center[0] -= j;  center[1] -= k;  center[2] -= l;
          for( int m=0; m < 3; m++ )  {
            if( center[m] < 0 )        center[m] += dim[m];
            if( center[m] >= dim[m] )  center[m] -= dim[m];
          }
          map.Data[(int)center[0]][(int)center[1]][(int)center[2]] = val;
        }
      }
    }
  }
  if( structurePoints != NULL )  {
    long sp = 0;
    int mapX = map.Length1(), mapY = map.Length2(), mapZ = map.Length3();
    for(int i=0; i < mapX; i++ )
      for(int j=0; j < mapY; j++ )
        for(int k=0; k < mapZ; k++ )
          if( map.Data[i][j][k] == val )  {
            sp ++;
          }
    *structurePoints = sp;
  }

  for( int i=0; i < scatterers.Count(); i++ )
    delete scatterers.Object(i);
}

