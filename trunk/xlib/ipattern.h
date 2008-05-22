#ifndef ipatternH
#define ipatternH
#include "chembase.h"
#include "elist.h"
#include "poly.h"

BeginChemNamespace()

struct TSPoint
{
  double X, Y;
  static int SPointsSortA(const void *I, const void *I1);
  static int SPointsSortB(const void *I, const void *I1);
};

//---------------------------------------------------------------------------
// a wrapper for TIdistribution; use it for the calulation od isitope profiles
//---------------------------------------------------------------------------
class TIPattern
{
  void Clear();
  class TAtomsInfo *AtomsInfo;
  TEList * Points;
public:
  TIPattern(class TAtomsInfo *AI);
  ~TIPattern();
  inline int PointCount() const      {  return Points->Count();  }
  inline TSPoint* Point(int i) const {  return (TSPoint*)Points->Item(i);  }
  // calculates isotope pattern for Exp - chemical formula and fills Points with TSPoint
  // Msg - error message
  // Combine - if the masses have to be combined 
  // Delta - the difference between combined masses
  bool Calc( olxstr Exp, olxstr &Msg, bool Combine, double Delta);
  void SortDataByMolWeight();
  void SortDataByItensity();
};


EndChemNamespace()
#endif
