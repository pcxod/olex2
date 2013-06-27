/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_op_imp_H
#define __olx_sdl_op_imp_H
#include "integration.h"
#include "macrolib.h"

/* This is a default and sufficient implementation of the IOlex2Processor
interface
*/
namespace olex {

class OlexProcessorImp : public IOlex2Processor {
  TCSTypeList<olxstr, ABasicFunction*> CallbackFuncs;
  ALibraryContainer *LibraryContainer;
  // object destruction handler
  void ODH(IEObject *o) {
    if (o == LibraryContainer)
      LibraryContainer = NULL;
  }
protected:
  macrolib::TEMacroLib Macros;

  virtual void AnalyseError(TMacroError &error) {
    AnalyseErrorEx(error, false);
  }

  virtual void AnalyseErrorEx(const TMacroError &error, bool quiet) {
    if (!error.IsSuccessful()) {
      if (error.IsProcessingException()) {
        TBasicApp::NewLogEntry(logException) << error.GetLocation() << ": " <<
          error.GetInfo();
      }
      else if (!error.GetInfo().IsEmpty()) {
        TBasicApp::NewLogEntry(quiet ? logInfo : logError) <<
          error.GetLocation() << ": " <<  error.GetInfo();
      }
      error.PrintStack(quiet ? logInfo : logDefault, false, '\t');
    }
  }
public:
  OlexProcessorImp(ALibraryContainer *lc)
    : LibraryContainer(lc),
    Macros(*this)
  {}
  virtual ~OlexProcessorImp() {
    for (size_t i=0; i < CallbackFuncs.Count(); i++)
      delete CallbackFuncs.GetObject(i);
    if (LibraryContainer)
      LibraryContainer->RemoveDestructionHandler(*this, &OlexProcessorImp::ODH);
  }

  virtual TLibrary&  GetLibrary() {
    if (LibraryContainer == NULL) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "uninitialised library container");
    }
    return LibraryContainer->GetLibrary();
  }

  void SetLibraryContainer(ALibraryContainer &lc) {
    LibraryContainer = &lc;
    lc.AddDestructionHandler(*this, &OlexProcessorImp::ODH);
  }

  virtual bool registerCallbackFunc(const olxstr& cbEvent,
    ABasicFunction* fn)
  {
    CallbackFuncs.Add(cbEvent, fn);
    return true;
  }

  virtual void unregisterCallbackFunc(const olxstr& cbEvent,
    const olxstr& funcName)
  {
    size_t ind = CallbackFuncs.IndexOf(cbEvent),
      i = ind;
    if (ind == InvalidIndex)  return;
    // go forward
    while (i < CallbackFuncs.Count() &&
      CallbackFuncs.GetKey(i).Equals(cbEvent))
    {
      if (CallbackFuncs.GetObject(i)->GetName() == funcName) {
        delete CallbackFuncs.GetObject(i);
        CallbackFuncs.Delete(i);
        return;
      }
      i++;
    }
    // go backwards
    i = ind-1;
    while (i !=InvalidIndex && (!CallbackFuncs.GetKey(i).Compare(cbEvent))) {
      if (CallbackFuncs.GetObject(i)->GetName() == funcName) {
        delete CallbackFuncs.GetObject(i);
        CallbackFuncs.Delete(i);
        return;
      }
      i--;
    }
  }

  virtual bool processFunction(olxstr &cmd,
    const olxstr& location=EmptyString(), bool quiet=false)
  {
    TMacroError err;
    err.SetLocation(location);
    const bool rv = Macros.ProcessFunction(cmd, err, false);
    AnalyseErrorEx(err, quiet);
    return rv;
  }

  virtual bool processMacro(const olxstr& cmd,
    const olxstr& location=EmptyString(), bool quiet=false)
  {
    TMacroError err;
    err.SetLocation(location);
    Macros.ProcessTopMacro(cmd, err, *this,
      quiet ? NULL : &OlexProcessorImp::AnalyseError);
    return err.IsSuccessful();
  }

  virtual bool processMacroEx(const olxstr& cmd, TMacroError &err,
    const olxstr& location=EmptyString(), bool quiet=false)
  {
    err.SetLocation(location);
    Macros.ProcessTopMacro(cmd, err, *this,
      quiet ? NULL : &OlexProcessorImp::AnalyseError);
    return err.IsSuccessful();
  }
  virtual void callCallbackFunc(const olxstr& cbEvent, const TStrList& params) {
    TSizeList indexes;
    TMacroError me;
    CallbackFuncs.GetIndices(cbEvent, indexes);
    for( size_t i=0; i < indexes.Count(); i++ )  {
      CallbackFuncs.GetObject(indexes[i])->Run(params, me);
      AnalyseError(me);
      me.Reset();
    }
  }
}; // end OlexProcessorImp
} //end namespace olex
#endif
