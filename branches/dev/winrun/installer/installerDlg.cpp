#include "stdafx.h"

#include <atlbase.h>
#include "shellutil.h"
#include "efile.h"
#include "updateapi.h"
#include "patchapi.h"
#include "cdsfs.h"
#include "winzipfs.h"

#include "installer.h"
#include "installerDlg.h"
#include "reinstallDlg.h"
#include "licenceDlg.h"
#include "repositoriesDlg.h"
#include "mfc_ext.h"
#include "olxth.h"

using namespace updater;
using namespace patcher;
using namespace mfc_ext;
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

class TProgress: public AActionHandler  {
public:
  TProgress(){}
  virtual ~TProgress(){}
  bool Exit(const IEObject *Sender, const IEObject *Data)  {  
    return true;  
  }
  bool Enter(const IEObject *Sender, const IEObject *Data)  {
    if( !EsdlInstanceOf( *Data, TOnProgress) )  return false;
    const TOnProgress *A = dynamic_cast<const TOnProgress*>(Data);
    double div = 10;
    if( Sender != NULL )  {
      if( EsdlInstanceOf(*Sender, THttpFileSystem) )  {
        main_dlg->SetAction(olxstr("Downloading: ") << TEFile::ExtractFileName(A->GetAction().c_str()));
        div *= 10240;
      }
    }
    progress_bar::set_range(main_dlg, IDC_PB_PROGRESS, 0, (int)(A->GetMax()/div));
    return true;
  }
  bool Execute(const IEObject *Sender, const IEObject *Data)  {
    if( !EsdlInstanceOf( *Data, TOnProgress) )  return false;
    const TOnProgress *A = dynamic_cast<const TOnProgress*>(Data);
    if( Sender != NULL && EsdlInstanceOf(*Sender, THttpFileSystem) )
      progress_bar::set_pos(main_dlg, IDC_PB_PROGRESS, (int)(A->GetPos()/(10240*10)));
    else  {
      main_dlg->SetAction(TEFile::ExtractFileName(A->GetAction()));
      progress_bar::set_pos(main_dlg, IDC_PB_PROGRESS, (int)(A->GetPos()/10));
    }
    main_dlg->ProcessMessages();
    return true;
  }
};

class UpdaterTh : public AOlxThread {
public:
  UpdaterTh() {  Detached = false;  }
  int Run()  {
    CRepositoriesDlg dlg;
    dlg.Create(IDD_REPOSITORIES);
    dlg.ShowWindow(SW_SHOW);
    while( true )  {
      MSG msg;
      if( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) )  {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
      else  {
        olx_sleep(50);
        dlg.GetDlgItem(IDC_PG_REPOSTORIES)->SendMessage(PBM_STEPIT);
      }
      if( Terminate )  return 1;
    }
  }
};

const olxstr CInstallerDlg::exts[] = {".ins", ".res", ".mol", ".cif", ".xyz" };
const size_t CInstallerDlg::exts_sz = sizeof(exts)/sizeof(exts[0]);

CInstallerDlg::CInstallerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CInstallerDlg::IDD, pParent), bapp(LocateBaseDir())
{
  tooltipCtrl = NULL;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
  mouse_down = false;
  action = rename_status = 0;
  // init admin mode for shortcuts if required
  OSVERSIONINFO veri;
  memset(&veri, 0, sizeof(veri));
  veri.dwOSVersionInfoSize = sizeof(veri);
  GetVersionEx(&veri);
  // only after XP
  run_as_admin = veri.dwMajorVersion > 5;
}
CInstallerDlg::~CInstallerDlg()  {
  if( tooltipCtrl != NULL )  delete tooltipCtrl;
  if( ctrlBrush != NULL )  delete ctrlBrush;
}

void CInstallerDlg::DoDataExchange(CDataExchange* pDX)  {
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CInstallerDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
  ON_BN_CLICKED(IDC_BTN_CHOOSE_PATH, &CInstallerDlg::OnBnClickedBtnChoosePath)
  ON_WM_LBUTTONDOWN()
  ON_WM_LBUTTONUP()
  ON_WM_MOUSEMOVE()
  ON_BN_CLICKED(ID_INSTALL, &CInstallerDlg::OnBnClickedInstall)
  ON_BN_CLICKED(IDC_BTN_CLOSE, &CInstallerDlg::OnBnClickedBtnClose)
  ON_BN_CLICKED(IDC_CB_PROXY, &CInstallerDlg::OnBnClickedCbProxy)
  ON_WM_SHOWWINDOW()
  ON_WM_TIMER()
  ON_BN_CLICKED(IDC_BTN_PROXY, &CInstallerDlg::OnBnClickedBtnProxy)
  ON_BN_CLICKED(IDC_BTN_REPOSITORY, &CInstallerDlg::OnBnClickedBtnRepository)
  ON_WM_ERASEBKGND()
  ON_WM_CTLCOLOR()
  ON_CBN_SELENDOK(IDC_CB_REPOSITORY, &CInstallerDlg::OnCbnSelendokCbRepository)
END_MESSAGE_MAP()


// CInstallerDlg message handlers

BOOL CInstallerDlg::OnInitDialog()  {
	CDialog::OnInitDialog();
  ctrlBrush = new CBrush(bgColor);
  tooltipCtrl = new CToolTipCtrl;
  tooltipCtrl->Create(this);
  this->SendMessage(WM_SETTEXT, 0, (LPARAM)_T("Olex2 installer"));
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

  olex2_install_path = TShellUtil::GetSpecialFolderLocation(fiProgramFiles) << "Olex2";
  wnd::set_text(this, IDC_TE_INSTALL_PATH, olex2_install_path);
  wnd::set_text(this, IDC_TE_PROXY, EmptyString);
  wnd::set_enabled(this, IDC_TE_PROXY, false);
  check_box::set_checked(this, IDC_R_ALWAYS, true);
  check_box::set_checked(this, IDC_CB_SHORTCUT, true);
  check_box::set_checked(this, IDC_CB_DESKTOP, true);
  CBitmap bmp;
  bmp.LoadBitmap(IDB_UPDATE);
  GetDlgItem(IDC_BTN_PROXY)->SendMessage(BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)bmp.Detach());
  tooltipCtrl->AddTool( GetDlgItem(IDC_BTN_PROXY), _T("Reload repositories list"));
  tooltipCtrl->Activate(TRUE);
  if( olex2_installed )  {
    TEFile::AddPathDelimeter(olex2_installed_path);
    olxstr sfile = olex2_installed_path;
    sfile << UpdateAPI::GetSettingsFileName();
    if( TEFile::Exists(sfile) )  {
      const TSettingsFile Settings(sfile);
      wnd::set_text(this, IDC_TE_PROXY, Settings["proxy"]);
      olxstr update_i = Settings["update"];
      int update_c = -1;
      if( update_i.Equalsi("Always") )  update_c = IDC_R_ALWAYS;
      else if( update_i.Equalsi("Daily") )  update_c = IDC_R_DAILY;
      else if( update_i.Equalsi("Monthly") )  update_c = IDC_R_MONTHLY;
      else if( update_i.Equalsi("Never") )  update_c = IDC_R_NEVER;
      if( update_c != -1 )
        GetDlgItem(update_c)->SendMessage(BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
    }
    olex2_data_dir = PatchAPI::ComposeNewSharedDir(TShellUtil::GetSpecialFolderLocation(fiAppData), olex2_installed_path);
    SetAction(actionReinstall);
  }
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CInstallerDlg::OnPaint()  {
	if (IsIconic())	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CInstallerDlg::OnQueryDragIcon()  {
	return static_cast<HCURSOR>(m_hIcon);
}


void CInstallerDlg::OnBnClickedBtnChoosePath()  {
  olxstr dir( TShellUtil::PickFolder("Please select installation folder",
    TShellUtil::GetSpecialFolderLocation(fiProgramFiles), EmptyString) );
  if( !dir.IsEmpty() )  {
    olex2_install_path = dir << "\\Olex2";
    SetInstallationPath(dir << '-' << olex2_install_tag);
  }
}

void CInstallerDlg::OnLButtonDown(UINT nFlags, CPoint point)  {
  mouse_down = true;
  mouse_pos = point;
  CDialog::OnLButtonDown(nFlags, point);
}

void CInstallerDlg::OnLButtonUp(UINT nFlags, CPoint point)  {
  mouse_down = false;
  CDialog::OnLButtonUp(nFlags, point);
}

void CInstallerDlg::OnMouseMove(UINT nFlags, CPoint point)  {
  if( (nFlags&MK_LBUTTON) != 0 && mouse_down )  {
    RECT pos;
    GetWindowRect(&pos);
    int x_inc = point.x - mouse_pos.x;
    int y_inc = point.y - mouse_pos.y;
    SetWindowPos(&wndTopMost, pos.left + x_inc, pos.top + y_inc, 0, 0, SWP_NOSIZE|SWP_NOZORDER); 
  }
  CDialog::OnMouseMove(nFlags, point);
}

void CInstallerDlg::OnBnClickedBtnClose()  {
  EndDialog(IDCANCEL);
}

void CInstallerDlg::OnBnClickedCbProxy()  {
  wnd::set_enabled(this, IDC_TE_PROXY, check_box::is_checked(this, IDC_CB_PROXY));
}

void CInstallerDlg::OnShowWindow(BOOL bShow, UINT nStatus)  {
  InitRepositories();
  CDialog::OnShowWindow(bShow, nStatus);
  SetActiveWindow();
}

void CInstallerDlg::OnTimer(UINT_PTR nIDEvent)  {
  CDialog::OnTimer(nIDEvent);
}

void CInstallerDlg::OnBnClickedBtnProxy()  {
  InitRepositories();
}



void CInstallerDlg::OnBnClickedInstall()  {
  if( action == actionInstall )  {
    if( DoInstall() )
      SetAction(actionRun);
    else
      SetAction(actionInstall);
  }
  else if( action == actionReinstall )  {
    if( DoUninstall() )  {
      if( action == actionExit )
        EndDialog(IDOK);
      else  {
        if( DoInstall() )
          SetAction(actionRun);
        else
          SetAction(actionInstall);
      }
    }
    else
      SetAction(actionReinstall);
  }
  else
    DoRun();
}

void CInstallerDlg::DisableInterface(bool _v)  {
  const bool v = !_v;
  GetDlgItem(IDC_BTN_PROXY)->EnableWindow(v);
  GetDlgItem(IDC_BTN_REPOSITORY)->EnableWindow(v);
  GetDlgItem(IDC_BTN_CHOOSE_PATH)->EnableWindow(v);
  GetDlgItem(IDC_TE_INSTALL_PATH)->EnableWindow(v);
  if( v )
    GetDlgItem(IDC_TE_PROXY)->EnableWindow( check_box::is_checked(this, IDC_CB_PROXY) );
  else
    GetDlgItem(IDC_TE_PROXY)->EnableWindow(v);
  GetDlgItem(IDC_CB_PROXY)->EnableWindow(v);
  GetDlgItem(IDC_R_ALWAYS)->EnableWindow(v);
  GetDlgItem(IDC_R_DAILY)->EnableWindow(v);
  GetDlgItem(IDC_R_MONTHLY)->EnableWindow(v);
  GetDlgItem(IDC_R_NEVER)->EnableWindow(v);
  GetDlgItem(IDC_CB_SHORTCUT)->EnableWindow(v);
  GetDlgItem(IDC_CB_DESKTOP)->EnableWindow(v);
  GetDlgItem(IDC_CB_REPOSITORY)->EnableWindow(v);
  GetDlgItem(IDC_BTN_CLOSE)->EnableWindow(v);
  GetDlgItem(IDC_BTN_INSTALL)->EnableWindow(v);
}


olxstr CInstallerDlg::LocateBaseDir()  {
  CRegKey rc;
  olex2_installed = false;
  olex2_installed_path = EmptyString;
  olxstr app_name("olex2-");
  app_name << olex2_install_tag << ".exe";
  try  {
    if( rc.Open(HKEY_CLASSES_ROOT, (olxstr("Applications\\") << app_name << "\\shell\\open\\command").u_str(), KEY_READ) == ERROR_SUCCESS )  {
      olxch rv[MAX_PATH];
      ULONG sz_rv = MAX_PATH;
      if( rc.QueryStringValue(_T(""), rv, &sz_rv) == ERROR_SUCCESS )  {
        olex2_installed_path = rv;
        olex2_installed_path = TEFile::ExtractFilePath(olex2_installed_path.Trim('"'));
        rc.Close();
      }
    }
  }
  catch( ... )  {    }
  if( !TEFile::Exists(olex2_installed_path) )
    olex2_installed_path = EmptyString;
  else
    olex2_installed = true;
  return olex2_installed_path.IsEmpty() ? olxstr(GetCommandLine()) : olex2_installed_path;
}

void CInstallerDlg::SetAction(int a)  {
  action = a;
  if( a == actionInstall || a == actionReinstall )  {
    DisableInterface(false);

    GetDlgItem(IDC_BTN_INSTALL)->EnableWindow(true);
    if( a == actionInstall )
      GetDlgItem(IDC_BTN_INSTALL)->SendMessage(WM_SETTEXT, 0, (LPARAM)_T("Install"));
    else
      GetDlgItem(IDC_BTN_INSTALL)->SendMessage(WM_SETTEXT, 0, (LPARAM)_T("Re-install"));
    GetDlgItem(IDC_PB_PROGRESS)->SendMessage(PBM_SETPOS, (WPARAM)0, 0);
    GetDlgItem(IDC_PB_PROGRESS)->ShowWindow(SW_SHOW);
  }
  else  {
    DisableInterface(true);
    GetDlgItem(IDC_PB_PROGRESS)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_BTN_INSTALL)->SendMessage(WM_SETTEXT, 0, (LPARAM)_T("Run!"));
    GetDlgItem(IDC_BTN_INSTALL)->EnableWindow(true);
    GetDlgItem(IDC_BTN_CLOSE)->EnableWindow(true);
  }
}

bool CInstallerDlg::DoRun()  {
  TEFile::ChangeDir(olex2_installed_path);
  return LaunchFile(olex2_installed_path + "olex2.exe", false, true);
}

void CInstallerDlg::ProcessMessages()  {
  MSG msg;
  if( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) )  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}

bool CInstallerDlg::LaunchFile(const olxstr &fileName, bool quiet, bool do_exit)  {
  STARTUPINFO si;
  PROCESS_INFORMATION ProcessInfo;
  olxstr Tmp(fileName);
  ZeroMemory(&si, sizeof(STARTUPINFO));
  si.cb = sizeof(STARTUPINFO);
  si.wShowWindow = SW_SHOW;
  si.dwFlags = STARTF_USESHOWWINDOW;
  olxch* cmdl = NULL;
  if( quiet )  {
    olxstr cmd = Tmp;
    cmd << " /q";
    cmdl = new olxch[cmd.Length() + 1];
    memcpy(cmdl, cmd.u_str(), cmd.Length()*sizeof(olxch));
    cmdl[cmd.Length()] = '\0';
  }
  const bool rv = CreateProcess(Tmp.u_str(), cmdl,
        NULL, NULL, true, 0, NULL, NULL,
        &si, &ProcessInfo) != 0;
  if( cmdl != NULL )
    delete [] cmdl;
  if( !rv )  {
    MessageBox((olxstr("Could not start ") << fileName).u_str(), _T("Error"), MB_OK|MB_ICONERROR);
    return false;
  }
  if( do_exit )
    EndDialog(IDOK);
  else  {
    DWORD rv;
    int res = false;
    while( ((res = GetExitCodeProcess(ProcessInfo.hProcess, &rv)) != FALSE) && (rv == STILL_ACTIVE) )  {
      ProcessMessages();
      SleepEx(100, false);
    }
    if( res != FALSE )  return rv == 0;
    return false;
  }
  return true;
}

bool CInstallerDlg::DoInstall()  {
  DisableInterface(true);
  GetDlgItem(IDC_PB_PROGRESS)->ShowWindow(SW_SHOW);
  olxstr reposPath, proxyPath, installPath;
  if( check_box::is_checked(this, IDC_CB_PROXY) )
    proxyPath = wnd::get_text(this, IDC_TE_PROXY);

  reposPath = TEFile::UnixPath(wnd::get_text(this, IDC_CB_REPOSITORY));

  bool localInstall = TEFile::IsAbsolutePath(reposPath);

  if( !localInstall )
    if( !reposPath.IsEmpty() && !reposPath.EndsWith('/') )
      reposPath << '/';

  installPath = TEFile::WinPath(wnd::get_text(this, IDC_TE_INSTALL_PATH));
  if( !installPath.IsEmpty() && !installPath.EndsWith('\\') )
    installPath << '\\';

  if( !TEFile::Exists(installPath) )  {
    if( !TEFile::MakeDirs(installPath) )  {
      MessageBox(_T("Could not create installation folder.\
 Please make sure the folder is not opened in any other programs and try again."), _T("Error"), MB_OK|MB_ICONERROR);
      GetDlgItem(IDC_BTN_INSTALL)->EnableWindow(TRUE);
      return false;
    }
  }
  else if( !TEFile::IsEmptyDir(installPath) ) {
    int res = MessageBox(_T("The instalaltion folder already exists.\nThe installer needs to empty it.\nContinue?"),
      _T("Confirm"), MB_YESNOCANCEL|MB_ICONWARNING);
    if( res == IDYES )  {
      if( !TEFile::DeleteDir(installPath, true) )  {
        MessageBox(_T("Could not clean up the instalaltion folder. Please try later."), _T("Error"), MB_OK|MB_ICONERROR);
        GetDlgItem(IDC_BTN_INSTALL)->EnableWindow(TRUE);
        return false;
      }
    }
    else if( res == IDCANCEL )  {
      GetDlgItem(IDC_BTN_INSTALL)->EnableWindow(TRUE);
      return false;
    }
  }

  try  {
    olxstr StartDir = TBasicApp::GetBaseDir();  // it will be changed after install!!
    if( !localInstall )  {
      TUrl url( reposPath );
      if( !proxyPath.IsEmpty() )
        url.SetProxy(proxyPath);
      THttpFileSystem repos(url);
      repos.OnProgress.Add( new TProgress );
      TEFile* zipf = repos.OpenFileAsFile(url.GetPath() + updater::UpdateAPI::GetInstallationFileName());
      if( zipf == NULL )  {
        SetAction(_T("Failed..."));
        MessageBox(
          (olxstr("Could not locate the Olex2 archive: ") <<
          updater::UpdateAPI::GetInstallationFileName() <<
          "\nPlease try another repository.").u_str(),
          _T("Zip file fetching error"),
          MB_OK|MB_ICONERROR);
          GetDlgItem(IDC_BTN_INSTALL)->EnableWindow(TRUE);
        return false;
      }
      olxstr zipName(zipf->GetName());
      zipf->SetTemporary(false);
      delete zipf;
      SetAction(_T("Done"));
      bool res = _DoInstall(zipName, installPath);
      TEFile::DelFile(zipName);
      if( !res )
        return false;
    }
    else  {
      if( !_DoInstall( wnd::get_text(this, IDC_CB_REPOSITORY), installPath) )
        return false;
    }
    updater::UpdateAPI api;
    if( localInstall )
      api.GetSettings().repository = "http://www.olex2.org/olex2-distro/";
    else
      api.GetSettings().repository = api.TrimTagPart(reposPath);

    api.GetSettings().proxy = proxyPath;
    if( check_box::is_checked(this, IDC_R_ALWAYS) )  api.GetSettings().update_interval = "Always";
    else if( check_box::is_checked(this, IDC_R_DAILY) )  api.GetSettings().update_interval = "Daily";
    else if( check_box::is_checked(this, IDC_R_MONTHLY) )  api.GetSettings().update_interval = "Monthly";
    else if( check_box::is_checked(this, IDC_R_NEVER) )  api.GetSettings().update_interval = "Never";
    api.GetSettings().Save();

    SetAction(_T("Done"));
    InitRegistry(installPath);
    // create shortcuts
    if( check_box::is_checked(this, IDC_CB_SHORTCUT) )
      TShellUtil::CreateShortcut(TShellUtil::GetSpecialFolderLocation(fiStartMenu) << 
                                 "Olex2-" << olex2_install_tag << ".lnk",
                                 installPath + "olex2.exe", "Olex2 launcher", run_as_admin);
    if( check_box::is_checked(this, IDC_CB_DESKTOP) )
      TShellUtil::CreateShortcut(TShellUtil::GetSpecialFolderLocation(fiDesktop) << 
                                 "Olex2-" << olex2_install_tag << ".lnk",
                                 installPath + "olex2.exe", "Olex2 launcher", run_as_admin);
    olex2_installed_path = installPath;
  }
  catch(const TExceptionBase& )  {
    MessageBox(_T("The installation has failed. If using online installation please check, that\
your computers is online."), _T("Installation failed"), MB_OK|MB_ICONERROR);
    return false;
  }
  return true;
}

bool CInstallerDlg::_DoInstall(const olxstr& zipFile, const olxstr& installPath)  {
  TBasicApp::SetBaseDir(TEFile::AddPathDelimeter(installPath) << "installer.exe");
  TWinZipFileSystem zfs(zipFile);
  bool res = zfs.Exists("olex2.tag");
  if( res )  {
    zfs.OnProgress.Add(new TProgress);
    TEFile* lic_f = NULL;
    try  {  lic_f = zfs.OpenFileAsFile("licence.rtf");  }
    catch(...)  {  res = false;  }
    if( lic_f == NULL )  res = false;
    if( res )  {
      olxstr lic_fn = lic_f->GetName();
      delete lic_f;
      CLicenceDlg dlg(this, lic_fn);
      if( dlg.DoModal() != IDOK )
        return false;
      try  {  zfs.ExtractAll(installPath);  }
      catch(...) {  res = false;  }
      if( res )  {
#ifndef _WIN64
        olxstr redist_fn = TBasicApp::GetBaseDir() + "vcredist_x86.exe", redist_tag = "x86";
#else
        olxstr redist_fn = TBasicApp::GetBaseDir() + "vcredist_x64.exe", redist_tag = "x64";
#endif
        SetAction(_T("Installing MSVCRT..."));
        if( !LaunchFile(redist_fn, true, false) )  {
          if( MessageBox(_T("Could not install MSVC redistributables."), _T("Installation failed"), MB_RETRYCANCEL|MB_ICONERROR) == IDRETRY )  {
            if( !LaunchFile(redist_fn, false, false) )  {
              TShellUtil::CreateShortcut(TShellUtil::GetSpecialFolderLocation(fiDesktop) <<
                    "MSVCRT-" << redist_tag << ".lnk",
                    redist_fn, "MSVCRT installer", run_as_admin);
              MessageBox(_T("Olex2 installation will now be completed, however the MSVCRT\n\
installation has failed and you will need to install it manually.\n\
The installer has create a shortcut for MSVCRT installer on your desktop."), _T("Warning"), MB_OK|MB_ICONERROR);
              SetAction(_T("MSVCRT installation is required..."));
              return true;
             }
          }
          else  {
            SetAction(_T("Failed to install MSVCRT..."));
            return false;
          }
        }
        TEFile::DelFile(redist_fn);
      }
    }
  }
  if( !res )
    MessageBox(_T("Invalid installation archive."), _T("Installation failed"), MB_OK|MB_ICONERROR);
  else
    updater::UpdateAPI::TagInstallationAsNew();
  return res;
}

bool CInstallerDlg::InitRegistry(const olxstr &installPath)  {
  CRegKey reg;
  bool res = false;
  olxstr app_name("olex2-");
  app_name << olex2_install_tag << ".exe";
  try  {
    if( (res = (reg.Create(HKEY_CLASSES_ROOT, (olxstr("Applications\\") << app_name << "\\shell\\open\\command").u_str()) == ERROR_SUCCESS)) )  {
      olxstr val('\"');
      val << installPath << "olex2.exe" << "\" '%1'";
      reg.SetKeyValue(_T(""), val.u_str());
      reg.Close();
    }
    if( res && (res = (reg.Create(HKEY_CLASSES_ROOT, (olxstr("Applications\\") << app_name << "\\SupportedTypes").u_str()) == ERROR_SUCCESS)) )  {
      for( size_t i=0; i < exts_sz; i++ )
        reg.SetStringValue(exts[i].u_str(), _T(""));
      reg.Close();
    }
    for( size_t i=0; i < exts_sz; i++ )  {
      if( !res )  break;
      res = (reg.Create(HKEY_CLASSES_ROOT, (olxstr(exts[i]) << "\\OpenWithList\\" << app_name).u_str()) == ERROR_SUCCESS);
      reg.Close();
    }
  }
  catch( ... )  {    }
  return res;
}

bool CInstallerDlg::CleanRegistry()  {
  CRegKey reg;
  bool res = false;
  olxstr app_name("olex2-");
  app_name << olex2_install_tag << ".exe";
  try  {
    res = (reg.Open(HKEY_CLASSES_ROOT, NULL) == ERROR_SUCCESS);
    if( res )
      res = (reg.RecurseDeleteKey( (olxstr("Applications\\") << app_name).u_str()) == ERROR_SUCCESS);
    for( size_t i=0; i < exts_sz; i++ )  {
      if( !res )  break;
      res = (reg.DeleteSubKey((olxstr(exts[i])<< "\\OpenWithList\\" << app_name).u_str()) == ERROR_SUCCESS);
      // the old key, have to take care of as well...
      reg.DeleteSubKey((olxstr(exts[i])<< "\\OpenWithList\\olex2.dll").u_str());
    }
  }
  catch( ... )  {    }
  return res;
}

bool CInstallerDlg::CleanRegistryAndShortcuts(bool sc)  {
  if( !CleanRegistry() )  {
    MessageBox(_T("Could not remove registry entries"), _T("Error"), MB_OK|MB_ICONERROR);
    return false;
  }
  if( sc )  {
    // find and delete shortcuts
    try  {
      TEFile::DelFile(TShellUtil::GetSpecialFolderLocation(fiStartMenu) << "Olex2-" << olex2_install_tag <<".lnk");
      TEFile::DelFile(TShellUtil::GetSpecialFolderLocation(fiDesktop) <<  "Olex2-" << olex2_install_tag << ".lnk");
    }
    catch(const TExceptionBase&)  {
      MessageBox(_T("Could not remove shortcuts"), _T("Error"), MB_OK|MB_ICONERROR);
      return false;
    }
  }
  return true;
}


bool CInstallerDlg::DoUninstall()  {
  if( patcher::PatchAPI::IsOlex2Running() )  {
    MessageBox(_T("There are Olex2 instances are running or you do\
 not have sufficient rights to access the the installation folder..."), _T("Error"), MB_OK|MB_ICONERROR);
    return false;
  }
  CReinstallDlg dlg(this);
  if( dlg.DoModal() != IDOK )  return false;
  action = dlg.IsInstall() ? actionInstall : actionExit;
  BeginWaitCursor();
  if( !TEFile::DeleteDir(olex2_installed_path) )  {
    EndWaitCursor();
    MessageBox(_T("Could not remove Olex2 installation folder..."), _T("Error"), MB_OK|MB_ICONERROR);
    return false;
  }
  EndWaitCursor();
  if( dlg.IsRemoveUserData() )  {
    if( TEFile::Exists(olex2_data_dir) )
      TEFile::DeleteDir(olex2_data_dir);
  }
  return CleanRegistryAndShortcuts(true);
}

void CInstallerDlg::InitRepositories()  {
  UpdaterTh uth;
  uth.Start();
  try  {
    combo_box::clear_items(this, IDC_CB_REPOSITORY);
    updater::UpdateAPI api;
    if( check_box::is_checked(this, IDC_CB_PROXY) )
      api.GetSettings().proxy = check_box::get_text(this, IDC_TE_PROXY);
    TStrList repos;
    api.GetAvailableRepositories(repos);
    if( repos.IsEmpty() )  
      throw 1; // ust get to the catch...
    combo_box::add_items(this, IDC_CB_REPOSITORY, repos);
    combo_box::sel_item(this, IDC_CB_REPOSITORY, repos[0]);
  }
  catch(...)  {
    uth.Join(true);
    MessageBox(_T("Could not discover any Olex2 repositories, only offline installation will be available"),
      _T("Error"),
      MB_OK|MB_ICONERROR);
  }
  if( uth.IsRunning() )
    uth.Join(true);
  olxstr bd(GetCommandLine());
  olxstr zipfn( TEFile::ExtractFilePath(bd) + updater::UpdateAPI::GetInstallationFileName() );
  if( TEFile::Exists(zipfn) )  {
    if( !TEFile::IsAbsolutePath(zipfn) )  {
      zipfn = TEFile::CurrentDir();
      TEFile::AddPathDelimeterI(zipfn) << updater::UpdateAPI::GetInstallationFileName();
    }
    combo_box::add_item(this, IDC_CB_REPOSITORY, zipfn);
    combo_box::sel_item(this, IDC_CB_REPOSITORY, zipfn);
  }
  olex2_install_tag = ReadTag(wnd::get_text(this, IDC_CB_REPOSITORY));
  SetInstallationPath(olxstr(olex2_install_path) << '-'  << olex2_install_tag);
}


BOOL CInstallerDlg::PreTranslateMessage(MSG* pMsg)  {
  if( tooltipCtrl != NULL )
    tooltipCtrl->RelayEvent(pMsg);
  return CDialog::PreTranslateMessage(pMsg);
}

void CInstallerDlg::OnBnClickedBtnRepository()  {
  CFileDialog fd(TRUE, NULL, updater::UpdateAPI::GetInstallationFileName().u_str(), OFN_FILEMUSTEXIST, NULL, this);
  if( fd.DoModal() == IDOK )  {
    olxstr rv = fd.GetPathName().GetString();
    if( TEFile::ExtractFileName(rv) == updater::UpdateAPI::GetInstallationFileName() )  {
      olxstr new_tag = ReadTag(rv);
      if( !new_tag.IsEmpty() )  {
        combo_box::add_item(this, IDC_CB_REPOSITORY, rv);
        combo_box::sel_item(this, IDC_CB_REPOSITORY, rv);
        wnd::set_text(this, IDC_TE_INSTALL_PATH, olxstr(olex2_install_path) << '-'  << new_tag);
        olex2_install_tag = new_tag;
      }
      else
        MessageBox(_T("Invalid installation file"), _T("Error"), MB_OK|MB_ICONERROR);
    }
  }
}

BOOL CInstallerDlg::OnEraseBkgnd(CDC* pDC){
  CBrush* pOldBrush = pDC->SelectObject(ctrlBrush);
  CRect rect;
  pDC->GetClipBox(&rect);
  pDC->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), PATCOPY);
  pDC->SelectObject(pOldBrush);
  return TRUE;
}

HBRUSH CInstallerDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)  {
  pDC->SetBkColor(bgColor);
  return (HBRUSH)ctrlBrush->GetSafeHandle();
}

void CInstallerDlg::SetInstallationPath(const olxstr& path)  {
  wnd::set_text(this, IDC_TE_INSTALL_PATH, path);
  LocateBaseDir();
  if( olex2_installed )  {
    olex2_data_dir = PatchAPI::ComposeNewSharedDir(TShellUtil::GetSpecialFolderLocation(fiAppData), olex2_installed_path);
    SetAction(actionReinstall);
  }
  else
    SetAction(actionInstall);
}

olxstr CInstallerDlg::ReadTag(const olxstr& repo) const {
  if( TEFile::Exists(repo) )  {
    try  {
      TWinZipFileSystem zfs(repo);
      IInputStream* is = zfs.OpenFile("olex2.tag");
      if( is == NULL )  return EmptyString;
      TStrList sl;
      sl.LoadFromTextStream(*is);
      delete is;
      if( sl.IsEmpty() )  return EmptyString;
      return sl[0];
    }
    catch(...)  {  return EmptyString;  }
  }
  else  {
    size_t i = repo.LastIndexOf('/');
    if( i == InvalidIndex )  return EmptyString;
    return repo.SubStringFrom(i+1);
  }
}

void CInstallerDlg::OnCbnSelendokCbRepository()  {
  olex2_install_tag = ReadTag(wnd::get_text(this, IDC_CB_REPOSITORY));
  SetInstallationPath(olxstr(olex2_install_path) << '-' << olex2_install_tag);
}
