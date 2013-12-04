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
#include "glrender.h"

const int TGraphicsStyles::CurrentVersion = 2;
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
void TGraphicsStyle::Clear()  {
  for (size_t i =0; i < Styles.Count(); i++)
    delete Styles.GetObject(i);
  Styles.Clear();
  Params.Clear();
}
//..............................................................................
bool TGraphicsStyle::operator == (const TGraphicsStyle &GS) const  {
  const size_t pc = PrimitiveStyleCount();
  if( pc != GS.PrimitiveStyleCount() )
    return false;
  for( size_t i=0; i < pc; i++ )  {
    if( !(*PStyles[i] == GS.GetPrimitiveStyle(i)) )
      return false;
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
  if( !saveAll && !IsSaveable() )  return;
  Item.AddField("Name", Name);
  if( IsPersistent() )
    Item.AddField("Persistent", TrueString());
  for( size_t i=0; i < Params.Count(); i++ ) {
    if( !saveAll && !Params.GetObject(i).saveable )  continue;
    Item.AddField(Params.GetString(i), Params.GetObject(i).val);
  }

  size_t ssc = saveAll ? 1 : 0;
  const size_t sc = Styles.Count();
  if( !saveAll )  {
    for( size_t i=0; i < sc; i++ )
      if( Styles.GetObject(i)->IsSaveable() )
        ssc++;
  }
  if( ssc != 0 )  {
    TDataItem& RI = Item.AddItem("SubStyles");
    for( size_t i=0; i < sc; i++ )  {
      if( !saveAll&& !Styles.GetObject(i)->IsSaveable() )  continue;
      Styles.GetObject(i)->ToDataItem(RI.AddItem(olxstr("S_") <<i ), saveAll);
    }
  }
  for( size_t i=0; i < PStyles.Count(); i++ )
    PStyles[i]->ToDataItem(Item.AddItem(olxstr("S") << i));
}
//..............................................................................
bool TGraphicsStyle::FromDataItem(const TDataItem& Item)  {
  Name = Item.FindField("Name");
  SetPersistent(Item.FindField("Persistent", FalseString()).ToBool());
  size_t i = IsPersistent() ? 2 : 1;
  for( ; i < Item.FieldCount(); i++ )
    SetParam(Item.GetFieldName(i), Item.GetFieldByIndex(i), Level < 2);
//    SetParam(Item.FieldName(i), Item.Field(i), FParent->GetVersion() > 0 );
  TDataItem* I = Item.FindItem("SubStyles");
  size_t off = 0;
  if (I != NULL) {
    off = 1;
    for (i=0; i < I->ItemCount(); i++) {
      const TDataItem& si = I->GetItemByIndex(i);
      const olxstr& si_name = si.FindField("Name");
      TGraphicsStyle* GS = FindLocalStyle(si_name);
      if (GS == NULL)
        GS = Styles.Add(si_name, new TGraphicsStyle(Parent, this, si_name)).Object;
      GS->FromDataItem(I->GetItemByIndex(i));
    }
  }
  // merging happens here...
  for (i=off; i < Item.ItemCount(); i++) {
    const TDataItem& psi = Item.GetItemByIndex(i);
    const olxstr& psi_name = TPrimitiveStyle::ReadName(psi);
    TPrimitiveStyle* PS = NULL;
    for (size_t j=0; j < PStyles.Count(); j++) {
      if (PStyles[j]->GetName() == psi_name) {
        PS = PStyles[j];
        break;
      }
    }
    if (PS == NULL) {
      PS = Parent.NewPrimitiveStyle_(psi_name);
      if (PS->FromDataItem(Item.GetItemByIndex(i))) {
        PStyles.Add(Parent.AddPrimitiveStyle(PS));
      }
      else {
        delete PS;
      }
    }
    else
      PS->FromDataItem(Item.GetItemByIndex(i));
  }
  return true;
}
//..............................................................................
TGraphicsStyle *TGraphicsStyle::FindStyle(const olxstr& Name) {
  if (Name.Contains('.')) {
    TStrList Toks(Name, '.');
    size_t index=0;
    TGraphicsStyle* AS = FindLocalStyle(Toks[index]);
    while (AS != NULL) {
      if (++index >= Toks.Count())
        return AS;
      AS = AS->FindLocalStyle(Toks[index]);
    }
    return NULL;
  }
  else 
    return FindLocalStyle(Name);
}
//..............................................................................
TGraphicsStyle& TGraphicsStyle::NewStyle(const olxstr& Name, bool Force)  {
  if (Name.Contains('.')) {
    TStrList Toks(Name, '.');
    size_t index=0;
    TGraphicsStyle *PrevGS=NULL;
    TGraphicsStyle* GS = FindLocalStyle(Toks[index]);
    while (GS != NULL) {
      if (++index >= Toks.Count())
        return *GS;
      PrevGS = GS;  // save last valid value
      GS = GS->FindLocalStyle(Toks[index]);
    }
    // here we have GS==NULL, and index pointing to the following
    if (PrevGS == NULL) PrevGS = this;
    if (Force) {
      for (size_t i=index; i < Toks.Count(); i++)
        PrevGS = &PrevGS->NewStyle(Toks[i]);
      return *PrevGS;
    }
    else
      return PrevGS->NewStyle(Toks[index]);
  }
  else  {
    TGraphicsStyle* gs = FindLocalStyle(Name);
    return *(gs != NULL ? gs : Styles.Add(Name,
      new TGraphicsStyle(Parent, this, Name) ).Object);
  }
}
//..............................................................................
TGraphicsStyle *TGraphicsStyle::FindStyle(TGraphicsStyle* style) {
  for (size_t i=0; i < Styles.Count(); i++) {
    TGraphicsStyle* GS = Styles.GetObject(i);
    if (*GS == *style) return GS;
    GS = GS->FindStyle(style);
    if(GS != NULL) return GS;
  }
  return NULL;
}
//..............................................................................
// sets TCollectionItem::Tag of styles to Tag
void TGraphicsStyle::SetStylesTag(index_t Tag) {
  SetTag(Tag);
  const size_t sc = Styles.Count();
  for (size_t i=0; i < sc; i++)
    Styles.GetObject(i)->SetStylesTag(Tag);
}
//..............................................................................
// removes Styles with Style::Tag == Tag
void TGraphicsStyle::RemoveStylesByTag(index_t Tag) {
  bool changed = false;
  for (size_t i=0; i < Styles.Count(); i++) {
    TGraphicsStyle* GS = Styles.GetObject(i);
    if (GS->GetTag() == Tag) {
      if (GS->Styles.Count() != 0)
        GS->RemoveStylesByTag(Tag);
      if (GS->Styles.Count() == 0  && !GS->IsPersistent()) {
        delete GS;
        Styles.NullItem(i);
        changed = true;
      }
    }
  }
  if(changed)
    Styles.Pack();
}
//..............................................................................
void TGraphicsStyle::RemoveNonPersistent() {
  bool changed = false;
  for( size_t i=0; i < Styles.Count(); i++ )  {
    TGraphicsStyle* GS = Styles.GetObject(i);
    if( !GS->Styles.IsEmpty() )
      GS->RemoveNonPersistent();
    if( !GS->IsPersistent() )  {
      delete GS;
      Styles.NullItem(i);
      changed = true;
    }
  }
  if( changed )  
    Styles.Pack();
}
//..............................................................................
void TGraphicsStyle::RemoveNonSaveable() {
  bool changed = false;
  for (size_t i=0; i < Styles.Count(); i++) {
    TGraphicsStyle* GS = Styles.GetObject(i);
    if (!GS->Styles.IsEmpty())
      GS->RemoveNonSaveable();
    if (!GS->IsSaveable()) {
      delete GS;
      Styles.NullItem(i);
      changed = true;
    }
  }
  if (changed)
    Styles.Pack();
}
//..............................................................................
void TGraphicsStyle::RemoveNamedStyles(const TStrList& toks) {
  if (toks.Count() < Level) return;
  const size_t i = Styles.IndexOf(toks[Level]);
  if (i != InvalidIndex) {
    if (toks.Count() == size_t(Level+1)) {
      delete Styles.GetObject(i);
      Styles.Delete(i);
    }
    else if (Styles.GetObject(i)->Styles.Count() != 0)
      Styles.GetObject(i)->RemoveNamedStyles(toks);
  }
}
//..............................................................................
void TGraphicsStyle::_TrimFloats() {
  for (size_t i=0; i < Params.Count(); i++) {
    if (Params.GetObject(i).val.IsNumber()) {
      Params.GetObject(i).val.TrimFloat();
    }
  }
  for (size_t i=0; i < Styles.Count(); i++)
    Styles.GetObject(i)->_TrimFloats();
}
//..............................................................................
//------------------------------------------------------------------------------
//TGraphicsStyles implementation
//------------------------------------------------------------------------------
//..............................................................................
TGraphicsStyles::TGraphicsStyles(TGlRenderer& R) : Renderer(R)  {
  Root = new TGraphicsStyle(*this, NULL, "Root");
  Version = CurrentVersion;
}
//..............................................................................
TGraphicsStyles::~TGraphicsStyles()  {
  Clear();
  delete Root;
}
//..............................................................................
void TGraphicsStyles::Clear() {
  for (size_t i=0; i < PStyles.ObjectCount(); i++)
    delete &PStyles.GetObject(i);
  for (size_t i=0; i < PStyles.PropertiesCount(); i++)
    delete &PStyles.GetProperties(i);
  PStyles.Clear();
  Root->Clear();
  Renderer._OnStylesClear();
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
  Item.AddField("Version", CurrentVersion);
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
  size_t matc=0;
  const size_t sc = styles.Count();
  // recursion here on Count()
  for (size_t i=0; i < allStyles.Count(); i++) {
    TGraphicsStyle* gs = allStyles[i];
    if (i < sc)  // add only the top level styles
      root.AddStyle(gs);
    for (size_t j=0; j < gs->PrimitiveStyleCount(); j++ ) {
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
    for (size_t j=0; j < gs->StyleCount(); j++)  // recursion implementation
      allStyles.Add(gs->GetStyle(j));
  }
  item.AddField("Name", Name);
  item.AddField("LinkFile", LinkFile);
  item.AddField("Version", CurrentVersion);
  root.ToDataItem(item.AddItem("Root"), true);
  DataItems.Clear();
  root.ReleaseStyles();
}
//..............................................................................
bool TGraphicsStyles::FromDataItem(const TDataItem& Item, bool merge) {
  if (&Item == NULL)
    throw TInvalidArgumentException(__OlxSourceInfo, "item=NULL");
  if (!merge) Clear();
  TDataItem* SI = Item.FindItem("Materials");
  TPtrList<TGlMaterial> mats;
  for (size_t i=0; i < SI->ItemCount(); i++) {
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
  for (size_t i=0; i < mats.Count(); i++)
    delete mats[i];
  try {
    Renderer._OnStylesLoaded();
  }
  catch (const TExceptionBase &e) {
    Renderer.Clear();
    throw TFunctionFailedException(__OlxSourceInfo, e);
  }
  return true;
}
//..............................................................................
void TGraphicsStyles::CopyStyle(const olxstr &From, const olxstr &To)  {
}
//..............................................................................
TPrimitiveStyle *TGraphicsStyles::NewPrimitiveStyle(const olxstr &PName)  {
  TPrimitiveStyle *PS = new TPrimitiveStyle(PName, PStyles, *this);
  PStyles.AddObject(PS);
  return PS;
}
//..............................................................................
TPrimitiveStyle *TGraphicsStyles::NewPrimitiveStyle_(const olxstr &PName)  {
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
  if (i == InvalidIndex || DataItems[i] == NULL)
    throw TFunctionFailedException(__OlxSourceInfo, "unregistered properties");
  return DataItems[i];
}
//..............................................................................
TGlMaterial* TGraphicsStyles::GetMaterial(TDataItem& I) const {
  if( I.GetData() == NULL )  
    I.SetData( new TGlMaterial() );
  return (TGlMaterial*)I.GetData();
}
//..............................................................................
void TGraphicsStyles::DeleteStyle(TGraphicsStyle *Style)  {
  if( Style->GetParentStyle() != NULL )
    Style->GetParentStyle()->DeleteStyle(*Style);
}
//..............................................................................
bool TGraphicsStyles::LoadFromFile(const olxstr& FN, bool merge)  {
  TDataFile DF;
  if( !DF.LoadFromXLFile(FN, NULL) )  return false;
  TDataItem *DI = DF.Root().FindItem("DStyle");
  if( DI == NULL )  return false;
  FromDataItem(*DI, merge);
  return true;
}
//..............................................................................
void TGraphicsStyles::SaveToFile(const olxstr& FN)  {
  TDataFile DF;
  TDataItem& DI = DF.Root().AddItem("DStyle");
  this->ToDataItem(DI);
  DF.SaveToXLFile(FN);
}
//..............................................................................
