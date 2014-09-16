/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx2_app_H
#define __olx2_app_H
#include "gxapp.h"

namespace olex2 {

class AOlex2App : public TGXApp {
public:
  AOlex2App(const olxstr& FileName, AGlScene *scene=NULL)
    : TGXApp(FileName, scene)
  {}
  // throws an exception
  virtual TStrList GetPluginList() const = 0;
  virtual olxstr TranslateString(const olxstr& str) const = 0;

  static bool HasInstance() {
    try {
      TGXApp& app = TGXApp::GetInstance();
      return dynamic_cast<AOlex2App*>(&app) != NULL;
    }
    catch(...) { return false; }
  }
  static AOlex2App &GetInstance() {
    TGXApp& app = TGXApp::GetInstance();
    AOlex2App* olx2app = dynamic_cast<AOlex2App*>(&app);
    if (olx2app == NULL) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "unsuitable application instance");
    }
    return *olx2app;
  }
};

} // end namesapce olex
#endif
