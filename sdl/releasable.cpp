#include "releasable.h"

AReleasable::AReleasable(parent_t& parent)
  : ReleasableId(InvalidIndex), parent(parent)
{
  parent.Add(this);
}
void AReleasable::Release() { parent.Release(*this); }
//.............................................................................
void AReleasable::Restore() { parent.Restore(*this); }
//.............................................................................
