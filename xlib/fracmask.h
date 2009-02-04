#ifndef __xlib_fractional_mask_H
#define __xlib_fractional_mask_H
#include "xbase.h"
#include "arrays.h"
#include "threex3.h"

class FractMask {
  TArray3D<bool>* Mask;
  vec3d Norm;
public:
  FractMask() : Mask(NULL)  {  }
  ~FractMask()  {
    if( Mask )  
      delete Mask; 
  }
  /* min and max - fractional coordinates, norm - length of the sides,
  resolution - the mask resolution in anstrems */
  void Init(const vec3d& _min, const vec3d& _max, const vec3d& norms, double resolution=1.0)  {
    if( Mask != NULL )  {
      delete Mask;
      Mask = NULL; // in case of the exception
    }
    Norm = norms/resolution;
    vec3d min = _min*Norm,
          max = _max*Norm;
    if( min[0] >= max[0] ||
        min[1] >= max[1] ||
        min[2] >= max[2] )
      throw TInvalidArgumentException(__OlxSourceInfo, "mask size");
    Mask = new TArray3D<bool>(
      (int)min[0], (int)max[0], 
      (int)min[1], (int)max[1], 
      (int)min[2], (int)max[2]
    );
    Mask->FastInitWith(0);
  }
  // takes fractional coordinates
  inline void Set(const vec3d& fc, bool v)  {
    const vec3d ind = fc * Norm;
    if( Mask->IsInRange(ind) )
      Mask->Value(ind) = v;
  } 
  // takes fractional coordinates
  template <class vec> inline bool Get(const vec& fc) const {
    const vec3d ind = fc*Norm;
    return Mask->IsInRange(ind) ? Mask->Value(ind) : false;
  } 
  inline TArray3D<bool>* GetMask() const {  return Mask;  }
};


#endif

