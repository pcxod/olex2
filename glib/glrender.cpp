//---------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//
#include "glrender.h"
#include "glgroup.h"
#include "styles.h"
#include "glbackground.h"
#include "gltexture.h"
#include "library.h"
#include "bapp.h"
#include "log.h"

UseGlNamespace();
//..............................................................................
TGraphicsStyles* TGlRenderer::FStyles = NULL;
//..............................................................................
TGlRenderer::TGlRenderer(AGlScene *S, size_t width, size_t height) :
  StereoRightColor(1, 0, 0, 1), StereoLeftColor(0, 1, 1, 1),
  OnDraw(TBasicApp::GetInstance().NewActionQueue(olxappevent_GL_DRAW)),
  OnStylesClear(TBasicApp::GetInstance().NewActionQueue(olxappevent_GL_CLEAR_STYLES)),
  Top(0), Left(0), Width((int)width), Height((int)height), OWidth(0)
{
  CompiledListId = -1;
  FScene = S;
  FZoom = 1;
  FViewZoom = 1;
  FScene->Parent(this);
  FPerspective = false;
  FPAngle = 1;
  StereoFlag = 0;
  StereoAngle = 3;

  LookAt(0,0,1);

  Fog = false;
  FogType = GL_EXP;
  FogColor = 0x7f7f7f;
  FogDensity = 1;
  FogStart = 0;
  FogEnd = 10;
  SceneDepth = -1;

  FStyles = new TGraphicsStyles(*this);

  FSelection = new TGlGroup(*this, "Selection");
  FSelection->SetSelected(true);
  FBackground = new TGlBackground(*this, "Background", false);
  FBackground->SetVisible(false);
  FCeiling = new TGlBackground(*this, "Ceiling", true);
  FCeiling->SetVisible(false);
  FGlImageChanged = true; // will cause its update
  FGlImage = NULL;
  TextureManager = new TTextureManager();
  FTranslucentObjects.SetIncrement(16);
  FCollections.SetIncrement(16);
  FGObjects.SetIncrement(16);
}
//..............................................................................
TGlRenderer::~TGlRenderer()  {
  Clear();
  //GraphicsStyles = NULL;
  delete FStyles;
  delete FBackground;
  delete FCeiling;
  delete FSelection;
  delete FScene;
  delete TextureManager;
}
//..............................................................................
void TGlRenderer::Initialise()  {
  InitLights();
  for( size_t i=0; i < Primitives.ObjectCount(); i++ )
    Primitives.GetObject(i).Compile();
  FSelection->Create();
  FBackground->Create();
  FCeiling->Create();
  ATI = olxcstr((const char*)olx_gl::getString(GL_VENDOR)).StartsFrom("ATI");
}
//..............................................................................
void TGlRenderer::InitLights()  {
  SetView(true, 1);
  olx_gl::enable(GL_LIGHTING);
  olx_gl::enable(GL_DEPTH_TEST);
  olx_gl::hint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
  olx_gl::clearDepth(1.0f);
  olx_gl::depthFunc(GL_LEQUAL);
  LightModel.Init();
}
//..............................................................................
void TGlRenderer::ClearPrimitives()  {
  ClearGroups();
  FSelection->Clear();
  FListManager.ClearLists();
  if( CompiledListId != -1 )  {
    olx_gl::deleteLists(CompiledListId, 1);
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
  FTranslucentIdentityObjects.Clear();
  FTranslucentObjects.Clear();
  FIdentityObjects.Clear();
  FGObjects.Clear();
  ClearMinMax();
  ReleaseGlImage();
}
//..............................................................................
void TGlRenderer::Clear()  {
  FSelection->Clear();
  for( size_t i=0; i < FGroups.Count(); i++ )
    delete FGroups[i];
  FGroups.Clear();
  FListManager.ClearLists();
  if( CompiledListId != -1 )  {
    olx_gl::deleteLists(CompiledListId, 1);
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
  FTranslucentIdentityObjects.Clear();
  FTranslucentObjects.Clear();
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
  GlImageHeight = Height;
  GlImageWidth = Width;
  FGlImageChanged = false;
}
//..............................................................................
void TGlRenderer::ClearMinMax()  {
  FMaxV[0] = FMaxV[1] = FMaxV[2] = -100;
  FMinV[0] = FMinV[1] = FMinV[2] = +100;
  SceneDepth = -1;
}
//..............................................................................
void TGlRenderer::UpdateMinMax(const vec3d& Min, const vec3d& Max)  {
  if( Max[0] > FMaxV[0] )  FMaxV[0] = Max[0];
  if( Max[1] > FMaxV[1] )  FMaxV[1] = Max[1];
  if( Max[2] > FMaxV[2] )  FMaxV[2] = Max[2];

  if( Min[0] < FMinV[0] )  FMinV[0] = Min[0];
  if( Min[1] < FMinV[1] )  FMinV[1] = Min[1];
  if( Min[2] < FMinV[2] )  FMinV[2] = Min[2];
}
//..............................................................................
void TGlRenderer::operator = (const TGlRenderer &G)  { ; }
//..............................................................................
void TGlRenderer::_OnStylesClear()  {
  OnStylesClear.Enter(this);
  for( size_t i=0; i < FCollections.Count(); i++ )
    FCollections.GetObject(i)->SetStyle(NULL);
}
//..............................................................................
void TGlRenderer::_OnStylesLoaded()  {
  for( size_t i=0; i < FCollections.Count(); i++ )
    FCollections.GetObject(i)->SetStyle(&FStyles->NewStyle(FCollections.GetObject(i)->GetName(), true));
  AGDObjList GO = FGObjects.GetList();
  for( size_t i=0; i < GO.Count(); i++ )  {
    GO[i]->OnPrimitivesCleared();
    GO[i]->SetCreated(false);
  }
  ClearPrimitives();
  for( size_t i=0; i < GO.Count(); i++ )  {
    if( !GO[i]->IsCreated() )   {  // some loose objects as labels can be created twice otherwise
      GO[i]->Create();
      GO[i]->SetCreated(true);
    }
  }
  TGraphicsStyle* gs = FStyles->FindStyle("GL.Stereo");
  if( gs != NULL )  {
    StereoLeftColor = gs->GetParam("left", StereoLeftColor.ToString(), true);
    StereoRightColor = gs->GetParam("right", StereoRightColor.ToString(), true);
    StereoAngle = gs->GetParam("angle", StereoAngle, true).ToDouble();
  }
  OnStylesClear.Exit(this);
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
  olx_gl::fog(GL_FOG_MODE, FogType);
  olx_gl::fog(GL_FOG_DENSITY, FogDensity);
  olx_gl::fog(GL_FOG_COLOR, FogColor.Data());
  olx_gl::fog(GL_FOG_START, FogStart);
  olx_gl::fog(GL_FOG_END, FogEnd);
  if( Set )
    olx_gl::enable(GL_FOG);
  else
    olx_gl::disable(GL_FOG);
  Fog = Set;
}
//..............................................................................
void TGlRenderer::EnablePerspective(bool Set)  {
  if( Set == FPerspective )  return;
  FPerspective = Set;
}
//..............................................................................
void TGlRenderer::SetPerspectiveAngle(double angle)  {
  FPAngle = (float)tan(angle*M_PI/360);
}
//..............................................................................
void TGlRenderer::Resize(size_t w, size_t h)  {
  Resize(0, 0, w, h, 1);
}
//..............................................................................
void TGlRenderer::Resize(int l, int t, size_t w, size_t h, float Zoom)  {
  Left = l;
  Top = t;
  if( StereoFlag == glStereoCross )  {
    Width = (int)w/2;
    OWidth = (int)w;
  }
  else
    Width = (int)w;
  Height = (int)h;
  FZoom = Zoom;
  FGlImageChanged = true;
}
//..............................................................................
void TGlRenderer::SetView(bool i, short Res)  {  SetView(0, 0, i , false, Res);  }
//..............................................................................
void TGlRenderer::SetZoom(double V) {  
  //const double MaxZ = olx_max(FMaxV.DistanceTo(FMinV), 1);
  //double dv = V*MaxZ;
  if( V < 0.001 )  //  need to fix the zoom
    FBasis.SetZoom(0.001);
  else if( V > 100 )
    FBasis.SetZoom(100);
  else
    FBasis.SetZoom(V); 
}
//..............................................................................
void TGlRenderer::SetView(int x, int y, bool identity, bool Select, short Res)  {
  olx_gl::viewport(Left*Res, Top*Res, Width*Res, Height*Res);
  olx_gl::matrixMode(GL_PROJECTION);
  olx_gl::loadIdentity();
  if( Select )  {
    GLint vp[4];
    olx_gl::get(GL_VIEWPORT, vp);
    gluPickMatrix(x, Height-y, 2, 2, vp);
  }
  const double aspect = (double)Width/(double)Height;
  if( !identity )  {
    if( FPerspective )  {
      double right = FPAngle*aspect;
      olx_gl::frustum(right*FProjectionLeft, right*FProjectionRight,
        FPAngle*FProjectionTop, FPAngle*FProjectionBottom, 1, 10);
    }
    else  {
      olx_gl::ortho(aspect*FProjectionLeft, aspect*FProjectionRight,
        FProjectionTop, FProjectionBottom, 1, 10);
    }
    //glTranslated(0, 0, FMinV[2] > 0 ? -1 : FMinV[2]-FMaxV[2]);
    olx_gl::translate(0, 0, -2);
  }
  else  {
    olx_gl::ortho(aspect*FProjectionLeft, aspect*FProjectionRight,
      FProjectionTop, FProjectionBottom, -1, 1);
  }
  olx_gl::matrixMode(GL_MODELVIEW);
  /* Mxv ->
    x = {(Bf[0][0]*x+Bf[0][1]*y+Bf[0][2]*z+Bf[0][3]*w)},
    y = {(Bf[1][0]*x+Bf[1][1]*y+Bf[1][2]*z+Bf[1][3]*w)},
    z = {(Bf[2][0]*x+Bf[2][1]*y+Bf[2][2]*z+Bf[2][3]*w)},
    w = {(Bf[3][0]*x+Bf[3][1]*y+Bf[3][2]*z+Bf[3][3]*w)}
  */
  if( !identity )  {
    static float Bf[4][4];
    memcpy(&Bf[0][0], GetBasis().GetMData(), 12*sizeof(float));
    Bf[3][0] = Bf[3][1] = 0;
    Bf[3][2] = -1;
    Bf[3][3] = 1;
    olx_gl::loadMatrix(&Bf[0][0]);
    olx_gl::scale(GetBasis().GetZoom());
    olx_gl::translate(GetBasis().GetCenter());
  }
  else  {
    olx_gl::loadIdentity();
  }
  //glDepthRange(1, 0);
}
//..............................................................................
void TGlRenderer::Draw()  {
  if( Width < 50 || Height < 50 )  return;
  olx_gl::enable(GL_NORMALIZE);
  OnDraw.Enter(this);
  //glLineWidth( (float)(0.07/GetScale()) );
  //glPointSize( (float)(0.07/GetScale()) );  
  if( StereoFlag == glStereoColor )  {
    olx_gl::clearColor(0.0,0.0,0.0,0.0);
    olx_gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    const double ry = GetBasis().GetRY();
    olx_gl::disable(GL_ALPHA_TEST);
    olx_gl::disable(GL_BLEND);
    olx_gl::disable(GL_CULL_FACE);
    olx_gl::enable(GL_COLOR_MATERIAL);
    olx_gl::colorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    // right eye
    GetBasis().RotateY(ry+StereoAngle);
    olx_gl::colorMask(
      StereoRightColor[0] != 0,
      StereoRightColor[1] != 0,
      StereoRightColor[2] != 0,
      StereoRightColor[3] != 0);
    olx_gl::color(StereoRightColor.Data());
    DrawObjects(0, 0, false, false);
    //left eye
    GetBasis().RotateY(ry-StereoAngle);
    olx_gl::clear(GL_DEPTH_BUFFER_BIT);
    olx_gl::enable(GL_BLEND);
    olx_gl::blendFunc(GL_ONE, GL_ONE);
    olx_gl::colorMask(
      StereoLeftColor[0] != 0,
      StereoLeftColor[1] != 0,
      StereoLeftColor[2] != 0,
      StereoLeftColor[3] != 0);
    olx_gl::color(StereoLeftColor.Data());
    DrawObjects(0, 0, false, false);
    GetBasis().RotateY(ry);
    olx_gl::colorMask(true, true, true, true);
  }
  // http://local.wasp.uwa.edu.au/~pbourke/texture_colour/anaglyph/
  else if( StereoFlag == glStereoAnaglyph )  {
    const double ry = GetBasis().GetRY();
    olx_gl::clearColor(0.0,0.0,0.0,0.0);
    olx_gl::clearAccum(0.0,0.0,0.0,0.0);
    olx_gl::colorMask(true, true, true, true);
    olx_gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // right eye
    GetBasis().RotateY(ry+StereoAngle);
    olx_gl::colorMask(
      StereoRightColor[0] != 0,
      StereoRightColor[1] != 0,
      StereoRightColor[2] != 0,
      StereoRightColor[3] != 0);
    DrawObjects(0, 0, false, false);
    olx_gl::colorMask(true, true, true, true);
    olx_gl::accum(GL_LOAD, 1);
    // left eye
    GetBasis().RotateY(ry-StereoAngle);
    olx_gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    olx_gl::colorMask(
      StereoLeftColor[0] != 0,
      StereoLeftColor[1] != 0,
      StereoLeftColor[2] != 0,
      StereoLeftColor[3] != 0);
    DrawObjects(0, 0, false, false);
    olx_gl::colorMask(true, true, true, true);
    olx_gl::accum(GL_ACCUM, 1);
    olx_gl::accum(GL_RETURN, 1.0);
    GetBasis().RotateY(ry);
  }
  else if( StereoFlag == glStereoHardware )  {
    const double ry = GetBasis().GetRY();
    GetBasis().RotateY(ry+StereoAngle);
    olx_gl::drawBuffer(GL_BACK_LEFT);
    olx_gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    DrawObjects(0, 0, false, false);
    GetBasis().RotateY(ry-StereoAngle);
    olx_gl::drawBuffer(GL_BACK_RIGHT);
    olx_gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    DrawObjects(0, 0, false, false);
    olx_gl::drawBuffer(GL_BACK);
    GetBasis().RotateY(ry);
  }
  else if( StereoFlag == glStereoCross )  {
    const double ry = GetBasis().GetRY();
    olx_gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    const int _l = Left;
    GetBasis().RotateY(ry+StereoAngle);
    DrawObjects(0, 0, false, false);
    GetBasis().RotateY(ry-StereoAngle);
    Left = Width;
    DrawObjects(0, 0, false, false);
    GetBasis().RotateY(ry);
    Left = _l;
  }
  else  {
    GetScene().StartDraw();
    DrawObjects(0, 0, false, false);
  }
  GetScene().EndDraw();
  FGlImageChanged = true;
  OnDraw.Execute(this);
  OnDraw.Exit(this);
}
//..............................................................................
void TGlRenderer::DrawObjects(int x, int y, bool SelectObjects, bool SelectPrimitives)  {
#ifdef _DEBUG
  for( size_t i=0; i < PrimitiveCount(); i++ )  {
    GetPrimitive(i).SetFont(NULL);
    GetPrimitive(i).SetString(NULL);
  }
#endif
  const bool Select = SelectObjects || SelectPrimitives;
  const bool skip_mat = StereoFlag==glStereoColor;
  static const int DrawMask = sgdoVisible|sgdoSelected|sgdoDeleted|sgdoGrouped;
  if( !FIdentityObjects.IsEmpty() || FSelection->GetGlM().IsIdentityDraw() )  {
    SetView(x, y, true, Select, 1);
    const size_t id_obj_count = FIdentityObjects.Count();
    for( size_t i=0; i < id_obj_count; i++ )  {
      TGlMaterial* GlM = FIdentityObjects[i];
      if( !Select )  GlM->Init(skip_mat);
      const size_t obj_count = GlM->ObjectCount();
      for( size_t j=0; j < obj_count; j++ )  {
        TGlPrimitive& GlP = (TGlPrimitive&)GlM->GetObject(j);
        TGPCollection* GPC = GlP.GetParentCollection();
        const size_t c_obj_count = GPC->ObjectCount();
        for( size_t k=0; k < c_obj_count; k++ )  {
          AGDrawObject& GDO = GPC->GetObject(k);
          if( GDO.MaskFlags(DrawMask) != sgdoVisible )  continue;
          if( SelectObjects )  olx_gl::loadName((GLuint)GDO.GetTag());
          else if( SelectPrimitives )  olx_gl::loadName((GLuint)GlP.GetTag());
          olx_gl::pushMatrix();
          if( GDO.Orient(GlP) )  // the object has drawn itself
          {  olx_gl::popMatrix(); continue; }
          GlP.Draw();
          olx_gl::popMatrix();
        }
      }
    }
    if( FSelection->GetGlM().IsIdentityDraw() )  {
      olx_gl::pushAttrib(GL_ALL_ATTRIB_BITS);
      FSelection->Draw(SelectPrimitives, SelectObjects);
      olx_gl::popAttrib();
    }
    SetView(x, y, false, Select, 1);
  }
  else  {
    SetView(x, y, false, Select, 1);
  }

  if( !Select && IsCompiled() )  {
    olx_gl::callList(CompiledListId);
  }
  else  {
    const size_t prim_count = Primitives.PropertiesCount();
    for( size_t i=0; i < prim_count; i++ )  {
      TGlMaterial& GlM = Primitives.GetProperties(i);
      if( GlM.IsIdentityDraw() ) continue;  // already drawn
      if( GlM.IsTransparent() ) continue;  // will be drawn
      if( !Select )  GlM.Init(skip_mat);
      const size_t obj_count = GlM.ObjectCount();
      for( size_t j=0; j < obj_count; j++ )  {
        TGlPrimitive& GlP = (TGlPrimitive&)GlM.GetObject(j);
        TGPCollection* GPC = GlP.GetParentCollection();
        if( GPC == NULL )  continue;
        const size_t c_obj_count = GPC->ObjectCount();
        for( size_t k=0; k < c_obj_count; k++ )  {
          AGDrawObject& GDO = GPC->GetObject(k);
          if( GDO.MaskFlags(DrawMask) != sgdoVisible )  continue;
          if( SelectObjects )  olx_gl::loadName((GLuint)GDO.GetTag());
          else if( SelectPrimitives )  olx_gl::loadName((GLuint)GlP.GetTag());
          olx_gl::pushMatrix();
          if( GDO.Orient(GlP) )  // the object has drawn itself
          {  olx_gl::popMatrix(); continue; }
          GlP.Draw();
          olx_gl::popMatrix();
        }
      }
    }
  }
  const size_t trans_obj_count = FTranslucentObjects.Count();
  for( size_t i=0; i < trans_obj_count; i++ )  {
    TGlMaterial* GlM = FTranslucentObjects[i];
    if( !Select )  GlM->Init(skip_mat);
    const size_t obj_count = GlM->ObjectCount();
    for( size_t j=0; j < obj_count; j++ )  {
      TGlPrimitive& GlP = (TGlPrimitive&)GlM->GetObject(j);
      TGPCollection* GPC = GlP.GetParentCollection();
      const size_t c_obj_count = GPC->ObjectCount();
      for( size_t k=0; k < c_obj_count; k++ )  {
        AGDrawObject& GDO = GPC->GetObject(k);
        if( GDO.MaskFlags(DrawMask) != sgdoVisible )  continue;
        if( SelectObjects )  olx_gl::loadName((GLuint)GDO.GetTag());
        else if( SelectPrimitives )  olx_gl::loadName((GLuint)GlP.GetTag());
        olx_gl::pushMatrix();
        if( GDO.Orient(GlP) )  // the object has drawn itself
        {  olx_gl::popMatrix(); continue; }
        GlP.Draw();
        olx_gl::popMatrix();
      }
    }
  }
  const size_t group_count = FGroups.Count();
  for( size_t i=0; i < group_count; i++ )
    FGroups[i]->Draw(SelectPrimitives, SelectObjects);

  if( !FSelection->GetGlM().IsIdentityDraw() )  {
    olx_gl::pushAttrib(GL_ALL_ATTRIB_BITS);
    FSelection->Draw(SelectPrimitives, SelectObjects);
    olx_gl::popAttrib();
  }
  if( !FTranslucentIdentityObjects.IsEmpty() )  {
    SetView(x, y, true, Select, 1);
    const size_t trans_id_obj_count = FTranslucentIdentityObjects.Count();
    for( size_t i=0; i < trans_id_obj_count; i++ )  {
      TGlMaterial* GlM = FTranslucentIdentityObjects[i];
      if( !Select )  GlM->Init(skip_mat);
      const size_t obj_count = GlM->ObjectCount(); 
      for( size_t j=0; j < obj_count; j++ )  {
        TGlPrimitive& GlP = (TGlPrimitive&)GlM->GetObject(j);
        TGPCollection* GPC = GlP.GetParentCollection();
        const size_t c_obj_count = GPC->ObjectCount();
        for( size_t k=0; k < c_obj_count; k++ )  {
          AGDrawObject& GDO = GPC->GetObject(k);
          if( GDO.MaskFlags(DrawMask) != sgdoVisible )  continue;
          if( SelectObjects )  olx_gl::loadName((GLuint)GDO.GetTag());
          else if( SelectPrimitives )  olx_gl::loadName((GLuint)GlP.GetTag());
          olx_gl::pushMatrix();
          if( GDO.Orient(GlP) )  // the object has drawn itself
          {  olx_gl::popMatrix(); continue; }
          GlP.Draw();
          olx_gl::popMatrix();
        }
      }
    }
  }
}
//..............................................................................
AGDrawObject* TGlRenderer::SelectObject(int x, int y, int depth)  {
  if( (Width&Height) == 0 )  return NULL;
  AGDrawObject *Result = NULL;
  GLuint *selectBuf = new GLuint [MAXSELECT];
  for( size_t i=0; i < ObjectCount(); i++ )
    GetObject(i).SetTag((int)(i+1));
  GetScene().StartSelect(x, y, selectBuf);
  DrawObjects(x, y, true, false);
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
      if( (int)(in-depth)*4+3 < 0 )  return NULL;
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
  if( (Width&Height) == 0 )  return NULL;
  TGlPrimitive *Result = NULL;
  GLuint *selectBuf = new GLuint [MAXSELECT];
  const size_t prim_count = Primitives.ObjectCount();
  for( size_t i=0; i < prim_count; i++ )
    Primitives.GetObject(i).SetTag( (int)(i+1) );

  GetScene().StartSelect(x, y, selectBuf);
  DrawObjects(x, y, false, true);
  GetScene().EndSelect();

  int hits = olx_gl::renderMode(GL_RENDER);
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
TGlGroup* TGlRenderer::FindObjectGroup(const AGDrawObject& G) const {
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
  G.SetSelected(FSelection->Add(G));
}
//..............................................................................
void TGlRenderer::DeSelect(AGDrawObject& G)  {
  if( G.GetParentGroup() == FSelection )
    FSelection->Remove(G);
}
//..............................................................................
void TGlRenderer::Select(AGDrawObject& G, bool v)  {
  if( v )  {
    if( !G.IsSelected() )
      Select(G);
  }
  else if( G.IsSelected() )
    DeSelect(G);
}
//..............................................................................
void TGlRenderer::InvertSelection()  {
  AGDObjList Selected;
  const size_t oc = FGObjects.Count();
  for( size_t i=0; i < oc; i++ )  {
    AGDrawObject* GDO = FGObjects[i];
    if( !GDO->IsGrouped() && GDO->IsVisible() )
      Selected.Add(GDO);
  }
  FSelection->Clear();
  for( size_t i=0; i < Selected.Count(); i++ )
    Selected[i]->SetSelected(FSelection->Add(*Selected[i]));
}
//..............................................................................
void TGlRenderer::SelectAll(bool Select)  {
  if( Select )  {
    for( size_t i=0; i < ObjectCount(); i++ )  {
      AGDrawObject& GDO = GetObject(i);
      if( !GDO.IsGrouped() && GDO.IsVisible() && GDO.IsSelectable() )  // grouped covers selected
        FSelection->Add(GDO);
    }
    FSelection->SetSelected(true);
  }
  else
    FSelection->Clear();
}
//..............................................................................
void TGlRenderer::ClearGroups()  {
  if( FGroups.IsEmpty() )  return;
  for( size_t i=0; i < FGroups.Count(); i++ )  {
    if( FGroups[i]->IsSelected() )  
      DeSelect(*FGroups[i]);
    FGroups[i]->Clear();
  }
  for( size_t i=0; i < FGroups.Count(); i++ )
    delete FGroups[i];
  FGroups.Clear();
}
//..............................................................................
TGlGroup* TGlRenderer::FindGroupByName(const olxstr& colName) const {
  for( size_t i=0; i < FGroups.Count(); i++ )
    if( FGroups[i]->GetCollectionName() == colName )
      return FGroups[i];
  return NULL;
}
//..............................................................................
void TGlRenderer::ClearSelection()  {  FSelection->Clear();  }
//..............................................................................
TGlGroup* TGlRenderer::GroupSelection(const olxstr& groupName)  {
  if( FSelection->Count() > 1 )  {
    AGDObjList ungroupable;
    if( !FSelection->TryToGroup(ungroupable) )
      return NULL;
    TGlGroup *OS = FSelection;
    FGroups.Add(FSelection);
    OS->GetPrimitives().RemoveObject(*OS);
    FSelection = new TGlGroup(*this, "Selection");
    FSelection->Create();
    for( size_t i=0; i < ungroupable.Count(); i++ )
      FSelection->Add(*ungroupable[i]);
    // read style information for this particular group
    OS->SetSelected(false);
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
  FGroups.Remove(OS);
  if( FSelection->Contains(OS) )
    FSelection->Remove(OS);

  AGDObjList Objects(OS.Count());
  for( size_t i=0; i < OS.Count(); i++ )
    Objects[i] = &OS[i];
  OS.GetPrimitives().RemoveObject(OS); // 
  FGObjects.Remove(&OS);
  delete &OS;  // it will reset Parent group to NULL in the objects
  for( size_t i=0; i < Objects.Count(); i++ )
    FSelection->Add( *Objects[i] );
  FSelection->SetSelected(true);
}
//..............................................................................
void TGlRenderer::EnableClipPlane(TGlClipPlane *P, bool v)  {
  if( v )  {
    double v[4];
    v[0] = P->Equation()[0];
    v[1] = P->Equation()[1];
    v[2] = P->Equation()[2];
    v[3] = P->Equation()[3];
    olx_gl::clipPlane( P->Id(), &v[0]);
    olx_gl::enable(P->Id());
  }
  else
    olx_gl::disable(P->Id());
}
//..............................................................................
void TGlRenderer::SetProperties(TGlMaterial& P)  {  // tracks translucent and identity objects
  if( P.IsTransparent() && P.IsIdentityDraw() )  {
    FTranslucentIdentityObjects.AddUnique(&P);
    return;
  }
  if( P.IsTransparent() )  {
    FTranslucentObjects.AddUnique(&P);
    return;
  }
  if( P.IsIdentityDraw() )  {
    FIdentityObjects.AddUnique(&P);
    return;
  }
}
//..............................................................................
void TGlRenderer::OnSetProperties(const TGlMaterial& P)  {  // tracks translucent and identity objects
  //if( P == NULL )  return;
  if( P.ObjectCount() > 1 )  return; // the properties will not be removde
  if( P.IsTransparent() && P.IsIdentityDraw() )  {
    size_t index = FTranslucentIdentityObjects.IndexOf(&P);
    if( index != InvalidIndex )  
      FTranslucentIdentityObjects.Delete(index);
    return;
  }
  if( P.IsTransparent() )  {
    size_t index = FTranslucentObjects.IndexOf(&P);
    if( index != InvalidIndex )  
      FTranslucentObjects.Delete(index);
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
void TGlRenderer::RemoveObjects(const AGDObjList& objects)  {
  ACollectionItem::Exclude<>(FGObjects, objects);
}
//..............................................................................
void TGlRenderer::AddObject(AGDrawObject& G)  {
  FGObjects.AddUnique(&G);
  if( FSceneComplete || !G.IsVisible() )
    return;
  vec3d MaxV, MinV;
  if( G.GetDimensions(MaxV, MinV) )
    UpdateMinMax(MinV, MaxV);
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
  FTranslucentIdentityObjects.Clear();
  FTranslucentObjects.Clear();
  FIdentityObjects.Clear();

  Primitives.GetObjects().ForEach(ACollectionItem::TagSetter<>(-1));
  GP.GetPrimitives().ForEach(ACollectionItem::TagSetter<>(0));
  Primitives.RemoveObjectsByTag(0);
  FCollections.Delete(FCollections.IndexOfObject(&GP));
  for( size_t i=0; i < Primitives.PropertiesCount(); i++ )  {
    TGlMaterial& GlM = Primitives.GetProperties(i);
    if( GlM.IsTransparent() && GlM.IsIdentityDraw()  )
      FTranslucentIdentityObjects.Add(GlM);
    else if( GlM.IsTransparent() )
      FTranslucentObjects.Add(GlM);
    else if( GlM.IsIdentityDraw() )
      FIdentityObjects.Add(GlM);
  }
  delete &GP;
}
//..............................................................................
void TGlRenderer::RemoveCollections(const TPtrList<TGPCollection>& Colls)  {
  if( Colls.IsEmpty() )  return;
  FTranslucentIdentityObjects.Clear();
  FTranslucentObjects.Clear();
  FIdentityObjects.Clear();

  Primitives.GetObjects().ForEach(ACollectionItem::TagSetter<>(-1));
  for( size_t i=0; i < Colls.Count(); i++ )  {
    Colls[i]->GetPrimitives().ForEach(ACollectionItem::TagSetter<>(0));
    Primitives.RemoveObjectsByTag(0);
    const size_t col_ind = FCollections.IndexOfObject(Colls[i]);
    FCollections.Remove(col_ind);
    delete Colls[i];
  }
  for( size_t i=0; i < Primitives.PropertiesCount(); i++ )  {
    TGlMaterial& GlM = Primitives.GetProperties(i);
    if( GlM.IsTransparent() && GlM.IsIdentityDraw() )
      FTranslucentIdentityObjects.Add(&GlM);
    else if( GlM.IsTransparent() )   
      FTranslucentObjects.Add(GlM);
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
}
//..............................................................................
char* TGlRenderer::GetPixels(bool useMalloc, short aligment)  {
  char *Bf;
  short extraBytes = (4-(Width*3)%4)%4;  //for bitmaps with 4 bytes aligment
  if( useMalloc )  {
    Bf = (char*)malloc((Width*3+extraBytes)*Height);
  }
  else  {
    Bf = new char[(Width*3+extraBytes)*Height];
  }
  if( Bf == NULL )
    throw TOutOfMemoryException(__OlxSourceInfo);
  olx_gl::readBuffer(GL_BACK);
  olx_gl::pixelStore(GL_PACK_ALIGNMENT, aligment);
  olx_gl::readPixels(0, 0, Width, Height, GL_RGB, GL_UNSIGNED_BYTE, Bf);
  return Bf;
}
//..............................................................................
void TGlRenderer::RemovePrimitiveByTag(int in)  {
  Primitives.RemoveObjectsByTag(in);
  FTranslucentIdentityObjects.Clear();
  FTranslucentObjects.Clear();
  FIdentityObjects.Clear();
  for( size_t i=0; i < Primitives.PropertiesCount(); i++ )  {
    TGlMaterial& GlM = Primitives.GetProperties(i);
    if( GlM.IsTransparent() && GlM.IsIdentityDraw() )
      FTranslucentIdentityObjects.Add(GlM);
    else if( GlM.IsTransparent() )    
      FTranslucentObjects.Add(GlM);
    else if( GlM.IsIdentityDraw() )   
      FIdentityObjects.Add(GlM);
  }
}
//..............................................................................
void TGlRenderer::CleanUpStyles()  {// removes styles, which are not used by any collection
  OnStylesClear.Enter(this);
  GetStyles().SetStylesTag(0);
  for( size_t i=0; i < FCollections.Count(); i++ )
    FCollections.GetObject(i)->GetStyle().SetTag(1);
  GetStyles().RemoveStylesByTag(0);
  OnStylesClear.Exit(this);
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
    GLuint s = olx_gl::genLists(10);
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
    olx_gl::deleteLists(Lists[i], FInc);
  FPos = 0;
  Lists.Clear();
}
//..............................................................................
void TGlListManager::ReserveRange(unsigned int count)  {
  ClearLists();
  GLint s = olx_gl::genLists(count);
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
      CompiledListId = olx_gl::genLists(1);
    }
    olx_gl::newList(CompiledListId, GL_COMPILE);
    for( size_t i=0; i < Primitives.PropertiesCount(); i++ )  {
      TGlMaterial& GlM = Primitives.GetProperties(i);
      if( GlM.IsIdentityDraw() ) continue;  // already drawn
      if( GlM.IsTransparent() ) continue;  // will be drawn
      GlM.Init(false);
      for( size_t j=0; j < GlM.ObjectCount(); j++ )  {
        TGlPrimitive& GlP = (TGlPrimitive&)GlM.GetObject(j);
        TGPCollection* GPC = GlP.GetParentCollection();
        if( GPC == NULL )  continue;
        for( size_t k=0; k < GPC->ObjectCount(); k++ )  {
          AGDrawObject& GDO = GPC->GetObject(k);
          if( !GDO.IsVisible() )  continue;
          if( GDO.IsSelected() ) continue;
          if( GDO.IsGrouped() ) continue;
          olx_gl::pushMatrix();
          if( GDO.Orient(GlP) )  // the object has drawn itself
          {  olx_gl::popMatrix(); continue; }
          GlP.Draw();
          olx_gl::popMatrix();
        }
      }
    }
    olx_gl::endList();
  }
  else  {
    if( CompiledListId != -1 )  {
      olx_gl::deleteLists(CompiledListId, 1);
      CompiledListId = -1;
    }
  }
}
//..............................................................................
void TGlRenderer::DrawText(TGlPrimitive& p, double x, double y, double z)  {
  p.GetFont()->Reset_ATI(ATI);
  olx_gl::rasterPos(x, y, z);
  p.Draw();
}
//..............................................................................
void TGlRenderer::DrawTextSafe(const vec3d& pos, const olxstr& text, const TGlFont& fnt) {
  fnt.Reset_ATI(ATI);
  // set a valid raster position
  olx_gl::rasterPos(0.0, 0.0, pos[2]);
  olx_gl::bitmap(0, 0, 0, 0, (float)(pos[0]/FViewZoom), (float)(pos[1]/FViewZoom), NULL);
  fnt.DrawRasterText(text);
}
//..............................................................................
//..............................................................................
//..............................................................................
void TGlRenderer::LibCompile(const TStrObjList& Params, TMacroError& E)  {
  Compile(Params[0].ToBool());
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
  if( Cmds.Count() == 1 )  {
    SetFogType(GL_LINEAR);
    SetFogStart(0.0f);
    SetFogEnd(2.0f);
    SetFogColor(Cmds[0].SafeUInt<uint32_t>());
    EnableFog(true);
  }
  else
    EnableFog(false);
}
//..............................................................................
void TGlRenderer::LibZoom(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Cmds.IsEmpty() )  {
    SetZoom(CalcZoom());
  }
  else if( Cmds.Count() == 1 ) {
    double zoom = GetZoom() + Cmds[0].ToDouble();
    if( zoom < 0.001 )  zoom = 0.001;
    SetZoom(zoom);
  }
}
//..............................................................................
void TGlRenderer::LibCalcZoom(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal(CalcZoom());
}
//..............................................................................
void TGlRenderer::LibStereo(const TStrObjList& Params, TMacroError& E)  {
  if( Params.IsEmpty() )  {
    if( StereoFlag == glStereoColor )
      E.SetRetVal<olxstr>("color");
    else if( StereoFlag == glStereoCross )
      E.SetRetVal<olxstr>("cross");
    else if( StereoFlag == glStereoAnaglyph )
      E.SetRetVal<olxstr>("anaglyph");
    else if( StereoFlag == glStereoHardware )
      E.SetRetVal<olxstr>("hardware");
    else
      E.SetRetVal<olxstr>("none");
  }
  else  {
    if( OWidth != 0 )  {
      Width = OWidth;
      OWidth = 0;
    }
    if( Params[0].Equalsi("color") )
      StereoFlag = glStereoColor;
    else if( Params[0].Equalsi("anaglyph") )  {
      GLint bits = 0;
      olx_gl::get(GL_ACCUM_RED_BITS, &bits);
      if( bits == 0 )
        TBasicApp::GetLog().Error("Sorry accumulation buffer is not initialised/available");
      else
        StereoFlag = glStereoAnaglyph;
    }
    else if( Params[0].Equalsi("cross") )  {
      olx_gl::clearColor(LightModel.GetClearColor().Data());
      OWidth = Width;
      Width /= 2;
      StereoFlag = glStereoCross;
    }
    else if( Params[0].Equalsi("hardware") )  {
      GLboolean stereo_supported = GL_FALSE;
      olx_gl::get(GL_STEREO, &stereo_supported);
      if( stereo_supported == GL_FALSE )
        TBasicApp::GetLog().Error("Sorry stereo buffers are not initialised/available");
      else  {
        olx_gl::clearColor(LightModel.GetClearColor().Data());
        StereoFlag = glStereoHardware;
      }
    }
    else  {
      olx_gl::clearColor(LightModel.GetClearColor().Data());
      StereoFlag = 0;
    }
    if( Params.Count() == 2 )
      StereoAngle = Params[1].ToDouble();
  }
  TGraphicsStyle& gs = FStyles->NewStyle("GL.Stereo", true);
  gs.SetParam("angle", StereoAngle, true);
}
//..............................................................................
void TGlRenderer::LibStereoColor(const TStrObjList& Params, TMacroError& E)  {
  TGlOption* glo = Params[0].Equalsi("left") ? &StereoLeftColor : 
    (Params[0].Equalsi("right") ? &StereoRightColor : NULL);
  if( glo == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "undefined parameter, left/right is expected");
    return;
  }
  if( Params.Count() == 1 )  {
    E.SetRetVal(glo->ToString());
  }
  if( Params.Count() == 2 )  {
    *glo = Params[1].SafeUInt<uint32_t>();
    (*glo)[3] = 1;
  }
  else if( Params.Count() == 4 )  {
    (*glo)[0] = Params[1].ToFloat<float>();
    (*glo)[1] = Params[2].ToFloat<float>();
    (*glo)[2] = Params[3].ToFloat<float>();
    (*glo)[3] = 1;
  }
  TGraphicsStyle& gs = FStyles->NewStyle("GL.Stereo", true);
  gs.SetParam("left", StereoLeftColor.ToString(), true);
  gs.SetParam("right", StereoRightColor.ToString(), true);
}
//..............................................................................
void TGlRenderer::LibLineWidth(const TStrObjList& Params, TMacroError& E)  {
  if( Params.IsEmpty() )  {
    float LW = 0;
    olx_gl::get(GL_LINE_WIDTH, &LW);
    E.SetRetVal(LW);
  }
  else
    olx_gl::lineWidth(Params[0].ToFloat<float>());
}
//..............................................................................
TLibrary*  TGlRenderer::ExportLibrary(const olxstr& name)  {
  TLibrary* lib = new TLibrary( name.IsEmpty() ? olxstr("gl") : name);
  lib->RegisterFunction<TGlRenderer>( new TFunction<TGlRenderer>(this,  &TGlRenderer::LibCompile, "Compile",
    fpOne, "Compiles or decompiles the model according to the boolean parameter") );
  lib->RegisterMacro<TGlRenderer>( new TMacro<TGlRenderer>(this,  &TGlRenderer::LibPerspective, "Perspective",
    EmptyString, fpNone|fpOne, "Un/Sets perspective view") );
  lib->RegisterMacro<TGlRenderer>( new TMacro<TGlRenderer>(this,  &TGlRenderer::LibFog, "Fog",
    EmptyString, fpNone|fpOne, "fog color - sets fog, fog without arguments removes fog") );
  lib->RegisterMacro<TGlRenderer>( new TMacro<TGlRenderer>(this,  &TGlRenderer::LibZoom, "Zoom",
    EmptyString, fpNone|fpOne, "If no arguments provided - resets zoom to fit to screen, otherwise increments/\
decrements current zoom by provided value") );
  lib->RegisterFunction<TGlRenderer>( new TFunction<TGlRenderer>(this,  &TGlRenderer::LibCalcZoom, "CalcZoom",
    fpNone, "Returns optimal zoom value") );
  lib->RegisterFunction<TGlRenderer>( new TFunction<TGlRenderer>(this,  &TGlRenderer::LibStereo, "Stereo",
    fpNone|fpOne|fpTwo, "Returns/sets color/cross stereo mode and optionally stereo angle [3]") );
  lib->RegisterFunction<TGlRenderer>( new TFunction<TGlRenderer>(this,  &TGlRenderer::LibStereoColor,
    "StereoColor",
    fpOne|fpTwo|fpFour, "Returns/sets colors for left/right color stereo mode glasses") );
  lib->RegisterFunction<TGlRenderer>( new TFunction<TGlRenderer>(this,  &TGlRenderer::LibLineWidth,
    "LineWidth",
    fpNone|fpOne, "Returns/sets width of the raster OpenGl line") );
  lib->AttachLibrary(LightModel.ExportLibrary("lm"));
  return lib;
}
//..............................................................................

