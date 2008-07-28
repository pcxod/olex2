#ifndef __OLX_FAST_SYMM
#define __OLX_FAST_SYMM
#include "xbase.h"
#include "evector.h"
#include "estlist.h"

BeginXlibNamespace()

class SymmMatt {
  int R[9];
  double T[3];

};

struct FastSymm  {
  const int Count;
  typedef  void (*DV)(TVector<double>& v);
  typedef  void (*FV)(TVector<float>& v);
  typedef  void (*IV)(TVector<int>& v);
  typedef  void (*DA)(double* v);
  typedef  void (*FA)(float* v);
  typedef  void (*IA)(int* v);
  DV* DVs, *DVTs;
  FV* FVs, *FVTs;
  IV* IVs, *IVTs;
  DA* DAs, *DATs;
  FA* FAs, *FATs;
  IA* IAs, *IATs;
  inline void Mult(TVector<double>& v, int i)  const {  DVs[i](v);   }
  inline void Mult(TVector<float>& v, int i)   const {  FVs[i](v);   }
  inline void Mult(TVector<int>& v, int i)     const {  IVs[i](v);   }
  inline void Mult(double* v, int i)           const {  DAs[i](v);   }
  inline void Mult(float* v, int i)            const {  FAs[i](v);   }
  inline void Mult(int* v, int i)              const {  IAs[i](v);   }
  inline void MultT(TVector<double>& v, int i) const {  DVTs[i](v);  }
  inline void MultT(TVector<float>& v, int i)  const {  FVTs[i](v);  }
  inline void MultT(TVector<int>& v, int i)    const {  IVTs[i](v);  }
  inline void MultT(double* v, int i)          const {  DATs[i](v);  }
  inline void MultT(float* v, int i)           const {  FATs[i](v);  }
  inline void MultT(int* v, int i)             const {  IATs[i](v);  }
  FastSymm(int cnt) : Count(cnt)  {
    DVs = new DV[cnt];
    FVs = new FV[cnt];
    IVs = new IV[cnt];
    DVTs = new DV[cnt];
    FVTs = new FV[cnt];
    IVTs = new IV[cnt];
    DAs = new DA[cnt];
    FAs = new FA[cnt];
    IAs = new IA[cnt];
    DATs = new DA[cnt];
    FATs = new FA[cnt];
    IATs = new IA[cnt];
  }
  ~FastSymm()  {  
    delete [] DVs;  
    delete [] FVs;  
    delete [] IVs;  
    delete [] DVTs;  
    delete [] FVTs;  
    delete [] IVTs;  
    delete [] DAs;  
    delete [] FAs;  
    delete [] IAs;  
    delete [] DATs;  
    delete [] FATs;  
    delete [] IATs;  
  }
};

class TFastSymmLib  {
  TSStrPObjList<olxstr, FastSymm*, true> list;
  FastSymm::DV MultDV[64], MultDVT[64];
  FastSymm::FV MultFV[64], MultFVT[64];
  FastSymm::IV MultIV[64], MultIVT[64];
  FastSymm::DA MultDA[64], MultDAT[64];
  FastSymm::FA MultFA[64], MultFAT[64];
  FastSymm::IA MultIA[64], MultIAT[64];
  void processSymm(const TIntList& mind, int count, FastSymm& fs);
public:
  TFastSymmLib();
  ~TFastSymmLib();
  FastSymm* FindSymm(const olxstr& name)  {  return list[name];  }
};
/* automatically generated code */
struct FastMatrices {
// +X,+Y,+Z
  static inline void MultDV211121112(TVector<double>& v)  {  }
  static inline void MultDA211121112(double* v)  {  }
  static inline void MultFV211121112(TVector<float>& v)  {  }
  static inline void MultFA211121112(float* v)  {  }
  static inline void MultIV211121112(TVector<int>& v)  {  }
  static inline void MultIA211121112(int* v)  {  }
  static inline void MultDVT211121112(TVector<double>& v)  {  }
  static inline void MultDAT211121112(double* v)  {  }
  static inline void MultFVT211121112(TVector<float>& v)  {  }
  static inline void MultFAT211121112(float* v)  {  }
  static inline void MultIVT211121112(TVector<int>& v)  {  }
  static inline void MultIAT211121112(int* v)  {  }
// -X,-Y,+Z
  static inline void MultDV011101112(TVector<double>& v)  {
    v[0] = -v[0];    v[1] = -v[1];
  }
  static inline void MultDA011101112(double* v)  {
    v[0] = -v[0];    v[1] = -v[1];
  }
  static inline void MultFV011101112(TVector<float>& v)  {
    v[0] = -v[0];    v[1] = -v[1];
  }
  static inline void MultFA011101112(float* v)  {
    v[0] = -v[0];    v[1] = -v[1];
  }
  static inline void MultIV011101112(TVector<int>& v)  {
    v[0] = -v[0];    v[1] = -v[1];
  }
  static inline void MultIA011101112(int* v)  {
    v[0] = -v[0];    v[1] = -v[1];
  }
  static inline void MultDVT011101112(TVector<double>& v)  {
    v[0] = -v[0];    v[1] = -v[1];
  }
  static inline void MultDAT011101112(double* v)  {
    v[0] = -v[0];    v[1] = -v[1];
  }
  static inline void MultFVT011101112(TVector<float>& v)  {
    v[0] = -v[0];    v[1] = -v[1];
  }
  static inline void MultFAT011101112(float* v)  {
    v[0] = -v[0];    v[1] = -v[1];
  }
  static inline void MultIVT011101112(TVector<int>& v)  {
    v[0] = -v[0];    v[1] = -v[1];
  }
  static inline void MultIAT011101112(int* v)  {
    v[0] = -v[0];    v[1] = -v[1];
  }
// -X,-Y,-Z
  static inline void MultDV011101110(TVector<double>& v)  {
    v[0] = -v[0];    v[1] = -v[1];    v[2] = -v[2];
  }
  static inline void MultDA011101110(double* v)  {
    v[0] = -v[0];    v[1] = -v[1];    v[2] = -v[2];
  }
  static inline void MultFV011101110(TVector<float>& v)  {
    v[0] = -v[0];    v[1] = -v[1];    v[2] = -v[2];
  }
  static inline void MultFA011101110(float* v)  {
    v[0] = -v[0];    v[1] = -v[1];    v[2] = -v[2];
  }
  static inline void MultIV011101110(TVector<int>& v)  {
    v[0] = -v[0];    v[1] = -v[1];    v[2] = -v[2];
  }
  static inline void MultIA011101110(int* v)  {
    v[0] = -v[0];    v[1] = -v[1];    v[2] = -v[2];
  }
  static inline void MultDVT011101110(TVector<double>& v)  {
    v[0] = -v[0];    v[1] = -v[1];    v[2] = -v[2];
  }
  static inline void MultDAT011101110(double* v)  {
    v[0] = -v[0];    v[1] = -v[1];    v[2] = -v[2];
  }
  static inline void MultFVT011101110(TVector<float>& v)  {
    v[0] = -v[0];    v[1] = -v[1];    v[2] = -v[2];
  }
  static inline void MultFAT011101110(float* v)  {
    v[0] = -v[0];    v[1] = -v[1];    v[2] = -v[2];
  }
  static inline void MultIVT011101110(TVector<int>& v)  {
    v[0] = -v[0];    v[1] = -v[1];    v[2] = -v[2];
  }
  static inline void MultIAT011101110(int* v)  {
    v[0] = -v[0];    v[1] = -v[1];    v[2] = -v[2];
  }
// +X,+Y,-Z
  static inline void MultDV211121110(TVector<double>& v)  {
    v[2] = -v[2];
  }
  static inline void MultDA211121110(double* v)  {
    v[2] = -v[2];
  }
  static inline void MultFV211121110(TVector<float>& v)  {
    v[2] = -v[2];
  }
  static inline void MultFA211121110(float* v)  {
    v[2] = -v[2];
  }
  static inline void MultIV211121110(TVector<int>& v)  {
    v[2] = -v[2];
  }
  static inline void MultIA211121110(int* v)  {
    v[2] = -v[2];
  }
  static inline void MultDVT211121110(TVector<double>& v)  {
    v[2] = -v[2];
  }
  static inline void MultDAT211121110(double* v)  {
    v[2] = -v[2];
  }
  static inline void MultFVT211121110(TVector<float>& v)  {
    v[2] = -v[2];
  }
  static inline void MultFAT211121110(float* v)  {
    v[2] = -v[2];
  }
  static inline void MultIVT211121110(TVector<int>& v)  {
    v[2] = -v[2];
  }
  static inline void MultIAT211121110(int* v)  {
    v[2] = -v[2];
  }
// -X,+Y,-Z
  static inline void MultDV011121110(TVector<double>& v)  {
    v[0] = -v[0];    v[2] = -v[2];
  }
  static inline void MultDA011121110(double* v)  {
    v[0] = -v[0];    v[2] = -v[2];
  }
  static inline void MultFV011121110(TVector<float>& v)  {
    v[0] = -v[0];    v[2] = -v[2];
  }
  static inline void MultFA011121110(float* v)  {
    v[0] = -v[0];    v[2] = -v[2];
  }
  static inline void MultIV011121110(TVector<int>& v)  {
    v[0] = -v[0];    v[2] = -v[2];
  }
  static inline void MultIA011121110(int* v)  {
    v[0] = -v[0];    v[2] = -v[2];
  }
  static inline void MultDVT011121110(TVector<double>& v)  {
    v[0] = -v[0];    v[2] = -v[2];
  }
  static inline void MultDAT011121110(double* v)  {
    v[0] = -v[0];    v[2] = -v[2];
  }
  static inline void MultFVT011121110(TVector<float>& v)  {
    v[0] = -v[0];    v[2] = -v[2];
  }
  static inline void MultFAT011121110(float* v)  {
    v[0] = -v[0];    v[2] = -v[2];
  }
  static inline void MultIVT011121110(TVector<int>& v)  {
    v[0] = -v[0];    v[2] = -v[2];
  }
  static inline void MultIAT011121110(int* v)  {
    v[0] = -v[0];    v[2] = -v[2];
  }
// +X,-Y,+Z
  static inline void MultDV211101112(TVector<double>& v)  {
    v[1] = -v[1];
  }
  static inline void MultDA211101112(double* v)  {
    v[1] = -v[1];
  }
  static inline void MultFV211101112(TVector<float>& v)  {
    v[1] = -v[1];
  }
  static inline void MultFA211101112(float* v)  {
    v[1] = -v[1];
  }
  static inline void MultIV211101112(TVector<int>& v)  {
    v[1] = -v[1];
  }
  static inline void MultIA211101112(int* v)  {
    v[1] = -v[1];
  }
  static inline void MultDVT211101112(TVector<double>& v)  {
    v[1] = -v[1];
  }
  static inline void MultDAT211101112(double* v)  {
    v[1] = -v[1];
  }
  static inline void MultFVT211101112(TVector<float>& v)  {
    v[1] = -v[1];
  }
  static inline void MultFAT211101112(float* v)  {
    v[1] = -v[1];
  }
  static inline void MultIVT211101112(TVector<int>& v)  {
    v[1] = -v[1];
  }
  static inline void MultIAT211101112(int* v)  {
    v[1] = -v[1];
  }
// +X,-Y,-Z
  static inline void MultDV211101110(TVector<double>& v)  {
    v[1] = -v[1];    v[2] = -v[2];
  }
  static inline void MultDA211101110(double* v)  {
    v[1] = -v[1];    v[2] = -v[2];
  }
  static inline void MultFV211101110(TVector<float>& v)  {
    v[1] = -v[1];    v[2] = -v[2];
  }
  static inline void MultFA211101110(float* v)  {
    v[1] = -v[1];    v[2] = -v[2];
  }
  static inline void MultIV211101110(TVector<int>& v)  {
    v[1] = -v[1];    v[2] = -v[2];
  }
  static inline void MultIA211101110(int* v)  {
    v[1] = -v[1];    v[2] = -v[2];
  }
  static inline void MultDVT211101110(TVector<double>& v)  {
    v[1] = -v[1];    v[2] = -v[2];
  }
  static inline void MultDAT211101110(double* v)  {
    v[1] = -v[1];    v[2] = -v[2];
  }
  static inline void MultFVT211101110(TVector<float>& v)  {
    v[1] = -v[1];    v[2] = -v[2];
  }
  static inline void MultFAT211101110(float* v)  {
    v[1] = -v[1];    v[2] = -v[2];
  }
  static inline void MultIVT211101110(TVector<int>& v)  {
    v[1] = -v[1];    v[2] = -v[2];
  }
  static inline void MultIAT211101110(int* v)  {
    v[1] = -v[1];    v[2] = -v[2];
  }
// -X,+Y,+Z
  static inline void MultDV011121112(TVector<double>& v)  {
    v[0] = -v[0];
  }
  static inline void MultDA011121112(double* v)  {
    v[0] = -v[0];
  }
  static inline void MultFV011121112(TVector<float>& v)  {
    v[0] = -v[0];
  }
  static inline void MultFA011121112(float* v)  {
    v[0] = -v[0];
  }
  static inline void MultIV011121112(TVector<int>& v)  {
    v[0] = -v[0];
  }
  static inline void MultIA011121112(int* v)  {
    v[0] = -v[0];
  }
  static inline void MultDVT011121112(TVector<double>& v)  {
    v[0] = -v[0];
  }
  static inline void MultDAT011121112(double* v)  {
    v[0] = -v[0];
  }
  static inline void MultFVT011121112(TVector<float>& v)  {
    v[0] = -v[0];
  }
  static inline void MultFAT011121112(float* v)  {
    v[0] = -v[0];
  }
  static inline void MultIVT011121112(TVector<int>& v)  {
    v[0] = -v[0];
  }
  static inline void MultIAT011121112(int* v)  {
    v[0] = -v[0];
  }
// +Y,-X,-Z
  static inline void MultDV121011110(TVector<double>& v)  {
    double a0 = v[1];
    v[1] = -v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultDA121011110(double* v)  {
    double a0 = v[1];
    v[1] = -v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultFV121011110(TVector<float>& v)  {
    float a0 = v[1];
    v[1] = -v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultFA121011110(float* v)  {
    float a0 = v[1];
    v[1] = -v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultIV121011110(TVector<int>& v)  {
    int a0 = v[1];
    v[1] = -v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultIA121011110(int* v)  {
    int a0 = v[1];
    v[1] = -v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultDVT121011110(TVector<double>& v)  {
    double a0 = -v[1];
    v[1] = v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultDAT121011110(double* v)  {
    double a0 = -v[1];
    v[1] = v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultFVT121011110(TVector<float>& v)  {
    float a0 = -v[1];
    v[1] = v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultFAT121011110(float* v)  {
    float a0 = -v[1];
    v[1] = v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultIVT121011110(TVector<int>& v)  {
    int a0 = -v[1];
    v[1] = v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultIAT121011110(int* v)  {
    int a0 = -v[1];
    v[1] = v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
// -Y,+X,-Z
  static inline void MultDV101211110(TVector<double>& v)  {
    double a0 = -v[1];
    v[1] = v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultDA101211110(double* v)  {
    double a0 = -v[1];
    v[1] = v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultFV101211110(TVector<float>& v)  {
    float a0 = -v[1];
    v[1] = v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultFA101211110(float* v)  {
    float a0 = -v[1];
    v[1] = v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultIV101211110(TVector<int>& v)  {
    int a0 = -v[1];
    v[1] = v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultIA101211110(int* v)  {
    int a0 = -v[1];
    v[1] = v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultDVT101211110(TVector<double>& v)  {
    double a0 = v[1];
    v[1] = -v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultDAT101211110(double* v)  {
    double a0 = v[1];
    v[1] = -v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultFVT101211110(TVector<float>& v)  {
    float a0 = v[1];
    v[1] = -v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultFAT101211110(float* v)  {
    float a0 = v[1];
    v[1] = -v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultIVT101211110(TVector<int>& v)  {
    int a0 = v[1];
    v[1] = -v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultIAT101211110(int* v)  {
    int a0 = v[1];
    v[1] = -v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
// -X,+Z,-Y
  static inline void MultDV011112101(TVector<double>& v)  {
    v[0] = -v[0];
    double a1 = v[2];
    v[2] = -v[1];
    v[1] = a1;
  }
  static inline void MultDA011112101(double* v)  {
    v[0] = -v[0];
    double a1 = v[2];
    v[2] = -v[1];
    v[1] = a1;
  }
  static inline void MultFV011112101(TVector<float>& v)  {
    v[0] = -v[0];
    float a1 = v[2];
    v[2] = -v[1];
    v[1] = a1;
  }
  static inline void MultFA011112101(float* v)  {
    v[0] = -v[0];
    float a1 = v[2];
    v[2] = -v[1];
    v[1] = a1;
  }
  static inline void MultIV011112101(TVector<int>& v)  {
    v[0] = -v[0];
    int a1 = v[2];
    v[2] = -v[1];
    v[1] = a1;
  }
  static inline void MultIA011112101(int* v)  {
    v[0] = -v[0];
    int a1 = v[2];
    v[2] = -v[1];
    v[1] = a1;
  }
  static inline void MultDVT011112101(TVector<double>& v)  {
    v[0] = -v[0];
    double a1 = -v[2];
    v[2] = v[1];
    v[1] = a1;
  }
  static inline void MultDAT011112101(double* v)  {
    v[0] = -v[0];
    double a1 = -v[2];
    v[2] = v[1];
    v[1] = a1;
  }
  static inline void MultFVT011112101(TVector<float>& v)  {
    v[0] = -v[0];
    float a1 = -v[2];
    v[2] = v[1];
    v[1] = a1;
  }
  static inline void MultFAT011112101(float* v)  {
    v[0] = -v[0];
    float a1 = -v[2];
    v[2] = v[1];
    v[1] = a1;
  }
  static inline void MultIVT011112101(TVector<int>& v)  {
    v[0] = -v[0];
    int a1 = -v[2];
    v[2] = v[1];
    v[1] = a1;
  }
  static inline void MultIAT011112101(int* v)  {
    v[0] = -v[0];
    int a1 = -v[2];
    v[2] = v[1];
    v[1] = a1;
  }
// -X,-Z,+Y
  static inline void MultDV011110121(TVector<double>& v)  {
    v[0] = -v[0];
    double a1 = -v[2];
    v[2] = v[1];
    v[1] = a1;
  }
  static inline void MultDA011110121(double* v)  {
    v[0] = -v[0];
    double a1 = -v[2];
    v[2] = v[1];
    v[1] = a1;
  }
  static inline void MultFV011110121(TVector<float>& v)  {
    v[0] = -v[0];
    float a1 = -v[2];
    v[2] = v[1];
    v[1] = a1;
  }
  static inline void MultFA011110121(float* v)  {
    v[0] = -v[0];
    float a1 = -v[2];
    v[2] = v[1];
    v[1] = a1;
  }
  static inline void MultIV011110121(TVector<int>& v)  {
    v[0] = -v[0];
    int a1 = -v[2];
    v[2] = v[1];
    v[1] = a1;
  }
  static inline void MultIA011110121(int* v)  {
    v[0] = -v[0];
    int a1 = -v[2];
    v[2] = v[1];
    v[1] = a1;
  }
  static inline void MultDVT011110121(TVector<double>& v)  {
    v[0] = -v[0];
    double a1 = v[2];
    v[2] = -v[1];
    v[1] = a1;
  }
  static inline void MultDAT011110121(double* v)  {
    v[0] = -v[0];
    double a1 = v[2];
    v[2] = -v[1];
    v[1] = a1;
  }
  static inline void MultFVT011110121(TVector<float>& v)  {
    v[0] = -v[0];
    float a1 = v[2];
    v[2] = -v[1];
    v[1] = a1;
  }
  static inline void MultFAT011110121(float* v)  {
    v[0] = -v[0];
    float a1 = v[2];
    v[2] = -v[1];
    v[1] = a1;
  }
  static inline void MultIVT011110121(TVector<int>& v)  {
    v[0] = -v[0];
    int a1 = v[2];
    v[2] = -v[1];
    v[1] = a1;
  }
  static inline void MultIAT011110121(int* v)  {
    v[0] = -v[0];
    int a1 = v[2];
    v[2] = -v[1];
    v[1] = a1;
  }
// -Z,-Y,+X
  static inline void MultDV110101211(TVector<double>& v)  {
    double a0 = -v[2];
    v[1] = -v[1];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultDA110101211(double* v)  {
    double a0 = -v[2];
    v[1] = -v[1];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultFV110101211(TVector<float>& v)  {
    float a0 = -v[2];
    v[1] = -v[1];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultFA110101211(float* v)  {
    float a0 = -v[2];
    v[1] = -v[1];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultIV110101211(TVector<int>& v)  {
    int a0 = -v[2];
    v[1] = -v[1];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultIA110101211(int* v)  {
    int a0 = -v[2];
    v[1] = -v[1];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultDVT110101211(TVector<double>& v)  {
    double a0 = v[2];
    v[1] = -v[1];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultDAT110101211(double* v)  {
    double a0 = v[2];
    v[1] = -v[1];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultFVT110101211(TVector<float>& v)  {
    float a0 = v[2];
    v[1] = -v[1];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultFAT110101211(float* v)  {
    float a0 = v[2];
    v[1] = -v[1];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultIVT110101211(TVector<int>& v)  {
    int a0 = v[2];
    v[1] = -v[1];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultIAT110101211(int* v)  {
    int a0 = v[2];
    v[1] = -v[1];
    v[2] = -v[0];
    v[0] = a0;
  }
// +Z,-Y,-X
  static inline void MultDV112101011(TVector<double>& v)  {
    double a0 = v[2];
    v[1] = -v[1];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultDA112101011(double* v)  {
    double a0 = v[2];
    v[1] = -v[1];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultFV112101011(TVector<float>& v)  {
    float a0 = v[2];
    v[1] = -v[1];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultFA112101011(float* v)  {
    float a0 = v[2];
    v[1] = -v[1];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultIV112101011(TVector<int>& v)  {
    int a0 = v[2];
    v[1] = -v[1];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultIA112101011(int* v)  {
    int a0 = v[2];
    v[1] = -v[1];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultDVT112101011(TVector<double>& v)  {
    double a0 = -v[2];
    v[1] = -v[1];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultDAT112101011(double* v)  {
    double a0 = -v[2];
    v[1] = -v[1];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultFVT112101011(TVector<float>& v)  {
    float a0 = -v[2];
    v[1] = -v[1];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultFAT112101011(float* v)  {
    float a0 = -v[2];
    v[1] = -v[1];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultIVT112101011(TVector<int>& v)  {
    int a0 = -v[2];
    v[1] = -v[1];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultIAT112101011(int* v)  {
    int a0 = -v[2];
    v[1] = -v[1];
    v[2] = v[0];
    v[0] = a0;
  }
// +Z,+X,+Y
  static inline void MultDV112211121(TVector<double>& v)  {
    double a0 = v[2];
    double a1 = v[0];
    v[2] = v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultDA112211121(double* v)  {
    double a0 = v[2];
    double a1 = v[0];
    v[2] = v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultFV112211121(TVector<float>& v)  {
    float a0 = v[2];
    float a1 = v[0];
    v[2] = v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultFA112211121(float* v)  {
    float a0 = v[2];
    float a1 = v[0];
    v[2] = v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultIV112211121(TVector<int>& v)  {
    int a0 = v[2];
    int a1 = v[0];
    v[2] = v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultIA112211121(int* v)  {
    int a0 = v[2];
    int a1 = v[0];
    v[2] = v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultDVT112211121(TVector<double>& v)  {
    double a0 = v[1];
    v[1] = v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultDAT112211121(double* v)  {
    double a0 = v[1];
    v[1] = v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultFVT112211121(TVector<float>& v)  {
    float a0 = v[1];
    v[1] = v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultFAT112211121(float* v)  {
    float a0 = v[1];
    v[1] = v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultIVT112211121(TVector<int>& v)  {
    int a0 = v[1];
    v[1] = v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultIAT112211121(int* v)  {
    int a0 = v[1];
    v[1] = v[2];
    v[2] = v[0];
    v[0] = a0;
  }
// +Y,+Z,+X
  static inline void MultDV121112211(TVector<double>& v)  {
    double a0 = v[1];
    v[1] = v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultDA121112211(double* v)  {
    double a0 = v[1];
    v[1] = v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultFV121112211(TVector<float>& v)  {
    float a0 = v[1];
    v[1] = v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultFA121112211(float* v)  {
    float a0 = v[1];
    v[1] = v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultIV121112211(TVector<int>& v)  {
    int a0 = v[1];
    v[1] = v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultIA121112211(int* v)  {
    int a0 = v[1];
    v[1] = v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultDVT121112211(TVector<double>& v)  {
    double a0 = v[2];
    double a1 = v[0];
    v[2] = v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultDAT121112211(double* v)  {
    double a0 = v[2];
    double a1 = v[0];
    v[2] = v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultFVT121112211(TVector<float>& v)  {
    float a0 = v[2];
    float a1 = v[0];
    v[2] = v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultFAT121112211(float* v)  {
    float a0 = v[2];
    float a1 = v[0];
    v[2] = v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultIVT121112211(TVector<int>& v)  {
    int a0 = v[2];
    int a1 = v[0];
    v[2] = v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultIAT121112211(int* v)  {
    int a0 = v[2];
    int a1 = v[0];
    v[2] = v[1];
    v[0] = a0;
    v[1] = a1;
  }
// -Y,-Z,+X
  static inline void MultDV101110211(TVector<double>& v)  {
    double a0 = -v[1];
    v[1] = -v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultDA101110211(double* v)  {
    double a0 = -v[1];
    v[1] = -v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultFV101110211(TVector<float>& v)  {
    float a0 = -v[1];
    v[1] = -v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultFA101110211(float* v)  {
    float a0 = -v[1];
    v[1] = -v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultIV101110211(TVector<int>& v)  {
    int a0 = -v[1];
    v[1] = -v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultIA101110211(int* v)  {
    int a0 = -v[1];
    v[1] = -v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultDVT101110211(TVector<double>& v)  {
    double a0 = v[2];
    double a1 = -v[0];
    v[2] = -v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultDAT101110211(double* v)  {
    double a0 = v[2];
    double a1 = -v[0];
    v[2] = -v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultFVT101110211(TVector<float>& v)  {
    float a0 = v[2];
    float a1 = -v[0];
    v[2] = -v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultFAT101110211(float* v)  {
    float a0 = v[2];
    float a1 = -v[0];
    v[2] = -v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultIVT101110211(TVector<int>& v)  {
    int a0 = v[2];
    int a1 = -v[0];
    v[2] = -v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultIAT101110211(int* v)  {
    int a0 = v[2];
    int a1 = -v[0];
    v[2] = -v[1];
    v[0] = a0;
    v[1] = a1;
  }
// +Z,-X,-Y
  static inline void MultDV112011101(TVector<double>& v)  {
    double a0 = v[2];
    double a1 = -v[0];
    v[2] = -v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultDA112011101(double* v)  {
    double a0 = v[2];
    double a1 = -v[0];
    v[2] = -v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultFV112011101(TVector<float>& v)  {
    float a0 = v[2];
    float a1 = -v[0];
    v[2] = -v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultFA112011101(float* v)  {
    float a0 = v[2];
    float a1 = -v[0];
    v[2] = -v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultIV112011101(TVector<int>& v)  {
    int a0 = v[2];
    int a1 = -v[0];
    v[2] = -v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultIA112011101(int* v)  {
    int a0 = v[2];
    int a1 = -v[0];
    v[2] = -v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultDVT112011101(TVector<double>& v)  {
    double a0 = -v[1];
    v[1] = -v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultDAT112011101(double* v)  {
    double a0 = -v[1];
    v[1] = -v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultFVT112011101(TVector<float>& v)  {
    float a0 = -v[1];
    v[1] = -v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultFAT112011101(float* v)  {
    float a0 = -v[1];
    v[1] = -v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultIVT112011101(TVector<int>& v)  {
    int a0 = -v[1];
    v[1] = -v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultIAT112011101(int* v)  {
    int a0 = -v[1];
    v[1] = -v[2];
    v[2] = v[0];
    v[0] = a0;
  }
// -Y,+Z,-X
  static inline void MultDV101112011(TVector<double>& v)  {
    double a0 = -v[1];
    v[1] = v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultDA101112011(double* v)  {
    double a0 = -v[1];
    v[1] = v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultFV101112011(TVector<float>& v)  {
    float a0 = -v[1];
    v[1] = v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultFA101112011(float* v)  {
    float a0 = -v[1];
    v[1] = v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultIV101112011(TVector<int>& v)  {
    int a0 = -v[1];
    v[1] = v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultIA101112011(int* v)  {
    int a0 = -v[1];
    v[1] = v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultDVT101112011(TVector<double>& v)  {
    double a0 = -v[2];
    double a1 = -v[0];
    v[2] = v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultDAT101112011(double* v)  {
    double a0 = -v[2];
    double a1 = -v[0];
    v[2] = v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultFVT101112011(TVector<float>& v)  {
    float a0 = -v[2];
    float a1 = -v[0];
    v[2] = v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultFAT101112011(float* v)  {
    float a0 = -v[2];
    float a1 = -v[0];
    v[2] = v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultIVT101112011(TVector<int>& v)  {
    int a0 = -v[2];
    int a1 = -v[0];
    v[2] = v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultIAT101112011(int* v)  {
    int a0 = -v[2];
    int a1 = -v[0];
    v[2] = v[1];
    v[0] = a0;
    v[1] = a1;
  }
// -Z,-X,+Y
  static inline void MultDV110011121(TVector<double>& v)  {
    double a0 = -v[2];
    double a1 = -v[0];
    v[2] = v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultDA110011121(double* v)  {
    double a0 = -v[2];
    double a1 = -v[0];
    v[2] = v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultFV110011121(TVector<float>& v)  {
    float a0 = -v[2];
    float a1 = -v[0];
    v[2] = v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultFA110011121(float* v)  {
    float a0 = -v[2];
    float a1 = -v[0];
    v[2] = v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultIV110011121(TVector<int>& v)  {
    int a0 = -v[2];
    int a1 = -v[0];
    v[2] = v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultIA110011121(int* v)  {
    int a0 = -v[2];
    int a1 = -v[0];
    v[2] = v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultDVT110011121(TVector<double>& v)  {
    double a0 = -v[1];
    v[1] = v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultDAT110011121(double* v)  {
    double a0 = -v[1];
    v[1] = v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultFVT110011121(TVector<float>& v)  {
    float a0 = -v[1];
    v[1] = v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultFAT110011121(float* v)  {
    float a0 = -v[1];
    v[1] = v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultIVT110011121(TVector<int>& v)  {
    int a0 = -v[1];
    v[1] = v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultIAT110011121(int* v)  {
    int a0 = -v[1];
    v[1] = v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
// -Z,+X,-Y
  static inline void MultDV110211101(TVector<double>& v)  {
    double a0 = -v[2];
    double a1 = v[0];
    v[2] = -v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultDA110211101(double* v)  {
    double a0 = -v[2];
    double a1 = v[0];
    v[2] = -v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultFV110211101(TVector<float>& v)  {
    float a0 = -v[2];
    float a1 = v[0];
    v[2] = -v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultFA110211101(float* v)  {
    float a0 = -v[2];
    float a1 = v[0];
    v[2] = -v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultIV110211101(TVector<int>& v)  {
    int a0 = -v[2];
    int a1 = v[0];
    v[2] = -v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultIA110211101(int* v)  {
    int a0 = -v[2];
    int a1 = v[0];
    v[2] = -v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultDVT110211101(TVector<double>& v)  {
    double a0 = v[1];
    v[1] = -v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultDAT110211101(double* v)  {
    double a0 = v[1];
    v[1] = -v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultFVT110211101(TVector<float>& v)  {
    float a0 = v[1];
    v[1] = -v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultFAT110211101(float* v)  {
    float a0 = v[1];
    v[1] = -v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultIVT110211101(TVector<int>& v)  {
    int a0 = v[1];
    v[1] = -v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultIAT110211101(int* v)  {
    int a0 = v[1];
    v[1] = -v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
// +Y,-Z,-X
  static inline void MultDV121110011(TVector<double>& v)  {
    double a0 = v[1];
    v[1] = -v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultDA121110011(double* v)  {
    double a0 = v[1];
    v[1] = -v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultFV121110011(TVector<float>& v)  {
    float a0 = v[1];
    v[1] = -v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultFA121110011(float* v)  {
    float a0 = v[1];
    v[1] = -v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultIV121110011(TVector<int>& v)  {
    int a0 = v[1];
    v[1] = -v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultIA121110011(int* v)  {
    int a0 = v[1];
    v[1] = -v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultDVT121110011(TVector<double>& v)  {
    double a0 = -v[2];
    double a1 = v[0];
    v[2] = -v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultDAT121110011(double* v)  {
    double a0 = -v[2];
    double a1 = v[0];
    v[2] = -v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultFVT121110011(TVector<float>& v)  {
    float a0 = -v[2];
    float a1 = v[0];
    v[2] = -v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultFAT121110011(float* v)  {
    float a0 = -v[2];
    float a1 = v[0];
    v[2] = -v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultIVT121110011(TVector<int>& v)  {
    int a0 = -v[2];
    int a1 = v[0];
    v[2] = -v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultIAT121110011(int* v)  {
    int a0 = -v[2];
    int a1 = v[0];
    v[2] = -v[1];
    v[0] = a0;
    v[1] = a1;
  }
// -Y,-X,+Z
  static inline void MultDV101011112(TVector<double>& v)  {
    double a0 = -v[1];
    v[1] = -v[0];
    v[0] = a0;
  }
  static inline void MultDA101011112(double* v)  {
    double a0 = -v[1];
    v[1] = -v[0];
    v[0] = a0;
  }
  static inline void MultFV101011112(TVector<float>& v)  {
    float a0 = -v[1];
    v[1] = -v[0];
    v[0] = a0;
  }
  static inline void MultFA101011112(float* v)  {
    float a0 = -v[1];
    v[1] = -v[0];
    v[0] = a0;
  }
  static inline void MultIV101011112(TVector<int>& v)  {
    int a0 = -v[1];
    v[1] = -v[0];
    v[0] = a0;
  }
  static inline void MultIA101011112(int* v)  {
    int a0 = -v[1];
    v[1] = -v[0];
    v[0] = a0;
  }
  static inline void MultDVT101011112(TVector<double>& v)  {
    double a0 = -v[1];
    v[1] = -v[0];
    v[0] = a0;
  }
  static inline void MultDAT101011112(double* v)  {
    double a0 = -v[1];
    v[1] = -v[0];
    v[0] = a0;
  }
  static inline void MultFVT101011112(TVector<float>& v)  {
    float a0 = -v[1];
    v[1] = -v[0];
    v[0] = a0;
  }
  static inline void MultFAT101011112(float* v)  {
    float a0 = -v[1];
    v[1] = -v[0];
    v[0] = a0;
  }
  static inline void MultIVT101011112(TVector<int>& v)  {
    int a0 = -v[1];
    v[1] = -v[0];
    v[0] = a0;
  }
  static inline void MultIAT101011112(int* v)  {
    int a0 = -v[1];
    v[1] = -v[0];
    v[0] = a0;
  }
// +Y,+X,+Z
  static inline void MultDV121211112(TVector<double>& v)  {
    double a0 = v[1];
    v[1] = v[0];
    v[0] = a0;
  }
  static inline void MultDA121211112(double* v)  {
    double a0 = v[1];
    v[1] = v[0];
    v[0] = a0;
  }
  static inline void MultFV121211112(TVector<float>& v)  {
    float a0 = v[1];
    v[1] = v[0];
    v[0] = a0;
  }
  static inline void MultFA121211112(float* v)  {
    float a0 = v[1];
    v[1] = v[0];
    v[0] = a0;
  }
  static inline void MultIV121211112(TVector<int>& v)  {
    int a0 = v[1];
    v[1] = v[0];
    v[0] = a0;
  }
  static inline void MultIA121211112(int* v)  {
    int a0 = v[1];
    v[1] = v[0];
    v[0] = a0;
  }
  static inline void MultDVT121211112(TVector<double>& v)  {
    double a0 = v[1];
    v[1] = v[0];
    v[0] = a0;
  }
  static inline void MultDAT121211112(double* v)  {
    double a0 = v[1];
    v[1] = v[0];
    v[0] = a0;
  }
  static inline void MultFVT121211112(TVector<float>& v)  {
    float a0 = v[1];
    v[1] = v[0];
    v[0] = a0;
  }
  static inline void MultFAT121211112(float* v)  {
    float a0 = v[1];
    v[1] = v[0];
    v[0] = a0;
  }
  static inline void MultIVT121211112(TVector<int>& v)  {
    int a0 = v[1];
    v[1] = v[0];
    v[0] = a0;
  }
  static inline void MultIAT121211112(int* v)  {
    int a0 = v[1];
    v[1] = v[0];
    v[0] = a0;
  }
// +X,-Z,-Y
  static inline void MultDV211110101(TVector<double>& v)  {
    double a1 = -v[2];
    v[2] = -v[1];
    v[1] = a1;
  }
  static inline void MultDA211110101(double* v)  {
    double a1 = -v[2];
    v[2] = -v[1];
    v[1] = a1;
  }
  static inline void MultFV211110101(TVector<float>& v)  {
    float a1 = -v[2];
    v[2] = -v[1];
    v[1] = a1;
  }
  static inline void MultFA211110101(float* v)  {
    float a1 = -v[2];
    v[2] = -v[1];
    v[1] = a1;
  }
  static inline void MultIV211110101(TVector<int>& v)  {
    int a1 = -v[2];
    v[2] = -v[1];
    v[1] = a1;
  }
  static inline void MultIA211110101(int* v)  {
    int a1 = -v[2];
    v[2] = -v[1];
    v[1] = a1;
  }
  static inline void MultDVT211110101(TVector<double>& v)  {
    double a1 = -v[2];
    v[2] = -v[1];
    v[1] = a1;
  }
  static inline void MultDAT211110101(double* v)  {
    double a1 = -v[2];
    v[2] = -v[1];
    v[1] = a1;
  }
  static inline void MultFVT211110101(TVector<float>& v)  {
    float a1 = -v[2];
    v[2] = -v[1];
    v[1] = a1;
  }
  static inline void MultFAT211110101(float* v)  {
    float a1 = -v[2];
    v[2] = -v[1];
    v[1] = a1;
  }
  static inline void MultIVT211110101(TVector<int>& v)  {
    int a1 = -v[2];
    v[2] = -v[1];
    v[1] = a1;
  }
  static inline void MultIAT211110101(int* v)  {
    int a1 = -v[2];
    v[2] = -v[1];
    v[1] = a1;
  }
// +X,+Z,+Y
  static inline void MultDV211112121(TVector<double>& v)  {
    double a1 = v[2];
    v[2] = v[1];
    v[1] = a1;
  }
  static inline void MultDA211112121(double* v)  {
    double a1 = v[2];
    v[2] = v[1];
    v[1] = a1;
  }
  static inline void MultFV211112121(TVector<float>& v)  {
    float a1 = v[2];
    v[2] = v[1];
    v[1] = a1;
  }
  static inline void MultFA211112121(float* v)  {
    float a1 = v[2];
    v[2] = v[1];
    v[1] = a1;
  }
  static inline void MultIV211112121(TVector<int>& v)  {
    int a1 = v[2];
    v[2] = v[1];
    v[1] = a1;
  }
  static inline void MultIA211112121(int* v)  {
    int a1 = v[2];
    v[2] = v[1];
    v[1] = a1;
  }
  static inline void MultDVT211112121(TVector<double>& v)  {
    double a1 = v[2];
    v[2] = v[1];
    v[1] = a1;
  }
  static inline void MultDAT211112121(double* v)  {
    double a1 = v[2];
    v[2] = v[1];
    v[1] = a1;
  }
  static inline void MultFVT211112121(TVector<float>& v)  {
    float a1 = v[2];
    v[2] = v[1];
    v[1] = a1;
  }
  static inline void MultFAT211112121(float* v)  {
    float a1 = v[2];
    v[2] = v[1];
    v[1] = a1;
  }
  static inline void MultIVT211112121(TVector<int>& v)  {
    int a1 = v[2];
    v[2] = v[1];
    v[1] = a1;
  }
  static inline void MultIAT211112121(int* v)  {
    int a1 = v[2];
    v[2] = v[1];
    v[1] = a1;
  }
// -Z,+Y,-X
  static inline void MultDV110121011(TVector<double>& v)  {
    double a0 = -v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultDA110121011(double* v)  {
    double a0 = -v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultFV110121011(TVector<float>& v)  {
    float a0 = -v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultFA110121011(float* v)  {
    float a0 = -v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultIV110121011(TVector<int>& v)  {
    int a0 = -v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultIA110121011(int* v)  {
    int a0 = -v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultDVT110121011(TVector<double>& v)  {
    double a0 = -v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultDAT110121011(double* v)  {
    double a0 = -v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultFVT110121011(TVector<float>& v)  {
    float a0 = -v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultFAT110121011(float* v)  {
    float a0 = -v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultIVT110121011(TVector<int>& v)  {
    int a0 = -v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultIAT110121011(int* v)  {
    int a0 = -v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
// +Z,+Y,+X
  static inline void MultDV112121211(TVector<double>& v)  {
    double a0 = v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultDA112121211(double* v)  {
    double a0 = v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultFV112121211(TVector<float>& v)  {
    float a0 = v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultFA112121211(float* v)  {
    float a0 = v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultIV112121211(TVector<int>& v)  {
    int a0 = v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultIA112121211(int* v)  {
    int a0 = v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultDVT112121211(TVector<double>& v)  {
    double a0 = v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultDAT112121211(double* v)  {
    double a0 = v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultFVT112121211(TVector<float>& v)  {
    float a0 = v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultFAT112121211(float* v)  {
    float a0 = v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultIVT112121211(TVector<int>& v)  {
    int a0 = v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultIAT112121211(int* v)  {
    int a0 = v[2];
    v[2] = v[0];
    v[0] = a0;
  }
// -Y,+X,+Z
  static inline void MultDV101211112(TVector<double>& v)  {
    double a0 = -v[1];
    v[1] = v[0];
    v[0] = a0;
  }
  static inline void MultDA101211112(double* v)  {
    double a0 = -v[1];
    v[1] = v[0];
    v[0] = a0;
  }
  static inline void MultFV101211112(TVector<float>& v)  {
    float a0 = -v[1];
    v[1] = v[0];
    v[0] = a0;
  }
  static inline void MultFA101211112(float* v)  {
    float a0 = -v[1];
    v[1] = v[0];
    v[0] = a0;
  }
  static inline void MultIV101211112(TVector<int>& v)  {
    int a0 = -v[1];
    v[1] = v[0];
    v[0] = a0;
  }
  static inline void MultIA101211112(int* v)  {
    int a0 = -v[1];
    v[1] = v[0];
    v[0] = a0;
  }
  static inline void MultDVT101211112(TVector<double>& v)  {
    double a0 = v[1];
    v[1] = -v[0];
    v[0] = a0;
  }
  static inline void MultDAT101211112(double* v)  {
    double a0 = v[1];
    v[1] = -v[0];
    v[0] = a0;
  }
  static inline void MultFVT101211112(TVector<float>& v)  {
    float a0 = v[1];
    v[1] = -v[0];
    v[0] = a0;
  }
  static inline void MultFAT101211112(float* v)  {
    float a0 = v[1];
    v[1] = -v[0];
    v[0] = a0;
  }
  static inline void MultIVT101211112(TVector<int>& v)  {
    int a0 = v[1];
    v[1] = -v[0];
    v[0] = a0;
  }
  static inline void MultIAT101211112(int* v)  {
    int a0 = v[1];
    v[1] = -v[0];
    v[0] = a0;
  }
// +Y,-X,+Z
  static inline void MultDV121011112(TVector<double>& v)  {
    double a0 = v[1];
    v[1] = -v[0];
    v[0] = a0;
  }
  static inline void MultDA121011112(double* v)  {
    double a0 = v[1];
    v[1] = -v[0];
    v[0] = a0;
  }
  static inline void MultFV121011112(TVector<float>& v)  {
    float a0 = v[1];
    v[1] = -v[0];
    v[0] = a0;
  }
  static inline void MultFA121011112(float* v)  {
    float a0 = v[1];
    v[1] = -v[0];
    v[0] = a0;
  }
  static inline void MultIV121011112(TVector<int>& v)  {
    int a0 = v[1];
    v[1] = -v[0];
    v[0] = a0;
  }
  static inline void MultIA121011112(int* v)  {
    int a0 = v[1];
    v[1] = -v[0];
    v[0] = a0;
  }
  static inline void MultDVT121011112(TVector<double>& v)  {
    double a0 = -v[1];
    v[1] = v[0];
    v[0] = a0;
  }
  static inline void MultDAT121011112(double* v)  {
    double a0 = -v[1];
    v[1] = v[0];
    v[0] = a0;
  }
  static inline void MultFVT121011112(TVector<float>& v)  {
    float a0 = -v[1];
    v[1] = v[0];
    v[0] = a0;
  }
  static inline void MultFAT121011112(float* v)  {
    float a0 = -v[1];
    v[1] = v[0];
    v[0] = a0;
  }
  static inline void MultIVT121011112(TVector<int>& v)  {
    int a0 = -v[1];
    v[1] = v[0];
    v[0] = a0;
  }
  static inline void MultIAT121011112(int* v)  {
    int a0 = -v[1];
    v[1] = v[0];
    v[0] = a0;
  }
// +X,-Z,+Y
  static inline void MultDV211110121(TVector<double>& v)  {
    double a1 = -v[2];
    v[2] = v[1];
    v[1] = a1;
  }
  static inline void MultDA211110121(double* v)  {
    double a1 = -v[2];
    v[2] = v[1];
    v[1] = a1;
  }
  static inline void MultFV211110121(TVector<float>& v)  {
    float a1 = -v[2];
    v[2] = v[1];
    v[1] = a1;
  }
  static inline void MultFA211110121(float* v)  {
    float a1 = -v[2];
    v[2] = v[1];
    v[1] = a1;
  }
  static inline void MultIV211110121(TVector<int>& v)  {
    int a1 = -v[2];
    v[2] = v[1];
    v[1] = a1;
  }
  static inline void MultIA211110121(int* v)  {
    int a1 = -v[2];
    v[2] = v[1];
    v[1] = a1;
  }
  static inline void MultDVT211110121(TVector<double>& v)  {
    double a1 = v[2];
    v[2] = -v[1];
    v[1] = a1;
  }
  static inline void MultDAT211110121(double* v)  {
    double a1 = v[2];
    v[2] = -v[1];
    v[1] = a1;
  }
  static inline void MultFVT211110121(TVector<float>& v)  {
    float a1 = v[2];
    v[2] = -v[1];
    v[1] = a1;
  }
  static inline void MultFAT211110121(float* v)  {
    float a1 = v[2];
    v[2] = -v[1];
    v[1] = a1;
  }
  static inline void MultIVT211110121(TVector<int>& v)  {
    int a1 = v[2];
    v[2] = -v[1];
    v[1] = a1;
  }
  static inline void MultIAT211110121(int* v)  {
    int a1 = v[2];
    v[2] = -v[1];
    v[1] = a1;
  }
// +X,+Z,-Y
  static inline void MultDV211112101(TVector<double>& v)  {
    double a1 = v[2];
    v[2] = -v[1];
    v[1] = a1;
  }
  static inline void MultDA211112101(double* v)  {
    double a1 = v[2];
    v[2] = -v[1];
    v[1] = a1;
  }
  static inline void MultFV211112101(TVector<float>& v)  {
    float a1 = v[2];
    v[2] = -v[1];
    v[1] = a1;
  }
  static inline void MultFA211112101(float* v)  {
    float a1 = v[2];
    v[2] = -v[1];
    v[1] = a1;
  }
  static inline void MultIV211112101(TVector<int>& v)  {
    int a1 = v[2];
    v[2] = -v[1];
    v[1] = a1;
  }
  static inline void MultIA211112101(int* v)  {
    int a1 = v[2];
    v[2] = -v[1];
    v[1] = a1;
  }
  static inline void MultDVT211112101(TVector<double>& v)  {
    double a1 = -v[2];
    v[2] = v[1];
    v[1] = a1;
  }
  static inline void MultDAT211112101(double* v)  {
    double a1 = -v[2];
    v[2] = v[1];
    v[1] = a1;
  }
  static inline void MultFVT211112101(TVector<float>& v)  {
    float a1 = -v[2];
    v[2] = v[1];
    v[1] = a1;
  }
  static inline void MultFAT211112101(float* v)  {
    float a1 = -v[2];
    v[2] = v[1];
    v[1] = a1;
  }
  static inline void MultIVT211112101(TVector<int>& v)  {
    int a1 = -v[2];
    v[2] = v[1];
    v[1] = a1;
  }
  static inline void MultIAT211112101(int* v)  {
    int a1 = -v[2];
    v[2] = v[1];
    v[1] = a1;
  }
// +Z,+Y,-X
  static inline void MultDV112121011(TVector<double>& v)  {
    double a0 = v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultDA112121011(double* v)  {
    double a0 = v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultFV112121011(TVector<float>& v)  {
    float a0 = v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultFA112121011(float* v)  {
    float a0 = v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultIV112121011(TVector<int>& v)  {
    int a0 = v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultIA112121011(int* v)  {
    int a0 = v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultDVT112121011(TVector<double>& v)  {
    double a0 = -v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultDAT112121011(double* v)  {
    double a0 = -v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultFVT112121011(TVector<float>& v)  {
    float a0 = -v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultFAT112121011(float* v)  {
    float a0 = -v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultIVT112121011(TVector<int>& v)  {
    int a0 = -v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultIAT112121011(int* v)  {
    int a0 = -v[2];
    v[2] = v[0];
    v[0] = a0;
  }
// -Z,+Y,+X
  static inline void MultDV110121211(TVector<double>& v)  {
    double a0 = -v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultDA110121211(double* v)  {
    double a0 = -v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultFV110121211(TVector<float>& v)  {
    float a0 = -v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultFA110121211(float* v)  {
    float a0 = -v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultIV110121211(TVector<int>& v)  {
    int a0 = -v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultIA110121211(int* v)  {
    int a0 = -v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultDVT110121211(TVector<double>& v)  {
    double a0 = v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultDAT110121211(double* v)  {
    double a0 = v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultFVT110121211(TVector<float>& v)  {
    float a0 = v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultFAT110121211(float* v)  {
    float a0 = v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultIVT110121211(TVector<int>& v)  {
    int a0 = v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultIAT110121211(int* v)  {
    int a0 = v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
// +Y,+X,-Z
  static inline void MultDV121211110(TVector<double>& v)  {
    double a0 = v[1];
    v[1] = v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultDA121211110(double* v)  {
    double a0 = v[1];
    v[1] = v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultFV121211110(TVector<float>& v)  {
    float a0 = v[1];
    v[1] = v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultFA121211110(float* v)  {
    float a0 = v[1];
    v[1] = v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultIV121211110(TVector<int>& v)  {
    int a0 = v[1];
    v[1] = v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultIA121211110(int* v)  {
    int a0 = v[1];
    v[1] = v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultDVT121211110(TVector<double>& v)  {
    double a0 = v[1];
    v[1] = v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultDAT121211110(double* v)  {
    double a0 = v[1];
    v[1] = v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultFVT121211110(TVector<float>& v)  {
    float a0 = v[1];
    v[1] = v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultFAT121211110(float* v)  {
    float a0 = v[1];
    v[1] = v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultIVT121211110(TVector<int>& v)  {
    int a0 = v[1];
    v[1] = v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultIAT121211110(int* v)  {
    int a0 = v[1];
    v[1] = v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
// -Y,-X,-Z
  static inline void MultDV101011110(TVector<double>& v)  {
    double a0 = -v[1];
    v[1] = -v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultDA101011110(double* v)  {
    double a0 = -v[1];
    v[1] = -v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultFV101011110(TVector<float>& v)  {
    float a0 = -v[1];
    v[1] = -v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultFA101011110(float* v)  {
    float a0 = -v[1];
    v[1] = -v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultIV101011110(TVector<int>& v)  {
    int a0 = -v[1];
    v[1] = -v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultIA101011110(int* v)  {
    int a0 = -v[1];
    v[1] = -v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultDVT101011110(TVector<double>& v)  {
    double a0 = -v[1];
    v[1] = -v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultDAT101011110(double* v)  {
    double a0 = -v[1];
    v[1] = -v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultFVT101011110(TVector<float>& v)  {
    float a0 = -v[1];
    v[1] = -v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultFAT101011110(float* v)  {
    float a0 = -v[1];
    v[1] = -v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultIVT101011110(TVector<int>& v)  {
    int a0 = -v[1];
    v[1] = -v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultIAT101011110(int* v)  {
    int a0 = -v[1];
    v[1] = -v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
// -X,+Z,+Y
  static inline void MultDV011112121(TVector<double>& v)  {
    v[0] = -v[0];
    double a1 = v[2];
    v[2] = v[1];
    v[1] = a1;
  }
  static inline void MultDA011112121(double* v)  {
    v[0] = -v[0];
    double a1 = v[2];
    v[2] = v[1];
    v[1] = a1;
  }
  static inline void MultFV011112121(TVector<float>& v)  {
    v[0] = -v[0];
    float a1 = v[2];
    v[2] = v[1];
    v[1] = a1;
  }
  static inline void MultFA011112121(float* v)  {
    v[0] = -v[0];
    float a1 = v[2];
    v[2] = v[1];
    v[1] = a1;
  }
  static inline void MultIV011112121(TVector<int>& v)  {
    v[0] = -v[0];
    int a1 = v[2];
    v[2] = v[1];
    v[1] = a1;
  }
  static inline void MultIA011112121(int* v)  {
    v[0] = -v[0];
    int a1 = v[2];
    v[2] = v[1];
    v[1] = a1;
  }
  static inline void MultDVT011112121(TVector<double>& v)  {
    v[0] = -v[0];
    double a1 = v[2];
    v[2] = v[1];
    v[1] = a1;
  }
  static inline void MultDAT011112121(double* v)  {
    v[0] = -v[0];
    double a1 = v[2];
    v[2] = v[1];
    v[1] = a1;
  }
  static inline void MultFVT011112121(TVector<float>& v)  {
    v[0] = -v[0];
    float a1 = v[2];
    v[2] = v[1];
    v[1] = a1;
  }
  static inline void MultFAT011112121(float* v)  {
    v[0] = -v[0];
    float a1 = v[2];
    v[2] = v[1];
    v[1] = a1;
  }
  static inline void MultIVT011112121(TVector<int>& v)  {
    v[0] = -v[0];
    int a1 = v[2];
    v[2] = v[1];
    v[1] = a1;
  }
  static inline void MultIAT011112121(int* v)  {
    v[0] = -v[0];
    int a1 = v[2];
    v[2] = v[1];
    v[1] = a1;
  }
// -X,-Z,-Y
  static inline void MultDV011110101(TVector<double>& v)  {
    v[0] = -v[0];
    double a1 = -v[2];
    v[2] = -v[1];
    v[1] = a1;
  }
  static inline void MultDA011110101(double* v)  {
    v[0] = -v[0];
    double a1 = -v[2];
    v[2] = -v[1];
    v[1] = a1;
  }
  static inline void MultFV011110101(TVector<float>& v)  {
    v[0] = -v[0];
    float a1 = -v[2];
    v[2] = -v[1];
    v[1] = a1;
  }
  static inline void MultFA011110101(float* v)  {
    v[0] = -v[0];
    float a1 = -v[2];
    v[2] = -v[1];
    v[1] = a1;
  }
  static inline void MultIV011110101(TVector<int>& v)  {
    v[0] = -v[0];
    int a1 = -v[2];
    v[2] = -v[1];
    v[1] = a1;
  }
  static inline void MultIA011110101(int* v)  {
    v[0] = -v[0];
    int a1 = -v[2];
    v[2] = -v[1];
    v[1] = a1;
  }
  static inline void MultDVT011110101(TVector<double>& v)  {
    v[0] = -v[0];
    double a1 = -v[2];
    v[2] = -v[1];
    v[1] = a1;
  }
  static inline void MultDAT011110101(double* v)  {
    v[0] = -v[0];
    double a1 = -v[2];
    v[2] = -v[1];
    v[1] = a1;
  }
  static inline void MultFVT011110101(TVector<float>& v)  {
    v[0] = -v[0];
    float a1 = -v[2];
    v[2] = -v[1];
    v[1] = a1;
  }
  static inline void MultFAT011110101(float* v)  {
    v[0] = -v[0];
    float a1 = -v[2];
    v[2] = -v[1];
    v[1] = a1;
  }
  static inline void MultIVT011110101(TVector<int>& v)  {
    v[0] = -v[0];
    int a1 = -v[2];
    v[2] = -v[1];
    v[1] = a1;
  }
  static inline void MultIAT011110101(int* v)  {
    v[0] = -v[0];
    int a1 = -v[2];
    v[2] = -v[1];
    v[1] = a1;
  }
// +Z,-Y,+X
  static inline void MultDV112101211(TVector<double>& v)  {
    double a0 = v[2];
    v[1] = -v[1];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultDA112101211(double* v)  {
    double a0 = v[2];
    v[1] = -v[1];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultFV112101211(TVector<float>& v)  {
    float a0 = v[2];
    v[1] = -v[1];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultFA112101211(float* v)  {
    float a0 = v[2];
    v[1] = -v[1];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultIV112101211(TVector<int>& v)  {
    int a0 = v[2];
    v[1] = -v[1];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultIA112101211(int* v)  {
    int a0 = v[2];
    v[1] = -v[1];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultDVT112101211(TVector<double>& v)  {
    double a0 = v[2];
    v[1] = -v[1];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultDAT112101211(double* v)  {
    double a0 = v[2];
    v[1] = -v[1];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultFVT112101211(TVector<float>& v)  {
    float a0 = v[2];
    v[1] = -v[1];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultFAT112101211(float* v)  {
    float a0 = v[2];
    v[1] = -v[1];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultIVT112101211(TVector<int>& v)  {
    int a0 = v[2];
    v[1] = -v[1];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultIAT112101211(int* v)  {
    int a0 = v[2];
    v[1] = -v[1];
    v[2] = v[0];
    v[0] = a0;
  }
// -Z,-Y,-X
  static inline void MultDV110101011(TVector<double>& v)  {
    double a0 = -v[2];
    v[1] = -v[1];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultDA110101011(double* v)  {
    double a0 = -v[2];
    v[1] = -v[1];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultFV110101011(TVector<float>& v)  {
    float a0 = -v[2];
    v[1] = -v[1];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultFA110101011(float* v)  {
    float a0 = -v[2];
    v[1] = -v[1];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultIV110101011(TVector<int>& v)  {
    int a0 = -v[2];
    v[1] = -v[1];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultIA110101011(int* v)  {
    int a0 = -v[2];
    v[1] = -v[1];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultDVT110101011(TVector<double>& v)  {
    double a0 = -v[2];
    v[1] = -v[1];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultDAT110101011(double* v)  {
    double a0 = -v[2];
    v[1] = -v[1];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultFVT110101011(TVector<float>& v)  {
    float a0 = -v[2];
    v[1] = -v[1];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultFAT110101011(float* v)  {
    float a0 = -v[2];
    v[1] = -v[1];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultIVT110101011(TVector<int>& v)  {
    int a0 = -v[2];
    v[1] = -v[1];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultIAT110101011(int* v)  {
    int a0 = -v[2];
    v[1] = -v[1];
    v[2] = -v[0];
    v[0] = a0;
  }
// -Z,-X,-Y
  static inline void MultDV110011101(TVector<double>& v)  {
    double a0 = -v[2];
    double a1 = -v[0];
    v[2] = -v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultDA110011101(double* v)  {
    double a0 = -v[2];
    double a1 = -v[0];
    v[2] = -v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultFV110011101(TVector<float>& v)  {
    float a0 = -v[2];
    float a1 = -v[0];
    v[2] = -v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultFA110011101(float* v)  {
    float a0 = -v[2];
    float a1 = -v[0];
    v[2] = -v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultIV110011101(TVector<int>& v)  {
    int a0 = -v[2];
    int a1 = -v[0];
    v[2] = -v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultIA110011101(int* v)  {
    int a0 = -v[2];
    int a1 = -v[0];
    v[2] = -v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultDVT110011101(TVector<double>& v)  {
    double a0 = -v[1];
    v[1] = -v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultDAT110011101(double* v)  {
    double a0 = -v[1];
    v[1] = -v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultFVT110011101(TVector<float>& v)  {
    float a0 = -v[1];
    v[1] = -v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultFAT110011101(float* v)  {
    float a0 = -v[1];
    v[1] = -v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultIVT110011101(TVector<int>& v)  {
    int a0 = -v[1];
    v[1] = -v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultIAT110011101(int* v)  {
    int a0 = -v[1];
    v[1] = -v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
// -Y,-Z,-X
  static inline void MultDV101110011(TVector<double>& v)  {
    double a0 = -v[1];
    v[1] = -v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultDA101110011(double* v)  {
    double a0 = -v[1];
    v[1] = -v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultFV101110011(TVector<float>& v)  {
    float a0 = -v[1];
    v[1] = -v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultFA101110011(float* v)  {
    float a0 = -v[1];
    v[1] = -v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultIV101110011(TVector<int>& v)  {
    int a0 = -v[1];
    v[1] = -v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultIA101110011(int* v)  {
    int a0 = -v[1];
    v[1] = -v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultDVT101110011(TVector<double>& v)  {
    double a0 = -v[2];
    double a1 = -v[0];
    v[2] = -v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultDAT101110011(double* v)  {
    double a0 = -v[2];
    double a1 = -v[0];
    v[2] = -v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultFVT101110011(TVector<float>& v)  {
    float a0 = -v[2];
    float a1 = -v[0];
    v[2] = -v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultFAT101110011(float* v)  {
    float a0 = -v[2];
    float a1 = -v[0];
    v[2] = -v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultIVT101110011(TVector<int>& v)  {
    int a0 = -v[2];
    int a1 = -v[0];
    v[2] = -v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultIAT101110011(int* v)  {
    int a0 = -v[2];
    int a1 = -v[0];
    v[2] = -v[1];
    v[0] = a0;
    v[1] = a1;
  }
// +Y,+Z,-X
  static inline void MultDV121112011(TVector<double>& v)  {
    double a0 = v[1];
    v[1] = v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultDA121112011(double* v)  {
    double a0 = v[1];
    v[1] = v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultFV121112011(TVector<float>& v)  {
    float a0 = v[1];
    v[1] = v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultFA121112011(float* v)  {
    float a0 = v[1];
    v[1] = v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultIV121112011(TVector<int>& v)  {
    int a0 = v[1];
    v[1] = v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultIA121112011(int* v)  {
    int a0 = v[1];
    v[1] = v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultDVT121112011(TVector<double>& v)  {
    double a0 = -v[2];
    double a1 = v[0];
    v[2] = v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultDAT121112011(double* v)  {
    double a0 = -v[2];
    double a1 = v[0];
    v[2] = v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultFVT121112011(TVector<float>& v)  {
    float a0 = -v[2];
    float a1 = v[0];
    v[2] = v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultFAT121112011(float* v)  {
    float a0 = -v[2];
    float a1 = v[0];
    v[2] = v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultIVT121112011(TVector<int>& v)  {
    int a0 = -v[2];
    int a1 = v[0];
    v[2] = v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultIAT121112011(int* v)  {
    int a0 = -v[2];
    int a1 = v[0];
    v[2] = v[1];
    v[0] = a0;
    v[1] = a1;
  }
// -Z,+X,+Y
  static inline void MultDV110211121(TVector<double>& v)  {
    double a0 = -v[2];
    double a1 = v[0];
    v[2] = v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultDA110211121(double* v)  {
    double a0 = -v[2];
    double a1 = v[0];
    v[2] = v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultFV110211121(TVector<float>& v)  {
    float a0 = -v[2];
    float a1 = v[0];
    v[2] = v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultFA110211121(float* v)  {
    float a0 = -v[2];
    float a1 = v[0];
    v[2] = v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultIV110211121(TVector<int>& v)  {
    int a0 = -v[2];
    int a1 = v[0];
    v[2] = v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultIA110211121(int* v)  {
    int a0 = -v[2];
    int a1 = v[0];
    v[2] = v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultDVT110211121(TVector<double>& v)  {
    double a0 = v[1];
    v[1] = v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultDAT110211121(double* v)  {
    double a0 = v[1];
    v[1] = v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultFVT110211121(TVector<float>& v)  {
    float a0 = v[1];
    v[1] = v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultFAT110211121(float* v)  {
    float a0 = v[1];
    v[1] = v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultIVT110211121(TVector<int>& v)  {
    int a0 = v[1];
    v[1] = v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
  static inline void MultIAT110211121(int* v)  {
    int a0 = v[1];
    v[1] = v[2];
    v[2] = -v[0];
    v[0] = a0;
  }
// +Y,-Z,+X
  static inline void MultDV121110211(TVector<double>& v)  {
    double a0 = v[1];
    v[1] = -v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultDA121110211(double* v)  {
    double a0 = v[1];
    v[1] = -v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultFV121110211(TVector<float>& v)  {
    float a0 = v[1];
    v[1] = -v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultFA121110211(float* v)  {
    float a0 = v[1];
    v[1] = -v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultIV121110211(TVector<int>& v)  {
    int a0 = v[1];
    v[1] = -v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultIA121110211(int* v)  {
    int a0 = v[1];
    v[1] = -v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultDVT121110211(TVector<double>& v)  {
    double a0 = v[2];
    double a1 = v[0];
    v[2] = -v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultDAT121110211(double* v)  {
    double a0 = v[2];
    double a1 = v[0];
    v[2] = -v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultFVT121110211(TVector<float>& v)  {
    float a0 = v[2];
    float a1 = v[0];
    v[2] = -v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultFAT121110211(float* v)  {
    float a0 = v[2];
    float a1 = v[0];
    v[2] = -v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultIVT121110211(TVector<int>& v)  {
    int a0 = v[2];
    int a1 = v[0];
    v[2] = -v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultIAT121110211(int* v)  {
    int a0 = v[2];
    int a1 = v[0];
    v[2] = -v[1];
    v[0] = a0;
    v[1] = a1;
  }
// +Z,+X,-Y
  static inline void MultDV112211101(TVector<double>& v)  {
    double a0 = v[2];
    double a1 = v[0];
    v[2] = -v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultDA112211101(double* v)  {
    double a0 = v[2];
    double a1 = v[0];
    v[2] = -v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultFV112211101(TVector<float>& v)  {
    float a0 = v[2];
    float a1 = v[0];
    v[2] = -v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultFA112211101(float* v)  {
    float a0 = v[2];
    float a1 = v[0];
    v[2] = -v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultIV112211101(TVector<int>& v)  {
    int a0 = v[2];
    int a1 = v[0];
    v[2] = -v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultIA112211101(int* v)  {
    int a0 = v[2];
    int a1 = v[0];
    v[2] = -v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultDVT112211101(TVector<double>& v)  {
    double a0 = v[1];
    v[1] = -v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultDAT112211101(double* v)  {
    double a0 = v[1];
    v[1] = -v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultFVT112211101(TVector<float>& v)  {
    float a0 = v[1];
    v[1] = -v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultFAT112211101(float* v)  {
    float a0 = v[1];
    v[1] = -v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultIVT112211101(TVector<int>& v)  {
    int a0 = v[1];
    v[1] = -v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultIAT112211101(int* v)  {
    int a0 = v[1];
    v[1] = -v[2];
    v[2] = v[0];
    v[0] = a0;
  }
// +Z,-X,+Y
  static inline void MultDV112011121(TVector<double>& v)  {
    double a0 = v[2];
    double a1 = -v[0];
    v[2] = v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultDA112011121(double* v)  {
    double a0 = v[2];
    double a1 = -v[0];
    v[2] = v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultFV112011121(TVector<float>& v)  {
    float a0 = v[2];
    float a1 = -v[0];
    v[2] = v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultFA112011121(float* v)  {
    float a0 = v[2];
    float a1 = -v[0];
    v[2] = v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultIV112011121(TVector<int>& v)  {
    int a0 = v[2];
    int a1 = -v[0];
    v[2] = v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultIA112011121(int* v)  {
    int a0 = v[2];
    int a1 = -v[0];
    v[2] = v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultDVT112011121(TVector<double>& v)  {
    double a0 = -v[1];
    v[1] = v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultDAT112011121(double* v)  {
    double a0 = -v[1];
    v[1] = v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultFVT112011121(TVector<float>& v)  {
    float a0 = -v[1];
    v[1] = v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultFAT112011121(float* v)  {
    float a0 = -v[1];
    v[1] = v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultIVT112011121(TVector<int>& v)  {
    int a0 = -v[1];
    v[1] = v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultIAT112011121(int* v)  {
    int a0 = -v[1];
    v[1] = v[2];
    v[2] = v[0];
    v[0] = a0;
  }
// -Y,+Z,+X
  static inline void MultDV101112211(TVector<double>& v)  {
    double a0 = -v[1];
    v[1] = v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultDA101112211(double* v)  {
    double a0 = -v[1];
    v[1] = v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultFV101112211(TVector<float>& v)  {
    float a0 = -v[1];
    v[1] = v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultFA101112211(float* v)  {
    float a0 = -v[1];
    v[1] = v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultIV101112211(TVector<int>& v)  {
    int a0 = -v[1];
    v[1] = v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultIA101112211(int* v)  {
    int a0 = -v[1];
    v[1] = v[2];
    v[2] = v[0];
    v[0] = a0;
  }
  static inline void MultDVT101112211(TVector<double>& v)  {
    double a0 = v[2];
    double a1 = -v[0];
    v[2] = v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultDAT101112211(double* v)  {
    double a0 = v[2];
    double a1 = -v[0];
    v[2] = v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultFVT101112211(TVector<float>& v)  {
    float a0 = v[2];
    float a1 = -v[0];
    v[2] = v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultFAT101112211(float* v)  {
    float a0 = v[2];
    float a1 = -v[0];
    v[2] = v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultIVT101112211(TVector<int>& v)  {
    int a0 = v[2];
    int a1 = -v[0];
    v[2] = v[1];
    v[0] = a0;
    v[1] = a1;
  }
  static inline void MultIAT101112211(int* v)  {
    int a0 = v[2];
    int a1 = -v[0];
    v[2] = v[1];
    v[0] = a0;
    v[1] = a1;
  }
// -Y,+X-Y,+Z
  static inline void MultDV101201112(TVector<double>& v)  {
    double a0 = -v[1];
    v[1] = v[0]-v[1];
    v[0] = a0;
  }
  static inline void MultDA101201112(double* v)  {
    double a0 = -v[1];
    v[1] = v[0]-v[1];
    v[0] = a0;
  }
  static inline void MultFV101201112(TVector<float>& v)  {
    float a0 = -v[1];
    v[1] = v[0]-v[1];
    v[0] = a0;
  }
  static inline void MultFA101201112(float* v)  {
    float a0 = -v[1];
    v[1] = v[0]-v[1];
    v[0] = a0;
  }
  static inline void MultIV101201112(TVector<int>& v)  {
    int a0 = -v[1];
    v[1] = v[0]-v[1];
    v[0] = a0;
  }
  static inline void MultIA101201112(int* v)  {
    int a0 = -v[1];
    v[1] = v[0]-v[1];
    v[0] = a0;
  }
  static inline void MultDVT101201112(TVector<double>& v)  {
    double a0 = v[1];
    v[1] = -v[0]-v[1];
    v[0] = a0;
  }
  static inline void MultDAT101201112(double* v)  {
    double a0 = v[1];
    v[1] = -v[0]-v[1];
    v[0] = a0;
  }
  static inline void MultFVT101201112(TVector<float>& v)  {
    float a0 = v[1];
    v[1] = -v[0]-v[1];
    v[0] = a0;
  }
  static inline void MultFAT101201112(float* v)  {
    float a0 = v[1];
    v[1] = -v[0]-v[1];
    v[0] = a0;
  }
  static inline void MultIVT101201112(TVector<int>& v)  {
    int a0 = v[1];
    v[1] = -v[0]-v[1];
    v[0] = a0;
  }
  static inline void MultIAT101201112(int* v)  {
    int a0 = v[1];
    v[1] = -v[0]-v[1];
    v[0] = a0;
  }
// +Y,-X+Y,-Z
  static inline void MultDV121021110(TVector<double>& v)  {
    double a0 = v[1];
    v[1] = -v[0]+v[1];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultDA121021110(double* v)  {
    double a0 = v[1];
    v[1] = -v[0]+v[1];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultFV121021110(TVector<float>& v)  {
    float a0 = v[1];
    v[1] = -v[0]+v[1];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultFA121021110(float* v)  {
    float a0 = v[1];
    v[1] = -v[0]+v[1];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultIV121021110(TVector<int>& v)  {
    int a0 = v[1];
    v[1] = -v[0]+v[1];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultIA121021110(int* v)  {
    int a0 = v[1];
    v[1] = -v[0]+v[1];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultDVT121021110(TVector<double>& v)  {
    double a0 = -v[1];
    v[1] = v[0]+v[1];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultDAT121021110(double* v)  {
    double a0 = -v[1];
    v[1] = v[0]+v[1];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultFVT121021110(TVector<float>& v)  {
    float a0 = -v[1];
    v[1] = v[0]+v[1];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultFAT121021110(float* v)  {
    float a0 = -v[1];
    v[1] = v[0]+v[1];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultIVT121021110(TVector<int>& v)  {
    int a0 = -v[1];
    v[1] = v[0]+v[1];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultIAT121021110(int* v)  {
    int a0 = -v[1];
    v[1] = v[0]+v[1];
    v[2] = -v[2];
    v[0] = a0;
  }
// +Y-X,-X,+Z
  static inline void MultDV021011112(TVector<double>& v)  {
    double a0 = -v[0]+v[1];
    v[1] = -v[0];
    v[0] = a0;
  }
  static inline void MultDA021011112(double* v)  {
    double a0 = -v[0]+v[1];
    v[1] = -v[0];
    v[0] = a0;
  }
  static inline void MultFV021011112(TVector<float>& v)  {
    float a0 = -v[0]+v[1];
    v[1] = -v[0];
    v[0] = a0;
  }
  static inline void MultFA021011112(float* v)  {
    float a0 = -v[0]+v[1];
    v[1] = -v[0];
    v[0] = a0;
  }
  static inline void MultIV021011112(TVector<int>& v)  {
    int a0 = -v[0]+v[1];
    v[1] = -v[0];
    v[0] = a0;
  }
  static inline void MultIA021011112(int* v)  {
    int a0 = -v[0]+v[1];
    v[1] = -v[0];
    v[0] = a0;
  }
  static inline void MultDVT021011112(TVector<double>& v)  {
    double a0 = -v[0]-v[1];
    v[1] = v[0];
    v[0] = a0;
  }
  static inline void MultDAT021011112(double* v)  {
    double a0 = -v[0]-v[1];
    v[1] = v[0];
    v[0] = a0;
  }
  static inline void MultFVT021011112(TVector<float>& v)  {
    float a0 = -v[0]-v[1];
    v[1] = v[0];
    v[0] = a0;
  }
  static inline void MultFAT021011112(float* v)  {
    float a0 = -v[0]-v[1];
    v[1] = v[0];
    v[0] = a0;
  }
  static inline void MultIVT021011112(TVector<int>& v)  {
    int a0 = -v[0]-v[1];
    v[1] = v[0];
    v[0] = a0;
  }
  static inline void MultIAT021011112(int* v)  {
    int a0 = -v[0]-v[1];
    v[1] = v[0];
    v[0] = a0;
  }
// -Y+X,+X,-Z
  static inline void MultDV201211110(TVector<double>& v)  {
    double a0 = v[0]-v[1];
    v[1] = v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultDA201211110(double* v)  {
    double a0 = v[0]-v[1];
    v[1] = v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultFV201211110(TVector<float>& v)  {
    float a0 = v[0]-v[1];
    v[1] = v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultFA201211110(float* v)  {
    float a0 = v[0]-v[1];
    v[1] = v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultIV201211110(TVector<int>& v)  {
    int a0 = v[0]-v[1];
    v[1] = v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultIA201211110(int* v)  {
    int a0 = v[0]-v[1];
    v[1] = v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultDVT201211110(TVector<double>& v)  {
    double a0 = v[0]+v[1];
    v[1] = -v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultDAT201211110(double* v)  {
    double a0 = v[0]+v[1];
    v[1] = -v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultFVT201211110(TVector<float>& v)  {
    float a0 = v[0]+v[1];
    v[1] = -v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultFAT201211110(float* v)  {
    float a0 = v[0]+v[1];
    v[1] = -v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultIVT201211110(TVector<int>& v)  {
    int a0 = v[0]+v[1];
    v[1] = -v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultIAT201211110(int* v)  {
    int a0 = v[0]+v[1];
    v[1] = -v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
// +Y-X,+Y,-Z
  static inline void MultDV021121110(TVector<double>& v)  {
    v[0] = -v[0]+v[1];
    v[2] = -v[2];
  }
  static inline void MultDA021121110(double* v)  {
    v[0] = -v[0]+v[1];
    v[2] = -v[2];
  }
  static inline void MultFV021121110(TVector<float>& v)  {
    v[0] = -v[0]+v[1];
    v[2] = -v[2];
  }
  static inline void MultFA021121110(float* v)  {
    v[0] = -v[0]+v[1];
    v[2] = -v[2];
  }
  static inline void MultIV021121110(TVector<int>& v)  {
    v[0] = -v[0]+v[1];
    v[2] = -v[2];
  }
  static inline void MultIA021121110(int* v)  {
    v[0] = -v[0]+v[1];
    v[2] = -v[2];
  }
  static inline void MultDVT021121110(TVector<double>& v)  {
    v[0] = -v[0];
    v[1] = -v[0]+v[1];
    v[2] = -v[2];
  }
  static inline void MultDAT021121110(double* v)  {
    v[0] = -v[0];
    v[1] = -v[0]+v[1];
    v[2] = -v[2];
  }
  static inline void MultFVT021121110(TVector<float>& v)  {
    v[0] = -v[0];
    v[1] = -v[0]+v[1];
    v[2] = -v[2];
  }
  static inline void MultFAT021121110(float* v)  {
    v[0] = -v[0];
    v[1] = -v[0]+v[1];
    v[2] = -v[2];
  }
  static inline void MultIVT021121110(TVector<int>& v)  {
    v[0] = -v[0];
    v[1] = -v[0]+v[1];
    v[2] = -v[2];
  }
  static inline void MultIAT021121110(int* v)  {
    v[0] = -v[0];
    v[1] = -v[0]+v[1];
    v[2] = -v[2];
  }
// -Y+X,-Y,+Z
  static inline void MultDV201101112(TVector<double>& v)  {
    v[0] = v[0]-v[1];
    v[1] = -v[1];
  }
  static inline void MultDA201101112(double* v)  {
    v[0] = v[0]-v[1];
    v[1] = -v[1];
  }
  static inline void MultFV201101112(TVector<float>& v)  {
    v[0] = v[0]-v[1];
    v[1] = -v[1];
  }
  static inline void MultFA201101112(float* v)  {
    v[0] = v[0]-v[1];
    v[1] = -v[1];
  }
  static inline void MultIV201101112(TVector<int>& v)  {
    v[0] = v[0]-v[1];
    v[1] = -v[1];
  }
  static inline void MultIA201101112(int* v)  {
    v[0] = v[0]-v[1];
    v[1] = -v[1];
  }
  static inline void MultDVT201101112(TVector<double>& v)  {
    v[1] = -v[0]-v[1];
  }
  static inline void MultDAT201101112(double* v)  {
    v[1] = -v[0]-v[1];
  }
  static inline void MultFVT201101112(TVector<float>& v)  {
    v[1] = -v[0]-v[1];
  }
  static inline void MultFAT201101112(float* v)  {
    v[1] = -v[0]-v[1];
  }
  static inline void MultIVT201101112(TVector<int>& v)  {
    v[1] = -v[0]-v[1];
  }
  static inline void MultIAT201101112(int* v)  {
    v[1] = -v[0]-v[1];
  }
// +X,+X-Y,-Z
  static inline void MultDV211201110(TVector<double>& v)  {
    v[1] = v[0]-v[1];
    v[2] = -v[2];
  }
  static inline void MultDA211201110(double* v)  {
    v[1] = v[0]-v[1];
    v[2] = -v[2];
  }
  static inline void MultFV211201110(TVector<float>& v)  {
    v[1] = v[0]-v[1];
    v[2] = -v[2];
  }
  static inline void MultFA211201110(float* v)  {
    v[1] = v[0]-v[1];
    v[2] = -v[2];
  }
  static inline void MultIV211201110(TVector<int>& v)  {
    v[1] = v[0]-v[1];
    v[2] = -v[2];
  }
  static inline void MultIA211201110(int* v)  {
    v[1] = v[0]-v[1];
    v[2] = -v[2];
  }
  static inline void MultDVT211201110(TVector<double>& v)  {
    v[0] = v[0]+v[1];
    v[1] = -v[1];
    v[2] = -v[2];
  }
  static inline void MultDAT211201110(double* v)  {
    v[0] = v[0]+v[1];
    v[1] = -v[1];
    v[2] = -v[2];
  }
  static inline void MultFVT211201110(TVector<float>& v)  {
    v[0] = v[0]+v[1];
    v[1] = -v[1];
    v[2] = -v[2];
  }
  static inline void MultFAT211201110(float* v)  {
    v[0] = v[0]+v[1];
    v[1] = -v[1];
    v[2] = -v[2];
  }
  static inline void MultIVT211201110(TVector<int>& v)  {
    v[0] = v[0]+v[1];
    v[1] = -v[1];
    v[2] = -v[2];
  }
  static inline void MultIAT211201110(int* v)  {
    v[0] = v[0]+v[1];
    v[1] = -v[1];
    v[2] = -v[2];
  }
// -X,-X+Y,+Z
  static inline void MultDV011021112(TVector<double>& v)  {
    v[0] = -v[0];
    v[1] = v[0]+v[1];
  }
  static inline void MultDA011021112(double* v)  {
    v[0] = -v[0];
    v[1] = v[0]+v[1];
  }
  static inline void MultFV011021112(TVector<float>& v)  {
    v[0] = -v[0];
    v[1] = v[0]+v[1];
  }
  static inline void MultFA011021112(float* v)  {
    v[0] = -v[0];
    v[1] = v[0]+v[1];
  }
  static inline void MultIV011021112(TVector<int>& v)  {
    v[0] = -v[0];
    v[1] = v[0]+v[1];
  }
  static inline void MultIA011021112(int* v)  {
    v[0] = -v[0];
    v[1] = v[0]+v[1];
  }
  static inline void MultDVT011021112(TVector<double>& v)  {
    v[0] = -v[0]-v[1];
  }
  static inline void MultDAT011021112(double* v)  {
    v[0] = -v[0]-v[1];
  }
  static inline void MultFVT011021112(TVector<float>& v)  {
    v[0] = -v[0]-v[1];
  }
  static inline void MultFAT011021112(float* v)  {
    v[0] = -v[0]-v[1];
  }
  static inline void MultIVT011021112(TVector<int>& v)  {
    v[0] = -v[0]-v[1];
  }
  static inline void MultIAT011021112(int* v)  {
    v[0] = -v[0]-v[1];
  }
// -Y+X,-Y,-Z
  static inline void MultDV201101110(TVector<double>& v)  {
    v[0] = v[0]-v[1];
    v[1] = -v[1];
    v[2] = -v[2];
  }
  static inline void MultDA201101110(double* v)  {
    v[0] = v[0]-v[1];
    v[1] = -v[1];
    v[2] = -v[2];
  }
  static inline void MultFV201101110(TVector<float>& v)  {
    v[0] = v[0]-v[1];
    v[1] = -v[1];
    v[2] = -v[2];
  }
  static inline void MultFA201101110(float* v)  {
    v[0] = v[0]-v[1];
    v[1] = -v[1];
    v[2] = -v[2];
  }
  static inline void MultIV201101110(TVector<int>& v)  {
    v[0] = v[0]-v[1];
    v[1] = -v[1];
    v[2] = -v[2];
  }
  static inline void MultIA201101110(int* v)  {
    v[0] = v[0]-v[1];
    v[1] = -v[1];
    v[2] = -v[2];
  }
  static inline void MultDVT201101110(TVector<double>& v)  {
    v[1] = -v[0]-v[1];
    v[2] = -v[2];
  }
  static inline void MultDAT201101110(double* v)  {
    v[1] = -v[0]-v[1];
    v[2] = -v[2];
  }
  static inline void MultFVT201101110(TVector<float>& v)  {
    v[1] = -v[0]-v[1];
    v[2] = -v[2];
  }
  static inline void MultFAT201101110(float* v)  {
    v[1] = -v[0]-v[1];
    v[2] = -v[2];
  }
  static inline void MultIVT201101110(TVector<int>& v)  {
    v[1] = -v[0]-v[1];
    v[2] = -v[2];
  }
  static inline void MultIAT201101110(int* v)  {
    v[1] = -v[0]-v[1];
    v[2] = -v[2];
  }
// +Y-X,+Y,+Z
  static inline void MultDV021121112(TVector<double>& v)  {
    v[0] = -v[0]+v[1];
  }
  static inline void MultDA021121112(double* v)  {
    v[0] = -v[0]+v[1];
  }
  static inline void MultFV021121112(TVector<float>& v)  {
    v[0] = -v[0]+v[1];
  }
  static inline void MultFA021121112(float* v)  {
    v[0] = -v[0]+v[1];
  }
  static inline void MultIV021121112(TVector<int>& v)  {
    v[0] = -v[0]+v[1];
  }
  static inline void MultIA021121112(int* v)  {
    v[0] = -v[0]+v[1];
  }
  static inline void MultDVT021121112(TVector<double>& v)  {
    v[0] = -v[0];
    v[1] = -v[0]+v[1];
  }
  static inline void MultDAT021121112(double* v)  {
    v[0] = -v[0];
    v[1] = -v[0]+v[1];
  }
  static inline void MultFVT021121112(TVector<float>& v)  {
    v[0] = -v[0];
    v[1] = -v[0]+v[1];
  }
  static inline void MultFAT021121112(float* v)  {
    v[0] = -v[0];
    v[1] = -v[0]+v[1];
  }
  static inline void MultIVT021121112(TVector<int>& v)  {
    v[0] = -v[0];
    v[1] = -v[0]+v[1];
  }
  static inline void MultIAT021121112(int* v)  {
    v[0] = -v[0];
    v[1] = -v[0]+v[1];
  }
// -X,-X+Y,-Z
  static inline void MultDV011021110(TVector<double>& v)  {
    v[0] = -v[0];
    v[1] = v[0]+v[1];
    v[2] = -v[2];
  }
  static inline void MultDA011021110(double* v)  {
    v[0] = -v[0];
    v[1] = v[0]+v[1];
    v[2] = -v[2];
  }
  static inline void MultFV011021110(TVector<float>& v)  {
    v[0] = -v[0];
    v[1] = v[0]+v[1];
    v[2] = -v[2];
  }
  static inline void MultFA011021110(float* v)  {
    v[0] = -v[0];
    v[1] = v[0]+v[1];
    v[2] = -v[2];
  }
  static inline void MultIV011021110(TVector<int>& v)  {
    v[0] = -v[0];
    v[1] = v[0]+v[1];
    v[2] = -v[2];
  }
  static inline void MultIA011021110(int* v)  {
    v[0] = -v[0];
    v[1] = v[0]+v[1];
    v[2] = -v[2];
  }
  static inline void MultDVT011021110(TVector<double>& v)  {
    v[0] = -v[0]-v[1];
    v[2] = -v[2];
  }
  static inline void MultDAT011021110(double* v)  {
    v[0] = -v[0]-v[1];
    v[2] = -v[2];
  }
  static inline void MultFVT011021110(TVector<float>& v)  {
    v[0] = -v[0]-v[1];
    v[2] = -v[2];
  }
  static inline void MultFAT011021110(float* v)  {
    v[0] = -v[0]-v[1];
    v[2] = -v[2];
  }
  static inline void MultIVT011021110(TVector<int>& v)  {
    v[0] = -v[0]-v[1];
    v[2] = -v[2];
  }
  static inline void MultIAT011021110(int* v)  {
    v[0] = -v[0]-v[1];
    v[2] = -v[2];
  }
// +X,+X-Y,+Z
  static inline void MultDV211201112(TVector<double>& v)  {
    v[1] = v[0]-v[1];
  }
  static inline void MultDA211201112(double* v)  {
    v[1] = v[0]-v[1];
  }
  static inline void MultFV211201112(TVector<float>& v)  {
    v[1] = v[0]-v[1];
  }
  static inline void MultFA211201112(float* v)  {
    v[1] = v[0]-v[1];
  }
  static inline void MultIV211201112(TVector<int>& v)  {
    v[1] = v[0]-v[1];
  }
  static inline void MultIA211201112(int* v)  {
    v[1] = v[0]-v[1];
  }
  static inline void MultDVT211201112(TVector<double>& v)  {
    v[0] = v[0]+v[1];
    v[1] = -v[1];
  }
  static inline void MultDAT211201112(double* v)  {
    v[0] = v[0]+v[1];
    v[1] = -v[1];
  }
  static inline void MultFVT211201112(TVector<float>& v)  {
    v[0] = v[0]+v[1];
    v[1] = -v[1];
  }
  static inline void MultFAT211201112(float* v)  {
    v[0] = v[0]+v[1];
    v[1] = -v[1];
  }
  static inline void MultIVT211201112(TVector<int>& v)  {
    v[0] = v[0]+v[1];
    v[1] = -v[1];
  }
  static inline void MultIAT211201112(int* v)  {
    v[0] = v[0]+v[1];
    v[1] = -v[1];
  }
// +Y-X,-X,-Z
  static inline void MultDV021011110(TVector<double>& v)  {
    double a0 = -v[0]+v[1];
    v[1] = -v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultDA021011110(double* v)  {
    double a0 = -v[0]+v[1];
    v[1] = -v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultFV021011110(TVector<float>& v)  {
    float a0 = -v[0]+v[1];
    v[1] = -v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultFA021011110(float* v)  {
    float a0 = -v[0]+v[1];
    v[1] = -v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultIV021011110(TVector<int>& v)  {
    int a0 = -v[0]+v[1];
    v[1] = -v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultIA021011110(int* v)  {
    int a0 = -v[0]+v[1];
    v[1] = -v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultDVT021011110(TVector<double>& v)  {
    double a0 = -v[0]-v[1];
    v[1] = v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultDAT021011110(double* v)  {
    double a0 = -v[0]-v[1];
    v[1] = v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultFVT021011110(TVector<float>& v)  {
    float a0 = -v[0]-v[1];
    v[1] = v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultFAT021011110(float* v)  {
    float a0 = -v[0]-v[1];
    v[1] = v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultIVT021011110(TVector<int>& v)  {
    int a0 = -v[0]-v[1];
    v[1] = v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultIAT021011110(int* v)  {
    int a0 = -v[0]-v[1];
    v[1] = v[0];
    v[2] = -v[2];
    v[0] = a0;
  }
// -Y,+X-Y,-Z
  static inline void MultDV101201110(TVector<double>& v)  {
    double a0 = -v[1];
    v[1] = v[0]-v[1];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultDA101201110(double* v)  {
    double a0 = -v[1];
    v[1] = v[0]-v[1];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultFV101201110(TVector<float>& v)  {
    float a0 = -v[1];
    v[1] = v[0]-v[1];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultFA101201110(float* v)  {
    float a0 = -v[1];
    v[1] = v[0]-v[1];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultIV101201110(TVector<int>& v)  {
    int a0 = -v[1];
    v[1] = v[0]-v[1];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultIA101201110(int* v)  {
    int a0 = -v[1];
    v[1] = v[0]-v[1];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultDVT101201110(TVector<double>& v)  {
    double a0 = v[1];
    v[1] = -v[0]-v[1];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultDAT101201110(double* v)  {
    double a0 = v[1];
    v[1] = -v[0]-v[1];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultFVT101201110(TVector<float>& v)  {
    float a0 = v[1];
    v[1] = -v[0]-v[1];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultFAT101201110(float* v)  {
    float a0 = v[1];
    v[1] = -v[0]-v[1];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultIVT101201110(TVector<int>& v)  {
    int a0 = v[1];
    v[1] = -v[0]-v[1];
    v[2] = -v[2];
    v[0] = a0;
  }
  static inline void MultIAT101201110(int* v)  {
    int a0 = v[1];
    v[1] = -v[0]-v[1];
    v[2] = -v[2];
    v[0] = a0;
  }
// -Y+X,+X,+Z
  static inline void MultDV201211112(TVector<double>& v)  {
    double a0 = v[0]-v[1];
    v[1] = v[0];
    v[0] = a0;
  }
  static inline void MultDA201211112(double* v)  {
    double a0 = v[0]-v[1];
    v[1] = v[0];
    v[0] = a0;
  }
  static inline void MultFV201211112(TVector<float>& v)  {
    float a0 = v[0]-v[1];
    v[1] = v[0];
    v[0] = a0;
  }
  static inline void MultFA201211112(float* v)  {
    float a0 = v[0]-v[1];
    v[1] = v[0];
    v[0] = a0;
  }
  static inline void MultIV201211112(TVector<int>& v)  {
    int a0 = v[0]-v[1];
    v[1] = v[0];
    v[0] = a0;
  }
  static inline void MultIA201211112(int* v)  {
    int a0 = v[0]-v[1];
    v[1] = v[0];
    v[0] = a0;
  }
  static inline void MultDVT201211112(TVector<double>& v)  {
    double a0 = v[0]+v[1];
    v[1] = -v[0];
    v[0] = a0;
  }
  static inline void MultDAT201211112(double* v)  {
    double a0 = v[0]+v[1];
    v[1] = -v[0];
    v[0] = a0;
  }
  static inline void MultFVT201211112(TVector<float>& v)  {
    float a0 = v[0]+v[1];
    v[1] = -v[0];
    v[0] = a0;
  }
  static inline void MultFAT201211112(float* v)  {
    float a0 = v[0]+v[1];
    v[1] = -v[0];
    v[0] = a0;
  }
  static inline void MultIVT201211112(TVector<int>& v)  {
    int a0 = v[0]+v[1];
    v[1] = -v[0];
    v[0] = a0;
  }
  static inline void MultIAT201211112(int* v)  {
    int a0 = v[0]+v[1];
    v[1] = -v[0];
    v[0] = a0;
  }
// +Y,-X+Y,+Z
  static inline void MultDV121021112(TVector<double>& v)  {
    double a0 = v[1];
    v[1] = -v[0]+v[1];
    v[0] = a0;
  }
  static inline void MultDA121021112(double* v)  {
    double a0 = v[1];
    v[1] = -v[0]+v[1];
    v[0] = a0;
  }
  static inline void MultFV121021112(TVector<float>& v)  {
    float a0 = v[1];
    v[1] = -v[0]+v[1];
    v[0] = a0;
  }
  static inline void MultFA121021112(float* v)  {
    float a0 = v[1];
    v[1] = -v[0]+v[1];
    v[0] = a0;
  }
  static inline void MultIV121021112(TVector<int>& v)  {
    int a0 = v[1];
    v[1] = -v[0]+v[1];
    v[0] = a0;
  }
  static inline void MultIA121021112(int* v)  {
    int a0 = v[1];
    v[1] = -v[0]+v[1];
    v[0] = a0;
  }
  static inline void MultDVT121021112(TVector<double>& v)  {
    double a0 = -v[1];
    v[1] = v[0]+v[1];
    v[0] = a0;
  }
  static inline void MultDAT121021112(double* v)  {
    double a0 = -v[1];
    v[1] = v[0]+v[1];
    v[0] = a0;
  }
  static inline void MultFVT121021112(TVector<float>& v)  {
    float a0 = -v[1];
    v[1] = v[0]+v[1];
    v[0] = a0;
  }
  static inline void MultFAT121021112(float* v)  {
    float a0 = -v[1];
    v[1] = v[0]+v[1];
    v[0] = a0;
  }
  static inline void MultIVT121021112(TVector<int>& v)  {
    int a0 = -v[1];
    v[1] = v[0]+v[1];
    v[0] = a0;
  }
  static inline void MultIAT121021112(int* v)  {
    int a0 = -v[1];
    v[1] = v[0]+v[1];
    v[0] = a0;
  }
};

EndXlibNamespace()

#endif
