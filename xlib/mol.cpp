/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "mol.h"
#include "catom.h"
#include "unitcell.h"
#include "estrlist.h"
#include "exception.h"

TMol::TMol()  {  Clear();  }
//..............................................................................
TMol::~TMol()  {}
//..............................................................................
void TMol::Clear()  {
  GetAsymmUnit().Clear();
  Bonds.Clear();
  GetAsymmUnit().GetAxes() = vec3d(1,1,1);
  GetAsymmUnit().GetAngles() = vec3d(90,90,90);
  GetAsymmUnit().InitMatrices();
}
//..............................................................................
olxstr TMol::MOLAtom(TCAtom& A)  {
  olxstr Tmp;
  const vec3d& v = A.ccrd();
  for( int i=0; i < 3; i++ )
    Tmp << olxstr::FormatFloat(4, v[i]).LeftPadding(10, ' ');
  Tmp << ' ' << olxstr(A.GetType().symbol).RightPadding(3, ' ');
  for( int j=0; j < 12; j ++ )
    Tmp << "  0";
  return Tmp;
}
//..............................................................................
olxstr TMol::MOLBond(TMolBond& B)  {
  olxstr Tmp, Tmp1;
  Tmp1 = B.AtomA+1;
  Tmp1.LeftPadding(3, ' ');
  Tmp = Tmp1;
  Tmp1 = B.AtomB+1;
  Tmp1.LeftPadding(3, ' ');
  Tmp << Tmp1;
  Tmp1 = B.BondType;
  Tmp1.LeftPadding(3, ' '); // bond type single (1);
  Tmp << Tmp1;
  for( int j=0; j < 4; j ++ )
    Tmp << "  0";
  return Tmp;
}
//..............................................................................
void TMol::SaveToStrings(TStrList& Strings)  {
  olxstr Tmp, Tmp1;
  Strings.Add("-OLEX-");
  Strings.Add(EmptyString());
  Strings.Add(EmptyString());
  Tmp1 = GetAsymmUnit().AtomCount();
  Tmp1.LeftPadding(3, ' ');
  Tmp << Tmp1;
  Tmp1 = BondCount();
  Tmp1.LeftPadding(3, ' ');
  Tmp << Tmp1;
  Tmp << "  0  0  0  0  0  0  0  0  0 V2000";
  Strings.Add(Tmp);
  for( size_t i=0; i < GetAsymmUnit().AtomCount(); i++ )
    Strings.Add(MOLAtom(GetAsymmUnit().GetAtom(i)));
  for( size_t i=0; i < BondCount(); i++ )
    Strings.Add( MOLBond(Bond(i)) );
  Strings.Add("M END");
}
//..............................................................................
void TMol::LoadFromStrings(const TStrList& Strings)  {
  Clear();
  Title = "OLEX: imported from MDL MOL";
  bool AtomsCycle = false, BondsCycle = false;
  int AC=0, BC=0;
  for( size_t i=0; i < Strings.Count(); i++ )  {
    olxstr line = Strings[i].UpperCase();
    if( line.IsEmpty() )  continue;
    if( AtomsCycle && (line.Length() > 33) )  {
      vec3d crd(line.SubString(0, 9).ToDouble(), line.SubString(10, 10).ToDouble(), line.SubString(20, 10).ToDouble());
      olxstr atom_name = line.SubString(31, 3).Trim(' ');
      if( XElementLib::IsAtom(atom_name) )  {
        TCAtom& CA = GetAsymmUnit().NewAtom();
        CA.ccrd() = crd;
        CA.SetLabel( (atom_name << GetAsymmUnit().AtomCount()+1) );
      }
      AC--;
      if( AC <= 0 )  {
        BondsCycle = true;
        AtomsCycle = false;
      }
      continue;
    }
    if( BondsCycle && line.Length() >= 9)  {
      const size_t ai1  =  line.SubString(0, 3).ToSizeT()-1;
      const size_t ai2  =  line.SubString(3, 3).ToSizeT()-1;
      if( ai1 >= GetAsymmUnit().AtomCount() || ai2 >= GetAsymmUnit().AtomCount())  {
        throw TFunctionFailedException(__OlxSourceInfo, olxstr("TMol:: wrong atom indexes: ") << ai1 << ' ' << ai2);
      }
      TMolBond& MB = Bonds.Add(new TMolBond);
      MB.AtomA = ai1;
      MB.AtomB = ai2;
      MB.BondType = line.SubString(6, 3).ToInt();   // bond type
      BC--;
      if( BC <= 0 )
        BondsCycle = false;
      continue;
    }

    if( (line.FirstIndexOf("V2000") != InvalidIndex) || (line.FirstIndexOf("V3000") != InvalidIndex) ) {  // count line
      AC = line.SubString(0, 3).ToInt();
      BC = line.SubString(3, 3).ToInt();
      AtomsCycle = true;
      continue;
    }
  }
}

//..............................................................................
bool TMol::Adopt(TXFile& XF, int) {
  Clear();
  const ASObjectProvider& objects = XF.GetLattice().GetObjects();
  size_t id = 0;
  for (size_t i=0; i < objects.atoms.Count(); i++) {
    TSAtom& sa = objects.atoms[i];
    if( !sa.IsAvailable() )  continue;
    TCAtom& a = GetAsymmUnit().NewAtom();
    a.SetLabel(sa.GetLabel(), false);
    a.ccrd() = sa.crd();
    a.SetType(sa.GetType());
    sa.SetTag(id++);
  }
  for (size_t i=0; i < objects.bonds.Count(); i++) {
    TSBond& sb = objects.bonds[i];
    if( !sb.IsAvailable() )  continue;
    TMolBond& mb = Bonds.AddNew();
    mb.AtomA = sb.A().GetTag();
    mb.AtomB = sb.B().GetTag();
    mb.BondType = 1; // singlel bond, a proper encoding is required...
  }
  GetAsymmUnit().SetZ((short)XF.GetLattice().GetUnitCell().MatrixCount());
  return true;
}
//..............................................................................
