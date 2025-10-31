/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "sgtest.h"
#include "edlist.h"
#include "tptrlist.h"
#include "refmerge.h"
#include "xapp.h"

TSGTest::TSGTest() : minInd(100, 100, 100), maxInd(-100, -100, -100) {
  Hkl3DArray = NULL;
  // 84.47%, w: 86.32 hkl->MergeInP1<RefMerger::StandardMerger>(Refs);
  // 84.47%, w: 86.32 hkl->MergeInP1<RefMerger::ShelxMerger>(Refs);
  // 84.56%, w: 86.29%
  TXApp::GetInstance().XFile().GetRM().GetAllP1RefList<RefMerger::StandardMerger>(Refs);
  const size_t ref_cnt = Refs.Count();
  MinI = 1000;
  MaxI = -1000;
  TotalI = AverageI = AverageIS = TotalIS = 0;
  for( size_t i=0; i < ref_cnt; i++ )  {
    vec3i::UpdateMinMax(Refs[i].GetHkl(), minInd, maxInd);
    if( Refs[i].GetI() > MaxI )  {
      MaxI = Refs[i].GetI();
      MaxIS = Refs[i].GetS();
    }
    if( Refs[i].GetI() < MinI )  {
      MinI = Refs[i].GetI();
      MinIS = Refs[i].GetS();
    }
  }
  Hkl3DArray = new TArray3D<TReflection*>(
    minInd[0], maxInd[0],
    minInd[1], maxInd[1],
    minInd[2], maxInd[2]
  );
  TArray3D<TReflection*>& Hkl3D = *Hkl3DArray;
  for( size_t i=0; i < ref_cnt; i++ )  {
    TReflection& ref = Refs[i];
    Hkl3D(ref.GetHkl()) = &ref;
    TotalI += ref.GetI() < 0 ? 0 : ref.GetI();
    TotalIS += ref.GetS()*ref.GetS();
  }
  AverageI  = TotalI/ref_cnt;
  AverageIS = sqrt(TotalIS/ref_cnt);
}
//..............................................................................
TSGTest::~TSGTest()  {
  if( Hkl3DArray != NULL )
    delete Hkl3DArray;
}
//..............................................................................
void TSGTest::MergeTest(const TPtrList<TSpaceGroup>& sgList, TTypeList<TSGStats>& res) {
  typedef AnAssociation4<TSpaceGroup*, TwoDoublesInt, TwoDoublesInt, int> SGAss;
  typedef TPtrList<SGAss>  SGpList;
  typedef olx_pair_t<mat3i, SGpList> MatrixAss;
  typedef TTypeList<SGAss>  SGList;
  TTypeList< MatrixAss > UniqMatricesNT;
  SGList SGHitsNT(olx_reserve(sgList.Count()));
  smatd_list sgMl;

  const TArray3D<TReflection*>& Hkl3D = *Hkl3DArray;

  for (size_t i = 0; i < sgList.Count(); i++) {
    TSpaceGroup& sg = *sgList[i];
    SGAss& sgvNT =
      SGHitsNT.AddNew<TSpaceGroup*, TwoDoublesInt, TwoDoublesInt, int >(
        &sg, TwoDoublesInt(0.0, 0.0, 0), TwoDoublesInt(0.0, 0.0, 0), 0);
    sgMl.Clear();
    sg.GetMatrices(sgMl, mattAll);
    const size_t sgml_cnt = sgMl.Count();
    for (size_t j = 0; j < sgml_cnt; j++) {
      smatd& m = sgMl[j];
      bool uniq = true;
      const size_t umnt_cnt = UniqMatricesNT.Count();
      for (size_t k = 0; k < umnt_cnt; k++) {
        if (UniqMatricesNT[k].GetA() == m.r) {
          uniq = false;
          UniqMatricesNT[k].b.Add(&sgvNT);
          sgvNT.d++;
          break;
        }
      }
      if (uniq) {
        sgvNT.d++;
        UniqMatricesNT.AddNew<mat3i>(m.r).b.Add(&sgvNT);
      }
    }
  }
  const size_t umnt_cnt = UniqMatricesNT.Count();
  for (size_t i = 0; i < umnt_cnt; i++) {
    const mat3i& m = UniqMatricesNT[i].GetA();
    const size_t ref_cnt = Refs.Count();
    for (size_t j = 0; j < ref_cnt; j++) {
      if (Refs[j].GetI() < AverageI)  continue;
      vec3i hklv = Refs[j] * m;
      if (!vec3i::IsInRangeExc(hklv, minInd, maxInd))  continue;
      if (!Refs[j].EqHkl(hklv)) {
        const TReflection* ref = Hkl3D(hklv);
        if (ref == NULL)
          continue;

        //double weight = olx_abs(ref->Data()[0] - Hkl.Ref(j)->Data()[0]) / (olx_abs(ref->Data()[0]) +olx_abs(Hkl.Ref(j)->Data()[0]));
        double diff = olx_abs(ref->GetI() - Refs[j].GetI());
        //double weight = diff * HklScaleFactor;
        // 5% of an average reflection itensity
        for (size_t k = 0; k < UniqMatricesNT[i].GetB().Count(); k++) {
          UniqMatricesNT[i].GetB()[k]->c.a += diff;
          UniqMatricesNT[i].GetB()[k]->c.b += (olx_sqr(ref->GetS()) +
            olx_sqr(Refs[j].GetS()));
          UniqMatricesNT[i].GetB()[k]->c.c++;
        }
      }
    }
  }
  for (size_t i = 0; i < SGHitsNT.Count(); i++) {
    SGHitsNT[i].c.b = sqrt(SGHitsNT[i].GetC().GetB());
    res.AddNew<TSpaceGroup*, TwoDoublesInt>(
      SGHitsNT[i].GetA(), SGHitsNT[i].GetC());
  }
}
//..............................................................................
void TSGTest::LatticeSATest(TTypeList<TElementStats<TCLattice*> >& latRes, TTypeList<TSAStats>& saRes) {
  typedef AnAssociation3<TCLattice*, TwoDoublesInt*, TwoDoublesInt*> LatticeAss;
  typedef olx_pair_t<TSymmElement*, TwoDoublesInt*> NamedSE;
  typedef olx_pair_t<NamedSE*, TPtrList<NamedSE>*> SECondition;
  TTypeList<SECondition> SAExclusions;
  TTypeList<NamedSE> SAElements;
  TTypeList<LatticeAss> LatticeHits;
  TPtrList<NamedSE> PresentElements;

  const TSymmLib& sgLib = TSymmLib::GetInstance();

  LatticeHits.AddNew<TCLattice*, TwoDoublesInt*, TwoDoublesInt* >(sgLib.FindLattice("P"), new TwoDoublesInt(0, 0, 0), new TwoDoublesInt(0, 0, 0));
  LatticeAss& LattA = LatticeHits.AddNew<TCLattice*, TwoDoublesInt*, TwoDoublesInt*>(
    sgLib.FindLattice("A"), new TwoDoublesInt(0, 0, 0), new TwoDoublesInt(0, 0, 0));
  LatticeAss& LattB = LatticeHits.AddNew<TCLattice*, TwoDoublesInt*, TwoDoublesInt*>(
    sgLib.FindLattice("B"), new TwoDoublesInt(0, 0, 0), new TwoDoublesInt(0, 0, 0));
  LatticeAss& LattC = LatticeHits.AddNew<TCLattice*, TwoDoublesInt*, TwoDoublesInt*>(
    sgLib.FindLattice("C"), new TwoDoublesInt(0, 0, 0), new TwoDoublesInt(0, 0, 0));
  LatticeAss& LattF = LatticeHits.AddNew<TCLattice*, TwoDoublesInt*, TwoDoublesInt*>(
    sgLib.FindLattice("F"), new TwoDoublesInt(0, 0, 0), new TwoDoublesInt(0, 0, 0));
  LatticeAss& LattI = LatticeHits.AddNew<TCLattice*, TwoDoublesInt*, TwoDoublesInt*>(
    sgLib.FindLattice("I"), new TwoDoublesInt(0, 0, 0), new TwoDoublesInt(0, 0, 0));
  LatticeAss& LattR = LatticeHits.AddNew<TCLattice*, TwoDoublesInt*, TwoDoublesInt*>(
    sgLib.FindLattice("R"), new TwoDoublesInt(0, 0, 0), new TwoDoublesInt(0, 0, 0));

  NamedSE& GlideB1 = SAElements.AddNew<TSymmElement*, TwoDoublesInt*>(
    sgLib.FindSymmElement("b--"), new TwoDoublesInt(0.0, 0.0, 0));
  NamedSE& GlideC1 = SAElements.AddNew<TSymmElement*, TwoDoublesInt*>(
    sgLib.FindSymmElement("c--"), new TwoDoublesInt(0.0, 0.0, 0));
  NamedSE& GlideN1 = SAElements.AddNew<TSymmElement*, TwoDoublesInt*>(
    sgLib.FindSymmElement("n--"), new TwoDoublesInt(0.0, 0.0, 0));
  NamedSE& GlideD1 = SAElements.AddNew<TSymmElement*, TwoDoublesInt*>(
    sgLib.FindSymmElement("d--"), new TwoDoublesInt(0.0, 0.0, 0));

  NamedSE& GlideA2 = SAElements.AddNew<TSymmElement*, TwoDoublesInt*>(
    sgLib.FindSymmElement("-a-"), new TwoDoublesInt(0.0, 0.0, 0));
  NamedSE& GlideC2 = SAElements.AddNew<TSymmElement*, TwoDoublesInt*>(
    sgLib.FindSymmElement("-c-"), new TwoDoublesInt(0.0, 0.0, 0));
  NamedSE& GlideN2 = SAElements.AddNew<TSymmElement*, TwoDoublesInt*>
    (sgLib.FindSymmElement("-n-"), new TwoDoublesInt(0.0, 0.0, 0));
  NamedSE& GlideD2 = SAElements.AddNew<TSymmElement*, TwoDoublesInt*>(
    sgLib.FindSymmElement("-d-"), new TwoDoublesInt(0.0, 0.0, 0));

  NamedSE& GlideB3 = SAElements.AddNew<TSymmElement*, TwoDoublesInt*>(
    sgLib.FindSymmElement("--b"), new TwoDoublesInt(0.0, 0.0, 0));
  NamedSE& GlideA3 = SAElements.AddNew<TSymmElement*, TwoDoublesInt*>(
    sgLib.FindSymmElement("--a"), new TwoDoublesInt(0.0, 0.0, 0));
  NamedSE& GlideN3 = SAElements.AddNew<TSymmElement*, TwoDoublesInt*>(
    sgLib.FindSymmElement("--n"), new TwoDoublesInt(0.0, 0.0, 0));
  NamedSE& GlideD3 = SAElements.AddNew<TSymmElement*, TwoDoublesInt*>(
    sgLib.FindSymmElement("--d"), new TwoDoublesInt(0.0, 0.0, 0));

  NamedSE& Screw21a = SAElements.AddNew<TSymmElement*, TwoDoublesInt*>(
    sgLib.FindSymmElement("21--"), new TwoDoublesInt(0.0, 0.0, 0));
  NamedSE& Screw21b = SAElements.AddNew<TSymmElement*, TwoDoublesInt*>(
    sgLib.FindSymmElement("-21-"), new TwoDoublesInt(0.0, 0.0, 0));
  NamedSE& Screw21c = SAElements.AddNew<TSymmElement*, TwoDoublesInt*>(
    sgLib.FindSymmElement("--21"), new TwoDoublesInt(0.0, 0.0, 0));

  NamedSE& Screw41 = SAElements.AddNew<TSymmElement*, TwoDoublesInt*>(
    sgLib.FindSymmElement("41"), new TwoDoublesInt(0.0, 0.0, 0));
  NamedSE& Screw42 = SAElements.AddNew<TSymmElement*, TwoDoublesInt*>(
    sgLib.FindSymmElement("42"), new TwoDoublesInt(0.0, 0.0, 0));
  NamedSE& Screw43 = SAElements.AddNew<TSymmElement*, TwoDoublesInt*>(
    sgLib.FindSymmElement("43"), new TwoDoublesInt(0.0, 0.0, 0));

  NamedSE& Screw33 = SAElements.AddNew<TSymmElement*, TwoDoublesInt*>(
    sgLib.FindSymmElement("31"), new TwoDoublesInt(0.0, 0.0, 0));
  NamedSE& Screw61 = SAElements.AddNew<TSymmElement*, TwoDoublesInt*>(
    sgLib.FindSymmElement("61"), new TwoDoublesInt(0.0, 0.0, 0));

  //TPtrList<NamedSE> Glide1ExcG;
  //Glide1ExcG.Add(GlideB1);
  //Glide1ExcG.Add(GlideC1);
  //TPtrList<NamedSE> Glide1ExcGD(Glide1ExcG);
  //Glide1ExcGD.Add(GlideN1);
  TPtrList<NamedSE> Glide1ExcS;
  Glide1ExcS.Add(Screw21b);
  Glide1ExcS.Add(Screw21c);
  Glide1ExcS.Add(Screw42);
  Glide1ExcS.Add(Screw43);
  SAExclusions.AddNew<NamedSE*, TPtrList<NamedSE>*>(&GlideB1, &Glide1ExcS);
  SAExclusions.AddNew<NamedSE*, TPtrList<NamedSE>*>(&GlideC1, &Glide1ExcS);
  SAExclusions.AddNew<NamedSE*, TPtrList<NamedSE>*>(&GlideN1, &Glide1ExcS);
  //  SAExclusions.AddNew<NamedSE*, TPtrList<NamedSE>*>(&GlideN1, &Glide1ExcG);
  SAExclusions.AddNew<NamedSE*, TPtrList<NamedSE>*>(&GlideD1, &Glide1ExcS);
  //  SAExclusions.AddNew<NamedSE*, TPtrList<NamedSE>*>(&GlideD1, &Glide1ExcGD);
  TPtrList<NamedSE> Glide2ExcG;
  Glide2ExcG.Add(GlideA2);
  Glide2ExcG.Add(GlideC2);
  TPtrList<NamedSE> Glide2ExcGD(Glide2ExcG);
  Glide2ExcGD.Add(GlideN2);
  TPtrList<NamedSE> Glide2ExcS;
  Glide2ExcS.Add(Screw21a);
  Glide2ExcS.Add(Screw21c);
  Glide2ExcS.Add(Screw41);
  Glide2ExcS.Add(Screw43);
  SAExclusions.AddNew<NamedSE*, TPtrList<NamedSE>*>(&GlideA2, &Glide2ExcS);
  SAExclusions.AddNew<NamedSE*, TPtrList<NamedSE>*>(&GlideC2, &Glide2ExcS);
  SAExclusions.AddNew<NamedSE*, TPtrList<NamedSE>*>(&GlideN2, &Glide2ExcS);
  //  SAExclusions.AddNew<NamedSE*, TPtrList<NamedSE>*>(&GlideN2, &Glide2ExcG);
  SAExclusions.AddNew<NamedSE*, TPtrList<NamedSE>*>(&GlideD2, &Glide2ExcS);
  //  SAExclusions.AddNew<NamedSE*, TPtrList<NamedSE>*>(&GlideD2, &Glide2ExcGD);
  TPtrList<NamedSE> Glide3ExcG;
  Glide3ExcG.Add(GlideA3);
  Glide3ExcG.Add(GlideB3);
  TPtrList<NamedSE> Glide3ExcGD(Glide3ExcG);
  Glide3ExcGD.Add(GlideN3);
  TPtrList<NamedSE> Glide3ExcS;
  Glide3ExcS.Add(Screw21a);
  Glide3ExcS.Add(Screw21b);
  Glide3ExcS.Add(Screw41);
  Glide3ExcS.Add(Screw42);
  SAExclusions.AddNew<NamedSE*, TPtrList<NamedSE>*>(&GlideB3, &Glide3ExcS);
  SAExclusions.AddNew<NamedSE*, TPtrList<NamedSE>*>(&GlideA3, &Glide3ExcS);
  SAExclusions.AddNew<NamedSE*, TPtrList<NamedSE>*>(&GlideN3, &Glide3ExcS);
  //  SAExclusions.AddNew<NamedSE*, TPtrList<NamedSE>*>(&GlideN3, &Glide3ExcG);
  SAExclusions.AddNew<NamedSE*, TPtrList<NamedSE>*>(&GlideD3, &Glide3ExcS);
  //  SAExclusions.AddNew<NamedSE*, TPtrList<NamedSE>*>(&GlideD3, &Glide3ExcGD);
  const size_t ref_cnt = Refs.Count();
  for (size_t i = 0; i < ref_cnt; i++) {
    const TReflection& ref = Refs[i];
    int H = ref.GetH(),
      K = ref.GetK(),
      L = ref.GetL();

    if (((H + K) % 2) != 0) {
      LattC.GetC()->a += ref.GetI();
      LattC.GetC()->b += olx_sqr(ref.GetS());
      LattC.GetC()->c++;
    }
    else {
      LattC.GetB()->a += ref.GetI();
      LattC.GetB()->b += olx_sqr(ref.GetS());
      LattC.GetB()->c++;
    }
    if (((H + L) % 2) != 0) {
      LattB.GetC()->a += ref.GetI();
      LattB.GetC()->b += olx_sqr(ref.GetS());
      LattB.GetC()->c++;
    }
    else {
      LattB.GetB()->a += ref.GetI();
      LattB.GetB()->b += olx_sqr(ref.GetS());
      LattB.GetB()->c++;
    }
    if (((K + L) % 2) != 0) {
      LattA.GetC()->a += ref.GetI();
      LattA.GetC()->b += olx_sqr(ref.GetS());
      LattA.GetC()->c++;
    }
    else {
      LattA.GetB()->a += ref.GetI();
      LattA.GetB()->b += olx_sqr(ref.GetS());
      LattA.GetB()->c++;
    }
    if (((H + K + L) % 2) != 0) {
      LattI.GetC()->a += ref.GetI();
      LattI.GetC()->b += olx_sqr(ref.GetS());
      LattI.GetC()->c++;
    }
    else {
      LattI.GetB()->a += ref.GetI();
      LattI.GetB()->b += olx_sqr(ref.GetS());
      LattI.GetB()->c++;
    }
    if ((((H + K) % 2) != 0 && ((H + L) % 2) != 0 && ((K + L) % 2) != 0) ||
      ((H % 2) == 0 && (K % 2) == 0 && (L % 2) == 0) ||
      ((H % 2) != 0 && (K % 2) != 0 && (L % 2) != 0))
    {
      LattF.GetB()->a += ref.GetI();
      LattF.GetB()->b += olx_sqr(ref.GetS());
      LattF.GetB()->c++;
    }
    else {
      LattF.GetC()->a += ref.GetI();
      LattF.GetC()->b += olx_sqr(ref.GetS());
      LattF.GetC()->c++;
    }
    if (((-H + K + L) % 3) != 0) {
      LattR.GetC()->a += ref.GetI();
      LattR.GetC()->b += olx_sqr(ref.GetS());
      LattR.GetC()->c++;
    }
    else {
      LattR.GetB()->a += ref.GetI();
      LattR.GetB()->b += olx_sqr(ref.GetS());
      LattR.GetB()->c++;
    }

    if (H == 0) {
      if ((K % 2) != 0) {
        GlideB1.GetB()->a += ref.GetI();
        GlideB1.GetB()->b += olx_sqr(ref.GetS());
        GlideB1.GetB()->c++;
      }
      if ((L % 2) != 0) {
        GlideC1.GetB()->a += ref.GetI();
        GlideC1.GetB()->b += olx_sqr(ref.GetS());
        GlideC1.GetB()->c++;
      }
      if (((K + L) % 2) != 0) {
        GlideN1.GetB()->a += ref.GetI();
        GlideN1.GetB()->b += olx_sqr(ref.GetS());
        GlideN1.GetB()->c++;
      }
      if (((K + L) % 4) != 0) {
        GlideD1.GetB()->a += ref.GetI();
        GlideD1.GetB()->b += olx_sqr(ref.GetS());
        GlideD1.GetB()->c++;
      }
    }
    if (K == 0) {
      if ((H % 2) != 0) {
        GlideA2.GetB()->a += ref.GetI();
        GlideA2.GetB()->b += olx_sqr(ref.GetS());
        GlideA2.GetB()->c++;
      }
      if ((L % 2) != 0) {
        GlideC2.GetB()->a += ref.GetI();
        GlideC2.GetB()->b += olx_sqr(ref.GetS());
        GlideC2.GetB()->c++;
      }
      if (((H + L) % 2) != 0) {
        GlideN2.GetB()->a += ref.GetI();
        GlideN2.GetB()->b += olx_sqr(ref.GetS());
        GlideN2.GetB()->c++;
      }
      if (((H + L) % 4) != 0) {
        GlideD2.GetB()->a += ref.GetI();
        GlideD2.GetB()->b += olx_sqr(ref.GetS());
        GlideD2.GetB()->c++;
      }
    }
    if (L == 0) {
      if ((K % 2) != 0) {
        GlideB3.GetB()->a += ref.GetI();
        GlideB3.GetB()->b += olx_sqr(ref.GetS());
        GlideB3.GetB()->c++;
      }
      if ((H % 2) != 0) {
        GlideA3.GetB()->a += ref.GetI();
        GlideA3.GetB()->b += olx_sqr(ref.GetS());
        GlideA3.GetB()->c++;
      }
      if (((H + K) % 2) != 0) {
        GlideN3.GetB()->a += ref.GetI();
        GlideN3.GetB()->b += olx_sqr(ref.GetS());
        GlideN3.GetB()->c++;
      }
      if (((H + K) % 4) != 0) {
        GlideD3.GetB()->a += ref.GetI();
        GlideD3.GetB()->b += olx_sqr(ref.GetS());
        GlideD3.GetB()->c++;
      }
    }
    if (K == 0 && L == 0) {
      if ((H % 2) != 0) {
        Screw21a.GetB()->a += ref.GetI();
        Screw21a.GetB()->b += olx_sqr(ref.GetS());
        Screw21a.GetB()->c++;
      }
    }
    if (H == 0 && L == 0) {
      if ((K % 2) != 0) {
        Screw21b.GetB()->a += ref.GetI();
        Screw21b.GetB()->b += olx_sqr(ref.GetS());
        Screw21b.GetB()->c++;
      }
    }
    if (H == 0 && K == 0) {
      if ((L % 2) != 0) {
        Screw21c.GetB()->a += ref.GetI();
        Screw21c.GetB()->b += olx_sqr(ref.GetS());
        Screw21c.GetB()->c++;

        Screw42.GetB()->a += ref.GetI();
        Screw42.GetB()->b += olx_sqr(ref.GetS());
        Screw42.GetB()->c++;
      }
      if ((L % 4) != 0) {
        Screw41.GetB()->a += ref.GetI();
        Screw41.GetB()->b += olx_sqr(ref.GetS());
        Screw41.GetB()->c++;
        Screw43.GetB()->a += ref.GetI();
        Screw43.GetB()->b += olx_sqr(ref.GetS());
        Screw43.GetB()->c++;
      }

      if ((L % 3) != 0) {
        Screw33.GetB()->a += ref.GetI();
        Screw33.GetB()->b += olx_sqr(ref.GetS());
        Screw33.GetB()->c++;
      }
      if ((L % 6) != 0) {
        Screw61.GetB()->a += ref.GetI();
        Screw61.GetB()->b += olx_sqr(ref.GetS());
        Screw61.GetB()->c++;
      }
    }
  }
  //double averageSAI = 0;
  //size_t saCount = 0;
  //for( size_t i=0; i < SAElements.Count(); i++ )  {
  //  SAElements[i].GetB()->b = sqrt( SAElements[i].GetB()->GetB() );
  //  if( SAElements[i].GetB()->GetC() != 0 )  {
  //    averageSAI += SAElements[i].GetB()->GetA()/SAElements[i].GetB()->GetC();
  //    saCount++;
  //  }
  //}
  //if( saCount )  averageSAI /= saCount;
  //for( size_t i=0; i < SAElements.Count(); i++ )  {
  //  SAElements[i].GetB()->a /= 100;
  //  SAElements[i].GetB()->b = sqrt( SAElements[i].GetB()->GetB() )/100;
  //}
  for (size_t i = 0; i < SAElements.Count(); i++) {
    SAElements[i].GetB()->b = sqrt(SAElements[i].GetB()->GetB());
    if (SAElements[i].GetB()->GetC() != 0) {
      if (SAElements[i].GetB()->GetA() > 0) {
        if (SAElements[i].GetB()->GetA() / SAElements[i].GetB()->GetC() < AverageI / 7.5 ||
          SAElements[i].GetB()->GetA() < 4 * SAElements[i].GetB()->GetB())
        {
          PresentElements.Add(&SAElements[i]);
        }
      }
      else {
        PresentElements.Add(&SAElements[i]);
      }
    }
  }
  // second filtering ...
  if (PresentElements.Count() > 1) {
    double averageSAI = 0;
    double Ssq = 0;
    for (size_t i = 0; i < PresentElements.Count(); i++) {
      double v = PresentElements[i]->GetB()->GetA() / PresentElements[i]->GetB()->GetC();
      if (v < 0) {
        v = 0;
      }
      averageSAI += v;
    }
    averageSAI /= PresentElements.Count();
    for (size_t i = 0; i < PresentElements.Count(); i++) {
      double v = PresentElements[i]->GetB()->GetA() / PresentElements[i]->GetB()->GetC();
      Ssq += olx_abs(v - averageSAI);
    }
    Ssq /= PresentElements.Count();
    Ssq = sqrt(Ssq);
    double maxInt = 1.0*(averageSAI + 2 * Ssq);
    for (size_t i = 0; i < PresentElements.Count(); i++) {
      double v = PresentElements[i]->GetB()->GetA() / PresentElements[i]->GetB()->GetC();
      if (v > maxInt && v > 1 &&
        PresentElements[i]->GetB()->GetA() > 4 * PresentElements[i]->GetB()->GetB())
      {
        PresentElements.Delete(i);
        i--;
      }
    }
  } // check if the absolute scale of is not rediculus...
  else {
    if (PresentElements.Count() != 0) {
      double v = PresentElements[0]->GetB()->GetA() / PresentElements[0]->GetB()->GetC();
      if (v > 10) {
        PresentElements.Clear();
      }
    }
  }

  for (size_t i = 0; i < SAElements.Count(); i++) {
    TSAStats& sas = saRes.AddNew<TSymmElement*, TwoDoublesInt>(
      SAElements[i].GetA(), *SAElements[i].GetB());
    if (SAElements[i].GetB()->GetC() != 0) {
      if (PresentElements.Contains(&SAElements[i])) {
        sas.SetPresent();
        bool excluded = false;
        for (size_t j = 0; j < SAExclusions.Count(); j++) {
          if (PresentElements.Contains(SAExclusions[j].GetA())) {
            if (SAExclusions[j].GetB()->Contains(&SAElements[i])) {
              // mark removed elements
              excluded = true;
              size_t index = PresentElements.IndexOf(&SAElements[i]);
              PresentElements.Delete(index);
              break;
            }
          }
        }
        if (excluded) {
          sas.Exclude();
        }
      }
    }
  }
  for (size_t i = 0; i < LatticeHits.Count(); i++) {
    LatticeHits[i].GetB()->b = sqrt(LatticeHits[i].GetB()->GetB());
    LatticeHits[i].GetC()->b = sqrt(LatticeHits[i].GetC()->GetB());
    latRes.AddNew<TCLattice*, TwoDoublesInt, TwoDoublesInt>(LatticeHits[i].GetA(),
      *LatticeHits[i].GetB(),
      *LatticeHits[i].GetC());
    delete LatticeHits[i].GetB();
    delete LatticeHits[i].GetC();
  }
  for (size_t i = 0; i < SAElements.Count(); i++) {
    delete SAElements[i].GetB();
  }
}
//..............................................................................
void TSGTest::WeakRefTest(const TPtrList<TSpaceGroup>& sgList,
  TTypeList<TElementStats<TSpaceGroup*> >& res)
{
  typedef AnAssociation4<TSpaceGroup*, TwoDoublesInt, TwoDoublesInt, int> SGAss;
  typedef TPtrList<SGAss>  SGpList;
  typedef olx_pair_t<smatd, SGpList> MatrixAss;
  typedef TTypeList<SGAss>  SGList;
  TTypeList< MatrixAss > UniqMatrices;
  SGList SGHits;
  smatd_list sgMl;
  SGHits.SetCapacity(sgList.Count());
  for (size_t i = 0; i < sgList.Count(); i++) {
    TSpaceGroup& sg = *sgList[i];
    SGAss& sgv =
      SGHits.AddNew<TSpaceGroup*, TwoDoublesInt, TwoDoublesInt, int >(
        &sg, TwoDoublesInt(0.0, 0.0, 0), TwoDoublesInt(0.0, 0.0, 0), 0);
    sg.GetMatrices(sgMl, mattAll);
    const size_t ml_cnt = sgMl.Count();
    for (size_t j = 0; j < ml_cnt; j++) {
      smatd& m = sgMl[j];
      // skip -I matrix
      if (m.r[0][0] == -1 && m.r[1][1] == -1 && m.r[2][2] == -1 &&
        m.r[0][1] == 0 && m.r[0][2] == 0 &&
        m.r[1][0] == 0 && m.r[1][2] == 0 &&
        m.r[2][0] == 0 && m.r[2][1] == 0)
        continue;
      int iv = (int)m.t[0];  m.t[0] = olx_abs(m.t[0] - iv);
      iv = (int)m.t[1];  m.t[1] = olx_abs(m.t[1] - iv);
      iv = (int)m.t[2];  m.t[2] = olx_abs(m.t[2] - iv);
      // matrix should have a translation
      if (m.t.IsNull()) {
        continue;
      }

      if (m.t[0] > 0.5)  m.t[0] = 1 - m.t[0];
      if (m.t[1] > 0.5)  m.t[1] = 1 - m.t[1];
      if (m.t[2] > 0.5)  m.t[2] = 1 - m.t[2];

      bool uniq = true;
      for (size_t k = 0; k < UniqMatrices.Count(); k++) {
        if (UniqMatrices[k].GetA() == m) {
          uniq = false;
          UniqMatrices[k].b.Add(&sgv);
          sgv.d++;
          break;
        }
      }
      if (uniq) {
        sgv.d++;
        UniqMatrices.AddNew<smatd>(m).b.Add(&sgv);
      }
    }
    sgMl.Clear();
  }
  // collect statistics for the matrices and the spacegroups
  const size_t um_cnt = UniqMatrices.Count();
  for (size_t i = 0; i < um_cnt; i++) {
    const smatd& m = UniqMatrices[i].GetA();
    const size_t ref_cnt = Refs.Count();
    for (size_t j = 0; j < ref_cnt; j++) {
      if (Refs[j].IsSymmetric(m)) {
        double len = Refs[j].PhaseShift(m);
        if (olx_abs(len - olx_round(len)) < 0.01) {
          for (size_t k = 0; k < UniqMatrices[i].GetB().Count(); k++) {
            UniqMatrices[i].GetB()[k]->b.a += Refs[j].GetI();
            UniqMatrices[i].GetB()[k]->b.b += Refs[j].GetS();
            UniqMatrices[i].GetB()[k]->b.c++;
          }
        }
        else {
          for (size_t k = 0; k < UniqMatrices[i].GetB().Count(); k++) {
            UniqMatrices[i].GetB()[k]->c.a += Refs[j].GetI();
            UniqMatrices[i].GetB()[k]->c.b += Refs[j].GetS();
            UniqMatrices[i].GetB()[k]->c.c++;
          }
        }
      }
    }
  }
  for (size_t i = 0; i < SGHits.Count(); i++) {
    res.AddNew<TSpaceGroup*, TwoDoublesInt, TwoDoublesInt>(SGHits[i].GetA(),
      SGHits[i].GetB(),
      SGHits[i].GetC());
  }
}
