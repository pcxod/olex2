#ifndef __XD_MAS_FILE
#define __XD_MAS_FILE

#include "xbase.h"
#include "estrlist.h"

#include "xfiles.h"
#include "catom.h"

#include "estlist.h"

#include "bapp.h"
#include "log.h"

BeginXlibNamespace()

class TXDMas: public TBasicCFile  {
  double Radiation;
public:
  TXDMas() : Radiation(0.71) {  }
  virtual void SaveToStrings(TStrList& Strings)  {  throw TNotImplementedException(__OlxSourceInfo);  }
  virtual void LoadFromStrings(const TStrList& Strings);
  virtual bool Adopt(TXFile *XF)  {  throw TNotImplementedException(__OlxSourceInfo);  }
  virtual void DeleteAtom(TCAtom *CA)  {  return;  }
  virtual IEObject* Replicate()  const {  return new TXDMas;  }
  DefPropP(double, Radiation)
};

EndXlibNamespace()
#endif
