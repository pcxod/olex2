//---------------------------------------------------------------------------//
// namespace TXFiles: TMol - basic procedures for loading MDL MOL files
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "mol.h"

#include "catom.h"
#include "unitcell.h"
#include "estrlist.h"
#include "exception.h"

//----------------------------------------------------------------------------//
// TMol function bodies
//----------------------------------------------------------------------------//
TMol::TMol(TAtomsInfo *S):TBasicCFile(S)  {   }
//..............................................................................
TMol::~TMol()  {  Clear();    }
//..............................................................................
void TMol::Clear()  {
  GetAsymmUnit().Clear();
  Bonds.Clear();
}
//..............................................................................
olxstr TMol::MOLAtom(TCAtom& A)  {
  olxstr Tmp, Tmp1;
  vec3d V = A.ccrd();
  GetAsymmUnit().CellToCartesian(V);
  Tmp1 = olxstr::FormatFloat(4, V[0] );
  Tmp1.Format(10, false, ' ');
  Tmp = Tmp1;
  Tmp1 = olxstr::FormatFloat(4, V[1] );
  Tmp1.Format(10, false, ' ');
  Tmp << Tmp1;
  Tmp1 = olxstr::FormatFloat(4, V[2] );
  Tmp1.Format(10, false, ' ');
  Tmp << Tmp1;
  Tmp1 = " ";    Tmp1 << A.GetAtomInfo().GetSymbol();
  Tmp1.Format(3, true, ' ');
  Tmp << Tmp1;
  for( int j=0; j < 12; j ++ )
    Tmp << "  0";
  return Tmp;
}
//..............................................................................
olxstr TMol::MOLBond(TMolBond& B)  {
  olxstr Tmp, Tmp1;
  Tmp1 = B.AtomA+1;
  Tmp1.Format(3, false, ' ');
  Tmp = Tmp1;
  Tmp1 = B.AtomB+1;
  Tmp1.Format(3, false, ' ');
  Tmp << Tmp1;
  Tmp1 = B.BondType;
  Tmp1.Format(3, false, ' '); // bond type single (1);
  Tmp << Tmp1;
  for( int j=0; j < 4; j ++ )
    Tmp << "  0";
  return Tmp;
}
//..............................................................................
void TMol::SaveToStrings(TStrList& Strings)  {
  olxstr Tmp, Tmp1;
  Strings.Add("-OLEX-");
  Strings.Add(EmptyString);
  Strings.Add(EmptyString);
  Tmp1 = GetAsymmUnit().AtomCount();
  Tmp1.Format(3, false, ' ');
  Tmp << Tmp1;
  Tmp1 = BondCount();
  Tmp1.Format(3, false, ' ');
  Tmp << Tmp1;
  Tmp << "  0  0  0  0  0  0  0  0  0 V2000";
  Strings.Add(Tmp);
  for( int i=0; i < GetAsymmUnit().AtomCount(); i++ )
    Strings.Add( MOLAtom(GetAsymmUnit().GetAtom(i)));
  for( int i=0; i < BondCount(); i++ )
    Strings.Add( MOLBond(Bond(i)) );
  Strings.Add("M END");
}
//..............................................................................
void TMol::LoadFromStrings(const TStrList& Strings)  {
  Clear();

  olxstr Tmp1, Tmp, Msg;
  vec3d StrCenter;
  FTitle = "OLEX: imported from MDL MOL";

  GetAsymmUnit().Axes()[0] = 1;
  GetAsymmUnit().Axes()[1] = 1;
  GetAsymmUnit().Axes()[2] = 1;
  GetAsymmUnit().Angles()[0] = 90;
  GetAsymmUnit().Angles()[1] = 90;
  GetAsymmUnit().Angles()[2] = 90;
  GetAsymmUnit().InitMatrices();
  bool AtomsCycle = false, BondsCycle = false;
  int AC=0, BC=0, ai1, ai2;
  TMolBond *MB;
  double Ax, Ay, Az;
  for( int i=0; i < Strings.Count(); i++ )  {
    Tmp = Strings.String(i).UpperCase();
    if( !Tmp.Length() )  continue;
    if( AtomsCycle && (Tmp.Length() > 33) )  {
      Ax = Tmp.SubString(0, 9).ToDouble();
      Ay = Tmp.SubString(10, 10).ToDouble();
      Az = Tmp.SubString(20, 10).ToDouble();
      Tmp1 = Tmp.SubString(31, 3).Trim(' ');
      if( AtomsInfo->IsAtom(Tmp1) )  {
        TCAtom& CA = GetAsymmUnit().NewAtom();
        CA.ccrd()[0] = Ax;
        CA.ccrd()[1] = Ay;
        CA.ccrd()[2] = Az;
        CA.SetLabel( (Tmp1 + GetAsymmUnit().AtomCount()+1) );
        CA.SetLoaderId(GetAsymmUnit().AtomCount()-1);
      }
      AC--;
      if( AC <= 0 )  {
        BondsCycle = true;
        AtomsCycle = false;
      }
      continue;
    }
    if( BondsCycle && Tmp.Length() >= 9)  {
      ai1  =  Tmp.SubString(0, 3).ToInt()-1;
      ai2  =  Tmp.SubString(3, 3).ToInt()-1;
      if( (ai1 >= GetAsymmUnit().AtomCount() || ai2 >= GetAsymmUnit().AtomCount()) ||
          ai1 < 0 || ai2 < 0 )  {
        throw TFunctionFailedException(__OlxSourceInfo, olxstr("TMol:: wrong atom indexes: ") << ai1 << ' ' << ai2);
      }
      MB = new TMolBond;
      MB->AtomA = ai1;
      MB->AtomB = ai2;
      MB->BondType = Tmp.SubString(6, 3).ToInt();   // bond type
      Bonds.Add(*MB);
      BC--;
      if( BC <= 0 )
        BondsCycle = false;
      continue;
    }
    
    if( (Tmp.FirstIndexOf("V2000") != -1) || (Tmp.FirstIndexOf("V3000") != -1) ) // count line
    {
      AC = Tmp.SubString(0, 3).ToInt();
      BC = Tmp.SubString(3, 3).ToInt();
      AtomsCycle = true;
      continue;
    }
  }
}

//..............................................................................
bool TMol::Adopt(TXFile *XF)  {
  Clear();
  GetAsymmUnit().Assign(XF->GetAsymmUnit());
  GetAsymmUnit().SetZ( (short)XF->GetLattice().GetUnitCell().MatrixCount() );
  return true;
}
//..............................................................................
void TMol::DeleteAtom(TCAtom *CA)  {  return;  }
//..............................................................................

 
