//---------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "gllightmodel.h"
#include "dataitem.h"

UseGlNamespace();
//..............................................................................
//..............................................................................
TGlLightModel::TGlLightModel()  {
  FAmbient = 0x7f7f7f;
  for( int i = 0; i < 8; i++ )
    FLights[i].Index( GL_LIGHT0 + i );
  FClearColor = 0x7f7f7f7f;
  FLights[0].Enabled( true );
  FLights[0].SpotCutoff(180);
  FLights[0].Ambient().Clear();
  FLights[0].Diffuse() = 0x7f7f7f;
  FLights[0].Specular() = 0x7f7f7f;
  FLights[0].Position().Clear();
  FLights[0].Position()[2] = 100;
  FLights[0].Position()[3] = 1;
  FLights[0].Attenuation().Clear();
  FLights[0].Attenuation()[0] = 1;
  FFlags = 0;
  SmoothShade(true);
  TwoSide(true);
}
//..............................................................................
TGlLightModel& TGlLightModel::operator = (TGlLightModel& M)  {
  for( int i=0; i < 8; i++ )
    Light(i) = M.Light(i);

  ClearColor() = M.ClearColor();
  AmbientColor() = M.AmbientColor();
  FFlags = M.Flags();
  return *this;
}
//..............................................................................
void TGlLightModel::Init()  {
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, FAmbient.Data());
  if( SmoothShade() )  glShadeModel(GL_SMOOTH);
  else                 glShadeModel(GL_FLAT);
  if( LocalViewer() )  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);
  else                 glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 0);
  if( TwoSide() )      glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
  else                 glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);
  glClearColor(  FClearColor[0], FClearColor[1], FClearColor[2], FClearColor[3]);

  TGlOption GLO;
  for( int i=0; i<8; i++ )  {
    if( FLights[i].Enabled() )  {
      glLightfv(FLights[i].Index(), GL_AMBIENT, FLights[i].Ambient().Data());
      glLightfv(FLights[i].Index(), GL_DIFFUSE, FLights[i].Diffuse().Data());
      glLightfv(FLights[i].Index(), GL_SPECULAR, FLights[i].Specular().Data());
      GLO = FLights[i].Position();

      glLightfv(FLights[i].Index(), GL_POSITION, GLO.Data());
      glLighti(FLights[i].Index(), GL_SPOT_CUTOFF, FLights[i].SpotCutoff());

      glLightfv(FLights[i].Index(), GL_SPOT_DIRECTION, FLights[i].SpotDirection().Data());
      glLighti(FLights[i].Index(), GL_SPOT_EXPONENT, FLights[i].SpotExponent());

      glLightf(FLights[i].Index(), GL_CONSTANT_ATTENUATION, FLights[i].Attenuation()[0]);
      glLightf(FLights[i].Index(), GL_LINEAR_ATTENUATION, FLights[i].Attenuation()[1]);
      glLightf(FLights[i].Index(), GL_QUADRATIC_ATTENUATION, FLights[i].Attenuation()[2]);
      glEnable(FLights[i].Index());
    }
    else
    {
      glDisable(FLights[i].Index());
    }
  }
/*  glEnable(GL_POINT_SMOOTH);
  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_POLYGON_SMOOTH);
  glHint(GL_POINT_SMOOTH_HINT,GL_NICEST);
  glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
  glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST); */
}
//..............................................................................
bool TGlLightModel::FromDataItem(const TDataItem& Item)  {
  FFlags = Item.GetFieldValue("Flags").ToInt();
  FAmbient.FromString(Item.GetFieldValue("Ambient"));
  FClearColor.FromString(Item.GetFieldValue("ClearColor"));
  TDataItem *SI;
  for( int i=0; i < 8; i++ )  {
    SI = Item.FindItem( olxstr("Light") << i);
    if( SI == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "invalid OpenGL light description");
    FLights[i].FromDataItem(*SI);
  }
  return true;
}
//..............................................................................
void TGlLightModel::ToDataItem(TDataItem& Item) const {
  Item.AddField("Flags", Flags());
  Item.AddField("Ambient", FAmbient.ToString());
  Item.AddField("ClearColor", FClearColor.ToString());
  for( int i=0; i < 8; i++ )
    FLights[i].ToDataItem(Item.AddItem( olxstr("Light") << i ));
}
 
