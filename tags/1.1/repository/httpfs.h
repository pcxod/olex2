#ifndef __olx_win_httpfs_H
#define __olx_win_httpfs_H
/* POSIX HTTP file fetching utility,
(c) O Dolomanov, 2004-2009 */
#include "defs.h"
#ifdef __WIN32__
  #include <winsock.h>
  #include <windows.h>
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <netdb.h>
  #include <unistd.h>
#endif
#include "filesystem.h"
#include "url.h"
#include "efile.h"
//  #pragma link "../..lib/psdk/mswsock.lib"

class THttpFileSystem: public AFileSystem, public IEObject  {
  int Socket;
  bool Connected, Successful;
  TUrl Url;
protected:
  void GetAddress(struct sockaddr* Result);
  bool Connect();
  void Disconnect();
  virtual bool _DoDelFile(const olxstr& f) {  return false;  }
  virtual bool _DoDelDir(const olxstr& f)  {  return false;  }
  virtual bool _DoNewDir(const olxstr& f)  {  return false;  }
  virtual bool _DoAdoptFile(const TFSItem& Source) {  return false;  }
  virtual bool _DoesExist(const olxstr& df);
  virtual IInputStream* _DoOpenFile(const olxstr& src);
  virtual bool _DoAdoptStream(IInputStream& file, const olxstr& name) {  return false;  }
public:
  THttpFileSystem(const TUrl& url);
  virtual ~THttpFileSystem();
  // returns temporary file, which gets deleted when object is deleted, use SetTemporary to change it
  TEFile* OpenFileAsFile(const olxstr& Source)  {
    return (TEFile*)OpenFile(Source);
  }
};

#endif
 
