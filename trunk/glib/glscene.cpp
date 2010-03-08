//---------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

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
void AGlScene::StartDraw()  {  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); }
//..............................................................................
void AGlScene::EndDraw()    {  glFlush();  }
//..............................................................................
void AGlScene::StartSelect(int x, int y, GLuint *Bf)  {
  glSelectBuffer(MAXSELECT, Bf);
  glRenderMode(GL_SELECT);
  glInitNames();
  glPushName(~0);
  FParent->SetView(x, y, false, true, 1);
}
//..............................................................................
int AGlScene::EndSelect()  {
  int hits = glRenderMode(GL_RENDER);
  glFlush();
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

