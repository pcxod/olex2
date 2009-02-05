//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "main.h"
#include "winhttpfs.h"
#include "winzipfs.h"
#include "splash.h"
#include "actions.h"

#include "log.h"
#include "exception.h"

#include "settingsfile.h"
#include "datafile.h"
#include "dataitem.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TdlgMain *dlgMain;

class TProgress: public AActionHandler
{
public:
  TProgress(){  return; }
  ~TProgress(){  return; }
  bool Exit(const IEObject *Sender, const IEObject *Data)  {
    return true;
  }
  bool Enter(const IEObject *Sender, const IEObject *Data)  {
    if( !EsdlInstanceOf(*Data, TOnProgress) )  return false;
    IEObject *d_p = const_cast<IEObject*>(Data);
    TOnProgress *A = dynamic_cast<TOnProgress *>(d_p);
    if( Sender!= NULL && (EsdlInstanceOf(*Sender, TWinHttpFileSystem)) || EsdlInstanceOf(*Sender, TWinZipFileSystem) )
      dlgSplash->pbFProgress->Max = A->GetMax();
    else
      dlgSplash->pbOProgress->Max = A->GetMax();
    //Application->MessageBox(A->GetAction().c_str(), "", MB_OK );
    return true;
  }
  bool Execute(const IEObject *Sender, const IEObject *Data)  {
    if( !EsdlInstanceOf( *Data, TOnProgress) )
      throw TInvalidArgumentException(__OlxSourceInfo, "invalid type");

    IEObject *d_p = const_cast<IEObject*>(Data);
    TOnProgress *A = dynamic_cast<TOnProgress *>(d_p);
    if( Sender!= NULL && EsdlInstanceOf(*Sender, TWinHttpFileSystem) || EsdlInstanceOf(*Sender, TWinZipFileSystem))  {
      dlgSplash->pbFProgress->Position = A->GetPos();
      dlgSplash->stFileName->Caption = TEFile::ExtractFileName(A->GetAction()).c_str();
    }
    else
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

  TProgress *P = new TProgress;
  TBasicApp::GetInstance()->OnProgress->Add(P);

  olxstr vfn = (TBasicApp::GetInstance()->BaseDir()+ "version.txt");
  // check updates ...
  //asm {  int 3  }
  // reading version info
  olxstr OlexFN( (TBasicApp::GetInstance()->BaseDir()+ "olex2.dll") );
  if( TEFile::FileExists(OlexFN) )  {
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
        if( TEFile::FileExists(vfn) )  {
          try  {
            TEFile vf(vfn, "rb");
            TStrList sl;
            sl.LoadFromTextStream( vf );
            if( sl.Count() >= 1 )
              Tmp += '-';
              Tmp += sl.String(0).c_str();
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
  if( !SetFileAttributes(checkFN.c_str(), FILE_ATTRIBUTE_SYSTEM) )  {
    Application->MessageBox("Please make sure that you have enough right to modify the installation folder",
      "Scheduled update failed", MB_OK|MB_ICONINFORMATION);
    Application->Terminate();
    return;
  }
  else
    SetFileAttributes(checkFN.c_str(), FILE_ATTRIBUTE_NORMAL);
  olxstr SettingsFile( TBasicApp::GetInstance()->BaseDir() + "usettings.dat" );
  TSettingsFile settings;
  if( TEFile::FileExists(SettingsFile) )  {
    olxstr Proxy, Repository = "http://dimas.dur.ac.uk/olex-distro/update",
             UpdateInterval = "Always";
    int LastUpdate = 0;
    settings.LoadSettings( SettingsFile );
    Proxy = settings.ParamValue("proxy");
    if( settings.ParamExists("repository") )
      Repository = settings.ParamValue("repository");
    if( settings.ParamExists("update") )
      UpdateInterval = settings.ParamValue("update");
    if( settings.ParamExists("lastupdate") )  {
      LastUpdate = settings.ParamValue("lastupdate", '0').RadInt<long>();
    }
    if( TEFile::ExtractFileExt(Repository).Comparei("zip") != 0 )
      if( Repository.Length() && !Repository.EndsWith('/') )
        Repository << '/';

    bool Update = false, succeded = false;
    // evaluate properties
    TStrList props;
    props.Add("olex-update");
    olxstr pluginFile = TBasicApp::GetInstance()->BaseDir() + "plugins.xld";
    if( TEFile::FileExists( pluginFile ) )  {
      try  {
        TDataFile df;
        df.LoadFromXLFile( pluginFile, NULL );
        TDataItem* PluginItem = df.Root().FindItem("Plugin");
        if( PluginItem != NULL )  {
          for( int i=0; i < PluginItem->ItemCount(); i++ )
            props.Add( PluginItem->GetItem(i).GetName() );
        }
      }
      catch( TExceptionBase &e )  {  ;  }
    }
    // end properties evaluation
    if( TEFile::ExtractFileExt(Repository).Comparei("zip") == 0 )  {
      if( !TEFile::IsAbsolutePath(Repository) )
        Repository = TBasicApp::GetInstance()->BaseDir() + Repository;
      if( TEFile::FileAge(Repository) > LastUpdate )  {
        Update = true;
        succeded = UpdateInstallationZ( Repository, props );
      }
    }
    else  {
      if( UpdateInterval == "Always" )  Update = true;
      else if( UpdateInterval == "Daily" )
        Update = ((TETime::EpochTime() - LastUpdate ) > SecsADay );
      else if( UpdateInterval == "Weekly" )
        Update = ((TETime::EpochTime() - LastUpdate ) > SecsADay*7 );
      else if( UpdateInterval == "Monthly" )
        Update = ((TETime::EpochTime() - LastUpdate ) > SecsADay*30 );

      TUrl url(Repository);
      if( !Proxy.IsEmpty() )  url.SetProxy( Proxy );
      if( Update )
        succeded = UpdateInstallationH( url, props );
    }
    if( Update && succeded )  { // have to save lastupdate in anyway
      settings.UpdateParam("lastupdate", TETime::EpochTime() );
      settings.SaveSettings( SettingsFile );
    }
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
void TdlgMain::Launch()
{
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
bool TdlgMain::UpdateInstallationH( const TUrl& url, const TStrList& properties )  {
  try  {
    TOSFileSystem DestFS; // local file system
    TWinHttpFileSystem SrcFS( url ); // remote FS

    DestFS.SetBase( TBasicApp::GetInstance()->BaseDir() );
    olxstr tmp = DestFS.GetBase();
    TEFile::AddTrailingBackslash( tmp );
    DestFS.SetBase( tmp );
    TFSIndex FI( SrcFS );
    TFSItem::SkipOptions so;
    TStrList filesToSkip;
    filesToSkip.Add("olex2.exe");
    so.filesToSkip = &filesToSkip;
    return FI.Synchronise(DestFS, properties, &so);
  }
  catch( TExceptionBase& exc )  {
    TStrList out;
    Application->MessageBox("Please make sure that your computer is online and/or you have enough right to modify the installation folder\
\nAlso the Olex2 repository might be down - once Olex2 is running you may choose another one from 'Update Options' in the 'Help' menu",
      "Scheduled update failed", MB_OK|MB_ICONINFORMATION);
    return false;
  }
}
//---------------------------------------------------------------------------
bool TdlgMain::UpdateInstallationZ( const olxstr& zip_name, const TStrList& properties )  {
  try  {
    TOSFileSystem DestFS; // local file system
    TWinZipFileSystem SrcFS(zip_name);

    DestFS.SetBase( TBasicApp::GetInstance()->BaseDir() );
    olxstr tmp = DestFS.GetBase();
    TEFile::AddTrailingBackslash( tmp );
    DestFS.SetBase( tmp );
    TFSIndex FI( SrcFS );

    return FI.Synchronise(DestFS, properties);
  }
  catch( TExceptionBase& exc )  {
    TStrList out;
    exc.GetException()->GetStackTrace(out);
    Application->MessageBox(out.Text('\n').u_str(),
      "Update failed", MB_OK|MB_ICONERROR);
    return false;
  }
}

void __fastcall TdlgMain::tTimerTimer(TObject *Sender)
{
  // force the GC to free temporray obejcts
  FBApp->OnIdle->Execute(NULL, NULL);
}
//---------------------------------------------------------------------------

