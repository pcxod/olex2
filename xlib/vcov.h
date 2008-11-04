#ifndef __olxs_v_co_v_h
#define __olxs_v_co_v_h
#include "asymmunit.h"
#include "lattice.h"
//#include "ematrix.h"
BeginXlibNamespace()

const short // constants decribing the stored values
  vcoviX = 0,
  vcoviY = 1,
  vcoviZ = 2,
  vcoviO = 3;

// stores X,Y,Z,SOF for each atom and their correlations
class VcoVMatrix {
  double **data;
  int count;
  // atom label, 
  TTypeList< AnAssociation3<olxstr, short, int> > Index;
protected:
  void Allocate(int w) {
    Clear();
    count = w;
    data = new double*[w];
    for( int i=0; i < w; i++ ) // bottom diagonal agrrangement
      data[i] = new double[i+1];
  }
  int FindAtomIndex(const TCAtom& a) const {
    for( int i=0; i < Index.Count(); i++ )
      if( Index[i].GetC() == a.GetLoaderId() )
        return i;
    return -1;
  }
public:
  VcoVMatrix()  {
    data = NULL;
    count = 0;
  }
  ~VcoVMatrix() {  Clear();  }
  void Clear() {
    if( data == NULL )  return;
    Index.Clear();
    for( int i=0; i < count; i++ )
      delete [] data[i];
    delete [] data;
    data = NULL;
  }
  double operator () (int i, int j) const {
    return (j <= i ) ? data[i][j] : data[j][i];
  }
  double Get(int i, int j) const {
    return (j <= i ) ? data[i][j] : data[j][i];
  }
  // reads the shelxl VcoV matrix and initliases atom loader Ids'
  void ReadShelxMat(const olxstr& fileName, TAsymmUnit& au);
  // finds s(x1), s(y1), s(z1), s(o1), s(x2), s(y2), s(z2), s(o2), cov(x1,x2) ... 
  void FindVcoV(const TCAtom& a1, const TCAtom& a2, ematd& m ) const;
  // for tests
  double Find(const olxstr& atom, const short va, const short vy) const;
};

class VcoVContainer {
  VcoVMatrix vcov;
public:

  void ReadShelxMat(const olxstr& fileName, TAsymmUnit& au) {
    vcov.ReadShelxMat(fileName, au);
  }
  TEValue<double> CalcDistance(const TSAtom& a1, const TSAtom& a2) {
    ematd m(4,4);
    vcov.FindVcoV(a1.CAtom(), a2.CAtom(), m);

    return TEValue<double>(0,0);
  }
  TEValue<double> CalcCentroid(const TSAtomPList& atoms) {
    return TEValue<double>(0,0);
  }

  TEValue<double> CalcAngle(const TSAtom& a1, const TSAtom& a2, const TSAtom& a3) {
    return TEValue<double>(0,0);
  }

  TEValue<double> CalcTAng(const TSAtom& a1, const TSAtom& a2, const TSAtom& a3, const TSAtom& a4) {
    return TEValue<double>(0,0);
  }
  const VcoVMatrix& GetMatrix() const {  return vcov;  }
};

EndXlibNamespace()
#endif
