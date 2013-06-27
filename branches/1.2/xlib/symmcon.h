/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xlib_symcon_H
#define __olx_xlib_symcon_H
#include "xbase.h"
#include "edict.h"
#include "mat_id.h"

BeginXlibNamespace()

struct SymmConItem  {
  int param;
  double multiplier;
};

struct SymmCon  {
  int r_id;
  const SymmConItem* map;  // U + coordinates
};

struct SiteSymmCon  {
private:
  bool added;
public:
  SymmConItem map[9];
  SiteSymmCon();
  bool IsConstrained() const;
  SiteSymmCon& operator += (const SymmCon* sc);
  olxstr ToString() const;
};

class SymmConReg  {
  static const SymmCon* _Find(int rot_id);
public:
  static const SymmCon* Find(int rot_id)  {
    return _Find(rot_id); // both matrices are stored
    //const SymmCon* rv = _Find(rot_id);
    //return rv == NULL ? _Find(rotation_id::invert(rot_id)) : rv;
  }
};

EndXlibNamespace()

#endif
