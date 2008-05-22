//---------------------------------------------------------------------------

#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "bapp.h"

// for InterlockedIncrement in
#if defined __WIN32__ && __BORLANDC__
  #include <windows.h>
  #include <winbase.h>
#endif

#include "wxglscene.h"
#include "exception.h"
#include "glfont.h"
#include "glrender.h"

//---------------------------------------------------------------------------
// AGlScene
//---------------------------------------------------------------------------
#if defined(__WXWIDGETS__)

TwxGlScene::TwxGlScene()  { ; }
//..............................................................................
TwxGlScene::~TwxGlScene()  {
  Destroy();
}
//..............................................................................
TGlFont* TwxGlScene::CreateFont(const olxstr& name, void *Data, TGlFont *ReplaceFnt, bool BmpF, bool FixedW)  {
  TGlFont *Fnt;
  wxFont Font;

  try  {  Font = *static_cast<wxFont*>(Data);  }
  catch(...)  {
    throw TInvalidArgumentException(__OlxSourceInfo, "invalid data type");
  }

  if( ReplaceFnt )  {
    Fnt = ReplaceFnt;
    Fnt->ClearData();
  }
  else
    Fnt = new TGlFont(name);

  // LINUZ port - ... native font string is system dependent...
  if( Font.GetPointSize() <= 1 )
    Font.SetPointSize(6);
    
  Fnt->IdString(Font.GetNativeFontInfoDesc().c_str());

  TEList Images;
  wxImage *Image;
  int ImageW = Font.GetPointSize()*2;
  wxMemoryDC memDC;
#ifdef __WXGTK__
  wxBitmap Bmp(ImageW, ImageW);
  //memDC.SetPen(*wxWHITE_PEN);
#else
  wxBitmap Bmp(ImageW, ImageW, 1);
#endif
  memDC.SetFont(Font);
  memDC.SetPen(*wxBLACK_PEN);
  memDC.SetBackground(*wxWHITE_BRUSH);
  memDC.SetBackgroundMode(wxSOLID);
  //memDC.SetTextBackground(*wxWHITE);
  memDC.SetTextForeground(*wxBLACK);
  for( int i=0; i < 256; i++ )  {
    memDC.SelectObject(Bmp);
    memDC.Clear();
    memDC.DrawText(wxString((olxch)i), 0, 0);
    memDC.SelectObject(wxNullBitmap);
    Image = new wxImage( Bmp.ConvertToImage() );
    Fnt->CharFromRGBArray(i, Image->GetData(), ImageW, ImageW);
    Images.Add(Image);
  }
  Fnt->CreateGlyphs(FixedW, ImageW, ImageW);
  for( int i=0; i < 256; i++ )  // to find maximum height and width
    delete (wxImage*)Images[i];
  
  if( ReplaceFnt == NULL )
    Fonts.Add(Fnt);
  return Fnt;
}
//..............................................................................
void TwxGlScene::StartDraw()
{
  AGlScene::StartDraw();
}
//..............................................................................
void TwxGlScene::EndDraw()
{
  AGlScene::EndDraw();
}
//..............................................................................
void TwxGlScene::StartSelect(int x, int y, GLuint *Bf)
{
  AGlScene::StartSelect(x, y, Bf);
}
//..............................................................................
void TwxGlScene::EndSelect()
{
  AGlScene::EndSelect();
}
//..............................................................................
void TwxGlScene::Destroy()
{
  AGlScene::Destroy();
}
//..............................................................................
//..............................................................................
#endif // end Win32 section

