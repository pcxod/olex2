/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "function.h"
#include "egc.h"
#include "bapp.h"
UseEsdlNamespace()

//.............................................................................
olxstr ABasicLibrary::GetQualifiedName() const {
  olxstr res = GetName();
  ABasicLibrary* lib = this->GetParentLibrary();
  while( lib && lib->GetParentLibrary() )  {
    res.Insert(lib->GetName() + '.', 0 );
    lib = lib->GetParentLibrary();
  }
  return res;
}
//.............................................................................
olxstr ABasicFunction::SubstituteArgs(const olxstr &arg_, const TStrList &argv,
  const olxstr &location)
{
  olxstr arg(arg_);
  size_t index = arg.FirstIndexOf('%');  // argument by index
  while( index != InvalidIndex && index < (arg.Length()-1) )  {
    size_t iindex = index;
    while (++iindex < arg.Length() && olxstr::o_isdigit(arg.CharAt(iindex)))
      ;
    olxstr args = arg.SubString(index+1, iindex-index-1);
    if( !args.IsEmpty() )  {
      size_t pindex = args.ToSizeT()-1;  // index of the parameter
      if( pindex < argv.Count() )  {  // check if valid argument index
        arg.Delete(index, args.Length()+1); // delete %xx value
        arg.Insert(argv[pindex], index);  // insert value parameter
      }
      else  {
        TBasicApp::NewLogEntry(logError) << location << ": wrong argument "
          "index - " << (pindex+1);
      }
    }
    if( index++ < arg.Length() )
      index = arg.FirstIndexOf('%', index);  // next argument by index
    else
      index = InvalidIndex;
  }
  return arg;
}
//.............................................................................
olxstr ABasicFunction::GetSignature() const {
  olxstr res(GetName());
  res << " arguments [";
  unsigned int ArgC = GetArgStateMask();
  short argc = 0, currentArg = 0;
  for( short i=0; i < 16; i++ )  if( ArgC & (1 << i) )  argc++;
  if( argc  > 5 )  {
    if( argc == 16 )  res << "any";
    else  {
      argc = 0;
      for( short i=0; i < 16; i++ )  if( !(ArgC & (1 << i)) )  argc++;
      res << "any except ";
     for( short i=0; i < 16; i++ )  {
        if( !(ArgC & (0x001 << i)) )  {
          if( !i )  res << "none";
          else      res << i;
          if( currentArg < (argc - 2) )    res << ", ";
          if( ++currentArg == (argc - 1) ) res << " or ";
        }
      }
    }
  }
  else  {
    for( short i=0; i < 16; i++ )  {
      if( ArgC & (0x001 << i) )  {
        if( !i )  res << "none";
        else      res << (i);
        if( currentArg < (argc - 2) )    res << ", ";
        if( ++currentArg == (argc - 1) ) res << " or ";
      }
    }
  }
  if( (ArgC & 0xffff0000) )  {
    res << "] states - ";
    if( this->GetParentLibrary()->GetOwner() )
      res << this->GetParentLibrary()->GetOwner()->GetStateName( ArgC );
    else
      res << "illegal states, program might crash if this function is called";
  }
  else
    res << ']';
  return res;
}
//.............................................................................
olxstr ABasicFunction::GetQualifiedName() const {
  olxstr res = GetName();
  ABasicLibrary* lib = this->GetParentLibrary();
  while( lib && lib->GetParentLibrary() )  {
    res.Insert(lib->GetName() + '.', 0 );
    lib = lib->GetParentLibrary();
  }
  return res;
}
//.............................................................................
void ABasicFunction::ParseOptions(const olxstr& Options,
  TCSTypeList<olxstr,olxstr>& list)
{
  if( Options.IsEmpty() )  return;
  TStrList toks(Options, "&;");
  for( size_t i=0; i < toks.Count(); i++ )  {
    size_t mi = toks[i].IndexOf('-');
    if( mi != InvalidIndex )
      list.Add( toks[i].SubStringTo(mi), olxstr(toks[i].SubStringFrom(mi+1)));
    else
      list.Add( toks[i], EmptyString());
  }
}
//.............................................................................
olxstr ABasicFunction::OptionsToString(
  const TCSTypeList<olxstr,olxstr>& list) const
{
  olxstr_buf rv;
  olxstr sep1 = '-', sep2 = "&;";
  for( size_t i=0; i < list.Count(); i++ )  {
    rv << list.GetKey(i);
    if( !list.GetObject(i).IsEmpty() )
      rv << sep1 << list.GetObject(i);
    rv << sep2;
  }
  return rv;
}
//.............................................................................
void ABasicFunction::MacroRun(const TStrObjList& Params, TMacroError& E,
  const TStrList &argv)
{
  if (argv.IsEmpty()) {
    Run(Params, E);
    return;
  }
  TStrObjList params(Params);
  for (size_t i=0; i < params.Count(); i++)
    params[i] = SubstituteArgs(Params[i], argv, GetName());
  Run(params, E);
}
//.............................................................................
void ABasicFunction::MacroRun(TStrObjList& Params, const TParamList& options,
  TMacroError& E, const TStrList &argv)
{
  if (argv.IsEmpty()) {
    Run(Params, options, E);
    return;
  }
  TStrObjList params(Params);
  TParamList opts(options);
  for (size_t i=0; i < params.Count(); i++)
    params[i] = SubstituteArgs(Params[i], argv, GetName());
  for (size_t i=0; i < opts.Count(); i++)
    opts.GetValue(i) = SubstituteArgs(options.GetValue(i), argv, GetName());
  params.Pack();
  Run(params, opts, E);
}
//.............................................................................
//.............................................................................
//.............................................................................
void FunctionChainer::RunMacro(TStrObjList &Params, const TParamList &Options,
    TMacroError& E)
{
  for (size_t i=0; i < functions.Count(); i++) {
    functions[i]->Run(Params, Options, E);
    if (E.IsHandled() || !E.IsSuccessful())
      break;
    if (!E.IsHandled() && (i+1) < functions.Count())
      E.SetUnhandled(false);
  }
  if (!E.IsHandled())
    E.ProcessingError(__OlxSourceInfo, "unhandled function call");
}
//.............................................................................
void FunctionChainer::RunFunction(const TStrObjList &Params, TMacroError& E) {
  for (size_t i=0; i < functions.Count(); i++) {
    functions[i]->Run(Params, E);
    if (E.IsHandled() || !E.IsSuccessful())
      break;
    if (!E.IsHandled() && (i+1) < functions.Count())
      E.SetUnhandled(false);
  }
  if (!E.IsHandled())
    E.ProcessingError(__OlxSourceInfo, "unhandled function call");
}
//.............................................................................
void FunctionChainer::Update(TMacro<FunctionChainer> &m) {
  uint32_t args=0;
  TCSTypeList<olxstr,olxstr> options;
  TStrList description;
  description << NewLineSequence();
  for (size_t i=0; i < functions.Count(); i++) {
    args |= functions[i]->GetArgStateMask();
    options.Merge(functions[i]->GetOptions());
    description.Add() << '#' << (i+1);
    description.Add(functions[i]->GetDescription());
  }
  m.SetDescription(description.Text(NewLineSequence()));
  m.SetArgStateMask(args);
  m.SetOptions(options);
}
//.............................................................................
