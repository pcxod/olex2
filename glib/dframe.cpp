/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "dframe.h"
#include "glmaterial.h"
#include "gpcollection.h"
#include "glmouse.h"
#include "glprimitive.h"
UseGlNamespace()

// TDFrame - a drawing object for selection frame
TDFrame::TDFrame(TGlRenderer& Render, const olxstr& collectionName) :
  AGDrawObject(Render, collectionName),
  OnSelect(Actions.New("ONSELECT"))
{
  FPrimitive = NULL;
  SetSelectable(false);
};
//..............................................................................
void TDFrame::Create(const olxstr& cName) {
  if( !cName.IsEmpty() )
    SetCollectionName(cName);
  TGPCollection& GPC = Parent.FindOrCreateCollection( GetCollectionName() );
  GPC.AddObject(*this);
  if( GPC.PrimitiveCount() != 0 )  return;

  TGlMaterial GlM;
  GlM.SetFlags(sglmIdentityDraw);
  FPrimitive = &GPC.NewPrimitive("Lines", sgloLineLoop);
  FPrimitive->SetProperties(GlM);
  FPrimitive->Vertices.SetCount(4);
  FPrimitive->Colors.SetCount(4);
  FPrimitive->Colors[0] = 0x000000FF;
  FPrimitive->Colors[0] = 0x00FF0000;
  FPrimitive->Colors[0] = 0x00000000;
  FPrimitive->Colors[0] = 0x0000FF00;
  FPrimitive->Params[0] = 1;              // line width
}
//..............................................................................
bool TDFrame::OnMouseDown(const IEObject *Sender, const TMouseData& Data)  {
  if( FPrimitive == NULL )  return false;
  float Scale = (float)Parent.GetScale();
  int hW = Parent.GetWidth()/2 + Parent.GetLeft(),
      hH = Parent.GetHeight()/2 - Parent.GetTop();
  // the translation is currently disabled, so, just null it
//  Translation.Null(); // = FRender->Basis().Center();
//  Translation *= FRender->GetBasis().GetMatrix();
  FPrimitive->Vertices[0][0] = (-hW + Data.X)*Scale - Translation[0];
  FPrimitive->Vertices[0][1] = (+hH - Data.Y)*Scale - Translation[1];

  FPrimitive->Vertices[1][0] = (-hW + Data.X)*Scale - Translation[0];
  FPrimitive->Vertices[1][1] = (hH - Data.Y)*Scale - Translation[1];

  FPrimitive->Vertices[2][0] = (-hW + Data.X)*Scale - Translation[0];
  FPrimitive->Vertices[2][1] = (hH - Data.Y)*Scale - Translation[1];

  FPrimitive->Vertices[3][1] = (hH - Data.Y)*Scale - Translation[1];
  FPrimitive->Vertices[3][0] = (-hW + Data.X)*Scale - Translation[0];

  SetVisible(true);
  return true;
}
//..............................................................................
bool TDFrame::OnMouseUp(const IEObject *Sender, const TMouseData& Data)  {
  if( FPrimitive == NULL )  return false;
  SetVisible(false);
  Parent.Draw();
  TSelectionInfo SI;
  SI.From[0] = olx_min(FPrimitive->Vertices[0][0], FPrimitive->Vertices[2][0]);
  SI.From[1] = olx_min(FPrimitive->Vertices[0][1], FPrimitive->Vertices[2][1]);

  SI.To[0] = olx_max(FPrimitive->Vertices[0][0], FPrimitive->Vertices[2][0]);
  SI.To[1] = olx_max(FPrimitive->Vertices[0][1], FPrimitive->Vertices[2][1]);

  OnSelect.Execute(this, &SI);
  return true;
}
//..............................................................................
bool TDFrame::OnMouseMove(const IEObject *Sender, const TMouseData& Data)  {
  if( FPrimitive == NULL )  return false;
  float Scale = (float)Parent.GetScale();
  int hW = Parent.GetWidth()/2 + Parent.GetLeft(),
      hH = Parent.GetHeight()/2 - Parent.GetTop();
  FPrimitive->Vertices[2][0] = (-hW + Data.X)*Scale - Translation[0];
  FPrimitive->Vertices[2][1] = (hH - Data.Y )*Scale - Translation[1];

  FPrimitive->Vertices[1][0] = (-hW + Data.X)*Scale - Translation[0];
  FPrimitive->Vertices[3][1] = (hH - Data.Y)*Scale - Translation[1];
  return true;
}
//..............................................................................
bool TDFrame::Orient(TGlPrimitive& P)  {
  return false;
}
//..............................................................................
