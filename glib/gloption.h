//---------------------------------------------------------------------------

#ifndef gloptionH
#define gloptionH

#include "glbase.h"

BeginGlNamespace()

class TGlOption: public IEObject  {
  float FV[4];
public:
  TGlOption();
  TGlOption(int RGB);

  void operator =(int c);
  void operator =(float *c);
  void operator *= (double V);
  TGlOption operator * (double V) const;
  TGlOption operator + (const TGlOption& V) const;
  void operator -= (const TGlOption& V);
  void operator += (const TGlOption& V);

  int GetRGB() const;
  inline const float* Data()  const {  return FV;  }
  bool IsEmpty() const;
  void Clear();
  void operator = (const TGlOption& S);
  inline float& operator[] (int i) {  return FV[i]; }
  bool operator == (const TGlOption& S) const;

  TIString ToString() const;
  bool FromString(const olxstr &S);
};

EndGlNamespace()
#endif

