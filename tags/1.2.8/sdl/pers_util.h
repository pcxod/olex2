/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_persistence_util_H
#define __olx_sdl_persistence_util_H
#include "threex3.h"
#include "estrlist.h"
BeginEsdlNamespace()

namespace PersUtil {

  template <class vec> static olxstr VecToStr(const vec& v)  {
    olxstr rv = olxstr().Allocate(3*8+3);
    return rv << v[0] << ',' << v[1] << ',' << v[2];
  }

  template <class vec_t> static vec_t &VecFromStr(const olxstr& v, vec_t &rv)  {
    TStrList toks(v, ',');
    if( toks.Count() != 3 )
      throw TInvalidArgumentException(__OlxSourceInfo, "invalid vector size");
    toks[0].ToNumber(rv[0]);
    toks[1].ToNumber(rv[1]);
    toks[2].ToNumber(rv[2]);
    return rv;
  }

  template <class vec_t> static vec_t VecFromStr(const olxstr& v)  {
    vec_t rv;
    return VecFromStr(v, rv);
  }

  template <class lc> static olxstr NumberListToStr(const lc& v)  {
    return olxstr(',').Join(v);
  }

  template <class lc> static olxstr ComplexListToStr(const lc& v)  {
    typedef typename lc::list_item_type lit;
    return olxstr(',').Join(v,
      FunctionAccessor::MakeConst(&lit::ToString));
  }

  template <class vl> static olxstr VecArrayToStr(const vl& l, size_t count)  {
    olxstr_buf rv;
    olxstr sep = ';';
    for( size_t i=0; i < count; )  {
      rv << VecToStr(l[i]);
      if( ++i < count )
        rv << sep;
    }
    return rv;
  }
  template <class vl> static olxstr VecListToStr(const vl& l)  {
    return VecArrayToStr(l, l.Count());
  }

  template <class vl> static vl& VecListFromStr(const olxstr& v, vl& l)  {
    TStrList toks(v, ';');
    l.SetCount(toks.Count());
    for( size_t i=0; i < toks.Count(); i++ )
      VecFromStr(toks[i], l[i]);
    return l;
  }

  template <class vec_t>
  static vec_t *VecArrayFromStr(const olxstr& v, vec_t* l, size_t sz)  {
    TStrList toks(v, ';');
    if (sz != toks.Count())
      throw TInvalidArgumentException(__OlxSourceInfo, "array size");
    for( size_t i=0; i < sz; i++ )
      VecFromStr(toks[i], l[i]);
    return l;
  }

  template <class vl> static vl& NumberListFromStr(const olxstr& v, vl& l)  {
    TStrList toks(v, ',');
    l.SetCount(toks.Count());
    for( size_t i=0; i < toks.Count(); i++ )
      toks[i].ToNumber(l[i]);
    return l;
  }

  template <class vl> static vl& ComplexListFromStr(const olxstr& v, vl& l)  {
    TStrList toks(v, ',');
    l.SetCount(toks.Count());
    for (size_t i = 0; i < toks.Count(); i++)
      l[i] = toks[i];
    return l;
  }
};

EndEsdlNamespace()
#endif
