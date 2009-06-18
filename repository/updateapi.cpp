#ifdef __BORLANC__
  #pragma hdrstop
#endif

#include "updateapi.h"
#include "log.h"
#include "efile.h"
#include "filesystem.h"
#include "settingsfile.h"
#include "url.h"
#include "datafile.h"
#include "dataitem.h"

//#define __WXWIDGETS__

#ifdef __WIN32__
  #include "winhttpfs.h"
  #include "winzipfs.h"
  typedef TWinHttpFileSystem HttpFS;
  typedef TWinZipFileSystem ZipFS;
#else
  #include "wxhttpfs.h"
  #include "wxzipfs.h"
  typedef TwxHttpFileSystem HttpFS;
  typedef TwxZipFileSystem ZipFS;
#endif

using namespace updater;

//..............................................................................
short UpdateAPI::DoUpdate(AActionHandler* f_lsnr, AActionHandler* p_lsnr)  {
  TBasicApp& bapp = *TBasicApp::GetInstance();

  olxstr SettingsFile( bapp.BaseDir() + "usettings.dat" );
  TSettingsFile settings;
  if( !TEFile::Exists(SettingsFile) )  {
    log.Add("Could not locate settings file: ") << SettingsFile;
    return uapi_NoSettingsFile;
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
    log.Add("Skipping the following extensions: ") << extensionsToSkip.Text(' ');
  }
  if( settings.ParamExists("ftpToSync") )  {
    TStrList toks(settings.ParamValue("ftpToSync", EmptyString), ';');
    ftpToSync = toks[0];
    if( toks.Count() > 1 )  ftpUser = toks[1];
    if( toks.Count() > 2 )  ftpPwd = toks[2];
  #ifdef __WXWIDGETS__
    log.Add("Synchronising the following ftp mirror: ") << ftpToSync;
  #endif
  }
  syncSrc = settings.ParamValue("sync", "fs2Ftp");

  if( settings.ParamExists("skip") )  {
    filesToSkip.Strtok(settings.ParamValue("skip", EmptyString), ';');
    log.Add("Skipping the following files: ") << filesToSkip.Text(' ');
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
    log.Add("Portable executable update tag: ") << olex_port;
  }
  // end
#endif
  olxstr pluginFile = bapp.BaseDir() + "plugins.xld";
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
        Repository = bapp.BaseDir() + Repository;
      if( TEFile::FileAge(Repository) > LastUpdate )  {
        Update = true;
        srcFS = new ZipFS(Repository, false);
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
      srcFS = new HttpFS(url);
    }
  }
  if( Update && srcFS != NULL )  { // have to save lastupdate in anyway
    bool skip = (extensionsToSkip.IsEmpty() && filesToSkip.IsEmpty());
    UpdateInstallation(*srcFS, props, skip ? NULL : &toSkip, f_lsnr, p_lsnr);
    delete srcFS;
    settings.UpdateParam("lastupdate", TETime::EpochTime() );
    settings.SaveSettings( SettingsFile );
  }
  #ifdef __WXWIDGETS__
  short res = updater::uapi_OK;
  if( !ftpToSync.IsEmpty() )  {
    AFileSystem* FS = NULL;
    try  {
      if( syncSrc.Equalsi("fs2Ftp") )
        FS = new TOSFileSystem(bapp.BaseDir()); // local file system
      else if( syncSrc.Equalsi("http2Ftp") )  {
        TUrl url(Repository);
        if( !Proxy.IsEmpty() )
          url.SetProxy( Proxy );
        FS = new HttpFS(url);
      }
    }
    catch(const TExceptionBase& exc)  {
      if( FS != NULL )  {
        delete FS;
        FS = NULL;
      }
      res = updater::uapi_FTP_Error;
      log.Add( exc.GetException()->GetFullMessage() );
    }
    if( FS != NULL )  {
      try  {
        TwxFtpFileSystem ftp(ftpToSync, ftpUser, ftpPwd, NULL);
        UpdateMirror(*FS, ftp, f_lsnr, p_lsnr);
      }
      catch(const TExceptionBase& exc)  {
        res = updater::uapi_FTP_Error;
        log.Add( exc.GetException()->GetFullMessage() );
      }
      delete FS;
    }
  }
#endif
  return res;
}



