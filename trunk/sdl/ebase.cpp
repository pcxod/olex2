#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "exception.h"

UseEsdlNamespace()
  const IEObject& esdl::NullObject = (const IEObject&)(*(IEObject*)NULL);

TIString IEObject::ToString() const  {
  throw TNotImplementedException(__OlxSourceInfo);
}

IEObject* IEObject::Replicate() const  {
  throw TNotImplementedException(__OlxSourceInfo);
}

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

void TExceptionBase::ThrowFunctionFailed(const char* file, const char* func, int line, const char* msg) {
  throw TFunctionFailedException( olxstr(EmptyString, 384) << '[' << file << '(' << func << "):" << line << ']', msg);
}

const TBasicException* TExceptionBase::GetException() const {
  const TBasicException* exc = dynamic_cast<const TBasicException*>(this);
  if( exc == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "invalid exception type");
  return exc;;
}
