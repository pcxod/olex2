//----------------------------------------------------------------------------//
// namespace TXClasses: crystallographic core
// 
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//
//---------------------------------------------------------------------------
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "xbase.h"

const float XlibObject(dcMaxCBLength) = 3.5f;
const float XlibObject(dcMaxHBLength) = 4.5f; //maximu length of a short interaction
const float XlibObject(caDefIso) = 0.05f;  // default atom isotropic parameter;
//----------------------------------------------------------------------------//
// TSObject call function bodies
//----------------------------------------------------------------------------//
TSObject::TSObject(TNetwork *P)  {
  Network   = P;
  SetType(sotNone);
}
//..............................................................................
TSObject::~TSObject()  {  }
//..............................................................................
void  TSObject::Assign(TSObject *S)  {
  SetType( S->GetType() );
  Network = &S->GetNetwork();
  SetTag( S->GetTag() );
  SetNetId( S->GetNetId() );
  SetLatId( S->GetLatId() );
}
//..............................................................................

//----------------------------------------------------------------------------//
// TBasicNode function bodies
//----------------------------------------------------------------------------//
// TBasicBond function bodies
//----------------------------------------------------------------------------//
TBasicBond::TBasicBond(TNetwork *P):TSObject(P)
{
  FA = FB = NULL;
  SetType(sotBBond);
}
//..............................................................................
TBasicBond::~TBasicBond()
{
  ;
}


