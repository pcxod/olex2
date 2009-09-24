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
#include "glprimitive.h"

UseGlNamespace()
//..............................................................................
//..............................................................................

TGlGroup::TGlGroup(TGlRenderer& R, const olxstr& collectionName) :
  AGDrawObject(R, collectionName)  
{
  FGlM = NULL;
  Flags |= sgdoGroup;
  DefaultColor = true;
}
//..............................................................................
void TGlGroup::Create(const olxstr& cName, const ACreationParams* cpar)  {
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);

  TGPCollection& GPC = Parent.FindOrCreateCollection( GetCollectionName() );
  GPC.AddObject(*this);
  if( GPC.PrimitiveCount() != 0 )  return;
  TGraphicsStyle& GS = GPC.GetStyle();
  TGlMaterial GlM;
  if( GetParentGroup() != NULL )  {
    GlM.SetFlags( sglmAmbientF|sglmDiffuseF|sglmSpecularF|sglmShininessF );
    GlM.ShininessF = 128;
    GlM.AmbientF = 0xff0fff0f;
    GlM.DiffuseF = 0xff00f0ff;
  }
  else  {
    GlM.SetFlags( sglmAmbientF|sglmDiffuseF|sglmSpecularF|sglmShininessF|
      sglmAmbientB|sglmDiffuseB|sglmSpecularB|sglmShininessB|sglmTransparent );
    GlM.ShininessF = 128;
    GlM.AmbientF = 0x7f00ff00;
    GlM.DiffuseF = 0x7f0000ff;
    GlM.ShininessB = 128;
    GlM.AmbientB = 0x7f00ff00;
    GlM.DiffuseB = 0x7f0000ff;
  }
  DefaultColor = (GS.IndexOfMaterial("mat") != -1);
  FGlM = &GS.GetMaterial("mat", GlM);
}
//..............................................................................
TGlGroup::~TGlGroup()  {
  if( GetParentGroup() != NULL )
    GetParentGroup()->Remove(*this);
  Clear();
} 
//..............................................................................
void TGlGroup::Clear()  {
  for( int i=0; i < FObjects.Count(); i++ )  {
    FObjects[i]->SetParentGroup(NULL);
    FObjects[i]->SetSelected(false);
  }
  FObjects.Clear();
}
//..............................................................................
void TGlGroup::Remove(AGDrawObject& G)  {
  FObjects.Remove(G);
  G.SetParentGroup(NULL);
  G.SetSelected(false);
}
//..............................................................................
void TGlGroup::RemoveDeleted()  {
  for( int i=0; i < FObjects.Count(); i++ )  {
    if( FObjects[i]->IsDeleted() )  {
      FObjects[i]->SetParentGroup(NULL);
      FObjects[i]->SetSelected(false);
      FObjects[i] = NULL;
    }
  }
  FObjects.Pack();
}
//..............................................................................
bool TGlGroup::Add(AGDrawObject& GO)  {
  AGDrawObject* go = &GO;
  if( go == this )
    throw TInvalidArgumentException(__OlxSourceInfo, "cannot add itself");
  TGlGroup *GlG = Parent.FindObjectGroup(GO);
  if( GlG != NULL )  
    go = GlG;
  int i = FObjects.IndexOf(go);
  if( i == -1 )  {
    FObjects.Add(go);
    go->SetParentGroup(this);
    return true;
  }
  else  {
    FObjects.Delete(i);
    go->SetParentGroup(NULL);
    return false;
  }
}
//..............................................................................
void TGlGroup::SetVisible(bool On)  {
  for( int i=0; i < FObjects.Count(); i++ )
    FObjects[i]->SetVisible(On); 
}
//..............................................................................
void TGlGroup::SetSelected(bool On)  {
  for( int i=0; i < FObjects.Count(); i++ )
    FObjects[i]->SetSelected(On);
  AGDrawObject::SetSelected(On);
}
//..............................................................................
void TGlGroup::InitMaterial() const {
  if( GetParentGroup() != NULL )
    GetParentGroup()->InitMaterial();
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
    if( !G->IsVisible() )  continue;
    if( G->IsDeleted() )  continue;
    if( G->IsGroup() )    { G->Draw();  continue; }
    const int pc = G->GetPrimitives().PrimitiveCount();
    for( int j=0; j < pc; j++ )  {
      TGlPrimitive& GlP = G->GetPrimitives().GetPrimitive(j);
      if( SelectObjects )     glLoadName(G->GetTag());
      if( SelectPrimitives )  glLoadName(GlP.GetTag());
      glPushMatrix();
      if( G->Orient(GlP) )  {  glPopMatrix();  continue;  }
      GlP.Draw();
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
  FGlM = &GetPrimitives().GetStyle().SetMaterial("mat", G);
}
//..............................................................................

