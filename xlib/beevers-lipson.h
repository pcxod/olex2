/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

/* Beevers-Lipson Fourier summation
*/
#ifndef __beevers_lipson_h
#define __beevers_lipson_h
#include "ecomplex.h"
#include "arrays.h"
#include "xbase.h"
#include "sfutil.h"
#include "arrays.h"
#include "olxmps.h"
#include "estopwatch.h"
BeginXlibNamespace()

using namespace olx_array;
class BVFourier {
public:
  struct MapInfo {
    double sigma, minVal, maxVal;
  };
  typedef TArrayList<SFUtil::StructureFactor> SFList;

  template <class FloatT>
  static MapInfo CalcEDM(const TArrayList<SFUtil::StructureFactor>& F,
    array_3d<FloatT> &map, double vol)
  {
    vec3s d = map.dim();
    return Calculate<FloatT, BVFourier::TCalcEDMTask<FloatT> >(
      F, map, vol, vec3s(0, 0, 0), vec3s(d[0] - 1, d[1] - 1, d[2] - 1));
  }

  /* As above but only for the grid points from..to inclusive. The summation
  is separable - once per x, then per x,y, then per x,y,z - so restricting
  each axis costs proportionally less, and the values inside the box are the
  ones the whole map would have had. Points outside are not written, so the
  caller has to have initialised them.

  The returned sigma is over the points computed, which is NOT the sigma of
  the cell: a box around the atoms is far from empty while the cell mostly is.
  Use CellSigma for anything that quotes a level in sigma.
  */
  template <class FloatT>
  static MapInfo CalcEDM(const TArrayList<SFUtil::StructureFactor>& F,
    array_3d<FloatT> &map, double vol, const vec3s &from, const vec3s &to)
  {
    return Calculate<FloatT, BVFourier::TCalcEDMTask<FloatT> >(
      F, map, vol, from, to);
  }

  /* Sigma of the map over the whole cell, from a coarse full map.

  A partial map cannot be contoured on its own sigma - a box around the atoms
  is far denser than the cell, which is mostly empty - and every level Olex2
  quotes is in sigma. Taking it from the structure factors by Parseval looks
  right and is not: measured against a grid-converged full map it comes out
  1.17x too large, consistently, so the relation between the P1 expanded list
  and the full sphere is not the plain one. Rather than ship a factor that
  cannot be derived, this computes the value the way it has always been
  computed and simply uses a grid too coarse to be expensive - within about
  1.5% of the converged sigma from 0.6 to 0.8 A, which is well inside what a
  contour level cares about, and a fraction of a percent of the cost of the
  map itself.
  */
  template <class FloatT>
  static double CellSigma(const TArrayList<SFUtil::StructureFactor>& F,
    double vol, const vec3d &axes, double resolution = 0.7)
  {
    vec3i d(axes * (1.0 / resolution));
    for (int i = 0; i < 3; i++) {
      d[i] = olx_max(4, d[i]);
    }
    olx_array::TArray3D<FloatT> m(0, d[0]-1, 0, d[1]-1, 0, d[2]-1);
    return Calculate<FloatT, BVFourier::TCalcEDMTask<FloatT> >(
      F, m.Data, vol, vec3s(0, 0, 0),
      vec3s(d[0]-1, d[1]-1, d[2]-1)).sigma;
  }

  template <class FloatT>
  static MapInfo CalcPatt(const TArrayList<SFUtil::StructureFactor>& F,
    array_3d<FloatT> &map, double vol)
  {
    vec3s d = map.dim();
    return Calculate<FloatT, BVFourier::TCalcPattTask<FloatT> >(
      F, map, vol, vec3s(0, 0, 0), vec3s(d[0] - 1, d[1] - 1, d[2] - 1));
  }


  template <typename FloatT, class Task> static MapInfo Calculate(
    const TArrayList<SFUtil::StructureFactor>& F,
    array_3d<FloatT> &map, double vol, const vec3s &from, const vec3s &to)
  {
    TStopWatch st(olxstr(__FUNC__) << '<' << typeid(Task).name() << '>');
    st.start("Initialising");
    vec3i mini, maxi;
    SFUtil::FindMinMax(F, mini, maxi);
    const double T_PI = 2 * M_PI;
    // precalculations
    const int minInd = olx_min(mini[2], olx_min(mini[0], mini[1]));
    const int maxInd = olx_max(maxi[2], olx_max(maxi[0], maxi[1]));
    const size_t iLen = maxInd - minInd + 1;
    vec3s dim = map.dim();
    compd** sin_cosX = new compd*[dim[0]],
      **sin_cosY, **sin_cosZ;
    for (size_t i = 0; i < dim[0]; i++) {
      sin_cosX[i] = new compd[iLen];
      for (int j = minInd; j <= maxInd; j++) {
        double rv = (double)i*j / (double)dim[0], ca, sa;
        rv *= T_PI;
        olx_sincos(-rv, &sa, &ca);
        sin_cosX[i][j - minInd].SetRe(ca);
        sin_cosX[i][j - minInd].SetIm(sa);
      }
    }
    if (dim[0] == dim[1]) {
      sin_cosY = sin_cosX;
    }
    else {
      sin_cosY = new compd*[dim[1]];
      for (size_t i = 0; i < dim[1]; i++) {
        sin_cosY[i] = new compd[iLen];
        for (int j = mini[1]; j <= maxi[1]; j++) {
          double rv = (double)i*j / (double)dim[1], ca, sa;
          rv *= T_PI;
          olx_sincos(-rv, &sa, &ca);
          sin_cosY[i][j - minInd].SetRe(ca);
          sin_cosY[i][j - minInd].SetIm(sa);
        }
      }
    }
    if (dim[0] == dim[2]) {
      sin_cosZ = sin_cosX;
    }
    else if (dim[1] == dim[2]) {
      sin_cosZ = sin_cosY;
    }
    else {
      sin_cosZ = new compd*[dim[2]];
      for (size_t i = 0; i < dim[2]; i++) {
        sin_cosZ[i] = new compd[iLen];
        for (int j = mini[2]; j <= maxi[2]; j++) {
          double rv = (double)i*j / (double)dim[2], ca, sa;
          rv *= T_PI;
          olx_sincos(-rv, &sa, &ca);
          sin_cosZ[i][j - minInd].SetRe(ca);
          sin_cosZ[i][j - minInd].SetIm(sa);
        }
      }
    }
    /* http://smallcode.weblogs.us/2006/11/27/calculate-standard-deviation-in-one-pass/
    * https://www.strchr.com/standard_deviation_in_one_pass
    for one pass calculation of the variance
    */
    MapInfo mi = { 0, 1000, -1000 };
    double sum = 0, sq_sum = 0;
    st.start("Calculating");
    Task xtask(map.data, dim, vol, F, mini, maxi, sin_cosX, sin_cosY, sin_cosZ,
      minInd, from, to);
    // one task per x plane of the box, not of the cell
    TListIteratorManager<Task> tasks(xtask, to[0] - from[0] + 1,
      tLinearTask, 50);
    for (size_t i = 0; i < tasks.Count(); i++) {
      sum += tasks[i].sum;
      sq_sum += tasks[i].sq_sum;
      if (tasks[i].minVal < mi.minVal) {
        mi.minVal = tasks[i].minVal;
      }
      if (tasks[i].maxVal > mi.maxVal) {
        mi.maxVal = tasks[i].maxVal;
      }
    }
    // over the points computed, which is the whole cell unless boxed
    const double n_pt = (double)(to[0] - from[0] + 1) *
      (to[1] - from[1] + 1) * (to[2] - from[2] + 1);
    double map_mean = sum / n_pt;
    mi.sigma = sqrt(olx_max(0., sq_sum / n_pt - map_mean * map_mean));
    // clean up of allocated data
    if (sin_cosY == sin_cosX) {
      sin_cosY = 0;
    }
    if (sin_cosZ == sin_cosX || sin_cosZ == sin_cosY) {
      sin_cosZ = 0;
    }
    for (size_t i = 0; i < dim[0]; i++) {
      delete[] sin_cosX[i];
    }
    delete[] sin_cosX;
    if (sin_cosY != 0) {
      for (size_t i = 0; i < dim[1]; i++) {
        delete[] sin_cosY[i];
      }
      delete[] sin_cosY;
    }
    if (sin_cosZ != 0) {
      for (size_t i = 0; i < dim[2]; i++) {
        delete[] sin_cosZ[i];
      }
      delete[] sin_cosZ;
    }
    return mi;
  }

  template <typename FloatT>struct TCalcEDMTask : public TaskBase {
    FloatT*** map;
    const SFList& F;
    const vec3s& dim;
    compd  **sin_cosX, **sin_cosY, **sin_cosZ;
    compd ** S, *T;
    const vec3i &mini, &maxi;
    // the grid points wanted, inclusive; the whole cell unless restricted
    const vec3s &from, &to;
    size_t kLen, lLen;
    int minInd;
    double sum, sq_sum, vol;
    double maxVal, minVal;
    TCalcEDMTask(FloatT*** _map, const vec3s& _dim, double _volume,
      const SFList& _F, const vec3i& _min, const vec3i& _max,
      compd** _scX, compd** _scY, compd** _scZ, int _minInd,
      const vec3s& _from, const vec3s& _to) :
      map(_map), F(_F), dim(_dim),
      sin_cosX(_scX), sin_cosY(_scY), sin_cosZ(_scZ),
      mini(_min), maxi(_max), from(_from), to(_to),
      kLen(_max[1] - _min[1] + 1), lLen(_max[2] - _min[2] + 1), minInd(_minInd),
      sum(0), sq_sum(0), vol(_volume),
      maxVal(-1000), minVal(1000)
    {
      S = new compd*[kLen];
      for (size_t i = 0; i < kLen; i++) {
        S[i] = new compd[lLen];
      }
      T = new compd[lLen];
    }
    ~TCalcEDMTask() {
      for (size_t i = 0; i < kLen; i++) {
        delete[] S[i];
      }
      delete[] S;
      delete[] T;
    }
    void Run(size_t i_x) {
      const size_t ix = from[0] + i_x;
      const size_t f_count = F.Count();
      for (size_t i = 0; i < f_count; i++) {
        const SFUtil::StructureFactor& sf = F[i];
        S[sf.hkl[1] - mini[1]][sf.hkl[2] - mini[2]] +=
          sf.val*sin_cosX[ix][sf.hkl[0] - minInd];
      }
      for (size_t iy = from[1]; iy <= to[1]; iy++) {
        for (size_t i = 0; i < kLen; i++) {
          int idxi = (int)i + mini[1] - minInd;
          for (size_t j = 0; j < lLen; j ++) {
            T[j] += S[i][j] * sin_cosY[iy][idxi];
          }
        }
        int d2 = mini[2] - minInd;
        for (size_t iz = from[2]; iz <= to[2]; iz++) {
          compd R;
          for (size_t i = 0; i < lLen; i++) {
            R += T[i] * sin_cosZ[iz][i + d2];
          }
          const double val = R.Re() / vol;
          sum += val;
          sq_sum += val*val;
          if (val > maxVal) {
            maxVal = val;
          }
          if (val < minVal) {
            minVal = val;
          }
          map[ix][iy][iz] = (FloatT)val;
        }
        for (size_t i = 0; i < lLen; i++) {
          T[i].Null();
        }
      }
      for (size_t i = 0; i < kLen; i++) {
        for (size_t j = 0; j < lLen; j++) {
          S[i][j].Null();
        }
      }
    }
    TCalcEDMTask* Replicate() {
      return new TCalcEDMTask(map, dim, vol,
        F, mini, maxi, sin_cosX, sin_cosY, sin_cosZ, minInd, from, to);
    }
  };

  template <typename FloatT>struct TCalcPattTask : public TaskBase {
    FloatT*** map;
    const SFList& F;
    const vec3s& dim;
    const vec3i &mini, &maxi;
    compd  **sin_cosX, **sin_cosY, **sin_cosZ;
    size_t kLen, lLen;
    compd ** S, *T;
    int minInd;
    double sum, sq_sum, vol;
    double maxVal, minVal;
    /* takes the box for signature compatibility with the EDM task - a
    Patterson is always wanted over the whole cell, so it is not used
    */
    const vec3s &from, &to;
    TCalcPattTask(FloatT*** _map, const vec3s& _dim, double _volume,
      const SFList& _F, const vec3i& _min, const vec3i& _max,
      compd** _scX, compd** _scY, compd** _scZ, int _minInd,
      const vec3s& _from, const vec3s& _to) :
      map(_map), F(_F), dim(_dim),
      mini(_min), maxi(_max), from(_from), to(_to),
      sin_cosX(_scX), sin_cosY(_scY), sin_cosZ(_scZ),
      kLen(_max[1] - _min[1] + 1), lLen(_max[2] - _min[2] + 1), minInd(_minInd),
      sum(0), sq_sum(0), vol(_volume), maxVal(-1000), minVal(1000)
    {
      S = new compd*[kLen];
      for (size_t i = 0; i < kLen; i++) {
        S[i] = new compd[lLen];
      }
      T = new compd[lLen];
    }
    ~TCalcPattTask() {
      for (size_t i = 0; i < kLen; i++) {
        delete[] S[i];
      }
      delete[] S;
      delete[] T;
    }
    void Run(size_t ix) {
      const size_t f_count = F.Count();
      for (size_t i = 0; i < f_count; i++) {
        const SFUtil::StructureFactor& sf = F[i];
        S[sf.hkl[1] - mini[1]][sf.hkl[2] - mini[2]] += sf.val*sin_cosX[ix][sf.hkl[0] - minInd];
      }
      for (size_t iy = 0; iy < dim[1]; iy++) {
        for (int i = mini[1]; i <= maxi[1]; i++) {
          for (int j = mini[2]; j <= maxi[2]; j++) {
            T[j - mini[2]] += S[i - mini[1]][j - mini[2]] * sin_cosY[iy][i - minInd];
          }
        }
        for (size_t iz = 0; iz < dim[2]; iz++) {
          compd R;
          for (int i = mini[2]; i <= maxi[2]; i++) {
            R += T[i - mini[2]] * sin_cosZ[iz][i - minInd];
          }
          const double val = R.mod() / vol;
          sum += val;
          sq_sum += val*val;
          olx_update_min_max(val, minVal, maxVal);
          map[ix][iy][iz] = (FloatT)val;
        }
        for (size_t i = 0; i < lLen; i++) {
          T[i].Null();
        }
      }
      for (size_t i = 0; i < kLen; i++) {
        for (size_t j = 0; j < lLen; j++) {
          S[i][j].Null();
        }
      }
    }
    TCalcPattTask* Replicate() {
      return new TCalcPattTask(map, dim, vol,
        F, mini, maxi, sin_cosX, sin_cosY, sin_cosZ, minInd, from, to);
    }
  };
};

EndXlibNamespace()
#endif
