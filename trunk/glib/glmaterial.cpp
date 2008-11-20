//---------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "glmaterial.h"
#include "dataitem.h"
#include "egc.h"

UseGlNamespace();
//..............................................................................
TGlOption GlobalGlFunction( BlackColor );
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
  FMark = false;
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
  FMark = G.FMark;
  return *this;
}
//..............................................................................
void TGlMaterial::Init() const  {
  if( Flags & sglmTransparent )  {
    //if( !glIsEnabled(GL_ALPHA_TEST) )  {
      glEnable(GL_ALPHA_TEST);
//      glEnable( GL_POINT_SMOOTH);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glEnable(GL_CULL_FACE);
    //}
  }
  else  {
    //if( glIsEnabled(GL_ALPHA_TEST) )  {
      glDisable(GL_ALPHA_TEST);
//      glDisable(GL_POINT_SMOOTH);
      glDisable(GL_BLEND);
      glDisable(GL_CULL_FACE);
    //}
  }
//  if( Flags & sglmLighting )  glEnable(GL_LIGHTING);
//  else                        glDisable(GL_LIGHTING);
  if( (Flags & sglmColorMat) == 0 )  {
    glDisable(GL_COLOR_MATERIAL);
    if( Flags & sglmDiffuseFB )  {
      if( Flags & sglmDiffuseF )    glMaterialfv(GL_FRONT, GL_DIFFUSE, DiffuseF.Data());
      if( Flags & sglmDiffuseB )    glMaterialfv(GL_BACK, GL_DIFFUSE, DiffuseB.Data());
    }
    else  {
      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, BlackColor.Data());
    }
    if( Flags & sglmAmbientFB )  {
      if( Flags & sglmAmbientF )    glMaterialfv(GL_FRONT, GL_AMBIENT, AmbientF.Data());
      if( Flags & sglmAmbientB )    glMaterialfv(GL_BACK, GL_AMBIENT, AmbientB.Data());
    }
    else  {
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, BlackColor.Data());
    }
    if( Flags & sglmSpecularFB )  {
      if( Flags & sglmSpecularF )    glMaterialfv(GL_FRONT, GL_SPECULAR, SpecularF.Data());
      if( Flags & sglmSpecularB )    glMaterialfv(GL_BACK, GL_SPECULAR, SpecularB.Data());
    }
    else  {
      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, BlackColor.Data());
    }
    if( Flags & sglmEmissionFB )  {
      if( Flags & sglmEmissionF )    glMaterialfv(GL_FRONT, GL_EMISSION, EmissionF.Data());
      if( Flags & sglmEmissionB )    glMaterialfv(GL_BACK, GL_EMISSION, EmissionB.Data());
    }
    else  {
      glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, BlackColor.Data());
    }
    if( Flags & sglmShininessFB )  {
      if( Flags & sglmShininessF )    glMateriali(GL_FRONT, GL_SHININESS, ShininessF);
      if( Flags & sglmShininessB )    glMateriali(GL_BACK, GL_SHININESS, ShininessB);
    }
    else  {
      glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, 0);
    }
  }
  else  {
    glEnable(GL_COLOR_MATERIAL);
    if( Flags & sglmAmbientF )
    {  glColorMaterial(GL_FRONT, GL_AMBIENT);  glColor4fv(AmbientF.Data());  }
    if( Flags & sglmAmbientB )  glColorMaterial(GL_BACK, GL_AMBIENT);
    {  glColorMaterial(GL_BACK, GL_AMBIENT);   glColor4fv(AmbientB.Data());  }
    if( Flags & sglmDiffuseF )
    {  glColorMaterial(GL_FRONT, GL_DIFFUSE);  glColor4fv(DiffuseF.Data());  }
    if( Flags & sglmDiffuseB )
    {  glColorMaterial(GL_BACK, GL_DIFFUSE);   glColor4fv(DiffuseB.Data());  }
    if( Flags & sglmEmissionF )
    {  glColorMaterial(GL_FRONT, GL_EMISSION); glColor4fv(EmissionF.Data());  }
    if( Flags & sglmEmissionB )
    {  glColorMaterial(GL_BACK, GL_EMISSION);  glColor4fv(EmissionB.Data());  }
    if( Flags & sglmSpecularF )
    {  glColorMaterial(GL_FRONT, GL_SPECULAR); glColor4fv(SpecularF.Data());  }
    if( Flags & sglmEmissionB )
    {  glColorMaterial(GL_BACK, GL_SPECULAR);  glColor4fv(SpecularB.Data());  }

    if( Flags & sglmShininessF )    glMateriali(GL_FRONT, GL_SHININESS, ShininessF);
    if( Flags & sglmShininessB )    glMateriali(GL_BACK, GL_SHININESS, ShininessB);
  }
}
//..............................................................................
const TGlMaterial& TGlMaterial::Intensity(TGlOption& ClearColor, double intensity) const
{
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
void TGlMaterial::ToDataItem(TDataItem& Item)  {
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
TIString TGlMaterial::ToString() const  {
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
void TGlMaterial::FromString( const olxstr& str )  {
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

