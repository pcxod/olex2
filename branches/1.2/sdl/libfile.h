/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_libfile_H
#define __olx_sdl_libfile_H
#include "library.h"
BeginEsdlNamespace()

struct LibFile {
  static void FileExists(const TStrObjList& Params, TMacroData& E);
  static void FileName(const TStrObjList& Params, TMacroData& E);
  static void FilePath(const TStrObjList& Params, TMacroData& E);
  static void FileDrive(const TStrObjList& Params, TMacroData& E);
  static void FileExt(const TStrObjList& Params, TMacroData& E);
  static void ChangeFileExt(const TStrObjList& Params, TMacroData& E);
  static void Copy(const TStrObjList& Params, TMacroData& E);
  static void Rename(const TStrObjList& Params, TMacroData& E);
  static void Delete(const TStrObjList& Params, TMacroData& E);
  static void CurDir(const TStrObjList& Params, TMacroData& E);
  static void ChDir(const TStrObjList& Params, TMacroData& E);
  static void MkDir(const TStrObjList& Params, TMacroData& E);
  static void OSPath(const TStrObjList& Params, TMacroData& E);
  static void Which(const TStrObjList& Params, TMacroData& E);
  static void Age(const TStrObjList& Params, TMacroData& E);
  static void ListDirForGUI(const TStrObjList& Params, TMacroData& E);
  static void CreateRelativePath(const TStrObjList& Params, TMacroData& E);
  static void ExpandRelativePath(const TStrObjList& Params, TMacroData& E);
  static TLibrary *ExportLibrary(const olxstr &name=EmptyString());
};

EndEsdlNamespace()
#endif
