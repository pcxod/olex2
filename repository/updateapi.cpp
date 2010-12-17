#include "updateapi.h"
#include "log.h"
#include "efile.h"
#include "filesystem.h"
#include "settingsfile.h"
#include "url.h"
#include "datafile.h"
#include "dataitem.h"
#include "patchapi.h"

#include "cdsfs.h"

#if defined(__WIN32__) && !defined(__WXWIDGETS__)
  #include "winzipfs.h"
  typedef TWinZipFileSystem ZipFS;
#else
  #include "wxzipfs.h"
  typedef TwxZipFileSystem ZipFS;
#endif
#ifdef __WXWIDGETS__
  #include "wxftpfs.h"
#endif

using namespace updater;
using namespace patcher;
const olxstr UpdateAPI::new_installation_fn("new_installation");
//..............................................................................
UpdateAPI::UpdateAPI() : f_lsnr(NULL), p_lsnr(NULL), 
  settings(GetSettingsFileName()), Tag( patcher::PatchAPI::ReadRepositoryTag() )  {  }
//..............................................................................
short UpdateAPI::DoUpdate(AActionHandler* _f_lsnr, AActionHandler* _p_lsnr)  {
  CleanUp(_f_lsnr, _p_lsnr); 
  if( !settings.IsValid() )  {
    log.Add("Invalid settings file: ") << settings.source_file;
    return uapi_NoSettingsFile;
  }
  if( Tag.IsEmpty() )
    return updater::uapi_InvalidInstallation;
  if( !PatchAPI::LockUpdater() )
    return updater::uapi_Busy;
  SettingsFile& sf = settings;
  TFSItem::SkipOptions toSkip;
  toSkip.extsToSkip = sf.extensions_to_skip.IsEmpty() ? NULL : &sf.extensions_to_skip;
  toSkip.filesToSkip = sf.files_to_skip.IsEmpty() ? NULL : &sf.files_to_skip;

  short res = updater::uapi_NoSource;
  AFileSystem* srcFS = FindActiveUpdateRepositoryFS(&res);
  if( srcFS == NULL )  return res;
  srcFS->SetBase( AddTagPart(srcFS->GetBase(), false) );
  // evaluate properties
  TStrList props;
  EvaluateProperties(props);
  bool skip = (toSkip.extsToSkip == NULL && toSkip.filesToSkip == NULL);
  res = _Update(*srcFS, props, skip ? NULL : &toSkip);
  delete srcFS;
  if( res == updater::uapi_OK )  {
    sf.last_updated = TETime::EpochTime();
    sf.Save();
  }
  PatchAPI::UnlockUpdater();
  return res;
}
//.............................................................................
short UpdateAPI::DoInstall(AActionHandler* download_lsnr, AActionHandler* extract_lsnr, const olxstr& repo)  {
  CleanUp(download_lsnr, extract_lsnr);
  if( !IsInstallRequired() )  return updater::uapi_OK;
  if( repo.IsEmpty() )
    return updater::uapi_InvaildRepository;
  if( !TBasicApp::GetInstance().IsBaseDirWriteable() )
    return updater::uapi_AccessDenied;
  // check for local installation
  olxstr src_fn = GetInstallationFileName(),
         inst_zip_fn = TBasicApp::GetBaseDir() + src_fn;
  if( repo == inst_zip_fn && TEFile::Exists(inst_zip_fn) )  {
    try  {
      if( !PatchAPI::LockUpdater() )
        return updater::uapi_Busy;
      ZipFS zfs(inst_zip_fn, false);
      if( p_lsnr != NULL )  {
        zfs.OnProgress.Add(p_lsnr);
        p_lsnr = NULL;
      }
      if( !zfs.Exists(patcher::PatchAPI::GetTagFileName()) )
        return updater::uapi_InvaildRepository;
      zfs.ExtractAll(TBasicApp::GetBaseDir());
      settings.repository = GetDefaultRepositories()[0];
      settings.last_updated = TETime::EpochTime();
      settings.Save();
      PatchAPI::UnlockUpdater();
      return updater::uapi_OK;
    }
    catch( const TExceptionBase& exc )  {
      log.Add( exc.GetException()->GetFullMessage() );
      PatchAPI::UnlockUpdater();
      return updater::uapi_InstallError;
    }
  }
  if( !PatchAPI::LockUpdater() )
    return updater::uapi_Busy;
  AFileSystem* fs = FSFromString(repo, settings.proxy);
  if( fs == NULL )  {
    PatchAPI::UnlockUpdater();
    return updater::uapi_NoSource;
  }
  if( f_lsnr != NULL )  {
    fs->OnProgress.Add(f_lsnr);
    f_lsnr = NULL;
  }
  IInputStream* src_s = NULL;
  short res = updater::uapi_OK;
  try {  src_s = fs->OpenFile( fs->GetBase() + src_fn ); }
  catch( const TExceptionBase& exc )  {
    log.Add( exc.GetException()->GetFullMessage() );
    res = updater::uapi_InvaildRepository;
  }
  if( src_s == NULL )
    res = updater::uapi_InvaildRepository;
  
  if( res != updater::uapi_OK )  {
    delete fs;
    PatchAPI::UnlockUpdater();
    return res;
  }
  try  {
    src_fn = TBasicApp::GetBaseDir() + src_fn; 
    TEFile src_f(src_fn, "w+b");
    src_f << *src_s;
    delete src_s;
    src_s = NULL;
    src_f.Close();
    {  // make sure the zipfs goes before deleting the file
      ZipFS zfs(src_fn, false);
      if( p_lsnr != NULL )  {
        zfs.OnProgress.Add(p_lsnr);
        p_lsnr = NULL;
      }
      zfs.ExtractAll(TBasicApp::GetBaseDir());
    }
    delete fs;
    fs = NULL;
    TEFile::DelFile(src_fn);
    settings.repository = TrimTagPart(repo);
    settings.last_updated = TETime::EpochTime();
    settings.update_interval = "Always";
    settings.Save();
  }
  catch( const TExceptionBase& exc )  {
    log.Add( exc.GetException()->GetFullMessage() );
    res = updater::uapi_InstallError;
  }
  if( fs != NULL )     delete fs;
  if( src_s != NULL )  delete src_s;
  PatchAPI::UnlockUpdater();
  return res;
}
//.............................................................................
short UpdateAPI::InstallPlugin(AActionHandler* d_lsnr, AActionHandler* e_lsnr, const olxstr& name) {
  CleanUp(d_lsnr, e_lsnr); 
  if( !TBasicApp::GetInstance().IsBaseDirWriteable() )
    return updater::uapi_AccessDenied;
  if( Tag.IsEmpty() )
    return updater::uapi_InvalidInstallation;
  if( !PatchAPI::LockUpdater() )
    return updater::uapi_Busy;
  AFileSystem* fs = FindActiveRepositoryFS();
  if( fs == NULL )  {
    PatchAPI::UnlockUpdater();
    return updater::uapi_NoSource;
  }
  if( f_lsnr != NULL )  {
    fs->OnProgress.Add(f_lsnr);
    f_lsnr = NULL;
  }
  fs->SetBase( AddTagPart(fs->GetBase(), false) );
  olxstr zip_fn( TEFile::UnixPath(olxstr(fs->GetBase()) << name << ".zip") );
  IInputStream* is = NULL;
  try { is = fs->OpenFile(zip_fn);  }
  catch( const TExceptionBase& exc )  {
    log.Add( exc.GetException()->GetFullMessage() );
    delete fs;
    PatchAPI::UnlockUpdater();
    return updater::uapi_InvaildRepository;
  }
  if( is == NULL )  {
    delete fs;
    PatchAPI::UnlockUpdater();
    return updater::uapi_NoSource;
  }
  try  {
    zip_fn = (TBasicApp::GetBaseDir() + name) << ".zip"; 
    TEFile src_f(zip_fn, "w+b");
    src_f << *is;
    delete is;
    is = NULL;
    src_f.Close();
    {  // make sure the zipfs goes before deleting the file
      ZipFS zfs(zip_fn, false);
      TFSIndex fsi(zfs);
      TOSFileSystem osf(TBasicApp::GetBaseDir());
      osf.RemoveAccessRight(afs_DeleteAccess);
      TStrList props;
      props.Add(olxstr("plugin-") << name);
      if( p_lsnr != NULL )  {
        fsi.OnProgress.Add(p_lsnr);
        p_lsnr = NULL;
      }
      fsi.Synchronise(osf, props);
      //zfs.ExtractAll(TBasicApp::GetBaseDir());
    }
    TEFile::DelFile(zip_fn);
    delete fs;
    fs = NULL;
    PatchAPI::UnlockUpdater();
  }
  catch( const TExceptionBase& exc )  {
    log.Add( exc.GetException()->GetFullMessage() );
    if( fs != NULL )     delete fs;
    if( is != NULL )  delete is;
    PatchAPI::UnlockUpdater();
    return updater::uapi_InstallError;
  }
  PatchAPI::UnlockUpdater();
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
    srcFS = new TOSFileSystem(TBasicApp::GetBaseDir());
  else if( sf.src_for_dest.Equalsi("remote") )
    srcFS = FSFromString( sf.repository, sf.proxy );
  
  if( srcFS == NULL )  {
    log.Add("Could not locate source for synchronisation");
    return updater::uapi_NoSource;
  }
  AFileSystem* destFS = FSFromString( sf.dest_repository, sf.proxy );
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
#if defined(__WIN32__) //&& !defined(_DEBUG)  // disable updates by the debug version 
#  if defined(_DEBUG)
#    if !defined(_WIN64)
       props.Add("port-win32-portable");  // but can change this ...
       props.Add("port-win32");
#    else
       props.Add("port-win64");
#    endif
#  elif _WIN64
     props.Add("port-win64");
#  else
     props.Add("port-win32-portable");  // but can change this ...
#    if _M_IX86_FP == 0
       props.Add("port-win32-nosse");
#    elif _M_IX86_FP == 1
       props.Add("port-win32-sse");
#    elif _M_IX86_FP == 2  // cannot change it! olex2 does not get upadted and this is it...
       props.Add("port-win32");
#    endif
#  endif
#else
  if( !settings.olex2_port.IsEmpty() )  {
    props.Add(settings.olex2_port);
    log.Add("Portable executable update tag: ") << settings.olex2_port;
  }
#endif
  olxstr pluginFile = TBasicApp::GetBaseDir() + "plugins.xld";
  if( TEFile::Exists( pluginFile ) )  {
    try  {
      TDataFile df;
      df.LoadFromXLFile( pluginFile, NULL );
      TDataItem* PluginItem = df.Root().FindItem("Plugin");
      if( PluginItem != NULL )  {
        for( size_t i=0; i < PluginItem->ItemCount(); i++ )  {
          if( PluginItem->GetItem(i).GetName().StartsFrom("plugin-") )
            props.Add(PluginItem->GetItem(i).GetName());
          else
            props.Add(olxstr("plugin-") << PluginItem->GetItem(i).GetName());
        }
      }
    }
    catch(...)  {}  // unlucky
  }
}
//.............................................................................
AFileSystem* UpdateAPI::FSFromString(const olxstr& _repo, const olxstr& _proxy)  {
  if( _repo.IsEmpty() )  return NULL;
  try  {
    AFileSystem* FS = NULL;
    olxstr repo = _repo;
    if( TEFile::Exists(repo) )  {
      if( TEFile::ExtractFileExt(repo).Equalsi("zip") )  {
        if( !TEFile::IsAbsolutePath(repo) )
          repo = TBasicApp::GetBaseDir() + repo;
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
        FS = new TSocketFS(url);
#ifdef __WXWIDGETS__
      else if( url.GetProtocol() == "ftp" )
        FS = new TwxFtpFileSystem(url);
#endif
    }
    return FS;
	}
	catch(const TExceptionBase& )  {
	  return NULL;
	}
}
//.............................................................................
AFileSystem* UpdateAPI::FindActiveUpdateRepositoryFS(short* res) const {
  if( Tag.IsEmpty() )  {
    if( res != NULL )  *res = updater::uapi_InvalidInstallation;
    return NULL;
  }
  if( res != NULL )  *res = updater::uapi_UptoDate;
  olxstr repo = settings.repository;
  if( TEFile::Exists(repo) )  {
    if( TEFile::ExtractFileExt(repo).Equalsi("zip") )  {
      if( TEFile::FileAge(repo) > settings.last_updated )
        return new ZipFS(repo, false);
    }
    else if( TEFile::IsDir(repo) )
      return new TOSFileSystem(AddTagPart(repo, true));
  }
  else  {
    bool update = false;
    const SettingsFile& sf = settings;
    if( sf.update_interval.IsEmpty() || sf.update_interval.Equalsi("Always") )  
      update = true;
    else if( sf.update_interval.Equalsi("Daily") )
      update = ((TETime::EpochTime() - sf.last_updated ) > SecsADay );
    else if( sf.update_interval.Equalsi("Weekly") )
      update = ((TETime::EpochTime() - sf.last_updated ) > SecsADay*7 );
    else if( sf.update_interval.Equalsi("Monthly") )
      update = ((TETime::EpochTime() - sf.last_updated ) > SecsADay*30 );

    if( update )  {  
      AFileSystem* FS = FindActiveRepositoryFS(&repo, (AddTagPart(EmptyString, true)+"index.ind").SubStringFrom(1));
      if( FS == NULL )  {
        if( res != NULL )  *res = updater::uapi_NoSource;
        return NULL;
      }
      FS->SetBase(AddTagPart(FS->GetBase(), true));
      return FS;
    }
  }
  if( res != NULL )  *res = updater::uapi_UptoDate;
  return NULL;
}
//.............................................................................
AFileSystem* UpdateAPI::FindActiveRepositoryFS(olxstr* repo_name, const olxstr& check_file) const  {
  TStrList repositories;
  GetAvailableMirrors(repositories);
  for( size_t i=0; i < repositories.Count(); i++ )  {
    AFileSystem* fs = FSFromString(repositories[i], settings.proxy);
    if( fs != NULL )  {
#ifdef _DEBUG
      if( !check_file.IsEmpty() )  {
        TBasicApp::NewLogEntry() << "Checking repository: " << repositories[i] <<
          " for file: " << check_file;
      }
#endif
      if( !check_file.IsEmpty() && !fs->Exists(fs->GetBase()+check_file, true) )  {
        delete fs;
        continue;
      }
      if( repo_name != NULL )
        *repo_name = repositories[i];
      return fs;
    }
  }
  return NULL;
}
//.............................................................................
void UpdateAPI::GetAvailableMirrors(TStrList& res) const  {
  olxstr mirrors_fn = GetMirrorsFileName();
  if( TEFile::Exists(mirrors_fn) )
    res.LoadFromFile(mirrors_fn);
  TStrList defs = GetDefaultRepositories();
  for( size_t i=0; i < defs.Count(); i++ )  {
    if( res.IndexOf(defs[defs.Count()-i-1]) == InvalidIndex )
      res.Insert(0, defs[defs.Count()-i-1]);
  }
  if( settings.IsValid() && !settings.repository.IsEmpty() )  {
    const size_t ind = res.IndexOf(settings.repository);
    if( ind != InvalidIndex && ind != 0 )
      res.Delete(ind);
    if( ind != 0 )
      res.Insert(0, settings.repository);
  }
}
//.............................................................................
void UpdateAPI::GetAvailableRepositories(TStrList& res) const {
  olxstr repo_name, 
         inst_zip_fn = TBasicApp::GetBaseDir() + GetInstallationFileName();
  if( TEFile::Exists(inst_zip_fn) )  
    res.Add(inst_zip_fn);
  AFileSystem* fs = FindActiveRepositoryFS(&repo_name, GetTagsFileName());
  if( fs == NULL )  return;
  IInputStream* is= NULL;
  try  { is = fs->OpenFile(fs->GetBase() + GetTagsFileName());  }
  catch( const TExceptionBase& exc )  {
    log.Add(exc.GetException()->GetFullMessage());
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
  for( size_t i=0; i < res.Count(); i++ )
    res[i] = repo_name + res[i];
  // LoadFromTextStream clears the list...
  if( TEFile::Exists(inst_zip_fn) )  
    res.Insert(0, inst_zip_fn);
}
//.............................................................................
void UpdateAPI::GetAvailableTags(TStrList& res, olxstr& repo_name) const {
  AFileSystem* fs = FindActiveRepositoryFS(&repo_name, GetTagsFileName());
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
}
//.............................................................................
olxstr UpdateAPI::TrimTagPart(const olxstr& path) const {
  if( Tag.IsEmpty() )  return path;
  olxstr rv = TEFile::UnixPath(path);
  if( !rv.EndsWith('/') )  rv << '/';
  if( rv.EndsWith("update/") )
    rv.SetLength( rv.Length() - 7 );
  if( rv.EndsWith(Tag+'/') )
    rv.SetLength( rv.Length() - Tag.Length() - 1 );
  return rv;
}
//.............................................................................
olxstr UpdateAPI::AddTagPart(const olxstr& path, bool Update) const {
  if( Tag.IsEmpty() )  return path;
  olxstr rv = TrimTagPart(path);
  rv << Tag << '/';
  if( Update )
    rv << "update/";
  return rv;
}
//.............................................................................
TStrList UpdateAPI::GetDefaultRepositories() {
  static TStrList rv;
  if( rv.IsEmpty() )  {
    rv.Add("http://www.olex2.org/olex2-distro/");
    rv.Add("http://www1.olex2.org/olex2-distro/");
    rv.Add("http://www2.olex2.org/olex2-distro/");
  }
  return rv;
}
//.............................................................................
//http://www.jorgon.freeserve.co.uk/TestbugHelp/XMMfpins2.htm
olxstr UpdateAPI::GetInstallationFileName()  {
#ifdef __WIN32__
#ifndef _WIN64
  try  {
    if( IsWow64() )
      return "olex2-x64.zip";
  }
  catch(...)  {}  // stay quiet (?)
  unsigned int cpu_features = 0;
  _asm  {
    push EAX
    push EBX
    push ECX
    push EDX
      mov EAX, 1
      cpuid
      mov [cpu_features], EDX
    pop EDX
    pop ECX
    pop EBX
    pop EAX
  }
  bool has_sse2 = (cpu_features & (0x1 << 26)) != 0;
  bool has_sse = (cpu_features & (0x1 << 25)) != 0;
  if( has_sse2 ) return "olex2.zip";
  else if( has_sse )  return "olex2-sse.zip";
  else  return "olex2-nosse.zip";
#else
  return "olex2-x64.zip";
#endif
#else
  return "portable-gui.zip";
#endif
}

