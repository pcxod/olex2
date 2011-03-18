#include "stdafx.h"
#include <atlbase.h>
#include "ebase.h"
#include "bapp.h"
#include "estrlist.h"
#include "installer.h"
#include "installerDlg.h"
#include "filetree.h"
#include "patchapi.h"
#include "utf8file.h"
#include "cdsfs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CInstallerDlg* main_dlg;

BEGIN_MESSAGE_MAP(CInstallerApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


CInstallerApp::CInstallerApp()  {}

CInstallerApp theApp;

BOOL CInstallerApp::InitInstance()  {
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);
  AfxInitRichEdit();
	CWinApp::InitInstance();
#ifdef _DEBUG  // this to be used for debugging of the re-launched process
  //MessageBox(NULL, _T("entering"), _T("info"), MB_OK);
#endif
  TBasicApp bapp(LocateBaseDir());
  try  {
    const olxstr log_dir = patcher::PatchAPI::GetSharedDirRoot();
    if( !TEFile::Exists(log_dir) )
      TEFile::MakeDirs(log_dir);
    TBasicApp::GetLog().AddStream(TUtf8File::Open(log_dir + "installer.log", false), true);
    TSocketFS::InitLocalFSBase(log_dir + ".cds/");
  }
  catch(...)  {
    MessageBox(NULL, _T("Failed to enable logging"), _T("Error"), MB_OK|MB_ICONERROR);
  }
  TExceptionBase::SetAutoLogging(true);
  bapp.NewLogEntry() << "Log started at " << TETime::FormatDateTime(TETime::Now());
  int argc = 0;
  const LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
  const olxstr module_name = TEFile::ExtractFileName(argv[0]);
  olxstr uninst_dir;
  bool uninstall = false;
  if(
#ifdef _DEBUG  // debugging of current process
      false &&
#endif
    !module_name.Equals("olex2un.exe") )  {  //allow to run from tmp folder only
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
    try  {
      bapp.NewLogEntry(logInfo, true) << "Copying '" << module_name << "' into '" << copy_name;
      do_run = TEFile::Copy(module_name, copy_name, true);
      do_run = TEFile::Exists(copy_name);
    }
    catch(const TExceptionBase&)  {
      bapp.NewLogEntry(logInfo, true) << "Copying failed";
      do_run = false;
    }
    if( do_run )  {
      const olxstr exe_name = olxstr('\"') << copy_name << '\"';
      uninst_dir = olxstr('\"') <<
        TEFile::TrimPathDelimeter(TEFile::ExtractFilePath(module_name)) << '\"';
      _execl(copy_name.c_str(), exe_name.c_str(), uninst_dir.c_str(), NULL);
    }
    else  {
      MessageBox(NULL, (olxstr("Failed to initialise the installer.\n"
        "Please make sure only one Olex2 installer is running,\n"
        "alternatively try deleting this file manually:\n")
        << copy_name).u_str(),
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
//....................................................................................
olxstr CInstallerApp::LocateBaseDir(const olxstr& install_tag)  {
  CRegKey rc;
  olex2_installed = false;
  olex2_installed_path = EmptyString();
  olxstr app_name("olex2-");
  app_name << install_tag << ".exe";
  try  {
    if( rc.Open(HKEY_CLASSES_ROOT, (olxstr("Applications\\") << app_name <<
      "\\shell\\open\\command").u_str(), KEY_READ) == ERROR_SUCCESS )
    {
      olxch* rv = new olxch[MAX_PATH];
      ULONG sz_rv = MAX_PATH;
      if( rc.QueryStringValue(_T(""), rv, &sz_rv) == ERROR_SUCCESS )  {
        olex2_installed_path = rv;
        olex2_installed_path = TEFile::ExtractFilePath(olex2_installed_path.Trim('"'));
      }
      delete [] rv;
      rc.Close();
    }
  }
  catch( ... )  {}
  if( !TEFile::Exists(olex2_installed_path) )
    olex2_installed_path = EmptyString();
  else
    olex2_installed = true;
  if( olex2_installed_path.IsEmpty() )  {
    int argc = 0;
    const LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    return argv[0];
  }
  return olex2_installed_path;
}
//....................................................................................
void CInstallerApp::ShowError(const olxstr& msg, const olxstr& title)  {
  MessageBox(NULL, msg.u_str(), title.u_str(), MB_OK|MB_ICONERROR);
  TBasicApp::NewLogEntry(logDefault, true) << msg;
}
//....................................................................................
//http://msdn.microsoft.com/en-us/library/aa376389(VS.85).aspx
bool CInstallerApp::IsUserAdmin(VOID) {
  BOOL b;
  SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
  PSID AdministratorsGroup; 
  b = AllocateAndInitializeSid(
    &NtAuthority,
    2,
    SECURITY_BUILTIN_DOMAIN_RID,
    DOMAIN_ALIAS_RID_ADMINS,
    0, 0, 0, 0, 0, 0,
    &AdministratorsGroup); 
  if( b == TRUE )   {
    if( !CheckTokenMembership( NULL, AdministratorsGroup, &b) )
      b = FALSE;
    FreeSid(AdministratorsGroup); 
  }
  return b == TRUE;
}
//....................................................................................
