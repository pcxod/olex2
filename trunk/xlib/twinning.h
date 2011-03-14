#ifndef __olx_xlib_twinning_H
#define __olx_xlib_twinning_H
#include "reflection.h"
#include "refmodel.h"
#include "refutil.h"

BeginXlibNamespace()
namespace twinning  {

  struct merohedral  {
    struct Iterator  {
      const merohedral& parent;
      size_t current;
      vec3i index;
      Iterator(const merohedral& _parent, const vec3i& _index)
        : parent(_parent), index(_index), current(0) {}
      bool HasNext() const {  return current < (size_t)olx_abs(parent.n);  }
      const vec3i& Next() {
        if( current == 0 ) {
          current++;
          return index;
        }
        if( parent.n < 0 && current == olx_abs(parent.n)/2 )
          index = -index;
        return (index = parent.matrix*index);
      }
    };
    merohedral(const mat3i& tm, int _n) : matrix(tm), n(_n)  {}
    mat3i matrix;
    int n;
  };

  struct general  {
    struct Iterator  {
      const general& parent;
      size_t current;
      Iterator(const general& _parent, size_t start) : parent(_parent), current(start)  {}
      bool HasNext() const {
        return (current > 0 && parent.all_refs[current-1].GetFlag() < 0);
      }
      const TReflection& Next() {
        return parent.all_refs[--current];
      }
    };
    general(const TRefList& _all_refs) : all_refs(_all_refs)  {}
    const TRefList& all_refs;
  };

  struct HKLF5  {
    TArray3D<size_t>* F_indices;
    const RefinementModel& rm;
    RefinementModel::HklStat ms;
    const SymSpace::InfoEx& sym_info;
    vec3i_list unique_indices;
    TRefList reflections;
    HKLF5(const RefinementModel& _rm, const SymSpace::InfoEx& _sym_info, TRefList* _refs=NULL)
    : rm(_rm), sym_info(_sym_info),
      F_indices(NULL)
    {
      const TRefList& all_refs = rm.GetReflections();
      TRefList& refs = (_refs == NULL ? reflections : *_refs);
      ms = rm.GetreflectionStat();
      vec3i mi = ms.FileMinInd, mx = ms.FileMaxInd;
      vec3i::UpdateMinMax(TReflection::Standardise(mi, sym_info), mi, mx);
      vec3i::UpdateMinMax(TReflection::Standardise(mx, sym_info), mi, mx);
      TArray3D<size_t>& hkl3d = *(F_indices = new TArray3D<size_t>(mi, mx));
      F_indices->FastInitWith(-1);
      RefUtil::ResolutionAndSigmaFilter rsf(rm, ms);
      const vec3i_list& omits = rm.GetOmits();
      reflections.Clear().SetCapacity(all_refs.Count());
      for( size_t i=all_refs.Count()-1; i != InvalidIndex; i-- )  {
        if( all_refs[i].IsOmitted() )  {
          ms.OmittedByUser++;
          continue;
        }
        if( rsf.IsOutside(all_refs[i]) )  {
          all_refs[i].SetTag(-1);
          continue;
        }
        vec3i hkl = TReflection::Standardise(all_refs[i].GetHkl(), sym_info);
        if( TReflection::IsAbsent(hkl, sym_info) || omits.IndexOf(hkl) != InvalidIndex )  {
          if( all_refs[i].GetFlag() > 0 )  {
            size_t j=i;
            bool all_absent = true;
            while( --j != InvalidIndex && all_refs[j].GetFlag() < 0 )  {
              if( !TReflection::IsAbsent(all_refs[j].GetHkl(), sym_info) &&
                omits.IndexOf(TReflection::Standardise(
                  all_refs[j].GetHkl(), sym_info)) == InvalidIndex )
              {
                all_absent = false;
                break;
              }
            }
            if( all_absent )  {
              all_refs[i].SetTag(-1);
              ms.SystematicAbsentcesRemoved++;
              i = j+1;
              continue;
            }
          }
        }
        if( hkl3d(hkl) == InvalidIndex )  {
          all_refs[i].SetTag(hkl3d(hkl) = unique_indices.Count());
          unique_indices.AddCCopy(hkl);
        }
        else
          all_refs[i].SetTag(hkl3d(hkl));
        if( all_refs[i].GetFlag() >= 0 )
          refs.AddCCopy(all_refs[i]).SetTag(i);
      }
    }
    ~HKLF5()  {
      if( F_indices != NULL )
        delete F_indices;
    }
  };

}; //end of the twinning namespace
EndXlibNamespace()
#endif
