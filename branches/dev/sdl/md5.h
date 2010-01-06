#ifndef __olx_md5__H
#define __olx_md5__H
#include "hashing.h"
/* MD5 message digest implementation, for reference look at:
  
  http://en.wikipedia.org/wiki/MD5
  Simple tests are implemented in the testsuit.cpp

  (c) O Dolomanov, 2009
*/

BeginEsdlNamespace()

class MD5Impl  {
  unsigned char digest[16];
  uint32_t state[4], bf[16];
  static const uint32_t consts[];
  static const unsigned char rotations[];
  // digest 64 byte message updating current stat
protected:
  MD5Impl();
  void digest64(const uint32_t* msg);
  olxcstr formatDigest(); 
};

typedef HashingBase<MD5Impl, HashingUtilsLE> MD5;

EndEsdlNamespace()
#endif
