/******************************************************************************
* Copyright (c) 2004-2026 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/
#pragma once
#include "xbase.h"

BeginXlibNamespace()
namespace SFUtil {
  enum MapType {
    Diff,
    Obs,
    Calc,
    TwoObs_Calc
  };
  enum ScaleType {  // scale for difference map
    Sigma,
    Shelx,
    External,
    ExternalForced
  };
  enum SFOrigin { // structure factor origin
    Fcf,
    Olex2
  };
  // merge Friedel pairs
  enum FPMerge {
    Default,  // depending on SG
    Merge,
    DoNotMerge
  };

  static const double T_PI = M_PI * 2;
  static const double MT_PI = -M_PI * 2;
  const static double EQ_PI = 8 * M_PI * M_PI;
  const static double TQ_PI = 2 * M_PI * M_PI;

  // EXTI destination - Fc/Fo
  enum EXTIDest {
    Fo,
    Fc
  };
};
EndXlibNamespace()
