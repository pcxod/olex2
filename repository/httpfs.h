/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_httpfs_H
#define __olx_httpfs_H
#include "ebase.h"
#if !defined(__WIN32__)
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <netdb.h>
  #include <unistd.h>
#else
  #include <WinInet.h>
  #pragma comment (lib, "Wininet.lib")
#endif
#include "filesystem.h"
#include "url.h"
#include "efile.h"
#include "edict.h"
#ifdef _OPENSSL
  #include <openssl/bio.h>
  #include <openssl/ssl.h>
  #include <openssl/err.h>
#endif

/* POSIX HTTP file fetching utility,
*/

const uint16_t  // extra request headers
  httpHeaderESession = 0x0001,
  httpHeaderPlatform = 0x0002;

class AnHttpFileSystem : public AFileSystem {
private:
  static olxcstr& SessionInfo_() {
    static olxcstr s;
    return s;
  }
  uint16_t ExtraHeaders;
  bool Connected;
protected:
  TUrl Url;
  virtual bool Connect() {
    return (Connected = true);
  }
  virtual void Disconnect() {
    Connected = false;
  }
  virtual bool _DoDelFile(const olxstr& f) { return false; }
  virtual bool _DoDelDir(const olxstr& f) { return false; }
  virtual bool _DoNewDir(const olxstr& f) { return false; }
  virtual bool _DoAdoptFile(const TFSItem& Source) { return false; }
  virtual bool _DoAdoptStream(IInputStream& file, const olxstr& name) {
    return false;
  }
  virtual bool _DoesExist(const olxstr& df, bool);

  typedef olxdict<olxcstr, olxcstr, olxstrComparator<false> > HeadersDict;
  struct ResponseInfo {
    HeadersDict headers;
    olxcstr status, contentMD5;
    olxstr source;
    uint64_t contentLength;
    ResponseInfo() : contentLength(~0) {}
    bool IsOK() const { return status.EndsWithi("200 OK"); }
    // note that only length and digest are compared
    bool operator == (const ResponseInfo& i) const {
      return contentLength == i.contentLength &&
        contentMD5 == i.contentMD5;
    }
    bool operator != (const ResponseInfo& i) const {
      return !(this->operator == (i));
    }
    // does basic checks to identify if any data is attached
    bool HasData() const {
      return (contentLength != InvalidSize && IsOK() && headers.HasKey("ETag"));
    }
  };
  struct AllocationInfo {
    olx_object_ptr<TEFile> file;
    olxcstr digest;
    bool truncated;
    AllocationInfo(TEFile* _file, const olxcstr& _digest, bool _truncated)
      : file(_file), digest(_digest), truncated(_truncated)
    {}
  };
  virtual ResponseInfo ParseResponseInfo(const olxcstr& str, const olxcstr& sep,
    const olxstr& src);
  /* if position is valid and not 0 it is appended to the file name like
  + ('#'+pos)
  */
  static olxcstr GenerateRequest(const TUrl& Url, uint16_t ExtraHeaders,
    const olxcstr& cmd, const olxstr& file_name, uint64_t position = 0);
  olxcstr GenerateRequest(const olxcstr& cmd, const olxstr& file_name,
    uint64_t position = 0)
  {
    return GenerateRequest(Url, ExtraHeaders, cmd, file_name, position);
  }
  size_t GetDataOffset(const char* bf, size_t len) const;

  /* does the file allocation, always new or 'truncated'. the truncated is
  false for the first time file allocation and true if the download restarts
  due to the file changed
  */
  virtual AllocationInfo _DoAllocateFile(const olxstr& fileName) {
    return AllocationInfo(TEFile::TmpFile(), CEmptyString(), true);
  }
  virtual AllocationInfo& _DoTruncateFile(AllocationInfo& file) {
    if (!file.file.ok()) {
      throw TInvalidArgumentException(__OlxSourceInfo, "file");
    }
    file.file->SetTemporary(true);
    file.file = TEFile::TmpFile();
    file.digest.SetLength(0);
    file.truncated = true;
    return file;
  }
  /* version 1.0 of CDS returns Content-MD5, however the MD5 checks are done in
  the OSFS, when a foreign item is adopted
  */
  virtual bool _DoValidate(const ResponseInfo& info, TEFile& data) const {
    return data.Length() == info.contentLength;
  }
  /* if false returned, the procedure is terminated, true means the the
  connection was re-established
  */
  virtual bool _OnReadFailed(const ResponseInfo& info,
    uint64_t position)
  {
    return false;
  }
  virtual olx_object_ptr<IInputStream> _DoOpenFile(const olxstr& src);
  virtual int http_read(char* dest, size_t dest_sz) const = 0;
  virtual int http_write(const olxcstr& str) = 0;
public:
  AnHttpFileSystem()
    : ExtraHeaders(0),
    Connected(false)
  {}

  DefPropP(uint16_t, ExtraHeaders);
  const TUrl& GetUrl() const { return Url; }
  void SetUrl(const TUrl& url);
  bool IsConnected() const { return Connected; }

  // intialises if empty
  static const olxcstr& GetSessionInfo();
  static void SetSessionInfo(const olxcstr& si) {
    SessionInfo_() = si;
  }
};

class THttpFileSystem : public AnHttpFileSystem {
#ifdef __WIN32__
  static bool& Initialised_() {
    static bool i = false;
    return i;
  }
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
  class Finaliser : public IOlxObject {
  public:
    ~Finaliser() { THttpFileSystem::Finalise(); }
  };

  // reads as many bytes as available within [0..dest_sz)
  int http_read(char* dest, size_t dest_sz) const;
  int http_write(const olxcstr& str);
  void GetAddress(struct sockaddr* Result);
  bool Connect();
  void Disconnect();
  void Init();
public:
  THttpFileSystem() { Init(); }
  THttpFileSystem(const TUrl& url) {
    Init();
    SetUrl(url);
  }
  ~THttpFileSystem();
  /* returns temporary file, which gets deleted when object is deleted, use
  SetTemporary to change it
  */
  olx_object_ptr<TEFile> OpenFileAsFile(const olxstr& Source) {
    return (TEFile*)OpenFile(Source).release();
  }
};

#ifdef __WIN32__
// a simple WinINet based implementation
class TWinHttpFileSystem : public AFileSystem {
  TUrl Url;
  uint16_t ExtraHeaders;
  struct INET_CLOSE {
    HINTERNET handle;
    INET_CLOSE(HINTERNET handle)
      : handle(handle)
    {}
    ~INET_CLOSE() {
      if (handle != 0) {
        InternetCloseHandle(handle);
      }
    }
  };
protected:
  const TUrl& GetUrl() const { return Url; }
  /* if false returned, the procedure is terminated, true means the the
  connection was re-established
  */
  virtual bool _DoDelFile(const olxstr& f) { return false; }
  virtual bool _DoDelDir(const olxstr& f) { return false; }
  virtual bool _DoNewDir(const olxstr& f) { return false; }
  virtual bool _DoAdoptFile(const TFSItem& Source) { return false; }
  virtual bool _DoesExist(const olxstr& df, bool);
  virtual olx_object_ptr<IInputStream> _DoOpenFile(const olxstr& src);
  virtual bool _DoAdoptStream(IInputStream& file, const olxstr& name) {
    return false;
  }
  void Init();
  void SetUrl(const TUrl& url);
public:
  TWinHttpFileSystem() { Init(); }
  TWinHttpFileSystem(const TUrl& url) {
    Init();
    SetUrl(url);
  }
  ~TWinHttpFileSystem();
  /* returns temporary file, which gets deleted when object is deleted, use
  SetTemporary to change it
  */
  olx_object_ptr<TEFile> OpenFileAsFile(const olxstr& Source) {
    return (TEFile*)OpenFile(Source).release();
  }
  DefPropP(uint16_t, ExtraHeaders);
};
#endif // __WIN32__

#ifdef _OPENSSL
class TSSLHttpFileSystem : public AnHttpFileSystem {
private:
  SSL_CTX* ctx;
  SSL* ssl;
  void cleanup();
protected:
  virtual bool Connect();
  virtual void Disconnect();
  virtual int http_read(char* dest, size_t dest_sz) const;
  virtual int http_write(const olxcstr& str);
  void Init();
public:
  TSSLHttpFileSystem() { Init(); }
  TSSLHttpFileSystem(const TUrl& url) {
    Init();
    SetUrl(url);
  }
  ~TSSLHttpFileSystem();
};
#endif //_OPENSSL
#endif
