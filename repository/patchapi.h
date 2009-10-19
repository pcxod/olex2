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

class  PatchAPI  {
  class DeletionExc: public TBasicException {
  public:
    DeletionExc(const olxstr& location, const olxstr& msg) :
      TBasicException(location, msg )  {    }
    virtual IEObject* Replicate()  const    {  return new DeletionExc(*this);  }
  };

  static void CleanUp(AActionHandler* A, AActionHandler* B)  {
    if( A != NULL )
      delete A;
    if( B != NULL )
      delete B;
  }

  static void AfterFileCopy(const olxstr& src, const olxstr& dest)  {
    if( !TEFile::DelFile(src) )
      throw DeletionExc(__OlxSourceInfo, olxstr("Failed to delete file: ") << src);
  }
  static TEFile* lock_file;
public:
  ~PatchAPI()  {  UnlockUpdater();  }
  // if action handlers are passed along - they eill be automatically deleted
  static short DoPatch(AActionHandler* OnFileCopy=NULL,
    AActionHandler* OnOverallCopy=NULL);
  static olxstr GetUpdateLocationFileName()  {  return TBasicApp::GetBaseDir() + "__location.update";  }
  static olxstr GetUpdateLocation()  {
    olxstr update_location = GetUpdateLocationFileName();
    if( TEFile::Exists(update_location) )  {
      TCStrList fc;
      fc.LoadFromFile(update_location);
      if( fc.Count() == 1 )
        return fc[0];
    }
    return EmptyString;
  }
  static olxstr GetUpdaterPIDFileName()  {  return TBasicApp::GetBaseDir() + "pid.update";  }
  static const char* GetUpdaterCmdFileName()  {  return "__cmds.update";  }
  static const char* GetOlex2PIDFileExt()  {  return "olex2_pid";  }
  static bool IsOlex2Running() {  return GetNumberOfOlex2Running() != 0;  }
  static int GetNumberOfOlex2Running();
  static bool LockUpdater();
  static bool UnlockUpdater();

  static const char* GetTagFileName()  {  return "olex2.tag";  }
  //reads current repository tag, returns EmptyString in the case of error
  static olxstr ReadRepositoryTag();
  // composes new shared dir and saves its info
  static olxstr ComposeNewSharedDir(const olxstr& shared_dir, const olxstr& base_dir=EmptyString)  {
    olxstr new_shared_dir = shared_dir;
#ifdef __WIN32__
    new_shared_dir << "Olex2Data/";
#else
    TEFile::RemoveTrailingBackslashI(new_shared_dir) << "data/";
#endif
    return TEFile::AddTrailingBackslashI( 
      new_shared_dir << MD5::Digest(esdl::olxcstr( 
        TEFile::AddTrailingBackslash((base_dir.IsEmpty() ? TBasicApp::GetBaseDir() : base_dir)) + ReadRepositoryTag())) );
  }
  static void SaveLocationInfo(const olxstr& shared_dir, const olxstr& base_dir=EmptyString)  {
    TCStrList location_file_content;
    location_file_content.Add(TEFile::AddTrailingBackslash((base_dir.IsEmpty() ? TBasicApp::GetBaseDir() : base_dir))) 
      << patcher::PatchAPI::ReadRepositoryTag();
    location_file_content.SaveToFile( shared_dir + "folder.info" );
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
    return TEFile::AddTrailingBackslashI(new_shared_dir);
  }

};

};

#endif
