/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xlib_atom_registry_H
#define __olx_xlib_atom_registry_H
#include "sbond.h"
#include "arrays.h"

BeginXlibNamespace()

template <class, class> class ObjectCaster;
/* Object provide interface
*/
template <class obj_t> class TIObjectProvider {
public:
  virtual ~TIObjectProvider() {}
  virtual size_t Count() const = 0;
  virtual obj_t& New(TNetwork* n) = 0;
  virtual obj_t& Get(size_t i) const = 0;
  inline obj_t& operator [] (size_t i) const { return Get(i); }
  obj_t& GetLast() const { return Get(Count() - 1); }
  virtual void Delete(size_t i) = 0;
  inline void DeleteLast() { Delete(Count() - 1); }
  virtual void Clear() = 0;
  virtual void Null(size_t i) = 0;
  virtual obj_t& Detach(size_t i) = 0;
  virtual obj_t& Attach(obj_t& o) = 0;
  virtual void Pack() = 0;
  virtual void IncCapacity(size_t v) = 0;
  inline bool IsEmpty() const { return Count() == 0; }
  template <class Functor>
  const TIObjectProvider& ForEach(const Functor& f) const {
    for (size_t i = 0; i < Count(); i++) {
      f.OnItem(Get(i), i);
    }
    return *this;
  }
  template <class act_t> ObjectCaster<obj_t, act_t> GetAccessor() {
    return ObjectCaster<obj_t, act_t>(*this);
  }
};
/* Object caster - object taking a list of objects and providing interface
to access it as a list of another, related object type
*/
template <class obj_t, class act_t> class ObjectCaster
  : public TIObjectProvider<act_t>
{
  TIObjectProvider<obj_t>& list;
protected:
  // dummy function...
  virtual act_t& New(TNetwork*) {
    throw TNotImplementedException(__OlxSourceInfo);
  }
public:
  ObjectCaster(TIObjectProvider<obj_t>& _list) : list(_list) {}
  virtual size_t Count() const { return list.Count(); }
  virtual act_t& Get(size_t i) const { return (act_t&)list.Get(i); }
  inline act_t& operator [] (size_t i) const { return Get(i); }
  virtual void Delete(size_t i) { list.Delete(i); }
  virtual void Clear() { list.Clear(); }
  virtual void Null(size_t i) { list.Null(i); }
  virtual act_t& Detach(size_t i) { return (act_t&)list.Detach(i); }
  virtual act_t& Attach(act_t& o) { return (act_t&)list.Attach(o); }
  virtual void Pack() { list.Pack(); }
  virtual void IncCapacity(size_t v) { list.IncCapacity(v); }
  template <class Functor>
  const ObjectCaster& ForEach(const Functor& f) const {
    for (size_t i = 0; i < Count(); i++) {
      f.OnItem(Get(i), i);
    }
    return *this;
  }
};
/* Implementation of the object provide based on the pointer list
*/
template <class obj_t> class TObjectProvider
  : public TIObjectProvider<obj_t>
{
protected:
  TPtrList<obj_t> items;
  void UpdateOwnerIds() {
    for (size_t i = 0; i < Count(); i++) {
      Get(i).SetOwnerId(i);
    }
  }
  inline obj_t& AddNew(obj_t* o) {
    o->SetOwnerId(Count());
    return *items.Add(o);
  }
public:
  virtual size_t Count() const { return items.Count(); }
  virtual obj_t& New(TNetwork* n) { return AddNew(new obj_t(n)); }
  virtual obj_t& Get(size_t i) const { return *items[i]; }
  inline obj_t& operator [] (size_t i) const { return Get(i); }
  obj_t& GetLast() const { return *items.GetLast(); }
  virtual void Delete(size_t i) {
    delete items[i];
    items.Delete(i);
    UpdateOwnerIds();
  }
  inline void DeleteLast() { Delete(Count() - 1); }
  virtual void Clear() { items.DeleteItems(false).Clear(); }
  virtual void Null(size_t i) {
    delete items[i];
    items.Set(i, 0);
  }
  virtual obj_t& Detach(size_t i) {
    obj_t* rv = items[i];
    items.Delete(i);
    return *rv;
  }
  virtual obj_t& Attach(obj_t& o) { return *items.Add(o); }
  virtual void Pack() {
    items.Pack();
    UpdateOwnerIds();
  }
  virtual void IncCapacity(size_t v) { items.SetCapacity(items.Count() + v); }
  inline bool IsEmpty() const { return items.IsEmpty(); }
  template <class Functor>
  const TObjectProvider& ForEach(const Functor& f) const {
    items.ForEach(f);
    return *this;
  }
};
/* TSAtom registry - an object for quick locating of atoms using their index in
the asymmetric unit*/
class AtomRegistry {
  struct DataStruct {
    olx_array::TArray3D<TArrayList<TSAtomPList*>*> registry;
    mutable int ref_cnt;
    DataStruct(const vec3i& mind, const vec3i& maxd)
      : registry(mind, maxd), ref_cnt(1) {}
    ~DataStruct() {
      for (size_t i = 0; i < registry.Length1(); i++) {
        for (size_t j = 0; j < registry.Length2(); j++) {
          for (size_t k = 0; k < registry.Length3(); k++) {
            TArrayList<TSAtomPList*>* aum_slice = registry.Data[i][j][k];
            if (aum_slice == 0) {
              continue;
            }
            for (size_t l = 0; l < aum_slice->Count(); l++) {
              if ((*aum_slice)[l] != 0) {
                delete (*aum_slice)[l];
              }
            }
            delete aum_slice;
          }
        }
      }
    }
  };
protected:
  DataStruct* data;
public:
  typedef olx_array::TArray3D<TArrayList<TSAtomPList*>*> RegistryType;
  //...........................................................................
  AtomRegistry() : data(0) {}
  //...........................................................................
  AtomRegistry(const AtomRegistry& r) : data(r.data) { data->ref_cnt++; }
  //...........................................................................
  RegistryType& Init(const vec3i& mind, const vec3i& maxd) {
    if (data != 0 && --data->ref_cnt == 0) {
      delete data;
    }
    data = new DataStruct(mind, maxd);
    return data->registry;
  }
  //...........................................................................
  void Clear() {
    if (data != 0 && --data->ref_cnt == 0) {
      delete data;
    }
    data = 0;
  }
  //...........................................................................
  ~AtomRegistry() {
    if (data != 0 && --data->ref_cnt == 0) {
      delete data;
    }
  }
  //...........................................................................
  AtomRegistry& operator = (const AtomRegistry& ar) {
    if (data != 0 && --data->ref_cnt == 0) {
      delete data;
    }
    data = ar.data;
    return *this;
  }
  //...........................................................................
  RegistryType& GetRegistry() { return data->registry; }
  //...........................................................................
  ConstPtrList<TSAtom> FindAll(const TCAtom &a) const {
    TSAtomPList rv;
    for (size_t i = 0; i < data->registry.Length1(); i++) {
      for (size_t j = 0; j < data->registry.Length2(); j++) {
        for (size_t k = 0; k < data->registry.Length3(); k++) {
          TArrayList<TSAtomPList*>* aum_slice = data->registry.Data[i][j][k];
          if (aum_slice == 0) {
            continue;
          }
          for (size_t l = 0; l < aum_slice->Count(); l++) {
            TSAtomPList *au_slice = (*aum_slice)[l];
            if (au_slice == 0) {
              continue;
            }
            if (a.GetId() >= au_slice->Count()) {
              throw TFunctionFailedException(__OlxSourceInfo, "assert");
            }
            TSAtom *sa = (*au_slice)[a.GetId()];
            if (sa != 0 && !sa->IsDeleted()) {
              rv.Add(sa);
            }
          }
        }
      }
    }
    return rv;
  }
  //...........................................................................
  TSAtom* Find(const TSAtom::Ref &r) const {
    return Find(r.catom->GetId(), r.matrix_id);
  }
  //...........................................................................
  TSAtom* Find(size_t catom_id, uint32_t matrix_id) const {
    if (data == 0) {
      return 0;
    }
    const vec3i t = smatd::GetT(matrix_id);
    if (!data->registry.IsInRange(t)) {
      return 0;
    }
    TArrayList<TSAtomPList*>* aum_slice = data->registry.Value(t);
    if (aum_slice == 0) {
      return 0;
    }
    TSAtomPList* au_slice = (*aum_slice)[smatd::GetContainerId(matrix_id)];
    // the latter condition - if AU is the same but some objects use Q-peaks and
    // their number differs
    if (au_slice == 0 || au_slice->Count() <= catom_id) {
      return 0;
    }
    return (*au_slice)[catom_id];
  }
  //...........................................................................
  TSBond* Find(const TSBond::Ref& ref) const {
    TSAtom* a = Find(ref.a);
    if (a == 0) {
      return 0;
    }
    for (size_t i = 0; i < a->BondCount(); i++) {
      if (!a->Bond(i).IsDeleted() && a->Bond(i).Another(*a) == ref.b) {
        return &a->Bond(i);
      }
    }
    return 0;
  }
};

struct ASObjectProvider : public IOlxObject {
  AtomRegistry atomRegistry;
  TIObjectProvider<TSAtom>& atoms;
  TIObjectProvider<TSBond>& bonds;
  TIObjectProvider<class TSPlane>& planes;
  ASObjectProvider(
    TIObjectProvider<TSAtom>& _as,
    TIObjectProvider<TSBond>& _bs,
    TIObjectProvider<TSPlane>& _ps)
    : atoms(_as), bonds(_bs), planes(_ps)
  {}
  virtual class TXFile* CreateXFile() = 0;
  virtual IOlxObject* Replicate() const = 0;
};

struct SObjectProvider : public ASObjectProvider {
  SObjectProvider() : ASObjectProvider(*(new TObjectProvider<TSAtom>),
    *(new TObjectProvider<TSBond>), *(new TObjectProvider<TSPlane>))
  {}
  ~SObjectProvider() {
    atoms.Clear();
    bonds.Clear();
    planes.Clear();
    delete &atoms;
    delete &bonds;
    delete &planes;
  }
  virtual TXFile *CreateXFile();
  virtual IOlxObject* Replicate() const { return new SObjectProvider(); }
};

EndXlibNamespace()
#endif
