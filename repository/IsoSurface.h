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
////////////////////////////////////////////////////////////////////////
  struct IsoPoint {
    size_t newID;
    int data;
    vec3f crd;
    bool initialised;
    IsoPoint()
      : initialised(false), data(-1)
    {}
  };
  class T4DIndex {
    struct Points3  {
      IsoPoint ps[3];
    };
    typedef olx_pdict<int, Points3> IsoPointListZ;  //x
    typedef olx_pdict<int, IsoPointListZ*> IsoPointListY;  //y
    typedef olx_pdict<int, IsoPointListY*> IsoPointListX;  //z
    IsoPointListX Data;
    size_t Count;
  public:
    T4DIndex() : Count(0) {  }
    ~T4DIndex()  {  Clear();  }
    IsoPoint &Add(int x, int y, int z, int edgeId,
      float vx, float vy, float vz, int p_data)
    {
      adjust(x, y, z, edgeId);
      IsoPoint* ip;
      const size_t yi = Data.IndexOf(x);
      if (yi == InvalidIndex) {
        ip = &Data.Add(x, new IsoPointListY)->Add(y, new IsoPointListZ)->
          Add(z, Points3()).ps[edgeId];
      }
      else {
        IsoPointListY* ly = Data.GetValue(yi);
        const size_t zi = ly->IndexOf(y);
        if (zi == InvalidIndex) {
          ip = &ly->Add(y, new IsoPointListZ)->Add(z, Points3()).ps[edgeId];
        }
        else {
          IsoPointListZ* lz = ly->GetValue(zi);
          const size_t pi = lz->IndexOf(z);
          if (pi == InvalidIndex) {
            ip = &lz->Add(z, Points3()).ps[edgeId];
          }
          else {
            ip = &lz->GetValue(pi).ps[edgeId];
          }
        }
      }
      ip->crd[0] = vx;
      ip->crd[1] = vy;
      ip->crd[2] = vz;
      ip->data = p_data;
      ip->initialised = true;
      Count++;
      return *ip;
    }
    const IsoPoint& Get(int x, int y, int z, int extra) const {
      return Data.Get(x)->Get(y)->Get(z).ps[extra];
    }
    const IsoPoint& Get(uint64_t crd) const {
      int x, y, z, e;
      decode(crd, x, y, z, e);
      return Data.Get(x)->Get(y)->Get(z).ps[e];
    }
    void Clear() {
      for (size_t i = 0; i < Data.Count(); i++) {
        IsoPointListY* ly = Data.GetValue(i);
        for (size_t j = 0; j < ly->Count(); j++) {
          delete ly->GetValue(j);
        }
        delete ly;
      }
      Data.Clear();
      Count = 0;
    }
    void GetVertices(TArrayList<vec3f>& v, TArrayList<int> *d) {
      v.SetCount(Count);
      if (d != 0) {
        d->SetCount(Count);
      }
      size_t ind = 0;
      for (size_t i = 0; i < Data.Count(); i++) {
        IsoPointListY* ly = Data.GetValue(i);
        for (size_t j = 0; j < ly->Count(); j++) {
          IsoPointListZ* lz = ly->GetValue(j);
          for (size_t k = 0; k < lz->Count(); k++) {
            Points3& p = lz->GetValue(k);
            for (int l = 0; l < 3; l++) {
              if (p.ps[l].initialised) {
                p.ps[l].newID = ind;
                v[ind] = p.ps[l].crd;
                if (d != 0) {
                  (*d)[ind] = p.ps[l].data;
                }
                ind++;
              }
              else {
                p.ps[l].newID = ind;
              }
            }
          }
        }
      }
    }
    static void adjust(int& x, int& y, int& z, int& edgeId) {
      int extra = 0;
      switch (edgeId) {
      case 0:  extra = 1;              break; // (x,y,z) + 1
      case 1:  y++;                    break; // (x,y+1,z)
      case 2:  x++;  extra = 1;        break;  // (x+1,y,z) + 1;
      case 3:                          break; // (x,y,z)
      case 4:  z++;  extra = 1;        break;  // (x,y,z+1) + 1
      case 5:  y++;  z++;              break; //(x,y+1,z+1)
      case 6:  x++;  z++;  extra = 1;  break;  //(x+1,y,z+1) + 1
      case 7:  z++;                    break;  //(x,y,z+1)
      case 8:  extra = 2;              break;  //(x,y,z)+2;
      case 9:  y++;  extra = 2;        break;  //(x,y+1,z) + 2;
      case 10: x++;  y++;  extra = 2;  break;  //(x+1,y+1,z) + 2;
      case 11: x++;  extra = 2;        break;  //(x+1,y,z) + 2
      default: break;         // Invalid edge no.
      }
      edgeId = extra;
    }
    static uint64_t encode(int x, int y, int z, int edgeId) {
      int extra = 0;
      switch (edgeId) {
      case 0:  extra = 1;              break; // (x,y,z) + 1
      case 1:  y++;                    break; // (x,y+1,z)
      case 2:  x++;  extra = 1;        break;  // (x+1,y,z) + 1;
      case 3:                          break; // (x,y,z)
      case 4:  z++;  extra = 1;        break;  // (x,y,z+1) + 1
      case 5:  y++;  z++;              break; //(x,y+1,z+1)
      case 6:  x++;  z++;  extra = 1;  break;  //(x+1,y,z+1) + 1
      case 7:  z++;                    break;  //(x,y,z+1)
      case 8:  extra = 2;              break;  //(x,y,z)+2;
      case 9:  y++;  extra = 2;        break;  //(x,y+1,z) + 2;
      case 10: x++;  y++;  extra = 2;  break;  //(x+1,y+1,z) + 2;
      case 11: x++;  extra = 2;        break;  //(x+1,y,z) + 2
      default: return ~0;         // Invalid edge no.
      }
      // max grid size is 1024x1024x1022
      return (uint64_t)(((uint64_t)extra) << 60)    |
             ((((uint64_t)x) << 40) & 0xFFFFF0000000000) |
             ((((uint64_t)y) << 20) & 0x00000FFFFF00000) |
             (((uint64_t)z) & 0x0000000000FFFFF);
    }
    static inline void decode(uint64_t code, int& x, int& y, int& z, int& extra) {
      extra = static_cast<int>(((code & 0xF000000000000000) >> 60));
      z =     static_cast<int>(((code & 0x0000000000FFFFF)));
      y =     static_cast<int>(((code & 0x00000FFFFF00000) >> 20));
      x =     static_cast<int>(((code & 0xFFFFF0000000000) >> 40));
    }
  };
///////////////////////////////////////////////////////////////////////////////////
  const int ZSlice;

  TArrayList<vec3f> Normals;
  TArrayList<vec3f> Vertices;
  TArrayList<int> VertexData;

  // List of POINT3Ds which form the isosurface.
  T4DIndex IsoPoints;

  // List of IsoTriangleS which form the triangulation of the isosurface.
  TArrayList<IsoTriangle> Triangles, AllTriangles;

  void AddSurfacePoint(unsigned int nX, unsigned int nY,
    unsigned int nZ, unsigned int nEdgeNo);
  /* Renames vertices and triangles so that they can be accessed more
   efficiently.
   */
  void RenameVerticesAndTriangles();

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
