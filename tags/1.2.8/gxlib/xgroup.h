/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_glx_xgroup_H
#define __olx_glx_xgroup_H
#include "gxbase.h"
#include "glgroup.h"
#include "xatom.h"
#include "xbond.h"
BeginGxlNamespace()

/* The group now updates the atom coordinates in real time to make the process
more transparent, however, it could use the TEBasis in the following way:
  virtual bool DoTranslate(const vec3d& t)  {  Basis.Translate(t);  return true;  }
  virtual bool DoRotate(const vec3d& vec, double angle)  {
    Basis.Rotate(vec, angle);
    return true;
  }
  virtual void DoDraw(bool SelectPrimitives, bool SelectObjects) const {
    olx_gl::translate(RotationCenter);  // + Basis.GetCenter() - in the next call to orient...
    olx_gl::orient(Basis);
    olx_gl::translate(-RotationCenter);
    ...
  }
*/
class TXGroup : public TGlGroup, public AGlMouseHandler {
  vec3d RotationCenter, RotationDir;
  const vec3d_alist* original_crds;
  TXAtomPList Atoms;
  TXBondPList Bonds;
  AGDrawObject *rotation_anchor;
  double AngleInc, AngleAcc;
protected:
  virtual void DoDraw(bool SelectPrimitives, bool SelectObjects) const {
    if (GetParentGroup() != NULL) {  // is inside a group?
      TGlGroup::DoDraw(SelectPrimitives, SelectObjects);
      return;
    }
    for (int ditr=0; ditr < 2; ditr++) {
      if (ditr == 0) {
        if (original_crds != NULL && original_crds->Count() == Atoms.Count()) {
          for (size_t i=0; i < Atoms.Count(); i++)
            olx_swap((*original_crds)[i], Atoms[i]->crd());
          for (size_t i=0; i < Bonds.Count(); i++)
            Bonds[i]->Update();
        }
        else
          continue;
      }
      else if (original_crds != NULL && original_crds->Count() == Atoms.Count()) {
        for (size_t i=0; i < Atoms.Count(); i++)
          olx_swap((*original_crds)[i], Atoms[i]->crd());
        for (size_t i=0; i < Bonds.Count(); i++)
          Bonds[i]->Update();
      }
      for (size_t i=0; i < TGlGroup::Count(); i++) {
        AGDrawObject& G = GetObject(i);
        if (!G.IsVisible()) continue;
        if (G.IsGroup()) {
          TGlGroup* group = dynamic_cast<TGlGroup*>(&G);
          if (group != NULL) {
            group->Draw(SelectPrimitives, SelectObjects);
            continue;
          }
        }
        bool Select = (SelectObjects|SelectPrimitives);
        const size_t pc = G.GetPrimitives().PrimitiveCount();
        for (size_t j=0; j < pc; j++) {
          TGlPrimitive& GlP = G.GetPrimitives().GetPrimitive(j);
          if (!Select) {
            bool initialised=false;
            if (&G == rotation_anchor) {
              if (EsdlInstanceOf(G, TXAtom)) {
                TXAtom &a = (TXAtom &)G;
                if (a.crd().Equals(RotationCenter, 1e-3))
                  initialised = true;
              }
              else if (EsdlInstanceOf(G, TXBond)) {
                TXBond &b = (TXBond &)G;
                if (RotationCenter.Equals((b.A().crd()+b.B().crd())/2, 1e-3))
                  initialised = true;
              }
            }
            if (initialised) {
              TGlGroup::GetGlM().Init(false);
            }
            else {
              TGlMaterial glm = GlP.GetProperties();
              glm.SetFlags(glm.GetFlags()|sglmShininessF|sglmSpecularF);
              glm.AmbientF *= 0.75;
              if (ditr == 1)
                glm.AmbientF = glm.AmbientF.GetRGB() | 0x007070;
              else
                glm.AmbientF = glm.AmbientF.GetRGB() | 0x700070;
              glm.ShininessF = 32;
              glm.SpecularF = 0xff00;
              glm.Init(false);
            }
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
  }
  virtual bool DoTranslate(const vec3d& t) {
    for (size_t i=0; i < Atoms.Count(); i++)
      Atoms[i]->crd() += t;
    RotationCenter += t;
    for (size_t i=0; i < Count(); i++)
      GetObject(i).Update();
    return true;
  }
  virtual bool DoRotate(const vec3d& vec, double angle) {
    if (AngleInc != 0) {
      if (olx_abs(AngleAcc += angle) < AngleInc)
        return true;
      angle = AngleInc*olx_sign(AngleAcc);
      AngleAcc = 0;
    }
    mat3d m;
    if (RotationDir.IsNull())
      olx_create_rotation_matrix(m, vec, cos(angle), sin(angle));
    else
      olx_create_rotation_matrix(m, RotationDir, cos(angle), sin(angle));
    for (size_t i=0; i < Atoms.Count(); i++)
      Atoms[i]->crd() = (Atoms[i]->crd()-RotationCenter)*m+RotationCenter;
    for (size_t i=0; i < Bonds.Count(); i++)
      Bonds[i]->Update();
    return true;
  }
  virtual bool DoZoom(double, bool)  {  return false;  }
  virtual const TGlRenderer& DoGetRenderer() const {  return GetParent();  }
  bool OnMouseDown(const IOlxObject *Sender, const TMouseData& Data)  {
    return GetHandler().OnMouseDown(*this, Data);
  }
  bool OnMouseUp(const IOlxObject *Sender, const TMouseData& Data)  {
    if (Data.Button == smbRight) {
      if (Data.Object != NULL)  {
        if (EsdlInstanceOf(*Data.Object, TXAtom)) {
          if (rotation_anchor == Data.Object) {
            rotation_anchor = NULL;
            UpdateRotationCenter();
          }
          else {
            rotation_anchor = Data.Object;
            RotationCenter = ((TXAtom*)Data.Object)->crd();
            RotationDir.Null();
          }
        }
        else if (EsdlInstanceOf(*Data.Object, TXBond)) {
          if (rotation_anchor == Data.Object) {
            rotation_anchor = NULL;
            RotationDir.Null();
            UpdateRotationCenter();
          }
          else {
            rotation_anchor = Data.Object;
            TXBond* xb = (TXBond*)Data.Object;
            RotationCenter = (xb->A().crd() + xb->B().crd())/2;
            RotationDir = (xb->B().crd() - xb->A().crd()).Normalise();
          }
        }
        return true;
      }
    }
    return GetHandler().OnMouseUp(*this, Data);
  }
  bool OnMouseMove(const IOlxObject *Sender, const TMouseData& Data)  {
    return GetHandler().OnMouseMove(*this, Data);
  }
  bool OnDblClick(const IOlxObject *Sender, const TMouseData& Data)  {
    return GetHandler().OnDblClick(*this, Data);
  }
public:
  TXGroup(TGlRenderer& R, const olxstr& colName) :
    TGlGroup(R, colName),
    original_crds(NULL),
    rotation_anchor(NULL),
    AngleInc(0), AngleAcc(0)
  {
    SetMoveable(true);
    SetRoteable(true);
  }
  void AddAtoms(const TPtrList<TXAtom>& atoms)  {
    original_crds = NULL;
    TGlGroup::AddObjects(atoms);
    Atoms.SetCapacity(atoms.Count());
    for (size_t i=0; i < atoms.Count(); i++) {
      Bonds.AddAll(Atoms.Add(atoms[i])->GetBonds(),
        StaticCastAccessor<TXBond>());
    }
    Bonds.ForEach(ACollectionItem::IndexTagSetter());
    Bonds.Pack(
      olx_alg::olx_or(
        olx_alg::olx_not(ACollectionItem::IndexTagAnalyser()),
        AGDrawObject::FlagsAnalyser(sgdoHidden)
      )
    );
    TGlGroup::AddObjects(Bonds);
    UpdateRotationCenter();
  }
  // beware the life time of the objects here!
  void SetOrgiginalCrds(const vec3d_alist& vec)  {  original_crds = &vec;  }
  void Clear()  {
    TGlGroup::Clear();
    Atoms.Clear();
    Bonds.Clear();
    original_crds = NULL;
  }
  void Update() {
    original_crds = NULL;
    Atoms.Clear();
    Bonds.Clear();
    for (size_t i=0; i < TGlGroup::Count(); i++) {
      AGDrawObject& G = GetObject(i);
      if (EsdlInstanceOf(G, TXAtom))
        Atoms.Add((TXAtom&)G);
      else if (EsdlInstanceOf(G, TXBond))
        Bonds.Add((TXBond&)G);
    }
  }
  void UpdateRotationCenter() {
    RotationCenter.Null();
    for (size_t i=0; i < Atoms.Count(); i++)
      RotationCenter += Atoms[i]->crd();
    RotationCenter /= Atoms.Count();
  }
  const vec3d& GetRotationCenter() const {  return RotationCenter;  }
  DefPropP(double, AngleInc)
};

EndGxlNamespace()
#endif
