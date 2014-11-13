/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sld_integration_H
#define __olx_sld_integration_H

#ifdef HAVE_GCCVISIBILITYPATCH
  #define DllExport __attribute__ ((visibility("default")))
#else
  #ifdef _MSC_VER
    #define DllImport   __declspec( dllimport )
    #define DllExport   __declspec( dllexport )
  #endif
  #ifdef __BORLANDC__
    #define DllExport __export
  #endif
  #ifdef __GNUC__
    #define DllExport
  #endif
#endif

#include "library.h"

namespace olex2 {

  // this can be safelly used in DLLs
  class IDllOlex2 {
  public:
    virtual olx_dll_ptr<olxch> process_function(const olxch *f) = 0;
    virtual bool process_macro(const olxch *m) = 0;
    virtual void log_message(const olxch *m, int level) = 0;
    virtual bool extend_macros(const olxch *name,
      bool (*func)(uint32_t, const olxch **, void *),
      void *instance) = 0;
    virtual bool extend_functions(const olxch *name,
      olx_dll_ptr<olxch> (*func)(uint32_t, const olxch **, void *),
      void *instance) = 0;
  };

  class IOlex2Processor : public IDllOlex2 {
  public:
    IOlex2Processor() { GetInstance() = this; }
    virtual ~IOlex2Processor() {}

    // uses custom macro error to set args, get rv
    virtual bool processMacro(const olxstr& cmdLine,
      const olxstr &location=EmptyString(), bool quiet=false) = 0;
    virtual bool processMacroEx(const olxstr& cmdLine,
      TMacroData& err, const olxstr &location=EmptyString(),
      bool quiet=false) = 0;
    virtual bool processFunction(olxstr& cmdl,
      const olxstr &location=EmptyString(), bool quiet=false) = 0;

    virtual bool registerCallbackFunc(const olxstr& cbEvent,
      ABasicFunction* fn) = 0;
    virtual void unregisterCallbackFunc(const olxstr& cbEvent,
      const olxstr& funcName) = 0;
    virtual void callCallbackFunc(const olxstr& cbEvent,
      const TStrList& params) = 0;

    virtual TLibrary&  GetLibrary() = 0;

    //IDllOlex2 implementation
    virtual olx_dll_ptr<olxch> process_function(const olxch *f);
    virtual bool process_macro(const olxch *m);
    // lvele is onr of logInfo/logWarning/logError/logException/logDefault
    virtual void log_message(const olxch *m, int level=logDefault);
    virtual bool extend_macros(const olxch *name,
      bool (*func)(uint32_t, const olxch **, void *),
      void *instance);
    virtual bool extend_functions(const olxch *name,
      olx_dll_ptr<olxch> (*func)(uint32_t, const olxch **, void *),
      void *instance);

    static IOlex2Processor *&GetInstance() {
      static IOlex2Processor* Instance=NULL;
      return Instance;
    }
  };

  class IOlex2Runnable : public IEObject {
  public:
    IOlex2Runnable() { GetOlex2Runnable() = this; }
    virtual ~IOlex2Runnable() {}
    /* if run returns true - the library can be unloaded, otherwise */
    virtual bool Init(IDllOlex2 *dll_olex2_inst) = 0;

    static IOlex2Runnable *&GetOlex2Runnable() {
      static IOlex2Runnable *Olex2Runnable = NULL;
      return Olex2Runnable;
    }
  };
  //............................................................................
};  // end namespace olex

#endif
