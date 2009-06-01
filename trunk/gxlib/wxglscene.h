#ifndef _xl_wxglsceneH
#define _xl_wxglsceneH

#include "gxbase.h"

//---------------------------------------------------------------------------
// wxWidgets dependent staff
//__________________________________________
#if defined (__WXWIDGETS__)
//#define wxUSE_EXCEPTIONS
//#define wxUSE_XML
//#define wxUSE_XML
#include "wx/glcanvas.h"
#include "wx/wx.h"
#include "wx/image.h"

#include "glfont.h"
#include "glscene.h"

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

  class MetaFont {
    bool Bold, Italic, Fixed, Underlined;
    short Size;
    olxstr OriginalId, FileName;
  public:
    MetaFont(const olxstr& fontId);
    olxstr GetIdString() const;
    olxstr GetFileIdString() const;
    void SetIdString(const olxstr& idstr);
    static bool IsOlexFont(const olxstr& fntId) {  return fntId.IsEmpty()? false : fntId.CharAt(0) == '#';  }
    static olxstr BuildOlexFontId(const olxstr& fileName, short size, bool fixed, bool bold, bool italic);
    DefPropC(olxstr, FileName)
    DefPropB(Bold)
    DefPropB(Fixed)
    DefPropB(Italic)
    inline bool IsUnderlined() const {  return Underlined; }
    DefPropP(short, Size)
  };
};

EndGxlNamespace()

#endif
#endif
//---------------------------------------------------------------------------


