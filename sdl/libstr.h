/******************************************************************************
* Copyright (c) 2004-2024 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/
#pragma once
#include "library.h"
BeginEsdlNamespace()

struct LibStr {
  static void ToUpper(const TStrObjList& Params, TMacroData& E);
  static void ToLower(const TStrObjList& Params, TMacroData& E);
  static void Add(const TStrObjList& Params, TMacroData& E);
  static void Eq(const TStrObjList& Params, TMacroData& E);
  static void Eqi(const TStrObjList& Params, TMacroData& E);
  static void Like(const TStrObjList& Params, TMacroData& E);
  static void Likei(const TStrObjList& Params, TMacroData& E);
  static TLibrary* ExportLibrary(const olxstr& name = EmptyString());
};

EndEsdlNamespace()
