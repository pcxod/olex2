//---------------------------------------------------------------------------

#ifndef glsceneH
#define glsceneH

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
  class TGlRender *FParent;
  TPtrList<TGlFont> Fonts;
public:
  AGlScene();
  virtual ~AGlScene();
  inline TGlRender *Parent()  {  return FParent; }
  /* must be called by TGlrender */
  void Parent(TGlRender *P)   {  FParent = P; }
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
  virtual void EndSelect();

  virtual void StartDraw();
  virtual void EndDraw();

  inline int FontCount() const {  return Fonts.Count(); }
  inline TGlFont* Font(int i)  {
    return (i < 0 || i >= FontCount())  ? NULL : Fonts[i];
  }
  /* be sure it exists */
  inline TGlFont* DefFont() const  {  return Fonts[0]; }
  TGlFont* FindFont(const olxstr& name);
};

EndGlNamespace()
#endif
