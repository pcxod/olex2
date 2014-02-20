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
  TXApp::GetInstance().XFile().GetRM().GetAllP1RefList<RefMerger::UnitMerger>(Refs);
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
void TSGTest::MergeTest(const TPtrList<TSpaceGroup>& sgList,  TTypeList<TSGStats>& res )  {
  typedef AnAssociation4<TSpaceGroup*, TwoDoublesInt, TwoDoublesInt, int> SGAss;
  typedef TPtrList<SGAss>  SGpList;
  typedef AnAssociation2<mat3i, SGpList> MatrixAss;
  typedef TTypeList<SGAss>  SGList;
  TTypeList< MatrixAss > UniqMatricesNT;
  SGList SGHitsNT;
  smatd_list sgMl;

  const TArray3D<TReflection*>& Hkl3D = *Hkl3DArray;

  SGHitsNT.SetCapacity( sgList.Count() );

  for( size_t i=0; i < sgList.Count(); i++ )  {
    TSpaceGroup& sg = *sgList[i];
    SGAss& sgvNT =
      SGHitsNT.AddNew<TSpaceGroup*, TwoDoublesInt, TwoDoublesInt, int > (
                  &sg, TwoDoublesInt(0.0,0.0,0), TwoDoublesInt(0.0,0.0,0), 0);
    sgMl.Clear();
    sg.GetMatrices( sgMl, mattAll );
    const size_t sgml_cnt = sgMl.Count();
    for( size_t j=0; j < sgml_cnt; j++ )  {
      smatd& m = sgMl[j];
      bool uniq = true;
      const size_t umnt_cnt = UniqMatricesNT.Count();
      for( size_t k=0; k < umnt_cnt; k++ )  {
        if( UniqMatricesNT[k].GetA() == m.r )  {
          uniq = false;
          UniqMatricesNT[k].B().Add( &sgvNT );
          sgvNT.D() ++;
          break;
        }
      }
      if( uniq )  {
        sgvNT.D() ++;
        UniqMatricesNT.AddNew<mat3i>(m.r).B().Add(&sgvNT);
      }
    }
  }
  const size_t umnt_cnt = UniqMatricesNT.Count();
  for( size_t i=0; i < umnt_cnt; i++ )  {
    const mat3i& m = UniqMatricesNT[i].GetA();
    const size_t ref_cnt = Refs.Count();
    for( size_t j=0; j < ref_cnt; j++ )  {
      if( Refs[j].GetI() < AverageI )  continue;
      vec3i hklv = Refs[j] * m;
      if( !vec3i::IsInRangeExc(hklv, minInd, maxInd) )  continue;
      if( !Refs[j].EqHkl(hklv) )  {
        const TReflection* ref = Hkl3D(hklv);
        if( ref == NULL )  
          continue;

        //double weight = olx_abs(ref->Data()[0] - Hkl.Ref(j)->Data()[0]) / (olx_abs(ref->Data()[0]) +olx_abs(Hkl.Ref(j)->Data()[0]));
        double diff = olx_abs(ref->GetI() - Refs[j].GetI());
        //double weight = diff * HklScaleFactor;
        // 5% of an average reflection itensity
        for( size_t k=0; k < UniqMatricesNT[i].GetB().Count(); k++ )  {
          UniqMatricesNT[i].GetB()[k]->C().A() += diff;
          UniqMatricesNT[i].GetB()[k]->C().B() += (olx_sqr(ref->GetS()) + olx_sqr(Refs[j].GetS()));
          UniqMatricesNT[i].GetB()[k]->C().C() ++;
        }
      }
    }
  }
  for( size_t i=0; i < SGHitsNT.Count(); i++ )  {
    SGHitsNT[i].C().B() = sqrt( SGHitsNT[i].GetC().GetB() );
    res.AddNew<TSpaceGroup*, TwoDoublesInt>( SGHitsNT[i].GetA(), SGHitsNT[i].GetC());
  }
}
//..............................................................................
void TSGTest::LatticeSATest(TTypeList<TElementStats<TCLattice*> >& latRes, TTypeList<TSAStats>& saRes )  {
  typedef AnAssociation3<TCLattice*, TwoDoublesInt*, TwoDoublesInt*> LatticeAss;
  typedef AnAssociation2<TSymmElement*, TwoDoublesInt*> NamedSE;
  typedef AnAssociation2<NamedSE*, TPtrList<NamedSE>*> SECondition;
  TTypeList<SECondition> SAExclusions;
  TTypeList<NamedSE> SAElements;
  TTypeList<LatticeAss> LatticeHits;
  TPtrList<NamedSE> PresentElements;

  const TSymmLib& sgLib = TSymmLib::GetInstance();

  LatticeHits.AddNew<TCLattice*,TwoDoublesInt*,TwoDoublesInt* >(sgLib.FindLattice("P"), new TwoDoublesInt(0,0,0), new TwoDoublesInt(0,0,0) );
  LatticeAss& LattA = LatticeHits.AddNew<TCLattice*,TwoDoublesInt*,TwoDoublesInt*>(sgLib.FindLattice("A"), new TwoDoublesInt(0,0,0), new TwoDoublesInt(0,0,0) );
  LatticeAss& LattB = LatticeHits.AddNew<TCLattice*,TwoDoublesInt*,TwoDoublesInt*>(sgLib.FindLattice("B"), new TwoDoublesInt(0,0,0), new TwoDoublesInt(0,0,0) );
  LatticeAss& LattC = LatticeHits.AddNew<TCLattice*,TwoDoublesInt*,TwoDoublesInt*>(sgLib.FindLattice("C"), new TwoDoublesInt(0,0,0), new TwoDoublesInt(0,0,0) );
  LatticeAss& LattF = LatticeHits.AddNew<TCLattice*,TwoDoublesInt*,TwoDoublesInt*>(sgLib.FindLattice("F"), new TwoDoublesInt(0,0,0), new TwoDoublesInt(0,0,0) );
  LatticeAss& LattI = LatticeHits.AddNew<TCLattice*,TwoDoublesInt*,TwoDoublesInt*>(sgLib.FindLattice("I"), new TwoDoublesInt(0,0,0), new TwoDoublesInt(0,0,0) );
  LatticeAss& LattR = LatticeHits.AddNew<TCLattice*,TwoDoublesInt*,TwoDoublesInt*>(sgLib.FindLattice("R"), new TwoDoublesInt(0,0,0), new TwoDoublesInt(0,0,0) );

  NamedSE& GlideB1 = SAElements.AddNew<TSymmElement*, TwoDoublesInt*>(sgLib.FindSymmElement("b--"), new TwoDoublesInt(0.0,0.0,0));
  NamedSE& GlideC1 = SAElements.AddNew<TSymmElement*, TwoDoublesInt*>(sgLib.FindSymmElement("c--"), new TwoDoublesInt(0.0,0.0,0));
  NamedSE& GlideN1 = SAElements.AddNew<TSymmElement*, TwoDoublesInt*>(sgLib.FindSymmElement("n--"), new TwoDoublesInt(0.0,0.0,0));
  NamedSE& GlideD1 = SAElements.AddNew<TSymmElement*, TwoDoublesInt*>(sgLib.FindSymmElement("d--"), new TwoDoublesInt(0.0,0.0,0));

  NamedSE& GlideA2 = SAElements.AddNew<TSymmElement*, TwoDoublesInt*>(sgLib.FindSymmElement("-a-"), new TwoDoublesInt(0.0,0.0,0));
  NamedSE& GlideC2 = SAElements.AddNew<TSymmElement*, TwoDoublesInt*>(sgLib.FindSymmElement("-c-"), new TwoDoublesInt(0.0,0.0,0));
  NamedSE& GlideN2 = SAElements.AddNew<TSymmElement*, TwoDoublesInt*>(sgLib.FindSymmElement("-n-"), new TwoDoublesInt(0.0,0.0,0));
  NamedSE& GlideD2 = SAElements.AddNew<TSymmElement*, TwoDoublesInt*>(sgLib.FindSymmElement("-d-"), new TwoDoublesInt(0.0,0.0,0));

  NamedSE& GlideB3 = SAElements.AddNew<TSymmElement*, TwoDoublesInt*>(sgLib.FindSymmElement("--b"), new TwoDoublesInt(0.0,0.0,0));
  NamedSE& GlideA3 = SAElements.AddNew<TSymmElement*, TwoDoublesInt*>(sgLib.FindSymmElement("--a"), new TwoDoublesInt(0.0,0.0,0));
  NamedSE& GlideN3 = SAElements.AddNew<TSymmElement*, TwoDoublesInt*>(sgLib.FindSymmElement("--n"), new TwoDoublesInt(0.0,0.0,0));
  NamedSE& GlideD3 = SAElements.AddNew<TSymmElement*, TwoDoublesInt*>(sgLib.FindSymmElement("--d"), new TwoDoublesInt(0.0,0.0,0));

  NamedSE& Screw21 = SAElements.AddNew<TSymmElement*, TwoDoublesInt*>(sgLib.FindSymmElement("21--"), new TwoDoublesInt(0.0,0.0,0));
  NamedSE& Screw22 = SAElements.AddNew<TSymmElement*, TwoDoublesInt*>(sgLib.FindSymmElement("-21-"), new TwoDoublesInt(0.0,0.0,0));
  NamedSE& Screw23 = SAElements.AddNew<TSymmElement*, TwoDoublesInt*>(sgLib.FindSymmElement("--21"), new TwoDoublesInt(0.0,0.0,0) );

  NamedSE& Screw41 = SAElements.AddNew<TSymmElement*, TwoDoublesInt*>(sgLib.FindSymmElement("41"), new TwoDoublesInt(0.0,0.0,0));
  NamedSE& Screw42 = SAElements.AddNew<TSymmElement*, TwoDoublesInt*>(sgLib.FindSymmElement("42"), new TwoDoublesInt(0.0,0.0,0));
  NamedSE& Screw43 = SAElements.AddNew<TSymmElement*, TwoDoublesInt*>(sgLib.FindSymmElement("43"), new TwoDoublesInt(0.0,0.0,0));

  NamedSE& Screw33 = SAElements.AddNew<TSymmElement*, TwoDoublesInt*>(sgLib.FindSymmElement("31"), new TwoDoublesInt(0.0,0.0,0));
  NamedSE& Screw63 = SAElements.AddNew<TSymmElement*, TwoDoublesInt*>(sgLib.FindSymmElement("61"), new TwoDoublesInt(0.0,0.0,0));

  TPtrList<NamedSE> Glide1ExcG;
  Glide1ExcG.Add(GlideB1);
  Glide1ExcG.Add(GlideC1);
  TPtrList<NamedSE> Glide1ExcGD( Glide1ExcG );
  Glide1ExcGD.Add(GlideN1);
  TPtrList<NamedSE> Glide1ExcS;
  Glide1ExcS.Add(Screw22);
  Glide1ExcS.Add(Screw23);
  Glide1ExcS.Add(Screw42);
  Glide1ExcS.Add(Screw43);
  SAExclusions.AddNew<NamedSE*, TPtrList<NamedSE>*>(&GlideB1, &Glide1ExcS );
  SAExclusions.AddNew<NamedSE*, TPtrList<NamedSE>*>(&GlideC1, &Glide1ExcS );
  SAExclusions.AddNew<NamedSE*, TPtrList<NamedSE>*>(&GlideN1, &Glide1ExcS );
//  SAExclusions.AddNew<NamedSE*, TPtrList<NamedSE>*>(&GlideN1, &Glide1ExcG );
  SAExclusions.AddNew<NamedSE*, TPtrList<NamedSE>*>(&GlideD1, &Glide1ExcS );
//  SAExclusions.AddNew<NamedSE*, TPtrList<NamedSE>*>(&GlideD1, &Glide1ExcGD );
  TPtrList<NamedSE> Glide2ExcG;
  Glide2ExcG.Add(GlideA2);
  Glide2ExcG.Add(GlideC2);
  TPtrList<NamedSE> Glide2ExcGD( Glide2ExcG );
  Glide2ExcGD.Add(GlideN2);
  TPtrList<NamedSE> Glide2ExcS;
  Glide2ExcS.Add(Screw21);
  Glide2ExcS.Add(Screw23);
  Glide2ExcS.Add(Screw41);
  Glide2ExcS.Add(Screw43);
  SAExclusions.AddNew<NamedSE*, TPtrList<NamedSE>*>(&GlideA2, &Glide2ExcS );
  SAExclusions.AddNew<NamedSE*, TPtrList<NamedSE>*>(&GlideC2, &Glide2ExcS );
  SAExclusions.AddNew<NamedSE*, TPtrList<NamedSE>*>(&GlideN2, &Glide2ExcS );
//  SAExclusions.AddNew<NamedSE*, TPtrList<NamedSE>*>(&GlideN2, &Glide2ExcG );
  SAExclusions.AddNew<NamedSE*, TPtrList<NamedSE>*>(&GlideD2, &Glide2ExcS );
//  SAExclusions.AddNew<NamedSE*, TPtrList<NamedSE>*>(&GlideD2, &Glide2ExcGD );
  TPtrList<NamedSE> Glide3ExcG;
  Glide3ExcG.Add(GlideA3);
  Glide3ExcG.Add(GlideB3);
  TPtrList<NamedSE> Glide3ExcGD( Glide3ExcG );
  Glide3ExcGD.Add(GlideN3);
  TPtrList<NamedSE> Glide3ExcS;
  Glide3ExcS.Add(Screw21);
  Glide3ExcS.Add(Screw22);
  Glide3ExcS.Add(Screw41);
  Glide3ExcS.Add(Screw42);
  SAExclusions.AddNew<NamedSE*, TPtrList<NamedSE>*>(&GlideB3, &Glide3ExcS );
  SAExclusions.AddNew<NamedSE*, TPtrList<NamedSE>*>(&GlideA3, &Glide3ExcS );
  SAExclusions.AddNew<NamedSE*, TPtrList<NamedSE>*>(&GlideN3, &Glide3ExcS );
//  SAExclusions.AddNew<NamedSE*, TPtrList<NamedSE>*>(&GlideN3, &Glide3ExcG );
  SAExclusions.AddNew<NamedSE*, TPtrList<NamedSE>*>(&GlideD3, &Glide3ExcS );
//  SAExclusions.AddNew<NamedSE*, TPtrList<NamedSE>*>(&GlideD3, &Glide3ExcGD );
  const size_t ref_cnt = Refs.Count();
  for( size_t i=0; i < ref_cnt; i++ )  {
    const TReflection& ref = Refs[i];
    int H = ref.GetH(), 
        K = ref.GetK(), 
        L = ref.GetL();

    if( ((H+K)%2) != 0 )  {  LattC.GetC()->A() += ref.GetI();  LattC.GetC()->B() += olx_sqr(ref.GetS());  LattC.GetC()->C()++;  }
    else                  {  LattC.GetB()->A() += ref.GetI();  LattC.GetB()->B() += olx_sqr(ref.GetS());  LattC.GetB()->C()++;  }
    if( ((H+L)%2) != 0 )  {  LattB.GetC()->A() += ref.GetI();  LattB.GetC()->B() += olx_sqr(ref.GetS());  LattB.GetC()->C()++;  }
    else                  {  LattB.GetB()->A() += ref.GetI();  LattB.GetB()->B() += olx_sqr(ref.GetS());  LattB.GetB()->C()++;  }
    if( ((K+L)%2) != 0 )  {  LattA.GetC()->A() += ref.GetI();  LattA.GetC()->B() += olx_sqr(ref.GetS());  LattA.GetC()->C()++;  }
    else                  {  LattA.GetB()->A() += ref.GetI();  LattA.GetB()->B() += olx_sqr(ref.GetS());  LattA.GetB()->C()++;  }
    if( ((H+K+L)%2) != 0 ){  LattI.GetC()->A() += ref.GetI();  LattI.GetC()->B() += olx_sqr(ref.GetS());  LattI.GetC()->C()++;  }
    else                  {  LattI.GetB()->A() += ref.GetI();  LattI.GetB()->B() += olx_sqr(ref.GetS());  LattI.GetB()->C()++;  }
    if( (((H+K)%2) != 0 && ((H+L)%2) !=0 && ((K+L)%2) != 0 )  ||
        ( (H%2)==0 && (K%2) == 0 && (L%2) == 0 )  ||
        ( (H%2)!=0 && (K%2) != 0 && (L%2) != 0 )  )
                          {  LattF.GetB()->A() += ref.GetI();  LattF.GetB()->B() += olx_sqr(ref.GetS());  LattF.GetB()->C()++;  }
    else                   {  LattF.GetC()->A() += ref.GetI();  LattF.GetC()->B() += olx_sqr(ref.GetS());  LattF.GetC()->C()++;  }
    if( ((-H+K+L)%3) != 0 ){  LattR.GetC()->A() += ref.GetI();  LattR.GetC()->B() += olx_sqr(ref.GetS());  LattR.GetC()->C()++;  }
    else                   {  LattR.GetB()->A() += ref.GetI();  LattR.GetB()->B() += olx_sqr(ref.GetS());  LattR.GetB()->C()++;  }

    if( H == 0 )  {
      if( (K%2) != 0 )    {  GlideB1.GetB()->A() += ref.GetI();  GlideB1.GetB()->B() += olx_sqr(ref.GetS());  GlideB1.GetB()->C()++;  }
      if( (L%2) != 0 )    {  GlideC1.GetB()->A() += ref.GetI();  GlideC1.GetB()->B() += olx_sqr(ref.GetS());  GlideC1.GetB()->C()++;  }
      if( ((K+L)%2) != 0 ){  GlideN1.GetB()->A() += ref.GetI();  GlideN1.GetB()->B() += olx_sqr(ref.GetS());  GlideN1.GetB()->C()++;  }
      if( ((K+L)%4) != 0 ){  GlideD1.GetB()->A() += ref.GetI();  GlideD1.GetB()->B() += olx_sqr(ref.GetS());  GlideD1.GetB()->C()++;  }
    }
    if( K == 0 )  {
      if( (H%2) != 0 )    {  GlideA2.GetB()->A() += ref.GetI();  GlideA2.GetB()->B() += olx_sqr(ref.GetS());  GlideA2.GetB()->C()++;  }
      if( (L%2) != 0 )    {  GlideC2.GetB()->A() += ref.GetI();  GlideC2.GetB()->B() += olx_sqr(ref.GetS());  GlideC2.GetB()->C()++;  }
      if( ((H+L)%2) != 0 ){  GlideN2.GetB()->A() += ref.GetI();  GlideN2.GetB()->B() += olx_sqr(ref.GetS());  GlideN2.GetB()->C()++;  }
      if( ((H+L)%4) != 0 ){  GlideD2.GetB()->A() += ref.GetI();  GlideD2.GetB()->B() += olx_sqr(ref.GetS());  GlideD2.GetB()->C()++;  }
    }
    if( L == 0 )  {
      if( (K%2) != 0 )    {  GlideB3.GetB()->A() += ref.GetI();  GlideB3.GetB()->B() += olx_sqr(ref.GetS());  GlideB3.GetB()->C()++;  }
      if( (H%2) != 0 )    {  GlideA3.GetB()->A() += ref.GetI();  GlideA3.GetB()->B() += olx_sqr(ref.GetS());  GlideA3.GetB()->C()++;  }
      if( ((H+K)%2) != 0 ){  GlideN3.GetB()->A() += ref.GetI();  GlideN3.GetB()->B() += olx_sqr(ref.GetS());  GlideN3.GetB()->C()++;  }
      if( ((H+K)%4) != 0 ){  GlideD3.GetB()->A() += ref.GetI();  GlideD3.GetB()->B() += olx_sqr(ref.GetS());  GlideD3.GetB()->C()++;  }
    }
    if( K == 0 && L == 0 )  {
      if( (H%2) != 0 )    {  Screw21.GetB()->A() += ref.GetI();  Screw21.GetB()->B() += olx_sqr(ref.GetS());  Screw21.GetB()->C()++;  }
      if( (H%4) != 0 )    {  Screw41.GetB()->A() += ref.GetI();  Screw41.GetB()->B() += olx_sqr(ref.GetS());  Screw41.GetB()->C()++;  }
    }
    if( H == 0 && L == 0 )  {
      if( (K%2) != 0 )    {  Screw22.GetB()->A() += ref.GetI();  Screw22.GetB()->B() += olx_sqr(ref.GetS());  Screw22.GetB()->C()++;  }
      if( (K%4) != 0 )    {  Screw42.GetB()->A() += ref.GetI();  Screw42.GetB()->B() += olx_sqr(ref.GetS());  Screw42.GetB()->C()++;  }
    }
    if( H == 0 && K == 0 )  {
      if( (L%2) != 0 )    {  Screw23.GetB()->A() += ref.GetI();  Screw23.GetB()->B() += olx_sqr(ref.GetS());  Screw23.GetB()->C()++;  }
      if( (L%4) != 0 )    {  Screw43.GetB()->A() += ref.GetI();  Screw43.GetB()->B() += olx_sqr(ref.GetS());  Screw43.GetB()->C()++;  }
      if( (L%3) != 0 )    {  Screw33.GetB()->A() += ref.GetI();  Screw33.GetB()->B() += olx_sqr(ref.GetS());  Screw33.GetB()->C()++;  }
      if( (L%6) != 0 )    {  Screw63.GetB()->A() += ref.GetI();  Screw63.GetB()->B() += olx_sqr(ref.GetS());  Screw63.GetB()->C()++;  }
    }
  }
  //double averageSAI = 0;
  //size_t saCount = 0;
  //for( size_t i=0; i < SAElements.Count(); i++ )  {
  //  SAElements[i].GetB()->B() = sqrt( SAElements[i].GetB()->GetB() );
  //  if( SAElements[i].GetB()->GetC() != 0 )  {
  //    averageSAI += SAElements[i].GetB()->GetA()/SAElements[i].GetB()->GetC();
  //    saCount++;
  //  }
  //}
  //if( saCount )  averageSAI /= saCount;
  //for( size_t i=0; i < SAElements.Count(); i++ )  {
  //  SAElements[i].GetB()->A() /= 100;
  //  SAElements[i].GetB()->B() = sqrt( SAElements[i].GetB()->GetB() )/100;
  //}
  for( size_t i=0; i < SAElements.Count(); i++ )  {
    SAElements[i].GetB()->B() = sqrt( SAElements[i].GetB()->GetB() );
    if( SAElements[i].GetB()->GetC() != 0 )  {
      if( SAElements[i].GetB()->GetA() > 0 )  {
        if( SAElements[i].GetB()->GetA()/SAElements[i].GetB()->GetC() < AverageI/7.5 || 
            SAElements[i].GetB()->GetA() < 4*SAElements[i].GetB()->GetB() )
          PresentElements.Add( &SAElements[i] );
      }
      else
        PresentElements.Add( &SAElements[i] );
    }
  }
  // second filtering ...
  if( PresentElements.Count() > 1 )  {
    double averageSAI = 0;
    double Ssq = 0;
    for( size_t i=0; i < PresentElements.Count(); i++ )  {
      double v = PresentElements[i]->GetB()->GetA()/PresentElements[i]->GetB()->GetC();
      if( v < 0 )  v = 0;
      averageSAI += v;
    }
    averageSAI /= PresentElements.Count();
    for( size_t i=0; i < PresentElements.Count(); i++ )  {
      double v = PresentElements[i]->GetB()->GetA()/PresentElements[i]->GetB()->GetC();
      Ssq += olx_abs(v-averageSAI);
    }
    Ssq /= PresentElements.Count();
    Ssq = sqrt( Ssq );                
    double maxInt = 1.0*(averageSAI + 2*Ssq);
    for( size_t i=0; i < PresentElements.Count(); i++ )  {
      double v = PresentElements[i]->GetB()->GetA()/PresentElements[i]->GetB()->GetC();
      if( v > maxInt && v > 1 && PresentElements[i]->GetB()->GetA() > 4*PresentElements[i]->GetB()->GetB() )  {
        PresentElements.Delete(i);
        i--;
      }
    }
  } // check if the absolute scale of is not rediculus...
  else  {
    if( PresentElements.Count() != 0 )  {
      double v = PresentElements[0]->GetB()->GetA()/PresentElements[0]->GetB()->GetC();
      if( v > 10 )  PresentElements.Clear();
    }
  }

  for( size_t i=0; i < SAElements.Count(); i++ )  {
    TSAStats& sas = saRes.AddNew<TSymmElement*, TwoDoublesInt>( SAElements[i].GetA(), *SAElements[i].GetB() );
    if( SAElements[i].GetB()->GetC() != 0 )  {
      if( PresentElements.IndexOf( &SAElements[i] ) != InvalidIndex )  {
        sas.SetPresent();
        bool excluded = false;
        for( size_t j=0; j < SAExclusions.Count(); j++ )  {
          if( PresentElements.IndexOf( SAExclusions[j].GetA() ) != InvalidIndex )  {
            if( SAExclusions[j].GetB()->IndexOf( &SAElements[i] ) != InvalidIndex )  {
             // mark removed elements
             excluded = true;
             size_t index = PresentElements.IndexOf( &SAElements[i] );
             PresentElements.Delete(index);
             break;
            }
          }
        }
        if( excluded )  sas.Exclude();
      }
    }
  }
  for( size_t i=0; i < LatticeHits.Count(); i++ )  {
    LatticeHits[i].GetB()->B() = sqrt( LatticeHits[i].GetB()->GetB() );
    LatticeHits[i].GetC()->B() = sqrt( LatticeHits[i].GetC()->GetB() );
    latRes.AddNew<TCLattice*, TwoDoublesInt, TwoDoublesInt>(LatticeHits[i].GetA(),
      *LatticeHits[i].GetB(),
      *LatticeHits[i].GetC() );
    delete LatticeHits[i].GetB();
    delete LatticeHits[i].GetC();
  }
  for( size_t i=0; i < SAElements.Count(); i++ )  {
    delete SAElements[i].GetB();
  }
}
//..............................................................................
void TSGTest::WeakRefTest(const TPtrList<TSpaceGroup>& sgList, TTypeList<TElementStats<TSpaceGroup*> >& res)  {
  typedef AnAssociation4<TSpaceGroup*, TwoDoublesInt, TwoDoublesInt, int> SGAss;
  typedef TPtrList<SGAss>  SGpList;
  typedef AnAssociation2<smatd, SGpList> MatrixAss;
  typedef TTypeList<SGAss>  SGList;
  TTypeList< MatrixAss > UniqMatrices;
  SGList SGHits;
  smatd_list sgMl;
  SGHits.SetCapacity( sgList.Count() );
  for( size_t i=0; i < sgList.Count(); i++ )  {
    TSpaceGroup& sg = *sgList[i];
    SGAss& sgv =
      SGHits.AddNew<TSpaceGroup*, TwoDoublesInt, TwoDoublesInt, int > (
                    &sg, TwoDoublesInt(0.0,0.0,0), TwoDoublesInt(0.0,0.0,0), 0 );
    sg.GetMatrices( sgMl, mattAll);
    const size_t ml_cnt = sgMl.Count();
    for( size_t j=0; j < ml_cnt; j++ )  {
      smatd& m = sgMl[j];
      // skip -I matrix
      if( m.r[0][0] == -1 && m.r[1][1] == -1 && m.r[2][2] == -1 &&
          m.r[0][1] == 0 && m.r[0][2] == 0 &&
          m.r[1][0] == 0 && m.r[1][2] == 0 &&
          m.r[2][0] == 0 && m.r[2][1] == 0 )
        continue;
      int iv = (int)m.t[0];  m.t[0] = olx_abs( m.t[0] - iv );
          iv = (int)m.t[1];  m.t[1] = olx_abs( m.t[1] - iv );
          iv = (int)m.t[2];  m.t[2] = olx_abs( m.t[2] - iv );
      // matrix should have a translation
      if( m.t.IsNull() )
        continue;

      if( m.t[0] > 0.5 )  m.t[0] = 1 - m.t[0];
      if( m.t[1] > 0.5 )  m.t[1] = 1 - m.t[1];
      if( m.t[2] > 0.5 )  m.t[2] = 1 - m.t[2];

      bool uniq = true;
      for( size_t k=0; k < UniqMatrices.Count(); k++ )  {
        if( UniqMatrices[k].GetA() == m )  {
          uniq = false;
          UniqMatrices[k].B().Add( &sgv );
          sgv.D() ++;
          break;
        }
      }
      if( uniq )  {
        sgv.D() ++;
        UniqMatrices.AddNew<smatd>(m).B().Add(&sgv);
      }
    }
    sgMl.Clear();
  }
  // collect statistrics for the matrices and the spacegroups
  const size_t um_cnt = UniqMatrices.Count();
  for( size_t i=0; i < um_cnt; i++ )  {
    const smatd& m = UniqMatrices[i].GetA();
    const size_t ref_cnt = Refs.Count(); 
    for( size_t j=0; j < ref_cnt; j++ )  {
      if(  Refs[j].IsSymmetric(m)  )  {
        double len = Refs[j].PhaseShift(m);
        if( olx_abs(len - olx_round(len)) < 0.01 )  {
          for( size_t k=0; k < UniqMatrices[i].GetB().Count(); k++ )  {
            UniqMatrices[i].GetB()[k]->B().A() += Refs[j].GetI();
            UniqMatrices[i].GetB()[k]->B().B() += Refs[j].GetS();
            UniqMatrices[i].GetB()[k]->B().C() ++;
          }
        }
        else  {
          for( size_t k=0; k < UniqMatrices[i].GetB().Count(); k++ )  {
            UniqMatrices[i].GetB()[k]->C().A() += Refs[j].GetI();
            UniqMatrices[i].GetB()[k]->C().B() += Refs[j].GetS();
            UniqMatrices[i].GetB()[k]->C().C() ++;
          }
        }
      }
    }
  }
  for( size_t i=0; i < SGHits.Count(); i++ )  {
    res.AddNew<TSpaceGroup*, TwoDoublesInt, TwoDoublesInt>( SGHits[i].GetA(),
      SGHits[i].GetB(),
      SGHits[i].GetC() );
  }
}
