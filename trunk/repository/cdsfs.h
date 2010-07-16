#ifndef __olx_socketfs_H
#define __olx_socketfs_H
/* POSIX socket based file fetching utility,
(c) O Dolomanov, 2010 */
#include "httpfs.h"

class TSocketFS: public THttpFileSystem  {
protected:
  int attempts, max_attempts;
  virtual IInputStream* _DoOpenFile(const olxstr& src)  {
    attempts = max_attempts;
    return THttpFileSystem::_DoOpenFile(src);
  }
  virtual bool _OnReadFailed(const olxcstr& server_name, const olxstr& src, uint64_t position) {
    if( !server_name.Equals("Olex2-CDS") )  return false;
    if( --attempts <= 0 )  return false;
    DoConnect();  // reconnect
    _write(GenerateRequest(GetUrl(), "GET", src + olxstr('#') << position));
    return true;
  }
  // this will be useful when Olex2-CDS returns MD5 digest in ETag...
  virtual bool _DoValidate(const olxcstr& server_name, const olxstr& etag, TEFile& data) const {
    return true;
  }
public:
  TSocketFS(const TUrl& url, int _max_attempts=25) : THttpFileSystem(url), max_attempts(_max_attempts)  {
    if( max_attempts < 0 )
      max_attempts = 0;
    else if( max_attempts > 100 )
      max_attempts = 100;
  }
  virtual ~TSocketFS()  {}
};

#endif
 
