/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#define ID_TIMER 1000
#include "glcursor.h"
#include "glrender.h"
#include "gpcollection.h"
#include "glmaterial.h"
#include "styles.h"
#include "glprimitive.h"
#include "library.h"

TGlCursor::TGlCursor(TGlRenderer& R, const olxstr& collectionName, bool TextStyle) :
  AGDrawObject(R, collectionName), FontIndex(~0)
{
  TextStyle = TextStyle;
  Primitive = NULL;
  X = Y = 0;
  SetSelectable(false);
}
//..............................................................................
void TGlCursor::Create(const olxstr& cName)  {
  FontIndex = Parent.GetScene().FindFontIndexForType<TGlCursor>(FontIndex);
  if( !cName.IsEmpty() )
    SetCollectionName(cName);

  TGPCollection& GPC = Parent.FindOrCreateCollection(GetCollectionName());
  GPC.AddObject(*this);
  if( GPC.PrimitiveCount() != 0 )  return;

  TGraphicsStyle& GS = GPC.GetStyle();
  Symbol = GS.GetParam("Char", '|', true).CharAt(0);
  Primitive = &GPC.NewPrimitive("Text", sgloText);
  Primitive->SetProperties(GS.GetMaterial("Text", GetFont().GetMaterial()));
  Primitive->Params[0] = -1;  //bitmap; TTF by default
}
//..............................................................................
bool TGlCursor::Orient(TGlPrimitive& P)  {
  static olxstr Char = "|";
  Char[0] = Symbol;
  if( Primitive == NULL )  return true;
  Primitive->SetFont(&GetFont());
  Primitive->SetString(&Char);
  Parent.DrawText(P, X, Y, Z);
  return true;
}
//..............................................................................
bool TGlCursor::Dispatch( int MsgId, short MsgSubId, const IEObject *Sender,
  const IEObject *Data, TActionQueue *)
{
  static int count = 0;
  count ++;
  if( count < 20 )  return true;
  count = 0;
  //Parent()->DrawObject(this, false);
  return true;
}
//..............................................................................
void TGlCursor::SetSymbol(olxch v)  {
  GetPrimitives().GetStyle().SetParam("Char", v, true);
  Symbol = v;
}
//..............................................................................
TGlFont& TGlCursor::GetFont() const {
  return Parent.GetScene().GetFont(FontIndex, true);
}
//..............................................................................
//..............................................................................
//..............................................................................

void TGlCursor::LibSymbol(const TStrObjList& Params, TMacroData& E)  {
  if( Params.Count() == 0 )
    E.SetRetVal<olxstr>(Symbol);
  else
    SetSymbol( Params[0][0] );
}
//..............................................................................
TLibrary*  TGlCursor::ExportLibrary(const olxstr& name)  {
  TLibrary* lib = new TLibrary(name);
  lib->Register(
    new TFunction<TGlCursor>(this,  &TGlCursor::LibSymbol, "Symbol",
      fpNone|fpOne,
    "Returns or sets current symbol used to draw the cursor") );
  return lib;
}
//..............................................................................
