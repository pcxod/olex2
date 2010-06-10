//----------------------------------------------------------------------------//
// namespace TEXLib
// TGlConsole - a console
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//
#define ID_TIMER 1000

#include "glcursor.h"
#include "glrender.h"
#include "gpcollection.h"
#include "glmaterial.h"
#include "styles.h"
#include "glprimitive.h"

#include "library.h"

UseGlNamespace()
//..............................................................................
//..............................................................................

TGlCursor::TGlCursor(TGlRenderer& R, const olxstr& collectionName, bool TextStyle) :
  AGDrawObject(R, collectionName)
{
  TextStyle = TextStyle;
  Primitive = NULL;
  X = Y = 0;
  SetSelectable(false);
}
//..............................................................................
void TGlCursor::Create(const olxstr& cName, const ACreationParams* cpar)  {
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);

  TGPCollection& GPC = Parent.FindOrCreateCollection( GetCollectionName() );
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
  Parent.DrawText(P, X, Y, 0);
  return true;
}
//..............................................................................
bool TGlCursor::Dispatch( int MsgId, short MsgSubId, const IEObject *Sender, const IEObject *Data)  {
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
  TGlFont* fnt = Parent.GetScene().GetFont(FontIndex); 
  if( fnt == NULL )
    throw TInvalidArgumentException(__OlxSourceInfo, "font index");
  return *fnt;
}
//..............................................................................
//..............................................................................
//..............................................................................

void TGlCursor::LibSymbol(const TStrObjList& Params, TMacroError& E)  {
  if( Params.Count() == 0 )
    E.SetRetVal<olxstr>(Symbol);
  else
    SetSymbol( Params[0][0] );
}
//..............................................................................
TLibrary*  TGlCursor::ExportLibrary(const olxstr& name)  {
  TLibrary* lib = new TLibrary(name);
  lib->RegisterFunction<TGlCursor>( new TFunction<TGlCursor>(this,  &TGlCursor::LibSymbol, "Symbol",
    fpNone|fpOne, "Returns or sets current symbol used to draw the cursor") );
  return lib;
}
//..............................................................................

