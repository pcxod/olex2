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
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);
  TGPCollection& GPC = Parent.FindOrCreateCollection(GetCollectionName());
  if( GPC.ObjectCount() == 0 && GPC.PrimitiveCount() != 0 )
    GPC.ClearPrimitives();
  size_t deleted_cnt = 0;
  for( size_t i=0; i < GPC.ObjectCount(); i++ )  {
    if( EsdlInstanceOf(GPC.GetObject(i), TXPlane) &&
      ((TXPlane&)GPC.GetObject(i)).IsDeleted() )
    {
      deleted_cnt++;
    }
  }
  if( deleted_cnt == GPC.ObjectCount() )
    GPC.ClearPrimitives();
  GPC.AddObject(*this);
  const mat3d& m = GetBasis();
  Params().Resize(16);
  FParams[0] = m[0][0];  FParams[1] = m[0][1];  FParams[2] = m[0][2];  FParams[3] = 0;
  FParams[4] = m[1][0];  FParams[5] = m[1][1];  FParams[6] = m[1][2];  FParams[7] = 0;
  FParams[8] = m[2][0];  FParams[9] = m[2][1];  FParams[10]= m[2][2];  FParams[11]= 0;
  FParams[12] = 0;       FParams[13] = 0;       FParams[14]= 0;        FParams[15]= 1;

  if( GPC.PrimitiveCount() != 0 )  return;

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
    if( !IsRegular() )  
      GlP.Vertices.SetCount(CrdCount());
    else                 
      GlP.Vertices.SetCount(4);

    PlaneSort::Sorter sp(*this);
    const mat3d transform = GetBasis();
    if( !IsRegular() )  {
      for( size_t i=0; i < sp.sortedPlane.Count(); i++ )  {
        const vec3d* crd = sp.sortedPlane.GetObject(i);
        GlP.Vertices[i] = transform*GetNormal().Normal((*crd-GetCenter()));
      }
    }
    else  {
      double maxrs = 0;
      for( size_t i=0; i < sp.sortedPlane.Count(); i++ )  {
        const vec3d* crd = sp.sortedPlane.GetObject(i);
        const double qd = (*crd-GetCenter()).QLength();
        if( qd > maxrs )
          maxrs = qd;
      }
      vec3d marv(0, sqrt(maxrs), 0);
      mat3d rm;
      olx_create_rotation_matrix(rm, vec3d(1,0,0), cos(M_PI*(360/4)/180));
      for( int i=0; i < 4; i++ )  {
        GlP.Vertices[i] = marv;    
        marv *= rm;
      }
    }

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
  olx_gl::translate(GetCenter());
  if( P.GetType() != sgloSphere )  {
    olx_gl::orient(Params().GetRawData());
    olx_gl::normal(GetNormal());
  }
  return false;
}
//..............................................................................
void TXPlane::ListPrimitives(TStrList &List) const {
  List.Add("Plane");
  List.Add("Centroid");
}
//..............................................................................
TStrList TXPlane::PovDeclare()  {
  TStrList out;
  out.Add("#declare plane_centroid=object{ sphere {<0,0,0>, 0.25} }");
  return out;
}
//..............................................................................
TStrList TXPlane::ToPov(olxdict<const TGlMaterial*, olxstr,
  TPrimitiveComparator> &materials) const
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
      const mat3d &m = GetBasis();
      for( size_t j=0; j < glp.Vertices.Count(); j++ )  {
        out.Add("    smooth_triangle {");
        out.Add("     ") << pov::to_str(zv) << pov::to_str(n);
        out.Add("     ") << pov::to_str(crdc.normal(glp.Vertices[j]*m)) << pov::to_str(n);
        out.Add("     ") << pov::to_str(
          crdc.normal(glp.Vertices[j == glp.Vertices.Count()-1 ? 0 : j+1]*m))
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
