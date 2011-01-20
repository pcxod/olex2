#ifndef __olx_xlib_rot_id_H
#define __olx_xlib_rot_id_H
#include "xbase.h"
#include "symmat.h"
BeginXlibNamespace()

/*bit packed integer rotation matrix - uses 9 bits for the values
  and 9 bits for the signs 
*/
namespace rotation_id {
  // derived/using classes might need to know this
  static const size_t size = 18;
  // generates id for a rotation matrix
  static int get(const mat3i& m)  {
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
  static mat3i get(int id)  {
    mat3i rv;
    for( int i=0; i < 9; i++ )
      if( (id & (1<<i)) != 0 )
        rv[i/3][i%3] = (id & (1<<(i+9))) ? -1 : 1;
    return rv;
  }
  // generates an id for -matrix
  static int negate(int id)  {
    return (id&0x1FF)|((id&0x3FE00)^((id&0x1FF)<<9));
  }
  // compares only first 18 bits
  static int compare(int id1, int id2)  {
    return (id1&0x3FFFF)-(id2&0x3FFFF);
  }
  // compares only first 18 bits
  static bool equals(int id1, int id2)  {
    return compare(id1, id2) == 0;
  }
  void Tests(OlxTests& t);
}; // end of the namespace rot_id

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
      int t = static_cast<int>(m.t[i]*base);
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
      rv.t[i] = static_cast<double>(
        (id >>(tr_len*i + rotation_id::size))&tr_mask)/base;
      if( (id & ((uint64_t)1<<(sig_off+i))) != 0 )
        rv.t[i] *= -1;
    }
    return rv;
  }
  static void Tests(OlxTests& t)  {
    t.description = __OlxSrcInfo;
    smatd m(mat3d(0, 1, 1, 1, 0, 1, 0, 0, 0), vec3d(-1./base, 10000./base, -21./base));
    uint64_t id = full_smatd_id::get(m);
    smatd id_r = full_smatd_id::get(id);
    if( !(m == id_r) )
      throw TFunctionFailedException(__OlxSourceInfo, "m != id_r");
    uint64_t i_id = full_smatd_id::negate(id);
    smatd i_id_r = full_smatd_id::get(i_id);
    i_id_r *= -1;
    if( !(m == i_id_r) )
      throw TFunctionFailedException(__OlxSourceInfo, "m != -i_id_r");
  }
};
EndXlibNamespace()
#endif
