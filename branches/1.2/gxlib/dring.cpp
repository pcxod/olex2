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
bool TDRing::TStylesClear::Enter(const IEObject *Sender, const IEObject *Data,
  TActionQueue *)
{
  TDRing::GlobalStyle() = NULL;
  TDRing::ClearStaticObjects();
  return true;
}
//..............................................................................
bool TDRing::TStylesClear::Exit(const IEObject *Sender, const IEObject *Data,
  TActionQueue *)
{
  TDRing::ValidateGlobalStyle();
  TDRing::ClearStaticObjects();
  return true;
}
//..............................................................................
//..............................................................................
//..............................................................................
TDRing::TContextClear::TContextClear(TGlRenderer& Render) {
  Render.OnClear.Add(this);
}
//..............................................................................
bool TDRing::TContextClear::Enter(const IEObject *, const IEObject *,
  TActionQueue *)
{
  TDRing::ClearStaticObjects();
  return true;
}
//..............................................................................
//..............................................................................
//..............................................................................
TDRing::TDRing(TGlRenderer& R, const olxstr& collectionName) :
  AGlMouseHandlerImp(R, collectionName),
  material("85;0;4286611584;4290822336;64")
{
  SetSelectable(false);
  if (!Initialised()) {
    Initialised() = true;
    new TContextClear(R);
    new TStylesClear(R);
  }
}
//...........................................................................
double TDRing::GetRadius() const {
  return 1.0 + DefTubeRadius();
}
//...........................................................................
int16_t TDRing::Quality(int16_t v) {
  static int16_t qv = -1;
  if (v == -1) v = qaMedium;
  if (!ValidateGlobalStyle()) return -1;
  olxstr &tus = GlobalStyle()->GetParam("TubeSections", "8", true);
  olxstr &trs = GlobalStyle()->GetParam("TorusSections", "25", true);
  switch (v) {
  case qaPict:
    tus = 25;
    trs = 40;
    break;
  case qaHigh:
    tus = 15;
    trs = 35;
    break;
  case qaMedium:
    tus = 8;
    trs = 25;
    break;
  case qaLow:
    tus = 5;
    trs = 10;
    break;
  }
  int rv = qv;
  qv = v;
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
  if (GetStaticPrimitives().IsEmpty()) {
    CreateStaticObjects(Parent);
  }
  GlP.StartList();
  GlP.CallList(GetStaticPrimitives().GetObject(0));
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
bool TDRing::ValidateGlobalStyle() {
  if (GlobalStyle() == NULL) {
    if (!Initialised()) {
      return false;
    }
    GlobalStyle() = &TGlRenderer::_GetStyles().NewStyle("RingParams", true);
    GlobalStyle()->SetPersistent(true);
  }
  return true;
}
//..............................................................................
void TDRing::CreateStaticObjects(TGlRenderer &r) {
  if (!Initialised()) {
    Initialised() = true;
    new TContextClear(r);
    new TStylesClear(r);
  }
  ClearStaticObjects();
  TGlPrimitive &torus = r.NewPrimitive(sgloCommandList);
  torus.StartList();
  ValidateGlobalStyle();
  int tus = GlobalStyle()->GetNumParam("TubeSections", 8, true);
  int trs = GlobalStyle()->GetNumParam("TorusSections", 25, true);
  double tr = GlobalStyle()->GetNumParam("TubeRadius", 0.075, true);
  GlTorus::Render(tr, 1, tus, trs);
  torus.EndList();
  GetStaticPrimitives().Add("Torus", &torus);
}
//...........................................................................
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
double TDRing::GetDefTubeRadius() {
  if (DefTubeRadius() > 0) return DefTubeRadius();
  if (!ValidateGlobalStyle()) return DefTubeRadius();
  return (DefTubeRadius() =
    GlobalStyle()->GetNumParam("TubeRadius", 0.075, true));
}
//...........................................................................
void TDRing::SetDefTubeRadius(double v) {
  if (!ValidateGlobalStyle()) return;
  GlobalStyle()->SetParam("TubeRadius", (DefTubeRadius() = v), true);
}
//...........................................................................
