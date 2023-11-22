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
  : AMacro(name, EmptyString(), fpAny_Options|fpAny, desc)
{

}
void TEMacro::AddCmd(const olxstr& cmd)  {
  Commands.AddNew(cmd).root->expand_cmd();
}
void TEMacro::AddOnAbortCmd(const olxstr& cmd)  {
  OnAbort.AddNew(cmd).root->expand_cmd();
}
void TEMacro::DoRun(TStrObjList &Params, const TParamList &Options,
  TMacroData& E)
{
  TStrList args;
  args.SetCapacity(Args.Count());
  olex2::IOlex2Processor *ip = olex2::IOlex2Processor::GetInstance();
  olxstr location = __OlxSourceInfo;
  for (size_t i = 0; i < Args.Count(); i++) {
    if (i < Params.Count()) {
      args.Add(Params[i]);
    }
    // processing needs to be done for the defaults only - the rest already have been
    else {
      ip->processFunction(args.Add(Args.GetObject(i)), location);
    }
  }
  for (size_t i = 0; i < Commands.Count(); i++) {
    TEMacroLib::ProcessEvaluator(Commands[i].root, E, args, &Options);
    if (!E.IsSuccessful()) {
      break;
    }
  }
  if (!E.IsSuccessful() && !OnAbort.IsEmpty()) {
    E.ClearErrorFlag();
    for (size_t i = 0; i < OnAbort.Count(); i++) {
      TEMacroLib::ProcessEvaluator(OnAbort[i].root, E, args, &Options);
      if (!E.IsSuccessful()) {
        break;
      }
    }
  }
}
ABasicFunction *TEMacro::Replicate() const {
  throw 1;
  return 0;
}
//.............................................................................
//.............................................................................
//.............................................................................
olxstr TEMacroLib::SubstituteArgs(const olxstr& arg_, const TStrList& argv,
  const olxstr& location)
{
  if (is_quoted(arg_) || argv.IsEmpty()) return arg_;
  olxstr arg(arg_);
  size_t index = arg.FirstIndexOf('%');  // argument by index
  while (index != InvalidIndex && index < (arg.Length() - 1)) {
    size_t iindex = index;
    while (++iindex < arg.Length() && olxstr::o_isdigit(arg.CharAt(iindex)))
      ;
    olxstr args = arg.SubString(index + 1, iindex - index - 1);
    if (!args.IsEmpty()) {
      size_t pindex = args.ToSizeT() - 1;  // index of the parameter
      // check if valid argument index
      if (pindex < argv.Count()) {
        arg.Delete(index, args.Length() + 1); // delete %xx value
        arg.Insert(argv[pindex], index);  // insert value parameter
        index += argv[pindex].Length();
      }
      else {
        TBasicApp::NewLogEntry(logError) << location << ": wrong argument "
          "index - " << (pindex + 1);
      }
    }
    if (++index < arg.Length()) {
      index = arg.FirstIndexOf('%', index);
    }
    else {
      break;
    }
  }
  return arg;
}
//.............................................................................
ABasicFunction *TEMacroLib::FindEvaluator(const olxstr &name, bool macro_first) {
  olex2::IOlex2Processor *ip = olex2::IOlex2Processor::GetInstance();
  ABasicFunction *f = macro_first ? ip->GetLibrary().FindMacro(name)
    : ip->GetLibrary().FindFunction(name);
  if (f == 0) {
    f = macro_first ? ip->GetLibrary().FindFunction(name)
      : ip->GetLibrary().FindMacro(name);
  }
  return f;
}
//.............................................................................
ABasicFunction* TEMacroLib::FindEvaluator(exparse::expression_tree*& e,
  olxstr& name)
{
  if (e->evator != 0) {
    return FindEvaluator(name = e->evator->name, e->macro_call);
  }
  return FindEvaluator(name = e->data, e->macro_call);
}
//.............................................................................
olxstr TEMacroLib::ProcessEvaluator(
  exparse::expression_tree *e, TMacroData& me, const TStrList &argv,
  const TParamList* global_options, bool allow_dummy)
{
  me.GetStack().Push(e->ToStringBuffer());
  olxstr name = e->evator == 0 ? e->data : e->evator->name;
  size_t bi_ind = GetBuiltins().IndexOf(name);
  if (bi_ind != InvalidIndex && e->evator != 0) {
    (*GetBuiltins().GetValue(bi_ind))(e->evator, me, argv);
  }
  else {
    ABasicFunction *f = FindEvaluator(e, name);
    if (f == 0) {
      if (e->evator == 0 && allow_dummy) {
        return unquote(SubstituteArgs(e->data, argv));
      }
      me.NonexitingMacroError(name);
      return EmptyString();
    }
    TStrObjList Cmds;
    TParamList Options;
    if (e->evator != 0) {
      bool math_module = name.StartsFromi("math.");
      for (size_t i = 0; i < e->evator->args.Count(); i++) {
        if (!math_module) {
          arg_t r = EvaluateArg(e->evator->args[i], me, argv);
          if (!me.IsSuccessful()) {
            return EmptyString();
          }
          if (r.GetA().IsEmpty()) {
            Cmds.Add(r.GetB());
          }
          else {
            Options.AddParam(r.GetA(), r.GetB());
          }
        }
        else {
          Cmds.Add(SubstituteArgs(e->evator->args[i]->data, argv));
        }
      }
    }
    if (f->HasOptions()) {
      Cmds.Pack();
      if (global_options != 0) {
        for (size_t i = 0; i < global_options->Count(); i++) {
          // do not overwrite local
          if (Options.Contains(global_options->GetName(i))) {
            continue;
          }
          Options.AddParam(global_options->GetName(i), global_options->GetValue(i));
        }
      }
      f->Run(Cmds, Options, me);
    }
    else {
      // put options back
      for (size_t oi = 0; oi < Options.Count(); oi++) {
        olxstr_buf bf('-');
        bf << Options.GetName(oi) << '=' << Options.GetValue(oi);
        Cmds.Add(bf);
      }
      f->Run(Cmds, me);
    }
  }
  if (me.IsSuccessful()) {
    olxstr rv;
    try {
      rv = me.GetRetVal();
      if (e->left != 0) {
        rv = EvaluateArg(e->left, me, argv).b << rv;
      }
      if (e->right != 0) {
        rv << EvaluateArg(e->right, me, argv).GetB();
      }
    }
    catch (const TExceptionBase &e) {
      TBasicApp::NewLogEntry(logInfo) << e.GetException()->GetFullMessage();
    }
    me.GetStack().Pop();
    return rv;
  }
  return EmptyString();
}
//.............................................................................
TEMacroLib::arg_t TEMacroLib::EvaluateArg(exparse::expression_tree* t,
  TMacroData& me, const TStrList& argv)
{
  if (t->data.StartsFrom('-')) {
    if (t->data.IsNumber() ||
      (t->data.Length() > 1 && olxstr::o_isdigit(t->data.CharAt(1))))
    {
      return arg_t(EmptyString(), t->data);
    }
    size_t ei = t->data.IndexOf('=');
    if (ei != InvalidIndex) {
      return arg_t(t->data.SubString(1, ei - 1),
        unquote(SubstituteArgs(t->data.SubStringFrom(ei + 1), argv)));
    }
    return arg_t(t->data.SubStringFrom(1), EmptyString());
  }
  if (t->data == '=') {
    if (t->left == 0 || t->left->data.IsEmpty()) {
      throw TInvalidArgumentException(__OlxSourceInfo, "option name");
    }
    olxstr n = t->left->data.StartsFrom('-') ? t->left->data.SubStringFrom(1)
      : t->left->data;
    olxstr v = t->right ? ProcessEvaluator(t->right, me, argv) : EmptyString();
    return arg_t(n, unquote(v));
  }
  if (t->evator != 0) {
    return arg_t(EmptyString(), ProcessEvaluator(t, me, argv));
  }
  olxstr val;
  if (t->priority) {
    val << '(';
  }
  if (t->left) {
    val << ProcessEvaluator(t->left, me, argv);
  }
  val << unquote(SubstituteArgs(t->data, argv));
  if (t->priority) {
    val << ')';
  }
  if (t->right) {
    val << ProcessEvaluator(t->right, me, argv);
  }
  return arg_t(EmptyString(), val);
}
//.............................................................................
//.............................................................................
//.............................................................................
TEMacroLib::TEMacroLib(olex2::IOlex2Processor& olexProcessor)
  : OlexProcessor(olexProcessor), LogLevel(macro_log_macro)
{
  if (GetBuiltins().IsEmpty()) {
    GetBuiltins().Add("if", &TEMacroLib::funIF);
    GetBuiltins().Add("and", &TEMacroLib::funAnd);
    GetBuiltins().Add("or", &TEMacroLib::funOr);
    GetBuiltins().Add("not", &TEMacroLib::funNot);
  }
}
//.............................................................................
void TEMacroLib::Load(const TDataItem& m_root) {
  for (size_t i = 0; i < m_root.ItemCount(); i++) {
    const TDataItem& m_def = m_root.GetItemByIndex(i);
    const TDataItem* di = m_def.FindItem("body");
    if (di == 0) {
      TBasicApp::NewLogEntry(logError) << "Macro body is not defined: " <<
        m_def.GetName();
      continue;
    }
    TEMacro* m = new TEMacro(m_def.GetName(), m_def.FindField("help"));
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
    new TFunction<TEMacroLib>(this,  &TEMacroLib::funProcess,
    "Process", fpOne,
    "Processes a function passed as the argument and returns the result"));
  lib.Register(
    new TMacro<TEMacroLib>(this,  &TEMacroLib::macAbort, "Abort",
    EmptyString(), fpNone,
    "'abort' statement to terminate a macro execution"));
  lib.Register(
    new TMacro<TEMacroLib>(this, &TEMacroLib::macCallback, "Callback",
    EmptyString(), (fpAny^fpNone)|fpAny_Options,
    "Internal use for calling registered callback functions"));
}
//.............................................................................
bool TEMacroLib::ProcessFunction(olxstr& Cmd, TMacroData& E, bool has_owner,
  const TStrList &argv)
{
  if (Cmd.IndexOf('(') == InvalidIndex) {
    if (is_quoted(Cmd)) {
      Cmd = unquote(Cmd);
    }
    return true;
  }
  if ((LogLevel & macro_log_function) != 0) {
    TBasicApp::NewLogEntry(logInfo) << Cmd;
  }
  E.GetStack().Push(Cmd);
  size_t specialFunctionIndex = Cmd.IndexOf('$');
  while( specialFunctionIndex != InvalidIndex &&
    is_escaped(Cmd, specialFunctionIndex) )
  {
    Cmd.Delete(specialFunctionIndex-1, 1);
    specialFunctionIndex = Cmd.FirstIndexOf('$', specialFunctionIndex);
  }
  if (specialFunctionIndex != InvalidIndex) {
    size_t i = specialFunctionIndex;
    int bc = 0;
    bool funcStarted = false;
    while (++i < Cmd.Length()) {
      olxch ch = Cmd.CharAt(i);
      if (is_quote(ch)) {
        if (!skip_string(Cmd, i)) {
          return false;
        }
        continue;
      }
      if (ch == '(' && !is_escaped(Cmd, i)) {
        bc++;
        funcStarted = true;
      }
      else if (ch == ')' && !is_escaped(Cmd, i)) {
        bc--;
      }
      else if (!funcStarted && !is_allowed_in_name(Cmd.CharAt(i))) {
        specialFunctionIndex = next_unescaped('$', Cmd, i);
        if (specialFunctionIndex == InvalidIndex) {
          E.GetStack().Pop();
          return true;
        }
        i = specialFunctionIndex;
        funcStarted = false;
        continue;
      }
      if (bc == 0 && funcStarted) {
        olxstr spFunction =
          Cmd.SubString(specialFunctionIndex + 1, i - specialFunctionIndex);
        if (ProcessFunction(spFunction, E, false, argv)) {
          Cmd.Delete(specialFunctionIndex, i - specialFunctionIndex + 1);
          Cmd.Insert(spFunction, specialFunctionIndex);
        }
        else {
          Cmd.Delete(specialFunctionIndex, i - specialFunctionIndex + 1);
        }
        specialFunctionIndex = next_unescaped('$', Cmd, 0);
        if (specialFunctionIndex == InvalidIndex) {
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
  if (is_quoted(Cmd)) {
    Cmd = unquote(Cmd);
    return true;
  }
  exparse::expression_parser expr(Cmd);
  try {
    expr.root->expand_cmd();
    Cmd = ProcessEvaluator(expr.root, E, argv);
    return E.IsSuccessful();
  }
  catch (const TExceptionBase &e) {
    E.ProcessingException(__OlxSourceInfo, e);
    return false;
  }
}
//.............................................................................
void TEMacroLib::ProcessMacro(const olxstr& Cmd, TMacroData& Error,
  const TStrList &argv)
{
  if (Cmd.IsEmpty()) {
    return;
  }
  if ((GetLogLevel() & macro_log_macro) != 0) {
    TBasicApp::NewLogEntry(logInfo) << Cmd;
  }
  olxstr Command = olxstr(Cmd).TrimWhiteChars();
  if (Command.IsEmpty()) {
    return;
  }
  // processing environment variables
  size_t ind = Command.FirstIndexOf('|');
  while (ind != InvalidIndex) {
    if (is_escaped(Command, ind)) {
      Command.Delete(ind - 1, 1);
      if (++ind >= Command.Length()) {
        break;
      }
      ind = Command.FirstIndexOf('|', ind);
      continue;
    }
    if (ind + 1 >= Command.Length()) {
      break;
    }
    size_t ind1 = Command.FirstIndexOf('|', ind+1);
    if (ind1 == InvalidIndex) break;
    if (is_escaped(Command, ind1)) {
      Command.Delete(ind1 - 1, 1);
      if (++ind1 >= Command.Length()) {
        break;
      }
      ind = Command.FirstIndexOf('|', ind1);
      continue;
    }
    if (ind1 == ind+1) { // leave these - could be operators
      if (++ind1 >= Command.Length()) {
        break;
      }
      ind = Command.FirstIndexOf('|', ind1);
      continue;
    }
    if (ind1 == InvalidIndex) {
      break;
    }
    const olxstr var_name = Command.SubString(ind+1, ind1-ind-1);
    const olxstr eval = olx_getenv(var_name);
    if (!eval.IsEmpty()) {
      Command.Delete(ind, ind1-ind+1);
      Command.Insert(eval, ind);
      ind1 = ind + eval.Length();
    }
    else { // variable is not found - leave as it is
      ind1 = ind + var_name.Length();
    }

    if (++ind1 >= Command.Length()) {
      break;
    }
    ind = Command.FirstIndexOf('|', ind1);
  }
  try {
    exparse::expression_parser expr(Command);
    expr.root->expand_cmd();
    ProcessEvaluator(expr.root, Error, argv);
  } 
  catch (const TExceptionBase &e) {
    Error.ProcessingException(__OlxSourceInfo, e);
  }
}
//.............................................................................
void TEMacroLib::ProcessMacro(const olxstr& Cmd, TStrObjList& args,
  const TParamList& options, TMacroData& me)
{
  olex2::IOlex2Processor* ip = olex2::IOlex2Processor::GetInstance();
  ABasicFunction *f = ip->GetLibrary().FindMacro(Cmd, (uint32_t)args.Count());
  if (f == 0) {
    me.NonexitingMacroError(Cmd);
    return;
  }
  try {
    f->Run(args, options, me);
  }
  catch (const TExceptionBase& e) {
    me.ProcessingException(__OlxSourceInfo, e);
  }
}
//.............................................................................
void TEMacroLib::ParseMacro(const TDataItem& macro_def, TEMacro& macro) {
  olxstr Tmp;
  TDataItem* di = macro_def.FindItem("cmd");
  if (di != 0) {
    for (size_t i = 0; i < di->ItemCount(); i++) {
      if (ExtractItemVal(di->GetItemByIndex(i), Tmp)) {
        macro.AddCmd(Tmp);
      }
    }
  }
  di = macro_def.FindItem("args");
  if (di != 0) {
    for (size_t i = 0; i < di->ItemCount(); i++) {
      const TDataItem& tdi = di->GetItemByIndex(i);
      macro.AddArg(tdi.FindField("name"), tdi.FindField("def"));
    }
  }
  di = macro_def.FindItem("onabort");
  if (di != 0) {
    for (size_t i = 0; i < di->ItemCount(); i++) {
      if (ExtractItemVal(di->GetItemByIndex(i), Tmp)) {
        macro.AddOnAbortCmd(Tmp);
      }
    }
  }
}
//.............................................................................
void TEMacroLib::funIF(exparse::evaluator<exparse::expression_tree>* t,
  TMacroData& me, const TStrList& argv)
{
  if (t->args.Count() < 2) {
    me.ProcessingError(__OlxSrcInfo,
      "incorrect syntax - condition and at least one command are expected");
    return;
  }
  arg_t r = EvaluateArg(t->args[0], me, argv);
  if (!me.IsSuccessful()) {
    return;
  }
  bool condition = r.GetB().ToBool();
  size_t bi = InvalidIndex;
  if (condition) {
    if (t->args.Count() == 2) { // if cond expr
      bi = 1;
    }
    // if cond then expr
    else if (t->args[1]->data.Equalsi("then")) {
      bi = 2;
    }
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
      else if (t->args.Count() == 3) {
        bi = 0;
      }
    }
  }
  // is none?
  if (bi == 0) {
    return;
  }
  if (bi == InvalidIndex) {
    me.ProcessingError(__OlxSrcInfo,
      "incorrect syntax - condition and at least one command are expected");
    return;
  }
  r = EvaluateArg(t->args[bi], me, argv);
  if (r.GetB().Equalsi("none") || r.GetB().IsBool()) {
    return;
  }
  TStrList toks = TParamList::StrtokLines(r.GetB(), ">>", false);
  for (size_t i = 0; i < toks.Count(); i++) {
    exparse::expression_parser expr(toks[i]);
    expr.root->expand_cmd();
    ProcessEvaluator(expr.root, me, argv, 0, true);
  }
}
//.............................................................................
void TEMacroLib::funAnd(exparse::evaluator<exparse::expression_tree> *t,
  TMacroData &E, const TStrList &argv)
{
  if (t->args.Count() < 2) {
    E.ProcessingError(__OlxSrcInfo, "at least two arguments are expected");
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
  TMacroData &E, const TStrList &argv)
{
  if (t->args.Count() < 2) {
    E.ProcessingError(__OlxSrcInfo, "at least two arguments are expected");
    return;
  }
  for( size_t i=0; i < t->args.Count(); i++ )  {
    arg_t a = EvaluateArg(t->args[i], E, argv);
    if (!E.IsSuccessful()) {
      return;
    }
    if (a.GetB().ToBool()) {
      E.SetRetVal(true);
      return;
    }
  }
  E.SetRetVal(false);
}
//.............................................................................
void TEMacroLib::funNot(exparse::evaluator<exparse::expression_tree> *t,
  TMacroData &E, const TStrList &argv)
{
  if (t->args.Count() != 1) {
    E.ProcessingError(__OlxSrcInfo, "one argument is expected");
    return;
  }
  arg_t a = EvaluateArg(t->args[0], E, argv);
  if (E.IsSuccessful()) {
    E.SetRetVal(!a.GetB().ToBool());
  }
}
//..............................................................................
void TEMacroLib::funLastError(const TStrObjList& Params, TMacroData &E) {
  E.SetRetVal(E.GetInfo());
}
//..............................................................................
void TEMacroLib::funLogLevel(const TStrObjList& Params, TMacroData &E) {
  if( Params.IsEmpty() )  {
    olxstr ll;
    if( (GetLogLevel()&macro_log_macro) != 0 )  ll << 'm';
    if( (GetLogLevel()&macro_log_function) != 0 )  ll << 'f';
    E.SetRetVal(ll);
  }
  else  {
    uint8_t ll = 0;
    if (Params[0].IndexOfi('m') != InvalidIndex) {
      ll |= macro_log_macro;
    }
    if (Params[0].IndexOfi('f') != InvalidIndex) {
      ll |= macro_log_function;
    }
    SetLogLevel(ll);
  }
}
//.............................................................................
void TEMacroLib::macAbort(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  E.ProcessingError(__OlxSrcInfo, "abnormally terminated");
}
//.............................................................................
void TEMacroLib::funProcess(const TStrObjList& Params, TMacroData &E) {
  olxstr cmd = Params[0];
  this->ProcessFunction(cmd, E, false);
}
//..............................................................................
void TEMacroLib::macCallback(TStrObjList& Params, const TParamList &opts,
  TMacroData &E)
{
  // will thi comment fix the SF svn?
  this->OlexProcessor.callCallbackFunc(Params[0],
    TStrList(Params.SubListFrom(1)));
}
//..............................................................................
