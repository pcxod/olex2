/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_hashing__H
#define __olx_hashing__H
#include "talist.h"
BeginEsdlNamespace()

struct HashingUtils  {
  static inline uint32_t hs_rotl(uint32_t v, uint32_t c)  {
    return (v << c) | (v >> (32-c));
  }
  static inline uint32_t hs_rotr(uint32_t v, uint32_t c)  {
    return (v >> c) | (v << (32-c));
  }
};
struct HashingUtilsLE : public HashingUtils {
  // operates on 64 byte blocks only, little endian
  static inline void hs_copy(const uint8_t* src, uint32_t* dest, size_t src_len)  {
    for( size_t i=0, j=0; i < src_len; i += 4, j++ )
      dest[j] = ((uint32_t)src[i]) |
      (((uint32_t)src[i+1]) << 8) |
      (((uint32_t)src[i+2]) << 16) |
      (((uint32_t)src[i+3]) << 24);
  }
  static inline void hs_copy(const uint32_t* src, uint8_t* dest, size_t src_len)  {
    for( size_t i=0, j=0; j < src_len; i += 4, j++ )  {
      dest[i] =   (char)src[j];
      dest[i+1] = (char)(src[j] >> 8);
      dest[i+2] = (char)(src[j] >> 16);
      dest[i+3] = (char)(src[j] >> 24);
    }
  }
  static inline void hs_write_len(uint8_t* dest, const uint64_t& len)  {
    for( int i=8; i >= 1; i-- )
      dest[64-i] = ((uint8_t*)&len)[8-i];
  }
};

struct HashingUtilsBE : public HashingUtils {
public:
  // operates on 64 byte blocks only, big endian
  static inline void hs_copy(const uint8_t* src, uint32_t* dest, size_t src_len)  {
    for( size_t i=0, j=0; i < src_len; i += 4, j++ )
      dest[j] = ((uint32_t)src[i+3]) |
      (((uint32_t)src[i+2]) << 8) |
      (((uint32_t)src[i+1]) << 16) |
      (((uint32_t)src[i]) << 24);
  }
  static inline void hs_copy(const uint32_t* src, uint8_t* dest, size_t src_len)  {
    for( size_t i=0, j=0; j < src_len; i += 4, j++ )  {
      dest[i+3] = (char)src[j];
      dest[i+2] = (char)(src[j] >> 8);
      dest[i+1] = (char)(src[j] >> 16);
      dest[i] =   (char)(src[j] >> 24);
    }
  }
  static inline void hs_write_len(uint8_t* dest, const uint64_t& len)  {
    for( int i=8; i >= 1; i-- )
      dest[64-i] = ((uint8_t*)&len)[i-1];
  }
};

template <class Impl, class Tools>
class HashingBase : public Impl  {
protected:
  uint32_t bf[16];
  uint8_t cbf[64];
  void DoRawDigest(const void* msg, size_t len) {
    const size_t blocks = len>>6;
    for (size_t i=0; i < blocks; i++) {
      Tools::hs_copy(&((const uint8_t*)msg)[i<<6], bf, 64);
      Impl::digest64(bf);
    }
    const size_t part = len&0x3F;
    memset(cbf, 0, 64);
    if (part > 0)
      memcpy(cbf, msg, part);
    cbf[part] |= (0x01 << 7);
    if (part > 56) { // will not the length fit?
      Tools::hs_copy(cbf, bf, 64);
      Impl::digest64(bf);
      memset(cbf, 0, 64);
    }
    uint64_t len_bits_o = len<<3;
    Tools::hs_write_len(cbf, len_bits_o);
    Tools::hs_copy(cbf, bf, 64);
    Impl::digest64(bf);
    Impl::finalise();
  }

  olxcstr DoDigest(const void* msg, size_t len) {
    DoRawDigest(msg, len);
    // prepeare the digest
    return Impl::formatDigest();
  }

  void DoRawDigest(IInputStream& stream) {
    uint8_t cbf[64];
    const uint64_t blocks = stream.GetSize() >> 6;
    for (uint64_t i=0; i < blocks; i++) {
      stream.Read(cbf, 64);
      Tools::hs_copy(cbf, bf, 64);
      Impl::digest64(bf);
    }
    const size_t part = stream.GetSize()&0x3F;
    memset(cbf, 0, 64);
    if (part > 0)
      stream.Read(cbf, part);
    cbf[part] |= (0x01 << 7);
    if (part >= 56) { // will not the length fit?
      Tools::hs_copy(cbf, bf, 64);
      Impl::digest64(bf);
      memset(cbf, 0, 64);
    }
    uint64_t len_bits_o = stream.GetSize()<<3;
    Tools::hs_write_len(cbf, len_bits_o);
    Tools::hs_copy(cbf, bf, 64);
    Impl::digest64(bf);
    Impl::finalise();
  }

  olxcstr DoDigest(IInputStream& stream) {
    DoRawDigest(stream);
    // prepeare the digest
    return Impl::formatDigest();
  }
  HashingBase() : Impl() {}
public:
  static olxcstr Digest(const olxcstr& str) {
    return HashingBase<Impl,Tools>().DoDigest(str.raw_str(), str.RawLen());
  }
  static olxcstr Digest(const olxwstr& str) {
    return HashingBase<Impl,Tools>().DoDigest(
      str.raw_str(), str.RawLen());
  }
  static olxcstr Digest(const char* str, size_t len) {
    return HashingBase<Impl,Tools>().DoDigest(str, len);
  }
  static olxcstr Digest(const wchar_t* str, size_t len)  {
    return HashingBase<Impl,Tools>().DoDigest(str, len*sizeof(wchar_t));
  }
  static olxcstr Digest(const char* str) {
    return Digest(str, olxstr::o_strlen(str));
  }
  static olxcstr Digest(const wchar_t* str) {
    return Digest(str, olxstr::o_strlen(str));
  }
  static olxcstr Digest(IInputStream& stream){
    return HashingBase<Impl,Tools>().DoDigest(stream);
  }

  static ConstArrayList<uint8_t> RawDigest(const olxcstr& str) {
    HashingBase<Impl,Tools> t;
    t.DoRawDigest(str.raw_str(), str.RawLen());
    return new TArrayList<uint8_t>(t.DigestSize(), t.GetDigest());
  }
  static ConstArrayList<uint8_t> RawDigest(const olxwstr& str) {
    HashingBase<Impl,Tools> t;
    t.DoRawDigest(str.raw_str(), str.RawLen());
    return new TArrayList<uint8_t>(t.DigestSize(), t.GetDigest());
  }
  static ConstArrayList<uint8_t> RawDigest(const char* str, size_t len) {
    HashingBase<Impl,Tools> t;
    t.DoRawDigest(str, len);
    return new TArrayList<uint8_t>(t.DigestSize(), t.GetDigest());
  }
  static ConstArrayList<uint8_t> RawDigest(const wchar_t* str, size_t len)  {
    HashingBase<Impl,Tools> t;
    t.DoRawDigest(str, len*sizeof(wchar_t));
    return new TArrayList<uint8_t>(t.DigestSize(), t.GetDigest());
  }
  static ConstArrayList<uint8_t> RawDigest(const char* str) {
    return RawDigest(str, olxstr::o_strlen(str));
  }
  static ConstArrayList<uint8_t> RawDigest(const wchar_t* str) {
    return RawDigest(str, olxstr::o_strlen(str));
  }
  static ConstArrayList<uint8_t> RawDigest(IInputStream& stream){
    HashingBase<Impl,Tools> t;
    t.DoDigest(stream);
    return new TArrayList<uint8_t>(t.DigestSize(), t.GetDigest());
  }
};

EndEsdlNamespace()
#endif
