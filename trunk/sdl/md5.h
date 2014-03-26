/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_md5_H
#define __olx_sdl_md5_H
#include "hashing.h"
BeginEsdlNamespace()

/* MD5 message digest implementation, for reference look at:
  http://en.wikipedia.org/wiki/MD5
*/
class MD5Impl  {
  uint8_t digest[16];
  uint32_t state[4], bf[16];
  static const uint32_t *consts_();
  static const unsigned char *rotations();
  // digest 64 byte message updating current stat
protected:
  MD5Impl();
  void digest64(const uint32_t* msg);
  const uint8_t *GetDigest() { return &digest[0]; }
  size_t DigestSize() const { return 16; }
  void finalise() {
    HashingUtilsLE::hs_copy(state, digest, 4);
  }
  olxcstr formatDigest();
};

typedef HashingBase<MD5Impl, HashingUtilsLE> MD5;

EndEsdlNamespace()
#endif
