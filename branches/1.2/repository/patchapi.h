/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_patch_api_H
#define __olx_patch_api_H
#include "actions.h"
#include "efile.h"
#include "exception.h"
#include "estrlist.h"
#include "bapp.h"
#include "md5.h"
#include "utf8file.h"

namespace patcher  {
const short
  papi_OK            = 0,
  papi_Busy          = 1,
  papi_DeleteError   = 2,
  papi_CopyError     = 3,
  papi_AccessDenied  = 4,
  papi_InvalidUpdate = 5;

class PatchAPI  {
  class DeletionExc: public TBasicException {
  public:
    DeletionExc(const olxstr& location, const olxstr& msg) :
      TBasicException(location, msg )  {}
    virtual IEObject* Replicate() const {  return new DeletionExc(*this);  }
  };

  static void CleanUp(AActionHandler* A, AActionHandler* B)  {
    if( A != NULL )
      delete A;
    if( B != NULL )
      delete B;
  }

  static void AfterFileCopy(const olxstr& src, const olxstr& dest)  {
    if( !TEFile::DelFile(src) ) {
      throw DeletionExc(__OlxSourceInfo,
        olxstr("Failed to delete file: ") << src);
    }
  }

  struct DataDirSettings {
    bool is_static, is_manually_set;
    olxstr data_dir;
    DataDirSettings();
    // does nothing if set manually
    bool Refresh();
  };
  static DataDirSettings &GetDDSetting() {
    static DataDirSettings st;
    return st;
  }
  static TEFile* lock_file;
  static void _RestoreExecuableFlags();
  static olxstr repository_tag, repository_base_dir,
    shared_dir,
    instance_dir;
public:
  ~PatchAPI()  {  UnlockUpdater();  }
  // if action handlers are passed along - they will be automatically deleted
  static short DoPatch(AActionHandler* OnFileCopy=NULL,
    AActionHandler* OnOverallCopy=NULL);
  /* 2012.01.21 - the location is stored in the user dependent folder
  */
  static olxstr GetUpdateLocationFileName()  {
    return TBasicApp::GetInstanceDir() + "__location.update";
  }
  /* writes patch dir into the the update location file */
  static void MarkPatchComplete();
  static bool HaveUpdates() {
    return TEFile::Exists(GetUpdateLocationFileName());
  }
  static olxstr GetUpdateLocation();
  static olxstr GetUpdaterPIDFileName()  {
    return TBasicApp::GetInstanceDir() + "pid.update";
  }
  static const char* GetUpdaterCmdFileName()  {  return "__cmds.update";  }
  static const char* GetOlex2PIDFileExt()  {  return "olex2_pid";  }
  static bool IsOlex2Running() {  return GetNumberOfOlex2Running() != 0;  }
  static size_t GetNumberOfOlex2Running();
  static bool LockUpdater();
  static bool UnlockUpdater();

  static const char* GetTagFileName()  {  return "olex2.tag";  }
  static const char* GetPatchFolder()  {  return "patch";  }
  //reads current repository tag, returns EmptyString() in the case of error
  static olxstr ReadRepositoryTag(const olxstr& base_dir=EmptyString());
  static void SetDataDir(const olxstr &dir, bool is_static) {
    if (dir.IsEmpty())
      throw TInvalidArgumentException(__OlxSourceInfo, "empty data dir");
    GetDDSetting().is_static = is_static;
    GetDDSetting().data_dir = dir;
    GetDDSetting().is_manually_set = true;
  }
  /* return 'AppData' or OLEX2_DATADIR if defined
  */
  static olxstr _GetSharedDirRoot(bool refresh=false);
  /* return _GetSharedDirRoot()/Olex2Data on windows or
  _GetSharedDirRoot()/data on other platforms (unless the OLEX2_DATADIR_STATIC
  is set to true
  */
  static olxstr GetSharedDir(bool refresh=false);
  /* composes new instance dir and saves its info. The instance dir depends on
  the location of the executable and thus can be used to store instance
  specific updates etc
  */
  static olxstr ComposeInstanceDir(const olxstr& shared_dir,
    const olxstr& _base_dir=EmptyString())
  {
    olxstr new_shared_dir = shared_dir;
    const olxstr base_dir = _base_dir.IsEmpty() ? TBasicApp::GetBaseDir() :
      TEFile::AddPathDelimeter(_base_dir);
#ifdef __WIN32__
    new_shared_dir << "Olex2Data/";
#else
    new_shared_dir << "data/";
#endif
    return TEFile::AddPathDelimeterI(
      new_shared_dir << MD5::Digest(
        esdl::olxcstr(base_dir + ReadRepositoryTag(base_dir))));
  }
  /* checks for OLEX2_DATADIR, if OLEX2_DATADIR_STATIC is set to true, the root
  folder is returned and used (for all versions of programs which may be
  installed and using this API
  */
  static olxstr GetInstanceDir(bool refresh=false);
  /* mostly for internal use, saves the origin of the MD5 hash to folder.info
  file */
  static void SaveLocationInfo(const olxstr& shared_dir,
    const olxstr& base_dir=EmptyString())
  {
    TCStrList location_file_content;
    location_file_content.Add(TEFile::AddPathDelimeter(
      (base_dir.IsEmpty() ? TBasicApp::GetBaseDir() : base_dir)))
        << patcher::PatchAPI::ReadRepositoryTag();
    location_file_content.SaveToFile( shared_dir + "folder.info");
  }
  static olxstr ComposeOldSharedDir(const olxstr& shared_dir)  {
    olxstr new_shared_dir = shared_dir;
#ifdef __WIN32__
  #ifdef _UNICODE
    new_shared_dir << "Olex2u";
  #else
    new_shared_dir << "Olex2";
  #endif
#endif
    return TEFile::AddPathDelimeterI(new_shared_dir);
  }
};

};

#endif
