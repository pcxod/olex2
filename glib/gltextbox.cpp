//----------------------------------------------------------------------------//
// namespace TEXLib
// TGlTextBox - a text box
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "gltextbox.h"
#include "styles.h"
#include "glrender.h"
#include "glscene.h"
#include "gpcollection.h"
#include "glfont.h"

UseGlNamespace()
//..............................................................................
//..............................................................................

TGlTextBox::TGlTextBox(const olxstr& collectionName, TGlRender *Render):
  TGlMouseListener(collectionName, Render)
{
  Move2D(true);
  Moveable(true);
  Groupable(true);
  Roteable(false);
  Zoomable(false);

  LineSpacing = 1;
  Left = Top = 0;
  Width = Height = 0;
  MaxStringLength = 0;
  Groupable(false);
  FontIndex = -1;
  FScrollDirectionUp = true;
  Z = 0;
}
//..............................................................................
TGlTextBox::~TGlTextBox()
{ Clear(); }
//..............................................................................
void TGlTextBox::Create(const olxstr& cName)  {
  if( cName.Length() != 0)  SetCollectionName(cName);
  TGlPrimitive *GlP;
  TGPCollection *GPC;
  TGraphicsStyle *GS;

  TGlMaterial *GlM;

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

  GlM = const_cast<TGlMaterial*>(GS->Material("Plane"));
  if( GlM->Mark() )  {
    GlM->SetFlags(0);   GlM->ShininessF = 128;
    GlM->SetFlags(sglmAmbientF|sglmDiffuseF|sglmIdentityDraw|sglmTransparent);
    GlM->AmbientF = 0x800f0f0f;
    GlM->DiffuseF = 0x800f0f0f;
  }

  GlP = GPC->NewPrimitive("Plane");  // a sphere at the basis of the object {0,0,0}
  GlP->SetProperties(GlM);
  GlP->Type(sgloQuads);
  GlP->Data().Resize(3, 4);

  GlM = const_cast<TGlMaterial*>(GS->Material("Text"));
  if( GlM->Mark() )  {
    if( FontIndex == -1 )  FontIndex = 0;
    *GlM = Font()->GetMaterial();
  }
  GlP = GPC->NewPrimitive("Text");
  GlP->SetProperties(GlM);
  GlP->Type(sgloText);
  GlP->Params()[0] = -1;  //bitmap; TTF by default
}
//..............................................................................
bool TGlTextBox::Orient(TGlPrimitive *P)  {
  static olxstr stString;
/*  TVPointD Trans;
  Trans = FParent->Basis().Center();
  Trans *= FParent->Basis().Matrix();
  FParent->GlTranslate(-Trans[0], -Trans[1], -Trans[2] );*/

  glNormal3d(0, 0, 1);

  TGlFont *Fnt = Font();
  if( !Fnt )  return true;

  if( P->Type() == sgloText )  {
    P->Font(Fnt);
    int th = Fnt->TextHeight(EmptyString);
    double Scale = FParent->GetScale();
    double GlLeft = ((double)Left - (double)FParent->GetWidth()/2 + Basis.GetCenter()[0]) + 0.1;
    double GlTop = ((double)FParent->GetHeight()/2 - (Top-Basis.GetCenter()[1]))*FParent->GetExtraZoom() + 0.1;
    double LineInc = (th*LineSpacing)*FParent->GetViewZoom();
    TVPointD T;
    TGlMaterial *GlM;
    for(int i=0; i < FBuffer.Count() ; i++ )  {
      T[0] = GlLeft;
      T[1] = GlTop - (i+1)*LineInc;
      T *= Scale;
      T[2] = Z;  
      GlM = FBuffer.Object(i);
      if( GlM ) GlM->Init();
      Fnt->DrawTextSafe(T, Scale*FParent->GetViewZoom(), FBuffer[i] ); 
//      glRasterPos3d(T[0], T[1], Z);
//      stString = FBuffer.String(i);
//      P->String( &stString );
//      P->Draw();
    }
    return true;
  }
  else  {
    double Scale = Parent()->GetScale();
    double hw = Parent()->GetWidth()*Scale/2;
    double hh = Parent()->GetHeight()*Scale/2;
    Scale *= FParent->GetExtraZoom();
    double xx = Basis.GetCenter()[0], xy = -Basis.GetCenter()[1];
    P->Data()[0][0] = (Left+Width+xx)*Scale-hw;  P->Data()[1][0] = hh-(Top+Height+xy)*Scale;
    P->Data()[0][1] = (Left+Width+xx)*Scale-hw;  P->Data()[1][1] = hh-(Top+xy)*Scale;
    P->Data()[0][2] = (Left+xx)*Scale-hw; P->Data()[1][2] = hh-(Top+xy)*Scale;
    P->Data()[0][3] = (Left+xx)*Scale-hw; P->Data()[1][3] = hh-(Top+Height+xy)*Scale;
    P->Data()[2][0] = Z-0.1;
    P->Data()[2][1] = Z-0.1;
    P->Data()[2][2] = Z-0.1;
    P->Data()[2][3] = Z-0.1;
    return false;
  }
}
//..............................................................................
void TGlTextBox::Clear()  {
  for( int i=0; i < FBuffer.Count(); i++ )
    if( FBuffer.Object(i) != NULL )
      delete FBuffer.Object(i);

  FBuffer.Clear();
  Width = Height = 0;
}
//..............................................................................
void TGlTextBox::PostText(const olxstr& S, TGlMaterial* M)  {
  olxstr Tmp = S;
  int count;
  for(int i=0; i < Tmp.Length(); i++ )  {
    if( Tmp[i] == '\t' )  {
      Tmp[i] = ' ';
      count = 4-i%4-1;
      if( count > 0 ) Tmp.Insert(' ', i, count);
    }
  }
  if( MaxStringLength && (Tmp.Length() > MaxStringLength) )  {
    TStrList Txt;
    Txt.Hypernate(Tmp, MaxStringLength, true);
    PostText(Txt, M);
    return;
  }
  if( M )  {
    TGlMaterial *GlM = new TGlMaterial;
    *GlM = *M;
    FBuffer.Add(Tmp, GlM);
  }
  else  {
    FBuffer.Add(S);
  }
  int width = Font()->TextWidth(Tmp);
  if( width > Width )  Width = width + 3;
  if( FBuffer.Count() > 1 )
    Height = (int)(Font()->TextHeight()*(LineSpacing)*FBuffer.Count());
  else
    Height = Font()->TextHeight(FBuffer[0]);
}
//..............................................................................
void TGlTextBox::PostText(const TStrList &SL, TGlMaterial *M)
{
  int position = FBuffer.Count();
  for( int i=0; i < SL.Count(); i++ )
  {  PostText(SL.String(i), NULL);  }
  if( M )
  {
    TGlMaterial *GlM = new TGlMaterial;
    *GlM = *M;
//    FBuffer.Object(FBuffer.Count()-1) = GlM;
    FBuffer.Object(position) = GlM;
  }
}
//..............................................................................
void TGlTextBox::SetLeft(int l)  {
  Left = l;
  Primitives()->Style()->SetParameter("Left", Left);
}
//..............................................................................
void TGlTextBox::SetTop(int t)  {
  Top = t;
  Primitives()->Style()->SetParameter("Top", Top);
}
//..............................................................................
bool TGlTextBox::OnMouseUp(const IEObject *Sender, const TMouseData *Data)  {
  SetLeft( (int)(Left + Basis.GetCenter()[0]) );
  SetTop( (int)(Top - Basis.GetCenter()[1]) );

  TVPointD Null;
  Basis.SetCenter(Null);

  return TGlMouseListener::OnMouseUp(Sender, Data);
}
//..............................................................................
TGlFont *TGlTextBox::Font()  const   {
  return FParent->Scene()->Font(FontIndex);
}
//..............................................................................


