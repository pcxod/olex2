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
    GlM.SetFlags(sglmAmbientF|sglmDiffuseF|sglmSpecularF|sglmShininessF);
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
  Objects.ForEach(ObjectReleaser());
  GlM.SetIdentityDraw(false);  // most objects are 'normal'
  Objects.Clear();
}
//..............................................................................
void TGlGroup::Remove(AGDrawObject& G)  {
  Objects.Remove(&G);
  ObjectReleaser::OnItem(G);
}
//..............................................................................
void TGlGroup::RemoveDeleted()  {
  Objects.Pack(AGDrawObject::FlagsAnalyserEx<ObjectReleaser>(sgdoDeleted, ObjectReleaser()));
}
//..............................................................................
bool TGlGroup::Add(AGDrawObject& GO, bool remove)  {
  AGDrawObject* go = &GO;
  if( go == this || !GO.IsSelectable() )  return false;
  TGlGroup *GlG = Parent.FindObjectGroup(GO);
  if( GlG != NULL )
    go = GlG;
  if( go == this )
    return false;
  const size_t i = Objects.IndexOf(go);
  if( i == InvalidIndex )  {
    if( GO.GetPrimitives().PrimitiveCount() != 0 )  {  // check the compatibility of the selection
      if( Objects.IsEmpty() )
        GlM.SetIdentityDraw(GO.GetPrimitives().GetPrimitive(0).GetProperties().IsIdentityDraw());
      else if( GlM.IsIdentityDraw() != GO.GetPrimitives().GetPrimitive(0).GetProperties().IsIdentityDraw() ) 
        return false;
    }
    go->SetGrouped(true);
    Objects.Add(go);
    go->SetParentGroup(this);
    return true;
  }
  else if( remove )  {
    Objects.Delete(i);
    go->SetParentGroup(NULL);
    go->SetGrouped(false);
  }
  return false;
}
//..............................................................................
void TGlGroup::SetVisible(bool On)  {
  for( size_t i=0; i < Objects.Count(); i++ )
    Objects[i]->SetVisible(On); 
  AGDrawObject::SetVisible(On);
}
//..............................................................................
void TGlGroup::SetSelected(bool On)  {
  for( size_t i=0; i < Objects.Count(); i++ )
    Objects[i]->SetSelected(On);
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

  for( size_t i=0; i < Objects.Count(); i++ )  {
    AGDrawObject* G = Objects[i];
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
bool TGlGroup::TryToGroup(AGDObjList& ungroupable)  {
  size_t groupable_cnt=0;
  for( size_t i=0; i < Objects.Count(); i++ )
    if( Objects[i]->IsGroupable() )
      groupable_cnt++;
  if( groupable_cnt < 2 )  return false;
  if( groupable_cnt == Objects.Count() )
    return true;
  ungroupable.SetCapacity(ungroupable.Count() + Objects.Count() - groupable_cnt);
  for( size_t i=0; i < Objects.Count(); i++ )  {
    if( !Objects[i]->IsGroupable() )  {
      ungroupable.Add(Objects[i])->SetParentGroup(NULL);
      Objects.Delete(i--);
    }
  }
  return true;
}
//..............................................................................
bool TGlGroup::OnMouseDown(const IEObject *Sender, const struct TMouseData& Data)  {
  bool res = false;
  for( size_t i=0; i < Objects.Count(); i++ )
    if( Objects[i]->OnMouseDown(Sender, Data) )
      res = true;
  return res;
}
//..............................................................................
bool TGlGroup::OnMouseUp(const IEObject *Sender, const struct TMouseData& Data)  {
  bool res = false;
  for( size_t i=0; i < Objects.Count(); i++ )
    if( Objects[i]->OnMouseUp(Sender, Data) )
      res = true;
  return res;
}
//..............................................................................
bool TGlGroup::OnMouseMove(const IEObject *Sender, const struct TMouseData& Data)  {
  bool res = false;
  for( size_t i=0; i < Objects.Count(); i++ )
    if( Objects[i]->OnMouseMove(Sender, Data) )
      res = true;
  return res;
}
//..............................................................................
