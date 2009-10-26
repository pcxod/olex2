#ifndef molH
#define molH

#include "xfiles.h"

BeginXlibNamespace()


struct TMolBond  {
  size_t AtomA, AtomB, BondType;
};

class TMol: public TBasicCFile  {
private:
  void Clear();
  TTypeList<TMolBond> Bonds;
protected:
  olxstr MOLAtom(TCAtom& CA);
  olxstr MOLBond(TMolBond& B);
public:
  TMol();
  virtual ~TMol();

  inline size_t BondCount() const {  return Bonds.Count();  }
  inline TMolBond& Bond(size_t index) {  return Bonds[index];  }
  virtual void SaveToStrings(TStrList& Strings);
  virtual void LoadFromStrings(const TStrList& Strings);
  virtual bool Adopt(TXFile *XF);
  void DeleteAtom(TCAtom *CA);

  virtual IEObject* Replicate()  const {  return new TMol;  }
};

EndXlibNamespace()
#endif
