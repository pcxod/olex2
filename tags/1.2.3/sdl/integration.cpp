/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "integration.h"
#include "bapp.h"

namespace olex2 {

struct ExternalMacro : public IEObject {
  bool (*func)(uint32_t arc, const wchar_t **, void *);
  void *instance;
  ExternalMacro(bool (*func)(uint32_t, const wchar_t **, void *), void *inst)
    : func(func), instance(inst)
  {}
  void Run(TStrObjList &Cmds, const TParamList &Options,
    TMacroError &E)
  {
    olx_array_ptr<const wchar_t *> argv(new const wchar_t*[Cmds.Count()+1]);
    for (size_t i=0; i < Cmds.Count(); i++) {
      argv()[i] = Cmds[i].u_str();
    }
    if (!(*func)((uint32_t)Cmds.Count(), argv(), instance))
      E.ProcessingError(__OlxSrcInfo, "external function failed");
  }
};

struct ExternalFunction : public IEObject {
  olx_dll_ptr<wchar_t> (*func)(uint32_t arc, const wchar_t **, void *);
  void *instance;
  ExternalFunction(olx_dll_ptr<wchar_t> (*func)(uint32_t, const wchar_t **, void *),
    void *inst)
    : func(func), instance(inst)
  {}
  void Run(const TStrObjList& Params, TMacroError &E) {
    olx_array_ptr<const wchar_t *> argv(new const wchar_t*[Params.Count()+1]);
    for (size_t i=0; i < Params.Count(); i++) {
      argv()[i] = Params[i].u_str();
    }
    olx_dll_ptr<wchar_t> rv = (*func)((uint32_t)Params.Count(), argv(), instance);
    if (!rv.is_valid())
      E.ProcessingError(__OlxSrcInfo, "external function failed");
    else
      E.SetRetVal<olxstr>(rv());
  }
};

olx_dll_ptr<wchar_t> IOlex2Processor::process_function(const wchar_t *f) {
  IOlex2Processor *ip = GetInstance();
  if (ip == NULL)
    return olx_dll_ptr<wchar_t>();
  olxstr s(f);
  if (ip->processFunction(s)) {
    return olx_dll_ptr<wchar_t>::copy(s.u_str(), s.Length()+1);
  }
  return olx_dll_ptr<wchar_t>();
}

bool IOlex2Processor::process_macro(const wchar_t *f) {
  IOlex2Processor *ip = GetInstance();
  if (ip == NULL)
    return false;
  return ip->processMacro(f);
}

void IOlex2Processor::log_message(const wchar_t *f, int level) {
  if (!TBasicApp::HasInstance())
    return;
    TBasicApp::NewLogEntry(level) << f;
}

bool IOlex2Processor::extend_macros(const wchar_t *name,
  bool (*f)(uint32_t, const wchar_t **, void *), void *instance)
{
  IOlex2Processor *ip = GetInstance();
  if (ip == NULL) return false;
  ExternalMacro *em = new ExternalMacro(f, instance);
  TEGC::AddP(em);
  try {
    ip->GetLibrary().Register(
      new TMacro<ExternalMacro>(em, &ExternalMacro::Run, name, EmptyString(),
      fpAny, "External macro"),
      true);
    return true;
  }
  catch(const TExceptionBase &) {
    return false;
  }
}

bool IOlex2Processor::extend_functions(const wchar_t *name,
  olx_dll_ptr<wchar_t> (*f)(uint32_t, const wchar_t **, void *), void *instance)
{
  IOlex2Processor *ip = GetInstance();
  if (ip == NULL) return false;
  ExternalFunction *ef = new ExternalFunction(f, instance);
  TEGC::AddP(ef);
  try {
    ip->GetLibrary().Register(
      new TFunction<ExternalFunction>(ef, &ExternalFunction::Run, name,
      fpAny, "External function"),
      true);
    return true;
  }
  catch(const TExceptionBase &) {
    return false;
  }
}

} //namespace olex2
