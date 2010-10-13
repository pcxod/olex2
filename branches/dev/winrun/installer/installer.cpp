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
#ifdef _DEBUG
  //MessageBox(NULL, _T("entering"), _T("info"), MB_OK);
#endif
  int argc = 0;
  const LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
  const olxstr module_name = TEFile::ExtractFileName(argv[0]);
  olxstr uninst_dir;
  bool uninstall = false;
  if( !module_name.Equals("olex2un.exe") )  {  //allow to run from tmp folder only
    TCHAR* tmp_path_cstr = new TCHAR[_MAX_PATH];
    DWORD sz=_MAX_PATH, needed_sz;
    if( (needed_sz=GetTempPath(sz, tmp_path_cstr)) > sz )  {
      delete [] tmp_path_cstr;
      tmp_path_cstr = new TCHAR[needed_sz+1];
      sz = GetTempPath(needed_sz, tmp_path_cstr);
    }
    const olxstr tmp_path = olxstr::FromExternal(tmp_path_cstr, needed_sz);
    const olxstr tmppath = TEFile::AddPathDelimeter(tmp_path);
    TCHAR* this_name = new TCHAR[_MAX_PATH];
    DWORD this_name_sz = GetModuleFileName(NULL, this_name, _MAX_PATH);
    const olxstr module_name  = olxstr::FromExternal(this_name, this_name_sz);
    const olxstr copy_name  = tmppath + "olex2un.exe";
    bool do_run = true;
    try  {  do_run = TEFile::Copy(module_name, copy_name, true);  }
    catch(const TExceptionBase&)  {
      do_run = false;
    }
    if( do_run )  {
      const olxstr exe_name = olxstr('\"') << copy_name << '\"';
      uninst_dir = olxstr('\"') <<
        TEFile::TrimPathDelimeter(TEFile::ExtractFilePath(module_name)) << '\"';
      _execl(copy_name.c_str(), exe_name.c_str(), uninst_dir.c_str(), NULL);
    }
    else  {
      MessageBox(NULL, _T("Failed to initialise the installer. Please make sure only one Olex2 installer is running"),
        _T("Error"), MB_OK|MB_ICONERROR);
    }
    TEGC::Finalise();
    return FALSE;
  }
  else  {
    MoveFileEx(argv[0], NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
    if( argc == 2 )
      uninst_dir = argv[1];
    if( TEFile::Exists(uninst_dir) )  {
      const olxstr base_dir = TEFile::AddPathDelimeter(uninst_dir);
      // just some checks at least...
      uninstall = TEFile::Exists(base_dir+"olex2.dll") &&
        TEFile::Exists(base_dir+"index.ind") &&
        TEFile::Exists(base_dir+"olex2.tag");
    }
  }
  CInstallerDlg dlg;
	m_pMainWnd = &dlg;
  main_dlg = &dlg;
  if( uninstall )
    dlg.SetInstalledPath(uninst_dir);
	dlg.DoModal();
	return FALSE;
}
