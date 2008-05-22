//---------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "gdrawobject.h"
#include "glprimitive.h"
#include "gpcollection.h"

#include "library.h"

UseGlNamespace();
//..............................................................................
//..............................................................................
AGDrawObject::AGDrawObject(const olxstr& collectionName)  {
  FPrimitives = NULL;
  FParentGroup = NULL;
  Flags = 0;
  FParent = NULL;
  Visible(true);
  Groupable(true);
  if( collectionName.Length() )
    CollectionName = collectionName;
  else
    CollectionName = EsdlObjectName(*this);
}
//..............................................................................
AGDrawObject::~AGDrawObject()  {  ;  }
//..............................................................................
void AGDrawObject::Compile()  {
  for( int i=0; i < FPrimitives->PrimitiveCount(); i++ )
    FPrimitives->Primitive(i)->Compile();
}
//..............................................................................
void  AGDrawObject::Primitives( TGPCollection *GPC)  {
  FPrimitives = GPC;
  FParent = GPC->Parent();
}
//..............................................................................
void AGDrawObject::ParentGroup(TGlGroup *P)  {
  FParentGroup = P;
  if( P ) {    Grouped(true);  }
  else    {    Grouped(false); }
}
//..............................................................................
void AGDrawObject::UpdatePrimitives(int32_t Mask){  return; }
//..............................................................................
void AGDrawObject::OnPrimitivesCleared(){  return; }
//..............................................................................
//..............................................................................
//..............................................................................

void AGDrawObject::LibVisible(const TStrObjList& Params, TMacroError& E)  {
  if( Params.Count() ) Visible( Params[0].ToBool() );
  else
    E.SetRetVal<olxstr>( Visible() );
}
//..............................................................................
void AGDrawObject::LibIsGrouped(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal<olxstr>( Grouped() );
}
//..............................................................................
void AGDrawObject::LibIsSelected(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal<olxstr>( Selected() );
}
//..............................................................................
void AGDrawObject::LibGetName(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal<olxstr>( CollectionName );
}
//..............................................................................
void AGDrawObject::ExportLibrary(class TLibrary& lib)  {
  lib.RegisterFunction<AGDrawObject>( new TFunction<AGDrawObject>(this,  &AGDrawObject::LibVisible,
  "Visible", fpNone|fpOne, "Changes/returns object visiblity") );
  lib.RegisterFunction<AGDrawObject>( new TFunction<AGDrawObject>(this,  &AGDrawObject::LibIsGrouped,
  "IsGrouped", fpNone, "Returns true if the object is in a group") );
  lib.RegisterFunction<AGDrawObject>( new TFunction<AGDrawObject>(this,  &AGDrawObject::LibIsSelected,
  "IsSelected", fpNone, "Returns true if the object is selected") );
  lib.RegisterFunction<AGDrawObject>( new TFunction<AGDrawObject>(this,  &AGDrawObject::LibGetName,
  "GetName", fpNone, "Returns object collection name") );
}
//..............................................................................
