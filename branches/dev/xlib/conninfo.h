#ifndef __olxconn_info_H
#define __olxconn_info_H

#include "asymmunit.h"
#include "edict.h"
#include "bapp.h"

BeginXlibNamespace()

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
  }
  struct AtomConnInfo : public _connInfo  {
    TCAtom& atom;
    BondInfoList BondsToCreate, BondsToRemove;
    AtomConnInfo(TCAtom& ca) : atom(ca) {}
    AtomConnInfo(const AtomConnInfo& ci) : _connInfo(*this), atom(ci.atom) {}
  };
  struct TypeConnInfo : public _connInfo  {
    TBasicAtomInfo& atomInfo;
    TypeConnInfo(TBasicAtomInfo& bai) : atomInfo(bai) {}
    TypeConnInfo(const TypeConnInfo& ci) : _connInfo(*this), atomInfo(ci.atomInfo) {}
  };
  olxdict<TCAtom*, AtomConnInfo, TPointerPtrComparator> AtomInfo;
  olxdict<TBasicAtomInfo*, TypeConnInfo, TPointerPtrComparator> TypeInfo;
public:
  struct connInfo : public _connInfo{
    BondInfoList BondsToCreate, 
                 BondsToRemove;
    connInfo() : maxBonds(12), r(-1) {}
  };
  void Compile(const TAsymmUnit& au, TTypeList<ConnInfo::connInfo>& res)  {
    for( int i=0; i < au.AtomCount(); i++ )  {
      TCAtom& ca = au.GetAtom(i);
      int ai_ind = AtomInfo.IndexOf(&ca), 
          ti_ind;
      if( ai_ind != -1 )  {
        AtomConnInfo& aci = AtomInfo.GetValue(ai_ind);
        connInfo& ci = res.AddNew();  
        ci.r = aci.r < 0 ? ca.GetAtomInfo().GetR1() : aci.r;
        ci.maxBonds = aci.maxBonds;
        ci.BondsToCreate.AddListC(aci.BondsToCreate);
        ci.BondsToRemove.AddListC(aci.BondsToRemove);
      } 
      else if( (ti_ind = TypeInfo.IndexOf(&ca.GetAtomInfo())) != -1 )  {
        TypeConnInfo& aci = TypeInfo.GetValue(ti_ind);
        connInfo& ci = res.AddNew();  
        ci.r = (aci.r < 0 ? ca.GetAtomInfo().GetR1() : aci.r);
        ci.maxBonds = aci.maxBonds;
      }
      else
        res.Add(NULL);
    } 
  }
  template <class list> void ProcessConn(TAsymmUnit& au, list& ins)  {
    short maxB = 12;
    double r = -1;
    TIntList num_indexes;
    for( int i=0; i < ins.Count(); i++ )  {
      if( ins[i].IsNumber() )
        num_indexes.Add(i);
    }
    if( num_indexes.Count() == 2 )  {
      maxB = ins[num_indexes[0]].ToInt();
      r = ins[num_indexes[1]].ToDouble();
    }
    else if( num_indexes.Count() == 1 )  {
      if( ins[num_indexes[0]].IndexOf('.') != -1 )
        r = ins[num_indexes[0]].ToDouble();
      else
        maxB = ins[num_indexes[0]].ToInt();
    }
    else  // invalid argument set, skipping
      return;
    // remove numbers to leave atom names/types only
    for( int i=num_indexes.Count()-1; i >=0; i-- )
      ins.Delete(num_indexes[i]);
    // extract and remove atom types
    for( int i=0; i < ins.Count(); i++ )  {
      if( ins[i].CharAt(0) == '$' )  {
        TBasicAtomInfo* bai = TAtomsInfo::GetInstance().FindAtomInfoBySymbol(ins[i].SubStringFrom(1));
        if( bai == NULL )  {
          TBasicApp::GetLog().Error(olxstr("Undefined atom type in CONN: ") << ins[i].SubStringFrom(1));
          ins.Delete(i--);
          continue;
        }
        TypeConnInfo& ci = TypeInfo.Add(bai, TypeConInfo(*bai) );
        ci.maxBonds = maxB;
        ci.r = r;
        ins.Delete(i--);
      }
    }
    for( int i=0; i < ins.Count(); i++ )  {
      TCAtom* ca = au.FindCAtom(ins[i]);
      if( ca == NULL )  {
        TBasicApp::GetLog().Error(olxstr("Undefined atom name in CONN: ") << ins[i]);
        continue;
      }
      AtomConnInfo& ai = AtomInfo.Add(ca, AtomConnInfo(*ca));
      ai.maxBonds = maxB;
      ai.r = r;
    }
  }
  template <class list> void ProcessFree(TAsymmUnit& au, list& ins)  {
    
  }
};

EndXlibNamespace()
#endif