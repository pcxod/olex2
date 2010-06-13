#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "exception.h"

UseEsdlNamespace()

TIString IEObject::ToString() const {  throw TNotImplementedException(__OlxSourceInfo);  }
IEObject* IEObject::Replicate() const {  throw TNotImplementedException(__OlxSourceInfo);  }

#ifdef __WIN32__
  const char* EsdlObject(NewLineSequence) = "\r\n";
  const short EsdlObject(NewLineSequenceLength) = 2;
#else
  const char* EsdlObject(NewLineSequence) = "\n";
  const short EsdlObject(NewLineSequenceLength) = 1;
#endif

AReferencible::~AReferencible()  {
  if( This_RefCount != 0 )
    throw TFunctionFailedException(__OlxSourceInfo, "reference count is not zero");
}
//................................................................................................
//................................................................................................
//................................................................................................
TIString TExceptionBase::FormatSrc(const char* file, const char* func, int line)  {
  return olxstr(EmptyString, 384) << '[' << file << '(' << func << "):" << line << ']';
}
//................................................................................................
void TExceptionBase::ThrowFunctionFailed(const char* file, const char* func, int line, const char* msg) {
  throw TFunctionFailedException(FormatSrc(file,func,line), msg);
}
//................................................................................................
void TExceptionBase::ThrowInvalidIntegerFormat(const char* file, const char* func, int line, 
    const char* src, size_t src_len)
{
  throw TInvalidIntegerNumberException(FormatSrc(file,func,line),
    olxstr('\'') << olxstr(src, src_len) << '\'');
}
//................................................................................................
void TExceptionBase::ThrowInvalidIntegerFormat(const char* file, const char* func, int line, 
    const wchar_t* src, size_t src_len)
{
  throw TInvalidIntegerNumberException(FormatSrc(file,func,line),
    olxstr('\'') << olxstr(src, src_len) << '\'');
}
//................................................................................................
void TExceptionBase::ThrowInvalidFloatFormat(const char* file, const char* func, int line, 
    const char* src, size_t src_len)
{
  throw TInvalidFloatNumberException(FormatSrc(file,func,line),
    olxstr('\'') << olxstr(src, src_len) << '\'');
}
//................................................................................................
void TExceptionBase::ThrowInvalidFloatFormat(const char* file, const char* func, int line, 
    const wchar_t* src, size_t src_len)
{
  throw TInvalidFloatNumberException(FormatSrc(file,func,line),
    olxstr('\'') << olxstr(src, src_len) << '\'');
}
//................................................................................................
const TBasicException* TExceptionBase::GetException() const {
  const TBasicException* exc = dynamic_cast<const TBasicException*>(this);
  if( exc == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "invalid exception type");
  return exc;
}
