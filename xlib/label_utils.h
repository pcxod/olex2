/******************************************************************************
* Copyright (c) 2004-2025 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#pragma once
#include "xbase.h"

BeginXlibNamespace()

struct AtomLabelInfo {
  enum {
    no_part = -10000
  };

  olxstr label;
  int part;
  int resi_number;
  olxch chain_id;
  olxstr resi_class, equiv_id;

  AtomLabelInfo(const olxstr& Label);

  bool DoesMatch(const class TCAtom& a, bool match_label) const;
  // checks against equiv_id as well
  bool DoesMatch(const class TSAtom& a, bool match_label) const;

};

EndXlibNamespace()

