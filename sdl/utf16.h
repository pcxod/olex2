/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_utf16_H
#define __olx_sdl_utf16_H
#include "ebase.h"
#include "talist.h"
BeginEsdlNamespace()
/*
  original code and reference:
  http://en.wikipedia.org/wiki/UTF-16#Example_UTF-16_encoding_procedure
*/
struct TUtf16 {

  static TArrayList<uint32_t>::const_list_type
    Decode(const uint16_t *bf, size_t sz)
  {
    TArrayList<uint32_t> rv(sz);
    size_t pos = 0, cnt = 0;
    while (pos < sz) {
      uint32_t a = bf[pos++];
      if (a >= 0xD800 && a <= 0xDBFF) {
        if (pos >= sz) {
          TBasicException::ThrowFunctionFailed(__POlxSourceInfo,
            "premature buffer ending");
        }
        uint32_t b = bf[pos++];
        if (b >= 0xDC00 && b <= 0xDFFF) {
          a =(a << 10) + b - 0x35FDC00;
        }
        else {
          pos--;
        }
      }
      rv[cnt++] = a;
    }
    return rv.SetCount(cnt);
  }

  static TArrayList<uint16_t>::const_list_type
    Encode(const uint32_t *bf, size_t sz)
  {
    TArrayList<uint16_t> rv;
    rv.SetCapacity(sz);
    size_t cnt = 0;
    for (size_t i = 0; i < sz; i++) {
      uint32_t v = bf[i];
      if (v < 0x10000) {
        rv << v;
      }
      else if (v <= 0x10FFFF) {
        rv << ((v >> 10) + 0xD7C0);
        rv << ((v & 0x3FF) + 0xDC00);
      }
      else {
        TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo,
          "invalid UTF16 char");
      }
    }
    return rv;
  }

  static size_t CharCount(const uint16_t *bf, size_t sz) {
    size_t pos = 0, cnt = 0;
    while (pos < sz) {
      uint32_t a = bf[pos++];
      if (a >= 0xD800 && a <= 0xDBFF) {
        if (pos >= sz) {
          TBasicException::ThrowFunctionFailed(__POlxSourceInfo,
            "premature buffer ending");
        }
        uint32_t b = bf[pos++];
        if (b >= 0xDC00 && b <= 0xDFFF) {
          ;
        }
        else {
          pos--;
        }
      }
      cnt++;
    }
    return cnt;
  }

};

EndEsdlNamespace()
#endif
