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
  mask = msk.ToLowerCase();
  toks.Strtok(mask, '*');
  if (!mask.IsEmpty()) {
    toksStart = (mask.CharAt(0) != '*') ? 1 : 0;
    toksEnd = toks.Count() - ((mask.GetLast() != '*') ? 1 : 0);
  }
  else  {
    toksStart = 0;
    toksEnd = toks.Count();
  }
}

bool Wildcard::DoesMatch(const olxstr& _str) const {
  if (mask.IsEmpty() && !_str.IsEmpty()) return false;
  // this will work for '*' mask
  if (toks.IsEmpty())  return true;
  // need to check if the mask starts from a '*' or ends with it
  const olxstr str = _str.ToLowerCase();
  size_t off = 0, start = 0, end = str.Length();
  if (mask[0] != '*')  {
    const olxstr& tmp = toks[0];
    if (tmp.Length() > str.Length()) return false;
    for (size_t i=0; i < tmp.Length(); i++) {
      if (tmp.CharAt(i) != '?' && tmp.CharAt(i) != str.CharAt(i))
        return false;
    }
    start = tmp.Length();
    if (toks.Count() == 1)
      return tmp.Length() == str.Length() ? true : mask.GetLast() == '*';
  }
  if (mask.GetLast() != '*' && toks.Count() > (size_t)(mask[0] != '*' ? 1 : 0)) {
    const olxstr& tmp = toks[toks.Count()-1];
    if (tmp.Length() > (str.Length()-start)) return false;
    for (size_t i=0; i < tmp.Length(); i++) {
      if (!(tmp[i] == '?' || tmp[i] == str[str.Length()-tmp.Length()+i]))
        return false;
    }
    end = str.Length() - tmp.Length();
    if (toks.Count() == 1) return true;
  }

  for (size_t i=toksStart; i < toksEnd; i++) {
    const olxstr& tmp = toks[i];
    bool found = false;
    for (size_t j=start; j < end; j++) {
      if ((str.Length()-j) < tmp.Length()) return false;
      if (tmp[off] == '?' || str[j] == tmp[off]) {
        while (tmp[off] == '?' || tmp[off] == str[j+off]) {
          off++;
          if (off == tmp.Length()) break;
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
    if (!found) return false;
  }
  return true;
}
