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

class TAtomEnvi  {
  TTypeList<AnAssociation3<TCAtom*, smatd, vec3d> >  Envi;
  TSAtom *Base;
  int _SortByDistance(const AnAssociation3<TCAtom*, smatd, vec3d> &i1,
    const AnAssociation3<TCAtom*, smatd, vec3d> &i2) const
  {
    return olx_cmp(i1.GetC().QDistanceTo(Base->crd()),
      i2.GetC().QDistanceTo(Base->crd()));
  }
public:
  TAtomEnvi() : Base(NULL) {}
  virtual ~TAtomEnvi() {}

  void Add(TCAtom& ca, const smatd& matr, const vec3d& crd) {
    Envi.AddNew(&ca, matr, crd);
  }

  void Clear() {  Envi.Clear();  }

  size_t Count() const { return Envi.Count(); }
  bool IsEmpty() const { return Envi.IsEmpty(); }

  TSAtom& GetBase() const { return *Base; }
  void SetBase(TSAtom& base) { Base = &base; }
  const olxstr& GetLabel(size_t ind) const {
    return Envi[ind].A()->GetLabel();
  }
  const cm_Element& GetType(size_t ind) const {
    return Envi[ind].A()->GetType();
  }
  TCAtom& GetCAtom(size_t ind) const { return *Envi[ind].A(); }
  const vec3d& GetCrd(size_t ind) const { return Envi[ind].GetC(); }
  const smatd& GetMatrix(size_t ind) const { return Envi[ind].GetB(); }
  void Delete(size_t i) { Envi.Delete(i); }
  void SortByDistance() {
    QuickSorter::SortMF(Envi, *this, &TAtomEnvi::_SortByDistance);
  }
  void Exclude(TCAtom& ca) {
    for (size_t i=0; i < Envi.Count(); i++) {
      if (Envi[i].GetA() == &ca) {
        Envi.Delete(i);
        break;
      }
    }
  }
  /* counts covalent bonds only. The list may have mixed bonds if modified
  externally, like in the case of placing H atoms on O
  */
  size_t CountCovalent() const;
  /* applies a symmetry operation to all matrices and recalculates the
  coordinates
  */
  void ApplySymm(const smatd& sym);
#ifdef _PYTHON
  PyObject* PyExport(TPtrList<PyObject>& atoms);
#endif
};

typedef TPtrList<TAtomEnvi> TAtomEnviPList;
typedef TTypeList<TAtomEnvi> TAtomEnviList;

EndXlibNamespace()
#endif


