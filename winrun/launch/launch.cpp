#include "launch.h"
#include "launchDlg.h"

#include "egc.h"
#include "efile.h"
#include "patchapi.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


BEGIN_MESSAGE_MAP(LaunchApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

MainDlg* dlgSplash;


class TFileProgress: public AActionHandler {
public:
  TFileProgress(){}
  ~TFileProgress(){}
  bool Exit(const IEObject *Sender, const IEObject *Data)  {  return true;  }
  bool Enter(const IEObject *Sender, const IEObject *Data)  {
    if( !EsdlInstanceOf(*Data, TOnProgress) )  return false;
    const TOnProgress *A = dynamic_cast<const TOnProgress*>(Data);
    dlgSplash->SetFileName(TEFile::ExtractFileName(A->GetAction()));
    dlgSplash->SetFileProgressMax(A->GetMax()/1024);
    return true;
  }
  bool Execute(const IEObject *Sender, const IEObject *Data)  {
    if( !EsdlInstanceOf(*Data, TOnProgress) )  return false;
    const TOnProgress *A = dynamic_cast<const TOnProgress*>(Data);
    dlgSplash->SetFileProgress(A->GetPos()/1024);
    dlgSplash->UpdateWindow();
    return true;
  }
};
class TOverallProgress: public AActionHandler {
public:
  TOverallProgress(){}
  ~TOverallProgress(){}
  bool Exit(const IEObject *Sender, const IEObject *Data)  {  return true;  }
  bool Enter(const IEObject *Sender, const IEObject *Data)  {
    if( !EsdlInstanceOf(*Data, TOnProgress) )  return false;
    const TOnProgress *A = dynamic_cast<const TOnProgress*>(Data);
    // have to scale it doen to Mb's since mak value supported by the ProgressBar is int16
    dlgSplash->SetOverallProgressMax(A->GetMax()/(1024*1024));
    return true;
  }
  bool Execute(const IEObject *Sender, const IEObject *Data)  {
    if( !EsdlInstanceOf(*Data, TOnProgress) )  return false;
    const TOnProgress *A = dynamic_cast<const TOnProgress*>(Data);
    dlgSplash->SetOverallProgress(A->GetPos()/(1024*1024));
    //Application->ProcessMessages();
    return true;
  }
};

LaunchApp::LaunchApp() : Bapp(TBasicApp::GuessBaseDir(GetCommandLine(), NULL)) {
  launch_successful = true;
}


// The one and only LaunchApp object

LaunchApp theApp;


// LaunchApp initialization

BOOL LaunchApp::InitInstance()  {
  // InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Olex2 launcher"));
	MainDlg dlg;
	m_pMainWnd = &dlg;
  dlgSplash = &dlg;
  
  dlg.Create(IDD_SPLASH);
  dlg.ShowWindow(SW_SHOW);
  dlg.SetFileProgressMax(100);
  dlg.SetOverallProgressMax(100);
  olxstr BaseDir;
  TEGC::Initialise();
  olxstr vfn = (TBasicApp::GetBaseDir()+ "version.txt");
  olxstr tfn = (TBasicApp::GetBaseDir()+ patcher::PatchAPI::GetTagFileName());
  olxstr OlexFN( (TBasicApp::GetBaseDir()+ "olex2.dll") );
  if( TEFile::Exists(OlexFN) )  {
    DWORD len = GetFileVersionInfoSize(OlexFN.u_str(), &len);
    if( len > 0 )  {
      olxch *pBuf = (olxch*)malloc(len+1),
        *pValue[1];
      UINT pLen;
      GetFileVersionInfo(OlexFN.u_str(), 0, len, pBuf);
      if( VerQueryValue(pBuf, _T("StringFileInfo\\080904E4\\ProductVersion"), (void**)&pValue[0], &pLen) )  {
        olxstr Tmp = "Version: ";
        Tmp << pValue[0];
        try  {
          TStrList sl;
          if( TEFile::Exists(tfn) )  {
            sl.LoadFromFile( tfn );
            if( sl.Count() > 0 )  {
              Tmp << '-';
              Tmp << sl[0].c_str();
            }
          }
          if( TEFile::Exists(vfn) )  {
            sl.LoadFromFile( vfn );
            if( sl.Count() > 0 )  {
              Tmp << '-';
              Tmp << sl[0].c_str();
            }
          }
        }
        catch(const TIOExceptionBase&)  {}
        dlg.SetVersion(Tmp);
      }
      free(pBuf);
    } 
  }
  if( TBasicApp::GetInstance().IsBaseDirWriteable() )  {
    short res = patcher::PatchAPI::DoPatch(new TFileProgress, new TOverallProgress);
    if( res != patcher::papi_OK )  {
      olxstr msg;
      if( res == patcher::papi_Busy )
        msg = "Another update or Olex2 instance are running at the moment";
      else if( res == patcher::papi_CopyError || res == patcher::papi_DeleteError )  {
        msg = "Please make sure that no Olex2 instances are running,\n\
you have enough right to modify the installation folder and\n\
no Olex2 folders are opened in browsers";
     }
      MessageBox(NULL, msg.u_str(), _T("Update failed"), MB_OK|MB_ICONINFORMATION);
    }
  }
  // cheating :D
  dlgSplash->SetFileProgress(-1);
  dlgSplash->SetOverallProgress(-1);
  Launch();
  dlgSplash->SetFileName("Done. Launching Olex2");
  HWND olx2_wnd = FindWindow(NULL, _T("Olex2"));
  if( olx2_wnd != NULL )  {  // cannot do much here...
    int step = 0;
    while( ++step <= 100 )  {
      MSG msg;
      if( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) )  {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
      SleepEx(40, TRUE);
      if( !launch_successful )  break;
    }
  }
  else  {
    while( olx2_wnd == NULL || !IsWindowVisible(olx2_wnd) )  {
      if( olx2_wnd == NULL )
        olx2_wnd = FindWindow(NULL, _T("Olex2"));
      MSG msg;
      if( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) )  {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
      SleepEx(40, TRUE);
      if( !launch_successful )  break;
    }
  }
  dlg.ShowWindow(SW_HIDE);
	return FALSE;
}
void LaunchApp::Launch()  {
  // modify th epath to set the basedir to the basedir, so that correct python25.dll is loaded
  olxch* bf = new olxch[1024];
  bf[0] = '\0';
  int rv = GetEnvironmentVariable(_T("PATH"), bf, 1024);
  if( rv > 1024 )  {
    delete [] bf;
    bf = new olxch[rv+1];
    rv = GetEnvironmentVariable(_T("PATH"), bf, rv+1);
  }
  olxstr path(bf), bd = TBasicApp::GetBaseDir();
  delete [] bf;
  path.Insert(bd.SubStringTo(bd.Length()-1) + ';', 0);
  SetEnvironmentVariable(_T("PATH"), path.u_str());
  olxstr py_path = TBasicApp::GetBaseDir() + "Python26";
  SetEnvironmentVariable(_T("PYTHONHOME"), py_path.u_str());
  // remove all OLEX2_DATADIR and OLEX2_DIR variables
  SetEnvironmentVariable(_T("OLEX2_DIR"), NULL);
  //SetEnvironmentVariable("OLEX2_DATADIR", NULL);

  STARTUPINFO si;
  PROCESS_INFORMATION ProcessInfo;
  olxstr Tmp = TBasicApp::GetBaseDir();
  Tmp << "olex2.dll";
  ZeroMemory(&si, sizeof(STARTUPINFO));
  si.cb = sizeof(STARTUPINFO);
  si.wShowWindow = SW_SHOW;
  si.dwFlags = STARTF_USESHOWWINDOW;

  // Launch the child process.
  if( !CreateProcess(
        Tmp.u_str(),
        NULL,
        NULL, NULL,   true,
        0, NULL,
        NULL,
        &si, &ProcessInfo))
  {
    MessageBox(NULL, _T("Could not start OLEX2.DLL"), _T("Error"), MB_OK|MB_ICONERROR);
    launch_successful = false;
  }
  else
    launch_successful = true;
}
