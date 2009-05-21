#ifndef olx_mol_2_H
#define olx_mol_2_H

#include "xfiles.h"

BeginXlibNamespace()

const short 
  mol2btSingle       = 1,
  mol2btDouble       = 2,
  mol2btTriple       = 3,
  mol2btAmide        = 4,
  mol2btAromatic     = 5,
  mol2btDummy        = 6,
  mol2btUnknown      = 7,
  mol2btNotConnected = 8;

struct TMol2Bond  {
private:
  int Id;
public:
  TMol2Bond(int id) : Id(id) { }
  short BondType;
  TCAtom* a1, *a2;
  int GetId() const {  return Id;  }
};

class TMol2: public TBasicCFile  {
private:
  void Clear();
  TTypeList<TMol2Bond> Bonds;
  static const olxstr BondNames[];
protected:
  olxstr MOLAtom(TCAtom& CA);
  olxstr MOLBond(TMol2Bond& B);
  const olxstr& EncodeBondType(short type) const;
  short DecodeBondType(const olxstr& name) const;
public:
  TMol2() { }
  virtual ~TMol2() {  Clear();  }

  inline int BondCount() const     {  return Bonds.Count();  }
  inline TMol2Bond& Bond(int index) {  return Bonds[index];  }
  virtual void SaveToStrings(TStrList& Strings);
  virtual void LoadFromStrings(const TStrList& Strings);
  virtual bool Adopt(TXFile *XF);
  void DeleteAtom(TCAtom *CA);

  virtual IEObject* Replicate()  const {  return new TMol2;  }
};

EndXlibNamespace()
#endif
