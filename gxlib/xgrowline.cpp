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
#include "symmparser.h"
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
  if (!C.IsNull()) {
    Params()[3] = C.Length();
    C.Normalise();
    Params()[0] = (float)(acos(C[2])*180/M_PI);
    if (olx_abs(Params()[0]-180) < 1e-3) { // degenerate case with Pi rotation
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
  olxstr colName = cName;
  if (colName.IsEmpty()) {
    colName = olxstr("GrowBond_") << _CAtom.GetType().symbol << '-' <<
      _XAtom.GetType().symbol;
  }
  TGraphicsStyle *style = Parent.GetStyles().FindStyle(colName);
  if (style == 0 || style->IsNew()) {
    TGraphicsStyle &st = (style == 0 ? Parent.GetStyles().NewStyle(colName)
      : *style);
    st.SetParam(GetPrimitiveMaskName(), 1040, false);
    TGraphicsStyle *ast = Parent.GetStyles().FindStyle(_CAtom.GetType().symbol);
    if (ast != NULL) {
      TGlMaterial *glm = ast->FindMaterial("Sphere");
      if (glm != 0)
        st.SetMaterial("Top stipple cone", *glm);
    }
    // atom may be not created yet, so using it's style!
    ast = Parent.GetStyles().FindStyle(_XAtom.GetType().symbol);
    if (ast != 0) {
      TGlMaterial *glm = ast->FindMaterial("Sphere");
      if (glm != 0)
        st.SetMaterial("Bottom cone", *glm);
    }
    st.SetNew(false);
  }
  TXBond::Create(colName);
  GetPrimitives().GetStyle().SetSaveable(false);
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
void TXGrowLine::SetLength(float V)  {
  Params()[3] = V;
}
//..............................................................................
const vec3d &TXGrowLine::GetFromCrd() const {
  return _XAtom.crd();
}
//..............................................................................
TIString TXGrowLine::ToString() const {
  olxstr rv = _XAtom.GetLabel();
  rv << '-' << _CAtom.GetLabel() << ": "
    << olxstr::FormatFloat(3, Length()) << '('
    << TSymmParser::MatrixToSymmEx(GetTransform()) << ')';
  return rv;
}