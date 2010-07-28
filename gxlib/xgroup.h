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
  TXAtomPList Atoms;
protected:
  virtual void DoDraw(bool SelectPrimitives, bool SelectObjects) const {
    if( GetParentGroup() != NULL )  {  // is inside a group?
      TGlGroup::DoDraw(SelectPrimitives, SelectObjects);
      return;
    }
    for( size_t i=0; i < Count(); i++ )  {
      AGDrawObject& G = GetObject(i);
      if( !G.IsVisible() || G.IsDeleted() )  continue;
      if( G.IsGroup() )    {
        TGlGroup* group = dynamic_cast<TGlGroup*>(&G);
        if( group != NULL )  {
          group->Draw(SelectPrimitives, SelectObjects);
          continue;
        }
      }
      const size_t pc = G.GetPrimitives().PrimitiveCount();
      for( size_t j=0; j < pc; j++ )  {
        TGlPrimitive& GlP = G.GetPrimitives().GetPrimitive(j);
        TGlMaterial glm = GlP.GetProperties();
        glm.SetFlags(glm.GetFlags()|sglmColorMat|sglmShininessF|sglmSpecularF);
        glm.AmbientF *= 0.75;
        glm.AmbientF = glm.AmbientF.GetRGB() | 0x007070;
        glm.ShininessF = 32;
        glm.SpecularF = 0xff00;
        glm.Init(false);
        if( SelectObjects )     olx_gl::loadName((GLuint)G.GetTag());
        if( SelectPrimitives )  olx_gl::loadName((GLuint)GlP.GetTag());
        olx_gl::pushMatrix();
        if( G.Orient(GlP) )  {
          olx_gl::popMatrix();
          continue;
        }
        GlP.Draw();
        olx_gl::popMatrix();
      }
    }
  }
  virtual bool DoTranslate(const vec3d& t) {
    for( size_t i=0; i < Atoms.Count(); i++ )
      Atoms[i]->Atom().crd() += t;
    RotationCenter += t;
    for( size_t i=0; i < Count(); i++ )
      GetObject(i).Update();
    return true;
  }
  virtual bool DoRotate(const vec3d& vec, double angle)  {
    mat3d m;  
    if( RotationDir.IsNull() )
      olx_create_rotation_matrix(m, vec, cos(angle), sin(angle) );
    else
      olx_create_rotation_matrix(m, RotationDir, cos(angle), sin(angle) );
    for( size_t i=0; i < Atoms.Count(); i++ )
      Atoms[i]->Atom().crd() = (Atoms[i]->Atom().crd()-RotationCenter)*m+RotationCenter;
    for( size_t i=0; i < Count(); i++ )
      GetObject(i).Update();
    return true;
  }
  virtual bool DoZoom(double, bool)  {  return false;  }
  virtual const TGlRenderer& DoGetRenderer() const {  return GetParent();  }
  bool OnMouseDown(const IEObject *Sender, const TMouseData& Data)  {
    return GetHandler().OnMouseDown(*this, Data);
  }
  bool OnMouseUp(const IEObject *Sender, const TMouseData& Data)  {
    if( Data.Button == smbRight )  {
      if( Data.Object != NULL )  {
        if( EsdlInstanceOf(*Data.Object, TXAtom) )  {
          RotationCenter = ((TXAtom*)Data.Object)->Atom().crd();
          RotationDir.Null();
        }
        else if( EsdlInstanceOf(*Data.Object, TXBond) )  {
          TXBond* xb = (TXBond*)Data.Object;
          RotationCenter = (xb->Bond().A().crd() + xb->Bond().B().crd())/2;
          RotationDir = (xb->Bond().B().crd() - xb->Bond().A().crd()).Normalise();
        }
        return true;
      }
    }
    return GetHandler().OnMouseUp(*this, Data);
  }
  bool OnMouseMove(const IEObject *Sender, const TMouseData& Data)  {
    return GetHandler().OnMouseMove(*this, Data);
  }
  bool OnDblClick(const IEObject *Sender, const TMouseData& Data)  {
    return GetHandler().OnDblClick(*this, Data);
  }
public:
  TXGroup(TGlRenderer& R, const olxstr& colName) : TGlGroup(R, colName)  {
    SetMoveable(true);
    SetRoteable(true);
  }
  void AddAtoms(const TPtrList<TXAtom>& atoms)  {
    TGlGroup::AddObjects(atoms);
    Atoms.AddList(atoms);
    UpdateRotationCenter();
  }
  void Clear()  {
    TGlGroup::Clear();
    Atoms.Clear();
  }
  void Update()  {
    Atoms.Clear();
    for( size_t i=0; i < Count(); i++ )  {
      AGDrawObject& G = GetObject(i);
      if( EsdlInstanceOf(G, TXAtom) )
        Atoms.Add((TXAtom&)G);
    }
  }
  void UpdateRotationCenter()  {
    RotationCenter.Null();
    for( size_t i=0; i < Atoms.Count(); i++ )
      RotationCenter += Atoms[i]->Atom().crd();
    RotationCenter /= Atoms.Count();
  }
  const vec3d& GetRotationCenter() const {  return RotationCenter;  }
};

EndGxlNamespace()
#endif
