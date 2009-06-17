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
#include "utf8file.h"
#include <iostream>
using namespace std;

class TProgress: public AActionHandler  {
public:
  TProgress(){  return; }
  ~TProgress(){  return; }
  bool Exit(const IEObject *Sender, const IEObject *Data)  {
    //TBasicApp::GetLog() << '\n';
    return true;
  }
  bool Enter(const IEObject *Sender, const IEObject *Data)  {
    if( Data == NULL )  {
      return false;
    }
    IEObject *d_p = const_cast<IEObject*>(Data);
    TOnProgress *A = dynamic_cast<TOnProgress *>(d_p);
    TBasicApp::GetLog().Info( A->GetAction() );
    return true;
  }
  bool Execute(const IEObject *Sender, const IEObject *Data)  {
    if( !EsdlInstanceOf( *Data, TOnProgress) )
      throw TInvalidArgumentException(__OlxSourceInfo, "invalid type");
    return true;
  }
};

bool UpdateMirror( AFileSystem& src, TwxFtpFileSystem& dest )  {
  try  {
    TFSIndex FI( src );
    dest.OnProgress->Add( new TProgress );
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
bool UpdateInstallation( AFileSystem& SrcFS, const TStrList& properties, const TFSItem::SkipOptions* toSkip )  {
  try  {
    TOSFileSystem DestFS(TBasicApp::GetInstance()->BaseDir()); // local file system
    TFSIndex FI( SrcFS );
    SrcFS.OnProgress->Add( new TProgress );
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
        log << "(c) Oleg V. Dolomanov 2007-2009\n";
        return 0;
      }
      if( arg.EndsWith('.') || arg.EndsWith("..") )
        arg = TEFile::AbsolutePathTo(TEFile::CurrentDir(), arg);
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
  if( bapp != NULL )  {
    bapp->GetLog() << "\nFinished\n";
    delete bapp;
  }
  else  {
    cout << "\nFinished\n";
  }
  return res;
}

void DoRun()  {
  TBasicApp& bapp = *TBasicApp::GetInstance();
  if( &bapp == NULL )
    return;

  olxstr SettingsFile( TBasicApp::GetInstance()->BaseDir() + "usettings.dat" );
  TSettingsFile settings;
  if( !TEFile::Exists(SettingsFile) )  {
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
  if( !TEFile::ExtractFileExt(Repository).Equalsi("zip") )
    if( Repository.Length() && !Repository.EndsWith('/') )
      Repository << '/';

  toSkip.extsToSkip = extensionsToSkip.IsEmpty() ? NULL : &extensionsToSkip;
  toSkip.filesToSkip = filesToSkip.IsEmpty() ? NULL : &filesToSkip;
   
  bool Update = false;
  // evaluate properties
  TStrList props;
  props.Add("olex-update");
#ifdef __WIN32__
  props.Add("port-win32");
#else
  // updating ported executables
  olxstr olex_port = settings.ParamValue("olex-port");
  if( !olex_port.IsEmpty() )  {
    props.Add(olex_port);
    TBasicApp::GetLog() << "Portable executable update tag: " << olex_port << '\n'; 
  }
  // end
#endif
  olxstr pluginFile = TBasicApp::GetInstance()->BaseDir() + "plugins.xld";
  if( TEFile::Exists( pluginFile ) )  {
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
  AFileSystem* srcFS = NULL;
  // end properties evaluation
  if( TEFile::Exists(Repository) )  {
    if( TEFile::ExtractFileExt(Repository).Equalsi("zip") )  {
      if( !TEFile::IsAbsolutePath(Repository) )
        Repository = TBasicApp::GetInstance()->BaseDir() + Repository;
      if( TEFile::FileAge(Repository) > LastUpdate )  {
        Update = true;
        srcFS = new TwxZipFileSystem(Repository, false);
      }
    }
    else if( TEFile::IsDir(Repository) )  {
      Update = true;
      srcFS = new TOSFileSystem(Repository);
    }
  }
  else  {
    if( UpdateInterval.Equalsi("Always") )  Update = true;
    else if( UpdateInterval.Equalsi("Daily") )
      Update = ((TETime::EpochTime() - LastUpdate ) > SecsADay );
    else if( UpdateInterval.Equalsi("Weekly") )
      Update = ((TETime::EpochTime() - LastUpdate ) > SecsADay*7 );
    else if( UpdateInterval.Equalsi("Monthly") )
      Update = ((TETime::EpochTime() - LastUpdate ) > SecsADay*30 );
     
    if( Update )  {
      TUrl url(Repository);
      url.SetUser(ProxyUser);
      url.SetPassword(ProxyPasswd);
      if( !Proxy.IsEmpty() )  
        url.SetProxy( Proxy );
      srcFS = new TwxHttpFileSystem(url);
    }
  }
  if( Update && srcFS != NULL )  { // have to save lastupdate in anyway
    bool skip = (extensionsToSkip.IsEmpty() && filesToSkip.IsEmpty());
    UpdateInstallation(*srcFS, props, skip ? NULL : &toSkip);
    delete srcFS;
    settings.UpdateParam("lastupdate", TETime::EpochTime() );
    settings.SaveSettings( SettingsFile );
  }
  if( !ftpToSync.IsEmpty() )  {
    AFileSystem* FS = NULL;
    try  {
      if( syncSrc.Equalsi("fs2Ftp") )
        FS = new TOSFileSystem(TBasicApp::GetInstance()->BaseDir()); // local file system
      else if( syncSrc.Equalsi("http2Ftp") )  {
        TUrl url(Repository);
        if( !Proxy.IsEmpty() )  
          url.SetProxy( Proxy );
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

