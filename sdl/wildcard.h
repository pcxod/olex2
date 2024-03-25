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
#include "eset.h"

class Wildcard {
  TStrList toks;
  olxstr mask;
  size_t toksEnd, toksStart;
  bool hasWildcards, caseInsensitive;
public:
  Wildcard(bool caseInsensitive = true)
    : toksEnd(InvalidIndex), toksStart(InvalidIndex), hasWildcards(false),
    caseInsensitive(caseInsensitive)
  {}
  Wildcard(const olxstr& msk, bool caseInsensitive=true) {
    Build(msk);
  }
  void Build(const olxstr& msk);
  bool DoesMatch(const olxstr& _str) const;
  const olxstr &GetMask() const {
    return mask;
  }
  int Compare(const Wildcard &c) const {
    return mask.Compare(c.mask);
  }
  static bool IsMask(const olxstr &m);
};

// a helper class to deal with a list of masks or non-mask values
class WildcardList {
  TTypeList<Wildcard> w_cards;
  olxset<olxstr, olxstrComparator<true> > items;
public:
  WildcardList()
  {}
  // adds a mask or a string item
  void Add(const olxstr &item, bool caseInsensitive=true);
  bool DoesMatch(const olxstr &value) const;
  /* return [wildcard mask or the matching item] or empty string if there is
  no match
  */
  const olxstr &FindMatching(const olxstr &value) const;
};
#endif
