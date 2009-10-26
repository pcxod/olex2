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
#include "gltexture.h"
#include "library.h"

#include "bapp.h"

UseGlNamespace();
//..............................................................................
TGraphicsStyles* TGlRenderer::FStyles = NULL;
//..............................................................................
TGlRenderer::TGlRenderer(AGlScene *S, int width, int height) {
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

  FStyles = new TGraphicsStyles(*this);

  FSelection = new TGlGroup(*this, "Selection");
  FSelection->SetSelected(true);

  OnDraw = &TBasicApp::GetInstance().Actions().NewQueue(olxappevent_GL_DRAW);
  BeforeDraw = &TBasicApp::GetInstance().Actions().NewQueue(olxappevent_GL_BEFORE_DRAW);
  OnStylesClear  = &TBasicApp::GetInstance().Actions().NewQueue(olxappevent_GL_CLEAR_STYLES);
  //GraphicsStyles = FStyles;
  FBackground = new TGlBackground(*this, "Background", false);
  FBackground->SetVisible(false);
  FCeiling = new TGlBackground(*this, "Ceiling", true);
  FCeiling->SetVisible(false);
  FGlImageChanged = true; // will cause its update
  FGlImage = NULL;
  TextureManager = new TTextureManager();
  FTransluentObjects.SetIncrement(512);
  FCollections.SetIncrement(512);
  FGObjects.SetIncrement(512);

  FSelection->Create();
}
//..............................................................................
TGlRenderer::~TGlRenderer()  {
  Clear();
  //GraphicsStyles = NULL;
  delete FStyles;
  delete FBackground;
  delete FCeiling;
  delete FBasis;
  delete FSelection;
  delete FScene;
  delete TextureManager;
}
//..............................................................................
void TGlRenderer::Initialise()  {
  InitLights();
  for( size_t i=0; i < Primitives.ObjectCount(); i++ )
    Primitives.GetObject(i).Compile();
  FBackground->Create();
  FCeiling->Create();
  olxcstr vendor( (const char*)glGetString(GL_VENDOR) );
  ATI = vendor.StartsFrom("ATI");
}
//..............................................................................
void TGlRenderer::InitLights()  {
  glEnable(GL_LIGHTING);
  glEnable(GL_DEPTH_TEST);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
  glClearDepth(1.0);
  glDepthFunc(GL_LEQUAL);
  LightModel.Init();
}
//..............................................................................
void TGlRenderer::ClearPrimitives()  {
  ClearGroups();
  FListManager.ClearLists();
  if( CompiledListId != -1 )  {
    glDeleteLists(CompiledListId, 1);
    CompiledListId = -1;
  }
  for( size_t i=0; i < FCollections.Count(); i++ )
    delete FCollections.GetObject(i);
  FCollections.Clear();
  for( size_t i=0; i < Primitives.ObjectCount(); i++ )
    delete &Primitives.GetObject(i);
  for( size_t i=0; i < Primitives.PropertiesCount(); i++ )
    delete &Primitives.GetProperties(i);
  Primitives.Clear();
  FTransluentIdentityObjects.Clear();
  FTransluentObjects.Clear();
  FIdentityObjects.Clear();
  FGObjects.Clear();
  ClearMinMax();
  ReleaseGlImage();
}
//..............................................................................
void TGlRenderer::Clear()  {
  FSelection->SetSelected(false);
  FSelection->Clear();
  for( size_t i=0; i < FGroups.Count(); i++ )
    delete FGroups[i];
  FGroups.Clear();
  FListManager.ClearLists();
  if( CompiledListId != -1 )  {
    glDeleteLists(CompiledListId, 1);
    CompiledListId = -1;
  }
  for( size_t i=0; i < FCollections.Count(); i++ )
    delete FCollections.GetObject(i);
  FCollections.Clear();
  for( size_t i=0; i < Primitives.ObjectCount(); i++ )
    delete &Primitives.GetObject(i);
  for( size_t i=0; i < Primitives.PropertiesCount(); i++ )
    delete &Primitives.GetProperties(i);
  Primitives.Clear();
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
void TGlRenderer::ReleaseGlImage()  {
  if( FGlImage != NULL )  {
    delete [] FGlImage;
    FGlImage = NULL;
  }
}
//..............................................................................
void TGlRenderer::UpdateGlImage()  {
  ReleaseGlImage();
  FGlImage = GetPixels();
  FGlImageHeight = FHeight;
  FGlImageWidth = FWidth;
  FGlImageChanged = false;
}
//..............................................................................
void TGlRenderer::ClearMinMax()  {
  FMaxV[0] = FMaxV[1] = FMaxV[2] = -100;
  FMinV[0] = FMinV[1] = FMinV[2] = +100;
  CalculatedZoom = -1;
}
//..............................................................................
void TGlRenderer::UpdateMaxMin( const vec3d &Max, const vec3d &Min)  {
  if( Max[0] > FMaxV[0] )  FMaxV[0] = Max[0];
  if( Max[1] > FMaxV[1] )  FMaxV[1] = Max[1];
  if( Max[2] > FMaxV[2] )  FMaxV[2] = Max[2];

  if( Min[0] < FMinV[0] )  FMinV[0] = Min[0];
  if( Min[1] < FMinV[1] )  FMinV[1] = Min[1];
  if( Min[2] < FMinV[2] )  FMinV[2] = Min[2];
  SetChanged( true );
}
//..............................................................................
void TGlRenderer::operator = (const TGlRenderer &G)  { ; }
//..............................................................................
void TGlRenderer::_OnStylesClear()  {
  OnStylesClear->Enter(this);
  for( size_t i=0; i < FCollections.Count(); i++ )
    FCollections.GetObject(i)->SetStyle(NULL);
}
//..............................................................................
void TGlRenderer::_OnStylesLoaded()  {
  for( size_t i=0; i < FCollections.Count(); i++ )
    FCollections.GetObject(i)->SetStyle( &FStyles->NewStyle(FCollections.GetObject(i)->GetName(), true) );
  TPtrList<AGDrawObject> GO( FGObjects );
  for( size_t i=0; i < GO.Count(); i++ )
    GO[i]->OnPrimitivesCleared();
  ClearPrimitives();
  for( size_t i=0; i < GO.Count(); i++ )
    GO[i]->Create();
  OnStylesClear->Exit(this);
}
//..............................................................................
TGPCollection& TGlRenderer::NewCollection(const olxstr &Name)  {
  TGPCollection *GPC = FCollections.Add(Name, new TGPCollection(*this, Name)).Object;
  GPC->SetStyle( &FStyles->NewStyle(Name, true) );
  return *GPC;
}
//..............................................................................
int TGlRenderer_CollectionComparator(const olxstr& c1, const olxstr& c2) {
  const size_t l = olx_min(c1.Length(), c2.Length());
  int dc = 0;
  size_t i=0;
  for( ; i < l; i++ )  {
    if( c1.CharAt(i) != c2.CharAt(i) )  break;
    if( c1.CharAt(i) == '.' )
      dc++;
  }
  if( i == l )  {
    if( c1.Length() == c2.Length() )
      dc++;
    else {
      if( l < c1.Length() && c1.CharAt(l) == '.' )
        dc++;
      else
        dc--;
    }
  }
  return dc;
}
TGPCollection *TGlRenderer::FindCollectionX(const olxstr& Name, olxstr& CollName)  {
  const size_t di = Name.FirstIndexOf('.');
  if( di != InvalidIndex )  {
    size_t ind = FCollections.IndexOfComparable(Name);
    if( ind != InvalidIndex )  
      return FCollections.GetObject(ind);

    TGPCollection *BestMatch=NULL;
    short maxMatchLevels = 0;
    for( size_t i=0; i < FCollections.Count(); i++ )  {
      int dc = TGlRenderer_CollectionComparator(Name, FCollections.GetComparable(i));
      if( dc == 0 || dc < maxMatchLevels )  continue;
      if( BestMatch != NULL && dc == maxMatchLevels )  {  // keep the one with shortes name
        if( BestMatch->GetName().Length() > FCollections.GetComparable(i).Length() )
          BestMatch = FCollections.GetObject(i);
      }
      else
        BestMatch = FCollections.GetObject(i);
      maxMatchLevels = dc;
    }
    if( BestMatch != NULL )  {
      if( Name.StartsFrom( BestMatch->GetName() ) )  
        return BestMatch;
      CollName = Name.SubStringTo(di);
      return FindCollection(CollName);
    }
    CollName = Name.SubStringTo(di);
    return FindCollection(CollName);
  }
  else  {
    CollName = Name;
    return FindCollection(Name);
  }
}
//..............................................................................
TGlPrimitive& TGlRenderer::NewPrimitive(short type)  {
  return Primitives.AddObject(new TGlPrimitive(Primitives, *this, type));
}
//..............................................................................
void TGlRenderer::EnableFog(bool Set)  {
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
void TGlRenderer::EnablePerspective(bool Set)  {
  if( Set == FPerspective )  return;
  FPerspective = Set;
  SetChanged(true);
}
//..............................................................................
void TGlRenderer::SetPerspectiveAngle(double angle)  {
  if( FPerspective )  
    SetChanged(true);
  FPAngle = (float)tan(angle*M_PI/360);
}
//..............................................................................
void TGlRenderer::Resize(int w, int h)  {
  Resize(0, 0, w, h, 1);
}
//..............................................................................
void TGlRenderer::Resize(int l, int t, int w, int h, float Zoom)  {
  FLeft = l;    FTop = t;
  FWidth = w;  FHeight = h;
  FZoom = Zoom;
  FGlImageChanged = true;
  SetChanged(true);
}
//..............................................................................
void TGlRenderer::SetView(short Res)  {
  SetView(0, 0, false, Res);
}
//..............................................................................
void TGlRenderer::SetZoom(double V) {  
  double MaxZ = olx_max(olx_abs(FMaxV.DistanceTo(FMinV)), 1);
  double dv = V/MaxZ;
  if( dv < 0.01 )  //  need to fix the zoom
    FBasis->SetZoom( MaxZ*0.01);
  else
    FBasis->SetZoom(V); 
}
//..............................................................................
void TGlRenderer::SetView(int x, int y, bool Select, short Res)  {
  glViewport(FLeft*Res, FTop*Res, FWidth*Res, FHeight*Res);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  if( Select )  {
    GLint  vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    gluPickMatrix(x, FHeight-y, 2, 2, vp);
  }
  if( FPerspective )  {
    double top = FPAngle;
    double right = top*(double)FWidth/(double)FHeight;
    glFrustum(right*FProjectionLeft, right*FProjectionRight,
              top*FProjectionTop, top*FProjectionBottom, 1, 10.0);//MaxZ+0.1);
  }
  else  {
    double top = 1;
    double right = top*(double)FWidth/(double)FHeight;
    glOrtho(right*FProjectionLeft, right*FProjectionRight,
              top*FProjectionTop, top*FProjectionBottom, 0, 10.0);
  }
  glMatrixMode(GL_MODELVIEW);
}
//..............................................................................
void TGlRenderer::SetBasis(bool Identity)  {
  static float Bf[4][4];
  float MaxZ = (float)olx_max(olx_abs(FMaxV.DistanceTo(FMinV)), 1);
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
  if( FPerspective )  {
    if( Identity ) // heh? any better?
      Bf[3][2] *= 5;
    Bf[3][2] -= MaxZ;
  }
  Bf[3][3] = MaxZ;
  glLoadMatrixf(&Bf[0][0]);
  float dv = (float)(GetBasis().GetZoom()/MaxZ);
  glScalef(dv, dv, dv);
  if( !Identity )
    glTranslated(GetBasis().GetCenter()[0], GetBasis().GetCenter()[1], GetBasis().GetCenter()[2] );
}
//..............................................................................
void TGlRenderer::DrawObject(AGDrawObject *Object, bool DrawImage)  {
  if( DrawImage )  {
    if( GlImageChanged() )  {
//      BasicApp->Log->Info("Image updated");
      UpdateGlImage();
    }
    if( !FGlImage )  return;

//    double Scale = GetScale();
//    Scale = 1;
    GetScene().StartDraw();
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
    if( Object->IsDeleted() )  return;

    TGPCollection& GPC = Object->GetPrimitives();
    // draw identity objects
    SetView();
    SetBasis(true);
    for( size_t i=0; i < GPC.ObjectCount(); i++ )  {
      TGlPrimitive& GlP = GPC.GetPrimitive(i);
      const TGlMaterial& GlM = GlP.GetProperties();
      if( !GlM.IsIdentityDraw() ) continue;
      GlM.Init();
      glPushMatrix();
      if( Object->Orient(GlP) )  // the object has drawn itself
      {  glPopMatrix(); continue; }
      GlP.Draw();
    }
    // draw the rest of objects
    SetBasis();
    for( size_t i=0; i < GPC.ObjectCount(); i++ )  {
      TGlPrimitive& GlP = GPC.GetPrimitive(i);
      const TGlMaterial& GlM = GlP.GetProperties();
      if( GlM.IsIdentityDraw() ) continue;
      GlM.Init();
      glPushMatrix();
      if( Object->Orient(GlP) )  // the object has drawn itself
      {  glPopMatrix(); continue; }
      GlP.Draw();
    }
  }
  if( DrawImage )  
    GetScene().EndDraw();
  if( DrawImage || Object )  
    OnDraw->Execute(this);
}
//..............................................................................
void TGlRenderer::Draw()  {
  static double BZoom = 0;

  if( FWidth < 50 || !TBasicApp::GetInstance().IsMainFormVisible() )  return;
  

// check if the projection matrices have to be reinitialised..
  if( FWidth < 10 || FHeight < 10 )  return;
  if( BZoom != GetBasis().GetZoom() || IsChanged() )  {
    BZoom = GetBasis().GetZoom();
    SetView();
    SetChanged(false);
  }
  glEnable(GL_NORMALIZE);
  BeforeDraw->Execute(this);
  GetScene().StartDraw();
  //glLineWidth( (float)(0.07/GetScale()) );
  //glPointSize( (float)(0.07/GetScale()) );  
  DrawObjects(0, 0, false, false);
  GetScene().EndDraw();
  FGlImageChanged = true;
  OnDraw->Execute(this);
}
//..............................................................................
void TGlRenderer::DrawObjects(int x, int y, bool SelectPrimitives, bool SelectObjects)  {
  const bool Select = SelectPrimitives || SelectObjects;
  const int DrawMask = sgdoVisible|sgdoSelected|sgdoDeleted|sgdoGrouped;
  if( !FIdentityObjects.IsEmpty() )  {
    if( FPerspective )  {
      FPerspective = false;
      SetView(x, y, Select);
      FPerspective = true;
    }
    SetBasis(true);
    const size_t id_obj_count = FIdentityObjects.Count();
    for( size_t i=0; i < id_obj_count; i++ )  {
      TGlMaterial* GlM = FIdentityObjects[i];
      if( !Select )    GlM->Init();
      const size_t obj_count = GlM->ObjectCount();
      for( size_t j=0; j < obj_count; j++ )  {
        TGlPrimitive& GlP = (TGlPrimitive&)GlM->GetObject(j);
        TGPCollection* GPC = GlP.GetParentCollection();
        const size_t c_obj_count = GPC->ObjectCount();
        for( size_t k=0; k < c_obj_count; k++ )  {
          AGDrawObject& GDO = GPC->GetObject(k);
          if( GDO.MaskFlags(DrawMask) != sgdoVisible )  continue;
          if( SelectObjects )  glLoadName((GLuint)GDO.GetTag());
          if( SelectPrimitives )  glLoadName((GLuint)GlP.GetTag());
          glPushMatrix();
          if( GDO.Orient(GlP) )  // the object has drawn itself
          {  glPopMatrix(); continue; }
          GlP.Draw();
          glPopMatrix();
        }
      }
    }
    if( FPerspective )
      SetView(x, y, Select);
    SetBasis();
  }

  if( !Select && IsCompiled() )  {
    glCallList( CompiledListId );
  }
  else  {
    const size_t prim_count = Primitives.PropertiesCount();
    for( size_t i=0; i < prim_count; i++ )  {
      TGlMaterial& GlM = Primitives.GetProperties(i);
      if( GlM.IsIdentityDraw() ) continue;  // already drawn
      if( GlM.IsTransparent() ) continue;  // will be drawn
      if( !Select )    
        GlM.Init();
      const size_t obj_count = GlM.ObjectCount();
      for( size_t j=0; j < obj_count; j++ )  {
        TGlPrimitive& GlP = (TGlPrimitive&)GlM.GetObject(j);
        TGPCollection* GPC = GlP.GetParentCollection();
        if( GPC == NULL )  continue;
        const size_t c_obj_count = GPC->ObjectCount();
        for( size_t k=0; k < c_obj_count; k++ )  {
          AGDrawObject& GDO = GPC->GetObject(k);
          if( GDO.MaskFlags(DrawMask) != sgdoVisible )  continue;
          if( SelectObjects )  glLoadName((GLuint)GDO.GetTag());
          if( SelectPrimitives )  glLoadName((GLuint)GlP.GetTag());
          glPushMatrix();
          if( GDO.Orient(GlP) )  // the object has drawn itself
          {  glPopMatrix(); continue; }
          GlP.Draw();
          glPopMatrix();
        }
      }
    }
  }
  const size_t trans_obj_count = FTransluentObjects.Count();
  for( size_t i=0; i < trans_obj_count; i++ )  {
    TGlMaterial* GlM = FTransluentObjects[i];
    if( !Select )    
      GlM->Init();
    const size_t obj_count = GlM->ObjectCount();
    for( size_t j=0; j < obj_count; j++ )  {
      TGlPrimitive& GlP = (TGlPrimitive&)GlM->GetObject(j);
      TGPCollection* GPC = GlP.GetParentCollection();
      const size_t c_obj_count = GPC->ObjectCount();
      for( size_t k=0; k < c_obj_count; k++ )  {
        AGDrawObject& GDO = GPC->GetObject(k);
        if( GDO.MaskFlags(DrawMask) != sgdoVisible )  continue;
        if( SelectObjects )  glLoadName((GLuint)GDO.GetTag());
        if( SelectPrimitives )  glLoadName((GLuint)GlP.GetTag());
        glPushMatrix();
        if( GDO.Orient(GlP) )  // the object has drawn itself
        {  glPopMatrix(); continue; }
        GlP.Draw();
        glPopMatrix();
      }
    }
  }
  const size_t group_count = FGroups.Count();
  for( size_t i=0; i < group_count; i++ )
    FGroups[i]->Draw(SelectPrimitives, SelectObjects);

  glPushAttrib(GL_ALL_ATTRIB_BITS);
  FSelection->Draw(SelectPrimitives, SelectObjects);
  glPopAttrib();
  if( !FTransluentIdentityObjects.IsEmpty() )  {
    if( FPerspective )  {
      FPerspective = false;
      SetView(x, y, Select);
      FPerspective = true;
    }
    SetBasis(true);
    const size_t trans_id_obj_count = FTransluentIdentityObjects.Count();
    for( size_t i=0; i < trans_id_obj_count; i++ )  {
      TGlMaterial* GlM = FTransluentIdentityObjects[i];
      if( !Select )    GlM->Init();
      const size_t obj_count = GlM->ObjectCount(); 
      for( size_t j=0; j < obj_count; j++ )  {
        TGlPrimitive& GlP = (TGlPrimitive&)GlM->GetObject(j);
        TGPCollection* GPC = GlP.GetParentCollection();
        const size_t c_obj_count = GPC->ObjectCount();
        for( size_t k=0; k < c_obj_count; k++ )  {
          AGDrawObject& GDO = GPC->GetObject(k);
          if( GDO.MaskFlags(DrawMask) != sgdoVisible )  continue;
          if( SelectObjects )  glLoadName((GLuint)GDO.GetTag());
          if( SelectPrimitives )  glLoadName((GLuint)GlP.GetTag());
          glPushMatrix();
          if( GDO.Orient(GlP) )  // the object has drawn itself
          {  glPopMatrix(); continue; }
          GlP.Draw();
          glPopMatrix();
        }
      }
    }
    if( FPerspective )
      SetView(x, y, Select);
  }
}
//..............................................................................
AGDrawObject* TGlRenderer::SelectObject(int x, int y, int depth)  {
  if( FWidth*FHeight == 0 )  
    return NULL;

  AGDrawObject *Result = NULL;
  GLuint *selectBuf = new GLuint [MAXSELECT];

  for( size_t i=0; i < ObjectCount(); i++ )
    GetObject(i).SetTag((int)(i+1));
  GetScene().StartSelect(x, y, selectBuf);
  DrawObjects(x, y, false, true);
  int hits = GetScene().EndSelect();
  if (hits >= 1)  {
    if( hits == 1 )  {
      GLuint in = selectBuf[(hits-1)*4+3];
      if( in >=1 && in <= ObjectCount() )  
        Result = &GetObject(in-1);
    }
    else  {
      unsigned int maxz = ~0;
      GLuint in=0;
      for( int i=0; i < hits; i++ )  {
        if( selectBuf[i*4+1] < maxz )  {
          in = i;
          maxz = selectBuf[i*4+1];
        }
      }
      if( (in-depth)*4+3 < 0 )  return NULL;
      in = selectBuf[(in-depth)*4+3] - 1;
      if( in < ObjectCount() )  
        Result = &GetObject(in);
    }
  }
  delete [] selectBuf;
  return Result;
}
//..............................................................................
TGlPrimitive* TGlRenderer::SelectPrimitive(int x, int y)  {
  if( (FWidth&FHeight) == 0 )  // test for 0
    return NULL;

  TGlPrimitive *Result = NULL;
  GLuint *selectBuf = new GLuint [MAXSELECT];
  const size_t prim_count = Primitives.ObjectCount();
  for( size_t i=0; i < prim_count; i++ )
    Primitives.GetObject(i).SetTag( (int)(i+1) );

  GetScene().StartSelect(x, y, selectBuf);
  DrawObjects(x, y, true, false);
  GetScene().EndSelect();

  int hits = glRenderMode(GL_RENDER);
  if( hits >= 1 )  {
    if( hits == 1 )  {
      GLuint in = selectBuf[(hits-1)*4+3];
      if( in >=1 && in <= (PrimitiveCount()+1) )
        Result = &GetPrimitive(in-1);
    }
    else  {
      unsigned int maxz = ~0;
      GLuint in=0;
      for( int i=0; i < hits; i++ )  {
        if( selectBuf[i*4+1] < maxz )  {
          in = i;
          maxz = selectBuf[i*4+1];
        }
      }
      in = selectBuf[in*4+3];
      if( in >=1 && in <= (PrimitiveCount()+1) )
        Result = &GetPrimitive(in-1);
    }
  }
  delete [] selectBuf;
  return Result;
}
//..............................................................................
TGlGroup* TGlRenderer::FindObjectGroup(AGDrawObject& G)  {
  // get the topmost group
  TGlGroup* G1 = G.GetParentGroup();
  if( G1 == NULL )  
    return NULL;
  while( G1->GetParentGroup() != NULL )  {
    if( G1->GetParentGroup() == FSelection )  break;
    G1 = G1->GetParentGroup(); 
  }
  return (G1 == FSelection) ? NULL : G1;
}
//..............................................................................
void TGlRenderer::Select(AGDrawObject& G)  {
  if( !G.IsGroupable() )  
    return;
  G.SetSelected( FSelection->Add(G) );
}
//..............................................................................
void TGlRenderer::DeSelect(AGDrawObject& G)  {
  FSelection->Remove(G);
}
//..............................................................................
void TGlRenderer::InvertSelection()  {
  TPtrList<AGDrawObject> Selected;
  const size_t oc = FGObjects.Count();
  for( size_t i=0; i < oc; i++ )  {
    AGDrawObject* GDO = FGObjects[i];
    if( !GDO->IsGrouped() && GDO->IsVisible() )  {
      if( !GDO->IsSelected() && GDO->IsGroupable() && GDO != FSelection )
        Selected.Add(GDO);
    }
  }
  FSelection->SetSelected(false);
  FSelection->Clear();
  for( size_t i=0; i < Selected.Count(); i++ )
    FSelection->Add( *Selected[i] );
}
//..............................................................................
void TGlRenderer::SelectAll(bool Select)  {
  FSelection->SetSelected(false);
  FSelection->Clear();
  if( Select )  {
    for( size_t i=0; i < ObjectCount(); i++ )  {
      AGDrawObject& GDO = GetObject(i);
      if( !GDO.IsGrouped() && GDO.IsVisible() && GDO.IsGroupable() )  {
        if( EsdlInstanceOf(GDO, TGlGroup) )  {
          if( &GDO == FSelection )  continue;
          bool Add = false;
          for( size_t j=0; j < ((TGlGroup&)GDO).Count(); j++ )  {
            if( ((TGlGroup&)GDO).GetObject(j).IsVisible() )  {
              Add = true;
              break;
            }
          }
          if( Add )  
            FSelection->Add(GDO);
        }
        else
          FSelection->Add(GDO);
      }
    }
  }
  FSelection->SetSelected(true);
}
//..............................................................................
void TGlRenderer::ClearGroups()  {
  for( size_t i=0; i < FGroups.Count(); i++ )  {
    if( FGroups[i]->IsSelected() )  
      DeSelect(*FGroups[i]);
    FGroups[i]->Clear();
  }
  // just in case of groups in groups
  for( size_t i=0; i < FGroups.Count(); i++ )
    delete FGroups[i];
  FGroups.Clear();
}
//..............................................................................
TGlGroup* TGlRenderer::FindGroupByName(const olxstr& colName)  {
  for( size_t i=0; i < FGroups.Count(); i++ )
    if( FGroups[i]->GetCollectionName() == colName )
      return FGroups[i];
  return NULL;
}
//..............................................................................
void TGlRenderer::ClearSelection()  {
  FSelection->Clear();
}
//..............................................................................
TGlGroup* TGlRenderer::GroupSelection(const olxstr& groupName)  {
  if( FSelection->Count() > 1 )  {
    TGlGroup *OS = FSelection;
    FGroups.Add(FSelection);
    FSelection = new TGlGroup(*this, "Selection");
    FSelection->Create();
    FSelection->Add(*OS);
    // read style information for this particular group
    OS->GetPrimitives().RemoveObject(*OS);
    FGObjects.Remove(OS);  // avoid duplication in the list!
    OS->Create(groupName);
    return OS;
  }
  return NULL;
}
//..............................................................................
TGlGroup& TGlRenderer::NewGroup(const olxstr& collection_name) {  
  return *FGroups.Add(new TGlGroup(*this, collection_name));  
}
//..............................................................................
void TGlRenderer::UnGroup(TGlGroup& OS)  {
  if( !OS.IsGroup() )  return;
  FGroups.Remove(OS);
  if( FSelection->Contains(OS) )
    FSelection->Remove(OS);

  TPtrList<AGDrawObject> Objects(OS.Count());
  for( size_t i=0; i < OS.Count(); i++ )
    Objects[i] = &OS[i];
  OS.GetPrimitives().RemoveObject(OS); // 
  FGObjects.Remove(OS);
  delete &OS;  // it will reset Parent group to NULL in the objects
  for( size_t i=0; i < Objects.Count(); i++ )
    FSelection->Add( *Objects[i] );
  FSelection->SetSelected(true);
}
//..............................................................................
void TGlRenderer::UnGroupSelection()  {
  if( FSelection->Count() >= 1 )  {
    for( size_t i=0; i < FSelection->Count(); i++ )  {
      AGDrawObject& GDO = FSelection->GetObject(i);
      if( GDO.IsGroup() )
        UnGroup((TGlGroup&)GDO);
    }
  }
}
//..............................................................................
void TGlRenderer::EnableClipPlane(TGlClipPlane *P, bool v)  {
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
void TGlRenderer::SetProperties(TGlMaterial& P)  {  // tracks transluent and identity objects
  if( P.IsTransparent() && P.IsIdentityDraw() )  {
    FTransluentIdentityObjects.AddUnique(&P);
    return;
  }
  if( P.IsTransparent() )  {
    FTransluentObjects.AddUnique(&P);
    return;
  }
  if( P.IsIdentityDraw() )  {
    FIdentityObjects.AddUnique(&P);
    return;
  }
}
//..............................................................................
void TGlRenderer::OnSetProperties(const TGlMaterial& P)  {  // tracks transluent and identity objects
  //if( P == NULL )  return;
  if( P.ObjectCount() > 1 )  return; // the properties will not be removde
  if( P.IsTransparent() && P.IsIdentityDraw() )  {
    size_t index = FTransluentIdentityObjects.IndexOf(&P);
    if( index != InvalidIndex )  
      FTransluentIdentityObjects.Delete(index);
    return;
  }
  if( P.IsTransparent() )  {
    size_t index = FTransluentObjects.IndexOf(&P);
    if( index != InvalidIndex )  
      FTransluentObjects.Delete(index);
    return;
  }
  if( P.IsIdentityDraw() )  {
    size_t index = FIdentityObjects.IndexOf(&P);
    if( index != InvalidIndex ) 
      FIdentityObjects.Delete(index);
    return;
  }
}
//..............................................................................
void TGlRenderer::RemoveObjects(const TPtrList<AGDrawObject>& objects)  {
  for( size_t i=0; i < FGObjects.Count(); i++ )
    FGObjects[i]->SetTag(-1);
  for( size_t i=0; i < objects.Count(); i++ )
    objects[i]->SetTag(0);
  for( size_t i=0; i < FGObjects.Count(); i++ )
    if( FGObjects[i]->GetTag() == 0 )
      FGObjects[i] = NULL;
  FGObjects.Pack();
}
//..............................................................................
void TGlRenderer::AddObject(AGDrawObject& G)  {
  FGObjects.Add(G);
  if( FSceneComplete || !G.IsVisible() )  return;
  vec3d MaxV, MinV;
  if( G.GetDimensions(MaxV, MinV) )  {
    UpdateMaxMin(MaxV, MinV);
    SetChanged( true );
  }
}
//..............................................................................
/*
void TGlRenderer::ReplacePrimitives(TEList *CurObj, TEList *NewObj)
{
  if( CurObj->Count() != NewObj->Count() )
    BasicApp->Log->Exception("TGlRenderer:: lists count does not much!", true);
  Primitives.ReplaceObjects(CurObj, NewObj);
} */
//..............................................................................
void TGlRenderer::RemoveCollection(TGPCollection& GP)  {
  FTransluentIdentityObjects.Clear();
  FTransluentObjects.Clear();
  FIdentityObjects.Clear();

  for( size_t i=0; i < PrimitiveCount(); i++ )
    GetPrimitive(i).SetTag(-1);
  for( size_t i=0; i < GP.PrimitiveCount(); i++ )
    GP.GetPrimitive(i).SetTag(0);
  Primitives.RemoveObjectsByTag(0);
  FCollections.Delete( FCollections.IndexOfObject(&GP) );

  for( size_t i=0; i < Primitives.PropertiesCount(); i++ )  {
    TGlMaterial& GlM = Primitives.GetProperties(i);
    if( GlM.IsTransparent() && GlM.IsIdentityDraw()  )
      FTransluentIdentityObjects.Add(GlM);
    else if( GlM.IsTransparent() )
      FTransluentObjects.Add(GlM);
    else if( GlM.IsIdentityDraw() )
      FIdentityObjects.Add(GlM);
  }
  delete &GP;
}
//..............................................................................
void TGlRenderer::RemoveCollections(const TPtrList<TGPCollection>& Colls)  {
  if( Colls.Count() == 0 )  return;

  FTransluentIdentityObjects.Clear();
  FTransluentObjects.Clear();
  FIdentityObjects.Clear();

  for( size_t i=0; i < PrimitiveCount(); i++ )
    GetPrimitive(i).SetTag(-1);
  for( size_t i=0; i < Colls.Count(); i++ )  {
    for( size_t j=0; j < Colls[i]->PrimitiveCount(); j++ )
      Colls[i]->GetPrimitive(j).SetTag(0);
    Primitives.RemoveObjectsByTag(0);
    size_t col_ind = FCollections.IndexOfObject(Colls[i]);
    FCollections.Remove( col_ind );
    delete Colls[i];
  }
  for( size_t i=0; i < Primitives.PropertiesCount(); i++ )  {
    TGlMaterial& GlM = Primitives.GetProperties(i);
    if( GlM.IsTransparent() && GlM.IsIdentityDraw() )
      FTransluentIdentityObjects.Add(&GlM);
    else if( GlM.IsTransparent() )   
      FTransluentObjects.Add(GlM);
    else if( GlM.IsIdentityDraw() )  
      FIdentityObjects.Add(GlM);
  }
}
//..............................................................................
void TGlRenderer::LookAt(double x, double y, short res)  {
  FViewZoom = (float)(1.0/res);
  FProjectionLeft = (float)(x/(double)res - 0.5);
  FProjectionRight = (float)((x+1)/(double)res - 0.5);
  FProjectionTop = (float)(y/(double)res - 0.5);
  FProjectionBottom = (float)((y+1)/(double)res - 0.5);
  SetChanged(true);
}
//..............................................................................
char* TGlRenderer::GetPixels(bool useMalloc, short aligment)  {
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
void TGlRenderer::RemovePrimitiveByTag(int in)  {
  Primitives.RemoveObjectsByTag(in);
  FTransluentIdentityObjects.Clear();
  FTransluentObjects.Clear();
  FIdentityObjects.Clear();
  for( size_t i=0; i < Primitives.PropertiesCount(); i++ )  {
    TGlMaterial& GlM = Primitives.GetProperties(i);
    if( GlM.IsTransparent() && GlM.IsIdentityDraw() )
      FTransluentIdentityObjects.Add(GlM);
    else if( GlM.IsTransparent() )    
      FTransluentObjects.Add(GlM);
    else if( GlM.IsIdentityDraw() )   
      FIdentityObjects.Add(GlM);
  }
}
//..............................................................................
void TGlRenderer::CleanUpStyles()  {// removes styles, which are not used by any collection
  OnStylesClear->Enter(this);
  GetStyles().SetStylesTag(0);
  for( size_t i=0; i < FCollections.Count(); i++ )
    FCollections.GetObject(i)->GetStyle().SetTag(1);
  GetStyles().RemoveStylesByTag(0);
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
unsigned int TGlListManager::NewList()  {
  if( FPos >= Lists.Count() )  {
    unsigned int s = glGenLists(10);
    if( s == GL_INVALID_VALUE || s == GL_INVALID_OPERATION )
      throw TFunctionFailedException(__OlxSourceInfo, "glGenLists");
    for( int i=0; i < 10; i++ )
      Lists.Add(s+i);
  }
  return Lists[FPos ++];
}
//..............................................................................
void TGlListManager::ClearLists()  {
  for( size_t i=0; i < Lists.Count(); i+= FInc )
    glDeleteLists(Lists[i], FInc);
  FPos = 0;
  Lists.Clear();
}
//..............................................................................
void TGlListManager::ReserveRange(unsigned int count)  {
  ClearLists();
  unsigned int s = glGenLists(count);
  if( s == GL_INVALID_VALUE || s == GL_INVALID_OPERATION )
    throw TFunctionFailedException(__OlxSourceInfo, "glGenLists");
  for( unsigned int i=0; i < count; i++ )
    Lists.Add( s+i );
  FPos = (unsigned int)Lists.Count();
}
//..............................................................................
TGraphicsStyles& TGlRenderer::_GetStyles()  {
  if( TGlRenderer::FStyles == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "Object is not initialised");
  return *TGlRenderer::FStyles;
}
//..............................................................................
void TGlRenderer::Compile(bool v)  {
  /* remark: it works, but gives no performance boost ... */
  //return;
  if( v )  {
    if( CompiledListId == -1 )  {
      CompiledListId = glGenLists(1);
    }
    glNewList(CompiledListId, GL_COMPILE);
    for( size_t i=0; i < Primitives.PropertiesCount(); i++ )  {
      TGlMaterial& GlM = Primitives.GetProperties(i);
      if( GlM.IsIdentityDraw() ) continue;  // already drawn
      if( GlM.IsTransparent() ) continue;  // will be drawn
      GlM.Init();
      for( size_t j=0; j < GlM.ObjectCount(); j++ )  {
        TGlPrimitive& GlP = (TGlPrimitive&)GlM.GetObject(j);
        TGPCollection* GPC = GlP.GetParentCollection();
        if( GPC == NULL )  continue;
        for( size_t k=0; k < GPC->ObjectCount(); k++ )  {
          AGDrawObject& GDO = GPC->GetObject(k);
          if( !GDO.IsVisible() )  continue;
          if( GDO.IsSelected() ) continue;
          if( GDO.IsGrouped() ) continue;
          glPushMatrix();
          if( GDO.Orient(GlP) )  // the object has drawn itself
          {  glPopMatrix(); continue; }
          GlP.Draw();
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
void TGlRenderer::DrawText(TGlPrimitive& p, double x, double y, double z)  {
  if( ATI )  {
    glRasterPos3d(0, 0, 0);
    glCallList(p.GetFont()->GetFontBase() + ' ');
  }
  glRasterPos3d(x, y, z);
  p.Draw();
}
//..............................................................................
void TGlRenderer::DrawTextSafe(const vec3d& pos, const olxstr& text, const TGlFont& fnt) {
  if( ATI )  {
    glRasterPos3d(0, 0, 0);
    glCallList(fnt.GetFontBase() + ' ');
  }
  // set a valid raster position
  glRasterPos3d(0, 0, pos[2]);
  glBitmap(0, 0, 0, 0, (float)(pos[0]/FViewZoom), (float)(pos[1]/FViewZoom), NULL);
  for( size_t i=0; i < text.Length(); i++ ) 
    glCallList(fnt.GetFontBase() + text.CharAt(i));
}
//..............................................................................
//..............................................................................
//..............................................................................

void TGlRenderer::LibCompile(const TStrObjList& Params, TMacroError& E)  {
  Compile( Params[0].ToBool() );
}
//..............................................................................
void TGlRenderer::LibPerspective(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
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
void TGlRenderer::LibFog(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
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
void TGlRenderer::LibZoom(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
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
TLibrary*  TGlRenderer::ExportLibrary(const olxstr& name)  {
  TLibrary* lib = new TLibrary( name.IsEmpty() ? olxstr("gl") : name);
  lib->RegisterFunction<TGlRenderer>( new TFunction<TGlRenderer>(this,  &TGlRenderer::LibCompile, "Compile",
    fpOne, "Compiles or decompiles the model according to the boolean parameter") );

  lib->RegisterMacro<TGlRenderer>( new TMacro<TGlRenderer>(this,  &TGlRenderer::LibPerspective, "Perspective",
    EmptyString, fpNone|fpOne, "Un/Sets perspective view") );
  lib->RegisterMacro<TGlRenderer>( new TMacro<TGlRenderer>(this,  &TGlRenderer::LibFog, "Fog",
    EmptyString, fpNone|fpTwo, "Un/Sets fog") );
  lib->RegisterMacro<TGlRenderer>( new TMacro<TGlRenderer>(this,  &TGlRenderer::LibZoom, "Zoom",
    EmptyString, fpNone|fpOne, "If no arguments provided - resets zoom to fit to screen, otherwise increments/\
decrements current zoom by provided value") );

  return lib;
}
//..............................................................................

