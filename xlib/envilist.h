/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xl_envilist_H
#define __olx_xl_envilist_H
#include "xbase.h"
#include "symmat.h"
#include "typelist.h"
#include "tptrlist.h"
#include "satom.h"
BeginXlibNamespace()

class TUnitCell;

class TAtomEnvi : public IOlxObject {
  typedef AnAssociation3<TCAtom*, smatd, vec3d> elm_t;
  TTypeList<elm_t>  Envi;
  const TSAtom *Base;
  int _SortByDistance(const elm_t &i1, const elm_t &i2) const
  {
    return olx_cmp(i1.GetC().QDistanceTo(Base->crd()),
      i2.GetC().QDistanceTo(Base->crd()));
  }
public:
  TAtomEnvi(const TSAtom &base)
    : Base(&base)
  {}

  TAtomEnvi(const TAtomEnvi &e)
    : Envi(e.Envi),
    Base(e.Base)
  {}

  void Add(TCAtom& ca, const smatd& matr, const vec3d& crd) {
    Envi.AddNew(&ca, matr, crd);
  }

  const TSAtom& GetBase() const { return *Base; }

  size_t Count() const { return Envi.Count(); }
  bool IsEmpty() const { return Envi.IsEmpty(); }
  const TAsymmUnit &GetAU() const;
  const TUnitCell &GetUC() const;
  const TLattice &GetLatt() const;

  const olxstr& GetLabel(size_t ind) const {
    return Envi[ind].a->GetLabel();
  }
  const cm_Element& GetType(size_t ind) const {
    return Envi[ind].a->GetType();
  }
  TCAtom& GetCAtom(size_t ind) const { return *Envi[ind].a; }
  const vec3d& GetCrd(size_t ind) const { return Envi[ind].GetC(); }
  const vec3d& crd() const { return Base->crd(); }
  vec3d GetVec(size_t ind) const {
    return Envi[ind].GetC() - Base->crd();
  }
  const smatd& GetMatrix(size_t ind) const { return Envi[ind].GetB(); }
  
  // returns distance to the given atom index in Angstrems
  double Distance(size_t i) const {
    return GetCrd(i).DistanceTo(Base->crd());
  }
  // returns cosine of the angle between two vertices
  double CAngle(size_t i, size_t j) const {
    return (GetCrd(i) - Base->crd()).CAngle(GetCrd(j) - Base->crd());
  }
  // returns angle between two vertices in degrees
  double Angle(size_t i, size_t j) const {
    return acos(CAngle(i, j)) * M_PI / 180;
  }
  // counts number of atoms with particular Z
  size_t CountZ(int z) const;
  // returns indices of the atoms with specified Z
  TSizeList::const_list_type GetZIndices(int z) const;

  void Delete(size_t i) { Envi.Delete(i); }
  void SortByDistance() {
    QuickSorter::SortMF(Envi, *this, &TAtomEnvi::_SortByDistance);
  }
  /* beware as this may cause problems with symmetry equivalents - only
  the first instance will be removed!
  */
  void Exclude(TCAtom& ca);
  // excludes given indices
  void ExcludeIndices(const TSizeList &indices);
  //removes all but the given indices
  void LeaveIndices(const TSizeList &indices);
  /* counts covalent bonds only. The list may have mixed bonds if modified
  externally, like in the case of placing H atoms on O
  */
  size_t CountCovalent() const;
  /* applies a symmetry operation to all matrices and recalculates the
  coordinates
  */
  void ApplySymm(const smatd& sym);

  TAtomEnvi &operator = (const TAtomEnvi &e) {
    Base = e.Base;
    Envi = e.Envi;
    return *this;
  }
#ifdef _PYTHON
  PyObject* PyExport(TPtrList<PyObject>& atoms);
#endif
};

typedef TPtrList<TAtomEnvi> TAtomEnviPList;
typedef TTypeList<TAtomEnvi> TAtomEnviList;

EndXlibNamespace()
#endif
