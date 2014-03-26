/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xl_map_util_H
#define __olx_xl_map_util_H
#include "xbase.h"
#include "arrays.h"
#include "estack.h"
#undef QLength
BeginXlibNamespace()

class MapUtil  {
public:
  struct peak  {
    bool process;
    double summ;
    uint32_t count;
    vec3i center;
    peak() : process(true), summ(0), count(0)  {}
    peak(int _x, int _y, int _z) : process(true),
      summ(0), count(0), center(_x, _y, _z) {}
  };

protected:
  template <typename MapT>
  static void peak_search(MapT*** const data, const vec3s& dim,
    MapT pos_level, const TArray3D<bool>& Mask, TArrayList<peak>& maxima)
  {
    TStack<vec3i> stack;
    bool*** const mask = Mask.Data;
    const MapT neg_level = -pos_level;
    for( size_t mc=0; mc < maxima.Count(); mc++ )  {
      peak& peak = maxima[mc];
      if( mask[peak.center[0]][peak.center[1]][peak.center[2]] )  continue;
      stack.Push(peak.center);
      const MapT& ref_val = data[peak.center[0]][peak.center[1]][peak.center[2]];
      vec3d new_cent;
      while( !stack.IsEmpty() )  {
        vec3i cent = stack.Pop();
        vec3i norm_cent = cent;
        peak.count++;
        new_cent += cent;
        for( size_t i=0; i < 3; i++ )  {
          norm_cent[i] = norm_cent[i]%(int)dim[i];
          if( norm_cent[i] < 0 )
            norm_cent[i] += (int)dim[i];
        }
        mask[norm_cent[0]][norm_cent[1]][norm_cent[2]] = true;
        peak.summ += data[norm_cent[0]][norm_cent[1]][norm_cent[2]];
        vec3i pt = norm_cent, _pt = cent;
        for( int di=0; di < 3; di++ )  {
          pt[di] = (norm_cent[di]+1)%(int)dim[di];
          if( mask[pt[0]][pt[1]][pt[2]] )  continue;
          if( ref_val < 0 )  {
            if( data[pt[0]][pt[1]][pt[2]] < neg_level ||
                data[pt[0]][pt[1]][pt[2]] > pos_level )
            {
              _pt[di] = cent[di]+1;
              stack.Push(_pt);
              mask[pt[0]][pt[1]][pt[2]] = true;
            }
          }
          pt[di] = (norm_cent[di]-1)%(int)dim[di];
          if( pt[di] < 0 )  pt[di] += (int)dim[di];
          if( mask[pt[0]][pt[1]][pt[2]] )  continue;
          if( ref_val < 0 )  {
            if( data[pt[0]][pt[1]][pt[2]] < neg_level ||
                data[pt[0]][pt[1]][pt[2]] > pos_level )
            {
              _pt[di] = cent[di]-1;
              stack.Push(_pt);
              mask[pt[0]][pt[1]][pt[2]] = true;
            }
          }
          _pt[di] = cent[di];  // restore the original value
          pt[di] = norm_cent[di];
        }
      }
      new_cent /= peak.count;
      peak.center = new_cent.Round<int>();
      for( size_t i=0; i < 3; i++ )
        peak.center[i] = peak.center[i]%(int)dim[i];
    }
  }
public:
  // a simple map integration, considering the peaks and holes as spheres
  template <typename MapT> static void Integrate(MapT*** const map,
    const vec3s& map_dim_,
    const TArray3D<bool> &Mask,
    MapT pos_level, TArrayList<MapUtil::peak>& Peaks)
  {
    const vec3i map_dim(map_dim_);
    const vec3i dim_e = vec3i(Mask.GetSize());
    const MapT neg_level = -pos_level;
    for( int ix=0; ix < dim_e[0]; ix++ )  {
      for( int iy=0; iy < dim_e[1]; iy++ )  {
        for( int iz=0; iz < dim_e[2]; iz++ )  {
          if( Mask.Data[ix][iy][iz] )  continue;
          const MapT& ref_val = map[ix][iy][iz];
          if( ref_val > pos_level )  {
            bool add = true;
            for( int ii=-1; ii <=1; ii++ )  {
              int ind_x = ix + ii;
              if( ind_x < 0 ) ind_x += map_dim[0];
              if( ind_x >= map_dim[0] ) ind_x -= map_dim[0];
              for( int jj=-1; jj <=1; jj++ )  {
                int ind_y = iy + jj;
                if( ind_y < 0 ) ind_y += map_dim[1];
                if( ind_y >= map_dim[1] ) ind_y -= map_dim[1];
                for( int kk=-1; kk <=1; kk++ )  {
                  int ind_z = iz + kk;
                  if( ind_z < 0 ) ind_z += map_dim[2];
                  if( ind_z >= map_dim[2] ) ind_z -= map_dim[2];
                  if( map[ind_x][ind_y][ind_z] > ref_val )  {
                    add = false;
                    break;
                  }
                }
                if( !add )  break;
              }
              if( !add )  break;
            }
            if( add )
              Peaks.Add(peak(ix, iy, iz));
          }
          else if( ref_val < neg_level )  {
            bool add = true;
            for( int ii=-1; ii <=1; ii++ )  {
              int ind_x = ix + ii;
              if( ind_x < 0 ) ind_x += map_dim[0];
              if( ind_x >= map_dim[0] ) ind_x -= map_dim[0];
              for( int jj=-1; jj <=1; jj++ )  {
                int ind_y = iy + jj;
                if( ind_y < 0 ) ind_y += map_dim[1];
                if( ind_y >= map_dim[1] ) ind_y -= map_dim[1];
                for( int kk=-1; kk <=1; kk++ )  {
                  int ind_z = iz + kk;
                  if( ind_z < 0 ) ind_z += map_dim[2];
                  if( ind_z >= map_dim[2] ) ind_z -= map_dim[2];
                  if( map[ind_x][ind_y][ind_z] < ref_val )  {
                    add = false;
                    break;
                  }
                }
                if( !add )  break;
              }
              if( !add )  break;
            }
            if( add )
              Peaks.Add(peak(ix, iy, iz));
          }
        }
      }
    }
    peak_search<MapT>(map, map_dim_, pos_level, Mask, Peaks);
  }
  // a simple map integration, considering the peaks and holes as spheres
  template <typename MapT> static void Integrate(MapT*** const map,
    const vec3s& dim, MapT pos_level, TArrayList<MapUtil::peak>& Peaks)
  {
    TArray3D<bool> Mask(0, dim[0]-1, 0, dim[1]-1, 0, dim[2]-1);
    Integrate(map, dim, Mask, pos_level, Peaks);
  }
  // a simple map integration, considering the peaks and holes as spheres
  template <typename MapT> static double IntegrateMask(MapT*** const map,
    const vec3s& dim,
    const vec3i &_p, const TArray3D<bool> &mask)
  {
    const TVector3<index_t> d(mask.Length1()+mask.GetMin1(),
      mask.Length2()+mask.GetMin2(),
      mask.Length3()+mask.GetMin3());
    double val = 0;
    for( index_t ix=mask.GetMin1(); ix < d[0]; ix++ )  {
      for( index_t iy=mask.GetMin2(); iy < d[1]; iy++ )  {
        for( index_t iz=mask.GetMin3(); iz < d[2]; iz++ )  {
          if( !mask(ix, iy, iz) )  continue;
          TVector3<index_t> p(_p[0]+ix, _p[1]+iy, _p[2]+iz);
          for( int i=0; i < 3; i++ )  {
            if( (p[i] %= (int)dim[i]) < 0 )
              p[i] += (int)dim[i];
          }
          val += map[p[0]][p[1]][p[2]];
        }
      }
    }
    return val;
  }
  /* a flood fill based algorithm to find undulating channels, which however
  come and exit at one of the 3 crystallographic directions
  */
  template <typename map_type> static vec3i AnalyseChannels1(map_type*** map,
    const vec3s& dim, map_type max_level)
  {
    map_type*** map_copy = ReplicateMap(map, dim);
    vec3i dim_ind(0, 1, 2);
    vec3i res;
    for( size_t dim_n=0; dim_n < 3; dim_n++ )  {
      res[dim_n] = max_level;
      if( dim_n == 1 )  // Y,Z,X
        dim_ind = vec3i(1, 2, 0);
      else if( dim_n == 2 )  // Z,X,Y
        dim_ind = vec3i(2, 0, 1);
      while( true )  {
        bool level_accessible = false;
        vec3s pt;
        TStack<vec3i> stack;
        while( stack.IsEmpty() )  { // seed
          for( size_t j=0; j < dim[dim_ind[1]]; j++ )  {
            for( size_t k=0; k < dim[dim_ind[2]]; k++ )  {
              pt[dim_ind[1]] = j;
              pt[dim_ind[2]] = k;
              if( map[pt[0]][pt[1]][pt[2]] >= res[dim_n] )  {  // find suitable start
                stack.Push(pt);
                map[pt[0]][pt[1]][pt[2]] = res[dim_n]-1;
              }
            }
          }
          if( stack.IsEmpty() && --res[dim_n] == 0 ) // would be odd
            break;
        }
        while( !stack.IsEmpty() )  {
          pt = stack.Pop();
          // a weak condition, needs more work
          if( pt[dim_ind[0]] == dim[dim_ind[0]]-1 )  {
            level_accessible = true;
            break;
          }
          vec3i p(pt);
          for( int ii=0; ii < 3; ii++ )  {
            p[dim_ind[ii]] = (int)(pt[dim_ind[ii]]-1);
            if( p[dim_ind[ii]] >= 0 && map[p[0]][p[1]][p[2]] == res[dim_n] )  {
              stack.Push(p);
              map[p[0]][p[1]][p[2]] = res[dim_n]-1;
            }
            p[dim_ind[ii]] = (int)(pt[dim_ind[ii]]+1);
            if( p[dim_ind[ii]] < (int)dim[dim_ind[ii]] &&
                map[p[0]][p[1]][p[2]] >= res[dim_n] )
            {
              stack.Push(p);
              map[p[0]][p[1]][p[2]] = res[dim_n]-1;
            }
            p[dim_ind[ii]] = (int)pt[dim_ind[ii]];
          }
        }
        if( !level_accessible )  {
          if( --res[dim_n] == 0 )
            break;
        }
        else
          break;
      }
      CopyMap(map, map_copy, dim);
    }
    DeleteMap(map_copy, dim);
    return res;
  }
  /* returns fractional coordinates of the specified level, safe value must be
  unique and not occur in the map
  */
  template <typename map_type>
  static ConstTypeList<vec3d> FindLevelCenters(map_type*** map,
    const vec3i& dim, map_type level, map_type safe_value)
  {
    vec3d_list centers;
    for (int i=0; i < dim[0]; i++) {
      for (int j=0; j < dim[1]; j++) {
        for (int k=0; k < dim[2]; k++) {
          if (map[i][j][k] == level) {
            vec3i p;
            TStack<vec3i> stack;
            stack.Push(vec3i(i,j,k));
            map[i][j][k] = safe_value;
            int cnt=0;
            while( !stack.IsEmpty()) {
              vec3i p1 = stack.Pop();
              cnt++;
              p += p1;
              for (int ii=-1; ii <=1; ii++) {
                int ix = (p1[0] + ii)%dim[0];
                if( ix < 0 ) ix += dim[0];
                for (int ij=-1; ij <=1; ij++) {
                  int iy = (p1[1] + ij)%dim[1];
                  if( iy < 0 ) iy += dim[1];
                  for (int ik=-1; ik <=1; ik++) {
                    int iz = (p1[2] + ik)%dim[2];
                    if( iz < 0 ) iz += dim[2];
                    if (map[ix][iy][iz] == level) {
                      stack.Push(vec3i(p1[0]+ii, p1[1]+ij, p1[2]+ik));
                      map[ix][iy][iz] = safe_value;
                    }
                  }
                }
              }
            }
            centers.AddCopy(vec3d(p)/cnt) /= dim;
          }
        }
      }
    }
    // restore the voids
    for (int i=0; i < dim[0]; i++) {
      for (int j=0; j < dim[1]; j++) {
        for (int k=0; k < dim[2]; k++) {
          if (map[i][j][k] == safe_value) {
            map[i][j][k] = level;
          }
        }
      }
    }
    return centers;
  }
  static int PeakSortByCount(const MapUtil::peak& a, const MapUtil::peak& b)  {
    return b.count - a.count;
  }
  static int PeakSortBySum(const MapUtil::peak& a, const MapUtil::peak& b)  {
    return olx_cmp(b.summ, a.summ);
  }
  static int PeakSortByWeight(const MapUtil::peak &a, const MapUtil::peak &b)  {
    if( a.count == 0 )
      return b.count == 0 ? 0 : 1;
    else if( b.count == 0 )
      return a.count == 0 ? 0 : -1;
    return olx_cmp(b.summ/b.count, a.summ/a.count);
  }
protected:
  static int SortByDistance(const vec3d &a, const vec3d &b)  {
    return olx_cmp(a.QLength(), b.QLength());
  }
public:
  /*ml - all symmetry matrices, including identity matrix;
  norm - the reciprocal gridding (1/mapX, 1/mapY, 1/mapZ)*/
  template <class SymSpace>
  static void MergePeaks(const SymSpace& sp, const vec3d& norm,
    TArrayList<MapUtil::peak>& Peaks, TTypeList<MapUtil::peak>& out)
  {
    const size_t cnt = Peaks.Count();
    TTypeList<vec3d> crds;
    crds.SetCapacity(cnt);
    for( size_t i=0; i < cnt; i++ )  {
      crds.AddNew(Peaks[i].center) *= norm;
      Peaks[i].process = Peaks.Count() != 0;
    }
    QuickSorter::Sort(Peaks,
      FunctionComparator::Make(PeakSortByCount),
      SyncSortListener::Make(crds));
    for( size_t i=0; i < cnt; i++ )  {
      if( !Peaks[i].process )  continue;
      TPtrList<MapUtil::peak> toMerge;
      toMerge.Add(Peaks[i]);
      vec3d center(crds[i]), cmp_center(crds[i]);
      for( size_t j=i+1; j < cnt; j++ )  {
        if( !Peaks[j].process )  continue;
        for( size_t k=0; k < sp.Count(); k++ )  {
          const vec3d c = sp[k]*crds[j];
          vec3d tmp = c - cmp_center;
          const vec3i t = tmp.Round<int>();
          sp.OrthogonaliseI(tmp -= t);
          if( tmp.QLength() < 0.5 )  {
            toMerge.Add(Peaks[j]);
            const vec3d mc = c-t;
            center += mc;
            cmp_center = (cmp_center*toMerge.Count()+mc*0.25)/
              ((double)toMerge.Count()+0.25);
            Peaks[j].process = false;
            break;
          }
        }
      }
      MapUtil::peak& p = out.AddNew();
      for( size_t j=0; j < toMerge.Count(); j++ )  {
        p.count += toMerge[j]->count;
        p.summ += toMerge[j]->summ;
      }
      center /= toMerge.Count();
      center /= norm;
      p.center = center.Round<int>();
    }
    for( size_t i=0; i < out.Count(); i++ )  {
      if( out[i].count == 0 )
        out.NullItem(i);
    }
    out.Pack();
  }
  //...........................................................................
  // map getter, accessing an integral map using fractional index
  template <typename mapT, int type> struct MapGetter  {
    mapT*** const src;
    const vec3i dim;
    inline vec3i NormaliseIndex(const vec3d& i) const {
      vec3i p((int)i[0], (int)i[1], (int)i[2]);
      if( (p[0] %= dim[0]) < 0 )  p[0] += dim[0];
      if( (p[1] %= dim[1]) < 0 )  p[1] += dim[1];
      if( (p[2] %= dim[2]) < 0 )  p[2] += dim[2];
      return p;
    }
    MapGetter(mapT*** const _src, const vec3s& _dim) : src(_src), dim(_dim)  {}
    mapT Get(const TVector3<mapT>& fractional_crd) const {
      const TVector3<mapT> p(fractional_crd[0]*dim[0], fractional_crd[1]*dim[1],
        fractional_crd[2]*dim[2]);
      if( type == 0 )  {  // cropped index value
        const vec3i i = NormaliseIndex(p);
        return src[i[0]][i[1]][i[2]];
      }
      if( type == 1 )  {  // linear interpolation
        mapT val = 0;
        const vec3i fp((int)p[0], (int)p[1], (int)p[2]);
        const mapT _p = p[0]-fp[0];
        const mapT _q = p[1]-fp[1];
        const mapT _r = p[2]-fp[2];
        const mapT vx[2] = {1-_p, _p};
        const mapT vy[2] = {1-_q, _q};
        const mapT vz[2] = {1-_r, _r};
        for( int dx=0; dx <= 1; dx++ )  {
          const mapT _vx = vx[dx];
          for( int dy=0; dy <= 1; dy++ )  {
            const mapT _vy = vy[dy];
            for( int dz=0; dz <= 1; dz++ )  {
              const mapT _vz = vz[dz];
              const vec3i i = NormaliseIndex(vec3d(fp[0]+dx, fp[1]+dy, fp[2]+dz));
              val += src[i[0]][i[1]][i[2]]*_vx*_vy*_vz;
            }
          }
        }
        return val;
      }
      if( type == 2 )  {  // cubic interpolation
        vec3i fp((int)(p[0]), (int)(p[1]), (int)(p[2]));
        mapT val = 0;
        const mapT _p = p[0]-fp[0], _pc = _p*_p*_p, _ps = _p*_p;
        const mapT _q = p[1]-fp[1], _qc = _q*_q*_q, _qs = _q*_q;
        const mapT _r = p[2]-fp[2], _rc = _r*_r*_r, _rs = _r*_r;
        const mapT vx[4] = {-_pc/6 + _ps/2 -_p/3, (_pc-_p)/2 - _ps + 1, (-_pc + _ps)/2 + _p, (_pc - _p)/6 };
        const mapT vy[4] = {-_qc/6 + _qs/2 -_q/3, (_qc-_q)/2 - _qs + 1, (-_qc + _qs)/2 + _q, (_qc - _q)/6 };
        const mapT vz[4] = {-_rc/6 + _rs/2 -_r/3, (_rc-_r)/2 - _rs + 1, (-_rc + _rs)/2 + _r, (_rc - _r)/6 };
        for( int dx=-1; dx <= 2; dx++ )  {
          const mapT _vx = vx[dx+1];
          const int n_x = fp[0]+dx;
          for( int dy=-1; dy <= 2; dy++ )  {
            const float _vxy = vy[dy+1]*_vx;
            const int n_y = fp[1]+dy;
            for( int dz=-1; dz <= 2; dz++ )  {
              const float _vxyz = vz[dz+1]*_vxy;
              const vec3i i = NormaliseIndex(vec3d(n_x, n_y, fp[2]+dz));
              val += src[i[0]][i[1]][i[2]]*_vxyz;
            }
          }
        }
        return val;
      }
    }
  };
  /* fills a grid in cartesian coordinates with values from the map of the unit
  cell
  */
  template <class _MapGetter, typename dest_t> static dest_t*** Cell2Cart(
    const _MapGetter& src,
    dest_t*** dest, const vec3s& dest_d, const smatdd& grid_2_cart,
    const mat3d& cart2cell)
  {
    for( size_t i=0; i < dest_d[0]; i++ )  {
      for( size_t j=0; j < dest_d[1]; j++ )  {
        for( size_t k=0; k < dest_d[2]; k++ )  {
          // cartesian coordinates from grid
          const vec3d cc = vec3d(i,j,k)*grid_2_cart.r+grid_2_cart.t;
          const vec3d p(
            cc[0]*cart2cell[0][0] + cc[1]*cart2cell[1][0] + cc[2]*cart2cell[2][0],
            cc[1]*cart2cell[1][1] + cc[2]*cart2cell[2][1],
            cc[2]*cart2cell[2][2]
          );
          dest[i][j][k] = src.Get(p);
        }
      }
    }
    return dest;
  }
  //...........................................................................
  // map allocation/deallocation/copying uitilities
  template <typename map_type> static map_type*** ReplicateMap(
    map_type*** const map, const vec3s& dim)
  {
    map_type*** map_copy = new map_type**[dim[0]];
    for( size_t mi=0; mi < dim[0]; mi++ )  {
      map_copy[mi] = new map_type*[dim[1]];
      for( size_t mj=0; mj < dim[1]; mj++ )  {
        map_copy[mi][mj] = new map_type[dim[2]];
        memcpy(map_copy[mi][mj], map[mi][mj], dim[2]*sizeof(map_type));
      }
    }
    return map_copy;
  }
  template <typename map_type> static map_type*** CopyMap(map_type*** dest,
    map_type*** const src, const vec3s& dim)
  {
    for( size_t mi=0; mi < dim[0]; mi++ )  {
      for( size_t mj=0; mj < dim[1]; mj++ )  {
        memcpy(dest[mi][mj], src[mi][mj], dim[2]*sizeof(map_type));
      }
    }
    return dest;
  }
  // a more generic case, cannot use memcpy
  template <typename dest_map_type, typename src_map_type>
  static dest_map_type*** CopyMap(dest_map_type*** dest,
    src_map_type*** const src, const vec3s& dim)
  {
    for( size_t mi=0; mi < dim[0]; mi++ )  {
      for( size_t mj=0; mj < dim[1]; mj++ )  {
        for( size_t mk=0; mk < dim[2]; mk++ )  {
          dest[mi][mj][mk] = src[mi][mj][mk];
        }
      }
    }
    return dest;
  }
  template <typename map_type> static void DeleteMap(map_type*** map,
    const vec3s& dim)
  {
    for( size_t mi=0; mi < dim[0]; mi++ )  {
      for( size_t mj=0; mj < dim[1]; mj++ )
        delete [] map[mi][mj];
      delete [] map[mi];
    }
    delete [] map;
  }
};

EndXlibNamespace()
#endif
