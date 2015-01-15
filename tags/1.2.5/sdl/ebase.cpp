/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "exception.h"
UseEsdlNamespace()

void *GlobalEsdlFunction(olx_malloc_)(size_t sz)  {
  void *r = malloc(sz);
  if( sz != 0 && r == NULL )
    throw TOutOfMemoryException(__OlxSourceInfo);
  return r;
}
//.............................................................................
void *GlobalEsdlFunction(olx_realloc_)(void *a, size_t sz)  {
  void *r = realloc(a, sz);
  if( sz != 0 && r == NULL )
    throw TOutOfMemoryException(__OlxSourceInfo);
  return r;
}
//.............................................................................
//.............................................................................
//.............................................................................
IEObject::~IEObject()  {
  while (dsh_head != NULL) {
    dsh_head->call(this);
    a_destruction_handler *dsh = dsh_head;
    dsh_head = dsh_head->next;
    delete dsh;
  }
}
//.............................................................................
bool IEObject::_HasDestructionHandler(
  IEObject::a_destruction_handler *dh) const
{
  a_destruction_handler *e = dsh_head;
  while (e != NULL) {
    if ((*e) == dh)
      return true;
    e = e->next;
  }
  return false;
}
//.............................................................................
TIString IEObject::ToString() const {
  throw TNotImplementedException(__OlxSourceInfo);
}
//.............................................................................
IEObject* IEObject::Replicate() const {
  throw TNotImplementedException(__OlxSourceInfo);
}
//.............................................................................
void IEObject::_RemoveDestructionHandler(const a_destruction_handler &dh) {
  a_destruction_handler *cr = dsh_head, *prev=NULL;
  while (cr != NULL) {
    if (dh == cr) {
      if (prev != NULL)  prev->next = cr->next;
      if (cr == dsh_tail)  dsh_tail = prev;
      if (dsh_head == cr)  dsh_head = NULL;
      delete cr;
      break;
    }
    prev = cr;
    cr = cr->next;
  }
}
//.............................................................................
bool IEObject::AddDestructionHandler(void (*func)(IEObject*)) {
  if (dsh_head == NULL) {
    dsh_head = dsh_tail = new static_destruction_handler(NULL, func);
  }
  else {
    a_destruction_handler *e = new static_destruction_handler(NULL, func);
    if (_HasDestructionHandler(e)) {
      delete e;
      return false;
    }
    dsh_tail->next = e;
    dsh_tail = e;
  }
  return true;
}
//.............................................................................
//.............................................................................
//.............................................................................
bool TExceptionBase::AutoLog = false;
#ifdef __WIN32__
  const TICString& EsdlObject(CNewLineSequence)()  {
    static olxcstr rv("\r\n");
    return rv;
  }
  const TIWString& EsdlObject(WNewLineSequence)()  {
    static olxwstr rv("\r\n");
    return rv;
  }
#else
  const TICString& EsdlObject(CNewLineSequence)()  {
    static olxcstr rv("\n");
    return rv;
  }
  const TIWString& EsdlObject(WNewLineSequence)()  {
    static olxwstr rv("\n");
    return rv;
  }
#endif
#ifdef _UNICODE
  const TIString& EsdlObject(NewLineSequence)()  {
    return WNewLineSequence();
  }
#else
  const TIString& EsdlObject(NewLineSequence)()  {
    return CNewLineSequence();
  }
#endif
//.............................................................................
AReferencible::~AReferencible()  {
  if( This_RefCount != 0 ) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "reference count is not zero");
  }
}
//.............................................................................
//.............................................................................
//.............................................................................
TIString TExceptionBase::FormatSrc(const char* file, const char* func,
  int line)
{
  return olxstr(EmptyString(), 384) << '[' << file << '(' << func << "):" <<
    line << ']';
}
//.............................................................................
void TExceptionBase::ThrowFunctionFailed(const char* file, const char* func,
  int line, const char* msg)
{
  throw TFunctionFailedException(FormatSrc(file,func,line), msg);
}
//.............................................................................
void TExceptionBase::ThrowIndexOutOfRange(const char* file, const char* func,
  int line, size_t index, size_t min_ind, size_t max_ind)
{
  throw TIndexOutOfRangeException(
    FormatSrc(file,func,line), index, min_ind, max_ind);
}
//.............................................................................
void TExceptionBase::ThrowInvalidUnsignedFormat(const char* file,
  const char* func, int line, const char* src, size_t src_len)
{
  throw TInvalidUnsignedNumberException(
    FormatSrc(file,func,line), olxstr().quote() << olxcstr(src, src_len));
}
//.............................................................................
void TExceptionBase::ThrowInvalidUnsignedFormat(const char* file,
  const char* func, int line, const wchar_t* src, size_t src_len)
{
  throw TInvalidUnsignedNumberException(
    FormatSrc(file,func,line), olxstr().quote() << olxwstr(src, src_len));
}
//.............................................................................
void TExceptionBase::ThrowInvalidIntegerFormat(const char* file,
  const char* func, int line, const char* src, size_t src_len)
{
  throw TInvalidIntegerNumberException(
    FormatSrc(file,func,line), olxstr().quote() << olxcstr(src, src_len));
}
//.............................................................................
void TExceptionBase::ThrowInvalidIntegerFormat(const char* file,
  const char* func, int line, const wchar_t* src, size_t src_len)
{
  throw TInvalidIntegerNumberException(
    FormatSrc(file,func,line), olxstr().quote() << olxwstr(src, src_len));
}
//.............................................................................
void TExceptionBase::ThrowInvalidFloatFormat(const char* file,
  const char* func, int line, const char* src, size_t src_len)
{
  throw TInvalidFloatNumberException(
    FormatSrc(file,func,line), olxstr().quote() << olxcstr(src, src_len));
}
//.............................................................................
void TExceptionBase::ThrowInvalidFloatFormat(const char* file,
  const char* func, int line, const wchar_t* src, size_t src_len)
{
  throw TInvalidFloatNumberException(
    FormatSrc(file,func,line), olxstr().quote() << olxwstr(src, src_len));
}
//.............................................................................
void TExceptionBase::ThrowInvalidBoolFormat(const char* file,
  const char* func, int line, const char* src, size_t src_len)
{
  throw TInvalidBoolException(
    FormatSrc(file,func,line), olxstr().quote() << olxcstr(src, src_len));
}
//.............................................................................
void TExceptionBase::ThrowInvalidBoolFormat(const char* file,
  const char* func, int line, const wchar_t* src, size_t src_len)
{
  throw TInvalidBoolException(
    FormatSrc(file,func,line), olxstr().quote() << olxwstr(src, src_len));
}
//.............................................................................
const TBasicException* TExceptionBase::GetException() const {
  const TBasicException* exc = dynamic_cast<const TBasicException*>(this);
  if( exc == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "invalid exception type");
  return exc;
}