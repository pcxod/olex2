/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "gllight.h"
#include "dataitem.h"
UseGlNamespace();

TGlLight::TGlLight() : OnLibChange(actions.New("ONCHANGE")) {
  SpotCutoff = 180;
  Position[2] = 1;
  Attenuation[0] = 1;
  Specular = 0x0f0f0f0f;
  Enabled = false;
}
//..............................................................................
TGlLight& TGlLight::operator = (const TGlLight &S )  {
  Ambient       = S.GetAmbient();
  Diffuse       = S.GetDiffuse();
  Specular      = S.GetSpecular();
  Position      = S.GetPosition();
  SpotDirection = S.GetSpotDirection();
  SpotCutoff    = S.GetSpotCutoff();
  SpotExponent  = S.GetSpotExponent();
  Attenuation   = S.GetAttenuation();
  Enabled       = S.IsEnabled();
  return *this;
}
//..............................................................................
bool TGlLight::FromDataItem(const TDataItem& Item) {
  Ambient.FromString(Item.FindField("Ambient"));
  Diffuse.FromString(Item.FindField("Diffuse"));
  Specular.FromString(Item.FindField("Specular"));
  Position.FromString(Item.FindField("Position"));
  Attenuation.FromString(Item.FindField("Attenuation"));
  SpotDirection.FromString(Item.FindField("SpotDirection"));
  SpotCutoff = Item.FindField("SpotCutoff", "180").ToInt();
  SpotExponent = Item.FindField("SpotExponent", "0").ToInt();
  Enabled = Item.FindField("Enabled").ToBool();
  return true;
}
//..............................................................................
void TGlLight::ToDataItem(TDataItem& Item) const {
  Item.AddField("Ambient", Ambient.ToString());
  Item.AddField("Diffuse", Diffuse.ToString());
  Item.AddField("Specular", Specular.ToString());
  Item.AddField("Position", Position.ToString());
  Item.AddField("Attenuation", Attenuation.ToString());
  Item.AddField("SpotDirection", SpotDirection.ToString());
  Item.AddField("SpotCutoff", SpotCutoff);
  Item.AddField("SpotExponent", SpotExponent);
  Item.AddField("Enabled", Enabled);
}
//..............................................................................
void TGlLight::LibEnabled(const TStrObjList& Params, TMacroData& E)  {
  if( Params.IsEmpty() )  E.SetRetVal(Enabled);
  else  {
    const bool v = Enabled;
    Enabled = Params[0].ToBool();
    if( v != Enabled )
      OnLibChange.Execute(this);
  }
}
//..............................................................................
void TGlLight::LibSpotCutoff(const TStrObjList& Params, TMacroData& E)  {
  if( Params.IsEmpty() )  E.SetRetVal(SpotCutoff);
  else  {
    const short v = SpotCutoff;
    SpotCutoff = Params[0].ToInt();
    if( v != SpotCutoff )
      OnLibChange.Execute(this);
  }
}
//..............................................................................
void TGlLight::LibSpotExponent(const TStrObjList& Params, TMacroData& E)  {
  if( Params.IsEmpty() )  E.SetRetVal(SpotExponent);
  else  {
    const short v = SpotExponent;
    SpotExponent = Params[0].ToInt();
    if( v != SpotExponent )
      OnLibChange.Execute(this);
  }
}
//..............................................................................
void TGlLight::LibAmbient(const TStrObjList& Params, TMacroData& E)  {
  if( Params.IsEmpty() )  E.SetRetVal(Ambient.ToString());
  else  {
    const TGlOption v = Ambient;
    Ambient.FromString(Params[0]);
    if( v != Ambient )
      OnLibChange.Execute(this);
  }
}
//..............................................................................
void TGlLight::LibDiffuse(const TStrObjList& Params, TMacroData& E)  {
  if( Params.IsEmpty() )  E.SetRetVal(Diffuse.ToString());
  else  {
    const TGlOption v = Diffuse;
    Diffuse.FromString(Params[0]);
    if( v != Diffuse )
      OnLibChange.Execute(this);
  }
}
//..............................................................................
void TGlLight::LibSpecular(const TStrObjList& Params, TMacroData& E)  {
  if( Params.IsEmpty() )  E.SetRetVal(Specular.ToString());
  else  {
    const TGlOption v = Specular;
    Specular.FromString(Params[0]);
    if( v != Specular )
      OnLibChange.Execute(this);
  }
}
//..............................................................................
void TGlLight::LibPosition(const TStrObjList& Params, TMacroData& E)  {
  if( Params.IsEmpty() )  E.SetRetVal(Position.ToString());
  else  {
    const TGlOption v = Position;
    Position.FromString(Params[0]);
    if( v != Position )
      OnLibChange.Execute(this);
  }
}
//..............................................................................
void TGlLight::LibSpotDirection(const TStrObjList& Params, TMacroData& E)  {
  if( Params.IsEmpty() )  E.SetRetVal(SpotDirection.ToString());
  else  {
    const TGlOption v = SpotDirection;
    SpotDirection.FromString(Params[0]);
    if( v != SpotDirection )
      OnLibChange.Execute(this);
  }
}
//..............................................................................
void TGlLight::LibAttenuation(const TStrObjList& Params, TMacroData& E)  {
  if( Params.IsEmpty() )  E.SetRetVal(Attenuation.ToString());
  else  {
    const TGlOption v = Attenuation;
    Attenuation.FromString(Params[0]);
    if( v != Attenuation )
      OnLibChange.Execute(this);
  }
}
//..............................................................................
TLibrary* TGlLight::ExportLibrary(const olxstr& name)  {
  TLibrary* lib = new TLibrary(name);
  lib->Register(new TFunction<TGlLight>(this, &TGlLight::LibEnabled,
    "Enabled", fpNone|fpOne, "Returns/sets enabled property of the light") );
  lib->Register(new TFunction<TGlLight>(this, &TGlLight::LibSpotCutoff,
    "SpotCutoff", fpNone|fpOne, "Returns/sets spot cutoff property of the light") );
  lib->Register(new TFunction<TGlLight>(this, &TGlLight::LibSpotExponent,
    "SpotExponent", fpNone|fpOne, "Returns/sets enabled property of the light") );
  lib->Register(new TFunction<TGlLight>(this, &TGlLight::LibAmbient,
    "Ambient", fpNone|fpOne, "Returns/sets enabled property of the light") );
  lib->Register(new TFunction<TGlLight>(this, &TGlLight::LibDiffuse,
    "Diffuse", fpNone|fpOne, "Returns/sets enabled property of the light") );
  lib->Register(new TFunction<TGlLight>(this, &TGlLight::LibSpecular,
    "Specular", fpNone|fpOne, "Returns/sets enabled property of the light") );
  lib->Register(new TFunction<TGlLight>(this, &TGlLight::LibPosition,
    "Position", fpNone|fpOne, "Returns/sets enabled property of the light") );
  lib->Register(new TFunction<TGlLight>(this, &TGlLight::LibSpotDirection,
    "SpotDirection", fpNone|fpOne, "Returns/sets enabled property of the light") );
  lib->Register(new TFunction<TGlLight>(this, &TGlLight::LibAttenuation,
    "Attenuation", fpNone|fpOne, "Returns/sets enabled property of the light") );
  return lib;
}
//..............................................................................
