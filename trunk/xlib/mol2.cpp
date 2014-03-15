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
const olxstr TMol2::BondNames[] = {"1", "2", "3", "am", "ar", "du", "un", "nc"};
//..............................................................................
void TMol2::Clear()  {
  GetAsymmUnit().Clear();
  Bonds.Clear();
  GetAsymmUnit().GetAxes() = vec3d(1,1,1);
  GetAsymmUnit().GetAngles() = vec3d(90,90,90);
  GetAsymmUnit().InitMatrices();
}
//..............................................................................
olxstr TMol2::MOLAtom(TCAtom& A)  {
  olxstr rv(A.GetId(), 64);
  rv << '\t' << A.GetLabel() 
     << '\t' << A.ccrd()[0] 
     << '\t' << A.ccrd()[1] 
     << '\t' << A.ccrd()[2]
     << '\t' << A.GetType().symbol;
  return rv;
}
//..............................................................................
const olxstr& TMol2::EncodeBondType(short type) const  {
  if( type >= 8 || type < 0 )
    throw TInvalidArgumentException(__OlxSourceInfo, "bond type");
  return BondNames[type-1];
}
//..............................................................................
short TMol2::DecodeBondType(const olxstr& name) const  {
  for( int i=0; i < 8; i++ )
    if( BondNames[i].Equalsi(name) )
      return i+1;
  return mol2btUnknown;
}
//..............................................................................
olxstr TMol2::MOLBond(TMol2Bond& B)  {
  olxstr rv(B.GetId(), 32);
  rv << '\t' << B.a1->GetId() 
     << '\t' << B.a2->GetId() 
     << '\t' << EncodeBondType(B.BondType);
  return rv;
}
//..............................................................................
void TMol2::SaveToStrings(TStrList& Strings)  {
  Strings.Add("@<TRIPOS>MOLECULE");
  Strings.Add(GetTitle());
  Strings.Add(GetAsymmUnit().AtomCount())  << '\t' << Bonds.Count() << "\t1";
  Strings.Add("SMALL");
  Strings.Add("NO_CHARGES");
  Strings.Add(EmptyString());
  Strings.Add("@<TRIPOS>ATOM");
  for( size_t i=0; i < GetAsymmUnit().AtomCount(); i++ )
    Strings.Add( MOLAtom(GetAsymmUnit().GetAtom(i) ) );
  if( !Bonds.IsEmpty() )  {
    Strings.Add(EmptyString());
    Strings.Add("@<TRIPOS>BOND");
    for( size_t i=0; i < Bonds.Count(); i++ )
      Strings.Add( MOLBond(Bonds[i]) );
  }
}
//..............................................................................
void TMol2::LoadFromStrings(const TStrList& Strings)  {
  Clear();
  Title = "OLEX2: imported from TRIPOS MOL2";
  bool AtomsCycle = false, BondsCycle = false;
  olxdict<int, TCAtom*, TPrimitiveComparator> atoms;
  for( size_t i=0; i < Strings.Count(); i++ )  {
    olxstr line = Strings[i].UpperCase().Replace('\t', ' ').Trim(' ');
    if( line.IsEmpty() )  continue;
    if( AtomsCycle )  {
      if( line.Compare("@<TRIPOS>BOND") == 0 )  {
        BondsCycle = true;
        AtomsCycle = false;
        continue;
      }
      TStrList toks(line, ' ');
      if( toks.Count() < 6 )  continue;
      vec3d crd(toks[2].ToDouble(), toks[3].ToDouble(), toks[4].ToDouble());
      TStrList ent(toks[5], '.');
      cm_Element *elm = XElementLib::FindBySymbol(ent[0]);
      if (elm != NULL) {
        TCAtom& CA = GetAsymmUnit().NewAtom();
        CA.ccrd() = crd;
        CA.SetLabel(toks[1], false);
        CA.SetType(*elm);
        atoms(toks[0].ToInt(), &CA);
      }
      continue;
    }
    if( BondsCycle )  {
      if( line.CharAt(0) == '@' )  {
        BondsCycle = false;
        break;
      }
      TStrList toks(line, ' ');
      if( toks.Count() < 4 )
        continue;
      TMol2Bond& MB = Bonds.Add(new TMol2Bond(Bonds.Count()));
      MB.a1 = atoms.Get(toks[1].ToInt());
      MB.a2 = atoms.Get(toks[2].ToInt());
      MB.BondType = DecodeBondType(toks[3]);  // bond type
      continue;
    }
    if( line.Equals("@<TRIPOS>ATOM") )  {
      AtomsCycle = true;
      continue;
    }
  }
}

//..............................................................................
bool TMol2::Adopt(TXFile& XF)  {
  Clear();
  const ASObjectProvider& objects = XF.GetLattice().GetObjects();
  for( size_t i=0; i < objects.atoms.Count(); i++ )  {
    TSAtom& sa = objects.atoms[i];
    if( !sa.IsAvailable() )  continue;
    TCAtom& a = GetAsymmUnit().NewAtom();
    a.SetLabel(sa.GetLabel(), false);
    a.ccrd() = sa.crd();
    a.SetType(sa.GetType());
    sa.SetTag(i);
  }
  for( size_t i=0; i < objects.bonds.Count(); i++ )  {
    TSBond& sb = objects.bonds[i];
    if( !sb.IsAvailable() )  continue;
    TMol2Bond& mb = Bonds.AddNew(Bonds.Count());
    mb.a1 = &GetAsymmUnit().GetAtom(sb.A().GetTag());
    mb.a2 = &GetAsymmUnit().GetAtom(sb.B().GetTag());
    mb.BondType = 1; // singlel bond, a proper encoding is required...
  }
  GetAsymmUnit().SetZ((short)XF.GetLattice().GetUnitCell().MatrixCount());
  return true;
}
//..............................................................................
