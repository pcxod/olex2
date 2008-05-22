#ifndef xlcongenH
#define xlcongenH

#include "congen.h"
#include "ins.h"

BeginXlibNamespace()
class TXlConGen : public AConstraintGenerator {
  TIns* InsFile;
public:
  TXlConGen(TIns* ins );
  virtual bool FixParam(const short paramMask, TStrList& res, const TCAtomPList& atoms, const TFixedValueList& values);
  virtual bool FixAtom( TAtomEnvi& envi, const short Group, const TBasicAtomInfo& atomType, TAtomEnvi* pivoting = NULL);
};

EndXlibNamespace()
#endif
