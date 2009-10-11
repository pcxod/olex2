//---------------------------------------------------------------------------//
// Binominal polynomial routins
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//

#ifndef polyH
#define polyH
#include <math.h>
#include <stdlib.h>
#include "ebase.h"
//---------------------------------------------------------------------------
BeginEsdlNamespace()

struct TPMember
{
  int Id;
  int Extent;
  void *Data;
  bool Used;
  TPMember()
  {
    Id = 0;
    Extent = 1;
    Data = NULL;
  }
  bool operator == (TPMember *P)
  {
    if( P->Id != Id )          return false;
    if( P->Extent != Extent )  return false;
    if( P->Data != Data )      return false;
    return true;
  }
  void operator = (TPMember *P)
  {
    Id = P->Id;
    Extent = P->Extent;
    Data = P->Data;
  }
};
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
class TPolynomMember
{
  class TEList *FMembers;
  double FMult;
protected:
  void SetUsed( bool Value);
  void Clear();
public:
  TPolynomMember();
  ~TPolynomMember();
  TPMember *AddMember();
  bool operator == (TPolynomMember *P);
  void operator = (TPolynomMember *P);
  class TEList *Members(){return FMembers;};
  void Mul(TPolynomMember *P);
  double GetMult() {return FMult;};
  void IncMult(double c) { FMult +=c; };
  void MulMult(double c) { FMult *=c; };
  void Combine();
  olxstr Values();
};
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
typedef double (Evaluator)(TPolynomMember *P);
typedef bool (AddEvaluator)(TPolynomMember *P);
typedef int (PolySort)(const void *P, const void *P1);
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
class TPolynom: public IEObject  {
  TEList *FMembers;
protected:
  void Clear();
  void Combine();
  AddEvaluator *FAddEvaluator;
  Evaluator *FEvaluator;
  PolySort *FPolySort;
  void SetSize(int s);
public:
  TPolynom(AddEvaluator *av, Evaluator *v, PolySort *v1);
  ~TPolynom();
  TPolynomMember *AddMember();
  TEList *Members(){return FMembers;};
  TPolynom* Pow(short p);
  void SetThreshold(double Threshold);
  TPolynom* PowX(short MembersToLeave, short p);
  TPolynom* Mul(TPolynom *P);
  void MulSelf(TPolynom *P);
  void operator = (TPolynom *P);
  TPolynom* Qrt();
  void SetEvaluator(Evaluator *v){ FEvaluator = v; };
  olxstr Values();
};

EndEsdlNamespace()
#endif
