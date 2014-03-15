/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_shellutil_H
#define __olx_shellutil_H
#include "ebase.h"
#include "estrlist.h"
#include "typelist.h"

enum {
  fiDesktop = 1,
  fiStartMenu,
  fiPrograms,
  fiStartup,
  fiControls,
  fiProgramFiles, // returns Program Files native to the process x32 vs x64
  fiSysProgramFiles,  // returns Program Files folder native to the system
  fiMyDocuments,
  fiAppData,    // returns application data folder for specific user
  fiCommonStartMenu,
  fiCommonAppData,
  fiCommonPrograms,
  fiCommonDesktop
}; 


class TShellUtil  {
public:
  static bool CreateShortcut(const olxstr& ShortcutPath,
    const olxstr& ObjectPath, const olxstr& description, bool AddRunAs);
  static olxstr GetSpecialFolderLocation(short folderId);
  static olxstr PickFolder(const olxstr& Title, const olxstr& SelectedFolder,
     const olxstr& RootFolder);
  static olxstr PickFile(const olxstr& Title, const olxstr &Filter,
    bool open=false,
    const olxstr& DefFolder=EmptyString(),
    const olxstr &DefFile=EmptyString());
  // lists all interface names and related MAC addresses
  typedef TStringToList<olxstr, TArrayList<unsigned char> > MACInfo;
protected:
  static bool _MACFromArray(const unsigned char* bf, const char* name,
    MACInfo& mi, size_t len=6, bool accept_empty=false);
public:
  static void ListMACAddresses(MACInfo& rv);
  static olxstr QuoteArg(const olxstr &a);
  static olxstr GetCmdLineArgs(const olxstr &fn,
    bool args=true, bool options=false);
#ifdef __WIN32__
  static bool VerifyEmbeddedSignature(const olxstr &file_name);
  static olxstr GetFileVersion(const olxstr &fn,
    const olxstr &lang="080904E4");
  static bool RunElevated(const olxstr &fn, const olxstr &args=EmptyString());
  static bool IsAdmin();
#endif
};
#endif
