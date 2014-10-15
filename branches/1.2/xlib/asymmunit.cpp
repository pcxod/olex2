/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

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
#include "math/align.h"
#include "label_corrector.h"

#undef GetObject

class TAU_SfacSorter {
public:
  template <class item_t>
  static int Compare(const item_t& s1, const item_t &s2) {
    return olx_cmp(olx_ptr::get(s1)->GetMr(), olx_ptr::get(s2)->GetMr());
  }
};

const olxstr TAsymmUnit::IdName("catom");

TAsymmUnit::TAsymmUnit(TLattice *L) : MainResidue(*(new TResidue(*this, 0))),
  OnSGChange(Actions.New("AU_SG_CHANGE"))
{
  Lattice = L;
  Latt = -1;
  Assigning = false;
  Z = 1;
  RefMod = NULL;
  MaxQPeak = MinQPeak = 0;
}
//..............................................................................
TAsymmUnit::~TAsymmUnit()  {
  Clear();
  delete &MainResidue;
}
//..............................................................................
void  TAsymmUnit::Clear()  {
  Residues.Clear();
  ResidueRegistry.Clear();
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
  MaxQPeak = MinQPeak = 0;
}
//..............................................................................
void TAsymmUnit::Assign(const TAsymmUnit& C)  {
  Clear();
  Assigning = true;
  Axes   = C.Axes;
  AxisEsds = C.AxisEsds;
  Angles = C.Angles;
  AngleEsds = C.AngleEsds;
  RAxes   = C.GetRAxes();
  RAngles = C.GetRAngles();
  Z = C.GetZ();
  Latt = C.GetLatt();

  for( size_t i = 0; i < C.MatrixCount(); i++ )
    Matrices.AddNew(C.GetMatrix(i));

  for( size_t i = 0; i < C.EllpCount(); i++ )
    this->NewEllp() = C.GetEllp(i);

  for( size_t i=0; i < C.Residues.Count(); i++ )  {
    TResidue& resi = C.Residues[i];
    NewResidue(resi.GetClassName(), resi.GetNumber(), resi.GetAlias()).
      SetCapacity(resi.Count());
  }
  for( size_t i = 0; i < C.AtomCount(); i++ )
    NewAtom(&GetResidue(C.GetAtom(i).GetResiId())).SetId(i);

  for( size_t i = 0; i < C.AtomCount(); i++ )  {
    TCAtom& ca = GetAtom(i);
    ca.Assign(C.GetAtom(i));
    ca.SetId(i);
  }
  // copy matrices
  Cartesian2Cell = C.GetCartesianToCell();
  Cartesian2CellT = C.Cartesian2CellT;
  Cell2CartesianT = C.Cell2CartesianT;
  Cell2Cartesian = C.GetCellToCartesian();
  Cell2CartesianT = C.Cell2CartesianT;
  Hkl2Cartesian =  C.GetHklToCartesian();
  UcifToUxyz = C.UcifToUxyz;
  UxyzToUcif = C.UxyzToUcif;
  UcifToUxyzT = C.UcifToUxyzT;
  UxyzToUcifT = C.UxyzToUcifT;

  MaxQPeak = C.GetMaxQPeak();
  MinQPeak = C.GetMinQPeak();
  Assigning = false;
}
//..............................................................................
void TAsymmUnit::ComplyToResidues()  {
  CAtoms.ForEach(ACollectionItem::TagSetter(-1));
  size_t ac = 0;
  for( size_t i=0; i < MainResidue.Count(); i++ )
    MainResidue[i].SetTag(ac++);
  for( size_t i=0; i < Residues.Count(); i++ )  {
    TResidue& resi = Residues[i];
    for( size_t j=0; j < resi.Count(); j++ )
      resi[j].SetTag(ac++);
  }
  QuickSorter::Sort(CAtoms, ACollectionItem::TagComparator());
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
  if( Axes.Prod() == 0 )
    throw TFunctionFailedException(__OlxSourceInfo, "zero cell parameters");
  // just to check the validity of my deductions put this in seems to be the same ...
  double cG = cos(Angles[2]/180*M_PI),
         cB = cos(Angles[1]/180*M_PI),
         cA = cos(Angles[0]/180*M_PI),
         sG = sin(Angles[2]/180*M_PI),
         sB = sin(Angles[1]/180*M_PI),
         sA = sin(Angles[0]/180*M_PI);
  const double Vp = sqrt((1-cA*cA-cB*cB-cG*cG) + 2*(cA*cB*cG));
  const double V = Axes.Prod()*Vp;

  const double
    cBs = (cA*cG-cB)/(sA*sG),
    cAs = (cB*cG-cA)/(sB*sG),
    as = Axes[1]*Axes[2]*sA/V,
    bs = Axes[0]*Axes[2]*sB/V,
    cs = Axes[0]*Axes[1]*sG/V;
  // cartesian to cell transformation matrix
  Cartesian2Cell.Null();
  Cartesian2Cell[0][0] =  1./Axes[0];
  Cartesian2Cell[1][0] = -cG/(sG*Axes[0]);
  Cartesian2Cell[2][0] = as*cBs;

  Cartesian2Cell[1][1] = 1./(sG*Axes[1]);
  Cartesian2Cell[2][1] = bs*cAs;

  Cartesian2Cell[2][2] = cs;
  Cartesian2CellT = mat3d::Transpose(Cartesian2Cell);
  // cell to cartesian transformation matrix
  Cell2Cartesian.Null();
  Cell2Cartesian[0][0] = Axes[0];
  Cell2Cartesian[1][0] = Axes[1]*cG;
  Cell2Cartesian[2][0] = Axes[2]*cB;

  Cell2Cartesian[1][1] = Axes[1]*sG;
  Cell2Cartesian[2][1] = -Axes[2]*(cB*cG-cA)/sG;

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
  //ematd DG(6,6);
  //// a b c alpha beta gamma
  //// o11 o21 o22 o31 o32 o33
  //DG[0][0] = 1; DG[0][1] = 0; DG[0][2] = 0; DG[0][3] = 0; DG[0][4] = 0; DG[0][5] = 0;
  //DG[1][0] = 0; DG[1][1] = cG; DG[1][2] = 0; DG[1][3] = 0; DG[1][4] = 0; DG[1][5] = -b*cG;
  //DG[2][0] = 0; DG[2][1] = sG; DG[2][2] = 0; DG[2][3] = 0; DG[2][4] = 0; DG[2][5] = b*cG;

  //DG[3][0] = 0; DG[3][1] = 0; DG[3][2] = cB; DG[3][3] = 0; DG[3][4] = -c*sB; DG[0][5] = 0;
  //DG[4][0] = 0; DG[4][1] = 0;
  //DG[4][2] = (cA-cB*cG)/sG;
  //DG[4][3] = -c*sA/sG;
  //DG[4][4] = c*sB*cG/sG;
  //DG[4][5] = c*(cB + (cB*cG-cA)*cG/(sG*sG));
  //
  //
  //DG[5][0] = 0;
  //DG[5][1] = 0;
  //DG[5][2] = Vp/sG;
  //DG[5][3] = c*sA*(cB*cG-cA)/Vp;
  //DG[5][4] = c*sB*(cB-cA*cG)/Vp;
  //DG[0][5] = c*(cA*(-cB-cB*cG*cG+cG*cA)+cG*cB*cB)/(Vp*sG*sG);
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
TResidue& TAsymmUnit::NewResidue(const olxstr& RClass, int number, int alias)  {
  if (number == 0) {
    if (!RClass.IsEmpty()) {
      throw TInvalidArgumentException(__OlxSourceInfo,
        "Cannot rename main residue");
    }
    return MainResidue;
  }
  TResidue *er = ResidueRegistry.Find(number, NULL);
  if (er == NULL && alias != number)
    er = ResidueRegistry.Find(alias, NULL);
  if (er != NULL) {
    if (!er->GetClassName().Equalsi(RClass)) {
      throw TInvalidArgumentException(__OlxSourceInfo,
        olx_print("Residue number %d is already assigned class %w", number,
          &er->GetClassName())
        );
    }
    return *er;
  }
  TResidue &r = Residues.Add(
    new TResidue(*this, (uint32_t)Residues.Count()+1, RClass, number, alias)
  );
  ResidueRegistry(number, &r);
  if (alias != number)
    ResidueRegistry(alias, &r);
  return r;
}
//..............................................................................
ConstPtrList<TResidue> TAsymmUnit::FindResidues(const olxstr& resi) const{
  TPtrList<TResidue> list;
  if (resi.IsEmpty())
    list.Add(&MainResidue);
  else if (resi.IsNumber()) {
    int n = resi.ToInt();
    TResidue *r = ResidueRegistry.Find(n, NULL);
    if (r != NULL)
      list.Add(r);
  }
  else  {
    if( resi == '*' )  {  //special case
      list.SetCapacity(Residues.Count()+1);
      list.Add(MainResidue);
      for (size_t i=0; i < Residues.Count(); i++)
        list.Add(Residues[i]);
    }
    else {
      for (size_t i=0; i < Residues.Count(); i++)
        if (Residues[i].GetClassName().Equalsi(resi))
          list.Add(Residues[i]);
    }
  }
  return list;
}
//..............................................................................
TResidue* TAsymmUnit::NextResidue(const TResidue& r) const {
  return FindResidue(r.GetNumber()+1);
}
//..............................................................................
TResidue* TAsymmUnit::PrevResidue(const TResidue& r) const {
  return FindResidue(r.GetNumber()-1);
}
//..............................................................................
void TAsymmUnit::Release(const TPtrList<TResidue> &rs) {
  for (size_t i=0; i < rs.Count(); i++) {
    size_t idx = ResidueRegistry.IndexOf(rs[i]->GetNumber());
    if (idx == InvalidIndex)
      throw TInvalidArgumentException(__OlxSourceInfo, "residue");
    ResidueRegistry.Delete(idx);
    if (rs[i]->HasAlias()) {
      idx = ResidueRegistry.IndexOf(rs[i]->GetAlias());
      if (idx == InvalidIndex)
        throw TInvalidArgumentException(__OlxSourceInfo, "residue");
      ResidueRegistry.Delete(idx);
    }
    for (size_t j=0; j < Residues.Count(); j++) {
      if (&Residues[j] == rs[i]) {
        Residues.Release(j);
        break;
      }
    }
  }
  for (size_t i=0; i < Residues.Count(); i++)
    Residues[i].SetId((uint32_t)i+1);
}
//..............................................................................
void TAsymmUnit::Restore(const TPtrList<TResidue> &rs) {
  // validate if unique
  for (size_t i=0; i < rs.Count(); i++) {
    size_t idx = ResidueRegistry.IndexOf(rs[i]->GetNumber());
    if (idx == InvalidIndex && rs[i]->HasAlias())
      idx = ResidueRegistry.IndexOf(rs[i]->GetAlias());
    if (idx != InvalidIndex)
      throw TInvalidArgumentException(__OlxSourceInfo, "residue number/alias");
  }
  for (size_t i=0; i < rs.Count(); i++) {
    Residues.Add(rs[i]);
    ResidueRegistry.Add(rs[i]->GetNumber(), rs[i]);
    if (rs[i]->HasAlias())
      ResidueRegistry.Add(rs[i]->GetAlias(), rs[i]);
  }
  BubbleSorter::Sort(Residues);
  for (size_t i=0; i < Residues.Count(); i++)
    Residues[i].SetId((uint32_t)i+1);
}
//..............................................................................
void TAsymmUnit::AssignResidues(const TAsymmUnit& au) {
  if( CAtoms.Count() != au.CAtoms.Count() )
    throw TFunctionFailedException(__OlxSourceInfo, "asymmetric units mismatch");
  Residues.Clear();
  ResidueRegistry.Clear();
  MainResidue.Clear();
  MainResidue.SetCapacity(au.MainResidue.Count());
  for (size_t i=0; i < au.MainResidue.Count(); i++)
    MainResidue._Add(*CAtoms[au.MainResidue[i].GetId()]);
  for (size_t i=0; i < au.Residues.Count(); i++) {
    TResidue& that_resi = au.Residues[i];
    TResidue& this_resi = NewResidue(
      that_resi.GetClassName(), that_resi.GetNumber(), that_resi.GetAlias());
    this_resi.SetCapacity(that_resi.Count());
    for (size_t j=0; j < that_resi.Count(); j++)
      this_resi._Add(*CAtoms[that_resi[j].GetId()]);
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
    if (Label.SubStringFrom(us_ind).IsNumber()) {  // residue number?
      int resi_num = Label.SubStringFrom(us_ind).ToInt();
      resi = ResidueRegistry.Find(resi_num, resi);
      if (resi == NULL)
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
        if( !resi->GetAtom(i).IsDeleted() &&
            resi->GetAtom(i).GetLabel().Equalsi(lb) )
        {
          if( part == DefNoPart || resi->GetAtom(i).GetPart() == part )
            return &resi->GetAtom(i);
        }
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
TCAtom *TAsymmUnit::FindCAtomDirect(const olxstr &label) const {
  const size_t ac = CAtoms.Count();
  for( size_t i =0; i < ac; i++ )  {
    if (CAtoms[i]->GetLabel().Equalsi(label))
      return CAtoms[i];
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
    GetResidue(i).Atoms.Pack(TCAtom::FlagsAnalyser(catom_flag_Deleted));
  CAtoms.Pack(TCAtom::FlagsAnalyser(catom_flag_Deleted));
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
/* since this is the AU, only the crystallographic occupancies must be summed
up, atoms' degeracy should not be taken into account ...
*/
ContentList::const_list_type TAsymmUnit::GetContentList(double mult) const {
  ElementPList elements;
  ContentList rv;
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
olxstr TAsymmUnit::_SummFormula(const olxstr &Sep, double mult) const {
  ContentList cl = GetContentList(mult);
  olxstr rv;
  for( size_t i=0; i < cl.Count(); i++)  {
    rv << cl[i].element.symbol;
    if( olx_abs(cl[i].count-1.0) > 1e-3 )
      rv << olxstr::FormatFloat(3, cl[i].count).TrimFloat();
    if( (i+1) < cl.Count() )
      rv << Sep;
  }
  return rv;
}
//..............................................................................
olxstr TAsymmUnit::SummFormula(const olxstr &Sep, bool MultiplyZ) const  {
  size_t matrixInc = 0;
  // searching the identity matrix
  bool hasI = false;
  for (size_t i=0; i < MatrixCount(); i++) {
    if (GetMatrix(i).IsI()) {
      hasI = true;
      break;
    }
  }
  if (!hasI)  matrixInc ++;
  double m = 1;
  if (MultiplyZ) {
    m =(double) (MatrixCount() + matrixInc)*TCLattice::GetLattMultiplier(this->Latt);
  }
  return _SummFormula(Sep, m);
}
//..............................................................................
double TAsymmUnit::GetZPrime() const {
  return (double)Z/(TCLattice::GetLattMultiplier(Latt)*(MatrixCount()+1));
}
//..............................................................................
double TAsymmUnit::MolWeight() const  {
  double Mw = 0;
  for( size_t i=0; i < AtomCount(); i++ )  {
    if( GetAtom(i).IsDeleted() )  continue;
    Mw += GetAtom(i).GetOccu()*GetAtom(i).GetType().GetMr();

  }
  return Mw;
}
//..............................................................................
void TAsymmUnit::AddMatrix(const smatd& a)  {
  if( a.r.IsI() )
    Matrices.InsertCopy(0, a);
  else
    Matrices.AddCopy(a);
}
//..............................................................................
olxstr TAsymmUnit::CheckLabel(const TCAtom* ca, const olxstr &Label,
  char a, char b, char c) const
{
  olxstr LB((Label.Length() > 4) ? Label.SubStringTo(2) : Label);
  if (ca != NULL) {
    const TResidue& resi = GetResidue(ca->GetResiId());
    for (size_t i=0; i < resi.Count(); i++) {
      const TCAtom& atom = resi[i];
      if( atom.GetPart() != ca->GetPart() &&
          (atom.GetPart()|ca->GetPart()) != 0)
      {
        continue;
      }
      if (!atom.IsDeleted() && (atom.GetLabel().Equalsi(Label) ) &&
        (atom.GetId() != ca->GetId()))
      {
        LB = atom.GetType().symbol;
        LB << a << b;
        if (LB.Length() < 4)  LB << c;
        if (a < '9')  return CheckLabel(ca, LB, (char)(a+1), b, c);
        if (b < 'z')  return CheckLabel(ca, LB, '0', (char)(b+1), c);
        if (c < 'z')  return CheckLabel(ca, LB, '0', 'a', (char)(c+1));
        throw TFunctionFailedException(__OlxSourceInfo, "cannot create label");
      }
    }
    return LB;
  }
  for (size_t i=0; i < AtomCount(); i++) {
    const TCAtom& CA = GetAtom(i);
    if (!CA.IsDeleted() && CA.GetLabel().Equalsi(Label)) {
      LB = CA.GetType().symbol;
      LB << a << b;
      if (LB.Length() < 4)  LB << c;
      if (a < '9')  return CheckLabel(ca, LB, (char)(a+1), b, c);
      if (b < 'z')  return CheckLabel(ca, LB, '0', (char)(b+1), c);
      if (c < 'z')  return CheckLabel(ca, LB, '0', 'a', (char)(c+1));
      throw TFunctionFailedException(__OlxSourceInfo, "cannot create label");
    }
  }
  return LB;
}
//..............................................................................
double TAsymmUnit::CountElementOccupancy(const olxstr& Symbol) const  {
  cm_Element* elm = XElementLib::FindBySymbol(Symbol);
  if( elm == NULL ) {
    throw TInvalidArgumentException(__OlxSourceInfo,
      olxstr("unknown element: ").quote() << Symbol);
  }
  double sum = 0;
  for (size_t i=0; i < AtomCount(); i++) {
    if (!GetAtom(i).IsDeleted() && GetAtom(i).GetType() == *elm) {
      sum += GetAtom(i).GetOccu();
    }
  }
  return sum;
}
//..............................................................................
void TAsymmUnit::Sort(TCAtomPList* list) {
 // sorting by four params
  if( list == NULL )  list = &MainResidue.Atoms;
  QuickSorter::Sort(*list, TCAtomComparator());
}
//..............................................................................
int TAsymmUnit::GetNextPart(bool neg) const {
  if( !neg )  {
    int part = 0;
    for( size_t i=0; i < AtomCount(); i++ )
      if( !GetAtom(i).IsDeleted() && GetAtom(i).GetPart() > part )
        part = GetAtom(i).GetPart();
    return part+1;
  }
  else  {
    int part = 0;
    for( size_t i=0; i < AtomCount(); i++ )
      if( !GetAtom(i).IsDeleted() && GetAtom(i).GetPart() < part )
        part = GetAtom(i).GetPart();
    return part-1;
  }
}
//..............................................................................
void TAsymmUnit::ChangeSpaceGroup(const TSpaceGroup& sg)  {
  OnSGChange.Execute(this, &sg);
  Latt = sg.GetLattice().GetLatt();
  if( !sg.IsCentrosymmetric() && Latt > 0 )
    Latt = -Latt;
  Matrices.Clear();
  if (sg.IsCentrosymmetric() && !sg.GetInversionCenter().IsNull(1e-3)) {
    sg.GetMatrices(Matrices, mattAll^mattCentering);
    Matrices.Delete(0);
    Latt = -sg.GetLattice().GetLatt();
  }
  else {
    for( size_t i=0; i < sg.MatrixCount(); i++ )
      Matrices.AddCopy(sg.GetMatrix(i));
  }
}
//..............................................................................
double TAsymmUnit::CalcCellVolume() const {
  double cosa = cos(Angles[0]*M_PI/180),
         cosb = cos(Angles[1]*M_PI/180),
         cosg = cos(Angles[2]*M_PI/180);
  return  Axes.Prod()*sqrt((1-cosa*cosa-cosb*cosb-cosg*cosg) + 2*(cosa*cosb*cosg));
}
double TAsymmUnit::EstimateZ(double atomCount) const  {
  double auv = (double)(CalcCellVolume()/
    (TCLattice::GetLattMultiplier(GetLatt())*(MatrixCount()+1)));
  int zp = olx_round(auv/(18.6*atomCount));
  return (double)olx_max((TCLattice::GetLattMultiplier(GetLatt())*
    (MatrixCount()+1) * zp), 1);
}
//..............................................................................
void TAsymmUnit::FitAtoms(TTypeList<AnAssociation3<TCAtom*, const cm_Element*,
  bool> >& _atoms, const vec3d_list& _crds, bool _try_invert)
{
  // validate input
  if( _atoms.Count() != _crds.Count() ) {
    throw TInvalidArgumentException(__OlxSourceInfo,
      "mismatching atoms and coordinates lists");
  }
  size_t _atom_cnt = 0;
  for( size_t i=0; i < _atoms.Count(); i++ )  {
    if( _atoms[i].GetA() != NULL )  {
      if( _atoms[i].GetC() )
        _atom_cnt++;
    }
    else if( _atoms[i].GetB() == NULL ) {
      throw TInvalidArgumentException(__OlxSourceInfo,
        "neither atom or element type is provided");
    }
  }
  if( _atom_cnt < 3 ) {
    throw TInvalidArgumentException(__OlxSourceInfo,
      "too few atoms for fitting");
  }
  else if( _atom_cnt == 3 )
    _try_invert = false;
  TTypeList<align::pair> pairs;
  for( size_t i=0; i < _atoms.Count(); i++ )  {
    if( _atoms[i].GetA() != NULL && _atoms[i].GetC() ) {
      CellToCartesian(
        pairs.AddNew(_atoms[i].GetA()->ccrd(), _crds[i]).a.value);
    }
  }
  align::out ao = align::FindAlignmentQuaternions(pairs);
  // normal coordinate match
  smatdd tm;
  QuaternionToMatrix(ao.quaternions[0], tm.r);
  tm.r.Transpose();
  tm.t = ao.center_a;
  vec3d tr = ao.center_b;
  bool invert = false;
  if( _try_invert )  {  // try inverted coordinate set
    TTypeList<align::pair> ipairs;
    for( size_t i=0; i < _atoms.Count(); i++ )  {
      if( _atoms[i].GetA() != NULL && _atoms[i].GetC() ) {
        CellToCartesian(
          ipairs.AddNew(_atoms[i].GetA()->ccrd()*-1, _crds[i]).a.value);
      }
    }
    align::out iao = align::FindAlignmentQuaternions(ipairs);
    smatdd itm;
    QuaternionToMatrix(iao.quaternions[0], itm.r);
    itm.r.Transpose();
    if( iao.rmsd[0] < ao.rmsd[0] )  {
      tr = iao.center_b;
      tm.r = itm.r;
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
      _atoms[i].a = &NewAtom();
      _atoms[i].a->SetType(*_atoms[i].GetB());
      _atoms[i].a->SetLabel(
        _atoms[i].GetA()->GetType().symbol+(olxstr('x') << (char)('a'+i)), false);
      GetRefMod()->Vars.SetParam(*_atoms[i].a, catom_var_name_Sof, 11.0);
    }
    _atoms[i].a->ccrd() = CartesianToCell(v);
  }
}
//..............................................................................
void TAsymmUnit::ToDataItem(TDataItem& item) const  {
  TDataItem& cell = item.AddItem("cell");
  cell.AddField("a", TEValueD(Axes[0], AxisEsds[0]).ToString());
  cell.AddField("b", TEValueD(Axes[1], AxisEsds[1]).ToString());
  cell.AddField("c", TEValueD(Axes[2], AxisEsds[2]).ToString());
  cell.AddField("alpha", TEValueD(Angles[0], AngleEsds[0]).ToString());
  cell.AddField("beta",  TEValueD(Angles[1], AngleEsds[1]).ToString());
  cell.AddField("gamma", TEValueD(Angles[2], AngleEsds[2]).ToString());
  cell.AddField("Z", Z);
  TDataItem& symm = item.AddItem("symm");
  symm.AddField("latt", Latt);
  for (size_t i=0; i < Matrices.Count(); i++)
    symm.AddItem("symmop", TSymmParser::MatrixToSymmEx(Matrices[i]));
  size_t aid=0;
  for( size_t i=0; i < ResidueCount(); i++ )  {
    TResidue& r = GetResidue(i);
    for (size_t j=0; j < r.Count(); j++)
      r[j].SetTag(r[j].IsDeleted() ? -1 : aid++);
  }
  TDataItem& resi = item.AddItem("residues");
  for( size_t i=0; i < ResidueCount(); i++ )  {
    TResidue& r = GetResidue(i);
    if( r.IsEmpty() )  continue;
    TDataItem* ri;
    if( i == 0 )
      ri = &resi.AddItem("default");
    else  {
      ri = &resi.AddItem(r.GetNumber());
      ri->AddField("class_name", r.GetClassName());
      if (r.HasAlias())
        ri->AddField("alias", r.GetAlias());
    }
    olxstr atom_di = "atom";
    for( size_t j=0; j < r.Count(); j++ )  {
      if( r[j].GetTag() < 0  )  continue;
      r[j].ToDataItem(ri->AddItem(atom_di));
    }
  }
}
//..............................................................................
#ifdef _PYTHON
PyObject* TAsymmUnit::PyExport(TPtrList<PyObject>& _atoms, bool export_conn)  {
  for( size_t i=0; i < CAtoms.Count(); i++ )
    CAtoms[i]->SetId(i);
  PyObject* main = PyDict_New(), *cell = PyDict_New();
  PythonExt::SetDictItem(cell, "a",
    Py_BuildValue("(dd)", Axes[0], AxisEsds[0]));
  PythonExt::SetDictItem(cell, "b",
    Py_BuildValue("(dd)", Axes[1], AxisEsds[1]));
  PythonExt::SetDictItem(cell, "c",
    Py_BuildValue("(dd)", Axes[2], AxisEsds[2]));
  PythonExt::SetDictItem(cell, "alpha",
    Py_BuildValue("(dd)", Angles[0], AngleEsds[0]));
  PythonExt::SetDictItem(cell, "beta",
    Py_BuildValue("(dd)", Angles[1], AngleEsds[1]));
  PythonExt::SetDictItem(cell, "gamma",
    Py_BuildValue("(dd)", Angles[2], AngleEsds[2]));
  PythonExt::SetDictItem(cell, "z", Py_BuildValue("i", Z));
  PythonExt::SetDictItem(main, "cell", cell);
  // pre-set atom tags
  TEBitArray deleted(CAtoms.Count());
  size_t aid=0;
  for( size_t i=0; i < ResidueCount(); i++ )  {
    TResidue& r = GetResidue(i);
    for (size_t j=0; j < r.Count(); j++) {
      if (r[j].GetType() == iQPeakZ && !r[j].IsDeleted()) {
        deleted.SetTrue(r[j].GetId());
        r[j].SetDeleted(true);
        r[j].SetTag(-1);
      }
      else
        r[j].SetTag(r[j].IsDeleted() ? -1 : aid++);
    }
  }
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
      if( r[j].GetTag() < 0 )  continue;
      atom_cnt++;
    }
    PyObject* atoms = PyTuple_New(atom_cnt),
      *ri = PyDict_New();

    if( i == 0 )
      PythonExt::SetDictItem(ri, "class", PythonExt::BuildString("default"));
    else  {
      PythonExt::SetDictItem(ri, "class", PythonExt::BuildString(r.GetClassName()));
      PythonExt::SetDictItem(ri, "alias", Py_BuildValue("i", r.GetAlias()));
      PythonExt::SetDictItem(ri, "number", Py_BuildValue("i", r.GetNumber()));
    }
    atom_cnt = 0;
    for( size_t j=0; j < r.Count(); j++ )  {
      if( r[j].GetTag() < 0 )  continue;
      PyObject* atom = _atoms.Add(r[j].PyExport(export_conn));
      PythonExt::SetDictItem(atom, "aunit_id", Py_BuildValue("i", r[j].GetId()));
      PyTuple_SetItem(atoms, atom_cnt++, atom);
    }
    PythonExt::SetDictItem(ri, "atoms", atoms);
    PyTuple_SetItem(residues, resi_cnt++, ri);
  }
  PythonExt::SetDictItem(main, "residues", residues);
  for (size_t i=0; i < CAtoms.Count(); i++) {
    if (deleted[i])
      CAtoms[i]->SetDeleted(false);
  }
  return main;
}
#endif
//..............................................................................
void TAsymmUnit::FromDataItem(TDataItem& item)  {
  Clear();
  TDataItem& cell = item.GetItemByName("cell");
  TEValueD evalue;
  evalue = cell.GetFieldByName("a");
  Axes[0] = evalue.GetV();  AxisEsds[0] = evalue.GetE();
  evalue = cell.GetFieldByName("b");
  Axes[1] = evalue.GetV();  AxisEsds[1] = evalue.GetE();
  evalue = cell.GetFieldByName("c");
  Axes[2] = evalue.GetV();  AxisEsds[2] = evalue.GetE();

  evalue = cell.GetFieldByName("alpha");
  Angles[0] = evalue.GetV();  AngleEsds[0] = evalue.GetE();
  evalue = cell.GetFieldByName("beta");
  Angles[1] = evalue.GetV();  AngleEsds[1] = evalue.GetE();
  evalue = cell.GetFieldByName("gamma");
  Angles[2] = evalue.GetV();  AngleEsds[2] = evalue.GetE();
  Z = cell.GetFieldByName("Z").RadUInt<unsigned short>();
  TDataItem& symm = item.GetItemByName("symm");
  Latt = symm.GetFieldByName("latt").ToInt();
  TPtrList<TDataItem> atom_items;
  for (size_t i = 0; i < symm.ItemCount(); i++) {
    Matrices.AddCopy(
      TSymmParser::SymmToMatrix(symm.GetItemByIndex(i).GetValue()));
  }
  TDataItem& resis = item.GetItemByName("residues");
  for( size_t i=0; i < resis.ItemCount(); i++ )  {
    TDataItem& resi = resis.GetItemByIndex(i);
    TResidue& r = (i==0 ? MainResidue
      : NewResidue(resi.GetFieldByName("class_name"),
        resi.GetName().ToInt(),
        resi.FindField("alias",resi.GetName()).ToInt()));
    for( size_t j=0; j < resi.ItemCount(); j++ )  {
      atom_items.Add(resi.GetItemByIndex(j));
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
  if( index >= AtomCount() )
    throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  E.SetRetVal(GetAtom(index).ccrd().ToString());
}
//..............................................................................
void TAsymmUnit::LibGetAtomName(const TStrObjList& Params, TMacroError& E)  {
  size_t index = Params[0].ToSizeT();
  if( index >= AtomCount() )
    throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  E.SetRetVal(GetAtom(index).GetLabel());
}
//..............................................................................
void TAsymmUnit::LibGetAtomType(const TStrObjList& Params, TMacroError& E)  {
  size_t index = Params[0].ToSizeT();
  if( index >= AtomCount() )
    throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  E.SetRetVal(GetAtom(index).GetType().symbol);
}
//..............................................................................
void TAsymmUnit::LibGetPeak(const TStrObjList& Params, TMacroError& E)  {
  if( Params[0].IsNumber() )  {
    size_t index = Params[0].ToSizeT();
    if( index >= AtomCount() )
      throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
    E.SetRetVal(GetAtom(index).GetQPeak());
  }
  else  {
    TCAtom* ca = FindCAtom(Params[0]);
    if( ca != NULL && ca->GetType() == iQPeakZ )
      E.SetRetVal(ca->GetQPeak());
    else {
      throw TInvalidArgumentException(__OlxSourceInfo,
        olxstr("unknown peak \'") << Params[0] << '\'');
    }
  }
}
//..............................................................................
void TAsymmUnit::LibGetAtomU(const TStrObjList& Params, TMacroError& E)  {
  size_t index = Params[0].ToSizeT();
  if( index >= AtomCount() )
    throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  evecd Q;
  if( GetAtom(index).GetEllipsoid() == NULL )  {
    Q.Resize(1);
    // TODO: a special condition - the atom is isotropic, but a user wishes it to be
    // anisotropic - six values a, a, a, 0, 0, 0 have to be passed
    //if( GetAtom(index)->
    Q[0] = GetAtom(index).GetUiso();
  }
  else  {  // the function resises the vector automatically
    Q.Resize(6);
    GetAtom(index).GetEllipsoid()->GetShelxQuad(Q);
  }

  E.SetRetVal(Q.ToString());
}
//..............................................................................
void TAsymmUnit::LibGetAtomUiso(const TStrObjList& Params, TMacroError& E)  {
  size_t index = Params[0].ToSizeT();
  if( index >= AtomCount() )
    throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  E.SetRetVal(GetAtom(index).GetUiso());
}
//..............................................................................
void TAsymmUnit::LibGetCell(const TStrObjList& Params, TMacroError& E)  {
  evecd V(6);
  if (Params.IsEmpty() || Params[0].Equalsi("cell")) {
    V[0] = Axes[0];    V[1] = Axes[1];    V[2] = Axes[2];
    V[3] = Angles[0];  V[4] = Angles[1];  V[5] = Angles[2];
  }
  else if (Params[0].Equalsi("esd")) {
    V[0] = AxisEsds[0];    V[1] = AxisEsds[1];    V[2] = AxisEsds[2];
    V[3] = AngleEsds[0];  V[4] = AngleEsds[1];  V[5] = AngleEsds[2];
  }
  E.SetRetVal(V.ToString());
}
//..............................................................................
void TAsymmUnit::LibGetVolume(const TStrObjList& Params, TMacroError& E)  {
  double v = CalcCellVolume()/Lattice->GetUnitCell().MatrixCount();
  E.SetRetVal(v);
}
//..............................................................................
void TAsymmUnit::LibGetCellVolume(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal(CalcCellVolume());
}
//..............................................................................
void TAsymmUnit::LibGetSymm(const TStrObjList& Params, TMacroError& E)  {
  TSpaceGroup& sg = TSymmLib::GetInstance().FindSG(*this);
  if (Params.IsEmpty())
    E.SetRetVal(sg.GetName());
  else if (Params[0].Equalsi("hall"))
    E.SetRetVal(sg.GetHallSymbol());
}
//..............................................................................
void TAsymmUnit::LibSetAtomCrd(const TStrObjList& Params, TMacroError& E)  {
  size_t index = Params[0].ToSizeT();
  if( index >= AtomCount() )
    throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  TCAtom& ca = GetAtom(index);
  for( int i=0; i < 3; i++ )  {
    XVarReference* vr = ca.GetVarRef(catom_var_name_X+i);
    const double val = Params[i+1].ToDouble();
    if( vr != NULL )  {  // should preserve the variable - smtbx
      if( vr->relation_type == relation_AsVar )
        vr->Parent.SetValue(val/vr->coefficient);
      else if( vr->relation_type == relation_AsOneMinusVar )
        vr->Parent.SetValue(1.0 - val/vr->coefficient);
      ca.ccrd()[i] = val;
    }
    else
      GetRefMod()->Vars.SetParam(ca, catom_var_name_X+i, val);
  }
  E.SetRetVal(true);
}
//..............................................................................
void TAsymmUnit::LibSetAtomLabel(const TStrObjList& Params, TMacroError& E)  {
  size_t index = Params[0].ToSizeT();
  if (index >= AtomCount())
    throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  olxstr newLabel;
  if (Params[1].IsNumber()) {
    int inc = Params[1].ToInt();
    int v = GetAtom(index).GetType().index + inc;
    if (v >= 0 && v <= iQPeakIndex) {
      newLabel << XElementLib::GetByIndex(v).symbol <<
        GetAtom(index).GetLabel().SubStringFrom(
          GetAtom(index).GetType().symbol.Length());
    }
  }
  else {
    newLabel = Params[1];
  }
  newLabel = CheckLabel(&GetAtom(index), newLabel);
  if (!newLabel.Length()) {
    E.ProcessingError(__OlxSrcInfo, "incorrect label ").quote() << Params[1];
    return;
  }
  GetAtom(index).SetLabel(newLabel);
}
//..............................................................................
void TAsymmUnit::LibGetAtomLabel(const TStrObjList& Params, TMacroError& E)  {
  size_t index = Params[0].ToSizeT();
  if( index >= AtomCount() )
    throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
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
    E.ProcessingError(__OlxSrcInfo, "a number is expected");
    E.SetRetVal(E.GetInfo());
    return;
  }
}
//..............................................................................
void TAsymmUnit::LibIsAtomDeleted(const TStrObjList& Params, TMacroError& E)  {
  size_t index = Params[0].ToSizeT();
  if( index >= AtomCount() )
    throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  E.SetRetVal(GetAtom(index).IsDeleted());
}
//..............................................................................
void TAsymmUnit::LibGetAtomOccu(const TStrObjList& Params, TMacroError& E)  {
  size_t index = Params[0].ToSizeT();
  if( index >= AtomCount() )
    throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  E.SetRetVal(GetAtom(index).GetOccu());
}
//..............................................................................
void TAsymmUnit::LibGetAtomAfix(const TStrObjList& Params, TMacroError& E)  {
  size_t index = Params[0].ToSizeT();
  if( index >= AtomCount() )
    throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  E.SetRetVal(GetAtom(index).GetAfix());
}
//..............................................................................
void TAsymmUnit::LibIsPeak(const TStrObjList& Params, TMacroError& E)  {
  if( Params[0].IsNumber() )  {
    size_t index = Params[0].ToSizeT();
    if( index >= AtomCount() )
      throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
    E.SetRetVal(GetAtom(index).GetType() == iQPeakZ);
  }
  else  {
    TCAtom* ca = FindCAtom(Params[0]);
    if( ca != NULL )
      E.SetRetVal(ca->GetType() == iQPeakZ);
    else
      E.SetRetVal(false);
  }
}
//..............................................................................
void TAsymmUnit::LibSetAtomU(const TStrObjList& Params, TMacroError& E)  {
  size_t index = Params[0].ToSizeT();
  if( index >= AtomCount() )
    throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  TCAtom& ca = GetAtom(index);
  if( (GetAtom(index).GetEllipsoid() != NULL) && (Params.Count() == 7) )  {
    double V[6];
    for( int i=0; i < 6; i++ )  {
      XVarReference* vr = ca.GetVarRef(catom_var_name_U11+i);
      const double val = Params[i+1].ToDouble();
      if( vr != NULL )  {  // should preserve the variable - smtbx
        if( vr->relation_type == relation_AsVar )
          vr->Parent.SetValue(val/vr->coefficient);
        else if( vr->relation_type == relation_AsOneMinusVar )
          vr->Parent.SetValue(1.0 - val/vr->coefficient);
        V[i] = val;
      }
      else
        V[i] = GetRefMod()->Vars.SetParam(ca, catom_var_name_U11+i, val);
    }
    ca.GetEllipsoid()->Initialise(V);
    ca.SetUiso(ca.GetEllipsoid()->GetUeq());
  }
  else if( (ca.GetEllipsoid() == NULL) && (Params.Count() == 2) ) {
    XVarReference* vr = ca.GetVarRef(catom_var_name_Uiso);
    const double val = Params[1].ToDouble();
    if( vr != NULL )  {  // should preserve the variable - smtbx
      if( vr->relation_type == relation_AsVar )
        vr->Parent.SetValue(val/vr->coefficient);
      else if( vr->relation_type == relation_AsOneMinusVar )
        vr->Parent.SetValue(1.0 - val/vr->coefficient);
      ca.SetUiso(val);
    }
    else
      GetRefMod()->Vars.SetParam(ca, catom_var_name_Uiso, val);
  }
  else {
    olxstr at = ca.GetEllipsoid() == NULL ? "isotropic" : "anisotropic";
    E.ProcessingError(__OlxSrcInfo,
      "invalid number of arguments: ") << Params.Count() << " for " <<
      at << " atom " << ca.GetLabel();
  }
}
//..............................................................................
void TAsymmUnit::LibSetAtomOccu(const TStrObjList& Params, TMacroError& E)  {
  size_t index = Params[0].ToSizeT();
  if( index >= AtomCount() )
    throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  TCAtom& a = GetAtom(index);
  const double val = Params[1].ToDouble();
  XVarReference* vr = a.GetVarRef(catom_var_name_Sof);
  if( vr != NULL )  {  // should preserve the variable - smtbx
    if( vr->relation_type == relation_AsVar )
      vr->Parent.SetValue(val/vr->coefficient);
    else if( vr->relation_type == relation_AsOneMinusVar )
      vr->Parent.SetValue(1.0 - val/vr->coefficient);
    a.SetOccu(val);
  }
  else
    GetRefMod()->Vars.SetParam(a, catom_var_name_Sof, val);
}
//..............................................................................
void TAsymmUnit::_UpdateQPeaks() {
  sorted::PrimitiveAssociation<double, TCAtom*> sortedPeaks;
  size_t ac = CAtoms.Count();
  for (size_t i=0; i < ac; i++) {
    if (CAtoms[i]->GetType() != iQPeakZ || CAtoms[i]->IsDeleted()) continue;
    sortedPeaks.Add(CAtoms[i]->GetQPeak(), CAtoms[i]);
  }
  ac = sortedPeaks.Count();
  for (size_t i=0; i < ac; i++)
    sortedPeaks.GetValue(i)->SetLabel(olxstr('Q') << olxstr(ac - i), false);
  if (ac != 0) {
    MinQPeak = sortedPeaks.GetKey(0);
    MaxQPeak = sortedPeaks.GetLast().Key;
  }
  else {
    MaxQPeak = -1000;
    MinQPeak = 1000;
  }
}
//..............................................................................
void TAsymmUnit::LibNewAtom(const TStrObjList& Params, TMacroError& E)  {
  vec3d crd(Params[1].ToDouble(), Params[2].ToDouble(), Params[3].ToDouble());
  bool is_q_peak = Params[0].IsNumber(),
    validate = (Params.Count() == 5 ? Params[4].ToBool() : true);
  if (Lattice != NULL && validate) {
    vec3d test_pos(crd);
    TCAtom* ca =
      Lattice->GetUnitCell().FindOverlappingAtom(test_pos, is_q_peak, 0.01);
    if (ca != NULL) {
      if( is_q_peak && (ca->GetType() == iQPeakZ || ca->IsDeleted()))  {
        ca->SetDeleted(false);
        ca->SetType(XElementLib::GetByIndex(iQPeakIndex));
        ca->SetQPeak(Params[0].ToDouble());
        _UpdateQPeaks();
        E.SetRetVal(ca->GetId());
        return;
      }
      // q-peak on an atom - skip
      E.SetRetVal(-1);
      return;
    }
  }
  TCAtom& ca = this->NewAtom();
  if( is_q_peak )  {
    ca.SetType(XElementLib::GetByIndex(iQPeakIndex));
    ca.SetQPeak(Params[0].ToDouble());
    _UpdateQPeaks();
  }
  else
    ca.SetLabel(Params[0]);
  ca.SetOccu(1./Lattice->GetUnitCell().GetPositionMultiplicity(crd));
  GetRefMod()->Vars.SetParam(ca, catom_var_name_Sof,
    1./Lattice->GetUnitCell().GetPositionMultiplicity(crd));
  GetRefMod()->Vars.FixParam(ca, catom_var_name_Sof);
  GetRefMod()->Vars.SetParam(ca, catom_var_name_Uiso, 0.5);
  for( short i=0; i < 3; i++ )
    GetRefMod()->Vars.SetParam(ca, catom_var_name_X+i, crd[i]);
  ca.AssignEllp(NULL);
  E.SetRetVal(AtomCount() - 1);
}
//..............................................................................
void TAsymmUnit::LibGetAtomPart(const TStrObjList& Params, TMacroError& E)  {
  size_t index = Params[0].ToSizeT();
  if (index >= AtomCount())
    throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  E.SetRetVal(GetAtom(index).GetPart());
}
//..............................................................................
void TAsymmUnit::LibSetAtomPart(const TStrObjList& Params, TMacroError& E)  {
  size_t index = Params[0].ToSizeT();
  if (index >= AtomCount())
    throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  GetAtom(index).SetPart(Params[0].ToDouble());
}
//..............................................................................
void TAsymmUnit::LibGetZ(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal(Z);
}
//..............................................................................
void TAsymmUnit::LibSetZ(const TStrObjList& Params, TMacroError& E)  {
  if (Params[0].IsEmpty()) return;
  Z = Params[0].ToInt();
  if( Z <= 0 )  Z = 1;
}
//..............................................................................
void TAsymmUnit::LibGetZprime(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal(olxstr::FormatFloat(3,GetZPrime()));
}
//..............................................................................
void TAsymmUnit::LibSetZprime(const TStrObjList& Params, TMacroError& E)  {
  if (Params[0].IsEmpty()) return;
  double zp = Params[0].ToDouble();
  Z = (short)olx_round(
    TCLattice::GetLattMultiplier(Latt)*(MatrixCount()+1)*zp);
  if( Z <= 0 ) Z = 1;
}
//..............................................................................
void TAsymmUnit::LibFormula(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal(_SummFormula(' ', 1./olx_max(GetZPrime(), 0.01)));
}
//..............................................................................
void TAsymmUnit::LibWeight(const TStrObjList& Params, TMacroError& E)  {
  double m = (Params.Count() == 1 ? Params[0].ToDouble()
    : 1./olx_max(GetZPrime(), 0.01));
  E.SetRetVal(olxstr::FormatFloat(2, MolWeight()*m));
}
//..............................................................................
void TAsymmUnit::LibNPDCount(const TStrObjList& Params, TMacroError& E) {
  size_t cnt = 0;
  for (size_t i=0; i < CAtoms.Count(); i++) {
    if (CAtoms[i]->IsDeleted()) continue;
    if (CAtoms[i]->GetEllipsoid() != NULL &&
        CAtoms[i]->GetEllipsoid()->IsNPD())
    {
      cnt++;
    }
  }
  E.SetRetVal(cnt);
}
//..............................................................................
void TAsymmUnit::LibOrthogonolise(const TStrObjList& Params, TMacroError& E) {
  vec3d rv;
  if (Params.Count() == 3) {
    rv = vec3d(
      Params[0].ToDouble(), Params[1].ToDouble(), Params[2].ToDouble());
  }
  else {
    TStrList toks(olxstr(Params[0]).Replace(',', ' '), ' ');
    if (toks.Count() != 3) {
      E.ProcessingError(__OlxSrcInfo, "invalid number of tokens");
      return;
    }
    rv = vec3d(toks[0].ToDouble(), toks[1].ToDouble(), toks[2].ToDouble());
  }
  E.SetRetVal(Orthogonalise(rv).ToString());
}
//..............................................................................
void TAsymmUnit::LibFractionalise(const TStrObjList& Params, TMacroError& E) {
  vec3d rv;
  if (Params.Count() == 3) {
    rv = vec3d(
      Params[0].ToDouble(), Params[1].ToDouble(), Params[2].ToDouble());
  }
  else {
    TStrList toks(olxstr(Params[0]).Replace(',', ' '), ' ');
    if (toks.Count() != 3) {
      E.ProcessingError(__OlxSrcInfo, "invalid number of tokens");
      return;
    }
    rv = vec3d(toks[0].ToDouble(), toks[1].ToDouble(), toks[2].ToDouble());
  }
  E.SetRetVal(Fractionalise(rv).ToString());
}
//..............................................................................

TLibrary* TAsymmUnit::ExportLibrary(const olxstr& name)  {
  TLibrary* lib = new TLibrary(name.IsEmpty() ? olxstr("au") : name);
  lib->Register(new TFunction<TAsymmUnit>(this,
    &TAsymmUnit::LibNewAtom, "NewAtom", fpFour|fpFive,
    "Adds a new atom to the asymmetric unit and return its ID, by which it can"
    " be referred. The function takes the atom name and coordinates, the "
    "optional 5th parameter specifies if the position has to be tested for an "
    "existing atoms. If -1 is returned, the atom is not created"));
  lib->Register(new TFunction<TAsymmUnit>(this,
    &TAsymmUnit::LibGetAtomCount, "GetAtomCount", fpNone,
    "Returns the atom count in the asymmetric unit"));
  lib->Register(new TFunction<TAsymmUnit>(this,
    &TAsymmUnit::LibGetSymm, "GetCellSymm", fpNone|fpOne,
    "Returns space group of currently loaded file as name: 'C2', 'I41/amd', "
    "etc. Optionally, Hall symbol may be returned if 'hall' is provided as an"
    " argument"));
  lib->Register(new TFunction<TAsymmUnit>(this,
    &TAsymmUnit::LibGetAtomCrd, "GetAtomCrd", fpOne,
    "Returns a comma separated list of fractional coordinates for the "
    "specified atom"));
  lib->Register(new TFunction<TAsymmUnit>(this,
    &TAsymmUnit::LibGetAtomPart, "GetAtomPart", fpOne,
    "Returns part of the specified atom"));
  lib->Register(new TFunction<TAsymmUnit>(this,
    &TAsymmUnit::LibGetAtomName, "GetAtomName", fpOne,
    "Returns atom label"));
  lib->Register(new TFunction<TAsymmUnit>(this,
    &TAsymmUnit::LibGetAtomType, "GetAtomType", fpOne,
    "Returns atom type (element)"));
  lib->Register(new TFunction<TAsymmUnit>(this,
    &TAsymmUnit::LibGetAtomOccu, "GetAtomOccu", fpOne,
    "Returns atom occupancy"));
  lib->Register(new TFunction<TAsymmUnit>(this,
    &TAsymmUnit::LibGetAtomAfix, "GetAtomAfix", fpOne,
    "Returns atom AFIX"));
  lib->Register(new TFunction<TAsymmUnit>(this,
    &TAsymmUnit::LibGetPeak, "GetPeak", fpOne,
    "Returns peak intensity"));
  lib->Register(new TFunction<TAsymmUnit>(this,
    &TAsymmUnit::LibGetAtomU, "GetAtomU", fpOne,
    "Returns a single number or six, comma separated values"));
  lib->Register(new TFunction<TAsymmUnit>(this,
    &TAsymmUnit::LibGetAtomUiso, "GetAtomUiso", fpOne,
    "Returns a single number Uiso or (U11+U22+U33)/3"));
  lib->Register(new TFunction<TAsymmUnit>(this,
    &TAsymmUnit::LibNPDCount, "NPDCount", fpNone,
    "Returns number of the NPD atoms"));
  lib->Register(new TFunction<TAsymmUnit>(this,
    &TAsymmUnit::LibGetCell, "GetCell", fpNone|fpOne,
    "Returns six comma separated values for a, b, c and alpha, beta, gamma"));
  lib->Register(new TFunction<TAsymmUnit>(this,
    &TAsymmUnit::LibGetVolume, "GetVolume", fpNone,
    "Returns volume of the unit cell divided by the number of symmetry "
    "elements"));
  lib->Register(new TFunction<TAsymmUnit>(this,
    &TAsymmUnit::LibGetCellVolume, "GetCellVolume", fpNone,
    "Returns volume of the unit cell") );
  lib->Register(new TFunction<TAsymmUnit>(this,
    &TAsymmUnit::LibSetAtomCrd, "SetAtomCrd", fpFour,
    "Sets atom coordinates to specified values, first parameters is the atom "
    "ID"));
  lib->Register(new TFunction<TAsymmUnit>(this,
    &TAsymmUnit::LibSetAtomPart, "SetAtomPart", fpTwo,
    "Sets part of the atom specified atom"));
  lib->Register(new TFunction<TAsymmUnit>(this,
    &TAsymmUnit::LibSetAtomU, "SetAtomU", fpSeven | fpTwo,
    "Sets atoms Uiso/anis first paramater is the atom ID followed by 1 or six "
    "parameters"));
  lib->Register(new TFunction<TAsymmUnit>(this,
    &TAsymmUnit::LibSetAtomOccu, "SetAtomOccu", fpTwo,
    "Sets atom's occupancy; first parameter is the atom ID followed by "
    "occupancy"));
  lib->Register(new TFunction<TAsymmUnit>(this,
    &TAsymmUnit::LibSetAtomLabel, "SetAtomlabel", fpTwo,
    "Sets atom labels to provided value. The first parameter is the atom ID"));
  lib->Register(new TFunction<TAsymmUnit>(this,
    &TAsymmUnit::LibGetAtomLabel, "GetAtomlabel", fpTwo,
    "The takes two arguments - the atom ID and increment. The increment is "
    "used to navigate through the periodic table, so increment +1 will return "
    "next element and -1 the previous element in the periodic table"));
  lib->Register(new TFunction<TAsymmUnit>(this,
    &TAsymmUnit::LibIsAtomDeleted, "IsAtomDeleted", fpOne,
    "Checks status of specified atom"));
  lib->Register(new TFunction<TAsymmUnit>(this,
    &TAsymmUnit::LibIsPeak, "IsPeak", fpOne,
    "Checks if specified atom is  peak"));
  lib->Register(new TFunction<TAsymmUnit>(this,
    &TAsymmUnit::LibGetZ, "GetZ", fpNone,
    "Returns current Z"));
  lib->Register(new TFunction<TAsymmUnit>(this,
    &TAsymmUnit::LibSetZ, "SetZ", fpOne,
    "Sets current Z. Does not update content or whatsoever"));
  lib->Register(new TFunction<TAsymmUnit>(this,
    &TAsymmUnit::LibGetZprime, "GetZprime", fpNone,
    "Returns current Z divided byt the number of matrices of current "
    "spacegroup"));
  lib->Register(new TFunction<TAsymmUnit>(this,
    &TAsymmUnit::LibSetZprime, "SetZprime", fpOne,
    "Sets Z' for the structure"));
  lib->Register(new TFunction<TAsymmUnit>(this,
    &TAsymmUnit::LibFormula, "GetFormula", fpNone,
    "Returns chemical formula of the asymmetric unit"));
  lib->Register(new TFunction<TAsymmUnit>(this,
    &TAsymmUnit::LibWeight, "GetWeight", fpNone|fpOne,
    "Returns molecular mass of the asymmetric unit"));
  lib->Register(new TFunction<TAsymmUnit>(this,
    &TAsymmUnit::LibOrthogonolise, "Orthogonalise", fpOne|fpThree,
    "Returns orthogonalised coordinates"));
  lib->Register(new TFunction<TAsymmUnit>(this,
    &TAsymmUnit::LibFractionalise, "Fractionalise", fpOne|fpThree,
    "Returns fractional coordinates"));
  return lib;
}
//..............................................................................
