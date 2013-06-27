/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef _xl_wxglsceneH
#define _xl_wxglsceneH
#include "gxbase.h"

#if defined (__WXWIDGETS__)
#include "glfont.h"
#include "glscene.h"
#include "wx/glcanvas.h"
#include "wx/wx.h"
#include "wx/image.h"
BeginGxlNamespace()

class TwxGlScene: public AGlScene {
private:
  olxstr FontsFolder;
  TIntList FontSizes;
  wxGLCanvas *Canvas;
  wxGLContext *Context;
protected:
  //olxstr ComposeIdString();
  virtual TGlFont& DoCreateFont(TGlFont& fnt, bool half_size) const;
public:
  TwxGlScene(const olxstr& fontsFolder);
  virtual ~TwxGlScene();
  // font names are seperated by the '&' char
  void ExportFont(const olxstr& name, const olxstr& fileName) const;
  // imports font from a file, if the font Name exists, replaces it
  TGlFont& ImportFont(TGlFont& fnt) const;
  // used to scale font when drawing on a scaled surface
  virtual void ScaleFonts(double scale);
  // restores the font sizes after a call to the ScaleFonts
  virtual void RestoreFontScale();

  virtual olxstr ShowFontDialog(TGlFont* glf=NULL,
    const olxstr& fontDesc=EmptyString());

  virtual void Destroy()  { AGlScene::Destroy();  }
  virtual void StartSelect(int x, int y, GLuint *Bf) {
    AGlScene::StartSelect(x, y, Bf);
  }
  virtual int EndSelect()  {  return AGlScene::EndSelect();  }
  /* the canvas must be set for this operation to succeed. 
  */
  virtual bool MakeCurrent();
  virtual void StartDraw()  {  AGlScene::StartDraw();  }
  virtual void EndDraw()  {  AGlScene::EndDraw();  }
  DefPropP(wxGLCanvas *, Canvas)
  DefPropP(wxGLContext *, Context)
  // final object (at least the constructor calls only the SetIdString of THIS object
  class MetaFont : public AGlScene::MetaFont {
  public:
    MetaFont(const olxstr& fontId)  {  SetIdString(fontId);  }
    virtual olxstr GetIdString() const;
    virtual bool SetIdString(const olxstr& idstr);
  };
};

EndGxlNamespace()
#endif // wxwidgets
#endif // guard
