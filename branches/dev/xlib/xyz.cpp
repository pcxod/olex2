//---------------------------------------------------------------------------//
// TXyz - basic class for loading XYZ files
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//
#include "xyz.h"

#include "catom.h"
#include "unitcell.h"

TXyz::TXyz() { 
  TAsymmUnit& au = GetAsymmUnit();
  au.Axes()[0].V() = 1;
  au.Axes()[1].V() = 1;
  au.Axes()[2].V() = 1;
  au.Angles()[0].V() = 90;
  au.Angles()[1].V() = 90;
  au.Angles()[2].V() = 90;
  au.InitMatrices();
}
//..............................................................................
TXyz::~TXyz() {  Clear();  }
//..............................................................................
void TXyz::Clear()  {
  GetAsymmUnit().Clear();
}
//..............................................................................
void TXyz::SaveToStrings(TStrList& Strings)  {
  for( size_t i=0; i < GetAsymmUnit().AtomCount(); i++ )  {
    TCAtom& CA = GetAsymmUnit().GetAtom(i);
    if( CA.IsDeleted() )  continue;
    olxstr Tmp = CA.GetAtomInfo().GetSymbol();
    Tmp << ' ';
    const vec3d& v = CA.ccrd();
    for( int j=0; j < 3; j++ )
      Tmp << olxstr::FormatFloat(4, v[j]).Format(10, false, ' ');
    Strings.Add(Tmp);
  }
}
//..............................................................................
void TXyz::LoadFromStrings(const TStrList& Strings)  {
  Clear();
  TAtomsInfo& AtomsInfo = TAtomsInfo::GetInstance();
  Title = "OLEX2: imported from XYZ";
  GetAsymmUnit().Axes()[0] = 1;
  GetAsymmUnit().Axes()[1] = 1;
  GetAsymmUnit().Axes()[1] = 1;
  GetAsymmUnit().Angles()[0] = 90;
  GetAsymmUnit().Angles()[1] = 90;
  GetAsymmUnit().Angles()[2] = 90;
  GetAsymmUnit().InitMatrices();
  for( size_t i=0; i < Strings.Count(); i++ )  {
    olxstr line = Strings[i];
    if( line.IsEmpty() )  continue;
    TStrList toks(line, ' ');
    if( toks.Count() != 4 )  continue;
    if( AtomsInfo.IsAtom(toks[0]) )  {
      TCAtom& CA = GetAsymmUnit().NewAtom();
      CA.ccrd()[0] = toks[1].ToDouble();
      CA.ccrd()[1] = toks[2].ToDouble();
      CA.ccrd()[2] = toks[3].ToDouble();
      CA.SetLabel(toks[0] << (GetAsymmUnit().AtomCount()+1));
    }
  }
}

//..............................................................................
bool TXyz::Adopt(TXFile& XF)  {
  Clear();
  Title = XF.LastLoader()->GetTitle();
  GetRM().SetHKLSource(XF.LastLoader()->GetRM().GetHKLSource());
  TLattice& latt = XF.GetLattice();
  for( size_t i=0; i < latt.AtomCount(); i++ )  {
    TSAtom& sa = latt.GetAtom(i);
    if( !sa.IsAvailable() )  continue;
    TCAtom& a = GetAsymmUnit().NewAtom();
    a.Label() = sa.GetLabel();
    a.ccrd() = sa.crd();
    a.SetAtomInfo(sa.GetAtomInfo());
  }
  GetAsymmUnit().SetZ((short)XF.GetLattice().GetUnitCell().MatrixCount());
  return true;
}
//..............................................................................


 
