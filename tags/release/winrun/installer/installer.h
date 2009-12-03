#pragma once
#include "stdafx.h"

#include "resource.h"		// main symbols

class CInstallerApp : public CWinApp  {
public:
	CInstallerApp();

public:
  virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
extern class CInstallerDlg* main_dlg;
extern CInstallerApp theApp;