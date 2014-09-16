/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "ciftab.h"
#include "cif.h"

// TLLTBondSort function bodies - bond sorting procedure in TLinkedLoopTable
int TLLTBondSort::Compare_(const CifTabBond &I, const CifTabBond &I1) const {
  double v;
  if( (SortType & cCifTabSortLength) != 0 )  {  // length, Mr, Label
    v = I.Value.ToDouble() - I1.Value.ToDouble();
    if( v < 0 ) return -1;
    if( v > 0 ) return 1;
    v = I.Another(Atom).CA.GetType().GetMr() - I1.Another(Atom).CA.GetType().GetMr();
    if( v > 0 ) return 1;
    if( v < 0 ) return -1;
    v = I.Another(Atom).Label.Compare(I1.Another(Atom).Label);
    if( v == 0 )
      return olx_cmp(Symmetry.IndexOf(I.S2), Symmetry.IndexOf(I1.S2));
  }
  if( (SortType & cCifTabSortName) != 0 )  {  // Name, length
    v = I.Another(Atom).Label.Compare(I1.Another(Atom).Label);
    if( v < 0 ) return -1;
    if( v > 0 ) return 1;
    if( v == 0 )  {
      v = (int)(Symmetry.IndexOf(I.S2) - Symmetry.IndexOf(I1.S2));
      if( v < 0 ) return -1;
      if( v > 0 ) return 1;
    }
    v = I.Value.ToDouble() - I1.Value.ToDouble();
    if( v < 0 ) return -1;
    if( v > 0 ) return 1;
    return 0;
  }
  if( (SortType & cCifTabSortMw) != 0 )  {  // Mr, Length, Label
    v = I.Another(Atom).CA.GetType().GetMr() - I1.Another(Atom).CA.GetType().GetMr();
    if( v < 0 ) return -1;
    if( v > 0 ) return 1;
    v = I.Value.ToDouble() - I1.Value.ToDouble();
    if( v > 0 ) return 1;
    if( v < 0 ) return -1;
    if( v == 0 )
      return olx_cmp(Symmetry.IndexOf(I.S2), Symmetry.IndexOf(I1.S2));
  }
  return 0;
}
//----------------------------------------------------------------------------//
// CifTabBond function bodies - bond objsect for TLinkedLoopTable
//----------------------------------------------------------------------------//
const CifTabAtom& CifTabBond::Another(CifTabAtom& A) const {
  if(&A == &A1)  return A2;
  if(&A == &A2)  return A1;
  throw TInvalidArgumentException(__OlxSourceInfo, "atom");
}
//..............................................................................
bool CifTabBond::operator == (const CifTabBond &B) const {
  if( A1 == B.A1 && A2 == B.A2 && S2 == B.S2 )
    return true;
  return false;
}
//----------------------------------------------------------------------------//
// CifTabAngle function bodies - angle objsect for TLinkedLoopTable
//----------------------------------------------------------------------------//
bool CifTabAngle::Contains(const CifTabAtom& A) const {
  if( A1==A || A2==A || A3 == A ) return true;
  return false;
}
//..............................................................................
bool CifTabAngle::FormedBy(const CifTabBond& B, const CifTabBond& B1) const {
  CifTabAtom *a1=NULL, *a3=NULL;
  olxstr s1, s3;
  if( B.A1 == A2 )  {  a1 = &B.A2;  s1 = B.S2;  }
  if( B.A2 == A2 )  {  a1 = &B.A1;  s1 = ".";  }
  if( B1.A1 == A2 )  {  a3 = &B1.A2;  s3 = B1.S2;  }
  if( B1.A2 == A2 )  {  a3 = &B1.A1;  s3 = ".";  }
  if( a1 == &A1 && a3 == &A3 )  {
//    if( LA.S1 == S1 && LA.S3 == S3 )
      return true;
  }
  if( a1 == &A3 && a3 == &A1 )  {
//    if( LA.S1 == S3 && LA.S3 == S1 )
      return true;
  }
  return false;
}
//----------------------------------------------------------------------------//
// TLinkedLoopTable function bodies
//----------------------------------------------------------------------------//
TLinkedLoopTable::TLinkedLoopTable(const TCif& C) : FCif(C)  {
  for( size_t j=0; j < C.GetAsymmUnit().AtomCount(); j++ )  {
    TCAtom& CA = C.GetAsymmUnit().GetAtom(j);
    CifTabAtom* LA = new CifTabAtom(CA);
    FAtoms.Add(LA);
    LA->Label = CA.GetLabel();
  }
  cif_dp::cetTable *CL = C.FindLoop("_geom_bond");
  if( CL == NULL ) return;
  const cif_dp::CifTable& BondTab = CL->GetData();
  size_t index =  BondTab.ColIndex("_geom_bond_atom_site_label_1");
  size_t index1 = BondTab.ColIndex("_geom_bond_atom_site_label_2");
  size_t index2 = BondTab.ColIndex("_geom_bond_distance");
  size_t index3 = BondTab.ColIndex("_geom_bond_site_symmetry_2");
  if( (index|index1|index2|index3) == InvalidIndex )  return;  // will not work then ...
  for( size_t j=0; j < BondTab.RowCount(); j++ )  {
    const cif_dp::CifRow& row = BondTab[j];
    CifTabBond *LB = new CifTabBond(AtomByName(row[index]->GetStringValue()),
      AtomByName(row[index1]->GetStringValue()));
    LB->Value = row[index2]->GetStringValue();
    LB->S2 = row[index3]->GetStringValue();
    bool uniq = true;
    for( size_t k=0; k < FBonds.Count(); k++ )  {
      CifTabBond *LB1 = FBonds[k];
      if( ((LB->A1 == LB1->A2) && (LB->A2 == LB1->A1)) )  {  // only then atoms are inverted !!
        if( LB->Value == LB1->Value )  {
          uniq = false;
          break;
        }
      }
    }
    uniq = true;
    if( uniq )  {
      FBonds.Add(LB);
      LB->A1.Bonds.Add(LB);
      if( LB->S2 == "." )
        LB->A2.Bonds.Add(LB);
    }
    else
      delete LB;
  }

  CL = C.FindLoop("_geom_angle");
  if( CL == NULL )  return;
  const cif_dp::CifTable& AngTab = CL->GetData();
  index =  AngTab.ColIndex("_geom_angle_atom_site_label_1");
  index1 = AngTab.ColIndex("_geom_angle_atom_site_label_2");
  index2 = AngTab.ColIndex("_geom_angle_atom_site_label_3");
  index3 = AngTab.ColIndex("_geom_angle");
  size_t index4 = AngTab.ColIndex("_geom_angle_site_symmetry_1");
  size_t index5 = AngTab.ColIndex("_geom_angle_site_symmetry_3");
  if( (index|index1|index2|index3|index4|index5) == InvalidIndex )
    return;  // will not work then ...
  for( size_t j=0; j < AngTab.RowCount(); j++ )  {
    const cif_dp::CifRow& row = AngTab[j];
    CifTabAngle* ang = new CifTabAngle(
      AtomByName(row[index]->GetStringValue()),
      AtomByName(row[index1]->GetStringValue()),
      AtomByName(row[index2]->GetStringValue()));
    FAngles.Add(ang);
    ang->Value = row[index3]->GetStringValue();
    ang->S1 = row[index4]->GetStringValue();
    ang->S3 = row[index5]->GetStringValue();
  }
}
//..............................................................................
TLinkedLoopTable::~TLinkedLoopTable()  {
  FAtoms.DeleteItems();
  FBonds.DeleteItems();
  FAngles.DeleteItems();
}
//..............................................................................
CifTabAtom& TLinkedLoopTable::AtomByName(const olxstr &Name)  {
  for( size_t i=0; i < FAtoms.Count(); i++ )  {
    if( FAtoms[i]->Label == Name )
      return *FAtoms[i];
  }
  throw TInvalidArgumentException(__OlxSourceInfo, olxstr("atom_name=") << Name);
}
//..............................................................................
TTTable<TStrList>* TLinkedLoopTable::MakeTable(const olxstr &Atom)  {
  CifTabAtom& A = AtomByName(Atom);
  if( A.Bonds.IsEmpty() )
    return NULL;
  size_t bc = A.Bonds.Count();
  TStrList Symm;
  Symm.Add("."); // to give proper numbering of symm operations
  // search for symm operations
  for( size_t i=0; i < bc; i++ )  {
    CifTabBond* LB = A.Bonds[i];
    size_t sind = Symm.IndexOf(LB->S2);
    if( sind == InvalidIndex )
      Symm.Add(LB->S2);
  }
  // sort bonds according to the requirements
  ;
  QuickSorter::Sort(A.Bonds, TLLTBondSort(A, Symm, cCifTabSortLength));
  olxstr Tmp;
  Table.Clear();
  Table.Resize(bc+1, bc);
  Table.ColName(0) = A.Label;
  for( size_t i=0; i < bc-1; i++ )  {
    CifTabBond* LB = A.Bonds[i];
    const CifTabAtom& AAtom = LB->Another(A);
    size_t sind = Symm.IndexOf(LB->S2);
    if( sind != 0 )
      Tmp << "<sup>" << sind << "</sup>";
    Table.ColName(i+1) = AAtom.Label;
  }
  for( size_t i=0; i < bc; i++ )  {
    CifTabBond* LB = A.Bonds[i];
    const CifTabAtom& AAtom = LB->Another(A);
    size_t sind = Symm.IndexOf(LB->S2);
    if( sind != 0 )
      Tmp << "<sup>" << sind << "</sup>";

    Table.RowName(i) = AAtom.Label;
    Table[i][0] = LB->Value;
    for( size_t j=0; j < bc-1; j++ )  {
      CifTabBond* LB1 = A.Bonds[j];
      if( i==j )  {
        Table[i][j+1] = "-";
        continue;
      }
      if( i < j )  {
        Table[i][j+1].SetLength(0);
        continue;
      }
      bool found = false;
      for( size_t k=0; k < A.Angles.Count(); k++ )  {
        CifTabAngle* LA = A.Angles[k];
        if( LA->FormedBy(*LB, *LB1) )  {
          found = true;
          Table[i][j+1] = LA->Value;
          break;
        }
      }
      if( !found )
        Table[i][j+1] = '?';
    }
  }
  Tmp.SetLength(0);
  for( size_t i=0; i < Symm.Count(); i++ )  {
    if( Symm[i].Length() > 0 &&  (Symm[i] != '.') )  {
      const olxstr Tmp1 = FCif.SymmCodeToSymm(Symm[i]);
      if( !Tmp1.IsEmpty() )  {
        Tmp << "<sup>" << i << "</sup>" << ": " << Tmp1;
        if( i < (Symm.Count()-1) )
          Tmp << "<br>";
      }
    }
  }
  Table[A.Bonds.Count()][0] = Tmp;
  return &Table;
}
