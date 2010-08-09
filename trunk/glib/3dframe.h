#ifndef __olx_gl_3dframe_H
#define __olx_gl_3dframe_H
#include "glmousehandler.h"

class ACtrl3D  {
public:
  virtual bool OnTranslate(size_t sender, const vec3d& t) = 0;
  virtual bool OnRotate(size_t sender, const vec3d& vec, double angle) = 0;
  virtual bool OnZoom(size_t sender, double zoom, bool inc) = 0;
  virtual const vec3d& GetCenter() const = 0;
};
// a sphere which allows 3D translation and rotation around
class TCtrl3D : public AGlMouseHandlerImp {
  vec3d Center;
  ACtrl3D& ParentCtrl;
  TGlPrimitive* pPrimitive;
  size_t Id;
protected:
  virtual bool DoTranslate(const vec3d& t)  {
    Center += t;
    ParentCtrl.OnTranslate(GetId(), t);
    return true;
  }
  virtual bool DoRotate(const vec3d& vec, double angle)  {
    return ParentCtrl.OnRotate(GetId(), vec, angle);
  }
  virtual bool DoZoom(double zoom, bool inc)  {
    return ParentCtrl.OnZoom(GetId(), zoom, inc);
  }
public:
  TCtrl3D(TGlRenderer& prnt, const olxstr& cName, size_t _Id, const vec3d& center,
    ACtrl3D& _ParentCtrl) : AGlMouseHandlerImp(prnt, cName), ParentCtrl(_ParentCtrl),
    Id(_Id), Center(center)  {}
  size_t GetId() const {  return Id;  }
  TGlPrimitive& GetPrimitive() const {  return *pPrimitive;  }
  virtual bool Orient(class TGlPrimitive&)  {
    olx_gl::translate(ParentCtrl.GetCenter());
    return false;
  }
};
// a line which allows 2D translation and rotation around
class TCtrl2D : public AGlMouseHandlerImp {
  vec3d From, To;
  ACtrl3D& ParentCtrl;
  TGlPrimitive* pPrimitive;
  size_t Id;
protected:
  virtual bool DoTranslate(const vec3d& t)  {
    From += t;
    To += t;
    ParentCtrl.OnTranslate(GetId(), t);
    return true;
  }
  virtual bool DoRotate(const vec3d& vec, double angle)  {
    return ParentCtrl.OnRotate(GetId(), vec, angle);
  }
  virtual bool DoZoom(double zoom, bool inc)  {
    return ParentCtrl.OnZoom(zoom, inc);
  }
public:
  TCtrl2D(TGlRenderer& prnt, const olxstr& cName, size_t _Id, const vec3d& from, const vec3d& to,
    ACtrl3D& _ParentCtrl) : AGlMouseHandlerImp(prnt, cName), ParentCtrl(_ParentCtrl),
    Id(_Id), From(from), To(to)  {}
  size_t GetId() const {  return Id;  }
  const vec3d& GetFrom() const {  return From;  }
  const vec3d& GetTo() const {  return To;  }
  TGlPrimitive& GetPrimitive() const {  return *Primitive;  }
  virtual bool Orient(class TGlPrimitive&)  {
    olx_gl::translate(ParentCtrl.GetCenter());
    return false;
  }
};
// the frame class itself...
class T3DFrame : public AGlMouseHandlerImp {
  TEBasis Basis;
  TTypeList<TCtrl3D> Edges;
  TTypeList<TCtrl2D> Vertices;
protected:
  virtual bool DoTranslate(const vec3d& t)  {
    Basis.Translate(t);
    return true;
  }
  virtual bool DoRotate(const vec3d& vec, double angle)  {
    Basis.Rotate(vec, angle);
    return true;
  }
  virtual bool DoZoom(double zoom, bool inc)  {
    if( inc )
      Basis.SetZoom(Basis.GetZoom()+zoom);
    else
      Basis.SetZoom(zoom, inc);
  }
  virtual bool OnTranslate(size_t sender, const vec3d& t)  {
    return true;
  }
  virtual bool OnRotate(size_t sender, const vec3d& vec, double angle)  {
    return true;
  }
  virtual bool OnZoom(size_t sender, double zoom, bool inc)  {
    return true;
  }
public:
  T3DFrame(TGlRenderer& prnt, const olxstr& cName) : AGlMouseHandlerImp(prnt, cName)
  {
    const vec3d edges[8] = {
      vec3d(0,0,0), vec3d(1,0,0), vec3d(0,1,0), vec3d(1,1,0),
      vec3d(0,0,1), vec3d(1,0,1), vec3d(0,1,1), vec3d(1,1,1)
    }
    for( size_t i=0; i < 8; i++ )
      Edges.Add(new TCtrl3D(prnt, olxstr("edge_") << i, i, edges[i], *this);
    Vertices.Add(new TCtrl2D(prnt, "vert_0",  9, edges[0], edges[1], *this);
    Vertices.Add(new TCtrl2D(prnt, "vert_1", 10, edges[0], edges[2], *this);
    Vertices.Add(new TCtrl2D(prnt, "vert_2", 11, edges[1], edges[3], *this);
    Vertices.Add(new TCtrl2D(prnt, "vert_3", 12, edges[2], edges[3], *this);

    Vertices.Add(new TCtrl2D(prnt, "vert_4", 13, edges[4], edges[5], *this);
    Vertices.Add(new TCtrl2D(prnt, "vert_5", 14, edges[4], edges[6], *this);
    Vertices.Add(new TCtrl2D(prnt, "vert_6", 15, edges[5], edges[7], *this);
    Vertices.Add(new TCtrl2D(prnt, "vert_7", 16, edges[6], edges[7], *this);

    Vertices.Add(new TCtrl2D(prnt, "vert_8", 15, edges[0], edges[4], *this);
    Vertices.Add(new TCtrl2D(prnt, "vert_9", 16, edges[1], edges[5], *this);
    Vertices.Add(new TCtrl2D(prnt, "vert_10", 17, edges[2], edges[6], *this);
    Vertices.Add(new TCtrl2D(prnt, "vert_11", 18, edges[3], edges[7], *this);
  }
  virtual const vec3d& GetCenter() const {  return Basis.GetCenter();  }
};

#endif
