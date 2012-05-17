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

namespace olex {
  const short
    mtNone      = 0,
    mtInfo      = 1,
    mtWarning   = 2,
    mtError     = 3,
    mtException = 4;

  class IOlexProcessor  {
    static IOlexProcessor* Instance;
  public:
    IOlexProcessor()  {  Instance = this;  }
    virtual ~IOlexProcessor()  {}
    // uses custom macro error to set args, get rv
    virtual bool processMacro(const olxstr& cmdLine,
      const olxstr &location=EmptyString(), bool quiet=false) = 0;
    virtual bool processMacroEx(const olxstr& cmdLine,
      TMacroError& err, const olxstr &location=EmptyString(),
      bool quiet=false) = 0;
    virtual void print(const olxstr& Text,
      const short MessageType = mtNone) = 0;
    virtual bool processFunction(olxstr& cmdl,
      const olxstr &location=EmptyString(), bool quiet=false) = 0;
    //virtual IEObject* executeFunction(const olxstr& funcName) = 0;
    virtual TLibrary&  GetLibrary() = 0;
    virtual bool registerCallbackFunc(const olxstr& cbEvent,
      ABasicFunction* fn) = 0;
    virtual void unregisterCallbackFunc(const olxstr& cbEvent,
      const olxstr& funcName) = 0;

    virtual const olxstr& getDataDir() const = 0;
    virtual const olxstr& getVar(const olxstr &name,
      const olxstr &defval=EmptyString()) const = 0;
    virtual void setVar(const olxstr &name, const olxstr &val) const = 0;
    virtual TStrList GetPluginList() const = 0;
    virtual olxstr TranslateString(const olxstr& str) const = 0;
    virtual bool IsControl(const olxstr& cname) const = 0;
    static const olxstr SGListVarName;
    static DllExport IOlexProcessor* GetInstance()  {  return Instance;  }
  };

  class IOlexRunnable : public IEObject  {
  public:
    virtual ~IOlexRunnable()  {}
    virtual bool Run( IOlexProcessor& olexProcessor ) = 0;
  };
  class OlexPort : public IEObject  {
    static IOlexRunnable *OlexRunnable;
  protected:
  public:
    OlexPort()  {  OlexRunnable = NULL;  }
    ~OlexPort()  {
      if( OlexRunnable != NULL )
        delete OlexRunnable;
    }
  //............................................................................
    static void SetOlexRunnable( IOlexRunnable* o_r )  {  OlexRunnable = o_r;  }
  //............................................................................
    static DllExport IOlexRunnable* GetOlexRunnable();
  //............................................................................
  };
};  // end namespace olex

#endif
