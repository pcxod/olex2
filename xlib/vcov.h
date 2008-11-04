#ifndef __olxs_v_co_v_h
#define __olxs_v_co_v_h
#include "lattice.h"
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
  TTypeList< AnAssociation2<olxstr, short> > Index;
protected:
  void Allocate(int w) {
    Clear();
    count = w;
    data = new double*[w];
    for( int i=0; i < w; i++ ) // bottom diagonal agrrangement
      data[i] = new double[i+1];
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
  void ReadShelxMat(const olxstr& fileName);
  // for tests
  double Find(const olxstr& atom, const short va, const short vy) const;
};

class VcoVContainer {
  VcoVMatrix vcov;
public:

  void ReadShelxMat(const olxstr& fileName) {
    vcov.ReadShelxMat(fileName);
  }
  TEValue<double> CalcDistance(const TSAtom& a1, const TSAtom& a2) {
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
