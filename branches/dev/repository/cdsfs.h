#ifndef __olx_socketfs_H
#define __olx_socketfs_H
/* POSIX socket based file fetching utility,
(c) O Dolomanov, 2010 */
#include "httpfs.h"
#include "md5.h"
#include "eutf8.h"

class TSocketFS: public THttpFileSystem  {
protected:
  static bool UseLocalFS;
  static olxstr Base;
  int attempts, max_attempts;
  bool BaseValid;
  virtual IInputStream* _DoOpenFile(const olxstr& src)  {
    attempts = max_attempts;
    return THttpFileSystem::_DoOpenFile(src);
  }
  virtual bool _OnReadFailed(const ResponseInfo& info, uint64_t position);
  // this will be useful when Olex2-CDS returns MD5 digest in ETag...
  virtual bool _DoValidate(const ResponseInfo& info, TEFile& data, uint64_t toBeread) const;
  virtual AllocationInfo _DoAllocateFile(const olxstr& src, bool truncated);
public:
  TSocketFS(const TUrl& url, int _max_attempts=100) :
      THttpFileSystem(url), max_attempts(_max_attempts), BaseValid(false)
  {
    if( Base.IsEmpty() )
      Base = TEFile::AddPathDelimeter(TBasicApp::GetBaseDir() + ".cds");
    if( UseLocalFS )  {
      try  {
        if( !TEFile::Exists(Base) )  {
          if( TBasicApp::IsBaseDirWriteable() && TEFile::MakeDir(Base) )
            BaseValid = true;
        }
        else
          BaseValid = true;
      }
      catch(...)  {}
    }
    if( max_attempts < 0 )
      max_attempts = 0;
    else if( max_attempts > 32000 )
      max_attempts = 32000;
  }
  virtual ~TSocketFS()  {}
  static bool CanUseLocalFS()  {  return UseLocalFS;  }
  // allows creating temporary files in basedir/.cds/
  static void SetUseLocalFS(bool v)  {  UseLocalFS = true;  } 
};

#endif
 

