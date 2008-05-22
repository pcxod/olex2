#ifndef hklH
#define hklH

#include "xbase.h"
#include "evector.h"
#include "arrays.h"
#include "asymmunit.h"

#include "reflection.h"

BeginXlibNamespace()

class THklFile: public IEObject  {
  static int HklCmp(const TReflection* I1, const TReflection* I2);
  TRefPList Refs;
  TArray3D< TRefPList* > *Hkl3D;
protected:
  TVPoint<int> MaxHkl, MinHkl;
  double MaxI, MaxIS, MinI, MinIS;
  // the function must be caled before the reflection is added to the list, as
  // it needs to initialise the starting values of min and max
  void UpdateMinMax(const TReflection& r);
  void InitHkl3D();
  void Clear3D();
public:
  THklFile();
  virtual ~THklFile();

  void Append(const THklFile& hkls);
  void Append(const TRefPList& hkls);
  // the reflection will be deleted by this object
  void Append(TReflection& hkl);
  // function has to be called to sort the list of reflections
  void EndAppend();

  /*this function analyses if reflections are centric or non-centric and
    their degeneracy
   */
  void AnalyseReflections( const TSpaceGroup& sg );

  inline TReflection& Reflection(int i)  const {  return *Refs[i];  }
  inline TReflection& operator [](int i) const {  return *Refs[i];  }
  inline int RefCount() const                  {  return Refs.Count();  }

  inline const TVectorI& GetMaxHkl()  const {  return MaxHkl;  }
  inline const TVectorI& GetMinHkl()  const {  return MinHkl;  }
  inline double GetMaxI()             const { return MaxI;  }
  inline double GetMaxIS()            const { return MaxIS;  }
  inline double GetMinI()             const { return MinI;  }
  inline double GetMinIS()            const { return MinIS;  }

  void Clear();
  inline void Sort()  {  Refs.QuickSorter.SortSF(Refs, HklCmp);  }
  /* if ins loader is passed and the hkl file has CELL and SFAC in it, 
  it will be initalised
  */
  bool LoadFromFile(const olxstr& FN, class TIns* ins = NULL);
  bool SaveToFile(const olxstr& FN);
  void UpdateRef(const TReflection& R);
  // returns reflections owned by this object
  void AllRefs(int h, int k, int l, const TAsymmUnit& AU, TRefPList& Res)  {
    TReflection r(h, k, l);
    AllRefs(r, AU, Res);
  }
  template <class VC>
    void AllRefs(const TVector<VC> &hkl, const TAsymmUnit& AU, TRefPList& Res)  {
      AllRefs(hkl[0], hkl[1], hkl[2],AU, Res);
    }
  void AllRefs(const TReflection& R, const TAsymmUnit& AU, TRefPList& Res);

//..............................................................................
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
      Rint = Rsigma = 0;
      TotalReflections = UniqueReflections = OmittedReflections = 0;
      CentricReflections = SystematicAbsentcesRemoved = InconsistentEquivalents = 0;
      FriedelOppositesMerged = false;
    }
//    MergeStats(const MergeStats &ms)  {
//      Rint = ms.Rint;
//      Rsigma = ms.Rsigma;
//      SystematicAbsentcesRemoved = ms.SystematicAbsentcesRemoved;
//      InconsistentEquivalents = ms.InconsistentEquivalents;
//    }
  };
/* Function merges reflections of current hkl file (file data stays unchanged!)
 and fills the 'output' list with merged reflections. RefListmerger class must provide
 Merge(const TRefPList& refs) static function which returns a new merged reflection, this
 reflection will be automatically deleted afterwards.
 The refturned value is Rint = Sum(|F^2-F^2mean|)/Sum(|F^2|)
*/
template <class RefListMerger>
  MergeStats Merge(TMatrixDList& ml, TRefList& output)  {
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
    for( int i=0; i < ml.Count(); i++ )  {
      bool found = true;
      for( int j=0; j < 3; j++ )  {
        for( int k=0; k < 3; k++ )  {
          if( j == k && ml[i][j][k] != -1 )  {  found = false;  break;  }
          if( j != k && ml[i][j][k] != 0 )   {  found = false;  break;  }
        }
        if( !found )  break;
      }
      if( found )  {
        inverseMatIndex = i;
        break;
      }
    }

    stats.FriedelOppositesMerged = (inverseMatIndex != -1);
    // standartise reflection indexes according to provieded list of symmetry operators
    for( int i=0; i < refs.Count(); i++ )
      refs[i]->Standardise(ml, stats.FriedelOppositesMerged);
    // sort the list
    TReflection::SortPList(refs);
    // merge reflections
    double Sdiff = 0, SI = 0;
    toMerge.Add(refs[0]);  // reference reflection
    output.SetCapacity( refs.Count() ); // better more that none :)
    for( int i=0; i < refs.Count(); )  {
      while( (++i < refs.Count()) && (toMerge[0]->CompareTo(*refs[i]) == 0) )
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

      if( i >= refs.Count() )  break;

      toMerge.Clear();
      toMerge.Add( refs[i] );
    }

    for( int i=0; i < refs.Count(); i++ )
      delete refs[i];

    stats.Rint = Sdiff/SI;
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
    stats.Rsigma = SS/SI;
    stats.UniqueReflections = output.Count();
    return stats;
  }
  inline MergeStats SimpleMerge(TMatrixDList& ml, TRefList& output)  {
    return  Merge<TSimpleMerger>(ml, output);
  }
//..............................................................................
  MergeStats Merge(const class TSpaceGroup& AU, bool MergeInverse, TRefList& output);

  // saves to file a list of reflections
  static bool SaveToFile(const olxstr& FN, const TRefPList& Reflections, bool Append = true);
};
//---------------------------------------------------------------------------

EndXlibNamespace()
#endif

