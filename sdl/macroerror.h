/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_macroerror_H
#define __olx_sdl_macroerror_H
#include "exception.h"
#include "ptypes.h"
#include "estack.h"
#include "emath.h"
#include "log.h"
BeginEsdlNamespace()

class ABasicFunction;

const unsigned short
  peNonexistingFunction = 0x0001,
  peProcessingError     = 0x0002,
  peProcessingException = 0x0004,
  peInvalidOption       = 0x0008,
  peInvalidArgCount     = 0x0010,
  peIllegalState        = 0x0020,
  // these are for special handling
  peUnhandled           = 0x1000;

class TMacroData : public IOlxObject {
  unsigned short ProcessError;
  bool DeleteObject;
  olxstr ErrorInfo, Location;
  IOlxObject* RetValue;
  str_stack Stack;
public:
  TMacroData();
  virtual ~TMacroData() {
    if (DeleteObject) {
      delete RetValue;
    }
  }

  void operator = (const TMacroData& ME);
  olxstr& ProcessingError(const olxstr& location, const olxstr& errMsg);
  void NonexitingMacroError(const olxstr& macroName);
  void WrongArgCount(const ABasicFunction& caller, size_t provided);
  void WrongOption(const ABasicFunction& caller, const olxstr& option);
  void WrongState(const ABasicFunction& caller);
  void SetUnhandled(bool v) { olx_set_bit(v, ProcessError, peUnhandled); }

  void Reset() {
    ProcessError = 0;
    ErrorInfo.SetLength(0);
    Location.SetLength(0);
    if (DeleteObject) {
      delete RetValue;
    }
    DeleteObject = false;
    RetValue = 0;
    Stack.Clear();
  }
  void ClearErrorFlag() { ProcessError = 0; }

  void ProcessingException(const ABasicFunction& caller,
    const TExceptionBase& exc);

  void ProcessingException(const olxstr& location,
    const TExceptionBase& exc);

  bool IsSuccessful() const {
    return ((ProcessError & 0x0FFF) == 0);
  }
  bool DoesFunctionExist() const {
    return (ProcessError & peNonexistingFunction) == 0;
  }
  bool IsProcessingError() const {
    return (ProcessError & peProcessingError) != 0;
  }
  bool IsProcessingException() const {
    return (ProcessError & peProcessingException) != 0;
  }
  bool IsInvalidOption() const {
    return (ProcessError & peInvalidOption) != 0;
  }
  bool IsInvalidArguments() const {
    return (ProcessError & peInvalidArgCount) != 0;
  }
  bool IsIllegalState() const {
    return (ProcessError & peIllegalState) != 0;
  }

  bool IsHandled() const {
    return (ProcessError & peUnhandled) == 0;
  }

  const olxstr& GetInfo() const { return ErrorInfo; }
  DefPropC(olxstr, Location)

    olxstr GetRetVal() const;

  bool HasRetVal() const { return RetValue != 0; }
  IOlxObject* RetObj() const { return RetValue; }

  str_stack& GetStack() { return Stack; }

  void PrintStack(int logEvt = logError, bool annotate = false,
    const olxstr& prefix = EmptyString()) const;

  // the type is validated
  template <class EObj> EObj* GetRetObj() {
    EObj* r = dynamic_cast<EObj*>(RetValue);
    if (r == 0) {
      throw TCastException(__OlxSourceInfo, EsdlObjectName(*RetValue),
        EsdlClassName(EObj));
    }
    return r;
  }
  template <class PT> void SetRetVal(const PT& val) {
    if (DeleteObject) {
      delete RetValue;
    }
    DeleteObject = true;
    RetValue = createWrapper(val);
  }

  template <class PT> void SetRetVal(PT* val) {
    if (DeleteObject) {
      delete RetValue;
    }
    DeleteObject = false;
    RetValue = val;
  }

  class TCastException : public TBasicException {
  public:
    TCastException(const olxstr& location, const olxstr& from,
      const olxstr& to)
      : TBasicException(location,
        olxstr("Cannot cast '") << from << "' to '" << to << '\'')
    {}
    virtual IOlxObject* Replicate() const { return new TCastException(*this); }
  };

};

EndEsdlNamespace()
#endif
