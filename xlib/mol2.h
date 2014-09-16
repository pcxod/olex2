/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef olx_mol2_H
#define olx_mol2_H

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
  size_t Id;
public:
  TMol2Bond(size_t id) : Id(id) { }
  size_t BondType;
  index_t a1, a2;
  size_t GetId() const {  return Id;  }
};

class TMol2: public TBasicCFile  {
private:
  void Clear();
  TTypeList<TMol2Bond> Bonds;

  static const TStrList &BondNames() {
    static TStrList rv("1;2;3;am;ar;du;un;nc", ';');
    return rv;
  }
protected:
  olxstr MOLAtom(TCAtom& CA);
  olxstr MOLBond(TMol2Bond& B);
  const olxstr& EncodeBondType(size_t type) const;
  size_t DecodeBondType(const olxstr& name) const;
public:
  TMol2() { }
  virtual ~TMol2() {  Clear();  }

  size_t BondCount() const {  return Bonds.Count();  }
  TMol2Bond& Bond(size_t index) {  return Bonds[index];  }
  virtual void SaveToStrings(TStrList& Strings);
  virtual void LoadFromStrings(const TStrList& Strings);
  virtual bool Adopt(TXFile& XF);
  virtual IEObject* Replicate()  const {  return new TMol2;  }
};

EndXlibNamespace()
#endif
