#include "exception.h"

UseEsdlNamespace()

TIString IEObject::ToString() const {  throw TNotImplementedException(__OlxSourceInfo);  }
IEObject* IEObject::Replicate() const {  throw TNotImplementedException(__OlxSourceInfo);  }
bool TExceptionBase::AutoLog = false;
#ifdef __WIN32__
  const TICString EsdlObject(CNewLineSequence) = olxcstr("\r\n");
  const TIWString EsdlObject(WNewLineSequence) = olxwstr("\r\n");
#else
  const TICString EsdlObject(CNewLineSequence) = olxcstr("\n");
  const TIWString EsdlObject(WNewLineSequence) = olxwstr("\n");
#endif
#ifdef _UNICODE
  const TIString& EsdlObject(NewLineSequence) = WNewLineSequence;
#else
  const TIString& EsdlObject(NewLineSequence) = CNewLineSequence;
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
void TExceptionBase::ThrowIndexOutOfRange(const char* file, const char* func, int line,
  size_t index, size_t min_ind, size_t max_ind)
{
  throw TIndexOutOfRangeException(FormatSrc(file,func,line), index, min_ind, max_ind);
}
//................................................................................................
void TExceptionBase::ThrowInvalidUnsignedFormat(const char* file, const char* func, int line, 
    const char* src, size_t src_len)
{
  throw TInvalidUnsignedNumberException(FormatSrc(file,func,line),
    olxstr('\'') << olxstr(src, src_len) << '\'');
}
//................................................................................................
void TExceptionBase::ThrowInvalidUnsignedFormat(const char* file, const char* func, int line, 
    const wchar_t* src, size_t src_len)
{
  throw TInvalidUnsignedNumberException(FormatSrc(file,func,line),
    olxstr('\'') << olxstr(src, src_len) << '\'');
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
