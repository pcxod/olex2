#ifndef __olxconn_info_H
#define __olxconn_info_H

#include "edict.h"
#include "bapp.h"
#include "catom.h"

BeginXlibNamespace()

class RefinementModel;

class ConnInfo  {
public:
  struct bondInfo  {
    TCAtom& to;
    const smatd* matr;
    bondInfo(const bondInfo& bi) : to(bi.to), matr(bi.matr) {}
    bondInfo(TCAtom& ca, const smatd* m=NULL) : to(ca), matr(m) {}
  };
  typedef TTypeList<bondInfo> BondInfoList;
protected:
  struct _connInfo  {
    short maxBonds;
    double r;
    _connInfo() : maxBonds(12), r(-1) {}
    _connInfo(const _connInfo& ci) : maxBonds(ci.maxBonds), r(ci.r) {}
    _connInfo& operator = (const _connInfo& ci)  {
      maxBonds = ci.maxBonds;
      r = ci.r;
      return *this;
    }
  };
  struct AtomConnInfo : public _connInfo  {
    TCAtom* atom;
    BondInfoList BondsToCreate, BondsToRemove;
    AtomConnInfo() : atom(NULL) {}
    AtomConnInfo(TCAtom& ca) : atom(&ca) {}
    AtomConnInfo(const AtomConnInfo& ci) : _connInfo(ci), atom(ci.atom) {}
    AtomConnInfo& operator = (const AtomConnInfo& ci)  {
      _connInfo::operator = (ci);
      atom = ci.atom;
      BondsToCreate = ci.BondsToCreate;
      BondsToRemove = ci.BondsToRemove;
      return *this;
    }
  };
  struct TypeConnInfo : public _connInfo  {
    TBasicAtomInfo* atomInfo;
    TypeConnInfo() : atomInfo(NULL) {}
    TypeConnInfo(TBasicAtomInfo& bai) : atomInfo(&bai) {}
    TypeConnInfo(const TypeConnInfo& ci) : _connInfo(ci), atomInfo(ci.atomInfo) {}
    TypeConnInfo& operator = (const TypeConnInfo& ti)  {
      _connInfo::operator = (ti);
      atomInfo = ti.atomInfo;
      return *this;
    }
  };
  olxdict<TCAtom*, AtomConnInfo, TPointerPtrComparator> AtomInfo;
  olxdict<TBasicAtomInfo*, TypeConnInfo, TPointerPtrComparator> TypeInfo;
public:
  RefinementModel& RM;
  
  ConnInfo(RefinementModel& rm) : RM(rm) {}

  struct connInfo : public _connInfo{
    BondInfoList BondsToCreate, 
                 BondsToRemove;
  };

  // prepares a list of extra connectivity info for each atom of the AUnit
  void Compile(TTypeList<ConnInfo::connInfo>& res) const;
  
  void ProcessConn(TStrList& ins);

  void AddBond(TCAtom& a1, TCAtom& a2, const smatd* eqiv)  {
    AtomConnInfo& ai = AtomInfo.Add(&a1, AtomConnInfo(a1));
    bool found = false;
    for( int i=0; i < ai.BondsToCreate.Count(); i++ )  {
      if( ai.BondsToCreate[i].to == a2 )  {
        if( ai.BondsToCreate[i].matr && eqiv == NULL )  {
          found = true;
          break;
        }
        if( ai.BondsToCreate[i].matr != NULL && eqiv != NULL &&
            *ai.BondsToCreate[i].matr == *eqiv )  
        {
          found = true;
          break;
        }
      }
    }
    if( found )
      return;
    // need to check if the same bond is not in the BondsToRemove List
    ai.BondsToCreate.Add( new bondInfo(a2, eqiv) );
  }
  void RemBond(TCAtom& a1, TCAtom& a2, const smatd* eqiv)  {
    AtomConnInfo& ai = AtomInfo.Add(&a1, AtomConnInfo(a1));
    bool found = false;
    for( int i=0; i < ai.BondsToRemove.Count(); i++ )  {
      if( ai.BondsToCreate[i].to == a2 )  {
        if( ai.BondsToRemove[i].matr && eqiv == NULL )  {
          found = true;
          break;
        }
        if( ai.BondsToRemove[i].matr != NULL && eqiv != NULL &&
            *ai.BondsToRemove[i].matr == *eqiv )  
        {
          found = true;
          break;
        }
      }
    }
    if( found )
      return;
    // need to check if the same bond is not in the BondsToCreate List
    ai.BondsToRemove.Add( new bondInfo(a2, eqiv) );
  }
  //.................................................................
  void ProcessFree(const TStrList& ins);
  void ProcessBind(const TStrList& ins);
  
  void Clear()  {
    AtomInfo.Clear();
    TypeInfo.Clear();
  }

  void ToInsList(TStrList& ins) const;

  void Assign(const ConnInfo& ci);

};

EndXlibNamespace()
#endif