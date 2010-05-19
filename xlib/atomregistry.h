#ifndef __olx_xlib_atom_registry_H
#define __olx_xlib_atom_registry_H
#include "sbond.h"
#include "arrays.h"

BeginXlibNamespace()

class AtomRegistry  {
  struct DataStruct  {
    TArray3D<TArrayList<TSAtomPList*>*> registry;
    int ref_cnt;
    DataStruct(const vec3i& mind, const vec3i& maxd) : registry(mind, maxd), ref_cnt(1) {} 
  };
protected:
  mutable DataStruct& data;
  AtomRegistry(const vec3i& mind, const vec3i& maxd) : data(*(new DataStruct(mind, maxd))) {}
public:
  AtomRegistry(const AtomRegistry& r) : data(r.data) {
    data.ref_cnt++;
  }
  ~AtomRegistry()  {
    if( --data.ref_cnt  == 0 )  {
      for( size_t i=0; i < data.registry.Length1(); i++ )  {
        for( size_t j=0; j < data.registry.Length2(); j++ )  {
          for( size_t k=0; k < data.registry.Length3(); k++ )  {
            TArrayList<TSAtomPList*>* aum_slice = data.registry.Data[i][j][k];
            if( aum_slice == NULL )  continue;
            for( size_t l=0; l < aum_slice->Count(); l++ )
              if( (*aum_slice)[l] != NULL )
                delete (*aum_slice)[l];
            delete aum_slice;
          }
        }
      }
      delete &data;
    }
  }
  TArray3D<TArrayList<TSAtomPList*>*>& GetRegistry()  {  return data.registry;  }
  TSAtom* Find(const TSAtom::Ref& ref) const {
    const vec3i t = smatd::GetT(ref.matrix_id);
    if( !data.registry.IsInRange(t) ) return false;
    TArrayList<TSAtomPList*>* aum_slice = data.registry.Value(t);
    if( aum_slice == NULL )  return NULL;
    TSAtomPList* au_slice = (*aum_slice)[smatd::GetContainerId(ref.matrix_id)];
    if( au_slice == NULL ) return false;
    return (*au_slice)[ref.catom_id];
  }
  TSBond* Find(const TSBond::Ref& ref) const {
    TSAtom* a = Find(ref.a);
    if( a == NULL )  return NULL;
    for( size_t i=0; i < a->BondCount(); i++ )
      if( a->Bond(i).Another(*a) == ref.b )
        return &a->Bond(i);
    return NULL;
  }
  friend class TLattice;
};
EndXlibNamespace()
#endif