/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx2_app_imp_H
#define __olx2_app_imp_H
#include "olex2app.h"
#include "olxstate.h"

class Olex2App : public TGXApp, public olex2::AOlex2App {
  size_t statePluginInstalled;
  bool CheckState(size_t state, const olxstr& stateData) const {
    return state == statePluginInstalled ? plugins.Contains(stateData) : false;
  }
public:

  Olex2App(const olxstr& FileName, AGlScene *scene=0)
    : TGXApp(FileName, scene)
  {
    TStateRegistry &states = GetStatesRegistry();
    statePluginInstalled = TStateRegistry::GetInstance().Register(
      "pluginInstalled",
      new TStateRegistry::Slot(
        states.NewGetter<Olex2App>(this, &Olex2App::CheckState),
        new TStateRegistry::TMacroSetter("InstallPlugin")
      )
    );
  }

  void InitOlex2App() {
    olex2::AOlex2App::InitOlex2App();
    for (size_t i = 0; i < olex2::AOlex2App::plugins.Count(); i++) {
      TStateRegistry::GetInstance().SetState(
        statePluginInstalled, true, olex2::AOlex2App::plugins[i], true);
    }
  }

  virtual bool AddPlugin(const olxstr &pn) {
    if (olex2::AOlex2App::AddPlugin(pn)) {
      TStateRegistry::GetInstance().SetState(statePluginInstalled, true,
        pn, true);
      return true;
    }
    return false;
  }

  virtual bool RemovePlugin(const olxstr &pn) {
    if (olex2::AOlex2App::RemovePlugin(pn)) {
      TStateRegistry::GetInstance().SetState(statePluginInstalled, false,
        pn, true);
      return true;
    }
    return false;
  }

  static Olex2App &GetInstance() {
    AOlex2App* app = olex2::AOlex2App::GetInstance_();
    Olex2App* olx2app = dynamic_cast<Olex2App*>(app);
    if (olx2app == 0) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "unsuitable application instance");
    }
    return *olx2app;
  }
};

#endif
