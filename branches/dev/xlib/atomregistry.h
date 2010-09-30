#ifndef __olx_xlib_atom_registry_H
#define __olx_xlib_atom_registry_H
#include "sbond.h"
#include "arrays.h"

BeginXlibNamespace()

class AtomRegistry  {
  struct DataStruct  {
    TArray3D<TArrayList<TSAtomPList*>*> registry;
    mutable int ref_cnt;
    DataStruct(const vec3i& mind, const vec3i& maxd) : registry(mind, maxd), ref_cnt(1) {} 
    ~DataStruct()  {
      for( size_t i=0; i < registry.Length1(); i++ )  {
        for( size_t j=0; j < registry.Length2(); j++ )  {
          for( size_t k=0; k < registry.Length3(); k++ )  {
            TArrayList<TSAtomPList*>* aum_slice = registry.Data[i][j][k];
            if( aum_slice == NULL )  continue;
            for( size_t l=0; l < aum_slice->Count(); l++ )
              if( (*aum_slice)[l] != NULL )
                delete (*aum_slice)[l];
            delete aum_slice;
          }
        }
      }
    }
  };
protected:
  DataStruct* data;
public:
  typedef TArray3D<TArrayList<TSAtomPList*>*> RegistryType;
  //..................................................................................................
  AtomRegistry() : data(NULL) {}
  //..................................................................................................
  AtomRegistry(const AtomRegistry& r) : data(r.data) {  data->ref_cnt++;  }
  //..................................................................................................
  RegistryType& Init(const vec3i& mind, const vec3i& maxd)  {
    if( data != NULL && --data->ref_cnt == 0 )
      delete data;
    data = new DataStruct(mind, maxd);
    return data->registry;
  }
  //..................................................................................................
  ~AtomRegistry()  {
    if( data != NULL && --data->ref_cnt  == 0 )
      delete data;
  }
  //..................................................................................................
  AtomRegistry& operator = (const AtomRegistry& ar)  {
    if( data != NULL && --data->ref_cnt == 0 )
      delete data;
    data = ar.data;
    return *this;
  }
  //..................................................................................................
  RegistryType& GetRegistry()  {  return data->registry;  }
  //..................................................................................................
  TSAtom* Find(const TSAtom::Ref& ref) const {
    if( data == NULL )  return NULL;
    const vec3i t = smatd::GetT(ref.matrix_id);
    if( !data->registry.IsInRange(t) ) return false;
    TArrayList<TSAtomPList*>* aum_slice = data->registry.Value(t);
    if( aum_slice == NULL )  return NULL;
    TSAtomPList* au_slice = (*aum_slice)[smatd::GetContainerId(ref.matrix_id)];
    if( au_slice == NULL ) return false;
    return (*au_slice)[ref.catom_id];
  }
  //..................................................................................................
  TSBond* Find(const TSBond::Ref& ref) const {
    TSAtom* a = Find(ref.a);
    if( a == NULL )  return NULL;
    for( size_t i=0; i < a->BondCount(); i++ )  {
      if( !a->Bond(i).IsDeleted() && a->Bond(i).Another(*a) == ref.b )
        return &a->Bond(i);
    }
    return NULL;
  }
};
EndXlibNamespace()
#endif
