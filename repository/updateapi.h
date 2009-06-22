#ifndef __olx_update_api_H
#define __olx_update_api_H

#include "filesystem.h"
#include "bapp.h"
#include "settingsfile.h"

/* a usettings.dat file processor since the launch and unirun do not their original
job now, their API is here, night be useful in the future... */
namespace updater  {

const short
  uapi_OK             = 0,  // operation is done successfully
  uapi_UptoDate       = 1,  // repositories are upto date
  uapi_NoSettingsFile = 2,  // setings file does not exist
  uapi_NoSource       = 3,  // no source is provided
  uapi_NoDestination  = 4,  // destination is not specified
  uapi_UpdateError    = 5,  // error happend while updating
  uapi_NoTask         = 6,  // nor source or destination exists
  uapi_InstallError   = 7,  // an installation error has occured
  uapi_InvaildRepository = 8,  // repository does not contain required files
  uapi_MissingTag     = 9,  // tag is missing on the repo, new isntall might be required
  uapi_AccessDenied   = 10, // the destination folder is not writeable
  uapi_InvalidInstallation = 11,  // required files are missing
  uapi_Busy           = 12;  // updater API is busy

// Olex2 settings file wrapper
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
struct SettingsFile  {
  olxstr repository,  // repository for local update
    repository_user,  // for ftp/http
    repository_pswd,  // for ftp/http
    proxy,            // repository proxy
    proxy_user,       // proxy user name
    proxy_pswd,       // proxy password
    dest_user,        // like ftp user name
    dest_pswd,        // like ftp pswd
    dest_repository,  // like ftp
    src_for_dest,     // source for dest, local or remote (repository)
    update_interval,
    olex2_port;

  const olxstr source_file;

  TStrList extensions_to_skip, files_to_skip;
  time_t last_updated;
  //...................................................................
  SettingsFile(const olxstr& file_name);
  olxstr GetRepositoryUrlStr() const;
  olxstr GetDestinationUrlStr() const;
  olxstr GetProxyUrlStr() const;
  bool IsValid() const {  return TEFile::Exists(source_file);  }
  // save will change repo/update/ to repo...
  void Save();
};

// Olex2 updater/installer API
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

class UpdateAPI  {
  mutable TStrList log;
  AActionHandler *f_lsnr, *p_lsnr;
  void CleanUp(AActionHandler* fl=NULL, AActionHandler* pl=NULL)  {
    if( f_lsnr != NULL )  delete f_lsnr;
    if( p_lsnr != NULL )  delete p_lsnr;
    p_lsnr = pl;
    f_lsnr = fl;
  }
  void InitAQueues(TActionQueue* f, TActionQueue* p)  {
    if( f_lsnr != NULL )  {
      f->Add(f_lsnr);
      f_lsnr = NULL;
    }
    if( p_lsnr != NULL )  {
      p->Add( p_lsnr );
      p_lsnr = NULL;
    }
  }
  //.................................................................................................
  short _Update(AFileSystem& SrcFS, const TStrList& properties, const TFSItem::SkipOptions* toSkip)  {
    try  {
      TOSFileSystem DestFS(TBasicApp::GetInstance()->BaseDir()); // local file system
      TFSIndex FI( SrcFS );
      InitAQueues(SrcFS.OnProgress, FI.OnProgress);
      if( FI.Synchronise(DestFS, properties, toSkip) == 0)
        return uapi_UptoDate;
      return uapi_OK;
    }
    catch( TExceptionBase& exc )  {
      TStrList out;
      exc.GetException()->GetStackTrace(out);
      log.AddList(out);
      return uapi_UpdateError;
    }
  }
  //.................................................................................................
  short _Update(AFileSystem& src, AFileSystem& dest)  {
    try  {
      TFSIndex FI( src );
      InitAQueues(src.OnProgress, FI.OnProgress);
      TStrList empty;
      if( FI.Synchronise(dest, empty, NULL) == 0 )
        return uapi_UptoDate;
      return uapi_OK;
    }
    catch( TExceptionBase& exc )  {
      TStrList out;
      exc.GetException()->GetStackTrace(out);
      log.AddList(out);
      return uapi_UpdateError;
    }
  }
  //.................................................................................................
  SettingsFile settings;
  olxstr Tag;
protected:
  // transforms /olex2-distro/tag or /olex2-distro/tag/update to /olex2-distro
  olxstr TrimTagPart(const olxstr& path) const;
  // transforms /olex2-distro to to /olex2/distro/tag or /olex2/distro/tag/update
  olxstr AddTagPart(const olxstr& path, bool Update) const;
public:
  UpdateAPI() : f_lsnr(NULL), p_lsnr(NULL), 
    settings(GetSettingsFileName()),
    Tag( ReadRepositoryTag() )
  {  }
  ~UpdateAPI()  {  CleanUp();  }
  /* calls IsInstallRequired and if required, checks for GetInstallationFileName in the basedir, 
  fetches (using default or provided repository or zip file) GetInstallationFileName 
  and extracts to basedir return uapi_OK if install is not required or was successful and uapi_InstallError 
  or uapi_InvalidRepository or uapi_NoSource in case of an error*/
  short DoInstall(AActionHandler* download_lsnr, AActionHandler* extract_lsnr, const olxstr& repo);
  // provided handlers must be created with new, and will be deleted
  short InstallPlugin(AActionHandler* download_lsnr, AActionHandler* extract_lsnr, const olxstr& name);

  SettingsFile& GetSettings() {  return settings;  }

  const SettingsFile& GetSettings() const {  return settings;  }
  void EvaluateProperties(TStrList& props) const;

  short DoUpdate(AActionHandler* file_slnr, AActionHandler* progress_lsnr);
  // updates specified repository (if any provided)
  short DoSynch(AActionHandler* file_slnr, AActionHandler* progress_lsnr);
  const TStrList& GetLog() const {  return log;  }
  /* if fails or the repository is uptodate return NULL, res can be NULL, if not it will be
  set to updater::uapi_UptoDate or an error code */
  AFileSystem* FindActiveUpdateRepositoryFS(short* res) const;
  // creates an FS from string - ftpfs, httpfs, os-fs or zipfs
  static AFileSystem* FSFromString(const olxstr& repo_str, const olxstr& proxy_str);
  static const char* GetDefaultRepository() {  return "http://www.olex2.org/olex2-distro/";  }
  static olxstr GetSettingsFileName()  {  return TBasicApp::GetBaseDir() + "usettings.dat";  }
  static olxstr GetIndexFileName()  {  return TBasicApp::GetBaseDir() + "index.ind";  }
  static olxstr GetMirrorsFileName()  {  return TBasicApp::GetBaseDir() + "mirrors.txt";  }
  static const char* GetTagsFileName()  {  return "tags.txt";  }
  static const char* GetTagFileName()  {  return "olex2.tag";  }
  static bool IsInstallRequired() {  return !TEFile::Exists(GetIndexFileName());  }
  /* checks the repository if in the settings, if down - then the default URL (http://www.olex2.org/olex2-distro/, 
  if down - checks the mirrors.txt file, if no valid repositories found, returns empty string */
  olxstr FindActiveRepositoryUrl() const  {
    olxstr repo_name;
    AFileSystem* fs = FindActiveRepositoryFS(&repo_name);
    if( fs != NULL )  delete fs;
    return repo_name;
  }
  // as above, but returns newly created file system wrapper or NULL if failed
  AFileSystem* FindActiveRepositoryFS(olxstr* repo_name=NULL) const;
  // fills list with available repositories
  void GetAvailableRepositories(TStrList& res) const;
  // fills list with available repositories
  void GetAvailableMirrors(TStrList& res) const;
  //reads current repository tag, returns EmptyString in the case of error
  static olxstr ReadRepositoryTag();
  // returns platform-dependen instalaltion file name
  static olxstr GetInstallationFileName()  {
#ifdef __WIN32__
    return "olex2.zip";
#else
    return "portable-gui.zip";
#endif
  }
  //static 
};

};

#endif
