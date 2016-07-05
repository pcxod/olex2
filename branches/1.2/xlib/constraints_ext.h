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

BeginXlibNamespace()

class RefinementModel;

static const uint16_t
  direction_static = 0,
  direction_vector = 1,
  direction_normal = 2,
  direction_centroid = 3;

struct adirection : public AReferencible {
  olxstr id;
  static const olxstr* type_names() {
    static olxstr ts[] = {"static", "vector", "normal", "centroid"};
    return &ts[0];
  }
  adirection() {}
  adirection(const olxstr &_id) : id(_id) {}
  virtual vec3d get() const = 0;

  static adirection* FromDataItem(const TDataItem& di,
    const RefinementModel& rm);
  static void FromToks(const TStrList& toks, RefinementModel& rm,
    TTypeList<adirection>& out);
  virtual void ToDataItem(TDataItem& di) const = 0;
  virtual bool IsValid() const = 0;
  virtual adirection* DoCopy(RefinementModel& rm) const = 0;
  virtual olxstr ToInsStr(const RefinementModel& rm) const = 0;
  virtual olxstr Describe() const = 0;
  static adirection* Copy(RefinementModel& rm, const adirection& c) {
    return c.DoCopy(rm);
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
  static_direction() {}
  static_direction(const olxstr& id, const vec3d &_value)
    : adirection(id), value(_value) {}
  virtual vec3d get() const {  return value;  }
  virtual bool IsValid() const {  return true;  }
  virtual adirection* DoCopy(RefinementModel &) const {
    return new static_direction(id, value);
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
  direction() {}
  direction(const olxstr &id, const TCAtomGroup &_atoms,
    uint16_t _type)
  : adirection(id),
    atoms(_atoms),
    type(_type)
  {}
  virtual vec3d get() const;
  virtual adirection* DoCopy(RefinementModel& rm) const;
  virtual adirection* CreateFromDataItem(const TDataItem& di,
    const RefinementModel& rm) const;
  virtual bool IsValid() const {
    for( size_t i=0; i < atoms.Count(); i++ )
      if( atoms[i].GetAtom()->IsDeleted() )
        return false;
    return true;
  }
  olxstr ToInsStr(const RefinementModel& rm) const;
  virtual olxstr Describe() const;
  void ToDataItem(TDataItem& di) const;
#ifdef _PYTHON
  PyObject* PyExport() const;
#endif
};

struct rotated_adp_constraint  {
  const TCAtom &source, &destination;
  adirection &dir;
  double angle;
  bool refine_angle;
  rotated_adp_constraint(TCAtom &_source, TCAtom &_destination,
    adirection &_dir,
    double _angle, bool _refine_angle)
  : source(_source),
    destination(_destination),
    dir(_dir),
    angle(_angle),
    refine_angle(_refine_angle)
  {
    _dir.IncRef();
  }
  ~rotated_adp_constraint()  {
    dir.DecRef();
  }
  bool IsValid() const {
    return !(!dir.IsValid() || source.IsDeleted() || destination.IsDeleted());
  }
  olxstr ToInsStr(const RefinementModel& rm) const;
  static void FromToks(const TStrList& toks, RefinementModel& rm,
    TTypeList<rotated_adp_constraint>& out);
  static rotated_adp_constraint*
    Copy(RefinementModel& rm, const rotated_adp_constraint& c);
  static const olxstr& GetName();
  olxstr Describe() const;
  void UpdateParams(const TStrList& toks);
  void ToDataItem(TDataItem& di) const;
  static rotated_adp_constraint* FromDataItem(const TDataItem& di,
    const RefinementModel& rm);
#ifdef _PYTHON
  PyObject* PyExport() const;
#endif
};

struct same_group_constraint  {
protected:
  same_group_constraint() {}
public:
  TTypeList<TCAtomPList> groups;
  same_group_constraint(const TTypeList<TCAtomPList>::const_list_type &groups_)
  : groups(groups_)
  {}
  bool IsValid()  {
    size_t cnt=0;
    for( size_t i=0; i < groups.Count(); i++ )  {
      bool complete = true;
      for( size_t j=0; j < groups[i].Count(); j++ )  {
        if( groups[i][j]->IsDeleted() )  {
          complete = false;
          break;
        }
      }
      if( complete )
        cnt++;
      else
        groups.NullItem(i);
    }
    groups.Pack();
    return cnt > 1;
  }
  olxstr ToInsStr(const RefinementModel& rm) const;
  static void FromToks(const TStrList& toks, RefinementModel& rm,
    TTypeList<same_group_constraint>& out);
  static same_group_constraint*
    Copy(RefinementModel& rm, const same_group_constraint& c);
  static const olxstr& GetName();
  olxstr Describe() const;
  void UpdateParams(const TStrList& toks);
  void ToDataItem(TDataItem& di) const;
  static same_group_constraint* FromDataItem(const TDataItem& di,
    const RefinementModel& rm);
#ifdef _PYTHON
  PyObject* PyExport() const;
#endif
};

struct tls_group_constraint {
protected:
  tls_group_constraint() {}
public:
  TCAtomPList atoms;
  tls_group_constraint(const TCAtomPList &atoms_)
  : atoms(atoms_)
  {}
  bool IsValid() {
    for (size_t i=0; i < atoms.Count(); i++) {
      if (atoms[i]->IsDeleted())
        atoms[i] = NULL;
    }
    atoms.Pack();
    return atoms.Count() > 3;
  }
  olxstr ToInsStr(const RefinementModel& rm) const;
  static void FromToks(const TStrList& toks, RefinementModel& rm,
    TTypeList<tls_group_constraint>& out);
  static tls_group_constraint*
    Copy(RefinementModel& rm, const tls_group_constraint& c);
  static const olxstr& GetName();
  olxstr Describe() const;
  void UpdateParams(const TStrList& toks);
  void ToDataItem(TDataItem& di) const;
  static tls_group_constraint* FromDataItem(const TDataItem& di,
    const RefinementModel& rm);
#ifdef _PYTHON
  PyObject* PyExport() const;
#endif
};
class IConstraintContainer {
public:
  virtual ~IConstraintContainer() {}
  virtual void FromToks(const TStrList& toks, RefinementModel& rm) = 0;
  virtual const_strlist ToInsList(const RefinementModel& rm) const = 0;
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
class ConstraintContainer : public IConstraintContainer  {
public:
  ConstraintContainer() {}
  ~ConstraintContainer() {}
  // validates all the constraints and removes the invalid ones
  void ValidateAll()  {
    for( size_t i=0; i < items.Count(); i++ )
      if( !items[i].IsValid() )
        items.NullItem(i);
    items.Pack();
  }
  void Clear()  {  items.Clear();  }
  void Assign(RefinementModel& rm, const IConstraintContainer& cn_) {
    const ConstraintContainer* cn =
      dynamic_cast<const ConstraintContainer*>(&cn_);
    items.Clear();
    for( size_t i=0; i < cn->items.Count(); i++ )
      items.Add(constraint_t::Copy(rm, cn->items[i]));
  }
  void FromToks(const TStrList& toks, RefinementModel& rm)  {
    constraint_t::FromToks(toks, rm, items);
  }
  const_strlist ToInsList(const RefinementModel& rm) const {
    TStrList out;
    out.SetCapacity(items.Count());
    for( size_t i=0; i < items.Count(); i++ )
      if( items[i].IsValid() )
        out.Add(items[i].ToInsStr(rm));
    return out;
  }
  TDataItem& ToDataItem(TDataItem& di) const {
    size_t cnt=0;
    for( size_t i=0; i < items.Count(); i++ )  {
      if( items[i].IsValid() )
        items[i].ToDataItem(di.AddItem(cnt++));
    }
    return di;
  }
  void FromDataItem(const TDataItem* di, const RefinementModel& rm)  {
    Clear();
    if( di != NULL )
      FromDataItem(*di, rm);
  }
  void FromDataItem(const TDataItem& di, const RefinementModel& rm)  {
    Clear();
    for( size_t i=0; i < di.ItemCount(); i++ )
      items.Add(constraint_t::FromDataItem(di.GetItemByIndex(i), rm));
  }
  void UpdateParams(size_t index, const TStrList& toks)  {
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, index, 0, items.Count());
    items[index].UpdateParams(toks);
  }
#ifdef _PYTHON
  PyObject* PyExport() const {
    size_t cnt = 0;
    for( size_t i=0; i < items.Count(); i++ )  {
      if( items[i].IsValid() )
        cnt++;
    }
    PyObject* all = PyTuple_New(cnt);
    cnt = 0;
    for( size_t i=0; i < items.Count(); i++ )  {
      if( items[i].IsValid() )
        PyTuple_SetItem(all, cnt++, items[i].PyExport());
    }
    return all;
  }
#endif
  const olxstr& GetName() const {  return constraint_t::GetName();  }
  TTypeList<constraint_t> items;
};

EndXlibNamespace()
#endif
