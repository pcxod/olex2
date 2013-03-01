/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "esort.h"

ReverseComparator_<TComparableComparator> EsdlObject(olx_reverse_cmp)() {
  return ReverseComparator_<TComparableComparator>(TComparableComparator());
}
