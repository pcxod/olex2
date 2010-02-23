//---------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//
#include "gdrawobject.h"
#include "glprimitive.h"
#include "gpcollection.h"
#include "library.h"
#include "styles.h"

UseGlNamespace();
//..............................................................................
//..............................................................................
AGDrawObject::AGDrawObject(TGlRenderer& parent, const olxstr& collectionName) :
  Parent(parent)
{
  Primitives = NULL;
  ParentGroup = NULL;
  Flags = 0;
  SetVisible(true);
  SetGroupable(true);
  if( collectionName.Length() )
    CollectionName = collectionName;
  else
    CollectionName = EsdlObjectName(*this);
}
//..............................................................................
AGDrawObject::~AGDrawObject()  {  ;  }
//..............................................................................
void AGDrawObject::Compile()  {
  for( size_t i=0; i < Primitives->PrimitiveCount(); i++ )
    Primitives->GetPrimitive(i).Compile();
}
//..............................................................................
void AGDrawObject::UpdatePrimitives(int32_t Mask, const ACreationParams* cpar)  {
  olxstr& mstr = GetPrimitives().GetStyle().GetParam(GetPrimitiveMaskName(), "0");
  if( mstr.ToInt() == Mask )  return;
  mstr = Mask;
  GetPrimitives().ClearPrimitives();
  GetPrimitives().RemoveObject(*this);
  Create(EmptyString, cpar);
}
//..............................................................................
//..............................................................................
//..............................................................................

void AGDrawObject::LibVisible(const TStrObjList& Params, TMacroError& E)  {
  if( !Params.IsEmpty() ) 
    SetVisible( Params[0].ToBool() );
  else
    E.SetRetVal<olxstr>( IsVisible() );
}
//..............................................................................
void AGDrawObject::LibIsGrouped(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal<olxstr>( IsGrouped() );
}
//..............................................................................
void AGDrawObject::LibIsSelected(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal<olxstr>( IsSelected() );
}
//..............................................................................
void AGDrawObject::LibGetName(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal<olxstr>( CollectionName );
}
//..............................................................................
void AGDrawObject::ExportLibrary(TLibrary& lib)  {
  lib.RegisterFunction<AGDrawObject>( new TFunction<AGDrawObject>(this,  &AGDrawObject::LibVisible,
  "Visible", fpNone|fpOne, "Changes/returns object visibility") );
  lib.RegisterFunction<AGDrawObject>( new TFunction<AGDrawObject>(this,  &AGDrawObject::LibIsGrouped,
  "IsGrouped", fpNone, "Returns true if the object is in a group") );
  lib.RegisterFunction<AGDrawObject>( new TFunction<AGDrawObject>(this,  &AGDrawObject::LibIsSelected,
  "IsSelected", fpNone, "Returns true if the object is selected") );
  lib.RegisterFunction<AGDrawObject>( new TFunction<AGDrawObject>(this,  &AGDrawObject::LibGetName,
  "GetName", fpNone, "Returns object collection name") );
}
//..............................................................................
