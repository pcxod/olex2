#include "releasable.h"

AReleasable::AReleasable(parent_t& parent, bool tmp)
  : ReleasableId(InvalidIndex), parent(parent)
{
  if (!tmp) {
    parent.Add(this);
  }
}
void AReleasable::Release() { parent.Release(*this); }
//.............................................................................
void AReleasable::Restore() { parent.Restore(*this); }
//.............................................................................
