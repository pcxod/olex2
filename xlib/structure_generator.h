/******************************************************************************
* Copyright (c) 2004-2026 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/
// Claude's work on the generator
#pragma once
#include <random>

#include "asymmunit.h"
#include "unitcell.h"
#include "lattice.h"
#include "atomregistry.h"
#include "symmlib.h"
#include "refmodel.h"

BeginXlibNamespace()

struct RandomStructure {
  std::mt19937 rng;

  double special_probability, outside_probability, min_distance;

  RandomStructure(uint32_t seed)
    : rng(seed), special_probability(0.05), outside_probability(0.1),
    min_distance(1.15)
  {}

  double uniform(double lo, double hi) {
    return std::uniform_real_distribution<double>(lo, hi)(rng);
  }

  size_t uniform_index(size_t n) {
    return std::uniform_int_distribution<size_t>(0, n - 1)(rng);
  }

  static bool is_identity(const smatd& m) {
    return m.r.IsI() && m.t.IsNull();
  }

  // shortest squared Cartesian length of a fractional difference
  static double min_qdist(const TAsymmUnit& au, vec3d d);

  /* a point fixed by some involution: the midpoint of p and its nearest
    image. Screw axes and glides have no fixed points, so the result is
    verified and another operator tried
    */
  bool special_position(const TAsymmUnit& au, const TUnitCell& uc,
    vec3d& out);

  /* true if f or any of its images comes closer than min_distance to an
  accepted atom, or to another image of itself. An exact self image is a
  special position and allowed only when asked for
  */
  bool too_close(const TAsymmUnit& au, const TUnitCell& uc,
    const vec3d_list& crds, const vec3d& f, bool special) const;

  // adds up to count atoms to the AU, returns how many were placed
  size_t generate_atoms(TLattice& latt, size_t count);

  static bool check_metric(const mat3d& G, const smatd_list& ops);

  struct structure {
    TLattice latt;
    RefinementModel RefMod;
    structure()
      : latt(*(new SObjectProvider())), RefMod(latt.GetAsymmUnit())
    {}
    void reset_conn();
  };

  static olx_object_ptr<structure> create_structure(
    const vec3d& axes, const vec3d& angles, const TSpaceGroup& sg);

  static void sorted_sites(const TCAtom& a, bool interactions,
    TTypeList<TCAtom::Site>& out);

  static void compare_conn(const TAsymmUnit& ref, const TAsymmUnit& au,
    const olxstr& name);
};

EndXlibNamespace()
