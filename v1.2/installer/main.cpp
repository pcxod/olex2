//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <FileCtrl.hpp>
#include <Registry.hpp>

#include "main.h"

#include "licence.h"

#include "efile.h"
#include "winhttpfs.h"
#include "unzip.h"
#include "settingsfile.h"
#include "shellutil.h"
#include "estrlist.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "frame1"
#pragma resource "*.dfm"

TfMain *fMain;

class TProgress: public AActionHandler
{
public:
  TProgress(){  return; }
  virtual ~TProgress(){  return; }
  bool Exit(const IEObject *Sender, const IEObject *Data)  {
    return true;
  }
  bool Enter(const IEObject *Sender, const IEObject *Data)  {
    if( !EsdlInstanceOf( *Data, TOnProgress) )  return false;
    IEObject *d_p = const_cast<IEObject*>(Data);
    TOnProgress *A = dynamic_cast<TOnProgress *>(d_p);
//    fMain->frMain->pbProgress->Max = A->GetMax();
    return true;
  }
  bool Execute(const IEObject *Sender, const IEObject *Data)  {
    if( !EsdlInstanceOf( *Data, TOnProgress) )
      throw TInvalidArgumentException(__OlxSourceInfo, "invalid type");

    IEObject *d_p = const_cast<IEObject*>(Data);
    TOnProgress *A = dynamic_cast<TOnProgress *>(d_p);
//    fMain->frMain->pbProgress->Position = A->GetPos();
    fMain->frMain->stAction->Caption = A->GetAction().c_str();
    Application->ProcessMessages();
    return true;
  }
};

olxstr TfMain::SettingsFile = "usettings.dat";

//---------------------------------------------------------------------------
__fastcall TfMain::TfMain(TComponent* Owner)
  : TForm(Owner)
{
  MouseDown = Dragging = false;
  Expanded = false;
  Bapp = new TBasicApp( CmdLine[0] );
  Progress = new TProgress();
  Bapp->OnProgress->Add( Progress );
  olxstr ip( TShellUtil::GetSpecialFolderLocation(fiProgramFiles) );
  TEFile::AddTrailingBackslashI( ip ) << "Olex2";
  frMain->eInstallationPath->Text = ip.c_str();
  OlexInstalled = CheckOlexInstalled( OlexInstalledPath );
  if( OlexInstalled )  {
    TEFile::AddTrailingBackslash( OlexInstalledPath );
    frMain->eInstallationPath->Text = OlexInstalledPath.c_str();
    olxstr sfile = OlexInstalledPath;
           sfile << TfMain::SettingsFile;
    if( TEFile::FileExists( sfile ) )  {
      TSettingsFile Settings( sfile );
      frMain->eRepositoryPath->Text = Settings.ParamValue("repository").c_str();
      frMain->eProxy->Text = Settings.ParamValue("proxy").c_str();
      int updateInd = frMain->rgAutoUpdate->Items->IndexOfName( Settings.ParamValue("update").c_str() );
      if( updateInd != -1 )
        frMain->rgAutoUpdate->ItemIndex = updateInd;
    }
    frMain->sbPickZip->Visible = false;
    frMain->eRepositoryPath->Width = 278;
    frMain->bbInstall->Caption = "Update";
    frMain->bbUninstall->Visible = true;
    frMain->bbBrowse->Enabled = false;
    frMain->eInstallationPath->Enabled = false;
    frMain->cbCreateShortcut->Visible = false;
    frMain->cbCreateDesktopShortcut->Visible = false;
  }
  else  {
    olxstr zipfn( Bapp->BaseDir() + "olex2.zip" );
    if( TEFile::FileExists(zipfn) )  {
      if( !TEFile::IsAbsolutePath(zipfn) )  {
        zipfn = TEFile::CurrentDir();
        zipfn << "\\olex2.zip";
      }
      frMain->eRepositoryPath->Text = zipfn.c_str();
    }
  }
  frMain->bbInstall->Default = true;
}
//---------------------------------------------------------------------------
__fastcall TfMain::~TfMain()
{
  delete Bapp;
}
//---------------------------------------------------------------------------
void __fastcall TfMain::iSplashMouseMove(TObject *Sender,
      TShiftState Shift, int X, int Y)
{
  if( !MouseDown )  return;
  if( !Dragging )
  {
    if( abs(X-MouseDownX) >= 3 || abs(Y-MouseDownY) >= 3 )
      Dragging = true;
  }
  if( !Dragging )  return;
  Left = Left + (X-MouseDownX);
  Top = Top + (Y-MouseDownY);
  //MouseDownX = X;
  //MouseDownY = Y;
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
    dir = TShellUtil::GetSpecialFolderLocation(fiDesktop);
  }
  catch( ... )  {  ;  }
  this->Enabled = true;
}
//---------------------------------------------------------------------------
void __fastcall TfMain::bbDoneClick(TObject *Sender)
{
  Application->Terminate();  
}
//---------------------------------------------------------------------------
bool ExtractZip(TWinHttpFileSystem& repos, const olxstr& zipName, const olxstr& extractPath)  {
  TEFile* zipf = repos.OpenFileAsFile(repos.GetBase() + zipName);
  if( zipf == NULL )  {
    //Application->MessageBoxA("Could not locate the olex distribution", "Zip file fetching error", MB_OK|MB_ICONERROR);
    return false;
  }
  olxstr zipfn( zipf->Name() );
  delete zipf;
  HZIP hz = OpenZip(zipfn.c_str(), NULL);
  ZIPENTRY ze;
  GetZipItem(hz, -1, &ze);
  int numitems = ze.index;
  TOnProgress Progress;
  Progress.SetMax( numitems );
  TBasicApp::GetInstance()->OnProgress->Enter( NULL, &Progress );
  // -1 gives overall information about the zipfile
  for( int zi=0; zi < numitems; zi++ )  {
    ZIPENTRY ze;
    GetZipItem(hz, zi, &ze); // fetch individual details
    Progress.SetPos( zi );
    Progress.SetAction( ze.name );
    TBasicApp::GetInstance()->OnProgress->Execute( NULL, &Progress );
    UnzipItem(hz, zi, (extractPath + ze.name).c_str() );         // e.g. the item's name.
  }
  CloseZip(hz);
  return true;
}
//---------------------------------------------------------------------------
bool TfMain::DoInstall(const olxstr& zipFile, const olxstr& installPath)  {
  HZIP hz = OpenZip(zipFile.c_str(), NULL);
  ZIPENTRY ze;
  GetZipItem(hz, -1, &ze);
  int numitems = ze.index;
  TOnProgress Progress;
  Progress.SetMax( numitems );
  TBasicApp::GetInstance()->OnProgress->Enter( NULL, &Progress );
  olxstr licFile("licence.rtf");
  bool accepted = false;
  for( int zi=0; zi < numitems; zi++ )  {
    GetZipItem(hz, zi, &ze); // fetch individual details
    if( !licFile.Comparei( ze.name ) )  {
      UnzipItem(hz, zi, (installPath + ze.name).c_str() );         // e.g. the item's name.
      dlgLicence->reEdit->Lines->LoadFromFile( (installPath + ze.name).c_str() );
      accepted = (dlgLicence->ShowModal() == mrOk );
      break;
    }
  }
  if( !accepted )  {
    Application->Terminate();
    return false;  // nobody cares thougth :)
  }

  for( int zi=0; zi < numitems; zi++ )  {
    GetZipItem(hz, zi, &ze); // fetch individual details
    Progress.SetPos( zi );
    Progress.SetAction( ze.name );
    TBasicApp::GetInstance()->OnProgress->Execute( NULL, &Progress );
    UnzipItem(hz, zi, (installPath + ze.name).c_str() );         // e.g. the item's name.
  }
  CloseZip(hz);
  return true;
}
//---------------------------------------------------------------------------
void __fastcall TfMain::bbInstallClick(TObject *Sender)  {
  olxstr reposPath, proxyPath, installPath;
  TSettingsFile Settings;

  if( frMain->eProxy->Enabled )  {
    proxyPath = frMain->eProxy->Text.c_str();
  }

  reposPath = frMain->eRepositoryPath->Text.c_str();

  bool localInstall = TEFile::IsAbsolutePath( reposPath );

  if( !localInstall )
    if( reposPath.Length() && !reposPath.EndsWith('/') )
      reposPath << '/';

  installPath = frMain->eInstallationPath->Text.c_str();
  installPath = TEFile::WinPath( installPath );
  if( installPath.Length() && !installPath.EndsWith('\\') )  installPath << '\\';

  if( !OlexInstalled && !TEFile::FileExists( installPath ) )  {
    if( !ForceDirectories( installPath.c_str() ) )  {
      Application->MessageBoxA("Could not create installation directory", "IO Error", MB_OK|MB_ICONERROR);
      return;
    }
  }
  if( !localInstall )
    Settings.UpdateParam("repository", reposPath);
  else  // use default
    Settings.UpdateParam("repository", "http://dimas.dur.ac.uk/olex-distro/update");
  Settings.UpdateParam("proxy", proxyPath);
  Settings.UpdateParam("update", frMain->rgAutoUpdate->Items->Strings[frMain->rgAutoUpdate->ItemIndex].c_str());

  if( OlexInstalled )  {
    // forcr launch to update
    Settings.UpdateParam("lastupdate", EmptyString);
    try  {
      Settings.SaveSettings( installPath + TfMain::SettingsFile );  }
    catch( TExceptionBase& exc )  {
      Application->MessageBoxA("Was not able to save settings", "Exception", MB_OK|MB_ICONERROR);  }
    TEFile::ChangeDir( OlexInstalledPath );
    LaunchFile(OlexInstalledPath + "olex2.exe", true);
    return;
  }

  try  {
    if( !localInstall )  {
      TUrl url( reposPath );
      if( !proxyPath.IsEmpty() )
        url.SetProxy(proxyPath);
      TWinHttpFileSystem repos(url);
      TEFile* zipf = repos.OpenFileAsFile( url.GetPath() + "olex2.zip");
      if( zipf == NULL )  {
        Application->MessageBoxA("Could not locate the olex distribution", "Zip file fetching error", MB_OK|MB_ICONERROR);
        return;
      }
      olxstr zipName( zipf->Name() );
      delete zipf;
      // the file gets deleted with the FS ... have to hurry!
      if( !DoInstall( zipName, installPath ) )
        return;
    }
    else  {
      if( !DoInstall( frMain->eRepositoryPath->Text.c_str(), installPath ) )
        return;
    }
//    frMain->pbProgress->Position = 0;
    // install MSVC redistributables
    olxstr redist_path( TBasicApp::GetInstance()->BaseDir() + "redist/vcredist_x86.exe");
    if( TEFile::FileExists(redist_path) && !TEFile::IsAbsolutePath(redist_path) )  {
      redist_path = TEFile::CurrentDir() + "/redist/vcredist_x86.exe";
    }
    if( TEFile::FileExists(redist_path) )
      LaunchFile( redist_path, false );
    frMain->stAction->Caption = "Done";
    InitRegistry( installPath.c_str() );
    // create shortcuts
    if( frMain->cbCreateShortcut->Checked )
      TShellUtil::CreateShortcut(TShellUtil::GetSpecialFolderLocation(fiStartMenu) + "Olex2.lnk",
                                 installPath + "olex2.exe", "Olex2 launcher");
    if( frMain->cbCreateDesktopShortcut->Checked )
      TShellUtil::CreateShortcut(TShellUtil::GetSpecialFolderLocation(fiDesktop) + "Olex2.lnk",
                                 installPath + "olex2.exe", "Olex2 launcher");
    if( !localInstall )  {
      Settings.UpdateParam("repository", reposPath + "update/");
      frMain->eRepositoryPath->Text = AnsiString(reposPath.c_str()) + "update/";
    }
    Settings.SaveSettings( installPath + TfMain::SettingsFile );
  }
  catch( const TExceptionBase& exc )  {
    Application->MessageBox(exc.GetException()->GetFullMessage().c_str(), "Exception", MB_OK|MB_ICONERROR);
  }
  OlexInstalled = CheckOlexInstalled( OlexInstalledPath );
  if( OlexInstalled )  {
    frMain->bbInstall->Caption = "Run!";
    frMain->bbInstall->Enabled = true;
  }
}
//---------------------------------------------------------------------------
bool TfMain::CheckOlexInstalled(olxstr& installPath)  {
  TRegistry* Reg = new TRegistry();
  bool res = false;
  try  {
    Reg->RootKey = HKEY_CLASSES_ROOT;
    AnsiString cmdKey = "Applications\\olex2.dll\\shell\\open\\command";
    res = Reg->OpenKey(cmdKey, false);
    if( res )  {
      installPath = Reg->ReadString("").c_str();
      if( installPath.Length() && installPath[0] == '\"' )  {
        int lind = installPath.LastIndexOf('"');
        if( lind > 0 )
          installPath = TEFile::ExtractFilePath(installPath.SubString(1, lind-1));
      }
      Reg->CloseKey();
    }
  }
  catch( ... )  {    }
  delete Reg;
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
    if( res )
    {
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
  }
  catch( ... )  {    }
  delete Reg;
  return res;
}
//---------------------------------------------------------------------------
bool TfMain::CleanRegistry()
{
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
void __fastcall TfMain::bbUninstallClick(TObject *Sender)  {
  if( !OlexInstalled )  return;
  olxstr indexFileName( OlexInstalledPath);
         indexFileName << "index.ind";

  frMain->bbUninstall->Enabled = false;
  frMain->bbInstall->Enabled = false;

  if( !TEFile::FileExists(indexFileName) )  {
    Application->MessageBoxA("Could not locate installation database\nProcessing registry and shortcuts...", "Error", MB_OK|MB_ICONINFORMATION);

    frMain->bbInstall->Enabled = true;
    frMain->bbInstall->Caption = "Install";
    frMain->bbUninstall->Visible = false;
    frMain->bbBrowse->Enabled = true;
    frMain->eInstallationPath->Enabled = true;
    frMain->cbCreateShortcut->Visible = true;
    frMain->cbCreateDesktopShortcut->Visible = true;
  }
  if( !CleanRegistry() )  {
    Application->MessageBoxA("Could not remove registry entries", "Error", MB_OK|MB_ICONERROR);
    return;
  }
  if( TEFile::FileExists(indexFileName) )  {
    TOSFileSystem osFS;
    osFS.SetBase(OlexInstalledPath);
    TFSIndex FSIndex(osFS);
    FSIndex.LoadIndex( indexFileName );
    CleanInstallationFolder( FSIndex.GetRoot() );
    TEFile::DelFile( indexFileName );
  }
  // find and delete shortcuts
  try  {
    olxstr sf = TShellUtil::GetSpecialFolderLocation(fiStartMenu) + "Olex2.lnk";
    if( TEFile::FileExists( sf ) )  TEFile::DelFile( sf );
    sf = TShellUtil::GetSpecialFolderLocation(fiDesktop) + "Olex2.lnk";
    if( TEFile::FileExists( sf ) )  TEFile::DelFile( sf );
  }
  catch( const TExceptionBase& exc )  {
    Application->MessageBoxA("Could not remove shortcuts", "Error", MB_OK|MB_ICONERROR);
  }
  Application->Terminate();
}
//---------------------------------------------------------------------------
bool TfMain::CleanInstallationFolder(TFSItem& item)  {
  if( !item.IsFolder() && item.GetParent() != NULL )  return false;
  for( int i=0; i < item.Count(); i++ )  {
    if( item.Item(i).IsFolder() )
      CleanInstallationFolder( item.Item(i) );
    else
      TEFile::DelFile( OlexInstalledPath + item.Item(i).GetFullName() );
  }
  if( (item.GetParent() != NULL) && TEFile::ChangeDir( OlexInstalledPath + item.GetFullName() ) )  {
    TStrList leftItems;
    TEFile::ListCurrentDir(leftItems, "*.*", sefAll);
    if( !leftItems.Count() )  {
      TEFile::ChangeDir( OlexInstalledPath );
      TEFile::DelDir( item.GetFullName() );
    }
  }
  return true;
}
//---------------------------------------------------------------------------
bool TfMain::LaunchFile( const olxstr& fileName, bool do_exit )  {
  STARTUPINFO si;
  PROCESS_INFORMATION ProcessInfo;
  AnsiString Tmp;
  Tmp += fileName.c_str();
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
    Application->MessageBox( (AnsiString("Could not start ") += fileName.c_str()).c_str(),
      "Error",
      MB_OK|MB_ICONERROR);
      return false;
  }
  if( do_exit )
    Application->Terminate();
  return true;
}
//---------------------------------------------------------------------------

void __fastcall TfMain::cbProxyClick(TObject *Sender)
{
  frMain->eProxy->Enabled = frMain->cbProxy->Checked;
}
//---------------------------------------------------------------------------

void __fastcall TfMain::FormPaint(TObject *Sender)
{
  Canvas->Rectangle(0, 0, Width, Height);
}
//---------------------------------------------------------------------------

void __fastcall TfMain::sbPickZipClick(TObject *Sender)
{
  if( frMain->dlgOpen->Execute() )  {
    frMain->eRepositoryPath->Text = frMain->dlgOpen->FileName;
  }
}
//---------------------------------------------------------------------------


