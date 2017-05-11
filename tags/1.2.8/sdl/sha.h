/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_sha_H
#define __olx_sdl_sha_H
#include "hashing.h"
BeginEsdlNamespace()

/* SHA-1 message digest implementation, for reference look at:
  http://en.wikipedia.org/wiki/SHA_hash_functions
*/
class SHA1Impl  {
  uint8_t digest[20];
  uint32_t state[5], bf[80];
protected:
  SHA1Impl();
  void digest64(const uint32_t* msg);
  const uint8_t *GetDigest() { return &digest[0]; }
  size_t DigestSize() const { return 20; }
  void finalise() {
    HashingUtilsBE::hs_copy(state, digest, 5);
  }
  olxcstr formatDigest();
};

template <class Impl>
class SHA2 {
protected:
  static const uint32_t *table_();
  uint8_t digest[32];
  uint32_t state[8], bf[64];
  SHA2() {}
protected:
  void digest64(const uint32_t* msg);
  const uint8_t *GetDigest() { return &digest[0]; }
};

class SHA256Impl : public SHA2<SHA256Impl> {
protected:
  SHA256Impl();
  void finalise() {
    HashingUtilsBE::hs_copy(state, digest, 8);
  }
  size_t DigestSize() const { return 32; }
  olxcstr formatDigest();
};

class SHA224Impl : public SHA2<SHA224Impl> {
protected:
  SHA224Impl();
  void finalise() {
    HashingUtilsBE::hs_copy(state, digest, 7);
  }
  size_t DigestSize() const { return 28; }
  olxcstr formatDigest();
};

typedef HashingBase<SHA1Impl, HashingUtilsBE> SHA1;
typedef HashingBase<SHA256Impl, HashingUtilsBE> SHA256;
typedef HashingBase<SHA224Impl, HashingUtilsBE> SHA224;

EndEsdlNamespace()
#endif
