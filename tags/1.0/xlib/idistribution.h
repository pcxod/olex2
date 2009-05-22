#ifndef idustributionH
#define idustributionH

#include "chembase.h"
#include "elist.h"
#include "poly.h"

BeginChemNamespace()

struct TIsotopeData    // tree node
{
  TEList *Children;
  double M, W;
  TIsotopeData();
  ~TIsotopeData();
  void Evail(TEList *S, double &eM, double &eW);
};
//---------------------------------------------------------------------------
// calculates itsotopic distribution of a given formula; use TIPattern instead
//---------------------------------------------------------------------------
class TIDistribution
{
  TEList * FPolynomes;
  TIsotopeData FRoot;
  double FThreshold;
  int FMaxPoints;
protected:
  void Evail(TPolynomMember *PM, double &M, double &W);
  TEList* PolynomToSerie(TPolynom *P);
public:
  TIDistribution();
  ~TIDistribution();
  void AddIsotope(class TBasicAtomInfo *ai, int count);
  void Calc(TEList *S);
  void SetThreshold(double v){FThreshold = v;};
  double GetThreshold(){return FThreshold;};
  void SetMaxPoints(int v){FMaxPoints = v;};
  static void CombineSerie(TEList *S, double threshold);
  static void DeleteSerie(TEList *S);
};

EndChemNamespace()
#endif
