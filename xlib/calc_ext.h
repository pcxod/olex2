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
#include "cifdp.h"
#include "vcov.h"

BeginXlibNamespace()

class RefinementModel;

const uint16_t
  cv_ot_centroid = 1,
  cv_ot_line = 2,
  cv_ot_plane = 3,
  cv_vt_distance = 1,
  cv_vt_angle = 2,
  cv_vt_shift = 3;

class CalculatedVars {
public:
  struct Object : public AReferencible {
    olxstr name;
    TCAtomGroup atoms;
    uint16_t type;
    // initialised when saving
    mutable size_t group_id, obj_id;
    Object(uint16_t type)
    : type(type), group_id(InvalidIndex), obj_id(InvalidIndex)
    {}
    Object(const Object &o, CalculatedVars &parent);
    ConstPtrList<class TSAtom> GetAtoms() const;
    virtual TDataItem& ToDataItem(TDataItem &i, bool use_id) const;
    virtual void FromDataItem_(const TDataItem &i,
      CalculatedVars &parent, bool use_id);
    static olx_object_ptr<Object> FromDataItem(const TDataItem &i,
      CalculatedVars &parent, bool use_id);
    olxstr GetTypeName() const;
    olxstr GetQualifiedName() const {
      return GetTypeName() << '.' << name;
    }
    bool IsValid() const;

    bool SameAtoms(const Object& o, bool order_matters) const;

    static Object *create(class TSAtom &a, CalculatedVars &parent);
    static Object *create(class TSBond &b, CalculatedVars &parent);
    static Object *create(class TSPlane &p, CalculatedVars &parent);
    static Object* Clone(Object& a, CalculatedVars& parent);
  protected:
    static uint16_t DecodeType(const olxstr& v);
  };

  struct Plane : public Object {
    vec3d normal;
    Plane() : Object(cv_ot_plane) {}
    Plane(const Plane &o, CalculatedVars &parent)
      : Object(o, parent), normal(o.normal)
    {}
    TDataItem& ToDataItem(TDataItem &i, bool use_id) const;
    void FromDataItem_(const TDataItem &i, CalculatedVars &parent, bool use_id);
  };

  struct ObjectRef {
    Object &object;
    olxstr prop; // referred property, like c or n for plane
    ObjectRef(const ObjectRef &o) : object(o.object), prop(o.prop) {
      object.IncRef();
    }
    ObjectRef(Object &o) : object(o) {
      o.IncRef();
    }
    ObjectRef(Object &o, const olxstr &p) : object(o), prop(p) {
      o.IncRef();
    }
    ~ObjectRef() {
      object.DecRef();
    }
    olxstr GetQualifiedName() const {
      olxstr rv = object.GetQualifiedName();
      return prop.IsEmpty() ? rv : (rv << '.' << prop);
    }
    olxstr GetName() const;
  };

  struct Var {
    olxstr name;
    TTypeList<ObjectRef> refs;
    uint16_t type;
    // transient field
    mutable TEValueD value;
    Var(uint16_t type, const olxstr &n)
      : type(type), name(n)
    {}
    Var &AddRef(Object& o, const olxstr &prop=EmptyString()) {
      refs.Add(new ObjectRef(o, prop));
      return *this;
    }
    TEValueD Calculate(class VcoVContainer &vcov) const;
    void ToDataItem(TDataItem &i) const;
    bool IsValid() const {
      for (size_t i = 0; i < refs.Count(); i++) {
        if (!refs[i].object.IsValid()) {
          return false;
        }
      }
      return true;
    }
    olxstr GetName() const;
    static Var *Clone(const Var &v, CalculatedVars &parent);
    static olx_pair_t<olxstr, olxstr> parseObject(const olxstr &on);
    static Var *FromDataItem(const TDataItem &i, CalculatedVars &parent);
    bool ToCIF(olx_pdict<uint16_t, cif_dp::cetTable*> out) const;
  protected:
    static uint16_t DecodeType(const olxstr& v);
  };
protected:
  RefinementModel &rm;
  mutable olxstr_dict<Object *> objects;
  mutable olxstr_dict<Var *> vars;
  mutable olx_pdict<uint64_t, smatd*> eqivs;
  smatd * GetEqiv(size_t idx) { return eqivs.GetValue(idx); }
public:
  CalculatedVars(RefinementModel &);
  virtual ~CalculatedVars() { Clear(); }

  void Clear();
  void Assign(const CalculatedVars &cv);
  smatd * GetEqiv(const smatd &m);
  Object &AddObject(Object *o);
  Var &AddVar(Var *v);

  // validates the object and returns true if it is not empty
  bool Validate() const;
  void ToDataItem(TDataItem &i, bool use_id) const;
  void FromDataItem(const TDataItem &i, bool use_id);

  void CalcAll(olx_object_ptr<VcoVContainer> vcovc=0) const;
  TTypeList<cif_dp::cetTable>::const_list_type ToCIF(const class TCif &cif) const;
};

EndXlibNamespace()
#endif
