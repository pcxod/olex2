/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "httpfs.h"
#include "efile.h"
#include "bapp.h"
#include "log.h"
#include "eutf8.h"
#include "md5.h"
#include <errno.h>

//..............................................................................
const olxcstr& AnHttpFileSystem::GetSessionInfo() {
  if (SessionInfo_().IsEmpty()) { // not UUID, but quite unique
    SessionInfo_() = MD5::Digest(olxstr(TETime::msNow()));
    olx_sleep(1);
    SessionInfo_() = MD5::Digest(SessionInfo_() + olxcstr(TETime::msNow()));
  }
  return SessionInfo_();
}
//..............................................................................
AnHttpFileSystem::ResponseInfo AnHttpFileSystem::ParseResponseInfo(
  const olxcstr& str, const olxcstr& sep, const olxstr& src)
{
  ResponseInfo rv;
  if (str.IsEmpty()) {
    return rv;
  }
  const TCStrList header_toks(str, sep);
  rv.status = header_toks[0];
  for (size_t i = 1; i < header_toks.Count(); i++) {
    const size_t si = header_toks[i].IndexOf(':');
    if (si == InvalidIndex) {
      continue;
    }
    rv.headers(header_toks[i].SubStringTo(si),
      header_toks[i].SubStringFrom(si + 1).Trim(' '));
  }
  size_t ii = rv.headers.IndexOf("Content-MD5");
  if (ii != InvalidIndex) {
    rv.contentMD5 = rv.headers.GetValue(ii);
    rv.headers.Delete(ii);
  }
  ii = rv.headers.IndexOf("Content-Length");
  if (ii != InvalidIndex) {
    try {
      rv.contentLength = rv.headers.GetValue(ii).ToInt();
    }
    catch (...) {}  // toInt may throw an exception
    rv.headers.Delete(ii);
  }
  rv.source = src;
  return rv;
}
//..............................................................................
size_t AnHttpFileSystem::GetDataOffset(const char* bf, size_t len) const {
  for (size_t i = 3; i < len; i++) {
    if (bf[i] == '\n' && bf[i - 1] == '\r' && bf[i - 2] == '\n' && bf[i - 3] == '\r') {
      return i;
    }
  }
  return InvalidIndex;
}
//..............................................................................
olxcstr AnHttpFileSystem::GenerateRequest(const TUrl& Url, uint16_t ExtraHeaders,
  const olxcstr& cmd, const olxstr& FileName, uint64_t position)
{
  static const olxcstr le = "\r\n";
  olxcstr request = cmd;
  request << ' '
    << TUtf8::Encode(Url.GetFullHost() << '/'
    << TEFile::UnixPath(TUtf8::Encode(FileName))).Replace(' ', "%20")
    << " HTTP/1.0" << le;
  if (Url.HasProxy() && !Url.GetProxy().GetUser().IsEmpty() &&
    !Url.GetProxy().GetPassword().IsEmpty())
  {
    request << "Authorization: " << olxcstr(Url.GenerateHTTPAuthString()) << le;
  }
  request << "Host: " << Url.GetHost() << ':' << Url.GetPort() << le;
  if ((ExtraHeaders & httpHeaderPlatform) != 0) {
    request << "Platform: " << TBasicApp::GetPlatformString(false) << le;
  }
  if ((ExtraHeaders & httpHeaderESession) != 0) {
    request << "ESession: " << SessionInfo_() << le;
  }
  if (position != InvalidIndex && position != 0) {
    request << "Resume-From: " << position << le;
  }
  //request << "Connection: " << "keep-alive" << le;
  request << "Olex2-version: " << "1.5" << le;
  return request << le;
}
//..............................................................................
void AnHttpFileSystem::SetUrl(const TUrl& url) {
  Url = url;
  SetBase(url.GetPath());
}
//..............................................................................
olx_object_ptr<IInputStream> AnHttpFileSystem::_DoOpenFile(const olxstr& Source) {
  if (!Connect()) {
    return 0;
  }
  const size_t BufferSize = 1024 * 64;
  olx_array_ptr<char> Buffer(BufferSize + 1);
  AllocationInfo allocation_info = _DoAllocateFile(Source);
  if (!allocation_info.file.ok()) {
    return 0;
  }
  // if the file length is not 0, it means that the CDS of any kind is in place...
  uint64_t starting_file_len = allocation_info.file->Length();
  // read and extract headers
  http_write(GenerateRequest("GET", Source, starting_file_len));
  int ThisRead = http_read(Buffer, 512);
  uint64_t TotalRead = starting_file_len;
  size_t data_off = GetDataOffset(Buffer, ThisRead);
  if (data_off == InvalidIndex) {
    return 0;
  }
  const olxcstr line_break = "\r\n";
  ResponseInfo info =
    ParseResponseInfo(olxcstr(&Buffer, data_off), line_break, Source);
  if (!info.HasData()) {
    return 0;
  }
  // validate if we continue download for the same file...
  if (!allocation_info.truncated && allocation_info.digest != info.contentMD5) {
    _DoTruncateFile(allocation_info);
    if (!allocation_info.file.ok()) {
      return 0;
    }
    starting_file_len = TotalRead = 0;
  }
  TOnProgress Progress;
  Progress.SetPos(0);
  Progress.SetAction(Source);
  Progress.SetMax(info.contentLength);
  OnProgress.Enter(this, &Progress);
  TotalRead = starting_file_len + ThisRead - data_off - 1;
  allocation_info.file->Write(&Buffer[data_off + 1],
    (size_t)(TotalRead - starting_file_len));
  bool restarted = false;
  while (TotalRead != info.contentLength) {
    if (TotalRead > info.contentLength) { // could happen?
      break;
    }
    ThisRead = http_read(Buffer, BufferSize);
    if (restarted && ThisRead > 0) {
      data_off = GetDataOffset(Buffer, ThisRead);
      if (data_off == InvalidIndex) {
        TBasicApp::NewLogEntry(logInfo, true) <<
          "Restarted download: header is to short, retrying";
        if (_OnReadFailed(info, TotalRead)) {
          continue;  //try another attempt
        }
        break;
      }
      // get the info
      ResponseInfo new_info =
        ParseResponseInfo(olxcstr(&Buffer, data_off), line_break, Source);
      if (!new_info.HasData()) {
        break;
      }

      if (TotalRead == 0) { // 1. previously restarted from 2
        info = new_info;
      }
      else if (new_info.contentMD5 != info.contentMD5) {  // have to restart...
        TBasicApp::NewLogEntry(logInfo, true) <<
          "Restarted download: info mismatch old={" << '(' <<
          info.contentMD5 << ',' << info.contentLength << "}, new={" <<
          new_info.contentMD5 << ',' << new_info.contentLength << '}';
        info = new_info;
        _DoTruncateFile(allocation_info);
        if (!allocation_info.file.ok()) {
          return 0;
        }
        TotalRead = 0;
        if (_OnReadFailed(info, TotalRead)) {
          /* 2. continue to the same block to 1 re-init the header, restarted
          is true
          */
          continue;
        }
        break;
      }
      ThisRead = ThisRead - data_off - 1;
      allocation_info.file->Write(&Buffer[data_off + 1], ThisRead);
      Progress.SetPos(TotalRead += ThisRead);
      OnProgress.Execute(this, &Progress);
      restarted = false;  //proceed to normal operation
      continue;
    }
    if (ThisRead <= 0) {
      if (_OnReadFailed(info, TotalRead)) {
        restarted = true;
        continue;
      }
      break;
    }
    else {
      if (this->Break) { // user terminated
        break;
      }
      allocation_info.file->Write(Buffer, ThisRead);
      Progress.SetPos(TotalRead += ThisRead);
      OnProgress.Execute(this, &Progress);
    }
  }
  allocation_info.file->SetPosition(0);
  if (!_DoValidate(info, allocation_info.file)) {  // premature completion?
    Progress.SetPos(0);
    OnProgress.Exit(this, &Progress);
    return 0;
  }
  Progress.SetPos(info.contentLength);
  OnProgress.Exit(this, &Progress);
  return allocation_info.file.release();
}
//..............................................................................
bool AnHttpFileSystem::_DoesExist(const olxstr& f, bool forced_check) {
  if (Index != 0) {
    olxstr fn = TEFile::UnixPath(f);
    if (fn.StartsFrom(GetBase())) {
      return Index->GetRoot().FindByFullName(
        fn.SubStringFrom(GetBase().Length())) != 0;
    }
    else {
      return Index->GetRoot().FindByFullName(fn) != 0;
    }
  }
  if (!forced_check) {
    return false;
  }
  try {
    if (!Connect()) {
      return false;
    }
    const size_t BufferSize = 1024;
    olx_array_ptr<char> Buffer(BufferSize + 1);
    http_write(GenerateRequest("HEAD", f));
    int read = http_read(&Buffer, BufferSize);
    if (read <= 0) {
      return false;
    }
    TCStrList toks;
    // make sure that \r\n or \n\n are treated properly
    toks.LoadFromTextArray(Buffer.release(), read, true);
    return toks[0].EndsWith("200 OK");
  }
  catch (...) { return false; }
}
//..............................................................................
//..............................................................................
//..............................................................................
THttpFileSystem::~THttpFileSystem() {
  Disconnect();
}
//..............................................................................
void THttpFileSystem::Init() {
  Initialise();
  Access = afs_ReadOnlyAccess;
}
//..............................................................................
void THttpFileSystem::Initialise() {
#ifdef __WIN32__
  volatile olx_scope_cs _cs(TBasicApp::GetCriticalSection());
  if (!Initialised_()) {
    WSADATA WsaData;
    if (WSAStartup(0x0001, &WsaData) != 0) {
      throw TFunctionFailedException(__OlxSourceInfo, "Uninitialised WinSocks");
    }
    Initialised_() = true;
    TEGC::AddP(new THttpFileSystem::Finaliser());
  }
#endif
}
//..............................................................................
void THttpFileSystem::Finalise() {
#ifdef __WIN32__
  volatile olx_scope_cs _cs(TBasicApp::GetCriticalSection());
  if (Initialised_()) {
    WSACleanup();
    Initialised_() = false;
  }
#endif
}
//..............................................................................
void THttpFileSystem::GetAddress(struct sockaddr* Result)  {
  struct hostent* Host;
  sockaddr_in Address;
  memset(Result, 0, sizeof(*Result));
  memset(&Address, 0, sizeof(Address));
  olxstr HostAdd = Url.HasProxy() ? Url.GetProxy().GetHost() : Url.GetHost();
  // c_str() on unicode is not thread safe!
  Host = gethostbyname(olxcstr(HostAdd).c_str());
  if (Host != 0) {
    Address.sin_family  = AF_INET;
    Address.sin_port    = htons((unsigned short)(Url.HasProxy()
      ? Url.GetProxy().GetPort() : Url.GetPort()));
    memcpy(&Address.sin_addr, Host->h_addr_list[0], Host->h_length);
    memcpy(Result, &Address, sizeof(Address));
   }
   else {
     throw TFunctionFailedException(__OlxSourceInfo,
       olxstr("Can't map hostname: ") << Url.GetHost());
   }
}
//..............................................................................
void THttpFileSystem::Disconnect() {
  if (this->IsConnected()) {
#ifdef __WIN32__
    closesocket(Socket);
#else
    close(Socket);
#endif
  }
  AnHttpFileSystem::Disconnect();
}
//..............................................................................
bool THttpFileSystem::Connect() {
  Disconnect();
  struct sockaddr  SockAddr;
  GetAddress(&SockAddr);
  int  Status;
  Socket = socket(AF_INET, SOCK_STREAM, 0);
#ifdef __WIN32__
  int timeout = 10000; // ms ?
#else
  struct timeval timeout;
  memset(&timeout, 0, sizeof(timeout));
  timeout.tv_sec = 10;
#endif
  if (setsockopt(Socket, SOL_SOCKET, SO_RCVTIMEO,
      (char*)&timeout, sizeof(timeout)) != 0 ||
      setsockopt(Socket, SOL_SOCKET, SO_SNDTIMEO,
        (char*)&timeout, sizeof(timeout)) != 0)
  {
    throw TFunctionFailedException(__OlxSourceInfo,
      olxstr("Failed to setup timeout: ") << errno);
  }

  Status = connect(Socket, &SockAddr, sizeof(SockAddr));
  if (Status >= 0) {
    AnHttpFileSystem::Connect();
  }
  else {
    throw TFunctionFailedException(__OlxSourceInfo, "connection failed");
  }
  return this->IsConnected();
}
//..............................................................................
int THttpFileSystem::http_read(char* dest, size_t dest_sz) const {
  int rl = recv(Socket, dest, dest_sz, 0);
  if (rl <= 0 || (size_t)rl == dest_sz) {
    return rl;
  }
  size_t total_sz = rl;
  while (total_sz < dest_sz) {
    rl = recv(Socket, &dest[total_sz], dest_sz - total_sz, 0);
    if (rl <= 0) {
      return total_sz;
    }
    total_sz += rl;
  }
  return total_sz;
}
//..............................................................................
int THttpFileSystem::http_write(const olxcstr& str) {
  int sv = send(Socket, str.c_str(), str.Length(), 0);
  if (sv <= 0 || (size_t)sv == str.Length()) {
    return sv;
  }
  size_t total_sz = sv;
  while (total_sz < str.Length()) {
    sv = send(Socket, &str.c_str()[total_sz], str.Length()-total_sz, 0);
    if (sv <= 0) {
      return total_sz;
    }
    total_sz += sv;
  }
  return total_sz;
}
//..............................................................................
//..............................................................................
//..............................................................................
#ifdef __WIN32__
TWinHttpFileSystem::~TWinHttpFileSystem() {
}
//..............................................................................
void TWinHttpFileSystem::Init() {
  ExtraHeaders = 0;
}
//..............................................................................
// inspired by
//https://stackoverflow.com/questions/18910463/how-to-send-https-request-using-wininet
//..............................................................................
void TWinHttpFileSystem::SetUrl(const TUrl& url) {
  Url = url;
  SetBase(url.GetPath());
}
//..............................................................................
olx_object_ptr<IInputStream> TWinHttpFileSystem::_DoOpenFile(const olxstr& src) {
  DWORD service_id = rand();
  INET_CLOSE hInternet = InternetOpen(L"Olex2",
    INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
  if (hInternet.handle == 0) {
    return 0;
  }
  INET_CLOSE hConnect = InternetConnect(hInternet.handle,
    Url.GetHost().u_str(),
    INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0,
    service_id);
  if (hConnect.handle == 0) {
    return 0;
  }
  //PCTSTR rgpszAcceptTypes[] = { _T("application/zip"), NULL };
  INET_CLOSE hRequest = HttpOpenRequest(hConnect.handle,
    L"GET", src.u_str(),
    NULL, // version
    NULL, // referrer
    NULL, //rgpszAcceptTypes
    INTERNET_FLAG_SECURE,
    service_id);
  if (hRequest.handle == 0) {
    return 0;
  }
  olxstr rl = "\r\n", headers;
  if ((ExtraHeaders & httpHeaderPlatform) != 0) {
    headers << "Platform: " << TBasicApp::GetPlatformString(false) << rl;
  }
  if ((ExtraHeaders & httpHeaderESession) != 0) {
    headers << "ESession: " << AnHttpFileSystem::GetSessionInfo() << rl;
  }
  BOOL isSend = HttpSendRequest(hRequest.handle, headers.u_str(), headers.Length(), NULL, 0);
  if (!isSend) {
    return 0;
  }
  size_t content_len = 200000000;
  {
    DWORD bf_sz = 256;
    olx_array_ptr<char> cl(256);
    if (HttpQueryInfo(hRequest.handle, HTTP_QUERY_CONTENT_LENGTH, &cl, &bf_sz, NULL)) {
      HttpQueryInfo(hRequest.handle, HTTP_QUERY_CONTENT_LENGTH, &cl, &bf_sz, NULL);
      olxcstr t = &cl;
      if (t.IsNumber()) {
        content_len = t.ToSizeT();
      }
      else {
        return 0;
      }
    }
  }
  TOnProgress Progress;
  Progress.SetPos(0);
  Progress.SetAction(src);
  Progress.SetMax(content_len);
  OnProgress.Enter(this, &Progress);

  olx_object_ptr<TEFile> tmp = TEFile::TmpFile();
  DWORD read;
  size_t data_sz = 1024 * 64, total_read = 0;
  olx_array_ptr<char> bf(data_sz);
  BOOL isRead;
  while ((isRead=InternetReadFile(hRequest.handle, (void*)&bf, data_sz, &read)) == TRUE) {
    if (read == 0) {
      break;
    }
    tmp->Write(&bf, read);
    Progress.SetPos(total_read += read);
    OnProgress.Execute(this, &Progress);
    if (this->Break) { // user terminated
      if (total_read != content_len) {
        Progress.SetPos(0);
        OnProgress.Exit(this, &Progress);
        return 0;
      }
    }
  }
  if (isRead != TRUE) {
    return 0;
  }
  Progress.SetPos(content_len);
  OnProgress.Exit(this, &Progress);
  tmp->SetPosition(0);
  return tmp.release();
}
//..............................................................................
bool TWinHttpFileSystem::_DoesExist(const olxstr& f, bool forced_check) {
  if (Index != 0) {
    olxstr fn = TEFile::UnixPath(f);
    if (fn.StartsFrom(GetBase())) {
      return Index->GetRoot().FindByFullName(
        fn.SubStringFrom(GetBase().Length())) != 0;
    }
    else
      return Index->GetRoot().FindByFullName(fn) != 0;
  }
  if (!forced_check) {
    return false;
  }
  try {
    DWORD service_id = rand();
    INET_CLOSE hInternet = InternetOpen(L"Olex2",
      INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (hInternet.handle == 0) {
      return false;
    }
    INET_CLOSE hConnect = InternetConnect(hInternet.handle,
      Url.GetHost().u_str(),
      INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0,
      service_id);
    if (hConnect.handle == 0) {
      return false;
    }
    //PCTSTR rgpszAcceptTypes[] = { _T("application/zip"), NULL };
    INET_CLOSE hRequest = HttpOpenRequest(hConnect.handle,
      L"HEAD", f.u_str(),
      NULL, // version
      NULL, // referrer
      NULL, //rgpszAcceptTypes
      INTERNET_FLAG_SECURE,
      service_id);
    if (hRequest.handle == 0) {
      return false;
    }
    olxstr rl = "\r\n", headers;
    if ((ExtraHeaders & httpHeaderPlatform) != 0) {
      headers << "Platform: " << TBasicApp::GetPlatformString(false) << rl;
    }
    if ((ExtraHeaders & httpHeaderESession) != 0) {
      headers << "ESession: " << AnHttpFileSystem::GetSessionInfo() << rl;
    }
    BOOL isSend = HttpSendRequest(hRequest.handle, headers.u_str(), headers.Length(), NULL, 0);
    return isSend == TRUE;
  }
  catch (...) { return false; }
}
#endif // __WIN32__
//..............................................................................
//..............................................................................
//..............................................................................
#ifdef _OPENSSL
/* Helper function to create a BIO connected to the server */
static BIO* create_socket_bio(const char* hostname, const char* port,
  int family)
{
  BIO_ADDRINFO* res;
  /*
   * Lookup IP address info for the server.
   */
  if (!BIO_lookup(hostname, port, BIO_LOOKUP_CLIENT, family, SOCK_STREAM, &res)) {
    return 0;
  }

  /*
   * Loop through all the possible addresses for the server and find one
   * we can connect to.
   */
  const BIO_ADDRINFO* ai = 0;
  int sock = -1;
  for (ai = res; ai != 0; ai = BIO_ADDRINFO_next(ai)) {
    /*
     * Create a TCP socket. We could equally use non-OpenSSL calls such
     * as "socket" here for this and the subsequent connect and close
     * functions. But for portability reasons and also so that we get
     * errors on the OpenSSL stack in the event of a failure we use
     * OpenSSL's versions of these functions.
     */
    sock = BIO_socket(BIO_ADDRINFO_family(ai), SOCK_STREAM, 0, 0);
    if (sock == -1) {
      continue;
    }
    /* Connect the socket to the server's address */
    if (!BIO_connect(sock, BIO_ADDRINFO_address(ai), BIO_SOCK_NODELAY)) {
      BIO_closesocket(sock);
      sock = -1;
      continue;
    }
    /* We have a connected socket so break out of the loop */
    break;
  }

  /* Free the address information resources we allocated earlier */
  BIO_ADDRINFO_free(res);

  /* If sock is -1 then we've been unable to connect to the server */
  if (sock == -1) {
    return 0;
  }

  /* Create a BIO to wrap the socket */
  BIO* bio = BIO_new(BIO_s_socket());
  if (bio == 0) {
    BIO_closesocket(sock);
    return 0;
  }

  /*
   * Associate the newly created BIO with the underlying socket. By
   * passing BIO_CLOSE here the socket will be automatically closed when
   * the BIO is freed. Alternatively you can use BIO_NOCLOSE, in which
   * case you must close the socket explicitly when it is no longer
   * needed.
   */
  BIO_set_fd(bio, sock, BIO_CLOSE);
  return bio;
}
//..............................................................................
TSSLHttpFileSystem::~TSSLHttpFileSystem() {
  Disconnect();
}
//..............................................................................
void TSSLHttpFileSystem::Init() {
  ssl = 0;
  ctx = 0;
}
//..............................................................................
void TSSLHttpFileSystem::cleanup() {
  if (ssl != 0) {
    SSL_free(ssl);
    ssl = 0;
  }
  if (ctx != 0) {
    SSL_CTX_free(ctx);
    ctx = 0;
  }
}
//..............................................................................
bool TSSLHttpFileSystem::Connect() {
  ctx = SSL_CTX_new(TLS_client_method());
  if (ctx == 0) {
    return false;
  }
  SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, 0);
  if (!SSL_CTX_set_default_verify_paths(ctx)) {
    //printf("Failed to set the default trusted certificate store\n");
    return false;
  }
  if (!SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION)) {
    //printf("Failed to set the minimum TLS protocol version\n");
    TBasicApp::NewLogEntry() << "Failed 3";
    return false;
  }
  ssl = SSL_new(ctx);
  if (ssl == 0) {
    //printf("Failed to create the SSL object\n");
    SSL_CTX_free(ctx);
    return false;
  }
  olxcstr host = Url.GetHost();
  BIO* bio = create_socket_bio(host.c_str(), olxcstr(Url.GetPort()).c_str(), AF_INET);
  if (bio == 0) {
    //printf("Failed to crete the BIO\n");
    SSL_CTX_free(ctx);
    return false;
  }

  SSL_set_bio(ssl, bio, bio);
  if (!SSL_set_tlsext_host_name(ssl, host.c_str())) {
    //printf("Failed to set the SNI hostname\n");
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    return 0;
  }
   /*
    * Ensure we check during certificate verification that the server has
    * supplied a certificate for the hostname that we were expecting.
    * Virtually all clients should do this unless you really know what you
    * are doing.
    */
  if (!SSL_set1_host(ssl, host.c_str())) {
    //printf("Failed to set the certificate verification hostname");
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    return false;
  }

  /* Do the handshake with the server */
  if (SSL_connect(ssl) < 1) {
    /*
     * If the failure is due to a verification error we can get more
     * information about it from SSL_get_verify_result().
     */
    if (SSL_get_verify_result(ssl) != X509_V_OK) {
      TBasicApp::NewLogEntry() << "Verify error: "
        << X509_verify_cert_error_string(SSL_get_verify_result(ssl));
    }
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    return false;
  }
  return true;
}
//..............................................................................
void TSSLHttpFileSystem::Disconnect() {
  if (IsConnected()) {
    SSL_shutdown(ssl);
    cleanup();
  }
}
//..............................................................................
int TSSLHttpFileSystem::http_read(char* dest, size_t dest_sz) const {
  int rl = SSL_read(ssl, dest, dest_sz);
  if (rl <= 0 || (size_t)rl == dest_sz) {
    return rl;
  }
  size_t total_sz = rl;
  while (total_sz < dest_sz) {
    rl = SSL_read(ssl, &dest[total_sz], dest_sz - total_sz);
    if (rl <= 0) {
      return total_sz;
    }
    total_sz += rl;
  }
  return total_sz;
}
//..............................................................................
int TSSLHttpFileSystem::http_write(const olxcstr& str) {
  return SSL_write(ssl, str.c_str(), str.Length());
}
//..............................................................................
#endif // _OPENSSL
//..............................................................................
//..............................................................................
//..............................................................................
olx_object_ptr<AFileSystem> HttpFSFromURL(const TUrl& url) {
  if (url.GetProtocol() == "http") {
    return new THttpFileSystem(url);
  }
  if (url.GetProtocol() == "https") {
#ifdef __WIN32__
    return new TWinHttpFileSystem(url);
#else
    return new TSSLHttpFileSystem(url);
#endif
  }
  return 0;
}
//..............................................................................
