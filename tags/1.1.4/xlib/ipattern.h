#ifndef olx_cl_ipattern_H
#define olx_cl_ipattern_H
#include "chembase.h"
#include "poly.h"
BeginChemNamespace()
//---------------------------------------------------------------------------
// a wrapper for TIdistribution; use it for the calulation od isitope profiles
//---------------------------------------------------------------------------
class TIPattern  {
  void Clear();
  TPolySerie Points;
public:
  TIPattern() {}
  ~TIPattern() {}
  inline size_t PointCount() const {  return Points.Count();  }
  inline TSPoint& Point(size_t i) const {  return Points[i];  }
  // calculates isotope pattern for Exp - chemical formula and fills Points with TSPoint
  // Msg - error message
  // Combine - if the masses have to be combined 
  // Delta - the difference between combined masses
  bool Calc(const olxstr& Exp, olxstr& Msg, bool Combine, double Delta);
  void SortDataByMolWeight();
  void SortDataByItensity();
};


EndChemNamespace()
#endif
