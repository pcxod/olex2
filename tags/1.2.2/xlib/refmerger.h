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
  double Rint, Rsigma;
  int SystematicAbsentcesRemoved,
    InconsistentEquivalents,
    TotalReflections,
    UniqueReflections,
    OmittedReflections,
    CentricReflections;
  bool FriedelOppositesMerged;
  MergeStats()  {
    Reset();
  }
  // no need to define assignment operator or copy constructor for this one...
  // resets all values...
  void Reset()  {
    Rint = Rsigma = 0;
    TotalReflections = UniqueReflections = OmittedReflections = 0;
    CentricReflections = SystematicAbsentcesRemoved = InconsistentEquivalents = 0;
    FriedelOppositesMerged = false;
  }
  bool IsEmpty() const {  return TotalReflections == 0;  }
};
//..............................................................................
class RefMerger {
public:
/* Function merges reflections of current hkl file (file data stays unchanged!)
 and fills the 'output' list with merged reflections. RefListmerger class must provide
 Merge(const TRefPList& refs) static function which returns a new merged reflection, this
 reflection will be automatically deleted afterwards.
 The refturned value is Rint = Sum(|F^2-F^2mean|)/Sum(|F^2|)
*/
template <class RefListMerger>
  static MergeStats Merge(smatd_list& ml, const TRefPList& Refs, TRefList& output)  {
    MergeStats stats;
    // replicate reflections, to leave this object as it is
    TRefPList refs, toMerge;  // list of replicated reflections
    refs.SetCapacity( Refs.Count() );
    for( int i=0; i < Refs.Count(); i++ )  {
      if( Refs[i]->GetTag() <= 0 )  continue;  // skip omited reflections
      TReflection* ref = refs.Add( new TReflection(*Refs[i]) );
      if( ref->GetI() < 0 )  ref->SetI(0);
    }
    stats.TotalReflections = Refs.Count();
    stats.OmittedReflections = Refs.Count() - refs.Count();
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
    double Sdiff = 0, SI = 0;
    toMerge.Add(refs[0]);  // reference reflection
    output.SetCapacity( ref_cnt ); // better more that none :)
    for( int i=0; i < ref_cnt; )  {
      while( (++i < ref_cnt) && (toMerge[0]->CompareTo(*refs[i]) == 0) )
        toMerge.Add( refs[i] );

      if( !toMerge[0]->IsAbsent() )  {
        TReflection &rf = *RefListMerger::Merge( toMerge );
        if( toMerge.Count() > 1 )  {
          double esd = 0;
          for( int l=0; l < toMerge.Count(); l++ )  {
            double id = toMerge[l]->GetI() - rf.GetI();
            esd += id*id;
            Sdiff += fabs(id);
            SI += toMerge[l]->GetI();
          }
          esd = sqrt(esd/(toMerge.Count()*(toMerge.Count()-1)));
          if( esd > rf.GetS() )  {
            if( esd > 5*rf.GetS() )
              stats.InconsistentEquivalents ++;
            rf.SetS( esd );
          }
        }
        output.Add(rf);
      }
      else
        stats.SystematicAbsentcesRemoved++;

      if( i >= ref_cnt )  break;

      toMerge.Clear();
      toMerge.Add( refs[i] );
    }

    for( int i=0; i < ref_cnt; i++ )
      delete refs[i];

    stats.Rint = (SI != 0) ? Sdiff/SI : 0.0;
    double SS = 0;
    SI = 0;
    if( inverseMatIndex != -1 )  // all reflection will be cenrtic othewise
      ml.Delete( inverseMatIndex );
    for( int i=0; i < output.Count(); i++ )  {
      output[i].Analyse(ml);
      output[i].SetTag(1);  // negative tag means the reflection is omitted
      if( output[i].IsCentric() )  stats.CentricReflections++;
      SS += output[i].GetS();
      SI += output[i].GetI();
    }
    stats.Rsigma = (SI != 0) ? SS/SI : 0.0;
    stats.UniqueReflections = output.Count();
    if( inverseMatIndex != -1 )  {
      smatd& i = ml.InsertNew(inverseMatIndex);
      i.r.I() *= -1;
    }
    return stats;
  }
template <class RefListMerger>
  static MergeStats Merge(smatd_list& ml, const TRefList& Refs, TRefList& output)  {
    MergeStats stats;
    // replicate reflections, to leave this object as it is
    TRefPList refs, toMerge;  // list of replicated reflections
    refs.SetCapacity( Refs.Count() );
    for( int i=0; i < Refs.Count(); i++ )  {
      if( Refs[i].GetTag() <= 0 )  continue;  // skip omited reflections
      TReflection* ref = refs.Add( new TReflection(Refs[i]) );
      if( ref->GetI() < 0 )  ref->SetI(0);
    }
    stats.TotalReflections = Refs.Count();
    stats.OmittedReflections = Refs.Count() - refs.Count();
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
    double Sdiff = 0, SI = 0;
    toMerge.Add(refs[0]);  // reference reflection
    output.SetCapacity( ref_cnt ); // better more that none :)
    for( int i=0; i < ref_cnt; )  {
      while( (++i < ref_cnt) && (toMerge[0]->CompareTo(*refs[i]) == 0) )
        toMerge.Add( refs[i] );

      if( !toMerge[0]->IsAbsent() )  {
        TReflection &rf = *RefListMerger::Merge( toMerge );
        if( toMerge.Count() > 1 )  {
          double esd = 0;
          for( int l=0; l < toMerge.Count(); l++ )  {
            double id = toMerge[l]->GetI() - rf.GetI();
            esd += id*id;
            Sdiff += fabs(id);
            SI += toMerge[l]->GetI();
          }
          esd = sqrt(esd/(toMerge.Count()*(toMerge.Count()-1)));
          if( esd > rf.GetS() )  {
            if( esd > 5*rf.GetS() )
              stats.InconsistentEquivalents ++;
            rf.SetS( esd );
          }
        }
        output.Add(rf);
      }
      else
        stats.SystematicAbsentcesRemoved++;

      if( i >= ref_cnt )  break;

      toMerge.Clear();
      toMerge.Add( refs[i] );
    }

    for( int i=0; i < ref_cnt; i++ )
      delete refs[i];

    stats.Rint = (SI != 0) ? Sdiff/SI : 0.0;
    double SS = 0;
    SI = 0;
    if( inverseMatIndex != -1 )  // all reflection will be cenrtic othewise
      ml.Delete( inverseMatIndex );
    for( int i=0; i < output.Count(); i++ )  {
      output[i].Analyse(ml);
      output[i].SetTag(1);  // negative tag means the reflection is omitted
      if( output[i].IsCentric() )  stats.CentricReflections++;
      SS += output[i].GetS();
      SI += output[i].GetI();
    }
    stats.Rsigma = (SI != 0) ? SS/SI : 0.0;
    stats.UniqueReflections = output.Count();
    if( inverseMatIndex != -1 )  {
      smatd& i = ml.InsertNew(inverseMatIndex);
      i.r.I() *= -1;
    }
    return stats;
  }
};

EndXlibNamespace()
#endif
