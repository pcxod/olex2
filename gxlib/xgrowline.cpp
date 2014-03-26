/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "xgrowline.h"
#include "gpcollection.h"
#include "asymmunit.h"
#include "xatom.h"

TXGrowLine::TXGrowLine(TGlRenderer& r, const olxstr& collectionName, TXAtom& A,
  TCAtom& CA, const smatd& transform) :
  TXBond(NULL, r, collectionName),
  _XAtom(A),
  _CAtom(CA),
  Transform(transform)
{
  AGDrawObject::SetSelectable(false);
  FEdge = transform * CA.ccrd();
  CA.GetParent()->CellToCartesian(FEdge);
  vec3d C = FEdge - A.crd();
  if( !C.IsNull() )  {
    Params()[3] = C.Length();
    C.Normalise();
    Params()[0] = (float)(acos(C[2])*180/M_PI);
    if( olx_abs(Params()[0]-180) < 1e-3 )  { // degenerate case with Pi rotation
      Params()[1] = 0;
      Params()[2] = 1;
    }
    else {
      Params()[1] = -C[1];
      Params()[2] = C[0];
    }
  }
}
//..............................................................................
void TXGrowLine::Create(const olxstr& cName)  {
  TXBond::Create(cName);
}
//..............................................................................
bool TXGrowLine::Orient(TGlPrimitive& GlP)  {
  olx_gl::translate(_XAtom.crd());
  olx_gl::rotate(Params()[0], Params()[1], Params()[2], 0.0);
  olx_gl::scale(Params()[4], Params()[4], Params()[3]);
  return false;
}
//..............................................................................
void TXGrowLine::Radius(float V)  {
  Params()[4] = V;
}
//..............................................................................
void TXGrowLine::Length(float V)  {
  Params()[3] = V;
}
//..............................................................................
const vec3d &TXGrowLine::GetBaseCrd() const {  return _XAtom.crd();  }
//..............................................................................
