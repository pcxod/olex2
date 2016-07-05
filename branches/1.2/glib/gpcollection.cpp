/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "gpcollection.h"
#include "glrender.h"
#include "gdrawobject.h"
#include "styles.h"
#include "glprimitive.h"

void TGPCollection::ClearPrimitives()  {
  for( size_t i=0; i < Parent.PrimitiveCount(); i++ )
    Parent.GetPrimitive(i).SetTag(-1);
  Primitives.ForEach(ACollectionItem::TagSetter(0));
  Parent.RemovePrimitiveByTag(0);
  Primitives.Clear();
}
//..............................................................................
TGlPrimitive& TGPCollection::NewPrimitive(const olxstr& Name, short type)  {
  TGlPrimitive& GlP = Parent.NewPrimitive(type);
  GlP.SetName(Name);
  AddPrimitive(GlP);
  GlP.SetParentCollection(this);
  return GlP;
};
//..............................................................................
TGlPrimitive* TGPCollection::FindPrimitiveByName(const olxstr &Name) const {
  for( size_t i = 0; i < Primitives.Count(); i++ )
    if( Primitives[i]->GetName() == Name )
      return Primitives[i];
  return NULL;
}
//..............................................................................
void TGPCollection::Draw()  {
  for( size_t i = 0; i < Primitives.Count(); i++ )
    Primitives[i]->Draw();
}
//..............................................................................
void TGPCollection::AddObject(AGDrawObject& Obj)  {
  GObjects.Add(Obj);
  Obj.SetPrimitives(*this);
  Parent.AddObject(Obj);
};
//..............................................................................
void TGPCollection::ListParams(TStrList &List, TGlPrimitive *Primitive)  {
  if( !GObjects.IsEmpty() )
    GetObject(0).ListParams(List, Primitive);
}
bool TGPCollection::ContainsPrimitive(TGlPrimitive& GlP)  {
  return Primitives.IndexOf(GlP) != InvalidIndex;
}
//..............................................................................
void TGPCollection::SetStyle(TGraphicsStyle *S)  {
  Style = S;
  if( S == NULL )  return;
  // update materials of primitives & filling the style
  for( size_t i=0; i < PrimitiveCount(); i++ )  {
    TGlPrimitive& GlP = GetPrimitive(i);
    if (S->IndexOfMaterial(GlP.GetName()) == InvalidIndex )
      S->SetMaterial(GlP.GetName(), GlP.GetProperties());
  }
}
