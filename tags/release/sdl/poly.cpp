//---------------------------------------------------------------------------//
// Binominal polynomial routins
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "poly.h"
#include "elist.h"
#include "exception.h"

UseEsdlNamespace()
//----------------------------------------------------------------------------//
int _PMembersSort(const void *I, const void *I1)
{
  TPMember   *M = (TPMember*)I,
        *M1= (TPMember*)I1;
  if( M->Id == M1->Id )
    return M->Extent - M1->Extent;
  else
    return M->Id - M1->Id;
}
//----------------------------------------------------------------------------//
int _PolynomMembersSort(void *I, void *I1)
{
  TPolynomMember *PM = (TPolynomMember*)I,
          *PM1 = (TPolynomMember*)I1;
  if( *PM == PM1 )  return 0;
  else
  {
    if(PM->GetMult() < PM1->GetMult())
      return -1;
    else
      return 1;
  }
}
//----------------------------------------------------------------------------//
// TPolynomMember function bodies
//----------------------------------------------------------------------------//
TPolynomMember::TPolynomMember()
{
  FMembers = new TEList;
  FMult = 1;
}
//..............................................................................
TPolynomMember::~TPolynomMember()
{
  Clear();
  delete FMembers;
}
//..............................................................................
void TPolynomMember::Clear()
{
  for( int i=0; i < FMembers->Count(); i++ )
    delete (TPMember*)FMembers->Item(i);
  FMembers->Clear();
}
//..............................................................................
TPMember *TPolynomMember::AddMember()
{
  TPMember *P = new TPMember;
  P->Id = FMembers->Count();
  FMembers->Add(P);
  return P;
}
//..............................................................................
void TPolynomMember::SetUsed( bool Value)
{
  for( int i=0; i < FMembers->Count(); i++ )
    ((TPMember*)FMembers->Item(i))->Used = Value;
}
//..............................................................................
void TPolynomMember::operator = (TPolynomMember *P)
{
  Clear();
    FMult = P->FMult;
  TPMember *PM, *PM1;
  for( int i=0; i < P->FMembers->Count(); i++ )
  {
    PM = (TPMember*)P->FMembers->Item(i);
    PM1 = new TPMember();
    *PM1 = PM;
    FMembers->Add(PM1);
  }
}
//..............................................................................
bool TPolynomMember::operator == (TPolynomMember *P)
{
  if( P->FMembers->Count() != FMembers->Count() )
    return false;
  int count = FMembers->Count();
  TPMember *P1, *P2;
  for( int i = 0; i < count; i++ )
  {
    P1 = (TPMember*)FMembers->Item(i);
    P2 = (TPMember*)P->FMembers->Item(i);
    if( !(*P2 == P1) )  return false;
  }
  
/*  bool found;
  P->SetUsed(false);
  for( int i = 0; i < count; i++ )
  {
    P1 = (TPMember*)FMembers->Item(i);
    found = false;
    for( int j=0; j < count; j++ )
    {
      P2 = (TPMember*)P->FMembers->Item(j);
      if( P2->Used )  continue;
      if( *P2 == P1 )
      {
        P2->Used = true;
        found = true;
        break;
      }
    }
    if( !found )  return false;
  }*/
  return true;
}
//..............................................................................
olxstr TPolynomMember::Values()  {
  TPMember *P;
  olxstr T;
  if( FMult != 1)  {
    T = (double)FMult;
    T << '*';
  }
  int count = FMembers->Count();
  for( int i = 0; i < count; i++ )  {
    P = (TPMember*)FMembers->Item(i);
    T << (char)('a'+P->Id);
    if( P->Extent != 1 )  {
      T << '^' << P->Extent;
    }
  }
  return T;
}
//..............................................................................
void TPolynomMember::Combine()  {
  TPMember *P1, *P2;
  int count = FMembers->Count();
  for( int i = 0; i < count; i++ )
  {
    P1 = (TPMember*)FMembers->Item(i);
    if( !P1 )  continue;
    for( int j=i+1; j < count; j++ )
    {
      P2 = (TPMember*)FMembers->Item(j);
      if( !P2 )  continue;

      if( P2->Id == P1->Id )
      {
        P1->Extent += P2->Extent;
        delete P2;
        FMembers->Item(j) = NULL;
      }
    }
  }
  FMembers->Pack();
  FMembers->Sort(_PMembersSort);
}
//..............................................................................
void TPolynomMember::Mul(TPolynomMember *P)
{
  FMult = FMult*P->FMult;
  TPMember *P1, *P2;
  for( int i=0; i < P->FMembers->Count(); i++ )
  {
    P1 = (TPMember*)P->FMembers->Item(i);
    P2 = new TPMember();
    *P2 = P1;
    FMembers->Add(P2);
  }  
  Combine();
}
//----------------------------------------------------------------------------//
// TPolynom function bodies
//----------------------------------------------------------------------------//
TPolynom::TPolynom(AddEvaluator *av, Evaluator *v, PolySort *v1)
{
  FMembers = new TEList;
  FEvaluator = v;
  FPolySort = v1;
  FAddEvaluator = av;
}
//..............................................................................
TPolynom::~TPolynom()
{
  Clear();
  delete FMembers;
}
//..............................................................................
void TPolynom::Clear()
{
  for( int i=0; i < FMembers->Count(); i++ )
    delete (TPolynomMember*)FMembers->Item(i);
  FMembers->Clear();
}
//..............................................................................
TPolynomMember * TPolynom::AddMember()
{
  TPolynomMember *P = new TPolynomMember;
  FMembers->Add(P);
  return P;
}
//..............................................................................
void TPolynom::operator = (TPolynom *P)
{
  Clear();
  TPolynomMember *PM1, *PM2;
  for( int i=0; i < P->FMembers->Count(); i++ )
  {
    PM1 = (TPolynomMember*)P->FMembers->Item(i);
    PM2 = new TPolynomMember();
    *PM2 = PM1;
    FMembers->Add(PM2);
  }
}
//..............................................................................
void TPolynom::SetThreshold(double Threshold)
{
  if( FPolySort && FEvaluator && (FMembers->Count() > 0) )
  {
    FMembers->Sort(FPolySort);
    double v = FEvaluator((TPolynomMember*)FMembers->Item(0));
    if( !v )  return;
    TPolynomMember *PM;
    int S = FMembers->Count(), i;
    for( i=0; i < FMembers->Count(); i++ )
    {
      PM = (TPolynomMember*)FMembers->Item(i);
      if( FEvaluator(PM)/v < Threshold )
      {
        S = i;
        break;
      }
    }
    for( i=S; i < FMembers->Count(); i++ )
      delete (TPolynomMember*)FMembers->Item(i);
    FMembers->SetCount(S);
  }
  else
    throw TFunctionFailedException(__OlxSourceInfo, "Cannot compact the polynom!");
}
//..............................................................................
void TPolynom::SetSize(int S)
{
  if( FMembers->Count() < S )  return;
  if( FPolySort )
  {
    FMembers->Sort(FPolySort);
    for(int i=S; i < FMembers->Count(); i++ )
    {
      delete (TPolynomMember*)FMembers->Item(i);
    }
    FMembers->SetCount(S);
  }
  else
    throw TFunctionFailedException(__OlxSourceInfo, "The evaluation function is not defined");
}
//..............................................................................
TPolynom* TPolynom::PowX(short Members, short p)
{
  TPolynom *R, *C;
  if( p == 1 )
  {
    C = new TPolynom(FAddEvaluator, FEvaluator, FPolySort);
    *C = this;
    return C;
  }
  if( p == 2 )
  {
    R = this->Qrt();
    return R;
  }
  R = PowX((short)(sqrt((double)(Members*2)+1)), (short)(p/2));
  if( p%2 )
    Members /= (short)FMembers->Count();
  if( Members < 5 )  Members = 5;
  if( R->FMembers->Count() > Members )
    R->SetSize(Members);
  C = R->Qrt();
  delete R;
  if( p%2 )
  {
    R = C->Mul(this);
    delete C;
    return R;
  }
  return C;
}
//..............................................................................
TPolynom* TPolynom::Pow(short p)
{
  TPolynom *R, *C;
  if( p == 1 )
  {
    C = new TPolynom(FAddEvaluator, FEvaluator, FPolySort);
    *C = this;
    return C;
  }
  if( p == 2 )
  {
    R = this->Qrt();
    return R;
  }
  R = Pow((short)(p/2));
  C = R->Qrt();
  delete R;
  if( p%2 )
  {
    R = C->Mul(this);
    delete C;
    return R;
  }
  return C;
}
//..............................................................................
void TPolynom::Combine()
{
  TPolynomMember *PM, *PM1;
  int count = FMembers->Count();
  for( int i=0; i < count; i++ )
  {
    PM = (TPolynomMember*)FMembers->Item(i);
    if( !PM )  continue;
    for( int j=i+1; j < count; j++ )
    {
      PM1 = (TPolynomMember*)FMembers->Item(j);
      if( !PM1 )  continue;
      if( *PM == PM1 )
      {
        PM->IncMult(PM1->GetMult());
        delete PM1;
        FMembers->Item(j) = NULL;
      }
    }
  }
  FMembers->Pack();
}
//..............................................................................
olxstr TPolynom::Values()  {
  TPolynomMember *PM;
  olxstr T;
  for( int i=0; i < FMembers->Count(); i++ )  {
    PM = (TPolynomMember*)FMembers->Item(i);
    T << PM->Values() <<  "; ";
  }
  return T;
}
//..............................................................................
TPolynom* TPolynom::Mul(TPolynom *P)
{
  TPolynom *NP = new TPolynom(FAddEvaluator, FEvaluator, FPolySort);
  TPolynomMember *PM, *PM1, *PM2;
  TEList *NL = NP->Members();
  for( int i=0; i < FMembers->Count(); i++ )
  {
    PM = (TPolynomMember*)FMembers->Item(i);
    for( int j=0; j < P->FMembers->Count(); j++ )
    {
      PM1 = (TPolynomMember*)P->FMembers->Item(j);
      PM2 = new TPolynomMember();
      *PM2 = PM1;
      PM2->Mul(PM);

      if( FAddEvaluator )
      {
        if( FAddEvaluator(PM2) )
          NL->Add(PM2);
        else
          delete PM2;
      }
      else
      {
        NL->Add(PM2);
      }
    }
  }
  NP->Combine();
  return NP;
}
//..............................................................................
TPolynom* TPolynom::Qrt()
{
  TPolynom *NP = new TPolynom(FAddEvaluator, FEvaluator, FPolySort);
  TPolynomMember *PM, *PM1, *PM2;
  TPMember *PPM, *PPM1;
  TEList *L, *NL = NP->Members();
  NP->Members()->SetCapacity(FMembers->Count() + (FMembers->Count()-1)*FMembers->Count()/2);
  int i;
  for( i=0; i < FMembers->Count(); i++ )
  {
    PM = (TPolynomMember*)FMembers->Item(i);
    PM1 = new TPolynomMember();
    *PM1 = PM;
    L = PM1->Members();
    for( int j=0; j < L->Count(); j++ )
    {
      PPM = (TPMember*)L->Item(j);
      PPM->Extent *= 2;
    }
    PM1->MulMult(PM1->GetMult());
    if( FAddEvaluator )
    {
      if( FAddEvaluator(PM1) )
        NL->Add(PM1);
      else
        delete PM1;
    }
    else
    {
      NL->Add(PM1);
    }
  }
  for( i=0; i < FMembers->Count(); i++ )
  {
    PM = (TPolynomMember*)FMembers->Item(i);
    L = PM->Members();
    for( int j=i+1; j < FMembers->Count(); j++ )
    {
      PM1 = (TPolynomMember*)FMembers->Item(j);
      PM2 = new TPolynomMember();
      *PM2 = PM1;
      PM2->MulMult(PM->GetMult()*2);
      for( int k=0; k < L->Count(); k++ )
      {
        PPM = (TPMember*)L->Item(k);
        PPM1 = PM2->AddMember();
        *PPM1 = PPM;
      }
      PM2->Combine();
      if( FAddEvaluator )
      {
        if( FAddEvaluator(PM2) )
          NL->Add(PM2);
        else
          delete PM2;
      }
      else
      {
        NL->Add(PM2);
      }
    }
  }
  NP->Combine();
  return NP;
}
//..............................................................................

