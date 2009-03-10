/* Map utilities 
 (c) O Dolomanov, 2008
*/
#ifndef __map_util_h
#define __map_util_h

#include "xbase.h"
#include "arrays.h"

BeginXlibNamespace()

class MapUtil  {
public:
  struct peak  { 
    int x, y, z, count;  //center
    bool process;
    double summ;
    peak() : process(true), summ(0), count(0) {}
    peak(int _x, int _y, int _z, double val) : process(true),  
      summ(val), count(1), x(_x), y(_y), z(_z) {}
  };
protected:
  struct level {
    int x, y, z;
    level(int _x, int _y, int _z) : x(_x), y(_y), z(_z) {}
    bool operator == (const level& l) const {
      if( x == l.x && y == l.y && z == l.z )  return true;
      return false;
    }
  };
  template <typename MapT> static void peak_search(MapT*** const data, int mapX, int mapY, int mapZ,
    const TArray3D<bool>& Mask, const TPtrList< TTypeList<level> >& SphereMask, TArrayList<peak>& maxima)  {
      bool*** const mask = Mask.Data;
      int lev = 1;
      bool done = false;
      while( !done )  {
        done = true;
        const TTypeList<level>& il = *SphereMask[lev];
        for( int i=0; i < maxima.Count(); i++ )  {
          peak& peak = maxima[i];
          if( !peak.process )  continue;
          for( int j=0; j < il.Count(); j++ )  {
            int x = peak.x + il[j].x;
            if( x < 0 )     x += mapX;
            if( x >= mapX ) x -= mapX; 
            int y = peak.y + il[j].y;
            if( y < 0 )     y += mapY;
            if( y >= mapY ) y -= mapY; 
            int z = peak.z + il[j].z;
            if( z < 0 )     z += mapZ;
            if( z >= mapZ ) z -= mapZ; 
            if( mask[x][y][z] )  continue;
            if( peak.summ > 0 && data[x][y][z] <= 0 )  {
              peak.process = false;
              break;
            }
            if( peak.summ < 0 && data[x][y][z] >= 0 )  {
              peak.process = false;
              break;
            }
            peak.count++;
            peak.summ += data[x][y][z];
            done = false;
            mask[x][y][z] = true;
          }
        }
        if( ++lev >= SphereMask.Count() )  break;
      }
  }

public:
  // a simple map integration, considering the peaks and holes as spheres
  template <typename MapT, typename FloatT> static void Integrate(MapT*** const map, int mapX, int mapY, int mapZ, 
    FloatT mapMin, FloatT mapMax, FloatT mapSig, FloatT resolution, TArrayList<MapUtil::peak>& Peaks)  {
    TPtrList< TTypeList<level> > SphereMask;
    const int maxLevel = Round(2./resolution);
    for( int l=0; l < maxLevel; l++ )
      SphereMask.Add( new TTypeList<level> );

    for( int x=-maxLevel+1; x < maxLevel; x++ )  {
      for( int y=-maxLevel+1; y < maxLevel; y++ )  {
        for( int z=-maxLevel+1; z < maxLevel; z++ )  {
          int r = Round(sqrt((double)(x*x + y*y + z*z)));
          if( r < maxLevel && r > 0 )  // skip 0
            SphereMask[r]->AddNew(x,y,z);
        }
      }
    }
    // eliminate duplicate indexes
    for( int i=0; i < maxLevel; i++ )  {
      TTypeList<level>& l1 = *SphereMask[i];
      for( int j= i+1; j < maxLevel; j++ )  {
        TTypeList<level>& l2 = *SphereMask[j];
        for( int k=0; k < l1.Count(); k++ )  {
          if( l1.IsNull(k) )  continue;
          for( int l=0; l < l2.Count(); l++ )  {
            if( l2[l] == l1[k] )  {
              l2.NullItem(l);
              break;
            }
          }
        }
      }
      l1.Pack();
    }
    const int s_level = 3;
    TArray3D<bool> Mask(0, mapX-1, 0, mapY-1, 0, mapZ-1);
    bool*** const maskData = Mask.Data;
//    double pos_level = 0.5*mapMax, neg_level = 0.8*mapMin; 
    double pos_level = 3*mapSig, neg_level = -3*mapSig; 
    for( int ix=0; ix < mapX; ix++ )  {
      for( int iy=0; iy < mapY; iy++ )  {
        for( int iz=0; iz < mapZ; iz++ )  {
          if( !maskData[ix][iy][iz] && ((map[ix][iy][iz] > pos_level) ||
            (map[ix][iy][iz] < neg_level)) )  {
              const double refval = map[ix][iy][iz];
              bool located = true;
              if( refval > 0 )  {
                for( int i=-s_level; i <= s_level; i++ )  {
                  int x = ix+i;
                  if( x < 0 )      x += mapX;
                  if( x >= mapX )  x -= mapX;
                  for( int j=-s_level; j <= s_level; j++ )  {
                    int y = iy+j;
                    if( y < 0 )      y += mapY;
                    if( y >= mapY )  y -= mapY;
                    for( int k=-s_level; k <= s_level; k++ )  {
                      if( i==0 && j==0 && k == 0 )  continue;
                      int z = iz+k;
                      if( z < 0 )      z += mapZ;
                      if( z >= mapZ )  z -= mapZ;
                      if( map[x][y][z] > refval )  {
                        located = false;
                        break;
                      }
                    }
                    if( !located )  break;
                  }
                  if( !located )  break;
                }
              }
              else  {
                for( int i=-s_level; i <= s_level; i++ )  {
                  int x = ix+i;
                  if( x < 0 )      x += mapX;
                  if( x >= mapX )  x -= mapX;
                  for( int j=-s_level; j <= s_level; j++ )  {
                    int y = iy+j;
                    if( y < 0 )      y += mapY;
                    if( y >= mapY )  y -= mapY;
                    for( int k=-s_level; k <= s_level; k++ )  {
                      if( i==0 && j==0 && k == 0 )  continue;
                      int z = iz+k;
                      if( z < 0 )      z += mapZ;
                      if( z >= mapZ )  z -= mapZ;
                      if( map[x][y][z] < refval )  {
                        located = false;
                        break;
                      }
                    }
                    if( !located )  break;
                  }
                  if( !located )  break;
                }
              }
              if( located )
                Peaks.Add( peak(ix, iy, iz, refval) );
          }
        }
      }
    }
    //int PointCount = mapX*mapY*mapZ;
    peak_search(map, mapX, mapY, mapZ, Mask, SphereMask, Peaks);
    for( int i=0; i < maxLevel; i++ )
      delete SphereMask[i];
  }
  /* Calculates the deepest hole and its fractional coordinates, initialising the map with 'levels'
  expects a map with structure points marked as negative values and the rest - 0
  */
  template <typename map_type> static map_type AnalyseVoids(map_type*** map, int mapX, int mapY, int mapZ, 
    vec3d& void_center)  {
      int level = 0, MaxLevel = 0;
      while( true )  {
        bool levelUsed = false;
        for(int i=0; i < mapX; i++ )  {
          for(int j=0; j < mapY; j++ )  {
            for(int k=0; k < mapZ; k++ )  {
              // neigbouring points analysis
              bool inside = true;
              for(int ii = -1; ii <= 1; ii++)  {
                for(int jj = -1; jj <= 1; jj++)  {
                  for(int kk = -1; kk <= 1; kk++)  {
                    int iind = i+ii, jind = j+jj, kind = k+kk;
                    // index "rotation" step
                    if( iind < 0 )  iind += mapX;
                    if( jind < 0 )  jind += mapY;
                    if( kind < 0 )  kind += mapZ;
                    if( iind >= mapX )  iind -= mapX;
                    if( jind >= mapY )  jind -= mapY;
                    if( kind >= mapZ )  kind -= mapZ;
                    // main condition
                    if( map[iind][jind][kind] < level )  {
                      inside = false;
                      break;
                    }
                  }
                  if( !inside )  break;
                }
                if( !inside )  break;
              }
              if( inside )  {
                map[i][j][k] = level + 1;
                levelUsed = true;
                MaxLevel = level;
                void_center[0] = (double)i/mapX;
                void_center[1] = (double)j/mapY;
                void_center[2] = (double)k/mapZ;
              }
            }
          }
        }
        if( !levelUsed ) // reached the last point
          break;
        level ++;
      }
      return MaxLevel;
  }
  static int PeakSortByCount(const MapUtil::peak& a, const MapUtil::peak& b)  {
    return b.count - a.count;
  }
  static int PeakSortBySum(const MapUtil::peak& a, const MapUtil::peak& b)  {
    double diff = b.summ - a.summ;
    return diff < 0 ? -1 : (diff > 0 ? 1 : 0); 
  }
protected:
  static int SortByDistance(const vec3d& a, const vec3d& b)  {
    const double d = a.QLength() - b.QLength();
    return d < 0 ? -1 : (d > 0 ? 1 : 0);
  }
  static void StandardiseVec(vec3d& v, const smatd_list& ml)  {
    vec3d tmp;
    for( int i=0; i < ml.Count(); i++ )  {
      tmp = ml[i]*v;
      for( int j=0; j < 3; j++ )  {
        while( tmp[j] < 0 )  tmp[j] += 1.0;
        while( tmp[j] > 1.0 )  tmp[j] -= 1.0;
      }
      if( (tmp[0] < v[0]) ||        // sdandardise then ...
          ( olx_abs(tmp[0]-v[0]) < 1e-5 && (tmp[1] < v[1])) ||
          (olx_abs(tmp[0]-v[0]) < 1e-5 && olx_abs(tmp[1]-v[1]) < 1e-5 && (tmp[2] < v[2])) )    {
          v = tmp;
      }
    }
  }
public:
  static void MergePeaks(const smatd_list& ml, const mat3d& cell2cart, const vec3d& norm, TArrayList<MapUtil::peak>& Peaks, TTypeList<MapUtil::peak>& out)  {
    const int cnt = Peaks.Count();
    TTypeList<vec3d> crds;
    mat3d cart2cell = cell2cart.Inverse();
    crds.SetCapacity(cnt);
    for( int i=0; i < cnt; i++ )  {
      crds.AddNew(Peaks[i].x, Peaks[i].y, Peaks[i].z) *= norm;
      Peaks[i].process = true;
    }
    
    for( int i=0; i < cnt; i++ )  {
      StandardiseVec(crds[i], ml);
      crds[i] *= cell2cart;
    }
    crds.QuickSorter.SyncSortSF(crds, Peaks, SortByDistance);
    double* Distances = new double[ cnt + 1];
    TPtrList<MapUtil::peak> toMerge;
    for( int i=0; i < cnt; i++ )
      Distances[i] = crds[i].Length();
    for( int i=0; i < cnt; i++ )  {
      if( !Peaks[i].process )  continue;
      toMerge.Clear();
      toMerge.Add( &Peaks[i] ); 
      vec3d center(crds[i]);
      for( int j=i+1; j < cnt; j++ )  {
        if( olx_abs(Distances[i]-Distances[j]) > 0.01 )  break;
        if( !Peaks[j].process )  continue;
        if( crds[i].QDistanceTo(crds[j]) < 0.0001 )  {
          toMerge.Add( &Peaks[j] );
          center += crds[j];
          Peaks[j].process = false;
        }
      }
      MapUtil::peak& p = out.AddNew();
      for( int j=0; j < toMerge.Count(); j++ )  {
        p.count += toMerge[j]->count;
        p.summ += toMerge[j]->summ;
      }
      center /= toMerge.Count();
      center *= cart2cell;
      center /= norm;
      p.x = Round(center[0]);
      p.y = Round(center[1]);
      p.z = Round(center[2]);
    }
    delete [] Distances;
  }
};

EndXlibNamespace()
#endif
