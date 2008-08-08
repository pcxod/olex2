#include <stdio.h>

#include "wx/app.h"

#include "efile.h"
#include "bapp.h"
#include "log.h"
#include "outstream.h"
#include "efile.h"
#include "wxzipfs.h"
#ifdef __WIN32__
  #include "winzipfs.h"
  #include "winhttpfs.h"
#endif
#include "etime.h"
#include "settingsfile.h"
#include "datafile.h"
#include "dataitem.h"
#include "wxhttpfs.h"

class TProgress: public AActionHandler  {
  inline bool IsFSSender(const IEObject* obj) const {
//#ifdef __WIN32__
//    if( obj != NULL && (EsdlInstanceOf(*obj, TWinHttpFileSystem) || 
//                        EsdlInstanceOf(*obj, TWinZipFileSystem) )
//                        ) return true;
//#else
    if( obj != NULL && (EsdlInstanceOf(*obj, TwxHttpFileSystem) || 
                        EsdlInstanceOf(*obj, TwxZipFileSystem)  )
                        ) return true;
//#endif
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

bool UpdateInstallationH( const TUrl& url, const TStrList& properties )  {
  try  {
    TOSFileSystem DestFS; // local file system
    TwxHttpFileSystem SrcFS( url ); // remote FS
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
    TBasicApp::GetLog() << "Update failed due to :\n" << out.Text('\n');
    return false;
  }
}
//---------------------------------------------------------------------------
bool UpdateInstallationZ( const olxstr& zip_name, const TStrList& properties )  {
  try  {
    TOSFileSystem DestFS; // local file system
    TwxZipFileSystem SrcFS(zip_name, false);

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
    TBasicApp::GetLog() << "Update failed due to :\n" << out.Text('\n');
    return false;
  }
}

void DoRun(const olxstr& basedir);

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
  wxAppConsole::SetInstance(&app);
  DoRun( ((argc <= 1) ? olxstr(TEFile::CurrentDir())  : olxstr(argv[1])) << "/dummy.txt" );
  return 0;
}

void DoRun(const olxstr& basedir)  {
  TBasicApp bapp(  basedir );
  bapp.GetLog().AddStream( new TOutStream, true);

  bapp.OnProgress->Add( new TProgress );

  olxstr SettingsFile( TBasicApp::GetInstance()->BaseDir() + "usettings.dat" );
  TSettingsFile settings;
  if( !TEFile::FileExists(SettingsFile) )  {
    TBasicApp::GetLog() << "Could not locate settings file" << SettingsFile << '\n';
    return;
  }
  olxstr Proxy, Repository = "http://dimas.dur.ac.uk/olex-distro/update",
           UpdateInterval = "Always";
  int LastUpdate = 0;
  settings.LoadSettings( SettingsFile );
  if( settings.ParamExists("proxy") )
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
   
  bool Update = false;
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
          props.Add( PluginItem->Item(i).GetName() );
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
      UpdateInstallationZ( Repository, props );
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
      UpdateInstallationH( url, props );
  }
  if( Update )  { // have to save lastupdate in anyway
    settings.UpdateParam("lastupdate", TETime::EpochTime() );
    settings.SaveSettings( SettingsFile );
  }
}