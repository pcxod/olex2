/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "wildcard.h"

void Wildcard::Build(const olxstr& msk) {
  mask = caseInsensitive ? msk.ToLowerCase() : msk;
  hasWildcards = mask.ContainAnyOf("*?");
  if (hasWildcards) {
    toks.Strtok(mask, '*');
    if (!mask.IsEmpty()) {
      toksStart = (mask.CharAt(0) != '*') ? 1 : 0;
      toksEnd = toks.Count() - ((mask.GetLast() != '*') ? 1 : 0);
    }
    else {
      toksStart = 0;
      toksEnd = toks.Count();
    }
  }
}

bool Wildcard::DoesMatch(const olxstr& _str) const {
  if (mask.IsEmpty() && !_str.IsEmpty()) {
    return false;
  }
  if (!hasWildcards) {
    return caseInsensitive ? mask.Equalsi(_str.ToLowerCase())
      : mask.Equals(_str.ToLowerCase());
  }
  // this will work for '*' mask
  if (toks.IsEmpty()) {
    return true;
  }
  // need to check if the mask starts from a '*' or ends with it
  const olxstr str = caseInsensitive ? _str.ToLowerCase() : _str;
  size_t off = 0, start = 0, end = str.Length();
  if (mask[0] != '*')  {
    const olxstr& tmp = toks[0];
    if (tmp.Length() > str.Length()) {
      return false;
    }
    for (size_t i=0; i < tmp.Length(); i++) {
      if (tmp.CharAt(i) != '?' && tmp.CharAt(i) != str.CharAt(i)) {
        return false;
      }
    }
    start = tmp.Length();
    if (toks.Count() == 1) {
      return tmp.Length() == str.Length() ? true : mask.GetLast() == '*';
    }
  }
  if (mask.GetLast() != '*' && toks.Count() > (size_t)(mask[0] != '*' ? 1 : 0)) {
    const olxstr& tmp = toks[toks.Count()-1];
    if (tmp.Length() > (str.Length() - start)) {
      return false;
    }
    for (size_t i=0; i < tmp.Length(); i++) {
      if (!(tmp[i] == '?' || tmp[i] == str[str.Length() - tmp.Length() + i])) {
        return false;
      }
    }
    end = str.Length() - tmp.Length();
    if (toks.Count() == 1) {
      return true;
    }
  }

  for (size_t i=toksStart; i < toksEnd; i++) {
    const olxstr& tmp = toks[i];
    bool found = false;
    for (size_t j=start; j < end; j++) {
      if ((str.Length() - j) < tmp.Length()) {
        return false;
      }
      if (tmp[off] == '?' || str[j] == tmp[off]) {
        while (tmp[off] == '?' || tmp[off] == str[j+off]) {
          off++;
          if (off == tmp.Length()) {
            break;
          }
        }
        start = j+off;
        if (off == tmp.Length()) {  // found the mask string
          found = true;
          off = 0;
          break;
        }
        off = 0;
      }
    }
    if (!found) {
      return false;
    }
  }
  return true;
}
//.............................................................................
bool Wildcard::IsMask(const olxstr &m) {
  return m.ContainAnyOf("*?");
}
//.............................................................................
//.............................................................................
//.............................................................................
void WildcardList::Add(const olxstr &item, bool caseInsensitive) {
  if (Wildcard::IsMask(item)) {
    w_cards.AddNew(item, caseInsensitive);
  }
  else {
    items.Add(item);
  }
}
//.............................................................................

bool WildcardList::DoesMatch(const olxstr &value) const {
  size_t idx = items.IndexOf(value);
  if (idx != InvalidIndex) {
    return true;
  }
  for (size_t i = 0; i < w_cards.Count(); i++) {
    if (w_cards[i].DoesMatch(value)) {
      return true;
    }
  }
  return false;
}
//.............................................................................
const olxstr &WildcardList::FindMatching(const olxstr &value) const {
  size_t idx = items.IndexOf(value);
  if (idx != InvalidIndex) {
    return items[idx];
  }
  for (size_t i = 0; i < w_cards.Count(); i++) {
    if (w_cards[i].DoesMatch(value)) {
      return w_cards[i].GetMask();
    }
  }
  return EmptyString();
}
//.............................................................................
