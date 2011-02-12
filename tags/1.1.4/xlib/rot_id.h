#ifndef __olx_xlib_rot_id_H
#define __olx_xlib_rot_id_H
#include "xbase.h"
BeginXlibNamespace()

/*bit packed integer rotation matrix - uses 9 bits for the values
  and 9 bits for the signs 
*/
namespace rotation_id {
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
  static mat3d get(int id)  {
    mat3i rv;
    for( int i=0; i < 9; i++ )
      if( (id & (1<<i)) != 0 )
        rv[i/3][i%3] = (id & (1<<(i+9))) ? -1 : 1;
    return rv;
  }
  static int invert(int id)  {
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
}; // end of the namespace rot_id

EndXlibNamespace()
#endif
