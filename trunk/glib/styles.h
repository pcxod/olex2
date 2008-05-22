//---------------------------------------------------------------------------

#ifndef stylesH
#define stylesH
#include "glbase.h"
#include "groupobj.h"
#include "dataitem.h"
#include "estlist.h"

BeginGlNamespace()

class TGraphicsStyles;

class TPrimitiveStyle: public AGroupObject  {
  olxstr FPrimitiveName;
  class TGraphicsStyles *FParent;
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
  void ToDataItem(TDataItem *Item);
  bool FromDataItem(TDataItem *Item);
};

class TGraphicsStyle: public ACollectionItem  {
  olxstr FLabel;
  TPtrList<TPrimitiveStyle> FPStyles;
  TPtrList<TGraphicsStyle> FStyles;  // a sublist of the tree
  TGraphicsStyle *FParentStyle;
  TGraphicsStyles *FParent;
  TSStrStrList<olxstr,true> FParams;  // a list of parameters
  int FLevel;
  bool Saveable; // if the style is saveable to dataitems
  bool Persistent; // specifies if RemovesStylesByTag can delete it
protected:
public:
  TGraphicsStyle(TGraphicsStyles *S, TGraphicsStyle *Parent, const olxstr &ALabel);
  ~TGraphicsStyle();
  void Clear();
  inline const olxstr& Label() const {  return FLabel; }

  const class TGlMaterial* Material(const olxstr &PName);
  void PrimitiveMaterial(const olxstr& PName, TGlMaterial *GlM);

  inline int PrimitiveStyleCount()              const {  return FPStyles.Count(); }
  inline TPrimitiveStyle* PrimitiveStyle(int i) const {  return FPStyles[i];  }

  void SetParameter(const olxstr &Name, const olxstr& val);
  olxstr& ParameterValue(const olxstr &Name, const olxstr& defval);

  bool operator == (const TGraphicsStyle &GS) const;

  TGraphicsStyle *Style(const olxstr &CollectionName);
  TGraphicsStyle *FindStyle(TGraphicsStyle *Style);
  void DeleteStyle(TGraphicsStyle *Style);

  TGraphicsStyle *NewStyle(const olxstr& Name, bool Force=false);
  // if force = true, then whole path "a.b.c" will be created
  inline int Level() const {  return FLevel; }

  inline TGraphicsStyle*  ParentStyle(){  return FParentStyle; }

  DefPropB(Saveable)
  DefPropB(Persistent)

  void ToDataItem(TDataItem *Item);
  bool FromDataItem(TDataItem *Item);

  void SetStylesTag(int Tag); // sets ICollectionItem::Tag of styles to Tag
  void RemoveStylesByTag( int Tag); // removes Styles with Style::Tag == Tag
};

class TGraphicsStyles: public IEObject  {
//  TEList *FStyles; // styles for indivisual objects, top level
  olxstr FName;
  olxstr FLinkFile;
  TObjectGroup *FPStyles;
  TPtrList<TDataItem> FDataItems;
  TGraphicsStyle *FRoot;
  class TGlRender *FRender;
protected:
  void Clear();
public:
  TGraphicsStyles(TGlRender *R);
  virtual ~TGraphicsStyles();

  bool LoadFromFile(const olxstr &FN);
  void SaveToFile(const olxstr &FN);

  void Update();
  void Apply();
  // for extenal use
  TPrimitiveStyle *NewPrimitiveStyle(const olxstr &PName);
  TDataItem *GetDataItem(TPrimitiveStyle *Style);
  TGlMaterial* GetMaterial(TDataItem *I);
  //
  inline TGraphicsStyle* Style(const olxstr& collName)  {  return FRoot->Style(collName);  }
  TGraphicsStyle* FindStyle(TGraphicsStyle *Style)  { return FRoot->FindStyle(Style);  }
  TGraphicsStyle* NewStyle(const olxstr& Name, bool Force=false)  {
    return FRoot->NewStyle(Name, Force);
  }
  void DeleteStyle(TGraphicsStyle *Style);

  void CopyStyle(const olxstr &COllectionFrom, const olxstr &COllectionTo);
  inline olxstr Name()                {  return FName; }
  inline const olxstr GetName() const {  return FName; }
  inline void Name(const olxstr &N)   { FName = N; }

  void ToDataItem(TDataItem *Item);
  bool FromDataItem(TDataItem *Item);

  inline const olxstr& LinkFile()  const {  return FLinkFile; }
  inline void LinkFile(const olxstr& F)  {  FLinkFile = F; }

  void SetStylesTag(int Tag); // sets ICollectionItem::Tag of styles to Tag
  void RemoveStylesByTag( int Tag); // removes Styles with Style::Tag == Tag
};

EndGlNamespace()
#endif
