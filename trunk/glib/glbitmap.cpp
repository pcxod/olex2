#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "glbitmap.h"
#include "glrender.h"
#include "styles.h"
#include "gpcollection.h"
#include "gltexture.h"


TGlBitmap::TGlBitmap(const olxstr& collectionName, TGlRender *Render, 
  int left, int top, int width, int height,
  unsigned char* RGB, GLenum format) : TGlMouseListener(collectionName, Render)  {

  Z = -10.0;
  Left = left;
  Top = top;
  Width = width;
  Height = height;

  Move2D(true);
  Moveable(true);
  Groupable(false);
  Roteable(false);
  Zoomable(false);

  TextureId = Render->GetTextureManager().Add2DTexture(GetCollectionName(), 0, width, height, 0,
                         format, RGB);

  TGlTexture* tex = Render->GetTextureManager().GetTexture(TextureId);
  tex->SetEnvMode( tpeDecal );
  tex->SetSCrdWrapping( tpCrdClamp );
  tex->SetTCrdWrapping( tpCrdClamp );

  tex->SetMagFilter( tpFilterNearest );
  tex->SetMinFilter( tpFilterLinear );
  tex->SetEnabled( true );
}
void TGlBitmap::Create(const olxstr& cName)  {
  if( cName.Length() != 0)  SetCollectionName(cName);
  TGlPrimitive *GlP;
  TGPCollection *GPC;
  TGraphicsStyle *GS;

  TGlTexture* tex = FParent->GetTextureManager().GetTexture(TextureId);

  TGlMaterial *GlM;
  olxstr Name = EsdlObjectName(*this) + tex->GetName();
  GPC = FParent->FindCollection( GetCollectionName() );
  if( !GPC )    GPC = FParent->NewCollection( GetCollectionName() );
  else  {
    GPC->AddObject(this);
    return;
  }

  GS = GPC->Style();
  GPC->AddObject(this);
  Left = GS->ParameterValue("Left", Left).ToInt();
  Top = GS->ParameterValue("Top", Top).ToInt();
  Z = GS->ParameterValue("Z", Z).ToDouble();

  GlM = const_cast<TGlMaterial*>( GS->Material("Plane") );
  if( GlM->Mark() )  {
    GlM->SetFlags(0);   GlM->ShininessF = 128;
    GlM->SetFlags(sglmAmbientF|sglmDiffuseF|sglmIdentityDraw|sglmTransparent);
    GlM->AmbientF = 0x800f0f0f;
    GlM->DiffuseF = 0x800f0f0f;
  }

  GlP = GPC->NewPrimitive("Plane");  //
  GlP->Texture( TextureId );
  GlP->SetProperties(GlM);
  GlP->Type(sgloQuads);
  GlP->Data().Resize(5, 4);
  // texture coordinates
  GlP->Data()[3][0] = 0;  GlP->Data()[4][0] = 0;
  GlP->Data()[3][1] = 1;  GlP->Data()[4][1] = 0;
  GlP->Data()[3][2] = 1;  GlP->Data()[4][2] = 1;
  GlP->Data()[3][3] = 0;  GlP->Data()[4][3] = 1;
}

TGlBitmap::~TGlBitmap()  {
}

void TGlBitmap::ReplaceData(int width, int height, unsigned char* RGB, GLenum format) {
  TGlTexture* tex = FParent->GetTextureManager().GetTexture(TextureId);
  FParent->GetTextureManager().Replace2DTexture(*tex, 0, width, height, 0,
                         format, RGB);
}
bool TGlBitmap::Orient(TGlPrimitive *P)  {
  P->Texture( TextureId );
  double hw = Parent()->GetWidth()/2;
  double hh = Parent()->GetHeight()/2;
  double xx = Basis.GetCenter()[0],
         xy = -Basis.GetCenter()[1],
         zm = Basis.GetZoom();
  P->Data()[0][0] = (Left+Width*zm)-hw + xx;  P->Data()[1][0] = hh-(Top+Height*zm) - xy ;
  P->Data()[0][1] = P->Data()[0][0];             P->Data()[1][1] = hh-Top - xy;
  P->Data()[0][2] = Left-hw + xx;          P->Data()[1][2] = P->Data()[1][1];
  P->Data()[0][3] = P->Data()[0][2];             P->Data()[1][3] = P->Data()[1][0];
  P->Data()[2][0] = Z;
  P->Data()[2][1] = Z;
  P->Data()[2][2] = Z;
  P->Data()[2][3] = Z;

  Parent()->GlScale( (float)(FParent->GetScale()*FParent->GetExtraZoom()) );
  return false;
}

bool TGlBitmap::GetDimensions(TVPointD &Max, TVPointD &Min)  {
  return false;
}

void TGlBitmap::SetWidth(int w)   {
  Width = w;
//  Primitives()->Style()->ParameterValue("Width") = w;
}
void TGlBitmap::SetHeight(int w)  {
  Height = w;
//  Primitives()->Style()->ParameterValue("Height") = w;
}
void TGlBitmap::SetLeft(int w)    {
  Left = w;
  Primitives()->Style()->SetParameter("Left", w);
}
void TGlBitmap::SetTop(int w)     {
  Top = w;
  Primitives()->Style()->SetParameter("Top", w);
}
void TGlBitmap::SetZ( double z )  {
  Z = z;
  Primitives()->Style()->SetParameter("Z", z);
}

int TGlBitmap::GetWidth() const   {  return (int)(Width*Basis.GetZoom());  }
int TGlBitmap::GetHeight() const  {  return (int)(Height*Basis.GetZoom());  }
int TGlBitmap::GetLeft()  const   {  return Left; }
int TGlBitmap::GetTop()  const    {  return Top; }

