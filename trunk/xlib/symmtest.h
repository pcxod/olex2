#ifndef symmtestH
#define symmtestH
#include "xbase.h"
#include "lattice.h"
#include "unitcell.h"
#undef QLength

BeginXlibNamespace()

  typedef TTypeList< AnAssociation2<int,int> > TIntPairList;

struct TSymmTestData  {
  vec3d Center;
  TIntPairList Atoms;
  TSymmTestData(const vec3d& v, int i, int j )  {
    Center = v;
    Atoms.AddNew<int, int>(i, j);
  }
  inline int Count() const  {  return Atoms.Count();  }
};

class TSymmTest : public IEObject {
  TTypeList< AnAssociation2<vec3d,TCAtom*> > Atoms;
  vec3d GCenter;
  TTypeList< TSymmTestData > Vecs;

protected:
  static inline int VecsCmpByCount( const TSymmTestData& a , const TSymmTestData& b )  {
    return a.Count() - b.Count();
  }
  static inline int VecsCmpByRadius( const AnAssociation3<vec3d,int,int>& a ,
                                     const AnAssociation3<vec3d,int,int>& b )  {
    double v = a.GetA().QLength() - b.GetA().QLength();
    //double v = a.GetA().QDistanceTo( b.GetA() );
    if( v < 0 )  return -1;
    return (v > 0) ? 1 : 0;
  }

public:
  TSymmTest(const TUnitCell& cell)  {
    // get the whole unit cell context (including symmetry eqs)
    cell.GenereteAtomCoordinates( Atoms, false );
    for( int i=0; i < Atoms.Count(); i++ )
      GCenter += Atoms[i].GetA();
    if( Atoms.Count() != 0 )
      GCenter /= Atoms.Count();
  }

  inline const vec3d GetGravityCenter()  const  {  return GCenter;  }
  inline const  TTypeList< TSymmTestData >& GetResults() const  {  return Vecs;  }

  inline const int AtomCount() const  {  return Atoms.Count();  }
  void Push(const vec3d& t)  {
    for( int i=0; i < Atoms.Count(); i++ )
      Atoms[i].A() += t; 
  }

  //bool EvaluateMatrix(int rowIndex, const vec3d& trans, symmd& res)  {
  //  if( rowIndex < 0 || rowIndex >= Vecs.Count() )
  //    throw TInvalidArgumentException(__OlxSourceInfo, "row index");

  //  res.Null();

  //  const TIntPairList& Row = Vecs[rowIndex].Atoms;

  //  if( Row.Count() >= 4 )  {
  //    TMatrixD gs(4,4), lm(Row.Count(), 4), lmt(4, Row.Count());
  //    TVectorD b(Row.Count()), gr(4), sol(4);
  //    vec3d cent;
  //    for( int i=0; i < Row.Count(); i++ )  {
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
  //      for( int j=0; j < Row.Count(); j++ )  {
  //        b[j] = Atoms[Row[j].GetA()].GetA()[i] - trans[i];
  //        if( b[j] > 0.5 )  b[j] -= 1;
  //      }

  //      gs = lmt * lm;
  //      gr = lmt * b;

  //      TMatrixD::GauseSolve(gs, gr, sol);
  //      res[i] = sol;
  //      sol.Null();
  //    }
  //    return true;
  //  }
  //  return false;
  //}

  void TestMatrix(const symmd& matr, double tol)  {
    TTypeList< AnAssociation3<vec3d,int,int> > tmpVecs;
    Vecs.Clear();
    Vecs.SetCapacity( (Atoms.Count()*Atoms.Count()-1)/2+1 );
    tmpVecs.SetCapacity( (Atoms.Count()*Atoms.Count()-1)/2+1 );
    vec3d a, b;
    for( int i=0; i < Atoms.Count(); i++ )  {
      for( int j=i+1; j < Atoms.Count(); j++ )  {
        a = Atoms[i].GetA();
        b = Atoms[j].GetA();
        b = matr * b;
        a += b;
        for( int k=0; k < 3; k++ )  {
          a[k] -= Round(a[k]);
          if( a[k] < 0 )  a[k] += 1;
          if( a[k] < tol )  a[k] = 0;
          else if( (1-a[k]) < tol )  a[k] = 0;
        }
        tmpVecs.AddNew<vec3d,int,int>(a, i, j);
      }
    }

    // get the square, as we compare quadratic distances
    double stol = 3*tol*tol, qval;  //!!

    // optimisation technique
    tmpVecs.QuickSorter.SortSF(tmpVecs,VecsCmpByRadius);
    float* radii = new float[ tmpVecs.Count()+1];
    for( int i=0; i < tmpVecs.Count(); i++ )
      radii[i] = (float)tmpVecs[i].GetA().QLength();

    // collect the results
    for(int i=0; i < tmpVecs.Count(); i++ )  {
      if( tmpVecs.IsNull(i) )  continue;
      TSymmTestData* sd = new TSymmTestData( tmpVecs[i].GetA(), tmpVecs[i].GetB(), tmpVecs[i].GetC());
      Vecs.Add(*sd);
      a = tmpVecs[i].GetA(); // "crawling" center
      qval = radii[i];
      tmpVecs.NullItem(i);
      for( int j=i+1; j < tmpVecs.Count(); j++ )  {
        if( (radii[j] - qval) > stol )  break;
        if( tmpVecs.IsNull(j) )  continue;
        if( fabs(tmpVecs[j].GetA()[0]-a[0])  < tol &&
            fabs(tmpVecs[j].GetA()[1]-a[1])  < tol &&
            fabs(tmpVecs[j].GetA()[2]-a[2])  < tol  )  {
          sd->Atoms.AddNew<int,int>( tmpVecs[j].GetB(), tmpVecs[j].GetC() );
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
    for( int i=0; i < Vecs.Count(); i++ )
      Vecs[i].Center /= Vecs[i].Count();

    delete [] radii;
    Vecs.QuickSorter.SortSF(Vecs,VecsCmpByCount);
  }
  static void TestDependency(const TTypeList< AnAssociation2<vec3d,TCAtom*> >& lista,
                             const TTypeList< AnAssociation2<vec3d,TCAtom*> >& listb,
                             TTypeList< TSymmTestData >& Vecs,
                             const symmd& matr, double tol)  {
    TTypeList< AnAssociation3<vec3d,int,int> > tmpVecs;
    Vecs.Clear();
    Vecs.SetCapacity( lista.Count()*listb.Count()+1 );
    tmpVecs.SetCapacity( lista.Count()*listb.Count()+1 );
    vec3d a, b;
    for( int i=0; i < lista.Count(); i++ )  {
      for( int j=0; j < listb.Count(); j++ )  {
        a = lista[i].GetA();
        b = listb[j].GetA();
        b = matr * b;
        a += b;
        for( int k=0; k < 3; k++ )  {
          a[k] -= Round(a[k]);
          if( a[k] < 0 )  a[k] += 1;
          if( a[k] < tol )  a[k] = 0;
          else if( (1-a[k]) < tol )  a[k] = 0;
        }
        tmpVecs.AddNew<vec3d,int,int>(a, i, j);
      }
    }

    // get the square, as we compare quadratic distances
    double stol = 3*tol*tol, qval;  //!!

    // optimisation technique
    tmpVecs.QuickSorter.SortSF(tmpVecs,VecsCmpByRadius);
    float* radii = new float[ tmpVecs.Count()+1];
    for( int i=0; i < tmpVecs.Count(); i++ )
      radii[i] = (float)tmpVecs[i].GetA().QLength();

    // collect the results
    for(int i=0; i < tmpVecs.Count(); i++ )  {
      if( tmpVecs.IsNull(i) )  continue;
      TSymmTestData* sd = new TSymmTestData( tmpVecs[i].GetA(), tmpVecs[i].GetB(), tmpVecs[i].GetC());
      Vecs.Add(*sd);
      a = tmpVecs[i].GetA(); // "crawling" center
      qval = radii[i];
      tmpVecs.NullItem(i);
      for( int j=i+1; j < tmpVecs.Count(); j++ )  {
        if( (radii[j] - qval) > stol )  break;
        if( tmpVecs.IsNull(j) )  continue;
        if( fabs(tmpVecs[j].GetA()[0]-a[0])  < tol &&
            fabs(tmpVecs[j].GetA()[1]-a[1])  < tol &&
            fabs(tmpVecs[j].GetA()[2]-a[2])  < tol  )  {
          sd->Atoms.AddNew<int,int>( tmpVecs[j].GetB(), tmpVecs[j].GetC() );
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
    for( int i=0; i < Vecs.Count(); i++ )
      Vecs[i].Center /= Vecs[i].Count();

    delete [] radii;
    Vecs.QuickSorter.SortSF(Vecs,VecsCmpByCount);
  }
};

EndXlibNamespace()
#endif
