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
  SetMove2D(true);
  SetMoveable(true);
  SetZoomable(false);
  SetGroupable(false);
  OffsetX = OffsetY = 0;
  FFontIndex = -1;
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
  glpText.SetProperties( GS.GetMaterial("Text", Font()->GetMaterial()) );
  glpText.Params[0] = -1;  //bitmap; TTF by default
  glpText.SetFont(Font());
}
//..............................................................................
void TXGlLabel::SetLabel(const olxstr& L)   {
  FLabel = L;  
  if( Font() != NULL )  {
    OffsetX = Font()->TextWidth( FLabel )/2;
    OffsetY = Font()->TextHeight()/2;
  }
}
//..............................................................................
vec3d TXGlLabel::GetRasterPosition() const {
  vec3d T( Parent.GetBasis().GetCenter() );
  const double Scale = Parent.GetScale();
  const double ScaleR = Parent.GetExtraZoom()*Parent.GetViewZoom();
  T += Center;
  T *= Parent.GetBasis().GetMatrix();
  T /= Scale;
  vec3d off(Basis.GetCenter()[0]-OffsetX, Basis.GetCenter()[1]-OffsetY, Basis.GetCenter()[2]);
  off *= ScaleR;
  return T += off;
}
//..............................................................................
bool TXGlLabel::Orient(TGlPrimitive& P)  {
  vec3d T( Parent.GetBasis().GetCenter() );
  const double Scale = Parent.GetScale();
  const double ScaleR = Parent.GetExtraZoom()*Parent.GetViewZoom();
  const double ScaleVR = Scale*ScaleR;
  if( P.GetType() == sgloText )  {
    T += Center;
    T *= Parent.GetBasis().GetMatrix();
    T[2] += 5;
    T /= Scale;
    vec3d off(Basis.GetCenter()[0]-OffsetX, Basis.GetCenter()[1]-OffsetY, Basis.GetCenter()[2]);
    off *= ScaleR;
    T += off;
    T[2] = Parent.GetMaxRasterZ();
    Parent.DrawTextSafe(T, FLabel, *Font());
    return true;
  }
  else  {
    double xinc = OffsetX*ScaleVR;
    double yinc = OffsetY*ScaleVR;
    vec3d off(Basis.GetCenter()[0], Basis.GetCenter()[1], Basis.GetCenter()[2]);
    off = Parent.GetBasis().GetMatrix()*off;  // unproject
    T += Center;
    T += off*ScaleVR;
    T *= Parent.GetBasis().GetMatrix();
    T[2] += 4.8;
    Parent.GlTranslate(T);
    P.Vertices[0] = vec3d(-xinc, yinc, -0.1);
    P.Vertices[1] = vec3d(xinc, yinc, -0.1);
    P.Vertices[2] = vec3d(xinc, -yinc, -0.1);
    P.Vertices[4] = vec3d(-xinc, -yinc, -0.1);
  }
  return false;
}
//..............................................................................
TGlFont* TXGlLabel::Font() const {  return Parent.GetScene().GetFont(FFontIndex); }
//..............................................................................
void TXGlLabel::ToDataItem(TDataItem& item) const {
  item.AddField("text", FLabel);
  item.AddField("visible", IsVisible());
  item.AddField("font_id", FFontIndex);
  item.AddField("center", PersUtil::VecToStr(Center));
  Basis.ToDataItem(item.AddItem("Basis"));
}
//..............................................................................
void TXGlLabel::FromDataItem(const TDataItem& item) {
  SetVisible( item.GetRequiredField("visible").ToBool() );
  FFontIndex = item.GetRequiredField("font_id").ToInt();
  SetLabel( item.GetRequiredField("text") );
  Center = PersUtil::FloatVecFromStr( item.GetRequiredField("center") );
  Basis.FromDataItem( item.FindRequiredItem("Basis") );
}
//..............................................................................
