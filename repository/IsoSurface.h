#ifndef CISOSURFACE_H
#define CISOSURFACE_H
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

struct IsoTriangle {
  int pointID[3];
  const IsoTriangle& operator = (const IsoTriangle& it)  {
    pointID[0] = it.pointID[0];
    pointID[1] = it.pointID[1];
    pointID[2] = it.pointID[2];
    return *this;
  }
};

template <class T> class CIsoSurface {
public:
  // Constructor and destructor.
  CIsoSurface(TArray3D<T>& points);
  ~CIsoSurface()  {  DeleteSurface();  }

  // Generates the isosurface from the scalar field contained in the
  // buffer ptScalarField[].
  void GenerateSurface(T tIsoLevel);

  // Returns true if a valid surface has been generated.
  inline bool IsSurfaceValid() const {  return m_bValidSurface;  }

  // Deletes the isosurface.
  void DeleteSurface();

  inline TArrayList<vec3f>& NormalList()         {  return Normals;  }
  inline TArrayList<IsoTriangle>& TriangleList() {  return Triangles;  }
  inline TArrayList<vec3f>& VertexList()         {  return Vertices;  }

protected:
////////////////////////////////////////////////////////////////////////
  struct IsoPoint {
    int newID;
    float x, y, z;
    bool initialised;
    IsoPoint(): initialised(false) {  }
  };
  class T4DIndex  {
    struct Points3  {
      IsoPoint ps[3];
    };
    typedef TPSTypeList<int, Points3> IsoPointListZ;  //x
    typedef TPSTypeList<int, IsoPointListZ*> IsoPointListY;  //y
    typedef TPSTypeList<int, IsoPointListY*> IsoPointListX;  //z
    IsoPointListX Data;
    int Count;
  public:
    T4DIndex()   {  Count = 0;  }
    ~T4DIndex()  {  Clear();  }
    void Add(int x, int y, int z, int edgeId, float vx, float vy, float vz)  {
      adjust(x,y,z,edgeId);
      IsoPoint* ip;
      int yi = Data.IndexOfComparable(x);
      if( yi == -1 )  {
        ip = &Data.Add(x, new IsoPointListY).Object()->Add(y, new IsoPointListZ).
          Object()->Add(z, Points3() ).Object().ps[edgeId]; 
      }
      else  {
        IsoPointListY* ly = Data.Object(yi);
        int zi = ly->IndexOfComparable(y);
        if( zi == -1 )  {
          ip = &ly->Add(y, new IsoPointListZ).Object()->Add(z, Points3() ).Object().ps[edgeId]; 
        }
        else  {
          IsoPointListZ* lz = ly->Object(zi);
          int pi = lz->IndexOfComparable(z);
          if( pi == -1 )  {
            ip = &lz->Add(z, Points3() ).Object().ps[edgeId]; 
          }
          else  {
            ip = &lz->Object(pi).ps[edgeId];
          }
        }
      }
      ip->x = vx;
      ip->y = vy;
      ip->z = vz;
      ip->initialised = true;
      Count++;
    }
    const IsoPoint& Get(int x, int y, int z, int extra) const {
      return Data[x]->Item(y)->Item(z).ps[extra];
    }
    const IsoPoint& Get(uint32_t crd) const {
      int x, y, z, e;
      decode(crd, x, y, z, e);
      return Data[x]->Item(y)->Item(z).ps[e];
    }
    void Clear()  {
      for( int i=0; i < Data.Count(); i++ )  {
        IsoPointListY* ly = Data.Object(i);
        for( int j=0; j < ly->Count(); j++ ) 
          delete ly->Object(j);
        delete ly;
      }
      Data.Clear();
      Count = 0;
    }
    void GetVertices(TArrayList<vec3f>& v) {  // an upper estimate
      v.SetCount(Count);
      int ind = 0;
      for( int i=0; i < Data.Count(); i++ )  {
        IsoPointListY* ly = Data.Object(i);
        for( int j=0; j < ly->Count(); j++ )  {
          IsoPointListZ* lz = ly->Object(j);
          for( int k=0; k < lz->Count(); k++ )  {
            Points3& p = lz->Object(k);
            for(int l=0; l < 3; l++ )  {
              if( p.ps[l].initialised )  {
                p.ps[l].newID = ind;
                vec3f& fp = v[ind];
                fp[0] = p.ps[l].x;
                fp[1] = p.ps[l].y;
                fp[2] = p.ps[l].z;
                ind++;
              }
              else
                p.ps[l].newID = ind;
            }
          }
        }
      }
    }
    static inline void adjust(int& x, int& y, int& z, int& edgeId) {
      int extra = 0;
      switch( edgeId ) {
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
    static inline uint32_t encode(int x, int y, int z, int edgeId) {
      int extra = 0;
      switch( edgeId ) {
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
        default: return -1;         // Invalid edge no.
      }
      uint32_t id = x;
      id <<= 10;  // max grid size is 1024x1024x1022
      id |= y;
      id <<= 10;
      id |= z;
      id <<= 2;
      id |= extra;
      return id;  
    }
    static inline void decode(uint32_t code, int& x, int& y, int& z, int& extra ) {
      extra = (code & 0x0002);
      code >>= 2;
      z = (code & 0x03FF);
      code >>= 10;
      y = (code & 0x03FF);
      code >>= 10;
      x = (code & 0x03FF);
    }
  };
///////////////////////////////////////////////////////////////////////////////////
  const int DimmX, DimmY, DimmZ, ZSlice;
  // The normals.
  TArrayList<vec3f> Normals;
  // The vertices
  TArrayList<vec3f> Vertices;
  // List of POINT3Ds which form the isosurface.
  T4DIndex IsoPoints;

  // List of IsoTriangleS which form the triangulation of the isosurface.
  TArrayList<IsoTriangle> Triangles, AllTriangles;

  void AddSurfacePoint(unsigned int nX, unsigned int nY, unsigned int nZ, unsigned int nEdgeNo);
  // Renames vertices and triangles so that they can be accessed more
  // efficiently.
  void RenameVerticesAndTriangles();

  // Calculates the normals.
  void CalculateNormals();

  // The buffer holding the scalar field.
  TArray3D<T>& Points;

  // The isosurface value.
  T m_tIsoLevel;

  // Indicates whether a valid surface is present.
  bool m_bValidSurface;

  // Lookup tables used in the construction of the isosurface.
  static const unsigned int m_edgeTable[256];
  static const unsigned int m_triTable[256][16];
};
#endif // CISOSURFACE_H

