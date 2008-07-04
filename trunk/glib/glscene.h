//---------------------------------------------------------------------------

#ifndef glsceneH
#define glsceneH

#include "glbase.h"
#include "estrlist.h"

#include "tptrlist.h"

#ifdef CreateFont
  #undef CreateFont
#endif

BeginGlNamespace()

/* abstarct class */
class AGlScene: public IEObject  {
private:
protected:
  class TGlRender *FParent;
  TPtrList<class TGlFont> Fonts;
public:
  AGlScene();
  virtual ~AGlScene();
  inline TGlRender *Parent()  {  return FParent; }
  /* must be called by TGlrender */
  void Parent(TGlRender *P)   {  FParent = P; }
  /* takes whatever appropriate; any other way? */
  virtual TGlFont* CreateFont(const olxstr& name, void *Data,
    TGlFont *Replace=NULL, bool Bmp=true, bool FixedW=true) = 0;
  virtual void ExportFont(const olxstr& name, const olxstr& fileName) {  return;  }
  virtual TGlFont* ImportFont(const olxstr& Name, const olxstr& fileName, short Size, 
    bool FixedWidth, TGlFont *Replace=NULL)     {  return NULL;  }
  virtual void Destroy();

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
