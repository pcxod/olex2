//----------------------------------------------------------------------------//
// namespace TEXLib
// TGlConsole - a console
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif
#define ID_TIMER 1000

#include "glcursor.h"
#include "glrender.h"
#include "glscene.h"
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
  FTextStyle = TextStyle;
  FPrimitive = NULL;
  FX = FY = 0;
  SetGroupable(false);
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
  TGlMaterial GlM;
  GlM.SetFlags(sglmAmbientF|sglmIdentityDraw);
  GlM.AmbientF  = 0x00ffff;
  
  FPrimitive = &GPC.NewPrimitive("Text", sgloText);
  FPrimitive->SetProperties( GS.GetMaterial("On", GlM) );
  FPrimitive->Params[0] = -1;  //bitmap; TTF by default
}
//..............................................................................
bool TGlCursor::Orient(TGlPrimitive& P)  {
  static olxstr Char = "|";
  Char[0] = Symbol;
  TGlFont *Fnt = Font();
  if( Fnt == NULL || FPrimitive == NULL )  return true;
  FPrimitive->SetFont(Fnt);
  FPrimitive->SetString( &Char );
//  if( drawn )  {  FOffMat.Init();  drawn = false;  }
//  else         {  FOnMat.Init();  drawn = true;  }

//  FGlM->Init();
  glRasterPos3d(FX, FY, 0);
  FPrimitive->Draw();
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
TGlFont *TGlCursor::Font()  const   {  return Parent.GetScene().GetFont(FFontIndex); }
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

