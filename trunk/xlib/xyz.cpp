//---------------------------------------------------------------------------//
// namespace TXFiles: TXyz - basic procedures for loading XYZ files
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "xyz.h"

#include "catom.h"
#include "unitcell.h"
#include "estrlist.h"

//----------------------------------------------------------------------------//
// TMol function bodies
//----------------------------------------------------------------------------//
TXyz::TXyz() { }
//..............................................................................
TXyz::~TXyz() {  Clear();  }
//..............................................................................
void TXyz::Clear()  {
  GetAsymmUnit().Clear();
}
//..............................................................................
void TXyz::SaveToStrings(TStrList& Strings)  {
  olxstr Tmp, Tmp1;
  vec3d V;
  for( int i=0; i < GetAsymmUnit().AtomCount(); i++ )  {
    TCAtom& CA = GetAsymmUnit().GetAtom(i);
    if( CA.IsDeleted() )  continue;
    V = CA.ccrd();
    GetAsymmUnit().CellToCartesian(V);
    Tmp = CA.GetAtomInfo().GetSymbol();
    Tmp << ' ';
    Tmp1 = olxstr::FormatFloat(4, V[0] );
    Tmp <<  Tmp1.Format(10, false, ' ');
    Tmp1 = olxstr::FormatFloat(4, V[1] );
    Tmp << Tmp1.Format(10, false, ' ');
    Tmp1 = olxstr::FormatFloat(4, V[2] );
    Tmp << Tmp1.Format(10, false, ' ');
    Strings.Add(Tmp);
  }
}
//..............................................................................
void TXyz::LoadFromStrings(const TStrList& Strings)  {
  Clear();
  TAtomsInfo& AtomsInfo = TAtomsInfo::GetInstance();
  olxstr Tmp;
  vec3d StrCenter;
  Title = "OLEX: imported from XYZ";

  GetAsymmUnit().Axes()[0] = 1;
  GetAsymmUnit().Axes()[1] = 1;
  GetAsymmUnit().Axes()[1] = 1;
  GetAsymmUnit().Angles()[0] = 90;
  GetAsymmUnit().Angles()[1] = 90;
  GetAsymmUnit().Angles()[2] = 90;
  GetAsymmUnit().InitMatrices();
  double Ax, Ay, Az;
  TStrList toks;
  for( int i=0; i < Strings.Count(); i++ )  {
    Tmp = Strings.String(i).UpperCase();
    if( Tmp.IsEmpty() )  continue;
    toks.Clear();
    toks.Strtok(Tmp, ' ');
    if( toks.Count() != 4 )  continue;
    Ax = toks.String(1).ToDouble();
    Ay = toks.String(2).ToDouble();
    Az = toks.String(3).ToDouble();
    if( AtomsInfo.IsAtom(toks[0]) )  {
      TCAtom& CA = GetAsymmUnit().NewAtom();
      CA.ccrd()[0] = Ax;
      CA.ccrd()[1] = Ay;
      CA.ccrd()[2] = Az;
      CA.SetLabel( (toks[0] + GetAsymmUnit().AtomCount()+1) );
      CA.SetLoaderId(GetAsymmUnit().AtomCount()-1);
    }
  }
}

//..............................................................................
bool TXyz::Adopt(TXFile *XF)  {
  Clear();
  if( XF->HasLastLoader() )  {
    Title = XF->LastLoader()->GetTitle();
    GetRM().SetHKLSource( XF->LastLoader()->GetRM().GetHKLSource() );
  }
  else  {
    GetRM().SetHKLSource(EmptyString);
    Title = "?";
  }
  GetAsymmUnit().Assign(XF->GetAsymmUnit());
  GetAsymmUnit().SetZ( (short)XF->GetLattice().GetUnitCell().MatrixCount() );
  return true;
}
//..............................................................................
void TXyz::DeleteAtom(TCAtom *CA)  {  return;  }
//..............................................................................


 
