/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_str_mask_h
#define __olx_sdl_str_mask_h
#include "ebase.h"
BeginEsdlNamespace()

class TStrMask {
  bool matchAll, matchAny, matchEnd, matchStart, matchBoth;
  olxstr mask, start, end;
public:
  TStrMask(const olxstr& _mask) : mask(_mask)  {
    matchAll = matchAny = matchEnd = matchStart = matchBoth = false;
    if( mask.IsEmpty() || (mask.Length() == 1 && mask.CharAt(0) == '*') )
      matchAll = true;
    else {
      size_t si = mask.FirstIndexOf('*');
      if( si == InvalidIndex )  {
        matchAny = true;
      }
      else  {
        if( si == (mask.Length()-1) )  {
          matchStart = true;
          start = mask.SubStringTo(si);
        }
        else if( si == 0 )  {
          matchEnd = true;
          end = mask.SubStringFrom(si+1);
        }
        else  {
          matchBoth = true;
          start = mask.SubStringTo(si);
          end = mask.SubStringFrom(si+1);
        }
      }
    }
  }
  bool DoesMatch(const olxstr& str) const {
    if( matchAll )  return true;
    if( matchBoth )  
      return (str.StartsFrom(start) && str.EndsWith(end));
    if( matchAny )
      return str.IndexOf(mask) != InvalidIndex;
    if( matchStart )
      return str.StartsFrom(start);
    if( matchEnd )
      return str.EndsWith(end);
    return false;
  }
  bool DoesMatchi(const olxstr& str) const {
    if( matchAll )  return true;
    if( matchBoth )  
      return (str.StartsFromi(start) && str.EndsWithi(end));
    if( matchAny )
      return str.IndexOfi(mask) != InvalidIndex;
    if( matchStart )
      return str.StartsFromi(start);
    if( matchEnd )
      return str.EndsWithi(end);
    return false;
  }
};

EndEsdlNamespace()
#endif
