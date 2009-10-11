#ifndef sbondH
#define sbondH

#include "satom.h"
#include "typelist.h"
#include "tptrlist.h"
#include "dataitem.h"

BeginXlibNamespace()

class TSBond: public TBasicBond<class TNetwork, TSAtom>  {
private:
//  int FTag;
  virtual void OnAtomSet();
protected:
  bool Deleted;
public:
  TSBond(TNetwork* Parent);
  virtual ~TSBond() { }

  DefPropBIsSet(Deleted)

  double Length()  const {  return FA->crd().DistanceTo(FB->crd()); }
  double QLength() const {  return FA->crd().QDistanceTo(FB->crd()); }

  void ToDataItem(TDataItem& item) const;
  void FromDataItem(const TDataItem& item, TPtrList<TNetwork>& net_pool);
};
  typedef TTypeList<TSBond> TSBondList;
  typedef TPtrList<TSBond> TSBondPList;

EndXlibNamespace()
#endif

