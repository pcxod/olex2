//---------------------------------------------------------------------------
#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "macroerror.h"
#include "function.h"

//..............................................................................
TMacroError::TMacroError()  {
  DeleteObject = false;
  ProcessError = 0;
  RetValue = NULL;
}
//..............................................................................
void TMacroError::operator = (const TMacroError& ME)  {
  ProcessError = ME.ProcessError;
  ErrorInfo = ME.ErrorInfo;
  RetValue = ME.RetValue;
}
//..............................................................................
olxstr& TMacroError::ProcessingError(const olxstr& location, const olxstr& errMsg)  {
  ErrorInfo = errMsg;
  ProcessError |= peProcessingError;
  Location = location;
  return ErrorInfo;
}
//..............................................................................
void TMacroError::NonexitingMacroError(const olxstr& macroName)  {
  ErrorInfo = "Macro/function '";
  ErrorInfo << macroName;
  ErrorInfo << "' does not exist";
  ProcessError |= peNonexistingFunction;
}
//..............................................................................
void TMacroError::WrongArgCount(const ABasicFunction& func, size_t ArgC)  {
  ErrorInfo = "Macro/function '";
  ErrorInfo << func.GetSignature() << "' is provided with " << (int)ArgC << " arguments";
  ProcessError |= peInvalidArgCount;
}
//..............................................................................
void TMacroError::ProcessingException(const ABasicFunction& caller, const TExceptionBase& Exc)  {
  ErrorInfo = caller.GetRuntimeSignature();
  const TBasicException* exc = Exc.GetException();
  if( exc->GetCause() == NULL )
    ErrorInfo << NewLineSequence << ' ' << exc->GetFullMessage();
  else  {
    TStrList output;
    exc->GetStackTrace(output);
    ErrorInfo << NewLineSequence << ' ' << output.Text(NewLineSequence);
  }
  ProcessError |= peProcessingException;
}
//..............................................................................
void TMacroError::WrongOption(const ABasicFunction& func, const olxstr& option)  {
  ErrorInfo = "Wrong option ";
  ErrorInfo << option << " for macro " << func.GetSignature();
  ProcessError |= peInvalidOption;
}
//..............................................................................
void TMacroError::WrongState(const ABasicFunction& func)  {
  ErrorInfo = "Wrong program state ";
  ErrorInfo << func.GetParentLibrary()->GetOwner()->GetStateName( func.GetArgStateMask() );
  ErrorInfo << " for macro/function " << func.GetSignature();
  ProcessError |= peIllegalState;
}
//..............................................................................
olxstr TMacroError::GetRetVal() const  {
  if( RetValue == NULL )  return EmptyString;
//  if( !EsdlInstanceOf(*RetValue, olxstr) )
//    throw TCastException(*this, EsdlObjectName(RetValue), EsdlClassName(olxstr) );
//  olxstr cn = EsdlObjectName(*RetValue);
//  return cn;
  return RetValue->ToString();
}

