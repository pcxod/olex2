/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_macrolib
#define __olx_sdl_macrolib
#include "bapp.h"
#include "estrlist.h"
#include "datafile.h"
#include "dataitem.h"
#include "integration.h"
#include "library.h"
#include "estack.h"
#include "edict.h"

BeginEsdlNamespace()
namespace macrolib {

const uint8_t
  macro_log_macro    = 0x01,  //default log level
  macro_log_function = 0x02;

class TEMacro : public AMacro {
  TTypeList<exparse::expression_parser> Commands, OnAbort;
  TStrStrList Args;
public:
  TEMacro(const olxstr& name, const olxstr& desc);
  
  virtual void DoRun(TStrObjList &Params, const TParamList &Options,
    TMacroError& E);
  void AddCmd(const olxstr& cmd);
  void AddOnAbortCmd(const olxstr& cmd);
  void AddArg(const olxstr& name, const olxstr& val)  {
    Args.Add(name, val);
  }
  virtual ABasicFunction *Replicate() const;
};

class TEMacroLib {
  olex::IOlexProcessor& OlexProcessor;
  static bool is_allowed_in_name(olxch ch) {
    return (olxstr::o_isalphanumeric(ch) || ch == '_' || ch == '.');
  }
  uint8_t LogLevel;
  static olxstr_dict<void (*)(
    exparse::evaluator<exparse::expression_tree> *t,
    TMacroError& E, const TStrList &argv)> builtins;
  static olxstr SubstituteArgs(const olxstr &arg, const TStrList &argv,
    const olxstr &location=EmptyString());
protected:
/////////////////////////////////////////////////////////////////////////////////////////
  bool ExtractItemVal(const TDataItem& tdi, olxstr& val)  {  // helper function
    val = tdi.GetValue();
    if( val.IsEmpty() )
      val = tdi.GetFieldValue("cmd", EmptyString());
    return !val.IsEmpty();
  }
  void ParseMacro(const TDataItem& macro_def, TEMacro& macro);
  DefMacro(Abort)
  DefFunc(LastError)
  DefFunc(LogLevel)
  static void funIF(exparse::evaluator<exparse::expression_tree> *t,
    TMacroError& E, const TStrList &argv);
  static void funOr(exparse::evaluator<exparse::expression_tree> *t,
    TMacroError& E, const TStrList &argv);
  static void funAnd(exparse::evaluator<exparse::expression_tree> *t,
    TMacroError& E, const TStrList &argv);
  static void funNot(exparse::evaluator<exparse::expression_tree> *t,
    TMacroError& E, const TStrList &argv);
public:
  TEMacroLib(olex::IOlexProcessor& olexProcessor);
  ~TEMacroLib() {}
  void Init();  // extends the Library with functionality
  void Load(const TDataItem& m_root);
  
  typedef AnAssociation2<olxstr,olxstr> arg_t;
  static ABasicFunction *FindEvaluator(const olxstr &name, bool macro_first);
  static ABasicFunction *FindEvaluator(exparse::expression_tree *&e,
    olxstr &name);
  /* allow dummy - if the expression is not an evaluator - it returns unquoted
  version of the tree data, otherwise if the value of dummy is false - it
  reports non-existent macro/function
  */
  static olxstr ProcessEvaluator(exparse::expression_tree *e,
    TMacroError& me, const TStrList &argv, bool allow_dummy=false);
  static arg_t EvaluateArg(exparse::expression_tree *t,
    TMacroError& me, const TStrList &argv);
  /* if has_owner is true, then in the case the function does not exist no
  flags are set in E
  */
  bool ProcessFunction(olxstr& Cmd, TMacroError& E, bool has_owner,
    const TStrList &argv=TStrList());
  //..............................................................................
  template <class Base> void ProcessTopMacro(const olxstr& Cmd, TMacroError& Error,
    Base& base_instance, void (Base::*ErrorAnalyser)(TMacroError& error))
  {
    ProcessMacro(Cmd, Error);
    if (ErrorAnalyser != NULL)
      (base_instance.*ErrorAnalyser)(Error);
  }
  //..............................................................................
  void ProcessMacro(const olxstr& Cmd, TMacroError& Error,
    const TStrList &argv=TStrList());
  DefPropP(uint8_t, LogLevel)
};

} // end namespace macrolib
EndEsdlNamespace()
#endif