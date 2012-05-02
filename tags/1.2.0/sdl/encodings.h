/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_encodings_H
#define __olx_encodings_H
#include "ebase.h"
BeginEsdlNamespace()

namespace encoding  {
  namespace base64  {
    static const char *_base64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
    inline olxcstr encode(const char* data, size_t len)  {
      size_t from = 0;
      olxcstr rv;
      while (len >= 3) { // encode full blocks first
        rv << _base64[(data[from] >> 2) & 0x3f] << 
              _base64[((data[from] << 4) & 0x30) | ((data[from+1] >> 4) & 0xf)];
        rv << _base64[((data[from+1] << 2) & 0x3c) | ((data[from+2] >> 6) & 0x3)] <<
              _base64[data[from+2] & 0x3f];
        from += 3;
        len -= 3;
      }
      if( len > 0 ) { // pad the remaining characters
        rv << _base64[(data[from] >> 2) & 0x3f];
        if (len == 1) {
          rv << _base64[(data[from] << 4) & 0x30] << '=';
        } 
        else {
          rv << _base64[((data[from] << 4) & 0x30)| ((data[from+1] >> 4) & 0xf)] <<
                _base64[(data[from+1] << 2) & 0x3c];
        }
        rv << '=';
      }
      return rv;
    }
    inline size_t _index_of(char ch)  {
      if( ch >= 'A' && ch <= 'Z' )  return ch-'A';
      if( ch >= 'a' && ch <= 'z' )  return ch-'a' + 26;
      if( ch >= '0' && ch <= '9' )  return ch-'0' + 52;
      if( ch == '+' )  return 62;
      if( ch == '/' )  return 63;
      if( ch == '=' )  return 64;
      throw TInvalidArgumentException(__OlxSourceInfo, "base64 char");
      return InvalidIndex;
    }
    inline olxcstr decode(const char* data, size_t len)  {
      if( (len%4) != 0 )
        throw TInvalidArgumentException(__OlxSourceInfo, "base64 length");
      olxcstr rv;
      for( size_t i=0; i < len; i+=4 )  {
        const size_t i1 = _index_of(data[i]);
        const size_t i2 = _index_of(data[i+1]);
        const size_t i3 = _index_of(data[i+2]);
        const size_t i4 = _index_of(data[i+3]);
        const char ch1 = (char)((i1 << 2) | (i2 >> 4));
        const char ch2 = (char)(((i2 & 0xf) << 4) | (i3 >> 2));
        const char ch3 = (char)(((i3 & 0x3) << 6) | i4);
        rv << ch1;
        if( i3 != 64 )  rv << ch2;
        if( i4 != 64 )  rv << ch3;
      }
      return rv;
    }
    inline olxcstr encode(const unsigned char* data, size_t len)  {
      return encode((const char*)data, len);
    }
    inline olxcstr decode(const unsigned char* data, size_t len)  {
      return encode((const char*)data, len);
    }
    inline olxcstr decode(const olxcstr& str)  {
      return decode(str.raw_str(), str.RawLen());
    }
    inline olxcstr encode(const olxcstr& str)  {
      return encode(str.raw_str(), str.RawLen());
    }
  } // end namespace base64
};

EndEsdlNamespace()
#endif
