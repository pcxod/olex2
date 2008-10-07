//----------------------------------------------------------------------------//
// TGlGroup - a group of drawing objects
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "glgroup.h"
#include "glrender.h"
#include "gpcollection.h"
#include "styles.h"

UseGlNamespace()
//..............................................................................
//..............................................................................

TGlGroup::TGlGroup(const olxstr& collectionName, TGlRender *P) :
  AGDrawObject(collectionName)  {

  FGlM = NULL;
  Flags |= sgdoGroup;
  AGDrawObject::FParent = P;
  DefaultColor = true;
}
//..............................................................................
void TGlGroup::Create(const olxstr& cName)  {
  if( !cName.IsEmpty() )  SetCollectionName(cName);

  TGPCollection *GPC = FParent->FindCollection( GetCollectionName() );
  if( GPC == NULL )  
    GPC = FParent->NewCollection( GetCollectionName() );
  else  {
    TGraphicsStyle *GS = GPC->Style();
    FGlM = const_cast<TGlMaterial*>(GS->Material("mat"));
    GPC->AddObject(this);
    return;
  }
  GPC->AddObject(this);
  TGraphicsStyle *GS = GPC->Style();
  FGlM = const_cast<TGlMaterial*>(GS->Material("mat"));
  DefaultColor = FGlM->Mark();
  if( FGlM->Mark() )  {
    if( ParentGroup() != NULL )  {
      FGlM->SetFlags( sglmAmbientF|sglmDiffuseF|sglmSpecularF|sglmShininessF );
      FGlM->ShininessF = 128;
      FGlM->AmbientF = 0xff0fff0f;
      FGlM->DiffuseF = 0xff00f0ff;
    }
    else  {
      FGlM->SetFlags( sglmAmbientF|sglmDiffuseF|sglmSpecularF|sglmShininessF|
                      sglmAmbientB|sglmDiffuseB|sglmSpecularB|sglmShininessB|sglmTransparent );
      FGlM->ShininessF = 128;
      FGlM->AmbientF = 0x7f00ff00;
      FGlM->DiffuseF = 0x7f0000ff;
      FGlM->ShininessB = 128;
      FGlM->AmbientB = 0x7f00ff00;
      FGlM->DiffuseB = 0x7f0000ff;
    }
  }
}
//..............................................................................
TGlGroup::~TGlGroup()  {
  if( ParentGroup() != NULL )
    ParentGroup()->Remove( (AGDrawObject*)this);
  Clear();
} 
//..............................................................................
void TGlGroup::Clear()  {
  for( int i=0; i < FObjects.Count(); i++ )  {
    FObjects[i]->ParentGroup(NULL);
    FObjects[i]->Selected(false);
  }
  FObjects.Clear();
}
//..............................................................................
void TGlGroup::Remove(AGDrawObject *G)  {
  FObjects.Remove(G);
  G->ParentGroup(NULL);
  G->Selected(false);
}
//..............................................................................
void TGlGroup::RemoveDeleted()  {
  for( int i=0; i < FObjects.Count(); i++ )  {
    if( FObjects[i]->Deleted() )  {
      FObjects[i]->ParentGroup(NULL);
      FObjects[i]->Selected(false);
      FObjects[i] = NULL;
    }
  }
  FObjects.Pack();
}
//..............................................................................
bool TGlGroup::Add(AGDrawObject *G)  {
  if( G == this )
    throw TInvalidArgumentException(__OlxSourceInfo, "cannot add itself");
  TGlGroup *GlG = Parent()->FindObjectGroup(G);
  if( GlG != NULL )  G = GlG;
  int i = FObjects.IndexOf(G);
  if( i == -1 )  {
    FObjects.Add(G);
    G->ParentGroup(this);
    return true;
  }
  else  {
    FObjects.Delete(i);
    G->ParentGroup(NULL);
    return false;
  }
}
//..............................................................................
void TGlGroup::Visible(bool On)  {
  for( int i=0; i < FObjects.Count(); i++ )
    FObjects[i]->Visible(On); 
}
//..............................................................................
void TGlGroup::Selected(bool On)  {
  for( int i=0; i < FObjects.Count(); i++ )
    FObjects[i]->Selected(On);
  AGDrawObject::Selected(On);
}
//..............................................................................
void TGlGroup::InitMaterial() const {
  if( ParentGroup() != NULL )
    ParentGroup()->InitMaterial();
  else
    FGlM->Init();
}
//..............................................................................
void TGlGroup::Draw(bool SelectPrimitives, bool SelectObjects) const  {
//  if( SelectObjects )     glLoadName(this->Tag());
  if( !SelectPrimitives && !SelectObjects )
      InitMaterial();

  for( int i=0; i < FObjects.Count(); i++ )  {
    AGDrawObject* G = FObjects[i];
    if( !G->Visible() )  continue;
    if( G->Deleted() )  continue;
    if( G->Group() )    { G->Draw();  continue; }
    int pc = G->Primitives()->PrimitiveCount();
    for( int j=0; j < pc; j++ )  {
      TGlPrimitive* GlP = G->Primitives()->Primitive(j);
      if( SelectObjects )     glLoadName(G->GetTag());
      if( SelectPrimitives )  glLoadName(GlP->GetTag());
      glPushMatrix();
      if( G->Orient(GlP) )  {  glPopMatrix();  continue;  }
      GlP->Draw();
      glPopMatrix();
    }
  }
}
//..............................................................................
bool TGlGroup::OnMouseDown(const IEObject *Sender, const TMouseData *Data)  {
  for( int i=0; i < FObjects.Count(); i++ )
    FObjects[i]->OnMouseDown(Sender, Data);
  return true;
}
//..............................................................................
bool TGlGroup::OnMouseUp(const IEObject *Sender, const TMouseData *Data)  {
  for( int i=0; i < FObjects.Count(); i++ )
    FObjects[i]->OnMouseUp(Sender, Data);
  return true;
}
//..............................................................................
bool TGlGroup::OnMouseMove(const IEObject *Sender, const TMouseData *Data)  {
  for( int i=0; i < FObjects.Count(); i++ )
    FObjects[i]->OnMouseMove(Sender, Data);
  return true;
}
//..............................................................................
void TGlGroup::GlM(const TGlMaterial& G)  {
  FGlM = Primitives()->Style()->PrimitiveMaterial("mat", G);
}
//..............................................................................

