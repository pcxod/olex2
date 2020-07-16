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
#endif
#include "filesystem.h"
#include "url.h"
#include "efile.h"
#include "edict.h"

/* POSIX HTTP file fetching utility,
*/

const uint16_t  // extra request headers
  httpHeaderESession = 0x0001,
  httpHeaderPlatform = 0x0002;

class THttpFileSystem : public AFileSystem {
  bool Connected;
  TUrl Url;
  static olxcstr& SessionInfo_() {
    static olxcstr s;
    return s;
  }
  uint16_t ExtraHeaders;
#ifdef __WIN32__
  static bool& Initialised_() {
    static bool i;
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

  void DoConnect();
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
  ResponseInfo ParseResponseInfo(const olxcstr& str, const olxcstr& sep,
    const olxstr& src);
  /* if position is valid and not 0 it is appended to the file name like
  + ('#'+pos)
  */
  olxcstr GenerateRequest(const olxcstr& cmd, const olxcstr& file_name,
    uint64_t position = 0);
  bool IsConnected() const { return Connected; }
  const TUrl& GetUrl() const { return Url; }
  /* if false returned, the procedure is terminated, true means the the
  connection was re-established
  */
  virtual bool _OnReadFailed(const ResponseInfo& info,
    uint64_t position)
  {
    return false;
  }
  /* version 1.0 of CDS returns Content-MD5, however the MD5 checks are done in
  the OSFS, when a foreign item is adopted
  */
  virtual bool _DoValidate(const ResponseInfo& info, TEFile& data) const {
    return data.Length() == info.contentLength;
  }
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
  static bool IsCrLf(const char* bf, size_t len);
  static size_t GetDataOffset(const char* bf, size_t len, bool crlf);
  // reads as many bytes as available within [0..dest_sz)
  int _read(char* dest, size_t dest_sz) const;
  int _write(const olxcstr& str);
  void GetAddress(struct sockaddr* Result);
  bool Connect();
  void Disconnect();
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
  void SetUrl(const TUrl& url);
  DefPropP(uint16_t, ExtraHeaders)
    static const olxcstr& GetSessionInfo() {
    return SessionInfo_();
  }
  static void SetSessionInfo(const olxcstr& si) {
    SessionInfo_() = si;
  }
};

#endif
