/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "rm_base.h"

size_t IXVarReferencer::GetReferencerId() const {
  return const_cast<IXVarReferencer*>(this)->GetParentContainer().GetIdOf(*this);
}
//........................................................................................
size_t IXVarReferencer::GetPersistentId() const {
  return const_cast<IXVarReferencer*>(this)->GetParentContainer().GetPersistentIdOf(*this);
}
