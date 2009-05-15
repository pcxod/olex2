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

#include "pers_util.h"

TXGlLabel::TXGlLabel(const olxstr& collectionName, TGlRenderer *Render) :
    TGlMouseListener(collectionName, Render)  {
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

  TGPCollection* GPC = FParent->FindCollection( GetCollectionName() );
  if( GPC == NULL )   
    GPC = FParent->NewCollection( GetCollectionName() );
  GPC->AddObject(this);
  if( GPC->PrimitiveCount() != 0 )  return;

  TGraphicsStyle* GS = GPC->Style();
  GS->SetPersistent(true);
  TGlMaterial* GlM = const_cast<TGlMaterial*>(GS->Material("Plane"));
  if( GlM->HasMark() )  {
    GlM->SetFlags( sglmAmbientF|sglmIdentityDraw|sglmTransparent  );
    GlM->AmbientF = 0x800f0f0f;
    GlM->DiffuseF = 0x800f0f0f;
    GlM->ShininessF = 128;
    GlM->SetMark(false);
  }
  TGlPrimitive* GlP = GPC->NewPrimitive("Plane", sgloQuads);  // a sphere at the basis of the object {0,0,0}
  GlP->SetProperties(GlM);
  GlP->Data.Resize(3, 4);

  GlM = const_cast<TGlMaterial*>(GS->Material("Text"));
  if( GlM->HasMark() )
    *GlM = Font()->GetMaterial();

  GlP = GPC->NewPrimitive("Text", sgloText);
  GlP->SetProperties(GlM);
  GlP->Params[0] = -1;  //bitmap; TTF by default
  GlP->SetFont(Font());
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
  vec3d T( FParent->GetBasis().GetCenter() );
  const double Scale = FParent->GetScale();
  const double ScaleR = FParent->GetExtraZoom()*FParent->GetViewZoom();
  T += Center;
  T *= FParent->GetBasis().GetMatrix();
  T /= Scale;
  vec3d off(Basis.GetCenter()[0]-OffsetX, Basis.GetCenter()[1]-OffsetY, Basis.GetCenter()[2]);
  off *= ScaleR;
  return T += off;
}
//..............................................................................
bool TXGlLabel::Orient(TGlPrimitive *P)  {
  vec3d T( FParent->GetBasis().GetCenter() );
  const double Scale = FParent->GetScale();
  const double ScaleR = FParent->GetExtraZoom()*FParent->GetViewZoom();
  const double ScaleVR = Scale*ScaleR;
  if( P->GetType() == sgloText )  {
    T += Center;
    T *= FParent->GetBasis().GetMatrix();
    T[2] += 5;
    T /= Scale;
    vec3d off(Basis.GetCenter()[0]-OffsetX, Basis.GetCenter()[1]-OffsetY, Basis.GetCenter()[2]);
    off *= ScaleR;
    T += off;
    T[2] = FParent->GetMaxRasterZ();
    FParent->DrawTextSafe(T, FLabel, *Font());
    return true;
  }
  else  {
    double xinc = OffsetX*ScaleVR;
    double yinc = OffsetY*ScaleVR;
    vec3d off(Basis.GetCenter()[0], Basis.GetCenter()[1], Basis.GetCenter()[2]);
    off = FParent->GetBasis().GetMatrix()*off;  // unproject
    T += Center;
    T += off*ScaleVR;
    T *= FParent->GetBasis().GetMatrix();
    T[2] += 4.8;
    Parent()->GlTranslate(T);
    P->Data[0][0] = -xinc;  P->Data[1][0] = yinc;
    P->Data[0][1] = xinc;   P->Data[1][1] = yinc;
    P->Data[0][2] = xinc;   P->Data[1][2] = -yinc;
    P->Data[0][3] = -xinc;  P->Data[1][3] = -yinc;
    P->Data[2][0] = -0.1;
    P->Data[2][1] = -0.1;
    P->Data[2][2] = -0.1;
    P->Data[2][3] = -0.1;
  }
  return false;
}
//..............................................................................
TGlFont* TXGlLabel::Font() const {  return FParent->Scene()->Font(FFontIndex); }
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
