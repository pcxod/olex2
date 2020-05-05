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
#include "olxvar.h"

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
olxstr TMol::MOLAtom(TCAtom& A) {
  olxstr_buf tmp;
  vec3d v = GetAsymmUnit().Orthogonalise(A.ccrd());
  int op = TOlxVars::FindValue("file_output_precision", "4").ToInt();
  for (int i = 0; i < 3; i++) {
    tmp << olxstr::FormatFloat(op, v[i]).LeftPadding(10, ' ', true);
  }
  tmp << ' ' << olxstr(A.GetType().symbol).RightPadding(3, ' ');
  for (int j = 0; j < 12; j++) {
    tmp << padding();
  }
  return tmp;
}
//..............................................................................
olxstr TMol::MOLBond(TMolBond& B) {
  olxstr_buf tmp;
  tmp << olxstr(B.AtomA+1).LeftPadding(3, ' ')
    << olxstr(B.AtomB+1).LeftPadding(3, ' ')
    << olxstr(B.BondType).LeftPadding(3, ' ');
  for (int j = 0; j < 4; j++) {
    tmp << padding();
  }
  return tmp;
}
//..............................................................................
void TMol::SaveToStrings(TStrList& Strings) {
  Strings.Add("-OLEX2-");
  Strings.Add();
  Strings.Add();
  olxstr &cl = Strings.Add();
  cl << olxstr(GetAsymmUnit().AtomCount()).LeftPadding(3, ' ')
    << olxstr(BondCount()).LeftPadding(3, ' ')
    << "  0  0  0  0  0  0  0  0  0 V2000";
  for (size_t i = 0; i < GetAsymmUnit().AtomCount(); i++) {
    Strings.Add(MOLAtom(GetAsymmUnit().GetAtom(i)));
  }
  for (size_t i = 0; i < BondCount(); i++) {
    Strings.Add(MOLBond(Bond(i)));
  }
  Strings.Add("M END");
}
//..............................................................................
void TMol::LoadFromStrings(const TStrList& Strings) {
  Clear();
  Title = "OLEX2: imported from MDL MOL";
  bool AtomsCycle = false, BondsCycle = false;
  int AC=0, BC=0;
  for (size_t i=0; i < Strings.Count(); i++) {
    olxstr line = Strings[i].UpperCase();
    if (line.IsEmpty())  continue;
    if (AtomsCycle && (line.Length() > 33)) {
      vec3d crd(line.SubString(0, 9).ToDouble(),
        line.SubString(10, 10).ToDouble(), line.SubString(20, 10).ToDouble());
      olxstr atom_name = line.SubString(31, 3).Trim(' ');
      if( XElementLib::IsAtom(atom_name) )  {
        TCAtom& CA = GetAsymmUnit().NewAtom();
        CA.ccrd() = crd;
        CA.SetLabel( (atom_name << GetAsymmUnit().AtomCount()+1) );
      }
      if (--AC <= 0) {
        BondsCycle = true;
        AtomsCycle = false;
      }
      continue;
    }
    if (BondsCycle && line.Length() >= 9) {
      const size_t ai1 = line.SubString(0, 3).ToSizeT()-1;
      const size_t ai2 = line.SubString(3, 3).ToSizeT()-1;
      if (ai1 >= GetAsymmUnit().AtomCount() || ai2 >= GetAsymmUnit().AtomCount()) {
        throw TFunctionFailedException(__OlxSourceInfo,
          olxstr("TMol:: wrong atom indices: ") << ai1 << ' ' << ai2);
      }
      TMolBond& MB = Bonds.Add(new TMolBond);
      MB.AtomA = ai1;
      MB.AtomB = ai2;
      MB.BondType = line.SubString(6, 3).ToInt();   // bond type
      if (--BC <= 0) {
        BondsCycle = false;
      }
      continue;
    }

    if ((line.FirstIndexOf("V2000") != InvalidIndex) || // count line
      (line.FirstIndexOf("V3000") != InvalidIndex))
    {
      AC = line.SubString(0, 3).ToInt();
      BC = line.SubString(3, 3).ToInt();
      AtomsCycle = true;
      continue;
    }
  }
  GenerateCellForCartesianFormat();
}

//..............................................................................
bool TMol::Adopt(TXFile& XF, int flags) {
  Clear();
  const ASObjectProvider& objects = XF.GetLattice().GetObjects();
  size_t id = 0;
  for (size_t i=0; i < objects.atoms.Count(); i++) {
    TSAtom& sa = objects.atoms[i];
    if (!sa.IsAvailable() || (flags == 0 && !sa.IsAUAtom())) {
      sa.SetTag(-1);
      continue;
    }
    TCAtom& a = GetAsymmUnit().NewAtom();
    a.SetLabel(sa.GetLabel(), false);
    a.ccrd() = sa.crd();
    a.SetType(sa.GetType());
    sa.SetTag(id++);
  }
  for (size_t i=0; i < objects.bonds.Count(); i++) {
    TSBond& sb = objects.bonds[i];
    if (!sb.IsAvailable() || sb.A().GetTag() < 0 || sb.B().GetTag() < 0) {
      continue;
    }
    TMolBond& mb = Bonds.AddNew();
    mb.AtomA = sb.A().GetTag();
    mb.AtomB = sb.B().GetTag();
    mb.BondType = TSBond::PercieveOrder(sb.A().GetType(), sb.B().GetType(),
      sb.Length());
    if (mb.BondType == 0) {
      mb.BondType = 1;
    }
  }
  GetAsymmUnit().SetZ((double)XF.GetLattice().GetUnitCell().MatrixCount());
  return true;
}
//..............................................................................
