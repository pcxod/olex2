#include "rm_base.h"

size_t IXVarReferencer::GetReferencerId() const {  
  return const_cast<IXVarReferencer*>(this)->GetParentContainer().GetIdOf(*this);  
}
//........................................................................................
size_t IXVarReferencer::GetPersistentId() const {  
  return const_cast<IXVarReferencer*>(this)->GetParentContainer().GetPersistentIdOf(*this);  
}
