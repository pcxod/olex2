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
    TVector3<int16_t> center;
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
      peak.center = new_cent.Round<int16_t>();
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
    vec3d& void_center)
  {
    map_type level = 0, MaxLevel = 0;
    const vec3i dim(mapX, mapY, mapZ);
    //vec3i[3] dims = {vec3i(0,1,2), vec3i(2,0,1), vec3i(1,2,0)};
    while( true )  {
      bool levelUsed = false;
      for( uint16_t i=0; i < mapX; i++ )  {
        for( uint16_t j=0; j < mapY; j++ )  {
          for( uint16_t k=0; k < mapZ; k++ )  {
            // neigbouring points analysis
            bool inside = true;
            const vec3i _pt(i,j,k);
            vec3i pt = _pt;
            for( int di=0; di < 3; di++ )  {
              pt[di]++;
              if( pt[di] >= dim[di] )  pt[di] -= dim[di];
              if( map[pt[0]][pt[1]][pt[2]] < level )  {
                inside = false;
                break;
              }
              pt[di] = _pt[di] - 1;
              if( pt[di] < 0 )  pt[di] += dim[di];
              if( map[pt[0]][pt[1]][pt[2]] < level )  {
                inside = false;
                break;
              }
              pt[di] = _pt[di];  // restore...
            }
            //for( int ii = -1; ii <= 1; ii++ )  {
            //  int iind = i+ii;
            //  if( iind < 0 )  iind += mapX;
            //  if( iind >= mapX )  iind -= mapX;
            //  for( int jj = -1; jj <= 1; jj++ )  {
            //    int jind = j+jj;
            //    if( jind < 0 )  jind += mapY;
            //    if( jind >= mapY )  jind -= mapY;
            //    for( int kk = -1; kk <= 1; kk++ )  {
            //      int kind = k+kk;
            //      if( kind < 0 )  kind += mapZ;
            //      if( kind >= mapZ )  kind -= mapZ;
            //      // main condition
            //      if( map[iind][jind][kind] < level )  {
            //        inside = false;
            //        break;
            //      }
            //    }
            //    if( !inside )  break;
            //  }
            //  if( !inside )  break;
            //}
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
  // same as above, but stops the process when threshold level is reached
  template <typename map_type> static void AnalyseVoidsX(map_type*** map, uint16_t mapX, 
    uint16_t mapY, uint16_t mapZ, map_type threshold)
  {
    const vec3i dim(mapX, mapY, mapZ);
    map_type level = 0;
    while( true )  {
      bool levelUsed = false;
      for( uint16_t i=0; i < mapX; i++ )  {
        for( uint16_t j=0; j < mapY; j++ )  {
          for( uint16_t k=0; k < mapZ; k++ )  {
            // neigbouring points analysis
            bool inside = true;
            const vec3i _pt(i,j,k);
            vec3i pt = _pt;
            for( int di=0; di < 3; di++ )  {
              pt[di]++;
              if( pt[di] >= dim[di] )  pt[di] -= dim[di];
              if( map[pt[0]][pt[1]][pt[2]] < level )  {
                inside = false;
                break;
              }
              pt[di] = _pt[di] - 1;
              if( pt[di] < 0 )  pt[di] += dim[di];
              if( map[pt[0]][pt[1]][pt[2]] < level )  {
                inside = false;
                break;
              }
              pt[di] = _pt[di];  // restore...
            }
            //for( int ii = -1; ii <= 1; ii++ )  {
            //  int iind = i+ii;
            //  if( iind < 0 )  iind += mapX;
            //  if( iind >= mapX )  iind -= mapX;
            //  for( int jj = -1; jj <= 1; jj++ )  {
            //    int jind = j+jj;
            //    if( jind < 0 )  jind += mapY;
            //    if( jind >= mapY )  jind -= mapY;
            //    for( int kk = -1; kk <= 1; kk++ )  {
            //      int kind = k+kk;
            //      if( kind < 0 )  kind += mapZ;
            //      if( kind >= mapZ )  kind -= mapZ;
            //      // main condition
            //      if( map[iind][jind][kind] < level )  {
            //        inside = false;
            //        break;
            //      }
            //    }
            //    if( !inside )  break;
            //  }
            //  if( !inside )  break;
            //}
            if( inside )  {
              map[i][j][k] = level + 1;
              levelUsed = true;
            }
          }
        }
      }
      if( ++level >= threshold || !levelUsed )
        break;
    }
  }
  /* Finds the largest 'level' at which it is possible to penetratethe structure;
  expects a map prepared with AnalyseVoids; only finds unidirectional channels, it
  will also not be able to 'climb' long walls unless they are thivk enough for dir++ condition
  */
  template <typename map_type> static vec3i AnalyseChannels(map_type*** map, uint16_t mapX, uint16_t mapY, uint16_t mapZ,
    map_type max_level)
  {
    const vec3i dim(mapX, mapY, mapZ);
    map_type*** map_copy = new map_type**[mapX];
    for( size_t mi=0; mi < mapX; mi++ )  {
      map_copy[mi] = new map_type*[mapY];
      for( size_t mj=0; mj < mapY; mj++ )  {
        map_copy[mi][mj] = new map_type[mapZ];
        memcpy(map_copy[mi][mj], map[mi][mj], mapZ*sizeof(map_type));
      }
    }
    vec3i dim_ind(0, 1, 2);
    vec3i res;
    for( int dim_n=0; dim_n < 3; dim_n++ )  {
      res[dim_n] = max_level;
      if( dim_n == 1 )  // Y,Z,X
        dim_ind = vec3i(1, 2, 0);
      else if( dim_n == 2 )  // Z,X,Y
        dim_ind = vec3i(2, 0, 1);
      while( true )  {
        bool level_accessible = true;
        vec3i pt;
        // initialise starting layer
        for( uint16_t j=0; j < dim[dim_ind[1]]; j++ )  {
          for( uint16_t k=0; k < dim[dim_ind[2]]; k++ )  {
            pt[dim_ind[1]] = j;
            pt[dim_ind[2]] = k;
            if( map[pt[0]][pt[1]][pt[2]] >= res[dim_n] )  // find suitable start
              map[pt[0]][pt[1]][pt[2]] = max_level+1;
          }
        }
        for( int16_t i=1; i < dim[dim_ind[0]]; i++ )  {  // flow direction
          const int src_ind = i - 1;
          const int dest_ind = (i == dim[dim_ind[0]]-1 ? 0 : i+1);
          size_t leaks = 0;
          pt[dim_ind[0]] = i;
          for( uint16_t j=0; j < dim[dim_ind[1]]; j++ )  {
            for( uint16_t k=0; k < dim[dim_ind[2]]; k++ )  {
              pt[dim_ind[1]] = j;
              pt[dim_ind[2]] = k;
              if( map[pt[0]][pt[1]][pt[2]] < res[dim_n] )  // find suitable start
                continue;
              bool src_exists = false, dest_exists = false;
              for( int ii = -1; ii <= 1; ii++)  {
                pt[dim_ind[1]] = j+ii;
                if( pt[dim_ind[1]] < 0 )  pt[dim_ind[1]] += dim[dim_ind[1]];
                if( pt[dim_ind[1]] >= dim[dim_ind[1]] )  pt[dim_ind[1]] -= dim[dim_ind[1]];
                for( int jj = -1; jj <= 1; jj++)  {
                  pt[dim_ind[2]] = k+jj;
                  if( pt[dim_ind[2]] < 0 )  pt[dim_ind[2]] += dim[dim_ind[2]];
                  if( pt[dim_ind[2]] >= dim[dim_ind[2]] )  pt[dim_ind[2]] -= dim[dim_ind[2]];
                  pt[dim_ind[0]] = src_ind;  // check the 'source' condition
                  if( !src_exists && map[pt[0]][pt[1]][pt[2]] > max_level )
                    src_exists = true;
                  pt[dim_ind[0]] = dest_ind;  // check the 'flow' condition
                  if( !dest_exists && map[pt[0]][pt[1]][pt[2]] >= res[dim_n] )
                    dest_exists = true;
                }
              }
              if( src_exists && dest_exists )  {
                pt[dim_ind[0]] = i;  pt[dim_ind[1]] = j;  pt[dim_ind[2]] = k;
                map[pt[0]][pt[1]][pt[2]] = max_level+1;
                leaks++;
              }
            }
          }
          if( leaks == 0 )  {
            level_accessible = false;
            break;
          }
        }
        if( !level_accessible )  {
          if( res[dim_n] == 1 )  {
            res[dim_n] = 0;
            break;
          }
          res[dim_n]--;
        }
        else
          break;
      }
      for( size_t mi=0; mi < mapX; mi++ )
        for( size_t mj=0; mj < mapY; mj++ )
          memcpy(map[mi][mj], map_copy[mi][mj], mapZ*sizeof(map_type));
    }
    for( size_t mi=0; mi < mapX; mi++ )  {
      for( size_t mj=0; mj < mapY; mj++ )
        delete [] map_copy[mi][mj];
      delete [] map_copy[mi];
    }
    delete [] map_copy;
    return res;
  }
  /* a flood fill based algorithm to find undulating channels, which however come and exit at one of the 
  3 crystallographic directions */
  template <typename map_type> static vec3i AnalyseChannels1(map_type*** map, uint16_t mapX, uint16_t mapY, uint16_t mapZ,
    map_type max_level)
  {
    map_type*** map_copy = ReplicateMap(map, mapX, mapY, mapZ);
    const vec3i dim(mapX, mapY, mapZ);
    vec3i dim_ind(0, 1, 2);
    vec3i res;
    for( int dim_n=0; dim_n < 3; dim_n++ )  {
      res[dim_n] = max_level;
      if( dim_n == 1 )  // Y,Z,X
        dim_ind = vec3i(1, 2, 0);
      else if( dim_n == 2 )  // Z,X,Y
        dim_ind = vec3i(2, 0, 1);
      while( true )  {
        bool level_accessible = false;
        vec3i pt;
        TStack<vec3i> stack;
        // seed
        for( uint16_t j=0; j < dim[dim_ind[1]]; j++ )  {
          for( uint16_t k=0; k < dim[dim_ind[2]]; k++ )  {
            pt[dim_ind[1]] = j;
            pt[dim_ind[2]] = k;
            if( map[pt[0]][pt[1]][pt[2]] == res[dim_n] )  {  // find suitable start
              stack.Push(pt);
              map[pt[0]][pt[1]][pt[2]] = res[dim_n]-1;
            }
          }
        }
        while( !stack.IsEmpty() )  {
          pt = stack.Pop();
          if( pt[dim_ind[0]] == dim[dim_ind[0]]-1 )  { // a weak condition, needs more work
            level_accessible = true;
            break;
          }
          vec3i p(pt);
          for( int ii=0; ii < 3; ii++ )  {
            p[dim_ind[ii]] = pt[dim_ind[ii]]-1;
            if( p[dim_ind[ii]] >= 0 && map[p[0]][p[1]][p[2]] == res[dim_n] )  {
              stack.Push(p);
              map[p[0]][p[1]][p[2]] = res[dim_n]-1;
            } 
            p[dim_ind[ii]] = pt[dim_ind[ii]]+1;
            if( p[dim_ind[ii]] < dim[dim_ind[ii]] && map[p[0]][p[1]][p[2]] == res[dim_n] )  {
              stack.Push(p);
              map[p[0]][p[1]][p[2]] = res[dim_n]-1;
            } 
            p[dim_ind[ii]] = pt[dim_ind[ii]];
          }
        }
        if( !level_accessible )  {
          if( res[dim_n] == 1 )  {
            res[dim_n] = 0;
            break;
          }
          res[dim_n]--;
        }
        else
          break;
      }
      CopyMap(map, map_copy, mapX, mapY, mapZ);
    }
    DeleteMap(map_copy, mapX, mapY, mapZ);
    return res;
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
public:
  static void StandardiseVec(vec3d& v, const smatd_list& ml)  {
    // make sure we start from the right point...
    for( size_t j=0; j < 3; j++ )  {
      while( v[j] < 0 )  v[j] += 1.0;
      while( v[j] >= 1.0 )  v[j] -= 1.0;
    }
    bool changes = true;
    while( changes )  {
      changes = false;
      for( size_t i=0; i < ml.Count(); i++ )  {
        vec3d tmp = ml[i]*v;
        for( size_t j=0; j < 3; j++ )  {
          while( tmp[j] < 0 )  tmp[j] += 1.0;
          while( tmp[j] >= 1.0 )  tmp[j] -= 1.0;
        }
        if( (tmp[0] < v[0]) ||        // standardise then ...
            (olx_abs(tmp[0]-v[0]) < 1e-5 && (tmp[1] < v[1])) ||
            (olx_abs(tmp[0]-v[0]) < 1e-5 && olx_abs(tmp[1]-v[1]) < 1e-5 && (tmp[2] < v[2])) )    
        {
          v = tmp;
          changes = true;
        }
      }
    }
  }
  //
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
        if( crds[i].QDistanceTo(crds[j]) < 0.5 )  {
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
      center /= norm;
      p.center = center.Round<int16_t>();
    }
    for( size_t i=0; i < out.Count(); i++ )  {
      if( out[i].count == 0 )
        out.NullItem(i);
    }
    out.Pack();
  }
  template <typename map_type> static map_type*** ReplicateMap(map_type*** const map,
    size_t mapX, size_t mapY, size_t mapZ)
  {
    map_type*** map_copy = new map_type**[mapX];
    for( size_t mi=0; mi < mapX; mi++ )  {
      map_copy[mi] = new map_type*[mapY];
      for( size_t mj=0; mj < mapY; mj++ )  {
        map_copy[mi][mj] = new map_type[mapZ];
        memcpy(map_copy[mi][mj], map[mi][mj], mapZ*sizeof(map_type));
      }
    }
    return map_copy;
  }
  template <typename map_type> static map_type*** CopyMap(map_type*** dest,
    map_type*** const src, size_t mapX, size_t mapY, size_t mapZ)
  {
    for( size_t mi=0; mi < mapX; mi++ )  {
      for( size_t mj=0; mj < mapY; mj++ )  {
        memcpy(dest[mi][mj], src[mi][mj], mapZ*sizeof(map_type));
      }
    }
    return dest;
  }
  template <typename map_type> static void DeleteMap(map_type*** map,
    size_t mapX, size_t mapY, size_t mapZ)
  {
    for( size_t mi=0; mi < mapX; mi++ )  {
      for( size_t mj=0; mj < mapY; mj++ )
        delete [] map[mi][mj];
      delete [] map[mi];
    }
    delete [] map;
  }
};

EndXlibNamespace()
#endif
