//----------------------------------------------------------------------------//
// namespace TEXLib
// TGlXLabel - a drawing object for atom label
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "gllabel.h"
#include "gpcollection.h"
#include "styles.h"

#include "glrender.h"
#include "glscene.h"
#include "glprimitive.h"

#include "pers_util.h"

TXGlLabel::TXGlLabel(TGlRenderer& R, const olxstr& collectionName) :
  TGlMouseListener(R, collectionName)  
{
  SetMove2DZ(true);
  SetMoveable(true);
  SetZoomable(false);
  SetGroupable(true);
  FontIndex = ~0;
};
TXGlLabel::~TXGlLabel(){}

void TXGlLabel::Create(const olxstr& cName, const ACreationParams* cpar)  {
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);

  TGPCollection& GPC = Parent.FindOrCreateCollection( GetCollectionName() );
  GPC.AddObject(*this);
  if( GPC.PrimitiveCount() != 0 )  return;

  TGraphicsStyle& GS = GPC.GetStyle();
  GS.SetPersistent(true);
  TGlMaterial GlM;
  GlM.SetFlags( sglmAmbientF|sglmIdentityDraw|sglmTransparent  );
  GlM.AmbientF = 0x800f0f0f;
  GlM.DiffuseF = 0x800f0f0f;
  GlM.ShininessF = 128;

  TGlPrimitive& glpPlane = GPC.NewPrimitive("Plane", sgloQuads);  // a sphere at the basis of the object {0,0,0}
  glpPlane.SetProperties( GS.GetMaterial("Plane", GlM) );
  glpPlane.Vertices.SetCount(4);

  TGlPrimitive& glpText = GPC.NewPrimitive("Text", sgloText);
  glpText.SetProperties( GS.GetMaterial("Text", GetFont().GetMaterial()) );
  glpText.Params[0] = -1;  //bitmap; TTF by default
  glpText.SetFont(&GetFont());
}
//..............................................................................
void TXGlLabel::SetLabel(const olxstr& L)   {
  FLabel = L;  
  TGlFont& glf = GetFont();
  if( glf.IsVectorFont() )  {
    text_rect = glf.GetTextRect(FLabel);
  }
  else  {
    text_rect.width = (double)glf.TextWidth(FLabel);
    text_rect.height = (double)glf.TextHeight(FLabel);
  }
}
//..............................................................................
vec3d TXGlLabel::GetRasterPosition() const {
  vec3d T(Parent.GetBasis().GetCenter());
  const double Scale = Parent.GetScale();
  const double ScaleR = Parent.GetExtraZoom()*Parent.GetViewZoom();
  vec3d off = (Basis.GetCenter()*Parent.GetBasis().GetZoom());
  T += Center;
  T *= Parent.GetBasis().GetMatrix();
  T += off*(Scale*ScaleR);
  T /= Scale;
  return T;
}
//..............................................................................
vec3d TXGlLabel::GetVectorPosition() const {
  vec3d T(Parent.GetBasis().GetCenter());
  const double Scale = Parent.GetScale();
  const double ScaleR = Parent.GetExtraZoom()*Parent.GetViewZoom();
  const double ScaleVR = Scale*ScaleR;
  vec3d off = Parent.GetBasis().GetMatrix()*(Basis.GetCenter()*Parent.GetBasis().GetZoom());
  T += Center;
  T += off*(ScaleVR);
  return T * Parent.GetBasis().GetMatrix();
}
//..............................................................................
bool TXGlLabel::Orient(TGlPrimitive& P)  {
  vec3d T(Parent.GetBasis().GetCenter());
  const double Scale = Parent.GetScale();
  const double ScaleR = Parent.GetExtraZoom()*Parent.GetViewZoom();
  const double ScaleVR = Scale*ScaleR;
  TGlFont& glf = GetFont();
  if( P.GetType() == sgloText )  {
    if( !glf.IsVectorFont() )  {
      vec3d off = (Basis.GetCenter()*Parent.GetBasis().GetZoom());
      T += Center;
      T *= Parent.GetBasis().GetMatrix();
      T += off*(ScaleVR);
      T /= Scale;
      T[2] = Parent.GetMaxRasterZ();
      Parent.DrawTextSafe(T, FLabel, glf);
    }
    else  {
      vec3d off = Parent.GetBasis().GetMatrix()*(Basis.GetCenter()*Parent.GetBasis().GetZoom());
      T += Center;
      T += off*(ScaleVR);
      T *= Parent.GetBasis().GetMatrix();
      T[2] = Parent.MaxDim()[2];
      //float glw;
      //glGetFloatv(GL_LINE_WIDTH, &glw);
      //glLineWidth((float)(1./Scale)/50);
      glf.DrawGlText(T, FLabel, true);
      //glLineWidth(glw);
    }
    return true;
  }
  else  {
    vec3d off = Parent.GetBasis().GetMatrix()*(Basis.GetCenter()*Parent.GetBasis().GetZoom());
    T += Center;
    T += off*ScaleVR;
    T *= Parent.GetBasis().GetMatrix();
    T[2] = Parent.MaxDim()[2];
    Parent.GlTranslate(T);
    if( !glf.IsVectorFont() )  {
      P.Vertices[0] = vec3d(0, 0, -0.1);
      P.Vertices[1] = vec3d(text_rect.width*ScaleVR, 0, -0.1);
      P.Vertices[2] = vec3d(text_rect.width*ScaleVR, text_rect.height*ScaleVR, -0.1);
      P.Vertices[3] = vec3d(0, text_rect.height*ScaleVR, -0.1);
    }
    else  {
      P.Vertices[0] = vec3d(text_rect.left, text_rect.top, -0.1);
      P.Vertices[1] = vec3d(text_rect.left+text_rect.width, text_rect.top, -0.1);
      P.Vertices[2] = vec3d(text_rect.left+text_rect.width, text_rect.top+text_rect.height, -0.1);
      P.Vertices[3] = vec3d(text_rect.left, text_rect.top+text_rect.height, -0.1);
    }
  }
  return false;
}
//..............................................................................
TGlFont& TXGlLabel::GetFont() const {  
  TGlFont* fnt = Parent.GetScene().GetFont(FontIndex); 
  if( fnt == NULL )
    throw TInvalidArgumentException(__OlxSourceInfo, "font index");
  return *fnt;
}
//..............................................................................
void TXGlLabel::ToDataItem(TDataItem& item) const {
  item.AddField("text", FLabel);
  item.AddField("visible", IsVisible());
  item.AddField("font_id", FontIndex);
  item.AddField("center", PersUtil::VecToStr(Center));
  Basis.ToDataItem(item.AddItem("Basis"));
}
//..............................................................................
void TXGlLabel::FromDataItem(const TDataItem& item) {
  SetVisible( item.GetRequiredField("visible").ToBool() );
  FontIndex = item.GetRequiredField("font_id").ToInt();
  SetLabel( item.GetRequiredField("text") );
  Center = PersUtil::FloatVecFromStr( item.GetRequiredField("center") );
  Basis.FromDataItem( item.FindRequiredItem("Basis") );
}
//..............................................................................
