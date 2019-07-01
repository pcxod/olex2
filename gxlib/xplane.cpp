/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "xplane.h"
#include "estlist.h"
#include "planesort.h"
#include "network.h"
#include "lattice.h"
#include "glprimitive.h"
#include "styles.h"
#include "gpcollection.h"
#include "dring.h"
#include "povdraw.h"
#include "wrldraw.h"
#include "xline.h"

void TXPlane::Create(const olxstr& cName) {
  olxstr colName = cName;
  if (colName.IsEmpty()) {
    colName = NamesRegistry().Find(GetDefId(), EmptyString());
  }
  else {
    NamesRegistry().Add(GetDefId(), colName);
  }
  if (colName.IsEmpty()) {
    colName = olxstr("TXPlane") << GetDefId();
  }
  if (!colName.IsEmpty()) {
    SetCollectionName(colName);
  }
  if (GetNetwork().GetLattice().GetPlaneDefinitions().Count() <=
    this->GetDefId())
  {
    SetDeleted(true);
    return;
  }
  TGPCollection& GPC = Parent.FindOrCreateCollection(GetCollectionName());
  if (GPC.ObjectCount() == 0 && GPC.PrimitiveCount() != 0) {
    GPC.ClearPrimitives();
  }
  size_t deleted_cnt = 0;
  for (size_t i = 0; i < GPC.ObjectCount(); i++) {
    if (GPC.GetObject(i).Is<TXPlane>() &&
      ((TXPlane&)GPC.GetObject(i)).IsDeleted())
    {
      deleted_cnt++;
    }
  }
  if (deleted_cnt == GPC.ObjectCount()) {
    GPC.ClearPrimitives();
  }
  GPC.AddObject(*this);
  const TSPlane::Def &def =
    GetNetwork().GetLattice().GetPlaneDefinitions()[this->GetDefId()];
  double maxrs = (GetAtom(0).crd() - GetCenter()).Project(GetNormal()).QLength(),
    minrs = maxrs;
  size_t maxr_i = 0;
  for (size_t i = 1; i < Count(); i++) {
    const double qd = (GetAtom(i).crd() - GetCenter()).Project(GetNormal())
      .QLength();
    if (qd > maxrs) {
      maxrs = qd;
      maxr_i = i;
    }
    if (qd < minrs) {
      minrs = qd;
    }
  }
  if (def.GetSides() > 2) {
    MaxV = (GetAtom(maxr_i).crd() - GetCenter());
    MaxV.Project(GetNormal());
    olx_create_rotation_matrix(RM, GetNormal(),
      cos(2 * M_PI / def.GetSides()));
  }
  else {
    PlaneSort::Sorter::SortPlane(*this);
  }
  if (GPC.PrimitiveCount() != 0) {
    return;
  }

  TGraphicsStyle& GS = GPC.GetStyle();
  GS.SetPersistent(true);
  const int PMask = GS.GetParam(GetPrimitiveMaskName(), "3", true).ToInt();
  if (PMask == 0) {
    return;
  }
  if ((PMask & 1) != 0) {
    TGlMaterial GlM;
    GlM.SetFlags(sglmAmbientF | sglmDiffuseF | sglmAmbientB | sglmDiffuseB | sglmTransparent);
    GlM.AmbientF = 0x7f00007f;
    GlM.DiffuseF = 0x7f3f3f3f;
    GlM.AmbientB = 0x7f00007f;
    GlM.DiffuseB = 0x7f3f3f3f;
    TGlPrimitive& GlP = GPC.NewPrimitive("Plane", sgloPolygon);
    GlP.SetProperties(GS.GetMaterial(GlP.GetName(), GlM));
    GlP.SetOwnerId(0);
  }
  TGlMaterial glm("511;5460819;0;12632256;6250335;4292861919;4294967295;12;12");
  if ((PMask & 2) != 0) {
    TGlPrimitive& glpC = GPC.NewPrimitive("Centroid", sgloSphere);
    glpC.SetProperties(GS.GetMaterial(glpC.GetName(), glm));
    glpC.Params[0] = 0.1;  glpC.Params[1] = 10;  glpC.Params[2] = 10;
    glpC.SetOwnerId(1);
  }
  if ((PMask & 4) != 0) {
    TGlPrimitive& GlP = GPC.NewPrimitive("Ring", sgloCommandList);
    GlP.SetProperties(GS.GetMaterial("Ring", glm));
    TDRing::Settings &rset = TDRing::GetSettings(Parent);
    const TStringToList<olxstr, TGlPrimitive*> &primtives =
      rset.GetPrimitives(true);
    GlP.StartList();
    minrs = sqrt(minrs)*cos(M_PI / Count()) / (rset.GetTubeR() + 1);
    olx_gl::scale(minrs*0.85);
    GlP.CallList(primtives.GetObject(0));
    GlP.EndList();
    GlP.SetOwnerId(2);
  }
  if ((PMask & 8) != 0) {
    TXBond::Settings st = TXBond::GetSettings(Parent);
    TGlPrimitive& GlP = GPC.NewPrimitive("Normal", sgloCommandList);
    GlP.SetProperties(GS.GetMaterial("Normal", glm));
    GlP.StartList();
    GlP.CallList(st.GetPrimitives(true).GetObject(4));
    GlP.CallList(st.GetPrimitives().GetObject(19));
    GlP.EndList();
    GlP.SetOwnerId(3);
  }
  Compile();
}
//..............................................................................
bool TXPlane::Orient(TGlPrimitive& P) {
  if (P.GetOwnerId() == 0) {
    olx_gl::FlagDisabler fc(GL_CULL_FACE);
    const TSPlane::Def &def =
      GetNetwork().GetLattice().GetPlaneDefinitions()[this->GetDefId()];
    if (def.GetSides() > 2) {
      olx_gl::translate(GetCenter());
      olx_gl::normal(GetNormal());
      vec3d v = MaxV;
      olx_gl::begin(GL_POLYGON);
      for (size_t i = 0; i < def.GetSides(); i++) {
        olx_gl::vertex(v);
        v *= RM;
      }
      olx_gl::end();
    }
    else {
      olx_gl::normal(GetNormal());
      olx_gl::begin(GL_POLYGON);
      for (size_t i = 0; i < Count(); i++) {
        olx_gl::vertex(GetAtom(i).crd());
      }
      olx_gl::end();
    }
    return true;
  }
  else if (P.GetOwnerId() == 2) {
    olx_gl::translate(GetCenter());
    mat3d m = GetBasis();
    m.SwapRows(0, 2);
    m[1] = m[2].XProdVec(m[0]).Normalise();//m[0] *= -1;
    olx_gl::orient(m);
  }
  else if (P.GetOwnerId() == 3) {
    TXBond::Settings st = TXBond::GetSettings(Parent);
    vec3d v = GetNormal();
    double scale = 1.5 - 0.2;
    olx_gl::translate(GetCenter());
    olx_gl::rotate(acos(v[2])*180/M_PI, -v[1], v[0], 0.0);
    olx_gl::scale(0.5, 0.5, scale);
    st.GetStockPrimitives().GetObject(1)->Draw();
    olx_gl::scale(1.0, 1.0, 1. / scale);
    olx_gl::translate(0.0, 0.0, scale);
    st.GetStockPrimitives().GetObject(5)->Draw();
    st.GetStockPrimitives().GetObject(4)->Draw();
    return true;
  }
  else {
    olx_gl::translate(GetCenter());
  }
  return false;
}
//..............................................................................
void TXPlane::ListPrimitives(TStrList &List) const {
  List.Add("Plane");
  List.Add("Centroid");
  List.Add("Ring");
  List.Add("Normal");
}
//..............................................................................
const_strlist TXPlane::PovDeclare()  {
  TStrList out;
  out.Add("#declare plane_centroid=object{ sphere {<0,0,0>, 0.1} }");
  out.Add("#declare plane_ring=union{");
  out.Add("  object{ torus {1, 0.075} }");
  out.Add("  transform{ rotate <90,0,0> }}");

  out.Add("#declare plane_normal=union{");
  out.Add("  cylinder {<0,0,0>, <0,0,1.0>, 0.05}");
  out.Add("  cone {<0,0,1.0>, 0.10, <0,0,1.2>, 0}}");
  return out;
}
//..............................................................................
const_strlist TXPlane::ToPov(olx_cdict<TGlMaterial, olxstr> &materials) const {
  TStrList out;
  pov::CrdTransformer crdc(Parent.GetBasis());
  out.Add(" object { union {");
  const TGPCollection &gpc = GetPrimitives();
  const TSPlane::Def &def =
    GetNetwork().GetLattice().GetPlaneDefinitions()[this->GetDefId()];
  const mat3d tm = TEBasis::CalcBasis<vec3d, mat3d>(crdc.normal(GetNormal()));
  for (size_t i = 0; i < gpc.PrimitiveCount(); i++) {
    TGlPrimitive &glp = gpc.GetPrimitive(i);
    if (glp.GetOwnerId() == 0) {
      out.Add("   object { union {");
      vec3d zv = vec3d(),
        n = crdc.normal(GetNormal());
      if (def.GetSides() > 2) {
        vec3d v = MaxV;
        for (size_t j = 0; j < def.GetSides(); j++) {
          out.Add("    smooth_triangle {");
          out.Add("     ") << pov::to_str(zv) << pov::to_str(n);
          out.Add("     ") << pov::to_str(crdc.normal(v))
            << pov::to_str(n);
          out.Add("     ") << pov::to_str(
            crdc.normal((j == Count() - 1 ? MaxV : v*RM)))
            << pov::to_str(n);
          out.Add("     }");
          v *= RM;
        }
      }
      else {
        const mat3f m = GetBasis();
        for (size_t j = 0; j < Count(); j++) {
          out.Add("    smooth_triangle {");
          out.Add("     ") << pov::to_str(zv) << pov::to_str(n);
          out.Add("     ") << pov::to_str(crdc.normal(GetAtom(j).crd() - GetCenter()))
            << pov::to_str(n);
          out.Add("     ") << pov::to_str(
            crdc.normal(GetAtom(j == Count() - 1 ? 0 : j + 1).crd() - GetCenter()))
            << pov::to_str(n);
          out.Add("     }");
        }
      }
      out.Add("    }");
    }
    else if (glp.GetOwnerId() == 2) {
      double minrs = (GetAtom(0).crd() - GetCenter()).QLength();
      for (size_t i = 1; i < Count(); i++) {
        const double qd = (GetAtom(i).crd() - GetCenter()).QLength();
        if (qd < minrs) {
          minrs = qd;
        }
      }
      out.Add("   object { object {plane_ring} ");
      out.Add("      transform { ");
      minrs = sqrt(minrs)*cos(M_PI / Count()) / (0.075 + 1);
      mat3d rm = tm * (minrs*0.85);
      out.Add("        matrix") << pov::to_str(rm, vec3d());
      out.Add("      }");
    }
    else {
      out.Add("   object {") << "plane_"
        << glp.GetName().ToLowerCase().Replace(' ', '_');
      out.Add("      transform { ");
      out.Add("        matrix") << pov::to_str(tm, vec3d());
      out.Add("      }");
    }
    olxstr p_mat = pov::get_mat_name(glp.GetProperties(), materials);
    out.Add("    texture {") << pov::get_mat_name(glp.GetProperties(), materials) << '}';
    out.Add("   }");
  }
  out.Add("  }");
  out.Add("  translate ") << pov::to_str(crdc.crd(GetCenter()));
  out.Add(" }");
  return out;
}
//..............................................................................
const_strlist TXPlane::WrlDeclare(TGlRenderer &r)  {
  TStrList out;
  out.Add("PROTO plane_centroid[exposedField SFNode appr NULL]{") <<
    " Transform{ children Shape{ appearance IS appr "
    "geometry Sphere{ radius 0.25}}}}";
  return out;
}
//..............................................................................
const_strlist TXPlane::ToWrl(olx_cdict<TGlMaterial, olxstr> &materials) const {
  TStrList out;
  pov::CrdTransformer crdc(Parent.GetBasis());
  out.Add(" Group { children [");
  out.Add("   Transform {");
  out.Add("     translation ") << wrl::to_str(crdc.crd(GetCenter()));
  out.Add("   children [");
  const TGPCollection &gpc = GetPrimitives();
  for (size_t i = 0; i < gpc.PrimitiveCount(); i++) {
    TGlPrimitive &glp = gpc.GetPrimitive(i);
    if (glp.GetType() == sgloPolygon) {
      TStrList geom;
      out.Add("  Shape{ appearance ") << wrl::get_mat_str("Plane",
        GetPrimitives().GetStyle(), materials, this);
      geom.Add("   geometry IndexedFaceSet{ coord Coordinate{ point[");
      for (size_t j = 0; j < Count(); j++) {
        // no need for translations here, use normal vs crd
        geom.Add("    ") << wrl::to_str(crdc.normal(GetAtom(j).crd() - GetCenter()));
        if (j + 1 < Count()) {
          geom.GetLastString() << ',';
        }
      }
      geom.GetLastString() << "]}";
      out << geom;
      olxstr idx, idx1;
      for (size_t j = 0; j < Count(); j++) {
        idx << ' ' << j;
        idx1 << ' ' << (Count() - j - 1);
      }
      out.Add("    coordIndex[") << idx << " -1] }}";
      out.Add("  Shape{ appearance ") << wrl::get_mat_str("Plane",
        GetPrimitives().GetStyle(), materials, this, true);
      out << geom;
      out.Add("    coordIndex[") << idx1 << " -1] }}";
    }
    else if (!glp.GetName().Equals("Ring")) {
      olxstr glp_name = glp.GetName().ToLowerCase().Replace(' ', '_');
      olxstr p_mat = wrl::get_mat_str(glp.GetProperties(), materials, this);
      out.Add("   DEF a ") << "plane_" << glp_name << "{appr " <<
        p_mat << '}';
    }
  }
  out.Add(" ]}]}");  // Transform, Group
  return out;
}
//..............................................................................
void TXPlane::Invert() {
  TSPlane::Invert();
  RM.Transpose();
}
//..............................................................................
