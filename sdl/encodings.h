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
  struct data {
    static const char *base16() {
      static const char *_base16 = "0123456789abcdef";
      return _base16;
    }
    static const char *base16Capitals() {
      static const char *_base16 = "0123456789ABCDEF";
      return _base16;
    }
    static const char *base64() {
      static const char *_base64 =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
      return _base64;
    }
  };
  struct base64  {
    static olxcstr encode(const char* data, size_t len)  {
      size_t from = 0;
      olxcstr rv;
      const char *_base64 = data::base64();
      while (len >= 3) { // encode full blocks first
        rv << _base64[(data[from] >> 2) & 0x3f] <<
          _base64[((data[from] << 4) & 0x30) | ((data[from+1] >> 4) & 0xf)];
        rv << _base64[((data[from+1] << 2) & 0x3c) |
          ((data[from+2] >> 6) & 0x3)] << _base64[data[from+2] & 0x3f];
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
    static size_t _index_of(char ch)  {
      if( ch >= 'A' && ch <= 'Z' )  return ch-'A';
      if( ch >= 'a' && ch <= 'z' )  return ch-'a' + 26;
      if( ch >= '0' && ch <= '9' )  return ch-'0' + 52;
      if( ch == '+' )  return 62;
      if( ch == '/' )  return 63;
      if( ch == '=' )  return 64;
      throw TInvalidArgumentException(__OlxSourceInfo, "base64 char");
      return InvalidIndex;
    }
    static olxcstr decode(const char* data, size_t len)  {
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
    static olxcstr encode(const uint8_t* data, size_t len)  {
      return encode((const char*)data, len);
    }
    static olxcstr decode(const uint8_t* data, size_t len)  {
      return encode((const char*)data, len);
    }
    static olxcstr decode(const olxcstr& str)  {
      return decode(str.raw_str(), str.RawLen());
    }
    static olxcstr encode(const olxcstr& str)  {
      return encode(str.raw_str(), str.RawLen());
    }
  }; // end struct base64
  struct base16 {
    static olxcstr encode(const uint8_t* data, size_t len, const char *base16)  {
      size_t rv_len = len*2+1;
      char *rv = olx_malloc<char>(rv_len);
      for (size_t i=0; i < len; i++) {
        size_t idx = i<<1;
        rv[idx] = base16[data[i]>>4];
        rv[idx+1] = base16[data[i]&0x0F];
      }
      rv[rv_len-1] = '\0';
      olxcstr s = olxcstr::FromExternal(rv, rv_len);
      s.SetLength(s.Length()-1);
      return s;
    }
    // inserts a hyphen each given bytes
    static olxcstr encode(const uint8_t* data, size_t len, const char *base16,
      size_t hyphen_each, char hypeh_char='-')
    {
      if (hyphen_each == 0)
        return encode(data, len, base16);
      size_t rv_len = len*2+len/hyphen_each+((len%hyphen_each)==0 ? 0 : 1);
      char *rv = olx_malloc<char>(rv_len);
      for (size_t i=0, j=0; i < len; i++, j+=2) {
        rv[j] = base16[data[i] >> 4];
        rv[j+1] = base16[data[i]&0x0F];
        if (((i+1)%hyphen_each) == 0 && (i+1) < len) {
          rv[j+2] = hypeh_char;
          j++;
        }
      }
      rv[rv_len-1] = '\0';
      olxcstr s = olxcstr::FromExternal(rv, rv_len);
      s.SetLength(s.Length()-1);
      return s;
    }
    /* decodes hex string in to dest, if the decoder encountered an invalid
    char it returns InvalidIndex, otherwise return the number of decoded chars,
    len at max
    */
    template <class T>
    static size_t decode(const T &in, char *dest, size_t dest_len) {
      size_t off=0;
      size_t len = olxstr::o_strlen(in);
      for (size_t i=0; i < len; i++) {
        olxch c = in[i];
        if (c >= '0' && c <= '9')       c -= '0';
        else if (c >= 'a' && c <= 'z')  c -= ('a'-10);
        else if (c >= 'A' && c <= 'Z')  c -= ('A'-10);
        else if (c == '-')
          continue;
        else
          return InvalidIndex;
        size_t ro = off >> 1;
        if (ro >= dest_len)
          return dest_len;
        if ((off&1) == 0)
          dest[ro] = c << 4;
        else
          dest[ro] |= c;
        off++;
      }
      return (off>>1);
    }
    static olxcstr encode(const uint8_t *data, size_t len) {
      return encode(data, len, data::base16());
    }
    static olxcstr encode(const uint8_t *data, size_t len, size_t hyphen_each,
      char hyphen_char='-')
    {
      return encode(data, len, data::base16(),
        hyphen_each, hyphen_char);
    }
  }; // end struct base16
};

EndEsdlNamespace()
#endif
