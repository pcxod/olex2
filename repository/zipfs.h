/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_zipfs_H
#define __olx_zipfs_H
#include "filesystem.h"

class AZipFS : public AFileSystem {
public:
  virtual bool ExtractAll(const olxstr &dest) = 0;
};

class ZipFSFactory  {
public:
  typedef AZipFS *(*instance_maker_t)(const olxstr &zip_name, bool use_cache);
protected:
  static instance_maker_t &instance_maker_() {
    static instance_maker_t instance_maker;
    return instance_maker;
  }
public:
  static AZipFS *GetInstance(const olxstr &zip_name, bool use_cache=false) {
    if (instance_maker_() == NULL) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "due to uninitialised ZipfFS factory");
    }
    return instance_maker_()(zip_name, use_cache);
  }
  // returns previously registered maker
  static instance_maker_t Register(instance_maker_t maker)  {
    olx_swap(instance_maker_(), maker);
    return maker;
  }
};

#endif
