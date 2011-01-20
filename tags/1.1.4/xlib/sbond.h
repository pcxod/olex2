#ifndef __olx_xl_sbond_H
#define __olx_xl_sbond_H
#include "satom.h"
#include "typelist.h"
#include "tptrlist.h"
#include "dataitem.h"

BeginXlibNamespace()

class TSBond: public TBasicBond<class TNetwork, TSAtom>  {
private:
  virtual void OnAtomSet();
protected:
  bool Deleted;
public:
  TSBond(TNetwork* Parent);
  virtual ~TSBond() { }

  DefPropBIsSet(Deleted)
  bool IsAvailable() const {  return (!IsDeleted() && FA->IsAvailable() && FB->IsAvailable()); }
  vec3d GetCenter() const {  return (FA->crd()+FB->crd())/2;  }
  double Length() const {  return FA->crd().DistanceTo(FB->crd());  }
  double QLength() const {  return FA->crd().QDistanceTo(FB->crd());  }

  struct Ref  {
    TSAtom::Ref a, b;
    Ref(const TSAtom::Ref& _a, const TSAtom::Ref& _b) : a(_a), b(_b)  {}
    Ref(const Ref& r) : a(r.a), b(r.b)  {}
    Ref& operator = (const Ref& r)  {
      a = r.a;
      b = r.b;
      return *this;
    }
    bool operator == (const TSBond::Ref& r) const {
      return (a == r.a && b == r.b) || (a == r.b && b == r.a); 
    }
    int Compare(const Ref& r) const {
      const int rv = a.Compare(r.a);
      return rv == 0 ? b.Compare(r.b) : rv;
    }
  };

  Ref GetRef() const {  return Ref(FA->GetRef(), FB->GetRef());  }
  // despite the fact that atoms are alsways sorted, compare both end...
  bool operator == (const Ref& r) const {
    return (*FA == r.a && *FB == r.b) || (*FA == r.b && *FB == r.a);
  }
  void ToDataItem(TDataItem& item) const;
  void FromDataItem(const TDataItem& item, class TLattice& parent);
};

typedef TTypeList<TSBond> TSBondList;
typedef TPtrList<TSBond> TSBondPList;

EndXlibNamespace()
#endif

