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
    olxstr rv(v[0]);
    return rv << ',' << v[1] << ',' << v[2];
  }
  static vec3d FloatVecFromStr(const olxstr& v)  {
    TStrList toks(v, ',');
    if( toks.Count() != 3 )
      throw TInvalidArgumentException(__OlxSourceInfo, "invalid vector size");
    return vec3d(toks[0].ToDouble(), toks[1].ToDouble(), toks[2].ToDouble());
  }
  static vec3i IntVecFromStr(const olxstr& v)  {
    TStrList toks(v, ',');
    if( toks.Count() != 3 )
      throw TInvalidArgumentException(__OlxSourceInfo, "invalid vector size");
    return vec3i(toks[0].ToInt(), toks[1].ToInt(), toks[2].ToInt());
  }
  template <class lc> static olxstr NumberListToStr(const lc& v)  {
    olxstr rv = v.IsEmpty() ? EmptyString() : olxstr(v[0]);
    for( size_t i=1; i < v.Count(); i++ )
      rv << ',' << v[i];
    return rv;
  }
  template <class lc> static lc& FloatNumberListFromStr(const olxstr& str, lc& v)  {
    TStrList toks(str, ',');
    for( size_t i=0; i < toks.Count(); i++ )
      v.Add( toks[i].ToDouble() );
    return v;
  }
  template <class lc> static lc& IntNumberListFromStr(const olxstr& str, lc& v)  {
    TStrList toks(str, ',');
    for( size_t i=0; i < toks.Count(); i++ )
      v.Add( toks[i].ToInt() );
    return v;
  }
  template <class vl> static olxstr VecArrayToStr(const vl& l, size_t count)  {
    olxstr rv;
    for( size_t i=0; i < count; )  {
      rv << VecToStr(l[i]);
      if( ++i < count )
        rv << ';';
    }
    return rv;
  }
  template <class vl> static olxstr VecListToStr(const vl& l)  {
    return VecArrayToStr(l, l.Count());
  }
  template <class vl>
  static vl& FloatVecListFromStr(const olxstr& v, vl& l)  {
    TStrList toks(v, ';');
    for( size_t i=0; i < toks.Count(); i++ )  {
      vec3d tv = FloatVecFromStr(toks[i]);
      l.AddNew(tv[0], tv[1], tv[2]);
    }
    return l;
  }
  template <class vl>
  static vl& FloatVecArrayFromStr(const olxstr& v, vl& l, size_t cnt)  {
    TStrList toks(v, ';');
    if (cnt != toks.Count())
      throw TFunctionFailedException(__OlxSourceInfo, "size mismatch");
    for( size_t i=0; i < toks.Count(); i++ )
      l[i] = FloatVecFromStr(toks[i]);
    return l;
  }
  template <class vl> static vl& IntVecListFromStr(const olxstr& v, vl& l)  {
    TStrList toks(v, ';');
    for( size_t i=0; i < toks.Count(); i++ )  {
      vec3i tv = IntVecFromStr(toks[i]);
      l.AddNew(tv[0], tv[1], tv[2]);
    }
    return l;
  }
};

EndEsdlNamespace()
#endif
