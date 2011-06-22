/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gxl_oxmFile_H
#define __olx_gxl_oxmFile_H
#include "gxapp.h"
BeginGxlNamespace()

// Olex2 model file
class TOXMFile : public TBasicCFile  {
  TGXApp& gxapp;
public:
  TOXMFile(TGXApp& app) : gxapp(app) {  }
  virtual ~TOXMFile() {  }

  virtual void SaveToStrings(TStrList& Strings)  {
    throw TNotImplementedException(__OlxSourceInfo);
  }
  virtual void LoadFromStrings(const TStrList& Strings)  {
    throw TNotImplementedException(__OlxSourceInfo);
  }
  virtual bool IsNative() const {  return true;  }
  virtual bool Adopt(TXFile&)  {  throw TNotImplementedException(__OlxSourceInfo);  }
  virtual void LoadFromFile(const olxstr& fn)  {
    gxapp.LoadModel(fn);
    RefMod.Assign( gxapp.XFile().GetRM(), true);
    FileName = fn;
  }
  virtual void SaveToFile(const olxstr& fn)  {
    gxapp.SaveModel(fn);
  }
  virtual IEObject* Replicate() const {  return new TOXMFile(gxapp);  }
};

EndGxlNamespace()
#endif
