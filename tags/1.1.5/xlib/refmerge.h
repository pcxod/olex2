/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_ref_merge_H
#define __olx_ref_merge_H
#include "reflection.h"
BeginXlibNamespace()

struct MergeStats  {
  double Rint, Rsigma, MeanIOverSigma;
  size_t SystematicAbsentcesRemoved,
    InconsistentEquivalents,
    UniqueReflections,
    CentricReflections,
    ReflectionAPotMax,
    OmittedByUser;      // OMIT h k l, all equivs
  bool FriedelOppositesMerged;
  vec3i MinIndexes, MaxIndexes;
  MergeStats()  {
    SetDefaults();
  }
  MergeStats(const MergeStats& ms) {
    this->operator = (ms);
  }
  MergeStats& operator = (const MergeStats& ms) {
    Rint = ms.Rint;
    Rsigma = ms.Rsigma;
    MeanIOverSigma = ms.MeanIOverSigma;
    SystematicAbsentcesRemoved = ms.SystematicAbsentcesRemoved;
    InconsistentEquivalents = ms.InconsistentEquivalents;
    UniqueReflections = ms.UniqueReflections;
    OmittedByUser = ms.OmittedByUser;
    CentricReflections = ms.CentricReflections;
    FriedelOppositesMerged = ms.FriedelOppositesMerged;
    ReflectionAPotMax = ms.ReflectionAPotMax;
    MinIndexes = ms.MinIndexes;
    MaxIndexes = ms.MaxIndexes;
    return *this;
  }
  void SetDefaults()  {
    Rint = Rsigma = 0;
    MeanIOverSigma = 0;
    MinIndexes[0] = MinIndexes[1] = MinIndexes[2] = 100;
    MaxIndexes[0] = MaxIndexes[1] = MaxIndexes[2] = -100;
    OmittedByUser = UniqueReflections = 0;
    CentricReflections = SystematicAbsentcesRemoved = InconsistentEquivalents = 0;
    ReflectionAPotMax = 0;
    FriedelOppositesMerged = false;
  }
  //bool IsEmpty() const {  return TotalReflections == 0;  }
};
//..............................................................................
class RefMerger {
  template <class RefListMerger> 
  static MergeStats _DoMerge(const SymSpace::InfoEx& info_ex, TRefPList& refs,
    const vec3i_list& omits, TRefList& output)
  {
    if( refs.IsEmpty() )
      throw TInvalidArgumentException(__OlxSourceInfo, "empty reflection list");
    MergeStats stats;
    const size_t ref_cnt = refs.Count();
    for( size_t i=0; i < ref_cnt; i++ )
      refs[i]->Standardise(info_ex);
    stats.FriedelOppositesMerged = info_ex.centrosymmetric;
    // standartise reflection indexes according to provided list of symmetry operators
    // sort the list
    TReflection::SortList(refs);
    output.SetCapacity(ref_cnt); // better more that none :)
    // merge reflections
    double Sdiff = 0, SI = 0, SS = 0, SI_tot = 0;
    TReflection* ref = refs[0];  // reference reflection
    for( size_t i=0; i < ref_cnt; )  {
      const size_t from = i;
      while( (++i < ref_cnt) && (ref->CompareTo(*refs[i]) == 0) )
        ;
      const size_t merged_count = i - from;
      bool omitted = false;
      for( size_t j=0; j < omits.Count(); j++ )  {
        if( ref->GetHkl() == omits[j] )  {
          stats.OmittedByUser += merged_count;
          omitted = true;
          break;
        }
      }
      if( !omitted )  {
        if( merged_count > stats.ReflectionAPotMax )
          stats.ReflectionAPotMax = merged_count;
        if( !ref->IsAbsent() )  {
          // the reflection is replicated if only one in the list
          MergerOut mo = RefListMerger::Merge(refs, from, i);
          if( merged_count > 1 )  {
            SI_tot += mo.sumI;
            Sdiff += mo.sumDiff;
            if( mo.sigInt > mo.ref->GetS() )  {
              if( mo.sigInt > 5*mo.ref->GetS() )  {
                stats.InconsistentEquivalents ++;
                mo.ref->SetTag(-1);  // mark as unusable
              }
              mo.ref->SetS(mo.sigInt);
            }
            mo.ref->SetBatch(ref->GetBatch());
          }
          output.Add(mo.ref).Analyse(info_ex);
          if( mo.ref->IsCentric() )
            stats.CentricReflections++;
          SS += mo.ref->GetS();
          SI += mo.ref->GetI();
          stats.MeanIOverSigma += mo.ref->GetI()/mo.ref->GetS();
          vec3i::UpdateMinMax(ref->GetHkl(), stats.MinIndexes, stats.MaxIndexes);
        }
        else
          stats.SystematicAbsentcesRemoved += merged_count;
      }
      if( i >= ref_cnt )  break;
      ref = refs[i];
    }
    stats.Rint = (SI_tot != 0) ? Sdiff/SI_tot : 0.0;
    stats.Rsigma = (SI != 0) ? SS/SI : 0.0;
    stats.UniqueReflections = output.Count();
    stats.MeanIOverSigma /= (stats.UniqueReflections == 0 ? 1 : stats.UniqueReflections);
    return stats;
  }
  template <class RefListMerger> 
  static MergeStats _DryMerge(const SymSpace::InfoEx& info_ex, TRefPList& refs,
    const vec3i_list& omits)
  {
    if( refs.IsEmpty() )
      throw TInvalidArgumentException(__OlxSourceInfo, "empty reflection list");
    MergeStats stats;
    // standartise reflection indexes according to provided list of symmetry operators
    const size_t ref_cnt = refs.Count();
    for( size_t i=0; i < ref_cnt; i++ )
      refs[i]->Standardise(info_ex);
    stats.FriedelOppositesMerged = info_ex.centrosymmetric;
    // sort the list
    TReflection::SortList(refs);
    // merge reflections
    double Sdiff = 0, SI = 0, SS = 0, SI_tot = 0;
    TReflection* ref = refs[0];  // reference reflection
    for( size_t i=0; i < ref_cnt; )  {
      size_t from = i;
      while( (++i < ref_cnt) && (ref->CompareTo(*refs[i]) == 0) )
        ;
      bool omitted = false;
      const size_t merged_count = i - from;
      for( size_t j=0; j < omits.Count(); j++ )  {
        if( ref->GetHkl() == omits[j] )  {
          stats.OmittedByUser += merged_count;
          omitted = true;
          break;
        }
      }
      if( !omitted )  {
        if( merged_count > stats.ReflectionAPotMax )
          stats.ReflectionAPotMax = merged_count;
        if( !ref->IsAbsent() )  {
          // the reflection is replicated if only one in the list
          DryMergerOut mo = RefListMerger::DryMerge(refs, from, i);
          if( merged_count > 1 )  {
            SI_tot += mo.sumI;
            Sdiff += mo.sumDiff;
            if( mo.sigInt > mo.rSig )  {
              if( mo.sigInt > 5*mo.rSig )
                stats.InconsistentEquivalents ++;
              mo.rSig = mo.sigInt;
            }
          }
          ref->Analyse(info_ex);
          if( ref->IsCentric() )
            stats.CentricReflections++;
          SS += mo.rSig;
          SI += mo.rI;
          stats.MeanIOverSigma += mo.rI/mo.rSig;
          vec3i::UpdateMinMax(ref->GetHkl(), stats.MinIndexes, stats.MaxIndexes);
          stats.UniqueReflections++;
        }
        else
          stats.SystematicAbsentcesRemoved += merged_count;
      }
      if( i >= ref_cnt )  break;
      ref = refs[i];
    }
    stats.Rint = (SI_tot != 0) ? Sdiff/SI_tot : 0.0;
    stats.Rsigma = (SI != 0) ? SS/SI : 0.0;
    stats.MeanIOverSigma /= (stats.UniqueReflections == 0 ? 1 : stats.UniqueReflections);
    return stats;
  }
  template <class MatList>
  static MergeStats _DoSGFilter(const MatList& ml, TRefPList& refs, const vec3i_list& omits, 
    TRefList& output)  
  {
    if( refs.IsEmpty() )
      throw TInvalidArgumentException(__OlxSourceInfo, "empty reflection list");
    MergeStats stats;
    stats.FriedelOppositesMerged = false;
    const size_t ref_cnt = refs.Count();
    // sort the list
    TReflection::SortList(refs);
    output.SetCapacity(ref_cnt); // better more that none :)
    TReflection* ref = refs[0];  // reference reflection
    for( size_t i=0; i < ref_cnt; )  {
      ref->Analyse(ml);
      const size_t from = i;
      while( (++i < ref_cnt) && (ref->CompareTo(*refs[i]) == 0) )
        ;
      const size_t merged_count = i - from;
      bool omitted = false;
      for( size_t j=0; j < omits.Count(); j++ )  {
        if( ref->GetHkl() == omits[j] )  {
          stats.OmittedByUser += merged_count;
          omitted = true;
          break;
        }
      }
      if( !omitted )  {
        if( merged_count > stats.ReflectionAPotMax )
          stats.ReflectionAPotMax = merged_count;
        if( !ref->IsAbsent() )  {
          for( size_t j=from; j < i; j++ )  {
            TReflection& _r = output.AddCCopy(*refs[j]);
            _r.SetCentric(ref->IsCentric());
            _r.SetMultiplicity(ref->GetMultiplicity());
            stats.MeanIOverSigma += _r.GetI()/_r.GetS();
            vec3i::UpdateMinMax(_r.GetHkl(), stats.MinIndexes, stats.MaxIndexes);
          }
          if( ref->IsCentric() )
            stats.CentricReflections++;
        }
        else
          stats.SystematicAbsentcesRemoved ++;
      }
      if( i >= ref_cnt )  break;
      ref = refs[i];
    }
    stats.UniqueReflections = output.Count();
    stats.Rint = -1;
    stats.Rsigma = -1;
    stats.MeanIOverSigma = -1;
    return stats;
  }
  template <class MatList>
  static MergeStats _DoDrySGFilter(const MatList& ml, TRefPList& refs, const vec3i_list& omits)  {
    if( refs.IsEmpty() )
      throw TInvalidArgumentException(__OlxSourceInfo, "empty reflection list");
    MergeStats stats;
    stats.FriedelOppositesMerged = false;
    const size_t ref_cnt = refs.Count();
    // sort the list
    TReflection::SortList(refs);
    TReflection* ref = refs[0];  // reference reflection
    for( size_t i=0; i < ref_cnt; )  {
      ref->Analyse(ml);
      const size_t from = i;
      while( (++i < ref_cnt) && (ref->CompareTo(*refs[i]) == 0) )
        ;
      const size_t merged_count = i - from;
      bool omitted = false;
      for( size_t j=0; j < omits.Count(); j++ )  {
        if( ref->GetHkl() == omits[j] )  {
          stats.OmittedByUser += merged_count;
          omitted = true;
          break;
        }
      }
      if( !omitted )  {
        if( merged_count > stats.ReflectionAPotMax )
          stats.ReflectionAPotMax = merged_count;
        if( !ref->IsAbsent() )  {
          for( size_t j=from; j < i; j++ )  {
            stats.MeanIOverSigma += refs[j]->GetI()/refs[j]->GetS();
            vec3i::UpdateMinMax(refs[j]->GetHkl(), stats.MinIndexes, stats.MaxIndexes);
          }
          if( ref->IsCentric() )
            stats.CentricReflections++;
          stats.UniqueReflections += merged_count;
        }
        else
          stats.SystematicAbsentcesRemoved ++;
      }
      if( i >= ref_cnt )  break;
      ref = refs[i];
    }
    stats.Rint = -1;
    stats.Rsigma = -1;
    stats.MeanIOverSigma /= (stats.UniqueReflections == 0 ? 1 : stats.UniqueReflections) ;
    return stats;
  }

  template <class RefListMerger>
  static MergeStats _DoMergeInP1(TPtrList<const TReflection>& refs, const vec3i_list& omits,
    TRefList& output)
  {
    if( refs.IsEmpty() )
      throw TInvalidArgumentException(__OlxSourceInfo, "empty reflection list");
    MergeStats stats;
    // sort the list
    TReflection::SortList(refs);
    const size_t ref_cnt = refs.Count();
    output.SetCapacity( ref_cnt ); // better more that none :)
    // merge reflections
    double Sdiff = 0, SI_tot = 0, SI = 0, SS = 0;
    const TReflection* ref = refs[0];  // reference reflection
    for( size_t i=0; i < ref_cnt; )  {
      const size_t from = i;
      while( (++i < ref_cnt) && (ref->CompareTo(*refs[i]) == 0) )
        ;
      const size_t merged_count = i - from;
      bool omitted = false;
      for( size_t j=0; j < omits.Count(); j++ )  {
        if( ref->GetHkl() == omits[j] )  {
          stats.OmittedByUser += merged_count;
          omitted = true;
          break;
        }
      }
      if( !omitted )  {
        if( merged_count > stats.ReflectionAPotMax )
          stats.ReflectionAPotMax = merged_count;
        MergerOut mo = RefListMerger::Merge(refs, from, i);
        if( merged_count > 1 )  {
          SI_tot += mo.sumI;
          Sdiff += mo.sumDiff;
          if( mo.sigInt > mo.ref->GetS() )  {
            if( mo.sigInt > 5*mo.ref->GetS() )  {
              stats.InconsistentEquivalents ++;
              mo.ref->SetTag(-1);  // mark as unusable
            }
            mo.ref->SetS(mo.sigInt);
          }
          mo.ref->SetBatch(ref->GetBatch());
        }
        output.Add(mo.ref);
        SS += mo.ref->GetS();
        SI += mo.ref->GetI();
        stats.MeanIOverSigma += mo.ref->GetI()/mo.ref->GetS();
        vec3i::UpdateMinMax(ref->GetHkl(), stats.MinIndexes, stats.MaxIndexes);
      }
      if( i >= ref_cnt )  break;
      ref = refs[i];
    }
    stats.Rint = (SI_tot != 0) ? Sdiff/SI_tot : 0.0;
    stats.Rsigma = (SI != 0) ? SS/SI : 0.0;
    stats.UniqueReflections = output.Count();
    stats.MeanIOverSigma /= (stats.UniqueReflections == 0 ? 1 : stats.UniqueReflections);
    return stats;
  }
  template <class RefListMerger>
  static MergeStats _DryMergeInP1(TPtrList<const TReflection>& refs, const vec3i_list& omits)  {
    if( refs.IsEmpty() )
      throw TInvalidArgumentException(__OlxSourceInfo, "empty reflection list");
    MergeStats stats;
    // sort the list
    TReflection::SortList(refs);
    // merge reflections
    const size_t ref_cnt = refs.Count();
    double Sdiff = 0, SI_tot = 0, SI = 0, SS = 0;
    const TReflection* ref = refs[0];
    for( size_t i=0; i < ref_cnt; )  {
      const size_t from = i;
      while( (++i < ref_cnt) && (ref->CompareTo(*refs[i]) == 0) )
        ;
      const size_t merged_count = i - from;
      bool omitted = false;
      for( size_t j=0; j < omits.Count(); j++ )  {
        if( ref->GetHkl() == omits[j] )  {
          stats.OmittedByUser += merged_count;
          omitted = true;
          break;
        }
      }
      if( !omitted )  {
        if( merged_count > stats.ReflectionAPotMax )
          stats.ReflectionAPotMax = merged_count;
        DryMergerOut mo = RefListMerger::DryMerge(refs, from, i);
        if( merged_count > 1 )  {
          SI_tot += mo.sumI;
          Sdiff += mo.sumDiff;
          if( mo.sigInt > mo.rSig )  {
            if( mo.sigInt > 5*mo.rSig )
              stats.InconsistentEquivalents ++;
          }
        }
        SS += mo.rSig;
        SI += mo.rI;
        stats.MeanIOverSigma += mo.rI/mo.rSig;
        vec3i::UpdateMinMax(ref->GetHkl(), stats.MinIndexes, stats.MaxIndexes);
        stats.UniqueReflections++;
      }
      if( i >= ref_cnt )  break;
      ref = refs[i];
    }
    stats.Rint = (SI_tot != 0) ? Sdiff/SI_tot : 0.0;
    stats.Rsigma = (SI != 0) ? SS/SI : 0.0;
    stats.MeanIOverSigma /= (stats.UniqueReflections == 0 ? 1 : stats.UniqueReflections);
    return stats;
  }
public:
/* Function merges provided reflections using provided list of matrices. 
   The reflections are standardised. The resulting reflections are stored in the output .
*/
  template <class MatList, class RefListMerger, class RefList> 
  static MergeStats Merge(const MatList& ml, RefList& Refs, TRefList& output, 
    const vec3i_list& omits, bool mergeFP)  
  {
    SymSpace::InfoEx info_ex = SymSpace::Compact(ml);
    if( mergeFP )  info_ex.centrosymmetric = true; 
    TRefPList refs(Refs, DirectAccessor());
    return _DoMerge<RefListMerger>(info_ex, refs, omits, output);
  }
  template <class RefListMerger, class RefList> 
  static MergeStats Merge(const SymSpace::InfoEx& si, RefList& Refs, TRefList& output, 
    const vec3i_list& omits)  
  {
    TRefPList refs(Refs, DirectAccessor());
    return _DoMerge<RefListMerger>(si, refs, omits, output);
  }
  /* Functions gets the statistic on the list of provided reflections (which get stantardised) */
  template <class MatList, class RefListMerger, class RefList> 
  static MergeStats DryMerge(const MatList& ml, RefList& Refs, const vec3i_list& omits, bool mergeFP)  {
    SymSpace::InfoEx info_ex = SymSpace::Compact(ml);
    if( mergeFP )
      info_ex.centrosymmetric = true; 
    TRefPList refs(Refs, DirectAccessor());
    return _DryMerge<RefListMerger>(info_ex, refs, omits);
  }
  template <class RefListMerger, class RefList> 
  static MergeStats DryMerge(const SymSpace::InfoEx& si, RefList& Refs, const vec3i_list& omits)  {
    TRefPList refs(Refs, DirectAccessor());
    return _DryMerge<RefListMerger>(si, refs, omits);
  }
  /* The function merges provided reflections in P1 and strores the result in the output */
  template <class RefListMerger, class RefList> 
  static MergeStats MergeInP1(const RefList& Refs, TRefList& output, const vec3i_list& omits)  {
    TPtrList<const TReflection> refs(Refs, DirectAccessor());
    return _DoMergeInP1<RefListMerger>(refs, omits, output);
  }
  template <class RefListMerger, class RefList> 
  static MergeStats DryMergeInP1(const RefList& Refs, const vec3i_list& omits)  {
    TPtrList<const TReflection> refs(Refs, DirectAccessor());
    return _DryMergeInP1<RefListMerger>(refs, omits);
  }
  /* The function filters out systematic absences */
  template <class MatList, class RefList> 
  static MergeStats SGFilter(const MatList& ml, RefList& Refs, TRefList& output, const vec3i_list& omits)  {
    TRefPList refs(Refs, DirectAccessor());
    return _DoSGFilter<MatList>(ml.SubListFrom(ml[0].IsI() ? 1 : 0), refs, omits, output);
  }
  template <class MatList, class RefList> 
  static MergeStats DrySGFilter(const MatList& ml, RefList& Refs, const vec3i_list& omits)  {
    TRefPList refs(Refs, DirectAccessor());
    return _DoDrySGFilter<MatList>(ml.SubListFrom(ml[0].IsI() ? 1 : 0), refs, omits);
  }

  struct MergerOut  {
    TReflection* ref;
    double sigInt, sumI, sumDiff;
    MergerOut(const MergerOut& mo) :
      ref(mo.ref),
      sigInt(mo.sigInt),
      sumI(mo.sumI),
      sumDiff(mo.sumDiff)  {  }
    MergerOut(TReflection* _ref, double _sigInt = 0, double _sumI = 0, double _sumDiff = 0 ) :
      ref(_ref),
      sigInt(_sigInt),
      sumI(_sumI),  
      sumDiff(_sumDiff)  {  }
    MergerOut& operator = (const MergerOut& mo)  {
      ref = mo.ref;
      sigInt = mo.sigInt;
      sumI = mo.sumI;
      sumDiff = mo.sumDiff;
      return *this;
    }
  };
  struct DryMergerOut  {
    double sigInt, sumI, sumDiff, rI, rSig;
    DryMergerOut(const DryMergerOut& mo) :
      sigInt(mo.sigInt),
      sumI(mo.sumI),
      sumDiff(mo.sumDiff),
      rI(mo.rI),
      rSig(mo.rSig)  {  }
    DryMergerOut(double _rI, double _rSig, double _sigInt = 0, double _sumI = 0, double _sumDiff = 0 ) :
      rI(_rI),
      rSig(_rSig),
      sigInt(_sigInt),
      sumI(_sumI),  
      sumDiff(_sumDiff)  {  }
    DryMergerOut& operator = (const DryMergerOut& mo)  {
      sigInt = mo.sigInt;
      sumI = mo.sumI;
      sumDiff = mo.sumDiff;
      rI = mo.rI;
      rSig = mo.rSig;
      return *this;
    }
  };
/* merges using statistical weights for the intensities Imean = sum( I/Sig^2 )/sum( 1./Sig^2 )
   http://www.crystal.chem.uu.nl/distr/mergehklf5/mergehklf5.html
   we cannot use a single pass algorithm like:
      double sum_wght = 0, sum_wght_i_sq = 0, sum_wght_i = 0, sum_i = 0;
      for( size_t l=0; l < rl.Count(); l++ )  {
        const double s = rl[l]->GetS() != 0 ? rl[l]->GetS() : 0.001;
        const double w = 1./(s*s);
        sum_wght += w;
        sum_wght_i_sq += w*rl[l]->GetI()*rl[l]->GetI();
        sum_wght_i += w*rl[l]->GetI();
        sum_i += rl[l]->GetI();
      }
      const double mean = sum_wght_i/sum_wght;
      const double sig_sq = (sum_wght_i_sq - sum_wght_i*sum_wght_i/sum_wght)/( (rl.Count()-1)*sum_wght );
      const double sum_diff = (sum_i*sum_wght - rl.Count()*sum_wght_i)/sum_wght;
      return MergerOut( new TReflection( rl[0]->GetH(), rl[0]->GetK(), rl[0]->GetL(), mean, 1./sqrt(sum_wght)), sqrt(sig_sq), sum_i, sum_diff);
  since we need to return the sum( |Ii-Imean| )...
*/
  class StandardMerger  {
  public:
    // returns a newly created reflection do be deleted with delete
    template <class RefPList> static MergerOut Merge(const RefPList& rl, size_t from, size_t to)  {
      const size_t cnt = to-from;
      if( cnt == 1 )  return MergerOut(new TReflection(*rl[from]) );
      double sum_wght = 0, sum_wght_i = 0, sum_i = 0;
      for( size_t i=from; i < to; i++ )  {
        const double w = 1./(rl[i]->GetS() != 0 ? rl[i]->GetS() : 0.001);
        sum_wght += w;
        sum_wght_i += w*rl[i]->GetI();
        sum_i += rl[i]->GetI();
      }
      const double mean = sum_wght_i/sum_wght;
      double sum_diff = 0, summ_i = 0, sig_top = 0;
      for( size_t i=from; i < to; i++ )  {
        const double diff = rl[i]->GetI() - mean;
        sum_diff += (diff < 0 ? -diff : diff);
        summ_i += rl[i]->GetI();
        const double w = 1./(rl[i]->GetS() != 0 ? rl[i]->GetS() : 0.001);
        sig_top += w*diff*diff; 
      }
      return MergerOut( 
        new TReflection( rl[from]->GetH(), rl[from]->GetK(), rl[from]->GetL(), mean, 1./sqrt(sum_wght)), 
        sqrt(sig_top/(sum_wght*(cnt-1))), 
        sum_i, 
        sum_diff
        );
    }
    // returns just statistics
    template <class RefPList> static DryMergerOut DryMerge(const RefPList& rl, size_t from, size_t to)  {
      const size_t cnt = to-from;
      if( cnt == 1 )  return DryMergerOut(rl[from]->GetI(), rl[from]->GetS() );
      double sum_wght = 0, sum_wght_i = 0, sum_i = 0;
      for( size_t i=from; i < to; i++ )  {
        const double w = 1./(rl[i]->GetS() != 0 ? rl[i]->GetS() : 0.001);
        sum_wght += w;
        sum_wght_i += w*rl[i]->GetI();
        sum_i += rl[i]->GetI();
      }
      const double mean = sum_wght_i/sum_wght;
      double sum_diff = 0, summ_i = 0, sig_top = 0;
      for( size_t i=from; i < to; i++ )  {
        const double diff = rl[i]->GetI() - mean;
        sum_diff += (diff < 0 ? -diff : diff);
        summ_i += rl[i]->GetI();
        const double w = 1./(rl[i]->GetS() != 0 ? rl[i]->GetS() : 0.001);
        sig_top += w*diff*diff; 
      }
      return DryMergerOut( 
        mean, 
        1./sqrt(sum_wght), 
        sqrt(sig_top/(sum_wght*(cnt-1))), 
        sum_i, 
        sum_diff
        );
    }
  };
  class UnitMerger  {
  public:
    // returns a newly created reflection do be deleted with delete
    template <class RefPList> static MergerOut Merge(const RefPList& rl, size_t from, size_t to)  {
      const size_t cnt = to-from;
      if( cnt == 1 )  return MergerOut(new TReflection(*rl[from]) );
      double sum_sig_sq = 0, sum_i = 0;
      for( size_t i=from; i < to; i++ )  {
        sum_i += rl[i]->GetI();
        sum_sig_sq += rl[i]->GetS()*rl[i]->GetS();
      }
      const double mean = sum_i/cnt;
      double sum_diff = 0, sum_diff_sq = 0;
      for( size_t i=from; i < to; i++ )  {
        const double diff = rl[i]->GetI() - mean;
        sum_diff += (diff < 0 ? -diff : diff);
        sum_diff_sq += diff*diff;
      }
      return MergerOut( 
        new TReflection( rl[from]->GetH(), rl[from]->GetK(), rl[from]->GetL(), mean, sqrt(sum_sig_sq/cnt)), 
        sqrt(sum_diff_sq/(cnt*(cnt-1))), 
        sum_i, 
        sum_diff
        );
    }
    // returns just statistic
    template <class RefPList> static DryMergerOut DryMerge(const RefPList& rl, size_t from, size_t to)  {
      const size_t cnt = to-from;
      if( cnt == 1 )  return DryMergerOut(rl[from]->GetI(), rl[from]->GetS() );
      double sum_sig_sq = 0, sum_i = 0;
      for( size_t i=from; i < to; i++ )  {
        sum_i += rl[i]->GetI();
        sum_sig_sq += rl[i]->GetS()*rl[i]->GetS();
      }
      const double mean = sum_i/rl.Count();
      double sum_diff = 0, sum_diff_sq = 0;
      for( size_t i=from; i < to; i++ )  {
        const double diff = rl[i]->GetI() - mean;
        sum_diff += (diff < 0 ? -diff : diff);
        sum_diff_sq += diff*diff;
      }
      return DryMergerOut( 
        mean, 
        sqrt(sum_sig_sq/cnt), 
        sqrt(sum_diff_sq/(cnt*(cnt-1))), 
        sum_i, 
        sum_diff
        );
    }
  };
/* merges using George's approach 
   http://www.crystal.chem.uu.nl/distr/mergehklf5/mergehklf5.html
*/
  class ShelxMerger  {
  public:
    // returns a newly created reflection do be deleted with delete
    template <class RefPList> static MergerOut Merge(const RefPList& rl, size_t from, size_t to)  {
      const size_t cnt = to-from;
      if( cnt == 1 )  return MergerOut(new TReflection(*rl[from]) );
      double sum_wght = 0, sum_wght_i = 0, sum_rec_sig_sq = 0;
      for( size_t i=from; i < to; i++ )  {
        const double s = rl[i]->GetS() != 0 ? rl[i]->GetS() : 0.001;
        const double rec_sig_sq = 1./(s*s);
        const double w = (rl[i]->GetI() > 3.0*s) ? rl[i]->GetI()*rec_sig_sq : 3./s;
        sum_wght += w;
        sum_rec_sig_sq += rec_sig_sq;
        sum_wght_i += w*rl[i]->GetI();
      }
      const double mean = sum_wght_i/sum_wght;
      double sum_diff = 0, sum_i = 0;
      for( size_t i=from; i < to; i++ )  {
        const double diff = rl[i]->GetI() - mean;
        sum_diff += (diff < 0 ? -diff : diff);
        sum_i += rl[i]->GetI();
      }
      return MergerOut( 
        new TReflection( rl[from]->GetH(), rl[from]->GetK(), rl[from]->GetL(), mean, 1./sqrt(sum_rec_sig_sq)), 
        sum_diff/(cnt*sqrt((double)cnt-1.0)), 
        sum_i, 
        sum_diff
        );
    }
    // returns a just statistic
    template <class RefPList> static DryMergerOut DryMerge(const RefPList& rl, size_t from, size_t to)  {
      const size_t cnt = to - from;
      if( cnt == 1 )  return DryMergerOut(rl[from]->GetI(), rl[from]->GetS());
      double sum_wght = 0, sum_wght_i = 0, sum_rec_sig_sq = 0;
      for( size_t i=from; i < to; i++ )  {
        const double s = rl[i]->GetS() != 0 ? rl[i]->GetS() : 0.001;
        const double rec_sig_sq = 1./(s*s);
        const double w = (rl[i]->GetI() > 3.0*s) ? rl[i]->GetI()*rec_sig_sq : 3./s;
        sum_wght += w;
        sum_rec_sig_sq += rec_sig_sq;
        sum_wght_i += w*rl[i]->GetI();
      }
      const double mean = sum_wght_i/sum_wght;
      double sum_diff = 0, sum_i = 0;
      for( size_t i=from; i < to; i++ )  {
        const double diff = rl[i]->GetI() - mean;
        sum_diff += (diff < 0 ? -diff : diff);
        sum_i += rl[i]->GetI();
      }
      return DryMergerOut(  
        mean, 
        1./sqrt(sum_rec_sig_sq), 
        sum_diff/(cnt*sqrt((double)cnt-1.0)), 
        sum_i, 
        sum_diff
        );
    }
  };
};

EndXlibNamespace()
#endif
