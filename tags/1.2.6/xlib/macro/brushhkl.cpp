/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "egc.h"
#include "xmacro.h"
#include "hkl.h"
#include "symmlib.h"
#include "log.h"

struct HklBrushRef  {
  TReflection* ref;
  int H, K, L;
  bool Deleted;
  HklBrushRef(TReflection &r)  {
    ref = &r;
    H = r.GetH();  K = r.GetK();  L = r.GetL();
    Deleted = false;
  }
  int CompareTo(const HklBrushRef &r) const {
    int res = L - r.L;
    if( res == 0 )  {
      res = K - r.K;
      if( res == 0 )
        res = H - r.H;
    }
    return res;
  }
  template <class MatList> void Standardise(const MatList& ml, bool CheckInversion)  {
    for( size_t i=0; i < ml.Count(); i++ )  {
      vec3i hklv = (*ref)*ml[i];
      if( (hklv[2] > L) ||        // sdandardise then ...
          ((hklv[2] == L) && (hklv[1] > K)) ||
          ((hklv[2] == L) && (hklv[1] == K) && (hklv[0] > H)) )    {
          H = hklv[0];  K = hklv[1];  L = hklv[2];
      }
      if( CheckInversion )  {
        hklv *= -1;
        if( (hklv[2] > L) ||
          ((hklv[2] == L) && (hklv[1] > K) ) ||
          ((hklv[2] == L) && (hklv[1] == K) && (hklv[0] > H)) )   {
          H = hklv[0];  K = hklv[1];  L = hklv[2];
        }
      }
    }
  }
  static int CompareHkl(const HklBrushRef &r1, const HklBrushRef &r2)  {
    return r1.CompareTo(r2);
  }
  static int CompareSig(const HklBrushRef &r1, const HklBrushRef &r2)  {
    return r1.CompareTo(r2);
  }
};

void XLibMacros::macHklBrush(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TXApp &XApp = TXApp::GetInstance();
  olxstr HklFN(XApp.XFile().LocateHklFile());

  if( HklFN.IsEmpty() )  {
    E.ProcessingError(__OlxSrcInfo, "could not locate HKL file");
    return;
  }
  THklFile Hkl;
  Hkl.LoadFromFile(HklFN, false);

  TAsymmUnit& au = XApp.XFile().GetAsymmUnit();
  TSpaceGroup &sg = TSymmLib::GetInstance().FindSG(au);

  const bool useFriedelLaw = Options.Contains("f");
  smatd_list ml;
  sg.GetMatrices(ml, mattAll^(mattIdentity|mattCentering));
  if( !sg.IsCentrosymmetric() && useFriedelLaw )
    ml.InsertNew(0).r.I() *= -1;

  TPtrList< HklBrushRef > refs, eqs;
  refs.SetCapacity( Hkl.RefCount() );
  for( size_t i=0; i < Hkl.RefCount(); i++ )  {
    refs.Add(new HklBrushRef(Hkl[i]));
    refs[i]->Standardise(ml, useFriedelLaw);
  }
  QuickSorter::SortSF(refs, HklBrushRef::CompareHkl);

  eqs.Add(refs[0]);  // reference reflection
  for( size_t i=0; i < refs.Count(); )  {
    while( (++i < refs.Count()) && (eqs[0]->CompareTo(*refs[i]) == 0) )
      eqs.Add(refs[i]);
    // do mergings etc
    if( eqs.Count() > 3)  {
      QuickSorter::SortSF(eqs, HklBrushRef::CompareSig);
      size_t ind = eqs.Count()-1;
      while( eqs[ind]->ref->GetS() > 2*eqs[0]->ref->GetS() && --ind >= 2 )  {
        eqs[ind]->Deleted = true;
      }
    }
    if( i >= refs.Count() )  break;
    eqs.Clear();
    eqs.Add(refs[i]);
  }
  // keep the order in which equivalent refs are next to each other
  TRefPList toSave;
  toSave.SetCapacity(refs.Count());
  size_t deletedRefs = 0;
  for( size_t i=0; i < refs.Count(); i++ )  {
    toSave.Add( refs[i]->ref );
    if( refs[i]->Deleted )  {
      refs[i]->ref->SetOmitted(true);
      deletedRefs++;
    }
    delete refs[i];
  }
  XApp.NewLogEntry() << "Ommited " << deletedRefs << " reflections";
  Hkl.SaveToFile("brushed.hkl", toSave, false);
}
