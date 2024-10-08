/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gl_styles_H
#define __olx_gl_styles_H
#include "glmaterial.h"
#include "dataitem.h"
#include "estlist.h"
BeginGlNamespace()

class TGraphicsStyles;
class TGraphicsStyle;

class TPrimitiveStyle : public AGroupObject {
  TGraphicsStyles& Parent;
  olxstr Name;
protected:
  virtual AGOProperties* NewProperties() const {
    return new TGlMaterial;
  }
public:
  TPrimitiveStyle(const olxstr& name, TObjectGroup& GroupParent,
    TGraphicsStyles& parent)
    : AGroupObject(GroupParent), Parent(parent), Name(name) {}
  ~TPrimitiveStyle() { }

  const olxstr& GetName() const { return Name; }
  static const olxstr& ReadName(const TDataItem& Item) {
    return Item.FindField("PName");
  }

  TGlMaterial& SetProperties(const AGOProperties& C) {
    return dynamic_cast<TGlMaterial&>(AGroupObject::SetProperties(C));
  }
  TGlMaterial& GetProperties() const {
    return (TGlMaterial&)AGroupObject::GetProperties();
  }

  bool operator == (const TPrimitiveStyle& S) const {
    if (Name != S.GetName() || !(GetProperties() == S.GetProperties())) {
      return false;
    }
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

class TGraphicsStyle : public ACollectionItem {
  TGraphicsStyles& Parent;
  TGraphicsStyle* ParentStyle;
  olxstr Name;
  TPtrList<TPrimitiveStyle> PStyles;
  sorted::StringAssociation<TGraphicsStyle*, true> Styles;  // a sublist of the tree
  olxstr_dict<TGSParam, true> Params;  // a list of parameters
  uint16_t Level;
  bool Saveable; // if the style is saveable to dataitems
  bool Persistent; // specifies if RemovesStylesByTag can delete it
  bool New; // style is created with defaults
protected:
  // does not delete the styles
  void ReleaseStyles() { Styles.Clear(); }
  void AddStyle(TGraphicsStyle* style) {
    Styles.Add(style->GetName(), style);
  }
  TGraphicsStyle* FindLocalStyle(const olxstr& name) const {
    return Styles.Find(name, 0);
  }
  template <class T>
  TGlMaterial* FindInheritedMaterial(const T& PName, TGlMaterial* def = 0) const {
    TGraphicsStyle* gs = ParentStyle;
    while (gs != 0) {
      for (size_t i = 0; i < gs->PStyles.Count(); i++) {
        if (gs->PStyles[i]->GetName() == PName) {
          return &gs->PStyles[i]->GetProperties();
        }
      }
      gs = gs->ParentStyle;
    }
    return def;
  }
  template <class T>
  TGlMaterial* FindLocalMaterial(const T& PName, TGlMaterial* def = 0) const {
    for (size_t i = 0; i < PrimitiveStyleCount(); i++) {
      if (PStyles[i]->GetName() == PName) {
        return &PStyles[i]->GetProperties();
      }
    }
    return def;
  }
  TGlMaterial& CreatePrimitiveMaterial(const olxstr& pname, const TGlMaterial& glm);
  // used in import of old styles v < 2
  void _TrimFloats();
public:
  TGraphicsStyle(TGraphicsStyles& parent, TGraphicsStyle* parent_style,
    const olxstr& name)
    : Parent(parent),
    ParentStyle(parent_style),
    Name(name),
    Level(parent_style == 0 ? 0 : parent_style->Level + 1),
    Saveable(true),
    Persistent(false),
    New(true)
  {}

  ~TGraphicsStyle() { Clear(); }

  void Clear();

  inline const olxstr& GetName() const { return Name; }
  /* searches this and parent styles for specified primitive material, if the material
  is not found a new entry is created and assigned the def */
  template <class T>
  TGlMaterial& GetMaterial(const T& PName, const TGlMaterial& def) {
    TGlMaterial* glm = FindLocalMaterial(PName);
    if (glm != 0) {
      return *glm;
    }
    glm = FindInheritedMaterial(PName);
    return CreatePrimitiveMaterial(PName, glm == 0 ? def : *glm);
  }
  // finds primitive material index (in local primitive styles)
  template <class T> size_t IndexOfMaterial(const T& PName) const {
    for (size_t i = 0; i < PStyles.Count(); i++) {
      if (PStyles[i]->GetName() == PName) {
        return i;
      }
    }
    return InvalidIndex;
  }
  template <class T>
  TGlMaterial& SetMaterial(const T& PName, const TGlMaterial& mat) {
    for (size_t i = 0; i < PStyles.Count(); i++) {
      if (PStyles[i]->GetName() == PName) {
        return (TGlMaterial&)PStyles[i]->SetProperties(mat);
      }
    }
    return CreatePrimitiveMaterial(PName, mat);
  }


  /* tries to find primitive material in this or parent styles */
  template <class T>
  TGlMaterial* FindMaterial(const T& PName, TGlMaterial* def = 0) {
    TGlMaterial* glm = FindLocalMaterial(PName);
    return glm != 0 ? glm : FindInheritedMaterial(PName, def);
  }

  inline size_t PrimitiveStyleCount() const { return PStyles.Count(); }
  inline TPrimitiveStyle& GetPrimitiveStyle(size_t i) const {
    return *PStyles[i];
  }

  inline size_t StyleCount() const { return Styles.Count(); }
  inline TGraphicsStyle& GetStyle(size_t i) const {
    return *Styles.GetValue(i);
  }

  /* sets a parameter, if the parameters is creates, saveable is used,
  otherwise it stays as defined when the parameter was firstly created
  */
  template <class T, class VT>
  void SetParam(const T& name, const VT& val, bool saveable = false) {
    size_t ind = Params.IndexOf(name);
    if (ind != InvalidIndex) {
      Params.GetValue(ind).val = val;
    }
    else {
      Params.Add(name, TGSParam(val, saveable));
    }
  }
  /* returns value of specified parameter, if the parameter does not exist, a
  new one is created using the default value and the saveable flag.
  */
  template <class T, class T1>
  olxstr& GetParam(const T& name, const T1& defval, bool saveable = false) {
    size_t index = Params.IndexOf(name);
    if (index == InvalidIndex) {
      olxstr dv(defval);
      TGraphicsStyle* gs = ParentStyle;
      while (gs != 0) {
        index = gs->Params.IndexOf(name);
        if (index != InvalidIndex) {
          dv = gs->Params.GetValue(index).val;
          break;
        }
        gs = gs->ParentStyle;
      }
      return Params.Add(name, TGSParam(defval, saveable)).val;
    }
    Params.GetValue(index).saveable = saveable;
    return Params.GetValue(index).val;
  }
  /* convenience method, defval defines the type for the conversion,
  be careful with floats 0.0 to int will throw an exception */
  template <class T, class T1>
  T1 GetNumParam(const T& name, const T1& defval, bool saveable = false) {
    try {
      return olx_str_to_num<T1>()(GetParam(name, defval, saveable));
    }
    catch (...) {
      return defval;
    }
  }

  /* returns value of specified parameter or default value.
  Does not store the value in the style
  */
  template <class T, class T1>
  olxstr FindParam(const T& name, const T1& defval) const {
    size_t index = Params.IndexOf(name);
    if (index == InvalidIndex) {
      return defval;
    }
    return Params.GetValue(index).val;
  }
  /* convenience method */
  template <class T, class T1>
  T1 FindNumParam(const T& name, const T1& defval) const {
    try {
      return olx_str_to_num<T1>()(FindParam(name, defval));
    }
    catch (...) {
      return defval;
    }
  }

  bool operator == (const TGraphicsStyle& GS) const;

  TGraphicsStyle* FindStyle(const olxstr& CollectionName);
  // searches based on the comparison of the styles, not the pointers
  TGraphicsStyle* FindStyle(TGraphicsStyle* Style);
  // deletes by the pointer
  void DeleteStyle(TGraphicsStyle& Style) {
    const size_t index = Styles.IndexOfValue(&Style);
    if (index != InvalidIndex) {
      delete Styles.GetValue(index);
      Styles.Delete(index);
    }
  }
  //removes a style by name
  template <typename T> bool RemoveStyle(const T& name) {
    const size_t index = Styles.IndexOf(name);
    if (index != InvalidIndex) {
      delete Styles.GetValue(index);
      Styles.Delete(index);
      return true;
    }
    return false;
  }
  template <typename T> bool RemoveMaterial(const T& name) {
    const size_t index = IndexOfMaterial(name);
    if (index != InvalidIndex) {
      PStyles.Delete(index); // managed by Parent
      return true;
    }
    return false;
  }
  // if force = true, then whole path "a.b.c" will be created
  TGraphicsStyle& NewStyle(const olxstr& Name, bool Force = false);
  inline uint16_t GetLevel() const { return Level; }

  inline TGraphicsStyle* GetParentStyle() const { return ParentStyle; }

  DefPropBIsSet(Saveable);
  DefPropBIsSet(Persistent);
  DefPropBIsSet(New);
  bool IsEmpty() const {
    return Params.IsEmpty() && Styles.IsEmpty() && PStyles.IsEmpty();
  }
  void ToDataItem(TDataItem& Item, bool saveAll = false) const;
  bool FromDataItem(const TDataItem& Item);
  // sets ICollectionItem::Tag of styles to Tag
  void SetStylesTag(index_t Tag);
  // removes Styles with Style::Tag == Tag
  void RemoveStylesByTag(index_t Tag);
  // removes non-persistens styles
  void RemoveNonPersistent();
  // removes non-saveable styles
  void RemoveNonSaveable();
  //removes styles by name
  void RemoveNamedStyles(const TStrList& toks);
  friend class TGraphicsStyles;
};

class TGraphicsStyles : public IOlxObject {
  olxstr Name;
  olxstr LinkFile;
  short Version;
  ObjectGroup<TGlMaterial, TPrimitiveStyle> PStyles;
  mutable TPtrList<TDataItem> DataItems;
  TGraphicsStyle* Root;
public:
  TGraphicsStyles();
  virtual ~TGraphicsStyles();

  void Clear();
  bool LoadFromFile(const olxstr& FN, bool merge);
  void SaveToFile(const olxstr& FN);

  void Update();
  void Apply();
  // for external use
  TPrimitiveStyle* NewPrimitiveStyle(const olxstr& PName);
  // creates but does not store in this object - can be safely deleted
  TPrimitiveStyle* NewPrimitiveStyle_(const olxstr& PName);
  TPrimitiveStyle* AddPrimitiveStyle(TPrimitiveStyle*);
  TDataItem* GetDataItem(const TPrimitiveStyle* Style) const;
  TGlMaterial* GetMaterial(TDataItem& I) const;
  //
  inline TGraphicsStyle* FindStyle(const olxstr& collName) {
    return Root->FindStyle(collName);
  }
  TGraphicsStyle* FindStyle(TGraphicsStyle* Style) { return Root->FindStyle(Style); }
  TGraphicsStyle& NewStyle(const olxstr& Name, bool Force = false) {
    return Root->NewStyle(Name, Force);
  }
  void SetDefaultMaterial(const olxstr& object_name, const olxstr& p_name,
    const TGlMaterial& m);
  void DeleteStyle(TGraphicsStyle* Style);

  void CopyStyle(const olxstr& COllectionFrom, const olxstr& COllectionTo);
  DefPropC(olxstr, Name);
  DefPropC(olxstr, LinkFile);

  void ToDataItem(TDataItem& Item) const;
  bool FromDataItem(const TDataItem& Item, bool merge);
  // saves only provided styles
  void ToDataItem(TDataItem& item, const TPtrList<TGraphicsStyle>& styles);

  // sets ICollectionItem::Tag of styles to Tag
  void SetStylesTag(index_t Tag) { Root->SetStylesTag(Tag); }
  // removes Styles with Style::Tag == Tag
  void RemoveStylesByTag(index_t Tag) {
    OnClear.Enter(this);
    Root->RemoveStylesByTag(Tag);
    OnClear.Exit(this);
  }
  // removes non-persisten styles
  void RemoveNonPersistent() { Root->RemoveNonPersistent(); }
  // removes non-saveable styles
  void RemoveNonSaveable() { Root->RemoveNonSaveable(); }
  // removes named styles, case sensitive
  void RemoveNamedStyles(const olxstr& name) { Root->RemoveNamedStyles(TStrList(name, '.')); }

  short GetVersion() const { return Version; }
  TActionQueue& OnClear;
  // reads the style version (0 - no version) from a dataitem
  static int ReadStyleVersion(const TDataItem& Item) {
    return Item.FindField("Version", "0").ToInt();
  }
  // this is the current version of the styles
  static int CurrentVersion() { return 2; }
};

EndGlNamespace()
#endif
