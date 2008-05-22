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
protected:
public:
  TwxGlScene();
  virtual ~TwxGlScene();

  TGlFont* CreateFont(const olxstr& name, void *WXFont, TGlFont *Replace=NULL, bool Bmp=true, bool FixedW=true); // takes wxFont*
  void Destroy();

  void StartSelect(int x, int y, GLuint *Bf);
  void EndSelect();

  void StartDraw();
  void EndDraw();
};

EndGxlNamespace()

#endif
#endif
//---------------------------------------------------------------------------


