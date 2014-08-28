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
#include "glprimitive.h"
#include "styles.h"
#include "gpcollection.h"
#include "povdraw.h"

void TXPlane::Create(const olxstr& cName)  {
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
  TGPCollection& GPC = Parent.FindOrCreateCollection(GetCollectionName());
  if (GPC.ObjectCount() == 0 && GPC.PrimitiveCount() != 0)
    GPC.ClearPrimitives();
  size_t deleted_cnt = 0;
  for (size_t i=0; i < GPC.ObjectCount(); i++) {
    if (EsdlInstanceOf(GPC.GetObject(i), TXPlane) &&
      ((TXPlane&)GPC.GetObject(i)).IsDeleted())
    {
      deleted_cnt++;
    }
  }
  if (deleted_cnt == GPC.ObjectCount())
    GPC.ClearPrimitives();
  GPC.AddObject(*this);
  if (IsRegular()) {
    double maxrs = (GetAtom(0).crd()-GetCenter()).QLength();
    size_t maxr_i = 0;
    for (size_t i=1; i < Count(); i++) {
      const double qd = (GetAtom(i).crd()-GetCenter()).QLength();
      if (qd > maxrs) {
        maxrs = qd;
        maxr_i = i;
      }
    }
    MaxV = (GetAtom(maxr_i).crd()-GetCenter());
    olx_create_rotation_matrix(RM, GetNormal(), cos(M_PI*(360/4)/180));
  }
  else {
    PlaneSort::Sorter::SortPlane(*this);
  }
  if (GPC.PrimitiveCount() != 0)
    return;

  TGraphicsStyle& GS = GPC.GetStyle();
  GS.SetPersistent(true);
  const int PMask = GS.GetParam(GetPrimitiveMaskName(), "3", true).ToInt();
  if( PMask == 0 )  return;
  if( (PMask & 1) != 0 )  {
    TGlMaterial GlM;
    GlM.SetFlags(sglmAmbientF|sglmDiffuseF|sglmAmbientB|sglmDiffuseB|sglmTransparent);
    GlM.AmbientF = 0x7f00007f;
    GlM.DiffuseF = 0x7f3f3f3f;
    GlM.AmbientB = 0x7f00007f;
    GlM.DiffuseB = 0x7f3f3f3f;
    TGlPrimitive& GlP = GPC.NewPrimitive("Plane", sgloPolygon);
    GlP.SetProperties(GS.GetMaterial(GlP.GetName(), GlM));
  }
  if( (PMask & 2) != 0 )  {
    TGlMaterial GlM1;
    GlM1.SetFlags(sglmAmbientF);
    GlM1.AmbientF = 0;
    TGlPrimitive& glpC = GPC.NewPrimitive("Centroid", sgloSphere);
    glpC.SetProperties(GS.GetMaterial(glpC.GetName(), GlM1));
    glpC.Params[0] = 0.25;  glpC.Params[1] = 10;  glpC.Params[2] = 10;
  }
  Compile();
}
//..............................................................................
bool TXPlane::Orient(TGlPrimitive& P)  {
  if (P.GetType() != sgloSphere) {
    olx_gl::FlagDisabler fc(GL_CULL_FACE);
    if (IsRegular()) {
      olx_gl::translate(GetCenter());
      olx_gl::normal(GetNormal());
      vec3d v = MaxV;
      olx_gl::begin(GL_QUADS);
      for (int i=0; i < 4; i++) {
        olx_gl::vertex(v);
        v *= RM;
      }
      olx_gl::end();
    }
    else {
      olx_gl::normal(GetNormal());
      olx_gl::begin(GL_POLYGON);
      for (size_t i=0; i < Count(); i++) {
        olx_gl::vertex(GetAtom(i).crd());
      }
      olx_gl::end();
    }
    return true;
  }
  else
    olx_gl::translate(GetCenter());
  return false;
}
//..............................................................................
void TXPlane::ListPrimitives(TStrList &List) const {
  List.Add("Plane");
  List.Add("Centroid");
  List.Add("Ring");
}
//..............................................................................
const_strlist TXPlane::PovDeclare()  {
  TStrList out;
  out.Add("#declare plane_centroid=object{ sphere {<0,0,0>, 0.25} }");
  return out;
}
//..............................................................................
const_strlist TXPlane::ToPov(olxdict<TGlMaterial, olxstr,
  TComparableComparator> &materials) const
{
  TStrList out;
   pov::CrdTransformer crdc(Parent.GetBasis());
  out.Add(" object { union {");
  const TGPCollection &gpc = GetPrimitives();
  for( size_t i=0; i < gpc.PrimitiveCount(); i++ )  {
    TGlPrimitive &glp = gpc.GetPrimitive(i);
    if( glp.GetType() == sgloPolygon )  {
      out.Add("   object { union {");
      vec3d zv = vec3d(),
        n = crdc.normal(GetNormal());
      const mat3f m = GetBasis();
      for (size_t j=0; j < Count(); j++) {
        out.Add("    smooth_triangle {");
        out.Add("     ") << pov::to_str(zv) << pov::to_str(n);
        out.Add("     ") << pov::to_str(crdc.normal(GetAtom(j).crd()-GetCenter()))
          << pov::to_str(n);
        out.Add("     ") << pov::to_str(
          crdc.normal(GetAtom(j == Count()-1 ? 0 : j+1).crd()-GetCenter()))
          << pov::to_str(n);
        out.Add("     }");
      }
      out.Add("    }");
    }
    else {
      out.Add("   object {") << "plane_"
        << glp.GetName().ToLowerCase().Replace(' ', '_');
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
