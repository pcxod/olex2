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

UseGlNamespace();

//------------------------------------------------------------------------------
//TPrimitiveStyle implementation
//------------------------------------------------------------------------------
TPrimitiveStyle::TPrimitiveStyle(const olxstr &PN, 
  TObjectGroup *GroupParent, TGraphicsStyles *parent):AGroupObject(GroupParent)  {
  FPrimitiveName = PN;
  FParent = parent;
}
//..............................................................................
TPrimitiveStyle::~TPrimitiveStyle()  { }
//..............................................................................
AGOProperties *TPrimitiveStyle::NewProperties()  {
  TGlMaterial *GlM = new TGlMaterial;
  return GlM;
}
//..............................................................................
AGOProperties * TPrimitiveStyle::SetProperties(const AGOProperties *C)  {
  return AGroupObject::SetProperties(C);
}
//..............................................................................
void TPrimitiveStyle::ToDataItem(TDataItem& Item) const {
  Item.AddField("PName", PrimitiveName());
  Item.AddItem("Material", FParent->GetDataItem(this));
}
//..............................................................................
bool TPrimitiveStyle::FromDataItem(const TDataItem& Item)  {
  FPrimitiveName = Item.GetFieldValue("PName");
  TDataItem *MI = Item.FindItem("Material");
  if( MI != NULL )  {
    if( MI->ItemCount() != 0 )  {
      TGlMaterial* GlM = FParent->GetMaterial(MI->GetItem(0));
      if( GlM != NULL )  SetProperties( GlM );
    }
  }
  return true;
}
//..............................................................................
//------------------------------------------------------------------------------
//TGraphicsStyle implementation
//------------------------------------------------------------------------------
TGraphicsStyle::TGraphicsStyle(TGraphicsStyles *P, TGraphicsStyle *PS, const olxstr &ALabel)  {
  FParent = P;
  FParentStyle = PS;
  FLabel = ALabel;
  if( !PS )  {  FLevel = 0; }
  else       {  FLevel = PS->Level()+1; }
  Saveable = true;
  Persistent = false;
}
//..............................................................................
TGraphicsStyle::~TGraphicsStyle()  {
  Clear();
}
//..............................................................................
void TGraphicsStyle::Clear()  {
  for( int i =0; i < FStyles.Count(); i++ )
    delete FStyles[i];
  FStyles.Clear();
  FParams.Clear();
}
//..............................................................................
const TGlMaterial* TGraphicsStyle::Material(const olxstr &PName) {
  for( int i=0; i < FPStyles.Count(); i++ )  {
    if( FPStyles[i]->PrimitiveName() == PName )  {
      return (TGlMaterial*)FPStyles[i]->GetProperties();
    }
  }
  // have to create one then...
  TPrimitiveStyle *PS = FParent->NewPrimitiveStyle(PName);
//  PS->StyleName(FLabel + PName);
  TGlMaterial GlM;
  GlM.Mark(true); // specify that the parameter is empty
  PS->SetProperties(&GlM);
  FPStyles.Add(PS);
  ((TGlMaterial*)PS->GetProperties())->Mark(true);
  return (TGlMaterial*)PS->GetProperties();
}
//..............................................................................
TGlMaterial* TGraphicsStyle::PrimitiveMaterial(const olxstr &PName, const TGlMaterial& GlM)  {
  TPrimitiveStyle *PS=NULL;
  for( int i=0; i < PrimitiveStyleCount(); i++ )  {
    if( FPStyles[i]->PrimitiveName() == PName )  {
      PS = FPStyles[i];
      break;
    }
  }
  if( PS == NULL )  { // have to create one then...
    PS = FParent->NewPrimitiveStyle(PName);
    FPStyles.Add(PS);
  }
  return (TGlMaterial*)PS->SetProperties(&GlM);
}
//..............................................................................
bool TGraphicsStyle::operator == (const TGraphicsStyle &GS) const  {
  int pc = PrimitiveStyleCount();
  if( pc != GS.PrimitiveStyleCount() )  return false;
  for( int i=0; i < pc; i++ )  {
    if( !(*FPStyles[i] == *GS.PrimitiveStyle(i)) )  return false;
  }
  return true;
}
//..............................................................................
void TGraphicsStyle::ToDataItem(TDataItem& Item) const {
  if( !IsSaveable() )  return;
  Item.AddField("Name", FLabel);
  if( IsPersistent() )  
    Item.AddField("Persistent", TrueString);
  for( int i=0; i < FParams.Count(); i++ ) {
    if( !FParams.GetObject(i).saveable )  continue;
    Item.AddField(FParams.GetString(i), FParams.GetObject(i).val);
  }
  
  int ssc = 0;
  for( int i=0; i < FStyles.Count(); i++ )
    if( FStyles[i]->IsSaveable() )  
      ssc++;
  if( ssc != 0 )  {
    TDataItem& RI = Item.AddItem("SubStyles");
    for( int i=0; i < FStyles.Count(); i++ )  {
      if( !FStyles[i]->IsSaveable() )  continue;
      FStyles[i]->ToDataItem(RI.AddItem(olxstr("S_") <<i ));
    }
  }
  for( int i=0; i < FPStyles.Count(); i++ )
    FPStyles[i]->ToDataItem(Item.AddItem(olxstr("S") << i));
}
//..............................................................................
bool TGraphicsStyle::FromDataItem(const TDataItem& Item)  {
  TGraphicsStyle *GS;
  TPrimitiveStyle *PS;
  FLabel = Item.GetFieldValue("Name");
  SetPersistent( Item.GetFieldValue("Persistent", FalseString).ToBool() );
  int i = IsPersistent() ? 2 : 1;
  for( ; i < Item.FieldCount(); i++ )
    SetParameter(Item.FieldName(i), Item.Field(i), true );
  TDataItem* I = Item.FindItem("SubStyles");
  int off = 0;
  if( I != NULL )  {
    off = 1;
    for( i=0; i < I->ItemCount(); i++ )  {
      GS = new TGraphicsStyle(FParent, this, EmptyString);
      GS->FromDataItem(I->GetItem(i));
      FStyles.Add(GS);
    }
  }
  for( i=off; i < Item.ItemCount(); i++ )  {
    PS = FParent->NewPrimitiveStyle(EmptyString);
    PS->FromDataItem( Item.GetItem(i) );
    FPStyles.Add(PS);
  }
  return true;
}
//..............................................................................
TGraphicsStyle *TGraphicsStyle::Style(const olxstr &Name)  {
  TGraphicsStyle *AS;
  if( Name.FirstIndexOf('.') >= 0 )  {
    TStrList Toks(Name, '.');
    int index=0;
    AS = Style(Toks.String(index));
    while( AS != NULL )  {
      index++;
      if( index >= Toks.Count() )  return AS;
      AS = AS->Style(Toks.String(index));
    }
    return NULL;
  }
  else  {
    for( int i=0; i < FStyles.Count(); i++ )  {
      if( FStyles[i]->Label() == Name )  return FStyles[i];
    }
  }
  return NULL;
}
//..............................................................................
TGraphicsStyle *TGraphicsStyle::NewStyle(const olxstr &Name, bool Force)  {
  TGraphicsStyle *GS;
  if( Name.FirstIndexOf('.') >= 0 )  {
    TStrList Toks(Name, '.');
    int index=0;
    TGraphicsStyle *PrevGS=NULL;
    GS = Style(Toks.String(index));
    while( GS != NULL )  {
      index++;
      if( index >= Toks.Count() )  return GS;
      PrevGS = GS;  // save last valid value
      GS = GS->Style(Toks.String(index));
    }
    // here we have GS==NULL, and index pointing to the following
    if( PrevGS == NULL )  PrevGS = this;
    if( Force )  {
      for( int i=index; i < Toks.Count(); i++ )
        PrevGS = PrevGS->NewStyle(Toks.String(i));
      return PrevGS;
    }
    else
      return PrevGS->NewStyle(Toks.String(index));
  }
  else  {
    for( int i=0; i < FStyles.Count(); i++ )
      if( FStyles[i]->Label() == Name )
        return FStyles[i];
    return FStyles.Add( new TGraphicsStyle(FParent, this, Name) );
  }
}
//..............................................................................
TGraphicsStyle *TGraphicsStyle::FindStyle(TGraphicsStyle *style)  {
  TGraphicsStyle *GS;
  for( int i=0; i < FStyles.Count(); i++ )  {
    GS = FStyles[i];
    if( *GS == *style )  {  return GS;  }
    GS = GS->FindStyle(style);
    if( GS != NULL )  return GS;
  }
  return NULL;
}
//..............................................................................
void TGraphicsStyle::DeleteStyle(TGraphicsStyle *Style)  {
  int index = FStyles.IndexOf(Style);
  if( index >= 0 )  {
    FStyles.Delete(index);
    delete Style;
  }
}
//..............................................................................
void TGraphicsStyle::SetStylesTag(int Tag)  {  // sets TCollectionItem::Tag of styles to Tag
  SetTag(Tag);
  for( int i=0; i < FStyles.Count(); i++ )
    FStyles[i]->SetStylesTag(Tag);
}
//..............................................................................
void TGraphicsStyle::RemoveStylesByTag( int Tag)  {  // removes Styles with Style::Tag == Tag
  TGraphicsStyle *GS;
  for( int i=0; i < FStyles.Count(); i++ )  {
    GS = FStyles[i];
    if( GS->GetTag() == Tag )  {
      if( GS->FStyles.Count() != 0 )
        GS->RemoveStylesByTag(Tag);
      if( GS->FStyles.Count() == 0  && !GS->IsPersistent() )  {
        delete GS;
        FStyles[i] = NULL;
      }
    }
  }
  FStyles.Pack();
}
//..............................................................................
//------------------------------------------------------------------------------
//TGraphicsStyles implementation
//------------------------------------------------------------------------------
//..............................................................................
TGraphicsStyles::TGraphicsStyles(TGlRender *Render)  {
  FRoot = new TGraphicsStyle(this, NULL, "Root");
  FPStyles = new TObjectGroup;
  FRender = Render;
}
//..............................................................................
TGraphicsStyles::~TGraphicsStyles()  {
  Clear();
  delete FRoot;
  delete FPStyles;
}
//..............................................................................
void TGraphicsStyles::Clear()  {
  for( int i=0; i < FPStyles->ObjectCount(); i++ )
    delete FPStyles->Object(i);
  for( int i=0; i < FPStyles->PropCount(); i++ )
    delete FPStyles->Properties(i);
  FPStyles->Clear();
  FRoot->Clear();
  FRender->_OnStylesClear();
}
//..............................................................................
void TGraphicsStyles::ToDataItem(TDataItem& Item) const {
  TDataItem& SI = Item.AddItem("Materials");
  FDataItems.Clear();
  for( int i=0; i < FPStyles->PropCount(); i++ )  {
    TDataItem& SI1 = SI.AddItem(olxstr("Prop")<<i);
    ((TGlMaterial*)FPStyles->Properties(i))->ToDataItem(SI1);
    FDataItems.Add(&SI1);
  }
  Item.AddField("Name", FName);
  Item.AddField("LinkFile", FLinkFile);
  FRoot->ToDataItem(Item.AddItem("Root"));
  FDataItems.Clear();
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
  FName = Item.GetFieldValue("Name");
  FLinkFile = Item.GetFieldValue("LinkFile");
  SI = Item.FindItem("Root");
  if( SI != NULL )  
    FRoot->FromDataItem(*SI);
  for( int i=0; i < mats.Count(); i++ )
    delete mats[i];
  FRender->_OnStylesLoaded();
  return true;
}
//..............................................................................
void TGraphicsStyles::CopyStyle(const olxstr &From, const olxstr &To)  {
}
//..............................................................................
TPrimitiveStyle *TGraphicsStyles::NewPrimitiveStyle(const olxstr &PName)  {
  TPrimitiveStyle *PS = new TPrimitiveStyle(PName, FPStyles, this);
  FPStyles->AddObject(PS);
  return PS;
}
//..............................................................................
TDataItem* TGraphicsStyles::GetDataItem(const TPrimitiveStyle* Style) const {
  int i = FPStyles->IndexOf( Style->GetProperties() );
  if( i == -1 )  
    throw TFunctionFailedException(__OlxSourceInfo, "unregistered properties");
  return FDataItems[i];
}
//..............................................................................
TGlMaterial* TGraphicsStyles::GetMaterial(TDataItem& I) const {
  if( I.GetData() == NULL )  
    I.SetData( new TGlMaterial() );
  return (TGlMaterial*)I.GetData();
}
//..............................................................................
void TGraphicsStyles::DeleteStyle(TGraphicsStyle *Style)  {
  if( Style->ParentStyle() )
    Style->ParentStyle()->DeleteStyle(Style);
}
//..............................................................................
void TGraphicsStyles::SetStylesTag(int Tag)  {  // sets TCollectionItem::Tag of styles to Tag
  FRoot->SetStylesTag(Tag);
}
//..............................................................................
void TGraphicsStyles::RemoveStylesByTag( int Tag)  { // removes Styles with Style::Tag == Tag
  FRoot->RemoveStylesByTag(Tag);
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

