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
    void ToDataItem(TDataItem& item);
    void FromDataItem(TDataItem& item, RefinementModel& rm, TCAtom& atom);
  };
  struct TypeConnInfo : public CXConnInfoBase  {
    TBasicAtomInfo* atomInfo;
    TypeConnInfo() : atomInfo(NULL) {}
    TypeConnInfo(TBasicAtomInfo& bai) : atomInfo(&bai) {}
    TypeConnInfo(const TypeConnInfo& ci) : CXConnInfoBase(ci), atomInfo(ci.atomInfo) {}
    TypeConnInfo& operator = (const TypeConnInfo& ti)  {
      CXConnInfoBase::operator = (ti);
      atomInfo = ti.atomInfo;
      return *this;
    }
    void ToDataItem(TDataItem& item);
    void FromDataItem(TDataItem& item, TBasicAtomInfo* bai);
  };
  olxdict<TCAtom*, AtomConnInfo, TPointerPtrComparator> AtomInfo;
  olxdict<TBasicAtomInfo*, TypeConnInfo, TPointerPtrComparator> TypeInfo;
  // 
  const smatd* GetCorrectMatrix(const smatd* eqiv1, const smatd* eqiv2, bool release) const;
public:
  ConnInfo(RefinementModel& _rm) : rm(_rm) {}

  RefinementModel& rm;

  // prepares a list of extra connectivity info for each atom of the AUnit
  CXConnInfo& GetConnInfo(const TCAtom& ca) const;
  // an object created with new is returned always
  CXConnInfo& GetConnInfo(TBasicAtomInfo& bai) const;

  void ProcessConn(TStrList& ins);
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

  void ToDataItem(TDataItem& item);
  void FromDataItem(TDataItem& item);

};

EndXlibNamespace()
#endif
