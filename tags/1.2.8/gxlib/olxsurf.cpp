#include "olxsurf.h"

TMolSurf::TMolSurf(const TXAtomPList &atoms_, float probe_radius)
  : atoms(atoms_),
  pr(probe_radius),
  crds(atoms.Count(), false),
  radii(atoms.Count(), false)
{
  vec3d min_v_(1000), max_v_(-1000);
  float max_r = 0;
  for (size_t i = 0; i < atoms.Count(); i++) {
    TXAtom &a = *atoms[i];
    radii[i] = a.GetType().r_vdw;
    crds[i] = a.crd();
    for (int j = 0; j < 3; j++) {
      if ((a.crd()[j] + radii[i]) > max_v_[j]) {
        max_v_[j] = a.crd()[j] + radii[i];
      }
      if ((a.crd()[j] - radii[i]) < min_v_[j]) {
        min_v_[j] = a.crd()[j] - radii[i];
      }
    }
    if (radii[i] > max_r) {
      max_r = radii[i];
    }
    center += a.crd();
  }
  center /= atoms.Count();
  max_v_ += pr * 2;
  min_v_ -= pr * 2;
  vec3d sz_ = max_v_ - min_v_;
  double max_d_ = olx_max(sz_[2], olx_max(sz_[0], sz_[1]));
  res = olx_round(96 / max_d_);
  min_v = min_v_*res;
  arr.resize(vec3d(sz_*res).Round<int>()+1, false);
  data.resize(arr.dim(), true, -1);
  for (size_t i = 0; i < arr.width; i++) {
    for (size_t j = 0; j < arr.height; j++) {
      for (size_t k = 0; k < arr.depth; k++) {
        arr[i][j][k] = 1;
      }
    }
  }
  int pid = olx_round((max_r + pr)*res),
    pidq = pid*pid;
  array_3d<bool> mask(pid+1, pid+1, pid+1, true, 0);
  for (int i1 = 0; i1 <= pid; i1++) {
    for (int i2 = 0; i2 <= pid; i2++) {
      for (int i3 = 0; i3 <= pid; i3++) {
        int rq = i1*i1 + i2*i2 + i3*i3;
        if (rq < pidq) {
          mask[i1][i2][i3] = true;
        }
      }
    }
  }
  for (size_t i = 0; i < crds.Count(); i++) {
    vec3i v = crds[i]*res - min_v;
    for (int i1 = 0; i1 < mask.width; i1++) {
      for (int i2 = 0; i2 < mask.height; i2++) {
        for (int i3 = 0; i3 < mask.depth; i3++) {
          if (!mask[i1][i2][i3]) {
            continue;
          }
          const int sm[2] = { -1, 1 };
          for (int j1 = 0; j1 < 2; j1++) {
            for (int j2 = 0; j2 < 2; j2++) {
              for (int j3 = 0; j3 < 2; j3++) {
                int x = v[0] + i1*sm[j1],
                  y = v[1] + i2*sm[j2],
                  z = v[2] + i3*sm[j3];
                if (x >= 0 && x < arr.width &&
                  y >= 0 && y < arr.height &&
                  z >= 0 && z < arr.depth)
                {
                  arr[x][y][z] = 0;
                }
              }
            }
          }
        }
      }
    }
  }
  TSurfCalculationTask calc_task(arr, data,
    min_v, res, crds, radii, pr);
  TListIteratorManager<TSurfCalculationTask> tasks(calc_task, arr.width,
    tLinearTask, 8);
}

olx_object_ptr<CIsoSurface> TMolSurf::Calculate(float iso_level) {
  olx_object_ptr<CIsoSurface> sf(new CIsoSurface(arr, &data));
  sf().GenerateSurface(iso_level);
  TArrayList<vec3f> &vertices = sf().VertexList();
  for (size_t i = 0; i < vertices.Count(); i++) {
    vertices[i] = (vertices[i] + min_v) / res;
  }
  TArrayList<int> &vertex_data = sf().GetVertexData();
  TArrayList<vec3f> &normals = sf().NormalList();
  for (size_t i = 0; i < vertex_data.Count(); i++) {
    if (vertex_data[i] != -1) {
      int idx = vertex_data[i];
      normals[i] = (vertices[i] - crds[idx]).Normalise();
      vertices[i] = crds[idx] + normals[i] * (radii[idx] + pr);
    }
  }
  return sf;

}


