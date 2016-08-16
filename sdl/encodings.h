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
#include "eutf8.h"
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
    static const char *base85() {
      static const char *_base85 =
        "0123456789abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ.-:+=^!/*?&<>()[]{}@%$#";
      return _base85;
    }
  };
  
  struct base64 {
    static olxcstr encode(const char* data, size_t len) {
      size_t from = 0;
      olxcstr rv(CEmptyString(), len*4/3+1);
      const char *_base64 = data::base64();
      while (len >= 3) { // encode full blocks first
        rv << _base64[(data[from] >> 2) & 0x3f] <<
          _base64[((data[from] << 4) & 0x30) | ((data[from + 1] >> 4) & 0xf)];
        rv << _base64[((data[from + 1] << 2) & 0x3c) |
          ((data[from + 2] >> 6) & 0x3)] << _base64[data[from + 2] & 0x3f];
        from += 3;
        len -= 3;
      }
      if (len > 0) { // pad the remaining characters
        rv << _base64[(data[from] >> 2) & 0x3f];
        if (len == 1) {
          rv << _base64[(data[from] << 4) & 0x30] << '=';
        }
        else {
          rv << _base64[((data[from] << 4) & 0x30) | ((data[from + 1] >> 4) & 0xf)] <<
            _base64[(data[from + 1] << 2) & 0x3c];
        }
        rv << '=';
      }
      return rv;
    }
    static size_t _index_of(char ch) {
      if (ch >= 'A' && ch <= 'Z') {
        return ch - 'A';
      }
      if (ch >= 'a' && ch <= 'z') {
        return ch - 'a' + 26;
      }
      if (ch >= '0' && ch <= '9') {
        return ch - '0' + 52;
      }
      if (ch == '+') {
        return 62;
      }
      if (ch == '/') {
        return 63;
      }
      if (ch == '=') {
        return 64;
      }
      throw TInvalidArgumentException(__OlxSourceInfo, "base64 char");
      return InvalidIndex;
    }
    static olxcstr decode(const char* data, size_t len) {
      if ((len % 4) != 0) {
        throw TInvalidArgumentException(__OlxSourceInfo, "base64 length");
      }
      olxcstr rv;
      for (size_t i = 0; i < len; i += 4) {
        const size_t i1 = _index_of(data[i]);
        const size_t i2 = _index_of(data[i + 1]);
        const size_t i3 = _index_of(data[i + 2]);
        const size_t i4 = _index_of(data[i + 3]);
        const char ch1 = (char)((i1 << 2) | (i2 >> 4));
        const char ch2 = (char)(((i2 & 0xf) << 4) | (i3 >> 2));
        const char ch3 = (char)(((i3 & 0x3) << 6) | i4);
        rv << ch1;
        if (i3 != 64) {
          rv << ch2;
        }
        if (i4 != 64) {
          rv << ch3;
        }
      }
      return rv;
    }
    static olxcstr encode(const uint8_t* data, size_t len) {
      return encode((const char*)data, len);
    }
    static olxcstr decode(const uint8_t* data, size_t len) {
      return encode((const char*)data, len);
    }
    static olxcstr decode(const olxcstr& str) {
      return decode(str.raw_str(), str.RawLen());
    }
    static olxcstr encode(const olxcstr& str) {
      return encode(str.raw_str(), str.RawLen());
    }
  }; // end struct base64

  /*
    For reference see:
    https://github.com/zeromq/rfc/blob/master/src/spec_32.c
  */
  struct base85 {
    static void encode32_(uint32_t x, olxcstr &out) {
      out << data::base85()[(x / (85 * 85 * 85 * 85)) % 85]
        << data::base85()[(x / (85 * 85 * 85)) % 85]
        << data::base85()[(x / (85 * 85)) % 85]
        << data::base85()[(x / 85) % 85]
        << data::base85()[x % 85];
    }
    static olxcstr encode(const uint8_t* data, size_t len) {
      size_t from = 0;
      olxcstr rv(CEmptyString(), len*5/4+2);
      const char *_base85 = data::base85();
      while (len >= 4) {
        uint32_t x = (uint32_t)data[from] << 24
          | (uint32_t) data[from + 1] << 16
          | (uint32_t)data[from + 2] << 8
          | (uint32_t)data[from + 3];
        encode32_(x, rv);
        from += 4;
        len -= 4;
      }
      if (len > 0) { // pad the remaining characters
        uint32_t x = (uint32_t)data[from] << 24;
        if (len > 1) {
          x |= (uint32_t)data[from + 1] << 16;
        }
        if (len > 2) {
          x |= (uint32_t)data[from + 2] << 8;
        }
        encode32_(x, rv);
        rv << len;
      }
      return rv;
    }
    static uint32_t _index_of(char ch) {
      static int8_t map85_to256[] = {
        -1,68,-1,84,83,82,72,-1,75,76,70,65,-1,63,62,69,0,1,2,3,4,5,6,7,8,9,64,
        -1,73,66,74,71,81,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,
        53,54,55,56,57,58,59,60,61,77,-1,78,67,-1,-1,10,11,12,13,14,15,16,17,
        18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,79,-1,80
      };
      ch -= 32;
      if (ch < 0 || map85_to256[ch] < 0) {
        throw TInvalidArgumentException(__OlxSourceInfo, "base85 char");
      }
      return map85_to256[ch];
    }
    static olxcstr decode(const char* data, size_t len) {
      if (len == 0) {
        return CEmptyString();
      }
      {
        size_t p = len % 5;
        if (p == 1) {
          if ((!olxstr::o_isdigit(data[len - 1]) || (data[len - 1] - '0') > 3)) {
            throw TInvalidArgumentException(__OlxSourceInfo, "base85 padding");
          }
        }
        else if (p != 0) {
          throw TInvalidArgumentException(__OlxSourceInfo, "base85 length");
        }
      }
      olxcstr rv(CEmptyString(), len*4/5+1);
      size_t l_ = (len / 5) * 5;
      for (size_t i = 0; i < l_; i += 5) {
        uint32_t x = _index_of(data[i]);
        for (int j = 1; j < 5; j++) {
          x = x * 85 + _index_of(data[i + j]);
        }
        rv << (char)((x>>24) & 0xFF)
          << (char)((x>>16) & 0xFF)
          << (char)((x>>8) & 0xFF)
          << (char)(x & 0xFF);
      }
      if ((len % 5) == 1) {
        rv.SetLength(rv.Length() - 4 + (size_t)(data[len-1]-'0'));
      }
      return rv;
    }
    static olxcstr encode(const char* data, size_t len) {
      return encode((const uint8_t *)data, len);
    }
    static olxcstr decode(const uint8_t* data, size_t len) {
      return encode(data, len);
    }
    static olxcstr decode(const olxcstr& str) {
      return decode(str.raw_str(), str.RawLen());
    }
    static olxcstr encode(const olxcstr& str) {
      return encode((const uint8_t *)str.raw_str(), str.RawLen());
    }
  }; // end struct base85

  struct base16 {
    static olxcstr encode(const uint8_t* data, size_t len, const char *base16) {
      size_t rv_len = len * 2 + 1;
      char *rv = olx_malloc<char>(rv_len);
      for (size_t i = 0; i < len; i++) {
        size_t idx = i << 1;
        rv[idx] = base16[data[i] >> 4];
        rv[idx + 1] = base16[data[i] & 0x0F];
      }
      rv[rv_len - 1] = '\0';
      olxcstr s = olxcstr::FromExternal(rv, rv_len);
      s.SetLength(s.Length() - 1);
      return s;
    }
    // inserts a hyphen each given bytes
    static olxcstr encode(const uint8_t* data, size_t len, const char *base16,
      size_t hyphen_each, char hypeh_char = '-')
    {
      if (hyphen_each == 0) {
        return encode(data, len, base16);
      }
      size_t rv_len = len * 2 + len / hyphen_each + ((len%hyphen_each) == 0 ? 0 : 1);
      char *rv = olx_malloc<char>(rv_len);
      for (size_t i = 0, j = 0; i < len; i++, j += 2) {
        rv[j] = base16[data[i] >> 4];
        rv[j + 1] = base16[data[i] & 0x0F];
        if (((i + 1) % hyphen_each) == 0 && (i + 1) < len) {
          rv[j + 2] = hypeh_char;
          j++;
        }
      }
      rv[rv_len - 1] = '\0';
      olxcstr s = olxcstr::FromExternal(rv, rv_len);
      s.SetLength(s.Length() - 1);
      return s;
    }
    /* decodes hex string in to dest, if the decoder encountered an invalid
    char it returns InvalidIndex, otherwise return the number of decoded chars,
    len at max
    */
    template <class T>
    static size_t decode(const T &in, char *dest, size_t dest_len) {
      size_t off=0;
      size_t len = in.Length();
      for (size_t i=0; i < len; i++) {
        olxch c = in[i];
        if (c >= '0' && c <= '9') {
          c -= '0';
        }
        else if (c >= 'a' && c <= 'z') {
          c -= ('a' - 10);
        }
        else if (c >= 'A' && c <= 'Z') {
          c -= ('A' - 10);
        }
        else if (c == '-') {
          continue;
        }
        else {
          return InvalidIndex;
        }
        size_t ro = off >> 1;
        if (ro >= dest_len) {
          return dest_len;
        }
        if ((off & 1) == 0) {
          dest[ro] = c << 4;
        }
        else {
          dest[ro] |= c;
        }
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

    static olxcstr encode(const olxcstr &data) {
      return encode((const uint8_t *)data.c_str(), data.Length(), data::base16());
    }
    static olxcstr decode(const olxcstr &data) {
      size_t len = data.Length() / 2 + 1;
      char *bf = olx_malloc<char>(len);
      size_t al = decode(data, bf, len);
      return olxcstr::FromExternal(bf, al, len);
    }
  }; // end struct base16

  struct percent {
    static olxcstr encode(const wchar_t* data, size_t len) {
      olxcstr us = TUtf8::Encode(data, len).Replace('%', "%25"),
        rv;
      rv.SetCapacity(us.Length());
      for (size_t i = 0; i < us.Length(); i++) {
        if (us.CharAt(i) < 33 || us.CharAt(i) > 126) {
          if (us.CharAt(i) == '%') {
            i += 2;
          }
          else {
            rv << '%' << data::base16()[((uint8_t)us.CharAt(i)) >> 4] <<
              data::base16()[((uint8_t)us.CharAt(i)) & 0x0F];
          }
        }
        else {
          rv << us.CharAt(i);
        }
      }
      return rv;
    }
    static olxstr decode(const uint8_t *data, size_t len) {
      olxcstr s;
      bool has_percents = false;
      s.SetCapacity(len);
      for (size_t i = 0; i < len; i++) {
        if (data[i] == '%') {
          if (i + 2 < len) {
            char ch = olxcstr::o_atoi<char>((const char*)&data[i + 1], 2, 16);
            if (ch != '%') {
              s << ch;
            }
            else {
              s << "%25";
              has_percents = true;
            }
            i += 2;
          }
        }
        else {
          s << (char)data[i];
        }
      }
      return TUtf8::Decode(has_percents ? s.Replace("%25", '%') : s);
    }
    static olxcstr encode(const olxstr &w) {
      return encode(w.u_str(), w.Length());
    }
    static olxstr decode(const olxcstr &data) {
      return decode((const uint8_t *)data.c_str(), data.Length());
    }
  }; // end of struct percent
};

EndEsdlNamespace()
#endif
