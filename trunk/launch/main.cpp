//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "main.h"
#include "splash.h"
#include "actions.h"

#include "log.h"
#include "exception.h"

#include "filetree.h"
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
enum
{
  ID_Progress
};

const char* Updates[] = {"Always", "Daily", "Weekly", "Monthly", "Never"};
const short uAlways  = 0,
            uDaily   = 1,
            uWeekly  = 2,
            uMonthly = 3,
            uNever   = 4,
            uUnknown = -1;

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
  if( !UpdateInstallation() )  {
    Application->MessageBox("Please make sure that no Olex2 instances are running,\n\
you have enough right to modify the installation folder and\n\
no oOlex2 folders are opened in browsers",
      "Update failed", MB_OK|MB_ICONINFORMATION);
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
void AfterFileCopy(const olxstr& src, const olxstr& dest)  {
  if( !TEFile::DelFile(src) )
    throw TFunctionFailedException(__OlxSourceInfo, olxstr("Failed to delete file: ") << src);
}
bool TdlgMain::UpdateInstallation()  {
  TBasicApp& bapp = *TBasicApp::GetInstance();
  
  olxstr patch_dir(bapp.BaseDir() + "patch/");
  olxstr download_vf(bapp.BaseDir() + "__completed.update");
  olxstr cmd_file(bapp.BaseDir() + "__cmds.update");
  // make sure that only one instance is running
  olxstr pid_file(bapp.BaseDir() + "updater.pid_");
  if( TEFile::Exists(pid_file) )  {
    if( !TEFile::DelFile(pid_file) )
      return false;
  }
  if( !TEFile::Exists(patch_dir) || !TEFile::Exists(download_vf) )
    return true;
  // create lock if possible or exit
  TEFile* pid_file_inst = NULL;
  try  {
    pid_file_inst = new TEFile(pid_file, "w+b");
  }
  catch( TExceptionBase& )  {  return false;  }
  // check that there are no olex2 instances...
  TStrList pid_files;
  TEFile::ListDir(bapp.BaseDir(), pid_files, "*.olex2_pid", sefAll);
  for( int i=0; i < pid_files.Count(); i++ )  {
    if( TEFile::DelFile( bapp.BaseDir() + pid_files[i]) )
      pid_files[i].SetLength(0);
  }
  pid_files.Pack();
  if( !pid_files.IsEmpty() )
    return false;
  // clean
  bool res = true;
  if( TEFile::Exists(cmd_file) )  {
    TWStrList _cmds;
    TUtf8File::ReadLines(cmd_file, _cmds);
    TStrList cmds(_cmds);
    for( int i=0; i < cmds.Count(); i++ )  {
      if( cmds[i].StartsFrom("rm ") )  {
        olxstr fdn = cmds[i].SubStringFrom(3).Trim('\'');
        if( !TEFile::Exists(fdn) )  {
          cmds[i].SetLength(0);
          continue;
        }
        if( TEFile::IsDir(fdn) )  {
          if( !TEFile::DeleteDir(fdn) )  {
            res = false;
            break;  // next time then...
          }
        }
        else  {
          if( !TEFile::DelFile(fdn) )  {
            res = false;
            break; // next time then...
          }
        }
        cmds[i].SetLength(0);
      }
    }
    if( res )
      TEFile::DelFile(cmd_file);
    else  {
      cmds.Pack();
      TUtf8File::WriteLines(cmd_file, cmds, true);
    }
  }
  // copy...
  if( res )  {
    try  {
      TFileTree ft(patch_dir);
      ft.Root.Expand();
      TOnProgress OnFileCopy, OnSync;
      ft.OnFileCopy->Add( new TFileProgress );
      ft.OnSynchronise->Add( new TOverallProgress );
      ft.CopyTo(bapp.BaseDir(), &AfterFileCopy, &OnFileCopy, &OnSync);
      TEFile::DeleteDir(patch_dir);
      TEFile::DelFile(download_vf);
    }
    catch(...)  {  res = false;  }
  }
  pid_file_inst->Delete();
  delete pid_file_inst;
  return res;
}
//---------------------------------------------------------------------------

void __fastcall TdlgMain::tTimerTimer(TObject *Sender)
{
  // force the GC to free temporray obejcts
  FBApp->OnIdle->Execute(NULL, NULL);
}
//---------------------------------------------------------------------------

