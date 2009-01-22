#ifndef __olx_ref_util_H
#define __olx_ref_util_H
/*  The tests have shown, that using FastSymm for reflection merging is not appropriate.
  So this file is just a prove of the concept... User RefMerger instead!
  Although some ideas, like using matrices of the unit cell , when the space group
  information is not available need to be investigated further.
*/
#include "xbase.h"
#include "symmat.h"
#include "refmerge.h"
#include "fastsymm.h"

BeginXlibNamespace()

class IRef_analysis {
public:
  virtual void Standardize(TRefPList& refs) const = 0;
  virtual void Standardize(TRefList& refs) const = 0;
  virtual int GetSGOrder() const = 0;
};

template <class sg> class Ref_analysis : public IRef_analysis {
public:
  virtual void Standardize(TRefPList& refs) const {
    if( sg::size == 0 )  return;
    TArrayList<vec3i> rv(sg::size);
    TArrayList<double> ps(sg::size);
    const int ref_cnt = refs.Count();
    for( int i=0; i < ref_cnt; i++ )  {
      const vec3i ref(refs[i]->GetH(), refs[i]->GetK(), refs[i]->GetL());
      const vec3i neg_ref(-refs[i]->GetH(), -refs[i]->GetK(), -refs[i]->GetL());
      sg::GenHkl(ref, rv, ps);
      // index 0 is always the original
      for( int j=1; j < sg::size; j++ )  {
        if( (rv[j][2] > ref[2]) ||        // sdandardise then ...
            ((rv[j][2] == ref[2]) && (rv[j][1] > ref[1])) ||
            ((rv[j][2] == ref[2]) && (rv[j][1] == ref[1]) && (rv[j][0] > ref[0])) )    
        {
          refs[i]->SetH(rv[j][0]);  
          refs[i]->SetK(rv[j][1]);  
          refs[i]->SetL(rv[j][2]);  
        }
        if( rv[j] == ref )  {
          refs[i]->IncMultiplicity();
          if( !refs[i]->IsAbsent() && olx_abs( ps[j] - Round(ps[j]) ) > 0.01 )
            refs[i]->SetAbsent( true );
        }
        else if( !refs[i]->IsCentric() && rv[j] == neg_ref )  {
          refs[i]->SetCentric( true );
        }
      }
    }
  }
  virtual void Standardize(TRefList& refs) const {
    if( sg::size == 0 )  return;
    TArrayList<vec3i> rv(sg::size);
    TArrayList<double> ps(sg::size);
    const int ref_cnt = refs.Count();
    for( int i=0; i < ref_cnt; i++ )  {
      const vec3i ref(refs[i].GetH(), refs[i].GetK(), refs[i].GetL());
      const vec3i neg_ref(-refs[i].GetH(), -refs[i].GetK(), -refs[i].GetL());
      sg::GenHkl(ref, rv, ps);
      // index 0 is always the original
      for( int j=1; j < sg::size; j++ )  {
        if( (rv[j][2] > ref[2]) ||        // sdandardise then ...
            ((rv[j][2] == ref[2]) && (rv[j][1] > ref[1])) ||
            ((rv[j][2] == ref[2]) && (rv[j][1] == ref[1]) && (rv[j][0] > ref[0])) )    
        {
          refs[i].SetH(rv[j][0]);  
          refs[i].SetK(rv[j][1]);  
          refs[i].SetL(rv[j][2]);  
        }
        if( rv[j] == ref )  {
          refs[i].IncMultiplicity();
          if( !refs[i].IsAbsent() && olx_abs( ps[j] - Round(ps[j]) ) > 0.01 )
            refs[i].SetAbsent( true );
        }
        else if( !refs[i].IsCentric() && rv[j] == neg_ref )  {
          refs[i].SetCentric( true );
        }
      }
    }
  }
  virtual int GetSGOrder() const {  return sg::size;  }
};

/* the class does the reflection analysis "safely". So if there is not space group can be determined
 for the loaded file, the matrices of the unit cell are used instead of the FastSymm approach with
 the class defined above
*/

class RefUtil {
  // functions for undefined space group
  static void _Analyse(const smatd_list ml, TRefPList& refs, MergeStats& ms)  {
    if( ml.IsEmpty() )  return;
    const int ref_cnt = refs.Count();
    for( int i=0; i < ref_cnt; i++ )  {
      refs[i]->Analyse(ml);
      if( refs[i]->IsCentric() )
        ms.CentricReflections++;
    }
  }
  static void _Analyse(const smatd_list ml, TRefList& refs, MergeStats& ms)  {
    if( ml.IsEmpty() )  return;
    const int ref_cnt = refs.Count();
    for( int i=0; i < ref_cnt; i++ )  {
      refs[i].Analyse(ml);
      if( refs[i].IsCentric() )
        ms.CentricReflections++;
    }
  }
  static void _Standardize(const smatd_list ml, TRefPList& refs)  {
    if( ml.IsEmpty() )  return;
    const int ref_cnt = refs.Count();
    for( int i=0; i < ref_cnt; i++ )
      refs[i]->Standardise(ml);
  }
  static void _Standardize(const smatd_list ml, TRefList& refs)  {
    if( ml.IsEmpty() )  return;
    const int ref_cnt = refs.Count();
    for( int i=0; i < ref_cnt; i++ )
      refs[i].Standardise(ml);
  }
  smatd_list matrices;
  IRef_analysis* analyser;
  bool Centrosymmetric;
public:
  RefUtil();
  ~RefUtil()  {
    if( analyser != NULL )
      delete analyser;
  }

  DefPropB(Centrosymmetric)

  void Standardize(TRefPList& refs) const {
    if( analyser != NULL )  analyser->Standardize(refs);
    else                    _Standardize(matrices, refs);
  }
  void Standardize(TRefList& refs) const {
    if( analyser != NULL )  analyser->Standardize(refs);
    else                    _Standardize(matrices, refs);
  }

protected:
  template <class RefListMerger, class RefList> 
  static MergeStats DoMerge(TRefPList& refs, const RefList& original, TRefList& output)  {
    MergeStats stats;
    RefUtil rf;
    // replicate reflections, to leave this object as it is
    TRefPList toMerge;  // list of replicated reflections
    stats.TotalReflections = original.Count();
    stats.OmittedReflections = original.Count() - refs.Count();
    stats.FriedelOppositesMerged = rf.IsCentrosymmetric();
    // standartise reflection indexes according to provieded list of symmetry operators
    rf.Standardize(refs);
    // sort the list
    TReflection::SortPList(refs);
    // merge reflections
    double Sdiff = 0, SI_tot = 0, SI = 0, SS = 0;
    toMerge.Add(refs[0]);  // reference reflection
    const int ref_cnt = refs.Count();
    output.SetCapacity( ref_cnt ); // better more that none :)
    for( int i=0; i < ref_cnt; )  {
      while( (++i < ref_cnt) && (toMerge[0]->CompareTo(*refs[i]) == 0) )
        toMerge.Add( refs[i] );
      if( !toMerge[0]->IsAbsent() )  {
        RefMerger::MergerOut mo = RefListMerger::Merge( toMerge );
        if( toMerge.Count() > 1 )  {
          SI_tot += mo.sumI;
          Sdiff += mo.sumDiff;
          if( mo.sigInt > mo.ref->GetS() )  {
            if( mo.sigInt > 5*mo.ref->GetS() )
              stats.InconsistentEquivalents ++;
            mo.ref->SetS( mo.sigInt );
          }
          if( toMerge[0]->IsCentric() )  {
            stats.CentricReflections++;
            mo.ref->SetCentric(true);
          }
          mo.ref->SetMultiplicity( toMerge[0]->GetMultiplicity() );
          mo.ref->SetFlag( toMerge[0]->GetFlag() );
        }
        output.Add(mo.ref);
        SS += mo.ref->GetS();
        SI += mo.ref->GetI();
      }
      else
        stats.SystematicAbsentcesRemoved++;

      if( i >= ref_cnt )  break;

      toMerge.Clear();
      toMerge.Add( refs[i] );
    }

    for( int i=0; i < ref_cnt; i++ )
      delete refs[i];

    stats.Rint = (SI_tot != 0) ? Sdiff/SI_tot : 0.0;
    stats.Rsigma = (SI != 0) ? SS/SI : 0.0;
    stats.UniqueReflections = output.Count();
    return stats;
  }
public:
  template <class RefListMerger> static MergeStats Merge(const TRefList& Refs, TRefList& output)  {
    // replicate reflections, to leave this object as it is
    TRefPList refs;  // list of replicated reflections
    refs.SetCapacity( Refs.Count() );
    for( int i=0; i < Refs.Count(); i++ )  {
      if( Refs[i].GetTag() <= 0 )  continue;  // skip omited reflections
      refs.Add( new TReflection(Refs[i]) );
    }
    return DoMerge<RefListMerger, TRefList>(refs, Refs, output);
  }
  template <class RefListMerger> static MergeStats Merge(const TRefPList& Refs, TRefList& output)  {
    // replicate reflections, to leave this object as it is
    TRefPList refs;  // list of replicated reflections
    refs.SetCapacity( Refs.Count() );
    for( int i=0; i < Refs.Count(); i++ )  {
      if( Refs[i]->GetTag() <= 0 )  continue;  // skip omited reflections
      refs.Add( new TReflection(*Refs[i]) );
    }
    return DoMerge<RefListMerger, TRefPList>(refs, Refs, output);
  }

};

EndXlibNamespace()
#endif
