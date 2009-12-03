#ifndef __olx_xl_xyz_H
#define __olx_xl_xyz_H
#include "xfiles.h"

BeginXlibNamespace()

class TXyz: public TBasicCFile  {
private:
  void Clear();
public:
  TXyz();
  virtual ~TXyz();
  virtual void SaveToStrings(TStrList& Strings);
  virtual void LoadFromStrings(const TStrList& Strings);
  virtual bool Adopt(TXFile&);
  virtual IEObject* Replicate()  const {  return new TXyz;  }
};

EndXlibNamespace()
#endif
 
