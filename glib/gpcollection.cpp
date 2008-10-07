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

UseGlNamespace();
//..............................................................................
//..............................................................................
TGPCollection::TGPCollection( TGlRender *P)  {
  FParent = P;
}
//..............................................................................
TGPCollection::~TGPCollection()  {  }
//..............................................................................
void TGPCollection::ClearPrimitives()  {
  TGlPrimitive *GlP;
  for( int i=0; i < FParent->PrimitiveCount(); i++ )
    FParent->Primitive(i)->SetTag(-1);
  for( int i=0; i < PrimitiveCount(); i++ )  {
    GlP = Primitive(i);
    GlP->SetTag(0);
  }
  FParent->RemovePrimitive(0);
  Primitives.Clear();
}
//..............................................................................
TGlPrimitive* TGPCollection::NewPrimitive(const olxstr &Name)  {
  TGlPrimitive *GlP = FParent->NewPrimitive();
  GlP->Name(Name);
  AddPrimitive(GlP);
  GlP->ParentCollection(this);
  return GlP;
};
//..............................................................................
TGlPrimitive *TGPCollection::PrimitiveByName(const olxstr &Name)  {
  for( int i = 0; i < Primitives.Count(); i++ )
    if( Primitives[i]->Name() == Name )  return Primitives[i];
  return NULL;
}
//..............................................................................
void TGPCollection::Draw()  {
  for( int i = 0; i < Primitives.Count(); i++ )
    Primitives[i]->Draw();
}
//..............................................................................
void TGPCollection::AddObject(AGDrawObject *Obj)  {
  GObjects.Add(Obj);
  Obj->Primitives( this );
  FParent->AddGObject( Obj );
};
//..............................................................................
void TGPCollection::ListParams(TStrList &List, TGlPrimitive *Primitive)  {
  if( GObjects.Count() != 0 )
    Object(0)->ListParams(List, Primitive);
}
bool TGPCollection::ContainsPrimitive(TGlPrimitive *GlP)  {
  if( Primitives.IndexOf(GlP) >= 0 )  return true;
  return false;
}
//..............................................................................
void TGPCollection::Style(TGraphicsStyle *S)  {
  FStyle = S;
  if( !S )  return;
  TGlMaterial *GlM;
  TGlPrimitive *GlP;
  // update materials of primitives & filling the style
  for( int i=0; i < PrimitiveCount(); i++ )  {
    GlP = Primitive(i);
    GlM = (TGlMaterial*)GlP->GetProperties();
    if( S->Material(GlP->Name())->Mark() )
      S->PrimitiveMaterial(GlP->Name(), *GlM);
  }
}

