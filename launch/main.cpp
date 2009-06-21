//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "main.h"
#include "splash.h"
#include "patchapi.h"

#include "log.h"

#include "filetree.h"
#include "shellutil.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TdlgMain *dlgMain;

class TFileProgress: public AActionHandler {
public:
  TFileProgress(){}
  ~TFileProgress(){}
  bool Exit(const IEObject *Sender, const IEObject *Data)  {  return true;  }
  bool Enter(const IEObject *Sender, const IEObject *Data)  {
    if( !EsdlInstanceOf(*Data, TOnProgress) )  return false;
    const TOnProgress *A = dynamic_cast<const TOnProgress*>(Data);
    dlgSplash->stFileName->Caption = TEFile::ExtractFileName(A->GetAction()).c_str();
    dlgSplash->pbFProgress->Max = A->GetMax();
    return true;
  }
  bool Execute(const IEObject *Sender, const IEObject *Data)  {
    if( !EsdlInstanceOf(*Data, TOnProgress) )  return false;
    const TOnProgress *A = dynamic_cast<const TOnProgress*>(Data);
    dlgSplash->pbFProgress->Position = A->GetPos();
    Application->ProcessMessages();
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
    dlgSplash->pbOProgress->Max = A->GetMax();
    return true;
  }
  bool Execute(const IEObject *Sender, const IEObject *Data)  {
    if( !EsdlInstanceOf(*Data, TOnProgress) )  return false;
    const TOnProgress *A = dynamic_cast<const TOnProgress*>(Data);
    dlgSplash->pbOProgress->Position = A->GetPos();
    Application->ProcessMessages();
    return true;
  }
};
//---------------------------------------------------------------------------
__fastcall TdlgMain::TdlgMain(TComponent* Owner)
  : TForm(Owner)
{
  olxstr BaseDir;
  char* olex_dir = getenv("OLEX2_DIR");
  if( olex_dir != NULL )
    BaseDir = olxstr(olex_dir) << "/dummy.txt";
  else
    BaseDir = ParamStr(0).c_str();
  if( BaseDir.Length() > 0 && BaseDir[1] != ':' )
    BaseDir = TEFile::CurrentDir()+'\\';
  FBApp = new TBasicApp(BaseDir);
  FBApp->SetSharedDir( TShellUtil::GetSpecialFolderLocation(fiAppData) + "Olex2u");
  dlgSplash = new TdlgSplash(this);

  olxstr vfn = (TBasicApp::GetInstance()->BaseDir()+ "version.txt");
  // check updates ...
  //asm {  int 3  }
  // reading version info
  olxstr OlexFN( (TBasicApp::GetInstance()->BaseDir()+ "olex2.dll") );
  if( TEFile::Exists(OlexFN) )  {
    DWORD len = GetFileVersionInfoSize( const_cast<char*>(OlexFN.c_str()), &len);
    if( len > 0 )  {
      AnsiString Tmp;
      char *pBuf = (char*)malloc(len+1),
        *pValue[1];
      UINT pLen;
      GetFileVersionInfo(const_cast<char*>(OlexFN.c_str()), 0, len, pBuf);
      Tmp = "StringFileInfo\\080904E4\\FileVersion";
      if( VerQueryValue(pBuf, Tmp.c_str(), (void**)&pValue[0], &pLen) )  {
        Tmp = "Version: ";
        Tmp += pValue[0];
        if( TEFile::Exists(vfn) )  {
          try  {
            TEFile vf(vfn, "rb");
            TStrList sl;
            sl.LoadFromTextStream( vf );
            if( sl.Count() >= 1 )
              Tmp += '-';
              Tmp += sl[0].c_str();
          }
          catch( const TIOExceptionBase& exc )  {
            ;
          }
        }
        dlgSplash->stVersion->Caption = Tmp;
      }
      free(pBuf);
    } 
  }
  dlgSplash->Show();
  dlgSplash->Repaint();
  olxstr checkFN(FBApp->BaseDir()+"index.ind");
  if( TEFile::Exists(checkFN) && !SetFileAttributes(checkFN.c_str(), FILE_ATTRIBUTE_SYSTEM) )  {
    Application->MessageBox("Please make sure that you have enough right to modify the installation folder",
      "Scheduled update failed", MB_OK|MB_ICONINFORMATION);
    Application->Terminate();
    return;
  }
  else
    SetFileAttributes(checkFN.c_str(), FILE_ATTRIBUTE_NORMAL);
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
    Application->MessageBox(msg.c_str(), "Update failed", MB_OK|MB_ICONINFORMATION);
  }
  // cheating :D
  dlgSplash->pbOProgress->Position = dlgSplash->pbOProgress->Max;
  dlgSplash->pbFProgress->Position = dlgSplash->pbFProgress->Max;
  Launch();
  dlgSplash->stFileName->Caption = "Done. Launching Olex2";
  dlgSplash->Repaint();
  SleepEx(4000, FALSE);
  Application->Terminate();
}
//---------------------------------------------------------------------------
__fastcall TdlgMain::~TdlgMain()
{
  FBApp->OnIdle->Execute(NULL, NULL);
  delete FBApp;
  FBApp = NULL;
  delete dlgSplash;
}
//---------------------------------------------------------------------------
void TdlgMain::Launch()  {
  // modify th epath to set the basedir to the basedir, so that correct python25.dll is loaded
  char* bf = new char [1024];
  bf[0] = '\0';
  int rv = GetEnvironmentVariable("PATH", bf, 1024);
  if( rv > 1024 )  {
    delete [] bf;
    bf = new char [rv+1];
    rv = GetEnvironmentVariable("PATH", bf, rv+1);
  }
  olxstr path(bf), bd = TBasicApp::GetInstance()->BaseDir();
  delete [] bf;
  path.Insert(bd.SubStringTo(bd.Length()-1) + ';', 0);
  SetEnvironmentVariable("PATH", path.c_str());
  olxstr py_path = TBasicApp::GetInstance()->BaseDir() + "Python26";
  SetEnvironmentVariable("PYTHONHOME", py_path.c_str());

  STARTUPINFO si;
  PROCESS_INFORMATION ProcessInfo;
  AnsiString Tmp;
  Tmp += TBasicApp::GetInstance()->BaseDir().u_str();
  Tmp += "olex2.dll";
  ZeroMemory(&si, sizeof(STARTUPINFO));
  si.cb = sizeof(STARTUPINFO);
  si.wShowWindow = SW_SHOW;
  si.dwFlags = STARTF_USESHOWWINDOW;

  // Launch the child process.
  if( !CreateProcess(
        Tmp.c_str(),
        NULL,
        NULL, NULL,   true,
        0, NULL,
        NULL,
        &si, &ProcessInfo))
  {
    Application->MessageBox("Could not start OLEX2.DLL", "Error", MB_OK|MB_ICONERROR);
    // unlucky...
  }
}
//---------------------------------------------------------------------------
void __fastcall TdlgMain::tTimerTimer(TObject *Sender)
{
  // force the GC to free temporray obejcts
  FBApp->OnIdle->Execute(NULL, NULL);
}
//---------------------------------------------------------------------------

