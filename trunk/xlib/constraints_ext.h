#ifndef __olx_xlib_conxtraints_ext_H
#define __olx_xlib_conxtraints_ext_H
#include "catom.h"
#include "dataitem.h"

BeginXlibNamespace()

class RefinementModel;

struct rotated_adp_constraint  {
  const TCAtom &source, &destination;
  TGroupCAtom dir_from, dir_to;
  double angle;
  bool refine_angle;
  rotated_adp_constraint(TCAtom &_source, TCAtom &_destination,
    const TGroupCAtom& _dir_from, const TGroupCAtom& _dir_to,
    double _angle, bool _refine_angle)
  : source(_source),
    destination(_destination),
    dir_from(_dir_from),
    dir_to(_dir_to),
    angle(_angle),
    refine_angle(_refine_angle)
  {}
  bool IsValid() const {
    return !(dir_from.GetAtom()->IsDeleted() || dir_to.GetAtom()->IsDeleted() ||
      source.IsDeleted() || destination.IsDeleted());
  }
  olxstr ToInsStr(const RefinementModel& rm) const;
  static void FromToks(const TStrList& toks, RefinementModel& rm,
    TTypeList<rotated_adp_constraint>& out);
  static rotated_adp_constraint*
    Copy(RefinementModel& rm, const rotated_adp_constraint& c);
  static const olxstr& GetName();

  void UpdateParams(const TStrList& toks);
  void ToDataItem(TDataItem& di) const;
  static rotated_adp_constraint* FromDataItem(const TDataItem& di,
    const RefinementModel& rm);
#ifndef _NO_PYTHON
  PyObject* PyExport() const;
#endif
};

class IConstraintContainer {
public:
  virtual void FromToks(const TStrList& toks, RefinementModel& rm) = 0;
  virtual TStrList ToInsList(const RefinementModel& rm) const = 0;
  virtual void UpdateParams(size_t index, const TStrList& toks) = 0;
};

template <typename constraint_t>
class ConstraintContainer : public IConstraintContainer  {
public:
  ConstraintContainer() {}
  ~ConstraintContainer() {}
  // validates all the constraints and removes the invalid ones
  void Validate()  {
    for( size_t i=0; i < items.Count(); i++ )
      if( !items[i].IsValid() )
        items.NullItem(i);
    items.Pack();
  }
  void Clear()  {  items.Clear();  }
  void Assign(RefinementModel& rm, const ConstraintContainer& c) {
    items.Clear();
    for( size_t i=0; i < c.items.Count(); i++ )
      items.Add(constraint_t::Copy(rm, c.items[i]));
  }
  void FromToks(const TStrList& toks, RefinementModel& rm)  {
    constraint_t::FromToks(toks, rm, items);
  }
  TStrList ToInsList(const RefinementModel& rm) const {
    TStrList out;
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
  void FromDataItem(const TDataItem& di, const RefinementModel& rm)  {
    Clear();
    for( size_t i=0; i < di.ItemCount(); i++ )
      items.Add(constraint_t::FromDataItem(di.GetItem(i), rm));
  }
  void UpdateParams(size_t index, const TStrList& toks)  {
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, index, 0, items.Count());
    items[index].UpdateParams(toks);
  }
#ifndef _NO_PYTHON
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
  static const olxstr& GetName() {  return constraint_t::GetName();  }
  TTypeList<constraint_t> items;
};

EndXlibNamespace()
#endif
