//----------------------------------------------------------------------------//
// TGlTextBox - a text box
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//
#include "gltextbox.h"
#include "styles.h"
#include "glrender.h"
#include "gpcollection.h"
#include "glprimitive.h"
#include "glfont.h"

UseGlNamespace()
//..............................................................................
TGlTextBox::TGlTextBox(TGlRenderer& Render, const olxstr& collectionName):
  AGlMouseHandlerImp(Render, collectionName)
{
  SetMove2D(true);
  SetMoveable(true);
  SetRoteable(false);
  SetZoomable(false);

  LineSpacing = 1;
  Left = Top = 0;
  Width = Height = 0;
  MaxStringLength = 0;
  SetSelectable(false);
  FontIndex = ~0;
  ScrollDirectionUp = true;
  Z = 0;
}
//..............................................................................
TGlTextBox::~TGlTextBox()  {  Clear();  }
//..............................................................................
void TGlTextBox::Create(const olxstr& cName)  {
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);
  TGPCollection& GPC = Parent.FindOrCreateCollection(GetCollectionName());
  GPC.AddObject(*this);
  if( GPC.PrimitiveCount() != 0 )  return;

  TGraphicsStyle& GS = GPC.GetStyle();
  Left = GS.GetParam("Left", Left, true).ToInt();
  Top = GS.GetParam("Top", Top, true).ToInt();
  TGlMaterial GlM;
  GlM.SetFlags(0);   
  GlM.ShininessF = 128;
  GlM.SetFlags(sglmAmbientF|sglmDiffuseF|sglmIdentityDraw|sglmTransparent);
  GlM.AmbientF = 0x800f0f0f;
  GlM.DiffuseF = 0x800f0f0f;

  TGlPrimitive& glpPlane = GPC.NewPrimitive("Plane", sgloQuads);
  glpPlane.SetProperties(GS.GetMaterial("Plane", GlM) );
  glpPlane.Vertices.SetCount(4);

  TGlPrimitive& glpText = GPC.NewPrimitive("Text", sgloText);
  glpText.SetProperties( GS.GetMaterial("Text", GetFont().GetMaterial()) );
  glpText.Params[0] = -1;  //bitmap; TTF by default
}
//..............................................................................
bool TGlTextBox::Orient(TGlPrimitive& P)  {
/*  vec3d Trans;
  Trans = Parent.Basis().Center();
  Trans *= Parent.Basis().Matrix();
  Parent.GlTranslate(-Trans[0], -Trans[1], -Trans[2] );*/
  if( Width == 0 || Height == 0 )
    return true;
  olx_gl::normal(0, 0, 1);
  if( P.GetType() == sgloText )  {
    TGlFont& Fnt = GetFont();
    const double rscale = Parent.GetExtraZoom()*Parent.GetViewZoom();
    const uint16_t th = Fnt.TextHeight(EmptyString);
    const double GlLeft = ((Left+GetCenter()[0])*rscale - (double)Parent.GetWidth()/2) + 0.1;
    const double GlTop = ((double)Parent.GetHeight()/2 -
      (Top-GetCenter()[1]+Height)*rscale) + 0.1;
    const double LineSpacer = (0.05+LineSpacing-1)*th;
    const double scale = Parent.GetViewZoom() == 1.0 ? 1.0 : 1./Parent.GetExtraZoom();
    bool mat_changed = false;
    vec3d T(GlLeft, GlTop, Z);
    for( size_t i=0; i < FBuffer.Count(); i++ )  {
      const size_t ii = FBuffer.Count() - i - 1;
      TGlMaterial* GlM = FBuffer.GetObject(ii);
      if( GlM != NULL )  {
        GlM->Init(Parent.IsColorStereo());
        mat_changed = true;
      }
      olxstr const line = FBuffer[ii].SubStringTo(Fnt.LengthForWidth(FBuffer[ii], Parent.GetWidth()));
      const TTextRect tr = Fnt.GetTextRect(line);
      T[1] -= tr.top*scale;
      Parent.DrawTextSafe(T, line, Fnt);
      T[1] += (olx_max(tr.height, Fnt.GetMaxHeight())+LineSpacer)*scale;
    }
    if( mat_changed )
      P.GetProperties().Init(Parent.IsColorStereo());
    return true;
  }
  else  {
    double Scale = Parent.GetScale();
    const double hw = Parent.GetWidth()*Scale/2;
    const double hh = Parent.GetHeight()*Scale/2;
    Scale = Scale*Parent.GetExtraZoom()*Parent.GetViewZoom();
    double xx = GetCenter()[0], xy = -GetCenter()[1];
    const double z = Z-0.1;
    P.Vertices[0] = vec3d((Left+Width+xx)*Scale-hw, hh-(Top+Height+xy)*Scale, z);
    P.Vertices[1] = vec3d((Left+Width+xx)*Scale-hw, hh-(Top+xy)*Scale, z);
    P.Vertices[2] = vec3d((Left+xx)*Scale-hw, hh-(Top+xy)*Scale, z);
    P.Vertices[3] = vec3d((Left+xx)*Scale-hw, hh-(Top+Height+xy)*Scale, z); 
    return false;
  }
}
//..............................................................................
void TGlTextBox::Clear()  {
  for( size_t i=0; i < FBuffer.Count(); i++ )
    if( FBuffer.GetObject(i) != NULL )
      delete FBuffer.GetObject(i);

  FBuffer.Clear();
  Width = Height = 0;
}
//..............................................................................
void TGlTextBox::PostText(const olxstr& S, TGlMaterial* M)  {
  if( S.IndexOf('\n') != InvalidIndex )  {
    TStrList toks(S, '\n');
    PostText(toks, M);
    return;
  }
  if( MaxStringLength && (S.Length() > MaxStringLength) )  {
    TStrList Txt;
    Txt.Hyphenate(S, MaxStringLength, true);
    PostText(Txt, M);
    return;
  }
  FBuffer.Add(S, M != NULL ? new TGlMaterial(*M) : NULL);
  const size_t width = GetFont().TextWidth(S);
  if( width > Width )
    Width = (uint16_t)(width + 3);
}
//..............................................................................
void TGlTextBox::PostText(const TStrList &SL, TGlMaterial *M)  {
  size_t position = FBuffer.Count();
  for( size_t i=0; i < SL.Count(); i++ )
    PostText(SL[i], NULL);
  if( M != NULL )
    FBuffer.GetObject(position) = new TGlMaterial(*M);
}
//..............................................................................
void TGlTextBox::Fit()  {
  if( FBuffer.Count() > 1 )  {
    const TGlFont& glf = GetFont();
    const uint16_t th = glf.TextHeight(EmptyString);
    const double LineSpacer = (0.05+LineSpacing-1)*th;
    Height = 0;
    for( size_t i=0; i < FBuffer.Count(); i++ )  {
      const TTextRect tr = glf.GetTextRect(FBuffer[i]);
      Height -= (uint16_t)olx_round(tr.top);
      Height += (uint16_t)olx_round(olx_max(tr.height, glf.GetMaxHeight())+LineSpacer);
    }
  }
  else if( FBuffer.Count() == 1 )  {
    const TTextRect tr = GetFont().GetTextRect(FBuffer[0]);
    Height = (uint16_t)olx_round(tr.height);
    Width = (uint16_t)olx_round(tr.width);
  }
  else  {
    Height = Width = 0;
  }
}
//..............................................................................
void TGlTextBox::SetLeft(int l)  {
  Left = l;
  GetPrimitives().GetStyle().SetParam("Left", Left, true);
}
//..............................................................................
void TGlTextBox::SetTop(int t)  {
  Top = t;
  GetPrimitives().GetStyle().SetParam("Top", Top, true);
}
//..............................................................................
bool TGlTextBox::OnMouseUp(const IEObject *Sender, const TMouseData& Data)  {
  SetLeft((int)(Left + GetCenter()[0]));
  SetTop((int)(Top - GetCenter()[1]));
  Center.Null();
  return AGlMouseHandlerImp::OnMouseUp(Sender, Data);
}
//..............................................................................
TGlFont& TGlTextBox::GetFont() const {  return Parent.GetScene().GetFont(FontIndex, true);  }
//..............................................................................
