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
    Sorter(const TSPlane& sp)  {  DoSort(sp);  }
    Sorter() { }
    void DoSort(const TSPlane& sp)  {
      sortedPlane.SetCount(sp.Count());
      for( size_t i=0; i < sp.Count(); i++ )
        sortedPlane[i] = sp.GetAtom(i).crd();
      olx_plane::Sort(sortedPlane, ListAccessor(sortedPlane), sp.GetCenter(),
        sp.GetNormal());
    }

    struct PointAccessor {
      const vec3d &operator() (
        const AnAssociation2<vec3d, TSAtom*> &p) const
      {
        return p.GetA();
      }
    };

    static void SortPlane(TSPlane &p) {
      TArrayList<AnAssociation2<vec3d, TSAtom*> > sorted(p.Count());
      for (size_t i=0; i < p.Count(); i++) {
        sorted[i].A() = p.GetAtom(i).crd();
        sorted[i].B() = &p.GetAtom(i);
      }
      olx_plane::Sort(sorted, PointAccessor(), p.GetCenter(), p.GetNormal());
      for (size_t i=0; i < sorted.Count(); i++)
        sorted[i].B()->SetTag((index_t)i);
      p._PlaneSortByAtomTags();
    }

    static void DoSort(const TSAtomPList& atoms,
      // tag dependent translations
      const olxdict<index_t, vec3d, TPrimitiveComparator>& transforms,
      const vec3d& center, const vec3d& normal, TSAtomPList& output)
    {
      if (atoms.IsEmpty())
        throw TInvalidArgumentException(__OlxSourceInfo, "atom list");
      TArrayList<AnAssociation2<vec3d, TSAtom*> > sorted(atoms.Count());
      for( size_t i=0; i < atoms.Count(); i++ )  {
        sorted[i].A() = atoms[i]->crd()+transforms[atoms[i]->GetTag()];
        sorted[i].B() = atoms[i];
      }
      olx_plane::Sort(sorted, PointAccessor(), center, normal);
      output.SetCount(sorted.Count());
      for( size_t i=0; i < sorted.Count(); i++ )
        output[i] = sorted[i].B();
    }
  };
};
EndXlibNamespace()
#endif
