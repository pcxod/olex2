#ifndef __gxlib_olxsurf_H
#define __gxlib_olxsurf_H
#include "arrays.h"
#include "olxmps.h"
#include "unitcell.h"
#include "xatom.h"
#include "IsoSurface.h"

using namespace olx_array;

struct TSurfCalculationTask : public TaskBase {
  array_3d<float> &array;
  array_3d<int> &data;
  const vec3f &min_val;
  float res;
  float pr;
  const array_1d<vec3f> &crds;
  const array_1d<float> &radii;
  TSurfCalculationTask(
    array_3d<float> &array_,
    array_3d<int> &data_,
    const vec3f &min_val_,
    float res_,
    const array_1d<vec3f> &crds_,
    const array_1d<float> &radii_, float pr_)
    : array(array_),
    data(data_),
    min_val(min_val_),
    res(res_), crds(crds_), radii(radii_), pr(pr_)
  {}
  void Run(size_t i1) {
    float x = (min_val[0] + i1) / res;
    for (size_t i2 = 0; i2 < array.height; i2++) {
      const float y = (min_val[1] + i2) / res;
      for (size_t i3 = 0; i3 < array.depth; i3++) {
        if (array[i1][i2][i3] != 0) {
          continue;
        }
        vec3f p(x, y, (min_val[2] + i3) / res);
        for (size_t i = 0; i < crds.width; i++) {
          float qd = crds[i].QDistanceTo(p);
          if (qd < olx_sqr(radii[i] + pr)) {
            data[i1][i2][i3] = (int)i;
            array[i1][i2][i3] = -1;
          }
        }
      }
    }
  }
  TSurfCalculationTask * Replicate() const {
    return new TSurfCalculationTask(array, data,
      min_val, res, crds, radii, pr);
  }
};

struct TSurfCalculationTask1 : public TaskBase {
  const TTypeList<vec3f> &vertices;
  array_1d<float> &values;
  TUnitCell &uc;
  const TAsymmUnit &au;
  TTypeList<AnAssociation3<TCAtom*, smatd, vec3d> > res;
  TSurfCalculationTask1(
    const TTypeList<vec3f> &vertices_,
    array_1d<float> &values_,
    TUnitCell &uc_)
    : vertices(vertices_),
    values(values_),
    uc(uc_), au(uc_.GetLattice().GetAsymmUnit())
  {}
  void Run(size_t i) {
    uc.FindInRangeAMC(au.Fractionalise(vertices[i]), 1e-3, 5,
      res.SetCount(0, false));
    double v = 0;
    for (size_t j = 0; j < res.Count(); j++) {
      v += res[j].GetA()->GetType().GetMr()*res[j].GetA()->GetChemOccu() /
        vertices[i].QDistanceTo(res[j].GetC());
    }
    if (v != 0) {
      v = 1.0 / v;
    }
    values[i] = (float)v;
  }
  TSurfCalculationTask1 * Replicate() const {
    return new TSurfCalculationTask1(vertices, values, uc);
  }
};

class TMolSurf {
  const TXAtomPList &atoms;
  float pr;
  vec3f center;
  array_1d<vec3f> crds;
  array_1d<float> radii;
  array_3d<float> arr;
  array_3d<int> data;
  vec3f min_v;
  float res;
public:
  TMolSurf(const TXAtomPList &atoms_, float probe_radius);
  olx_object_ptr<CIsoSurface> Calculate(float iso_level);

  const vec3f &GetCenter() const {
    return center;
  }
};

#endif
