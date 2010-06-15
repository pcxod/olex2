#ifndef __olx_glx_xgroup_H
#define __olx_glx_xgroup_H
#include "gxbase.h"
#include "glgroup.h"
#include "xatom.h"
#include "xbond.h"

BeginGxlNamespace()

class TXGroup : public TGlGroup, public AGlMouseHandler {
  vec3d RotationCenter;
protected:
  virtual void DoDraw(bool SelectPrimitives, bool SelectObjects) const {
    olx_gl::translate(RotationCenter);  // + Basis.GetCenter() - in the next call to orient...
    olx_gl::orient(Basis);
    olx_gl::translate(-RotationCenter);
    TGlGroup::DoDraw(SelectPrimitives, SelectObjects);
  }
  TEBasis Basis;
  virtual bool DoTranslate(const vec3d& t) {  Basis.Translate(t);  return true;  }
  virtual bool DoRotate(const vec3d& vec, double angle) {  Basis.Rotate(vec, angle);  return true;  }
  virtual bool DoZoom(double, bool)  {  return false;  }
  virtual const TGlRenderer& DoGetRenderer() const {  return GetParent();  }
  bool OnMouseDown(const IEObject *Sender, const TMouseData *Data)  {
    return GetHandler().OnMouseDown(*this, Data);
  }
  bool OnMouseUp(const IEObject *Sender, const TMouseData *Data)  {
    return GetHandler().OnMouseUp(*this, Data);
  }
  bool OnMouseMove(const IEObject *Sender, const TMouseData *Data)  {
    return GetHandler().OnMouseMove(*this, Data);
  }
  bool OnDblClick(const IEObject *Sender, const TMouseData *Data)  {
    return GetHandler().OnDblClick(*this, Data);
  }
public:
  TXGroup(TGlRenderer& R, const olxstr& colName) : TGlGroup(R, colName)  {
    SetMoveable(true);
    SetRoteable(true);
  }
  void AddAtoms(const TPtrList<TXAtom>& atoms)  {
    for( size_t i=0; i < atoms.Count(); i++ )
      RotationCenter += atoms[i]->Atom().crd();
    RotationCenter /= atoms.Count();
    TGlGroup::AddObjects(atoms);
  }
  const vec3d& GetCenter() const {  return Basis.GetCenter();  }
  const mat3d& GetMatrix() const {  return Basis.GetMatrix();  }
  const vec3d& GetRotationCenter() const {  return RotationCenter;  }
};

EndGxlNamespace()
#endif
