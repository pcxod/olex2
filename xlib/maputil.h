/* Map utilities 
 (c) O Dolomanov, 2008
*/
#ifndef __map_util_h
#define __map_util_h

#include "xbase.h"
#include "arrays.h"
#include "estack.h"

BeginXlibNamespace()

class MapUtil  {
public:
  struct peak  { 
    uint32_t count;  //center
    TVector3<uint16_t> center;
    bool process;
    double summ;
    peak() : process(true), summ(0), count(0) {}
    peak(uint16_t _x, uint16_t _y, uint16_t _z) : process(true),  
      summ(0), count(0), center(_x, _y, _z) {}
  };
protected:
  template <typename MapT> 
  static void peak_search(MapT*** const data, uint16_t mapX, uint16_t mapY, uint16_t mapZ,
    MapT pos_level, const TArray3D<bool>& Mask, TArrayList<peak>& maxima)  
  {
    TStack<vec3i> stack;
    const TVector3<uint16_t> dim(mapX, mapY, mapZ);
    bool*** const mask = Mask.Data;
    const MapT neg_level = -pos_level; 
    for( size_t mc=0; mc < maxima.Count(); mc++ )  {
      peak& peak = maxima[mc];
      if( mask[peak.center[0]][peak.center[1]][peak.center[2]] )  continue;
      stack.Push( peak.center );
      const MapT& ref_val = data[peak.center[0]][peak.center[1]][peak.center[2]];
      vec3d new_cent;
      while( !stack.IsEmpty() )  {
        vec3i cent = stack.Pop();
        vec3i norm_cent = cent;
        peak.count++;
        new_cent += cent;
        for( size_t i=0; i < 3; i++ )  {
          while( norm_cent[i] < 0 )  
            norm_cent[i] += dim[i];
          while( norm_cent[i] >= dim[i] ) 
            norm_cent[i] -= dim[i]; 
        }
        mask[norm_cent[0]][norm_cent[1]][norm_cent[2]] = true;
        peak.summ += data[norm_cent[0]][norm_cent[1]][norm_cent[2]];
        for( int i=-1; i <= 1; i++ )  {
          int x = cent[0] + i;
          while( x < 0 )     x += mapX;
          while( x >= mapX ) x -= mapX; 
          for( int j=-1; j <= 1; j++ )  {
            int y = cent[1] + j;
            while( y < 0 )     y += mapY;
            while( y >= mapY ) y -= mapY;
            for( int k=-1; k <=1; k++ )  {
              int z = cent[2] + k;
              while( z < 0 )     z += mapZ;
              while( z >= mapZ ) z -= mapZ; 
              if( mask[x][y][z] )  continue;
              if( ref_val < 0 )  {
                if( data[x][y][z] < neg_level )  {
                  stack.Push( vec3i(cent[0]+i, cent[1]+j, cent[2]+k) );
                  mask[x][y][z] = true;
                }
              }
              else if( data[x][y][z] > pos_level )  {
                stack.Push( vec3i(cent[0]+i, cent[1]+j, cent[2]+k) );
                mask[x][y][z] = true;
              }
            }
          }
        }
      }
      new_cent /= peak.count;
      peak.center = new_cent.olx_round<int16_t>();
      for( size_t i=0; i < 3; i++ )  {
        while( peak.center[i] < 0 )  
          peak.center[i] += dim[i];
        while( peak.center[i] >= dim[i] ) 
          peak.center[i] -= dim[i]; 
      }
    }
  }

public:
  // a simple map integration, considering the peaks and holes as spheres
  template <typename MapT> static void Integrate(MapT*** const map, uint16_t mapX, uint16_t mapY, uint16_t mapZ, 
    MapT pos_level, TArrayList<MapUtil::peak>& Peaks)  
  {
    TArray3D<bool> Mask(0, mapX-1, 0, mapY-1, 0, mapZ-1);
    const MapT neg_level = -pos_level; 
    for( uint16_t ix=0; ix < mapX; ix++ )  {
      for( uint16_t iy=0; iy < mapY; iy++ )  {
        for( uint16_t iz=0; iz < mapZ; iz++ )  {
          const MapT& ref_val = map[ix][iy][iz];
          if( ref_val > pos_level || ref_val < neg_level )
            Peaks.Add( peak(ix, iy, iz) );
        }
      }
    }
    //int PointCount = mapX*mapY*mapZ;
    peak_search<MapT>(map, mapX, mapY, mapZ, pos_level, Mask, Peaks);
  }
  /* Calculates the deepest hole and its fractional coordinates, initialising the map with 'levels'
  expects a map with structure points marked as negative values and the rest - 0
  */
  template <typename map_type> static map_type AnalyseVoids(map_type*** map, uint16_t mapX, uint16_t mapY, uint16_t mapZ, 
    vec3d& void_center)  {
      map_type level = 0, MaxLevel = 0;
      while( true )  {
        bool levelUsed = false;
        for( uint16_t i=0; i < mapX; i++ )  {
          for( uint16_t j=0; j < mapY; j++ )  {
            for( uint16_t k=0; k < mapZ; k++ )  {
              // neigbouring points analysis
              bool inside = true;
              for( int ii = -1; ii <= 1; ii++)  {
                for( int jj = -1; jj <= 1; jj++)  {
                  for( int kk = -1; kk <= 1; kk++)  {
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
                MaxLevel = level+1;
                void_center[0] = i;  
                void_center[1] = j;  
                void_center[2] = k;
              }
            }
          }
        }
        if( !levelUsed ) // reached the last point
          break;
        level ++;
      }
      void_center[0] /= mapX;
      void_center[1] /= mapY;
      void_center[2] /= mapZ;
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
    bool changes = true;
    while( changes )  {
      changes = false;
      for( size_t i=0; i < ml.Count(); i++ )  {
        tmp = ml[i]*v;
        for( size_t j=0; j < 3; j++ )  {
          while( tmp[j] < 0 )  tmp[j] += 1.0;
          while( tmp[j] >= 1.0 )  tmp[j] -= 1.0;
        }
        if( (tmp[0] < v[0]) ||        // sdandardise then ...
          ( olx_abs(tmp[0]-v[0]) < 1e-5 && (tmp[1] < v[1])) ||
          (olx_abs(tmp[0]-v[0]) < 1e-5 && olx_abs(tmp[1]-v[1]) < 1e-5 && (tmp[2] < v[2])) )    
        {
          v = tmp;
          changes = true;
        }
      }
    }
  }
public:
  static void MergePeaks(const smatd_list& ml, const mat3d& cell2cart, const vec3d& norm, 
    TArrayList<MapUtil::peak>& Peaks, TTypeList<MapUtil::peak>& out)  
  {
    const size_t cnt = Peaks.Count();
    TTypeList<vec3d> crds;
    mat3d cart2cell = cell2cart.Inverse();
    crds.SetCapacity(cnt);
    for( size_t i=0; i < cnt; i++ )  {
      crds.AddNew(Peaks[i].center) *= norm;
      Peaks[i].process = true;
    }
    
    for( size_t i=0; i < cnt; i++ )  {
      StandardiseVec(crds[i], ml);
      crds[i] *= cell2cart;
    }
    crds.QuickSorter.SyncSortSF(crds, Peaks, SortByDistance);
    TPtrList<MapUtil::peak> toMerge;
    for( size_t i=0; i < cnt; i++ )  {
      if( !Peaks[i].process )  continue;
      toMerge.Clear();
      toMerge.Add( Peaks[i] ); 
      vec3d center(crds[i]);
      for( size_t j=i+1; j < cnt; j++ )  {
        if( !Peaks[j].process )  continue;
        if( crds[i].QDistanceTo(crds[j]) < 0.25 )  {
          toMerge.Add( Peaks[j] );
          center += crds[j];
          Peaks[j].process = false;
        }
      }
      MapUtil::peak& p = out.AddNew();
      for( size_t j=0; j < toMerge.Count(); j++ )  {
        p.count += toMerge[j]->count;
        p.summ += toMerge[j]->summ;
      }
      center /= toMerge.Count();
      center *= cart2cell;
      center[0] /= norm[0];  center[1] /= norm[1];  center[2] /= norm[2];
      p.center = center.olx_round<int16_t>();
    }
    for( size_t i=0; i < out.Count(); i++ )  {
      if( out[i].count == 0 )
        out.NullItem(i);
    }
    out.Pack();
  }
};

EndXlibNamespace()
#endif
