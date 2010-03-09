//---------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//
#include "glscene.h"
#include "glrender.h"
#include "glfont.h"

UseGlNamespace();
//..............................................................................
//..............................................................................
AGlScene::AGlScene()  {
  FParent = NULL;
}
//..............................................................................
AGlScene::~AGlScene()  {
  for( size_t i=0; i < Fonts.Count(); i++ )  {
    delete Fonts[i];
  }
}
//..............................................................................
void AGlScene::StartDraw()  {  olx_gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); }
//..............................................................................
void AGlScene::EndDraw()    {  olx_gl::flush();  }
//..............................................................................
void AGlScene::StartSelect(int x, int y, GLuint *Bf)  {
  olx_gl::selectBuffer(MAXSELECT, Bf);
  olx_gl::renderMode(GL_SELECT);
  olx_gl::initNames();
  olx_gl::pushName(~0);
  FParent->SetView(x, y, false, true, 1);
}
//..............................................................................
int AGlScene::EndSelect()  {
  int hits = olx_gl::renderMode(GL_RENDER);
  olx_gl::flush();
  FParent->SetView(false, 1);
  return hits;
}
//..............................................................................
void AGlScene::Destroy()    {  return; }
//..............................................................................
TGlFont* AGlScene::FindFont(const olxstr& name)  {
  for( size_t i=0; i < Fonts.Count(); i++ )
    if( Fonts[i]->GetName().Equalsi(name) )
      return Fonts[i];
  return NULL;
}
//..............................................................................

