/******************************************************************************
* Copyright (c) 2004-2022 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/
#pragma once
#include "efile.h"

#ifndef __WIN32__
#include <dlfcn.h>
#endif

struct OlxDll : public IOlxObject {
#ifdef __WIN32__
  HMODULE handle;
#else
  void* handle;
#endif
  OlxDll(const olxstr& file, bool has_ext=false, bool check_file=true) : handle(0) {
    olxstr lib_name = file;
    if (!has_ext) {
#ifdef __WIN32__
      lib_name << ".dll";
#else
      lib_name << ".so";
#endif
    }
    if (check_file && !TEFile::Exists(lib_name)) {
      return;
    }

#ifdef __WIN32__
    handle = LoadLibrary(lib_name.wc_str());
#else
    handle = dlopen(TUtf8::Encode(lib_name).c_str(), RTLD_GLOBAL | RTLD_LAZY);
#endif
  }
  ~OlxDll() {
    if (handle != 0) {
#ifdef __WIN32__
      FreeLibrary(handle);
#else
      dlclose(handle);
#endif
    }
  }
  bool ok() const { return handle != 0; }
  
  template <typename func_t>
  func_t get(const olxcstr& fn) const {
    if (handle == 0) {
      throw TFunctionFailedException(__OlxSourceInfo, "uninitialised");
    }
#ifdef __WIN32__
    return (func_t)GetProcAddress(handle, fn.c_str());
#else
    return (func_t)dlsym(handle, fn.c_str());
#endif
  }

};
#pragma once
