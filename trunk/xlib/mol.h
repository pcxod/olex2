#ifndef molH
#define molH

#include "xfiles.h"

BeginXlibNamespace()


struct TMolBond  {
  int AtomA, AtomB, BondType;
};

class TMol: public TBasicCFile  {
private:
  void Clear();
  TTypeList<TMolBond> Bonds;
protected:
  olxstr MOLAtom(TCAtom& CA);
  olxstr MOLBond(TMolBond& B);
public:
  TMol(TAtomsInfo *S);
  virtual ~TMol();

  inline int BondCount() const     {  return Bonds.Count();  }
  inline TMolBond& Bond(int index) {  return Bonds[index];  }
  virtual void SaveToStrings(TStrList& Strings);
  virtual void LoadFromStrings(const TStrList& Strings);
  virtual bool Adopt(TXFile *XF);
  void DeleteAtom(TCAtom *CA);

  virtual IEObject* Replicate()  const {  return new TMol(AtomsInfo);  }
};

EndXlibNamespace()
#endif
