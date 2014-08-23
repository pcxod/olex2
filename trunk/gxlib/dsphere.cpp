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
#include "estopwatch.h"

TDSphere::TDSphere(TGlRenderer& R, const olxstr& collectionName)
  : AGlMouseHandlerImp(R, collectionName),
    analyser(NULL),
    Generation(6)
{
  SetSelectable(false);
  SetZoomable(true);
  SetMoveable(true);
}
//...........................................................................
class PATask : public TaskBase {
public:
  APointAnalyser &analyser;
  TTypeList<TVector3<float> > &vecs;
  TArrayList<uint32_t> &colors;
  bool delete_analyser;
  PATask(APointAnalyser &analyser,
    TTypeList<TVector3<float> > &vecs, TArrayList<uint32_t> &colors,
    bool delete_analyser=false)
    : analyser(analyser), vecs(vecs), colors(colors),
    delete_analyser(delete_analyser)
  {
  }
  ~PATask() {
    if (delete_analyser) {
      delete &analyser;
    }
  }
  PATask * Replicate() {
    return new PATask(*(APointAnalyser*)analyser.Replicate(),
      vecs, colors, true);
  }
  void Run(size_t i) const {
    colors[i] = analyser.Analyse(vecs[i]);
  }
};
//...........................................................................
TGlPrimitive &TDSphere::CreatePrimitive(TGPCollection &GPC,
  const olxstr &name, size_t gen, bool update_vec_cnt)
{
  TStopWatch sw(__FUNC__);
  TGraphicsStyle& GS = GPC.GetStyle();
  TGlPrimitive& GlP = GPC.NewPrimitive(name, sgloCommandList);
  TGlMaterial &m = GS.GetMaterial("Object",
    TGlMaterial("85;0;4286611584;4290822336;64"));
  m.SetColorMaterial(true);
  m.SetTransparent(true);
  GlP.SetProperties(m);
  sw.start("Generating sphere");
  TTypeList<TVector3<float> > vecs;
  TTypeList<IndexTriangle> triags;
  OlxSphere<float, OctahedronFP<vec3f> >::Generate(1.0f, gen, vecs,
    triags);
  if (update_vec_cnt) {
    vec_cnt = vecs.Count();
  }
  const size_t tc = triags.Count();
  uint32_t last_cl = 0;
  bool color_initialised = false;
  TArrayList<uint32_t> colors(vecs.Count());
  sw.start("Running the analysis");
  PATask pa_task(*analyser, vecs, colors);
  OlxListTask::Run(pa_task, vecs.Count(), tLinearTask, 1000);
  sw.start("Building the object");
  GlP.StartList();
  olx_gl::colorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  olx_gl::begin(GL_TRIANGLES);
  for (size_t i = 0; i < tc; i++) {
    const IndexTriangle& t = triags[i];
    try {
      for (size_t j = 0; j < 3; j++) {
        uint32_t cl = colors[t.vertices[j]];
        olx_gl::normal(vecs[t.vertices[j]]);
        if (!color_initialised || cl != last_cl) {
          olx_gl::color((float)OLX_GetRValue(cl) / 255,
            (float)OLX_GetGValue(cl) / 255,
            (float)OLX_GetBValue(cl) / 255,
            (float)OLX_GetAValue(cl) / 255);
          last_cl = cl;
        }
        olx_gl::vertex(vecs[t.vertices[j]]);
      }
    }
    catch (TDivException) {
      break;
    }
  }
  olx_gl::end();
  GlP.EndList();
  return GlP;
}
//...........................................................................
void TDSphere::Create(const olxstr& cName)  {
  if (analyser == NULL) {
    return;
  }
  volatile TStopWatch sw(__FUNC__);
  if (!cName.IsEmpty())
    SetCollectionName(cName);
  olxstr NewL;
  TGPCollection* GPC = Parent.FindCollectionX(GetCollectionName(), NewL);
  if (GPC == NULL)
    GPC = &Parent.NewCollection(NewL);
  GPC->AddObject(*this);
  if (GPC->PrimitiveCount() != 0) {
    GPC->ClearPrimitives();
  }

  TGraphicsStyle& GS = GPC->GetStyle();
  GS.SetSaveable(false);
  analyser->SetDryRun(false);
  analyser->GetReady();
  highq = &CreatePrimitive(*GPC, "Sphere", Generation, true);
  if (Generation > 6) {
    analyser->SetDryRun(true);
    lowq = &CreatePrimitive(*GPC, "SphereLQ", 5, false);
  }
  else {
    lowq = NULL;
  }
}
//...........................................................................
bool TDSphere::Orient(TGlPrimitive& P) {
  if (TGlMouse::GetInstance().GetMouseData().Button != 0) {
    if (lowq != NULL && &P != lowq)
      return true;
  }
  else {
    if (&P != highq)
      return true;
  }
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
    analyser->Analyse(intersect);
  return true;
}
//...........................................................................
void TDSphere::ToDataItem(TDataItem &di) const {
  if (analyser == NULL) {
    return;
  }
  analyser->ToDataItem(di.AddItem("Analyser"));
  di.AddField("generation", Generation);
  Basis.ToDataItem(di.AddItem("Basis"));
}
//...........................................................................
void TDSphere::FromDataItem(const TDataItem &di) {
  Generation = di.GetFieldByName("generation").ToUInt();
  Basis.FromDataItem(di.GetItemByName("Basis"));
  APointAnalyser *pa = APointAnalyser::FromDataItem(
    di.GetItemByName("Analyser"));
  if (pa == NULL) {
    return;
  }
  SetAnalyser(pa);
}
//...........................................................................
