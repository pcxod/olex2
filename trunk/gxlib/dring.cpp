/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "dring.h"
#include "glprimitive.h"
#include "glmaterial.h"
#include "glrender.h"
#include "gpcollection.h"
#include "styles.h"
#include "glutil.h"

//..............................................................................
TDRing::TDRing(TGlRenderer& R, const olxstr& collectionName) :
  AGlMouseHandlerImp(R, collectionName),
  material("85;0;4286611584;4290822336;64"),
  settings(0)
{
  SetSelectable(false);
}
//...........................................................................
double TDRing::GetRadius() const {
  return 1.0 + GetSettings().GetTubeR();
}
//...........................................................................
int TDRing::Quality(TGlRenderer &r, int v) {
  if (v == -1) v = qaMedium;
  Settings &defs = GetSettings(r);
  switch (v) {
  case qaPict:
    defs.SetTubeS(25);
    defs.SetTorusS(40);
    break;
  case qaHigh:
    defs.SetTubeS(15);
    defs.SetTorusS(35);
    break;
  case qaMedium:
    defs.SetTubeS(8);
    defs.SetTorusS(25);
    break;
  case qaLow:
    defs.SetTubeS(5);
    defs.SetTorusS(10);
    break;
  }
  int rv = defs.QualityValue;
  defs.QualityValue = v;
  return rv;
}
//...........................................................................
void TDRing::Create(const olxstr& cName) {
  if (!cName.IsEmpty())
    SetCollectionName(cName);
  olxstr NewL;
  TGPCollection* GPC = Parent.FindCollectionX(GetCollectionName(), NewL);
  if (GPC == NULL)
    GPC = &Parent.NewCollection(NewL);
  GPC->AddObject(*this);
  if (GPC->PrimitiveCount() != 0) return;

  TGraphicsStyle& GS = GPC->GetStyle();
  GS.SetSaveable(false);
  TGlPrimitive& GlP = GPC->NewPrimitive("Torus", sgloCommandList);
  GlP.SetProperties(GS.GetMaterial("Object", material));
  const TStringToList<olxstr, TGlPrimitive*> &primtives =
    GetSettings().GetPrimitives(true);
  GlP.StartList();
  GlP.CallList(primtives.GetObject(0));
  GlP.EndList();
}
//...........................................................................
bool TDRing::Orient(TGlPrimitive &) {
  olx_gl::orient(Basis.GetMDataT());
  olx_gl::scale(Basis.GetZoom());
  return false;
}
//...........................................................................
void TDRing::Update() {
}
//...........................................................................
void TDRing::ToDataItem(TDataItem &i) const {
  i.SetValue(GetCollectionName());
  Basis.ToDataItem(i.AddItem("basis"));
}
//...........................................................................
void TDRing::FromDataItem(const TDataItem &i) {
  SetCollectionName(i.GetValue());
  Basis.FromDataItem(i.GetItemByName("basis"));
}
//...........................................................................
//...........................................................................
//...........................................................................
void TDRing::Settings::CreatePrimitives() {
  ClearPrimitives();
  TGlPrimitive &torus = parent.NewPrimitive(sgloCommandList);
  torus.StartList();
  GlTorus::Render(GetTubeR(), 1, GetTubeS(), GetTorusS());
  torus.EndList();
  primitives.Add("Torus", &torus);
}
//...........................................................................
