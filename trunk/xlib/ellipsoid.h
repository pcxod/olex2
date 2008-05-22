#ifndef ellipsoidH
#define ellipsoidH

#include "xbase.h"
#include "evector.h"
#include "ematrix.h"
#include "tptrlist.h"

BeginXlibNamespace()

/* Ellipsoid always must be in the cartesian frame */

class TEllipsoid: public IEObject  {
  bool FNPD;  // not positive defined
  double FQuad[6];       // quadratic form of the elipsoid
  TMatrixD Matrix;  // a normalised matrix, 4x4 to call to OpenGl
  double SX, SY, SZ;    // lengths of the vectores
  int Id; // do not change this value ! it is equal to the position in the AsymmUnit list
public:
  TEllipsoid();
  TEllipsoid(const TVectorD &Q);
  virtual ~TEllipsoid();
  void operator = (const TEllipsoid &E);
  void MultMatrix(const TMatrixD &M);
  // processes a symmetry matrix and updates object's data
  inline bool IsNPD()  const   {  return FNPD; };
  // return true if the ellipsoid is not positively defined
  void Initialise(const TVectorD &Q);  // calculates eigen values and vectors
  // Q - six values representing the quadratic form
  void Initialise();  // calculates eigen values and vectors; FQuad must be initialised
  // from example by loading from a stream
  void GetQuad(TVectorD &V) const;

  inline double GetSX()               const {  return SX;  }
  inline double GetSY()               const {  return SY;  }
  inline double GetSZ()               const {  return SZ;  }

  inline const TMatrixD&  GetMatrix() const {  return Matrix;  }
  // test versions
  olxstr UcifToUcart( class TAsymmUnit& au );
  olxstr UcartToUcif( class TAsymmUnit& au );

  DefPropP(int, Id)
};
  typedef TTypeList<TEllipsoid>  TEllpList;
  typedef TPtrList<TEllipsoid>  TEllpPList;
EndXlibNamespace()
#endif

