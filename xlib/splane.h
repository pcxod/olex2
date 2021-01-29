/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xl_splane_H
#define __olx_xl_splane_H
#include "xbase.h"
#include "typelist.h"
#include "satom.h"
#include "math/plane.h"

BeginXlibNamespace()

class TAtomEnvi;

const uint16_t
  plane_best = 1,
  plane_worst = 2;
const uint16_t
  plane_flag_deleted = 0x0001;

class TSPlane : public TSObject<TNetwork> {
  TTypeList<olx_pair_t<TSAtom*, double> > Crds;
  vec3d Center;
  mat3d Normals;
  double Distance, wRMSD;  uint16_t Flags;
  size_t DefId;
  void _Init(const TTypeList<olx_pair_t<vec3d, double> >& points);
  // for Crd sorting by atom tag
  struct AtomTagAccessor {
    index_t operator() (
      const olx_pair_t<TSAtom*, double>* p) const
    {
      return p->GetA()->GetTag();
    }
  };
public:
  TSPlane() : TSObject<TNetwork>(0), DefId(InvalidIndex)
  {}
  TSPlane(TNetwork* Parent, size_t def_id = InvalidIndex)
    : TSObject<TNetwork>(Parent), Distance(0), wRMSD(0), Flags(0),
    DefId(def_id)
  {}
  virtual ~TSPlane() {}

  DefPropBFIsSet(Deleted, Flags, plane_flag_deleted);

  // an association point, weight is provided
  void Init(const TTypeList<olx_pair_t<TSAtom*, double> >& Points);

  inline const vec3d& GetNormal() const { return Normals[0]; }
  // note that the Normal (or Z) is the first row of the matrix
  inline const mat3d& GetBasis() const { return Normals; }
  inline const vec3d& GetCenter() const { return Center; }

  double DistanceTo(const vec3d& Crd) const {
    return Crd.DotProd(GetNormal()) - Distance;
  }
  double DistanceTo(const TSAtom& a) const { return DistanceTo(a.crd()); }
  double Angle(const vec3d& A, const vec3d& B) const {
    return acos(GetNormal().CAngle(B - A)) * 180 / M_PI;
  }
  double Angle(const vec3d& v) const {
    return acos(GetNormal().CAngle(v)) * 180 / M_PI;
  }
  double Angle(const class TSBond& B) const;
  double Angle(const TSPlane& P) const { return Angle(P.GetNormal()); }
  double GetD() const { return Distance; }
  // direct calculation with unit weights
  double CalcRMSD() const;
  // inverts the plane normal
  void Invert();
  // calculation with actual weights (should be the same as GetWeightedRMSD)
  double CalcWeightedRMSD() const;
  double GetWeightedRMSD() const { return wRMSD; }
  double GetZ(const double& X, const double& Y) const {
    return (GetNormal()[2] == 0) ? 0.0
      : (GetNormal()[0] * X + GetNormal()[1] * Y + Distance) / GetNormal()[2];
  }
  size_t Count() const { return Crds.Count(); }
  const TSAtom& GetAtom(size_t i) const { return *Crds[i].GetA(); }
  TSAtom& GetAtom(size_t i) { return *Crds[i].a; }
  double GetWeight(size_t i) const { return Crds[i].GetB(); }
  void _PlaneSortByAtomTags() {
    InsertSorter::Sort(Crds,
      ComplexComparator::Make(AtomTagAccessor(), TPrimitiveComparator()),
      DummySortListener());
  }
  /* returns inverse intersects with the lattice vectors, the vector is divided
  by modulus of the smallest non-zero value. Takes the orthogonalisation matrix
  plane normal and a point on the plane
  */
  static vec3d GetCrystallographicDirection(const mat3d& m,
    const vec3d& n, const vec3d& p);
  vec3d GetCrystallographicDirection() const;
  // returns sqrt(smallest eigen value/point.Count())
  static double CalcRMSD(const TSAtomPList& atoms);
  static double CalcRMSD(const TAtomEnvi& atoms);
  olxstr StrRepr() const;

  class Def : public ACollectionItem {
    struct DefData {
      TSAtom::Ref ref;
      double weight;
      // need one for TypeList allocation...
      DefData() {}
      DefData(const TSAtom::Ref& r, double w)
        : ref(r), weight(w)
      {}
      DefData(const DefData& r)
        : ref(r.ref), weight(r.weight)
      {}
      DefData(const TDataItem& item, const class TXApp& app)
        : ref(item.GetItemByIndex(0), app)
      {
        weight = item.GetFieldByName("weight").ToDouble();
      }
      DefData& operator = (const DefData& r) {
        ref = r.ref;
        weight = r.weight;
        return *this;
      }
      int Compare(const DefData& d) const {
        return ref.Compare(d.ref);
      }
      void ToDataItem(TDataItem& item, const TXApp& app, bool use_id=false) const;
      void FromDataItem(const TDataItem& item, const TXApp &app);
    };
    TTypeList<DefData> atoms;
    size_t sides;
  public:
    Def(const TSPlane& plane);
    Def(const Def& r) : atoms(r.atoms), sides(r.sides)
    {}
    Def(const TDataItem& item, const class TXApp& app) {
      FromDataItem(item, app);
    }
    Def& operator = (const Def& r) {
      atoms = r.atoms;
      sides = r.sides;
      return *this;
    }
    bool operator == (const Def& d)  const {
      return Compare(d) == 0;
    }
    int Compare(const Def& d) const;
    TSPlane* FromAtomRegistry(const TXApp& app, struct ASObjectProvider& ar, size_t def_id,
      class TNetwork* parent, const smatd& matr) const;
    void ToDataItem(TDataItem& item, class TXApp& app, bool use_id=false) const;
    void FromDataItem(const TDataItem& item, const TXApp& app);
    size_t GetSides() const { return sides; }
    void SetSides(size_t s) { sides = s; }
    TSPlane::Def& Sort() {
      QuickSorter::Sort(atoms);
      return *this;
    }
  };

  Def GetDef() const { return Def(*this); }
  size_t GetDefId() const { return DefId; }
  // not for external use
  void _SetDefId(size_t id) { DefId = id; }

  virtual void ToDataItem(TDataItem& item, const class TXApp& app) const;
  virtual void FromDataItem(const TDataItem& item, const TXApp& app);
};

typedef TTypeList<TSPlane> TSPlaneList;
typedef TPtrList<TSPlane> TSPlanePList;

EndXlibNamespace()
#endif
