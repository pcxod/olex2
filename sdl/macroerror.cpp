/******************************************************************************
* Copyright (c) 2004-2026 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "macroerror.h"
#include "function.h"
#include "bapp.h"

TMacroData::TMacroData() {
  DeleteObject = false;
  ProcessError = 0;
  RetValue = 0;
}
//.............................................................................
void TMacroData::operator = (const TMacroData& ME) {
  ProcessError = ME.ProcessError;
  ErrorInfo = ME.ErrorInfo;
  RetValue = ME.RetValue;
}
//.............................................................................
olxstr& TMacroData::ProcessingError(const olxstr& location,
  const olxstr& errMsg)
{
  ErrorInfo = errMsg;
  ProcessError |= peProcessingError;
  Location = location;
  return ErrorInfo;
}
//.............................................................................
void TMacroData::NonexitingMacroError(const olxstr& macroName) {
  ErrorInfo = "Macro/function '";
  ErrorInfo << macroName;
  ErrorInfo << "' does not exist";
  ProcessError |= peNonexistingFunction;
}
//.............................................................................
void TMacroData::WrongArgCount(const ABasicFunction& func, size_t ArgC)  {
  ErrorInfo = "Macro/function '";
  ErrorInfo << func.GetSignature() << "' is provided with " << ArgC <<
    " arguments";
  ProcessError |= peInvalidArgCount;
}
//.............................................................................
void TMacroData::ProcessingException(const ABasicFunction& caller,
  const TExceptionBase& Exc)
{
  ErrorInfo = caller.GetRuntimeSignature();
  const TBasicException* exc = Exc.GetException();
  if (exc->GetCause() == 0) {
    ErrorInfo << NewLineSequence() << ' ' << exc->GetFullMessage();
  }
  else  {
    TStrList output;
    exc->GetStackTrace(output);
    ErrorInfo << NewLineSequence() << ' ' << output.Text(NewLineSequence());
  }
  ProcessError |= peProcessingException;
}
//.............................................................................
void TMacroData::ProcessingException(const olxstr& location,
  const TExceptionBase& Exc)
{
  (ErrorInfo = location) << ':' << NewLineSequence();
  const TBasicException* exc = Exc.GetException();
  TStrList output;
  ErrorInfo << ' ' << exc->GetStackTrace(output).Text(NewLineSequence());
  ProcessError |= peProcessingException;
}
//.............................................................................
void TMacroData::WrongOption(const ABasicFunction& func,
  const olxstr& option)
{
  ErrorInfo = "Wrong option ";
  ErrorInfo << option << " for macro " << func.GetSignature();
  ProcessError |= peInvalidOption;
}
//.............................................................................
void TMacroData::WrongState(const ABasicFunction& func)  {
  ErrorInfo = "Wrong program state ";
  ErrorInfo << func.GetParentLibrary()->GetOwner()->GetStateName(
    func.GetArgStateMask());
  ErrorInfo << " for macro/function " << func.GetSignature();
  ProcessError |= peIllegalState;
}
//.............................................................................
olxstr TMacroData::GetRetVal() const {
  if (RetValue == 0) {
    return EmptyString();
  }
  //  if( !EsdlInstanceOf(*RetValue, olxstr) )
  //    throw TCastException(*this, EsdlObjectName(RetValue), EsdlClassName(olxstr) );
  //  olxstr cn = EsdlObjectName(*RetValue);
  //  return cn;
  return RetValue->ToString();
}

void TMacroData::PrintStack(Logging logEvt, bool annotate,
  const olxstr &prefix) const
{
  const str_stack::item *i = Stack.TopItem();
  while (i != 0) {
    TBasicApp::NewLogEntry(logEvt, annotate) << prefix <<
      olxstr(i->data).TrimWhiteChars();
    i = i->prev;
  }
}
