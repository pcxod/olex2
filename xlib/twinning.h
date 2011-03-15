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
      int current;
      const size_t src_index;
      vec3i index;
      Iterator(const merohedral& _parent, size_t _src_index)
        : parent(_parent),
          src_index(_src_index),
          index(parent.all_refs[src_index].GetHkl()),
          current(1) {}
      bool HasNext() const {  return current < olx_abs(parent.n);  }
      TReflection Next() {
        int i = current++;
        if( parent.n < 0 && i == olx_abs(parent.n)/2 )
          index = -index;
        return TReflection(parent.all_refs[src_index], (index = parent.matrix*index), -(i+1));
      }
    };
    merohedral(const TRefList& _all_refs, const TDoubleList& _scales,
      const mat3i& tm, int _n)
      : all_refs(_all_refs), scales(_scales), matrix(tm), n(_n)  {}
    mat3i matrix;
    int n;
    const TRefList& all_refs;
    const TDoubleList& scales;
  };
  /**/
  struct general  {
    struct Iterator  {
      const general& parent;
      size_t current;
      Iterator(const general& _parent, size_t start) : parent(_parent), current(start)  {}
      bool HasNext() const {
        return (current > 0 && parent.all_refs[current-1].GetBatch() < 0);
      }
      const TReflection& Next() {
        return parent.all_refs[--current];
      }
    };
    general(const TRefList& _all_refs, const TDoubleList& _scales)
      : all_refs(_all_refs), scales(_scales)  {}
    const TRefList& all_refs;
    const TDoubleList& scales;
  };

  struct HKLF5  {
    TArray3D<size_t>* F_indices;
    const RefinementModel& rm;
    RefinementModel::HklStat ms;
    const SymSpace::InfoEx& sym_info;
    vec3i_list unique_indices;
    TRefList reflections;
    double basf0;
    HKLF5(const RefinementModel& _rm, const SymSpace::InfoEx& _sym_info)
    : rm(_rm), sym_info(_sym_info),
      F_indices(NULL)
    {
      basf0 = 0;
      for( size_t bi=0; bi < rm.GetBASF().Count(); bi++ )
        basf0 += rm.GetBASF()[bi];
      basf0 = 1-basf0;

      const TRefList& all_refs = rm.GetReflections();
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
          if( all_refs[i].GetBatch() > 0 )  {
            size_t j=i;
            bool all_absent = true;
            while( --j != InvalidIndex && all_refs[j].GetBatch() < 0 )  {
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
        if( all_refs[i].GetBatch() >= 0 )
          reflections.AddCCopy(all_refs[i]).SetTag(i);
      }
      rm.AdjustIntensity(reflections, ms);
    }
    ~HKLF5()  {
      if( F_indices != NULL )
        delete F_indices;
    }
    void calc_fsq(const TArrayList<compd>& F, evecd& Fsq)  {
      const TRefList& all_refs = rm.GetReflections();
      general twin_generator(all_refs, rm.GetBASF());
      Fsq.Resize(reflections.Count());
      const TDoubleList& basf = rm.GetBASF();
      for( size_t i=0; i < reflections.Count(); i++ )  {
        const TReflection& r = reflections[i];
        twinning::general::Iterator itr(twin_generator, r.GetTag());
        while( itr.HasNext() )  {
          TReflection tmate = itr.Next();
          if( tmate.GetTag() < 0 )  continue;
          const size_t bi = olx_abs(tmate.GetBatch())-2;
          if( bi < basf.Count() )
            Fsq[i] += basf[bi]*F[tmate.GetTag()].qmod();
        }
        const TReflection& prime_ref = all_refs[r.GetTag()];
        if( prime_ref.GetTag() < 0 )  // is absent?
          continue;
        const size_t bi = olx_abs(prime_ref.GetBatch())-1;
        if( bi <= basf.Count() )  {
          double k = (bi == 0 ? basf0 : basf[bi-1]);
          Fsq[i] += k*F[prime_ref.GetTag()].qmod();
        }
      }
    }
    void detwin_and_merge(TRefList& out, const TArrayList<compd>& Fc, TArrayList<compd>* pF)  {
      const TRefList& all_refs = rm.GetReflections();
      twinning::general twin_generator(all_refs, rm.GetBASF());
      const TDoubleList& basf = rm.GetBASF();
      out = reflections;
      for( size_t i=0; i < out.Count(); i++ )  {
        TReflection& r = out[i];
        twinning::general::Iterator itr(twin_generator, r.GetTag());
        double d = 0;
        while( itr.HasNext() )  {
          TReflection tmate = itr.Next();
          if( tmate.GetTag() < 0  ) // absent?
            continue;
          const size_t bi = olx_abs(tmate.GetBatch())-2;
          if( bi < basf.Count() )
            d += basf[bi]*Fc[tmate.GetTag()].qmod();
        }
        const size_t bi = olx_abs(r.GetBatch())-1;
        if( bi > basf.Count() )  continue;
        double k = bi == 0 ? basf0 : basf[bi-1];
        double f = Fc[all_refs[r.GetTag()].GetTag()].qmod();
        f = f/(k*f+d);
        r.SetS(r.GetS()*f);
        r.SetI(r.GetI()*f);
        r.SetBatch(TReflection::NoBatchSet);
      }
      TRefPList to_merge(out, DirectAccessor());
      out.ReleaseAll();
      SymSpace::InfoEx si = sym_info;
      si.centrosymmetric = true;
      RefMerger::Merge<RefMerger::ShelxMerger>(sym_info, to_merge, out, rm.GetOmits());
      to_merge.DeleteItems(false);
      if( pF != NULL )  {
        TArrayList<compd>& F = *pF;
        TArray3D<size_t>& hkl3d = *F_indices;
        F.SetCount(out.Count());
        for( size_t i=0; i < out.Count(); i++ )  {
          size_t f_i;
          if( !hkl3d.IsInRange(out[i].GetHkl()) ||
            (f_i = hkl3d(out[i].GetHkl())) == InvalidIndex )
            throw TFunctionFailedException(__OlxSourceInfo, "merging does not match");
          F[i] = Fc[f_i];
        }
      }
    }
  };
}; //end of the twinning namespace
EndXlibNamespace()
#endif
