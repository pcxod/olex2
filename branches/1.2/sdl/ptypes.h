/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_ptypes_H
#define __olx_sdl_ptypes_H
#include "egc.h"
BeginEsdlNamespace()

// primtive type wrapper template
template <class PT> class TEPType : public AReferencible  {
  PT Value;
public:
  TEPType( const PT& val ) : Value(val)  {}
  TEPType( const TEPType& val ) : Value(val.GetValue())  {}
  PT& Val()  {  return Value;  }
  PT& operator ()()  {  return Value;  }
  const PT& GetValue() const {  return Value;  }
  void SetValue(const PT& v)  {  Value = v;  }
  TIString ToString() const {   return olxstr(Value); }
  virtual IOlxObject* Replicate() const {  return new TEPType(*this);  }
};

template <> class TEPType<const wchar_t*> : public AReferencible  {
  olxstr Value;
public:
  TEPType(const wchar_t* val) : Value(val)  {}
  TEPType(const TEPType& val) : Value(val.GetValue())  {}
  olxstr& Val()  {  return Value;  }
  const olxstr& GetValue() const {  return Value;  }
  void SetValue(const wchar_t* v)  {  Value = v;  }
  TIString ToString() const {  return Value;  }
  virtual IOlxObject* Replicate() const {  return new TEPType(*this);  }
};

template <> class TEPType<const char*> : public AReferencible  {
  olxstr Value;
public:
  TEPType(const char* val) : Value(val)  {}
  TEPType(const TEPType& val) : Value(val.GetValue())  {}
  olxstr& Val()  {  return Value;  }
  const olxstr& GetValue() const {  return Value;  }
  void SetValue(const char* v)  {  Value = v;  }
  TIString ToString() const {  return Value;  }
  virtual IOlxObject* Replicate() const {  return new TEPType(*this);  }
};

template <> class TEPType<IOlxObject> : public AReferencible  {
  olxstr Value;
public:
  TEPType(const IOlxObject &val) : Value(val.ToString())  {}
  TEPType(const TEPType& val) : Value(val.GetValue())  {}
  olxstr& Val()  {  return Value;  }
  const olxstr& GetValue() const {  return Value;  }
  void SetValue(const char* v)  {  Value = v;  }
  TIString ToString() const {  return Value;  }
  virtual IOlxObject* Replicate() const {  return new TEPType(*this);  }
};

EndEsdlNamespace()
#endif
