/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_wildcard_H
#define __olx_sdl_wildcard_H
#include "estrlist.h"

class Wildcard {
  TStrList toks;
  olxstr mask;
  size_t toksEnd, toksStart;
public:
  Wildcard() : toksEnd(InvalidIndex), toksStart(InvalidIndex) {}
  Wildcard(const olxstr& msk) { Build(msk); }
  void Build(const olxstr& msk);
  bool DoesMatch(const olxstr& _str) const;
};
#endif
