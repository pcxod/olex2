//----------------------------------------------------------------------------//
// namespace TXClasses: crystallographic core
// TAsymmUnit: a collection of symmetry independent atoms
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "asymmunit.h"
#include "catom.h"
#include "ellipsoid.h"
#include "unitcell.h"
#include "estrlist.h"
#include "exception.h"
#include "symmlib.h"
#include "library.h"
#include "symmlib.h"
#include "estlist.h"
#include "lattice.h"

#undef GetObject

class TSfacSorter  {
public:
  static int Compare(const TPrimitiveStrListData<olxstr,TBasicAtomInfo*>* s1, 
                    const TPrimitiveStrListData<olxstr,TBasicAtomInfo*>* s2)  {
    double diff = s1->GetObject()->GetMr() - s1->GetObject()->GetMr();
    if( diff < 0 )  return -1;
    if( diff > 0 )  return 1;
    return 0;
  }
};
//----------------------------------------------------------------------------//
// TAsymmetricUnit function bodies
//----------------------------------------------------------------------------//
TAsymmUnit::TAsymmUnit(TLattice *L, TAtomsInfo *AI) : 
  rDfix(rltBonds), rAfix(rltBonds), rDsim(rltBonds), rVfix(rltAtoms), rPfix(rltGroup),
    rRBnd(rltAtoms), rUsim(rltAtoms), rUiso(rltAtoms), rEADP(rltAtoms), rSAME(rltGroup), MainResidue(*this, -1)
  {
  AtomsInfo = AI;
  Lattice   = L;
  Latt = -1;
  Z = 1;
  ContainsEquivalents = false;
  ExyzGroups = NULL;
}
//..............................................................................
TAsymmUnit::~TAsymmUnit() {
  Clear();
  if( ExyzGroups != NULL)
    delete ExyzGroups;
}
//..............................................................................
void  TAsymmUnit::Clear()  {
  ClearExyz();
  Matrices.Clear();
  for( int i=0; i < CAtoms.Count(); i++ )
    delete CAtoms[i];
  CAtoms.Clear();
  for( int i=0; i < Centroids.Count(); i++ )
    delete Centroids[i];
  Centroids.Clear();
  for( int i=0; i < Ellipsoids.Count(); i++ )
    delete Ellipsoids[i];
  Ellipsoids.Clear();
  for( int i=0; i < SfacData.Count(); i++ )
    if( !SfacData.GetObject(i)->IsBuiltIn() )
      delete SfacData.GetObject(i);
  SfacData.Clear();
  for( int i=0; i < Residues.Count(); i++ )
    delete Residues[i];
  Residues.Clear();
  MainResidue.Clear();
  Latt = -1;
  Z = 1;
  ContainsEquivalents = false;
  ClearRestraints();
}
//..............................................................................
void TAsymmUnit::ClearRestraints()  {
  rDfix.Clear();
  rAfix.Clear();
  rDsim.Clear();
  rVfix.Clear();
  rPfix.Clear();
  rRBnd.Clear();
  rUsim.Clear();
  rUiso.Clear();
  rEADP.Clear();
  rSAME.Clear();
  UsedSymm.Clear();
}
//..............................................................................
void TAsymmUnit::ClearExyz() {
  if( ExyzGroups != NULL ) ExyzGroups->Clear();
}
//..............................................................................
void TAsymmUnit::Assign(const TAsymmUnit& C)  {
  Clear();
  FAxes   = C.FAxes;
  FAngles = C.FAngles;
  RAxes   = C.GetRAxes();
  RAngles = C.GetRAngles();
  Z = C.GetZ();
  Latt = C.GetLatt();

  for( int i = 0; i < C.MatrixCount(); i++ )
    Matrices.AddNew( C.GetMatrix(i) );

  for( int i = 0; i < C.EllpCount(); i++ )
    this->NewEllp() = C.GetEllp(i);

  for( int i=0; i < C.Residues.Count(); i++ )  {
    TResidue* resi = C.Residues[i];
    NewResidue( resi->GetClassName(), resi->GetNumber(), resi->GetAlias() ); 
  }
  for( int i = 0; i < C.AtomCount(); i++ )  {
    TCAtom& CA = this->NewAtom();
    CA.Assign( C.GetAtom(i) );
    CA.SetId(i);
    if( C.GetAtom(i).GetResiId() != -1 )  // main residue
      GetResidue(C.GetAtom(i).GetResiId()).AddAtom(&CA);
  }
  // copy matrices
  Cartesian2Cell = C.GetCartesianToCell();
  Cell2Cartesian = C.GetCellToCartesian();
  Hkl2Cartesian =  C.GetHklToCartesian();
  UcifToUxyz     = C.UcifToUxyz;
  UxyzToUcif     = C.UxyzToUcif;
  UcifToUxyzT    = C.UcifToUxyzT;
  UxyzToUcifT    = C.UxyzToUcifT;

  SetContainsEquivalents( C.DoesContainEquivalents() );
  MaxQPeak = C.GetMaxQPeak();
  MinQPeak = C.GetMinQPeak();
  if( C.ExyzGroupCount() != 0 )  {
    if( ExyzGroups == NULL )  ExyzGroups = new TTypeList<TCAtomPList>;
    for(int i=0; i < C.ExyzGroupCount(); i++ )  {
      TCAtomPList& Xyz = C.ExyzGroup(i);
      TCAtomPList& thisXyz = ExyzGroups->AddNew();
      for( int j=0; j < Xyz.Count(); j++ )
        thisXyz.Add(&GetAtom(Xyz[j]->GetId()));
    }
  }

  for( int i=0; i < C.SfacCount(); i++ )  {
    if( C.GetSfacData(i).IsBuiltIn() )
      SfacData.Add(C.GetSfacLabel(i), &C.GetSfacData(i) );
    else
      SfacData.Add(C.GetSfacLabel(i), new TLibScatterer(C.GetSfacData(i)) );
  }
  rDfix.Assign(*this, C.rDfix);
  rAfix.Assign(*this, C.rAfix);
  rDsim.Assign(*this, C.rDsim);
  rVfix.Assign(*this, C.rVfix);
  rPfix.Assign(*this, C.rPfix);
  rRBnd.Assign(*this, C.rRBnd);
  rUsim.Assign(*this, C.rUsim);
  rUiso.Assign(*this, C.rUiso);
  rEADP.Assign(*this, C.rEADP);
  rSAME.Assign(*this, C.rSAME);

  for( int i=0; i < C.UsedSymm.Count(); i++ )
    UsedSymm.AddCCopy( C.UsedSymm[i] );
}
//..............................................................................
void  TAsymmUnit::InitMatrices()  {
  if( !FAxes[0].GetV() || !FAxes[1].GetV() || !FAxes[2].GetV() )
    throw TFunctionFailedException(__OlxSourceInfo, "zero cell parameters");
  // just to check the validity of my deductions put this in seems to be the same ...
  double cG = cos(FAngles[2].GetV()/180*M_PI),
         cB = cos(FAngles[1].GetV()/180*M_PI),
         cA = cos(FAngles[0].GetV()/180*M_PI),
         sG = sin(FAngles[2].GetV()/180*M_PI),
         sB = sin(FAngles[1].GetV()/180*M_PI),
         sA = sin(FAngles[0].GetV()/180*M_PI);

  double V = FAxes[0].GetV() * FAxes[1].GetV() * FAxes[2].GetV()*sqrt( (1-cA*cA-cB*cB-cG*cG) + 2*(cA*cB*cG));

  double cGs = (cA*cB-cG)/(sA*sB),
         cBs = (cA*cG-cB)/(sA*sG),
         cAs = (cB*cG-cA)/(sB*sG),
         as = FAxes[1].GetV()*FAxes[2].GetV()*sA/V,
         bs = FAxes[0].GetV()*FAxes[2].GetV()*sB/V,
         cs = FAxes[0].GetV()*FAxes[1].GetV()*sG/V
         ;
  // cartesian to cell transformation matrix
  Cartesian2Cell.Null();
  Cartesian2Cell[0][0] =  1./FAxes[0].GetV();
  Cartesian2Cell[1][0] = -cG/(sG*FAxes[0].GetV());
  Cartesian2Cell[2][0] = as*cBs;

  Cartesian2Cell[1][1] = 1./(sG*FAxes[1].GetV());
  Cartesian2Cell[2][1] = bs*cAs;

  Cartesian2Cell[2][2] = cs;

  // cell to cartesian transformation matrix
  Cell2Cartesian.Null();
  Cell2Cartesian[0][0] = FAxes[0].GetV();
  Cell2Cartesian[1][0] = FAxes[1].GetV()*cG;
  Cell2Cartesian[2][0] = FAxes[2].GetV()*cB;

  Cell2Cartesian[1][1] = FAxes[1].GetV()*sG;
  Cell2Cartesian[2][1] = -FAxes[2].GetV()*(cB*cG-cA)/sG;

  Cell2Cartesian[2][2] = 1./cs;

  // init hkl to cartesian transformation matrix
//  TMatrixD m( *Cartesian2Cell );
  mat3d m( Cell2Cartesian );
  vec3d v1(m[0]), v2(m[1]), v3(m[2]);

  Hkl2Cartesian[0] = v2.XProdVec(v3)/V;
  Hkl2Cartesian[1] = v3.XProdVec(v1)/V;
  Hkl2Cartesian[2] = v1.XProdVec(v2)/V;

// init Uaniso traformation matices
  m.Null();
  m[0][0] = Hkl2Cartesian[0].Length();
  m[1][1] = Hkl2Cartesian[1].Length();
  m[2][2] = Hkl2Cartesian[2].Length();

  UcifToUxyz = Cell2Cartesian * m;
  UcifToUxyzT = UcifToUxyz;
  UcifToUxyz.Transpose();

  m[0][0] = 1./Hkl2Cartesian[0].Length();
  m[1][1] = 1./Hkl2Cartesian[1].Length();
  m[2][2] = 1./Hkl2Cartesian[2].Length();

  UxyzToUcif = m*Cartesian2Cell;
  UxyzToUcifT = UxyzToUcif;
  UxyzToUcif.Transpose();
}
//..............................................................................
void TAsymmUnit::InitData()  {
  // init QPeak intensities
  MaxQPeak = -1000;
  MinQPeak = 1000;
  double qpeak;
  for( int i =0; i < AtomCount(); i++ )  {
    if( CAtoms[i]->GetAtomInfo() == iQPeakIndex )  {
      qpeak = CAtoms[i]->GetQPeak();
      if( qpeak < MinQPeak )  MinQPeak = qpeak;
      if( qpeak > MaxQPeak )  MaxQPeak = qpeak;
    }
  }
}
//..............................................................................
TAsymmUnit::TResidue& TAsymmUnit::NewResidue(const olxstr& RClass, int number, const olxstr& alias)  {
  for( int i=0; i < Residues.Count(); i++ )
    if( Residues[i]->GetNumber() == number )  {
      return *Residues[i];
      //throw TInvalidArgumentException(__OlxSourceInfo, "dublicated residue number");
    }
  TResidue* resi = new TResidue(*this, Residues.Count(), RClass, number, alias);
  Residues.Add( resi );
  return *resi;
}
//..............................................................................
void TAsymmUnit::FindResidues(const olxstr& resi, TPtrList<TAsymmUnit::TResidue>& list) {
  if( resi.IsEmpty() )  {
    list.Add(&MainResidue);
    return;
  }
  if( resi.IsNumber() )  {
    int number = resi.ToInt();
    for( int i=0; i < Residues.Count(); i++ )  {
      if( Residues[i]->GetNumber() == number )  {
        list.Add(Residues[i]);
        break;  // number must be unique
      }
    }
  }
  else  {
    if( resi.Length() == 1 && resi.CharAt(0) == '*' )  {  //special case
      list.Assign(Residues);
      list.Add( &MainResidue );
    }
    for( int i=0; i < Residues.Count(); i++ )
      if( Residues[i]->GetClassName().Comparei(resi) == 0 || Residues[i]->GetAlias().Comparei(resi) == 0 ) 
        list.Add(Residues[i]);
  }
}
//..............................................................................
void TAsymmUnit::ClearResidues(bool moveToMain)  {
  for( int i=0;  i < Residues.Count(); i++ )
    delete Residues[i];
  Residues.Clear();
  MainResidue.Clear();
  if( moveToMain)  {
    MainResidue.SetCapacity(CAtoms.Count());
    for( int i=0; i < CAtoms.Count(); i++ )  {
      CAtoms[i]->SetResiId(-1);
      MainResidue.AddAtom(CAtoms[i]);
    }
  }
}
//..............................................................................
void TAsymmUnit::AssignResidues(const TAsymmUnit& au)  {
  ClearResidues(false);
  MainResidue = au.MainResidue;
  for( int i=0; i < au.Residues.Count(); i++ )  {
    TResidue* resi = au.Residues[i];
    NewResidue( resi->GetClassName(), resi->GetNumber() ) = *resi; 
  }
}
//..............................................................................
TCAtom& TAsymmUnit::NewAtom(TResidue* resi)  {
  TCAtom *A = new TCAtom(this);
  A->SetId( CAtoms.Count() );
  CAtoms.Add(A);
  if( resi == NULL )  resi = &MainResidue;
  resi->AddAtom(A);
  return *A;
}
//..............................................................................
TCAtom& TAsymmUnit::NewCentroid(const vec3d& CCenter)  {
  for( int i=0; i < CentroidCount(); i++ )  {
    if( (Centroids[i]->ccrd()-CCenter).QLength() < 0.001 ) // already exists
      return *Centroids[i];
  }
  TCAtom *A = new TCAtom(this);
  olxstr Label("Cnt");
  Label << CentroidCount();
  A->ccrd() = CCenter;
  A->SetId( Centroids.Count() );
  A->SetLoaderId( liCentroid );
  Centroids.Add(A);
  A->SetLabel( Label );
  return *A;
}
//..............................................................................
TCAtom * TAsymmUnit::FindCAtom(const olxstr &Label, TResidue* resi)  const {
  if( resi != NULL )  {
    for( int i=0; i < resi->Count(); i++ )
      if( resi->GetAtom(i).GetLabel().Comparei(Label) == 0  )
        return &resi->GetAtom(i);
  }
  else  {  // global search
    for( int i=0; i < CAtoms.Count(); i++ )
      if( CAtoms[i]->GetLabel().Comparei(Label) == 0  )
        return CAtoms[i];
  }
  return NULL;
}
//..............................................................................
TCAtom* TAsymmUnit::FindCAtomByLoaderId(int li) const  {
  for( int i=0; i < AtomCount(); i++ )
    if( CAtoms[i]->GetLoaderId() == li )
      return CAtoms[i];
  return NULL;
}
//..............................................................................
void TAsymmUnit::InitAtomIds()  {  // initialises atom ids if any were added or removed
  for( int i=0; i < AtomCount(); i++ )    GetAtom(i).SetId(i);
  for( int i=0; i < EllpCount(); i++ )    GetEllp(i).SetId(i);
}
//..............................................................................
void TAsymmUnit::DelAtom( size_t index )  {
  delete CAtoms[index];
  CAtoms.Delete(index);
}
//..............................................................................
void TAsymmUnit::NullAtom( size_t index )  {
  delete CAtoms[index];
  CAtoms[index] = NULL;
}
//..............................................................................
void TAsymmUnit::PackAtoms()  {
  CAtoms.Pack();
  InitAtomIds();
}
//..............................................................................
TEllipsoid& TAsymmUnit::NewEllp(const evecd& Q)  {
  TEllipsoid *E = new TEllipsoid(Q);
  Ellipsoids.Add(E);
  E->SetId( Ellipsoids.Count()-1 );
  return *E;
}
//..............................................................................
TEllipsoid& TAsymmUnit::NewEllp() {
  TEllipsoid *E = new TEllipsoid();
  Ellipsoids.Add(E);
  E->SetId( Ellipsoids.Count()-1 );
  return *E;
}
//..............................................................................
void TAsymmUnit::PackEllps() {
  int removed = 0;
  for( int i=0; i < Ellipsoids.Count(); i++ )  {
    if( Ellipsoids[i] == NULL )  {
      for( int j=0; j < CAtoms.Count(); j++ )  {
        if( CAtoms[j]->GetEllpId() > (i-removed) )
          CAtoms[j]->SetEllpId( CAtoms[j]->GetEllpId() - 1 );
      }
      removed++;
    }
    else
      Ellipsoids[i]->SetId(i-removed);
  }
  Ellipsoids.Pack();
}
//..............................................................................
void TAsymmUnit::NullEllp(size_t i)  {
  if( Ellipsoids[i] != NULL )  {
    delete Ellipsoids[i];
    Ellipsoids[i] = NULL;
  }
}
//..............................................................................
vec3d TAsymmUnit::GetOCenter(bool IncludeQ, bool IncludeH) const {
  vec3d P;
  int ac = 0;
  for( int i=0; i < AtomCount(); i++ )  {
    if( CAtoms[i]->IsDeleted() )  continue;
    if( !IncludeQ && CAtoms[i]->GetAtomInfo() == iQPeakIndex )  continue;
    if( !IncludeH && CAtoms[i]->GetAtomInfo() == iHydrogenIndex )  continue;
    P += CAtoms[i]->ccrd();
    ac++;
  }

  if( ac > 0 )
    P /= ac;
  return P;
}
//..............................................................................
void TAsymmUnit::SummFormula(TStrPObjList<olxstr,TBasicAtomInfo*>& BasicAtoms, olxstr &Elements,
                             olxstr &Numbers, bool MultiplyZ) const {
  BasicAtoms.Clear();
  TBasicAtomInfo *AI, *Carbon=NULL, *Hydrogen=NULL;

  for( int i=0; i < AtomCount(); i++ )  {
    if( CAtoms[i]->IsDeleted() )  continue;
    TCAtom& A = *CAtoms[i];
    bool Uniq = true;
    for( int j=0; j < BasicAtoms.Count(); j++)  {
      if( BasicAtoms.Object(j)->GetIndex() == A.GetAtomInfo().GetIndex() ) {  // already in the list ?
        A.GetAtomInfo().SetSumm( A.GetAtomInfo().GetSumm() + A.GetOccp() );       // update the quantity
        Uniq = false;
        break;
      }
    }
    if( Uniq )  {
      A.GetAtomInfo().SetSumm( A.GetOccp() );
      if( A.GetAtomInfo().GetIndex() == iCarbonIndex )   Carbon = &A.GetAtomInfo();
      if( A.GetAtomInfo().GetIndex() == iHydrogenIndex )  Hydrogen = &A.GetAtomInfo();
      BasicAtoms.Add(A.GetAtomInfo().GetSymbol(), &A.GetAtomInfo());
    }
  }
  BasicAtoms.QuickSort<TSfacSorter>();
  if( Carbon != NULL )
    BasicAtoms.Swap(0, BasicAtoms.IndexOfObject(Carbon));
  if( Hydrogen != NULL && BasicAtoms.Count() > 1 )
    BasicAtoms.Swap(1, BasicAtoms.IndexOfObject(Hydrogen));
  for( int i=0; i < BasicAtoms.Count(); i++)  {
    AI = BasicAtoms.Object(i);
    Elements << AI->GetSymbol();
    if( MultiplyZ )
      Numbers << olxstr::FormatFloat(3, AI->GetSumm()*GetZ());
    else
      Numbers << olxstr::FormatFloat(3, AI->GetSumm());
    if( i < (BasicAtoms.Count()-1) )  {
      Elements << ' ';
      Numbers  << ' ';
    }
  }
}
//..............................................................................
olxstr TAsymmUnit::SummFormula(const olxstr &Sep, bool MultiplyZ) const  {
  TCAtomPList UniqAtoms;
  olxstr T;

  int matrixInc = 0;
  // searching for the identity matrix
  bool Uniq = true;
  for( int i=0; i < MatrixCount(); i++ )
    if( GetMatrix(i).r.IsI() )  {
      Uniq = false;  break;
    }
  if( Uniq )  matrixInc ++;

  for( int i=0; i < AtomCount(); i++ )  {
    TCAtom& A = *CAtoms[i];
    if( A.IsDeleted() )  continue;
    Uniq = true;
    for( int j=0; j < UniqAtoms.Count(); j++)  {
      if( UniqAtoms[j]->GetAtomInfo().GetIndex() == A.GetAtomInfo().GetIndex() )  { // already in the list ?
        A.GetAtomInfo().SetSumm( A.GetAtomInfo().GetSumm() + A.GetOccp() );       // update the quantity
        Uniq = false;
        break;
      }
    }
    if( Uniq )  {
      A.GetAtomInfo().SetSumm( A.GetOccp() );
      UniqAtoms.Add(&A);
    }
  }
  for( int i=0; i < UniqAtoms.Count(); i++)  {
    TCAtom& A = *UniqAtoms[i];
    if( A.GetAtomInfo().GetIndex() == iQPeakIndex )  continue;
    T << A.GetAtomInfo().GetSymbol();
    if( MultiplyZ )
      T << olxstr::FormatFloat(3, A.GetAtomInfo().GetSumm()*(MatrixCount()+matrixInc) );
    else
      T << olxstr::FormatFloat(3, A.GetAtomInfo().GetSumm());
    if( i < (UniqAtoms.Count()-1) )
      T << Sep;
  }
  return T;
}
//..............................................................................
double TAsymmUnit::MolWeight() const  {
  double Mw = 0;
  for( int i=0; i < AtomCount(); i++ )
    Mw += CAtoms[i]->GetAtomInfo().GetMr();
  return Mw;
}
//..............................................................................
void TAsymmUnit::AddMatrix(const smatd& a)  {
  if( a.r.IsI() )  Matrices.InsertCCopy(0, a);
  else             Matrices.AddCCopy(a);
}
//..............................................................................
olxstr TAsymmUnit::CheckLabel(const TCAtom* ca, const olxstr &Label, char a, char b, char c) const  {
  olxstr LB( (Label.Length() > 4) ? Label.SubStringTo(2) : Label );
  if( ca != NULL )  {
    const TResidue& resi = GetResidue(ca->GetResiId() );
    for( int i=0; i < resi.Count(); i++ )  {
      TCAtom& atom = resi[i];
      if( atom.GetPart() != ca->GetPart() )  continue;
      if( !atom.IsDeleted() && (atom.Label().Comparei(Label) == 0) && 
        (atom.GetLoaderId() != ca->GetLoaderId()) )  {
        LB = atom.GetAtomInfo().GetSymbol();
        if( LB.Length() == 2 )  LB[0] = LB.o_toupper(LB[0]);
        LB << a << b;
        if( LB.Length() < 4 )  LB << c;
        if( a < '9' )  return CheckLabel(ca, LB, (char)(a+1), b, c);
        if( b < 'z' )  return CheckLabel(ca, LB, '0', (char)(b+1), c);
        if( c < 'z' )  return CheckLabel(ca, LB, '0', 'a', (char)(c+1));
        throw TFunctionFailedException(__OlxSourceInfo, "cannot create label");
      }
    }
    return LB;
  }
  for( int i=0; i < AtomCount(); i++ )  {
    TCAtom& CA = GetAtom(i);
    if( !CA.IsDeleted() && (CA.Label().Comparei(Label) == 0) )  {
      LB = CA.GetAtomInfo().GetSymbol();
      if( LB.Length() == 2 )  LB[0] = LB.o_toupper(LB[0]);
      LB << a << b;
      if( LB.Length() < 4 )  LB << c;
      if( a < '9' )  return CheckLabel(ca, LB, (char)(a+1), b, c);
      if( b < 'z' )  return CheckLabel(ca, LB, '0', (char)(b+1), c);
      if( c < 'z' )  return CheckLabel(ca, LB, '0', 'a', (char)(c+1));
      throw TFunctionFailedException(__OlxSourceInfo, "cannot create label");
    }
  }
  return LB;
}
//..............................................................................
size_t TAsymmUnit::CountElements(const olxstr &Symbol) const  {
  TBasicAtomInfo *BAI = GetAtomsInfo()->FindAtomInfoBySymbol(Symbol);
  if( BAI == NULL )
    throw TInvalidArgumentException(__OlxSourceInfo, olxstr("unknown atom: '") << Symbol << '\'');
  int cnt = 0;
  for( int i=0; i < AtomCount(); i++ )
    if( &(GetAtom(i).GetAtomInfo()) == BAI )
      cnt++;
  return cnt;
}
//..............................................................................
void TAsymmUnit::KeepH(TCAtomPList* list)  {
  TPtrList<TCAtomPList> lists;
  if( list == NULL )  {
    for( int i=-1; i < Residues.Count(); i++ )
      lists.Add( &GetResidue(i).AtomList() );
  }
  else
    lists.Add(list);

  typedef AnAssociation2<TCAtom*, TCAtomPList*> TANode;

  for( int i=0; i < lists.Count(); i++ )  {
    TTypeList <TANode> atomTree;
    for( int j=0; j < lists[i]->Count(); j++ ) {
      if( lists[i]->Item(j)->GetAfixAtomId() == -1 )
       atomTree.AddNew( lists[i]->Item(j), new TCAtomPList() );
    }
    for( int j=0; j < lists[i]->Count(); j++ ) {
      if( lists[i]->Item(j)->GetAfixAtomId() != -1 )  {
        for( int k=0; k < atomTree.Count(); k++ )  {
          if( atomTree[k].A()->GetLoaderId() == lists[i]->Item(j)->GetAfixAtomId() )  {
            atomTree[k].B()->Add( lists[i]->Item(j) );
            break;
          }
        }
      }
    }
    lists[i]->Clear();
    for( int j=0; j < atomTree.Count(); j++ )  {
      lists[i]->Add( atomTree[j].A() );
      for( int k=0; k < atomTree[j].GetB()->Count(); k++ )
        lists[i]->Add( atomTree[j].GetB()->Item(k) );
      delete atomTree[j].GetB();
    }
  }

 //fix afixes
/*
 int afixId;
 for( int i=0; i < AtomCount(); i++ ) {
   if( GetAtom(i)->GetAfixAtomId() != -1 )  {
     afixId = GetAtom(i)->GetAfixAtomId();
     for(int j=0; j < AtomCount(); j++ )  {
       if( GetAtom(j)->GetLoaderId() == afixId )  {
         while( ++j < AtomCount() )
           if( GetAtom(j)->GetAfixAtomId() == -1 )  break;
         if( (j+1) < AtomCount() )  {
           CAtoms->Move(i, j);
         }
         break;
       }
     }
   }
 }
*/
}
//..............................................................................
void TAsymmUnit::Sort(TCAtomPList* list)  {
 // sorting by four params
  if( list == NULL )  list = &CAtoms;
 TCAtomPList::QuickSorter.Sort<TCAtomPComparator>(*list);
 TCAtomPList::QuickSorter.Sort<TCAtomPComparator>(*list);
 TCAtomPList::QuickSorter.Sort<TCAtomPComparator>(*list);
 TCAtomPList::QuickSorter.Sort<TCAtomPComparator>(*list);
 KeepH(list);
}
//..............................................................................
int TAsymmUnit::GetMaxPart()  const {
  int part = 0;
  for( int i=0; i < AtomCount(); i++ )
    if( GetAtom(i).GetPart() > part )
      part = GetAtom(i).GetPart();

  return part+1;
}
//..............................................................................
int TAsymmUnit::GetMaxLoaderId() const  {
  int id = 0;
  for( int i=0; i < AtomCount(); i++ )
    if( GetAtom(i).GetLoaderId() > id )
      id = GetAtom(i).GetLoaderId();

  return id+1;
}
//..............................................................................
void TAsymmUnit::AddExyz(const TCAtomPList& cAtoms)  {
  if( ExyzGroups == NULL )
    ExyzGroups = new TTypeList<TCAtomPList>;
  TCAtomPList& Xyz = ExyzGroups->AddNew();
  for( int i=0; i < cAtoms.Count(); i++ )  {
    Xyz.Add(cAtoms[i]);
    cAtoms[i]->SetSharedSiteId(ExyzGroups->Count()-1);
  }
}
//..............................................................................
void TAsymmUnit::AddNewExyz(const TStrList& cAtoms)  {
  throw TNotImplementedException(__OlxSourceInfo);
}
//..............................................................................
void TAsymmUnit::ChangeSpaceGroup(const TSpaceGroup& sg)  {
  Latt = sg.GetLattice().GetLatt();
  if( !sg.IsCentrosymmetric() && Latt > 0 )  Latt = -Latt;

  Matrices.Clear();
  for( int i=0; i < sg.MatrixCount(); i++ )
    Matrices.AddCCopy( sg.GetMatrix(i) );
}
//..............................................................................
void TAsymmUnit::AddNewSfac(const olxstr& label,
                        double a1, double a2, double a3, double a4,
                        double b1, double b2, double b3, double b4,
                        double c)  {
  if( SfacData.IndexOfComparable(label) != -1 )
    throw TInvalidArgumentException(__OlxSourceInfo, "dublicate scatterer");
  SfacData.Add( label, new TLibScatterer(a1,a2,a3,a4,b1,b2,b3,b4,c) );
}
//..............................................................................
const smatd& TAsymmUnit::AddUsedSymm(const smatd& matr)  {
  int ind = UsedSymm.IndexOf(matr);
  smatd* rv = NULL;
  if( ind == -1 )  {
    rv = &UsedSymm.Add( *(new smatd(matr)) );
    rv->SetTag(1);
  }
  else  {
    UsedSymm[ind].IncTag();
    rv = &UsedSymm[ind];
  }
  return *rv;
}
//..............................................................................
void TAsymmUnit::RemUsedSymm(const smatd& matr)  {
  int ind = UsedSymm.IndexOf(matr);
  if( ind == -1 )
    throw TInvalidArgumentException(__OlxSourceInfo, "matrix is not in the list");
  UsedSymm[ind].DecTag();
  if( UsedSymm[ind].GetTag() == 0 )
    UsedSymm.Delete(ind);
}
//..............................................................................
double TAsymmUnit::FindRestrainedDistance(const TCAtom& a1, const TCAtom& a2)  {
  for(int i=0; i < rDfix.Count(); i++ )  {
    for( int j=0; j < rDfix[i].AtomCount(); j+=2 )  {
      if( (rDfix[i].GetAtom(j).GetAtom() == &a1 && rDfix[i].GetAtom(j+1).GetAtom() == &a2) ||
          (rDfix[i].GetAtom(j).GetAtom() == &a2 && rDfix[i].GetAtom(j+1).GetAtom() == &a1) )  {
        return rDfix[i].GetValue();
      }
    }
  }
  return -1;
}
//..............................................................................
void TAsymmUnit::OnCAtomCrdChange( TCAtom* ca, const smatd& matr )  {
  throw TNotImplementedException(__OlxSourceInfo);
}
//..............................................................................
void TAsymmUnit::UcifToUcart(evecd& v)  { // silly expansion of Q-form ...
  mat3d M;
  M[0][0] = v[0];  M[1][1] = v[1];  M[2][2] = v[2];
  M[1][2] = M[2][1] = v[3];
  M[0][2] = M[2][0] = v[4];
  M[0][1] = M[1][0] = v[5];

  M = UcifToUxyz*M*UcifToUxyzT;

  v[0] = M[0][0];  v[1] = M[1][1];  v[2] = M[2][2];
  v[3] = M[1][2];  v[4] = M[0][2];  v[5] = M[0][1];
}
//..............................................................................
void TAsymmUnit::UcartToUcif(evecd& v)  {
  mat3d M;
  M[0][0] = v[0];  M[1][1] = v[1];  M[2][2] = v[2];
  M[1][2] = M[2][1] = v[3];
  M[0][2] = M[2][0] = v[4];
  M[0][1] = M[1][0] = v[5];

  M = UxyzToUcif*M*UxyzToUcifT;

  v[0] = M[0][0];  v[1] = M[1][1];  v[2] = M[2][2];
  v[3] = M[1][2];  v[4] = M[0][2];  v[5] = M[0][1];
}
//..............................................................................
double TAsymmUnit::CalcCellVolume()  const  {
  double cosa = cos( FAngles[0].GetV()*M_PI/180 ),
         cosb = cos( FAngles[1].GetV()*M_PI/180 ),
         cosg = cos( FAngles[2].GetV()*M_PI/180 );
  return  FAxes[0].GetV()*
          FAxes[1].GetV()*
          FAxes[2].GetV()*sqrt( (1-cosa*cosa-cosb*cosb-cosg*cosg) + 2*(cosa*cosb*cosg));
}
double TAsymmUnit::EstimateZ(int atomCount) const  {
  double auv = CalcCellVolume()/(TUnitCell::GetMatrixMultiplier(GetLatt())*(MatrixCount()+1));
  int zp = Round(auv/(18.6*atomCount));
  return olx_max((TUnitCell::GetMatrixMultiplier(GetLatt())*(MatrixCount()+1) * zp), 1);
}
//..............................................................................
//..............................................................................
//..............................................................................
//..............................................................................
//..............................................................................



void TAsymmUnit::LibGetAtomCount(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal( AtomCount() );
}
//..............................................................................
void TAsymmUnit::LibGetAtomCrd(const TStrObjList& Params, TMacroError& E)  {
  int index = Params[0].ToInt();
  if( index < 0 || index >= AtomCount() )  throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  E.SetRetVal( GetAtom(index).ccrd().ToString() );
}
//..............................................................................
void TAsymmUnit::LibGetAtomName(const TStrObjList& Params, TMacroError& E)  {
  int index = Params[0].ToInt();
  if( index < 0 || index >= AtomCount() )  throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  E.SetRetVal( GetAtom(index).Label() );
}
//..............................................................................
void TAsymmUnit::LibGetAtomType(const TStrObjList& Params, TMacroError& E)  {
  int index = Params[0].ToInt();
  E.SetRetVal( GetAtom(index).GetAtomInfo().GetSymbol() );
}
//..............................................................................
void TAsymmUnit::LibGetPeak(const TStrObjList& Params, TMacroError& E)  {
  if( Params[0].IsNumber() )  {
    int index = Params[0].ToInt();
    if( index < 0 || index >= AtomCount() )  throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
    if( index < 0 || index >= AtomCount() )  throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
    E.SetRetVal( GetAtom(index).GetQPeak() );
  }
  else  {
    TCAtom* ca = FindCAtom( Params[0] );
    if( ca != NULL && ca->GetAtomInfo().GetIndex() == iQPeakIndex )
      E.SetRetVal( ca->GetQPeak() );
    else
      throw TInvalidArgumentException(__OlxSourceInfo, olxstr("unknown peak \'") << Params[0] << '\'');
  }
}
//..............................................................................
void TAsymmUnit::LibGetAtomU(const TStrObjList& Params, TMacroError& E)  {
  int index = Params[0].ToInt();
  if( index < 0 || index >= AtomCount() )  throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  evecd V(1);
  if( GetAtom(index).GetEllipsoid() == NULL )  {
    // TODO: a special condition - the atom is isotropic, but a user wishes it to be
    // anisotropic - six values a, a, a, 0, 0, 0 have to be passed
    //if( GetAtom(index)->
    V[0] = GetAtom(index).GetUiso();
  }
  else  // the function resises the vector automatically
    GetAtom(index).GetEllipsoid()->GetQuad(V);

  E.SetRetVal( V.ToString() );
}
//..............................................................................
void TAsymmUnit::LibGetAtomUiso(const TStrObjList& Params, TMacroError& E)  {
  int index = Params[0].ToInt();
  if( index < 0 || index >= AtomCount() )  throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  E.SetRetVal( GetAtom(index).GetUiso() );
}
//..............................................................................
void TAsymmUnit::LibGetCell(const TStrObjList& Params, TMacroError& E)  {
  evecd V(6);
  V[0] = FAxes[0].GetV();    V[1] = FAxes[1].GetV();    V[2] = FAxes[2].GetV();
  V[3] = FAngles[0].GetV();  V[4] = FAngles[1].GetV();  V[5] = FAngles[2].GetV();
  E.SetRetVal( V.ToString() );
}
//..............................................................................
void TAsymmUnit::LibGetVolume(const TStrObjList& Params, TMacroError& E)  {
  double v = CalcCellVolume()/Lattice->GetUnitCell().MatrixCount();
  E.SetRetVal( v );
}
//..............................................................................
void TAsymmUnit::LibGetCellVolume(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal( CalcCellVolume() );
}
//..............................................................................
void TAsymmUnit::LibGetSymm(const TStrObjList& Params, TMacroError& E)  {
  if( TSymmLib::GetInstance() == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "Symmetry librray is not initialised" );
    return;
  }
  TSpaceGroup* sg = TSymmLib::GetInstance()->FindSG( *this );
  if( sg == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "Could not locate spacegroup" );
    return;
  }
  E.SetRetVal( sg->GetName() );
}
//..............................................................................
void TAsymmUnit::LibSetAtomCrd(const TStrObjList& Params, TMacroError& E)  {
  int index = Params[0].ToInt();
  if( index < 0 || index >= AtomCount() )  throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  TCAtom& ca = GetAtom(index);
  ca.ccrd()[0] = Params[1].ToDouble();
  ca.ccrd()[1] = Params[2].ToDouble();
  ca.ccrd()[2] = Params[3].ToDouble();
  if( ca.GetAtomInfo() == iQPeakIndex && Lattice != NULL )  {
    if( Lattice->GetUnitCell().DoesOverlap( ca, 0.3 ) )  {
      DelAtom( index );
      E.SetRetVal(false);
      return;
    }
  }
  E.SetRetVal(true);
}
//..............................................................................
void TAsymmUnit::LibSetAtomLabel(const TStrObjList& Params, TMacroError& E)  {
  int index = Params[0].ToInt();
  if( index < 0 || index >= AtomCount() )  throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  olxstr newLabel;
  if( Params.String(1).IsNumber() )  {
    int inc = Params[1].ToInt();
    int v = GetAtom(index).GetAtomInfo().GetIndex() + inc;
    if( v >= 0 && v <= iQPeakIndex )  {
      newLabel << GetAtomsInfo()->GetAtomInfo(v).GetSymbol()
               << GetAtom(index).Label().SubStringFrom(
                    GetAtom(index).GetAtomInfo().GetSymbol().Length() );
    }
  }
  else  {
    newLabel = Params[1];
  }
  newLabel = CheckLabel(&GetAtom(index), newLabel );
  if( !newLabel.Length() || !GetAtom(index).SetLabel(newLabel) )  {
    E.ProcessingError(__OlxSrcInfo, "incorrect label ") << Params.String(1);
    return;
  }
}
//..............................................................................
void TAsymmUnit::LibGetAtomLabel(const TStrObjList& Params, TMacroError& E)  {
  int index = Params[0].ToInt();
  if( index < 0 || index >= AtomCount() )  throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  olxstr newLabel;
  if( Params.String(1).IsNumber() )  {
    int inc = Params[1].ToInt();
    int v = GetAtom(index).GetAtomInfo().GetIndex() + inc;
    if( v >= 0 && v <= iQPeakIndex )  {
      E.SetRetVal( GetAtomsInfo()->GetAtomInfo(v).GetSymbol() );
      return;
    }
  }
  else  {
    E.ProcessingError(__OlxSrcInfo, "a number is expected" );
    E.SetRetVal( E.GetInfo() );
    return;
  }
}
//..............................................................................
void TAsymmUnit::LibIsAtomDeleted(const TStrObjList& Params, TMacroError& E)  {
  int index = Params[0].ToInt();
  if( index < 0 || index >= AtomCount() )  throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  E.SetRetVal( GetAtom(index).IsDeleted() );
}
//..............................................................................
void TAsymmUnit::LibGetAtomOccu(const TStrObjList& Params, TMacroError& E)  {
  int index = Params[0].ToInt();
  if( index < 0 || index >= AtomCount() )  throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  E.SetRetVal( GetAtom(index).GetOccp() );
}
//..............................................................................
void TAsymmUnit::LibGetAtomAfix(const TStrObjList& Params, TMacroError& E)  {
  int index = Params[0].ToInt();
  if( index < 0 || index >= AtomCount() )  throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  E.SetRetVal( GetAtom(index).GetAfix() );
}
//..............................................................................
void TAsymmUnit::LibIsPeak(const TStrObjList& Params, TMacroError& E)  {
  if( Params[0].IsNumber() )  {
    int index = Params[0].ToInt();
    if( index < 0 || index >= AtomCount() )  throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
    E.SetRetVal( GetAtom(index).GetAtomInfo().GetIndex() == iQPeakIndex );
  }
  else  {
    TCAtom* ca = FindCAtom( Params[0] );
    if( ca != NULL )
      E.SetRetVal( ca->GetAtomInfo().GetIndex() == iQPeakIndex );
    else
      E.SetRetVal( false );
  }
}
//..............................................................................
void TAsymmUnit::LibSetAtomU(const TStrObjList& Params, TMacroError& E)  {
  int index = Params[0].ToInt();
  if( index < 0 || index >= AtomCount() )  throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  if( (GetAtom(index).GetEllipsoid() != NULL) && (Params.Count() == 7) )  {
    evecd V(6);
    V[0] = Params[1].ToDouble();
    V[1] = Params[2].ToDouble();
    V[2] = Params[3].ToDouble();
    V[3] = Params[4].ToDouble();
    V[4] = Params[5].ToDouble();
    V[5] = Params[6].ToDouble();
    GetAtom(index).GetEllipsoid()->Initialise( V );
  }
  else if( (GetAtom(index).GetEllipsoid() == NULL) && (Params.Count() == 2) ) {
    GetAtom(index).SetUiso( Params[1].ToDouble() );
  }
  else {
    olxstr at = GetAtom(index).GetEllipsoid() == NULL ? "isotropic" : "anisotropic";
    E.ProcessingError(__OlxSrcInfo, "invalid number of arguments: ") << Params.Count() << " for " <<
      at << " atom " << GetAtom(index).Label();
  }
}
//..............................................................................
void TAsymmUnit::LibSetAtomOccu(const TStrObjList& Params, TMacroError& E)  {
  int index = Params[0].ToInt();
  if( index < 0 || index >= AtomCount() )  throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  double v = Params[1].ToDouble();
  int iv = (int)v/10;
  if( iv != 10 && iv != 0 )
    throw TFunctionFailedException(__OlxSourceInfo, "usupported occupancy format");
  GetAtom(index).SetOccp(v-iv);
  GetAtom(index).SetOccpVar(iv);
}
//..............................................................................
void TAsymmUnit::LibNewAtom(const TStrObjList& Params, TMacroError& E)  {
  int QPeakIndex = -1;
  double qPeak = 0;
  olxstr qLabel("Q");
  if( Params[0].IsNumber() )  {
    TPSTypeList<double, TCAtom*> sortedPeaks;
    qPeak = Params[0].ToDouble();
    for( int i=0; i < AtomCount(); i++ )  {
      if( GetAtom(i).GetAtomInfo().GetIndex() != iQPeakIndex )  continue;
      sortedPeaks.Add(GetAtom(i).GetQPeak(), &GetAtom(i) );
    }
    sortedPeaks.Add( qPeak, NULL);
    for( int i=0; i < sortedPeaks.Count(); i++ )  {
      if( sortedPeaks.GetObject(i) )
        sortedPeaks.GetObject(i)->SetLabel( qLabel + olxstr(sortedPeaks.Count() - i) );
    }
    QPeakIndex = sortedPeaks.Count() - sortedPeaks.IndexOfComparable( qPeak );
  }

  TCAtom& ca = this->NewAtom();
  if( QPeakIndex >= 0 )  {
    ca.SetLabel( qLabel << olxstr(QPeakIndex) );
    ca.SetQPeak( qPeak );
    ca.SetOccp(11.0);
    ca.SetUiso( 0.05 );
    if( qPeak > MaxQPeak )  MaxQPeak = qPeak;
    if( qPeak < MaxQPeak )  MinQPeak = qPeak;
  }
  else
    ca.SetLabel( Params[0] );
  ca.SetLoaderId( liNewAtom );
  E.SetRetVal( AtomCount() -1 );
  ca.AssignEllps( NULL );
}
//..............................................................................
void TAsymmUnit::LibGetZ(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal( Z );
}
//..............................................................................
void TAsymmUnit::LibSetZ(const TStrObjList& Params, TMacroError& E)  {
  Z = Params[0].ToInt();
  if( Z <= 0 )  Z = 1;
}
//..............................................................................
void TAsymmUnit::LibGetZprime(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal( 1 );
}
//..............................................................................
void TAsymmUnit::LibSetZprime(const TStrObjList& Params, TMacroError& E)  {
  double zp = Params[0].ToDouble();
  Z = Round(TUnitCell::GetMatrixMultiplier(Latt)*MatrixCount()*zp);
  if( Z <= 0 ) Z = 1;
}
//..............................................................................

TLibrary* TAsymmUnit::ExportLibrary(const olxstr& name) {

  TLibrary* lib = new TLibrary( name.IsEmpty() ? olxstr("au") : name );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibNewAtom, "NewAtom", fpOne,
"Adds a new atom to the asymmetric unit and return its ID, by which it can be reffered.\
 The function takes a single argument - the atom name") );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibGetAtomCount, "GetAtomCount", fpNone,
"Returns the atom count in the asymmetric untit") );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibGetSymm, "GetCellSymm", fpNone,
"Returns spacegroup of currently loaded file as name: 'C2', 'I41/amd', etc") );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibGetAtomCrd, "GetAtomCrd", fpOne,
"Returns a comma sperated list of fractional acoordinates for the specified atom") );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibGetAtomName, "GetAtomName", fpOne,
"Returns atom label") );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibGetAtomType, "GetAtomType", fpOne,
"Returns atom type (element)"  ) );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibGetAtomOccu, "GetAtomOccu", fpOne,
"Returns atom occupancy"  ) );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibGetAtomAfix, "GetAtomAfix", fpOne,
"Returns atom AFIX"  ) );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibGetPeak, "GetPeak", fpOne,
"Returns peak intensity"  ) );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibGetAtomU, "GetAtomU", fpOne,
"Returns a single number or six, comma separated values") );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibGetAtomUiso, "GetAtomUiso", fpOne,
"Returns a single number Uiso or (U11+U22+U33)/3") );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibGetCell, "GetCell", fpNone,
"Returns six comma separated values for a, b, c and alpha, beta, gamma") );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibGetVolume, "GetVolume", fpNone,
"Returns volume of the unit cell divided by the number of symmetry elements") );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibGetCellVolume, "GetCellVolume", fpNone,
"Returns volume of the unit cell") );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibSetAtomCrd, "SetAtomCrd", fpFour,
"Sets atom coordinates to specified values, first parameters is the atom ID") );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibSetAtomU, "SetAtomU", fpSeven | fpTwo,
"Sets atoms Uiso/anis first paramater is the atom ID followed by 1 or six parameters"  ) );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibSetAtomOccu, "SetAtomOccu", fpTwo,
"Sets atom's occupancy; first paramater is the atom ID followed by occupancy"  ) );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibSetAtomLabel, "SetAtomlabel", fpTwo,
"Sets atom labels to provided value. The first parameter is the atom ID") );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibGetAtomLabel, "GetAtomlabel", fpTwo,
"The takes two arguments - the atom ID and increment. The increment is used to navigate through\
 the periodic table, so increment +1 will return nex element and -1 the privious element in the\
 periodic table"  ) );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibIsAtomDeleted, "IsAtomDeleted", fpOne,
"Checks status of specified atom"  ) );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibIsPeak, "IsPeak", fpOne,
"Checks if specified atom is  peak"  ) );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibGetZ, "GetZ", fpNone,
"Returns current Z"  ) );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibSetZ, "SetZ", fpOne,
"Sets current Z. Does not update content or whatsoever"  ) );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibGetZprime, "GetZprime", fpNone,
"Returns current Z divided byt the number of matrices of current spacegroup"  ) );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibSetZprime, "SetZprime", fpOne,
"Sets Z' for the structure"  ) );
  return lib;
}
//..............................................................................
//..............................................................................

