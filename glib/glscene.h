#ifndef __olx_gl_scene_H
#define __olx_gl_scene_H
#include "glbase.h"
#include "estrlist.h"
#include "tptrlist.h"
#include "glfont.h"

#ifdef CreateFont
  #undef CreateFont
#endif

BeginGlNamespace()

/* abstarct class */
class AGlScene: public IEObject  {
private:
protected:
  class TGlRenderer *FParent;
  TPtrList<TGlFont> Fonts;
public:
  AGlScene();
  virtual ~AGlScene();
  inline TGlRenderer *Parent()  {  return FParent; }
  /* must be called by TGlrender */
  void Parent(TGlRenderer *P)   {  FParent = P; }
  /* The function creates or replaces a font (if exists under the same name)  */
  virtual TGlFont* CreateFont(const olxstr& name, const olxstr& fntDescription, short Flags=TGlFont::fntBmp) = 0;
  // used to scale font when drawing on a scaled surface
  virtual void ScaleFonts(double scale) = 0;
  // restores the font sizes after a call to the ScaleFonts
  virtual void RestoreFontScale() = 0;
  virtual void Destroy();
  /* if the font is provided, it is replaced upon successful dialog.showmodal, otherwise
  it works as font chooser (default font-fontDescription). Returned string is the font Id string, 
  or empty string if the dialog is canceled  */
  virtual olxstr ShowFontDialog(TGlFont* glf = NULL, 
    const olxstr& fontDescription = EmptyString) = 0;
  virtual void StartSelect(int x, int y, GLuint *Bf);
  // returns number of selection hits
  virtual int EndSelect();

  virtual void StartDraw();
  virtual void EndDraw();

  inline size_t FontCount() const {  return Fonts.Count(); }
  inline TGlFont* GetFont(size_t i)  {
    return (i >= FontCount())  ? NULL : Fonts[i];
  }
  inline TGlFont* DefFont() const  {  return Fonts.IsEmpty() ? NULL : Fonts[0]; }
  TGlFont* FindFont(const olxstr& name);

  class MetaFont {
    bool Bold, Italic, Fixed, Underlined;
    short Size;
    olxstr OriginalId, FileName;
  public:
    MetaFont(const olxstr& fontId);
    virtual olxstr GetIdString() const;
    olxstr GetFileIdString() const;
    // this function returns true if the ID is known and handler and false otherwise 
    virtual bool SetIdString(const olxstr& idstr);
    static bool IsOlexFont(const olxstr& fntId) {  return fntId.IsEmpty()? false : fntId.CharAt(0) == '#';  }
    static bool IsVectorFont(const olxstr& fntId) {  return fntId.IsEmpty()? false : fntId.CharAt(0) == '@';  }
    static olxstr BuildOlexFontId(const olxstr& fileName, short size, bool fixed, bool bold, bool italic);
    DefPropC(olxstr, FileName)
    DefPropBIsSet(Bold)
    DefPropBIsSet(Fixed)
    DefPropBIsSet(Italic)
    inline bool IsUnderlined() const {  return Underlined; }
    DefPropP(short, Size)
  };
};

EndGlNamespace()
#endif
