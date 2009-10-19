#pragma once
#include "stdafx.h"
#include "resource.h"		// main symbols
#include "bapp.h"

class LaunchApp : public CWinApp  {
  TBasicApp Bapp;
  bool launch_successful;
public:
	LaunchApp();

public:
	virtual BOOL InitInstance();
  void Launch();
  DECLARE_MESSAGE_MAP()
};

extern class MainDlg* dlgSplash;

extern LaunchApp theApp;