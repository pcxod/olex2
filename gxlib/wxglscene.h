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
  virtual bool ShowFontDialog(TGlFont& glf);

  void Destroy();

  void StartSelect(int x, int y, GLuint *Bf);
  void EndSelect();

  void StartDraw();
  void EndDraw();

  static const olxstr OlexFontId;
};

EndGxlNamespace()

#endif
#endif
//---------------------------------------------------------------------------


