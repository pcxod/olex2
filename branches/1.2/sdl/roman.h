/******************************************************************************
* Copyright (c) 2004-2015 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_roman_H
#include "ebase.h"
BeginEsdlNamespace()

struct RomanNumber {
  /* accepts values [4-4000), throws TInvalidArgument exception if the value is
  not in the range. Returns "0" for 0
  */
  static olxstr To(size_t i);
  /* reverses the ToRoma function result. Does not support double reduction.
  */
  static size_t From(const olxstr &v);
};

EndEsdlNamespace()
#endif
