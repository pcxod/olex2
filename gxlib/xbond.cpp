/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "xbond.h"
#include "gpcollection.h"
#include "xatom.h"
#include "lattice.h"
#include "symmparser.h"
#include "unitcell.h"
#include "povdraw.h"
#include "wrldraw.h"

//----------------------------------------------------------------------------//
// TXBond function bodies
//----------------------------------------------------------------------------//
//..............................................................................
TXBond::TXBond(TNetwork* net, TGlRenderer& R, const olxstr& collectionName)
  : TSBond(net),
  AGDrawObject(R, collectionName),
  settings(0)
{
  SetGroupable(true);
  Params().Resize(6);
  Params()[4] = 0.8;
  Label = new TXGlLabel(GetParent(), PLabelsCollectionName());
  Label->SetVisible(false);
  label_forced = false;
}
//..............................................................................
TXBond::~TXBond() {
  if (GetParentGroup() != NULL) {
    GetParentGroup()->Remove(*this);
  }
  delete Label;
}
//..............................................................................
void TXBond::Update() {
  if( !IsValid() )  return;
  vec3d C = B().crd() - A().crd();
  if (C.IsNull()) {
    Params().Null();
  }
  else {
    Params()[3] = C.Length();
    C /= Params()[3];
    Params()[0] = acos(C[2])*180/M_PI;
    if (olx_abs(Params()[0]-180) < 1e-3) { // degenerate case with Pi rotation
      Params()[1] = 0;
      Params()[2] = 1;
    }
    else {
      Params()[1] = -C[1];
      Params()[2] = C[0];
    }
  }
}
//..............................................................................
void TXBond::EvaluatePrimitiveMaterial(TGlPrimitive &p, TGraphicsStyle &s) const {
  if (IsValid()) {
    TGlMaterial m;
    if (p.Params.GetLast() == ddsDefAtomA ||
      p.Params.GetLast() == ddsDef)
    {
      if (!A().IsCreated()) {
        A().Create();
      }
      const size_t mi = A().Style().IndexOfMaterial("Sphere");
      if (mi != InvalidIndex) {
        m = A().Style().GetPrimitiveStyle(mi).GetProperties();
      }
      else {
        TXAtom::GetDefSphereMaterial(A().CAtom(), m, A().GetSettings());
      }
    }
    else {
      if (!B().IsCreated()) {
        B().Create();
      }
      const size_t mi = B().Style().IndexOfMaterial("Sphere");
      if (mi != InvalidIndex) {
        m = B().Style().GetPrimitiveStyle(mi).GetProperties();
      }
      else {
        TXAtom::GetDefSphereMaterial(B().CAtom(), m, B().GetSettings());
      }
    }
    p.SetProperties(s.SetMaterial(p.GetName(), m));
  }
  else {  // no atoms
    p.SetProperties(s.GetMaterial(p.GetName(),
      TGlMaterial("85;2155839359;2155313015;1.000,1.000,1.000,0.502;36")));
  }
}
//..............................................................................
void TXBond::Create(const olxstr& cName) {
  SetCreated(true);
  olxstr Legend;
  if (EsdlInstanceOf(*this, TXBond)) {
    olxstr strRef = GetRef().ToString();
    if (!cName.IsEmpty())  {
      SetCollectionName(cName);
      NamesRegistry().Add(strRef, Legend = cName, true);
    }
    else {
      Legend = NamesRegistry().Find(strRef, EmptyString());
      if (Legend.IsEmpty()) {
        Legend = GetLegend(*this, 3);
      }
    }
  }
  else {
    if (cName.IsEmpty()) {
      Legend = GetCollectionName();
    }
    else {
      Legend = cName;
      SetCollectionName(cName);
    }
  }
  Settings &defs = GetSettings();
  const TStringToList<olxstr, TGlPrimitive*> &primitives =
    defs.GetPrimitives(true);
  Label->SetFontIndex(Parent.GetScene().FindFontIndexForType<TXBond>());
  Label->Create();
  // find collection
  olxstr NewL;
  TGPCollection* GPC = Parent.FindCollectionX(Legend, NewL);
  if (GPC == 0) {
    GPC = &Parent.NewCollection(NewL);
  }
  else if (GPC->PrimitiveCount() != 0) {
    GPC->AddObject(*this);
    Params()[4] = GPC->GetStyle().GetNumParam('R', defs.GetRadius());
    return;
  }
  TGraphicsStyle& GS = GPC->GetStyle();
  GS.SetSaveable(IsStyleSaveable());
  Params()[4] = GS.GetNumParam('R', defs.GetRadius());
  GPC->AddObject(*this);
  const int PrimitiveMask = GetPrimitiveMask();
  if (PrimitiveMask == 0 || primitives.IsEmpty()) { // nothing to create
    return;
  }
  else if (PrimitiveMask == 1) { // special case
    TGlPrimitive* SGlP = primitives.GetObject(0);
    TGlPrimitive& GlP = GPC->NewPrimitive(primitives[0], sgloCommandList);
    GlP.Params.Resize(GlP.Params.Count()+1);
    GlP.Params.GetLast() = SGlP->Params.GetLast();
    GlP.StartList();
    GlP.CallList(SGlP);
    GlP.EndList();
    TGlMaterial * m = defs.GetStyle()->FindMaterial("Single cone", 0);
    if (m != 0) {
      GlP.SetProperties(GS.GetMaterial(primitives[0], *m));
    }
    else {
      EvaluatePrimitiveMaterial(GlP, GS);
    }
    return;
  }

  const uint16_t legend_level = TXAtom::LegendLevel(GetPrimitives().GetName());
  for (size_t i = 0; i < primitives.Count(); i++) {
    if ((PrimitiveMask & (1<<i)) != 0) {
      TGlPrimitive* SGlP = primitives.GetObject(i);
      TGlPrimitive& GlP = GPC->NewPrimitive(primitives[i], sgloCommandList);
      GlP.SetOwnerId(i);
      /* copy the default drawing style tag source */
      GlP.Params.Resize(GlP.Params.Count()+1);
      GlP.Params.GetLast() = SGlP->Params.GetLast();

      GlP.StartList();
      GlP.CallList(SGlP);
      GlP.EndList();
      TGlMaterial* style_mat = GS.FindMaterial(primitives[i]);
      if (style_mat != 0) {
        GlP.SetProperties(*style_mat);
      }
      else {
        EvaluatePrimitiveMaterial(GlP, GS);
      }
    }
  }
}
//..............................................................................
void TXBond::UpdateStyle() {
  TGPCollection &gpc = GetPrimitives();
  const uint16_t legend_level = TXAtom::LegendLevel(gpc.GetName());
  if (legend_level == 3)  // is user managed?
    return;
  TGraphicsStyle& GS = gpc.GetStyle();
  const int PrimitiveMask = GetPrimitiveMask();
  const TStringToList<olxstr, TGlPrimitive*> &primitives =
    GetSettings().GetPrimitives();
  for (size_t i = 0; i < primitives.Count(); i++) {
    if ((PrimitiveMask & (1<<i)) != 0) {
      TGlPrimitive *GlP = gpc.FindPrimitiveByName(primitives[i]);
      if (GlP == NULL)  // must not ever happen...
        continue;
      EvaluatePrimitiveMaterial(*GlP, GS);
    }
  }
}
//..............................................................................
bool TXBond::Orient(TGlPrimitive& GlP)  {
  if (GlP.GetOwnerId() >= 14 && GlP.GetOwnerId() <= 17) {
    vec3d v = (GetToCrd() - GetFromCrd()).Normalise();
    const mat3d& m = Parent.GetBasis().GetMatrix();
    v = m*v*m;
    v = vec3d(m[0][2], m[1][2], m[2][2]).XProdVec(v);
    double vl = v.Length();
    if (vl > 0) {
      v *= (0.1*Params()[4] / vl);
      if (GlP.GetOwnerId() > 15) {
        olx_gl::pushMatrix();
        olx_gl::translate(GetFromCrd());
        olx_gl::rotate(Params()[0], Params()[1], Params()[2], 0.0);
        olx_gl::scale(Params()[4], Params()[4], Params()[3]);
        GlP.Draw();
        olx_gl::popMatrix();
      }
      olx_gl::pushMatrix();
      olx_gl::translate(GetFromCrd() + v);
      olx_gl::rotate(Params()[0], Params()[1], Params()[2], 0.0);
      olx_gl::scale(Params()[4], Params()[4], Params()[3]);
      GlP.Draw();
      olx_gl::popMatrix();
      olx_gl::translate(GetFromCrd() - v);
      olx_gl::rotate(Params()[0], Params()[1], Params()[2], 0.0);
      olx_gl::scale(Params()[4], Params()[4], Params()[3]);
      GlP.Draw();
    }
    return true;
  }
  else if (GlP.GetOwnerId() == 11) {
    Settings &st = GetSettings();
    olx_gl::translate(GetFromCrd());
    olx_gl::scale(Params()[4], Params()[4], Params()[4]);
    vec3d v = (GetToCrd() - GetFromCrd()).NormaliseTo(Params()[3]/(Params()[4]*10));
    for (int i = 0; i < 10; i++) {
      st.GetStockPrimitives().GetObject(0)->Draw();
      olx_gl::translate(v);
    }
    return true;
  }
  else if (GlP.GetOwnerId() == 18) {
    vec3d v = (GetToCrd() - GetFromCrd()).Normalise();
    Settings &st = GetSettings();
    olx_gl::translate(GetFromCrd());
    olx_gl::rotate(Params()[0], Params()[1], Params()[2], 0.0);
    olx_gl::scale(Params()[4], Params()[4], 1.0);
    st.GetStockPrimitives().GetObject(2)->Draw();
    olx_gl::translate(0.0, 0.0, 0.2);
    st.GetStockPrimitives().GetObject(3)->Draw();
    olx_gl::scale(1.0, 1.0, Params()[3]/2-0.2);
    st.GetStockPrimitives().GetObject(1)->Draw();
    return true;
  }
  else if (GlP.GetOwnerId() == 19) {
    vec3d v = (GetToCrd() - GetFromCrd()).Normalise();
    double scale = Params()[3]/2-0.2;
    Settings &st = GetSettings();
    olx_gl::translate(GetFromCrd()+v*Params()[3]/2);
    olx_gl::rotate(Params()[0], Params()[1], Params()[2], 0.0);
    olx_gl::scale(Params()[4], Params()[4], scale);
    st.GetStockPrimitives().GetObject(1)->Draw();
    olx_gl::scale(1.0, 1.0, 1./scale);
    olx_gl::translate(0.0, 0.0, scale);
    st.GetStockPrimitives().GetObject(5)->Draw();
    st.GetStockPrimitives().GetObject(4)->Draw();
    return true;
  }
  else if (GlP.GetOwnerId() == 4) {
    if (Params()[5] != 0) {
      Settings &st = GetSettings();
      olx_gl::translate(GetFromCrd());
      olx_gl::rotate(Params()[0], Params()[1], Params()[2], 0.0);
      //olx_gl::scale(Params()[4], Params()[4], Params()[3]*Params()[5]);
      olx_gl::scale(Params()[4], Params()[4], st.GetUnitLength()*Params()[5]);
      st.GetStockPrimitives().GetObject(6)->Draw();
      return true;
    }
  }
  else if (GlP.GetOwnerId() == 5) {
    if (Params()[5] != 0) {
      Settings &st = GetSettings();
      olx_gl::translate(GetToCrd());
      olx_gl::rotate(Params()[0], Params()[1], Params()[2], 0.0);
      //olx_gl::scale(Params()[4], Params()[4], Params()[3]*Params()[5]);
      olx_gl::scale(Params()[4], Params()[4], st.GetUnitLength()*Params()[5]);
      st.GetStockPrimitives().GetObject(7)->Draw();
      return true;
    }
  }
  olx_gl::translate(GetFromCrd());
  olx_gl::rotate(Params()[0], Params()[1], Params()[2], 0.0);
  olx_gl::scale(Params()[4], Params()[4], Params()[3]);
  return false;
}
//..............................................................................
void TXBond::ListParams(TStrList &List, TGlPrimitive *Primitive) {
}
//..............................................................................
void TXBond::ListParams(TStrList &List) {
}
//..............................................................................
void TXBond::UpdatePrimitiveParams(TGlPrimitive *Primitive) {
}
//..............................................................................
void TXBond::ListPrimitives(TStrList &List) const {
  List.Assign(GetSettings().GetPrimitives());
}
//..............................................................................
int TXBond::Quality(TGlRenderer &r, int Val) {
  int q = 0;
  switch (Val) {
    case qaPict:
    case qaHigh:   q = 30;  break;
    default:
    case qaMedium: q = 15;  break;
    case qaLow:    q = 5;  break;
  }
  Settings &defs = GetSettings(r);
  defs.SetConeQ(q);
  int pq = defs.GetQuality();
  defs.SetQuality(Val);
  return pq;
}
//..............................................................................
void TXBond::ListDrawingStyles(TStrList &L) {
  return;
}
//..............................................................................
const vec3d &TXBond::GetFromCrd() const {
  if (!IsValid()) {
    throw TFunctionFailedException(__OlxSourceInfo, "atoms are not defined");
  }
  return A().crd();
}
//..............................................................................
const vec3d &TXBond::GetToCrd() const {
  if (!IsValid()) {
    throw TFunctionFailedException(__OlxSourceInfo, "atoms are not defined");
  }
  return B().crd();
}
//..............................................................................
const_strlist TXBond::ToPov(olx_cdict<TGlMaterial, olxstr> &materials) const {
  TStrList out;
  out.Add(" object { union {");
  const TGPCollection &gpc = GetPrimitives();
  for (size_t i=0; i < gpc.PrimitiveCount(); i++) {
    TGlPrimitive &glp = gpc.GetPrimitive(i);
    olxstr p_mat = pov::get_mat_name(glp.GetProperties(), materials, this);
    out.Add("  object {") << "bond_" << glp.GetName().ToLowerCase().Replace(' ', '_')
      << " texture {" << p_mat << "}}";
  }
  out.Add("  }");
  mat3d m;
  if (olx_abs(Params()[0]) < 1e-3 ||
    (olx_abs(Params()[1]) + olx_abs(Params()[2])) < 1e-3)
  {
    m.I();
  }
  else {
    olx_create_rotation_matrix(m, vec3d(Params()[1], Params()[2], 0).Normalise(),
      cos(Params()[0] * M_PI / 180));
  }
  const mat3d &b = Parent.GetBasis().GetMatrix();
  m *= b;
  m[0] *= Params()[4];
  m[1] *= Params()[4];
  m[2] *= Params()[3];
  vec3d t = pov::CrdTransformer(Parent.GetBasis()).crd(GetFromCrd());
  out.Add("  transform {");
  out.Add("   matrix") << pov::to_str(m, t);
  out.Add("   }");
  out.Add(" }");
  return out;
}
//..............................................................................
const_strlist TXBond::PovDeclare(TGlRenderer &p)  {
  TStrList out;
  out.Add("#declare bond_single_cone=object{ cylinder {<0,0,0>, <0,0,1>, 0.1} }");
  out.Add("#declare bond_top_disk=object{ disc {<0,0,1><0,0,1>, 0.1} }");
  out.Add("#declare bond_bottom_disk=object{ disc {<0,0,0><0,0,-1>, 0.1} }");
  out.Add("#declare bond_middle_disk=object{ disc {<0,0,0.5><0,0,1>, 0.1} }");
  out.Add("#declare bond_bottom_cone=object{ cylinder {<0,0,0>, <0,0,0.5>, 0.1} }");
  out.Add("#declare bond_top_cone=object{ cylinder {<0,0,0.5>, <0,0,1>, 0.1} }");
  out.Add("#declare bond_bottom_line=object{ cylinder {<0,0,0>, <0,0,0.5>, 0.01} }");
  out.Add("#declare bond_top_line=object{ cylinder {<0,0,0.5>, <0,0,1>, 0.01} }");

  double ConeStipples = GetSettings(p).GetConeStipples();
  out.Add("#declare bond_stipple_cone=object { union {");
  for( double i=0; i < ConeStipples; i++ )  {
    out.Add(" disc {<0,0,") << i/ConeStipples << "><0,0,-1>, 0.1}";
    out.Add(" cylinder {<0,0,") << i/ConeStipples << ">, <0,0,"
      << (i+0.5)/ConeStipples << ">, 0.1}";
    out.Add(" disc {<0,0,") << (double)(i+0.5)/ConeStipples << "><0,0,1>, 0.1}";
  }
  out.Add("}}");

  out.Add("#declare bond_bottom_stipple_cone=object { union {");
  for( double i=0; i < ConeStipples/2; i++ )  {
    out.Add(" disc {<0,0,") << (i+0.5)/ConeStipples << "><0,0,-1>, 0.1}";
    out.Add(" cylinder {<0,0,") << (i+0.5)/ConeStipples << ">, <0,0,"
      << (i+1)/ConeStipples << ">, 0.1}";
    out.Add(" disc {<0,0,") << (i+1)/ConeStipples << "><0,0,1>, 0.1}";
  }
  out.Add("}}");

  out.Add("#declare bond_top_stipple_cone=object { union {");
  for( int i=0; i < ConeStipples/2; i++ )  {
    out.Add(" disc {<0,0,") << ((i+0.5)/ConeStipples+0.5) << "><0,0,-1>, 0.1}";
    out.Add(" cylinder {<0,0,") << ((i+0.5)/ConeStipples+0.5) << ">, <0,0,"
      << ((i+1)/ConeStipples+0.5) << ">, 0.1}";
    out.Add(" disc {<0,0,") << ((i+1)/ConeStipples+0.5) << "><0,0,1>, 0.1}";
  }
  out.Add("}}");

  out.Add("#declare bond_balls_bond=object { union {");
  for( int i=0; i < 12; i++ )
    out.Add(" sphere {<0,0,") << (double)i/12 << ">, 0.02}";
  out.Add("}}");

  out.Add("#declare bond_line=object{ cylinder {<0,0,0>, <0,0,1>, 0.01} }");
  out.Add("#declare bond_stippled_line=object{ cylinder {<0,0,0>, <0,0,1>, 0.01} }");

  out.Add("#declare bond_bottom_double_cone=object { union {");
  out.Add(" cylinder {<0.1,0,0>, <0.1,0,0.5>, 0.05}");
  out.Add(" cylinder {<-0.1,0,0>, <-0.1,0,0.5>, 0.05}");
  out.Add("}}");

  out.Add("#declare bond_top_double_cone=object { union {");
  out.Add(" cylinder {<0.1,0,0.5>, <0.1,0,1>, 0.05}");
  out.Add(" cylinder {<-0.1,0,0.5>, <-0.1,0,1>, 0.05}");
  out.Add("}}");

  out.Add("#declare bond_bottom_triple_cone=object { union {");
  out.Add(" cylinder {<0.1,0,0>, <0.1,0,0.5>, 0.03}");
  out.Add(" cylinder {<0,0,0>, <0,0,0.5>, 0.03}");
  out.Add(" cylinder {<-0.1,0,0>, <-0.1,0,0.5>, 0.03}");
  out.Add("}}");

  out.Add("#declare bond_top_triple_cone=object { union {");
  out.Add(" cylinder {<0.1,0,0.5>, <0.1,0,1>, 0.03}");
  out.Add(" cylinder {<0,0,0.5>, <0,0,1>, 0.03}");
  out.Add(" cylinder {<-0.1,0,0.5>, <-0.1,0,1>, 0.03}");
  out.Add("}}");

  return out;
}
//..............................................................................
const_strlist TXBond::ToWrl(olx_cdict<TGlMaterial, olxstr> &materials) const {
  TStrList out;
  if (olx_abs(Params()[1]) + olx_abs(Params()[2]) < 1e-3 || Params()[3] < 1e-3)
    return out;
  out.Add(" Group{ children[ Transform{");
  wrl::CrdTransformer crdt(Parent.GetBasis());
  vec3d t = crdt.crd(GetFromCrd());
  mat3d m;
  olx_create_rotation_matrix(m, vec3d(Params()[1], Params()[2], 0).Normalise(),
    cos(Params()[0]*M_PI/180));
  m = crdt.matr(m);
  vec3d r;
  double ang = wrl::decompose(m, r);
  out.Add("  translation ") << wrl::to_str(t);
  out.Add("  rotation ").stream(' ') << r[0] << r[1] << r[2] << ang;
  out.Add("  scale ").stream(' ') << Params()[4] << Params()[4] << Params()[3];
  out.Add("  children[");
  const TGPCollection &gpc = GetPrimitives();
  for (size_t i=0; i < gpc.PrimitiveCount(); i++) {
    TGlPrimitive &glp = gpc.GetPrimitive(i);
    olxstr p_mat = wrl::get_mat_str(glp.GetProperties(), materials, this);
    out.Add("   DEF b ") << "bond_" <<
      glp.GetName().ToLowerCase().Replace(' ', '_') << "{appr " << p_mat << '}';
  }
  out.Add(" ]}]}");
  return out;
}
//..............................................................................
const_strlist TXBond::WrlDeclare(TGlRenderer &p) {
  TStrList out;
  out.Add("PROTO bond_single_cone[exposedField SFNode appr NULL]{") <<
    " Transform{ rotation 1 0 0 1.5708 translation 0 0 0.5 "
    "children Shape{ appearance IS appr "
    "geometry Cylinder{ height 1 radius 0.1 top FALSE bottom FALSE}}}}";
  out.Add("PROTO bond_top_cone[exposedField SFNode appr NULL]{") <<
    " Transform{ rotation 1 0 0 1.5708 translation 0 0 0.75 "
    "children Shape{ appearance IS appr geometry "
    "Cylinder{ height 0.5 radius 0.1 top FALSE bottom FALSE}}}}";
  out.Add("PROTO bond_bottom_cone[exposedField SFNode appr NULL]{") <<
    " Transform{ rotation 1 0 0 1.5708 translation 0 0 0.25 "
    "children Shape{ appearance IS appr geometry "
    "Cylinder{ height 0.5 radius 0.1 top FALSE bottom FALSE}}}}";
  out.Add("PROTO bond_top_line[exposedField SFNode appr NULL]{") <<
    " Transform{ rotation 1 0 0 1.5708 translation 0 0 0.75 "
    "children Shape{ appearance IS appr geometry "
    "Cylinder{ height 0.5 radius 0.01 top FALSE bottom FALSE}}}}";
  out.Add("PROTO bond_bottom_line[exposedField SFNode appr NULL]{") <<
    " Transform{ rotation 1 0 0 1.5708 translation 0 0 0.25 "
    "children Shape{ appearance IS appr geometry "
    "Cylinder{ height 0.5 radius 0.01 top FALSE bottom FALSE}}}}";
  out.Add("PROTO bond_top_disk[exposedField SFNode appr NULL]{") <<
    " Transform{ rotation 1 0 0 1.5708 translation 0 0 1 "
    "children Shape{ appearance IS appr geometry "
    "Cylinder{ height 0 radius 0.1 side FALSE}}}}";
  out.Add("PROTO bond_bottom_disk[exposedField SFNode appr NULL]{") <<
    " Transform{ rotation 1 0 0 1.5708 "
    "children Shape{ appearance IS appr geometry "
    "Cylinder{ height 0 radius 0.1 side FALSE}}}}";
  out.Add("PROTO bond_middle_disk[exposedField SFNode appr NULL]{") <<
    " Transform{ rotation 1 0 0 1.5708 translation 0 0 0.5 "
    "children Shape{ appearance IS appr geometry "
    "Cylinder{ height 0 radius 0.1 side FALSE}}}}";

  double ConeStipples = GetSettings(p).GetConeStipples();
  double step = 0.5/ConeStipples;
  out.Add("PROTO bond_stipple_cone[exposedField SFNode appr NULL]{") <<
    " Transform{ rotation 1 0 0 1.5708 children[";
  for (double i=0; i < ConeStipples; i++) {
    out.Add(" Transform{ translation 0 ") << 0.05+i/ConeStipples <<
      " 0 children Shape{ appearance IS appr geometry Cylinder{ height " <<
      step << " radius 0.1}}}";
  }
  out.Add("]}}");
  out.Add("PROTO bond_bottom_stipple_cone[exposedField SFNode appr NULL]{") <<
    " Transform{ rotation 1 0 0 1.5708 children[";
  for( double i=0; i < ConeStipples/2; i++ )  {
    out.Add(" Transform{ translation 0 ") << 0.05+i/ConeStipples <<
      " 0 children Shape{ appearance IS appr geometry Cylinder{ height " <<
      step << " radius 0.1}}}";
  }
  out.Add("]}}");
  out.Add("PROTO bond_top_stipple_cone[exposedField SFNode appr NULL]{") <<
    " Transform{ rotation 1 0 0 1.5708 children[";
  for( double i=0; i < ConeStipples/2; i++ )  {
    out.Add(" Transform{ translation 0 ") << 0.55+i/ConeStipples <<
      " 0 children Shape{ appearance IS appr geometry Cylinder{ height " <<
      step << " radius 0.1}}}";
  }
  out.Add("]}}");
  out.Add("PROTO bond_balls_bond[exposedField SFNode appr NULL]{") <<
    " Transform{ children[";
  for( double i=0; i < 12; i++ )  {
    out.Add(" Transform{ translation 0 0 ") << i/12 <<
      " children Shape{ appearance IS appr geometry Sphere{ radius 0.02}}}";
  }
  out.Add("]}}");

  out.Add("PROTO bond_bottom_double_cone[exposedField SFNode appr NULL]{") <<
    " Transform{ rotation 1 0 0 1.5708 translation 0 0 0.25 "
    "children Shape{ appearance IS appr geometry "
    "Cylinder{ height 0.5 radius 0.1 top FALSE bottom FALSE}}}}";
  out.Add("PROTO bond_top_double_cone[exposedField SFNode appr NULL]{") <<
    " Transform{ rotation 1 0 0 1.5708 translation 0 0 0.75 "
    "children Shape{ appearance IS appr geometry "
    "Cylinder{ height 0.5 radius 0.1 top FALSE bottom FALSE}}}}";
  out.Add("PROTO bond_bottom_triple_cone[exposedField SFNode appr NULL]{") <<
    " Transform{ rotation 1 0 0 1.5708 translation 0 0 0.25 "
    "children Shape{ appearance IS appr geometry "
    "Cylinder{ height 0.5 radius 0.1 top FALSE bottom FALSE}}}}";
  out.Add("PROTO bond_top_triple_cone[exposedField SFNode appr NULL]{") <<
    " Transform{ rotation 1 0 0 1.5708 translation 0 0 0.75 "
    "children Shape{ appearance IS appr geometry "
    "Cylinder{ height 0.5 radius 0.1 top FALSE bottom FALSE}}}}";

  //out.Add("#declare bond_line=object{ cylinder {<0,0,0>, <0,0,1>, 0.01} }");
  //out.Add("#declare bond_stippled_line=object{ cylinder {<0,0,0>, <0,0,1>, 0.01} }");
  return out;
}
//..............................................................................
uint32_t TXBond::GetPrimitiveMask() const {
  return GetPrimitives().GetStyle().GetNumParam(GetPrimitiveMaskName(),
    (GetType() == sotHBond) ? 2048
      : GetSettings().GetMask(), IsMaskSaveable());
}
//..............................................................................
bool TXBond::OnMouseDown(const IOlxObject *Sender, const TMouseData& Data)  {
  return Label->IsVisible() ? Label->OnMouseDown(Sender, Data) : false;
}
//..............................................................................
bool TXBond::OnMouseUp(const IOlxObject *Sender, const TMouseData& Data)  {
  return Label->IsVisible() ? Label->OnMouseMove(Sender, Data) : false;
}
//..............................................................................
bool TXBond::OnMouseMove(const IOlxObject *Sender, const TMouseData& Data)  {
  return Label->IsVisible() ? Label->OnMouseMove(Sender, Data) : false;
}
//..............................................................................
olxstr TXBond::GetLegend(const TSBond& Bnd, size_t level) {
  olxstr L(EmptyString(), 32);
  const TSAtom *A = &Bnd.A(),
    *B = &Bnd.B();
  if (A->GetType() != B->GetType()) {
    if (A->GetType() < B->GetType()) {
      olx_swap(A, B);
    }
  }
  else {
    if (A->GetLabel().Compare(B->GetLabel()) < 0)
      olx_swap(A, B);
  }
  L << A->GetType().symbol << '-' << B->GetType().symbol;
  if (Bnd.GetType() == sotHBond) {
    L << "@H";
  }
  if (Bnd.GetOrder() > sboSingle) {
    L << "*" << Bnd.GetOrder();
  }
  if (level == 0) {
    return L;
  }
  L << '.' << A->CAtom().GetResiLabel() << '-' << B->CAtom().GetResiLabel();
  if (level == 1) {
    return L;
  }
  TUnitCell::SymmSpace sp =
    A->GetNetwork().GetLattice().GetUnitCell().GetSymmSpace();
  L << '.' << TSymmParser::MatrixToSymmCode(sp, A->GetMatrix()) <<
    '-' <<
    TSymmParser::MatrixToSymmCode(sp, B->GetMatrix());
  if (level == 2) {
    return L;
  }
  return L << ".u";
}
//..............................................................................
void TXBond::SetRadius(double V) {
  Params()[4] = V;
  if (this->Primitives != NULL) {
    GetPrimitives().GetStyle().SetParam("R", V, IsRadiusSaveable());
    // update radius for all members of the collection
    for (size_t i = 0; i < GetPrimitives().ObjectCount(); i++)
      GetPrimitives().GetObject(i).Params()[4] = V;
  }
}
//..............................................................................
//..............................................................................
//..............................................................................
void TXBond::Settings::CreatePrimitives() {
  ClearPrimitives();
  TGlMaterial GlM;
  double ConeQ = GetConeQ();
  double ConeStipples = GetConeStipples();
  //..............................
  // create single color cylinder
  TGlPrimitive *GlP = &parent.NewPrimitive(sgloCylinder);
  primitives.Add("Single cone", GlP);

  GlP->Params[0] = 0.1;  GlP->Params[1] = 0.1;  GlP->Params[2] = 1;
  GlP->Params[3] = ConeQ;   GlP->Params[4] = 1;
  GlP->Compile();
  GlP->Params.Resize(GlP->Params.Count() + 1);  //
  GlP->Params.GetLast() = ddsDefAtomA;
  //..............................
  // create top disk
  GlP = &parent.NewPrimitive(sgloCommandList);
  primitives.Add("Top disk", GlP);

  TGlPrimitive *GlPRC1 = &parent.NewPrimitive(sgloDisk);
  GlPRC1->Params[0] = 0;  GlPRC1->Params[1] = 0.1;  GlPRC1->Params[2] = ConeQ;
  GlPRC1->Params[3] = 1;
  GlPRC1->Compile();

  GlP->StartList();
  olx_gl::translate(0, 0, 1);
  GlP->CallList(GlPRC1);
  GlP->EndList();

  GlP->Params.Resize(GlP->Params.Count() + 1);  //
  GlP->Params.GetLast() = ddsDefAtomB;
  //..............................
  // create bottom disk
  GlP = &parent.NewPrimitive(sgloDisk);
  primitives.Add("Bottom disk", GlP);

  GlP->SetQuadricOrientation(GLU_INSIDE);
  GlP->Params[0] = 0;  GlP->Params[1] = 0.1;  GlP->Params[2] = ConeQ;
  GlP->Params[3] = 1;
  GlP->Compile();

  GlP->Params.Resize(GlP->Params.Count() + 1);  //
  GlP->Params.GetLast() = ddsDefAtomA;
  //..............................
  // create middle disk
  GlP = &parent.NewPrimitive(sgloCommandList);
  primitives.Add("Middle disk", GlP);

  GlPRC1 = &parent.NewPrimitive(sgloDisk);
  GlPRC1->Params[0] = 0;  GlPRC1->Params[1] = 0.1;  GlPRC1->Params[2] = ConeQ;
  GlPRC1->Params[3] = 1;
  GlPRC1->Compile();

  GlP->StartList();
  olx_gl::translate(0.0f, 0.0f, 0.5f);
  GlP->CallList(GlPRC1);
  GlP->EndList();

  GlP->Params.Resize(GlP->Params.Count() + 1);  //
  GlP->Params.GetLast() = ddsDefAtomA;
  //..............................
  // create bottom cylinder
  GlP = &parent.NewPrimitive(sgloCylinder);
  primitives.Add("Bottom cone", GlP);

  GlP->Params[0] = 0.1;  GlP->Params[1] = 0.1;  GlP->Params[2] = 0.5;
  GlP->Params[3] = ConeQ;   GlP->Params[4] = 1;
  GlP->Compile();

  GlP->Params.Resize(GlP->Params.Count() + 1);  //
  GlP->Params.GetLast() = ddsDefAtomA;
  //..............................
  // create top cylinder
  GlP = &parent.NewPrimitive(sgloCommandList);
  primitives.Add("Top cone", GlP);

  GlPRC1 = &parent.NewPrimitive(sgloCylinder);
  GlPRC1->Params[0] = 0.1;    GlPRC1->Params[1] = 0.1;  GlPRC1->Params[2] = 0.5;
  GlPRC1->Params[3] = ConeQ;  GlPRC1->Params[4] = 1;
  GlPRC1->Compile();

  GlP->StartList();
  olx_gl::translate(0.0f, 0.0f, 0.5f);
  GlP->CallList(GlPRC1);
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count() + 1);  //
  GlP->Params.GetLast() = ddsDefAtomB;
  //..............................
  // create bottom line
  GlP = &parent.NewPrimitive(sgloCommandList);
  primitives.Add("Bottom line", GlP);

  GlP->StartList();
  olx_gl::begin(GL_LINES);
  olx_gl::vertex(0, 0, 0);
  olx_gl::vertex(0.0f, 0.0f, 0.5f);
  olx_gl::end();
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count() + 1);  //
  GlP->Params.GetLast() = ddsDefAtomA;
  //..............................
  // create top line
  GlP = &parent.NewPrimitive(sgloCommandList);
  primitives.Add("Top line", GlP);

  GlP->StartList();
  olx_gl::begin(GL_LINES);
  olx_gl::vertex(0.0f, 0.0f, 0.5f);
  olx_gl::vertex(0, 0, 1);
  olx_gl::end();
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count() + 1);  //
  GlP->Params.GetLast() = ddsDefAtomB;
  //..............................
  // create stipple cone
  double CL = (double)(1.0 / (2 * ConeStipples));
  GlP = &parent.NewPrimitive(sgloCommandList);
  primitives.Add("Stipple cone", GlP);

  GlPRC1 = &parent.NewPrimitive(sgloCylinder);
  GlPRC1->Params[0] = 0.1;    GlPRC1->Params[1] = 0.1;  GlPRC1->Params[2] = CL;
  GlPRC1->Params[3] = ConeQ;  GlPRC1->Params[4] = 1;
  GlPRC1->Compile();

  TGlPrimitive *GlPRD1 = &parent.NewPrimitive(sgloDisk);
  GlPRD1->Params[0] = 0;  GlPRD1->Params[1] = 0.1;  GlPRD1->Params[2] = ConeQ;
  GlPRD1->Params[3] = 1;
  GlPRD1->Compile();

  TGlPrimitive *GlPRD2 = &parent.NewPrimitive(sgloDisk);
  GlPRD2->SetQuadricOrientation(GLU_INSIDE);
  GlPRD2->Params[0] = 0;  GlPRD2->Params[1] = 0.1;  GlPRD2->Params[2] = ConeQ;
  GlPRD2->Params[3] = 1;
  GlPRD2->Compile();

  GlP->StartList();
  for (int i = 0; i < ConeStipples; i++)  {
    if (i != 0)
      GlP->CallList(GlPRD2);
    GlP->CallList(GlPRC1);
    olx_gl::translate(0.0, 0.0, CL);
    GlP->CallList(GlPRD1);
    olx_gl::translate(0.0, 0.0, CL);
  }
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count() + 1);  //
  GlP->Params.GetLast() = ddsDef;
  //..............................
  GlP = &parent.NewPrimitive(sgloCommandList);
  primitives.Add("Bottom stipple cone", GlP);

  GlP->StartList();
  olx_gl::translate(0.0, 0.0, CL / 2);
  for (int i = 0; i < ConeStipples / 2; i++)  {
    if (i != 0)
      GlP->CallList(GlPRD2);
    GlP->CallList(GlPRC1);
    olx_gl::translate(0.0, 0.0, CL);
    GlP->CallList(GlPRD1);
    olx_gl::translate(0.0, 0.0, CL);
  }
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count() + 1);  //
  GlP->Params.GetLast() = ddsDefAtomA;
  //..............................
  GlP = &parent.NewPrimitive(sgloCommandList);
  primitives.Add("Top stipple cone", GlP);

  GlP->StartList();
  olx_gl::translate(0.0, 0.0, (0.5 + CL / 2));
  for (int i = 0; i < ConeStipples / 2; i++)  {
    GlP->CallList(GlPRD2);
    GlP->CallList(GlPRC1);
    olx_gl::translate(0.0, 0.0, CL);
    GlP->CallList(GlPRD1);
    olx_gl::translate(0.0, 0.0, CL);
  }
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count() + 1);  //
  GlP->Params.GetLast() = ddsDefAtomB;

  //..............................
  // create stipped ball bond
  CL = (double)(1.0 / (12.0));
  GlP = &parent.NewPrimitive(sgloCommandList);
  primitives.Add("Balls bond", GlP);

  GlPRC1 = &parent.NewPrimitive(sgloSphere);
  GlPRC1->Params[0] = 0.04;    GlPRC1->Params[1] = 5;  GlPRC1->Params[2] = 5;
  GlPRC1->Compile();
  stockPrimitives.Add("Balls bond sphere", GlPRC1);

  GlP->StartList();
  for (int i = 0; i < 12; i++)  {
    olx_gl::translate(0.0, 0.0, CL);
    GlP->CallList(GlPRC1);
  }
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count() + 1);  //
  GlP->Params.GetLast() = ddsDef;
  //..............................
  // create line
  GlP = &parent.NewPrimitive(sgloCommandList);
  primitives.Add("Line", GlP);

  GlP->StartList();
  olx_gl::begin(GL_LINES);
  olx_gl::vertex(0, 0, 0);
  olx_gl::vertex(0, 0, 1);
  olx_gl::end();
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count() + 1);  //
  GlP->Params.GetLast() = ddsDefAtomA;
  //..............................
  // create stippled line
  GlP = &parent.NewPrimitive(sgloCommandList);
  primitives.Add("Stippled line", GlP);

  GlP->StartList();
  olx_gl::enable(GL_LINE_STIPPLE);
  olx_gl::lineStipple(1, 0xf0f0);
  olx_gl::begin(GL_LINES);
  olx_gl::vertex(0, 0, 0);
  olx_gl::vertex(0, 0, 1);
  olx_gl::end();
  olx_gl::disable(GL_LINE_STIPPLE);
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count() + 1);  //
  GlP->Params.GetLast() = ddsDefAtomA;
  //..............................
  // create bottom double cylinder
  GlP = &parent.NewPrimitive(sgloCylinder);
  primitives.Add("Bottom double cone", GlP);
  GlP->Params[0] = 0.1/2;  GlP->Params[1] = 0.1/2;  GlP->Params[2] = 0.5;
  GlP->Params[3] = ConeQ;  GlP->Params[4] = 1;
  GlP->Compile();
  GlP->Params.Resize(GlP->Params.Count() + 1);  //
  GlP->Params.GetLast() = ddsDefAtomA;
  // create top double cylinder
  GlP = &parent.NewPrimitive(sgloCommandList);
  primitives.Add("Top double cone", GlP);

  GlPRC1 = &parent.NewPrimitive(sgloCylinder);
  GlPRC1->Params[0] = 0.1/2;  GlPRC1->Params[1] = 0.1/2;  GlPRC1->Params[2] = 0.5;
  GlPRC1->Params[3] = ConeQ;  GlPRC1->Params[4] = 1;
  GlPRC1->Compile();

  GlP->StartList();
  olx_gl::translate(0.0f, 0.0f, 0.5f);
  GlP->CallList(GlPRC1);
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count() + 1);  //
  GlP->Params.GetLast() = ddsDefAtomB;
  //..............................
  // create bottom tripple cylinder
  GlP = &parent.NewPrimitive(sgloCylinder);
  primitives.Add("Bottom triple cone", GlP);
  GlP->Params[0] = 0.1/3;  GlP->Params[1] = 0.1/3;  GlP->Params[2] = 0.5;
  GlP->Params[3] = ConeQ;  GlP->Params[4] = 1;
  GlP->Compile();
  GlP->Params.Resize(GlP->Params.Count() + 1);  //
  GlP->Params.GetLast() = ddsDefAtomA;
  // create top double cylinder
  GlP = &parent.NewPrimitive(sgloCommandList);
  primitives.Add("Top triple cone", GlP);

  GlPRC1 = &parent.NewPrimitive(sgloCylinder);
  GlPRC1->Params[0] = 0.1/3;  GlPRC1->Params[1] = 0.1/3;  GlPRC1->Params[2] = 0.5;
  GlPRC1->Params[3] = ConeQ;  GlPRC1->Params[4] = 1;
  GlPRC1->Compile();

  GlP->StartList();
  olx_gl::translate(0.0, 0.0, 0.5);
  GlP->CallList(GlPRC1);
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count() + 1);  //
  GlP->Params.GetLast() = ddsDefAtomB;

  // create arrows
  GlPRC1 = &parent.NewPrimitive(sgloCylinder);
  GlPRC1->Params[0] = 0.1;    GlPRC1->Params[1] = 0.1;  GlPRC1->Params[2] = 1;
  GlPRC1->Params[3] = ConeQ;  GlPRC1->Params[4] = 1;
  GlPRC1->Compile();
  stockPrimitives.Add("Arrow cylinder", GlPRC1);
  
  TGlPrimitive *GlPRC2 = &parent.NewPrimitive(sgloCylinder);
  GlPRC2->Params[0] = 0.0;    GlPRC2->Params[1] = 0.15;  GlPRC2->Params[2] = 0.2;
  GlPRC2->Params[3] = ConeQ;   GlPRC2->Params[4] = 1;
  GlPRC2->Compile();
  stockPrimitives.Add("Arrow bottom head", GlPRC2);
  GlPRD1 = &parent.NewPrimitive(sgloDisk);
  GlPRD1->Params[0] = 0;  GlPRD1->Params[1] = 0.15;  GlPRD1->Params[2] = ConeQ;
  GlPRD1->Params[3] = 1;
  GlPRD1->Compile();
  stockPrimitives.Add("Bottom arrow disk", GlPRD1);

  GlPRC2 = &parent.NewPrimitive(sgloCylinder);
  GlPRC2->Params[0] = 0.15;    GlPRC2->Params[1] = 0;  GlPRC2->Params[2] = 0.2;
  GlPRC2->Params[3] = ConeQ;   GlPRC2->Params[4] = 1;
  GlPRC2->Compile();
  stockPrimitives.Add("Arrow top head", GlPRC2);
  GlPRD1 = &parent.NewPrimitive(sgloDisk);
  GlPRD1->Params[0] = 0;  GlPRD1->Params[1] = 0.15;  GlPRD1->Params[2] = ConeQ;
  GlPRD1->Params[3] = 1;  GlPRD1->SetQuadricOrientation(GLU_INSIDE);
  GlPRD1->Compile();
  stockPrimitives.Add("Top arrow disk", GlPRD1);

  GlPRC1 = &parent.NewPrimitive(sgloCylinder);
  GlPRC1->Params[0] = 0.1;    GlPRC1->Params[1] = 0.1;  GlPRC1->Params[2] = 0.5;
  GlPRC1->Params[3] = ConeQ;  GlPRC1->Params[4] = 1;
  GlPRC1->Compile();
  GlPRD1 = &parent.NewPrimitive(sgloDisk);
  GlPRD1->Params[0] = 0;  GlPRD1->Params[1] = 0.1;  GlPRD1->Params[2] = ConeQ;
  GlPRD1->Params[3] = 1;
  GlPRD1->Compile();
  GlPRD2 = &parent.NewPrimitive(sgloDisk);
  GlPRD2->Params[0] = 0;  GlPRD2->Params[1] = 0.1;  GlPRD2->Params[2] = ConeQ;
  GlPRD2->Params[3] = 1;  GlPRD2->SetQuadricOrientation(GLU_INSIDE);
  GlPRD2->Compile();

  GlP = &parent.NewPrimitive(sgloCommandList);
  GlP->StartList();
  GlP->CallList(GlPRC1);
  olx_gl::translate(0.0, 0.0, 0.5);
  GlP->CallList(GlPRD1);
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count() + 1);  //
  GlP->Params.GetLast() = ddsDefAtomA;
  stockPrimitives.Add("Custom bottom cylinder", GlP);

  GlP = &parent.NewPrimitive(sgloCommandList);
  GlP->StartList();
  olx_gl::translate(0.0, 0.0, -0.5);
  GlP->CallList(GlPRC1);
  GlP->CallList(GlPRD2);
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count() + 1);  //
  GlP->Params.GetLast() = ddsDefAtomB;
  stockPrimitives.Add("Custom top cylinder", GlP);

  // create bottom arrow
  GlP = &parent.NewPrimitive(sgloCommandList);
  primitives.Add("Bottom arrow", GlP);
  GlP->Params.Resize(GlP->Params.Count() + 1);  //
  GlP->Params.GetLast() = ddsDefAtomA;
  //..............................
  // create top cylinder
  GlP = &parent.NewPrimitive(sgloCommandList);
  primitives.Add("Top arrow", GlP);
  GlP->Params.Resize(GlP->Params.Count() + 1);  //
  GlP->Params.GetLast() = ddsDefAtomB;
}
//..............................................................................
void TXBond::ToDataItem(TDataItem& item) const {
  TSBond::ToDataItem(item);
  if (Params()[5] != 0) {
    item.AddField("fbond", Params()[5]);
  }
}
//..............................................................................
void TXBond::FromDataItem(const TDataItem& item, class TLattice& parent) {
  TSBond::FromDataItem(item, parent);
  olxstr v = item.FindField("fbond", EmptyString());
  if (!v.IsEmpty()) {
    Params()[5] = v.ToDouble();
  }
}
//..............................................................................
