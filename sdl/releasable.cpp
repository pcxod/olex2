/******************************************************************************
* Copyright (c) 2004-2026 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

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
