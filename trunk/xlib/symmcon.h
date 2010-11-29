#ifndef __olx_xlib_symcon_H
#define __olx_xlib_symcon_H
#include "xbase.h"
#include "edict.h"
#include "threex3.h"

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
};

struct SymmConItem  {
  int param;
  double multiplier;
};

struct SymmCon  {
  int r_id;
  const SymmConItem* map;  // U + coordinates
};

struct SiteSymmCon  {
private:
  bool added;
public:
  SymmConItem map[9];
  SiteSymmCon();
  bool IsConstrained() const;
  SiteSymmCon& operator += (const SymmCon* sc);
  olxstr ToString() const;
};

class SymmConReg  {
  static const SymmCon* _Find(int rot_id);
public:
  static const SymmCon* Find(int rot_id)  {
    const SymmCon* rv = _Find(rot_id);
    return rv == NULL ? _Find(rotation_id::invert(rot_id)) : rv;
  }
};

EndXlibNamespace()

#endif
