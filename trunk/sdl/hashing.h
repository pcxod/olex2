#ifndef __olx_hashing__H
#define __olx_hashing__H
#include "ebase.h"

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
  static void hs_copy(const unsigned char* src, uint32_t* dest, size_t src_len)  {
    for( size_t i=0, j=0; i < src_len; i += 4, j++ )
      dest[j] = ((uint32_t)src[i]) | 
      (((uint32_t)src[i+1]) << 8) | 
      (((uint32_t)src[i+2]) << 16) | 
      (((uint32_t)src[i+3]) << 24);
  }
  static void hs_copy(const uint32_t* src, unsigned char* dest, size_t src_len)  {
    for( size_t i=0, j=0; j < src_len; i += 4, j++ )  {
      dest[i] =   (char)src[j];
      dest[i+1] = (char)(src[j] >> 8);
      dest[i+2] = (char)(src[j] >> 16);
      dest[i+3] = (char)(src[j] >> 24);
    }
  }
  static void hs_write_len(unsigned char* dest, const uint64_t& len)  {
    for( int i=8; i >= 1; i-- )
      dest[64-i] = ((unsigned char*)&len)[8-i];
  }
};

struct HashingUtilsBE : public HashingUtils {
public:
  // operates on 64 byte blocks only, big endian
  static void hs_copy(const unsigned char* src, uint32_t* dest, size_t src_len)  {
    for( size_t i=0, j=0; i < src_len; i += 4, j++ )
      dest[j] = ((uint32_t)src[i+3]) | 
      (((uint32_t)src[i+2]) << 8) | 
      (((uint32_t)src[i+1]) << 16) | 
      (((uint32_t)src[i]) << 24);
  }
  static void hs_copy(const uint32_t* src, unsigned char* dest, size_t src_len)  {
    for( size_t i=0, j=0; j < src_len; i += 4, j++ )  {
      dest[i+3] = (char)src[j];
      dest[i+2] = (char)(src[j] >> 8);
      dest[i+1] = (char)(src[j] >> 16);
      dest[i] =   (char)(src[j] >> 24);
    }
  }
  static void hs_write_len(unsigned char* dest, const uint64_t& len)  {
    for( int i=8; i >= 1; i-- )
      dest[64-i] = ((unsigned char*)&len)[i-1];
  }
};

template <class Impl, class Tools>
class HashingBase : public Impl  {
protected:
  uint32_t bf[16];
  unsigned char cbf[64];
  olxcstr DoDigest(const void* msg, size_t len)  {
    const size_t blocks = len/64;
    for( size_t i=0; i < blocks; i++ )  {
      Tools::hs_copy( &((const unsigned char*)msg)[i*64], bf, 64);
      Impl::digest64( bf );
    }
    const size_t part = len%64;
    memset(cbf, 0, 64);
    if( part > 0 )
      memcpy(cbf, msg, part);
    cbf[part] |= (0x01 << 7);
    if( part > 56 )  {  // will not the length fit?
      Tools::hs_copy(cbf, bf, 64);
      Impl::digest64( bf );
      memset(cbf, 0, 64);
    }
    uint64_t len_bits_o = len*8; 
    Tools::hs_write_len(cbf, len_bits_o);
    Tools::hs_copy(cbf, bf, 64);
    Impl::digest64( bf );
    // prepeare the digest
    return Impl::formatDigest();
  }

  olxcstr DoDigest(IInputStream& stream)  {
    unsigned char cbf[64];
    const uint64_t blocks = stream.GetSize()/64;
    for( uint64_t i=0; i < blocks; i++ )  {
      stream.Read(cbf, 64);
      Tools::hs_copy(cbf, bf, 64);
      Impl::digest64( bf );
    }
    const size_t part = stream.GetSize()%64;
    memset(cbf, 0, 64);
    if( part > 0 )
      stream.Read(cbf, part);
    cbf[part] |= (0x01 << 7);
    if( part > 56 )  {  // will not the length fit?
      Tools::hs_copy(cbf, bf, 64);
      Impl::digest64( bf );
      memset(cbf, 0, 64);
    }
    uint64_t len_bits_o = stream.GetSize()*8; 
    Tools::hs_write_len(cbf, len_bits_o);
    Tools::hs_copy(cbf, bf, 64);
    Impl::digest64( bf );
    // prepeare the digest
    return Impl::formatDigest();
  }
  HashingBase() : Impl() {}
public:
  static olxcstr Digest(const olxcstr& str)  {  return HashingBase<Impl,Tools>().DoDigest(str.raw_str(), str.RawLen());  }
  static olxcstr Digest(const olxwstr& str)  {  return HashingBase<Impl,Tools>().DoDigest(str.raw_str(), str.RawLen());  }
  static olxcstr Digest(const char* str)     {  return HashingBase<Impl,Tools>().DoDigest(str, olxstr::o_strlen(str));  }
  static olxcstr Digest(const wchar_t* str)  {  return HashingBase<Impl,Tools>().DoDigest(str, olxstr::o_strlen(str)*sizeof(wchar_t));  }
  static olxcstr Digest(IInputStream& stream){  return HashingBase<Impl,Tools>().DoDigest(stream);  }
};

EndEsdlNamespace()
#endif
