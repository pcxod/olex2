//---------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "gpcollection.h"
#include "glrender.h"
#include "gdrawobject.h"
#include "styles.h"
#include "glprimitive.h"

UseGlNamespace();
//..............................................................................
void TGPCollection::ClearPrimitives()  {
  for( int i=0; i < Parent.PrimitiveCount(); i++ )
    Parent.GetPrimitive(i).SetTag(-1);
  for( int i=0; i < PrimitiveCount(); i++ )
    GetPrimitive(i).SetTag(0);
  Parent.RemovePrimitive(0);
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
  for( int i = 0; i < Primitives.Count(); i++ )
    if( Primitives[i]->GetName() == Name )  
      return Primitives[i];
  return NULL;
}
//..............................................................................
void TGPCollection::Draw()  {
  for( int i = 0; i < Primitives.Count(); i++ )
    Primitives[i]->Draw();
}
//..............................................................................
void TGPCollection::AddObject(AGDrawObject& Obj)  {
  GObjects.Add(&Obj);
  Obj.SetPrimitives( *this );
  Parent.AddObject( Obj );
};
//..............................................................................
void TGPCollection::ListParams(TStrList &List, TGlPrimitive *Primitive)  {
  if( !GObjects.IsEmpty() )
    GetObject(0).ListParams(List, Primitive);
}
bool TGPCollection::ContainsPrimitive(TGlPrimitive& GlP)  {
  return Primitives.IndexOf(&GlP) != -1;
}
//..............................................................................
void TGPCollection::SetStyle(TGraphicsStyle *S)  {
  Style = S;
  if( S == NULL )  return;
  // update materials of primitives & filling the style
  for( int i=0; i < PrimitiveCount(); i++ )  {
    TGlPrimitive& GlP = GetPrimitive(i);
    if( S->IndexOfMaterial(GlP.GetName()) == -1 )
      S->SetMaterial(GlP.GetName(), GlP.GetProperties());
  }
}

