/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_update_api_H
#define __olx_update_api_H
#include "filesystem.h"
#include "bapp.h"
#include "settingsfile.h"

/* a usettings.dat file processor since the launch and unirun do not their
original job now, their API is here, night be useful in the future...
*/
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
  // tag is missing on the repo, new isntall might be required
  uapi_MissingTag     = 9,
  uapi_AccessDenied   = 10, // the destination folder is not writeable
  uapi_InvalidInstallation = 11,  // required files are missing
  uapi_Busy           = 12;  // updater API is busy

// Olex2 settings file wrapper
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct SettingsFile {
  olxstr source_file;
  olxstr repository,  // repository for local update
    proxy,            // repository proxy
    dest_repository,  // like ftp
    src_for_dest,     // source for dest, local or remote (repository)
    update_interval,
    olex2_port;
  bool ask_for_update;

  TStrList extensions_to_skip, files_to_skip;
  time_t last_updated;
  //...........................................................................
  SettingsFile() {}
  SettingsFile(const olxstr& file_name) {
    Init(file_name);
  }
  void Init(const olxstr& file_name);
  // save will change repo/update/ to repo...
  bool Save() const;
  bool IsValid() const { return TEFile::Exists(source_file); }
};

// Olex2 updater/installer API
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class UpdateAPI {
  mutable TStrList log;
  olx_object_ptr<AActionHandler> f_lsnr, p_lsnr;
  void InitAQueues(TActionQueue& f, TActionQueue& p);
  short _Update(AFileSystem& SrcFS, const TStrList& properties,
    const TFSItem::SkipOptions* toSkip);
  short _Update(AFileSystem& src, AFileSystem& dest);
  //.............................................................................
  SettingsFile settings;
  olxstr Tag;
  static olxstr NewInstallationFN() {
    return "new_installation";
  }
  static const_strlist GetSystemTags();
public:
  UpdateAPI();
  ~UpdateAPI();
  /* calls IsInstallRequired and if required, checks for
  GetInstallationFileName in the basedir, fetches (using default or provided
  repository or zip file) GetInstallationFileName and extracts to basedir
  return uapi_OK if install is not required or was successful and
  uapi_InstallError or uapi_InvalidRepository or uapi_NoSource in case of an
  error
  */
  short DoInstall(olx_object_ptr<AActionHandler> download_lsnr,
    olx_object_ptr < AActionHandler> extract_lsnr,
    const olxstr& repo);
  // provided handlers must be created with new, and will be deleted
  short InstallPlugin(olx_object_ptr<AActionHandler> download_lsnr,
    olx_object_ptr<AActionHandler> extract_lsnr, const olxstr& name);

  static const_strlist GetPluginProperties(const olxstr& p);

  SettingsFile& GetSettings() { return settings; }

  const SettingsFile& GetSettings() const { return settings; }
  void EvaluateProperties(TStrList& props) const;

  short DoUpdate(olx_object_ptr<AActionHandler> file_slnr,
    olx_object_ptr<AActionHandler> progress_lsnr);
  // updates specified repository (if any provided)
  short DoSynch(olx_object_ptr<AActionHandler> file_slnr,
    olx_object_ptr<AActionHandler> progress_lsnr);
  const TStrList& GetLog() const { return log; }
  /* if fails or the repository is uptodate return NULL, res can be NULL, if
  not it will be set to updater::uapi_UptoDate or an error code
  update - will add /update to non-zip FS
  */
  olx_object_ptr<AFileSystem> FindActiveRepositoryFS(short* res,
    bool force = false, bool update = true) const;
  //returns true if the program will/allowed try to update itself
  bool WillUpdate(bool force = false) const;
  // creates an FS from string - ftpfs, httpfs, os-fs or zipfs
  static olx_object_ptr < AFileSystem> FSFromString(const olxstr& repo_str,
    const olxstr& proxy_str);
  static const TStrList& GetDefaultRepositories();
  static olxstr GetSettingsFileName() {
    return TBasicApp::GetInstanceDir() + "usettings.dat";
  }
  static olxstr GetReinstallFileName() {
    return TBasicApp::GetInstanceDir() + "__reinstall";
  }
  static olxstr GetCleanUpFileName() {
    return TBasicApp::GetInstanceDir() + "__cleanup";
  }
  static olxstr GetIndexFileName() {
    return TBasicApp::GetBaseDir() + "index.ind";
  }
  static olxstr GetMirrorsFileName() {
    return TBasicApp::GetInstanceDir() + "mirrors.txt";
  }
  static bool IsNewInstallation() {
    return TEFile::Exists(TBasicApp::GetBaseDir() + NewInstallationFN());
  }
  static void TagInstallationAsNew() {
    TEFile f(TBasicApp::GetBaseDir() + NewInstallationFN(), "w+");
    f.Close();
  }
  static void TagInstallationAsOld() {
    TEFile::DelFile(TBasicApp::GetBaseDir() + NewInstallationFN());
  }
  static const char* GetTagsFileName() { return "tags.txt"; }
  // transforms /olex2-distro/tag or /olex2-distro/tag/update to /olex2-distro
  olxstr TrimTagPart(const olxstr& path) const;
  /* transforms /olex2-distro to to /olex2/distro/tag or
  /olex2/distro/tag/update
  */
  olxstr AddTagPart(const olxstr& path, bool Update) const;
  const olxstr& GetTag() const { return Tag; }

  static bool IsInstallRequired() {
    return !TEFile::Exists(GetIndexFileName());
  }
  /* checks the repository if in the settings, if down - then the default URL
  (http://www.olex2.org/olex2-distro/, if down - checks the mirrors.txt file,
  if no valid repositories found, returns empty string
  */
  olxstr FindActiveRepositoryUrl() const {
    olxstr repo_name;
    FindActiveRepositoryFS(&repo_name, GetInstallationFileName());
    return repo_name;
  }
  /* as above, but returns newly created file system wrapper or NULL if failed;
  if check file is not empty, the file existence will be also checked */
  olx_object_ptr<AFileSystem> FindActiveRepositoryFS(olxstr* repo_name = 0,
    const olxstr& check_file = EmptyString()) const;
  // fills list with available repositories
  void GetAvailableRepositories(TStrList& res) const;
  // fills list with available repositories
  void GetAvailableMirrors(TStrList& res) const;
  // fills list with available tags and initialises the chosen repository URL
  void GetAvailableTags(TStrList& res, olxstr& repo_URL) const;
  // returns platform-dependen instalaltion file name
  static olxstr GetInstallationFileName();
};

};

#endif
