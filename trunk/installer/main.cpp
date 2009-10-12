//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <FileCtrl.hpp>
#include <Registry.hpp>

#include "main.h"

#include "licence.h"

#include "efile.h"
#include "httpfs.h"
#include "winzipfs.h"
#include "settingsfile.h"
#include "shellutil.h"
#include "estrlist.h"
#include "updateapi.h"
#include "patchapi.h"
#include "egc.h"
#include "uninstall.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "frame1"
#pragma resource "*.dfm"

TfMain *fMain;

class TProgress: public AActionHandler  {
public:
  TProgress(){}
  virtual ~TProgress(){}
  bool Exit(const IEObject *Sender, const IEObject *Data)  {  return true;  }
  bool Enter(const IEObject *Sender, const IEObject *Data)  {
    if( !EsdlInstanceOf( *Data, TOnProgress) )  return false;
    const TOnProgress *A = dynamic_cast<const TOnProgress*>(Data);
    fMain->pbProgress->Max = A->GetMax();
    return true;
  }
  bool Execute(const IEObject *Sender, const IEObject *Data)  {
    if( !EsdlInstanceOf( *Data, TOnProgress) )  return false;
    const TOnProgress *A = dynamic_cast<const TOnProgress*>(Data);
    fMain->pbProgress->Position = A->GetPos();
    if( Sender != NULL && EsdlInstanceOf(*Sender, THttpFileSystem) )
      fMain->frMain->stAction->Caption = (olxstr("Downloading: ") << TEFile::ExtractFileName(A->GetAction().c_str())).c_str();
    else
      fMain->frMain->stAction->Caption = TEFile::ExtractFileName(A->GetAction()).c_str();
    Application->ProcessMessages();
    return true;
  }
};

olxstr TfMain::SettingsFile = "usettings.dat";

//---------------------------------------------------------------------------
__fastcall TfMain::TfMain(TComponent* Owner) :TForm(Owner)  {
  dlgUninstall = new TdlgUninstall(this);
  MouseDown = Dragging = false;
  Expanded = false;
  rename_status = 0;
  // init admin mode for shortcuts if required
  OSVERSIONINFO veri;
  memset(&veri, 0, sizeof(veri));
  veri.dwOSVersionInfoSize = sizeof(veri);
  GetVersionEx(&veri);
  // only after XP
  SetRunAs = veri.dwMajorVersion > 5;

  TEGC::Initialise();
  olxstr ip( TShellUtil::GetSpecialFolderLocation(fiProgramFiles) );
  TEFile::AddTrailingBackslashI( ip ) << "Olex2";
  frMain->eInstallationPath->Text = ip.c_str();
  OlexInstalled = CheckOlexInstalled( OlexInstalledPath );
  if( OlexInstalled )  {
    Bapp = new TBasicApp( TEFile::AddTrailingBackslash(OlexInstalledPath) );
    Bapp->OnProgress->Add( new TProgress );
    TEFile::AddTrailingBackslash( OlexInstalledPath );
    frMain->eInstallationPath->Text = OlexInstalledPath.c_str();
    olxstr sfile = OlexInstalledPath;
           sfile << TfMain::SettingsFile;
    if( TEFile::Exists( sfile ) )  {
      const TSettingsFile Settings( sfile );
      frMain->cbRepository->Text = Settings["repository"].c_str();
      frMain->eProxy->Text = Settings["proxy"].c_str();
      int updateInd = frMain->rgAutoUpdate->Items->IndexOfName( Settings["update"].c_str() );
      if( updateInd != -1 )
        frMain->rgAutoUpdate->ItemIndex = updateInd;
    }
    OlexDataDir = patcher::PatchAPI::ComposeNewSharedDir(TShellUtil::GetSpecialFolderLocation(fiAppData), OlexInstalledPath);
    OlexTag = patcher::PatchAPI::ReadRepositoryTag();
    SetAction(actionReinstall);
  }
  else
    Bapp = new TBasicApp( TBasicApp::GuessBaseDir(CmdLine) );
  frMain->bbInstall->Default = true;
}
//---------------------------------------------------------------------------
__fastcall TfMain::~TfMain()  {
  delete dlgUninstall; 
  Bapp->OnIdle->Execute(NULL, NULL);
  delete Bapp;
}
//---------------------------------------------------------------------------
void __fastcall TfMain::iSplashMouseMove(TObject *Sender,
      TShiftState Shift, int X, int Y)
{
  if( !MouseDown )  return;
  if( !Dragging )  {
    if( abs(X-MouseDownX) >= 3 || abs(Y-MouseDownY) >= 3 )
      Dragging = true;
  }
  if( !Dragging )  return;
  Left = Left + (X-MouseDownX);
  Top = Top + (Y-MouseDownY);
}
//---------------------------------------------------------------------------
void __fastcall TfMain::iSplashMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
  MouseDownX = X;
  MouseDownY = Y;
  MouseDown = true;
}
//---------------------------------------------------------------------------
void __fastcall TfMain::iSplashMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
  MouseDown = Dragging = false;
}
//---------------------------------------------------------------------------
void __fastcall TfMain::bbBrowseClick(TObject *Sender)
{
  this->Enabled = false;
  try  {
    olxstr dir( TShellUtil::PickFolder("Please select installation folder",
      TShellUtil::GetSpecialFolderLocation(fiPrograms), EmptyString) );
    if( dir.Length() != 0 )
      frMain->eInstallationPath->Text = dir.c_str();
    dir = TShellUtil::GetSpecialFolderLocation(fiProgramFiles);
  }
  catch( ... )  {  ;  }
  this->Enabled = true;
}
//---------------------------------------------------------------------------
void __fastcall TfMain::bbDoneClick(TObject *Sender)  {
  Application->Terminate();
}
//---------------------------------------------------------------------------
bool TfMain::DoRun()  {
  TEFile::ChangeDir( OlexInstalledPath );
  return LaunchFile(OlexInstalledPath + "olex2.exe", true);
}
//---------------------------------------------------------------------------
bool TfMain::_DoInstall(const olxstr& zipFile, const olxstr& installPath)  {
  TBasicApp::SetBaseDir(TEFile::AddTrailingBackslash(installPath) << "installer.exe");
  TWinZipFileSystem zfs( zipFile );
  bool res = zfs.Exists("olex2.tag");
  if( res )  {
    zfs.OnProgress->Add( new TProgress );
    TEFile* lic_f = NULL;
    try  {  lic_f = zfs.OpenFileAsFile("licence.rtf");  }
    catch(...)  {  res = false;  }
    if( lic_f == NULL )  res = false;
    if( res )  {
      olxstr lic_fn = lic_f->GetName();
      delete lic_f;
      dlgLicence->reEdit->Lines->LoadFromFile( lic_fn.c_str() );
      if( dlgLicence->ShowModal() != mrOk )
        return false;
      try  {  zfs.ExtractAll(installPath);  }
      catch(...) {  res = false;  }
      if( res )  {
        olxstr redist_fn = TBasicApp::GetBaseDir() + "vcredist_x86.exe";
        if( !RedistInstalled )  {
          frMain->stAction->Caption = "Installing MSVCRT...";
          if( !LaunchFile(redist_fn, false) )  {
            Application->MessageBoxA("Could not install MSVC redistributables.", "Installation failed", MB_OK|MB_ICONERROR);
            frMain->stAction->Caption = "Failed to install MSVCRT...";
            return false;
          }
        }
        TEFile::DelFile( redist_fn );
      }
    }
  }
  if( !res )
    Application->MessageBoxA("Invalid installation archive.", "Installation failed", MB_OK|MB_ICONERROR);
  else
    updater::UpdateAPI::TagInstallationAsNew();
  return res;
}
//---------------------------------------------------------------------------
void TfMain::DisableInterface(bool v)  {
  frMain->sbPickZip->Enabled = !v;
  frMain->bbBrowse->Enabled = !v;
  frMain->eInstallationPath->Enabled = !v;
  frMain->cbCreateShortcut->Enabled = !v;
  frMain->cbCreateDesktopShortcut->Enabled = !v;
  frMain->cbRepository->Enabled = !v;
  frMain->rgAutoUpdate->Enabled = !v;
  frMain->cbProxy->Enabled = !v;
}
//---------------------------------------------------------------------------
bool TfMain::DoInstall()  {
  DisableInterface(true);
  olxstr reposPath, proxyPath, installPath;
  frMain->bbInstall->Enabled = false;
  pbProgress->Visible = true;
  if( frMain->eProxy->Enabled )
    proxyPath = frMain->eProxy->Text.c_str();

  reposPath = TEFile::UnixPath(frMain->cbRepository->Text.c_str());

  bool localInstall = TEFile::IsAbsolutePath( reposPath );

  if( !localInstall )
    if( reposPath.Length() && !reposPath.EndsWith('/') )
      reposPath << '/';

  installPath = frMain->eInstallationPath->Text.c_str();
  installPath = TEFile::WinPath( installPath );
  if( !installPath.IsEmpty() && !installPath.EndsWith('\\') )  installPath << '\\';

  if( !TEFile::Exists(installPath) )  {
    if( !ForceDirectories( installPath.c_str() ) )  {
      Application->MessageBoxA("Could not create installation folder.\
 Please make sure the folder is not opened in any other programs and try again.", "Error", MB_OK|MB_ICONERROR);
      frMain->bbInstall->Enabled = true;
      return false;
    }
  }
  else if( !TEFile::IsEmptyDir(installPath) ) {
    int res =Application->MessageBoxA("The instalaltion folder already exists.\nThe installer needs to empty it.\nContinue?",
      "Confirm", MB_YESNOCANCEL|MB_ICONWARNING);
    if( res == IDYES )  {
      if( !TEFile::DeleteDir(installPath, true) )  {
        Application->MessageBoxA("Could not clean up the instalaltion folder. Please try later.", "Error", MB_OK|MB_ICONERROR);
        frMain->bbInstall->Enabled = true;
        return false;
      }
    }
    else if( res == IDCANCEL )  {
      frMain->bbInstall->Enabled = true;
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
      repos.OnProgress->Add( new TProgress );
      TEFile* zipf = repos.OpenFileAsFile( url.GetPath() + updater::UpdateAPI::GetInstallationFileName());
      if( zipf == NULL )  {
        frMain->stAction->Caption = "Failed...";
        Application->MessageBoxA(
          (AnsiString("Could not locate the Olex2 archive: ") +
          updater::UpdateAPI::GetInstallationFileName().c_str() +
          + "\nPlease try another repository.").c_str(),
          "Zip file fetching error",
          MB_OK|MB_ICONERROR);
        frMain->bbInstall->Enabled = true;
        return false;
      }
      olxstr zipName( zipf->GetName() );
      zipf->SetTemporary(false);
      delete zipf;
      bool res = _DoInstall( zipName, installPath );
      TEFile::DelFile(zipName);
      if( !res )
        return false;
    }
    else  {
      if( !_DoInstall( frMain->cbRepository->Text.c_str(), installPath ) )
        return false;
    }
    updater::UpdateAPI api;
    if( localInstall )
      api.GetSettings().repository = "http://www.olex2.org/olex2-distro/";
    else
      api.GetSettings().repository = api.TrimTagPart(reposPath);

    api.GetSettings().proxy = proxyPath;
    api.GetSettings().update_interval = frMain->rgAutoUpdate->Items->Strings[frMain->rgAutoUpdate->ItemIndex].c_str();
    api.GetSettings().Save();

    frMain->stAction->Caption = "Done";
    InitRegistry( installPath.c_str() );
    // create shortcuts
    if( frMain->cbCreateShortcut->Checked )
      TShellUtil::CreateShortcut(TShellUtil::GetSpecialFolderLocation(fiCommonStartMenu) + "Olex2.lnk",
                                 installPath + "olex2.exe", "Olex2 launcher", SetRunAs);
    if( frMain->cbCreateDesktopShortcut->Checked )
      TShellUtil::CreateShortcut(TShellUtil::GetSpecialFolderLocation(fiCommonDesktop) + "Olex2.lnk",
                                 installPath + "olex2.exe", "Olex2 launcher", SetRunAs);
    OlexInstalledPath = installPath;
  }
  catch( const TExceptionBase& exc )  {
    Application->MessageBox("The installation has failed. If using online installation please check, that\
your computers is online.", "Installation failed", MB_OK|MB_ICONERROR);
    return false;
  }
  return true;
}
//---------------------------------------------------------------------------
void __fastcall TfMain::bbInstallClick(TObject *Sender)  {
  if( action == actionInstall )  {
    if( DoInstall() )
      SetAction(actionRun);
    else
      SetAction(actionInstall);
  }
  else if( action == actionReinstall )  {
    if( DoUninstall() )  {
      if( action == actionExit )
        Application->Terminate();
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
//---------------------------------------------------------------------------
bool TfMain::CheckOlexInstalled(olxstr& installPath)  {
  TRegistry* Reg = new TRegistry();
  RedistInstalled = false;
  bool res = false;
  try  {
    Reg->RootKey = HKEY_CLASSES_ROOT;
    AnsiString cmdKey = "Applications\\olex2.dll\\shell\\open\\command";
    res = Reg->OpenKey(cmdKey, false);
    if( res )  {
      installPath = Reg->ReadString("").c_str();
      installPath = TEFile::ExtractFilePath(installPath.Trim('"'));
      Reg->CloseKey();
    }
    Reg->RootKey = HKEY_LOCAL_MACHINE;
    cmdKey = "SOFTWARE\\Olex2";
    bool res1 = Reg->OpenKey(cmdKey, false);
    if( res1 )  {
      RedistInstalled = Reg->ReadBool("RedistInstalled");
      Reg->CloseKey();
    }
  }
  catch( ... )  {    }
  delete Reg;
  if( res && !TEFile::Exists(installPath) )
    return false;
  return res;
}
//---------------------------------------------------------------------------
bool TfMain::InitRegistry(const AnsiString& installPath)  {
  TRegistry* Reg = new TRegistry();
  bool res = false;
  try  {
    Reg->RootKey = HKEY_CLASSES_ROOT;
    AnsiString cmdKey = "Applications\\olex2.dll\\shell\\open\\command";
    res = Reg->OpenKey(cmdKey, true);
    if( res )  {
      AnsiString val = '\"';
      val += installPath;
      val += "olex2.dll";
      val += "\" '%1'";
      Reg->WriteString("", val);
      Reg->CloseKey();
    }
    if( res )  {
      res = Reg->OpenKey(".ins\\OpenWithList\\olex2.dll", true);
      Reg->CloseKey();
    }
    if( res )  {
      res = Reg->OpenKey(".mol\\OpenWithList\\olex2.dll", true);
      Reg->CloseKey();
    }
    if( res )  {
      res = Reg->OpenKey(".res\\OpenWithList\\olex2.dll", true);
      Reg->CloseKey();
    }
    if( res )  {
      res = Reg->OpenKey(".cif\\OpenWithList\\olex2.dll", true);
      Reg->CloseKey();
    }
    if( res )  {
      res = Reg->OpenKey(".xyz\\OpenWithList\\olex2.dll", true);
      Reg->CloseKey();
    }
    Reg->RootKey = HKEY_LOCAL_MACHINE;
    cmdKey = "SOFTWARE\\Olex2";
    res = Reg->OpenKey(cmdKey, true);
    if( res )  {
      Reg->WriteBool("RedistInstalled", RedistInstalled);
      Reg->CloseKey();
    }
  }
  catch( ... )  {    }
  delete Reg;
  return res;
}
//---------------------------------------------------------------------------
bool TfMain::CleanRegistryX()  {
  TRegistry* Reg = new TRegistry();
  bool res = false;
  try  {
    Reg->RootKey = HKEY_LOCAL_MACHINE;
    AnsiString cmdKey = "SOFTWARE\\Olex2";
    res = Reg->KeyExists( cmdKey );
    if( res )
      res = Reg->DeleteKey(cmdKey);
    if( res )
      RedistInstalled = false;
  }
  catch( ... )  {    }
  delete Reg;
  return res;
}
//---------------------------------------------------------------------------
bool TfMain::CleanRegistry()  {
  TRegistry* Reg = new TRegistry();
  bool res = false;
  try  {
    Reg->RootKey = HKEY_CLASSES_ROOT;
    AnsiString cmdKey = "Applications\\olex2.dll\\shell\\open\\command";
    res = Reg->KeyExists( cmdKey );
    if( res )  res = Reg->DeleteKey(cmdKey);
    if( res )  res = Reg->DeleteKey(".ins\\OpenWithList\\olex2.dll");
    if( res )  res = Reg->DeleteKey(".mol\\OpenWithList\\olex2.dll");
    if( res )  res = Reg->DeleteKey(".res\\OpenWithList\\olex2.dll");
    if( res )  res = Reg->DeleteKey(".cif\\OpenWithList\\olex2.dll");
    if( res )  res = Reg->DeleteKey(".xyz\\OpenWithList\\olex2.dll");
  }
  catch( ... )  {    }
  delete Reg;
  return res;
}
//---------------------------------------------------------------------------
bool TfMain::LaunchFile( const olxstr& fileName, bool do_exit )  {
  STARTUPINFO si;
  PROCESS_INFORMATION ProcessInfo;
  AnsiString Tmp, cmdl;
  Tmp += fileName.c_str();
  ZeroMemory(&si, sizeof(STARTUPINFO));
  si.cb = sizeof(STARTUPINFO);
  si.wShowWindow = SW_SHOW;
  si.dwFlags = STARTF_USESHOWWINDOW;
  if( !do_exit )  {
    cmdl = Tmp;
    cmdl += " /q";
  }
  // Launch the child process.
  if( !CreateProcess(
        Tmp.c_str(),
        cmdl.IsEmpty() ? NULL : cmdl.c_str(),
        NULL, NULL,   true,
        0, NULL,
        NULL,
        &si, &ProcessInfo))
  {
    Application->MessageBox( (AnsiString("Could not start ") += fileName.c_str()).c_str(),
      "Error",
      MB_OK|MB_ICONERROR);
      return false;
  }
  if( do_exit )
    Application->Terminate();
  else  {
    DWORD rv;
    bool res = false;
    while( (res = GetExitCodeProcess(ProcessInfo.hProcess, &rv)) && rv == STILL_ACTIVE )  {
      Application->ProcessMessages();
      SleepEx(100, false);
    }
    if( res )
      return rv == 0;
    return false;
  }
  return true;
}
//---------------------------------------------------------------------------

void __fastcall TfMain::cbProxyClick(TObject *Sender)  {
  frMain->eProxy->Enabled = frMain->cbProxy->Checked;
}
//---------------------------------------------------------------------------

void __fastcall TfMain::FormPaint(TObject *Sender)  {
  Canvas->Rectangle(0, 0, Width, Height);
}
//---------------------------------------------------------------------------

void __fastcall TfMain::sbPickZipClick(TObject *Sender)  {
  if( frMain->dlgOpen->Execute() )  {
    frMain->cbRepository->Text = frMain->dlgOpen->FileName;
  }
}
//---------------------------------------------------------------------------
void __fastcall TfMain::FormShow(TObject *Sender)  {
  try  {
    frMain->cbRepository->Items->Clear();
    frMain->cbRepository->Text = "";
    updater::UpdateAPI api;
    TStrList repos;
    api.GetAvailableRepositories(repos);
    if( repos.IsEmpty() )  return;
    for( int i=0; i < repos.Count(); i++ )
      frMain->cbRepository->Items->Add( repos[i].u_str());
    frMain->cbRepository->Text = repos[0].u_str();
  }
  catch(...)  {
    //Application->MessageBox( "Could not discover any Olex2 repositories, only offline installation will be available",
    //  "Error",
    //  MB_OK|MB_ICONERROR);
  }
  olxstr bd( CmdLine );

  olxstr zipfn( TEFile::ExtractFilePath(bd) + updater::UpdateAPI::GetInstallationFileName() );
  if( TEFile::Exists(zipfn) )  {
    if( !TEFile::IsAbsolutePath(zipfn) )  {
      zipfn = TEFile::CurrentDir();
      zipfn << "\\olex2.zip";
    }
    frMain->cbRepository->Text = zipfn.c_str();
    frMain->cbRepository->Items->Add( zipfn.u_str());
  }
}
//---------------------------------------------------------------------------
void TfMain::SetAction(int a)  {
  action = a;
  if( a == actionInstall || a == actionReinstall )  {
    DisableInterface(false);
    frMain->bbInstall->Enabled = true;
    if( a == actionInstall )
      frMain->bbInstall->Caption = "Install";
    else
      frMain->bbInstall->Caption = "Re-install";
    pbProgress->Position = 0;
    pbProgress->Visible = true;
  }
  else  {
    DisableInterface(true);
    pbProgress->Visible = false;
    frMain->bbInstall->Caption = "Run!";
    frMain->bbInstall->Enabled = true;
  }
}
//---------------------------------------------------------------------------
bool TfMain::CleanRegistryAndShortcuts(bool sc)  {
  if( !CleanRegistry() )  {
    Application->MessageBoxA("Could not remove registry entries", "Error", MB_OK|MB_ICONERROR);
    return false;
  }
  if( sc )  {
    // find and delete shortcuts
    try  {
      olxstr sf = TShellUtil::GetSpecialFolderLocation(fiCommonStartMenu) + "Olex2.lnk";
      if( TEFile::Exists( sf ) )
        TEFile::DelFile( sf );
      sf = TShellUtil::GetSpecialFolderLocation(fiCommonDesktop) + "Olex2.lnk";
      if( TEFile::Exists( sf ) )
        TEFile::DelFile( sf );
    }
    catch( const TExceptionBase& exc )  {
      Application->MessageBoxA("Could not remove shortcuts", "Error", MB_OK|MB_ICONERROR);
      return false;
    }
  }
  return true;
}
//---------------------------------------------------------------------------
bool TfMain::DoUninstall()  {
  if( patcher::PatchAPI::IsOlex2Running() )  {
    Application->MessageBoxA("There are Olex2 instances are running or you do\
 not have sufficient rights to access the the instalaltion folder...", "Error", MB_OK|MB_ICONERROR);
    return false;
  }
  // init the append string
  if( rename_status == 0 )  {
    olxstr tag = OlexTag;
    if( tag.IsEmpty() )
      tag = TETime::FormatDateTime("yyyy-MM-dd", TETime::Now());
    dlgUninstall->eAppend->Text = tag.u_str();
  }
  else if( rename_status & rename_status_BaseDir )  {
    dlgUninstall->eAppend->Enabled = false;
  }

  if( dlgUninstall->ShowModal() == mrOk )  {
    action = dlgUninstall->cbInstall->Checked ? actionInstall : actionExit;
    if( dlgUninstall->rgRemove->Checked )  {
      Cursor = crHourGlass;
      if( !TEFile::DeleteDir(OlexInstalledPath) )  {
        Cursor = crArrow;
        Application->MessageBoxA("Could not remove Olex2 installation folder...", "Error", MB_OK|MB_ICONERROR);
        return false;
      }
      Cursor = crArrow;
      if( dlgUninstall->cbRemoveUserSettings->Checked )  {
        if( TEFile::Exists(OlexDataDir) )
          TEFile::DeleteDir(OlexDataDir);
      }
      CleanRegistryX();  // cleanup msvcrt installation flag - just in case...
      return CleanRegistryAndShortcuts(true);
    }
    else  {
      if( dlgUninstall->rgRename->Checked )  {
        if( TEFile::Exists( patcher::PatchAPI::GetUpdateLocationFileName()) )  {
          Application->MessageBoxA("The update for current installation is incomplete.\n\
Please run currently installed Olex2 to apply the updates and then exit Olex2 and press OK ", "Error", MB_OK|MB_ICONERROR);
          return false;
        }
        olxstr ip = TEFile::AddTrailingBackslash(frMain->eInstallationPath->Text.c_str());
        olxstr rp = TEFile::AddTrailingBackslash(
          TEFile::RemoveTrailingBackslash(Bapp->GetBaseDir()) << '-' << dlgUninstall->eAppend->Text.c_str());
        // this has to go first as otherwise the tag gets lost...
        if( (rename_status & rename_status_DataDir) == 0 )  {
          olxstr new_data_dir = patcher::PatchAPI::ComposeNewSharedDir(TShellUtil::GetSpecialFolderLocation(fiAppData), rp);
          if( TEFile::Exists(OlexDataDir) )  {
            if( !TEFile::Rename(OlexDataDir, new_data_dir, true) )  {
              Application->MessageBoxA("Failed to rename previous data folder", "Error", MB_OK|MB_ICONERROR);
              return false;
            }
            patcher::PatchAPI::SaveLocationInfo(new_data_dir, rp);
          }
          rename_status |= rename_status_DataDir;
        }
        if( (rename_status & rename_status_BaseDir) == 0 )  {  // has to be done if failed on the second rename
          if( ip.Equalsi(rp) )  {
            Application->MessageBoxA("The renamed and installation paths should differ", "Error", MB_OK|MB_ICONERROR);
            return false;
          }
          if( TEFile::Exists(rp) )  {
            Application->MessageBoxA("The renamed path already exists", "Error", MB_OK|MB_ICONERROR);
            return false;
          }
          if( !TEFile::Rename(Bapp->GetBaseDir(), rp, true) )  {
            Application->MessageBoxA("Failed to rename previous installation folder", "Error", MB_OK|MB_ICONERROR);
            return false;
          }
          rename_status |= rename_status_BaseDir;
        }
        CleanRegistryAndShortcuts(false);
        bool menu_sc = false, desktop_sc = false;
        olxstr m_sc_fn = TShellUtil::GetSpecialFolderLocation(fiCommonStartMenu) + "Olex2.lnk";
        if( TEFile::Exists( m_sc_fn ) )  {
          TShellUtil::CreateShortcut(TShellUtil::GetSpecialFolderLocation(fiCommonStartMenu) << "Olex2-" <<
            dlgUninstall->eAppend->Text.c_str() << ".lnk",
            rp + "olex2.exe", "Olex2 launcher", SetRunAs);
          menu_sc = true;
        }
        olxstr d_sc_fn = TShellUtil::GetSpecialFolderLocation(fiCommonDesktop) + "Olex2.lnk";
        if( TEFile::Exists( d_sc_fn ) )  {
          TShellUtil::CreateShortcut(TShellUtil::GetSpecialFolderLocation(fiCommonDesktop) << "Olex2-" <<
            dlgUninstall->eAppend->Text.c_str() << ".lnk",
            rp + "olex2.exe", "Olex2 launcher", SetRunAs);
          desktop_sc = true;
        }
        if( menu_sc )  TEFile::DelFile( m_sc_fn );
        if( desktop_sc )  TEFile::DelFile( d_sc_fn );
      }
    }
    return true;
  }
  else
    return false;
}

