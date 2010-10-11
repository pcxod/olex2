#include "stdafx.h"
#include "ebase.h"
#include "bapp.h"
#include "estrlist.h"
#include "installer.h"
#include "installerDlg.h"
#include "filetree.h"

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
  int argc = 0;
  const LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
  if( argc == 4 && olxstr("/uninstall").Equalsi(argv[1]) )  {
    const olxstr p_id = argv[2];
    const olxstr dir = argv[3];
    HANDLE pid = (HANDLE)p_id.RadInt<int64_t>();  
    DWORD Status;
    GetExitCodeProcess(pid, &Status);
    while( Status == STILL_ACTIVE )  {
      SleepEx(50, TRUE);
      GetExitCodeProcess(pid, &Status);
    }
    TFileTree::Delete(dir);
    TCHAR this_name[_MAX_PATH];
    GetModuleFileName(NULL, this_name, _MAX_PATH);
    MoveFileEx(this_name, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
    return TRUE;
  }
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
