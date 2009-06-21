#ifndef __olx_patch_api_H
#define __olx_patch_api_H
#include "actions.h"
#include "efile.h"
#include "exception.h"
#include "estrlist.h"
#include "bapp.h"
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
public:
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
};

};

#endif
