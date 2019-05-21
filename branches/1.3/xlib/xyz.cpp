/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "xyz.h"
#include "catom.h"
#include "unitcell.h"
#include "olxvar.h"

TXyz::TXyz()  {  Clear();  }
//..............................................................................
TXyz::~TXyz()  {}
//..............................................................................
void TXyz::Clear()  {
  GetAsymmUnit().Clear();
  GetAsymmUnit().GetAxes() = vec3d(1,1,1);
  GetAsymmUnit().GetAngles() = vec3d(90,90,90);
  GetAsymmUnit().InitMatrices();
}
//..............................................................................
void TXyz::SaveToStrings(TStrList& Strings) {
  Strings.Add();
  Strings.Add("Exported from Olex2");
  size_t cnt = 0;
  int op = TOlxVars::FindValue("file_output_precision", "4").ToInt();
  for (size_t i = 0; i < GetAsymmUnit().AtomCount(); i++) {
    TCAtom& CA = GetAsymmUnit().GetAtom(i);
    if (CA.IsDeleted()) {
      continue;
    }
    olxstr& aline = Strings.Add(CA.GetType().symbol);
    aline << ' ';
    vec3d v = GetAsymmUnit().Orthogonalise(CA.ccrd());
    for (int j = 0; j < 3; j++) {
      aline << olxstr::FormatFloat(op, v[j]).LeftPadding(10, ' ', true);
    }
    cnt++;
  }
  Strings[0] = cnt;
}
//..............................................................................
void TXyz::LoadFromStrings(const TStrList &Strings) {
  TAsymmUnit &au = GetAsymmUnit();
  au.Clear();
  Title = "OLEX2: imported from XYZ";
  for (size_t i = 0; i < Strings.Count(); i++) {
    olxstr line = Strings[i];
    if (line.IsEmpty()) {
      continue;
    }
    TStrList toks(line, ' ');
    if (toks.Count() != 4) {
      continue;
    }
    const cm_Element* elm = XElementLib::FindBySymbolEx(toks[0]);
    if (elm != 0) {
      TCAtom& CA = GetAsymmUnit().NewAtom();
      CA.ccrd()[0] = toks[1].ToDouble();
      CA.ccrd()[1] = toks[2].ToDouble();
      CA.ccrd()[2] = toks[3].ToDouble();
      CA.SetType(*elm);
      CA.SetLabel(olxstr(elm->symbol) << (GetAsymmUnit().AtomCount() + 1), false);
    }
  }
  GenerateCellForCartesianFormat();
}
//..............................................................................
bool TXyz::Adopt(TXFile &XF, int flags) {
  Clear();
  Title = XF.LastLoader()->GetTitle();
  GetRM().SetHKLSource(XF.LastLoader()->GetRM().GetHKLSource());
  const ASObjectProvider& objects = XF.GetLattice().GetObjects();
  for (size_t i=0; i < objects.atoms.Count(); i++) {
    TSAtom& sa = objects.atoms[i];
    if (!sa.IsAvailable()) {
      continue;
    }
    TCAtom& a = GetAsymmUnit().NewAtom();
    a.SetLabel(sa.GetLabel(), false);
    a.ccrd() = sa.crd();
    a.SetType(sa.GetType());
  }
  GetAsymmUnit().SetZ((double)XF.GetLattice().GetUnitCell().MatrixCount());
  return true;
}
//..............................................................................
