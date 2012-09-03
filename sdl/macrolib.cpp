/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include <stdlib.h>
#include "macrolib.h"

using namespace exparse::parser_util;
using namespace macrolib;
//.............................................................................
//.............................................................................
//.............................................................................
TEMacro::TEMacro(const olxstr& name, const olxstr& desc)
  : AMacro(name, EmptyString(), fpAny, desc)
{

}
void TEMacro::AddCmd(const olxstr& cmd)  {
  Commands.AddNew(cmd).root->expand_cmd();
}
void TEMacro::AddOnAbortCmd(const olxstr& cmd)  {
  OnAbort.AddNew(cmd).root->expand_cmd();
}
void TEMacro::DoRun(TStrObjList &Params, const TParamList &Options,
  TMacroError& E)
{
  TStrList args;
  args.SetCapacity(Args.Count());
  olex::IOlexProcessor *ip = olex::IOlexProcessor::GetInstance();
  olxstr location = __OlxSourceInfo;
  for (size_t i=0; i < Args.Count(); i++) {
    if (i < Params.Count()) {
      ip->processFunction(args.Add(Params[i]), location);
    }
    else {
      ip->processFunction(args.Add(Args.GetObject(i)), location);
    }
  }
  for (size_t i=0; i < Commands.Count(); i++) {
    TEMacroLib::ProcessEvaluator(Commands[i].root, E, args);
    if (!E.IsSuccessful())
      break;
  }
  if (!E.IsSuccessful() && !OnAbort.IsEmpty()) {
    E.ClearErrorFlag();
    for (size_t i=0; i < OnAbort.Count(); i++) {
      TEMacroLib::ProcessEvaluator(OnAbort[i].root, E, args);
      if (!E.IsSuccessful())
        break;
    }
  }
}
ABasicFunction *TEMacro::Replicate() const {
  throw 1;
  return NULL;
}
//.............................................................................
//.............................................................................
//.............................................................................
olxstr TEMacroLib::SubstituteArgs(const olxstr &arg_, const TStrList &argv,
  const olxstr &location)
{
  if (is_quoted(arg_) || argv.IsEmpty()) return arg_;
  olxstr arg(arg_);
  size_t index = arg.FirstIndexOf('%');  // argument by index
  while (index != InvalidIndex && index < (arg.Length()-1)) {
    size_t iindex = index;
    while (++iindex < arg.Length() && olxstr::o_isdigit(arg.CharAt(iindex)))
      ;
    olxstr args = arg.SubString(index+1, iindex-index-1);
    if (!args.IsEmpty()) {
      size_t pindex = args.ToSizeT()-1;  // index of the parameter
      // check if valid argument index
      if (pindex < argv.Count()) {
        arg.Delete(index, args.Length()+1); // delete %xx value
        arg.Insert(argv[pindex], index);  // insert value parameter
        index += argv[pindex].Length();
      }
      else  {
        TBasicApp::NewLogEntry(logError) << location << ": wrong argument "
          "index - " << (pindex+1);
      }
    }
    if (++index < arg.Length())
      index = arg.FirstIndexOf('%', index);
    else
      break;
  }
  return arg;
}
//.............................................................................
ABasicFunction *TEMacroLib::FindEvaluator(const olxstr &name, bool macro_first) {
  olex::IOlexProcessor *ip = olex::IOlexProcessor::GetInstance();
  ABasicFunction *f = macro_first ? ip->GetLibrary().FindMacro(name)
    : ip->GetLibrary().FindFunction(name);
  if (f == NULL) {
    f = macro_first ? ip->GetLibrary().FindFunction(name)
      : ip->GetLibrary().FindMacro(name);
  }
  return f;
}
//.............................................................................
ABasicFunction *TEMacroLib::FindEvaluator(exparse::expression_tree *&e,
  olxstr &name)
{
  if (e->evator != NULL)
    return FindEvaluator(name = e->evator->name, e->macro_call);
  return FindEvaluator(name = e->data, e->macro_call);
}
//.............................................................................
olxstr TEMacroLib::ProcessEvaluator(
  exparse::expression_tree *e, TMacroError& me,
  const TStrList &argv)
{
  me.GetStack().Push(e->ToStringBuffer());
  olxstr name = e->evator == NULL ? e->data : e->evator->name;
  size_t bi_ind = builtins.IndexOf(name);
  if (bi_ind != InvalidIndex && e->evator != NULL) {
    (*builtins.GetValue(bi_ind))(e->evator, me, argv);
  }
  else {
    ABasicFunction *f = FindEvaluator(e, name);
    if (f == NULL) {
      if (e->evator == NULL)
        return unquote(SubstituteArgs(e->data, argv));
      me.NonexitingMacroError(name);
      return EmptyString();
    }
    TStrObjList Cmds;
    TParamList Options;
    if (e->evator != NULL) {
      for (size_t i=0; i < e->evator->args.Count(); i++) {
        arg_t r = EvaluateArg(e->evator->args[i], me, argv);
        if (!me.IsSuccessful())
          return EmptyString();
        if (r.GetA().IsEmpty()) {
          Cmds.Add(r.GetB());
        }
        else {
          Options.AddParam(r.GetA(), r.GetB());
        }
      }
    }
    if (f->HasOptions()) {
      Cmds.Pack();
      f->Run(Cmds, Options, me);
    }
    else
      f->Run(Cmds, me);
  }
  if (me.IsSuccessful()) {
    olxstr rv = me.GetRetVal();
    if (e->left != NULL)
      rv = EvaluateArg(e->left, me, argv).B() << rv;
    if (e->right != NULL) {
      rv << EvaluateArg(e->right, me, argv).GetB();
    }
    me.GetStack().Pop();
    return rv;
  }
  return EmptyString();
}
//.............................................................................
TEMacroLib::arg_t TEMacroLib::EvaluateArg(exparse::expression_tree *t,
  TMacroError& me, const TStrList &argv)
{
  if (t->data.StartsFrom('-')) {
    if (t->data.IsNumber())
      return arg_t(EmptyString(), t->data);
    size_t ei = t->data.IndexOf('=');
    if (ei != InvalidIndex)
      return arg_t(t->data.SubString(1, ei-1), t->data.SubStringFrom(ei+1));
    return arg_t(t->data.SubStringFrom(1), EmptyString());
  }
  if (t->evator)
    return arg_t(EmptyString(), ProcessEvaluator(t, me, argv));
  olxstr val;
  if (t->priority) val << '(';
  if (t->left) val << ProcessEvaluator(t->left, me, argv);
  val << unquote(SubstituteArgs(t->data, argv));
  if (t->priority) val << ')';
  if (t->right) val << ProcessEvaluator(t->right, me, argv);
  return arg_t(EmptyString(), val);
}
//.............................................................................
//.............................................................................
olxstr_dict<void (*)(
  exparse::evaluator<exparse::expression_tree> *t,
  TMacroError& E, const TStrList &argv)> TEMacroLib::builtins;
//.............................................................................
TEMacroLib::TEMacroLib(olex::IOlexProcessor& olexProcessor)
  : OlexProcessor(olexProcessor), LogLevel(macro_log_macro)
{
  if (builtins.IsEmpty()) {
    builtins.Add("if", &TEMacroLib::funIF);
    builtins.Add("and", &TEMacroLib::funAnd);
    builtins.Add("or", &TEMacroLib::funOr);
    builtins.Add("not", &TEMacroLib::funNot);
  }
}
//.............................................................................
void TEMacroLib::Load(const TDataItem& m_root)  {
  for( size_t i=0; i < m_root.ItemCount(); i++ )  {
    const TDataItem& m_def = m_root.GetItem(i);
    const TDataItem* di = m_def.FindItem("body");
    if( di == NULL )  {
      TBasicApp::NewLogEntry(logError) << "Macro body is not defined: " <<
        m_def.GetName();
      continue;
    }
    TEMacro* m = new TEMacro(m_def.GetName(), m_def.GetFieldValue("help"));
    OlexProcessor.GetLibrary().Register(m, libReplace);
    ParseMacro(*di, *m);
  }
}
//.............................................................................
void TEMacroLib::Init()  {
  TLibrary &lib = OlexProcessor.GetLibrary();
  lib.Register(
    new TFunction<TEMacroLib>(this,  &TEMacroLib::funLastError,
    "LastError", fpNone,
    "Returns last error"));
  lib.Register(
    new TFunction<TEMacroLib>(this,  &TEMacroLib::funLogLevel,
    "LogLevel", fpNone|fpOne,
    "Returns/sets log level, default is 'm' - for macro, accepts/returns 'm', "
    "'mf' or 'f'"));
  lib.Register(
    new TMacro<TEMacroLib>(this,  &TEMacroLib::macAbort, "Abort",
    EmptyString(), fpNone,
    "'abort' statement to terminate a macro execution"));
}
//.............................................................................

bool TEMacroLib::ProcessFunction(olxstr& Cmd, TMacroError& E, bool has_owner,
  const TStrList &argv)
{
  if (Cmd.IndexOf('(') == InvalidIndex) return true;
  if ((LogLevel&macro_log_function) != 0)
    TBasicApp::NewLogEntry(logInfo) << Cmd;
  E.GetStack().Push(Cmd);
  size_t specialFunctionIndex = Cmd.IndexOf('$');
  while( specialFunctionIndex != InvalidIndex &&
    is_escaped(Cmd, specialFunctionIndex) )
  {
    Cmd.Delete(specialFunctionIndex-1, 1);
    specialFunctionIndex = Cmd.FirstIndexOf('$', specialFunctionIndex);
  }
  if( specialFunctionIndex != InvalidIndex )  {
    size_t i=specialFunctionIndex;
    int bc = 0;
    bool funcStarted = false;
    while( ++i < Cmd.Length() )  {
      if( Cmd.CharAt(i) == '(' )  {  bc++;  funcStarted = true;  }
      else if( Cmd.CharAt(i) == ')' )  bc--;
      else if( !funcStarted && !is_allowed_in_name(Cmd.CharAt(i)) )  {
        specialFunctionIndex = next_unescaped('$', Cmd, i);
        if( specialFunctionIndex == InvalidIndex )  {
          E.GetStack().Pop();
          return true;
        }
        i = specialFunctionIndex;
        funcStarted = false;
        continue;
      }
      if( bc == 0 && funcStarted )  {
        olxstr spFunction =
          Cmd.SubString(specialFunctionIndex+1, i-specialFunctionIndex);
        if( ProcessFunction(spFunction, E, false, argv) )  {
          Cmd.Delete(specialFunctionIndex, i - specialFunctionIndex + 1);
          Cmd.Insert(spFunction, specialFunctionIndex);
        }
        else
          Cmd.Delete(specialFunctionIndex, i - specialFunctionIndex + 1);
        specialFunctionIndex = next_unescaped('$', Cmd, 0);
        if( specialFunctionIndex == InvalidIndex )  {
          E.GetStack().Pop();
          return true;
        }
        i = specialFunctionIndex;
        funcStarted = false;
        continue;
      }
    }
    return false;
  }
  exparse::expression_parser expr(Cmd);
  try {
    expr.root->expand_cmd();
  }
  catch (const TExceptionBase &e) {
    throw TFunctionFailedException(__OlxSourceInfo, e);
  }
  Cmd = ProcessEvaluator(expr.root, E, argv);
  return E.IsSuccessful();
}
//.............................................................................
void TEMacroLib::ProcessMacro(const olxstr& Cmd, TMacroError& Error,
  const TStrList &argv)
{
  if (Cmd.IsEmpty()) return;
  if ((GetLogLevel()&macro_log_macro) != 0)
    TBasicApp::NewLogEntry(logInfo) << Cmd;
  olxstr Command = olxstr(Cmd).TrimWhiteChars();
  if (Command.IsEmpty()) return;
  Error.GetStack().Push(Cmd);
  // processing environment variables
  size_t ind = Command.FirstIndexOf('|');
  while( ind != InvalidIndex )  {
    if( ind+1 >= Command.Length() )  break;
    size_t ind1 = Command.FirstIndexOf('|', ind+1);
    if( ind1 == InvalidIndex )  break;
    if( ind1 == ind+1 )  { // %%
      Command.Delete(ind1, 1);
      ind = Command.FirstIndexOf('|', ind1);
      continue;
    }
    const olxstr var_name = Command.SubString(ind+1, ind1-ind-1);
    const olxstr eval = olx_getenv(var_name);
    if( !eval.IsEmpty() )  {
      Command.Delete(ind, ind1-ind+1);
      Command.Insert(eval, ind);
      ind1 = ind + eval.Length();
    }
    else  // variable is not found - leave as it is
      ind1 = ind + var_name.Length();

    if( ind1+1 >= Command.Length() )  break;
    ind = Command.FirstIndexOf('|', ind1+1);
  }
  exparse::expression_parser expr(Command);
  expr.root->expand_cmd();
  ProcessEvaluator(expr.root, Error, argv);
  if( Error.IsSuccessful() )
    Error.GetStack().Pop();
  else if (Command.Contains('('))
    ProcessFunction(Command, Error, false, argv);
}
//.............................................................................
void TEMacroLib::ParseMacro(const TDataItem& macro_def, TEMacro& macro)  {
  olxstr Tmp;
  TDataItem* di = macro_def.FindItem("cmd");
  if( di != NULL )  {
    for( size_t i=0; i < di->ItemCount(); i++ )
      if( ExtractItemVal(di->GetItem(i), Tmp) )
        macro.AddCmd(Tmp);
  }
  di = macro_def.FindItem("args");
  if( di != NULL )  {
    for( size_t i=0; i < di->ItemCount(); i++ )  {
      const TDataItem& tdi = di->GetItem(i);
      macro.AddArg(tdi.GetFieldValue("name",
        EmptyString()), tdi.GetFieldValue("def"));
    }
  }
  di = macro_def.FindItem("onabort");
  if( di != NULL )  {
    for( size_t i=0; i < di->ItemCount(); i++ )
      if( ExtractItemVal(di->GetItem(i), Tmp) )
        macro.AddOnAbortCmd(Tmp);
  }
}
//.............................................................................
void TEMacroLib::funIF(exparse::evaluator<exparse::expression_tree> *t,
  TMacroError &me, const TStrList &argv)
{
  if (t->args.Count() < 2) {
    me.ProcessingError(__OlxSrcInfo,
      "incorrect syntax - condition and at least one command are expected");
    return;
  }
  arg_t r = EvaluateArg(t->args[0], me, argv);
  if (!me.IsSuccessful()) return;
  bool condition = r.GetB().ToBool();
  size_t bi=InvalidIndex;
  if (condition) {
    if (t->args.Count() == 2) { // if cond expr
      bi = 1;
    }
    // if cond then expr
    else if (t->args[1]->data.Equalsi("then"))
      bi = 2;
  }
  else {
    if (t->args.Count() > 2) {
      // if cond true_ex false_exp
      if (!t->args[1]->data.Equalsi("then")) {
        bi = 2;
      }
      // if cond then true_exp else false_exp
      else if (t->args.Count() == 5 && t->args[3]->data.Equalsi("else")) {
        bi = 4;
      }
      // if cond the true_exp
      else if (t->args.Count() == 3)
        bi = 0;
    }
  }
  // is none?
  if (bi == 0) return;
  if (bi == InvalidIndex) {
    me.ProcessingError(__OlxSrcInfo,
      "incorrect syntax - condition and at least one command are expected");
    return;
  }
  r = EvaluateArg(t->args[bi], me, argv);
  if (r.GetB().Equalsi("none")) return;
  TStrList toks = TParamList::StrtokLines(r.GetB(), ">>", false);
  for (size_t i=0; i < toks.Count(); i++) {
    exparse::expression_parser expr(toks[i]);
    expr.root->expand_cmd();
    ProcessEvaluator(expr.root, me, argv);
  }
}
//.............................................................................
void TEMacroLib::funAnd(exparse::evaluator<exparse::expression_tree> *t,
  TMacroError &E, const TStrList &argv)
{
  if (t->args.Count() != 2) {
    E.ProcessingError(__OlxSrcInfo, "two arguments are expected");
    return;
  }
  for( size_t i=0; i < t->args.Count(); i++ )  {
    arg_t a = EvaluateArg(t->args[i], E, argv);
    if (!E.IsSuccessful()) return;
    if (!a.GetB().ToBool()) {
      E.SetRetVal(false);
      return;
    }
  }
  E.SetRetVal(true);
}
//.............................................................................
void TEMacroLib::funOr(exparse::evaluator<exparse::expression_tree> *t,
  TMacroError &E, const TStrList &argv)
{
  if (t->args.Count() != 2) {
    E.ProcessingError(__OlxSrcInfo, "two arguments are expected");
    return;
  }
  for( size_t i=0; i < t->args.Count(); i++ )  {
    arg_t a = EvaluateArg(t->args[i], E, argv);
    if (!E.IsSuccessful()) return;
    if (a.GetB().ToBool()) {
      E.SetRetVal(true);
      return;
    }
  }
  E.SetRetVal(false);
}
//.............................................................................
void TEMacroLib::funNot(exparse::evaluator<exparse::expression_tree> *t,
  TMacroError &E, const TStrList &argv)
{
  if (t->args.Count() != 1) {
    E.ProcessingError(__OlxSrcInfo, "one argument is expected");
    return;
  }
  arg_t a = EvaluateArg(t->args[0], E, argv);
  if (!E.IsSuccessful()) return;
  E.SetRetVal(!a.GetB().ToBool());
}
//..............................................................................
void TEMacroLib::funLastError(const TStrObjList& Params, TMacroError &E) {
  E.SetRetVal(E.GetInfo());
}
//..............................................................................
void TEMacroLib::funLogLevel(const TStrObjList& Params, TMacroError &E) {
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
void TEMacroLib::macAbort(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  E.ProcessingError(__OlxSrcInfo, "abnormally terminated");
}
//.............................................................................
