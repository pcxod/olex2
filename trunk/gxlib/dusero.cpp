/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "dusero.h"
#include "glprimitive.h"
#include "glmaterial.h"
#include "glrender.h"
#include "gpcollection.h"
#include "styles.h"
#include "pers_util.h"
#include "povdraw.h"

TDUserObj::TDUserObj(TGlRenderer& R, short type, const olxstr& collectionName) :
  AGlMouseHandlerImp(R, collectionName), Type(type), Vertices(NULL), Normals(NULL)
{
  SetSelectable(false);
  GlM.SetFlags(sglmAmbientF);
  GlM.AmbientF = 0;
  GlM.SetIdentityDraw(false);
  GlM.SetTransparent(false);
}
//.............................................................................
TDUserObj::TDUserObj(TGlRenderer& R, const TDataItem &di)
  : AGlMouseHandlerImp(R, EmptyString()),
    Vertices(NULL),
    Normals(NULL)
{
  FromDataItem(di);
}
//.............................................................................
void TDUserObj::Create(const olxstr& cName)  {
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);
  olxstr NewL;
  TGPCollection* GPC = Parent.FindCollectionX(GetCollectionName(), NewL);
  if( GPC == NULL )
    GPC = &Parent.NewCollection(NewL);
  GPC->AddObject(*this);
  if( GPC->PrimitiveCount() != 0 )  return;

  TGraphicsStyle& GS = GPC->GetStyle();
  TGlPrimitive& GlP = GPC->NewPrimitive("Object", Type);
  GlP.SetProperties(GS.GetMaterial("Object", GlM));
  if( Type == sgloSphere )  {
    if( Vertices != NULL && Vertices->Count() == 3 )  {
      Params().Resize(16);
      for( int i=0; i < 3; i++ )  {
        for( int j=0; j < 3; j++ )  {
            Params()[i*4+j] = (*Vertices)[i][j];
        }
      }
      Params()[15] = 1;
    }
    GlP.Params[0] = 1;  
    GlP.Params[1] = 25;  
    GlP.Params[2] = 25;
    GlP.Compile();
  }
  else  {
    if( Vertices != NULL )
      GlP.Vertices = *Vertices;
    if( Normals != NULL )
      GlP.Normals = *Normals;
  }
}
//.............................................................................
bool TDUserObj::Orient(TGlPrimitive& P)  {
  olx_gl::translate(Basis.GetCenter());
  olx_gl::scale(Basis.GetZoom());
  if( Type == sgloSphere )  {
    if( Params().Count() == 16 )
      olx_gl::orient(Params().GetRawData());
    else if( Params().Count() == 1 )
      olx_gl::scale(Params()[0]);
  }
  else  {
    if( Type == sgloSphere && Vertices != NULL )  {
      for( size_t i=0; i < Vertices->Count(); i++ )  {
        olx_gl::translate((*Vertices)[i]);
        P.Draw();
      }
      return true;
    }    
  }
  return false;
}
//.............................................................................
void TDUserObj::ToDataItem(TDataItem &di) const {
  di.AddField("type", Type)
    .AddField("name", GetCollectionName())
    .AddField("flags", AGlMouseHandlerImp::glml_Flags);
  Basis.ToDataItem(di.AddItem("Basis"));
  if (Vertices != NULL)
    di.AddField("vertices", PersUtil::VecListToStr(*Vertices));
  if (Normals != NULL)
    di.AddField("normals", PersUtil::VecListToStr(*Normals));
}
//.............................................................................
void TDUserObj::FromDataItem(const TDataItem &di) {
  di.GetRequiredField("type").ToNumber(Type);
  di.GetRequiredField("flags").ToNumber(AGlMouseHandlerImp::glml_Flags);
  SetCollectionName(di.GetRequiredField("name"));
  Basis.FromDataItem(di.FindRequiredItem("Basis"));
  olxstr v = di.GetFieldValue("verices");
  if (!v.IsEmpty()) {
    Vertices = new vec3f_alist;
    PersUtil::VecListFromStr(v, *Vertices);
  }
  v = di.GetFieldValue("normals");
  if (!v.IsEmpty()) {
    Normals = new vec3f_alist;
    PersUtil::VecListFromStr(v, *Vertices);
  }
}
//.............................................................................
const_strlist TDUserObj::ToPov(olxdict<const TGlMaterial*, olxstr,
    TPointerComparator> &materials) const
{
  TStrList out;
  out.Add(" object { union {");
  const TGPCollection &gpc = GetPrimitives();
  pov::CrdTransformer crdc(Parent.GetBasis());
  for( size_t i=0; i < gpc.PrimitiveCount(); i++ )  {
    TGlPrimitive &glp = gpc.GetPrimitive(i);
    if( glp.GetType() == sgloSphere ) {
      olxstr p_mat = pov::get_mat_name(glp.GetProperties(), materials);
      out.Add("  object { sphere {<0,0,0>, 1 } texture {") << p_mat << "}}";
    }
  }
  out.Add("  }");
  if(Vertices != NULL && Vertices->Count() == 3) {
    mat3d m((*Vertices)[0], (*Vertices)[1], (*Vertices)[2]);
    out.Add("  transform {");
    out.Add("   matrix") << pov::to_str(crdc.matr(m),
      crdc.crd(Basis.GetCenter()));
    out.Add("   }");
  }
  else {
    out.Add("  scale ") << Basis.GetZoom();
    out.Add("  translate") << pov::to_str(crdc.crd(Basis.GetCenter()));
  }
  out.Add(" }");
  return out;
}
//.............................................................................
