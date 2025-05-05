/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xlib_conxtraints_ext_H
#define __olx_xlib_conxtraints_ext_H
#include "catom.h"
#include "dataitem.h"
#include "releasable.h"

BeginXlibNamespace()

class RefinementModel;

static const uint16_t
  direction_static = 0,
  direction_vector = 1,
  direction_normal = 2,
  direction_centroid = 3;

struct adirection : public AReferencible, public AReleasable {
  olxstr id;
  static const olxstr* type_names() {
    static olxstr ts[] = {"static", "vector", "normal", "centroid"};
    return &ts[0];
  }
  adirection(AReleasableContainer<adirection>& parent, bool tmp=false)
  : AReleasable(parent, tmp)
  {}
  adirection(AReleasableContainer<adirection>& parent, const olxstr &_id, bool tmp = false)
    : AReleasable(parent, tmp), id(_id)
  {}

  virtual vec3d get() const = 0;

  static adirection* FromDataItem(AReleasableContainer<adirection>& to,
    const TDataItem& di, const RefinementModel& rm);
  static void FromToks(AReleasableContainer<adirection>& to,
    const TStrList& toks, RefinementModel& rm);
  virtual void ToDataItem(TDataItem& di) const = 0;
  virtual bool IsValid() const = 0;
  virtual adirection* DoCopy(AReleasableContainer<adirection>& to,
    RefinementModel& rm) const = 0;
  virtual olxstr ToInsStr(const RefinementModel& rm) const = 0;
  virtual olxstr Describe() const = 0;
  virtual bool DoesMatch(const TCAtomPList& atoms) const {
    return true;
  }
  static adirection* Copy(AReleasableContainer<adirection>& to,
    RefinementModel& rm, const adirection& c)
  {
    return c.DoCopy(to, rm);
  }
  virtual adirection* CreateFromDataItem(const TDataItem& di,
    const RefinementModel& rm) const = 0;
#ifdef _PYTHON
  virtual PyObject* PyExport() const = 0;
#endif
  static const olxstr& GetName();
  static const olxstr &EncodeType(uint16_t type);
  static uint16_t DecodeType(const olxstr &type);
  void UpdateParams(const TStrList &) {}
};

struct static_direction : public adirection {
  vec3d value;
  static_direction(AReleasableContainer<adirection>& parent, bool tmp=false)
  : adirection(parent, tmp)
  {}
  static_direction(AReleasableContainer<adirection>& parent, const olxstr& id,
    const vec3d &_value)
    : adirection(parent, id), value(_value)
  {}
  virtual vec3d get() const {  return value;  }
  virtual bool IsValid() const {  return true;  }
  virtual adirection* DoCopy(AReleasableContainer<adirection>& to,
    RefinementModel &) const
  {
    return new static_direction(to, id, value);
  }
  virtual adirection* CreateFromDataItem(const TDataItem& di,
    const RefinementModel& rm) const;
  virtual olxstr ToInsStr(const RefinementModel& rm) const;
  virtual void ToDataItem(TDataItem& di) const;
  virtual olxstr Describe() const;
#ifdef _PYTHON
  virtual PyObject* PyExport() const;
#endif
};

struct direction : public adirection {
  TCAtomGroup atoms;
  uint16_t type;
  direction(AReleasableContainer<adirection>& parent, uint16_t type, bool tmp = false)
    : adirection(parent, tmp), type(type)
  {}
  direction(AReleasableContainer<adirection>& parent, const olxstr &id,
    const TCAtomGroup &_atoms,
    uint16_t _type)
  : adirection(parent, id),
    atoms(_atoms),
    type(_type)
  {}
  virtual vec3d get() const;
  virtual adirection* DoCopy(AReleasableContainer<adirection>& to,
    RefinementModel& rm) const;
  virtual adirection* CreateFromDataItem(const TDataItem& di,
    const RefinementModel& rm) const;
  virtual bool IsValid() const {
    for (size_t i = 0; i < atoms.Count(); i++) {
      if (atoms[i].GetAtom()->IsDeleted()) {
        return false;
      }
    }
    return true;
  }
  olxstr ToInsStr(const RefinementModel& rm) const;
  virtual olxstr Describe() const;
  virtual bool DoesMatch(const TCAtomPList& atoms_) const {
    for (size_t i = 0; i < atoms.Count(); i++) {
      atoms[i].GetAtom()->SetTag(0);
    }
    atoms_.ForEach(ACollectionItem::TagSetter(1));
    for (size_t i = 0; i < atoms.Count(); i++) {
      if (atoms[i].GetAtom()->GetTag() == 1) {
        return true;
      }
    }
    return false;
  }
  void ToDataItem(TDataItem& di) const;
#ifdef _PYTHON
  PyObject* PyExport() const;
#endif
};

struct rotated_adp_constraint : public AReleasable {
  const TCAtom& source, & destination;
  const adirection& dir;
  double angle;
  bool refine_angle, refs_changed;
  rotated_adp_constraint(AReleasableContainer<rotated_adp_constraint>& parent,
    TCAtom& _source, TCAtom& _destination, const adirection& _dir,
    double _angle, bool _refine_angle)
    : AReleasable(parent),
    source(_source),
    destination(_destination),
    dir(_dir),
    angle(_angle),
    refine_angle(_refine_angle), refs_changed(false)
  {
    _dir.IncRef();
  }
  ~rotated_adp_constraint() {
    if (!refs_changed) {
      dir.DecRef();
    }
  }
  bool IsValid() const {
    return !(!dir.IsValid() || source.IsDeleted() || destination.IsDeleted());
  }
  olxstr ToInsStr(const RefinementModel& rm) const;
  static void FromToks(AReleasableContainer<rotated_adp_constraint>& to,
    const TStrList& toks, RefinementModel& rm);
  static rotated_adp_constraint* Copy(
    AReleasableContainer<rotated_adp_constraint>& to,
    RefinementModel& rm, const rotated_adp_constraint& c);
  static const olxstr& GetName();
  olxstr Describe() const;
  virtual bool DoesMatch(const TCAtomPList& atoms_) const {
    source.SetTag(0);
    destination.SetTag(0);
    atoms_.ForEach(ACollectionItem::TagSetter(1));
    if (source.GetTag() == 1 || destination.GetTag() == 1) {
      return true;
    }
    return dir.DoesMatch(atoms_);
  }
  virtual void OnReleasedDelete() {
    dir.DecRef();
    refs_changed = true;
  }
  void UpdateParams(const TStrList& toks);
  void ToDataItem(TDataItem& di) const;
  static rotated_adp_constraint* FromDataItem(
    AReleasableContainer<rotated_adp_constraint>& to,
    const TDataItem& di, const RefinementModel& rm);
#ifdef _PYTHON
  PyObject* PyExport() const;
#endif
};

struct rotating_adp_constraint : public AReleasable {
  const TCAtom& source, & destination;
  double size, alpha, beta, gamma;
  bool refine_angle, refine_size;
  rotating_adp_constraint(AReleasableContainer<rotating_adp_constraint>& parent,
    TCAtom& _source, TCAtom& _destination,
    double size,
    bool refine_size,
    double alpha, double beta, double gamma, bool refine_angle)
    : AReleasable(parent), 
    source(_source),
    destination(_destination),
    size(size),
    alpha(alpha),
    beta(beta),
    gamma(gamma),
    refine_size(refine_size),
    refine_angle(refine_angle)
  {}
  ~rotating_adp_constraint() {
  }
  bool IsValid() const {
    return !(source.IsDeleted() || destination.IsDeleted());
  }
  olxstr ToInsStr(const RefinementModel& rm) const;
  static void FromToks(AReleasableContainer<rotating_adp_constraint>& to,
    const TStrList& toks, RefinementModel& rm);
  static rotating_adp_constraint* Copy(
    AReleasableContainer<rotating_adp_constraint>& to,
      RefinementModel& rm, const rotating_adp_constraint& c);
  static const olxstr& GetName();
  olxstr Describe() const;
  virtual bool DoesMatch(const TCAtomPList& atoms_) const {
    source.SetTag(0);
    destination.SetTag(0);
    atoms_.ForEach(ACollectionItem::TagSetter(1));
    return source.GetTag() == 1 || destination.GetTag() == 1;
  }
  void UpdateParams(const TStrList& toks);
  void ToDataItem(TDataItem& di) const;
  static rotating_adp_constraint* FromDataItem(
    AReleasableContainer<rotating_adp_constraint>& to,
    const TDataItem& di, const RefinementModel& rm);
#ifdef _PYTHON
  PyObject* PyExport() const;
#endif
};

struct same_group_constraint : public AReleasable {
protected:
  same_group_constraint(AReleasableContainer<same_group_constraint>& parent)
  : AReleasable(parent)
  {}
  mutable TTypeList<TCAtomPList> groups;
public:
  same_group_constraint(AReleasableContainer<same_group_constraint>& parent,
    const TTypeList<TCAtomPList>::const_list_type& groups_)
    : AReleasable(parent),
    groups(groups_)
  {}
  bool IsValid() const {
    size_t cnt = 0;
    for (size_t i = 0; i < groups.Count(); i++) {
      bool complete = true;
      for (size_t j = 0; j < groups[i].Count(); j++) {
        if (groups[i][j]->IsDeleted()) {
          complete = false;
          break;
        }
      }
      if (complete) {
        cnt++;
      }
      else {
        groups.NullItem(i);
      }
    }
    groups.Pack();
    return cnt > 1;
  }
  olxstr ToInsStr(const RefinementModel& rm) const;
  static void FromToks(AReleasableContainer<same_group_constraint>& to,
    const TStrList& toks, RefinementModel& rm);
  static same_group_constraint* Copy(
    AReleasableContainer<same_group_constraint>& to,
    RefinementModel& rm, const same_group_constraint& c);
  static const olxstr& GetName();
  olxstr Describe() const;
  virtual bool DoesMatch(const TCAtomPList& atoms_) const {
    atoms_.ForEach(ACollectionItem::TagSetter(1));
    for (size_t i = 0; i < groups.Count(); i++) {
      groups[i].ForEach(ACollectionItem::TagSetter(0));
    }
    for (size_t i = 0; i < atoms_.Count(); i++) {
      if (atoms_[i]->GetTag() == 0) {
        return true;
      }
    }
    return false;
  }
  void UpdateParams(const TStrList& toks);
  void ToDataItem(TDataItem& di) const;
  static same_group_constraint* FromDataItem(
    AReleasableContainer<same_group_constraint>& to,
    const TDataItem& di, const RefinementModel& rm);
#ifdef _PYTHON
  PyObject* PyExport() const;
#endif
};

struct tls_group_constraint : public AReleasable {
protected:
  tls_group_constraint(AReleasableContainer<tls_group_constraint>& parent)
  : AReleasable(parent)
  {}
  mutable TCAtomPList atoms;
public:
  tls_group_constraint(AReleasableContainer<tls_group_constraint>& parent,
    const TCAtomPList &atoms_)
  : AReleasable(parent),
    atoms(atoms_)
  {}
  bool IsValid() const {
    for (size_t i=0; i < atoms.Count(); i++) {
      if (atoms[i]->IsDeleted()) {
        atoms[i] = 0;
      }
    }
    atoms.Pack();
    return atoms.Count() > 3;
  }
  olxstr ToInsStr(const RefinementModel& rm) const;
  static void FromToks(AReleasableContainer<tls_group_constraint>& to,
    const TStrList& toks, RefinementModel& rm);
  static tls_group_constraint* Copy(
    AReleasableContainer<tls_group_constraint>& to,
      RefinementModel& rm, const tls_group_constraint& c);
  static const olxstr& GetName();
  olxstr Describe() const;
  virtual bool DoesMatch(const TCAtomPList& atoms_) const {
    atoms.ForEach(ACollectionItem::TagSetter(1));
    atoms_.ForEach(ACollectionItem::TagSetter(0));
    for (size_t i = 0; i < atoms.Count(); i++) {
      if (atoms[i]->GetTag() == 0) {
        return true;
      }
    }
    return false;
  }
  void UpdateParams(const TStrList& toks);
  void ToDataItem(TDataItem& di) const;
  static tls_group_constraint* FromDataItem(
    AReleasableContainer<tls_group_constraint>& to,
    const TDataItem& di, const RefinementModel& rm);
#ifdef _PYTHON
  PyObject* PyExport() const;
#endif
};

struct same_disp_constraint : public AReleasable {
protected:
  same_disp_constraint(AReleasableContainer<same_disp_constraint>& parent)
  : AReleasable(parent)
  {}
public:
  mutable TCAtomPList atoms;

  same_disp_constraint(AReleasableContainer<same_disp_constraint>& parent,
    const TCAtomPList& atoms_)
    : AReleasable(parent),
    atoms(atoms_)
  {}
  bool IsValid() const {
    for (size_t i = 0; i < atoms.Count(); i++) {
      if (atoms[i]->IsDeleted()) {
        atoms[i] = 0;
      }
    }
    atoms.Pack();
    return atoms.Count() > 1;
  }
  olxstr ToInsStr(const RefinementModel& rm) const;
  static void FromToks(AReleasableContainer<same_disp_constraint>& to,
    const TStrList& toks, RefinementModel& rm);
  static same_disp_constraint* Copy(
    AReleasableContainer<same_disp_constraint>& to,
    RefinementModel& rm, const same_disp_constraint& c);
  static const olxstr& GetName();
  olxstr Describe() const;
  virtual bool DoesMatch(const TCAtomPList& atoms_) const {
    atoms.ForEach(ACollectionItem::TagSetter(1));
    atoms_.ForEach(ACollectionItem::TagSetter(0));
    for (size_t i = 0; i < atoms.Count(); i++) {
      if (atoms[i]->GetTag() == 0) {
        return true;
      }
    }
    return false;
  }
  void UpdateParams(const TStrList& toks);
  void ToDataItem(TDataItem& di) const;
  static same_disp_constraint* FromDataItem(
    AReleasableContainer<same_disp_constraint>& to,
    const TDataItem& di, const RefinementModel& rm);
#ifdef _PYTHON
  PyObject* PyExport() const;
#endif
};

class IConstraintContainer {
public:
  virtual ~IConstraintContainer() {}
  virtual void FromToks(const TStrList& toks, RefinementModel& rm) = 0;
  virtual const_strlist ToInsList(const RefinementModel& rm,
    const TCAtomPList* atoms, TPtrList<AReleasable>* processed) const = 0;
  virtual void UpdateParams(size_t index, const TStrList& toks) = 0;
  virtual const olxstr& GetName() const = 0;
  virtual void ValidateAll() = 0;
  virtual void Clear() = 0;
  virtual TDataItem& ToDataItem(TDataItem& di) const = 0;
  virtual void FromDataItem(const TDataItem* di, const RefinementModel& rm) = 0;
  virtual void Assign(RefinementModel& rm, const IConstraintContainer& c) = 0;
#ifdef _PYTHON
  virtual PyObject* PyExport() const = 0;
#endif
};

template <typename constraint_t>
class ConstraintContainer : public IConstraintContainer,
  public AReleasableContainer<constraint_t>
{
protected:
  virtual void OnRestore(constraint_t& item) {

  }
  virtual void OnRelease(constraint_t& item) {

  }
public:
  ConstraintContainer() {}
  ~ConstraintContainer() {
  }
  virtual void Clear() {
    AReleasableContainer<constraint_t>::Clear();
  }
  // validates all the constraints and removes the invalid ones
  void ValidateAll() {
    for (size_t i = 0; i < this->Count(); i++) {
      if (!this->GetItem(i).IsValid()) {
        this->items.NullItem(i);
      }
    }
    this->items.Pack();
  }

  void Assign(RefinementModel& rm, const IConstraintContainer& cn_) {
    const ConstraintContainer& cn =
      dynamic_cast<const ConstraintContainer&>(cn_);
    Clear();
    for (size_t i = 0; i < cn.Count(); i++) {
      constraint_t::Copy(*this, rm, cn.GetItem(i));
    }
  }
  void FromToks(const TStrList& toks, RefinementModel& rm) {
    constraint_t::FromToks(*this, toks, rm);
  }

  const_strlist ToInsList(const RefinementModel& rm,
    const TCAtomPList* atoms, TPtrList<AReleasable>* processed) const
  {
    TStrList out;
    out.SetCapacity(this->Count());
    for (size_t i = 0; i < this->Count(); i++) {
      constraint_t& c = this->GetItem(i);
      if (c.IsValid()) {
        if (atoms == 0 || c.DoesMatch(*atoms)) {
          out.Add(c.ToInsStr(rm));
          if (processed != 0) {
            processed->Add(c);
          }
        }
      }
    }
    return out;
  }

  TDataItem& ToDataItem(TDataItem& di) const {
    size_t cnt = 0;
    for (size_t i = 0; i < this->Count(); i++) {
      if (this->GetItem(i).IsValid()) {
        this->GetItem(i).ToDataItem(di.AddItem(cnt++));
      }
    }
    return di;
  }

  void FromDataItem(const TDataItem* di, const RefinementModel& rm) {
    Clear();
    if (di != 0) {
      FromDataItem(*di, rm);
    }
  }

  void FromDataItem(const TDataItem& di, const RefinementModel& rm) {
    Clear();
    for (size_t i = 0; i < di.ItemCount(); i++) {
      constraint_t::FromDataItem(*this, di.GetItemByIndex(i), rm);
    }
  }

  void UpdateParams(size_t index, const TStrList& toks) {
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, index, 0, this->Count());
    this->GetItem(index).UpdateParams(toks);
  }
#ifdef _PYTHON
  PyObject* PyExport() const {
    size_t cnt = 0;
    for (size_t i = 0; i < this->Count(); i++) {
      if (this->GetItem(i).IsValid()) {
        cnt++;
      }
    }
    PyObject* all = PyTuple_New(cnt);
    cnt = 0;
    for (size_t i = 0; i < this->Count(); i++) {
      if (this->GetItem(i).IsValid()) {
        PyTuple_SetItem(all, cnt++, this->GetItem(i).PyExport());
      }
    }
    return all;
  }
#endif
  const olxstr& GetName() const { return constraint_t::GetName(); }
  
};

EndXlibNamespace()
#endif
