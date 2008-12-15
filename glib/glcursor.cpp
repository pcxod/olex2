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

#include "library.h"

UseGlNamespace()
//..............................................................................
//..............................................................................

TGlCursor::TGlCursor(const olxstr& collectionName, TGlRender *Render, bool TextStyle) :
  AGDrawObject(collectionName)
{
  FTextStyle = TextStyle;
  AGDrawObject::Parent(Render);
  FPrimitive = NULL;
  FX = FY = 0;
  Groupable(false);
}
//..............................................................................
TGlCursor::~TGlCursor() {  }
//..............................................................................
void TGlCursor::Create(const olxstr& cName, const ACreationParams* cpar)  {
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);
  TGlPrimitive *GlP;
  TGPCollection *GPC;

  GPC = FParent->FindCollection( GetCollectionName() );
  if( !GPC )    GPC = FParent->NewCollection( GetCollectionName() );
  GPC->AddObject(this);
  TGraphicsStyle *GS = GPC->Style();
  Symbol = GS->ParameterValue("Char", '|')[0];
  TGlMaterial* FGlM = const_cast<TGlMaterial*>(GS->Material("On"));
  if( FGlM->Mark() )  {
    FGlM->SetFlags(sglmAmbientF|sglmIdentityDraw);
    FGlM->AmbientF  = 0x00ffff;
  }
  FPrimitive = GlP = GPC->NewPrimitive("Text");
  GlP->SetProperties(FGlM);
  GlP->Type(sgloText);
  GlP->Params()[0] = -1;  //bitmap; TTF by default
}
//..............................................................................
bool TGlCursor::Orient(TGlPrimitive *P)  {
  static olxstr Char = "|";
  Char[0] = Symbol;
  TGlFont *Fnt = Font();
  if( !Fnt )  return true;
  if( !FPrimitive  )  return true;
  FPrimitive->Font(Fnt);
  FPrimitive->String( &Char );
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
  Primitives()->Style()->SetParameter("Char", v);
  Symbol = v;
}
//..............................................................................
TGlFont *TGlCursor::Font()  const   {  return FParent->Scene()->Font(FFontIndex); }
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

