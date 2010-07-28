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
TGlTextBox::~TGlTextBox()  { Clear();  }
//..............................................................................
void TGlTextBox::Create(const olxstr& cName, const ACreationParams* cpar)  {
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);
  TGPCollection& GPC = Parent.FindOrCreateCollection( GetCollectionName() );
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
 olx_gl::normal(0, 0, 1);
  if( P.GetType() == sgloText )  {
    TGlFont& Fnt = GetFont();
    uint16_t th = Fnt.TextHeight(EmptyString);
    double Scale = Parent.GetScale();
    double GlLeft = ((double)Left - (double)Parent.GetWidth()/2 + GetCenter()[0]) + 0.1;
    double GlTop = ((double)Parent.GetHeight()/2 - (Top-GetCenter()[1])) + 0.1;
    double LineInc = (th*LineSpacing)*Parent.GetViewZoom();
    const size_t tl = (size_t)((double)Fnt.MaxTextLength(((Left+Width) > Parent.GetWidth()) ? 
      Parent.GetWidth()-Left : Width)/Parent.GetViewZoom());
    for( size_t i=0; i < FBuffer.Count() ; i++ )  {
      const vec3d T(GlLeft, GlTop - (i+1)*LineInc, Z);
      TGlMaterial* GlM = FBuffer.GetObject(i);
      if( GlM != NULL ) 
        GlM->Init(Parent.IsColorStereo());
      Parent.DrawTextSafe(T, (tl < FBuffer[i].Length()) ? FBuffer[i].SubStringTo(tl) : FBuffer[i], Fnt); 
    }
    return true;
  }
  else  {
    double Scale = Parent.GetScale();
    double hw = Parent.GetWidth()*Scale/2;
    double hh = Parent.GetHeight()*Scale/2;
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
  olxstr Tmp = S;
  Tmp.SetCapacity(S.CharCount('\t')*8);
  for( size_t i=0; i < Tmp.Length(); i++ )  {
    if( Tmp.CharAt(i) == '\t' )  {
      Tmp[i] = ' ';
      int count = 4-i%4-1;
      if( count > 0 ) Tmp.Insert(' ', i, count);
    }
  }
  if( MaxStringLength && (Tmp.Length() > MaxStringLength) )  {
    TStrList Txt;
    Txt.Hyphenate(Tmp, MaxStringLength, true);
    PostText(Txt, M);
    return;
  }
  if( M != NULL )  {
    TGlMaterial *GlM = new TGlMaterial;
    *GlM = *M;
    FBuffer.Add(Tmp, GlM);
  }
  else  {
    FBuffer.Add(S);
  }
  size_t width = GetFont().TextWidth(Tmp);
  if( width > Width )  Width = (uint16_t)(width + 3);
  if( FBuffer.Count() > 1 )
    Height = (int)(GetFont().TextHeight()*(LineSpacing)*FBuffer.Count());
  else
    Height = GetFont().TextHeight(FBuffer[0]);
}
//..............................................................................
void TGlTextBox::PostText(const TStrList &SL, TGlMaterial *M)  {
  size_t position = FBuffer.Count();
  for( size_t i=0; i < SL.Count(); i++ )
    PostText(SL[i], NULL);
  if( M != NULL )  {
    TGlMaterial *GlM = new TGlMaterial;
    *GlM = *M;
//    FBuffer.Object(FBuffer.Count()-1) = GlM;
    FBuffer.GetObject(position) = GlM;
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
  SetLeft( (int)(Left + GetCenter()[0]) );
  SetTop( (int)(Top - GetCenter()[1]) );
  Center.Null();
  return AGlMouseHandlerImp::OnMouseUp(Sender, Data);
}
//..............................................................................
TGlFont& TGlTextBox::GetFont() const {  return Parent.GetScene().GetFont(FontIndex, true);  }
//..............................................................................


