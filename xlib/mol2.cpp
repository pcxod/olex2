/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "mol2.h"
#include "catom.h"
#include "unitcell.h"
#include "estrlist.h"
#include "exception.h"

// TMol2 - basic procedures for loading Tripos MOL2 files
//..............................................................................
void TMol2::Clear() {
  GetAsymmUnit().Clear();
  Bonds.Clear();
  GetAsymmUnit().GetAxes() = vec3d(1,1,1);
  GetAsymmUnit().GetAngles() = vec3d(90,90,90);
  GetAsymmUnit().InitMatrices();
}
//..............................................................................
olxstr TMol2::MOLAtom(TCAtom& A) {
  olxstr rv(A.GetId(), 64);
  vec3d c = GetAsymmUnit().Orthogonalise(A.ccrd());
  rv << '\t' << A.GetLabel()
     << '\t' << c[0] << '\t' << c[1] << '\t' << c[2]
     << '\t' << A.GetType().symbol
     << '\t' << (A.GetFragmentId()+1)
     << '\t' << "FRAG_" << (A.GetFragmentId()+1)
     << "\t0";
  return rv;
}
//..............................................................................
const olxstr& TMol2::EncodeBondType(size_t type) const {
  if (BondNames().Count() < type)
    throw TInvalidArgumentException(__OlxSourceInfo, "bond type");
  return BondNames()[type];
}
//..............................................................................
size_t TMol2::DecodeBondType(const olxstr& name) const {
  for (int i = 0; i < 8; i++) {
    if (BondNames()[i].Equalsi(name))
      return i;
  }
  return mol2btUnknown;
}
//..............................................................................
olxstr TMol2::MOLBond(TMol2Bond& B) {
  olxstr rv(B.GetId(), 32);
  rv.stream('\t') << B.a1 << B.a2 << EncodeBondType(B.BondType);
  return rv;
}
//..............................................................................
void TMol2::SaveToStrings(TStrList& Strings) {
  const TAsymmUnit &au = GetAsymmUnit();
  Strings.Add("@<TRIPOS>MOLECULE");
  Strings.Add(GetTitle());
  Strings.Add(au.AtomCount())  << '\t' << Bonds.Count() << "\t1";
  Strings.Add("SMALL");
  Strings.Add("NO_CHARGES");
  Strings.Add("= OLEX2 =");
  Strings.Add("@<TRIPOS>ATOM");
  //olxdict<uint32_t, size_t, TPrimitiveComparator> frag_atoms;
  sorted::PrimitiveAssociation<uint32_t, size_t> frag_atoms;
  for (size_t i = 0; i < au.AtomCount(); i++) {
    TCAtom &a = au.GetAtom(i);
    Strings.Add(MOLAtom(a));
    size_t idx = frag_atoms.IndexOf(a.GetFragmentId());
    if (idx == InvalidIndex) {
      frag_atoms.Add(a.GetFragmentId(), a.GetId());
    }
  }
  if (!Bonds.IsEmpty()) {
    Strings.Add(EmptyString());
    Strings.Add("@<TRIPOS>BOND");
    for (size_t i=0; i < Bonds.Count(); i++)
      Strings.Add(MOLBond(Bonds[i]));
  }
  Strings.Add("@<TRIPOS>SUBSTRUCTURE");
  for (uint32_t i = 0; i < frag_atoms.Count(); i++) {
    Strings.Add(i+1) << '\t' << "FRAG_" << (frag_atoms.GetKey(i)+1)
      << '\t' << frag_atoms.GetValue(i)
      << "\tGROUP\t0\t****\t****\t0";
  }
}
//..............................................................................
void TMol2::LoadFromStrings(const TStrList& Strings) {
  Clear();
  Title = "OLEX2: imported from TRIPOS MOL2";
  bool AtomsCycle = false, BondsCycle = false;
  for (size_t i=0; i < Strings.Count(); i++) {
    olxstr line = Strings[i].UpperCase().Replace('\t', ' ').Trim(' ');
    if (line.IsEmpty())  continue;
    if (AtomsCycle) {
      if (line.Compare("@<TRIPOS>BOND") == 0) {
        BondsCycle = true;
        AtomsCycle = false;
        continue;
      }
      TStrList toks(line, ' ');
      if (toks.Count() < 6)  continue;
      vec3d crd(toks[2].ToDouble(), toks[3].ToDouble(), toks[4].ToDouble());
      TStrList ent(toks[5], '.');
      cm_Element *elm = XElementLib::FindBySymbol(ent[0]);
      if (elm != NULL) {
        TCAtom& CA = GetAsymmUnit().NewAtom();
        CA.ccrd() = crd;
        CA.SetLabel(toks[1], false);
        CA.SetType(*elm);
      }
      continue;
    }
    if (BondsCycle) {
      if (line.CharAt(0) == '@') {
        BondsCycle = false;
        break;
      }
      TStrList toks(line, ' ');
      if (toks.Count() < 4)
        continue;
      TMol2Bond& MB = Bonds.Add(new TMol2Bond(Bonds.Count()));
      MB.a1 = toks[1].ToSizeT();
      MB.a2 = toks[2].ToSizeT();
      MB.BondType = DecodeBondType(toks[3]);  // bond type
      continue;
    }
    if (line.Equals("@<TRIPOS>ATOM")) {
      AtomsCycle = true;
      continue;
    }
  }
  GenerateCellForCartesianFormat();
}

//..............................................................................
bool TMol2::Adopt(TXFile &XF, int) {
  Clear();
  const ASObjectProvider& objects = XF.GetLattice().GetObjects();
  size_t id = 0;
  for (size_t i=0; i < objects.atoms.Count(); i++) {
    TSAtom& sa = objects.atoms[i];
    if (!sa.IsAvailable()) continue;
    TCAtom& a = GetAsymmUnit().NewAtom();
    a.SetLabel(sa.GetLabel(), false);
    a.ccrd() = sa.crd();
    a.SetType(sa.GetType());
    a.SetFragmentId(sa.CAtom().GetFragmentId());
    sa.SetTag(id++);
  }
  for (size_t i=0; i < objects.bonds.Count(); i++) {
    TSBond& sb = objects.bonds[i];
    if (!sb.IsAvailable()) continue;
    TMol2Bond& mb = Bonds.AddNew(Bonds.Count());
    mb.a1 = sb.A().GetTag();
    mb.a2 = sb.B().GetTag();
    mb.BondType = TSBond::PercieveOrder(sb.A().GetType(), sb.B().GetType(),
      sb.Length());
    if (mb.BondType == 0) mb.BondType = 1;
  }
  GetAsymmUnit().SetZ((double)XF.GetLattice().GetUnitCell().MatrixCount());
  return true;
}
//..............................................................................
