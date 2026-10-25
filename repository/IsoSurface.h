/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_isosurface_H
#define __olx_isosurface_H
/* Modified by O. Dolomanov, 2007.11 to fit to Olex2 development framework */

// File Name: CIsoSurface.h
// Last Modified: 5/8/2000
// Author: Raghavendra Chandrashekara (basesd on source code
// provided by Paul Bourke and Cory Gene Bloyd)
// Email: rc99@doc.ic.ac.uk, rchandrashekara@hotmail.com
//
// Description: This is the interface file for the CIsoSurface class.
// CIsoSurface can be used to construct an isosurface from a scalar
// field.

#include "arrays.h"
#include "estlist.h"
#include "threex3.h"
#include "eset.h"
#include <vector>
#include <string.h>

using namespace olx_array;
struct IsoTriangle {
  int64_t pointID[3];
  int64_t operator [] (size_t i) const { return pointID[i]; }
  int64_t& operator [] (size_t i) { return pointID[i]; }
};

class CIsoSurface {
public:
  CIsoSurface(array_3d<float>& points, const array_3d<int>* data=0);
  ~CIsoSurface()  {  DeleteSurface();  }

  /*Generates the isosurface from the scalar field contained in the buffer
  ptScalarField[].
  */
  void GenerateSurface(float tIsoLevel);

  // Returns true if a valid surface has been generated
  inline bool IsSurfaceValid() const {  return m_bValidSurface;  }

  // Deletes the isosurface.
  void DeleteSurface();

  inline TArrayList<vec3f>& NormalList()  {  return Normals;  }
  inline TArrayList<IsoTriangle>& TriangleList() {  return Triangles;  }
  inline TArrayList<vec3f>& VertexList()  {  return Vertices;  }
  /* This list may be empty if no data was provided */
  inline TArrayList<int>& GetVertexData() { return VertexData; }
protected:
  /* Vertices sit on grid edges: every grid point (x,y,z) owns the three edges
  that start there, along x (slot 0), y (slot 1) and z (slot 2). A vertex with
  grid x = X can only come from the voxel slabs X-1 and X, so the scan keeps
  three planes of such edge slots, numbers plane X as soon as slab X is done
  and resolves the triangles of a slab once both planes they touch are
  numbered. This gives the same vertex order (x, y, z, slot) and the same
  triangles as the former dictionary based index, at a fraction of the cost
  and with memory proportional to one grid plane.
  */
  struct EdgePlane {
    TArrayList<vec3f> crd;
    TArrayList<int> data;
    TArrayList<uint32_t> id;  // valid where the occupancy bit is set
    /* occupancy bitmap: only ~1-2% of the slots carry a vertex, so numbering
    skips empty words and resetting clears a few KB only
    */
    TArrayList<uint64_t> used;
    void Reset(size_t sz) {
      if (id.Count() != sz) {
        crd.SetCount(sz);
        data.SetCount(sz);
        id.SetCount(sz);
        used.SetCount((sz + 63) / 64);
      }
      if (!used.IsEmpty()) {
        memset(&used[0], 0, used.Count() * sizeof(uint64_t));
      }
    }
    void Release() {
      crd.Clear();
      data.Clear();
      id.Clear();
      used.Clear();
    }
  };
  EdgePlane planes[3];
  /* triangle corners of the last two slabs: the slot index within its plane,
  bit 31 set when the corner lies in plane x+1 (a plane holds < 2^31 slots)
  */
  static const uint32_t next_plane_bit = 1u << 31;
  /* std::vector: clear() keeps the capacity, TArrayList::Clear frees it and
  the lists would regrow from scratch for every slab
  */
  std::vector<uint32_t> pending[2];
  // collected here, copied to the lists below once the size is known
  std::vector<vec3f> new_vertices;
  std::vector<int> new_data;
  std::vector<uint32_t> new_triangles;  // vertex id triples

  TArrayList<vec3f> Normals;
  TArrayList<vec3f> Vertices;
  TArrayList<int> VertexData;

  // List of IsoTriangleS which form the triangulation of the isosurface.
  TArrayList<IsoTriangle> Triangles;

  /* maps a cube edge (0-11) of voxel (x,y,z) to the grid point owning it and
  the slot there
  */
  static void EdgeOwner(int edgeNo, int& x, int& y, int& z, int& slot);
  /* the 4 corners of a voxel face as bits 1,2,4,8: for a positive level
  'inside' is above it, otherwise below it
  */
  unsigned int Classify(float a, float b, float c, float d) const {
    if (m_tIsoLevel > 0) {
      return (unsigned int)(a > m_tIsoLevel) |
        ((unsigned int)(b > m_tIsoLevel) << 1) |
        ((unsigned int)(c > m_tIsoLevel) << 2) |
        ((unsigned int)(d > m_tIsoLevel) << 3);
    }
    return (unsigned int)(a < m_tIsoLevel) |
      ((unsigned int)(b < m_tIsoLevel) << 1) |
      ((unsigned int)(c < m_tIsoLevel) << 2) |
      ((unsigned int)(d < m_tIsoLevel) << 3);
  }
  void AddSurfacePoint(unsigned int nX, unsigned int nY,
    unsigned int nZ, unsigned int nEdgeNo);
  // gives plane x its final vertex ids
  void NumberPlane(size_t x);
  // turns the corners of slab x into vertex ids, dropping degenerate triangles
  void ResolveSlab(size_t x);

  // Calculates the normals.
  void CalculateNormals();

  /* The buffer holding the scalar field. */
  array_3d<float>& Points;
  /* data associated with points */
  const array_3d<int>* PointData;

  // The isosurface value.
  float m_tIsoLevel;

  // Indicates whether a valid surface is present.
  bool m_bValidSurface;

  // Lookup tables used in the construction of the isosurface.
  static const unsigned int m_edgeTable[256];
  static const int m_triTable[256][16];
};

namespace olx_grid_util {
  /* inspired by the JACGrid library */
  class reducer {
    TTypeList<vec3f> &vertices;
    TTypeList<IsoTriangle> &triangles;
    TArrayList<olxset<size_t, TPrimitiveComparator> > vt_map;
    void reduce_to_line(size_t a, size_t b);
    void cut_triangle(size_t a, size_t b, size_t c);
  public:
    reducer(TTypeList<vec3f> &vertices_,
      TTypeList<IsoTriangle> &triangles_);
    /* eliminates triangles with sides smaller than the given threshold
    and returns the new vertex index mapping
    */
    TArrayList<size_t>::const_list_type reduce(float th = 0.03f);
  };

  class smoother {
    TTypeList<vec3f> &vertices;
    TArrayList<olxset<size_t, TPrimitiveComparator> > n_map;
  public:
    smoother(TTypeList<vec3f> &vertices,
      const TTypeList<IsoTriangle> &triangles);
    /* smooth the surface using a ratio contribution of any particular vertex
    and 1-ratio contribution of neighbouring vertices
    */
    void smooth(float ratio=0.5f);
  };
} // end namespace olx_grid_util
#endif // guard
