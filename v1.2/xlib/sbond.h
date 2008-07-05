#ifndef sbondH
#define sbondH

#include "xbase.h"
#include "typelist.h"
#include "tptrlist.h"

BeginXlibNamespace()

class TSBond: public TBasicBond  {
private:
//  int FTag;
  virtual void OnAtomSet();
protected:
  bool Deleted;
public:
  TSBond(TNetwork* Parent);
  virtual void Create();
  virtual ~TSBond();

  DefPropB(Deleted)

  double Length();
};
  typedef TTypeList<TSBond> TSBondList;
  typedef TPtrList<TSBond> TSBondPList;

EndXlibNamespace()
#endif

