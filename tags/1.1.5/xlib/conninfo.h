/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olxconn_info_H
#define __olxconn_info_H

#include "edict.h"
#include "bapp.h"
#include "connext.h"
#include "dataitem.h"

BeginXlibNamespace()

class RefinementModel;

class ConnInfo  {
public:
protected:
  struct AtomConnInfo : public CXConnInfoBase  {
    TCAtom* atom;
    BondInfoList BondsToCreate, BondsToRemove;
    AtomConnInfo() : atom(NULL) {}
    AtomConnInfo(TCAtom& ca) : atom(&ca) {}
    AtomConnInfo(const AtomConnInfo& ci) : CXConnInfoBase(ci), atom(ci.atom) {}
    AtomConnInfo& operator = (const AtomConnInfo& ci)  {
      CXConnInfoBase::operator = (ci);
      atom = ci.atom;
      BondsToCreate = ci.BondsToCreate;
      BondsToRemove = ci.BondsToRemove;
      return *this;
    }
    void ToDataItem(TDataItem& item) const;
    void FromDataItem(const TDataItem& item, RefinementModel& rm, TCAtom& atom);
#ifndef _NO_PYTHON
    PyObject* PyExport();
#endif
  };
  struct TypeConnInfo : public CXConnInfoBase  {
    const cm_Element* atomType;
    TypeConnInfo() : atomType(NULL) {}
    TypeConnInfo(const cm_Element& type) : atomType(&type) {}
    TypeConnInfo(const TypeConnInfo& ci) : CXConnInfoBase(ci), atomType(ci.atomType) {}
    TypeConnInfo& operator = (const TypeConnInfo& ti)  {
      CXConnInfoBase::operator = (ti);
      atomType = ti.atomType;
      return *this;
    }
    void ToDataItem(TDataItem& item) const;
    void FromDataItem(const TDataItem& item, const cm_Element* elm);
#ifndef _NO_PYTHON
    PyObject* PyExport();
#endif
  };
  olxdict<TCAtom*, AtomConnInfo, TPointerPtrComparator> AtomInfo;
  olxdict<const cm_Element*, TypeConnInfo, TPointerPtrComparator> TypeInfo;
  // 
  const smatd* GetCorrectMatrix(const smatd* eqiv1, const smatd* eqiv2, bool release) const;
public:
  ConnInfo(RefinementModel& _rm) : rm(_rm) {}

  RefinementModel& rm;

  // prepares a list of extra connectivity info for each atom of the AUnit
  CXConnInfo& GetConnInfo(const TCAtom& ca) const;
  // an object created with new is returned always
  CXConnInfo& GetConnInfo(const cm_Element& elm) const;

  void ProcessConn(TStrList& ins);
  // the atom's connetivity table to have no bonds
  void Disconnect(TCAtom& ca);
  // eqiv corresponds to a2
  static size_t FindBondIndex(const BondInfoList& list, TCAtom* key, TCAtom& a1, TCAtom& a2, const smatd* eqiv);
  void AddBond(TCAtom& a1, TCAtom& a2, const smatd* eqiv1, const smatd* eqiv2, bool release_eqiv);
  void RemBond(TCAtom& a1, TCAtom& a2, const smatd* eqiv1, const smatd* eqiv2, bool release_eqiv);
  /* combines all bonds to create and delete for this atom and imposed by others 
  assumes that all atoms already have connectivity information attached. 
  The newly created matrices are stored in the ml - note then the list is deleted
  the compiled information might become invalid (as some bonds will refer to matrices 
  in the list) */
  static void Compile(const TCAtom& a, BondInfoList& toCreate, BondInfoList& toDelete, smatd_list& ml);
  //.................................................................
  void ProcessFree(const TStrList& ins);
  void ProcessBind(const TStrList& ins);
  
  void Clear()  {
    AtomInfo.Clear();
    TypeInfo.Clear();
  }

  void ToInsList(TStrList& ins) const;

  void Assign(const ConnInfo& ci);

  void ToDataItem(TDataItem& item) const;
  void FromDataItem(const TDataItem& item);
#ifndef _NO_PYTHON
  PyObject* PyExport();
#endif
};

EndXlibNamespace()
#endif