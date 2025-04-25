/******************************************************************************
* Copyright (c) 2004-2025 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "gdrawobject.h"
#include "glprimitive.h"
#include "gpcollection.h"
#include "styles.h"
UseGlNamespace()

AGDrawObject::AGDrawObject(TGlRenderer& parent, const olxstr& collectionName)
  : Parent(parent)
{
  Primitives = 0;
  ParentGroup = 0;
  sgdo_Flags = sgdoPrintable;
  SetGroupable(false);
  SetSelectable(true);
  CollectionName = collectionName.IsEmpty() ? EsdlObjectName(*this)
    : collectionName;
}
//..............................................................................
void AGDrawObject::Compile() {
  for (size_t i = 0; i < Primitives->PrimitiveCount(); i++) {
    Primitives->GetPrimitive(i).Compile();
  }
}
//..............................................................................
void AGDrawObject::UpdatePrimitives(int32_t Mask) {
  olxstr& mstr = GetPrimitives().GetStyle().GetParam(GetPrimitiveMaskName(), "0");
  if (mstr.ToInt() == Mask) {
    return;
  }
  mstr = Mask;
  GetPrimitives().ClearPrimitives();
  GetPrimitives().RemoveObject(*this);
  Create();
}
//..............................................................................
//..............................................................................
//..............................................................................
void AGDrawObject::LibVisible(const TStrObjList& Params, TMacroData& E)  {
  if (!Params.IsEmpty()) {
    SetVisible(Params[0].ToBool());
  }
  else {
    E.SetRetVal<olxstr>(IsVisible());
  }
}
//..............................................................................
void AGDrawObject::LibIsGrouped(const TStrObjList& Params, TMacroData& E) {
  E.SetRetVal<olxstr>(IsGrouped());
}
//..............................................................................
void AGDrawObject::LibIsSelected(const TStrObjList& Params, TMacroData& E) {
  E.SetRetVal<olxstr>(IsSelected());
}
//..............................................................................
void AGDrawObject::LibGetName(const TStrObjList& Params, TMacroData& E) {
  E.SetRetVal<olxstr>(CollectionName);
}
//..............................................................................
void AGDrawObject::ExportLibrary(TLibrary& lib) {
  lib.Register(new TFunction<AGDrawObject>(this, &AGDrawObject::LibVisible,
    "Visible", fpNone | fpOne, "Changes/returns object visibility"));
  lib.Register(new TFunction<AGDrawObject>(this, &AGDrawObject::LibIsGrouped,
    "IsGrouped", fpNone, "Returns true if the object is in a group"));
  lib.Register(new TFunction<AGDrawObject>(this, &AGDrawObject::LibIsSelected,
    "IsSelected", fpNone, "Returns true if the object is selected"));
  lib.Register(new TFunction<AGDrawObject>(this, &AGDrawObject::LibGetName,
    "GetName", fpNone, "Returns object collection name"));
}
//..............................................................................
#if _MSC_VER == 1900
template <>
AGDrawObject const & esdl::olx_ref::get<AGDrawObject>(AGDrawObject const & x) {
  return x;
}
#endif
//..............................................................................
double AGDrawObject::CalcZ() const {
  return Parent.Project(CalcCenter())[2];
}
//..............................................................................
void AGDrawObject::DoDraw(bool SelectPrimitives, bool SelectObjects) {
  if (SelfDraw(SelectPrimitives, SelectObjects)) {
    return;
  }
  const TGPCollection &ps = GetPrimitives();
  if (ps.PrimitiveCount() == 0) {
    return;
  }
  for (size_t i = 0; i < ps.PrimitiveCount(); i++) {
    TGlPrimitive &GlP = ps.GetPrimitive(i);
    Parent.HandleSelection(*this, GlP, SelectObjects, SelectPrimitives);
    if (!Parent.ForcePlain()) {
      GlP.GetProperties().Init(false);
    }
    olx_gl::pushMatrix();
    if (Orient(GlP)) {
      olx_gl::popMatrix();
      continue;
    }
    GlP.Draw();
    olx_gl::popMatrix();
  }
}
//..............................................................................
