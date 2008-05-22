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
  FSpotCutoff = 180;
  FPosition[2] = 1;
  FAttenuation[0] = 1;
  FSpecular = 0x0f0f0f0f;
  FEnabled = false;
}
//..............................................................................
void  TGlLight::operator = (const TGlLight &S )  {
  FAmbient       = S.GetAmbient();
  FDiffuse       = S.GetDiffuse();
  FSpecular      = S.GetSpecular();
  FPosition      = S.GetPosition();
  FSpotDirection = S.GetSpotDirection();
  FSpotCutoff    = S.SpotCutoff();
  FSpotExponent  = S.SpotExponent();
  FAttenuation   = S.GetAttenuation();
  FEnabled       = S.Enabled();
}
//..............................................................................
bool TGlLight::FromDataItem(TDataItem *Item)  {
  FAmbient.FromString(Item->GetFieldValue("Ambient"));
  FDiffuse.FromString(Item->GetFieldValue("Diffuse"));
  FSpecular.FromString(Item->GetFieldValue("Specular"));
  FPosition.FromString(Item->GetFieldValue("Position"));
  FAttenuation.FromString(Item->GetFieldValue("Attenuation"));
  FSpotDirection.FromString(Item->GetFieldValue("SpotDirection"));
  FSpotCutoff = Item->GetFieldValue("SpotCutoff", "180").ToInt();
  FEnabled = Item->GetFieldValue("Enabled").ToBool();
  return true;
}
//..............................................................................
void TGlLight::ToDataItem(TDataItem *Item)  {
  Item->AddField("Ambient", FAmbient.ToString());
  Item->AddField("Diffuse", FDiffuse.ToString());
  Item->AddField("Specular", FSpecular.ToString());
  Item->AddField("Position", FPosition.ToString());
  Item->AddField("Attenuation", FAttenuation.ToString());
  Item->AddField("SpotDirection", FSpotDirection.ToString());
  Item->AddField("SpotCutoff", FSpotCutoff);
  Item->AddField("Enabled", FEnabled);
}

