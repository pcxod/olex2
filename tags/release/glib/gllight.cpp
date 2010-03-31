//---------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "gllight.h"
#include "dataitem.h"

UseGlNamespace();
//..............................................................................
//..............................................................................
TGlLight::TGlLight()  {
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
  Ambient.FromString(Item.GetFieldValue("Ambient"));
  Diffuse.FromString(Item.GetFieldValue("Diffuse"));
  Specular.FromString(Item.GetFieldValue("Specular"));
  Position.FromString(Item.GetFieldValue("Position"));
  Attenuation.FromString(Item.GetFieldValue("Attenuation"));
  SpotDirection.FromString(Item.GetFieldValue("SpotDirection"));
  SpotCutoff = Item.GetFieldValue("SpotCutoff", "180").ToInt();
  Enabled = Item.GetFieldValue("Enabled").ToBool();
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
  Item.AddField("Enabled", Enabled);
}

