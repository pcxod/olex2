/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "integration.h"

using namespace olex;

const olxch *c_process_function(const olxch *f) {
  IOlexProcessor *ip = IOlexProcessor::GetInstance();
  if (ip == NULL)
    return NULL;
  olxstr s(f);
  if (ip->processFunction(s)) {
    olxstr *rv = new olxstr(s);
    TEGC::Add(rv);
    return rv->u_str();
  }
  return NULL;
}

bool c_process_macro(const olxch *f) {
  IOlexProcessor *ip = IOlexProcessor::GetInstance();
  if (ip == NULL)
    return false;
  return ip->processMacro(f);
}
