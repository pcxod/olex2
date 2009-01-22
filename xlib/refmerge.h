#ifndef __olx_ref_merge_H
#define __olx_ref_merge_H
#include "reflection.h"
BeginXlibNamespace()

struct MergeStats  {
  double Rint, Rsigma;
  int SystematicAbsentcesRemoved,
    InconsistentEquivalents,
    TotalReflections,
    UniqueReflections,
    OmittedReflections,
    CentricReflections;
  bool FriedelOppositesMerged;
  MergeStats()  {
    SetDefaults();
  }
  MergeStats(const MergeStats& ms) {
    this->operator = (ms);
  }
  MergeStats& operator = (const MergeStats& ms) {
    Rint = ms.Rint;
    Rsigma = ms.Rsigma;
    SystematicAbsentcesRemoved = ms.SystematicAbsentcesRemoved;
    InconsistentEquivalents = ms.InconsistentEquivalents;
    TotalReflections = ms.TotalReflections;
    UniqueReflections = ms.UniqueReflections;
    OmittedReflections = ms.OmittedReflections;
    CentricReflections = ms.CentricReflections;
    FriedelOppositesMerged = ms.FriedelOppositesMerged;
    return *this;
  }
  void SetDefaults()  {
    Rint = Rsigma = 0;
    TotalReflections = UniqueReflections = OmittedReflections = 0;
    CentricReflections = SystematicAbsentcesRemoved = InconsistentEquivalents = 0;
    FriedelOppositesMerged = false;
  }
  bool IsEmpty() const {  return TotalReflections == 0;  }
};
//..............................................................................
class RefMerger {
  template <class RefListMerger, class RefList> 
  static MergeStats DoMerge(smatd_list& ml, TRefPList& refs, const RefList& original, TRefList& output)  {
    MergeStats stats;
    // replicate reflections, to leave this object as it is
    TRefPList toMerge;  // list of replicated reflections
    stats.TotalReflections = original.Count();
    stats.OmittedReflections = original.Count() - refs.Count();
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
    toMerge.Add(refs[0]);  // reference reflection
    output.SetCapacity( ref_cnt ); // better more that none :)
    for( int i=0; i < ref_cnt; )  {
      while( (++i < ref_cnt) && (toMerge[0]->CompareTo(*refs[i]) == 0) )
        toMerge.Add( refs[i] );
      if( !toMerge[0]->IsAbsent() )  {
        // the reflection is replicated if only one in the list
        MergerOut mo = RefListMerger::Merge( toMerge );
        if( toMerge.Count() > 1 )  {
          SI_tot += mo.sumI;
          Sdiff += mo.sumDiff;
          if( mo.sigInt > mo.ref->GetS() )  {
            if( mo.sigInt > 5*mo.ref->GetS() )
              stats.InconsistentEquivalents ++;
            mo.ref->SetS( mo.sigInt );
          }
          mo.ref->SetFlag( toMerge[0]->GetFlag() );
        }
        output.Add(mo.ref).Analyse(ml);
        if( mo.ref->IsCentric() )
          stats.CentricReflections++;
        SS += mo.ref->GetS();
        SI += mo.ref->GetI();
      }
      else
        stats.SystematicAbsentcesRemoved += toMerge.Count();

      if( i >= ref_cnt )  break;

      toMerge.Clear();
      toMerge.Add( refs[i] );
    }

    for( int i=0; i < ref_cnt; i++ )
      delete refs[i];

    stats.Rint = (SI_tot != 0) ? Sdiff/SI_tot : 0.0;
    //if( inverseMatIndex != -1 )  // all reflection will be cenrtic othewise
    //  ml.Delete( inverseMatIndex );
    stats.Rsigma = (SI != 0) ? SS/SI : 0.0;
    stats.UniqueReflections = output.Count();
    //if( inverseMatIndex != -1 )  {
    //  smatd& i = ml.InsertNew(inverseMatIndex);
    //  i.r.I() *= -1;
    // }
    return stats;
  }
  template <class RefListMerger, class RefList>
  static MergeStats DoMergeInP1(TPtrList<const TReflection>& refs, const RefList& original, TRefList& output)  {
    MergeStats stats;
    // replicate reflections, to leave this object as it is
    TPtrList<const TReflection> toMerge;  // list of replicated reflections
    stats.TotalReflections = original.Count();
    // sort the list
    TReflection::SortPList(refs);
    // merge reflections
    toMerge.Add(refs[0]);  // reference reflection
    const int ref_cnt = refs.Count();
    output.SetCapacity( ref_cnt ); // better more that none :)
    double Sdiff = 0, SI_tot = 0, SI = 0, SS = 0;
    for( int i=0; i < ref_cnt; )  {
      while( (++i < ref_cnt) && (toMerge[0]->CompareTo(*refs[i]) == 0) )
        toMerge.Add( refs[i] );
      MergerOut mo = RefListMerger::Merge( toMerge );
      if( toMerge.Count() > 1 )  {
        SI_tot += mo.sumI;
        Sdiff += mo.sumDiff;
        if( mo.sigInt > mo.ref->GetS() )  {
          if( mo.sigInt > 5*mo.ref->GetS() )
            stats.InconsistentEquivalents ++;
          mo.ref->SetS( mo.sigInt );
        }
      }
      output.Add(mo.ref);
      SS += mo.ref->GetS();
      SI += mo.ref->GetI();
      if( i >= ref_cnt )  break;

      toMerge.Clear();
      toMerge.Add( refs[i] );
    }
    stats.Rint = (SI_tot != 0) ? Sdiff/SI_tot : 0.0;
    stats.Rsigma = (SI != 0) ? SS/SI : 0.0;
    stats.UniqueReflections = output.Count();
    return stats;
  }
public:
/* Function merges reflections of current hkl file (file data stays unchanged!)
 and fills the 'output' list with merged reflections. RefListmerger class must provide
 Merge(const TRefPList& refs) static function which returns a new merged reflection, this
 reflection will be automatically deleted afterwards.
 The refturned value is Rint = Sum(|F^2-F^2mean|)/Sum(|F^2|)
*/
  template <class RefListMerger> static MergeStats Merge(smatd_list& ml, const TRefPList& Refs, TRefList& output)  {
    TRefPList refs;  // list of replicated reflections
    refs.SetCapacity( Refs.Count() );
    for( int i=0; i < Refs.Count(); i++ )  {
      if( Refs[i]->GetTag() <= 0 )  continue;  // skip omited reflections
      refs.Add( new TReflection(*Refs[i]) );
    }
    return DoMerge<RefListMerger, TRefPList>(ml, refs, Refs, output);
  }
  template <class RefListMerger> static MergeStats Merge(smatd_list& ml, const TRefList& Refs, TRefList& output)  {
    // replicate reflections, to leave this object as it is
    TRefPList refs;  // list of replicated reflections
    refs.SetCapacity( Refs.Count() );
    for( int i=0; i < Refs.Count(); i++ )  {
      if( Refs[i].GetTag() <= 0 )  continue;  // skip omited reflections
      refs.Add( new TReflection(Refs[i]) );
    }
    return DoMerge<RefListMerger, TRefList>(ml, refs, Refs, output);
  }
  template <class RefListMerger> static MergeStats MergeInP1(const TRefPList& Refs, TRefList& output)  {
    TPtrList<const TReflection> refs;
    refs.SetCapacity( Refs.Count() );
    for( int i=0; i < Refs.Count(); i++ )  {
      if( Refs[i]->GetTag() <= 0 )  continue;  // skip omited reflections
      refs.Add( Refs[i] );
    }
    return DoMergeInP1<RefListMerger,TRefPList>(refs, Refs, output);
  }
  template <class RefListMerger> static MergeStats MergeInP1(const TRefList& Refs, TRefList& output)  {
    TPtrList<const TReflection> refs;
    refs.SetCapacity( Refs.Count() );
    for( int i=0; i < Refs.Count(); i++ )  {
      if( Refs[i]->GetTag() <= 0 )  continue;  // skip omited reflections
      refs.Add( &Refs[i] );
    }
    return DoMergeInP1<RefListMerger,TRefList>(refs, Refs, output);
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
    template <class RefPList> static MergerOut Merge(const RefPList& rl)  {
      if( rl.IsEmpty() )  throw TInvalidArgumentException(__OlxSourceInfo, "empty reflection list");
      if( rl.Count() == 1 )  return MergerOut(new TReflection(*rl[0]) );
      double sum_wght = 0, sum_wght_i = 0, sum_i = 0;
      for( int i=0; i < rl.Count(); i++ )  {
        const double w = 1./(rl[i]->GetS() != 0 ? rl[i]->GetS() : 0.001);
        sum_wght += w;
        sum_wght_i += w*rl[i]->GetI();
        sum_i += rl[i]->GetI();
      }
      const double mean = sum_wght_i/sum_wght;
      double sum_diff = 0, summ_i = 0, sig_top = 0;
      for( int i=0; i < rl.Count(); i++ )  {
        const double diff = rl[i]->GetI() - mean;
        sum_diff += (diff < 0 ? -diff : diff);
        summ_i += rl[i]->GetI();
        const double w = 1./(rl[i]->GetS() != 0 ? rl[i]->GetS() : 0.001);
        sig_top += w*diff*diff; 
      }
      return MergerOut( new TReflection( rl[0]->GetH(), rl[0]->GetK(), rl[0]->GetL(), mean, 1./sqrt(sum_wght)), 
        sqrt(sig_top/(sum_wght*(rl.Count()-1))), 
        sum_i, 
        sum_diff);
    }
  };
  class UnitMerger  {
  public:
    // returns a newly created reflection do be deleted with delete
    template <class RefPList> static MergerOut Merge(const RefPList& rl)  {
      if( rl.IsEmpty() )  throw TInvalidArgumentException(__OlxSourceInfo, "empty reflection list");
      if( rl.Count() == 1 )  return MergerOut(new TReflection(*rl[0]) );
      double sum_sig_sq = 0, sum_i = 0;
      for( int i=0; i < rl.Count(); i++ )  {
        sum_i += rl[i]->GetI();
        sum_sig_sq += rl[i]->GetS()*rl[i]->GetS();
      }
      const double mean = sum_i/rl.Count();
      double sum_diff = 0, sum_diff_sq = 0;
      for( int i=0; i < rl.Count(); i++ )  {
        const double diff = rl[i]->GetI() - mean;
        sum_diff += (diff < 0 ? -diff : diff);
        sum_diff_sq += diff*diff;
      }
      return MergerOut( new TReflection( rl[0]->GetH(), rl[0]->GetK(), rl[0]->GetL(), mean, sqrt(sum_sig_sq/rl.Count())), 
        sqrt(sum_diff_sq/(rl.Count()*(rl.Count()-1))), 
        sum_i, 
        sum_diff);
    }
  };
/* merges using George's approach 
   http://www.crystal.chem.uu.nl/distr/mergehklf5/mergehklf5.html
*/
  class ShelxMerger  {
  public:
    // returns a newly created reflection do be deleted with delete
    template <class RefPList> static MergerOut Merge(const RefPList& rl)  {
      if( rl.IsEmpty() )  throw TInvalidArgumentException(__OlxSourceInfo, "empty reflection list");
      if( rl.Count() == 1 )  return MergerOut(new TReflection(*rl[0]) );
      double sum_wght = 0, sum_wght_i = 0, sum_rec_sig_sq = 0;
      for( int i=0; i < rl.Count(); i++ )  {
        const double s = rl[i]->GetS() != 0 ? rl[i]->GetS() : 0.001;
        const double rec_sig_sq = 1./(s*s);
        const double w = (rl[i]->GetI() > 3.0*s) ? rl[i]->GetI()*rec_sig_sq : 3./s;
        sum_wght += w;
        sum_rec_sig_sq += rec_sig_sq;
        sum_wght_i += w*rl[i]->GetI();
      }
      const double mean = sum_wght_i/sum_wght;
      double sum_diff = 0, sum_i = 0;
      for( int i=0; i < rl.Count(); i++ )  {
        const double diff = rl[i]->GetI() - mean;
        sum_diff += (diff < 0 ? -diff : diff);
        sum_i += rl[i]->GetI();
      }
      return MergerOut( new TReflection( rl[0]->GetH(), rl[0]->GetK(), rl[0]->GetL(), mean, 1./sqrt(sum_rec_sig_sq)), 
        sum_diff/(rl.Count()*sqrt((double)rl.Count()-1.0)), 
        sum_i, 
        sum_diff);
    }
  };
};

EndXlibNamespace()
#endif
