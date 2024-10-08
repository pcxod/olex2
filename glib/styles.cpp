/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "bapp.h"
#include "styles.h"
#include "glmaterial.h"

//------------------------------------------------------------------------------
//TPrimitiveStyle implementation
//------------------------------------------------------------------------------
void TPrimitiveStyle::ToDataItem(TDataItem& Item) const {
  Item.AddField("PName", GetName());
  Item.AddItem("Material", Parent.GetDataItem(this));
}
//..............................................................................
bool TPrimitiveStyle::FromDataItem(const TDataItem& Item)  {
  Name = ReadName(Item);
  TDataItem* MI = Item.FindItem("Material");
  if (MI != NULL) {
    if (MI->ItemCount() != 0) {
      TGlMaterial* GlM = Parent.GetMaterial(MI->GetItemByIndex(0));
      if (GlM != NULL) {
        SetProperties(*GlM);
        return true;
      }
      else {
        delete GlM;
      }
    }
  }
  return false;
}
//..............................................................................
//------------------------------------------------------------------------------
//TGraphicsStyle implementation
//------------------------------------------------------------------------------
void TGraphicsStyle::Clear() {
  for (size_t i = 0; i < Styles.Count(); i++) {
    delete Styles.GetValue(i);
  }
  Styles.Clear();
  PStyles.Clear();
  Params.Clear();
}
//..............................................................................
bool TGraphicsStyle::operator == (const TGraphicsStyle &GS) const {
  const size_t pc = PrimitiveStyleCount();
  if (pc != GS.PrimitiveStyleCount()) {
    return false;
  }
  for (size_t i = 0; i < pc; i++) {
    if (!(*PStyles[i] == GS.GetPrimitiveStyle(i))) {
      return false;
    }
  }
  return true;
}
//..............................................................................
TGlMaterial& TGraphicsStyle::CreatePrimitiveMaterial(const olxstr& pname,
  const TGlMaterial& glm)
{
  TPrimitiveStyle* PS = Parent.NewPrimitiveStyle(pname);
  PStyles.Add(PS);
  return (TGlMaterial&)PS->SetProperties(glm);
}
//..............................................................................
void TGraphicsStyle::ToDataItem(TDataItem& Item, bool saveAll) const {
  if (!saveAll && !IsSaveable()) {
    return;
  }
  Item.AddField("Name", Name);
  if (IsPersistent()) {
    Item.AddField("Persistent", TrueString());
  }
  for (size_t i = 0; i < Params.Count(); i++) {
    if (!saveAll && !Params.GetValue(i).saveable) {
      continue;
    }
    Item.AddField(Params.GetKey(i), Params.GetValue(i).val);
  }

  size_t ssc = saveAll ? 1 : 0;
  const size_t sc = Styles.Count();
  if (!saveAll) {
    for (size_t i = 0; i < sc; i++)
      if (Styles.GetValue(i)->IsSaveable()) {
        ssc++;
      }
  }
  if (ssc != 0) {
    TDataItem& RI = Item.AddItem("SubStyles");
    for (size_t i = 0; i < sc; i++) {
      if (!saveAll && !Styles.GetValue(i)->IsSaveable()) {
        continue;
      }
      Styles.GetValue(i)->ToDataItem(RI.AddItem(olxstr("S_") << i), saveAll);
    }
  }
  for (size_t i = 0; i < PStyles.Count(); i++) {
    PStyles[i]->ToDataItem(Item.AddItem(olxstr("S") << i));
  }
}
//..............................................................................
bool TGraphicsStyle::FromDataItem(const TDataItem& Item) {
  SetNew(false);
  bool name_set = false,
    persistence_set = false;
  for (size_t i = 0; i < Item.FieldCount(); i++) {
    if (!name_set && Item.GetFieldName(i) == "Name") {
      Name = Item.GetFieldByIndex(i);
      name_set = true;
    }
    else if (!persistence_set && Item.GetFieldName(i) == "Persistent") {
      SetPersistent(Item.GetFieldByIndex(i).ToBool());
      persistence_set = true;
    }
    else {
      SetParam(Item.GetFieldName(i), Item.GetFieldByIndex(i), Level < 2);
    }
  }
  //    SetParam(Item.FieldName(i), Item.Field(i), FParent->GetVersion() > 0 );
  TDataItem* I = Item.FindItem("SubStyles");
  size_t off = 0;
  if (I != 0) {
    off = 1;
    for (size_t i = 0; i < I->ItemCount(); i++) {
      const TDataItem& si = I->GetItemByIndex(i);
      const olxstr& si_name = si.FindField("Name");
      TGraphicsStyle* GS = FindLocalStyle(si_name);
      if (GS == 0) {
        GS = Styles.Add(si_name, new TGraphicsStyle(Parent, this, si_name)).Value;
      }
      GS->FromDataItem(I->GetItemByIndex(i));
    }
  }
  // merging happens here...
  for (size_t i = off; i < Item.ItemCount(); i++) {
    const TDataItem& psi = Item.GetItemByIndex(i);
    const olxstr& psi_name = TPrimitiveStyle::ReadName(psi);
    TPrimitiveStyle* PS = 0;
    for (size_t j = 0; j < PStyles.Count(); j++) {
      if (PStyles[j]->GetName() == psi_name) {
        PS = PStyles[j];
        break;
      }
    }
    if (PS == 0) {
      PS = Parent.NewPrimitiveStyle_(psi_name);
      if (PS->FromDataItem(Item.GetItemByIndex(i))) {
        PStyles.Add(Parent.AddPrimitiveStyle(PS));
      }
      else {
        delete PS;
      }
    }
    else {
      PS->FromDataItem(Item.GetItemByIndex(i));
    }
  }
  return true;
}
//..............................................................................
TGraphicsStyle *TGraphicsStyle::FindStyle(const olxstr& Name) {
  if (Name.Contains('.')) {
    TStrList Toks(Name, '.');
    size_t index=0;
    TGraphicsStyle* AS = FindLocalStyle(Toks[index]);
    while (AS != 0) {
      if (++index >= Toks.Count()) {
        return AS;
      }
      AS = AS->FindLocalStyle(Toks[index]);
    }
    return 0;
  }
  else {
    return FindLocalStyle(Name);
  }
}
//..............................................................................
TGraphicsStyle& TGraphicsStyle::NewStyle(const olxstr& Name, bool Force) {
  if (Name.Contains('.')) {
    TStrList Toks(Name, '.');
    size_t index = 0;
    TGraphicsStyle *PrevGS = 0;
    TGraphicsStyle* GS = FindLocalStyle(Toks[index]);
    while (GS != 0) {
      if (++index >= Toks.Count()) {
        return *GS;
      }
      PrevGS = GS;  // save last valid value
      GS = GS->FindLocalStyle(Toks[index]);
    }
    // here we have GS==NULL, and index pointing to the following
    if (PrevGS == 0) {
      PrevGS = this;
    }
    if (Force) {
      for (size_t i = index; i < Toks.Count(); i++) {
        PrevGS = &PrevGS->NewStyle(Toks[i]);
      }
      return *PrevGS;
    }
    else {
      return PrevGS->NewStyle(Toks[index]);
    }
  }
  else {
    TGraphicsStyle* gs = FindLocalStyle(Name);
    return *(gs != NULL ? gs : Styles.Add(Name,
      new TGraphicsStyle(Parent, this, Name)).Value);
  }
}
//..............................................................................
TGraphicsStyle *TGraphicsStyle::FindStyle(TGraphicsStyle* style) {
  for (size_t i=0; i < Styles.Count(); i++) {
    TGraphicsStyle* GS = Styles.GetValue(i);
    if (*GS == *style) {
      return GS;
    }
    GS = GS->FindStyle(style);
    if (GS != 0) {
      return GS;
    }
  }
  return NULL;
}
//..............................................................................
// sets TCollectionItem::Tag of styles to Tag
void TGraphicsStyle::SetStylesTag(index_t Tag) {
  SetTag(Tag);
  const size_t sc = Styles.Count();
  for (size_t i = 0; i < sc; i++) {
    Styles.GetValue(i)->SetStylesTag(Tag);
  }
}
//..............................................................................
// removes Styles with Style::Tag == Tag
void TGraphicsStyle::RemoveStylesByTag(index_t Tag) {
  bool changed = false;
  for (size_t i = 0; i < Styles.Count(); i++) {
    TGraphicsStyle* GS = Styles.GetValue(i);
    if (GS->GetTag() == Tag) {
      if (GS->Styles.Count() != 0) {
        GS->RemoveStylesByTag(Tag);
      }
      if (GS->Styles.Count() == 0 && !GS->IsPersistent()) {
        delete GS;
        Styles.NullItem(i);
        changed = true;
      }
    }
  }
  if (changed) {
    Styles.Pack();
  }
}
//..............................................................................
void TGraphicsStyle::RemoveNonPersistent() {
  bool changed = false;
  for (size_t i = 0; i < Styles.Count(); i++) {
    TGraphicsStyle* GS = Styles.GetValue(i);
    if (!GS->Styles.IsEmpty()) {
      GS->RemoveNonPersistent();
    }
    if (!GS->IsPersistent()) {
      delete GS;
      Styles.NullItem(i);
      changed = true;
    }
  }
  if (changed) {
    Styles.Pack();
  }
}
//..............................................................................
void TGraphicsStyle::RemoveNonSaveable() {
  bool changed = false;
  for (size_t i=0; i < Styles.Count(); i++) {
    TGraphicsStyle* GS = Styles.GetValue(i);
    if (!GS->Styles.IsEmpty()) {
      GS->RemoveNonSaveable();
    }
    if (!GS->IsSaveable()) {
      delete GS;
      Styles.NullItem(i);
      changed = true;
    }
  }
  if (changed) {
    Styles.Pack();
  }
}
//..............................................................................
void TGraphicsStyle::RemoveNamedStyles(const TStrList& toks) {
  if (toks.Count() <= Level) {
    return;
  }
  const size_t i = Styles.IndexOf(toks[Level]);
  if (i != InvalidIndex) {
    if (toks.Count() == size_t(Level + 1)) {
      delete Styles.GetValue(i);
      Styles.Delete(i);
    }
    else if (Styles.GetValue(i)->Styles.Count() != 0) {
      Styles.GetValue(i)->RemoveNamedStyles(toks);
    }
  }
}
//..............................................................................
void TGraphicsStyle::_TrimFloats() {
  for (size_t i=0; i < Params.Count(); i++) {
    if (Params.GetValue(i).val.IsNumber()) {
      Params.GetValue(i).val.TrimFloat();
    }
  }
  for (size_t i = 0; i < Styles.Count(); i++) {
    Styles.GetValue(i)->_TrimFloats();
  }
}
//..............................................................................
//------------------------------------------------------------------------------
//TGraphicsStyles implementation
//------------------------------------------------------------------------------
//..............................................................................
TGraphicsStyles::TGraphicsStyles() :
  OnClear(TBasicApp::GetInstance().NewActionQueue(olxappevent_GL_CLEAR_STYLES))
{
  Root = new TGraphicsStyle(*this, NULL, "Root");
  Version = CurrentVersion();
}
//..............................................................................
TGraphicsStyles::~TGraphicsStyles()  {
  Clear();
  delete Root;
}
//..............................................................................
void TGraphicsStyles::Clear() {
  OnClear.Enter(this);
  for (size_t i = 0; i < PStyles.ObjectCount(); i++) {
    delete &PStyles.GetObject(i);
  }
  for (size_t i = 0; i < PStyles.PropertiesCount(); i++) {
    delete &PStyles.GetProperties(i);
  }
  PStyles.Clear();
  Root->Clear();
  OnClear.Exit(this);
}
//..............................................................................
void TGraphicsStyles::ToDataItem(TDataItem& Item) const {
  TDataItem& SI = Item.AddItem("Materials");
  DataItems.Clear();
  for (size_t i=0; i < PStyles.PropertiesCount(); i++) {
    TDataItem& SI1 = SI.AddItem(olxstr("Prop")<<i);
    PStyles.GetProperties(i).ToDataItem(SI1);
    DataItems.Add(&SI1);
  }
  Item.AddField("Name", Name);
  Item.AddField("LinkFile", LinkFile);
  Item.AddField("Version", CurrentVersion());
  Root->ToDataItem(Item.AddItem("Root"));
  DataItems.Clear();
}
//..............................................................................
void TGraphicsStyles::ToDataItem(TDataItem& item,
  const TPtrList<TGraphicsStyle>& styles)
{
  TGraphicsStyle root(*this, NULL, "Root");
  TDataItem& SI = item.AddItem("Materials");
  DataItems.Clear();
  DataItems.SetCount(PStyles.PropertiesCount());
  TPtrList<TGraphicsStyle> allStyles(styles);
  size_t matc = 0;
  const size_t sc = styles.Count();
  // recursion here on Count()
  for (size_t i = 0; i < allStyles.Count(); i++) {
    TGraphicsStyle* gs = allStyles[i];
    if (i < sc) { // add only the top level styles
      root.AddStyle(gs);
    }
    for (size_t j = 0; j < gs->PrimitiveStyleCount(); j++) {
      const TGlMaterial& glm = gs->GetPrimitiveStyle(j).GetProperties();
      size_t mi = PStyles.IndexOfProperties(glm);
      if (mi == InvalidIndex) {
        root.ReleaseStyles();
        throw TFunctionFailedException(__OlxSourceInfo, "unregistered primitive style");
      }
      if (DataItems[mi] == NULL) {
        TDataItem& SI1 = SI.AddItem(olxstr("Prop") << matc++);
        glm.ToDataItem(SI1);
        DataItems[mi] = &SI1;
      }
    }
    for (size_t j = 0; j < gs->StyleCount(); j++) { // recursion implementation
      allStyles.Add(gs->GetStyle(j));
    }
  }
  item.AddField("Name", Name);
  item.AddField("LinkFile", LinkFile);
  item.AddField("Version", CurrentVersion());
  root.ToDataItem(item.AddItem("Root"), true);
  DataItems.Clear();
  root.ReleaseStyles();
}
//..............................................................................
bool TGraphicsStyles::FromDataItem(const TDataItem& Item, bool merge) {
  if (&Item == 0) {
    throw TInvalidArgumentException(__OlxSourceInfo, "item=NULL");
  }
  OnClear.Enter(this);
  TActionQueueLock ql_(&OnClear);
  if (!merge) {
    Clear();
  }
  TDataItem* SI = Item.FindItem("Materials");
  TPtrList<TGlMaterial> mats;
  for (size_t i = 0; i < SI->ItemCount(); i++) {
    TGlMaterial* GlM = new TGlMaterial;
    SI->GetItemByIndex(i).SetData(GlM);
    GlM->FromDataItem(SI->GetItemByIndex(i));
    mats.Add(GlM);
  }
  Name = Item.FindField("Name");
  LinkFile = Item.FindField("LinkFile");
  Version = Item.FindField("Version", "0").ToInt();
  SI = Item.FindItem("Root");
  if (SI != NULL) {
    Root->FromDataItem(*SI);
    if (Version == 0) {  // imported ?
      Root->_TrimFloats();
    }
  }
  for (size_t i = 0; i < mats.Count(); i++) {
    delete mats[i];
  }
  ql_.Unlock();
  OnClear.Exit(this);
  return true;
}
//..............................................................................
void TGraphicsStyles::CopyStyle(const olxstr &From, const olxstr &To) {
}
//..............................................................................
TPrimitiveStyle *TGraphicsStyles::NewPrimitiveStyle(const olxstr &PName) {
  TPrimitiveStyle *PS = new TPrimitiveStyle(PName, PStyles, *this);
  PStyles.AddObject(PS);
  return PS;
}
//..............................................................................
TPrimitiveStyle *TGraphicsStyles::NewPrimitiveStyle_(const olxstr &PName) {
  return new TPrimitiveStyle(PName, PStyles, *this);
}
//..............................................................................
TPrimitiveStyle *TGraphicsStyles::AddPrimitiveStyle(TPrimitiveStyle *PS) {
  PStyles.AddObject(PS);
  return PS;
}
//..............................................................................
TDataItem* TGraphicsStyles::GetDataItem(const TPrimitiveStyle* Style) const {
  TGlMaterial &m = Style->GetProperties();
  size_t i = PStyles.IndexOfProperties(m);
  if (i == InvalidIndex || DataItems[i] == 0) {
    throw TFunctionFailedException(__OlxSourceInfo, "unregistered properties");
  }
  return DataItems[i];
}
//..............................................................................
TGlMaterial* TGraphicsStyles::GetMaterial(TDataItem& I) const {
  if (I.GetData() == 0) {
    I.SetData(new TGlMaterial());
  }
  return (TGlMaterial*)I.GetData();
}
//..............................................................................
void TGraphicsStyles::DeleteStyle(TGraphicsStyle *Style) {
  if (Style->GetParentStyle() != 0) {
    Style->GetParentStyle()->DeleteStyle(*Style);
  }
}
//..............................................................................
bool TGraphicsStyles::LoadFromFile(const olxstr& FN, bool merge) {
  TDataFile DF;
  if (!DF.LoadFromXLFile(FN, 0)) {
    return false;
  }
  TDataItem *DI = DF.Root().FindItem("DStyle");
  if (DI == 0) {
    return false;
  }
  FromDataItem(*DI, merge);
  return true;
}
//..............................................................................
void TGraphicsStyles::SaveToFile(const olxstr& FN) {
  TDataFile DF;
  TDataItem& DI = DF.Root().AddItem("DStyle");
  this->ToDataItem(DI);
  DF.SaveToXLFile(FN);
}
//..............................................................................
void TGraphicsStyles::SetDefaultMaterial(
  const olxstr& object_name, const olxstr& p_name, const TGlMaterial& m)
{
  if (object_name.IsEmpty() || p_name.IsEmpty()) {
    return;
  }
  TGraphicsStyle* st = FindStyle(object_name);
  if (st == 0) {
    st = &NewStyle(object_name);
    st->CreatePrimitiveMaterial(p_name, m);
  }
  else {
    size_t i = st->IndexOfMaterial(p_name);
    if (i == InvalidIndex) {
      st->CreatePrimitiveMaterial(p_name, m);
    }
  }
}
//..............................................................................
