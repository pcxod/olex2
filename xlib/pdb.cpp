/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "pdb.h"
#include "unitcell.h"
#include "bapp.h"
#include "log.h"
#include "label_corrector.h"

void TPdb::Clear()  {
  GetRM().Clear(rm_clear_ALL);
  GetAsymmUnit().Clear();
}
//..............................................................................
void TPdb::SaveToStrings(TStrList& Strings) {
  olx_array_ptr<char> bf(120);
  double q[6];
  int iq[6];
  double bk = 8 * M_PI*M_PI;
  TSpaceGroup &sg = TSymmLib::GetInstance().FindSG(GetAsymmUnit());
  olxcstr z = GetAsymmUnit().GetZ();
  z.TrimFloat();
  if (z.Length() > 4) {
    z.SetLength(4);
  }
  sprintf(bf, "CRYST1%9.3f%9.3f%9.3f%7.2f%7.2f%7.2f %-11s%4s",
    GetAsymmUnit().GetAxes()[0],
    GetAsymmUnit().GetAxes()[1],
    GetAsymmUnit().GetAxes()[2],
    GetAsymmUnit().GetAngles()[0],
    GetAsymmUnit().GetAngles()[1],
    GetAsymmUnit().GetAngles()[2],
    sg.GetFullName().c_str(),
    z.c_str());
  Strings.Add(&bf);
  Strings.Add("TITLE OLEX2 export");
  for (size_t i = 0; i < GetAsymmUnit().AtomCount(); i++) {
    TCAtom& a = GetAsymmUnit().GetAtom(i);
    if (a.IsDeleted()) {
      continue;
    }
    olxstr label = a.GetLabel();
    vec3d crd = GetAsymmUnit().Orthogonalise(a.ccrd());
    TResidue &r = GetAsymmUnit().GetResidue(a.GetResiId());
    olxstr r_name = r.GetClassName();
    if (r_name.Length() > 3) {
      r_name.SetLength(3);
    }
    olxcstr r_num, p_num(' ');
    if (r.GetId() != 0) {
      r_num = r.GetNumber();
    }
    char p = olx_min(olx_abs(a.GetPart()), 25);
    if (p != 0) {
      p_num[0] = ('A' + p - 1);
    }
    sprintf(bf, "ATOM  %5d %4s%1s%3s %1c%4s    %8.3f%8.3f%8.3f%6.2f%6.2f          %2s  ",
      (int)(i + 1),
      label.c_str(),
      p_num.c_str(),
      r_name.c_str(),
      r.HasChainId() ? r.GetChainId() : ' ',
      r_num.c_str(),
      crd[0],
      crd[1],
      crd[2],
      a.GetOccu(),
      a.GetUiso()*bk,
      a.GetType().symbol.c_str()
    );
    Strings.Add(&bf);
    TEllipsoid* e = a.GetEllipsoid();
    if (e == 0) {
      continue;
    }
    e->GetShelxQuad(q);
    for (int j = 0; j < 6; j++) {
      iq[j] = (int)(q[j] * 10000);
    }
    sprintf(bf, "ANISOU%5d %4s            %7d%7d%7d%7d%7d%7d      %2s  ",
      (int)(i + 1),
      label.c_str(),
      iq[0],
      iq[1],
      iq[2],
      iq[5],
      iq[4],
      iq[3],
      a.GetType().symbol.c_str()
    );
    Strings.Add(&bf);
  }
}
//..............................................................................
void TPdb::LoadFromStrings(const TStrList& Strings) {
  Clear();

  evecd QE(6);
  TStrList toks;
  TSizeList CrystF;
  CrystF.Add(6);  // CRYST1
  CrystF.Add(9);  // a
  CrystF.Add(9);  // b
  CrystF.Add(9);  // c
  CrystF.Add(7);  // alpha
  CrystF.Add(7);  // beta
  CrystF.Add(7);  // gamma
  CrystF.Add(1);  // ws
  CrystF.Add(11);  // sg name
  CrystF.Add(4);  // z
  //http://www.wwpdb.org/documentation/format33/sect9.html#ATOM
  TSizeList AtomF;
  AtomF.Add(6);  //"ATOM  "
  AtomF.Add(5 + 1);  //serial number + ws
  AtomF.Add(4);  //name
  AtomF.Add(1);  //alternative location indicator
  AtomF.Add(3 + 1);  //residue name + ws
  AtomF.Add(1);  //chain ID
  AtomF.Add(4);  //  residue sequence number
  AtomF.Add(1 + 3);  //iCode + 3 ws
  AtomF.Add(8);  // x
  AtomF.Add(8);  // y
  AtomF.Add(8);  // z
  AtomF.Add(6);  // occupancy
  AtomF.Add(6 + 10);  // temperature factor + 10ws
  AtomF.Add(2);  // element
  AtomF.Add(2);  // charge

  TSizeList AnisF;
  AnisF.Add(6); // record name
  AnisF.Add(5);  // serial
  AnisF.Add(1);  // ws
  AnisF.Add(4);  // atom name
  AnisF.Add(1);  // alternative location
  AnisF.Add(3);  // residue name
  AnisF.Add(1);  // ws
  AnisF.Add(1);  // chain ID
  AnisF.Add(4);  // residue sequence number
  AnisF.Add(2);  // iCode +  1 ws
  AnisF.Add(7);  // U00
  AnisF.Add(7);  // U11
  AnisF.Add(7);  // U22
  AnisF.Add(7);  // U01
  AnisF.Add(7);  // U02
  AnisF.Add(7);  // U12
  AnisF.Add(2);  // element
  AnisF.Add(2);  // Charge


  Title = "OLEX2: imported from PDB";
  for (size_t i = 0; i < Strings.Count(); i++) {
    if (Strings[i].StartsFromi("CRYST1")) {
      toks.StrtokF(Strings[i], CrystF);
      if (toks.Count() < 7) {
        throw TFunctionFailedException(__OlxSourceInfo, "parsing failed");
      }
      GetAsymmUnit().GetAxes() = vec3d(
        toks[1].ToDouble(), toks[2].ToDouble(), toks[3].ToDouble());
      GetAsymmUnit().GetAngles() = vec3d(
        toks[4].ToDouble(), toks[5].ToDouble(), toks[6].ToDouble());
      GetAsymmUnit().InitMatrices();
      if (toks.Count() > 8) {
        TSymmLib& sl = TSymmLib::GetInstance();
        TSpaceGroup* sg = 0;
        toks[8].Trim(' ');
        for (size_t j = 0; j < sl.SGCount(); j++) {
          if (sl.GetGroup(j).GetFullName() == toks[8]) {
            sg = &sl.GetGroup(j);
            break;
          }
        }
        if (sg != 0) {
          GetAsymmUnit().ChangeSpaceGroup(*sg);
        }
      }
      if (toks.Count() > 9) {
        GetAsymmUnit().SetZ(toks[9].ToDouble());
      }
    }
    else if (Strings[i].StartsFromi("ATOM")) {
      toks.Clear();
      toks.StrtokF(Strings[i], AtomF);
      if (toks.Count() < 12) {
        throw TFunctionFailedException(__OlxSourceInfo, "parsing failed");
      }
      TResidue *resi = 0;
      if (toks[6].IsNumber()) {
        int r_num = toks[6].ToInt();
        resi = GetAsymmUnit().FindResidue(r_num);
        if (resi == 0) {
          resi = &GetAsymmUnit().NewResidue(toks[4].TrimWhiteChars(), r_num, r_num,
            toks[5].CharAt(0));
        }
      }
      TCAtom& CA = GetAsymmUnit().NewAtom(resi);
      CA.ccrd() = GetAsymmUnit().Fractionalise(
        vec3d(toks[8].ToDouble(), toks[9].ToDouble(), toks[10].ToDouble()));
      if (toks.Count() > 11) {
        CA.SetOccu(toks[11].ToDouble());
      }
      if (toks.Count() > 12) {
        CA.SetUiso(toks[12].ToDouble() / (8 * olx_sqr(M_PI)));
      }
      olxstr name = toks[2].TrimWhiteChars();
      const cm_Element *type = 0;
      if (toks.Count() > 13) {
        type = XElementLib::FindBySymbol(toks[13].TrimWhiteChars());
      }
      else {
        if (name.StartsFrom("CA") || name.StartsFrom("CD") ||
          name.StartsFrom("CE"))
        {
          type = &XElementLib::GetByIndex(iCarbonIndex);
        }
        else if (name == 'W') {
          type = &XElementLib::GetByIndex(iOxygenIndex);
        }
      }
      if (!toks[5].TrimWhiteChars().IsEmpty() && resi == 0) // chain ID
        name << '_' << toks[5];
      CA.SetLabel(name, type == 0);
      if (type != 0) {
        CA.SetType(*type);
      }
      if (toks[3].IsNumber()) { // altLoc
        CA.SetPart(toks[3].ToInt());
      }
      else if (olxstr::o_isalpha(toks[3].CharAt(0))) {
        CA.SetPart((int)(toks[3].UpperCase().CharAt(0) - 'A' + 1));
      }
    }
    else if (Strings[i].StartsFromi("ANISOU")) {
      toks.Clear();
      toks.StrtokF(Strings[i], AnisF);
      if (toks.Count() < 16) {
        throw TFunctionFailedException(__OlxSourceInfo, "parsing failed");
      }
      QE[0] = toks[10].ToDouble();
      QE[1] = toks[11].ToDouble();
      QE[2] = toks[12].ToDouble();
      QE[3] = toks[15].ToDouble();
      QE[4] = toks[14].ToDouble();
      QE[5] = toks[13].ToDouble();
      QE /= 10000;
      TCAtom* ca = GetAsymmUnit().FindCAtomById(toks[1].ToSizeT() - 1);
      if (ca != 0) {
        ca->UpdateEllp(QE);
        if (ca->GetEllipsoid()->IsNPD()) {
          TBasicApp::NewLogEntry(logError) << "Not positevely defined: "
            << ca->GetLabel();
        }
        ca->SetUiso((QE[0] + QE[1] + QE[2]) / 3);
      }
    }
  }
}
//..............................................................................
bool TPdb::Adopt(TXFile& XF, int flags) {
  Clear();
  GetRM().Assign(XF.GetRM(), true);
  if (flags != 0) {
    LabelCorrector cr(AsymmUnit, 4, true);
    const ASObjectProvider& objects = XF.GetLattice().GetObjects();
    for (size_t i = 0; i < objects.atoms.Count(); i++) {
      TSAtom& sa = objects.atoms[i];
      if (!sa.IsAvailable() || sa.GetType() == iQPeakZ || sa.IsAUAtom()) {
        continue;
      }
      TResidue* r = 0;
      if (sa.CAtom().GetResiId() != 0) {
        TResidue& ar = XF.GetAsymmUnit().GetResidue(sa.CAtom().GetResiId());
        r = GetAsymmUnit().FindResidue(ar.GetChainId(), ar.GetNumber());
      }
      TCAtom& a = GetAsymmUnit().NewAtom(r);
      a.SetLabel(sa.GetLabel(), false);
      a.ccrd() = sa.ccrd();
      a.SetType(sa.GetType());
      a.SetPart(sa.CAtom().GetPart());
      cr.Correct(a);
      TEllipsoid* se = sa.GetEllipsoid();
      if (se == 0) {
        continue;
      }
      TEllipsoid& e = GetAsymmUnit().NewEllp();
      e = *se;
      a.AssignEllp(&e);
    }
  }
  return true;
}
//..............................................................................
