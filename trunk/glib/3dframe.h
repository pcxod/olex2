/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gl_3dframe_H
#define __olx_gl_3dframe_H
#include "glmousehandler.h"
#include "glrender.h"
#include "styles.h"
#include "glprimitive.h"
BeginGlNamespace()

class A3DFrameCtrl  {
public:
  virtual bool OnTranslate(size_t sender, const vec3d& t) = 0;
  virtual bool OnRotate(size_t sender, const vec3d& vec, double angle) = 0;
  virtual bool OnZoom(size_t sender, double zoom, bool inc) = 0;
  virtual void SetBasis() const = 0;
};
// frame 'face' - the control sphere of the face
class TFaceCtrl : public AGlMouseHandlerImp {
  A3DFrameCtrl& ParentCtrl;
  vec3d &A, &B, &C, &D, &N;
  size_t Id;
protected:
  virtual bool DoTranslate(const vec3d& t)  {
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
  TFaceCtrl(TGlRenderer& prnt, const olxstr& cName, size_t _Id,
    vec3d& _A, vec3d& _B, vec3d& _C, vec3d& _D, vec3d& _N,
    A3DFrameCtrl& _ParentCtrl)
    : AGlMouseHandlerImp(prnt, cName),
      ParentCtrl(_ParentCtrl),
      A(_A), B(_B), C(_C), D(_D), N(_N), Id(_Id)
  {
    SetMoveable(true);
    SetRoteable(true);
  }
  size_t GetId() const {  return Id;  }
  void Create(const olxstr& cName);
  virtual bool GetDimensions(vec3d&, vec3d&)  {  return false;  }
  virtual bool Orient(TGlPrimitive&);
  vec3d& GetA()  {  return A;  }
  vec3d& GetB()  {  return B;  }
  vec3d& GetC()  {  return C;  }
  vec3d& GetD()  {  return D;  }
  const vec3d& GetN() const {  return N;  }
  vec3d GetCenter() const { return (A+B+C+D)/4; }
};
// the frame class itself...
class T3DFrameCtrl : public AGlMouseHandlerImp, public A3DFrameCtrl {
  vec3d edges[8], norms[6];
protected:
  virtual bool DoTranslate(const vec3d& t)  {  Translate(t);  return true;  }
  virtual bool DoRotate(const vec3d& vec, double angle);
  virtual bool DoZoom(double zoom, bool inc);
  virtual bool OnTranslate(size_t sender, const vec3d& t);
  virtual bool OnRotate(size_t sender, const vec3d&, double angle)  {
    return DoRotate(Faces[sender].GetN(), angle);
  }
  virtual bool OnZoom(size_t, double, bool)  {
    return true;
  }
public:
  T3DFrameCtrl(TGlRenderer& prnt, const olxstr& cName);
  void Create(const olxstr& cName);
  virtual bool GetDimensions(vec3d& _mn, vec3d& _mx)  {
    for( int i=0; i < 8; i++ )
      vec3d::UpdateMinMax(edges[i], _mn, _mx);
    return true;
  }
  virtual void SetBasis() const {}
  virtual void SetVisible(bool v)  {
    AGDrawObject::SetVisible(v);
    for( int i=0; i < 6; i++ )
      Faces[i].SetVisible(v);
  }
  virtual bool Orient(TGlPrimitive&)  {
    SetBasis();
    olx_gl::begin(GL_QUADS);
    for( int i=0; i < 6; i++ )  {
      olx_gl::normal(Faces[i].GetN());
      olx_gl::vertex(Faces[i].GetA());
      olx_gl::vertex(Faces[i].GetB());
      olx_gl::vertex(Faces[i].GetC());
      olx_gl::vertex(Faces[i].GetD());
    }
    olx_gl::end();
    return true;
  }
  const vec3d& GetEdge(size_t i) const {
    if( i >= 8 )
      throw TIndexOutOfRangeException(__OlxSourceInfo, i, 0, 8);
    return edges[i];
  }
  void SetEdge(size_t i, const vec3d& val)  {
    if( i >= 8 )
      throw TIndexOutOfRangeException(__OlxSourceInfo, i, 0, 8);
    edges[i] = val;
  }
  // recalculates normals etc for the new edges
  void UpdateEdges();
  void Translate(const vec3d& t)  {
    for( int i=0; i < 8; i++ )
      edges[i] += t;
  }
  double GetVolume() const {
    return sqrt((edges[1]-edges[0]).QLength()*(edges[3]-edges[0]).QLength()
      *(edges[4]-edges[0]).QLength());
  }
  vec3d GetSize() const {
    return vec3d(
      (edges[3]-edges[0]).Length(),
      (edges[1]-edges[0]).Length(),
      (edges[4]-edges[0]).Length());
  }
  vec3d GetCenter() const {
    vec3d cnt;
    for( int i=0; i < 8; i++ )
      cnt += edges[i];
    return cnt/8;
  }
  mat3d GetNormals() const {
    return mat3d(
      (edges[3]-edges[0]).Normalise(),
      (edges[1]-edges[0]).Normalise(),
      (edges[4]-edges[0]).Normalise());
  }
  void ToDataItem(TDataItem &di) const;
  void FromDataItem(const TDataItem &di);

  TTypeList<TFaceCtrl> Faces;
};

EndGlNamespace()
#endif
