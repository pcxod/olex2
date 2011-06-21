#ifndef __olx_sdl_sha_H
#define __olx_sdl_sha_H
#include "hashing.h"
BeginEsdlNamespace()

/* SHA-1 message digest implementation, for reference look at:
  http://en.wikipedia.org/wiki/SHA_hash_functions
*/
class SHA1Impl  {
  unsigned char digest[20];
  uint32_t state[5], bf[80];
protected:
  SHA1Impl();
  void digest64(const uint32_t* msg);
  olxcstr formatDigest(); 
};

template <class Impl>
class SHA2  {
protected:
  static uint32_t table[];
  unsigned char digest[64];
  uint32_t state[8], bf[64];
  SHA2() {}
protected:
  void digest64(const uint32_t* msg);
};

class SHA256Impl : public SHA2<SHA256Impl> {
protected:
  SHA256Impl();
  olxcstr formatDigest(); 
};

class SHA224Impl : public SHA2<SHA224Impl> {
protected:
  SHA224Impl();
  olxcstr formatDigest(); 
};

typedef HashingBase<SHA1Impl, HashingUtilsBE> SHA1;
typedef HashingBase<SHA256Impl, HashingUtilsBE> SHA256;
typedef HashingBase<SHA224Impl, HashingUtilsBE> SHA224;

EndEsdlNamespace()
#endif
