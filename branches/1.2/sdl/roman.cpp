/******************************************************************************
* Copyright (c) 2004-2015 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/
#include "roman.h"
#include "exception.h"

olxstr RomanNumber::To(size_t v) {
  olxstr rv;
  if (v == 0) {
    rv = '0';
    return rv;
  }
  else if (v >= 4000) {
    throw TInvalidArgumentException(__OlxSourceInfo, "value out of range");
  }
  char cd[] = "MDCLXV";
  size_t cv[] = {1000, 500, 100, 50, 10, 5};
  for (size_t i = 0; i < 6; i++) {
    if (v < cv[i]) continue;
    size_t j = v / cv[i];
    if (j == 4 && i > 0) {
      rv << 'I' << cd[i - 1];
    }
    else {
      for (size_t k = 0; k < j; k++) {
        rv << cd[i];
      }
    }
    if ((v -= j*cv[i]) == 0) {
      break;
    }
  }
  if (v == 4) {
    rv << 'I' << 'V';
  }
  else {
    for (size_t i = 0; i < v; i++) {
      rv << 'I';
    }
  }
  return rv;
}

size_t RomanNumber::From(const olxstr &v) {
  if (v == '0') {
    return 0;
  }
  size_t rv = 0;
  for (size_t i = 0; i < v.Length(); i++) {
    switch (v.CharAt(i)) {
    case 'M': rv += 1000; break;
    case 'D': rv += 500; break;
    case 'C': rv += 100; break;
    case 'L': rv += 50; break;
    case 'X': rv += 10; break;
    case 'V': rv += 5; break;
    case 'I':
      {
        size_t j = i + 1;
        if (j < v.Length()) {
          switch (v.CharAt(j)) {
          case 'M': rv += 900; break;
          case 'D': rv += 400; break;
          case 'C': rv += 90; break;
          case 'L': rv += 40; break;
          case 'X': rv += 9; break;
          case 'V': rv += 4; break;
          default:
            j = InvalidIndex;
          }
        }
        else {
          rv++;
        }
        if (j == InvalidIndex) {
          rv++;
        }
        else {
          i++;
        }
        break;
      }
    }
  }
  return rv;
}

