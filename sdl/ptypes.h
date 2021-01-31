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
#include "smart/ostring.h"
BeginEsdlNamespace()

// primtive type wrapper template
template <class PT>
class TEPType : public AReferencible {
  PT Value;
public:
  TEPType(const PT& val) : Value(val) {}
  TEPType(const TEPType& val) : Value(val.GetValue()) {}
  PT& Val() { return Value; }
  PT& operator ()() { return Value; }
  const PT& GetValue() const { return Value; }
  void SetValue(const PT& v) { Value = v; }
  TIString ToString() const { return olxstr(Value); }
  virtual IOlxObject* Replicate() const { return new TEPType(*this); }
};


template <class T>
AReferencible *createWrapper(const T &v) {
  return new TEPType<T>(v);
}


static AReferencible* createWrapper(const wchar_t* v) {
  return new TEPType<olxstr>(v);
}

static AReferencible* createWrapper(const char* v) {
  return new TEPType<olxstr>(v);
}

EndEsdlNamespace()
#endif
