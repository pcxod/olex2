/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "dunitcell.h"
#include "glprimitive.h"
#include "glmaterial.h"
#include "glrender.h"
#include "gpcollection.h"
#include "styles.h"
#include "povdraw.h"
#include "wrldraw.h"
#include "pers_util.h"

TDUnitCell::TDUnitCell(TGlRenderer& R, const olxstr& collectionName) :
  AGDrawObject(R, collectionName), Edges(24)
{
  SetSelectable(false);
  Reciprocal = false;
  Thickness = 1;

  CellToCartesian.I();
  HklToCartesian.I();
  olxstr label_cn("duc_label");
  for (int i = 0; i < 4; i++) {
    (Labels[i] = new TXGlLabel(Parent, label_cn))->SetVisible(false);
  }
}
//...........................................................................
TDUnitCell::~TDUnitCell() {
  for (int i = 0; i < 4; i++) {
    delete Labels[i];
  }
}
//...........................................................................
void TDUnitCell::Init(const TAsymmUnit &au) {
  double cell[6] = {
    au.GetAxes()[0], au.GetAxes()[1], au.GetAxes()[2],
    au.GetAngles()[0], au.GetAngles()[1], au.GetAngles()[2]
  };
  Init(&cell[0]);
}
//...........................................................................
void TDUnitCell::Init(const double *cell) {
  if( cell[0] == 0 )  return;
  const double cG = cos(cell[5]/180*M_PI),
         cB = cos(cell[4]/180*M_PI),
         cA = cos(cell[3]/180*M_PI),
         sG = sin(cell[5]/180*M_PI);

  const double V =
      cell[0]*cell[1]*cell[2]*sqrt((1-cA*cA-cB*cB-cG*cG) + 2*(cA*cB*cG));

  const double cs = cell[0]*cell[1]*sG/V;
  // cell to cartesian transformation matrix
  CellToCartesian.I();
  CellToCartesian[0][0] = cell[0];
  CellToCartesian[1][0] = cell[1]*cG;
  CellToCartesian[2][0] = cell[2]*cB;

  CellToCartesian[1][1] = cell[1]*sG;
  CellToCartesian[2][1] = -cell[2]*(cB*cG-cA)/sG;

  CellToCartesian[2][2] = 1./cs;

  vec3d v0(CellToCartesian[0]), v1(CellToCartesian[1]), v2(CellToCartesian[2]);

  HklToCartesian[0] = v1.XProdVec(v2);
  HklToCartesian[1] = v2.XProdVec(v0);
  HklToCartesian[2] = v0.XProdVec(v1);
  HklToCartesian[0] /= V;
  HklToCartesian[1] /= V;
  HklToCartesian[2] /= V;
  SetReciprocal(IsReciprocal());
}
//...........................................................................
void TDUnitCell::ExpandEdges() {
  Edges[1] -= Edges[0];
  Edges[3] -= Edges[0];
  Edges[5] -= Edges[0];
  Edges[2] = Edges[4].Null();
  Edges[6] = Edges[1];  //A
  Edges[7] = Edges[3] + Edges[1];  //AB
  Edges[8] = Edges[1];  //A
  Edges[9] = Edges[5] + Edges[1];  //AC
  Edges[10] = Edges[3];  //B
  Edges[11] = Edges[3] + Edges[1];  //AB
  Edges[12] = Edges[3];  //B
  Edges[13] = Edges[5] + Edges[3];  //BC
  Edges[14] = Edges[5];  //C
  Edges[15] = Edges[5] + Edges[1];  //AC
  Edges[16] = Edges[5];  //C
  Edges[17] = Edges[5] + Edges[3];  //BC
  Edges[18] = Edges[3] + Edges[1];  //AB
  Edges[19] = Edges[5] + Edges[1] + Edges[3];  //ABC
  Edges[20] = Edges[5] + Edges[3];  //BC
  Edges[21] = Edges[5] + Edges[3] + Edges[1];  //ABC
  Edges[22] = Edges[5] + Edges[1];  //AC
  Edges[23] = Edges[5] + Edges[3] + Edges[1];  //ABC
  for (size_t i = 1; i < 24; i++) {
    Edges[i] += Edges[0];
  }
}
//...........................................................................
void TDUnitCell::SetReciprocal(bool v, double scale)  {
  mat3d M = v ? HklToCartesian : CellToCartesian;
  Edges[0].Null();
  Edges[1] = M[0];  //000-A
  Edges[3] = M[1];  //000-B
  Edges[5] = M[2];  //000-C
  ExpandEdges();
  Reciprocal = v;
  for (size_t i=0; i < Edges.Count(); i++)
    Edges[i] *= scale;
  // reinitialise labels
  const size_t FontIndex = Parent.GetScene().FindFontIndexForType<TDUnitCell>();
  for (int i=0; i < 3; i++) {
    Labels[i]->SetOffset(Edges[i*2+1]);
  }
  Labels[3]->SetOffset(Edges[0]);
}
//...........................................................................
void TDUnitCell::Create(const olxstr& cName)  {
  if (!cName.IsEmpty())
    SetCollectionName(cName);
  TGPCollection& GPC = Parent.FindOrCreateCollection(GetCollectionName());
  if (GPC.PrimitiveCount() != 0) {  // GetDimensions will be called
    GPC.AddObject(*this);
    for (int i=0; i < 4; i++)
      Labels[i]->Create();
    return;
  }
  GPC.AddObject(*this);
  TGraphicsStyle& GS = GPC.GetStyle();
  Thickness = GS.GetNumParam("Thickness", 1.0, true);
  TGlMaterial GlM("85;2131693327;4286611584;41975936;32");
  if (GS.GetParam("RenderLine", FalseString(), true).ToBool()) {
    TGlPrimitive* GlP = &GPC.NewPrimitive("Lines", sgloLines);
    GlP->SetProperties(GS.GetMaterial(GlP->GetName(), GlM));
  }
  else {
    double d = 0.025;
    TGlPrimitive* GlP = &GPC.NewPrimitive("Sphere", sgloSphere);

    GlP->SetProperties(GS.GetMaterial(GlP->GetName(), GlM));
    GlP->Params[0] = d;  GlP->Params[1] = 6;  GlP->Params[2] = 6;

    GlP = &GPC.NewPrimitive("Cylinder", sgloCylinder);
    GlP->Params[0] = d;    GlP->Params[1] = d;  GlP->Params[2] = 1;
    GlP->Params[3] = 5;  GlP->Params[4] = 1;
    GlP->SetProperties(GS.GetMaterial(GlP->GetName(), GlM));
    Compile();
  }
  const size_t FontIndex = Parent.GetScene().FindFontIndexForType<TDUnitCell>();
  for (int i = 0; i < 3; i++) {
    Labels[i]->SetFontIndex(FontIndex);
    Labels[i]->SetLabel(olxstr((char)('a' + i)));
    Labels[i]->Create();
  }
  Labels[3]->SetFontIndex(FontIndex);
  Labels[3]->SetLabel('o');
  Labels[3]->Create();
}
//..............................................................................
bool TDUnitCell::GetDimensions(vec3d &Max, vec3d &Min) {
  if (Edges.IsEmpty()) return false;
  vec3d _min(100,100,100), _max(-100, -100, -100);
  for (int i=0; i < 8; i++)
    vec3d::UpdateMinMax(GetVertex(i), _min, _max);
  Min = _min;
  Max = _max;
  return true;
}
//..............................................................................
bool TDUnitCell::Orient(TGlPrimitive& P) {
  if (P.GetType() == sgloCylinder) {
    for (size_t i = 0; i < Edges.Count(); i += 2) {
      vec3d v = (Edges[i + 1] - Edges[i]);
      double l = v.Length();
      v *= 1.0 / l;
      olx_gl::pushMatrix();
      olx_gl::translate(Edges[i]);
      olx_gl::rotate(acos(v[2]) * 180 / M_PI, -v[1], v[0], 0.0);
      olx_gl::scale(Thickness, Thickness, l);
      P.Draw();
      olx_gl::popMatrix();
    }
  }
  else if (P.GetType() == sgloSphere) {
    for (size_t i = 0; i < VertexCount(); i++) {
      const vec3f &v = GetVertex(i);
      olx_gl::translate(v);
      if (Thickness != 1)
        olx_gl::scale(Thickness);
      P.Draw();
      if (Thickness != 1)
        olx_gl::scale(1./Thickness);
      olx_gl::translate(-v);
    }

  }
  else {
    GLdouble th;
    olx_gl::get(GL_LINE_WIDTH, &th);
    if (th != Thickness)
      olx_gl::lineWidth(Thickness);
    olx_gl::begin(GL_LINES);
    for (size_t i = 0; i < Edges.Count(); i += 2) {
      olx_gl::vertex(Edges[i]);
      olx_gl::vertex(Edges[i + 1]);
    }
    olx_gl::end();
    if (th != Thickness)
      olx_gl::lineWidth(th);
  }
  return true;
}
//..............................................................................
void TDUnitCell::ListPrimitives(TStrList &List) const {}
//..............................................................................
void TDUnitCell::UpdatePrimitives(int32_t Mask)  {}
//..............................................................................
void TDUnitCell::UpdateLabel() {
  for (int i = 0; i < 4; i++) {
    Labels[i]->Update();
  }
}
//..............................................................................
void TDUnitCell::Update() {
  Labels[3]->SetOffset(Edges[0]);
  for (size_t i = 0; i < 3; i++) {
    Labels[i]->SetOffset(Edges[i * 2 + 1]);
  }
}
//..............................................................................
void TDUnitCell::SetVisible(bool v) {
  AGDrawObject::SetVisible(v);
  for (int i = 0; i < 4; i++) {
    Labels[i]->SetVisible(v);
  }
}
//..............................................................................
void TDUnitCell::ToDataItem(TDataItem& di) const {
  di.AddField("reciprocal", IsReciprocal());
  di.AddField("visible", IsVisible());
  TDataItem& labels = di.AddItem("Labels");
  for (int i = 0; i < 4; i++) {
    Labels[i]->ToDataItem(labels.AddItem(olxstr((olxch)('a' + i))));
  }
  TDataItem& vertices = di.AddItem("Vertices");
  vertices.AddField("o", PersUtil::VecToStr(Edges[0]));
  vertices.AddField("a", PersUtil::VecToStr(Edges[1]));
  vertices.AddField("b", PersUtil::VecToStr(Edges[3]));
  vertices.AddField("c", PersUtil::VecToStr(Edges[5]));
}
//..............................................................................
void TDUnitCell::FromDataItem(const TDataItem& di) {
  Reciprocal = di.FindField("reciprocal").ToBool();
  SetVisible(di.FindField("visible", TrueString()).ToBool());
  const TDataItem& labels = di.GetItemByName("Labels");
  for (int i = 0; i < 4; i++) {
    Labels[i]->FromDataItem(labels.GetItemByIndex(i));
  }
  TDataItem* vertices = di.FindItem("Vertices");
  if (vertices != 0) {
    Edges[0] = PersUtil::VecFromStr<vec3d>(vertices->GetFieldByName('o'));
    Edges[1] = PersUtil::VecFromStr<vec3d>(vertices->GetFieldByName('a'));
    Edges[3] = PersUtil::VecFromStr<vec3d>(vertices->GetFieldByName('b'));
    Edges[5] = PersUtil::VecFromStr<vec3d>(vertices->GetFieldByName('c'));
    ExpandEdges();
  }
  else {
    SetReciprocal(IsReciprocal());
  }
}
//..............................................................................
const_strlist TDUnitCell::ToPov(olx_cdict<TGlMaterial, olxstr> &materials) const
{
  TStrList out;
  if (Edges.IsEmpty()) return out;
  out.Add(" object { union {");
  const TGPCollection &gpc = GetPrimitives();
  pov::CrdTransformer crdc(Parent.GetBasis());
  olxstr p_mat = pov::get_mat_name(
    GetPrimitives().FindPrimitiveByName("Cylinder")->GetProperties(),
    materials);
  for (int i=0; i < 24; i+=2) {
    out.Add("  object { cylinder {") << pov::to_str(crdc.crd(GetEdge(i))) <<
      ',' << pov::to_str(crdc.crd(GetEdge(i+1))) << ", 0.01} texture {" <<
      p_mat << "}}";
  }
  for (int i = 0; i < 4; i++) {
    out << Labels[i]->ToPov(materials);
  }
  out.Add("  }}");
  return out;
}
//..............................................................................
const_strlist TDUnitCell::ToWrl(olx_cdict<TGlMaterial, olxstr> &materials) const
{
  TStrList out;
  if (Edges.IsEmpty()) return out;
  const TGlMaterial &m = GetPrimitives()
    .FindPrimitiveByName("Cylinder")->GetProperties();
  olxstr p_mat = wrl::get_mat_str(m, materials);
  out.Add(" Group{ children[ Shape{ appearance ") << p_mat <<
    " geometry IndexedLineSet{ colorPerVertex FALSE coord Coordinate{ point[";
  const TGPCollection &gpc = GetPrimitives();
  wrl::CrdTransformer crdc(Parent.GetBasis());
  for (size_t i=0; i < VertexCount(); i++) {
    out.Add("  ") << wrl::to_str(crdc.crd(GetVertex(i)));
    if (i+1 < VertexCount())
      out.GetLastString() << ',';
  }
  out.GetLastString() << "]}";
  out.Add("  coordIndex[0 1 5 3 0 2 4 7 6 2 -1 3 6 -1 5 7 -1 1 4 -1]");
  out.Add("  color Color{ color[") <<
    wrl::to_str(m.AmbientF) << "]}";
  out.Add("  colorIndex[0 0 0 0]}}");
  for (int i = 0; i < 4; i++) {
    out << Labels[i]->ToWrl(materials);
  }
  out.Add(" ]}");
  return out;
}
//..............................................................................
void TDUnitCell::funThickness(const TStrObjList& Params, TMacroData& E) {
  if (Params.IsEmpty()) {
    E.SetRetVal(GetThickness());
  }
  else {
    SetThickness(Params[0].ToDouble());
    GetPrimitives().GetStyle().SetParam("Thickness", GetThickness(), true);
  }
}
//..............................................................................
void TDUnitCell::funDrawstyle(const TStrObjList& Params, TMacroData& E) {
  if (Params.IsEmpty()) {
    bool rl = GetPrimitives().GetStyle().GetParam(
      "RenderLine", FalseString(), true).ToBool();
    E.SetRetVal<olxstr>(rl ? "line" : "cone");
  }
  else {
    bool nv = false;
    if (Params[0].Equalsi("line"))
      nv = true;
    bool rl = GetPrimitives().GetStyle().GetParam(
      "RenderLine", FalseString(), true).ToBool();
    if (nv != rl) {
      GetPrimitives().GetStyle().SetParam("RenderLine", nv, true);
      GetPrimitives().ClearPrimitives();
      Create();
    }
  }
}
//..............................................................................
TLibrary *TDUnitCell::ExportLibrary(const olxstr &name) {
  TLibrary *l = new TLibrary(name.IsEmpty() ? olxstr("cell") : name);
  l->Register(
    new TFunction<TDUnitCell>(this,
      &TDUnitCell::funThickness, "Thickness", fpNone | fpOne,
    "Returns or sets current cell thickness")
    );
  l->Register(
    new TFunction<TDUnitCell>(this,
    &TDUnitCell::funDrawstyle, "DrawStyle", fpNone | fpOne,
    "Returns or sets current cell drawing style [line,cone]")
    );
  return l;
}
