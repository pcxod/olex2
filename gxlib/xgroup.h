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
  virtual bool DoTranslate(const vec3d& t) {
    Basis.Translate(t);
    return true;
  }
  virtual bool DoRotate(const vec3d& vec, double angle) {
    Basis.Rotate(vec, angle);
    return true;
  }
  virtual void DoDraw(bool SelectPrimitives, bool SelectObjects) const {
    // + Basis.GetCenter() - in the next call to orient...
    olx_gl::translate(RotationCenter);
    olx_gl::orient(Basis);
    olx_gl::translate(-RotationCenter);
    ...
  }
*/
class TXGroup : public TGlGroup, public AGlMouseHandler {
  vec3d RotationCenter, RotationDir;
  vec3d_alist src_crds;
  TXAtomPList Atoms;
  TXBondPList Bonds;
  AGDrawObject *rotation_anchor;
  double AngleInc, AngleAcc;
  bool mirror, MirrororingEnabled;
protected:
  virtual void DoDraw(bool SelectPrimitives, bool SelectObjects) {
    if (GetParentGroup() != 0) {  // is inside a group?
      TGlGroup::DoDraw(SelectPrimitives, SelectObjects);
      return;
    }
    for (int ditr=0; ditr < 2; ditr++) {
      for (size_t i = 0; i < Atoms.Count(); i++) {
        olx_swap(Atoms[i]->crd(), src_crds[i]);
      }
      for (size_t i = 0; i < Bonds.Count(); i++) {
        Bonds[i]->Update();
      }
      for (size_t i=0; i < TGlGroup::Count(); i++) {
        AGDrawObject& G = GetObject(i);
        if (!G.IsVisible()) {
          continue;
        }
        if (G.IsGroup()) {
          TGlGroup* group = dynamic_cast<TGlGroup*>(&G);
          if (group != 0) {
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
              if (G.Is<TXAtom>()) {
                TXAtom &a = (TXAtom &)G;
                if (a.crd().Equals(RotationCenter, 1e-3))
                  initialised = true;
              }
              else if (G.Is<TXBond>()) {
                TXBond &b = (TXBond &)G;
                if (RotationCenter.Equals((b.A().crd() + b.B().crd()) / 2, 1e-3)) {
                  initialised = true;
                }
              }
            }
            if (initialised) {
              TGlGroup::GetGlM().Init(false);
            }
            else {
              TGlMaterial glm = GlP.GetProperties();
              glm.SetFlags(glm.GetFlags()|sglmShininessF|sglmSpecularF);
              glm.AmbientF *= 0.75;
              if (ditr == 1) {
                glm.AmbientF = glm.AmbientF.GetRGB() | 0x007070;
              }
              else {
                glm.AmbientF = glm.AmbientF.GetRGB() | 0x700070;
              }
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
    for (size_t i = 0; i < Atoms.Count(); i++) {
      Atoms[i]->crd() += t;
      if (mirror) {
        src_crds[i] -= t;
      }
    }
    RotationCenter += t;
    for (size_t i = 0; i < Count(); i++) {
      GetObject(i).Update();
    }
    return true;
  }

  virtual bool DoRotate(const vec3d& vec, double angle) {
    if (AngleInc != 0) {
      if (olx_abs(AngleAcc += angle) < AngleInc) {
        return true;
      }
      angle = AngleInc*olx_sign(AngleAcc);
      AngleAcc = 0;
    }
    mat3d m;
    if (RotationDir.IsNull()) {
      olx_create_rotation_matrix(m, vec, cos(angle), sin(angle));
    }
    else {
      olx_create_rotation_matrix(m, RotationDir, cos(angle), sin(angle));
    }
    for (size_t i = 0; i < Atoms.Count(); i++) {
      Atoms[i]->crd() = (Atoms[i]->crd() - RotationCenter)*m + RotationCenter;
      if (mirror) {
        src_crds[i] = m*(src_crds[i] - RotationCenter) + RotationCenter;
      }
    }
    for (size_t i = 0; i < Bonds.Count(); i++) {
      Bonds[i]->Update();
    }
    return true;
  }

  virtual bool DoZoom(double, bool) {
    return true;
  }
  
  virtual const TGlRenderer& DoGetRenderer() const { return GetParent(); }

  bool OnMouseDown(const IOlxObject *Sender, const TMouseData& Data) {
    return GetHandler().OnMouseDown(*this, Data);
  }

  bool OnMouseUp(const IOlxObject *Sender, const TMouseData& Data) {
    if (Data.Button == smbRight) {
      if (Data.HasObject()) {
        if (Data.GetObject()->Is<TXAtom>()) {
          if (rotation_anchor == Data.GetObject()) {
            rotation_anchor = 0;
            UpdateRotationCenter();
          }
          else {
            rotation_anchor = Data.GetObject();
            RotationCenter = ((TXAtom*)Data.GetObject())->crd();
            RotationDir.Null();
          }
        }
        else if (Data.GetObject()->Is<TXBond>()) {
          if (rotation_anchor == Data.GetObject()) {
            rotation_anchor = 0;
            RotationDir.Null();
            UpdateRotationCenter();
          }
          else {
            rotation_anchor = Data.GetObject();
            TXBond* xb = (TXBond*)rotation_anchor;
            RotationCenter = (xb->A().crd() + xb->B().crd()) / 2;
            RotationDir = (xb->B().crd() - xb->A().crd()).Normalise();
          }
        }
        return true;
      }
    }
    return GetHandler().OnMouseUp(*this, Data);
  }

  bool OnMouseMove(const IOlxObject *Sender, const TMouseData& Data) {
    if (MirrororingEnabled && (Data.Shift & sssAlt) != 0) {
      mirror = true;
      TMouseData d = Data;
      d.Shift &= ~sssAlt;
      return GetHandler().OnMouseMove(*this, d);
    }
    mirror = false;
    return GetHandler().OnMouseMove(*this, Data);
  }

  bool OnDblClick(const IOlxObject *Sender, const TMouseData& Data) {
    return GetHandler().OnDblClick(*this, Data);
  }
public:
  TXGroup(TGlRenderer& R, const olxstr& colName) :
    TGlGroup(R, colName),
    rotation_anchor(0),
    AngleInc(0),
    AngleAcc(0),
    mirror(false)
  {
    SetMoveable(true);
    SetRoteable(true);
  }

  void AddAtoms(const TPtrList<TXAtom>& atoms) {
    TGlGroup::AddObjects(atoms);
    Atoms.SetCapacity(atoms.Count());
    src_crds.SetCount(atoms.Count());
    for (size_t i = 0; i < atoms.Count(); i++) {
      Bonds.AddAll(Atoms.Add(atoms[i])->GetBonds(),
        StaticCastAccessor<TXBond>());
      src_crds[i] = atoms[i]->crd();
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
  
  void Clear() {
    TGlGroup::Clear();
    Atoms.Clear();
    Bonds.Clear();
    src_crds.Clear();
  }

  void Update() {
    Atoms.Clear();
    Bonds.Clear();
    src_crds.Clear();
    src_crds.SetCapacity(TGlGroup::Count());
    for (size_t i = 0; i < TGlGroup::Count(); i++) {
      AGDrawObject& G = GetObject(i);
      if (G.Is<TXAtom>()) {
        src_crds.Add(Atoms.Add((TXAtom&)G)->crd());
      }
      else if (G.Is<TXBond>()) {
        Bonds.Add((TXBond&)G);
      }
    }
  }

  void UpdateRotationCenter() {
    RotationCenter.Null();
    for (size_t i = 0; i < Atoms.Count(); i++) {
      RotationCenter += Atoms[i]->crd();
    }
    RotationCenter /= Atoms.Count();
  }

  const vec3d& GetRotationCenter() const { return RotationCenter; }

  void SetSrcCoordinates(const vec3d_alist& crds) {
    if (Atoms.Count() == crds.Count()) {
      src_crds = crds;
    }
  }

  const vec3d_alist &GetSrcCoordinates() const {
    return src_crds;
  }

  DefPropP(double, AngleInc)
  DefPropBIsSet(MirrororingEnabled)
  
};

EndGxlNamespace()
#endif
