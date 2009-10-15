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
TMol::TMol()  {   }
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
  Title = "OLEX: imported from MDL MOL";
  TAtomsInfo& AtomsInfo = TAtomsInfo::GetInstance();
  GetAsymmUnit().Axes()[0] = 1;
  GetAsymmUnit().Axes()[1] = 1;
  GetAsymmUnit().Axes()[2] = 1;
  GetAsymmUnit().Angles()[0] = 90;
  GetAsymmUnit().Angles()[1] = 90;
  GetAsymmUnit().Angles()[2] = 90;
  GetAsymmUnit().InitMatrices();
  bool AtomsCycle = false, BondsCycle = false;
  int AC=0, BC=0;
  for( int i=0; i < Strings.Count(); i++ )  {
    olxstr line = Strings[i].UpperCase();
    if( line.IsEmpty() )  continue;
    if( AtomsCycle && (line.Length() > 33) )  {
      vec3d crd(line.SubString(0, 9).ToDouble(), line.SubString(10, 10).ToDouble(), line.SubString(20, 10).ToDouble());
      olxstr atom_name = line.SubString(31, 3).Trim(' ');
      if( AtomsInfo.IsAtom(atom_name) )  {
        TCAtom& CA = GetAsymmUnit().NewAtom();
        CA.ccrd() = crd;
        CA.SetLabel( (atom_name + GetAsymmUnit().AtomCount()+1) );
      }
      AC--;
      if( AC <= 0 )  {
        BondsCycle = true;
        AtomsCycle = false;
      }
      continue;
    }
    if( BondsCycle && line.Length() >= 9)  {
      const int ai1  =  line.SubString(0, 3).ToInt()-1;
      const int ai2  =  line.SubString(3, 3).ToInt()-1;
      if( (ai1 >= GetAsymmUnit().AtomCount() || ai2 >= GetAsymmUnit().AtomCount()) ||
          ai1 < 0 || ai2 < 0 )  {
        throw TFunctionFailedException(__OlxSourceInfo, olxstr("TMol:: wrong atom indexes: ") << ai1 << ' ' << ai2);
      }
      TMolBond* MB = new TMolBond;
      MB->AtomA = ai1;
      MB->AtomB = ai2;
      MB->BondType = line.SubString(6, 3).ToInt();   // bond type
      Bonds.Add(*MB);
      BC--;
      if( BC <= 0 )
        BondsCycle = false;
      continue;
    }
    
    if( (line.FirstIndexOf("V2000") != -1) || (line.FirstIndexOf("V3000") != -1) ) {  // count line
      AC = line.SubString(0, 3).ToInt();
      BC = line.SubString(3, 3).ToInt();
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

 
