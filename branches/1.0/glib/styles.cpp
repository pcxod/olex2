//----------------------------------------------------------------------------//
// namespace TEXLib: styles
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//
#ifdef __BORLANDC__
#pragma hdrstop
#endif

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
  Name = Item.GetFieldValue("PName");
  TDataItem* MI = Item.FindItem("Material");
  if( MI != NULL )  {
    if( MI->ItemCount() != 0 )  {
      TGlMaterial* GlM = Parent.GetMaterial(MI->GetItem(0));
      if( GlM != NULL )  
        SetProperties( *GlM );
    }
  }
  return true;
}
//..............................................................................
//------------------------------------------------------------------------------
//TGraphicsStyle implementation
//------------------------------------------------------------------------------
void TGraphicsStyle::Clear()  {
  for( int i =0; i < Styles.Count(); i++ )
    delete Styles.GetObject(i);
  Styles.Clear();
  Params.Clear();
}
//..............................................................................
bool TGraphicsStyle::operator == (const TGraphicsStyle &GS) const  {
  const int pc = PrimitiveStyleCount();
  if( pc != GS.PrimitiveStyleCount() )  
    return false;
  for( int i=0; i < pc; i++ )  {
    if( !(*PStyles[i] == GS.GetPrimitiveStyle(i)) )  
      return false;
  }
  return true;
}
//..............................................................................
TGlMaterial& TGraphicsStyle::CreatePrimitiveMaterial(const olxstr& pname, const TGlMaterial& glm) {
  TPrimitiveStyle* PS = Parent.NewPrimitiveStyle(pname);
  PStyles.Add(PS);
  return (TGlMaterial&)PS->SetProperties(glm);
}
//..............................................................................
void TGraphicsStyle::ToDataItem(TDataItem& Item, bool saveAll) const {
  if( !saveAll && !IsSaveable() )  return;
  Item.AddField("Name", Name);
  if( IsPersistent() )  
    Item.AddField("Persistent", TrueString);
  for( int i=0; i < Params.Count(); i++ ) {
    if( !saveAll && !Params.GetObject(i).saveable )  continue;
    Item.AddField(Params.GetString(i), Params.GetObject(i).val);
  }
  
  int ssc = saveAll ? 1 : 0;
  const int sc = Styles.Count();
  if( !saveAll )  {
    for( int i=0; i < sc; i++ )
      if( Styles.GetObject(i)->IsSaveable() )  
        ssc++;
  }
  if( ssc != 0 )  {
    TDataItem& RI = Item.AddItem("SubStyles");
    for( int i=0; i < sc; i++ )  {
      if( !saveAll&& !Styles.GetObject(i)->IsSaveable() )  continue;
      Styles.GetObject(i)->ToDataItem(RI.AddItem(olxstr("S_") <<i ), saveAll);
    }
  }
  for( int i=0; i < PStyles.Count(); i++ )
    PStyles[i]->ToDataItem(Item.AddItem(olxstr("S") << i));
}
//..............................................................................
bool TGraphicsStyle::FromDataItem(const TDataItem& Item)  {
  TGraphicsStyle *GS;
  TPrimitiveStyle *PS;
  Name = Item.GetFieldValue("Name");
  SetPersistent( Item.GetFieldValue("Persistent", FalseString).ToBool() );
  int i = IsPersistent() ? 2 : 1;
  for( ; i < Item.FieldCount(); i++ )
    SetParam(Item.FieldName(i), Item.GetField(i), Level < 2 );
//    SetParam(Item.FieldName(i), Item.Field(i), FParent->GetVersion() > 0 );
  TDataItem* I = Item.FindItem("SubStyles");
  int off = 0;
  if( I != NULL )  {
    off = 1;
    for( i=0; i < I->ItemCount(); i++ )  {
      GS = new TGraphicsStyle(Parent, this, EmptyString);
      GS->FromDataItem(I->GetItem(i));
      Styles.Add(GS->GetName(), GS);
    }
  }
  for( i=off; i < Item.ItemCount(); i++ )  {
    PS = Parent.NewPrimitiveStyle(EmptyString);
    PS->FromDataItem( Item.GetItem(i) );
    PStyles.Add(PS);
  }
  return true;
}
//..............................................................................
TGraphicsStyle *TGraphicsStyle::FindStyle(const olxstr& Name)  {
  if( Name.FirstIndexOf('.') >= 0 )  {
    TStrList Toks(Name, '.');
    int index=0;
    TGraphicsStyle* AS = FindLocalStyle(Toks[index]);
    while( AS != NULL )  {
      index++;
      if( index >= Toks.Count() )  
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
  if( Name.FirstIndexOf('.') >= 0 )  {
    TStrList Toks(Name, '.');
    int index=0;
    TGraphicsStyle *PrevGS=NULL;
    TGraphicsStyle* GS = FindLocalStyle(Toks[index]);
    while( GS != NULL )  {
      index++;
      if( index >= Toks.Count() )  
        return *GS;
      PrevGS = GS;  // save last valid value
      GS = GS->FindLocalStyle(Toks[index]);
    }
    // here we have GS==NULL, and index pointing to the following
    if( PrevGS == NULL )  PrevGS = this;
    if( Force )  {
      for( int i=index; i < Toks.Count(); i++ )
        PrevGS = &PrevGS->NewStyle(Toks[i]);
      return *PrevGS;
    }
    else
      return PrevGS->NewStyle(Toks[index]);
  }
  else  {
    TGraphicsStyle* gs = FindLocalStyle(Name);
    return *(gs != NULL ? gs : Styles.Add(Name, new TGraphicsStyle(Parent, this, Name) ).Object);
  }
}
//..............................................................................
TGraphicsStyle *TGraphicsStyle::FindStyle(TGraphicsStyle* style)  {
  for( int i=0; i < Styles.Count(); i++ )  {
    TGraphicsStyle* GS = Styles.GetObject(i);
    if( *GS == *style )  
      return GS;
    GS = GS->FindStyle(style);
    if( GS != NULL )  
      return GS;
  }
  return NULL;
}
//..............................................................................
void TGraphicsStyle::SetStylesTag(int Tag)  {  // sets TCollectionItem::Tag of styles to Tag
  SetTag(Tag);
  const int sc = Styles.Count();
  for( int i=0; i < sc; i++ )
    Styles.GetObject(i)->SetStylesTag(Tag);
}
//..............................................................................
void TGraphicsStyle::RemoveStylesByTag(int Tag)  {  // removes Styles with Style::Tag == Tag
  bool changed = false;
  for( int i=0; i < Styles.Count(); i++ )  {
    TGraphicsStyle* GS = Styles.GetObject(i);
    if( GS->GetTag() == Tag )  {
      if( GS->Styles.Count() != 0 )
        GS->RemoveStylesByTag(Tag);
      if( GS->Styles.Count() == 0  && !GS->IsPersistent() )  {
        delete GS;
        Styles.NullItem(i);
        changed = true;
      }
    }
  }
  if( changed )
    Styles.Pack();
}
//..............................................................................
void TGraphicsStyle::RemoveNonPersistent() {
  bool changed = false;
  for( int i=0; i < Styles.Count(); i++ )  {
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
  for( int i=0; i < Styles.Count(); i++ )  {
    TGraphicsStyle* GS = Styles.GetObject(i);
    if( !GS->Styles.IsEmpty() )
      GS->RemoveNonSaveable();
    if( !GS->IsSaveable() )  {
      delete GS;
      Styles.NullItem(i);
      changed = true;
    }
  }
  if( changed )
    Styles.Pack();
}
//..............................................................................
void TGraphicsStyle::RemoveNamedStyles(const TStrList& toks)  {
  if( toks.Count() < Level )  return;
  int i = Styles.IndexOfComparable(toks[Level]);
  if( i != -1 )  {
    if( toks.Count() == Level+1 )  {
      delete Styles.GetObject(i);
      Styles.Delete(i);
    }
    else if( Styles.GetObject(i)->Styles.Count() != 0 )
      Styles.GetObject(i)->RemoveNamedStyles(toks);
  }
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
void TGraphicsStyles::Clear()  {
  for( int i=0; i < PStyles.ObjectCount(); i++ )
    delete &PStyles.GetObject(i);
  for( int i=0; i < PStyles.PropertiesCount(); i++ )
    delete &PStyles.GetProperties(i);
  PStyles.Clear();
  Root->Clear();
  Renderer._OnStylesClear();
}
//..............................................................................
void TGraphicsStyles::ToDataItem(TDataItem& Item) const {
  TDataItem& SI = Item.AddItem("Materials");
  DataItems.Clear();
  for( int i=0; i < PStyles.PropertiesCount(); i++ )  {
//    if( FPStyles->Properties(i)->ObjectCount() == 0 )  continue;
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
void TGraphicsStyles::ToDataItem(TDataItem& item, const TPtrList<TGraphicsStyle>& styles) {
  TGraphicsStyle root(*this, NULL, "Root");
  TDataItem& SI = item.AddItem("Materials");
  DataItems.Clear();
  DataItems.SetCount(PStyles.PropertiesCount());
  TPtrList<TGraphicsStyle> allStyles(styles);
  int matc=0;
  const int sc=styles.Count();
  for( int i=0; i < allStyles.Count(); i++ )  {  // recursion here on Count()
    TGraphicsStyle* gs = allStyles[i];
    if( i < sc )  // add only the top level styles
      root.AddStyle(gs);
    for( int j=0; j < gs->PrimitiveStyleCount(); j++ ) {
      const TGlMaterial& glm = gs->GetPrimitiveStyle(j).GetProperties();
      int mi = PStyles.IndexOfProperties(glm);
      if( mi == -1 )  {
        root.ReleaseStyles();
        throw TFunctionFailedException(__OlxSourceInfo, "unregistered primitive style");
      }
      if( DataItems[mi] == NULL )  {
        TDataItem& SI1 = SI.AddItem( olxstr("Prop") << matc++ );
        glm.ToDataItem(SI1);
        DataItems[mi] = &SI1;
      }
    }
    for( int j=0; j < gs->StyleCount(); j++ )  // recursion implementation
      allStyles.Add( &gs->GetStyle(j) );
  }
  item.AddField("Name", Name);
  item.AddField("LinkFile", LinkFile);
  item.AddField("Version", CurrentVersion);
  root.ToDataItem(item.AddItem("Root"), true);
  DataItems.Clear();
  root.ReleaseStyles();
}
//..............................................................................
bool TGraphicsStyles::FromDataItem(const TDataItem& Item)  {
  Clear();
  if( &Item == NULL )  throw TInvalidArgumentException(__OlxSourceInfo, "item=NULL");
  TDataItem* SI = Item.FindItem("Materials");
  TPtrList<TGlMaterial> mats;
  for( int i=0; i < SI->ItemCount(); i++ )  {
    TGlMaterial* GlM = new TGlMaterial;
    SI->GetItem(i).SetData( GlM );
    GlM->FromDataItem( SI->GetItem(i) );
    mats.Add(GlM);
  }
  Name = Item.GetFieldValue("Name");
  LinkFile = Item.GetFieldValue("LinkFile");
  Version = Item.GetFieldValue("Version", "0").ToInt();
  SI = Item.FindItem("Root");
  if( SI != NULL )  
    Root->FromDataItem(*SI);
  for( int i=0; i < mats.Count(); i++ )
    delete mats[i];
  Renderer._OnStylesLoaded();
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
TDataItem* TGraphicsStyles::GetDataItem(const TPrimitiveStyle* Style) const {
  int i = PStyles.IndexOfProperties( Style->GetProperties() );
  if( i == -1 || DataItems[i] == NULL )  
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
bool TGraphicsStyles::LoadFromFile(const olxstr &FN)  {
  TDataFile DF;
  if( !DF.LoadFromXLFile(FN, NULL) )  return false;
  TDataItem *DI = DF.Root().FindItem("DStyle");
  if( DI == NULL )  return false;
  FromDataItem(*DI);
  return true;
}
//..............................................................................
void TGraphicsStyles::SaveToFile(const olxstr &FN)  {
  TDataFile DF;
  TDataItem& DI = DF.Root().AddItem("DStyle");
  this->ToDataItem(DI);
  DF.SaveToXLFile(FN);
}
//..............................................................................

