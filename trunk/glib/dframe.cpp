//----------------------------------------------------------------------------//
// namespace TGlObj
// TDFrame - a drawing object for selection frame
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "dframe.h"
#include "glmaterial.h"
#include "glrender.h"
#include "gpcollection.h"
#include "glmouse.h"

UseGlNamespace()
//..............................................................................
//..............................................................................

TDFrame::TDFrame(const olxstr& collectionName, TGlRenderer *Render) :
  AGDrawObject(collectionName)
{
  FPrimitive = NULL;
  FRender = Render;
  Groupable(false);
  FActions = new TActionQList;
  OnSelect = &FActions->NewQueue("ONSELECT");
};
//..............................................................................
TDFrame::~TDFrame()  {
  delete FActions;
}
//..............................................................................
void TDFrame::Create(const olxstr& cName, const ACreationParams* cpar) {
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);
  TGPCollection* GPC = FRender->FindCollection( GetCollectionName() );
  if( GPC == NULL )
    GPC = FRender->NewCollection( GetCollectionName() );
  GPC->AddObject(this);
  if( GPC->PrimitiveCount() != 0 )  return;

  TGlMaterial GlM;
  GlM.SetFlags(sglmIdentityDraw);
  FPrimitive = GPC->NewPrimitive("Lines", sgloLineLoop);
  FPrimitive->SetProperties(&GlM);
  FPrimitive->Data.Resize(4, 4);
  FPrimitive->Params[0] = 1;              // line width
}
//..............................................................................
bool TDFrame::OnMouseDown(const IEObject *Sender, const TMouseData *Data)  {
  if( !FPrimitive ) return false;
  double Scale = FRender->GetScale();
  int hW = FRender->GetWidth()/2 + FRender->GetLeft(),
      hH = FRender->GetHeight()/2 - FRender->GetTop();
  // the translation is currently disabled, so, just null it
//  Translation.Null(); // = FRender->Basis().Center();
//  Translation *= FRender->GetBasis().GetMatrix();

  FPrimitive->Data[0][0] = (-hW + Data->X)*Scale - Translation[0];
  FPrimitive->Data[1][0] = (+hH - Data->Y)*Scale - Translation[1];

  FPrimitive->Data[1][1] = (hH - Data->Y)*Scale - Translation[1];
  FPrimitive->Data[0][3] = (-hW + Data->X)*Scale - Translation[0];

  FPrimitive->Data[0][2] = (-hW + Data->X)*Scale - Translation[0];
  FPrimitive->Data[1][2] = (hH - Data->Y)*Scale - Translation[1];

  FPrimitive->Data[0][1] = (-hW + Data->X)*Scale - Translation[0];
  FPrimitive->Data[1][3] = (hH - Data->Y)*Scale - Translation[1];

  FPrimitive->Data[3][0] = 0x000000ff;
  FPrimitive->Data[3][1] = 0x00ff0000;
  FPrimitive->Data[3][2] = 0x00000000;
  FPrimitive->Data[3][3] = 0x0000ff00;

  Visible( true );
  return true;
}
//..............................................................................
bool TDFrame::OnMouseUp(const IEObject *Sender, const TMouseData *Data)
{
  if( !FPrimitive ) return false;
  Visible( false );
  FRender->Draw();
  TSelectionInfo SI;
  SI.From[0] = olx_min(FPrimitive->Data[0][0], FPrimitive->Data[0][2]);
  SI.From[1] = olx_min(FPrimitive->Data[1][0], FPrimitive->Data[1][2]);

  SI.To[0] = olx_max(FPrimitive->Data[0][0], FPrimitive->Data[0][2]);
  SI.To[1] = olx_max(FPrimitive->Data[1][0], FPrimitive->Data[1][2]);

  OnSelect->Execute(this, &SI);
  return true;
}
//..............................................................................
bool TDFrame::OnMouseMove(const IEObject *Sender, const TMouseData *Data)
{
  if( !FPrimitive ) return false;
  double Scale = FRender->GetScale();
  int hW = FRender->GetWidth()/2 + FRender->GetLeft(),
      hH = FRender->GetHeight()/2 - FRender->GetTop();
  FPrimitive->Data[0][2] = (-hW + Data->X)*Scale - Translation[0];
  FPrimitive->Data[1][2] = (hH - Data->Y )*Scale - Translation[1];

  FPrimitive->Data[0][1] = (-hW + Data->X)*Scale - Translation[0];
  FPrimitive->Data[1][3] = (hH - Data->Y)*Scale - Translation[1];
//  FRender->Draw();
  return true;
}
//..............................................................................
bool TDFrame::Orient(TGlPrimitive *P)
{
  return false;
}
//..............................................................................

