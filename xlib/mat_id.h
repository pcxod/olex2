/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xlib_rot_id_H
#define __olx_xlib_rot_id_H
#include "xbase.h"
#include "symmat.h"
#include "math/mmath.h"
BeginXlibNamespace()

/*bit packed integer rotation matrix - uses 9 bits for the values
  and 9 bits for the signs
*/
namespace rotation_id {
  // derived/using classes might need to know this
  static const size_t size = 18;
  // generates id for a rotation matrix
  static inline int get(const mat3i& m)  {
    int mask = 0;
    for( int i=0; i < 9; i++ )  {
      const int v = m[i/3][i%3];
      if( v != 0 )
        mask |= (1<<i);
      if( v < 0 )
        mask |= (1<<(i+9));
    }
    return mask;
  }
  // generates a matrix from an id
  static inline mat3i get(int id)  {
    mat3i rv;
    for( int i=0; i < 9; i++ )
      if( (id & (1<<i)) != 0 )
        rv[i/3][i%3] = (id & (1<<(i+9))) ? -1 : 1;
    return rv;
  }
  /* generates an id for -matrix, bits matchign signs, signs^(bits<<9) -
  inverts them
  */
  static inline int negate(int id)  {
    return (id&0x1FF)|((id&0x3FE00)^((id&0x1FF)<<9));
  }
  // compares only first 18 bits
  static int compare(int id1, int id2)  {
    return (id1&0x3FFFF)-(id2&0x3FFFF);
  }
  // compares only first 18 bits
  static inline bool equals(int id1, int id2)  {
    return compare(id1, id2) == 0;
  }
} // end of the namespace rotation_id

/* space group matrix identifier - uses 13 bit for the rotation matrix and
12 bits for the translation (all positive, < 12)
*/
namespace sg_mat_id {
  // derived/using classes might need to know this
  static const size_t
    translation_size = 4,
    size = rotation_id::size + 3*translation_size;
  // generates id for a rotation matrix
  static inline int get(const smatd& m) {
    int mask = rotation_id::get(m.r);
    for (int i=0; i < 3; i++) {
      int v = olx_round((m.t[i]-olx_floor(m.t[i]))*12);
      mask |= (v << (rotation_id::size+translation_size*i));
    }
    return mask;
  }
  // extracts translation from the id
  static inline vec3d get_t(int id)  {
    vec3d rv;
    for (int i=0; i < 3; i++) {
      int off = rotation_id::size+translation_size*i;
      int v = (((id&(0xF<<off))>>off)&0xF);
      rv[i] = (double)v/12;
    }
    return rv;
  }
  // generates a matrix from an id
  static inline smatd get(int id)  {
    smatd rv;
    rv.r = rotation_id::get(id);
    for (int i=0; i < 3; i++) {
      int off = rotation_id::size+translation_size*i;
      int v = (((id&(0xF<<off))>>off)&0xF);
      rv.t[i] = (double)v/12;
    }
    return rv;
  }
  // compares only first 25 bits
  static int compare(int id1, int id2)  {
    return (id1&0x1FFFFFF)-(id2&0x1FFFFFF);
  }
  // compares only first 18 bits
  static inline bool equals(int id1, int id2)  {
    return compare(id1, id2) == 0;
  }
};

/** Full Id stores the matrix rotation part in first 18 bits, although
for any crystallographic rotation matrix 13 bits are needed (nine bits
for non zero matrix elements and 4 for signs - no more than 4 non-zero
elements normally invloved, but holding the general rotaton with 64 bit
number still allows huge fraction of 12 translaton: (64-18)/3 = 15 (+1):
2^14/12 = 1365 + sign... even base 256 allows translations +-63 cells
*/
template <int base=12> struct full_smatd_id  {
  static const size_t
    tr_len = (64-rotation_id::size-3)/3, //reserve 3 bits for signs
    tr_mask = (1UL << tr_len)-1,
    sig_off = rotation_id::size+tr_len*3;
  static const uint64_t
    tr_full_mask = (((uint64_t)1 << 3*tr_len)-1) << rotation_id::size,
    sig_mask = (uint64_t)3 << sig_off;
  // returs id for a matrix and a translation
  static uint64_t get(const smatd& m)  {
    uint64_t res = rotation_id::get(m.r);
    for( int i=0; i < 3; i++ )  {
      int t = olx_round(m.t[i]*base);
#ifdef _DEBUG
      if( olx_abs(t) > tr_mask )
        throw TFunctionFailedException(__OlxSourceInfo, "translation is too large");
#endif
      res |= (((uint64_t)olx_abs(t)) << (rotation_id::size+tr_len*i));
      if( t < 0 )
        res |= ((uint64_t)1<<(sig_off+i));
    }
    return res;
  }
  // negates the matrix and the translation
  static uint64_t negate(uint64_t id)  {
    uint64_t res = rotation_id::negate((int)id);
    return res | (id&tr_full_mask) | ((id&sig_mask)^sig_mask);
  }
  // generates matrix and translation from an id
  static smatd get(uint64_t id)  {
    smatd rv;
    rv.r = rotation_id::get((int)id);
    for( int i=0; i < 3; i++ )  {
      rv.t[i] = static_cast<double>(
        (id >>(tr_len*i + rotation_id::size))&tr_mask)/base;
      if( (id & ((uint64_t)1<<(sig_off+i))) != 0 )
        rv.t[i] *= -1;
    }
    return rv;
  }
  // generates matrix from an id
  static mat3i get_m(uint64_t id)  {
    return rotation_id::get((int)id);
  }
  // generates translation from an id
  static vec3d get_t(uint64_t id)  {
    vec3d rv;
    for( int i=0; i < 3; i++ )  {
      rv[i] = static_cast<double>(
        (id >>(tr_len*i + rotation_id::size))&tr_mask)/base;
      if( (id & ((uint64_t)1<<(sig_off+i))) != 0 )
        rv[i] *= -1;
    }
    return rv;
  }
};
EndXlibNamespace()
#endif
