/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "glgroup.h"
#include "glrender.h"
#include "glmouse.h"
#include "gpcollection.h"
#include "styles.h"
#include "glprimitive.h"
UseGlNamespace()

TGlGroup::TGlGroup(TGlRenderer& R, const olxstr& collectionName) :
  AGDrawObject(R, collectionName)
{
  SetGroupable(true);
  SetTypeGroup();
  DefaultColor = true;
  Blended = false;
}
//..............................................................................
void TGlGroup::Create(const olxstr& cName) {
  if (!cName.IsEmpty()) {
    SetCollectionName(cName);
  }
  TGPCollection& GPC = Parent.FindOrCreateCollection(GetCollectionName());
  GPC.AddObject(*this);
  TGraphicsStyle& GS = GPC.GetStyle();
  if (!cName.IsEmpty()) {
    GlM.SetFlags(sglmAmbientF | sglmDiffuseF | sglmSpecularF | sglmShininessF);
    GlM.ShininessF = 128;
    GlM.AmbientF = 0xff0fff0f;
    GlM.DiffuseF = 0xff00f0ff;
  }
  else {
    GlM.SetFlags(sglmAmbientF | sglmDiffuseF | sglmSpecularF | sglmShininessF | sglmTransparent);
    GlM.ShininessF = 128;
    GlM.AmbientF = 0x7f00ff00;
    GlM.DiffuseF = 0x7f0000ff;
    GlM.ShininessB = 128;
    GlM.AmbientB = 0x7f00ff00;
    GlM.DiffuseB = 0x7f0000ff;
  }
  Blended = GS.GetParam("Blended", Blended, true).ToBool();
  DefaultColor = (GS.IndexOfMaterial("mat") == InvalidIndex);
  GlM = GS.GetMaterial("mat", GlM);
  BlendColor = GetBlendColor();
}
//..............................................................................
TGlGroup::~TGlGroup() {
  if (GetParentGroup() != 0) {
    GetParentGroup()->Remove(*this);
  }
  Clear();
}
//..............................................................................
void TGlGroup::Clear()  {
  Objects.ForEach(ObjectReleaser());
  GlM.SetIdentityDraw(false);  // most objects are 'normal'
  Objects.Clear();
}
//..............................................................................
bool TGlGroup::Remove(AGDrawObject& G)  {
  if (Objects.Remove(G)) {
    ObjectReleaser::OnItem(G, 0);  // dummy 0 arg...
    return true;
  }
  return false;
}
//..............................................................................
void TGlGroup::RemoveHidden()  {
  Objects.Pack(AGDrawObject::FlagsAnalyserEx<ObjectReleaser>(
    sgdoHidden, ObjectReleaser()));
}
//..............................................................................
bool TGlGroup::Add(AGDrawObject& GO, bool remove) {
  AGDrawObject* go = &GO;
  if (go == this || !GO.IsSelectable()) {
    return false;
  }
  TGlGroup *GlG = Parent.FindObjectGroup(GO);
  if (GlG != 0) {
    go = GlG;
  }
  if (go == this) {
    return false;
  }
  const size_t i = Objects.IndexOf(go);
  if (i == InvalidIndex) {
    /* GO.Primitives can be NULL when hierarchy of groups being restored before
    creating them check the compatibility of the selection
    */
    if (GO.HasPrimitives() && GO.GetPrimitives().PrimitiveCount() != 0) {
      bool idd = GO.GetPrimitives().GetPrimitive(0).GetProperties()
        .IsIdentityDraw();
      if (Objects.IsEmpty()) {
        GlM.SetIdentityDraw(idd);
      }
      else if (GlM.IsIdentityDraw() != idd) {
        return false;
      }
    }
    go->SetGrouped(true);
    Objects.Add(go);
    go->SetParentGroup(this);
    return true;
  }
  else {
    if (remove) {
      Objects.Delete(i);
      go->SetParentGroup(NULL);
      go->SetGrouped(false);
      return false;
    }
    return true;
  }
  return false;
}
//..............................................................................
void TGlGroup::SetVisible(bool On) {
  for (size_t i = 0; i < Objects.Count(); i++) {
    Objects[i]->SetVisible(On);
  }
  AGDrawObject::SetVisible(On);
}
//..............................................................................
void TGlGroup::SetSelected(bool On) {
  for (size_t i = 0; i < Objects.Count(); i++) {
    Objects[i]->SetSelected(On);
  }
  AGDrawObject::SetSelected(On);
}
//..............................................................................
void TGlGroup::InitMaterial() const {
  if (GetParentGroup() != 0) {
    GetParentGroup()->InitMaterial();
  }
  else {
    GlM.Init(Parent.ForcePlain());
  }
}
//..............................................................................
void TGlGroup::DoDraw(bool SelectPrimitives, bool SelectObjects) const {
  if (!SelectPrimitives && !SelectObjects && !Blended) {
    InitMaterial();
  }
  if (Blended) {
    BlendMaterialDraw(SelectPrimitives, SelectObjects);
  }
  else {
    OverrideMaterialDraw(SelectPrimitives, SelectObjects);
  }
}
//..............................................................................
void TGlGroup::OverrideMaterialDraw(bool SelectPrimitives,
  bool SelectObjects) const
{
  for (size_t i = 0; i < Count(); i++) {
    AGDrawObject& G = GetObject(i);
    if (G.MaskFlags(sgdoHidden) != 0) {
      continue;
    }
    if (G.IsGroup()) {
      TGlGroup* group = dynamic_cast<TGlGroup*>(&G);
      if (group != 0) {
        group->Draw(SelectPrimitives, SelectObjects);
        continue;
      }
    }
    const size_t pc = G.GetPrimitives().PrimitiveCount();
    for (size_t j = 0; j < pc; j++) {
      TGlPrimitive& GlP = G.GetPrimitives().GetPrimitive(j);
      Parent.HandleSelection(G, GlP, SelectObjects, SelectPrimitives);
      olx_gl::pushMatrix();
      if (G.Orient(GlP)) {
        olx_gl::popMatrix();
        continue;
      }
      GlP.Draw();
      olx_gl::popMatrix();
    }
  }
}
//..............................................................................
TGlOption TGlGroup::GetBlendColor() const {
  TGlOption ta = GlM.AmbientF;
  if (GetParentGroup() != 0) {
    TGlOption& pa = GetParentGroup()->GlM.AmbientF;
    const float m1 = pa[3];
    const float m2 = (1.0f - m1);
    ta[0] = pa[0] * m1 + ta[0] * m2;
    ta[1] = pa[1] * m1 + ta[1] * m2;
    ta[2] = pa[2] * m1 + ta[2] * m2;
    ta[3] = (m1 + m2) / 2;
    if (ta[3] > 1.0f) {
      ta[3] = 1.0f;
    }
  }
  return ta;
}
//..............................................................................
bool TGlGroup::CheckBlended() const {
  if (GetParentGroup() != 0) {
    return GetParentGroup()->CheckBlended();
  }
  return IsBlended();
}
//..............................................................................
void TGlGroup::BlendMaterialDraw(bool SelectPrimitives,
  bool SelectObjects) const
{
  if (!CheckBlended()) {
    OverrideMaterialDraw(SelectPrimitives, SelectObjects);
    return;
  }
  for (size_t i=0; i < Count(); i++) {
    AGDrawObject& G = GetObject(i);
    if (G.MaskFlags(sgdoHidden) != 0) {
      continue;
    }
    if (G.IsGroup()) {
      TGlGroup* group = dynamic_cast<TGlGroup*>(&G);
      if (group != 0) {
        group->Draw(SelectPrimitives, SelectObjects);
        continue;
      }
    }
    bool Select = (SelectPrimitives|SelectObjects);
    const size_t pc = G.GetPrimitives().PrimitiveCount();
    for (size_t j=0; j < pc; j++) {
      TGlPrimitive& GlP = G.GetPrimitives().GetPrimitive(j);
      if (!Select) {
        TGlMaterial glm = GlP.GetProperties();
        const float m1 = BlendColor[3];
        const float m2 = (1.0f-BlendColor[3]);
        glm.AmbientF[0] = BlendColor[0]*m1 + glm.AmbientF[0]*m2;
        glm.AmbientF[1] = BlendColor[1]*m1 + glm.AmbientF[1]*m2;
        glm.AmbientF[2] = BlendColor[2]*m1 + glm.AmbientF[2]*m2;
        glm.AmbientF[3] = 1.0f;
        if (GlM.IsTransparent() && GlM.HasDiffuseF()) {
          glm.SetTransparent(true);
          glm.SetDiffuseF(true);
          glm.DiffuseF = GlM.DiffuseF;
        }
        glm.Init(false);
      }
      Parent.HandleSelection(G, GlP, SelectObjects, SelectPrimitives);
      olx_gl::pushMatrix();
      if (G.Orient(GlP)) {
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
  BlendColor = GetBlendColor();
}
//..............................................................................
bool TGlGroup::TryToGroup(AGDObjList& ungroupable) {
  size_t groupable_cnt = 0;
  for (size_t i = 0; i < Objects.Count(); i++)
    if (Objects[i]->IsGroupable()) {
      groupable_cnt++;
    }
  if (groupable_cnt < 2) {
    return false;
  }
  if (groupable_cnt == Objects.Count()) {
    return true;
  }
  ungroupable.SetCapacity(ungroupable.Count() + Objects.Count() - groupable_cnt);
  for (size_t i = 0; i < Objects.Count(); i++) {
    if (!Objects[i]->IsGroupable()) {
      ungroupable.Add(Objects[i])->SetParentGroup(NULL);
      Objects.Delete(i--);
    }
  }
  return true;
}
//..............................................................................
bool TGlGroup::OnMouseDown(const IOlxObject *Sender, const struct TMouseData& Data) {
  bool res = false;
  for (size_t i = 0; i < Objects.Count(); i++) {
    if (Objects[i]->OnMouseDown(Sender, Data)) {
      res = true;
    }
  }
  return res;
}
//..............................................................................
bool TGlGroup::OnMouseUp(const IOlxObject *Sender, const struct TMouseData& Data) {
  bool res = false;
  for (size_t i = 0; i < Objects.Count(); i++) {
    if (Objects[i]->OnMouseUp(Sender, Data)) {
      res = true;
    }
  }
  return res;
}
//..............................................................................
bool TGlGroup::OnMouseMove(const IOlxObject *Sender, const struct TMouseData& Data) {
  bool res = false;
  for (size_t i = 0; i < Objects.Count(); i++) {
    if (Objects[i]->OnMouseMove(Sender, Data)) {
      res = true;
    }
  }
  return res;
}
//..............................................................................
void TGlGroup::SetBlended(bool v) {
  Blended = v;
  GetPrimitives().GetStyle().SetParam("Blended", v, true);
}
//..............................................................................
TGlMaterial TGlGroup::GetActualMaterial(const TGlMaterial &m) const {
  TGlMaterial glm;
  if (Blended) {
    glm = m;
    const float m1 = BlendColor[3];
    const float m2 = (1.0f-BlendColor[3]);
    glm.AmbientF[0] = BlendColor[0]*m1 + m.AmbientF[0]*m2;
    glm.AmbientF[1] = BlendColor[1]*m1 + m.AmbientF[1]*m2;
    glm.AmbientF[2] = BlendColor[2]*m1 + m.AmbientF[2]*m2;
    glm.AmbientF[3] = 1.0f;
  }
  else {
    glm = GlM;
  }
  return (ParentGroup != 0) ? ParentGroup->GetActualMaterial(glm) : glm;
}
//..............................................................................
