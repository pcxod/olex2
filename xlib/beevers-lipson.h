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
    return Calculate<FloatT, BVFourier::TCalcEDMTask<FloatT> >(
      F, map, vol);
  }

  template <class FloatT>
  static MapInfo CalcPatt(const TArrayList<SFUtil::StructureFactor>& F,
    array_3d<FloatT> &map, double vol)
  {
    return Calculate<FloatT, BVFourier::TCalcPattTask<FloatT> >(
      F, map, vol);
  }


  template <typename FloatT, class Task> static MapInfo Calculate(
    const TArrayList<SFUtil::StructureFactor>& F,
    array_3d<FloatT> &map, double vol)
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
    for one pass calculation of the variance
    */
    MapInfo mi = { 0, 1000, -1000 };
    double sum = 0, sq_sum = 0;
    st.start("Calculating");
    Task xtask(map.data, dim, vol, F, mini, maxi, sin_cosX, sin_cosY, sin_cosZ, minInd);
    TListIteratorManager<Task> tasks(xtask, dim[0], tLinearTask, 50);
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
    double map_mean = sum / dim.Prod();
    mi.sigma = sqrt(sq_sum / dim.Prod() - map_mean*map_mean);
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
    size_t kLen, lLen;
    int minInd;
    double sum, sq_sum, vol;
    double maxVal, minVal;
    TCalcEDMTask(FloatT*** _map, const vec3s& _dim, double _volume,
      const SFList& _F, const vec3i& _min, const vec3i& _max,
      compd** _scX, compd** _scY, compd** _scZ, int _minInd) :
      map(_map), F(_F), dim(_dim),
      sin_cosX(_scX), sin_cosY(_scY), sin_cosZ(_scZ),
      mini(_min), maxi(_max),
      kLen(_max[1] - _min[1] + 1), lLen(_max[2] - _min[2] + 1), minInd(_minInd),
      sum(0), sq_sum(0), vol(_volume), maxVal(-1000), minVal(1000)
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
    void Run(size_t ix) {
      const size_t f_count = F.Count();
      for (size_t i = 0; i < f_count; i++) {
        const SFUtil::StructureFactor& sf = F[i];
        S[sf.hkl[1] - mini[1]][sf.hkl[2] - mini[2]] +=
          sf.val*sin_cosX[ix][sf.hkl[0] - minInd];
      }
      for (size_t iy = 0; iy < dim[1]; iy++) {
        for (int i = 0; i < kLen; i++) {
          int idxi = i + mini[1] - minInd;
          for (int j = 0; j < lLen; j ++) {
            T[j] += S[i][j] * sin_cosY[iy][idxi];
          }
        }
        int d2 = mini[2] - minInd;
        for (size_t iz = 0; iz < dim[2]; iz++) {
          compd R;
          for (int i = 0; i < lLen; i++) {
            R += T[i] * sin_cosZ[iz][i + d2];
          }
          const double val = R.Re() / vol;
          sum += ((val < 0) ? -val : val);
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
        F, mini, maxi, sin_cosX, sin_cosY, sin_cosZ, minInd);
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
    TCalcPattTask(FloatT*** _map, const vec3s& _dim, double _volume,
      const SFList& _F, const vec3i& _min, const vec3i& _max,
      compd** _scX, compd** _scY, compd** _scZ, int _minInd) :
      map(_map), F(_F), dim(_dim),
      mini(_min), maxi(_max),
      sin_cosX(_scX), sin_cosY(_scY), sin_cosZ(_scZ),
      kLen(_max[1] - _min[1] + 1), lLen(_max[2] - _min[2] + 1), minInd(_minInd),
      sum(0), sq_sum(0), vol(_volume), maxVal(-1000), minVal(1000)
    {
      S = new compd*[kLen];
      for (size_t i = 0; i < kLen; i++)
        S[i] = new compd[lLen];
      T = new compd[lLen];
    }
    ~TCalcPattTask() {
      for (size_t i = 0; i < kLen; i++)
        delete[] S[i];
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
          if (val > maxVal)  maxVal = val;
          if (val < minVal)  minVal = val;
          map[ix][iy][iz] = (FloatT)val;
        }
        for (size_t i = 0; i < lLen; i++)
          T[i].Null();
      }
      for (size_t i = 0; i < kLen; i++)
        for (size_t j = 0; j < lLen; j++)
          S[i][j].Null();
    }
    TCalcPattTask* Replicate() {
      return new TCalcPattTask(map, dim, vol,
        F, mini, maxi, sin_cosX, sin_cosY, sin_cosZ, minInd);
    }
  };
};

EndXlibNamespace()
#endif
