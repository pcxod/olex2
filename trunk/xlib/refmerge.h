#ifndef __olx_ref_merge_H
#define __olx_ref_merge_H
#include "reflection.h"
BeginXlibNamespace()

struct MergeStats  {
  double Rint, Rsigma, MeanIOverSigma;
  int SystematicAbsentcesRemoved,
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
  static MergeStats DoMerge(smatd_list& ml, TRefPList& refs, const vec3i_list& omits, TRefList& output)  {
    MergeStats stats;
    // search for the inversion matrix
    int inverseMatIndex = -1;
    mat3d mI;
    mI.I() *= -1;
    for( int i=0; i < ml.Count(); i++ )  {
      if( ml[i].r == mI )  {
        inverseMatIndex = i;
        break;
      }
    }
    stats.FriedelOppositesMerged = (inverseMatIndex != -1);
    // standartise reflection indexes according to provieded list of symmetry operators
    const int ref_cnt = refs.Count();
    for( int i=0; i < ref_cnt; i++ )
      refs[i]->Standardise(ml);
    // sort the list
    TReflection::SortPList(refs);
    output.SetCapacity( ref_cnt ); // better more that none :)
    // merge reflections
    double Sdiff = 0, SI = 0, SS = 0, SI_tot = 0;
    TReflection* ref = refs[0];  // reference reflection
    for( int i=0; i < ref_cnt; )  {
      const int from = i;
      while( (++i < ref_cnt) && (ref->CompareTo(*refs[i]) == 0) )
        ;
      const int merged_count = i - from;
      bool omitted = false;
      for( int j=0; j < omits.Count(); j++ )  {
        if( ref->GetH() == omits[j][0] &&
          ref->GetK() == omits[j][1] &&
          ref->GetL() == omits[j][2] )  
        {
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
          MergerOut mo = RefListMerger::Merge( refs, from, i );
          if( merged_count > 1 )  {
            SI_tot += mo.sumI;
            Sdiff += mo.sumDiff;
            if( mo.sigInt > mo.ref->GetS() )  {
              if( mo.sigInt > 5*mo.ref->GetS() )  {
                stats.InconsistentEquivalents ++;
                mo.ref->SetTag(-1);  // mark as unusable
              }
              mo.ref->SetS( mo.sigInt );
            }
            mo.ref->SetFlag( ref->GetFlag() );
          }
          output.Add(mo.ref).Analyse(ml);
          if( mo.ref->IsCentric() )
            stats.CentricReflections++;
          SS += mo.ref->GetS();
          SI += mo.ref->GetI();
          stats.MeanIOverSigma += mo.ref->GetI()/mo.ref->GetS();
          if( ref->GetH() > stats.MaxIndexes[0] )  stats.MaxIndexes[0] = ref->GetH();
          if( ref->GetK() > stats.MaxIndexes[1] )  stats.MaxIndexes[1] = ref->GetK();
          if( ref->GetL() > stats.MaxIndexes[2] )  stats.MaxIndexes[2] = ref->GetL();
          if( ref->GetH() < stats.MinIndexes[0] )  stats.MinIndexes[0] = ref->GetH();
          if( ref->GetK() < stats.MinIndexes[1] )  stats.MinIndexes[1] = ref->GetK();
          if( ref->GetL() < stats.MinIndexes[2] )  stats.MinIndexes[2] = ref->GetL();
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
    if( stats.UniqueReflections != 0 )
      stats.MeanIOverSigma /= stats.UniqueReflections;
    return stats;
  }
  template <class RefListMerger> 
  static MergeStats DryMerge(smatd_list& ml, TRefPList& refs, const vec3i_list& omits)  {
    MergeStats stats;
    // search for the inversion matrix
    int inverseMatIndex = -1;
    mat3d mI;
    mI.I() *= -1;
    for( int i=0; i < ml.Count(); i++ )  {
      if( ml[i].r == mI )  {
        inverseMatIndex = i;
        break;
      }
    }
    stats.FriedelOppositesMerged = (inverseMatIndex != -1);
    // standartise reflection indexes according to provieded list of symmetry operators
    const int ref_cnt = refs.Count();
    for( int i=0; i < ref_cnt; i++ )
      refs[i]->Standardise(ml);
    // sort the list
    TReflection::SortPList(refs);
    // merge reflections
    double Sdiff = 0, SI = 0, SS = 0, SI_tot = 0;
    TReflection* ref = refs[0];  // reference reflection
    for( int i=0; i < ref_cnt; )  {
      int from = i;
      while( (++i < ref_cnt) && (ref->CompareTo(*refs[i]) == 0) )
        ;
      bool omitted = false;
      const int merged_count = i - from;
      for( int j=0; j < omits.Count(); j++ )  {
        if( ref->GetH() == omits[j][0] &&
          ref->GetK() == omits[j][1] &&
          ref->GetL() == omits[j][2] )  
        {
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
          DryMergerOut mo = RefListMerger::DryMerge( refs, from, i );
          if( merged_count > 1 )  {
            SI_tot += mo.sumI;
            Sdiff += mo.sumDiff;
            if( mo.sigInt > mo.rSig )  {
              if( mo.sigInt > 5*mo.rSig )
                stats.InconsistentEquivalents ++;
              mo.rSig = mo.sigInt;
            }
          }
          ref->Analyse(ml);
          if( ref->IsCentric() )
            stats.CentricReflections++;
          SS += mo.rSig;
          SI += mo.rI;
          stats.MeanIOverSigma += mo.rI/mo.rSig;
          if( ref->GetH() > stats.MaxIndexes[0] )  stats.MaxIndexes[0] = ref->GetH();
          if( ref->GetK() > stats.MaxIndexes[1] )  stats.MaxIndexes[1] = ref->GetK();
          if( ref->GetL() > stats.MaxIndexes[2] )  stats.MaxIndexes[2] = ref->GetL();
          if( ref->GetH() < stats.MinIndexes[0] )  stats.MinIndexes[0] = ref->GetH();
          if( ref->GetK() < stats.MinIndexes[1] )  stats.MinIndexes[1] = ref->GetK();
          if( ref->GetL() < stats.MinIndexes[2] )  stats.MinIndexes[2] = ref->GetL();
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
    if( stats.UniqueReflections != 0 )
      stats.MeanIOverSigma /= stats.UniqueReflections;
    return stats;
  }

  template <class RefListMerger>
  static MergeStats DoMergeInP1(TPtrList<const TReflection>& refs, const vec3i_list& omits, TRefList& output)  {
    MergeStats stats;
    // sort the list
    TReflection::SortPList(refs);
    const int ref_cnt = refs.Count();
    output.SetCapacity( ref_cnt ); // better more that none :)
    // merge reflections
    double Sdiff = 0, SI_tot = 0, SI = 0, SS = 0;
    const TReflection* ref = refs[0];  // reference reflection
    for( int i=0; i < ref_cnt; )  {
      const int from = i;
      while( (++i < ref_cnt) && (ref->CompareTo(*refs[i]) == 0) )
        ;
      const int merged_count = i - from;
      bool omitted = false;
      for( int j=0; j < omits.Count(); j++ )  {
        if( ref->GetH() == omits[j][0] &&
            ref->GetK() == omits[j][1] &&
            ref->GetL() == omits[j][2] )  
        {
          stats.OmittedByUser += merged_count;
          omitted = true;
          break;
        }
      }
      if( !omitted )  {
        if( merged_count > stats.ReflectionAPotMax )
          stats.ReflectionAPotMax = merged_count;
        MergerOut mo = RefListMerger::Merge( refs, from, i );
        if( merged_count > 1 )  {
          SI_tot += mo.sumI;
          Sdiff += mo.sumDiff;
          if( mo.sigInt > mo.ref->GetS() )  {
            if( mo.sigInt > 5*mo.ref->GetS() )  {
              stats.InconsistentEquivalents ++;
              mo.ref->SetTag(-1);  // mark as unusable
            }
            mo.ref->SetS( mo.sigInt );
          }
          mo.ref->SetFlag( ref->GetFlag() );
        }
        output.Add(mo.ref);
        SS += mo.ref->GetS();
        SI += mo.ref->GetI();
        stats.MeanIOverSigma += mo.ref->GetI()/mo.ref->GetS();
        if( ref->GetH() > stats.MaxIndexes[0] )  stats.MaxIndexes[0] = ref->GetH();
        if( ref->GetK() > stats.MaxIndexes[1] )  stats.MaxIndexes[1] = ref->GetK();
        if( ref->GetL() > stats.MaxIndexes[2] )  stats.MaxIndexes[2] = ref->GetL();
        if( ref->GetH() < stats.MinIndexes[0] )  stats.MinIndexes[0] = ref->GetH();
        if( ref->GetK() < stats.MinIndexes[1] )  stats.MinIndexes[1] = ref->GetK();
        if( ref->GetL() < stats.MinIndexes[2] )  stats.MinIndexes[2] = ref->GetL();
      }
      if( i >= ref_cnt )  break;
      ref = refs[i];
    }
    stats.Rint = (SI_tot != 0) ? Sdiff/SI_tot : 0.0;
    stats.Rsigma = (SI != 0) ? SS/SI : 0.0;
    stats.UniqueReflections = output.Count();
    if( stats.UniqueReflections != 0 )
      stats.MeanIOverSigma /= stats.UniqueReflections;
    return stats;
  }
  template <class RefListMerger>
  static MergeStats DryMergeInP1(TPtrList<const TReflection>& refs, const vec3i_list& omits)  {
    MergeStats stats;
    // sort the list
    TReflection::SortPList(refs);
    // merge reflections
    const int ref_cnt = refs.Count();
    double Sdiff = 0, SI_tot = 0, SI = 0, SS = 0;
    const TReflection* ref = refs[0];
    for( int i=0; i < ref_cnt; )  {
      const int from = i;
      while( (++i < ref_cnt) && (ref->CompareTo(*refs[i]) == 0) )
        ;
      const int merged_count = i - from;
      bool omitted = false;
      for( int j=0; j < omits.Count(); j++ )  {
        if( ref->GetH() == omits[j][0] &&
            ref->GetK() == omits[j][1] &&
            ref->GetL() == omits[j][2] )  
        {
          stats.OmittedByUser += merged_count;
          omitted = true;
          break;
        }
      }
      if( !omitted )  {
        if( merged_count > stats.ReflectionAPotMax )
          stats.ReflectionAPotMax = merged_count;
        DryMergerOut mo = RefListMerger::DryMerge( refs, from, i );
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
        if( ref->GetH() > stats.MaxIndexes[0] )  stats.MaxIndexes[0] = ref->GetH();
        if( ref->GetK() > stats.MaxIndexes[1] )  stats.MaxIndexes[1] = ref->GetK();
        if( ref->GetL() > stats.MaxIndexes[2] )  stats.MaxIndexes[2] = ref->GetL();
        if( ref->GetH() < stats.MinIndexes[0] )  stats.MinIndexes[0] = ref->GetH();
        if( ref->GetK() < stats.MinIndexes[1] )  stats.MinIndexes[1] = ref->GetK();
        if( ref->GetL() < stats.MinIndexes[2] )  stats.MinIndexes[2] = ref->GetL();
        stats.UniqueReflections++;
      }
      if( i >= ref_cnt )  break;
      ref = refs[i];
    }
    stats.Rint = (SI_tot != 0) ? Sdiff/SI_tot : 0.0;
    stats.Rsigma = (SI != 0) ? SS/SI : 0.0;
    if( stats.UniqueReflections != 0 )
      stats.MeanIOverSigma /= stats.UniqueReflections;
    return stats;
  }
public:
/* Function merges provided reflections using provided list of matrices. 
   The reflections are standardised. The resulting reflections are stored in the output .
*/
  template <class RefListMerger, class RefList> static MergeStats Merge(smatd_list& ml, RefList& Refs, TRefList& output, const vec3i_list& omits)  {
    TRefPList refs( Refs.Count() );  // list of replicated reflections
    for( int i=0; i < Refs.Count(); i++ )
      refs[i] = TReflection::RefP(Refs[i]);
    return DoMerge<RefListMerger>(ml, refs, omits, output);
  }
  /* Functions gets the statistic on the list of provided reflections (which get stantardised) */
  template <class RefListMerger, class RefList> static MergeStats DryMerge(smatd_list& ml, RefList& Refs, const vec3i_list& omits)  {
    TRefPList refs( Refs.Count() );  // list of replicated reflections
    for( int i=0; i < Refs.Count(); i++ ) 
      refs[i] = TReflection::RefP(Refs[i]);
    return DryMerge<RefListMerger>(ml, refs, omits);
  }
  /* The function merges provided reflections in P1 and strores the result in the output */
  template <class RefListMerger, class RefList> static MergeStats MergeInP1(const RefList& Refs, TRefList& output, const vec3i_list& omits)  {
    TPtrList<const TReflection> refs( Refs.Count() );
    for( int i=0; i < Refs.Count(); i++ )
      refs[i] = TReflection::GetRefP(Refs[i]);
    return DoMergeInP1<RefListMerger>(refs, omits, output);
  }
  template <class RefListMerger, class RefList> static MergeStats DryMergeInP1(const RefList& Refs, const vec3i_list& omits)  {
    TPtrList<const TReflection> refs( Refs.Count() );
    for( int i=0; i < Refs.Count(); i++ )
      refs[i] = TReflection::GetRefP(Refs[i]);
    return DryMergeInP1<RefListMerger>(refs, omits);
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
      for( int l=0; l < rl.Count(); l++ )  {
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
    template <class RefPList> static MergerOut Merge(const RefPList& rl, int from, int to)  {
      const int cnt = to-from;
      if( cnt == 1 )  return MergerOut(new TReflection(*rl[from]) );
      double sum_wght = 0, sum_wght_i = 0, sum_i = 0;
      for( int i=from; i < to; i++ )  {
        const double w = 1./(rl[i]->GetS() != 0 ? rl[i]->GetS() : 0.001);
        sum_wght += w;
        sum_wght_i += w*rl[i]->GetI();
        sum_i += rl[i]->GetI();
      }
      const double mean = sum_wght_i/sum_wght;
      double sum_diff = 0, summ_i = 0, sig_top = 0;
      for( int i=from; i < to; i++ )  {
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
    template <class RefPList> static DryMergerOut DryMerge(const RefPList& rl, int from, int to)  {
      const int cnt = to-from;
      if( cnt == 1 )  return DryMergerOut(rl[from]->GetI(), rl[from]->GetS() );
      double sum_wght = 0, sum_wght_i = 0, sum_i = 0;
      for( int i=from; i < to; i++ )  {
        const double w = 1./(rl[i]->GetS() != 0 ? rl[i]->GetS() : 0.001);
        sum_wght += w;
        sum_wght_i += w*rl[i]->GetI();
        sum_i += rl[i]->GetI();
      }
      const double mean = sum_wght_i/sum_wght;
      double sum_diff = 0, summ_i = 0, sig_top = 0;
      for( int i=from; i < to; i++ )  {
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
    template <class RefPList> static MergerOut Merge(const RefPList& rl, int from, int to)  {
      const int cnt = to-from;
      if( cnt == 1 )  return MergerOut(new TReflection(*rl[from]) );
      double sum_sig_sq = 0, sum_i = 0;
      for( int i=from; i < to; i++ )  {
        sum_i += rl[i]->GetI();
        sum_sig_sq += rl[i]->GetS()*rl[i]->GetS();
      }
      const double mean = sum_i/cnt;
      double sum_diff = 0, sum_diff_sq = 0;
      for( int i=from; i < to; i++ )  {
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
    template <class RefPList> static DryMergerOut DryMerge(const RefPList& rl, int from, int to)  {
      const int cnt = to-from;
      if( cnt == 1 )  return DryMergerOut(rl[from]->GetI(), rl[from]->GetS() );
      double sum_sig_sq = 0, sum_i = 0;
      for( int i=from; i < to; i++ )  {
        sum_i += rl[i]->GetI();
        sum_sig_sq += rl[i]->GetS()*rl[i]->GetS();
      }
      const double mean = sum_i/rl.Count();
      double sum_diff = 0, sum_diff_sq = 0;
      for( int i=from; i < to; i++ )  {
        const double diff = rl[i]->GetI() - mean;
        sum_diff += (diff < 0 ? -diff : diff);
        sum_diff_sq += diff*diff;
      }
      return MergerOut( 
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
    template <class RefPList> static MergerOut Merge(const RefPList& rl, int from, int to)  {
      const int cnt = to-from;
      if( cnt == 1 )  return MergerOut(new TReflection(*rl[from]) );
      double sum_wght = 0, sum_wght_i = 0, sum_rec_sig_sq = 0;
      for( int i=from; i < to; i++ )  {
        const double s = rl[i]->GetS() != 0 ? rl[i]->GetS() : 0.001;
        const double rec_sig_sq = 1./(s*s);
        const double w = (rl[i]->GetI() > 3.0*s) ? rl[i]->GetI()*rec_sig_sq : 3./s;
        sum_wght += w;
        sum_rec_sig_sq += rec_sig_sq;
        sum_wght_i += w*rl[i]->GetI();
      }
      const double mean = sum_wght_i/sum_wght;
      double sum_diff = 0, sum_i = 0;
      for( int i=from; i < to; i++ )  {
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
    template <class RefPList> static DryMergerOut DryMerge(const RefPList& rl, int from, int to)  {
      const int cnt = to - from;
      if( cnt == 1 )  return DryMergerOut(rl[from]->GetI(), rl[from]->GetS());
      double sum_wght = 0, sum_wght_i = 0, sum_rec_sig_sq = 0;
      for( int i=from; i < to; i++ )  {
        const double s = rl[i]->GetS() != 0 ? rl[i]->GetS() : 0.001;
        const double rec_sig_sq = 1./(s*s);
        const double w = (rl[i]->GetI() > 3.0*s) ? rl[i]->GetI()*rec_sig_sq : 3./s;
        sum_wght += w;
        sum_rec_sig_sq += rec_sig_sq;
        sum_wght_i += w*rl[i]->GetI();
      }
      const double mean = sum_wght_i/sum_wght;
      double sum_diff = 0, sum_i = 0;
      for( int i=from; i < to; i++ )  {
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
