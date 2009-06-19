#ifndef __olx_win_httpfs_H
#define __olx_win_httpfs_H
/* windows specific HTTP file fetching utility,
(c) O Dolomanov, 2004-2009 */
#include "defs.h"
#ifdef __WIN32__
  #include <winsock.h>
  #include <windows.h>

  #include "filesystem.h"
  #include "url.h"
  #include "efile.h"
//  #pragma link "../..lib/psdk/mswsock.lib"

class TWinHttpFileSystem: public AFileSystem, public IEObject  {
  TStrList TmpFiles;
  SOCKET  Socket;
  bool Connected, Successful;
  TUrl Url;
protected:
  void GetAddress(struct sockaddr* Result);
  bool Connect();
  void Disconnect();
public:
  TWinHttpFileSystem(const TUrl& url);
  virtual ~TWinHttpFileSystem();

  virtual IDataInputStream* OpenFile(const olxstr& Source);
  virtual bool FileExists(const olxstr& DN)  {  return true;  }

  virtual bool DelFile(const olxstr& FN)     {  throw TNotImplementedException(__OlxSourceInfo);    }
  virtual bool DelDir(const olxstr& DN)      {  throw TNotImplementedException(__OlxSourceInfo);     }
  virtual bool AdoptFile(const TFSItem& Source){  throw TNotImplementedException(__OlxSourceInfo);  }
  virtual bool NewDir(const olxstr& DN)      {  throw TNotImplementedException(__OlxSourceInfo);     }
  virtual bool ChangeDir(const olxstr& DN)   {  throw TNotImplementedException(__OlxSourceInfo);  }
  virtual bool AdoptStream(IInputStream& file, const olxstr& name)  {
    throw TNotImplementedException(__OlxSourceInfo);
  }

  TEFile* OpenFileAsFile(const olxstr& Source)  {
    return (TEFile*)OpenFile(Source);
  }
};

#endif // __WIN32__

#endif
 
