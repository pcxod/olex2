//---------------------------------------------------------------------------

#ifndef stylesH
#define stylesH
#include "glbase.h"
#include "groupobj.h"
#include "dataitem.h"
#include "estlist.h"

BeginGlNamespace()

class TGraphicsStyles;
class TGraphicsStyle;

class TPrimitiveStyle: public AGroupObject  {
  olxstr FPrimitiveName;
  TGraphicsStyles* FParent;
protected:
  AGOProperties *NewProperties();
public:
  TPrimitiveStyle(const olxstr &PrimitName, TObjectGroup *GropParent, TGraphicsStyles *Parent);
  ~TPrimitiveStyle();
  const olxstr& PrimitiveName() const {  return FPrimitiveName; }

  AGOProperties * SetProperties( const AGOProperties *C);

  bool operator == (const TPrimitiveStyle &S ) const  {
    if( !(FPrimitiveName == S.PrimitiveName()) )  return false;
    if( GetProperties() != S.GetProperties() )         return false;
    return true;  // the style name can be altered by a user ...
  }
  void ToDataItem(TDataItem& Item) const;
  bool FromDataItem(const TDataItem& Item);
  friend class TGraphicsStyle;
};

struct TGSParam {
  olxstr val;
  bool saveable;
  TGSParam() : saveable(false) {}
  TGSParam(const olxstr& _val, bool _saveable=false) : val(_val), saveable(_saveable) {}
  TGSParam(const TGSParam& gsp) : val(gsp.val), saveable(gsp.saveable) {}
  TGSParam& operator = (const TGSParam& gsp) {
    val = gsp.val;
    saveable = gsp.saveable;
    return *this;
  }
};

class TGraphicsStyle: public ACollectionItem  {
  olxstr FLabel;
  TPtrList<TPrimitiveStyle> FPStyles;
  //TPtrList<TGraphicsStyle> FStyles;  // a sublist of the tree
  TSStrObjList<olxstr, TGraphicsStyle*, true> FStyles;  // a sublist of the tree
  TGraphicsStyle *FParentStyle;
  TGraphicsStyles *FParent;
  TSStrObjList<olxstr, TGSParam, true> FParams;  // a list of parameters
  int FLevel;
  bool Saveable; // if the style is saveable to dataitems
  bool Persistent; // specifies if RemovesStylesByTag can delete it
protected:
  // does not delete the styles
  void ReleaseStyles() {  FStyles.Clear();  }
  void AddStyle(TGraphicsStyle* style)  {
    FStyles.Add(style->GetLabel(), style);
  }
  TGraphicsStyle* FindLocalStyle(const olxstr& name)  {
    int i = FStyles.IndexOf(name);
    return i == -1 ? NULL : FStyles.GetObject(i);
  }
public:
  TGraphicsStyle(TGraphicsStyles *S, TGraphicsStyle *Parent, const olxstr &ALabel);
  ~TGraphicsStyle();
  void Clear();
  inline const olxstr& GetLabel() const {  return FLabel; }
  const class TGlMaterial* Material(const olxstr& PName);
  TGlMaterial* PrimitiveMaterial(const olxstr& PName, const TGlMaterial& GlM);

  template <class T> int FindMaterialIndex(const T& PName) const {
    for( int i=0; i < FPStyles.Count(); i++ ) 
      if( FPStyles[i]->PrimitiveName() == PName )
        return i;
    return -1;
  }

  inline int PrimitiveStyleCount()              const {  return FPStyles.Count(); }
  inline TPrimitiveStyle* PrimitiveStyle(int i) const {  return FPStyles[i];  }
  
  inline int StyleCount() const {  return FStyles.Count();  }
  inline TGraphicsStyle* GetStyle(int i) const {  return FStyles.GetObject(i);  }

  template <class T>
  void SetParam(const T& name, const olxstr& val, bool saveable=false) {
    int ind = FParams.IndexOf(name);
    if( ind >= 0 )  
      FParams.GetObject(ind).val = val;
    else            
      FParams.Add(name, TGSParam(val, saveable));
  }  /* returns value of specified parameter, if the parameter does not exist, a new one is created
  using the default value and the saveable flag.
  */
  template <class T, class T1>
  olxstr& GetParam(const T& name, const T1& defval, bool saveable=false) {
    int index = FParams.IndexOf(name);
    if( index == -1 )  {
      olxstr dv(defval);
      TGraphicsStyle* gs = FParentStyle;
      while( gs != NULL )  {
        index = gs->FParams.IndexOf(name);
        if( index != -1 )  {
          dv = gs->FParams.GetObject(index).val;
          break;
        }
        gs = gs->FParentStyle;
      }
      return FParams.Add(name, TGSParam(defval, saveable)).Object.val;
    }
    FParams.GetObject(index).saveable = saveable;
    return FParams.GetObject(index).val;
  }

  bool operator == (const TGraphicsStyle& GS) const;

  TGraphicsStyle* FindStyle(const olxstr& CollectionName);
  // searches based on the comparison of the styles, not the pointers
  TGraphicsStyle* FindStyle(TGraphicsStyle* Style);
  // deletes by the pointer
  void DeleteStyle(TGraphicsStyle* Style);

  TGraphicsStyle *NewStyle(const olxstr& Name, bool Force=false);
  // if force = true, then whole path "a.b.c" will be created
  inline int Level() const {  return FLevel; }

  inline TGraphicsStyle*  ParentStyle() const {  return FParentStyle; }

  DefPropB(Saveable)
  DefPropB(Persistent)

  void ToDataItem(TDataItem& Item, bool saveAll=false) const;
  bool FromDataItem(const TDataItem& Item);
  // sets ICollectionItem::Tag of styles to Tag
  void SetStylesTag(int Tag); 
  // removes Styles with Style::Tag == Tag
  void RemoveStylesByTag( int Tag); 
  // removes non-persistens styles
  void RemoveNonPersistent();
  // removes non-saveable styles
  void RemoveNonSaveable();
  //removes styles by name
  void RemoveNamedStyles(const TStrList& toks);
  friend class TGraphicsStyles;
};

class TGraphicsStyles: public IEObject  {
//  TEList *FStyles; // styles for indivisual objects, top level
  olxstr FName;
  olxstr FLinkFile;
  short Version;
  TObjectGroup *FPStyles;
  mutable TPtrList<TDataItem> FDataItems;
  TGraphicsStyle *FRoot;
  class TGlRenderer *FRender;
protected:
  void Clear();
public:
  TGraphicsStyles(TGlRenderer *R);
  virtual ~TGraphicsStyles();

  bool LoadFromFile(const olxstr &FN);
  void SaveToFile(const olxstr &FN);

  void Update();
  void Apply();
  // for extenal use
  TPrimitiveStyle *NewPrimitiveStyle(const olxstr &PName);
  TDataItem* GetDataItem(const TPrimitiveStyle* Style) const;
  TGlMaterial* GetMaterial(TDataItem& I) const;
  //
  inline TGraphicsStyle* FindStyle(const olxstr& collName)  {  
    return FRoot->FindStyle(collName);  
  }
  TGraphicsStyle* FindStyle(TGraphicsStyle *Style)  { return FRoot->FindStyle(Style);  }
  TGraphicsStyle* NewStyle(const olxstr& Name, bool Force=false)  {
    return FRoot->NewStyle(Name, Force);
  }
  void DeleteStyle(TGraphicsStyle *Style);

  void CopyStyle(const olxstr &COllectionFrom, const olxstr &COllectionTo);
  inline olxstr Name()                {  return FName; }
  inline const olxstr GetName() const {  return FName; }
  inline void Name(const olxstr &N)   { FName = N; }

  void ToDataItem(TDataItem& Item) const;
  bool FromDataItem(const TDataItem& Item);
  // saves only provided styles
  void ToDataItem(TDataItem& item, const TPtrList<TGraphicsStyle>& styles);

  inline const olxstr& LinkFile()  const {  return FLinkFile; }
  inline void LinkFile(const olxstr& F)  {  FLinkFile = F; }
  
  // sets ICollectionItem::Tag of styles to Tag
  void SetStylesTag(int Tag)  {
    FRoot->SetStylesTag(Tag);
  }
  // removes Styles with Style::Tag == Tag
  void RemoveStylesByTag( int Tag)  {
    FRoot->RemoveStylesByTag(Tag);
  }
  // removes non-persisten styles
  void RemoveNonPersistent()  {
    FRoot->RemoveNonPersistent();
  }
  // removes non-saveable styles
  void RemoveNonSaveable()  {
    FRoot->RemoveNonSaveable();
  }
  // removes named styles, case sensitive
  void RemoveNamedStyles(const olxstr& name)  {
    TStrList toks(name, '.');
    FRoot->RemoveNamedStyles(toks);
  }

  inline short GetVersion() const {  return Version;  }
};

EndGlNamespace()
#endif
