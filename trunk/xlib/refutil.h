#ifndef __olx_ref_util_H
#define __olx_ref_util_H
#include "xbase.h"
#include "symmat.h"
#include "symspace.h"

BeginXlibNamespace()

namespace RefUtil {
  template <class MatList> size_t GetBijovetPairs(const TRefList& refs, const vec3i& min_indices,
    const vec3i& max_indices, TRefPList& pos, TRefPList& neg, const MatList& ml)
  {
    SymSpace::InfoEx sp = SymSpace::Compact(ml);
    if( sp.centrosymmetric )
      return 0;
    pos.SetCapacity(refs.Count()/2);
    neg.SetCapacity(refs.Count()/2);
    TArray3D<TReflection*> hkl3d(min_indices, max_indices);
    for( size_t i=0; i < refs.Count(); i++ )  {
      hkl3d(refs[i].GetHkl()) = &refs[i];
      refs[i].SetTag(i);
    }
    size_t cnt = 0;
    for( size_t i=0; i < refs.Count(); i++ )  {
      if( refs[i].GetTag() < 0 )  continue;
      refs[i].SetTag(-1);
      for( size_t mi=0; mi < sp.matrices.Count(); mi++ )  {
        const vec3i& pi = refs[i].GetHkl();
        vec3i ni;
        refs[i].MulHkl(ni, sp.matrices[mi]);
        ni *= -1;
        if( hkl3d.IsInRange(ni) && hkl3d(ni) != NULL ) {
          TReflection& n = *hkl3d(ni);
          if( n.GetTag() < 0 )  continue;
          pos.Add(refs[i]);
          neg.Add(n)->SetTag(-1);
          cnt++;
        }
      }
    }
    return cnt;
  }

};

EndXlibNamespace()
#endif
