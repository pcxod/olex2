/******************************************************************************
* Copyright (c) 2004-2026 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "integration.h"
#include "bapp.h"

namespace olex2 {

struct ExternalMacro : public IOlxObject {
  bool (*func)(uint32_t arc, const olxch **, void *);
  void *instance;
  ExternalMacro(bool (*func)(uint32_t, const olxch **, void *), void *inst)
    : func(func), instance(inst)
  {}
  void Run(TStrObjList &Cmds, const TParamList &Options,
    TMacroData &E)
  {
    olx_array_ptr<const olxch *> argv(new const olxch*[Cmds.Count()+1]);
    for (size_t i=0; i < Cmds.Count(); i++) {
      argv[i] = Cmds[i].u_str();
    }
    if (!(*func)((uint32_t)Cmds.Count(), argv, instance)) {
      E.ProcessingError(__OlxSrcInfo, "external function failed");
    }
  }
};

struct ExternalFunction : public IOlxObject {
  olx_dll_ptr<olxch>(*func)(uint32_t arc, const olxch**, void*);
  void* instance;
  ExternalFunction(olx_dll_ptr<olxch>(*func)(uint32_t, const olxch**, void*),
    void* inst)
    : func(func), instance(inst)
  {}
  void Run(const TStrObjList& Params, TMacroData& E) {
    olx_array_ptr<const olxch*> argv(new const olxch * [Params.Count() + 1]);
    for (size_t i = 0; i < Params.Count(); i++) {
      argv[i] = Params[i].u_str();
    }
    olx_dll_ptr<olxch> rv = (*func)((uint32_t)Params.Count(), argv, instance);
    if (!rv.ok()) {
      E.ProcessingError(__OlxSrcInfo, "external function failed");
    }
    else {
      E.SetRetVal<olxstr>(*rv);
    }
  }
};

olx_dll_ptr<olxch> IOlex2Processor::process_function(const olxch *f) {
  IOlex2Processor *ip = GetInstance();
  if (ip == 0) {
    return olx_dll_ptr<olxch>();
  }
  olxstr s(f);
  if (ip->processFunction(s)) {
    return olx_dll_ptr<olxch>::copy(s.u_str(), s.Length()+1);
  }
  return olx_dll_ptr<olxch>();
}

bool IOlex2Processor::process_macro(const olxch *f) {
  IOlex2Processor *ip = GetInstance();
  return ip == 0 ? false : ip->processMacro(f);
}

void IOlex2Processor::log_message(const olxch *f, Logging level) {
  if (TBasicApp::HasInstance()) {
    TBasicApp::NewLogEntry(level) << f;
  }
}

bool IOlex2Processor::extend_macros(const olxch* name,
  bool (*f)(uint32_t, const olxch**, void*), void* instance)
{
  IOlex2Processor* ip = GetInstance();
  if (ip == 0) {
    return false;
  }
  ExternalMacro* em = new ExternalMacro(f, instance);
  TEGC::AddP(em);
  try {
    ip->GetLibrary().Register(
      new TMacro<ExternalMacro>(em, &ExternalMacro::Run, name, EmptyString(),
        fpAny, "External macro"),
      true);
    return true;
  }
  catch (const TExceptionBase&) {
    return false;
  }
}

bool IOlex2Processor::extend_functions(const olxch* name,
  olx_dll_ptr<olxch>(*f)(uint32_t, const olxch**, void*), void* instance)
{
  IOlex2Processor* ip = GetInstance();
  if (ip == 0) {
    return false;
  }
  ExternalFunction* ef = new ExternalFunction(f, instance);
  TEGC::AddP(ef);
  try {
    ip->GetLibrary().Register(
      new TFunction<ExternalFunction>(ef, &ExternalFunction::Run, name,
        fpAny, "External function"),
      true);
    return true;
  }
  catch (const TExceptionBase&) {
    return false;
  }
}

} //namespace olex2
