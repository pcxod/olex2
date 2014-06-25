/******************************************************************************
* Copyright (c) 2004-2014 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xlib_calc_ext_H
#define __olx_xlib_calc_ext_H
#include "evalue.h"
#include "catom.h"
#include "dataitem.h"

BeginXlibNamespace()

class RefinementModel;

const uint16_t
  cv_ot_centroid = 1,
  cv_ot_line = 2,
  cv_ot_plane = 3,
  cv_vt_distance = 1,
  cv_vt_angle = 2;

class CalculatedVars {
public:
  struct Object : public AReferencible {
    olxstr name;
    TCAtomGroup atoms;
    uint16_t type;
    Object() {}
    Object(const Object &o, CalculatedVars &parent);
    ConstPtrList<const class TSAtom> GetAtoms() const;
    void ToDataItem(TDataItem &i, bool use_id) const;
    void FromDataItem(const TDataItem &i, CalculatedVars &parent, bool use_id);
    olxstr GetQualifiedName() const;
    bool IsValid() const;
  };

  struct ObjectRef {
    Object &object;
    olxstr prop; // referred property, like c or n for plane
    ObjectRef(const ObjectRef &o) : object(o.object), prop(o.prop) {
      object.IncRef();
    }
    ObjectRef(Object &o) : object(o) { o.IncRef(); }
    ObjectRef(Object &o, const olxstr &p) : object(o), prop(p) { o.IncRef(); }
    ~ObjectRef() {
      object.DecRef();
    }
    olxstr GetQualifiedName() const {
      olxstr rv = object.GetQualifiedName();
      return prop.IsEmpty() ? rv : (rv << '.' << prop);
    }
  };

  struct Var {
    olxstr name;
    ObjectRef r1, r2;
    uint16_t type;
    Var(const olxstr &n, const ObjectRef &r1, const ObjectRef &r2)
      : name(n), r1(r1), r2(r2)
    {}
    static Var *Clone(const Var &v, CalculatedVars &parent);
    TEValueD Calculate(class VcoVContainer &vcov) const;
    void ToDataItem(TDataItem &i) const;
    bool IsValid() const {
      return r1.object.IsValid() && r2.object.IsValid();
    }
    static olx_pair_t<olxstr, olxstr> parseObject(const olxstr on);
    static Var *FromDataItem(const TDataItem &i, CalculatedVars &parent);
  };
protected:
  RefinementModel &rm;
  mutable olxstr_dict<Object *> objects;
  mutable olxstr_dict<Var *> vars;
  mutable olxdict<uint64_t, smatd*, TPrimitiveComparator> eqivs;
  void Clear();
  smatd * GetEqiv(size_t idx) { return eqivs.GetValue(idx); }
public:
  CalculatedVars(RefinementModel &);
  virtual ~CalculatedVars() { Clear(); }

  void Assign(const CalculatedVars &cv);
  smatd * GetEqiv(const smatd &m);
  Object &AddObject(Object *o);
  Var &AddVar(Var *v);

  // validates the object and returns true if it is not empty
  bool Validate() const;
  void ToDataItem(TDataItem &i, bool use_id) const;
  void FromDataItem(const TDataItem &i, bool use_id);

  void CalcAll() const;
};

EndXlibNamespace()
#endif
