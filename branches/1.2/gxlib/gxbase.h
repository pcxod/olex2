/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gxl_base_H
#define __olx_gxl_base_H
#include "ebase.h"

#define BeginGxlNamespace()  namespace gxlib {
#define EndGxlNamespace()  };\
  using namespace gxlib;
#define UseGxlNamespace()  using namespace gxlib;
#define GlobalGxlFunction(fun)     gxlib::fun
#define GxlObject(obj)     gxlib::obj

BeginGxlNamespace()

static olxstr
  PLabelsCollectionName("PLabels");
const int16_t
  qaHigh    = 1,  // drawing quality
  qaMedium  = 2,
  qaLow     = 3,
  qaPict    = 4;

const short
  ddsDef       = 0, // default drawing style for primitives
  ddsDefAtomA  = 1,
  ddsDefAtomB  = 2,
  ddsDefRim    = 3,
  ddsDefSphere = 4;

EndGxlNamespace()
#endif
