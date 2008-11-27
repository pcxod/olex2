#include "leq.h"
#include "catom.h"

int XVar::ReferenceCount() const {
  int rv = 0;
  for( int i=0; i < References.Count(); i++ )  
    if( !References[i].atom->IsDeleted() )
      rv++;
  return rv;
}
