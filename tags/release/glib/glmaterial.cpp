//---------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//
#include "glmaterial.h"
#include "dataitem.h"
#include "egc.h"

UseGlNamespace();
//..............................................................................
const TGlOption GlobalGlFunction(BlackColor);
//..............................................................................
TGlMaterial::TGlMaterial()  {
  Flags = sglmShininessF | sglmSpecularF | sglmDiffuseF | sglmAmbientF;
  ShininessF = ShininessB = 36;
  SpecularF  = 0x80ffffff;
  DiffuseF   = 0x80777777;
  AmbientF   = 0x807f7f7f;
  SpecularB  = 0x80ffffff;
  DiffuseB   = 0x80777777;
  AmbientB   = 0x807f7f7f;
}
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
    if( Flags & sglmDiffuseFB )  {
      if( Flags & sglmDiffuseF )  olx_gl::material(GL_FRONT, GL_DIFFUSE, DiffuseF.Data());
      if( Flags & sglmDiffuseB )  olx_gl::material(GL_BACK, GL_DIFFUSE, DiffuseB.Data());
    }
    else  {
      olx_gl::material(GL_FRONT_AND_BACK, GL_DIFFUSE, BlackColor.Data());
    }
    if( Flags & sglmAmbientFB )  {
      if( Flags & sglmAmbientF )  olx_gl::material(GL_FRONT, GL_AMBIENT, AmbientF.Data());
      if( Flags & sglmAmbientB )  olx_gl::material(GL_BACK, GL_AMBIENT, AmbientB.Data());
    }
    else  {
      olx_gl::material(GL_FRONT_AND_BACK, GL_AMBIENT, BlackColor.Data());
    }
    if( Flags & sglmSpecularFB )  {
      if( Flags & sglmSpecularF )  olx_gl::material(GL_FRONT, GL_SPECULAR, SpecularF.Data());
      if( Flags & sglmSpecularB )  olx_gl::material(GL_BACK, GL_SPECULAR, SpecularB.Data());
    }
    else  {
      olx_gl::material(GL_FRONT_AND_BACK, GL_SPECULAR, BlackColor.Data());
    }
    if( Flags & sglmEmissionFB )  {
      if( Flags & sglmEmissionF )  olx_gl::material(GL_FRONT, GL_EMISSION, EmissionF.Data());
      if( Flags & sglmEmissionB )  olx_gl::material(GL_BACK, GL_EMISSION, EmissionB.Data());
    }
    else  {
      olx_gl::material(GL_FRONT_AND_BACK, GL_EMISSION, BlackColor.Data());
    }
    if( Flags & sglmShininessFB )  {
      if( Flags & sglmShininessF )  olx_gl::material(GL_FRONT, GL_SHININESS, ShininessF);
      if( Flags & sglmShininessB )  olx_gl::material(GL_BACK, GL_SHININESS, ShininessB);
    }
    else  {
      olx_gl::material(GL_FRONT_AND_BACK, GL_SHININESS, 0);
    }
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
    if( Flags & sglmEmissionB )  {
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
  Item.SetValue( ToString() );
//  Item->AddField("AmbientF", AmbientF.ToString());
//  Item->AddField("AmbientB", AmbientB.ToString());
//  Item->AddField("DiffuseF", DiffuseF.ToString());
//  Item->AddField("DiffuseB", DiffuseB.ToString());
//  Item->AddField("EmissionF", EmissionF.ToString());
//  Item->AddField("EmissionB", EmissionB.ToString());
//  Item->AddField("SpecularF", SpecularF.ToString());
//  Item->AddField("SpecularB", SpecularB.ToString());
//  Item->AddField("ShininessF", ShininessF);
//  Item->AddField("ShininessB", ShininessB);
//  Item->AddField("Flags", TEString( Flags) );
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
  olxstr str(EmptyString, 256);
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

