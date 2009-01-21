#ifndef scat_itH
#define scat_itH

#include "xbase.h"

#include "exception.h"
#include "estlist.h"
#include "evector.h"

BeginXlibNamespace()
/*
  coefficients for scattering factors approximation;
  Data is taken from CCTBX file it1992.cpp  and wk1995.cpp
*/

class TLibScatterer  {
  double *Data;
  bool BuiltIn;
  int Id, size;
  double (TLibScatterer::*evalFunc)(double square_val) const;
protected:
  inline double Calc_sq9(double sqv) const {
    return Data[0]*exp(Data[4]*sqv) + Data[1]*exp(Data[5]*sqv) + Data[2]*exp(Data[6]*sqv) +
           Data[3]*exp(Data[7]*sqv) + Data[8];
  }
  inline double Calc_sq11(double sqv) const {
    return Data[0]*exp(Data[5]*sqv) + Data[1]*exp(Data[6]*sqv) + Data[2]*exp(Data[7]*sqv) +
           Data[3]*exp(Data[8]*sqv) + Data[4]*exp(Data[9]*sqv) + Data[10];
  }
  friend  class TScattererLib;
  /* some scatterers will be created while importing data from files, wthose have
    to be exported to that files too and presumably deleted, so IsBuiltIn() function
    helpt to resolve the issues with the determination where the scattere came from...
  */
  inline void SetBuiltIn()  {  BuiltIn = true;  }
  inline void SetId(int v)  {  Id = v;  }
public:
  TLibScatterer(double a1, double a2, double a3, double a4,
                    double b1, double b2, double b3, double b4,
                    double c )  {
    size = 9;
    Data = new double[size];
    Data[0] = a1;  Data[1] = a2;  Data[2] = a3;  Data[3] = a4;
    Data[4] = b1;  Data[5] = b2;  Data[6] = b3;  Data[7] = b4;
    Data[8] = c;
    evalFunc = &TLibScatterer::Calc_sq9;
    BuiltIn = false;
  }

  TLibScatterer(double a1, double a2, double a3, double a4, double a5,
                    double b1, double b2, double b3, double b4, double b5,
                    double c )  {
    size = 11;
    Data = new double[size];
    Data[0] = a1;  Data[1] = a2;  Data[2] = a3;  Data[3] = a4;  Data[4] = a5;
    Data[5] = b1;  Data[6] = b2;  Data[7] = b3;  Data[8] = b4;  Data[9] = b5;
    Data[10] = c;
    evalFunc = &TLibScatterer::Calc_sq11;
    BuiltIn = false;
  }

  TLibScatterer( const TLibScatterer& it )  {
    size = it.size;
    Data = new double[size];
    for( int i=0; i < size; i++ )
      Data[i] = it.Data[i];
    BuiltIn = false;
    Id = it.Id;
    evalFunc = it.evalFunc;
  }

  ~TLibScatterer()  {  delete [] Data;  }

  inline double Calc(double Theta, double Lambda)  const {  return Calc(-sqr(sin(Theta)/Lambda));  }
  inline double Calc(double val)    const {  return (this->*evalFunc)(-val*val);  }
  inline double Calc_sq(double sqv) const {  return (this->*evalFunc)(-sqv);  }

  inline bool IsBuiltIn() const {  return BuiltIn;  }
  inline int GetId()      const {  return Id;  }

  inline int Size()       const {  return size;  }

  inline const double* GetData()  const {  return Data;  }
//  inline const TVectorD& GetData()  const {  return Data;  }
};

class TScattererLib  {
  TCSTypeList<olxstr, TLibScatterer*> Data;
  void InitData9();
  void InitData11();
  void Clear();
public:
  TScattererLib(int size)  {
    if( size == 9 )       InitData9();
    else if( size == 11 ) InitData11();
    else                  throw TInvalidArgumentException(__OlxSourceInfo, "unknown scatterer size");
  }
  virtual ~TScattererLib()  {  Clear();  }

  inline TLibScatterer* Find(const olxstr& name)  {
    int ind = Data.IndexOfComparable( name );
    if( ind == -1 )  return NULL;
    return Data.Object(ind);
  }

  inline int Count()  const {  return Data.Count();  }
  // Scatterer GetId() can be used to access elements directly ...
  inline TLibScatterer* operator [] (int index)  {  return Data.Object(index);  }
};


EndXlibNamespace()
#endif
