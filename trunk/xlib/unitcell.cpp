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
  TVPointD Vec, Vec1, Vec2;
  TEVPointD EP;
  TEStrBuffer Tmp;
  // check if the E matrix is in the list
  for( int i=0;  i < GetLattice().GetAsymmUnit().MatrixCount(); i++ )  {
    const TMatrixD& m = GetLattice().GetAsymmUnit().GetMatrix(i);
    if( m.IsE() )  continue;  // will need to insert the identity matrix at position 0
    Matrices.AddNew( m );
  }

  TMatrixD* M = new TMatrixD(3,4);  // insert the identity matrix at position 0
  M->E();
  Matrices.Insert(0, *M);  // I Matrix

  for( int i=0; i < Matrices.Count(); i++ )  {
    TMatrixD& m = Matrices[i];
    switch( abs(GetLattice().GetAsymmUnit().GetLatt()) )  {
      case 1: break;
      case 2:      // Body Centered (I)
        M = new TMatrixD(m);
        M->Data(0)[3] += 0.5f;  M->Data(1)[3] += 0.5f;  M->Data(2)[3] += 0.5f;
        Matrices.Insert(i+1, *M);
        i++;
        break;
      case 3:      // R Centered
        M = new TMatrixD(m);
        M->Data(0)[3] += (2./3.);  M->Data(1)[3] += (1./3.);  M->Data(2)[3] += (1./3.);
        Matrices.Insert(i+1, *M);
        i++;
        M = new TMatrixD(m);
        M->Data(0)[3] += (1./3.);  M->Data(1)[3] += (2./3.);  M->Data(2)[3] += (2./3.);
        Matrices.Insert(i+1, *M);
        i++;
        break;
      case 4:      // Face Centered (F)
        M = new TMatrixD(m);
        M->Data(0)[3] += 0.0;    M->Data(1)[3] += 0.5;  M->Data(2)[3] += 0.5;
        Matrices.Insert(i+1, *M);
        i++;
        M = new TMatrixD(m);
        M->Data(0)[3] += 0.5;  M->Data(1)[3] += 0;    M->Data(2)[3] += 0.5;
        Matrices.Insert(i+1, *M);
        i++;
        M = new TMatrixD(m);
        M->Data(0)[3] += 0.5;  M->Data(1)[3] += 0.5;  M->Data(2)[3] += 0;
        Matrices.Insert(i+1, *M);
        i++;
        break;
      case 5:      // A Centered (A)
        M = new TMatrixD(m);
        M->Data(0)[3] += 0;    M->Data(1)[3] += 0.5;  M->Data(2)[3] += 0.5;
        Matrices.Insert(i+1, *M);
        i++;
        break;
      case 6:      // B Centered (B)
        M = new TMatrixD(m);
        M->Data(0)[3] += 0.5;  M->Data(1)[3] += 0;    M->Data(2)[3] += 0.5;
        Matrices.Insert(i+1, *M);
        i++;
        break;
      case 7:      // C Centered (C);
        M = new TMatrixD(m);
        M->Data(0)[3] += 0.5;  M->Data(1)[3] += 0.5;  M->Data(2)[3] += 0;
        Matrices.Insert(i+1, *M);
        i++;
        break;
      default:
        throw TInvalidArgumentException(__OlxSourceInfo, "LATT");
    }
  }
  if( GetLattice().GetAsymmUnit().GetLatt() > 0 )  {
    for( int i=0; i < Matrices.Count(); i++ )  {
      M = new TMatrixD(Matrices[i]);
      *M *= -1;
      Matrices.Insert(i+1, *M);
      i++;
    }
  }

  ac = GetLattice().GetAsymmUnit().AtomCount();

  TMatrixD abc2xyz( GetLattice().GetAsymmUnit().GetCellToCartesian()),
           xyz2abc( GetLattice().GetAsymmUnit().GetCartesianToCell()), NM(3,3);
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
      for( int k=0; k < 3; k++ )
        for( int l=0; l < 3; l++ )
          NM[k][l] = Matrices[j][k][l];
      if( A1->GetEllipsoid() != NULL )  {
        NM = abc2xyz*NM*xyz2abc;
        E = new TEllipsoid;
        E->SetId( j*ac+A1->GetId() );
        *E = *A1->GetEllipsoid();
        E->MultMatrix(NM);
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
  const TMatrixDList& matrices, TStrList& report, double tol, bool initialise) :
                  Atoms(atoms), Matrices(matrices), Report(report), tolerance(tol)  {
  Initialise = initialise;
  AU = atoms[0]->GetParent();
  Latt = &AU->GetLattice();
}
//..............................................................................
void TUnitCell::TSearchSymmEqTask::Run(long ind)  {
  TVPointD Vec, Vec1;
  for( int i=ind; i < Atoms.Count(); i++ )  {
    if( Atoms[i]->GetTag() == -1 )  continue;
    for( int j=0; j < Matrices.Count(); j++ )  {
      Vec1 = Atoms[i]->CCenter();
      Vec1 = Matrices[j] * Vec1;
      Vec1[0] += Matrices[j][0][3];
      Vec1[1] += Matrices[j][1][3];
      Vec1[2] += Matrices[j][2][3];
      Vec = Atoms[ind]->CCenter();
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
        if( Latt->GetNetwork().CBondExists(*Atoms[ind], *Atoms[i], Dis) )  {
          Atoms[ind]->SetCanBeGrown(true);
          if( Atoms[ind]->IsAttachedTo( *Atoms[i] ) )  continue;
          Atoms[ind]->AttachAtom( Atoms[i] );
          Atoms[i]->SetCanBeGrown(true);
          Atoms[i]->AttachAtom(Atoms[ind]);
        }
        else if( Latt->GetNetwork().HBondExists(*Atoms[ind], *Atoms[i], Dis) )  {
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
TMatrixD* TUnitCell::GetClosest(const TCAtom& to, const TCAtom& atom, bool ConsiderOriginal) const  {
  TVPointD V1, V2;
  V1 = to.GetCCenter();
  V2 = atom.GetCCenter();
  return GetClosest(V1, V2, ConsiderOriginal);
}
//..............................................................................
TMatrixD* TUnitCell::GetClosest(const TVPointD& to, const TVPointD& from, bool ConsiderOriginal) const  {
  const TMatrixD* minMatr = NULL;
  TVPointD V1, V2;
  int ix, iy, iz, minix, miniy, miniz;
  double D, minD=1000;
  if( ConsiderOriginal )  {
    V2 = from-to;
    GetLattice().GetAsymmUnit().CellToCartesian(V2);
    minD = V2.Length();
  }
  for( int i=0; i < MatrixCount(); i++ )  {
    const TMatrixD* matr = &GetMatrix(i);

    V1 = *matr * from;
    V1[0] += matr->Data(0)[3];  V1[1] += matr->Data(1)[3];  V1[2] += matr->Data(2)[3];
    V2 = V1;
    V2 -=to;
    ix = Round(V2[0]);  V2[0] -= (ix);  // find closest distance
    iy = Round(V2[1]);  V2[1] -= (iy);
    iz = Round(V2[2]);  V2[2] -= (iz);
    // check for identity matrix
    if( !i && !ix && !iy && !iz )  continue;
    GetLattice().GetAsymmUnit().CellToCartesian(V2);
    D = V2.Length();
    if( D < minD )  {
      minD = D;
      minMatr = matr;
      minix = ix;  miniy = iy; miniz = iz;
    }
    else  {
      if( D == minD && minMatr == NULL )  {
        minMatr = matr;
        minix = ix;  miniy = iy; miniz = iz;
      }
    }
  }
  if( minMatr != NULL)  {
    TMatrixD* retVal = new TMatrixD(*minMatr);
    retVal->Data(0)[3] -= minix;
    retVal->Data(1)[3] -= miniy;
    retVal->Data(2)[3] -= miniz;
    return retVal;
  }
  return NULL;
}
//..............................................................................
double TUnitCell::FindClosestDistance(const class TCAtom& a_from, const TCAtom& a_to) const  {
  TVPointD V1, from(a_from.GetCCenter()), to(a_to.GetCCenter());
  V1 = from-to;
  GetLattice().GetAsymmUnit().CellToCartesian(V1);
  double minD = V1.Length();
  for( int i=0; i < MatrixCount(); i++ )  {
    const TMatrixD* matr = &GetMatrix(i);
    V1 = *matr * from;
    V1[0] += matr->Data(0)[3];  V1[1] += matr->Data(1)[3];  V1[2] += matr->Data(2)[3];
    V1 -=to;
    V1[0] -= Round(V1[0]);  // find closest distance
    V1[1] -= Round(V1[1]);
    V1[2] -= Round(V1[2]);
    GetLattice().GetAsymmUnit().CellToCartesian(V1);
    double D = V1.Length();
    if( D < minD )  
      minD = D;
  }
  return minD;
}
//..............................................................................
TMatrixDList* TUnitCell::GetBinding(const TCAtom& toA, const TCAtom& fromA,
    const TVPointD& to, const TVPointD& from, bool IncludeI, bool IncludeHBonds) const  {
  TMatrixD* newMatr;
  TMatrixDList* retVal = new TMatrixDList();
  TVPointD V1, V2;

  for( int i=0; i < MatrixCount(); i++ )  {
    const TMatrixD& matr = GetMatrix(i);
    V1 = matr * from;
    V1[0] += matr[0][3];  V1[1] += matr[1][3]; V1[2] += matr[2][3];
    V2 = V1;
    V2 -= to;
    int ix = Round(V2[0]);  V2[0] -= (ix);
    int iy = Round(V2[1]);  V2[1] -= (iy);
    int iz = Round(V2[2]);  V2[2] -= (iz);
    // check for identity matrix
    if( !IncludeI )  if( !i && !ix && !iy && !iz )  continue;
    GetLattice().GetAsymmUnit().CellToCartesian(V2);
    double D = V2.Length();
    if( GetLattice().GetNetwork().CBondExists(toA, fromA, D) )  {
      newMatr = new TMatrixD(matr);
      newMatr->Data(0)[3] -= ix;
      newMatr->Data(1)[3] -= iy;
      newMatr->Data(2)[3] -= iz;
      retVal->Add(*newMatr);
    }
    else  if( IncludeHBonds )  {
      if( GetLattice().GetNetwork().HBondExists(toA, fromA, D) )  {
        newMatr = new TMatrixD(matr);
        newMatr->Data(0)[3] -= ix;
        newMatr->Data(1)[3] -= iy;
        newMatr->Data(2)[3] -= iz;
        retVal->Add(*newMatr);
      }
    }
  }
  return retVal;
}
//..............................................................................
TMatrixDList* TUnitCell::GetInRange(const TVPointD& to, const TVPointD& from, double R, bool IncludeI) const  {
  TMatrixDList* retVal = new TMatrixDList();
  TMatrixD* retMatr;
  TVPointD V1;
  R *= R;
  for( int i=0; i < MatrixCount(); i++ )  {
    const TMatrixD& matr = GetMatrix(i);
    V1 = matr * from;
    V1[0] += matr[0][3];  V1[1] += matr[1][3];  V1[2] += matr[2][3];
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
      retMatr = new TMatrixD(matr);
      retMatr->Data(0)[3] -= ix;
      retMatr->Data(1)[3] -= iy;
      retMatr->Data(2)[3] -= iz;
      retVal->Add(*retMatr);
    }
  }
  return retVal;
}
//..............................................................................
void TUnitCell::FindInRange(const TCAtom& atom, double R, 
                            TArrayList< AnAssociation2<TCAtom const*,TVPointD> >& res) const {

  const TAsymmUnit& au = GetLattice().GetAsymmUnit();
  TVPointD to( atom.GetCCenter() ), from;
  TVPointD V1;
  R *= R;
  for( int i=0; i < au.AtomCount(); i++ )  {
    const TCAtom& a = au.GetAtom(i);
    if( a.IsDeleted() )  continue;
    from = a.GetCCenter();
    for( int j=0; j < MatrixCount(); j++ )  {
      const TMatrixD& matr = GetMatrix(j);
      V1 = matr * from;
      V1[0] += matr[0][3];  V1[1] += matr[1][3];  V1[2] += matr[2][3];
      V1 -= to;
      V1[0] -= Round(V1[0]);  // find closest distance
      V1[1] -= Round(V1[1]);
      V1[2] -= Round(V1[2]);
      GetLattice().GetAsymmUnit().CellToCartesian(V1);
      double D = V1.QLength();
      if( D < R && D > 0.01 )  {
        res.Add( AnAssociation2<TCAtom const*, TVPointD>(&a, V1) );
      }
    }
  }
}
//..............................................................................
TMatrixDList* TUnitCell::GetInRangeEx(const TVPointD& to, const TVPointD& from, 
                                       float R, bool IncludeI, const TMatrixDList& ToSkip) const  {
  TMatrixDList* retVal = new TMatrixDList();
  TMatrixD* retMatr;
  TVPointD V1, V2, V3;
  R *= R;
  for( int i=0; i < MatrixCount(); i++ )  {
    const TMatrixD& matr = GetMatrix(i);
    V1 = matr * from;
    V1[0] += matr[0][3];  V1[1] += matr[1][3];  V1[2] += matr[2][3];
    V2 = V1;
    V2 -= to;
    int ix = Round(V2[0]);  V2[0] -= (ix);
    int iy = Round(V2[1]);  V2[1] -= (iy);
    int iz = Round(V2[2]);  V2[2] -= (iz);
    for( int j=0; j < 3; j++ )  {
      for( int k=-1; k <= 1; k++ )  {
        V3 = V2;
        V3[j] += k;
      // check for identity matrix
        if( !IncludeI && !i & !k )
          if( !ix && !iy && !iz )  continue;
        GetLattice().GetAsymmUnit().CellToCartesian(V3);
        double D = V3.QLength();
        if( D < R )  {
          retMatr = new TMatrixD(matr);
          retMatr->Data(0)[3] -= ix;
          retMatr->Data(1)[3] -= iy;
          retMatr->Data(2)[3] -= iz;
          retMatr->Data(j)[3] += k;
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

  TMatrixD I(3,4);
  I.E();

  for( int i=0; i < atom.NodeCount(); i++ )  {
    TSAtom& A = atom.Node(i);
    if( A.IsDeleted() ) continue;
    if( !IncludeQ && A.GetAtomInfo() == iQPeakIndex )  continue;
    envi.Add( A.CAtom(), I, A.Center() );
  }
  TVPointD v;
  for( int i=0; i < atom.CAtom().AttachedAtomCount(); i++ )  {
    TCAtom& A = atom.CAtom().GetAttachedAtom(i);
    if( !IncludeQ && A.GetAtomInfo() == iQPeakIndex )  continue;

    v = A.CCenter();
    TMatrixD* m = GetClosest( atom.CCenter(), v, false );
    if( m == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "Could not find symmetry generated atom");
    v = A.CCenter();
    v = *m * v;
    v[0] += m->Data(0)[3];  v[1] += m->Data(1)[3];  v[2] += m->Data(2)[3];
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

  TMatrixD I(3,4);
  I.E();

  for( int i=0; i < atom.NodeCount(); i++ )  {
    TSAtom& A = atom.Node(i);
    if( A.IsDeleted() ) continue;
    if( A.GetAtomInfo() == iQPeakIndex )
      envi.Add( A.CAtom(), I, A.Center() );
  }
  TVPointD v;
  for( int i=0; i < atom.CAtom().AttachedAtomCount(); i++ )  {
    TCAtom& A = atom.CAtom().GetAttachedAtom(i);
    if( A.GetAtomInfo() != iQPeakIndex )  continue;

    v = A.CCenter();
    TMatrixD* m = GetClosest( atom.CCenter(), v, false );
    if( m == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "Could not find symmetry generated atom");
    v = A.CCenter();
    v = *m * v;
    v[0] += m->Data(0)[3];
    v[1] += m->Data(1)[3];
    v[2] += m->Data(2)[3];
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

  TMatrixD I(3,4);
  I.E();

  TAsymmUnit& au = GetLattice().GetAsymmUnit();

  TVPointD v, v1, v2;
  for( int i=0; i < au.AtomCount(); i++ )  {
    TCAtom& A = au.GetAtom(i);
    if( A.GetAtomInfo() == iQPeakIndex || A.IsDeleted() )  continue;

    bool considerI =  (A != ae.GetBase().CAtom());
    // O and N for a while
    if( !( A.GetAtomInfo() == iOxygenIndex ||
           A.GetAtomInfo() == iNitrogenIndex) )  continue;

    v = A.CCenter();
    TMatrixDList& ms = *GetInRange( ae.GetBase().CCenter(), v, 2.9, considerI );
    if( &ms == NULL )  continue;

    for( int j=0; j < ms.Count(); j++ )  {
      v = A.CCenter();
      v = ms[j] * v;
      v[0] += ms[j][0][3];
      v[1] += ms[j][1][3];
      v[2] += ms[j][2][3];
      au.CellToCartesian(v);
      double d = v.DistanceTo( ae.GetBase().Center() );
      if(  d < 2 || d > 2.9 )  continue;

      if( ae.Count() == 1 )  {
        v1 = ae.GetCrd(0);  v1 -= ae.GetBase().Center();
        v2 = v;             v2 -= ae.GetBase().Center();
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
  TVPointD V1, V2, to(ca.GetCCenter());
  TCAtomPList atoms;
  atoms.SetCapacity( GetLattice().GetAsymmUnit().AtomCount() );
  for( int i=0; i < GetLattice().GetAsymmUnit().AtomCount(); i++ )  {
    if( GetLattice().GetAsymmUnit().GetAtom(i).IsDeleted() )  continue;
    atoms.Add( &GetLattice().GetAsymmUnit().GetAtom(i) );
  }

  for( int i=0; i < MatrixCount(); i++ )  {
    const TMatrixD& matr = GetMatrix(i);
    V1 = matr * to;
    V1[0] += matr[0][3];  V1[1] += matr[1][3];  V1[2] += matr[2][3];
    for( int j=0; j < atoms.Count(); j++ )  {
      if( atoms[j]->GetLoaderId() == ca.GetLoaderId() )  
        continue;
      V2 = atoms[j]->GetCCenter();
      V2 -= V1;
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
TCAtom* TUnitCell::FindOverlappingAtom(TVPointD& pos, double delta) const  {
  TVPointD V1, V2, nearest(pos);
  double minD = 1000;
  delta *= delta;
  const TAsymmUnit& au = GetLattice().GetAsymmUnit();

  for( int i=0; i < MatrixCount(); i++ )  {
    const TMatrixD& matr = GetMatrix(i);
    V1 = matr * pos;
    V1[0] += matr[0][3];  V1[1] += matr[1][3];  V1[2] += matr[2][3];
    for( int j=0; j < au.AtomCount(); j++ )  {
      const TCAtom& ca = au.GetAtom(j);
      if( ca.IsDeleted() )  continue;
      V2 = ca.GetCCenter();
      V2 -= V1;
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
            nearest[k] = ca.GetCCenter()[k].GetV();
          }
          else if( V2[k] > 0.95 )  {
            V2[k] = 0;
            nearest[k] = ca.GetCCenter()[k].GetV();
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
TCAtom* TUnitCell::FindCAtom(const TVPointD& center) const  {
  TVPointD Vec;
  const TAsymmUnit& au = GetLattice().GetAsymmUnit();
  for( int i=0; i < au.AtomCount(); i++ )  {
    const TCAtom& A = au.GetAtom(i);
    if( A.IsDeleted() )  continue;
    for( int j=0; j < MatrixCount(); j ++ )  {
      const TMatrixD& M = Matrices[j];
      Vec = A.GetCCenter();
      Vec = M * Vec;
      Vec -= center;
      for( int l=0; l < 3; l++ )  {
        Vec[l] += M[l][3];
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
// helper function to sort atoms
static int AtomsSortByDistance(const AnAssociation2<TVPointD, TCAtom*>& A1, const AnAssociation2<TVPointD, TCAtom*>& A2)  {
  double d = A1.GetA().Length() - A2.GetA().Length();
  if( d < 0 )  return -1;
  if( d > 0 )  return 1;
  return 0;
}
//..............................................................................
void TUnitCell::GenereteAtomCoordinates(TTypeList< AnAssociation2<TVPointD,TCAtom*> >& list, bool IncludeH) const {
  TVPointD center;
  for( int i=0; i < Lattice->GetAsymmUnit().AtomCount(); i++ )  {
    TCAtom& ca = Lattice->GetAsymmUnit().GetAtom(i);
    if( ca.GetAtomInfo() == iQPeakIndex || ca.IsDeleted() )  continue;
    if( !IncludeH && ca.GetAtomInfo() == iHydrogenIndex )    continue;
    for( int j=0; j < Matrices.Count(); j++ )  {
      center = ca.GetCCenter();
      center = Matrices[j] * center;
      center[0] += Matrices[j][0][3];  center[1] += Matrices[j][1][3];  center[2] += Matrices[j][2][3];
      for( int k=0; k < 3; k++ )  {
        while( center[k] < 0 )  center[k] += 1;
        while( center[k] > 1 )  center[k] -= 1;
      }
      list.AddNew(center, &ca);
    }
  }
  // create a list of unique atoms
  float* distances = new float[ list.Count()+1 ];
  list.QuickSorter.SortSF( list, AtomsSortByDistance);
  for( int i=0; i < list.Count(); i++ )
    distances[i] = (float)list[i].GetA().Length();

  for( int i=0; i < list.Count(); i++ )  {
   if( list.IsNull(i) )  continue;
    for( int j=i+1; j < list.Count(); j++ )  {
     if( list.IsNull(j) )  continue;
      if( (distances[j] - distances[i]) > 0.01 )  break;
        double d = list[i].GetA().QDistanceTo( list[j].GetA() );
        if( d < 0.00001 )  {
          list.NullItem(j);
          continue;
        }
    }
  }
  delete [] distances;
  list.Pack();
}
//..............................................................................
void TUnitCell::BuildStructureMap( TArray3D<short>& map, double delta, short val, 
                                  long* structurePoints )  {

  TTypeList< AnAssociation2<TVPointD,TCAtom*> > allAtoms;
  TVPointD center, center1;
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
    const TBasicAtomInfo& bai = au.GetAtom(i).GetAtomInfo();
    int ind = scatterers.IndexOfComparable( bai.GetIndex() );
    if( ind != -1 )  continue;
    const double r = bai.GetRad2() + delta;
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
    scatterers.Add(bai.GetIndex(), spm);
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

