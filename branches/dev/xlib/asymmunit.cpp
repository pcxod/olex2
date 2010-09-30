//----------------------------------------------------------------------------//
// TAsymmUnit: a collection of symmetry independent atoms
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//
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
#include "symmparser.h"
#include "refmodel.h"
#include "residue.h"

#undef GetObject

class TAU_SfacSorter  {
public:
  static int Compare(const TPrimitiveStrListData<olxstr,const cm_Element*>* s1, 
                    const TPrimitiveStrListData<olxstr,const cm_Element*>* s2)
  {
    const double diff = s1->Object->GetMr() - s1->Object->GetMr();
    return diff < 0 ? -1 : (diff > 0 ? 1 : 0);
  }
};

const olxstr TAsymmUnit::IdName("catom");

//----------------------------------------------------------------------------//
// TAsymmetricUnit function bodies
//----------------------------------------------------------------------------//
TAsymmUnit::TAsymmUnit(TLattice *L) : MainResidue(*(new TResidue(*this, 0))),
  OnSGChange(Actions.New("AU_SG_CHANGE"))
{
  Lattice   = L;
  Latt = -1;
  Assigning = false;
  Z = 1;
  RefMod = NULL;
}
//..............................................................................
TAsymmUnit::~TAsymmUnit()  {  
  Clear();  
  delete &MainResidue;
}
//..............................................................................
void  TAsymmUnit::Clear()  {
  Residues.Clear();
  MainResidue.Clear();

  Matrices.Clear();
  for( size_t i=0; i < CAtoms.Count(); i++ )
    delete CAtoms[i];
  CAtoms.Clear();
  for( size_t i=0; i < Centroids.Count(); i++ )
    delete Centroids[i];
  Centroids.Clear();
  for( size_t i=0; i < Ellipsoids.Count(); i++ )
    delete Ellipsoids[i];
  Ellipsoids.Clear();
  Latt = -1;
  Z = 1;
}
//..............................................................................
void TAsymmUnit::Assign(const TAsymmUnit& C)  {
  Clear();
  Assigning = true;
  FAxes   = C.FAxes;
  FAngles = C.FAngles;
  RAxes   = C.GetRAxes();
  RAngles = C.GetRAngles();
  Z = C.GetZ();
  Latt = C.GetLatt();

  for( size_t i = 0; i < C.MatrixCount(); i++ )
    Matrices.AddNew( C.GetMatrix(i) );
  
  for( size_t i = 0; i < C.EllpCount(); i++ )
    this->NewEllp() = C.GetEllp(i);

  for( size_t i=0; i < C.Residues.Count(); i++ )  {
    TResidue& resi = C.Residues[i];
    NewResidue(resi.GetClassName(), resi.GetNumber(), resi.GetAlias()); 
  }
  for( size_t i = 0; i < C.AtomCount(); i++ )
    NewAtom( &GetResidue(C.GetAtom(i).GetResiId()) ).SetId(i);
  
  for( size_t i = 0; i < C.AtomCount(); i++ )  {
    TCAtom& ca = GetAtom(i);
    ca.Assign(C.GetAtom(i));
    ca.SetId(i);
    //ca.SetConnInfo( RefMod->Conn.GetConnInfo(ca) );
  }
  // copy matrices
  Cartesian2Cell = C.GetCartesianToCell();
  Cell2Cartesian = C.GetCellToCartesian();
  Hkl2Cartesian =  C.GetHklToCartesian();
  UcifToUxyz     = C.UcifToUxyz;
  UxyzToUcif     = C.UxyzToUcif;
  UcifToUxyzT    = C.UcifToUxyzT;
  UxyzToUcifT    = C.UxyzToUcifT;

  MaxQPeak = C.GetMaxQPeak();
  MinQPeak = C.GetMinQPeak();
  Assigning = false;
}
//..............................................................................
void TAsymmUnit::ComplyToResidues()  {
  for( size_t i=0; i < CAtoms.Count(); i++ )
    CAtoms[i]->SetTag(-1);
  size_t ac = 0;
  for( size_t i=0; i < MainResidue.Count(); i++ )
    MainResidue[i].SetTag(ac++);
  for( size_t i=0; i < Residues.Count(); i++ )  {
    TResidue& resi = Residues[i];
    for( size_t j=0; j < resi.Count(); j++ )
      resi[j].SetTag(ac++);
  }
  CAtoms.QuickSorter.Sort<TCAtomTagComparator>(CAtoms);
  for( size_t i=0; i < CAtoms.Count(); i++ )
    CAtoms[i]->SetId(i);
}
//..............................................................................
void TAsymmUnit::_UpdateConnInfo()  {
  for( size_t i = 0; i < AtomCount(); i++ )  {
    TCAtom& ca = GetAtom(i);
    ca.SetConnInfo(RefMod->Conn.GetConnInfo(ca));
  }
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
  Cartesian2CellT = mat3d::Transpose(Cartesian2Cell);
  // cell to cartesian transformation matrix
  Cell2Cartesian.Null();
  Cell2Cartesian[0][0] = FAxes[0].GetV();
  Cell2Cartesian[1][0] = FAxes[1].GetV()*cG;
  Cell2Cartesian[2][0] = FAxes[2].GetV()*cB;

  Cell2Cartesian[1][1] = FAxes[1].GetV()*sG;
  Cell2Cartesian[2][1] = -FAxes[2].GetV()*(cB*cG-cA)/sG;

  Cell2Cartesian[2][2] = 1./cs;
  Cell2CartesianT = mat3d::Transpose(Cell2Cartesian);

  // init hkl to cartesian transformation matrix
//  TMatrixD m( *Cartesian2Cell );
  mat3d m = Cell2Cartesian;
  const vec3d v1(m[0]), v2(m[1]), v3(m[2]);

  Hkl2Cartesian[0] = v2.XProdVec(v3)/V;
  Hkl2Cartesian[1] = v3.XProdVec(v1)/V;
  Hkl2Cartesian[2] = v1.XProdVec(v2)/V;

// init Uaniso traformation matices
  m.Null();
  m[0][0] = Hkl2Cartesian[0].Length();
  m[1][1] = Hkl2Cartesian[1].Length();
  m[2][2] = Hkl2Cartesian[2].Length();

  UcifToUxyz = m * Cell2Cartesian;
  UcifToUxyzT = UcifToUxyz;
  UcifToUxyz.Transpose();

  m[0][0] = 1./Hkl2Cartesian[0].Length();
  m[1][1] = 1./Hkl2Cartesian[1].Length();
  m[2][2] = 1./Hkl2Cartesian[2].Length();

  UxyzToUcif = Cartesian2Cell * m;
  UxyzToUcifT = UxyzToUcif;
  UxyzToUcif.Transpose();
}
//..............................................................................
void TAsymmUnit::InitData()  {
  // init QPeak intensities
  MaxQPeak = -1000;
  MinQPeak = 1000;
  for( size_t i =0; i < AtomCount(); i++ )  {
    if( !CAtoms[i]->IsDeleted() && CAtoms[i]->GetType() == iQPeakZ )  {
      const double qpeak = CAtoms[i]->GetQPeak();
      if( qpeak < MinQPeak )  MinQPeak = qpeak;
      if( qpeak > MaxQPeak )  MaxQPeak = qpeak;
    }
  }
}
//..............................................................................
TResidue& TAsymmUnit::NewResidue(const olxstr& RClass, int number, const olxstr& alias)  {
  for( size_t i=0; i < Residues.Count(); i++ )
    if( Residues[i].GetNumber() == number )  {
      return Residues[i];
      //throw TInvalidArgumentException(__OlxSourceInfo, "dublicated residue number");
    }
  return Residues.Add( new TResidue(*this, (uint32_t)Residues.Count(), RClass, number, alias) );
}
//..............................................................................
void TAsymmUnit::FindResidues(const olxstr& resi, TPtrList<TResidue>& list) {
  if( resi.IsEmpty() )  {
    list.Add(&MainResidue);
    return;
  }
  if( resi.IsNumber() )  {
    int number = resi.ToInt();
    for( size_t i=0; i < Residues.Count(); i++ )  {
      if( Residues[i].GetNumber() == number )  {
        list.Add(Residues[i]);
        break;  // number must be unique
      }
    }
  }
  else  {
    if( resi.Length() == 1 && resi.CharAt(0) == '*' )  {  //special case
      for( size_t i=0; i < Residues.Count(); i++ )
        list.Add( Residues[i] );
      list.Add( &MainResidue );
    }
    for( size_t i=0; i < Residues.Count(); i++ )
      if( Residues[i].GetClassName().Equalsi(resi) || Residues[i].GetAlias().Equalsi(resi) ) 
        list.Add(Residues[i]);
  }
}
//..............................................................................
TResidue* TAsymmUnit::NextResidue(const TResidue& r) const {  return ((r.GetId()-1) == Residues.Count()) ? NULL : &Residues[r.GetId()+1];  }
//..............................................................................
TResidue* TAsymmUnit::PrevResidue(const TResidue& r) const {  
  return (r.GetId() == 0) ? NULL : ((r.GetId() == 0) ? &const_cast<TAsymmUnit*>(this)->MainResidue : &Residues[r.GetId()-1]);  
}
//..............................................................................
void TAsymmUnit::AssignResidues(const TAsymmUnit& au)  {
  if( CAtoms.Count() != au.CAtoms.Count() )
    throw TFunctionFailedException(__OlxSourceInfo, "asymmetric units mismatch");
  Residues.Clear();
  MainResidue.Clear();
  for( size_t i=0; i < au.MainResidue.Count(); i++ )
    MainResidue._Add( *CAtoms[au.MainResidue[i].GetId()] );
  for( size_t i=0; i < au.Residues.Count(); i++ )  {
    TResidue& that_resi = au.Residues[i];
    TResidue& this_resi = NewResidue( that_resi.GetClassName(), that_resi.GetNumber(), that_resi.GetAlias() );
    for( size_t j=0; j < that_resi.Count(); j++ )  {
      this_resi._Add( *CAtoms[that_resi[j].GetId()] );
    }
  }
}
//..............................................................................
void TAsymmUnit::_OnAtomTypeChanged(TCAtom& caller)  {
  if( !Assigning )  
    caller.SetConnInfo( RefMod->Conn.GetConnInfo(caller) );
}
//..............................................................................
TCAtom& TAsymmUnit::NewAtom(TResidue* resi)  {
  TCAtom *A = new TCAtom(this);
  A->SetId(CAtoms.Count());
  CAtoms.Add(A);
  if( resi == NULL )  
    resi = &MainResidue;
  resi->_Add(*A);
  return *A;
}
//..............................................................................
TCAtom& TAsymmUnit::NewCentroid(const vec3d& CCenter)  {
  TCAtom& A = NewAtom();
  A.SetType(XElementLib::GetByIndex(iCarbonIndex));
  A.ccrd() = CCenter;
  A.SetLabel(olxstr("Cnt") << CAtoms.Count(), false);
  return A;
}
//..............................................................................
TCAtom * TAsymmUnit::FindCAtom(const olxstr &Label, TResidue* resi)  const {
  int part = DefNoPart;
  olxstr lb(Label);
  size_t us_ind = Label.IndexOf('_');
  if( us_ind != InvalidIndex && ++us_ind < Label.Length() )  {
    if( Label.SubStringFrom(us_ind).IsNumber() )  {  // residue number?
      size_t resi_num = Label.SubStringFrom(us_ind).ToInt();
      for( size_t i=0; i < Residues.Count(); i++ )  {
        if( Residues[i].GetNumber() == resi_num )  {
          resi = &Residues[i];
          break;  // number must be unique
        }
      }
      if( resi == NULL )  // invalid residue?
        return NULL;
    }
    else
      part = olxstr::o_tolower(Label.CharAt(us_ind)) - 'a' + 1;
    lb = lb.SubStringTo(us_ind-1);
  }
  if( resi != NULL )  {
    if( Label.Equalsi("first") )  {
      for( size_t i=0; i < resi->Count(); i++ )
        if( !resi->GetAtom(i).IsDeleted() )
          return &resi->GetAtom(i);
    }
    else if( Label.Equalsi("last") )  {
      for( size_t i=resi->Count(); i > 0; i-- )
        if( !resi->GetAtom(i-1).IsDeleted() )
          return &resi->GetAtom(i-1);
    }
    else  {
      for( size_t i=0; i < resi->Count(); i++ )
        if( !resi->GetAtom(i).IsDeleted() && resi->GetAtom(i).GetLabel().Equalsi(lb) )
          if( part == DefNoPart || resi->GetAtom(i).GetPart() == part )
            return &resi->GetAtom(i);
    }
  }
  else  {  // global search
    if( Label.Equalsi("first") )  {
      for( size_t i=0; i < CAtoms.Count(); i++ )
        if( !CAtoms[i]->IsDeleted() )
          return CAtoms[i];
    }
    else if( Label.Equalsi("last") )  {
      for( size_t i = CAtoms.Count(); i > 0; i-- )
        if( !CAtoms[i-1]->IsDeleted() )
          return CAtoms[i-1];
    }
    else  {
      for( size_t i=0; i < CAtoms.Count(); i++ )
        if( !CAtoms[i]->IsDeleted() && CAtoms[i]->GetLabel().Equalsi(lb) )
          if( part == DefNoPart || CAtoms[i]->GetPart() == part )
            return CAtoms[i];
    }
  }
  return NULL;
}
//..............................................................................
void TAsymmUnit::DetachAtomType(short type, bool detach)  {
  const size_t ac = CAtoms.Count();
  for( size_t i =0; i < ac; i++ )  {
    if( CAtoms[i]->GetType() == type )
      CAtoms[i]->SetDetached(detach);
  }
}
//..............................................................................
void TAsymmUnit::PackAtoms()  {
  for( size_t i=0; i < Residues.Count(); i++ )
    GetResidue(i).Atoms.Pack(TCAtom::FlagsAnalyser<>(catom_flag_Deleted));
  CAtoms.Pack(TCAtom::FlagsAnalyser<>(catom_flag_Deleted));
  for( size_t i=0; i < CAtoms.Count(); i++ )
    CAtoms[i]->SetId(i);
}
//..............................................................................
TEllipsoid& TAsymmUnit::NewEllp() {
  TEllipsoid *E = new TEllipsoid();
  E->SetId(Ellipsoids.Count());
  Ellipsoids.Add(E);
  return *E;
}
//..............................................................................
void TAsymmUnit::PackEllps() {
  size_t removed = 0;
  for( size_t i=0; i < Ellipsoids.Count(); i++ )  {
    if( Ellipsoids[i] == NULL )  {
      for( size_t j=0; j < CAtoms.Count(); j++ )  {
        if( olx_is_valid_index(CAtoms[j]->GetEllpId()) && CAtoms[j]->GetEllpId() > (i-removed) )
          CAtoms[j]->SetEllpId(CAtoms[j]->GetEllpId() - 1);
      }
      removed++;
    }
    else
      Ellipsoids[i]->SetId(i-removed);
  }
  if( removed != 0 )
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
void TAsymmUnit::ClearEllps()  {
  for( size_t i=0; i < Ellipsoids.Count(); i++ )
    delete Ellipsoids[i];
  for( size_t i=0; i < CAtoms.Count(); i++ )
    CAtoms[i]->AssignEllp(NULL);
  Ellipsoids.Clear();
}
//..............................................................................
vec3d TAsymmUnit::GetOCenter(bool IncludeQ, bool IncludeH) const {
  vec3d P;
  double wght = 0;
  for( size_t i=0; i < AtomCount(); i++ )  {
    if( CAtoms[i]->IsDeleted() )  continue;
    if( !IncludeQ && CAtoms[i]->GetType() == iQPeakZ )  continue;
    if( !IncludeH && CAtoms[i]->GetType() == iHydrogenZ )  continue;
    P += CAtoms[i]->ccrd()*CAtoms[i]->GetOccu();
    wght += CAtoms[i]->GetOccu();
  }

  if( wght != 0 )
    P /= wght;
  return P;
}
//..............................................................................
/* since this is the AU, only the crystallographic occupancies must be summed up, atoms' 
degenracy should not be taken into account ... */
ContentList TAsymmUnit::GetContentList(double mult) const {
  ElementPList elements;
  ContentList rv;
  const cm_Element *Carbon=NULL, *Hydrogen=NULL;
  for( size_t i=0; i < AtomCount(); i++ )  {
    const cm_Element& elm = CAtoms[i]->GetType();
    if( CAtoms[i]->IsDeleted() || elm == iQPeakZ )  continue;
    size_t ind = elements.IndexOf(elm);
    if( ind == InvalidIndex )  {
      rv.AddNew(elm, CAtoms[i]->GetOccu()*mult);
      elements.Add(elm);
    }
    else
      rv[ind] += CAtoms[i]->GetOccu()*mult;
  }
  return XElementLib::SortContentList(rv);
}
//..............................................................................
olxstr TAsymmUnit::SummFormula(const olxstr &Sep, bool MultiplyZ) const  {
  size_t matrixInc = 0;
  // searching the identity matrix
  bool Uniq = true;
  for( size_t i=0; i < MatrixCount(); i++ )  {
    if( GetMatrix(i).IsI() )  {
      Uniq = false;
      break;
    }
  }
  if( Uniq )  matrixInc ++;

  ContentList cl = GetContentList(MultiplyZ ? (MatrixCount()+matrixInc) : 1.0);
  olxstr rv;
  for( size_t i=0; i < cl.Count(); i++)  {
    rv << cl[i].element.symbol;
    rv << olxstr::FormatFloat(3, cl[i].count).TrimFloat();
    if( (i+1) < cl.Count() )
      rv << Sep;
  }
  return rv;
}
//..............................................................................
double TAsymmUnit::MolWeight() const  {
  double Mw = 0;
  for( size_t i=0; i < AtomCount(); i++ )
    Mw += GetAtom(i).GetType().GetMr();
  return Mw;
}
//..............................................................................
void TAsymmUnit::AddMatrix(const smatd& a)  {
  if( a.r.IsI() )
    Matrices.InsertCCopy(0, a);
  else
    Matrices.AddCCopy(a);
}
//..............................................................................
olxstr TAsymmUnit::CheckLabel(const TCAtom* ca, const olxstr &Label, char a, char b, char c) const  {
  olxstr LB( (Label.Length() > 4) ? Label.SubStringTo(2) : Label );
  if( ca != NULL )  {
    const TResidue& resi = GetResidue(ca->GetResiId());
    for( size_t i=0; i < resi.Count(); i++ )  {
      const TCAtom& atom = resi[i];
      if( atom.GetPart() != ca->GetPart() && (atom.GetPart()|ca->GetPart()) != 0 )  continue;
      if( !atom.IsDeleted() && (atom.GetLabel().Equalsi(Label) ) && 
        (atom.GetId() != ca->GetId()) )  {
        LB = atom.GetType().symbol;
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
  for( size_t i=0; i < AtomCount(); i++ )  {
    const TCAtom& CA = GetAtom(i);
    if( !CA.IsDeleted() && CA.GetLabel().Equalsi(Label) )  {
      LB = CA.GetType().symbol;
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
olxstr TAsymmUnit::ValidateLabel(const olxstr &Label) const  {
  olxstr LB( (Label.Length() > 4) ? Label.SubStringTo(4) : Label );
  int cnt=0;
  for( size_t i=0; i < AtomCount(); i++ )  {
    const TCAtom& CA = GetAtom(i);
    if( !CA.IsDeleted() && CA.GetLabel().Equalsi(Label) )
       cnt++;
    if( cnt > 1 )
      return CheckLabel(NULL, LB);
  }
  return LB;
}
//..............................................................................
size_t TAsymmUnit::CountElements(const olxstr& Symbol) const  {
  cm_Element* elm = XElementLib::FindBySymbol(Symbol);
  if( elm == NULL )
    throw TInvalidArgumentException(__OlxSourceInfo, olxstr("unknown element: '") << Symbol << '\'');
  size_t cnt = 0;
  for( size_t i=0; i < AtomCount(); i++ )
    if( GetAtom(i).GetType() == *elm )
      cnt++;
  return cnt;
}
//..............................................................................
void TAsymmUnit::Sort(TCAtomPList* list) {
 // sorting by four params
  if( list == NULL )  list = &MainResidue.Atoms;
  TCAtomPList::QuickSorter.Sort<TCAtomPComparator>(*list);
  TCAtomPList::QuickSorter.Sort<TCAtomPComparator>(*list);
  TCAtomPList::QuickSorter.Sort<TCAtomPComparator>(*list);
  TCAtomPList::QuickSorter.Sort<TCAtomPComparator>(*list);
}
//..............................................................................
int TAsymmUnit::GetNextPart() const {
  int part = 0;
  for( size_t i=0; i < AtomCount(); i++ )
    if( GetAtom(i).GetPart() > part )
      part = GetAtom(i).GetPart();

  return part+1;
}
//..............................................................................
void TAsymmUnit::ChangeSpaceGroup(const TSpaceGroup& sg)  {
  OnSGChange.Execute(this, &sg);
  Latt = sg.GetLattice().GetLatt();
  if( !sg.IsCentrosymmetric() && Latt > 0 )  Latt = -Latt;

  Matrices.Clear();
  for( size_t i=0; i < sg.MatrixCount(); i++ )
    Matrices.AddCCopy(sg.GetMatrix(i));
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
double TAsymmUnit::EstimateZ(double atomCount) const  {
  double auv = (double)(CalcCellVolume()/(TUnitCell::GetMatrixMultiplier(GetLatt())*(MatrixCount()+1)));
  int zp = olx_round(auv/(18.6*atomCount));
  return (double)olx_max((TUnitCell::GetMatrixMultiplier(GetLatt())*(MatrixCount()+1) * zp), 1);
}
//..............................................................................
void TAsymmUnit::FitAtoms(TTypeList<AnAssociation3<TCAtom*, const cm_Element*, bool> >& _atoms,
  const vec3d_list& _crds, bool _try_invert)
{
  // validate input
  if( _atoms.Count() != _crds.Count() )
    throw TInvalidArgumentException(__OlxSourceInfo, "mismatching atoms and coordinates lists");
  size_t _atom_cnt = 0;
  for( size_t i=0; i < _atoms.Count(); i++ )  {
    if( _atoms[i].GetA() != NULL )  {
      if( _atoms[i].GetC() )
        _atom_cnt++;
    }
    else if( _atoms[i].GetB() == NULL )
      throw TInvalidArgumentException(__OlxSourceInfo, "neither atom or element type is provided");
  }
  if( _atom_cnt < 3 )
    throw TInvalidArgumentException(__OlxSourceInfo, "too few atoms for fitting");
  else if( _atom_cnt == 3 )
    _try_invert = false;
  TTypeList< AnAssociation2<vec3d, vec3d> > crds;
  for( size_t i=0; i < _atoms.Count(); i++ )  {
    if( _atoms[i].GetA() != NULL && _atoms[i].GetC() )
      crds.AddNew(_atoms[i].GetA()->ccrd(), _crds[i]);
  }
  // normal coordinate match
  smatdd tm;
  vec3d tr, t;
  for( size_t k=0; k < crds.Count(); k++ )  {
    t += CellToCartesian(crds[k].A());
    tr += crds[k].GetB();
  }
  t /= crds.Count();
  tr /= crds.Count();
  tm.t = t;
  const double rms = TNetwork::FindAlignmentMatrix(crds, t, tr, tm);
  bool invert = false;
  if( _try_invert )  {  // try inverted coordinate set
    TTypeList< AnAssociation2<vec3d, vec3d> > icrds;
    for( size_t i=0; i < _atoms.Count(); i++ )  {
      if( _atoms[i].GetA() != NULL && _atoms[i].GetC() )
        icrds.AddNew(_atoms[i].GetA()->ccrd(), _crds[i]);
    }
    smatdd tmi;
    vec3d tri;
    for( size_t i=0; i < crds.Count(); i++ )  {
      icrds[i].A() = crds[i].GetA();
      CartesianToCell(icrds[i].B()) *= -1;
      CellToCartesian(icrds[i].B());
      tri += icrds[i].GetB();
    }
    tri /= crds.Count();
    tmi.t = t;
    const double irms = TNetwork::FindAlignmentMatrix(icrds, t, tri, tmi);
    if( irms < rms && irms >= 0 )  {
      tr = tri;
      tm = tmi;
      invert = true;
    }
  }
  for( size_t i=0; i < _atoms.Count(); i++ )  {
    vec3d v = _crds[i];
    if( invert )  {
      CartesianToCell(v);
      v *= -1;
      CellToCartesian(v);
    }
    v = tm*(v-tr);
    if( _atoms[i].GetA() == NULL )  {
      _atoms[i].A() = &NewAtom();
      _atoms[i].A()->SetType(*_atoms[i].GetB());
      _atoms[i].A()->SetLabel(_atoms[i].A()->GetType().symbol+(olxstr('x') << (char)('a'+i)), false);
      GetRefMod()->Vars.SetParam(*_atoms[i].A(), catom_var_name_Sof, 11.0);
    }
    _atoms[i].A()->ccrd() = CartesianToCell(v);
  }
}
//..............................................................................
void TAsymmUnit::ToDataItem(TDataItem& item) const  {
  TDataItem& cell = item.AddItem("cell");
  cell.AddField("a", FAxes[0].ToString());
  cell.AddField("b", FAxes[1].ToString());
  cell.AddField("c", FAxes[2].ToString());
  cell.AddField("alpha", FAngles[0].ToString());
  cell.AddField("beta",  FAngles[1].ToString());
  cell.AddField("gamma", FAngles[2].ToString());
  cell.AddField("Z", Z);
  TDataItem& symm = item.AddItem("symm");
  symm.AddField("latt", Latt);
  for( size_t i=0; i < Matrices.Count(); i++ )  
    symm.AddItem(i, TSymmParser::MatrixToSymmEx(Matrices[i]));
  size_t aid=0;
  for( size_t i=0; i < CAtoms.Count(); i++ )
    if( !CAtoms[i]->IsDeleted() )
      CAtoms[i]->SetTag(aid++);
  aid=0;
  TDataItem& resi = item.AddItem("residues");
  for( size_t i=0; i < ResidueCount(); i++ )  {
    TResidue& r = GetResidue(i);
    if( r.IsEmpty() )  continue;
    TDataItem* ri;
    if( i == 0 )
      ri = &resi.AddItem("default");
    else  {
      ri = &resi.AddItem( r.GetNumber() );
      ri->AddField("class_name", r.GetClassName());
      ri->AddField("alias", r.GetAlias());
    }
    for( size_t j=0; j < r.Count(); j++ )  {
      if( r[j].IsDeleted() )  continue;
      r[j].ToDataItem(ri->AddItem(aid++));
    }
  }
}
//..............................................................................
#ifndef _NO_PYTHON
PyObject* TAsymmUnit::PyExport(TPtrList<PyObject>& _atoms)  {
  for( size_t i=0; i < CAtoms.Count(); i++ )
    CAtoms[i]->SetId(i);
  PyObject* main = PyDict_New(), *cell = PyDict_New();
  PythonExt::SetDictItem(cell, "a", Py_BuildValue("(dd)", FAxes[0].GetV(), FAxes[0].GetE()));
  PythonExt::SetDictItem(cell, "b", Py_BuildValue("(dd)", FAxes[1].GetV(), FAxes[1].GetE()));
  PythonExt::SetDictItem(cell, "c", Py_BuildValue("(dd)", FAxes[2].GetV(), FAxes[2].GetE()));
  PythonExt::SetDictItem(cell, "alpha", Py_BuildValue("(dd)", FAngles[0].GetV(), FAngles[0].GetE()));
  PythonExt::SetDictItem(cell, "beta", Py_BuildValue("(dd)", FAngles[1].GetV(), FAngles[1].GetE()));
  PythonExt::SetDictItem(cell, "gamma", Py_BuildValue("(dd)", FAngles[2].GetV(), FAngles[2].GetE()));
  PythonExt::SetDictItem(cell, "z", Py_BuildValue("i", Z));
  PythonExt::SetDictItem(main, "cell", cell);
  // pre-set atom tags
  size_t aid=0;
  for( size_t i=0; i < CAtoms.Count(); i++ )
    if( !CAtoms[i]->IsDeleted() )
      CAtoms[i]->SetTag(aid++);
  size_t resi_cnt = 0;
  for( size_t i=0; i < ResidueCount(); i++ )  {
    TResidue& r = GetResidue(i);
    if( r.IsEmpty() )  continue;
    resi_cnt++;
  }
  PyObject* residues = PyTuple_New(resi_cnt);
  resi_cnt = 0;
  for( size_t i=0; i < ResidueCount(); i++ )  {
    TResidue& r = GetResidue(i);
    if( r.IsEmpty() )  continue;
    size_t atom_cnt = 0;
    for( size_t j=0; j < r.Count(); j++ )  {
      if( r[j].IsDeleted() )  continue;
      atom_cnt++;
    }
    PyObject* atoms = PyTuple_New(atom_cnt), 
      *ri = PyDict_New();

    if( i == 0 )
      PythonExt::SetDictItem(ri, "class", PythonExt::BuildString("default"));
    else  {
      PythonExt::SetDictItem(ri, "class", PythonExt::BuildString(r.GetClassName()));
      PythonExt::SetDictItem(ri, "alias", PythonExt::BuildString(r.GetAlias()));
      PythonExt::SetDictItem(ri, "number", Py_BuildValue("i", r.GetNumber()));
    }
    atom_cnt = 0;
    for( size_t j=0; j < r.Count(); j++ )  {
      if( r[j].IsDeleted() )  continue;
      PyObject* atom = _atoms.Add(r[j].PyExport());
      PythonExt::SetDictItem(atom, "aunit_id", Py_BuildValue("i", r[j].GetId()));
      PyTuple_SetItem(atoms, atom_cnt++, atom);
    }
    PythonExt::SetDictItem(ri, "atoms", atoms);
    PyTuple_SetItem(residues, resi_cnt++, ri);
  }
  PythonExt::SetDictItem(main, "residues", residues);
  return main;
}
#endif
//..............................................................................
void TAsymmUnit::FromDataItem(TDataItem& item)  {
  Clear();
  TDataItem& cell = item.FindRequiredItem("cell");
  FAxes[0] = cell.GetRequiredField("a");
  FAxes[1] = cell.GetRequiredField("b");
  FAxes[2] = cell.GetRequiredField("c");
  FAngles[0] = cell.GetRequiredField("alpha");
  FAngles[1] = cell.GetRequiredField("beta");
  FAngles[2] = cell.GetRequiredField("gamma");
  Z = cell.GetRequiredField("Z").RadUInt<unsigned short>();
  TDataItem& symm = item.FindRequiredItem("symm");
  Latt = symm.GetRequiredField("latt").ToInt();
  TPtrList<TDataItem> atom_items;
  for( size_t i=0; i < symm.ItemCount(); i++ )  
    TSymmParser::SymmToMatrix(symm.GetItem(i).GetValue(), Matrices.AddNew());
  TDataItem& resis = item.FindRequiredItem("residues");
  for( size_t i=0; i < resis.ItemCount(); i++ )  {
    TDataItem& resi = resis.GetItem(i);
    TResidue& r = (i==0 ? MainResidue : NewResidue(resi.GetRequiredField("class_name"),
      resi.GetValue().ToInt(), resi.GetRequiredField("alias")) );
    for( size_t j=0; j < resi.ItemCount(); j++ )  {
      atom_items.Add(resi.GetItem(j));
      NewAtom(&r);
    }
  }
  for( size_t i=0; i < atom_items.Count(); i++ )
    CAtoms[i]->FromDataItem(*atom_items[i]);
  InitMatrices();
  InitData();
}
//..............................................................................
//..............................................................................
//..............................................................................
//..............................................................................
void TAsymmUnit::LibGetAtomCount(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal( AtomCount() );
}
//..............................................................................
void TAsymmUnit::LibGetAtomCrd(const TStrObjList& Params, TMacroError& E)  {
  size_t index = Params[0].ToSizeT();
  if( index >= AtomCount() )  throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  E.SetRetVal(GetAtom(index).ccrd().ToString());
}
//..............................................................................
void TAsymmUnit::LibGetAtomName(const TStrObjList& Params, TMacroError& E)  {
  size_t index = Params[0].ToSizeT();
  if( index >= AtomCount() )  throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  E.SetRetVal(GetAtom(index).GetLabel());
}
//..............................................................................
void TAsymmUnit::LibGetAtomType(const TStrObjList& Params, TMacroError& E)  {
  size_t index = Params[0].ToSizeT();
  if( index >= AtomCount() )  throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  E.SetRetVal(GetAtom(index).GetType().symbol);
}
//..............................................................................
void TAsymmUnit::LibGetPeak(const TStrObjList& Params, TMacroError& E)  {
  if( Params[0].IsNumber() )  {
    size_t index = Params[0].ToSizeT();
    if( index >= AtomCount() )  throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
    E.SetRetVal(GetAtom(index).GetQPeak());
  }
  else  {
    TCAtom* ca = FindCAtom(Params[0]);
    if( ca != NULL && ca->GetType() == iQPeakZ )
      E.SetRetVal(ca->GetQPeak());
    else
      throw TInvalidArgumentException(__OlxSourceInfo, olxstr("unknown peak \'") << Params[0] << '\'');
  }
}
//..............................................................................
void TAsymmUnit::LibGetAtomU(const TStrObjList& Params, TMacroError& E)  {
  size_t index = Params[0].ToSizeT();
  if( index >= AtomCount() )  throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  evecd Q(1);
  if( GetAtom(index).GetEllipsoid() == NULL )  {
    // TODO: a special condition - the atom is isotropic, but a user wishes it to be
    // anisotropic - six values a, a, a, 0, 0, 0 have to be passed
    //if( GetAtom(index)->
    Q[0] = GetAtom(index).GetUiso();
  }
  else  {  // the function resises the vector automatically
    Q.Resize(6);
    GetAtom(index).GetEllipsoid()->GetQuad(Q);
  }

  E.SetRetVal( Q.ToString() );
}
//..............................................................................
void TAsymmUnit::LibGetAtomUiso(const TStrObjList& Params, TMacroError& E)  {
  size_t index = Params[0].ToSizeT();
  if( index >= AtomCount() )  throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
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
  TSpaceGroup* sg = TSymmLib::GetInstance().FindSG(*this);
  if( sg == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "Could not locate spacegroup" );
    return;
  }
  E.SetRetVal( sg->GetName() );
}
//..............................................................................
void TAsymmUnit::LibSetAtomCrd(const TStrObjList& Params, TMacroError& E)  {
  size_t index = Params[0].ToSizeT();
  if( index >= AtomCount() )  throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  TCAtom& ca = GetAtom(index);
  for( int i=0; i < 3; i++ )
    GetRefMod()->Vars.SetParam(ca, catom_var_name_X+i, Params[i+1].ToDouble());
  E.SetRetVal(true);
}
//..............................................................................
void TAsymmUnit::LibSetAtomLabel(const TStrObjList& Params, TMacroError& E)  {
  size_t index = Params[0].ToSizeT();
  if( index >= AtomCount() )  throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  olxstr newLabel;
  if( Params[1].IsNumber() )  {
    int inc = Params[1].ToInt();
    int v = GetAtom(index).GetType().index + inc;
    if( v >= 0 && v <= iQPeakIndex )  {
      newLabel << XElementLib::GetByIndex(v).symbol
               << GetAtom(index).GetLabel().SubStringFrom(
                    GetAtom(index).GetType().symbol.Length());
    }
  }
  else  {
    newLabel = Params[1];
  }
  newLabel = CheckLabel(&GetAtom(index), newLabel);
  if( !newLabel.Length() )  {
    E.ProcessingError(__OlxSrcInfo, "incorrect label ") << Params[1];
    return;
  }
  GetAtom(index).SetLabel(newLabel);
}
//..............................................................................
void TAsymmUnit::LibGetAtomLabel(const TStrObjList& Params, TMacroError& E)  {
  size_t index = Params[0].ToSizeT();
  if( index >= AtomCount() )  throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  olxstr newLabel;
  if( Params[1].IsNumber() )  {
    int inc = Params[1].ToInt();
    int v = GetAtom(index).GetType().index + inc;
    if( v >= 0 && v <= iQPeakIndex )  {
      E.SetRetVal(XElementLib::GetByIndex(v).symbol);
      return;
    }
  }
  else  {
    E.ProcessingError(__OlxSrcInfo, "a number is expected" );
    E.SetRetVal(E.GetInfo());
    return;
  }
}
//..............................................................................
void TAsymmUnit::LibIsAtomDeleted(const TStrObjList& Params, TMacroError& E)  {
  size_t index = Params[0].ToSizeT();
  if( index >= AtomCount() )  throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  E.SetRetVal(GetAtom(index).IsDeleted());
}
//..............................................................................
void TAsymmUnit::LibGetAtomOccu(const TStrObjList& Params, TMacroError& E)  {
  size_t index = Params[0].ToSizeT();
  if( index >= AtomCount() )  throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  E.SetRetVal( GetAtom(index).GetOccu() );
}
//..............................................................................
void TAsymmUnit::LibGetAtomAfix(const TStrObjList& Params, TMacroError& E)  {
  size_t index = Params[0].ToSizeT();
  if( index >= AtomCount() )  throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  E.SetRetVal( GetAtom(index).GetAfix() );
}
//..............................................................................
void TAsymmUnit::LibIsPeak(const TStrObjList& Params, TMacroError& E)  {
  if( Params[0].IsNumber() )  {
    size_t index = Params[0].ToSizeT();
    if( index >= AtomCount() )  throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
    E.SetRetVal(GetAtom(index).GetType() == iQPeakZ );
  }
  else  {
    TCAtom* ca = FindCAtom( Params[0] );
    if( ca != NULL )
      E.SetRetVal(ca->GetType() == iQPeakZ );
    else
      E.SetRetVal(false);
  }
}
//..............................................................................
void TAsymmUnit::LibSetAtomU(const TStrObjList& Params, TMacroError& E)  {
  size_t index = Params[0].ToSizeT();
  if( index >= AtomCount() )  throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  TCAtom& ca = GetAtom(index);
  if( (GetAtom(index).GetEllipsoid() != NULL) && (Params.Count() == 7) )  {
    double V[6];
    for( int i=0; i < 6; i++ )
      V[i] = GetRefMod()->Vars.SetParam(ca, catom_var_name_U11+i, Params[i+1].ToDouble());
    ca.GetEllipsoid()->Initialise( V );
  }
  else if( (ca.GetEllipsoid() == NULL) && (Params.Count() == 2) ) {
    GetRefMod()->Vars.SetParam(ca, catom_var_name_Uiso, Params[1].ToDouble());
  }
  else {
    olxstr at = ca.GetEllipsoid() == NULL ? "isotropic" : "anisotropic";
    E.ProcessingError(__OlxSrcInfo, "invalid number of arguments: ") << Params.Count() << " for " <<
      at << " atom " << ca.GetLabel();
  }
}
//..............................................................................
void TAsymmUnit::LibSetAtomOccu(const TStrObjList& Params, TMacroError& E)  {
  size_t index = Params[0].ToSizeT();
  if( index >= AtomCount() )  throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  GetRefMod()->Vars.SetParam(GetAtom(index), catom_var_name_Sof, Params[1].ToDouble());
}
//..............................................................................
void TAsymmUnit::LibNewAtom(const TStrObjList& Params, TMacroError& E)  {
  vec3d crd(Params[1].ToDouble(), Params[2].ToDouble(), Params[3].ToDouble());
  if( Lattice != NULL )  {
    vec3d test_pos(crd);
    if( Lattice->GetUnitCell().FindOverlappingAtom( test_pos, 0.3 ) != NULL )  {
      E.SetRetVal(-1);
      return;
    }
  }
  size_t QPeakIndex = InvalidIndex;
  double qPeak = 0;
  olxstr qLabel("Q");
  if( Params[0].IsNumber() )  {
    TPSTypeList<double, TCAtom*> sortedPeaks;
    qPeak = Params[0].ToDouble();
    size_t ac = CAtoms.Count();
    for( size_t i=0; i < ac; i++ )  {
      if( CAtoms[i]->GetType() != iQPeakZ || CAtoms[i]->IsDeleted() )  continue;
      sortedPeaks.Add(CAtoms[i]->GetQPeak(), CAtoms[i]);
    }
    sortedPeaks.Add(qPeak, NULL);
    ac = sortedPeaks.Count();
    for( size_t i=0; i < ac; i++ )  {
      if( sortedPeaks.GetObject(i) != NULL )
        sortedPeaks.GetObject(i)->SetLabel(qLabel + olxstr(ac-i), false);
    }
    QPeakIndex = ac - sortedPeaks.IndexOfComparable( qPeak );
    MinQPeak = sortedPeaks.GetComparable(0);
    MaxQPeak = sortedPeaks.Last().Comparable;
  }

  TCAtom& ca = this->NewAtom();
  if( QPeakIndex != InvalidIndex )  {
    ca.SetLabel(qLabel << olxstr(QPeakIndex), false);
    ca.SetType(XElementLib::GetByIndex(iQPeakIndex));
    ca.SetQPeak(qPeak);
    GetRefMod()->Vars.SetParam(ca, catom_var_name_Sof, 11.0);
    GetRefMod()->Vars.SetParam(ca, catom_var_name_Uiso, 0.5);
    for( short i=0; i < 3; i++ )
      GetRefMod()->Vars.SetParam(ca, catom_var_name_X+i, crd[i]);
  }
  else
    ca.SetLabel(Params[0]);
  E.SetRetVal( AtomCount() -1 );
  ca.AssignEllp(NULL);
}
//..............................................................................
void TAsymmUnit::LibGetZ(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal(Z);
}
//..............................................................................
void TAsymmUnit::LibSetZ(const TStrObjList& Params, TMacroError& E)  {
  Z = Params[0].ToInt();
  if( Z <= 0 )  Z = 1;
}
//..............................................................................
void TAsymmUnit::LibGetZprime(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal(olxstr::FormatFloat(3,(double)Z/(TUnitCell::GetMatrixMultiplier(Latt)*(MatrixCount()+1))));
}
//..............................................................................
void TAsymmUnit::LibSetZprime(const TStrObjList& Params, TMacroError& E)  {
  double zp = Params[0].ToDouble();
  Z = (short)olx_round(TUnitCell::GetMatrixMultiplier(Latt)*(MatrixCount()+1)*zp);
  if( Z <= 0 ) Z = 1;
}
//..............................................................................

TLibrary* TAsymmUnit::ExportLibrary(const olxstr& name)  {
  TLibrary* lib = new TLibrary( name.IsEmpty() ? olxstr("au") : name );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibNewAtom, "NewAtom", fpFour,
"Adds a new atom to the asymmetric unit and return its ID, by which it can be reffered.\
 The function takes the atom name and ccordinates, if -1 is returned, the atom is not created") );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibGetAtomCount, "GetAtomCount", fpNone,
"Returns the atom count in the asymmetric unit") );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibGetSymm, "GetCellSymm", fpNone,
"Returns spacegroup of currently loaded file as name: 'C2', 'I41/amd', etc") );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibGetAtomCrd, "GetAtomCrd", fpOne,
"Returns a comma separated list of fractional coordinates for the specified atom") );
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
"Sets atom's occupancy; first parameter is the atom ID followed by occupancy"  ) );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibSetAtomLabel, "SetAtomlabel", fpTwo,
"Sets atom labels to provided value. The first parameter is the atom ID") );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibGetAtomLabel, "GetAtomlabel", fpTwo,
"The takes two arguments - the atom ID and increment. The increment is used to navigate through\
 the periodic table, so increment +1 will return next element and -1 the previous element in the\
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

