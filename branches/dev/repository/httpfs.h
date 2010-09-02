#ifndef __olx_httpfs_H
#define __olx_httpfs_H
/* POSIX HTTP file fetching utility,
(c) O Dolomanov, 2004-2009 */
#include "ebase.h"
#ifdef __WIN32__
  #include <winsock.h>
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <netdb.h>
  #include <unistd.h>
#endif
#include "filesystem.h"
#include "url.h"
#include "efile.h"
#include "edict.h"
//  #pragma link "../..lib/psdk/mswsock.lib"

class THttpFileSystem: public AFileSystem  {
  bool Connected;
  TUrl Url;
#ifdef __WIN32__
  static bool Initialised;
#endif
protected:
#ifdef __WIN32__
  SOCKET Socket;
#else
  int Socket;
#endif
  // intitialise/finalise functionality
  static void Initialise();
  // should not be called directly, look below
  static void Finalise();
  class Finaliser : public IEObject {
  public:
    ~Finaliser()  {  THttpFileSystem::Finalise();  }
  };

  void DoConnect();
  typedef olxdict<olxcstr,olxcstr,olxstrComparator<false> > HeadersDict;
  struct ResponseInfo  {
    HeadersDict headers;
    olxcstr status;
  };
  ResponseInfo ParseResponseInfo(const olxcstr& str, const olxcstr& sep);
  static olxcstr GenerateRequest(const TUrl& url, const olxcstr& cmd, const olxcstr& file_name);
  bool IsConnected() const {  return Connected;  }
  const TUrl& GetUrl() const {  return Url;  }
  // if false returned, the procedure is terminated, true means the the connection was re-established
  virtual bool _OnReadFailed(const olxcstr& server_name, const olxstr& src, uint64_t position)  {  return false;  }
  virtual bool _DoValidate(const olxcstr& server_name, const olxstr& etag, TEFile& data) const {  return true;  }
  static bool IsCrLf(const char* bf, size_t len);
  static size_t GetDataOffset(const char* bf, size_t len, bool crlf);
  // reads as many bytes as available within [0..dest_sz)
  int _read(char* dest, size_t dest_sz) const;
  int _write(const olxcstr& str);
  void GetAddress(struct sockaddr* Result);
  bool Connect();
  void Disconnect();
  virtual bool _DoDelFile(const olxstr& f) {  return false;  }
  virtual bool _DoDelDir(const olxstr& f)  {  return false;  }
  virtual bool _DoNewDir(const olxstr& f)  {  return false;  }
  virtual bool _DoAdoptFile(const TFSItem& Source) {  return false;  }
  virtual bool _DoesExist(const olxstr& df, bool);
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
 
