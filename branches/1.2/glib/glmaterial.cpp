/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "glmaterial.h"
#include "dataitem.h"
#include "estrbuffer.h"

const TGlOption GlobalGlFunction(BlackColor);
//..............................................................................
bool TGlMaterial::operator == (const AGOProperties &G) const  {
  TGlMaterial *GlM = (TGlMaterial*)&G;
  return this->operator == (*GlM);
}
//..............................................................................
bool TGlMaterial::operator == (const TGlMaterial &C) const  {
  if( Flags != C.Flags )              return false;

  if( !( C.EmissionF == EmissionF))   return false;
  if( !( C.ShininessF == ShininessF)) return false;
  if( !( C.SpecularF == SpecularF))   return false;
  if( !( C.EmissionB == EmissionB))   return false;
  if( !( C.ShininessB == ShininessB)) return false;
  if( !( C.SpecularB == SpecularB))   return false;
  if( !( C.AmbientF == AmbientF))     return false;
  if( !( C.DiffuseF == DiffuseF))     return false;
  if( !( C.AmbientB == AmbientB))     return false;
  if( !( C.DiffuseB == DiffuseB))     return false;

  return true;
}
//..............................................................................
int TGlMaterial::Compare(const TGlMaterial &m) const {
  int v = olx_cmp(Flags, m.Flags);
  if (v != 0) return v;
  if (HasAmbientF() && (v=AmbientF.Compare(m.AmbientF)) != 0) return v;
  if (HasAmbientB() && (v=AmbientB.Compare(m.AmbientB)) != 0) return v;
  if (HasDiffuseF() && (v=DiffuseF.Compare(m.DiffuseF)) != 0) return v;
  if (HasDiffuseB() && (v=DiffuseB.Compare(m.DiffuseB)) != 0) return v;
  if (HasSpecularF() && (v=SpecularF.Compare(m.SpecularF)) != 0) return v;
  if (HasSpecularB() && (v=SpecularB.Compare(m.SpecularB)) != 0) return v;
  if (HasShininessF() && (v=olx_cmp(ShininessF, m.ShininessF)) != 0) return v;
  if (HasShininessB() && (v=olx_cmp(ShininessB, m.ShininessB)) != 0) return v;
  if (HasEmissionF() && (v=EmissionF.Compare(m.EmissionF)) != 0) return v;
  if (HasEmissionB() && (v=EmissionB.Compare(m.EmissionB)) != 0) return v;
  return 0;
}
//..............................................................................
AGOProperties& TGlMaterial::operator = (const AGOProperties& G)  {
  TGlMaterial *GlM = (TGlMaterial*)&G;
  this->operator = (*GlM);
  return *this;
}
//..............................................................................
TGlMaterial& TGlMaterial::operator = (const TGlMaterial& G)  {
  EmissionF    = G.EmissionF;
  ShininessF   = G.ShininessF;
  SpecularF    = G.SpecularF;
  EmissionB    = G.EmissionB;
  ShininessB   = G.ShininessB;
  SpecularB    = G.SpecularB;

  AmbientF     = G.AmbientF;
  AmbientB     = G.AmbientB;
  DiffuseF     = G.DiffuseF;
  DiffuseB     = G.DiffuseB;
  Flags = G.Flags;
  return *this;
}
//..............................................................................
void TGlMaterial::Init(bool skip) const  {
  if( skip )  return;  
  if( Flags & sglmTransparent )  {
    //if( !glIsEnabled(GL_ALPHA_TEST) )  {
      olx_gl::enable(GL_ALPHA_TEST);
//      olx_gl::enable( GL_POINT_SMOOTH);
      olx_gl::enable(GL_BLEND);
      olx_gl::blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      olx_gl::enable(GL_CULL_FACE);
    //}
  }
  else  {
    //if( glIsEnabled(GL_ALPHA_TEST) )  {
      olx_gl::disable(GL_ALPHA_TEST);
//      olx_gl::disable(GL_POINT_SMOOTH);
      olx_gl::disable(GL_BLEND);
      olx_gl::disable(GL_CULL_FACE);
    //}
  }
//  if( Flags & sglmLighting )  olx_gl::enable(GL_LIGHTING);
//  else                        olx_gl::disable(GL_LIGHTING);
  if( (Flags & sglmColorMat) == 0 )  {
    olx_gl::disable(GL_COLOR_MATERIAL);
    olx_gl::material(GL_FRONT, GL_DIFFUSE, ((Flags&sglmDiffuseF) != 0 ? DiffuseF : BlackColor).Data());
    olx_gl::material(GL_BACK, GL_DIFFUSE, ((Flags&sglmDiffuseB) != 0 ? DiffuseB : BlackColor).Data());
    olx_gl::material(GL_FRONT, GL_AMBIENT, ((Flags&sglmAmbientF) != 0 ? AmbientF : BlackColor).Data());
    olx_gl::material(GL_BACK, GL_AMBIENT, ((Flags&sglmAmbientB) != 0 ? AmbientB : BlackColor).Data());
    olx_gl::material(GL_FRONT, GL_SPECULAR, ((Flags&sglmSpecularF) != 0 ? SpecularF : BlackColor).Data());
    olx_gl::material(GL_BACK, GL_SPECULAR, ((Flags&sglmSpecularB) != 0 ? SpecularB : BlackColor).Data());
    olx_gl::material(GL_FRONT, GL_EMISSION, ((Flags&sglmEmissionF) != 0 ? EmissionF : BlackColor).Data());
    olx_gl::material(GL_BACK, GL_EMISSION, ((Flags&sglmEmissionB) != 0 ? EmissionB : BlackColor).Data());
    olx_gl::material(GL_FRONT, GL_SHININESS, ((Flags&sglmShininessF) != 0 ? ShininessF : 0));
    olx_gl::material(GL_BACK, GL_SHININESS, ((Flags&sglmShininessB) != 0 ? ShininessB : 0));
  }
  else  {
    olx_gl::enable(GL_COLOR_MATERIAL);
    if( Flags & sglmAmbientF )  {
      olx_gl::colorMaterial(GL_FRONT, GL_AMBIENT);
      olx_gl::color(AmbientF.Data());
    }
    if( Flags & sglmAmbientB )  {
      olx_gl::colorMaterial(GL_BACK, GL_AMBIENT);
      olx_gl::color(AmbientB.Data());
    }
    if( Flags & sglmDiffuseF )  {
      olx_gl::colorMaterial(GL_FRONT, GL_DIFFUSE);
      olx_gl::color(DiffuseF.Data());
    }
    if( Flags & sglmDiffuseB )  {
      olx_gl::colorMaterial(GL_BACK, GL_DIFFUSE);
      olx_gl::color(DiffuseB.Data());
    }
    if( Flags & sglmEmissionF )  {
      olx_gl::colorMaterial(GL_FRONT, GL_EMISSION);
      olx_gl::color(EmissionF.Data());
    }
    if( Flags & sglmEmissionB )  {
      olx_gl::colorMaterial(GL_BACK, GL_EMISSION);
      olx_gl::color(EmissionB.Data());
    }
    if( Flags & sglmSpecularF )  {
      olx_gl::colorMaterial(GL_FRONT, GL_SPECULAR);
      olx_gl::color(SpecularF.Data());
    }
    if( Flags & sglmSpecularB )  {
      olx_gl::colorMaterial(GL_BACK, GL_SPECULAR);
      olx_gl::color(SpecularB.Data());
    }
    if( Flags & sglmShininessF )
      olx_gl::material(GL_FRONT, GL_SHININESS, ShininessF);
    if( Flags & sglmShininessB )
      olx_gl::material(GL_BACK, GL_SHININESS, ShininessB);
  }
}
//..............................................................................
const TGlMaterial& TGlMaterial::Intensity(TGlOption& ClearColor, double intensity) const  {
  static TGlMaterial GlM;
  GlM.AmbientF = (AmbientF.operator*(intensity));
  GlM.AmbientB = (AmbientB.operator*(intensity));
  GlM.DiffuseF = ClearColor.operator+(DiffuseF.operator*(intensity));
  GlM.DiffuseB = ClearColor.operator+(DiffuseB.operator*(intensity));
  GlM.EmissionF = (EmissionF.operator*(intensity));
  GlM.EmissionB = (EmissionB.operator*(intensity));
  GlM.SpecularF = (SpecularF.operator*(intensity));
  GlM.SpecularB = (SpecularB.operator*(intensity));
  GlM.ShininessF *= (short)intensity;
  GlM.ShininessB *= (short)intensity;
  return GlM;
}
//..............................................................................
void TGlMaterial::ToDataItem(TDataItem& Item) const {
  Item.SetValue(ToString());
}
//..............................................................................
bool TGlMaterial::FromDataItem(const TDataItem& Item)  {
  if( Item.FieldCount() != 0 )  {  // backwards compatibility
    AmbientF.FromString(Item.GetFieldValue("AmbientF"));
    AmbientB.FromString(Item.GetFieldValue("AmbientB"));
    DiffuseF.FromString(Item.GetFieldValue("DiffuseF"));
    DiffuseB.FromString(Item.GetFieldValue("DiffuseB"));
    EmissionF.FromString(Item.GetFieldValue("EmissionF"));
    EmissionB.FromString(Item.GetFieldValue("EmissionB"));
    SpecularF.FromString(Item.GetFieldValue("SpecularF"));
    SpecularB.FromString(Item.GetFieldValue("SpecularB"));
    ShininessF = Item.GetFieldValue("ShininessF").ToInt();
    ShininessB = Item.GetFieldValue("ShininessB").ToInt();
    Flags = Item.GetFieldValue("Flags").ToInt();
  }
  else
    FromString( Item.GetValue() );
  return true;
}
//..............................................................................
TIString TGlMaterial::ToString() const {
  olxstr str(EmptyString(), 256);
  str << Flags;
  if( (Flags & sglmAmbientF) != 0 )
    str << ";" << AmbientF.ToString();
  if( (Flags & sglmAmbientB) != 0 )
    str << ";" << AmbientB.ToString();
  if( (Flags & sglmDiffuseF) != 0 )
    str << ";" << DiffuseF.ToString();
  if( (Flags & sglmDiffuseB) != 0 )
    str << ";" << DiffuseB.ToString();
  if( (Flags & sglmSpecularF) != 0 )
    str << ";" << SpecularF.ToString();
  if( (Flags & sglmSpecularB) != 0 )
    str << ";" << SpecularB.ToString();
  if( (Flags & sglmShininessF) != 0 )
    str << ";" << ShininessF;
  if( (Flags & sglmShininessB) != 0 )
    str << ";" << ShininessB;
  return str;
}
//..............................................................................
void TGlMaterial::FromString(const olxstr& str)  {
  if( str.IsEmpty() )
    throw TInvalidArgumentException(__OlxSourceInfo, "representation");
  TStrList toks( str, ';');
  Flags = toks[0].ToInt();
  int ind = 1;
  if( (Flags & sglmAmbientF) != 0 )
    AmbientF.FromString( toks[ind++] );
  if( (Flags & sglmAmbientB) != 0 )
    AmbientB.FromString( toks[ind++] );
  if( (Flags & sglmDiffuseF) != 0 )
    DiffuseF.FromString( toks[ind++] );
  if( (Flags & sglmDiffuseB) != 0 )
    DiffuseB.FromString( toks[ind++] );
  if( (Flags & sglmSpecularF) != 0 )
    SpecularF.FromString( toks[ind++] );
  if( (Flags & sglmSpecularB) != 0 )
    SpecularB.FromString( toks[ind++] );
  if( (Flags & sglmShininessF) != 0 )
    ShininessF = toks[ind++].ToInt();
  if( (Flags & sglmShininessB) != 0 )
    ShininessB = toks[ind++].ToInt();
}
//..............................................................................
olxstr TGlMaterial::ToPOV() const {
  TEStrBuffer bf;
  bf << olxT("texture {\n");
  if( (Flags & sglmAmbientF) != 0 )  {
    bf << olxT("  pigment { color rgb<");
    bf << olxstr(AmbientF[0]) << olxT(',') << olxstr(AmbientF[1])
      << olxT(',') << olxstr(AmbientF[2]) << olxT('>');
    if( (Flags & sglmDiffuseF) != 0 && (Flags & sglmTransparent) != 0 && DiffuseF[3] != 1 )
      bf << olxT(" filter ") << olxstr(DiffuseF[3]);
    bf << olxT("}\n");
  }
  bf << olxT("  finish {\n");
  if( (Flags & sglmDiffuseF) != 0 )  {
    bf << olxT("    diffuse ") << olxstr(DiffuseF.GetMean()) << olxT('\n');
  }
  if( (Flags & sglmSpecularF) != 0 )  {
    //if( !SpecularF.IsGrey() )
    //  bf << olxT("  metallic\n");
    const float k = DiffuseF.IsGrey() ? 1 : DiffuseF.GetMean();
    bf << olxT("    specular ") << olxstr(SpecularF.GetMean()*k) << olxT('\n');
  }
  if( (Flags & sglmShininessF) != 0 )  {
    bf << olxT("    roughness ") << olxstr(1./(double(ShininessF)+0.00001))
      << olxT('\n');
  }
  bf << olxT("}}\n");
  return bf.ToString();
}
//..............................................................................
