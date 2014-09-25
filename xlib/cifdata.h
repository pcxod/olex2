/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xl_xifdata_H
#define __olx_xl_xifdata_H
#include "xbase.h"
#include "evalue.h"
#include "satom.h"

BeginXlibNamespace()
class ACifValue : public IEObject  {
  TEValueD Value;
public:
  ACifValue(const TEValueD& v) : Value(v)  {}
  const TEValueD& GetValue() const {  return Value;  }
  virtual bool Match(const TSAtomCPList& atoms) const = 0;
  virtual size_t Count() const = 0;
};

class CifBond : public ACifValue {
  const TCAtom &base, &to;
  smatd mat;
public:
  CifBond(const TCAtom& _base, const TCAtom& _to, const smatd& _m, const TEValueD& _d) :
    ACifValue(_d),
    base(_base),
    to(_to),
    mat(_m)
  {
    mat.SetId(1);
  }
  CifBond(const TCAtom& _base, const TCAtom& _to, const TEValueD& _d) :
    ACifValue(_d),
    base(_base),
    to(_to)
  {
    mat.I();
    mat.SetId(0);
  }
  bool DoesMatch(const TSAtom& a, const TSAtom& b) const;
  virtual size_t Count() const {  return 2;  }
  virtual bool Match(const TSAtomCPList& atoms) const {
    if( atoms.Count() != 2 )  return false;
    return DoesMatch(*atoms[0], *atoms[1]) || DoesMatch(*atoms[1], *atoms[0]);
  }
};

class CifAngle : public ACifValue {
  const TCAtom &left, &middle, &right;
  smatd mat_l, mat_r;
public:
  CifAngle(const TCAtom& _l, const TCAtom& _m, const TCAtom& _r,
    const smatd& _lm, const smatd& _rm, const TEValueD& _d) :
    ACifValue(_d),
    left(_l),
    middle(_m),
    right(_r),
    mat_l(_lm),
    mat_r(_rm)
  {
    mat_l.SetId(1);
    mat_r.SetId(1);
  }
  CifAngle(const TCAtom& _l, const TCAtom& _m, const TCAtom& _r, const TEValueD& _d) :
    ACifValue(_d),
    left(_l),
    middle(_m),
    right(_r)
  {
    mat_l.I();
    mat_l.SetId(0);
    mat_r.I();
    mat_r.SetId(0);
  }
  bool DoesMatch(const TSAtom& l, const TSAtom& m, const TSAtom& r) const;
  virtual size_t Count() const {  return 3;  }
  virtual bool Match(const TSAtomCPList& atoms) const {
    if( atoms.Count() != 3 )  return false;
    return DoesMatch(*atoms[0], *atoms[1], *atoms[2]) ||
           DoesMatch(*atoms[2], *atoms[1], *atoms[0]);
  }
};
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
class TCifDataManager  {
  olx_pdict<size_t, TPtrList<ACifValue> > Data;
public:
  TCifDataManager()  {}
  virtual ~TCifDataManager()  {  Clear();  }
  ACifValue& AddValue(ACifValue* v)  {  return *Data.Add(v->Count()).Add(v);  }
  ACifValue* Match(const TSAtom& a, const TSAtom& b) const {
    const TSAtom* a_[] = {&a, &b};
    return Match(TSAtomCPList(2, a_));
  }
  ACifValue* Match(const TSAtom& a, const TSAtom& b, const TSAtom& c) const {
    const TSAtom* a_[] = {&a, &b, &c};
    return Match(TSAtomCPList(3, a_));
  }
  ACifValue* Match(const TSAtom& a, const TSAtom& b, const TSAtom& c, const TSAtom& d) const {
    const TSAtom* a_[] = {&a, &b, &c, &d};
    return Match(TSAtomCPList(4, a_));
  }
  // finds a cif value for a list of TSATOMS(!)
  ACifValue* Match(const TSAtomCPList& Atoms) const {
    const size_t ci = Data.IndexOf(Atoms.Count());
    if( ci == InvalidIndex )  return NULL;
    const TPtrList<ACifValue>& items = Data.GetValue(ci);
    for( size_t i=0; i < items.Count(); i++ )  {
      if( items[i]->Match(Atoms) )
        return items[i];
    }
    return NULL;
  }
  const TPtrList<ACifValue>* FindValues(size_t number_of_atoms) const {
    const size_t ci = Data.IndexOf(number_of_atoms);
    if( ci == InvalidIndex )  return NULL;
    return &Data.GetValue(ci);
  }
  void Clear()  {
    for( size_t i=0; i < Data.Count(); i++ )  {
      TPtrList<ACifValue>& items = Data.GetValue(i);
      for( size_t j=0; j < items.Count(); j++ )
        delete items[j];
    }
    Data.Clear();
  }
};
EndXlibNamespace()
#endif
