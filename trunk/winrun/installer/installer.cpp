// installer.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "ebase.h"
#include "bapp.h"
#include "estrlist.h"

#include "installer.h"
#include "installerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CInstallerDlg* main_dlg;

BEGIN_MESSAGE_MAP(CInstallerApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


CInstallerApp::CInstallerApp()  {
}


CInstallerApp theApp;


BOOL CInstallerApp::InitInstance()  {
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);
  AfxInitRichEdit();
	CWinApp::InitInstance();
  CInstallerDlg dlg;
	m_pMainWnd = &dlg;
  main_dlg = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
