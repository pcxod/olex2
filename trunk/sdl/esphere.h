/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

/* Sphere triangulation routines */
#ifndef __olx_esphere_H
#define __olx_esphere_H
#include "threex3.h"
#include "typelist.h"
BeginEsdlNamespace()

// simple structure to contain three indeces to vertices
struct IndexTriangle {
  TVector3<size_t> vertices;
  size_t operator [] (size_t i) const { return vertices[i]; }
  IndexTriangle(size_t i1, size_t i2, size_t i3) : vertices(i1,i2,i3)  {}
};
// Octahedron face provider
template <class vec_type> struct OctahedronFP  {
  static const vec_type *vertices() {
    static vec_type v[6] = {
      vec_type(0,0,1),  vec_type(1,0,0),  vec_type(0,1,0),
      vec_type(-1,0,0), vec_type(0,-1,0), vec_type(0,0,-1)
    };
    return &v[0];
  }
  static const IndexTriangle *faces() {
    static IndexTriangle f[8] = {
      IndexTriangle(0, 1, 2), IndexTriangle(0, 2, 3),
      IndexTriangle(0, 3, 4), IndexTriangle(0, 4, 1),
      IndexTriangle(5, 2, 1), IndexTriangle(5, 3, 2),
      IndexTriangle(5, 4, 3), IndexTriangle(5, 1, 4)
    };
    return &f[0];
  }
  static size_t vertex_count() { return 6; }
  static size_t face_count() { return 8; }
};
// tetrahedron face provider
// vertices are as from http://en.wikipedia.org/wiki/Tetrahedron
template <class vec_type> struct TetrahedronFP  {
  static const vec_type *vertices() {
    const static double k = 1./sqrt(3.0);
    static vec_type v[4] = {
      vec_type(k, k, k), vec_type(-k, -k, k), vec_type(-k, k, -k),
      vec_type(k, -k, -k)
    };
    return &v[0];
  }
  static const IndexTriangle *faces() {
    static IndexTriangle f[4] = {
      IndexTriangle(0, 2, 1), IndexTriangle(0, 1, 3),
      IndexTriangle(0, 3, 2), IndexTriangle(1, 2, 3)
    };
    return &f[0];
  }
  static size_t vertex_count() { return 4; }
  static size_t face_count() { return 4; }
};
/* a class to build sphere by partitioning an octahedron. Alows custom masks to
be aplied when rendering the sphere in OpenGL or other rendering engine. All
methods and members can be made static...
*/
template <typename float_type, class FaceProvider> class OlxSphere {
public:
  /* rad - radius of the sphere, ext - the generation to generate, vo - vector
  output, to - triangles output, normals - the normals output (one a vertex,
  not triangle). Since there is a huge redundancy, a unique set of vecteces is
  generated and triangles contain inly indeces to the verteces they are based
  on
  */
  static void Generate(float_type rad, size_t ext,
    TTypeList<TVector3<float_type> >& vo,
    TTypeList<IndexTriangle>& to,
    TArrayList<TVector3<float_type> >& normals)
  {
    Generate(rad, ext, vo, to, &normals);
  }
  static void Generate(float_type rad, size_t ext,
    TTypeList<TVector3<float_type> >& vo,
    TTypeList<IndexTriangle>& to,
    TArrayList<TVector3<float_type> >* _normals=NULL)
  {
    size_t nc = FaceProvider::face_count();
    for( size_t i=0; i < ext; i++ )
      nc += nc*3;

    to.SetCapacity(nc+1);
    vo.SetCapacity(nc-1);
    for( size_t i=0; i < FaceProvider::vertex_count(); i++ )
      vo.AddNew(FaceProvider::vertices()[i]);
    for( size_t i=0; i < FaceProvider::face_count(); i++ )
      to.AddNew(FaceProvider::faces()[i]);
    for( size_t i=0; i < ext; i++ ) {
      const size_t t_cnt = to.Count();
      for( size_t j=0; j < t_cnt; j++ )  {
        IndexTriangle& t = to[j];
        vo.AddNew(vo[t.vertices[0]]+vo[t.vertices[1]]).Normalise();
        vo.AddNew(vo[t.vertices[0]]+vo[t.vertices[2]]).Normalise();
        vo.AddNew(vo[t.vertices[1]]+vo[t.vertices[2]]).Normalise();
        // new triangle, (12, 23, 13)
        to.AddNew(vo.Count()-3, vo.Count()-1, vo.Count()-2);
        // new tringles partially based on old vertices (new, 23, 12)
        to.AddNew(t.vertices[1], vo.Count()-1, vo.Count()-3);
        // new tringles partially based on old vertices (new, 13, 23)
        to.AddNew(t.vertices[2], vo.Count()-2, vo.Count()-1);
        // modify the original triangle to take two new points
        t.vertices[1] = vo.Count()-3;  // 12
        t.vertices[2] = vo.Count()-2;  // 13
      }
    }
    if( _normals != NULL )  {
      TArrayList<TVector3<float_type> >& normals = *_normals;
      normals.SetCount(vo.Count());
      for( size_t i=0; i < vo.Count(); i++ )  { // normalise the vertices
        normals[i] = vo[i];
        vo[i] *= rad;
      }
    }
    else  {
      for( size_t i=0; i < vo.Count(); i++ )
        vo[i] *= rad;
    }
  }
};

EndEsdlNamespace()
#endif
