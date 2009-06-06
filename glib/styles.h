//---------------------------------------------------------------------------

#ifndef stylesH
#define stylesH
#include "glmaterial.h"
#include "dataitem.h"

BeginGlNamespace()

class TGraphicsStyles;
class TGraphicsStyle;

class TPrimitiveStyle: public AGroupObject  {
  olxstr Name;
  TGraphicsStyles& Parent;
protected:
  virtual AGOProperties* NewProperties() const {  return new TGlMaterial;  }
public:
  TPrimitiveStyle(const olxstr& name, TObjectGroup& GroupParent, TGraphicsStyles& parent) :
      AGroupObject(GroupParent), Parent(parent), Name(name) {  }
  ~TPrimitiveStyle() { }

  const olxstr& GetName() const {  return Name; }

  AGOProperties& SetProperties(const AGOProperties& C) {
    return AGroupObject::SetProperties(C);
  }
  TGlMaterial& GetProperties() const {  return (TGlMaterial&)AGroupObject::GetProperties();  }

  bool operator == (const TPrimitiveStyle &S ) const  {
    if( Name != S.GetName() || !(GetProperties() == S.GetProperties()) )  
      return false;
    return true; 
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
  olxstr Name;
  TPtrList<TPrimitiveStyle> PStyles;
  TSStrObjList<olxstr, TGraphicsStyle*, true> Styles;  // a sublist of the tree
  TGraphicsStyle* ParentStyle;
  TGraphicsStyles& Parent;
  TSStrObjList<olxstr, TGSParam, true> Params;  // a list of parameters
  int Level;
  bool Saveable; // if the style is saveable to dataitems
  bool Persistent; // specifies if RemovesStylesByTag can delete it
protected:
  // does not delete the styles
  void ReleaseStyles() {  Styles.Clear();  }
  void AddStyle(TGraphicsStyle* style)  {
    Styles.Add(style->GetName(), style);
  }
  TGraphicsStyle* FindLocalStyle(const olxstr& name) const {
    int i = Styles.IndexOf(name);
    return i == -1 ? NULL : Styles.GetObject(i);
  }
  template <class T> 
  TGlMaterial* FindInheritedMaterial(const T& PName, TGlMaterial* def=NULL) const {
    TGraphicsStyle* gs = ParentStyle;
    while( gs != NULL )  {
      for( int i=0; i < gs->PStyles.Count(); i++ )  {
        if( gs->PStyles[i]->GetName() == PName )
          return &gs->PStyles[i]->GetProperties();
      }
      gs = gs->ParentStyle;
    }
    return def;
  }
  template <class T> 
  TGlMaterial* FindLocalMaterial(const T&PName, TGlMaterial* def=NULL) const {
    for( int i=0; i < PrimitiveStyleCount(); i++ )  {
      if( PStyles[i]->GetName() == PName )
        return &PStyles[i]->GetProperties();
    }
    return def;
  }
  TGlMaterial& CreatePrimitiveMaterial(const olxstr& pname, const TGlMaterial& glm);
public:
  TGraphicsStyle(TGraphicsStyles& parent, TGraphicsStyle* parent_style, const olxstr& name) :
  Parent(parent), 
        ParentStyle(parent_style), 
        Name(name),
        Level(parent_style == 0 ? 0 : parent_style->Level+1),
        Saveable(true),
        Persistent(false) 
  { }

  ~TGraphicsStyle() {  Clear();  }
  
  void Clear();
 
  inline const olxstr& GetName() const {  return Name; }
  /* searches this and parent styles for specified primitive material, if the material
  is not found a new entry is created and assigned the def */
  template <class T>
  TGlMaterial& GetMaterial(const T& PName, const TGlMaterial& def) {
    TGlMaterial* glm = FindLocalMaterial(PName);
    if( glm != NULL )  return *glm;
    glm = FindInheritedMaterial(PName);
    return CreatePrimitiveMaterial( PName, glm == NULL ? def : *glm);
  }
  // finds primitive material index (in local primitive styles)
  template <class T> int IndexOfMaterial(const T& PName) const {
    for( int i=0; i < PStyles.Count(); i++ ) 
      if( PStyles[i]->GetName() == PName )
        return i;
    return -1;
  }
  template <class T> TGlMaterial& SetMaterial(const T& PName, const TGlMaterial& mat) {
    for( int i=0; i < PStyles.Count(); i++ ) 
      if( PStyles[i]->GetName() == PName )
        return (TGlMaterial&)PStyles[i]->SetProperties(mat);
    return CreatePrimitiveMaterial(PName, mat);
  }
  
  
  /* tries to find primitive material in this or parent styles */
  template <class T>
  TGlMaterial* FindMaterial(const T& PName, TGlMaterial* def=NULL) {
    TGlMaterial* glm = FindLocalMaterial(PName);
    if( glm != NULL )
      return glm;
    return FindParentMaterial(PName, def);
  }

  inline int PrimitiveStyleCount()              const {  return PStyles.Count(); }
  inline TPrimitiveStyle& GetPrimitiveStyle(int i) const {  return *PStyles[i];  }
  
  inline int StyleCount() const {  return Styles.Count();  }
  inline TGraphicsStyle& GetStyle(int i) const {  return *Styles.GetObject(i);  }

  /* sets a parameter, if the parameters is creates, saveable is used, otherwise it
  stays as defined when the parameter was firstly created */
  template <class T, class VT>
  void SetParam(const T& name, const VT& val, bool saveable=false) {
    int ind = Params.IndexOf(name);
    if( ind >= 0 )  
      Params.GetObject(ind).val = val;
    else            
      Params.Add(name, TGSParam(val, saveable));
  }  
  /* returns value of specified parameter, if the parameter does not exist, a new one is created
  using the default value and the saveable flag.  */
  template <class T, class T1>
  olxstr& GetParam(const T& name, const T1& defval, bool saveable=false) {
    int index = Params.IndexOf(name);
    if( index == -1 )  {
      olxstr dv(defval);
      TGraphicsStyle* gs = ParentStyle;
      while( gs != NULL )  {
        index = gs->Params.IndexOf(name);
        if( index != -1 )  {
          dv = gs->Params.GetObject(index).val;
          break;
        }
        gs = gs->ParentStyle;
      }
      return Params.Add(name, TGSParam(defval, saveable)).Object.val;
    }
    Params.GetObject(index).saveable = saveable;
    return Params.GetObject(index).val;
  }

  bool operator == (const TGraphicsStyle& GS) const;

  TGraphicsStyle* FindStyle(const olxstr& CollectionName);
  // searches based on the comparison of the styles, not the pointers
  TGraphicsStyle* FindStyle(TGraphicsStyle* Style);
  // deletes by the pointer
  void DeleteStyle(TGraphicsStyle& Style)  {
    const int index = Styles.IndexOfObject(&Style);
    if( index >= 0 )  {
      delete Styles.GetObject(index);
      Styles.Delete(index);
    }
  }
  TGraphicsStyle& NewStyle(const olxstr& Name, bool Force=false);
  // if force = true, then whole path "a.b.c" will be created
  inline int GetLevel() const {  return Level; }

  inline TGraphicsStyle* GetParentStyle() const {  return ParentStyle; }

  DefPropBIsSet(Saveable)
  DefPropBIsSet(Persistent)

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
  olxstr Name;
  olxstr LinkFile;
  short Version;
  ObjectGroup<TGlMaterial,TPrimitiveStyle> PStyles;
  mutable TPtrList<TDataItem> DataItems;
  TGraphicsStyle* Root;
  class TGlRenderer& Renderer;
protected:
  void Clear();
public:
  TGraphicsStyles(TGlRenderer& R);
  virtual ~TGraphicsStyles();

  bool LoadFromFile(const olxstr &FN);
  void SaveToFile(const olxstr &FN);

  void Update();
  void Apply();
  // for extenal use
  TPrimitiveStyle *NewPrimitiveStyle(const olxstr& PName);
  TDataItem* GetDataItem(const TPrimitiveStyle* Style) const;
  TGlMaterial* GetMaterial(TDataItem& I) const;
  //
  inline TGraphicsStyle* FindStyle(const olxstr& collName)  {  
    return Root->FindStyle(collName);  
  }
  TGraphicsStyle* FindStyle(TGraphicsStyle* Style)  { return Root->FindStyle(Style);  }
  TGraphicsStyle& NewStyle(const olxstr& Name, bool Force=false)  {
    return Root->NewStyle(Name, Force);
  }
  void DeleteStyle(TGraphicsStyle *Style);

  void CopyStyle(const olxstr &COllectionFrom, const olxstr &COllectionTo);
  DefPropC(olxstr, Name)
  DefPropC(olxstr, LinkFile)

  void ToDataItem(TDataItem& Item) const;
  bool FromDataItem(const TDataItem& Item);
  // saves only provided styles
  void ToDataItem(TDataItem& item, const TPtrList<TGraphicsStyle>& styles);

  // sets ICollectionItem::Tag of styles to Tag
  void SetStylesTag(int Tag)  {  Root->SetStylesTag(Tag);  }
  // removes Styles with Style::Tag == Tag
  void RemoveStylesByTag( int Tag)  {  Root->RemoveStylesByTag(Tag);  }
  // removes non-persisten styles
  void RemoveNonPersistent()  {  Root->RemoveNonPersistent();  }
  // removes non-saveable styles
  void RemoveNonSaveable()  {  Root->RemoveNonSaveable();  }
  // removes named styles, case sensitive
  void RemoveNamedStyles(const olxstr& name)  {  Root->RemoveNamedStyles(TStrList(name, '.'));  }

  inline short GetVersion() const {  return Version;  }
  // reads the style version (0 - no version) from a dataitem 
  static int ReadStyleVersion(const TDataItem& Item) {
    return Item.GetFieldValue("Version", "0").ToInt();
  }
  // this is the current version of the styles
  static const int CurrentVersion;
};

EndGlNamespace()
#endif
