/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_utf8_H
#define __olx_sdl_utf8_H
#include "ebase.h"
#include "ememstream.h"
BeginEsdlNamespace()

/* original code by Marius Bancila at
http://www.codeguru.com/cpp/misc/misc/multi-lingualsupport/article.php/c10451/
*/

#define  UTF8_MASKBITS                0x3F
#define  UTF8_MASKBYTE                0x80
#define  UTF8_MASK2BYTES              0xC0
#define  UTF8_MASK3BYTES              0xE0
#define  UTF8_MASK4BYTES              0xF0
#define  UTF8_MASK5BYTES              0xF8
#define  UTF8_MASK6BYTES              0xFC

class TUtf8  {
  olxcstr (*EncodeFunc)(const void* arr, size_t len);
  IDataOutputStream &(*EncodeStreamFunc)(const void* arr, size_t len,
    IDataOutputStream &);
  olxwstr (*DecodeFunc)(const char* arr, size_t len);
  size_t (*CharCountFunc)(const char* arr, size_t len);
  static TUtf8& GetInstance() {
    static TUtf8 i_;
    return i_;
  }

protected:
  TUtf8() {
    short sz = sizeof(wchar_t);
    if (sz == 2) {
      EncodeFunc = &Encode2;
      EncodeStreamFunc = &EncodeStream2;
      DecodeFunc = &Decode2;
      CharCountFunc = &CharCount2;
    }
    else if (sz == 4) {
      EncodeFunc = &Encode4;
      EncodeStreamFunc = &EncodeStream4;
      DecodeFunc = &Decode4;
      CharCountFunc = &CharCount4;
    }
  }
public:
  /* body defined in olx_istring.cpp
  */
  static olxcstr Encode(const olxcstr& str);
  static IDataOutputStream &Encode(const olxcstr& str,
    IDataOutputStream &out)
  { 
    return out << str;
  }
  static olxcstr Encode(const olxwstr& str) {
    olxcstr r = (*GetInstance().EncodeFunc)(str.raw_str(), str.Length());
    r.SetUTF8(true);
    return r;
  }

  static olxcstr Encode(const TTIString<wchar_t>& str) {
    olxcstr r = (*GetInstance().EncodeFunc)(str.raw_str(), str.Length());
    r.SetUTF8(true);
    return r;
  }

  static IDataOutputStream& Encode(const olxwstr& str,
    IDataOutputStream& out)
  {
    return (*GetInstance().EncodeStreamFunc)(str.raw_str(), str.Length(), out);
  }
  static IDataOutputStream& Encode(const TTIString<wchar_t>& str,
    IDataOutputStream& out)
  {
    return (*GetInstance().EncodeStreamFunc)(str.raw_str(), str.Length(), out);
  }

  static olxcstr Encode(const wchar_t* wstr, size_t len=InvalidSize)  {
    olxcstr r = (*GetInstance().EncodeFunc)(wstr,
      len == InvalidSize ? olxstr::o_strlen(wstr) : len);
    r.SetUTF8(true);
    return r;
  }
  static IDataOutputStream& Encode(const wchar_t* wstr,
    IDataOutputStream& out, size_t len=InvalidSize)
  {
    return (*GetInstance().EncodeStreamFunc)(wstr,
      len == InvalidSize ? olxstr::o_strlen(wstr) : len, out);
  }

  static olxwstr Decode(const olxwstr& str)  {  return str;  }
  static olxwstr Decode(const olxcstr& str)  {
    return (*GetInstance().DecodeFunc)(str.raw_str(), str.Length());
  }
  static olxwstr Decode(const TTIString<char>& str)  {
    return (*GetInstance().DecodeFunc)(str.raw_str(), str.Length());
  }
  static olxwstr Decode(const char* str, size_t len=InvalidSize) {
    return (*GetInstance().DecodeFunc)(str, len == InvalidSize ? olxstr::o_strlen(str) : len);
  }

  static size_t CharCount(const char * str, size_t len = InvalidSize) {
    return (*GetInstance().CharCountFunc)(str, len == InvalidSize ? olxstr::o_strlen(str) : len);
  }
  static size_t CharCount(const TTIString<char>& str) {
    return (*GetInstance().CharCountFunc)(str.raw_str(), str.Length());
  }
  static size_t CharCount(const olxcstr& str) {
    return (*GetInstance().CharCountFunc)(str.raw_str(), str.Length());
  }
  static size_t CharCount(const olxwstr& str) {
    return str.Length();
  }

  static uint32_t& GetFileSignature() {
    static uint32_t FileSignature = 0x00BFBBEF;
    return FileSignature;
  }

protected:  // functions below are unsafe to use if wchar_t size is unknown!!
  static IDataOutputStream& EncodeStream2(const void* vinput, size_t len,
    IDataOutputStream &out)
  {
    const uint16_t* input = (const uint16_t*)vinput;
    for (size_t i = 0; i < len; i++) {
      if (input[i] < 0x80) {  // 0xxxxxxx
        out << (uint8_t)input[i];
      }
      else if (input[i] < 0x800) {  // 110xxxxx 10xxxxxx
        out << (uint8_t)(UTF8_MASK2BYTES | (input[i] >> 6))
        << (uint8_t)((UTF8_MASKBYTE | input[i]) & UTF8_MASKBITS);
      }
      else { // if( input[i] < 0x10000 )  {  // 1110xxxx 10xxxxxx 10xxxxxx. always true
        out << (uint8_t)(UTF8_MASK3BYTES | (input[i] >> 12))
          << (uint8_t)((UTF8_MASKBYTE | (input[i] >> 6)) & UTF8_MASKBITS)
          << (uint8_t)((UTF8_MASKBYTE | input[i]) & UTF8_MASKBITS);
      }
    }
    return out;
  }

  static olxcstr Encode2(const void* vinput, size_t len)  {
    TEMemoryStream bf(len);
    EncodeStream2(vinput, len, bf).SetPosition(0);
    return bf.ToCString();
  }

  static olxwstr Decode2(const char* input, size_t len)  {
    TDirectionalList<wchar_t> bf(len);
    for (size_t i = 0; i < len; ) {
      uint16_t ch;
      if ((input[i] & UTF8_MASK3BYTES) == UTF8_MASK3BYTES) {  // 1110xxxx 10xxxxxx 10xxxxxx
        ch = ((input[i] & 0x0F) << 12) | (
          (input[i + 1] & UTF8_MASKBITS) << 6)
          | (input[i + 2] & UTF8_MASKBITS);
        i += 3;
      }
      else if ((input[i] & UTF8_MASK2BYTES) == UTF8_MASK2BYTES) {  // 110xxxxx 10xxxxxx
        ch = ((input[i] & 0x1F) << 6) | (input[i + 1] & UTF8_MASKBITS);
        i += 2;
      }
      else {  // if( input[i] < UTF8_MASKBYTE )  {  // 0xxxxxxx, always true
        ch = input[i];
        i += 1;
      }
      bf.Write(ch);
    }
    olxwstr str(WEmptyString(), bf.GetLength());
    bf.ToString(str);
    return str;
  }

  static size_t CharCount2(const char* input, size_t len) {
    size_t cnt = 0;
    for (size_t i = 0; i < len; cnt++) {
      if ((input[i] & UTF8_MASK3BYTES) == UTF8_MASK3BYTES) {  // 1110xxxx 10xxxxxx 10xxxxxx
        i += 3;
      }
      else if ((input[i] & UTF8_MASK2BYTES) == UTF8_MASK2BYTES) {  // 110xxxxx 10xxxxxx
        i += 2;
      }
      else {  // if( input[i] < UTF8_MASKBYTE )  {  // 0xxxxxxx, always true
        i += 1;
      }
    }
    return cnt;
  }

  static IDataOutputStream &EncodeStream4(const void* vinput, size_t len,
    IDataOutputStream &out)
  {
    const uint32_t* input = (const uint32_t*)vinput;
    for (size_t i = 0; i < len; i++) {
      if (input[i] < 0x80) {  // 0xxxxxxx
        out << (uint8_t)input[i];
      }
      else if (input[i] < 0x800) {  // 110xxxxx 10xxxxxx
        out << (uint8_t)(UTF8_MASK2BYTES | (input[i] >> 6))
          << (uint8_t)((UTF8_MASKBYTE | input[i]) & UTF8_MASKBITS);
      }
      else if (input[i] < 0x10000) {  // 1110xxxx 10xxxxxx 10xxxxxx
        out << (uint8_t)(UTF8_MASK3BYTES | (input[i] >> 12))
          << (uint8_t)((UTF8_MASKBYTE | (input[i] >> 6)) & UTF8_MASKBITS)
          << (uint8_t)((UTF8_MASKBYTE | input[i]) & UTF8_MASKBITS);
      }
      else if (input[i] < 0x200000) {  // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        out << (uint8_t)(UTF8_MASK4BYTES | (input[i] >> 18))
          << (uint8_t)((UTF8_MASKBYTE | (input[i] >> 12)) & UTF8_MASKBITS)
          << (uint8_t)((UTF8_MASKBYTE | (input[i] >> 6)) & UTF8_MASKBITS)
          << (uint8_t)((UTF8_MASKBYTE | input[i]) & UTF8_MASKBITS);
      }
      else if (input[i] < 0x4000000) {  // 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
        out << (uint8_t)(UTF8_MASK5BYTES | (input[i] >> 24))
          << (uint8_t)((UTF8_MASKBYTE | (input[i] >> 18)) & UTF8_MASKBITS)
          << (uint8_t)((UTF8_MASKBYTE | (input[i] >> 12)) & UTF8_MASKBITS)
          << (uint8_t)((UTF8_MASKBYTE | (input[i] >> 6)) & UTF8_MASKBITS)
          << (uint8_t)((UTF8_MASKBYTE | input[i]) & UTF8_MASKBITS);
      }
      else if (input[i] < 0x8000000) {  // 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
        out << (uint8_t)(UTF8_MASK6BYTES | (input[i] >> 30))
          << (uint8_t)((UTF8_MASKBYTE | (input[i] >> 18)) & UTF8_MASKBITS)
          << (uint8_t)((UTF8_MASKBYTE | (input[i] >> 12)) & UTF8_MASKBITS)
          << (uint8_t)((UTF8_MASKBYTE | (input[i] >> 6)) & UTF8_MASKBITS)
          << (uint8_t)((UTF8_MASKBYTE | input[i]) & UTF8_MASKBITS);
      }
    }
    return out;
  }

  static olxcstr Encode4(const void* vinput, size_t len) {
    TEMemoryStream bf(len);
    EncodeStream4(vinput, len, bf).SetPosition(0);
    return bf.ToCString();
  }

  static olxwstr Decode4(const char* input, size_t len) {
    TDirectionalList<wchar_t> bf(len);
    for (size_t i = 0; i < len; ) {
      uint32_t ch;
      if ((input[i] & UTF8_MASK6BYTES) == UTF8_MASK6BYTES) {  // 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
        ch = ((input[i] & 0x01) << 30) | ((input[i + 1] & UTF8_MASKBITS) << 24)
          | ((input[i + 2] & UTF8_MASKBITS) << 18) | ((input[i + 3]
            & UTF8_MASKBITS) << 12)
          | ((input[i + 4] & UTF8_MASKBITS) << 6) | (input[i + 5] & UTF8_MASKBITS);
        i += 6;
      }
      else if ((input[i] & UTF8_MASK5BYTES) == UTF8_MASK5BYTES) {  // 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
        ch = ((input[i] & 0x03) << 24) | ((input[i + 1]
          & UTF8_MASKBITS) << 18)
          | ((input[i + 2] & UTF8_MASKBITS) << 12) | ((input[i + 3]
            & UTF8_MASKBITS) << 6)
          | (input[i + 4] & UTF8_MASKBITS);
        i += 5;
      }
      else if ((input[i] & UTF8_MASK4BYTES) == UTF8_MASK4BYTES) {  // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        ch = ((input[i] & 0x07) << 18) | ((input[i + 1]
          & UTF8_MASKBITS) << 12)
          | ((input[i + 2] & UTF8_MASKBITS) << 6) | (input[i + 3] & UTF8_MASKBITS);
        i += 4;
      }
      else if ((input[i] & UTF8_MASK3BYTES) == UTF8_MASK3BYTES) {  // 1110xxxx 10xxxxxx 10xxxxxx
        ch = ((input[i] & 0x0F) << 12) | ((input[i + 1] & UTF8_MASKBITS) << 6)
          | (input[i + 2] & UTF8_MASKBITS);
        i += 3;
      }
      else if ((input[i] & UTF8_MASK2BYTES) == UTF8_MASK2BYTES) {  // 110xxxxx 10xxxxxx
        ch = ((input[i] & 0x1F) << 6) | (input[i + 1] & UTF8_MASKBITS);
        i += 2;
      }
      else {  // if( input[i] < UTF8_MASKBYTE )  {  // 0xxxxxxx, always true
        ch = input[i];
        i += 1;
      }
      bf.Write(ch);
    }
    olxwstr str(WEmptyString(), bf.GetLength());
    bf.ToString(str);
    return str;
  }

  static size_t CharCount4(const char* input, size_t len) {
    size_t cnt = 0;
    for (size_t i = 0; i < len; cnt++) {
      if ((input[i] & UTF8_MASK6BYTES) == UTF8_MASK6BYTES) {  // 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
        i += 6;
      }
      else if ((input[i] & UTF8_MASK5BYTES) == UTF8_MASK5BYTES) {  // 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
        i += 5;
      }
      else if ((input[i] & UTF8_MASK4BYTES) == UTF8_MASK4BYTES) {  // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        i += 4;
      }
      else if ((input[i] & UTF8_MASK3BYTES) == UTF8_MASK3BYTES) {  // 1110xxxx 10xxxxxx 10xxxxxx
        i += 3;
      }
      else if ((input[i] & UTF8_MASK2BYTES) == UTF8_MASK2BYTES) {  // 110xxxxx 10xxxxxx
        i += 2;
      }
      else {  // if( input[i] < UTF8_MASKBYTE )  {  // 0xxxxxxx, always true
        i += 1;
      }
    }
    return cnt;
  }
};

EndEsdlNamespace()
#endif
