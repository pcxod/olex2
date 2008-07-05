//---------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "glrender.h"
#include "glscene.h"
#include "glgroup.h"
#include "styles.h"
#include "glbackground.h"
#include "glprimitive.h"
#include "gpcollection.h"
#include "gltexture.h"
#include "library.h"

#include "bapp.h"

UseGlNamespace();
//..............................................................................
TGraphicsStyles* TGlRender::FStyles = NULL;
//..............................................................................
TGlRender::TGlRender(AGlScene *S, int width, int height)  {
  CompiledListId = -1;
  FScene = S;
  FZoom = 1;
  FViewZoom = 1;
  FScene->Parent(this);
  FBasis = new TEBasis;
  SetChanged(true);
  FWidth = width;
  FHeight = height;
  FLeft = FTop = 0;
  FPerspective = false;
  FPAngle = 1;
  LookAt(0,0,1);

  FFog = false;
  FFogType = GL_EXP;
  FFogColor = 0x7f7f7f;
  FFogDensity = 1;
  FFogStart = 0;
  FFogEnd = 10;
  CalculatedZoom = -1;

  FPrimitives = new TObjectGroup;

  FActions = new TActionQList;
  FStyles = new TGraphicsStyles(this);

  FSelection = new TGlGroup("Selection", this);
  FSelection->Selected(true);
  FSelection->Create();

  OnDraw = &FActions->NewQueue("GLDRAW");
  BeforeDraw = &FActions->NewQueue("BGLDRAW");
  OnStylesClear  = &FActions->NewQueue("DSCLEAR");
  //GraphicsStyles = FStyles;
  FBackground = new TGlBackground("Background", this, false);
  FBackground->Visible(false);
  FCeiling = new TGlBackground("Ceiling", this, true);
  FCeiling->Visible(false);
  FGlImageChanged = true; // will cause its update
  FGlImage = NULL;
  TextureManager = new TTextureManager();
  FTransluentObjects.SetIncrement(512);
  FCollections.SetIncrement(512);
  FGObjects.SetIncrement(512);
}
//..............................................................................
TGlRender::~TGlRender()  {
  Clear();
  //GraphicsStyles = NULL;
  delete FStyles;
  delete FBackground;
  delete FCeiling;
  delete FBasis;
  delete FPrimitives;
  delete FSelection;
  delete FScene;
  delete FActions;
  delete TextureManager;
}
//..............................................................................
void TGlRender::Initialise()  {
  TGlPrimitive *GlP;
  InitLights();
  for( int i=0; i < FPrimitives->ObjectCount(); i++ )  {
    GlP = (TGlPrimitive*)FPrimitives->Object(i);
    GlP->Compile();
  }
  FBackground->Create();
  FCeiling->Create();
}
//..............................................................................
void TGlRender::InitLights()  {
  glEnable(GL_LIGHTING);
  glEnable(GL_DEPTH_TEST);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
  glClearDepth(1.0);
  glDepthFunc(GL_LEQUAL);
  LightModel.Init();
}
//..............................................................................
void TGlRender::ClearPrimitives()  {
  ClearGroups();
  FListManager.ClearLists();
  if( CompiledListId != -1 )  {
    glDeleteLists(CompiledListId, 1);
    CompiledListId = -1;
  }
  for( int i=0; i < FCollections.Count(); i++ )
    delete FCollections.Object(i);
  FCollections.Clear();
  for( int i=0; i < FPrimitives->ObjectCount(); i++ )
    delete FPrimitives->Object(i);
  for( int i=0; i < FPrimitives->PropCount(); i++ )
    delete FPrimitives->Properties(i);
  FPrimitives->Clear();
  FTransluentIdentityObjects.Clear();
  FTransluentObjects.Clear();
  FIdentityObjects.Clear();
  FGObjects.Clear();
  ClearMinMax();
  ReleaseGlImage();
}
//..............................................................................
void TGlRender::Clear()  {
  FSelection->Selected(false);
  FSelection->Clear();
  for( int i=0; i < FGroups.Count(); i++ )
    delete FGroups[i];
  FGroups.Clear();
  FListManager.ClearLists();
  if( CompiledListId != -1 )  {
    glDeleteLists(CompiledListId, 1);
    CompiledListId = -1;
  }
  for( int i=0; i < FCollections.Count(); i++ )
    delete FCollections.Object(i);
  FCollections.Clear();
  for( int i=0; i < FPrimitives->ObjectCount(); i++ )
    delete FPrimitives->Object(i);
  for( int i=0; i < FPrimitives->PropCount(); i++ )
    delete FPrimitives->Properties(i);
  FPrimitives->Clear();
  FTransluentIdentityObjects.Clear();
  FTransluentObjects.Clear();
  FIdentityObjects.Clear();
  // the function automaticallt removes the objects and their properties
  FGObjects.Clear();
  ResetBasis();
  ClearMinMax();
  ReleaseGlImage();
}
//..............................................................................
void TGlRender::ReleaseGlImage()  {
  if( FGlImage != NULL )  {
    delete [] FGlImage;
    FGlImage = NULL;
  }
}
//..............................................................................
void TGlRender::UpdateGlImage()  {
  ReleaseGlImage();
  FGlImage = GetPixels();
  FGlImageHeight = FHeight;
  FGlImageWidth = FWidth;
  FGlImageChanged = false;
}
//..............................................................................
void TGlRender::ClearMinMax()  {
  FMaxV[0] = FMaxV[1] = FMaxV[2] = -100;
  FMinV[0] = FMinV[1] = FMinV[2] = +100;
  CalculatedZoom = -1;
}
//..............................................................................
void TGlRender::UpdateMaxMin( const TVPointD &Max, const TVPointD &Min)  {
  if( Max[0] > FMaxV[0] )  FMaxV[0] = Max[0];
  if( Max[1] > FMaxV[1] )  FMaxV[1] = Max[1];
  if( Max[2] > FMaxV[2] )  FMaxV[2] = Max[2];

  if( Min[0] < FMinV[0] )  FMinV[0] = Min[0];
  if( Min[1] < FMinV[1] )  FMinV[1] = Min[1];
  if( Min[2] < FMinV[2] )  FMinV[2] = Min[2];
  SetChanged( true );
}
//..............................................................................
void TGlRender::operator = (const TGlRender &G)  { ; }
//..............................................................................
void TGlRender::_OnStylesClear()  {
  OnStylesClear->Enter(this);
  for( int i=0; i < FCollections.Count(); i++ )
    FCollections.Object(i)->Style(NULL);
}
//..............................................................................
void TGlRender::_OnStylesLoaded()  {
  for( int i=0; i < FCollections.Count(); i++ )  {
    FCollections.Object(i)->Style( FStyles->NewStyle(FCollections.Object(i)->Name(), true) );
  }
  TPtrList<AGDrawObject> GO( FGObjects );
  for( int i=0; i < GO.Count(); i++ )
    GO[i]->OnPrimitivesCleared();
  ClearPrimitives();
  for( int i=0; i < GO.Count(); i++ )
    GO[i]->Create();
  OnStylesClear->Exit(this);
}
//..............................................................................
TGPCollection *TGlRender::NewCollection(const olxstr &Name)  {
  TGPCollection *GPC = new TGPCollection(this);
  GPC->Name(Name);
  FCollections.Add(Name, GPC);
  GPC->Style( FStyles->NewStyle(Name, true) );
  return GPC;
}
//..............................................................................
TGPCollection *TGlRender::FindCollection(const olxstr &Name)  {
  int ind = FCollections.IndexOfComparable(Name);
  return (ind != -1) ? FCollections.Object(ind) : NULL;
}
//..............................................................................
TGPCollection *TGlRender::Collection(int ind)  {
  return FCollections.Object(ind);
}
//..............................................................................
TGPCollection *TGlRender::CollectionX(const olxstr &Name, olxstr &CollName)  {
  if( Name.FirstIndexOf('.') != -1 )  {

    int ind = FCollections.IndexOfComparable(Name);
    if( ind != -1 )  return FCollections.Object(ind);

    TStrList Toks(Name, '.'), Toks1;
    TGPCollection *BestMatch=NULL;
    short minToks, maxMatchLevels = 0;
    for( int i=0; i < FCollections.Count(); i++ )  {
      Toks1.Clear();
      Toks1.Strtok(FCollections.GetComparable(i), '.');
      minToks = olx_min( Toks.Count(), Toks1.Count() );
      if( minToks < maxMatchLevels )  continue;
      for( int j=0; j < minToks; j++ )  {
        if( Toks.String(j) != Toks1.String(j) )  break;
        if( (j+1) > maxMatchLevels )  {
          BestMatch = FCollections.Object(i);
          maxMatchLevels = j;
        }
      }
    }
    if( BestMatch != NULL )  {
      if( Name.StartsFrom( BestMatch->Name() ) )  return BestMatch;
      CollName = Toks.String(0);
      return NULL;
    }
    CollName = Toks.String(0);
    return NULL;
  }
  else  {
    CollName = Name;
    return FindCollection(Name);
  }
}
//..............................................................................
TGlPrimitive * TGlRender::NewPrimitive()  {
  TGlPrimitive *GlP = new TGlPrimitive(FPrimitives, this);
  GlP->Id(FPrimitives->ObjectCount());
  FPrimitives->AddObject(GlP);
  return GlP;
}
//..............................................................................
void TGlRender::EnableFog(bool Set)  {
  glFogi( GL_FOG_MODE, FFogType);
  glFogf( GL_FOG_DENSITY, (float)FFogDensity);
  glFogfv( GL_FOG_COLOR, FFogColor.Data());
  glFogf( GL_FOG_START, FFogStart );
  glFogf( GL_FOG_END, FFogEnd );

  if( Set )          glEnable(GL_FOG);
  else               glDisable(GL_FOG);

  FFog = Set;
}
//..............................................................................
void TGlRender::EnablePerspective(bool Set)  {
  if( Set == FPerspective )  return;
  FPerspective = Set;
  SetChanged(true);
}
//..............................................................................
void TGlRender::SetPerspectiveAngle(double angle)  {
  if( FPerspective )  SetChanged(true);
  int ang = (int)angle;
  if( !(angle - ang) )  {
    if( !(ang%90) )  {
      if( !((ang%90)%2) )  //0, 180, ...
        ang = 1;
      else
        ang = 89; // 90, 270, ..
    }
  }
  FPAngle = (float)tan(ang*M_PI/180);
}
//..............................................................................
void TGlRender::Resize(int w, int h)  {
  Resize(0, 0, w, h, 1);
}
//..............................................................................
void TGlRender::Resize(int l, int t, int w, int h, float Zoom)  {
  FLeft = l;    FTop = t;
  FWidth = w;  FHeight = h;
  FZoom = Zoom;
  FGlImageChanged = true;
  SetChanged(true);
}
//..............................................................................
void TGlRender::SetView(short Res)  {
  SetView(0, 0, false, Res);
}
//..............................................................................
void TGlRender::SetView(int x, int y, bool Select, short Res)  {
  glViewport(FLeft*Res, FTop*Res, FWidth*Res, FHeight*Res);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  if( Select )  {
    GLint  vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    gluPickMatrix(x, FHeight-y, 2, 2, vp);
  }
  double top, right;
  if( FPerspective )  {
    top = FPAngle;
    right = top*(double)FWidth/(double)FHeight;

    glFrustum(right*FProjectionLeft, right*FProjectionRight,
              top*FProjectionTop, top*FProjectionBottom, 0.1, 10.0);//MaxZ+0.1);
  }
  else  {
    top = 1;
    right = top*(double)FWidth/(double)FHeight;
    glOrtho(right*FProjectionLeft, right*FProjectionRight,
              top*FProjectionTop, top*FProjectionBottom, 0, 10.0);
  }
  glMatrixMode(GL_MODELVIEW);
}
//..............................................................................
void TGlRender::SetBasis(bool Identity)  {
  static float Bf[4][4];
  float MaxZ, dv;
  MaxZ = (float)olx_max(fabs(FMaxV.DistanceTo(FMinV)), 0.001);
  if( !Identity )  {
    memcpy( &Bf[0][0], GetBasis().GetMData(), 12*sizeof(float));
    Bf[3][0] = Bf[3][1] = 0;
  }
  else  {
    memset(&Bf[0][0], 0, 16*sizeof(float));
    Bf[0][0] = Bf[1][1] = Bf[2][2] = 1;
  }
  // move the scene to 0
  Bf[3][2] = -MaxZ;

  Bf[3][3] = MaxZ;
  dv = (float)(GetBasis().GetZoom()/MaxZ);

  //glMatrixMode(GL_MODELVIEW);
  glLoadMatrixf(&Bf[0][0]);
  glScalef(dv, dv, dv);
  if( !Identity )  {
    glTranslated(GetBasis().GetCenter()[0], GetBasis().GetCenter()[1], GetBasis().GetCenter()[2] );
  }
}
//..............................................................................
void TGlRender::DrawObject(AGDrawObject *Object, bool DrawImage)  {
  if( DrawImage )  {
    if( GlImageChanged() )  {
//      BasicApp->Log->Info("Image updated");
      UpdateGlImage();
    }
    if( !FGlImage )  return;

//    double Scale = GetScale();
//    Scale = 1;
    Scene()->StartDraw();
    SetView();
    LoadIdentity();
//    glRasterPos3d((-FWidth/2+0.1)*Scale, (-FHeight/2)*Scale, -1);
    glRasterPos3d(-0.5*(double)FWidth/(double)FHeight, -0.5,  -1);
    glDrawPixels(
          FGlImageWidth,
          FGlImageHeight, GL_RGB, GL_UNSIGNED_BYTE, FGlImage);
  }
  if( Object != NULL )  {
//    if( !Object->Visible() )  return;
    if( Object->Deleted() )  return;

    TGPCollection *GPC = Object->Primitives();
    TGlPrimitive *GlP;
    TGlMaterial *GlM;
    // draw identity objects
    SetView();
    SetBasis(true);
    for( int i=0; i < GPC->ObjectCount(); i++ )  {
      GlP = GPC->Primitive(i);
      GlM = (TGlMaterial*)GlP->GetProperties();
      if( !GlM->GetIdentityDraw() ) continue;
      GlM->Init();
      glPushMatrix();
      if( Object->Orient(GlP) )  // the object has drawn itself
      {  glPopMatrix(); continue; }
      GlP->Draw();
    }
    // draw the rest of objects
    SetBasis();
    for( int i=0; i < GPC->ObjectCount(); i++ )  {
      GlP = GPC->Primitive(i);
      GlM = (TGlMaterial*)GlP->GetProperties();
      if( GlM->GetIdentityDraw() ) continue;
      GlM->Init();
      glPushMatrix();
      if( Object->Orient(GlP) )  // the object has drawn itself
      {  glPopMatrix(); continue; }
      GlP->Draw();
    }
  }
  if( DrawImage )  Scene()->EndDraw();
  if( DrawImage || Object )  OnDraw->Execute(this);
}
//..............................................................................
void TGlRender::Draw()  {
  static double BZoom = 0;

  if( FWidth < 50 || !TBasicApp::GetInstance()->IsMainFormVisible() )  return;
  

// check if the projection matrices have to be reinitialised..
  if( FWidth < 10 || FHeight < 10 )  return;
  if( BZoom != GetBasis().GetZoom() || IsChanged() )  {
    BZoom = GetBasis().GetZoom();
    SetView();
    SetChanged(false);
  }
  glEnable(GL_NORMALIZE);
  BeforeDraw->Execute(this);
  Scene()->StartDraw();
  //glLineWidth( (float)(0.07/GetScale()) );
  //glPointSize( (float)(0.07/GetScale()) );  
  DrawObjects(false, false);
  Scene()->EndDraw();
  FGlImageChanged = true;
  OnDraw->Execute(this);
}
//..............................................................................
void TGlRender::DrawObjects( bool SelectPrimitives, bool SelectObjects)  {
  bool Select = SelectPrimitives || SelectObjects, Pers=false;
  if( FIdentityObjects.Count() != 0 )  {
    if( FPerspective )  {
      FPerspective = false;
      SetView();
      Pers = true;
    }
    SetBasis(true);

    for( int i=0; i < FIdentityObjects.Count(); i++ )  {
      TGlMaterial* GlM = FIdentityObjects[i];
      if( !Select )    GlM->Init();
      for( int j=0; j < GlM->ObjectCount(); j++ )  {
        TGlPrimitive* GlP = (TGlPrimitive*)GlM->Object(j);
        TGPCollection* GPC = GlP->ParentCollection();
        for( int k=0; k < GPC->ObjectCount(); k++ )  {
          AGDrawObject* GDO = GPC->Object(k);
          if( !GDO->Visible() )  continue;
          if( GDO->Deleted() )  continue;
          if( GDO->Selected() ) continue;
          if( GDO->Grouped() ) continue;
          if( SelectObjects )     glLoadName(GDO->GetTag());
          if( SelectPrimitives )  glLoadName(GlP->GetTag());
          glPushMatrix();
          if( GDO->Orient(GlP) )  // the object has drawn itself
          {  glPopMatrix(); continue; }
          GlP->Draw();
          glPopMatrix();
        }
      }
    }
    if( Pers )  {
      FPerspective = true;
      SetView();
    }
    SetBasis();
  }

  if( !Select && IsCompiled() )  {
    glCallList( CompiledListId );
  }
  else  {
    for( int i=0; i < FPrimitives->PropCount(); i++ )  {
      TGlMaterial* GlM = (TGlMaterial*)FPrimitives->Properties(i);
      if( GlM->GetIdentityDraw() ) continue;  // already drawn
      if( GlM->GetTransparent() ) continue;  // will be drawn
      if( !Select )    GlM->Init();
      for( int j=0; j < GlM->ObjectCount(); j++ )  {
        TGlPrimitive* GlP = (TGlPrimitive*)GlM->Object(j);
        TGPCollection* GPC = GlP->ParentCollection();
        if( GPC == NULL )  continue;
        for( int k=0; k < GPC->ObjectCount(); k++ )  {
          AGDrawObject* GDO = GPC->Object(k);
          if( !GDO->Visible() )  continue;
          if( GDO->Deleted() )  continue;
          if( GDO->Selected() ) continue;
          if( GDO->Grouped() ) continue;
          if( SelectObjects )     glLoadName(GDO->GetTag());
          if( SelectPrimitives )  glLoadName(GlP->GetTag());
          glPushMatrix();
          if( GDO->Orient(GlP) )  // the object has drawn itself
          {  glPopMatrix(); continue; }
          GlP->Draw();
          glPopMatrix();
        }
      }
    }
  }
  for( int i=0; i < FTransluentObjects.Count(); i++ )  {
    TGlMaterial* GlM = FTransluentObjects[i];
    if( !Select )    GlM->Init();
    for( int j=0; j < GlM->ObjectCount(); j++ )  {
      TGlPrimitive* GlP = (TGlPrimitive*)GlM->Object(j);
      TGPCollection* GPC = GlP->ParentCollection();
      for( int k=0; k < GPC->ObjectCount(); k++ )  {
        AGDrawObject* GDO = GPC->Object(k);
        if( !GDO->Visible() )  continue;
        if( GDO->Deleted() )  continue;
        if( GDO->Selected() ) continue;
        if( GDO->Grouped() ) continue;
        if( SelectObjects )     glLoadName(GDO->GetTag());
        if( SelectPrimitives )  glLoadName(GlP->GetTag());
        glPushMatrix();
        if( GDO->Orient(GlP) )  // the object has drawn itself
        {  glPopMatrix(); continue; }
        GlP->Draw();
        glPopMatrix();
      }
    }
  }
  for( int i=0; i < GroupCount(); i++ )
    Group(i)->Draw(SelectPrimitives, SelectObjects);

  glPushAttrib(GL_ALL_ATTRIB_BITS);
  FSelection->Draw(SelectPrimitives, SelectObjects);
  glPopAttrib();
  if( FTransluentIdentityObjects.Count() != 0 )  {
    if( FPerspective )  {
      FPerspective = false;
      SetView();
      Pers = true;
    }
    SetBasis(true);
    for( int i=0; i < FTransluentIdentityObjects.Count(); i++ )  {
      TGlMaterial* GlM = FTransluentIdentityObjects[i];
      if( !Select )    GlM->Init();
      for( int j=0; j < GlM->ObjectCount(); j++ )  {
        TGlPrimitive* GlP = (TGlPrimitive*)GlM->Object(j);
        TGPCollection* GPC = GlP->ParentCollection();
        for( int k=0; k < GPC->ObjectCount(); k++ )  {
          AGDrawObject* GDO = GPC->Object(k);
          if( !GDO->Visible() )  continue;
          if( GDO->Deleted() )  continue;
          if( GDO->Selected() ) continue;
          if( GDO->Grouped() ) continue;
          if( SelectObjects )     glLoadName(GDO->GetTag());
          if( SelectPrimitives )  glLoadName(GlP->GetTag());
          glPushMatrix();
          if( GDO->Orient(GlP) )  // the object has drawn itself
          {  glPopMatrix(); continue; }
          GlP->Draw();
          glPopMatrix();
        }
      }
    }
    FPerspective = Pers;
  }
}
//..............................................................................
AGDrawObject* TGlRender::SelectObject(int x, int y, int depth)  {
  if( !FWidth || !FHeight )  return NULL;

  AGDrawObject *Result = NULL;
  GLuint *selectBuf = new GLuint [MAXSELECT];

  int hits;
  for( int i=0; i < GObjectCount(); i++ )
    GObject(i)->SetTag(i+1);
//  hits = GObjectCount();
//  FSelection->Tag( hits + 1);
//  for( i=0; i < GroupCount(); i++ )
//  {    Group(i)->Tag(hits + i + 2 );  }
  Scene()->StartSelect(x, y, selectBuf);
  DrawObjects(false, true);
  Scene()->EndSelect();

  hits = glRenderMode(GL_RENDER);
  if (hits >= 1)  {
    if( hits == 1 )  {
      int in = selectBuf[(hits-1)*4+3];
      if( in >=1 && in <= GObjectCount() )  Result = GObject(in-1);
    }
    else  {
      unsigned int maxz = ~0;
      int in=0;
      for( int i=0; i < hits; i++ )  {
        if( selectBuf[i*4+1] < maxz )  {
          in = i;
          maxz = selectBuf[i*4+1];
        }
      }
      if( (in-depth)*4+3 < 0 )  return NULL;
      in = selectBuf[(in-depth)*4+3] - 1;
      if( in >=0 && in < GObjectCount() )  Result = GObject(in);
    }
  }
  delete [] selectBuf;
  return Result;
}
//..............................................................................
TGlPrimitive* TGlRender::SelectPrimitive(int x, int y)
{
  if( !FWidth || !FHeight )  return NULL;

  TGlPrimitive *Result = NULL;
  GLuint *selectBuf = new GLuint [MAXSELECT];
  int i, hits;

  for( i=0; i < PrimitiveCount(); i++ )
  {
    Primitive(i)->SetTag( i+1 );
  }
  Scene()->StartSelect(x, y, selectBuf);
  DrawObjects(true, false);
  Scene()->EndSelect();

  hits = glRenderMode(GL_RENDER);
  if (hits >= 1)
  {
    if( hits == 1 )
    {
      int in = selectBuf[(hits-1)*4+3];
      if( in >=1 && in <= (PrimitiveCount()+1) )
        Result = Primitive(in-1);
    }
    else
    {
      unsigned int maxz = ~0;
      int in=0;
      for( int i=0; i < hits; i++ )
      {
        if( selectBuf[i*4+1] < maxz )
        {
          in = i;
          maxz = selectBuf[i*4+1];
        }
      }
      in = selectBuf[in*4+3];
      if( in >=1 && in <= (PrimitiveCount()+1) )
        Result = Primitive(in-1);
    }
  }

  delete [] selectBuf;
  return Result;
}
//..............................................................................
TGlGroup *TGlRender::FindObjectGroup(AGDrawObject *G)  {
  // get the topmost group
  if( G == NULL )  return NULL;
  TGlGroup *G1 = G->ParentGroup();
  if( G1 == NULL )  return NULL;
  while( G1->ParentGroup() != NULL )  {
    if( G1->ParentGroup() == FSelection )  break;
    G1 = G1->ParentGroup(); 
  }
  if( G1 == FSelection )  return NULL;
  return G1;
}
//..............................................................................
void TGlRender::Select(AGDrawObject *G)  {
  if( !G->Groupable() )  return;
  G->Selected( FSelection->Add(G));
}
//..............................................................................
void TGlRender::DeSelect(AGDrawObject *G)  {
  FSelection->Remove(G);
}
//..............................................................................
void TGlRender::InvertSelection()  {
  AGDrawObject *GDO;
  TPtrList<AGDrawObject> Selected;
  for( int i=0; i < GObjectCount(); i++ )  {
    GDO = GObject(i);
    if( !GDO->Grouped() && GDO->Visible() )  {
      if( !GDO->Selected() && GDO->Groupable())
        Selected.Add(GDO);
    }
  }
  FSelection->Selected(false);
  FSelection->Clear();
  for( int i=0; i < Selected.Count(); i++ )  {
    FSelection->Add( Selected[i] );
  }
}
//..............................................................................
void TGlRender::SelectAll(bool Select)  {
  FSelection->Selected(false);
  FSelection->Clear();
  if( Select )  {
    AGDrawObject *GDO;
    for( int i=0; i < GObjectCount(); i++ )  {
      GDO = GObject(i);
      if( !GDO->Grouped() && GDO->Visible() && GDO->Groupable() && !GDO->Deleted() )  {
        if( EsdlInstanceOf(*GDO, TGlGroup) )  {
          if( GDO == FSelection )  continue;
          bool Add = false;
          for( int j=0; j < ((TGlGroup*)GDO)->Count(); j++ )  {
            if( ((TGlGroup*)GDO)->Object(j)->Visible() )  {
              Add = true;
              break;
            }
          }
          if( Add )  FSelection->Add(GDO);
        }
        else
          FSelection->Add(GDO);
      }
    }
  }
  FSelection->Selected(true);
}
//..............................................................................
void TGlRender::ClearGroups()  {
  for( int i=0; i < FGroups.Count(); i++ )  {
    if( FGroups[i]->Selected() )  DeSelect(FGroups[i]);
    FGroups[i]->Clear();
  }
  // just in case of groups in groups
  for( int i=0; i < FGroups.Count(); i++ )
    delete FGroups[i];
  FGroups.Clear();
}
//..............................................................................
TGlGroup* TGlRender::FindGroupByName(const olxstr& colName)  {
  for( int i=0; i < FGroups.Count(); i++ )
    if( FGroups[i]->GetCollectionName() == colName )
      return FGroups[i];
  return NULL;
}
//..............................................................................
void TGlRender::ClearSelection()  {
  FSelection->Clear();
}
//..............................................................................
TGlGroup * TGlRender::GroupSelection(const olxstr& groupName)  {
  if( FSelection->Count() > 1 )  {
    TGlGroup *OS = FSelection;
    FGroups.Add(FSelection);
    FSelection = new TGlGroup("Selection", this);
    FSelection->Create();
    FSelection->Add(OS);
    // read style information for this particular group
    OS->Create(groupName);
    return OS;
  }
  return NULL;
}
//..............................................................................
void TGlRender::UnGroup(TGlGroup *OS)  {
  if( !OS->Group() )  return;
  FGroups.Remove(OS);
  if( FSelection->Contains(OS) )
    FSelection->Remove(OS);

  TPtrList<AGDrawObject> Objects;
  for( int i=0; i < OS->Count(); i++ )
    Objects.Add( OS->Object(i) );
  delete OS;  // it will reset Parent group to NULL in the objects
  for( int i=0; i < Objects.Count(); i++ )
    FSelection->Add( Objects[i] );
  FSelection->Selected(true);
}
//..............................................................................
void TGlRender::UnGroupSelection()  {
  if( FSelection->Count() >= 1 )  {
    TGlGroup *OS;
    AGDrawObject *GDO;
    for( int i=0; i < FSelection->Count(); i++ )  {
      GDO = FSelection->Object(i);
      if( GDO->Group() )  {
        OS = (TGlGroup*)GDO;
        UnGroup(OS);
      }
    }
  }
}
//..............................................................................
void TGlRender::EnableClipPlane(TGlClipPlane *P, bool v)  {
  if( v )  {
    double v[4];
    v[0] = P->Equation()[0];
    v[1] = P->Equation()[1];
    v[2] = P->Equation()[2];
    v[3] = P->Equation()[3];
    glClipPlane( P->Id(), &v[0]);
    glEnable(P->Id());
  }
  else
    glDisable(P->Id());
}
//..............................................................................
void TGlRender::SetProperties(TGlMaterial *P)  {  // tracks transluent and identity objects
  if( P->GetTransparent() && P->GetIdentityDraw() )  {
    FTransluentIdentityObjects.AddUnique(P);
    return;
  }
  if( P->GetTransparent() )  {
    FTransluentObjects.AddUnique(P);
    return;
  }
  if( P->GetIdentityDraw() )  {
    FIdentityObjects.AddUnique(P);
    return;
  }
}
//..............................................................................
void TGlRender::OnSetProperties(const TGlMaterial *P)  {  // tracks transluent and identity objects
  if( P == NULL )  return;
  if( P->ObjectCount() > 1 )  return; // the properties will not be removde
  if( P->GetTransparent() && P->GetIdentityDraw() )  {
    int index = FTransluentIdentityObjects.IndexOf(P);
    if( index != -1 )  FTransluentIdentityObjects.Delete(index);
    return;
  }
  if( P->GetTransparent() )  {
    int index = FTransluentObjects.IndexOf(P);
    if( index != -1 )  FTransluentObjects.Delete(index);
    return;
  }
  if( P->GetIdentityDraw() )  {
    int index = FIdentityObjects.IndexOf(P);
    if( index != -1 ) FIdentityObjects.Delete(index);
    return;
  }
}
//..............................................................................
void TGlRender::AddGObject(AGDrawObject *G)  {
  FGObjects.Add(G);
  if( FSceneComplete || !G->Visible() )  return;
  TVPointD MaxV, MinV;
  if( G->GetDimensions(MaxV, MinV) )  {
    UpdateMaxMin(MaxV, MinV);
    SetChanged( true );
  }
}
//..............................................................................
/*
void TGlRender::ReplacePrimitives(TEList *CurObj, TEList *NewObj)
{
  if( CurObj->Count() != NewObj->Count() )
    BasicApp->Log->Exception("TGlRender:: lists count does not much!", true);
  FPrimitives->ReplaceObjects(CurObj, NewObj);
} */
//..............................................................................
void TGlRender::RemoveCollection(TGPCollection *GP)  {
  FTransluentIdentityObjects.Clear();
  FTransluentObjects.Clear();
  FIdentityObjects.Clear();

  for( int i=0; i < PrimitiveCount(); i++ )
    Primitive(i)->SetTag(-1);
  for( int i=0; i < GP->PrimitiveCount(); i++ )
    GP->Primitive(i)->SetTag(0);
  FPrimitives->RemoveObjectsByTag(0);
  FCollections.Delete( FCollections.IndexOfObject(GP) );

  for( int i=0; i < FPrimitives->PropCount(); i++ )  {
    TGlMaterial* GlM = (TGlMaterial*)FPrimitives->Properties(i);
    if( GlM->GetTransparent() && GlM->GetIdentityDraw()  )
      FTransluentIdentityObjects.Add(GlM);
    else if( GlM->GetTransparent() )
      FTransluentObjects.Add(GlM);
    else if( GlM->GetIdentityDraw() )
      FIdentityObjects.Add(GlM);
  }
  delete GP;
}
//..............................................................................
void TGlRender::RemoveCollections(const TPtrList<TGPCollection>& Colls)  {
  if( Colls.Count() == 0 )  return;

  FTransluentIdentityObjects.Clear();
  FTransluentObjects.Clear();
  FIdentityObjects.Clear();

  for( int i=0; i < PrimitiveCount(); i++ )
    Primitive(i)->SetTag(-1);
  for( int i=0; i < Colls.Count(); i++ )  {
    for( int j=0; j < Colls[i]->PrimitiveCount(); j++ )
      Colls[i]->Primitive(j)->SetTag(0);
    FPrimitives->RemoveObjectsByTag(0);
    FCollections.Remove( FCollections.IndexOfObject(Colls[i]) );
    delete Colls[i];
  }
  for( int i=0; i < FPrimitives->PropCount(); i++ )  {
    TGlMaterial* GlM = (TGlMaterial*)FPrimitives->Properties(i);
    if( GlM->GetTransparent() && GlM->GetIdentityDraw() )  {
      FTransluentIdentityObjects.Add(GlM);
      continue;
    }
    if( GlM->GetTransparent() )   {  FTransluentObjects.Add(GlM);  continue;  }
    if( GlM->GetIdentityDraw() )  {  FIdentityObjects.Add(GlM);  continue;  }
  }
}
//..............................................................................
void TGlRender::LookAt(int x, int y, short res)  {
  FViewZoom = (float)(1.0/res);
  FProjectionLeft = (float)((double)x/(double)res - 0.5);
  FProjectionRight = (float)((double)(x+1)/(double)res - 0.5);
  FProjectionTop = (float)((double)y/(double)res - 0.5);
  FProjectionBottom = (float)((double)(y+1)/(double)res - 0.5);
  SetChanged(true);
}
//..............................................................................
char* TGlRender::GetPixels(bool useMalloc, short aligment)  {
  char *Bf;
  short extraBytes = (4-(FWidth*3)%4)%4;  //for bitmaps with 4 bytes aligment
  if( useMalloc )  {
    Bf = (char*)malloc((FWidth*3+extraBytes)*FHeight);
  }
  else  {
    Bf = new char[(FWidth*3+extraBytes)*FHeight];
  }
  if( Bf == NULL )
    throw TOutOfMemoryException(__OlxSourceInfo);
  glReadBuffer(GL_BACK);
  glPixelStorei(GL_PACK_ALIGNMENT, aligment);
  glReadPixels(0, 0, FWidth, FHeight, GL_RGB, GL_UNSIGNED_BYTE, Bf);
  return Bf;
}
//..............................................................................
void TGlRender::RemovePrimitive(int in)  {
  FPrimitives->RemoveObjectsByTag(in);
  FTransluentIdentityObjects.Clear();
  FTransluentObjects.Clear();
  FIdentityObjects.Clear();
  for( int i=0; i < FPrimitives->PropCount(); i++ )  {
    TGlMaterial* GlM = (TGlMaterial*)FPrimitives->Properties(i);
    if( GlM->GetTransparent() && GlM->GetIdentityDraw() )
    {  FTransluentIdentityObjects.Add(GlM);  continue;  }
    if( GlM->GetTransparent() )    {  FTransluentObjects.Add(GlM);  continue;  }
    if( GlM->GetIdentityDraw() )   {  FIdentityObjects.Add(GlM);  continue;  }
  }
}
//..............................................................................
void TGlRender::CleanUpStyles() // removes styles, which are not used by any collection
{
  OnStylesClear->Enter(this);
  Styles()->SetStylesTag(0);
  for( int i=0; i < FCollections.Count(); i++ )
    FCollections[i]->Style()->SetTag(1);
  Styles()->RemoveStylesByTag(0);
  OnStylesClear->Exit(this);
}
//...........TGLLISTMANAGER...................................................//
//............................................................................//
//............................................................................//
//............................................................................//
TGlListManager::TGlListManager()  {
  FInc = 10;
  FPos = 0;
}
//..............................................................................
TGlListManager::~TGlListManager()  {
  ClearLists();
}
//..............................................................................
int TGlListManager::NewList()  {
  if( FPos >= Lists.Count() )  {
    int s = glGenLists(10);
    for( int i=0; i < 10; i++ )
      Lists.Add(s+i);
  }
  return Lists[FPos ++];
}
//..............................................................................
void TGlListManager::ClearLists()  {
  for( int i=0; i < Lists.Count(); i+= FInc )
    glDeleteLists(Lists[i], FInc);
  FPos = 0;
  Lists.Clear();
}
//..............................................................................
void TGlListManager::ReserveRange(int count)  {
  ClearLists();
  int s = glGenLists(count);
  for( int i=0; i < count; i++ )
    Lists.Add( s+i );
  FPos = Lists.Count();
}
//..............................................................................
TGraphicsStyles* TGlRender::GetStyles()  {
  if( TGlRender::FStyles == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "Object is not initialised");
  return TGlRender::FStyles;
}
//..............................................................................
void TGlRender::Compile(bool v)  {
  /* remark: it works, but gives no performance boost ...

  */
  //return;
  if( v )  {
    if( CompiledListId == -1 )  {
      CompiledListId = glGenLists(1);
    }
    glNewList(CompiledListId, GL_COMPILE_AND_EXECUTE);
    for( int i=0; i < FPrimitives->PropCount(); i++ )  {
      TGlMaterial* GlM = (TGlMaterial*)FPrimitives->Properties(i);
      if( GlM->GetIdentityDraw() ) continue;  // already drawn
      if( GlM->GetTransparent() ) continue;  // will be drawn
      GlM->Init();
      for( int j=0; j < GlM->ObjectCount(); j++ )  {
        TGlPrimitive* GlP = (TGlPrimitive*)GlM->Object(j);
        TGPCollection* GPC = GlP->ParentCollection();
        if( GPC == NULL )  continue;
        for( int k=0; k < GPC->ObjectCount(); k++ )  {
          AGDrawObject* GDO = GPC->Object(k);
          if( !GDO->Visible() )  continue;
          if( GDO->Deleted() )  continue;
          if( GDO->Selected() ) continue;
          if( GDO->Grouped() ) continue;
          glPushMatrix();
          if( GDO->Orient(GlP) )  // the object has drawn itself
          {  glPopMatrix(); continue; }
          GlP->Draw();
          glPopMatrix();
        }
      }
    }
    glEndList();
  }
  else  {
    if( CompiledListId != -1 )  {
      glDeleteLists(CompiledListId, 1);
      CompiledListId = -1;
    }
  }
}
//..............................................................................
//..............................................................................
//..............................................................................

void TGlRender::LibCompile(const TStrObjList& Params, TMacroError& E)  {
  Compile( Params[0].ToBool() );
}
//..............................................................................
void TGlRender::LibPerspective(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Cmds.IsEmpty() )  {  EnablePerspective(false);  return;  }
  if( !Cmds[0].IsNumber() )  {
    E.ProcessingError(__OlxSrcInfo, "please specify a number in range [1-90]" );
    return;
  }
  float v = (float)Cmds[0].ToDouble();
  if( v < 0.5 )  v = 1;
  if( v > 180 )  v = 180;

  SetPerspectiveAngle(v);
  EnablePerspective(true);
}
//..............................................................................
void TGlRender::LibFog(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Cmds.Count() == 2 )  {
    SetFogType(GL_EXP);
    SetFogDensity( (float)Cmds[0].ToDouble() );
    SetFogColor( Cmds[0].ToInt() );
    EnableFog(true);
  }
  else
    EnableFog(false);
}
//..............................................................................
void TGlRender::LibZoom(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Cmds.IsEmpty() )  {
    SetZoom( CalcZoom()*1.3);
  }
  else if( Cmds.Count() == 1 ) {
    double zoom = GetZoom() + Cmds[0].ToDouble();
    if( zoom < 0.001 )  zoom = 0.001;
    SetZoom( zoom );
  }
  return;
}
//..............................................................................
TLibrary*  TGlRender::ExportLibrary(const olxstr& name)  {
  TLibrary* lib = new TLibrary( name.IsEmpty() ? olxstr("gl") : name);
  lib->RegisterFunction<TGlRender>( new TFunction<TGlRender>(this,  &TGlRender::LibCompile, "Compile",
    fpOne, "Compiles or decompiles the model according to the boolean parameter") );

  lib->RegisterMacro<TGlRender>( new TMacro<TGlRender>(this,  &TGlRender::LibPerspective, "Perspective",
    EmptyString, fpNone|fpOne, "Un/Sets perspective view") );
  lib->RegisterMacro<TGlRender>( new TMacro<TGlRender>(this,  &TGlRender::LibFog, "Fog",
    EmptyString, fpNone|fpTwo, "Un/Sets fog") );
  lib->RegisterMacro<TGlRender>( new TMacro<TGlRender>(this,  &TGlRender::LibZoom, "Zoom",
    EmptyString, fpNone|fpOne, "If no arguments provided - resets zoom to fit to screen, otherwise increments/\
decrements current zoom by rovided value") );

  return lib;
}
//..............................................................................

