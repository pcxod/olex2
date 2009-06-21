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

#if defined(__WIN32__) && !defined(__WXWIDGETS__)
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
#ifdef __WXWIDGETS__
  #include "wxftpfs.h"
#endif

using namespace updater;

//..............................................................................
short UpdateAPI::DoUpdate(AActionHandler* _f_lsnr, AActionHandler* _p_lsnr)  {
  CleanUp(_f_lsnr, _p_lsnr); 
  if( !settings.IsValid() )  {
    log.Add("Invalid settings file: ") << settings.source_file;
    return uapi_NoSettingsFile;
  }
  SettingsFile& sf = settings;
  TFSItem::SkipOptions toSkip;
  toSkip.extsToSkip = sf.extensions_to_skip.IsEmpty() ? NULL : &sf.extensions_to_skip;
  toSkip.filesToSkip = sf.files_to_skip.IsEmpty() ? NULL : &sf.files_to_skip;

  // evaluate properties
  TStrList props;
  short res = updater::uapi_NoSource;
  EvaluateProperties(props);
  AFileSystem* srcFS = GetRepositoryFS(sf, &res, &log);
  if( srcFS != NULL )  { // have to save lastupdate in anyway
    bool skip = (toSkip.extsToSkip == NULL && toSkip.filesToSkip == NULL);
    short res = _Update(*srcFS, props, skip ? NULL : &toSkip);
    delete srcFS;
    if( res == updater::uapi_OK )  {
      sf.last_updated = TETime::EpochTime();
      sf.Save();
    }
    return res;
  }
  else
    return res;
}
//.............................................................................
short UpdateAPI::DoInstall(AActionHandler* download_lsnr, AActionHandler* extract_lsnr, const olxstr& repo)  {
  CleanUp(download_lsnr, extract_lsnr);
  if( !IsInstallRequired() )  return updater::uapi_OK;
  // check for local installation
  olxstr src_fn = GetInstallationFileName(),
         inst_zip_fn = TBasicApp::GetInstance()->BaseDir() + src_fn;
  if( (repo.IsEmpty() || repo == inst_zip_fn) && TEFile::Exists(inst_zip_fn) )  {
    try  {
      ZipFS zfs(inst_zip_fn, false);
      if( p_lsnr != NULL )  {
        zfs.OnProgress->Add(p_lsnr);
        p_lsnr = NULL;
      }
      zfs.ExtractAll(TBasicApp::GetInstance()->BaseDir());
      return updater::uapi_OK;
    }
    catch( const TExceptionBase& exc )  {
      log.Add( exc.GetException()->GetFullMessage() );
      return updater::uapi_InstallError;
    }
  }
  AFileSystem* fs = repo.IsEmpty() ? GetRepositoryFS(&settings.repository) 
    : FSFromString(repo, settings.GetProxyUrlStr());
  if( fs == NULL )  return updater::uapi_NoSource;
  if( f_lsnr != NULL )  {
    fs->OnProgress->Add(f_lsnr);
    f_lsnr = NULL;
  }
  IInputStream* src_s = NULL;
  try {  src_s = fs->OpenFile( fs->GetBase() + src_fn ); }
  catch( const TExceptionBase& exc )  {
    log.Add( exc.GetException()->GetFullMessage() );
    delete fs;
    return updater::uapi_InvaildRepository;
  }
  if( src_s == NULL )  {
    delete fs;
    return updater::uapi_InvaildRepository;
  }
  try  {
    src_fn = TBasicApp::GetInstance()->BaseDir() + src_fn; 
    TEFile src_f(src_fn, "w+b");
    src_f << *src_s;
    delete src_s;
    src_s = NULL;
    src_f.Close();
    {  // make sure the zipfs goes before deleting the file
      ZipFS zfs(src_fn, false);
      if( p_lsnr != NULL )  {
        zfs.OnProgress->Add(p_lsnr);
        p_lsnr = NULL;
      }
      zfs.ExtractAll(TBasicApp::GetInstance()->BaseDir());
    }
    //TEFile::DelFile(src_fn);
    delete fs;
    fs = NULL;
    settings.last_updated = TETime::EpochTime();
    settings.update_interval = "Always";
    settings.Save();
  }
  catch( const TExceptionBase& exc )  {
    log.Add( exc.GetException()->GetFullMessage() );
    if( fs != NULL )     delete fs;
    if( src_s != NULL )  delete src_s;
    return updater::uapi_InstallError;
  }
  return updater::uapi_OK;
}
//.............................................................................
short UpdateAPI::InstallPlugin(AActionHandler* d_lsnr, AActionHandler* e_lsnr, const olxstr& name) {
  CleanUp(d_lsnr, e_lsnr); 
  AFileSystem* fs = GetRepositoryFS();
  if( fs == NULL )
    return updater::uapi_NoSource;
  if( f_lsnr != NULL )  {
    fs->OnProgress->Add(f_lsnr);
    f_lsnr = NULL;
  }
  olxstr zip_fn( olxstr("/") << 
    TEFile::UnixPath(TEFile::ParentDir(fs->GetBase())) << name << ".zip" );
  IInputStream* is = NULL;
  try { is = fs->OpenFile(zip_fn);  }
  catch( const TExceptionBase& exc )  {
    log.Add( exc.GetException()->GetFullMessage() );
    delete fs;
    return updater::uapi_InvaildRepository;
  }
  try  {
    zip_fn = TBasicApp::GetInstance()->BaseDir() + name; 
    TEFile src_f(zip_fn, "w+b");
    src_f << *is;
    delete is;
    is = NULL;
    src_f.Close();
    {  // make sure the zipfs goes before deleting the file
      ZipFS zfs(zip_fn, false);
      if( p_lsnr != NULL )  {
        zfs.OnProgress->Add(p_lsnr);
        p_lsnr = NULL;
      }
      zfs.ExtractAll(TBasicApp::GetInstance()->BaseDir());
    }
    //TEFile::DelFile(src_fn);
    delete fs;
    fs = NULL;
  }
  catch( const TExceptionBase& exc )  {
    log.Add( exc.GetException()->GetFullMessage() );
    if( fs != NULL )     delete fs;
    if( is != NULL )  delete is;
    return updater::uapi_InstallError;
  }
  return updater::uapi_OK;
}
//.............................................................................
short UpdateAPI::DoSynch(AActionHandler* _f_lsnr, AActionHandler* _p_lsnr)  {
  CleanUp(_f_lsnr, _p_lsnr); 
  if( !settings.IsValid() )  {
    log.Add("Invalid settings file: ") << settings.source_file;
    return uapi_NoSettingsFile;
  }
  const SettingsFile& sf = settings;
  if( sf.dest_repository.IsEmpty() || sf.src_for_dest.IsEmpty() )
    return updater::uapi_NoTask;
  AFileSystem* srcFS = NULL;
  if( sf.src_for_dest.Equalsi("local") )
    srcFS = new TOSFileSystem(TBasicApp::GetInstance()->BaseDir());
  else if( sf.src_for_dest.Equalsi("remote") )
    srcFS = FSFromString( sf.GetRepositoryUrlStr(), sf.GetProxyUrlStr() );
  
  if( srcFS == NULL )  {
    log.Add("Could not locate source for syncronisation");
    return updater::uapi_NoSource;
  }
  AFileSystem* destFS = FSFromString( sf.GetDestinationUrlStr(), sf.GetProxyUrlStr() );
  if( destFS == NULL )  {
    delete srcFS;
    return updater::uapi_NoDestination;
  }
  short res = _Update(*srcFS, *destFS);
  delete srcFS;
  delete destFS;
  return res;
}
//.............................................................................
void UpdateAPI::EvaluateProperties(TStrList& props) const  {
  props.Add("olex-update");
#ifdef __WIN32__
  props.Add("port-win32");
#else
  if( !settings.olex2_port.IsEmpty() )  {
    props.Add(settings.olex2_port);
    log.Add("Portable executable update tag: ") << settings.olex2_port;
  }
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
    catch(...)  {}  // unlucky
  }
}
//.............................................................................
AFileSystem* UpdateAPI::FSFromString(const olxstr& _repo, const olxstr& _proxy)  {
  if( _repo.IsEmpty() )  return NULL;
  AFileSystem* FS = NULL;
  olxstr repo = _repo;
  if( TEFile::Exists(repo) )  {
    if( TEFile::ExtractFileExt(repo).Equalsi("zip") )  {
      if( !TEFile::IsAbsolutePath(repo) )
        repo = TBasicApp::GetInstance()->BaseDir() + repo;
      FS = new ZipFS(repo, false);
    }
    else if( TEFile::IsDir(repo) )
      FS = new TOSFileSystem(repo);
  }
  else  {
    TUrl url(_repo);
    if( !_proxy.IsEmpty() )
      url.SetProxy( _proxy );
    if( url.GetProtocol() == "http" )
      FS = new HttpFS(url);
#ifdef __WXWIDGETS__
    else if( url.GetProtocol() == "ftp" )
      FS = new TwxFtpFileSystem(url);
#endif
  }
  return FS;
}
//.............................................................................
AFileSystem* UpdateAPI::GetRepositoryFS(const SettingsFile& sf, short* status, TStrList* log)  {
  AFileSystem* FS = NULL;
  olxstr repo = sf.repository;
  if( TEFile::Exists(repo) )  {
    if( TEFile::ExtractFileExt(repo).Equalsi("zip") )  {
      if( !TEFile::IsAbsolutePath(repo) )
        repo = TBasicApp::GetInstance()->BaseDir() + repo;
      if( TEFile::FileAge(repo) > sf.last_updated )
        FS = new ZipFS(repo, false);
    }
    else if( TEFile::IsDir(repo) )
      FS = new TOSFileSystem(repo);
  }
  else  {
    bool update = false;
    if( sf.update_interval.IsEmpty() || sf.update_interval.Equalsi("Always") )  
      update = true;
    else if( sf.update_interval.Equalsi("Daily") )
      update = ((TETime::EpochTime() - sf.last_updated ) > SecsADay );
    else if( sf.update_interval.Equalsi("Weekly") )
      update = ((TETime::EpochTime() - sf.last_updated ) > SecsADay*7 );
    else if( sf.update_interval.Equalsi("Monthly") )
      update = ((TETime::EpochTime() - sf.last_updated ) > SecsADay*30 );

    if( update )  {
      try  {
        if( !repo.EndsWith('/') )
          repo << '/';
        if( !repo.EndsWith("update/") )
          repo << "update/";
        TUrl url(repo);
        url.SetUser(sf.repository_user);
        url.SetPassword(sf.repository_pswd);
        if( !sf.proxy.IsEmpty() )  {
          TUrl proxy( sf.proxy );
          proxy.SetUser( sf.proxy_user );
          proxy.SetPassword( sf.proxy_pswd );
          url.SetProxy( proxy );
        }
        if( url.GetProtocol() == "http" )
          FS = new HttpFS(url);
#ifdef __WXWIDGETS__
        else if( url.GetProtocol() == "ftp" )
          FS = new TwxFtpFileSystem(url);
#endif
      }
      catch( const TExceptionBase& exc )  {
        if( log != NULL )
          log->Add( exc.GetException()->GetFullMessage() );
        return NULL;
      }
    }
    else if( status != NULL )
      *status = updater::uapi_UptoDate;
  }
  return FS;
}
//.............................................................................
AFileSystem* UpdateAPI::GetRepositoryFS(olxstr* repo_name) const  {
  olxstr repo, def_repo("http://www.olex2.org/olex2-distro/");
  //olxstr repo, def_repo("http://www.olex2.org/olex-distro-test/");
  TStrList repositories;
  olxstr mirrors_fn = GetMirrorsFileName();
  if( TEFile::Exists(mirrors_fn) )
    repositories.LoadFromFile(mirrors_fn);
  if( repositories.IndexOf(def_repo) == -1 )
    repositories.Insert(0, def_repo);
  if( settings.IsValid() && !settings.repository.IsEmpty() )  {
    int ind = repositories.IndexOf(settings.repository);
    if( ind != -1 && ind != 0 )
      repositories.Delete(ind);
    repositories.Insert(0, settings.repository);
  }
  olxstr proxy_url = settings.GetProxyUrlStr();
  for( int i=0; i < repositories.Count(); i++ )  {
    AFileSystem* fs = FSFromString(repositories[i], proxy_url);
    if( fs != NULL )  {
      if( repo_name != NULL )
        *repo_name = repositories[i];
      return fs;
    }
  }
  return NULL;
}
//.............................................................................
void UpdateAPI::GetAvailableRepositories(TStrList& res) const {
  olxstr repo_name, 
         inst_zip_fn = TBasicApp::GetInstance()->BaseDir() + GetInstallationFileName();
  if( TEFile::Exists(inst_zip_fn) )  
    res.Add( inst_zip_fn );
  AFileSystem* fs = GetRepositoryFS(&repo_name);
  if( fs == NULL )  return;
  IInputStream* is= NULL;
  try  { is = fs->OpenFile(fs->GetBase() + GetTagsFileName());  }
  catch( const TExceptionBase& exc )  {
    log.Add( exc.GetException()->GetFullMessage() );
    delete fs;
    return;
  }
  if( is == NULL )  {
    delete fs;
    return;
  }
  res.LoadFromTextStream(*is);
  delete is;
  delete fs;
  for( int i=0; i < res.Count(); i++ )
    res[i] = repo_name + res[i];
  // LoadFromTextStream clears the list...
  if( TEFile::Exists(inst_zip_fn) )  
    res.Insert( 0, inst_zip_fn );
}
/////////////////////////////////////////////////////////////////////////
SettingsFile::SettingsFile(const olxstr& file_name) : source_file(file_name)  {
  if( !TEFile::Exists(file_name) )
    return;
  const TSettingsFile settings(file_name);
  proxy = settings["proxy"];
  proxy_user = settings["proxy_user"];
  proxy_pswd = settings["proxy_passwd"];
  repository = settings["repository"];
  repository_user = settings["repository_user"];
  repository_pswd = settings["repository_passwd"];
  update_interval = settings["update"];
  const olxstr& last_update_str = settings.GetParam("lastupdate", "0");
  if( last_update_str.IsEmpty() )
    last_updated = 0;
  else
    last_updated = last_update_str.RadInt<int64_t>();
  extensions_to_skip.Strtok(settings["exceptions"], ';');
  dest_repository = settings["dest_repository"];
  dest_user = settings["dest_user"];
  dest_pswd = settings["dest_passwd"];
  src_for_dest = settings["src_for_dest"];
  files_to_skip.Strtok(settings["skip"], ';');
  olex2_port = settings["olex-port"];
}
//.......................................................................
olxstr SettingsFile::GetRepositoryUrlStr() const  {
  if( repository.IsEmpty() )
    return EmptyString;
  olxstr rv;
  if( !repository_user.IsEmpty() )
    rv << repository_user << '@';
  if( !repository_pswd.IsEmpty() )
    rv << ':' << repository_pswd;
  return rv << repository;
}
//.......................................................................
olxstr SettingsFile::GetDestinationUrlStr() const  {
  if( dest_repository.IsEmpty() )
    return EmptyString;
  olxstr rv;
  if( !dest_user.IsEmpty() )
    rv << dest_user << '@';
  if( !dest_pswd.IsEmpty() )
    rv << ':' << dest_pswd;
  return rv << dest_repository;
}
//.......................................................................
olxstr SettingsFile::GetProxyUrlStr() const  {
  if( proxy.IsEmpty() )
    return EmptyString;
  olxstr rv;
  if( !proxy_user.IsEmpty() )
    rv << proxy_user << '@';
  if( !proxy_pswd.IsEmpty() )
    rv << ':' << proxy_pswd;
  return rv << proxy;
}
//.......................................................................
void SettingsFile::Save() {
  TSettingsFile settings;
  settings["proxy"] = proxy;
  settings["proxy_user"] = proxy_user;
  settings["proxy_passwd"] = proxy_pswd;
  // a bit of hack here...
  if( repository.EndsWith("update/") )
    repository.SetLength(repository.Length()-7);
  settings["repository"] = repository;
  settings["repository_user"] = repository_user;
  settings["repository_passwd"] = repository_pswd;
  settings["update"] = update_interval;
  settings["lastupdate"] = last_updated;
  settings["exceptions"] = extensions_to_skip.Text(';');
  settings["dest_repository"] = dest_repository;
  settings["dest_user"] = dest_user;
  settings["dest_passwd"] = dest_pswd;
  settings["src_for_dest"] = src_for_dest;
  settings["skip"] = files_to_skip.Text(';');
  settings["olex-port"] = olex2_port;
  settings.SaveSettings(source_file);
}
//.......................................................................
