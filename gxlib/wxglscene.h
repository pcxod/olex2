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

class TwxGlScene: public AGlScene  {
private:
  olxstr FontsFolder;
  TIntList FontSizes;
protected:
  //olxstr ComposeIdString();
public:
  TwxGlScene(const olxstr& fontsFolder);
  virtual ~TwxGlScene();

  TGlFont* CreateFont(const olxstr& name, const olxstr& fntDescription, short Flags=TGlFont::fntBmp);
  // font names are seperated by the '&' char
  void ExportFont(const olxstr& name, const olxstr& fileName);
  // imports font from a file, if the font Name exists, replaces it
  TGlFont* ImportFont(const olxstr& Name, const olxstr& fntDescription,  
    short Flags=TGlFont::fntBmp);
  // used to scale font when drawing on a scaled surface
  virtual void ScaleFonts(double scale);
  // restores the font sizes after a call to the ScaleFonts
  virtual void RestoreFontScale();
  
  virtual olxstr ShowFontDialog(TGlFont* glf=NULL, const olxstr& fontDesc=EmptyString);

  virtual void Destroy()  { AGlScene::Destroy();  }
  virtual void StartSelect(int x, int y, GLuint *Bf) {  AGlScene::StartSelect(x, y, Bf);  }
  virtual int EndSelect()  {  return AGlScene::EndSelect();  }

  virtual void StartDraw() {  AGlScene::StartDraw();  }
  virtual void EndDraw()  {  AGlScene::EndDraw();  }

  class MetaFont : public AGlScene::MetaFont {
  public:
    MetaFont(const olxstr& fontId) : AGlScene::MetaFont(fontId)  {}
    virtual olxstr GetIdString() const;
    virtual bool SetIdString(const olxstr& idstr);
  };
};

EndGxlNamespace()

#endif
#endif
//---------------------------------------------------------------------------


