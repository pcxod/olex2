#ifndef chnexpH
#define chnexpH
#include "chembase.h"
#include "estrlist.h"

BeginChemNamespace()

class TCHNExp: public IEObject  {
  TStrPObjList<olxstr,double> Exp; // Objects - double for content
  TTypeList<TCHNExp>   Dependencies; // TCHNExp list of dependencies
  double FMult;
protected:
  void Clear();
  class TAtomsInfo *FAtomsInfo;
public:
  TCHNExp( TAtomsInfo *AI );
  virtual ~TCHNExp();
  // return summ formula of the compound with elements seprated by the Seprator
  olxstr SummFormula(const olxstr& Separator);
  double MolWeight();
  olxstr Composition();
  // calculates C, H and N and mol weight contents of  compound
  // to get %: %(c) = C*100/Mr
  void CHN(double &C, double &H, double &N, double &Mr);
  // a valid array with at least for elements is expected!
  void CHN(double *Bf)  {  CHN(Bf[0], Bf[1], Bf[2], Bf[4]);  }
  void LoadFromExpression(const olxstr &E);
  void CalcSummFormula(TStrPObjList<olxstr,double>& Exp);
  void SetMult(const olxstr &S)    {  FMult = S.ToDouble();  }
  inline double GetMult()  const   {  return FMult;  }

  inline TAtomsInfo* AtomsInfo()  const {  return FAtomsInfo;  }
};

EndChemNamespace()
#endif
