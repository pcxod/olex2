/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "xreflection.h"
#include "gpcollection.h"
#include "glrender.h"
#include "styles.h"
#include "glmaterial.h"
#include "glprimitive.h"

TXReflection::TXReflection(TGlRenderer& r, const olxstr& collectionName,
  double minI, double maxI, const TReflection& R, const TAsymmUnit& au)
  : AGDrawObject(r, collectionName)
{
  hkl = R.GetHkl();
  I = R.GetI();
  Center = vec3d(hkl)*au.GetHklToCartesian();
  Params().Resize(2);
  Params()[0] = (I - minI) / (maxI + olx_abs(minI));
  if( Params()[0] > 0 )
    Params()[0] = sqrt(Params()[0]); 
}
//..............................................................................
void TXReflection::Create(const olxstr& cName) {
  if( !cName.IsEmpty() )
    SetCollectionName(cName);

  TGPCollection& GPC = Parent.FindOrCreateCollection(GetCollectionName());
  GPC.AddObject(*this);
  if( GPC.PrimitiveCount() != 0 )  return;

  TGraphicsStyle& GS = GPC.GetStyle();
  TGlPrimitive& GlP = GPC.NewPrimitive("Reflection", sgloTriangles);

  TGlMaterial GlM;
  GlM.FromString("85;1.000,0.000,0.059,0.000;2138535799;1.000,1.000,1.000,0.500;36");
  GlP.SetProperties(GS.GetMaterial("Reflection", GlM));
  GlP.MakeTetrahedron(0.004);
}
//..............................................................................
TXReflection::~TXReflection()  {  }
//..............................................................................
bool TXReflection::Orient(TGlPrimitive& GlP)  {
  olx_gl::translate(Center);
  // scale the larger reflections up
  const double scale1 = sqrt(atan(FParams[0])*2/M_PI);
  olx_gl::scale(1.0+scale1*6);

  return false;
  //if (IsSelected() || !GlP.GetProperties().IsTransparent())
  //  return false;

  TGlMaterial& GlM = GlP.GetProperties();
  //const double scale = (1.0-FParams[0]);
  //GlM.AmbientF[3] = (float)scale;
  if (Params()[1] < 0)
    GlM.AmbientF = 0x0000ff;
  else
    GlM.AmbientF = 0xff0000;
  GlM.Init(Parent.ForcePlain());
  return false;
}
//..............................................................................
bool TXReflection::GetDimensions(vec3d &Max, vec3d &Min)  {
  Min = Center;
  Max = Center;
  return true;
};
//..............................................................................
