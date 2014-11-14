/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gl_scene_H
#define __olx_gl_scene_H
#include "glbase.h"
#include "estrlist.h"
#include "glfont.h"
#include "edict.h"
#include "library.h"
#undef CreateFont
BeginGlNamespace()

class TGlFont;
/* abstarct class */
class AGlScene: public IOlxObject {
private:
  olxstr_dict<TGlFont*> FontsDict;
  TPtrList<TGlFont> Fonts, SmallFonts;
  olxdict<std::type_info const*, size_t, TPointerComparator> FontRegistry;
  bool Enabled;
protected:
  class TGlRenderer *FParent;
  /* The function creates or replaces a font (if exists under the same name)
  */
  virtual TGlFont& DoCreateFont(TGlFont& glf, bool half_size) const = 0;
public:
  AGlScene() : FParent(NULL), Enabled(true) {}
  virtual ~AGlScene();
  TGlRenderer *Parent() { return FParent; }
  /* must be called by TGlRender */
  void Parent(TGlRenderer *P) { FParent = P;}
  /* The function creates or replaces a font (if exists under the same name)
  */
  TGlFont& CreateFont(const olxstr& name, const olxstr& fntDescription);
  // used to scale font when drawing on a scaled surface
  virtual void ScaleFonts(double scale) = 0;
  // restores the font sizes after a call to the ScaleFonts
  virtual void RestoreFontScale() = 0;
  virtual void Destroy()  {}
  /* if the font is provided, it is replaced upon successful dialog.showmodal,
  otherwise it works as font chooser (default font-fontDescription). Returned
  string is the font Id string, or empty string if the dialog is canceled
  */
  virtual olxstr ShowFontDialog(TGlFont* glf = NULL,
    const olxstr& fontDescription=EmptyString()) = 0;
  virtual void StartSelect(int x, int y, GLuint *Bf);
  // returns number of selection hits
  virtual int EndSelect();
  // returns true if successful
  virtual bool MakeCurrent() = 0;
  // if the Scene is disabled, MakeCurrent should not do anything
  DefPropBIsSet(Enabled)
  virtual void StartDraw();
  virtual void EndDraw();

  size_t FontCount() const { return Fonts.Count(); }
  TGlFont& GetSmallFont(size_t i) const {
    if( i >= SmallFonts.Count() ) {
      throw TInvalidArgumentException(__OlxSourceInfo,
        olxstr("invalid small font index: ") << i);
    }
    if( !SmallFonts[i]->IsCreated() )
      DoCreateFont(*SmallFonts[i], true);
    return *SmallFonts[i];
  }
  // the fonts is created if uninitialised
  TGlFont& GetFont(size_t i, bool use_default) const {
    TGlFont* rv = NULL;
    if( i >= Fonts.Count() )  {
      if( use_default )
        rv = &GetDefaultFont();
    }
    else
      rv = Fonts[i];
    if( rv == NULL ) {
      throw TInvalidArgumentException(__OlxSourceInfo,
        olxstr("invalid font index: ") << i);
    }
    if( !rv->IsCreated() )
      DoCreateFont(*rv, false);
    return *rv;
  }
  TGlFont& GetDefaultFont() const {
    if( Fonts.IsEmpty() )
      throw TFunctionFailedException(__OlxSourceInfo, "no fonts available");
    return GetFont(0, true);
  }
  /* this is motly for internal infrastructure calls - returned font might be
  not initialised
  */
  TGlFont& _GetFont(size_t i) const { return *Fonts[i]; }
  TGlFont* FindFont(const olxstr& name) {
    return FontsDict.Find(name, NULL);
  }
  template <class T> TGlFont& RegisterFontForType(TGlFont& fnt) {
    FontRegistry.Add(&typeid(T), fnt.GetId());
    return fnt;
  }
  template <class T> size_t FindFontIndexForType(size_t def_ind=~0) {
    return FontRegistry.Find(&typeid(T), def_ind);
  }

  class MetaFont {
  protected:
    short Size;
    bool Bold, Italic, Fixed, Underlined;
    olxstr OriginalId, FileName;
  public:
    MetaFont()
      : Size(0), Bold(false), Italic(false), Fixed(false), Underlined(false)
    {}
    virtual olxstr GetIdString() const;
    olxstr GetFileIdString() const;
    /* this function returns true if the ID is known and handler and false
    otherwise
    */
    virtual bool SetIdString(const olxstr& idstr);
    static bool IsOlexFont(const olxstr& fntId) {
      return fntId.IsEmpty()? false : fntId.CharAt(0) == '#';
    }
    static bool IsVectorFont(const olxstr& fntId) {
      return fntId.IsEmpty()? false : fntId.CharAt(0) == '@';
    }
    static olxstr BuildOlexFontId(const olxstr& fileName, short size,
      bool fixed, bool bold, bool italic);
    DefPropC(olxstr, FileName)
    DefPropBIsSet(Bold)
    DefPropBIsSet(Fixed)
    DefPropBIsSet(Italic)
    bool IsUnderlined() const {  return Underlined;  }
    DefPropP(short, Size)
  };

  void ToDataItem(TDataItem &di) const;
  void FromDataItem(const TDataItem &di);
  const_strlist ToPov() const;

  void LibMakeCurrent(TStrObjList& Cmds, const TParamList& Options,
    TMacroData& E);
  TLibrary* ExportLibrary(const olxstr& name=EmptyString());
};

EndGlNamespace()
#endif
