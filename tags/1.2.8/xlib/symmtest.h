/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef symmtestH
#define symmtestH
#include "xbase.h"
#include "lattice.h"
#include "unitcell.h"
#undef QLength

BeginXlibNamespace()

  typedef TTypeList< olx_pair_t<size_t,size_t> > TIntPairList;

struct TSymmTestData  {
  vec3d Center;
  TIntPairList Atoms;
  TSymmTestData(const vec3d& v, size_t i, size_t j )  {
    Center = v;
    Atoms.AddNew<size_t, size_t>(i, j);
  }
  inline size_t Count() const {  return Atoms.Count();  }
};

class TSymmTest : public IOlxObject {
  TTypeList<olx_pair_t<vec3d,TCAtom*> > Atoms;
  vec3d GCenter;
  TTypeList<TSymmTestData> Vecs;

protected:
  static inline int VecsCmpByCount(const TSymmTestData &a , const TSymmTestData &b)  {
    return olx_cmp(a.Count(), b.Count());
  }
  static inline int VecsCmpByRadius(const AnAssociation3<vec3d,size_t,size_t> &a ,
                                     const AnAssociation3<vec3d,size_t,size_t> &b)  {
    return olx_cmp(a.GetA().QLength(), b.GetA().QLength());
  }

public:
  TSymmTest(const TUnitCell& cell)  {
    // get the whole unit cell context (including symmetry eqs)
    cell.GenereteAtomCoordinates( Atoms, false );
    for( size_t i=0; i < Atoms.Count(); i++ )
      GCenter += Atoms[i].GetA();
    if( Atoms.Count() != 0 )
      GCenter /= Atoms.Count();
  }

  inline const vec3d GetGravityCenter() const {  return GCenter;  }
  inline const  TTypeList< TSymmTestData >& GetResults() const  {  return Vecs;  }

  inline size_t AtomCount() const  {  return Atoms.Count();  }
  void Push(const vec3d& t)  {
    for( size_t i=0; i < Atoms.Count(); i++ )
      Atoms[i].a += t;
  }

  //bool EvaluateMatrix(int rowIndex, const vec3d& trans, smatd& res)  {
  //  if( rowIndex < 0 || rowIndex >= Vecs.Count() )
  //    throw TInvalidArgumentException(__OlxSourceInfo, "row index");

  //  res.Null();

  //  const TIntPairList& Row = Vecs[rowIndex].Atoms;

  //  if( Row.Count() >= 4 )  {
  //    TMatrixD gs(4,4), lm(Row.Count(), 4), lmt(4, Row.Count());
  //    TVectorD b(Row.Count()), gr(4), sol(4);
  //    vec3d cent;
  //    for( size_t i=0; i < Row.Count(); i++ )  {
  //      cent = Atoms[Row[i].GetB()].GetA();
  //      cent -= trans;
  //      for( int j=0; j < 3; j++ )  {
  //        if( cent[j] > 0.5 )  cent[j] -= 1;
  //        lm[i][j] = cent[j];
  //        lmt[j][i] = cent[j];
  //      }
  //      lm[i][3] = 1;
  //      lmt[3][i] = 1;
  //    }
  //    for(int i=0; i < 3; i++ )  {
  //      for( size_t j=0; j < Row.Count(); j++ )  {
  //        b[j] = Atoms[Row[j].GetA()].GetA()[i] - trans[i];
  //        if( b[j] > 0.5 )  b[j] -= 1;
  //      }

  //      gs = lmt * lm;
  //      gr = lmt * b;

  //      TMatrixD::GaussSolve(gs, gr, sol);
  //      res[i] = sol;
  //      sol.Null();
  //    }
  //    return true;
  //  }
  //  return false;
  //}

  void TestMatrix(const smatd& matr, double tol)  {
    TTypeList< AnAssociation3<vec3d,size_t,size_t> > tmpVecs;
    Vecs.Clear();
    Vecs.SetCapacity( (Atoms.Count()*Atoms.Count()-1)/2+1 );
    tmpVecs.SetCapacity( (Atoms.Count()*Atoms.Count()-1)/2+1 );
    for( size_t i=0; i < Atoms.Count(); i++ )  {
      for( size_t j=i; j < Atoms.Count(); j++ )  {
        vec3d a = matr * Atoms[j].GetA() - Atoms[i].GetA();
        for( int k=0; k < 3; k++ )  {
          a[k] -= olx_round(a[k]);
          if( a[k] < 0 )  a[k] += 1;
        }
        tmpVecs.AddNew<vec3d,size_t,size_t>(a, i, j);
      }
    }

    // get the square, as we compare quadratic distances
    const double stol = 3*tol*tol;  //!!

    // optimisation technique
    QuickSorter::SortSF(tmpVecs,VecsCmpByRadius);
    float* radii = new float[ tmpVecs.Count()+1];
    for( size_t i=0; i < tmpVecs.Count(); i++ )
      radii[i] = (float)tmpVecs[i].GetA().QLength();

    // collect the results
    for( size_t i=0; i < tmpVecs.Count(); i++ )  {
      if( tmpVecs.IsNull(i) )  continue;
      TSymmTestData* sd = new TSymmTestData(
        tmpVecs[i].GetA(), tmpVecs[i].GetB(), tmpVecs[i].GetC());
      Vecs.Add(*sd);
      double qval = radii[i];
      tmpVecs.NullItem(i);
      for( size_t j=i+1; j < tmpVecs.Count(); j++ )  {
        if( (radii[j] - qval) > stol )  break;
        if( tmpVecs.IsNull(j) )  continue;
        sd->Atoms.AddNew<size_t,size_t>( tmpVecs[j].GetB(), tmpVecs[j].GetC() );
        qval *= 0.75;
        qval += radii[j]*0.25;
        sd->Center += tmpVecs[j].GetA();  // accumulating center
        tmpVecs.NullItem(j);
      }
    }
    // do the averaging
    for( size_t i=0; i < Vecs.Count(); i++ )
      Vecs[i].Center /= Vecs[i].Count();

    delete [] radii;
    QuickSorter::SortSF(Vecs,VecsCmpByCount);
  }
  static void TestDependency(
    const TTypeList< olx_pair_t<vec3d,TCAtom*> >& lista,
    const TTypeList< olx_pair_t<vec3d,TCAtom*> >& listb,
    TTypeList< TSymmTestData >& Vecs,
    const smatd& matr, double tol)
  {
    TTypeList< AnAssociation3<vec3d,size_t,size_t> > tmpVecs;
    Vecs.Clear();
    Vecs.SetCapacity( lista.Count()*listb.Count()+1 );
    tmpVecs.SetCapacity( lista.Count()*listb.Count()+1 );
    vec3d a, b;
    for( size_t i=0; i < lista.Count(); i++ )  {
      for( size_t j=0; j < listb.Count(); j++ )  {
        a = lista[i].GetA();
        b = listb[j].GetA();
        b = matr * b;
        a += b;
        for( int k=0; k < 3; k++ )  {
          a[k] -= olx_round(a[k]);
          if( a[k] < 0 )  a[k] += 1;
          if( a[k] < tol )  a[k] = 0;
          else if( (1-a[k]) < tol )  a[k] = 0;
        }
        tmpVecs.AddNew<vec3d,size_t,size_t>(a, i, j);
      }
    }

    // get the square, as we compare quadratic distances
    double stol = 3*tol*tol, qval;  //!!

    // optimisation technique
    QuickSorter::SortSF(tmpVecs,VecsCmpByRadius);
    float* radii = new float[ tmpVecs.Count()+1];
    for( size_t i=0; i < tmpVecs.Count(); i++ )
      radii[i] = (float)tmpVecs[i].GetA().QLength();

    // collect the results
    for( size_t i=0; i < tmpVecs.Count(); i++ )  {
      if( tmpVecs.IsNull(i) )  continue;
      TSymmTestData* sd = new TSymmTestData(
        tmpVecs[i].GetA(), tmpVecs[i].GetB(), tmpVecs[i].GetC());
      Vecs.Add(*sd);
      a = tmpVecs[i].GetA(); // "crawling" center
      qval = radii[i];
      tmpVecs.NullItem(i);
      for( size_t j=i+1; j < tmpVecs.Count(); j++ )  {
        if( (radii[j] - qval) > stol )  break;
        if( tmpVecs.IsNull(j) )  continue;
        if( olx_abs(tmpVecs[j].GetA()[0]-a[0])  < tol &&
            olx_abs(tmpVecs[j].GetA()[1]-a[1])  < tol &&
            olx_abs(tmpVecs[j].GetA()[2]-a[2])  < tol  )  {
          sd->Atoms.AddNew<size_t,size_t>(tmpVecs[j].GetB(), tmpVecs[j].GetC());
          a *= 0.75;
          a += tmpVecs[j].GetA()*0.25;  // implement "crawling"

          qval *= 0.75;
          qval += radii[j]*0.25;
          //qval /= 2;
          //a /= 2;
          sd->Center += tmpVecs[j].GetA();  // accumulating center
          tmpVecs.NullItem(j);
        }
      }
    }
    // do the averaging
    for( size_t i=0; i < Vecs.Count(); i++ )
      Vecs[i].Center /= Vecs[i].Count();

    delete [] radii;
    QuickSorter::SortSF(Vecs,VecsCmpByCount);
  }
};

EndXlibNamespace()
#endif
