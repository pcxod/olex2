/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef _olx_plane_sort_H
#define _olx_plane_sort_H
#include "splane.h"
#include "edict.h"
#include "math/plane.h"

BeginXlibNamespace()
namespace PlaneSort {
  struct Sorter {
    TArrayList<vec3d> sortedPlane;
    Sorter(const TSPlane& sp) { DoSort(sp); }
    Sorter() { }
    void DoSort(const TSPlane& sp) {
      sortedPlane.SetCount(sp.Count());
      for (size_t i = 0; i < sp.Count(); i++) {
        sortedPlane[i] = sp.GetAtom(i).crd();
      }
      plane::Sort(sortedPlane, ListAccessor(sortedPlane), sp.GetCenter(),
        sp.GetNormal());
    }

    struct PointAccessor {
      const vec3d &operator() (
        const olx_pair_t<vec3d, TSAtom*> &p) const
      {
        return p.GetA();
      }
    };

    /* Sorts the plane atoms and makes its normal  right-handed*/
    static void SortPlane(TSPlane &p) {
      TArrayList<olx_pair_t<vec3d, TSAtom*> > sorted(p.Count());
      for (size_t i = 0; i < p.Count(); i++) {
        sorted[i].a = p.GetAtom(i).crd();
        sorted[i].b = &p.GetAtom(i);
      }
      plane::Sort(sorted, PointAccessor(), p.GetCenter(), p.GetNormal());
      for (size_t i = 0; i < sorted.Count(); i++) {
        sorted[i].b->SetTag((index_t)i);
      }
      p._PlaneSortByAtomTags();
    }

    static void DoSort(const TSAtomPList& atoms,
      // tag dependent translations
      const olx_pdict<index_t, vec3d>& transforms,
      const vec3d& center, const vec3d& normal, TSAtomPList& output)
    {
      if (atoms.IsEmpty())
        throw TInvalidArgumentException(__OlxSourceInfo, "atom list");
      TArrayList<olx_pair_t<vec3d, TSAtom*> > sorted(atoms.Count());
      for (size_t i = 0; i < atoms.Count(); i++) {
        sorted[i].a = atoms[i]->crd() + transforms.Get(atoms[i]->GetTag());
        sorted[i].b = atoms[i];
      }
      plane::Sort(sorted, PointAccessor(), center, normal);
      output.SetCount(sorted.Count());
      for (size_t i = 0; i < sorted.Count(); i++) {
        output[i] = sorted[i].b;
      }
    }
  };
};
EndXlibNamespace()
#endif
