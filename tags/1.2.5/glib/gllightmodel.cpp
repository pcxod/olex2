/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "gllightmodel.h"
#include "dataitem.h"

TGlLightModel::TGlLightModel()  {
  AmbientColor = 0x7f7f7f;
  for( int i = 0; i < 8; i++ )
    Lights[i].SetIndex(GL_LIGHT0 + i);
  ClearColor = 0x7f7f7f7f;
  Lights[0].SetEnabled(true);
  Lights[0].SetSpotCutoff(180);
  Lights[0].SetAmbient(0);
  Lights[0].SetDiffuse(0x7f7f7f);
  Lights[0].SetSpecular(0x7f7f7f);
  Lights[0].SetPosition(TGlOption(0, 0, 100, 1));
  Lights[0].SetAttenuation(TGlOption(1,0,0,0));
  for( int i=0; i < 8; i++ )
    Lights[i].OnLibChange.Add(this);
  Flags = 0;
  SetSmoothShade(true);
  SetTwoSides(true);
  AActionHandler::SetToDelete(false);
}
//..............................................................................
TGlLightModel& TGlLightModel::operator = (TGlLightModel& M)  {
  for( int i=0; i < 8; i++ )
    Lights[i] = M.GetLight(i);
  ClearColor = M.ClearColor;
  AmbientColor = M.AmbientColor;
  Flags = M.Flags;
  return *this;
}
//..............................................................................
void TGlLightModel::Init()  {
  olx_gl::lightModel(GL_LIGHT_MODEL_AMBIENT, AmbientColor.Data());
  olx_gl::shadeModel(IsSmoothShade() ? GL_SMOOTH : GL_FLAT);
  olx_gl::lightModel(GL_LIGHT_MODEL_LOCAL_VIEWER, IsLocalViewer() ? 1 : 0);
  olx_gl::lightModel(GL_LIGHT_MODEL_TWO_SIDE, IsTwoSides() ? 1 : 0);
  olx_gl::clearColor(ClearColor.Data());
  for( int i=0; i<8; i++ )  {
    if( Lights[i].IsEnabled() )  {
      olx_gl::light(Lights[i].GetIndex(), GL_AMBIENT, Lights[i].GetAmbient().Data());
      olx_gl::light(Lights[i].GetIndex(), GL_DIFFUSE, Lights[i].GetDiffuse().Data());
      olx_gl::light(Lights[i].GetIndex(), GL_SPECULAR, Lights[i].GetSpecular().Data());

      olx_gl::light(Lights[i].GetIndex(), GL_POSITION, Lights[i].GetPosition().Data());
      olx_gl::light(Lights[i].GetIndex(), GL_SPOT_CUTOFF, Lights[i].GetSpotCutoff());

      olx_gl::light(Lights[i].GetIndex(), GL_SPOT_DIRECTION, Lights[i].GetSpotDirection().Data());
      olx_gl::light(Lights[i].GetIndex(), GL_SPOT_EXPONENT, Lights[i].GetSpotExponent());

      olx_gl::light(Lights[i].GetIndex(), GL_CONSTANT_ATTENUATION, Lights[i].GetAttenuation()[0]);
      olx_gl::light(Lights[i].GetIndex(), GL_LINEAR_ATTENUATION, Lights[i].GetAttenuation()[1]);
      olx_gl::light(Lights[i].GetIndex(), GL_QUADRATIC_ATTENUATION, Lights[i].GetAttenuation()[2]);
      olx_gl::enable(Lights[i].GetIndex());
    }
    else  {
      olx_gl::disable(Lights[i].GetIndex());
    }
  }
}
//..............................................................................
bool TGlLightModel::FromDataItem(const TDataItem& Item)  {
  Flags = Item.FindField("Flags").ToInt();
  AmbientColor.FromString(Item.FindField("Ambient"));
  ClearColor.FromString(Item.FindField("ClearColor"));
  for( int i=0; i < 8; i++ )  {
    TDataItem &SI = Item.GetItemByName(olxstr("Light") << i);
    Lights[i].FromDataItem(SI);
  }
  return true;
}
//..............................................................................
void TGlLightModel::ToDataItem(TDataItem& Item) const {
  Item.AddField("Flags", (int)Flags);  // need the cast or string will consider it as a char...
  Item.AddField("Ambient", AmbientColor.ToString());
  Item.AddField("ClearColor", ClearColor.ToString());
  for( int i=0; i < 8; i++ )
    Lights[i].ToDataItem(Item.AddItem(olxstr("Light") << i ));
}
//..............................................................................
void TGlLightModel::LibClearColor(const TStrObjList& Params, TMacroError& E)  {
  if( Params.IsEmpty() )  E.SetRetVal(ClearColor.ToString());
  else  {
    const TGlOption v = ClearColor;
    ClearColor.FromString(Params[0]);
    if( v != ClearColor )
      Init();
  }
}
//..............................................................................
void TGlLightModel::LibAmbientColor(const TStrObjList& Params, TMacroError& E)  {
  if( Params.IsEmpty() )  E.SetRetVal(AmbientColor.ToString());
  else  {
    const TGlOption v = AmbientColor;
    AmbientColor.FromString(Params[0]);
    if( v != AmbientColor )
      Init();
  }
}
//..............................................................................
void TGlLightModel::LibLocalViewer(const TStrObjList& Params, TMacroError& E)  {
  if( Params.IsEmpty() )  E.SetRetVal(IsLocalViewer());
  else  {
    const bool v = IsLocalViewer();
    SetLocalViewer(Params[0].ToBool());
    if( v != IsLocalViewer() )
      Init();
  }
}
//..............................................................................
void TGlLightModel::LibSmoothShade(const TStrObjList& Params, TMacroError& E)  {
  if( Params.IsEmpty() )  E.SetRetVal(IsSmoothShade());
  else  {
    const bool v = IsSmoothShade();
    SetSmoothShade(Params[0].ToBool());
    if( v != IsSmoothShade() )
      Init();
  }
}
//..............................................................................
void TGlLightModel::LibTwoSides(const TStrObjList& Params, TMacroError& E)  {
  if( Params.IsEmpty() )  E.SetRetVal(IsTwoSides());
  else  {
    const bool v = IsTwoSides();
    SetTwoSides(Params[0].ToBool());
    if( v != IsTwoSides() )
      Init();
  }
}
//..............................................................................
TLibrary* TGlLightModel::ExportLibrary(const olxstr& name)  {
  TLibrary* lib = new TLibrary( name.IsEmpty() ? olxstr("glm") : name);
  lib->Register(new TFunction<TGlLightModel>(this, &TGlLightModel::LibClearColor,
    "ClearColor", fpNone|fpOne, "Returns/sets background color of the model") );
  lib->Register(new TFunction<TGlLightModel>(this, &TGlLightModel::LibAmbientColor,
    "AmbientColor", fpNone|fpOne, "Returns/sets ambient color of the model") );
  lib->Register(new TFunction<TGlLightModel>(this, &TGlLightModel::LibLocalViewer,
    "LocalViewer", fpNone|fpOne, "Returns/sets local viewer property of the model") );
  lib->Register(new TFunction<TGlLightModel>(this, &TGlLightModel::LibSmoothShade,
    "SmoothShade", fpNone|fpOne, "Returns/sets smooth shading of the model") );
  lib->Register(new TFunction<TGlLightModel>(this, &TGlLightModel::LibTwoSides,
    "TwoSides", fpNone|fpOne, "Returns/sets two sides coloring of the model") );
  for( int i=0; i < 8; i++ )
    lib->AttachLibrary(Lights[i].ExportLibrary(olxstr("light") << (i+1)));
  return lib;
}
//..............................................................................
