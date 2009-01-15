#include <stdio.h>

#include "wx/app.h"

#include "efile.h"
#include "bapp.h"
#include "log.h"
#include "outstream.h"
#include "efile.h"
#include "wxzipfs.h"
#include "etime.h"
#include "settingsfile.h"
#include "datafile.h"
#include "dataitem.h"
#include "wxhttpfs.h"
#include "wxftpfs.h"
#include <iostream>
using namespace std;

class TProgress: public AActionHandler  {
  inline bool IsFSSender(const IEObject* obj) const {
    if( obj != NULL && (EsdlInstanceOf(*obj, TwxHttpFileSystem) || 
                        EsdlInstanceOf(*obj, TwxZipFileSystem)  ||
                        EsdlInstanceOf(*obj, TwxFtpFileSystem)  )
                        ) return true;
    return false;
  }
public:
  TProgress(){  return; }
  ~TProgress(){  return; }
  bool Exit(const IEObject *Sender, const IEObject *Data)  {
    //TBasicApp::GetLog() << '\n';
    return true;
  }
  bool Enter(const IEObject *Sender, const IEObject *Data)  {
    if( Data == NULL || !EsdlInstanceOf(*Data, TOnProgress) )  {
      return false;
    }
    IEObject *d_p = const_cast<IEObject*>(Data);
    TOnProgress *A = dynamic_cast<TOnProgress *>(d_p);
    if( IsFSSender(Sender) ) { // reset particular file preogress  
      TBasicApp::GetLog().Info( A->GetAction() );
    }
    else
      ;// update global progress
    return true;
  }
  bool Execute(const IEObject *Sender, const IEObject *Data)  {
    if( !EsdlInstanceOf( *Data, TOnProgress) )
      throw TInvalidArgumentException(__OlxSourceInfo, "invalid type");

    IEObject *d_p = const_cast<IEObject*>(Data);
    TOnProgress *A = dynamic_cast<TOnProgress *>(d_p);
    if( IsFSSender(Sender) )  { // update particular file info
      //TBasicApp::GetLog() << A->GetPos()*20/A->GetMax();
      ;
    }
    else
      ;// update global progress
    return true;
  }
};

bool UpdateMirror( AFileSystem& src, TwxFtpFileSystem& dest )  {
  try  {
    TFSIndex FI( src );
    TStrList empty;
    return FI.Synchronise(dest, empty, NULL);
  }
  catch( TExceptionBase& exc )  {
    TStrList out;
    exc.GetException()->GetStackTrace(out);
    TBasicApp::GetLog() << "Update failed due to :\n" << out.Text('\n');
    return false;
  }
}
//---------------------------------------------------------------------------
bool UpdateInstallationH( const TUrl& url, const TStrList& properties, const TFSItem::SkipOptions* toSkip )  {
  try  {
    TOSFileSystem DestFS; // local file system
    TwxHttpFileSystem SrcFS( url ); // remote FS
    DestFS.SetBase( TBasicApp::GetInstance()->BaseDir() );
    olxstr tmp = DestFS.GetBase();
    TEFile::AddTrailingBackslash( tmp );
    DestFS.SetBase( tmp );
    TFSIndex FI( SrcFS );
    return FI.Synchronise(DestFS, properties, toSkip);
  }
  catch( TExceptionBase& exc )  {
    TStrList out;
    exc.GetException()->GetStackTrace(out);
    TBasicApp::GetLog() << "Update failed due to :\n" << out.Text('\n');
    return false;
  }
}
//---------------------------------------------------------------------------
bool UpdateInstallationZ( const olxstr& zip_name, const TStrList& properties, const TFSItem::SkipOptions* toSkip )  {
  try  {
    TOSFileSystem DestFS; // local file system
    TwxZipFileSystem SrcFS(zip_name, false);

    DestFS.SetBase( TBasicApp::GetInstance()->BaseDir() );
    olxstr tmp = DestFS.GetBase();
    TEFile::AddTrailingBackslash( tmp );
    DestFS.SetBase( tmp );
    TFSIndex FI( SrcFS );

    return FI.Synchronise(DestFS, properties, toSkip);
  }
  catch( TExceptionBase& exc )  {
    TStrList out;
    exc.GetException()->GetStackTrace(out);
    TBasicApp::GetLog() << "Update failed due to :\n" << out.Text('\n');
    return false;
  }
}

void DoRun();

class MyApp: public wxAppConsole { 
  virtual bool OnInit() { 
//    wxSocketBase::Initialize();
    return true; 
  } 
  virtual int OnRun() {  return 0;  } 
};
IMPLEMENT_APP_NO_MAIN(MyApp)
int main(int argc, char** argv)  {
  MyApp app;
  TBasicApp* bapp = NULL;
  int res = 0;
  wxAppConsole::SetInstance(&app);
  try  {
    if( argc == 1 )  { // no folder to update provided
      char* olex_dir = getenv("OLEX2_DIR");
      if( olex_dir != NULL )
        bapp = new TBasicApp( olxstr(olex_dir) << "/dummy.txt" );
      else
        bapp = new TBasicApp(TEFile::CurrentDir() << "/dummy.txt");
    }
    else  {
      olxstr arg(argv[1]);
#ifdef _WIN32
      if( arg == "-help" || arg == "/help" )  {
#else
      if( arg == "--help" )  {
#endif     
        TBasicApp _bapp(  TEFile::CurrentDir() << "/dummy.txt" );
        TLog& log = _bapp.GetLog();
        log.AddStream( new TOutStream, true);
        log << "Unirun, Olex2 update program\n";
        log << "Compiled on " << __DATE__ << " at " << __TIME__ << '\n';
        log << "Usage: unirun [olex2_gui_dir]\n";
        log << "If no arguments provided, the system variable OLEX2_DIR will be checked first, if the variable is not set,\
               current folder will be updated\n";
        log << "(c) Oleg V. Dolomanov 2007-2008\n";
        return 0;
      }
      bapp = new TBasicApp(arg << "/dummy.txt");
    }
    bapp->GetLog().AddStream( new TOutStream, true);
    DoRun();
  }
  catch(const TExceptionBase& exc)  {
    if( bapp != NULL )  {
      TStrList out;
      exc.GetException()->GetStackTrace(out);
      bapp->GetLog() << out;
    }
    res = 1;
  }
  if( bapp != 0 )  {
    bapp->GetLog() << "\nFinished\n";
    delete bapp;
  }
  else  {
    cout << "\nFinished\n";
//#ifdef _UNICODE
//    wprintf( L"\nFinished\n");
//#else
//    printf("\nFinished\n");
//#endif
  }
  return res;
}

void DoRun()  {
  TBasicApp& bapp = *TBasicApp::GetInstance();
  if( &bapp == NULL )
    return;
  bapp.OnProgress->Add( new TProgress );

  olxstr SettingsFile( TBasicApp::GetInstance()->BaseDir() + "usettings.dat" );
  TSettingsFile settings;
  if( !TEFile::FileExists(SettingsFile) )  {
    TBasicApp::GetLog() << "Could not locate settings file: " << SettingsFile << '\n';
    return;
  }
  olxstr Proxy, ProxyUser, ProxyPasswd, 
    Repository = "http://dimas.dur.ac.uk/olex-distro/update",
           UpdateInterval = "Always",
           ftpToSync, ftpUser, ftpPwd, syncSrc;
  int LastUpdate = 0;
  TStrList extensionsToSkip, filesToSkip;
  TFSItem::SkipOptions toSkip;
  settings.LoadSettings( SettingsFile );
  if( settings.ParamExists("proxy") )        Proxy = settings.ParamValue("proxy");
  if( settings.ParamExists("proxy_user") )   ProxyUser = settings.ParamValue("proxy_user");
  if( settings.ParamExists("proxy_passwd") ) ProxyPasswd = settings.ParamValue("proxy_passwd");
  if( settings.ParamExists("repository") )   Repository = settings.ParamValue("repository");
  if( settings.ParamExists("update") )       UpdateInterval = settings.ParamValue("update");
  if( settings.ParamExists("lastupdate") )   LastUpdate = settings.ParamValue("lastupdate", '0').RadInt<long>();
  if( settings.ParamExists("exceptions") )  {
    extensionsToSkip.Strtok(settings.ParamValue("exceptions", EmptyString), ';');
    TBasicApp::GetLog() << "Skipping the following extensions: " << extensionsToSkip.Text(' ') << '\n';
  }
  if( settings.ParamExists("ftpToSync") )  {
    TStrList toks(settings.ParamValue("ftpToSync", EmptyString), ';');
    ftpToSync = toks[0];
    if( toks.Count() > 1 )  ftpUser = toks[1];
    if( toks.Count() > 2 )  ftpPwd = toks[2];
    TBasicApp::GetLog() << "Synchronising the following ftp mirror: " << ftpToSync << '\n';
  }
  syncSrc = settings.ParamValue("sync", "fs2Ftp");

  if( settings.ParamExists("skip") )  {
    filesToSkip.Strtok(settings.ParamValue("skip", EmptyString), ';');
    TBasicApp::GetLog() << "Skipping the following files: " << filesToSkip.Text(' ') << '\n';
  }
  if( TEFile::ExtractFileExt(Repository).Comparei("zip") != 0 )
    if( Repository.Length() && !Repository.EndsWith('/') )
      Repository << '/';

  toSkip.extsToSkip = extensionsToSkip.IsEmpty() ? NULL : &extensionsToSkip;
  toSkip.filesToSkip = filesToSkip.IsEmpty() ? NULL : &filesToSkip;
   
  bool Update = false;
  // evaluate properties
  TStrList props;
  props.Add("olex-update");
  // updating ported executables
  olxstr olex_port = settings.ParamValue("olex-port");
  if( !olex_port.IsEmpty() )
    props.Add(olex_port);
  // end
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
      bool skip = (extensionsToSkip.IsEmpty() && filesToSkip.IsEmpty());
      UpdateInstallationZ( Repository, props, skip ? NULL : &toSkip );
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
    url.SetUser(ProxyUser);
    url.SetPassword(ProxyPasswd);
    if( !Proxy.IsEmpty() )  url.SetProxy( Proxy );
      
    if( Update )  {
      bool skip = (extensionsToSkip.IsEmpty() && filesToSkip.IsEmpty());
      UpdateInstallationH( url, props, skip ? NULL : &toSkip );
    }
  }
  if( Update )  { // have to save lastupdate in anyway
    settings.UpdateParam("lastupdate", TETime::EpochTime() );
    settings.SaveSettings( SettingsFile );
  }
  if( !ftpToSync.IsEmpty() )  {
    AFileSystem* FS = NULL;
    try  {
      if( syncSrc.Comparei("fs2Ftp") == 0 )  {
        FS = new TOSFileSystem; // local file system
        FS->SetBase( TBasicApp::GetInstance()->BaseDir() );
        FS->SetBase( TEFile::AddTrailingBackslash( FS->GetBase() ) );
      }
      else if( syncSrc.Comparei("http2Ftp") == 0 )  {
        TUrl url(Repository);
        if( !Proxy.IsEmpty() )  url.SetProxy( Proxy );
        FS = new TwxHttpFileSystem(url);    
      }
    }
    catch(...)  {
      TBasicApp::GetLog() << "Was unable to initialise source file system to syncronise an ftp mirror\n";
    }
    if( FS != NULL )  {
      try  {
        TwxFtpFileSystem ftp(ftpToSync, ftpUser, ftpPwd, NULL);  
        UpdateMirror(*FS, ftp);
      }
      catch(...)  {
        TBasicApp::GetLog() << "An error accured while synchonising the ftp mirror\n";
      }
      delete FS;
    }
  }
}

