#pragma once
#include "stdafx.h"
#include "resource.h"		// main symbols
#include "ebase.h"

class CInstallerApp : public CWinApp  {
public:
	CInstallerApp();
  // initialises (if olex2 is installed) olex2_install_path and returns it or CmdLine
  bool olex2_installed;
  olxstr olex2_installed_path;
public:
  virtual BOOL InitInstance();
  bool IsOlex2Installed() const {  return olex2_installed;  }
  olxstr LocateBaseDir(const olxstr& install_tag=EmptyString);
  const olxstr& GetOlex2InstalledPath() const {  return olex2_installed_path;  }
  void SetOlex2InstalledPath(const olxstr& v)  {
    if( !v.IsEmpty() )
      olex2_installed = true;
    olex2_installed_path = v;
  }
  static bool IsUserAdmin();
  void ShowError(const olxstr& msg, const olxstr& title="Error");
	DECLARE_MESSAGE_MAP()
};
extern class CInstallerDlg* main_dlg;
extern CInstallerApp theApp;