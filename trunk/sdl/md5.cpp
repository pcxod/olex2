/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "md5.h"
#include "encodings.h"

const uint32_t *MD5Impl::consts_() {
  static const uint32_t c[] = {
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a,
    0xa8304613, 0xfd469501, 0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821, 0xf61e2562, 0xc040b340,
    0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8,
    0x676f02d9, 0x8d2a4c8a, 0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70, 0x289b7ec6, 0xeaa127fa,
    0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92,
    0xffeff47d, 0x85845dd1, 0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
  };
  return &c[0];
}

const unsigned char *MD5Impl::rotations() {
  static const unsigned char r[]  = {
    7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
    5, 9,  14, 20, 5, 9,  14, 20, 5, 9,  14, 20, 5, 9,  14, 20,
    4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
    6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
  };
  return &r[0];
}

MD5Impl::MD5Impl()  {
  state[0] = 0x67452301;
  state[1] = 0xEFCDAB89;
  state[2] = 0x98BADCFE;
  state[3] = 0x10325476;
}

void MD5Impl::digest64(const uint32_t* msg)  {
  uint32_t a = state[0],
           b = state[1],
           c = state[2],
           d = state[3];
  const uint32_t *consts = consts_();
  for( int i=0; i < 64; i++ )  {
    uint32_t f, g;
    if( i < 16 )  {
      f = (b & c) | (~b & d);
      g = i;
    }
    else if( i < 32 )  {
      f = (d & b) | (~d & c);
      //g = (i*5 + 1) % 16;
      g = (i*5 + 1)&(0x000F);
    }
    else if( i < 48 )  {
      f = b^c^d;
      //g = (i*3 + 5) % 16;
      g = (i*3 + 5)&(0x000F);
    }
    else  {
      f = c^(b | ~d);
      //g = (i*7) % 16;
      g = (i*7)&(0x000F);
    }
    const uint32_t tmp = d;
    d = c;
    c = b;
    b += HashingUtils::hs_rotl( a+f+consts[i]+ msg[g], rotations()[i]);
    a = tmp;
  }
  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;
}

olxcstr MD5Impl::formatDigest()  {
  return encoding::base16::encode(digest, 16);
}
