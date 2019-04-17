/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "pyext.h"
#include "efile.h"
#include "bapp.h"
#include "log.h"
#include "fsext.h"
#include "estrlist.h"
#include "function.h"
#include "macroerror.h"
#include "eutf8.h"
#include "ememstream.h"
#include "md5.h"
#include <stdarg.h>

// egc cannot be used here as python can be finalised before egc is called
// causing troubles with reference counting ...
//#include "egc.h"
#ifdef _PYTHON
//.............................................................................
//.............................................................................
class TFuncWrapper : public PythonExt::BasicWrapper {
  bool ProcessOutput;
public:
  TFuncWrapper(PyObject* callable, bool processOutput, bool profile) :
    PythonExt::BasicWrapper(true)
  {
    PyFunction = callable;
    Py_XINCREF(PyFunction);
    ProcessOutput = processOutput;
  }
  virtual ~TFuncWrapper() { Py_XDECREF(PyFunction); }

  void Call(const TStrObjList& Params, TMacroData& E) {
    OnCallEnter();
    PyObject *arglist = 0, *kwds = 0;
    if (!Params.IsEmpty()) {
      size_t kwd_cnt = 0;
      for (size_t i = 0; i < Params.Count(); i++) {
        if (Params[i].StartsFrom('-')) {
          size_t eidx = Params[i].IndexOf('=');
          if (eidx != InvalidIndex) {
            if (kwds == 0) {
              kwds = PyDict_New();
            }
            PyDict_SetItem(kwds,
              PythonExt::BuildString(Params[i].SubString(1, eidx-1)),
              PythonExt::BuildString(Params[i].SubStringFrom(eidx + 1))
            );
            kwd_cnt++;
          }
        }
      }
      arglist = PyTuple_New(Params.Count()- kwd_cnt);
      for (size_t i = 0; i < Params.Count(); i++) {
        if (!(Params[i].StartsFrom('-') && Params[i].Contains('='))) {
          PyTuple_SetItem(arglist, i, PythonExt::BuildString(Params[i]));
        }
      }
    }
    if (arglist == 0) {
      arglist = PyTuple_New(0);
    }
    PyObject* result = PyObject_Call(PyFunction, arglist, kwds);
    Py_DECREF(arglist);
    if (kwds != 0) {
      Py_DECREF(kwds);
    }

    if (result != 0) {
      if (ProcessOutput && result != Py_None) {
        E.SetRetVal<olxstr>(PythonExt::ParseStr(result));
      }
      Py_DECREF(result);
    }
    if (PyErr_Occurred()) {
      PyErr_Print();
    }
    OnCallLeave();
  }
};

//.............................................................................
//.............................................................................
class TMacroWrapper : public PythonExt::BasicWrapper {
public:
  TMacroWrapper(PyObject* callable, bool profile) :
    PythonExt::BasicWrapper(profile)
  {
    PyFunction = callable;
    Py_XINCREF(PyFunction);
  }
  virtual ~TMacroWrapper() {
    Py_XDECREF(PyFunction);
  }
  void Call(TStrObjList& Params, const TParamList &Options, TMacroData& E) {
    OnCallEnter();
    PyObject* arglist = PyTuple_New(Params.Count());
    for (size_t i = 0; i < Params.Count(); i++) {
      PyTuple_SetItem(arglist, i, PyString_FromString(Params[i].c_str()));
    }
    PyObject* options = PyDict_New();
    for (size_t i = 0; i < Options.Count(); i++) {
      PythonExt::SetDictItem(options,
        PythonExt::BuildString(Options.GetName(i)),
        PythonExt::BuildString(Options.GetValue(i)));
    }
    PyObject* result = PyObject_Call(PyFunction, arglist, options);
    if (arglist != 0) {
      Py_DECREF(arglist);
    }
    if (result != 0) {
      Py_DECREF(result);
    }
    if (PyErr_Occurred()) {
      PyErr_Print();
    }
    OnCallLeave();
  }
};

//.............................................................................
//.............................................................................
//.............................................................................
olx_critical_section py_io_cs_;
PyObject* runWriteImage(PyObject* self, PyObject* args) {
  char *data = 0;
  olxstr name;
  int persistenceId = 0;
  int length = 0;
  if (!PythonExt::ParseTuple(args, "ws#|i", &name, &data, &length,
    &persistenceId))
  {
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "ws#|i");
  }
  if (data != 0 && !name.IsEmpty()) {
    TFileHandlerManager::AddMemoryBlock(name, data, length, persistenceId);
    return Py_BuildValue("b", true);
  }
  return Py_BuildValue("b", false);
}
//.............................................................................
PyObject* runReadImage(PyObject* self, PyObject* args)  {
  olxstr name;
  if (!PythonExt::ParseTuple(args, "w", &name)) {
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "w");
  }
  if (!name.IsEmpty()) {
    olx_object_ptr<IInputStream> io = TFileHandlerManager::GetInputStream(name);
    if (io.is_valid()) {
      const size_t is = io().GetAvailableSizeT();
      olx_array_ptr<char> bf(is + 1);
      io().Read(bf(), is);
      PyObject* po = Py_BuildValue("s#", bf(), is);
      return po;
    }
  }
  return PythonExt::SetErrorMsg(
    PyExc_TypeError, __OlxSourceInfo, "Undefined object");
}
//.............................................................................
TLibrary *FindOrCreateLibrary(const olxstr& name) {
  TLibrary *lib = PythonExt::GetInstance()->GetBindLibrary();
  if (lib == 0) {
    return lib;
  }
  if (!name.IsEmpty()) {
    TStrList toks(name, '.');
    for (size_t i = 0; i < toks.Count(); i++) {
      TLibrary *sl = lib->GetLibraryByName(toks[i]);
      if (sl == 0) {
        sl = lib->AddLibrary(toks[i]);
      }
      lib = sl;
    }
  }
  return lib;
}
//.............................................................................
PyObject* runRegisterFunction(PyObject* self, PyObject* args) {
  PyObject* fun;
  olxstr lib_name;
  bool profile = false;
  if( !PythonExt::ParseTuple(args, "O|bw", &fun, &profile, &lib_name) )
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "O|bw");
  if (!PyCallable_Check(fun)) {
    return PythonExt::SetErrorMsg(PyExc_TypeError, __OlxSourceInfo,
      "Parameter must be callable");
  }
  TLibrary *lib = FindOrCreateLibrary(lib_name);
  if (lib == NULL)
    return PythonExt::SetErrorMsg(PyExc_RuntimeError, __OlxSourceInfo,
    "Olex2 binding python library is not initialised...");

  TFuncWrapper* fw = PythonExt::GetInstance()->AddToDelete(
    new TFuncWrapper(fun, true, profile));
  try {
    lib->Register(new TFunction<TFuncWrapper>(
      fw, &TFuncWrapper::Call, PyEval_GetFuncName(fun), fpAny),
      libReplace);
    return Py_BuildValue("b", true);
  }
  catch (const TExceptionBase& exc) {
    TStrList st;
    exc.GetException()->GetStackTrace(st);
    return PythonExt::SetErrorMsg(PyExc_TypeError,
      __OlxSourceInfo, st.Text('\n'));
  }
}
//.............................................................................
PyObject* runRegisterCallback(PyObject* self, PyObject* args) {
  olxstr cbEvent;
  PyObject* fun;
  bool profile = false;
  if (!PythonExt::ParseTuple(args, "wO|b", &cbEvent, &fun, &profile)) {
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "wO|b");
  }
  if (!PyCallable_Check(fun)) {
    return PythonExt::SetErrorMsg(PyExc_TypeError, __OlxSourceInfo,
      "Parameter must be callable");
  }
  if (PythonExt::GetInstance()->GetBindLibrary() == 0) {
    return PythonExt::SetErrorMsg(PyExc_RuntimeError, __OlxSourceInfo,
      "Olex2 binding python library is not initialised...");
  }
  // leave function wrapper util the end ..., but delete the binding
  TFuncWrapper* fw = PythonExt::GetInstance()->AddToDelete(
    new TFuncWrapper(fun, false, profile));
  PythonExt::GetInstance()->GetOlexProcessor()->registerCallbackFunc(cbEvent,
    new TFunction<TFuncWrapper>(
      fw, &TFuncWrapper::Call, PyEval_GetFuncName(fun), fpAny));
  return Py_BuildValue("b", true);
}
//.............................................................................
PyObject* runUnregisterCallback(PyObject* self, PyObject* args) {
  olxstr cbEvent;
  PyObject* fun;
  if (!PythonExt::ParseTuple(args, "wO", &cbEvent, &fun)) {
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "wO");
  }
  if (!PyCallable_Check(fun)) {
    return PythonExt::SetErrorMsg(PyExc_TypeError, __OlxSourceInfo,
      "Parameter must be callable");
  }
  PythonExt::GetInstance()->GetOlexProcessor()->unregisterCallbackFunc(
    cbEvent, PyEval_GetFuncName(fun));
  return Py_BuildValue("b", true);
}
//.............................................................................
PyObject* runRegisterMacro(PyObject* self, PyObject* args) {
  olxstr validOptions;
  PyObject* fun;
  olxstr lib_name;
  bool profile = false;
  if (!PythonExt::ParseTuple(args, "Ow|bw", &fun, &validOptions, &profile,
    &lib_name))
  {
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "Ow|bw");
  }
  if (!PyCallable_Check(fun)) {
    return PythonExt::SetErrorMsg(PyExc_TypeError, __OlxSourceInfo,
      "Parameter must be callable");
  }
  TLibrary *lib = FindOrCreateLibrary(lib_name);
  if (lib == 0) {
    return PythonExt::SetErrorMsg(PyExc_RuntimeError, __OlxSourceInfo,
      "Olex2 binding python library is not initialised...");
  }
  TMacroWrapper* mw = PythonExt::GetInstance()->AddToDelete(
    new TMacroWrapper(fun, profile));
  try {
    lib->Register(new TMacro<TMacroWrapper>(mw, &TMacroWrapper::Call,
      PyEval_GetFuncName(fun), validOptions, fpAny), true);
    return Py_BuildValue("b", true);
  }
  catch (const TExceptionBase& exc) {
    TStrList st;
    exc.GetException()->GetStackTrace(st);
    return PythonExt::SetErrorMsg(PyExc_TypeError, __OlxSourceInfo,
      st.Text('\n'));
  }
}
//.............................................................................
PyObject* runGetProfileInfo(PyObject* self, PyObject* args)  {
  return PythonExt::GetInstance()->GetProfileInfo();
}
//.............................................................................
PyObject* runOlexMacro(PyObject* self, PyObject* args)  {
  IOlex2Processor* o_r = PythonExt::GetInstance()->GetOlexProcessor();
  olxstr macroName;
  if( !PythonExt::ParseTuple(args, "w", &macroName) )
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "w");
  bool res = o_r->processMacro(macroName);
  return Py_BuildValue("b", res);
}
//.............................................................................
PyObject* runOlexFunction(PyObject* self, PyObject* args)  {
  IOlex2Processor* o_r = PythonExt::GetInstance()->GetOlexProcessor();
  olxstr functionName;
  if( !PythonExt::ParseTuple(args, "w", &functionName) )
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "w");
  olxstr retValue = functionName;
  bool res = o_r->processFunction(retValue);
  if (res)
    return PythonExt::BuildString(retValue);
  return PythonExt::SetErrorMsg(PyExc_RuntimeError, __OlxSourceInfo,
    olxstr("Function '") << functionName << "' failed");
}
//.............................................................................
PyObject* runOlexFunctionEx(PyObject* self, PyObject* args)  {
  using namespace macrolib;
  IOlex2Processor* o_r = PythonExt::GetInstance()->GetOlexProcessor();
  olxstr name;
  bool macro;
  PyObject *args_, *kwds_=NULL;
  if( !PythonExt::ParseTuple(args, "wbO|O", &name, &macro, &args_, &kwds_) )
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "wbO|O");
  TStrObjList params;
  for (Py_ssize_t i=0; i < PyList_Size(args_); i++) {
    olxstr val = exparse::parser_util::unquote(
      PythonExt::ParseStr(PyList_GetItem(args_, i)));
    //olxstr val = PythonExt::ParseStr(PyList_GetItem(args_, i));
    params.Add(val);
  }
  if (macro) {
    TParamList options;
    ABasicFunction* macro = o_r->GetLibrary().FindMacro(name);
    if( macro == NULL )
      return PythonExt::SetErrorMsg(PyExc_RuntimeError, __OlxSourceInfo,
        olxstr("Undefined macro '") << name << '\'');
    if (kwds_ != NULL && PyDict_Size(kwds_) != 0) {
      PyObject *keys_ = PyDict_Keys(kwds_);
      for (Py_ssize_t i=0; i < PyList_Size(keys_); i++) {
        PyObject *key_ = PyList_GetItem(keys_, i);
        PyObject *val = PyDict_GetItem(kwds_, key_);
        if (val == NULL || val == Py_None)
          continue;
        olxstr str_val = PythonExt::ParseStr(val);
        options.AddParam(PythonExt::ParseStr(key_), str_val);
      }
      Py_DECREF(keys_);
    }
    TMacroData er;
    macro->Run(params, options, er);
    if ((PythonExt::GetInstance()->GetLogLevel() & macro_log_macro) != 0){
      TBasicApp::NewLogEntry(logInfo) << "@py: " <<
        macro->GetRuntimeSignature();
    }
    if (er.IsSuccessful())
      return Py_BuildValue("b", true);
    else {
      TBasicApp::NewLogEntry() << "Macro '" << name << "' failed: " <<
        er.GetInfo();
      return Py_BuildValue("b", false);
    }
  }
  else {
    ABasicFunction* func = o_r->GetLibrary().FindFunction(name);
    if (func == NULL)
      return PythonExt::SetErrorMsg(PyExc_RuntimeError, __OlxSourceInfo,
        olxstr("Undefined function '") << name << '\'');
    TMacroData er;
    func->Run(params, er);
    if ((PythonExt::GetInstance()->GetLogLevel() & macro_log_function) != 0) {
      TBasicApp::NewLogEntry(logInfo) << "@py: " <<
        func->GetRuntimeSignature();
    }
    if (er.IsSuccessful()) {
      olxstr rv = (er.HasRetVal() ? olxstr(er.RetObj()->ToString())
        : EmptyString());
      return PythonExt::BuildString(rv);
    }
    else {
      return PythonExt::SetErrorMsg(PyExc_RuntimeError, __OlxSourceInfo,
        olxstr("Function '") << name << "' failed: " << er.GetInfo());
    }
  }
}
//.............................................................................
PyObject* runPrintText(PyObject* self, PyObject* args)  {
  for (Py_ssize_t i=0; i < PyTuple_Size(args); i++) {
    PyObject* po = PyTuple_GetItem(args, i);
    olxstr s = PythonExt::ParseStr(po).Trim('\'');
    bool nl=false;
    if (s.EndsWith("\r\n")) {
      nl = true;
      s.SetLength(s.Length()-1);
    }
    else if (s.EndsWith('\n') || s.EndsWith('\r')) {
      nl = true;
      s.SetLength(s.Length()-1);
    }
    if (nl) TBasicApp::NewLogEntry();
    TBasicApp::GetLog() << s;
  }
  return PythonExt::PyNone();
}
//.............................................................................
PyMethodDef Methods[] = {
  {"m", runOlexMacro, METH_VARARGS, "executes olex macro"},
  {"f", runOlexFunction, METH_VARARGS, "executes olex function"},
  {"f_ex", runOlexFunctionEx, METH_VARARGS, "executes olex function"},
  {"writeImage", runWriteImage, METH_VARARGS, "adds new image/object to olex2 "
    "memory; (name, date, persistence=0). Persistence: 0 - none, 1 - for "
    "current structure, 2 - global"},
  {"readImage", runReadImage, METH_VARARGS,
    "reads an image/object from olex2 memory"},
  {"registerFunction", runRegisterFunction, METH_VARARGS,
    "registers python function in olex2 domain"},
  {"registerMacro", runRegisterMacro, METH_VARARGS,
    "registers python macro in olex2 domain"},
  {"registerCallback", runRegisterCallback, METH_VARARGS,
    "registers a python callback function in olex2 domain"},
  {"unregisterCallback", runUnregisterCallback, METH_VARARGS,
    "unregisters a python callback function in olex2 domain"},
  {"getProfileInfo", runGetProfileInfo, METH_VARARGS,
    "returns a tuple of {function, number of calls, total time (ms)} records"},
  {"post", runPrintText, METH_VARARGS, "prints provided text in Olex2"},
  {NULL, NULL, 0, NULL}
};
//.............................................................................
olxcstr PyFuncBody(const olxcstr& olexName, const olxcstr& pyName, char sep,
  const olxstr &module_name)
{
  if( sep == ',' )  {
    olxcstr res("def ");
    res << pyName << "(*args):\n  ";
    res << "al = []\n  ";
    res << "for arg in args:\n    ";
    res << "al.append(unicode(arg))\n  ";
    res << "return " << module_name << ".f_ex('" << olexName << "', False, al)";
    return res;
  }
  else  {
    olxcstr res("def ");
    res << pyName << "(*args, **kwds):\n  ";
    res << "al = []\n  ";
    res << "for arg in args:\n    ";
    res << "al.append(unicode(arg))\n  ";
    res << "return " << module_name << ".f_ex('" << olexName <<
      "', True, al, kwds)";
    return res;
  }
}
//.............................................................................
PythonExt::PythonExt(IOlex2Processor* olexProcessor, const olxstr &module_name)
  : module_name(module_name),
    LogLevel(macrolib::macro_log_macro)
{
  if (Instance() != NULL)
    throw TFunctionFailedException(__OlxSourceInfo, "singleton");
  Instance() = this;
  OlexProcessor = olexProcessor;
}
//.............................................................................
PythonExt::~PythonExt() {
  ClearToDelete();
  if (Py_IsInitialized()) {
    Py_Finalize();
  }
  Instance() = NULL;
}
//.............................................................................
void PythonExt::CheckInitialised() {
  if (!Py_IsInitialized()) {
    Py_Initialize();
    PyEval_InitThreads();
    Py_InitModule(module_name.c_str(), Methods);
    for (size_t i=0; i < ToRegister.Count(); i++)
      (*ToRegister[i])();
  }
}
//.............................................................................
PyObject* PythonExt::GetProfileInfo()  {
  size_t picnt = 0;
  for( size_t i=0; i < ToDelete.Count(); i++ ) {
    if( ToDelete[i]->ProfileInfo() != NULL &&
        ToDelete[i]->ProfileInfo()->CallCount > 0 )
    {
      picnt++;
    }
  }

  PyObject *rv = PyTuple_New(picnt);
  picnt = 0;
  for( size_t i=0; i < ToDelete.Count(); i++ )  {
    PythonExt::ProfileInfo *pi = ToDelete[i]->ProfileInfo();
    if( pi != NULL && pi->CallCount > 0 )  {
      PyTuple_SetItem(rv, picnt, Py_BuildValue("sil",
        PyEval_GetFuncName(
          ToDelete[i]->GetFunction()), pi->CallCount, pi->TotalTimeMs));
      picnt++;
    }
  }
  return rv;
}
//.............................................................................
int PythonExt::RunPython(const olxstr& script)  {
  CheckInitialised();
  return PyRun_SimpleString(script.c_str());
}
//.............................................................................
void ExportLib(const olxstr &_root, const TLibrary& Lib,
  const olxstr &module_name)
{
  if (Lib.IsEmpty()) return;

  olxstr root = TEFile::AddPathDelimeter(_root);
  if( !TEFile::Exists(root) )
    TEFile::MakeDir(root);
  TEFile out(root + "__init__.py", "w+b");
  out.Write("import sys\n");
  out.Write(olxcstr("import ") << module_name << "\n");
  for( size_t i=0; i < Lib.LibraryCount(); i++ )  {
    TLibrary &lib = *Lib.GetLibraryByIndex(i);
    if( !lib.IsEmpty() )
      out.Write(olxcstr("import ") << lib.GetName() << '\n');
  }
  for( size_t i=0; i < Lib.FunctionCount(); i++ )  {
    ABasicFunction* fun = Lib.GetFunctionByIndex(i);
    olxstr pyName = fun->GetName();
    pyName.Replace('@', "At");
    out.Write(PyFuncBody(fun->GetQualifiedName(), pyName, ',', module_name)
      << '\n');
  }

  for( size_t i=0; i < Lib.MacroCount(); i++ )  {
    ABasicFunction* fun = Lib.GetMacroByIndex(i);
    if (dynamic_cast<macrolib::TEMacro*>(fun) != NULL)
      continue;
    olxstr pyName = fun->GetName();
    pyName.Replace('@', "At");
    out.Write(PyFuncBody(fun->GetQualifiedName(), pyName, ' ', module_name)
      << '\n');
  }
  for( size_t i=0; i < Lib.LibraryCount(); i++ )  {
    TLibrary &lib = *Lib.GetLibraryByIndex(i);
    ExportLib(root + lib.GetName(),  lib, module_name);
  }
}
//.............................................................................
void PythonExt::funExport(const TStrObjList& Cmds, TMacroData& E)  {
  IOlex2Processor* o_r = PythonExt::GetInstance()->GetOlexProcessor();
  if( o_r == NULL )  return;
  // clean up legacy export
  if (TEFile::Exists(Cmds[0]+".py")) {
    TEFile::DelFile(Cmds[0]+".py");
  }
  if (TEFile::Exists(Cmds[0]+".pyc")) {
    TEFile::DelFile(Cmds[0]+".pyc");
  }
  bool do_export = true;
  olxstr last_exp_fn = Cmds[0] + ".cache";
  if (TEFile::Exists(last_exp_fn)) {
    TCStrList l = TEFile::ReadCLines(last_exp_fn);
    if (l.Count() > 0 && l[0] == TBasicApp::GetModuleMD5Hash())
      do_export = false;
  }
  if (do_export || !TEFile::Exists(Cmds[0])) {
    TCStrList l;
    l << TBasicApp::GetModuleMD5Hash();
    TEFile::WriteLines(last_exp_fn, l);
    ExportLib(Cmds[0], o_r->GetLibrary(), module_name);
  }
}
//.............................................................................
void PythonExt::macReset(TStrObjList& Cmds, const TParamList &Options,
  TMacroData& E)
{
  if( Py_IsInitialized() )
    Py_Finalize();
  CheckInitialised();
}
//.............................................................................
void PythonExt::macRun(TStrObjList& Cmds, const TParamList &Options,
  TMacroData& E)
{
  olxstr fn = TEFile::OSPath(Cmds.Text(' '));
  if( !TEFile::Exists(fn) )  {
    E.ProcessingError(__OlxSrcInfo,
      "specified script file does not exist: ") << fn;
    return;
  }
  olxstr cd = TEFile::CurrentDir();
  TEFile::ChangeDir(TEFile::ExtractFilePath(fn));

  if( RunPython(
        olxstr("execfile(\'") << TEFile::ExtractFileName(fn) << "\')") == -1)
  {
    E.ProcessingError(__OlxSrcInfo, "script execution failed");
  }

  TEFile::ChangeDir(cd);
}
//.............................................................................
void PythonExt::funLogLevel(const TStrObjList& Params, TMacroData& E)  {
  using namespace macrolib;
  if( Params.IsEmpty() )  {
    olxstr ll;
    if( (GetLogLevel()&macro_log_macro) != 0 )  ll << 'm';
    if( (GetLogLevel()&macro_log_function) != 0 )  ll << 'f';
    E.SetRetVal(ll);
  }
  else  {
    uint8_t ll = 0;
    if( Params[0].IndexOfi('m') != InvalidIndex )  ll |= macro_log_macro;
    if( Params[0].IndexOfi('f') != InvalidIndex )  ll |= macro_log_function;
    SetLogLevel(ll);
  }
}
//.............................................................................
TLibrary* PythonExt::ExportLibrary(const olxstr& name)  {
  // binding library
  PythonExt::GetOlexProcessor()->GetLibrary().AttachLibrary(
    BindLibrary=new TLibrary("spy"));
  Library = new TLibrary(name.IsEmpty() ? olxstr("py") : name);
  Library->Register(
    new TFunction<PythonExt>(this, &PythonExt::funExport,
      "Export", fpOne,
      "Exports the library to a folder"));
  Library->Register(
    new TMacro<PythonExt>(this, &PythonExt::macReset,
      "Reset", EmptyString(), fpNone));
  Library->Register(
    new TMacro<PythonExt>(this, &PythonExt::macRun,
      "Run", EmptyString(),
      fpAny^fpNone, "Runs provided file"));
  Library->Register(
    new TFunction<PythonExt>(this, &PythonExt::funLogLevel,
      "LogLevel", fpNone|fpOne,
      "Sets log level - default is macro, look at LogLevel for more "
      "information"));
  return Library;
}

bool PythonExt::ParseTuple(PyObject* tuple, const char* format, ...) {
  va_list argptr;
  va_start(argptr, format);
  if (tuple == NULL) {
    va_end(argptr);
    return false;
  }
  const size_t slen = olxstr::o_strlen(format),
    tlen = PyTuple_Size(tuple);
  bool proceedOptional = (slen > 0 && (format[0] == '|'));
  size_t tind = InvalidIndex,
    start = proceedOptional ? 1 : 0;
  for (size_t i = start; i < slen; i++) {
    if (++tind >= tlen) {
      if (!proceedOptional) {
        va_end(argptr);
        return false;
      }
      else {
        break;
      }
    }
    PyObject* io = PyTuple_GetItem(tuple, tind);
    if (format[i] == 'i') {
      int* ip = va_arg(argptr, int*);
      if (!PyArg_Parse(io, "i", ip)) {
        va_end(argptr);
        return false;
      }
    }
    else if (format[i] == 'L') {
      int64_t* ip = va_arg(argptr, int64_t*);
      if (!PyArg_Parse(io, "L", ip)) {
        va_end(argptr);
        return false;
      }
    }
    else if (format[i] == 'w') {
      olxstr* os = va_arg(argptr, olxstr*);
      os->SetLength(0);
      if (io->ob_type == &PyString_Type) {
        char* str;
        int len;
        PyArg_Parse(io, "s#", &str, &len);
        os->Append(str, len);
      }
      else if (io->ob_type == &PyUnicode_Type) {
        Py_ssize_t usz = PyUnicode_GetSize(io);
        TTBuffer<wchar_t> wc_bf(usz + 1);
        usz = PyUnicode_AsWideChar((PyUnicodeObject*)io, wc_bf.Data(), usz);
        if (usz > 0)
          os->Append(wc_bf.Data(), usz);
      }
      else {
        va_end(argptr);
        return false;
      }
    }
    else if (format[i] == 's') {
      char** cstr = va_arg(argptr, char**);
      int len, *rlen = NULL;
      if ((i + 1) < slen && format[i + 1] == '#') {
        rlen = va_arg(argptr, int*);
        i++;
      }
      else
        rlen = &len;
      if (!PyArg_Parse(io, "s#", cstr, rlen)) {
        va_end(argptr);
        return false;
      }
    }
    else if (format[i] == 'f') {
      float* fp = va_arg(argptr, float*);
      if (!PyArg_Parse(io, "f", fp)) {
        va_end(argptr);
        return false;
      }
    }
    else if (format[i] == 'd') {
      double* fp = va_arg(argptr, double*);
      if (!PyArg_Parse(io, "d", fp)) {
        va_end(argptr);
        return false;
      }
    }
    else if (format[i] == 'O') {
      PyObject** fp = va_arg(argptr, PyObject**);
      *fp = io;
    }
    else if (format[i] == 'b') {
      bool* bp = va_arg(argptr, bool*);
      if (!PyArg_Parse(io, "b", bp)) {
        va_end(argptr);
        return false;
      }
    }
    else if (format[i] == '|') {
      continue;
    }
    else {
      va_end(argptr);
      return false;
    }
    if ((i + 1) < slen && format[i + 1] == '|') {
      proceedOptional = true;
      i++;
    }
  }
  va_end(argptr);
  return true;
}
#endif
