#ifndef __olx_xl_pdb_H
#define __olx_xl_pdb_H

#include "xfiles.h"

BeginXlibNamespace()

class TPdb: public TBasicCFile  {
private:
  void Clear();
protected:
public:
  TPdb()  {  }
  virtual ~TPdb()  {  Clear();  }
  virtual void SaveToStrings(TStrList& Strings);
  virtual void LoadFromStrings(const TStrList& Strings);
  virtual bool Adopt(TXFile& XF);
  virtual IEObject* Replicate()  const {  return new TPdb;  }
};

EndXlibNamespace()
#endif
