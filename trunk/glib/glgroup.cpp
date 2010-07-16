//----------------------------------------------------------------------------//
// TGlGroup - a group of drawing objects
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//
#include "glgroup.h"
#include "glrender.h"
#include "glmouse.h"
#include "gpcollection.h"
#include "styles.h"
#include "glprimitive.h"

UseGlNamespace()
//..............................................................................
TGlGroup::TGlGroup(TGlRenderer& R, const olxstr& collectionName) :
  AGDrawObject(R, collectionName)
{
  SetGroupable(true);
  Flags |= sgdoGroup;
  DefaultColor = true;
}
//..............................................................................
void TGlGroup::Create(const olxstr& cName, const ACreationParams* cpar)  {
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);

  TGPCollection& GPC = Parent.FindOrCreateCollection(GetCollectionName());
  GPC.AddObject(*this);
  TGraphicsStyle& GS = GPC.GetStyle();
  if( !cName.IsEmpty() )  {
    GlM.SetFlags( sglmAmbientF|sglmDiffuseF|sglmSpecularF|sglmShininessF);
    GlM.ShininessF = 128;
    GlM.AmbientF = 0xff0fff0f;
    GlM.DiffuseF = 0xff00f0ff;
  }
  else  {
    GlM.SetFlags(sglmAmbientF|sglmDiffuseF|sglmSpecularF|sglmShininessF|sglmTransparent);
    GlM.ShininessF = 128;
    GlM.AmbientF = 0x7f00ff00;
    GlM.DiffuseF = 0x7f0000ff;
    GlM.ShininessB = 128;
    GlM.AmbientB = 0x7f00ff00;
    GlM.DiffuseB = 0x7f0000ff;
  }
  DefaultColor = (GS.IndexOfMaterial("mat") == InvalidIndex);
  GlM = GS.GetMaterial("mat", GlM);
}
//..............................................................................
TGlGroup::~TGlGroup()  {
  if( GetParentGroup() != NULL )
    GetParentGroup()->Remove(*this);
  Clear();
} 
//..............................................................................
void TGlGroup::Clear()  {
  for( size_t i=0; i < FObjects.Count(); i++ )  {
    FObjects[i]->SetParentGroup(NULL);
    FObjects[i]->SetGrouped(false);
  }
  FObjects.Clear();
}
//..............................................................................
void TGlGroup::Remove(AGDrawObject& G)  {
  FObjects.Remove(G);
  G.SetParentGroup(NULL);
  G.SetGrouped(false);
}
//..............................................................................
void TGlGroup::RemoveDeleted()  {
  for( size_t i=0; i < FObjects.Count(); i++ )  {
    if( FObjects[i]->IsDeleted() )  {
      FObjects[i]->SetParentGroup(NULL);
      FObjects[i]->SetGrouped(false);
      FObjects[i] = NULL;
    }
  }
  FObjects.Pack();
}
//..............................................................................
bool TGlGroup::Add(AGDrawObject& GO, bool remove)  {
  AGDrawObject* go = &GO;
  if( go == this )
    throw TInvalidArgumentException(__OlxSourceInfo, "cannot add itself");
  TGlGroup *GlG = Parent.FindObjectGroup(GO);
  if( GlG != NULL )  
    go = GlG;
  const size_t i = FObjects.IndexOf(go);
  if( i == InvalidIndex )  {
    FObjects.Add(go)->SetGrouped(true);
    go->SetParentGroup(this);
    return true;
  }
  else if( remove )  {
    FObjects.Delete(i);
    go->SetParentGroup(NULL);
    go->SetGrouped(false);
  }
  return false;
}
//..............................................................................
void TGlGroup::SetVisible(bool On)  {
  for( size_t i=0; i < FObjects.Count(); i++ )
    FObjects[i]->SetVisible(On); 
  AGDrawObject::SetVisible(On);
}
//..............................................................................
void TGlGroup::SetSelected(bool On)  {
  for( size_t i=0; i < FObjects.Count(); i++ )
    FObjects[i]->SetSelected(On);
  AGDrawObject::SetSelected(On);
}
//..............................................................................
void TGlGroup::InitMaterial() const {
  if( GetParentGroup() != NULL )
    GetParentGroup()->InitMaterial();
  else
    GlM.Init(Parent.IsColorStereo());
}
//..............................................................................
void TGlGroup::DoDraw(bool SelectPrimitives, bool SelectObjects) const {
  if( !SelectPrimitives && !SelectObjects )
    InitMaterial();

  for( size_t i=0; i < FObjects.Count(); i++ )  {
    AGDrawObject* G = FObjects[i];
    if( !G->IsVisible() )  continue;
    if( G->IsDeleted() )  continue;
    if( G->IsGroup() )    {
      TGlGroup* group = dynamic_cast<TGlGroup*>(G);
      if( group != NULL )  {
        group->Draw(SelectPrimitives, SelectObjects);
        continue;
      }
    }
    const size_t pc = G->GetPrimitives().PrimitiveCount();
    for( size_t j=0; j < pc; j++ )  {
      TGlPrimitive& GlP = G->GetPrimitives().GetPrimitive(j);
      if( SelectObjects )     olx_gl::loadName((GLuint)G->GetTag());
      if( SelectPrimitives )  olx_gl::loadName((GLuint)GlP.GetTag());
      olx_gl::pushMatrix();
      if( G->Orient(GlP) )  {
        olx_gl::popMatrix();
        continue;
      }
      GlP.Draw();
      olx_gl::popMatrix();
    }
  }
}
//..............................................................................
void TGlGroup::SetGlM(const TGlMaterial& m)  {
  GlM = GetPrimitives().GetStyle().SetMaterial("mat", m);
}
//..............................................................................
bool TGlGroup::TryToGroup(TPtrList<AGDrawObject>& ungroupable)  {
  size_t groupable_cnt=0;
  for( size_t i=0; i < FObjects.Count(); i++ )
    if( FObjects[i]->IsGroupable() )
      groupable_cnt++;
  if( groupable_cnt < 2 )  return false;
  if( groupable_cnt == FObjects.Count() )
    return true;
  ungroupable.SetCapacity(ungroupable.Count() + FObjects.Count() - groupable_cnt);
  for( size_t i=0; i < FObjects.Count(); i++ )  {
    if( !FObjects[i]->IsGroupable() )  {
      ungroupable.Add(FObjects[i])->SetParentGroup(NULL);
      FObjects[i] = NULL;
    }
  }
  FObjects.Pack();
  return true;
}
//..............................................................................
bool TGlGroup::OnMouseDown(const IEObject *Sender, const struct TMouseData& Data)  {
  bool res = false;
  for( size_t i=0; i < FObjects.Count(); i++ )
    if( FObjects[i]->OnMouseDown(Sender, Data) )
      res = true;
  return res;
}
//..............................................................................
bool TGlGroup::OnMouseUp(const IEObject *Sender, const struct TMouseData& Data)  {
  bool res = false;
  for( size_t i=0; i < FObjects.Count(); i++ )
    if( FObjects[i]->OnMouseUp(Sender, Data) )
      res = true;
  return res;
}
//..............................................................................
bool TGlGroup::OnMouseMove(const IEObject *Sender, const struct TMouseData& Data)  {
  bool res = false;
  for( size_t i=0; i < FObjects.Count(); i++ )
    if( FObjects[i]->OnMouseMove(Sender, Data) )
      res = true;
  return res;
}
//..............................................................................
