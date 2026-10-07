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

static const olxstr PLabelsCollectionName() {
  static olxstr v = "PLabels";
  return v;
}
const int
  qaHigh    = 1,  // drawing quality
  qaMedium  = 2,
  qaLow     = 3,
  qaPict    = 4;
/* what a fresh style starts with (qual 1/2 and their tessellation). Android:
low - gl4es emulates fixed-function GL per vertex on the CPU, and on a
Cortex-A53 that halves the frame time of a small structure
*/
#ifdef __ANDROID__
const int qaDefault = qaLow, qaDefaultSteps = 5;
#else
const int qaDefault = qaMedium, qaDefaultSteps = 15;
#endif

const int
  ddsDef       = 0, // default drawing style for primitives
  ddsDefAtomA  = 1,
  ddsDefAtomB  = 2,
  ddsDefRim    = 3,
  ddsDefSphere = 4;

EndGxlNamespace()
#endif
