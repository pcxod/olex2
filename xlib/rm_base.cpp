/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "rm_base.h"
#include "leq.h"

size_t IXVarReferencer::GetReferencerId() const {
  return const_cast<IXVarReferencer*>(this)->GetParentContainer().GetIdOf(*this);
}
//........................................................................................
size_t IXVarReferencer::GetPersistentId() const {
  return const_cast<IXVarReferencer*>(this)->GetParentContainer().GetPersistentIdOf(*this);
}
//........................................................................................
bool IXVarReferencer::AreAllFixed(size_t start, size_t sz) const {
  OLX_VALIDATE_SUBRANGE(start, sz, 0, VarCount());
  for (size_t i = 0; i < sz; i++) {
    XVarReference* vr = GetVarRef(start+i);
    if (vr == 0 || vr->relation_type != relation_None) {
      return false;
    }
  }
  return true;
}
