#ifndef sbondH
#define sbondH

#include "xbase.h"
#include "typelist.h"
#include "tptrlist.h"
#include "dataitem.h"

BeginXlibNamespace()

class TSBond: public TBasicBond<class TNetwork, class TSAtom>  {
private:
//  int FTag;
  virtual void OnAtomSet();
protected:
  bool Deleted;
public:
  TSBond(TNetwork* Parent);
  virtual ~TSBond() { }

  DefPropB(Deleted)

  double Length();

  void ToDataItem(TDataItem& item) const;
  void FromDataItem(const TDataItem& item, TPtrList<TNetwork>& net_pool);
};
  typedef TTypeList<TSBond> TSBondList;
  typedef TPtrList<TSBond> TSBondPList;

EndXlibNamespace()
#endif

