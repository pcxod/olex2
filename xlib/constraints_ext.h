#ifndef __olx_xlib_conxtraints_ext_H
#define __olx_xlib_conxtraints_ext_H
#include "catom.h"

BeginXlibNamespace()

class RefinementModel;

struct shared_rotated_adp_constraint  {
  const TCAtom &reference, &atom;
  TGroupCAtom dir_from, dir_to;
  double angle;
  shared_rotated_adp_constraint(TCAtom &_reference, TCAtom &_atom,
    const TGroupCAtom& _dir_from, const TGroupCAtom& _dir_to,
    double _angle)
  : atom(_atom),
    reference(_reference),
    dir_from(_dir_from),
    dir_to(_dir_to),
    angle(_angle)
  {}
  bool IsValid() const {
    return !(dir_from.GetAtom()->IsDeleted() || dir_to.GetAtom()->IsDeleted() ||
      reference.IsDeleted() || atom.IsDeleted());
  }
  olxstr ToInsStr(const RefinementModel& rm) const;
  static void FromToks(const TStrList& toks, RefinementModel& rm,
    TTypeList<shared_rotated_adp_constraint>& out);
  static shared_rotated_adp_constraint*
    Copy(RefinementModel& rm, const shared_rotated_adp_constraint& c);
#ifndef _NO_PYTHON
  PyObject* PyExport() const;
#endif
};

template <typename constraint_t> class ConstraintContainer {
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
  TTypeList<constraint_t> items;
};

EndXlibNamespace()
#endif
