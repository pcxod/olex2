#ifndef pdbH
#define pdbH

#include "xfiles.h"

BeginXlibNamespace()


class TPdb: public TBasicCFile  {
private:
  void Clear();
protected:
public:
  TPdb(TAtomsInfo *S);
  virtual ~TPdb();

  virtual void SaveToStrings(TStrList& Strings);
  virtual void LoadFromStrings(const TStrList& Strings);
  virtual bool Adopt(TXFile *XF);
  void DeleteAtom(TCAtom *CA);

  virtual IEObject* Replicate()  const {  return new TPdb(AtomsInfo);  }
};

EndXlibNamespace()
#endif
