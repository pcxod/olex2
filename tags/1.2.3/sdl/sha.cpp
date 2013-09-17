/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "sha.h"
#include "encodings.h"

SHA1Impl::SHA1Impl()  {
  state[0] = 0x67452301;
  state[1] = 0xEFCDAB89;
  state[2] = 0x98BADCFE;
  state[3] = 0x10325476;
  state[4] = 0xC3D2E1F0;
}

void SHA1Impl::digest64(const uint32_t* msg)  {
  uint32_t a = state[0],
           b = state[1],
           c = state[2],
           d = state[3],
           e = state[4];
  for( int i=0; i < 16; i++ )
    bf[i] = msg[i];
  for( size_t i=16; i < 80; i++ )
    bf[i] = HashingUtils::hs_rotl((bf[i-3]^bf[i-8]^bf[i-14]^bf[i-16]), 1);
  for( size_t i=0; i < 80; i++ )  {
    uint32_t f, k;
    if( i < 20 )  {
      f = (b&c) | (~b & d);
      k = 0x5A827999;
    }
    else if( i < 40 )  {
      f = b^c^d;
      k = 0x6ED9EBA1;
    }
    else if ( i < 60 )  {
      f = (b&c) | (b&d) | (c&d);
      k = 0x8F1BBCDC;
    }
    else  {
      f = b^c^d;
      k = 0xCA62C1D6;
    }
    uint32_t temp = HashingUtils::hs_rotl(a, 5) + f + e + k + bf[i];
    e = d;
    d = c;
    c = HashingUtils::hs_rotl(b, 30);
    b = a;
    a = temp;
  }
  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;
  state[4] += e;
}

olxcstr SHA1Impl::formatDigest()  {
  return encoding::base16::encode(digest, 20, 4, ' ');
}


////////////////////////////// SHA 256 ///////////////////////////////////////////////////
template <class T> const uint32_t *SHA2<T>::table_() {
  static const uint32_t t[] = {
     0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1,
     0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
     0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786,
     0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
     0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
     0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
     0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
     0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
     0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
     0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
     0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
  };
  return &t[0];
}

template <class T>
void SHA2<T>::digest64(const uint32_t* msg)  {
  uint32_t a = state[0],
           b = state[1],
           c = state[2],
           d = state[3],
           e = state[4],
           f = state[5],
           g = state[6],
           h = state[7];
  const uint32_t *table = table_();
  for( int i=0; i < 16; i++ )
    bf[i] = msg[i];
  for( size_t i=16; i < 64; i++ )  {
    const uint32_t s0 = HashingUtils::hs_rotr(bf[i-15],7)^
      HashingUtils::hs_rotr(bf[i-15],18)^(bf[i-15] >> 3);
    const uint32_t s1 = HashingUtils::hs_rotr(bf[i-2],17)^
      HashingUtils::hs_rotr(bf[i-2],19)^(bf[i-2] >> 10);
    bf[i] = bf[i-16] + s0 + bf[i-7] + s1;
  }
  for( size_t i=0; i < 64; i++ )  {
    const uint32_t s0 = HashingUtils::hs_rotr(a,2)^
      HashingUtils::hs_rotr(a,13)^HashingUtils::hs_rotr(a,22);
    const uint32_t maj = (a&b) ^ (a&c) ^ (b&c);
    const uint32_t t2 = s0 + maj;
    const uint32_t s1 = HashingUtils::hs_rotr(e,6)^
      HashingUtils::hs_rotr(e,11)^HashingUtils::hs_rotr(e,25);
    const uint32_t ch = (e&f) ^ (~e & g);
    const uint32_t t1 = h + s1 + ch + table[i] + bf[i];
    h = g;
    g = f;
    f = e;
    e = d + t1;
    d = c;
    c = b;
    b = a;
    a = t1 + t2;
  }
  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;
  state[4] += e;
  state[5] += f;
  state[6] += g;
  state[7] += h;
}

SHA256Impl::SHA256Impl()  {
  state[0] = 0x6a09e667;
  state[1] = 0xbb67ae85;
  state[2] = 0x3c6ef372;
  state[3] = 0xa54ff53a;
  state[4] = 0x510e527f;
  state[5] = 0x9b05688c;
  state[6] = 0x1f83d9ab;
  state[7] = 0x5be0cd19;
}

olxcstr SHA256Impl::formatDigest()  {
  return encoding::base16::encode(digest, 32, 4, ' ');
}

SHA224Impl::SHA224Impl()  {
  state[0] = 0xc1059ed8;
  state[1] = 0x367cd507;
  state[2] = 0x3070dd17;
  state[3] = 0xf70e5939;
  state[4] = 0xffc00b31;
  state[5] = 0x68581511;
  state[6] = 0x64f98fa7;
  state[7] = 0xbefa4fa4;
}

olxcstr SHA224Impl::formatDigest()  {
  return encoding::base16::encode(digest, 28, 4, ' ');
}

template class esdl::SHA2<SHA224Impl>;
template class esdl::SHA2<SHA256Impl>;
