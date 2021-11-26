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
bool ABasicFunction::ValidateState(const TStrObjList &Params, TMacroData &E) {
  const size_t argC = Params.Count(),
    arg_m = ((size_t)1 << argC);
  if (argC <= 14 &&
      (ArgStateMask&fpAny) < fpAny &&
      (ArgStateMask&arg_m) == 0)
  {
    E.WrongArgCount(*this, argC);
    return false;
  }
  // the special checks are in the high word
  if( (ArgStateMask&0xFFFF0000) &&
    !GetParentLibrary()->CheckProgramState(ArgStateMask) )
  {
    E.WrongState(*this);
    return false;
  }
  return true;
}
//.............................................................................
olxstr ABasicFunction::GetSignature() const {
  olxstr_buf res = GetName();
  res << " arguments [";
  const short arg_bits = 15;
  unsigned int ArgC = GetArgStateMask();
  short argc = 0, currentArg = 0;
  for (short i = 0; i < arg_bits; i++) {
    if (ArgC & (1 << i)) {
      argc++;
    }
  }
  if (argc > 5) {
    if (argc == arg_bits) {
      res << "any";
    }
    else {
      argc = arg_bits - argc;
      res << "any except ";
      for (short i = 0; i < arg_bits; i++) {
        if (!(ArgC & (0x001 << i))) {
          if (i == 0) {
            res << "none";
          }
          else res << i;
          if (currentArg < (argc - 2)) {
            res << ", ";
          }
          if (++currentArg == (argc - 1)) {
            res << " or ";
          }
        }
      }
    }
  }
  else {
    for (short i = 0; i < arg_bits; i++) {
      if (ArgC & (0x001 << i)) {
        if (i == 0) {
          res << "none";
        }
        else {
          res << i;
        }
        if (currentArg < (argc - 2)) {
          res << ", ";
        }
        if (++currentArg == (argc - 1)) {
          res << " or ";
        }
      }
    }
  }
  if ((ArgC&fpAny_Options) != 0) {
    res << "] [any options";
  }
  if ((ArgC & 0xffff0000) == 0xffff0000) {
    res << "] [any state]";
  }
  else if ((ArgC & 0xffff0000)) {
    res << "] states - ";
    if (GetParentLibrary()->GetOwner()) {
      res << GetParentLibrary()->GetOwner()->GetStateName(ArgC);
    }
    else {
      res << "illegal states, program may crash if this function is called";
    }
  }
  else {
    res << ']';
  }
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
  olxstr_dict<olxstr>& list)
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
  const olxstr_dict<olxstr>& list) const
{
  olxstr_buf rv;
  olxstr sep1 = '-', sep2 = "&;";
  for( size_t i=0; i < list.Count(); i++ )  {
    rv << list.GetKey(i);
    if( !list.GetValue(i).IsEmpty() )
      rv << sep1 << list.GetValue(i);
    rv << sep2;
  }
  return rv;
}
//.............................................................................
void AFunction::Run(const TStrObjList &Params, class TMacroData& E) {
  if( !ValidateState(Params, E) )  return;
  const size_t argC = Params.Count();
  try  {
    RunSignature = olxstr(GetName(), 128);
    RunSignature << '(';
    for( size_t i=0; i < argC; i++ )  {
      RunSignature << '[' << Params[i] << ']';
      if( i < (argC-1) )  RunSignature << ", ";
    }
    RunSignature << ')';
    DoRun(Params, E);
  }
  catch( TExceptionBase& exc )  {
    E.ProcessingException(*this, exc);
  }
};

//.............................................................................
void AMacro::Run(TStrObjList &Params, const TParamList &Options,
  TMacroData& E)
{
  if( !ValidateState(Params, E) )  return;
  const size_t argC = Params.Count();
  if ((GetArgStateMask()&fpAny_Options) != fpAny_Options) {
    for( size_t i=0; i < Options.Count(); i++ )  {
      if( ValidOptions.IndexOf(Options.GetName(i)) == InvalidIndex )  {
        E.WrongOption(*this, Options.GetName(i) );
        return;
      }
    }
  }
  try  {
    RunSignature = olxstr(GetName(), 128);
    RunSignature << ' ';
    for( size_t i=0; i < argC; i++ )  {
      RunSignature << '[' << Params[i] << ']';
      if( i < (argC-1) )  RunSignature << ", ";
    }
    RunSignature << ' ';
    for( size_t i=0; i < Options.Count(); i++ )  {
      RunSignature << '{' << Options.GetName(i) << '=' <<
        Options.GetValue(i) << '}';
    }
    DoRun(Params, Options, E);
  }
  catch( TExceptionBase& exc )  {
    E.ProcessingException(*this, exc);
  }
}
//.............................................................................
olxstr AMacro::GetSignature() const {
  if (ValidOptions.Count()) {
    olxstr res = ABasicFunction::GetSignature();
    if ((GetArgStateMask()&fpAny_Options) != 0)
      res << "; registered options - ";
    else
      res << "; valid options - ";
    for (size_t i=0; i < ValidOptions.Count(); i++)
      res << ValidOptions.GetKey(i)  << ';';
    return res;
  }
  else
    return ABasicFunction::GetSignature();
}
//.............................................................................
//.............................................................................
//.............................................................................
void FunctionChainer::RunMacro(TStrObjList &Params, const TParamList &Options,
    TMacroData& E)
{
  for (size_t i=functions.Count()-1; i != InvalidIndex; i--) {
    functions[i]->Run(Params, Options, E);
    if (E.IsHandled() || !E.IsSuccessful())
      break;
    if (!E.IsHandled() && (i+1) < functions.Count())
      E.SetUnhandled(false);
  }
  if (!E.IsHandled())
    E.ProcessingError(__OlxSourceInfo, "unhandled macro call");
}
//.............................................................................
void FunctionChainer::RunFunction(const TStrObjList &Params, TMacroData& E) {
  for (size_t i = functions.Count()-1; i != InvalidIndex; i--) {
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
  olxstr_dict<olxstr> options;
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
void FunctionChainer::Update(TFunction<FunctionChainer> &f) {
  uint32_t args = 0;
  TStrList description;
  description << NewLineSequence();
  for (size_t i = 0; i < functions.Count(); i++) {
    args |= functions[i]->GetArgStateMask();
    description.Add() << '#' << (i + 1);
    description.Add(functions[i]->GetDescription());
  }
  f.SetDescription(description.Text(NewLineSequence()));
  f.SetArgStateMask(args);
}
//.............................................................................
void FunctionChainer::Update(ABasicFunction *f) {
  if (dynamic_cast<TMacro<FunctionChainer> *>(f) != 0) {
    Update(*(TMacro<FunctionChainer> *)f);
  }
  else if (dynamic_cast<TFunction<FunctionChainer> *>(f) != 0) {
    Update(*(TFunction<FunctionChainer> *)f);
  }
  else {
    throw TFunctionFailedException(__OlxSourceInfo, "Unexpected object");
  }
}
