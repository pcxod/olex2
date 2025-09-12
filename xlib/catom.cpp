/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "catom.h"
#include "ellipsoid.h"
#include "exception.h"
#include "estrlist.h"
#include "refmodel.h"
#include "pers_util.h"
#include "residue.h"
#include "symmcon.h"

olxstr TCAtom::VarNames[] = {"Scale", "X", "Y", "Z", "Sof",
  "Uiso", "U11", "U22", "U33", "U23", "U13", "U12"};

TCAtom::TCAtom(TAsymmUnit* _Parent) : Parent(_Parent)  {
  PartAndChargeAndChirality = 0;
  Occu   = 1;
  QPeak  = 0;
  SetId(0);
  SetResiId(0);  // default residue is unnamed one
  SetSameId(~0);
  EllpId = ~0;
  Uiso = caDefIso;
  OccuEsd = UisoEsd = 0;
  UisoScale = 0;
  UisoOwner = 0;
  FragmentId = ~0;
  Equivs = 0;
  Type = 0;
  SetTag(-1);
  DependentAfixGroup = ParentAfixGroup = 0;
  DependentHfixGroups = 0;
  ExyzGroup = 0;
  Flags = 0;
  ConnInfo = 0;
  SpecialPositionDeviation = 0;
  memset(Vars, 0, sizeof(Vars));
  disp = 0;
}
//..............................................................................
TCAtom::~TCAtom() {
  olx_del_obj(DependentHfixGroups);
  olx_del_obj(ConnInfo);
  olx_del_obj(Equivs);
}
//..............................................................................
void TCAtom::SetConnInfo(CXConnInfo& ci) {
  olx_del_obj(ConnInfo);
  ConnInfo = &ci;
}
//..............................................................................
void TCAtom::SetLabel(const olxstr& L, bool validate) {
  if (validate) {
    if (L.IsEmpty()) {
      throw TInvalidArgumentException(__OlxSourceInfo, "empty label");
    }
    const cm_Element *atype = XElementLib::FindBySymbolEx(L);
    if (atype == 0) {
      throw TInvalidArgumentException(__OlxSourceInfo,
        olxstr("Unknown element: '") << L << '\'');
    }
    if (Type != atype) {
      if (Type != 0 && *Type == iQPeakZ) {
        SetQPeak(0);
      }
      Type = atype;
      Parent->_OnAtomTypeChanged(*this);
    }
    Label = L;
    Label[0] = Label.o_toupper(Label.CharAt(0));
    if (Type->symbol.Length() == 2) {
      Label[1] = Label.o_tolower(Label.CharAt(1));
    }
  }
  else {
    Label = L;
  }
}
//..............................................................................
bool TCAtom::IsAttachedTo(const TCAtom& ca) const {
  for (size_t i = 0; i < AttachedSites.Count(); i++) {
    if (AttachedSites[i].atom == &ca) {
      return true;
    }
  }
  return false;
}
//..............................................................................
void TCAtom::SetType(const cm_Element& t) {
  if (Type != &t) {
    if (Type != 0 && *Type == iQPeakZ) {
      SetQPeak(0);
    }
    Type = &t;
    Parent->_OnAtomTypeChanged(*this);
  }
}
//..............................................................................
void TCAtom::AssignEquivs(const TCAtom& S)  {
  if (S.Equivs != 0) {
    if (Equivs == 0) {
      Equivs = new smatd_list(*S.Equivs);
    }
    else {
      *Equivs = *S.Equivs;
    }
  }
  else if (Equivs != 0) {
    delete Equivs;
    Equivs = 0;
  }
}
//..............................................................................
void TCAtom::ClearEquivs() {
  if (Equivs != 0) {
    delete Equivs;
    Equivs = 0;
  }
}
//..............................................................................
void TCAtom::Assign(const TCAtom& S)  {
  DependentAfixGroup = ParentAfixGroup = 0;  // managed by the group
  if( DependentHfixGroups != 0 )  {
    delete DependentHfixGroups;
    DependentHfixGroups = 0;
  }
  ExyzGroup = 0;  // also managed by the group
  SetPart(S.GetPart());
  SetCharge(S.GetCharge());
  SetSpecialPositionDeviation(S.GetSpecialPositionDeviation());
  SetOccu(S.GetOccu());
  SetOccuEsd(S.GetOccuEsd());
  SetQPeak(S.GetQPeak());
  SetResiId(S.GetResiId());
  SetSameId(S.GetSameId());
  EllpId = S.EllpId;
  SetUiso(S.GetUiso());
  SetUisoEsd(S.GetUisoEsd());
  SetUisoScale(S.GetUisoScale());
  if (S.UisoOwner != 0) {
    UisoOwner = Parent->FindCAtomById(S.UisoOwner->GetId());
    if (UisoOwner == 0) {
      throw TFunctionFailedException(__OlxSourceInfo, "asymmetric units mismatch");
    }
  }
  else {
    UisoOwner = 0;
  }
  Label   = S.Label;
  if (Type != &S.GetType()) {
    Type = &S.GetType();
    Parent->_OnAtomTypeChanged(*this);
  }
//  Frag    = S.Frag;
  //Id = S.GetId();
  FragmentId = S.GetFragmentId();
  Center = S.Center;
  Esd = S.Esd;
  AssignEquivs(S);
  Flags = S.Flags;
  disp = 0;
  if (S.GetDisp().ok()) {
    disp = new Disp(*S.GetDisp());
  }
  AttachedSites.Clear();
  AttachedSitesI.Clear();
  for (size_t i = 0; i < S.AttachedSiteCount(); i++) {
    const TCAtom::Site& s = S.GetAttachedSite(i);
    AttachedSites.AddNew(&Parent->GetAtom(s.atom->GetId()), s.matrix);
  }
  for (size_t i = 0; i < S.AttachedSiteICount(); i++) {
    const TCAtom::Site& s = S.GetAttachedSiteI(i);
    AttachedSitesI.AddNew(&Parent->GetAtom(s.atom->GetId()), s.matrix);
  }
}
//..............................................................................
int TCAtom::GetAfix() const {
  if (ParentAfixGroup == 0) {
    if (DependentAfixGroup != 0 && (DependentAfixGroup->HasExcplicitPivot() ||
      DependentAfixGroup->GetM() == 0))
    {
      return DependentAfixGroup->GetAfix();
    }
    //if( DependentHfixGroup != NULL && !DependentHfixGroup->IsRiding() )
    //  return DependentHfixGroup->GetAfix();
    return 0;
  }
  if (ParentAfixGroup->HasExcplicitPivot()) {
    return (ParentAfixGroup->GetAfix() / 10) * 10 + 5;
  }
  else {
    int a = ParentAfixGroup->GetAfix();
#ifdef _DEBUG
    return a;
#else
    return a == -1 ? 0 : a;
#endif
  }
}
//..............................................................................
TEllipsoid* TCAtom::GetEllipsoid() const {
  return EllpId == InvalidIndex ? 0 : &Parent->GetEllp(EllpId);
}
//..............................................................................
void TCAtom::AssignEllp(TEllipsoid* NV) {
  EllpId = (NV == 0 ? InvalidIndex : NV->GetId());
}
//..............................................................................
void TCAtom::UpdateEllp(const TEllipsoid &NV) {
  evecd Q(6), E(6);
  NV.GetShelxQuad(Q, E);
  if( EllpId == InvalidIndex )  {
    TEllipsoid& elp = Parent->NewEllp();
    elp.Initialise(Q, E);
    EllpId = elp.GetId();
  }
  else {
    Parent->GetEllp(EllpId).Initialise(Q);
  }
}
//..............................................................................
void TCAtom::ToDataItem(TDataItem& item) const {
  item.AddField("label", Label);
  item.AddField("type", Type->symbol);
  item.AddField("part", GetPart());
  item.AddField("charge", GetCharge());
  item.AddField("sof", TEValueD(Occu, OccuEsd).ToString());
  item.AddField("flags", Flags);
  item.AddField("x", TEValueD(Center[0], Esd[0]).ToString());
  item.AddField("y", TEValueD(Center[1], Esd[1]).ToString());
  item.AddField("z", TEValueD(Center[2], Esd[2]).ToString());
  if (!olx_is_valid_index(EllpId)) {
    item.AddField("Uiso", TEValueD(Uiso, UisoEsd).ToString());
    if (UisoOwner != NULL && UisoOwner->GetTag() >= 0) {
      TDataItem& uo = item.AddItem("Uowner");
      uo.AddField("id", UisoOwner->GetTag());
      uo.AddField("k", UisoScale);
    }
  }
  else {
    evecd Q(6), E(6);
    GetEllipsoid()->GetShelxQuad(Q, E);
    TDataItem& elp = item.AddItem("adp");
    elp.AddField("xx", TEValueD(Q[0], E[0]).ToString());
    elp.AddField("yy", TEValueD(Q[1], E[1]).ToString());
    elp.AddField("zz", TEValueD(Q[2], E[2]).ToString());
    elp.AddField("yz", TEValueD(Q[3], E[3]).ToString());
    elp.AddField("xz", TEValueD(Q[4], E[4]).ToString());
    elp.AddField("xy", TEValueD(Q[5], E[5]).ToString());
    if (GetEllipsoid()->IsAnharmonic()) {
      const tensor::tensor_rank_3 &c = GetEllipsoid()->GetAnharmonicPart()->C;
      const tensor::tensor_rank_4 &d = GetEllipsoid()->GetAnharmonicPart()->D;
      olxstr_buf t;
      for (size_t i = 0; i < c.data().Count(); i++) {
        t << c.data()[i] << ',';
      }
      for (size_t i = 0; i < d.data().Count(); i++) {
        t << d.data()[i] << ',';
      }
      elp.AddItem("CG", olxstr(t).TrimR(','));
    }
  }
  if (disp.ok()) {
    item.AddField("fp", TEValueD(disp->value.GetRe(), disp->fp_su).ToString());
    item.AddField("fdp", TEValueD(disp->value.GetIm(), disp->fdp_su).ToString());
  }
  if (*Type == iQPeakZ) {
    item.AddField("peak", QPeak);
  }
}
//..............................................................................
#ifdef _PYTHON
PyObject* TCAtom::PyExport(bool export_attached_sites) {
  PyObject* main = PyDict_New();
  PythonExt::SetDictItem(main, "label", PythonExt::BuildString(Label));
  PythonExt::SetDictItem(main, "type", PythonExt::BuildString(Type->symbol));
  PythonExt::SetDictItem(main, "part", Py_BuildValue("i", GetPart()));
  PythonExt::SetDictItem(main, "charge", Py_BuildValue("i", GetCharge()));
  PythonExt::SetDictItem(main, "occu", Py_BuildValue("(dd)", Occu, OccuEsd));
  PythonExt::SetDictItem(main, "tag", Py_BuildValue("i", GetTag()));
  PythonExt::SetDictItem(main, "crd",
    Py_BuildValue("(ddd)(ddd)", Center[0], Center[1], Center[2],
      Esd[0], Esd[1], Esd[2]));
  if (!olx_is_valid_index(EllpId)) {
    PythonExt::SetDictItem(main, "uiso", Py_BuildValue("(dd)", Uiso, UisoEsd));
    if (UisoOwner != 0 && UisoOwner->GetTag() >= 0) {
      PyObject* uo = PyDict_New();
      PythonExt::SetDictItem(uo, "id", Py_BuildValue("i", UisoOwner->GetTag()));
      PythonExt::SetDictItem(uo, "k", Py_BuildValue("d", UisoScale));
      PythonExt::SetDictItem(main, "uisoOwner", uo);
    }
  }
  else {
    double Q[6], E[6];
    GetEllipsoid()->GetShelxQuad(Q, E);
    PythonExt::SetDictItem(main, "adp",
      Py_BuildValue("(dddddd)(dddddd)", Q[0], Q[1], Q[2], Q[3], Q[4], Q[5],
        E[0], E[1], E[2], E[3], E[4], E[5])
    );
    if (GetEllipsoid()->IsAnharmonic()) {
      GramCharlier &a = GetEllipsoid()->GetAnharmonicPart();
      PyObject* anh = PyDict_New();
      if (a.order >= 3) {
        PythonExt::SetDictItem(anh, "C",
          Py_BuildValue("(dddddddddd)", a.C[0], a.C[1], a.C[2], a.C[3], a.C[4],
            a.C[5], a.C[6], a.C[7], a.C[8], a.C[9])
        );
      }
      if (a.order >= 4) {
        PythonExt::SetDictItem(anh, "D",
          Py_BuildValue("(ddddddddddddddd)",
            a.D[0], a.D[1], a.D[2], a.D[3], a.D[4], a.D[5], a.D[6], a.D[7],
            a.D[8], a.D[9], a.D[10], a.D[11], a.D[12], a.D[13], a.D[14])
        );
      }
      PythonExt::SetDictItem(anh, "order", Py_BuildValue("i", a.order));
      PythonExt::SetDictItem(main, "anharmonic_adp", anh);
    }
  }
  if (disp.ok()) {
    PythonExt::SetDictItem(main, "disp", Py_BuildValue("(dd)",
      disp->value.GetRe(), disp->value.GetIm()));
  }
  if (*Type == iQPeakZ) {
    PythonExt::SetDictItem(main, "peak", Py_BuildValue("d", QPeak));
  }
  else if (IsChiral()) {
    PyObject* rsa = PythonExt::BuildString(IsChiralR() ? "R" : "S");
    PythonExt::SetDictItem(main, "rsa", rsa);
  }
  if (export_attached_sites) {
    size_t cnt = 0;
    for (size_t i = 0; i < AttachedSites.Count(); i++) {
      if (AttachedSites[i].atom->GetTag() >= 0) {
        cnt++;
      }
    }
    PyObject* neighbours = PyTuple_New(cnt);
    const TAsymmUnit& au = *Parent;
    cnt = 0;
    for (size_t i = 0; i < AttachedSites.Count(); i++) {
      Site& s = AttachedSites[i];
      if (s.atom->GetTag() < 0) {
        continue;
      }
      const smatd& mat = s.matrix;
      const vec3d crd = au.Orthogonalise(mat*s.atom->ccrd());
      if (mat.IsFirst()) {
        PyTuple_SetItem(neighbours, cnt++, Py_BuildValue("i", s.atom->GetTag()));
      }
      else {
        PyTuple_SetItem(neighbours, cnt++,
          Py_BuildValue("OOO", Py_BuildValue("i", s.atom->GetTag()),
            Py_BuildValue("(ddd)", crd[0], crd[1], crd[2]),
            Py_BuildValue("(iii)(iii)(iii)(ddd)",
              mat.r[0][0], mat.r[0][1], mat.r[0][2],
              mat.r[1][0], mat.r[1][1], mat.r[1][2],
              mat.r[2][0], mat.r[2][1], mat.r[2][2],
              mat.t[0], mat.t[1], mat.t[2]
              )
            )
          );
      }
    }
    PythonExt::SetDictItem(main, "neighbours", neighbours);
  }
  return main;
}
#endif
//..............................................................................
void TCAtom::FromDataItem(TDataItem& item) {
  Type = XElementLib::FindBySymbol(item.GetFieldByName("type"));
  if (Type == 0) {
    throw TFunctionFailedException(__OlxSourceInfo, "invalid atom type");
  }
  TEValueD ev;
  Label = item.GetFieldByName("label");
  SetPart(item.GetFieldByName("part").ToInt());
  SetCharge(item.FindField("charge", '0').ToInt());
  ev = item.GetFieldByName("sof");
  Occu = ev.GetV();  OccuEsd = ev.GetE();
  Flags = item.GetFieldByName("flags").ToInt();
  ev = item.GetFieldByName("x");
  Center[0] = ev.GetV();  Esd[0] = ev.GetE();
  ev = item.GetFieldByName("y");
  Center[1] = ev.GetV();  Esd[1] = ev.GetE();
  ev = item.GetFieldByName("z");
  Center[2] = ev.GetV();  Esd[2] = ev.GetE();

  TDataItem* adp = item.FindItem("adp");
  if (adp != 0) {
    double Q[6], E[6];
    if (adp->FieldCount() != 6) {
      throw TInvalidArgumentException(__OlxSourceInfo,
        "6 parameters expected for the ADP");
    }
    const char *names[] = { "xx", "yy", "zz", "yz", "xz", "xy" };
    for (int i = 0; i < 6; i++) {
      ev = adp->GetFieldByName(names[i]);
      E[i] = ev.GetE();  Q[i] = ev.GetV();
    }
    EllpId = Parent->NewEllp().Initialise(Q, E).GetId();
    Uiso = GetEllipsoid()->GetUeq();
    TDataItem *cgi = adp->FindItem("CG");
    if (cgi != 0) {
      olx_object_ptr<GramCharlier> cg = new GramCharlier(
        TStrList(cgi->GetValue(), ','));
      GetEllipsoid()->SetAnharmonicPart(cg.release());
    }
  }
  else {
    EllpId = InvalidIndex;
    ev = item.GetFieldByName("Uiso");
    Uiso = ev.GetV();  UisoEsd = ev.GetE();
    TDataItem* uo = item.FindItem("Uowner");
    if (uo != 0) {
      UisoOwner = &GetParent()->GetAtom(uo->GetFieldByName("id").ToSizeT());
      UisoScale = uo->GetFieldByName("k").ToDouble();
    }
  }
  size_t fp_idx = item.FieldIndex("fp");
  if (fp_idx != InvalidIndex) {
    TEValueD fp(item.GetFieldByIndex(fp_idx));
    TEValueD fdp(item.GetFieldByName("fdp"));
    disp = new Disp(compd(fp.GetV(), fdp.GetV()));
    disp->fp_su = fp.GetE();
    disp->fdp_su = fdp.GetE();
  }
  if (*Type == iQPeakZ) {
    QPeak = item.GetFieldByName("peak").ToDouble();
  }
}
//..............................................................................
void DigitStrtok(const olxstr &str, TStringToList<olxstr, bool>& chars) {
  olxstr Dig, Char;
  for (size_t i = 0; i < str.Length(); i++) {
    if (str[i] <= '9' && str[i] >= '0') {
      if (!Char.IsEmpty()) {
        chars.Add(Char, true);
        Char.SetLength(0);
      }
      Dig << str[i];
    }
    else {
      if (!Dig.IsEmpty()) {
        chars.Add(Dig, false);
        Dig.SetLength(0);
      }
      Char << str[i];
    }
  }
  if (!Char.IsEmpty()) {
    chars.Add(Char, true);
  }
  if (!Dig.IsEmpty()) {
    chars.Add(Dig, false);
  }
}
//..............................................................................
int TCAtom::CompareAtomLabels(const olxstr& S, const olxstr& S1) {
  TStringToList<olxstr, bool> Chars1, Chars2;

  DigitStrtok(S, Chars1);
  DigitStrtok(S1, Chars2);
  for (size_t i = 0; i < olx_min(Chars1.Count(), Chars2.Count()); i++) {
    if (Chars1.GetObject(i) && Chars2.GetObject(i)) {
      int res = Chars1[i].Comparei(Chars2[i]);
      if (res != 0) {
        return res;
      }
    }
    if (!Chars1.GetObject(i) && !Chars2.GetObject(i)) {
      int res = Chars1[i].ToInt() - Chars2[i].ToInt();
      //if( !res )  // to tackle 01 < 1
      //{  res = Chars1->String(i).Length() - Chars2->String(i).Length();  }
      //the following commented line allows sorting like C01 C02 and C01A then
      //but though it looks better, it is not correct, so using C01 C01A C02
      //if( res && (Chars1->Count() == Chars2->Count()))  return res;
      if (res != 0)  return res;
    }

    if (!Chars1.GetObject(i) && Chars2.GetObject(i)) {
      return 1;
    }
    if (Chars1.GetObject(i) && !Chars2.GetObject(i)) {
      return -1;
    }
  }
  return olx_cmp(Chars1.Count(), Chars2.Count());
}
//..............................................................................
bool TCAtom::AttachSite(TCAtom* atom, const smatd& matrix) {
  for (size_t i = 0; i < AttachedSites.Count(); i++) {
    if (AttachedSites[i].atom == atom) {
      if (AttachedSites[i].matrix.GetId() == matrix.GetId()) {
        return false;
      }
      const vec3d v1 = matrix*atom->ccrd();
      const vec3d v2 = AttachedSites[i].matrix*atom->ccrd();
      if (v1.Equals(v2, 1e-3)) {
        return false;
      }
    }
  }
  AttachedSites.AddNew(atom, matrix);
  return true;
}
//..............................................................................
bool TCAtom::AttachSiteI(TCAtom* atom, const smatd& matrix) {
  for (size_t i = 0; i < AttachedSitesI.Count(); i++) {
    if (AttachedSitesI[i].atom == atom) {
      if (AttachedSitesI[i].matrix.GetId() == matrix.GetId()) {
        return false;
      }
      const vec3d v1 = matrix*atom->ccrd();
      const vec3d v2 = AttachedSitesI[i].matrix*atom->ccrd();
      if (v1.Equals(v2, 1e-3)) {
        return false;
      }
    }
  }
  AttachedSitesI.AddNew(atom, matrix);
  return true;
}
//..............................................................................
void TCAtom::UpdateAttachedSites() {
  // check if any symm eqivs were removed
  bool removed = false;
  for (size_t i = 0; i < AttachedSites.Count(); i++) {
    if (AttachedSites[i].atom->IsDeleted() ||
      (IsCentroid() && !AttachedSites[i].atom->IsCentroid()) ||
      (!IsCentroid() && AttachedSites[i].atom->IsCentroid()))
    {
      AttachedSites.NullItem(i);
      removed = true;
    }
  }
  if (removed) {
    AttachedSites.Pack();
  }
  // end of the removed eqivs test
  smatd_list ml;
  BondInfoList toCreate, toDelete;
  ConnInfo::Compile(*this, toCreate, toDelete, ml);
  smatd I;
  I.I().SetId(0);
  for (size_t i = 0; i < toCreate.Count(); i++) {
    AttachSite(&toCreate[i].to,
      toCreate[i].matr == NULL ? I : *toCreate[i].matr);
  }
  for (size_t i = 0; i < toDelete.Count(); i++) {
    for (size_t j = 0; j < AttachedSites.Count(); j++) {
      if (&toDelete[i].to == AttachedSites[j].atom) {
        if ((toDelete[i].matr == 0 && AttachedSites[j].matrix.IsFirst()) ||
          (toDelete[i].matr != 0 &&
            toDelete[i].matr->GetId() == AttachedSites[j].matrix.GetId()))
        {
          AttachedSites.Delete(j);
          break;
        }
      }
    }
  }
  const CXConnInfo& ci = GetConnInfo();
  if (AttachedSites.Count() > (size_t)olx_abs(ci.maxBonds)) {
    QuickSorter::SortMF(AttachedSites, *this,
      ci.maxBonds < 0 ? &TCAtom::SortSitesByDistanceDsc
      : &TCAtom::SortSitesByDistanceAsc);
    // prevent q-peaks and H atoms from affecting the max number of bonds...
    uint16_t bc2set = olx_abs(ci.maxBonds);
    for (uint16_t j = 0; j < bc2set; j++) {
      if (AttachedSites[j].atom->GetType() <= iHydrogenZ) {
        if (++bc2set >= AttachedSites.Count()) {
          break;
        }
      }
    }
    for (size_t i = bc2set; i < AttachedSites.Count(); i++) {
      if (AttachedSites[i].atom == this) {
        continue;
      }
      for (size_t j = 0; j < AttachedSites[i].atom->AttachedSites.Count(); j++) {
        if (AttachedSites[i].atom->AttachedSites[j].atom == this) {
          AttachedSites[i].atom->AttachedSites.Delete(j--);
        }
      }
    }
    AttachedSites.Shrink(bc2set);
  }
}
//..............................................................................
olxstr TCAtom::GetResiLabel(bool add_part) const {
  olxstr rv = GetLabel();
  if (GetResiId() != 0) {
    TResidue &r = GetParent()->GetResidue(GetResiId());
    rv << '_' << r.GetNumberStr();
  }
  if (add_part && GetPart() != 0) {
    rv << '^' << (olxch)('a' + olx_abs(GetPart()) - 1);
    if (GetPart() < 0) {
      rv << '*';
    }
  }
  return rv;
}
//..............................................................................
SiteSymmCon TCAtom::GetSiteConstraints() const {
  SiteSymmCon rv;
  for (size_t i = 0; i < EquivCount(); i++) {
    rv += SymmConReg::Find(rotation_id::get(GetEquiv(i).r));
  }
  return rv;
}
//..............................................................................
bool TCAtom::IsPositionFixed(bool any) const {
  size_t fixed_cnt = 0;
  for (size_t j = 0; j < 3; j++) {
    if (GetVarRef(catom_var_name_X + j) != 0 &&
      GetVarRef(catom_var_name_X + j)->relation_type == relation_None)
    {
      fixed_cnt++;
    }
  }
  return any ? fixed_cnt != 0 : fixed_cnt == 3;
}
//..............................................................................
bool TCAtom::IsADPFixed(bool any) const {
  if (GetEllipsoid() == 0) {
    return GetVarRef(catom_var_name_Uiso) != 0 &&
      GetVarRef(catom_var_name_Uiso)->relation_type == relation_None;
  }
  size_t fixed_cnt = 0;
  for (size_t j = 0; j < 3; j++) {
    if (GetVarRef(catom_var_name_U11 + j) != 0 &&
      GetVarRef(catom_var_name_U11 + j)->relation_type == relation_None)
    {
      fixed_cnt++;
    }
  }
  return any ? fixed_cnt != 0 : fixed_cnt == 6;
}
//..............................................................................
//..............................................................................
//..............................................................................
int TCAtomComparator::Compare(const TCAtom& a1, const TCAtom& a2) {
  if (a1.GetFragmentId() != a2.GetFragmentId()) {
    return a1.GetFragmentId() - a2.GetFragmentId();
  }
  if (a1.GetResiId() != a2.GetResiId()) {
    return olx_cmp(a1.GetResiId(), a2.GetResiId());
  }
  // by label
  if (a1.GetType() == a2.GetType()) {
    return TCAtom::CompareAtomLabels(a1.GetLabel(), a2.GetLabel());
  }
  // by mass
  return olx_cmp(a1.GetType().GetMr(), a2.GetType().GetMr());
}

//..............................................................................
//..............................................................................
//..............................................................................
olxstr TGroupCAtom::GetFullLabel(const RefinementModel& rm) const {
  olxstr name = Atom->GetResiLabel(true);
  if (Atom->GetResiId() == 0) {
    if (Matrix != 0) {
      name << "_$" << (rm.UsedSymmIndex(*Matrix) + 1);
    }
  }
  else {  // it is however shown that shelx just IGNORES $EQIV in this notation...
    if (Matrix != 0) {
      name << '$' << (rm.UsedSymmIndex(*Matrix) + 1);
    }
  }
  return name;
}
//..............................................................................
olxstr TGroupCAtom::GetFullLabel(const RefinementModel& rm,
  int resiId) const
{
  olxstr name = Atom->GetLabel();
  TResidue &r = Atom->GetParent()->GetResidue(Atom->GetResiId());
  if( Atom->GetResiId() == 0 || r.GetNumber() == resiId) {
    if (Matrix != 0) {
      name << "_$" << (rm.UsedSymmIndex(*Matrix) + 1);
    }
  }
  else {  // it is however shown that shelx just IGNORES $EQIV in this notation...
    name << '_' << r.GetNumberStr();
    if (Matrix != 0) {
      name << '$' << (rm.UsedSymmIndex(*Matrix) + 1);
    }
  }
  return name;
}
//..............................................................................
olxstr TGroupCAtom::GetFullLabel(const RefinementModel& rm,
  const olxstr& resiName) const
{
  if (resiName.IsEmpty()) {
    return GetFullLabel(rm);
  }

  olxstr name(Atom->GetLabel());
  if (resiName.IsNumber()) {
    if (Atom->GetResiId() == 0 ||
      (Atom->GetParent()->GetResidue(
        Atom->GetResiId()).GetNumber() == resiName.ToInt()))
    {
      if (Matrix != 0) {
        name << "_$" << (rm.UsedSymmIndex(*Matrix) + 1);
      }
    }
    else {  // it is however shown that shelx just IGNORES $EQIV in this notation...
      name << '_' << Atom->GetParent()->GetResidue(Atom->GetResiId()).GetNumber();
      if (Matrix != 0) {
        name << '$' << (rm.UsedSymmIndex(*Matrix) + 1);
      }
    }
  }
  else {
    if (!olx_is_valid_index(Atom->GetResiId()) ||
      (!resiName.IsEmpty() && Atom->GetParent()->GetResidue(
        Atom->GetResiId()).GetClassName().Equalsi(resiName)))
    {
      if (Matrix != 0) {
        name << "_$" << (rm.UsedSymmIndex(*Matrix) + 1);
      }
    }
    else {  // it is however shown that shelx just IGNORES $EQIV in this notation...
      name << '_' << Atom->GetParent()->GetResidue(Atom->GetResiId()).GetNumber();
      if (Matrix != 0) {
        name << '$' << (rm.UsedSymmIndex(*Matrix) + 1);
      }
    }
  }
  return name;
}
//..............................................................................
void TGroupCAtom::ToDataItem(TDataItem& di) const {
  di.AddField("atom_id", Atom->GetTag()).AddField("matr_id",
    Matrix == 0 ? -1 : Matrix->GetId());
}
//..............................................................................
void TGroupCAtom::FromDataItem(const TDataItem& di, const RefinementModel& rm)  {
  Atom = &rm.aunit.GetAtom(di.GetFieldByName("atom_id").ToSizeT());
  int m_id = di.GetFieldByName("matr_id").ToInt();
  Matrix = (m_id == -1 ? 0 :&rm.GetUsedSymm(m_id));
}
//..............................................................................
IXVarReferencerContainer& TCAtom::GetParentContainer() const {  return *Parent;  }
//..............................................................................
double TCAtom::GetValue(size_t var_index) const {
  switch( var_index)  {
    case catom_var_name_X:     return Center[0];
    case catom_var_name_Y:     return Center[1];
    case catom_var_name_Z:     return Center[2];
    case catom_var_name_Sof:   return Occu;
    case catom_var_name_Uiso:  return Uiso;
    case catom_var_name_U11:
    case catom_var_name_U22:
    case catom_var_name_U33:
    case catom_var_name_U23:
    case catom_var_name_U13:
    case catom_var_name_U12:
      if (!olx_is_valid_index(EllpId)) {
        throw TInvalidArgumentException(__OlxSourceInfo, "Uanis is not defined");
      }
      return Parent->GetEllp(EllpId).GetQuad(var_index-catom_var_name_U11);
    default:
      throw TInvalidArgumentException(__OlxSourceInfo, "parameter name");
  }
}
//..............................................................................
void TCAtom::SetValue(size_t var_index, const double& val) {
  switch( var_index)  {
    case catom_var_name_X:     Center[0] = val;  break;
    case catom_var_name_Y:     Center[1] = val;  break;
    case catom_var_name_Z:     Center[2] = val;  break;
    case catom_var_name_Sof:
    {
      // try to fix rounding
      double v = 192 * val;
      long rv = olx_round(v);
      if (olx_abs(v - rv) < 1e-3) {
        Occu = (double)rv/192;
      }
      else {
        Occu = val;
      }
    }
      break;
    case catom_var_name_Uiso:  Uiso = val;  break;
    case catom_var_name_U11:
    case catom_var_name_U22:
    case catom_var_name_U33:
    case catom_var_name_U23:
    case catom_var_name_U13:
    case catom_var_name_U12:
      break;
    default:
      throw TInvalidArgumentException(__OlxSourceInfo, "parameter name");
  }
}
//..............................................................................
int TCAtom::SortSitesByDistanceAsc(const Site &a1, const Site &a2) const {
  vec3d v1 = Parent->Orthogonalise(ccrd() - a1.matrix*a1.atom->ccrd());
  vec3d v2 = Parent->Orthogonalise(ccrd() - a2.matrix*a2.atom->ccrd());
  return olx_cmp(v1.QLength(), v2.QLength());
}
//..............................................................................
void TCAtom::SetTagRecursively(TCAtom &a, index_t v) {
  a.SetProcessed(true);
  for (size_t i = 0; i < a.AttachedSiteCount(); i++) {
    TCAtom &aa = a.GetAttachedAtom(i);
    if (aa.IsProcessed()) {
      continue;
    }
    aa.SetTag(v);
    SetTagRecursively(aa, v);
  }
}
//..............................................................................
TIString TCAtom::ToString() const {
  olxstr_buf rv = GetResiLabel(true);
  rv << " {Id:" << GetId() << ", Tag: " << GetTag() << '}';
  return olxstr(rv);
}
//..............................................................................
Atom3DId TCAtom::Get3DId() const {
  return Atom3DId((int8_t)GetType().z, Center);
}
//..............................................................................
//..............................................................................
//..............................................................................
Atom3DId::Atom3DId(int8_t z, const vec3d& crd, double multiplier) {
  id = ((uint64_t)z) & z_mask;
  static const int64_t k = mask_m / cell_m;
  int64_t x = multiplier == 1 ? (int64_t)(crd[0] * k)
    : ((int64_t)(crd[0] * multiplier)) / multiplier * k;
  if (x < 0) {
    id |= a_sig;
    id |= (((-x) << 8) & a_mask);
  }
  else {
    id |= ((olx_abs(x) << 8) & a_mask);
  }
  x = multiplier == 1 ? (int64_t)(crd[1] * k)
    : ((int64_t)(crd[1] * multiplier)) / multiplier * k;
  if (x < 0) {
    id |= b_sig;
    id |= (((-x) << 25) & b_mask);
  }
  else {
    id |= ((x << 25) & b_mask);
  }
  x = multiplier == 1 ? (int64_t)(crd[2] * k)
    : ((int64_t)(crd[2] * multiplier)) / multiplier * k;
  if (x < 0) {
    id |= c_sig;
    id |= (((-x) << 42) & c_mask);
  }
  else {
    id |= ((x << 42) & c_mask);
  }
}
//..............................................................................
vec3d Atom3DId::get_crd() const {
  static const double k = (double)cell_m / mask_m;
  vec3d r((double)((id & a_mask) >> 8) * k,
    (double)((id & b_mask) >> 25) * k,
    (double)((id & c_mask) >> 42) * k);
  if ((id & a_sig) != 0) {
    r[0] = -r[0];
  }
  if ((id & b_sig) != 0) {
    r[1] = -r[1];
  }
  if ((id & c_sig) != 0) {
    r[2] = -r[2];
  }
  return r;
}
//..............................................................................
olxstr Atom3DId::ToString() const {
  return olxstr(id);
}
//..............................................................................
Atom3DId& Atom3DId::operator = (const olxstr& s) {
  s.ToNumber(id);
  return *this;
}
//..............................................................................
