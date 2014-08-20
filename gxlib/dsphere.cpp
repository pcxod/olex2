/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "dsphere.h"
#include "glprimitive.h"
#include "glmaterial.h"
#include "glrender.h"
#include "gpcollection.h"
#include "styles.h"
#include "glutil.h"
#include "esphere.h"
#include "glmouse.h"

TDSphere::TDSphere(TGlRenderer& R, PointAnalyser &pa,
  const olxstr& collectionName)
  : AGlMouseHandlerImp(R, collectionName),
    analyser(pa),
    Generation(6)
{
  SetSelectable(false);
}
//...........................................................................
void TDSphere::Create(const olxstr& cName)  {
  if( !cName.IsEmpty() )
    SetCollectionName(cName);
  olxstr NewL;
  TGPCollection* GPC = Parent.FindCollectionX(GetCollectionName(), NewL);
  if( GPC == NULL )
    GPC = &Parent.NewCollection(NewL);
  GPC->AddObject(*this);
  if( GPC->PrimitiveCount() != 0 )  return;

  TGraphicsStyle& GS = GPC->GetStyle();
  GS.SetSaveable(false);
  TGlPrimitive& GlP = GPC->NewPrimitive("Sphere", sgloCommandList);
  TGlMaterial &m = GS.GetMaterial("Object",
      TGlMaterial("85;0;4286611584;4290822336;64"));
  m.SetColorMaterial(true);
  m.SetTransparent(true);
  GlP.SetProperties(m);
  TTypeList<TVector3<float> > vecs;
  TTypeList<IndexTriangle> triags;
  OlxSphere<float,OctahedronFP<vec3f> >::Generate(1.0f, Generation, vecs,
    triags);
  const size_t tc = triags.Count();
  uint32_t last_cl = 0;
  bool color_initialised = false;
  TArrayList<uint32_t> colors(vecs.Count());
  for (size_t i=0; i < vecs.Count(); i++)
    colors[i] = analyser.Analyse(vecs[i]);
  GlP.StartList();
  olx_gl::colorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  olx_gl::begin(GL_TRIANGLES);
  for( size_t i = 0; i < tc; i++ )  {
    const IndexTriangle& t = triags[i];
    try {
      for( size_t j=0; j < 3; j++ )  {
        uint32_t cl = colors[t.vertices[j]];
        vec3f n = (vecs[t.vertices[0]]-vecs[t.vertices[1]]).XProdVec(
          vecs[t.vertices[2]]-vecs[t.vertices[1]]).Normalise();
        olx_gl::normal(-n);
        if (!color_initialised || cl != last_cl) {
          olx_gl::color((float)OLX_GetRValue(cl)/255,
            (float)OLX_GetGValue(cl)/255,
            (float)OLX_GetBValue(cl)/255,
            (float)OLX_GetAValue(cl)/255);
          last_cl = cl;
        }
        olx_gl::vertex(vecs[t.vertices[j]]);
      }
    }
    catch(TDivException) {
      break;
    }
  }
  olx_gl::end();
  GlP.EndList();
}
//...........................................................................
bool TDSphere::Orient(TGlPrimitive& P)  {
  olx_gl::orient(Basis.GetMDataT());
  olx_gl::scale(Basis.GetZoom());
  return false;
}
//...........................................................................
bool TDSphere::OnDblClick(const IEObject *obj_, const TMouseData& md) {
  const TEBasis &b = Parent.GetBasis();
  vec3d l(md.DownX-Parent.GetWidth()/2,
    Parent.GetHeight()- md.DownY, 0),
    o(0, 0, 1),
    c = Basis.GetCenter();
  l *= Parent.GetScale();
  double r = Basis.GetZoom();
  vec3d dv = o - c;
  double x = l.DotProd(dv);
  double dq = olx_sqr(x) - dv.QLength() + r*r;
  if (dq < 0) {
    return true;
  }
  double d = -x + sqrt(dq);
  vec3f intersect = o + l*d;
  TBasicApp::NewLogEntry() << olxstr(' ').Join(intersect) <<
    analyser.Analyse(intersect);
  return true;
}
//...........................................................................
