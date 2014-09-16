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
THttpFileSystem::~THttpFileSystem() {
  if (Connected)
    Disconnect();
}
//..............................................................................
void THttpFileSystem::Init() {
  Initialise();
  ExtraHeaders = 0;
  Access = afs_ReadOnlyAccess;
  Connected = false;
}
//..............................................................................
void THttpFileSystem::SetUrl(const TUrl& url) {
  if (Connected) Disconnect();
  Initialise();
  if (SessionInfo_().IsEmpty()) { // not UUID, but quite unique
    SessionInfo_() = MD5::Digest(olxstr(TETime::msNow()));
    olx_sleep(1);
    SessionInfo_() = MD5::Digest(SessionInfo_() + olxcstr(TETime::msNow()));
  }
  Url = url;
  SetBase(url.GetPath());
}
//..............................................................................
void THttpFileSystem::Initialise() {
#ifdef __WIN32__
  volatile olx_scope_cs _cs(TBasicApp::GetCriticalSection());
  if (!Initialised_()) {
    WSADATA WsaData;
    if (WSAStartup(0x0001, &WsaData) != 0)
      throw TFunctionFailedException(__OlxSourceInfo, "Uninitialised WinSocks");
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
void THttpFileSystem::DoConnect() {
 if (IsConnected()) Disconnect();
  Connect();
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
  if( Host != NULL )  {
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
void THttpFileSystem::Disconnect()  {
  if( Connected )  {
    Connected = false;
#ifdef __WIN32__
    closesocket(Socket);
#else
  close(Socket);
#endif
  }
}
//..............................................................................
bool THttpFileSystem::Connect()  {
  if( Connected )
    Disconnect();
  struct sockaddr  SockAddr;
  Connected = false;
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
  if( setsockopt(Socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) != 0 ||
      setsockopt(Socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout)) != 0 )
  {
    throw TFunctionFailedException(__OlxSourceInfo,
      olxstr("Failed to setup timeout: ") << errno);
  }

  Status = connect(Socket, &SockAddr, sizeof(SockAddr));
  if( Status >= 0 )
    Connected = true;
  else
    throw TFunctionFailedException(__OlxSourceInfo, "connection failed");
  return Connected;
}
//..............................................................................
olxcstr THttpFileSystem::GenerateRequest(const olxcstr& cmd, const olxcstr& FileName,
    uint64_t position)
{
  olxcstr request(cmd);
  request << ' '
    << TUtf8::Encode(Url.GetFullHost() << '/' <<
    TEFile::UnixPath(FileName)).Replace(' ', "%20") << " HTTP/1.0\n";
  if (Url.HasProxy() && !Url.GetProxy().GetUser().IsEmpty() &&
      !Url.GetProxy().GetPassword().IsEmpty())
  {
    request << "Authorization: " << olxcstr(Url.GenerateHTTPAuthString()) << '\n';
  }
  request << "Host: " << Url.GetHost() << ':' << Url.GetPort() << '\n';
  if ((ExtraHeaders & httpHeaderPlatform) != 0)
    request << "Platform: " << TBasicApp::GetPlatformString() << '\n';
  if ((ExtraHeaders & httpHeaderESession) != 0)
    request << "ESession: " << SessionInfo_() << '\n';
  if (position != InvalidIndex && position != 0)
    request << "Resume-From: " << position << '\n';
  return request << '\n';
}
//..............................................................................
THttpFileSystem::ResponseInfo THttpFileSystem::ParseResponseInfo(
  const olxcstr& str, const olxcstr& sep, const olxstr& src)
{
  ResponseInfo rv;
  if( str.IsEmpty() )  return rv;
  const TCStrList header_toks(str, sep);
  rv.status = header_toks[0];
  for( size_t i=1; i < header_toks.Count(); i++ )  {
    const size_t si = header_toks[i].IndexOf(':');
    if( si == InvalidIndex )  continue;
    rv.headers(header_toks[i].SubStringTo(si),
      header_toks[i].SubStringFrom(si+1).Trim(' '));
  }
  size_t ii = rv.headers.IndexOf("Content-MD5");
  if( ii != InvalidIndex )  {
    rv.contentMD5 = rv.headers.GetValue(ii);
    rv.headers.Delete(ii);
  }
  ii = rv.headers.IndexOf("Content-Length");
  if( ii != InvalidIndex )  {
    try  {  rv.contentLength = rv.headers.GetValue(ii).ToInt();  }
    catch(...)  {}  // toInt my throw an exception
    rv.headers.Delete(ii);
  }
  rv.source = src;
  return rv;
}
//..............................................................................
bool THttpFileSystem::IsCrLf(const char* bf, size_t len)  {
  for( size_t i=1; i < len; i++ )  {
    if( bf[i] == '\n' )
      return (bf[i-1] == '\r');
  }
  throw TFunctionFailedException(__OlxSourceInfo, "could not locate reference char");
}
//..............................................................................
size_t THttpFileSystem::GetDataOffset(const char* bf, size_t len, bool crlf)  {
  if( crlf )  {
    for( size_t i=3; i < len; i++ )
      if( bf[i] == '\n' && bf[i-1] == '\r' && bf[i-2] == '\n' && bf[i-3] == '\r' )
        return i;
  }
  else  {
    for( size_t i=1; i < len; i++ )
      if( bf[i-1] == '\n' && bf[i] == '\n' )
        return i;
  }
  return 0;
}
//..............................................................................
IInputStream* THttpFileSystem::_DoOpenFile(const olxstr& Source)  {
  TOnProgress Progress;
  DoConnect();
  const size_t BufferSize = 1024*16;
  char* Buffer = new char[BufferSize+1];
  AllocationInfo allocation_info = _DoAllocateFile(Source);
  if( allocation_info.file == NULL )  {
    delete [] Buffer;
    return NULL;
  }
  // if the file length is not 0, it means that the CDS of any kind is in place...
  uint64_t starting_file_len = allocation_info.file->Length();
  // read and extract headers
  _write(GenerateRequest("GET", Source, starting_file_len));
  int ThisRead = _read(Buffer, 512);
  // find the data start offset
  bool crlf = false;
  try  {  crlf = IsCrLf(Buffer, ThisRead);  }
  catch(...)  {
    delete [] Buffer;
    delete allocation_info.file;
    return NULL;
  }
  uint64_t TotalRead = starting_file_len;
  size_t data_off = GetDataOffset(Buffer, ThisRead, crlf);
  const olxcstr line_break(crlf ? "\r\n" : "\n");
  ResponseInfo info =
    ParseResponseInfo(olxcstr(Buffer, data_off), line_break, Source);
  if( !info.HasData() )  {
    delete allocation_info.file;
    delete [] Buffer;
    return NULL;
  }
  // validate if we continue download for the same file...
  if( !allocation_info.truncated && allocation_info.digest != info.contentMD5 )  {
    _DoTruncateFile(allocation_info);
    if( allocation_info.file == NULL )  {
      delete [] Buffer;
      return NULL;
    }
    starting_file_len = TotalRead = 0;
  }
  Progress.SetPos(0);
  Progress.SetAction(Source);
  Progress.SetMax(info.contentLength);
  OnProgress.Enter(this, &Progress);
  TotalRead = starting_file_len+ThisRead-data_off-1;
  allocation_info.file->Write(&Buffer[data_off+1], (size_t)(TotalRead-starting_file_len));
  bool restarted = false;
  while( TotalRead != info.contentLength )  {
    if( TotalRead > info.contentLength ) // could happen?
      break;
    ThisRead = _read(Buffer, BufferSize);
    if( restarted && ThisRead > 0 )  {
      data_off = GetDataOffset(Buffer, ThisRead, crlf);
      if( data_off == 0 )  {
        TBasicApp::NewLogEntry(logInfo, true) << "Restarted download: header is to short, retrying";
        if( _OnReadFailed(info, TotalRead) )
          continue;  //try another attempt
        break;
      }
      // get the info
      ResponseInfo new_info =
        ParseResponseInfo(olxcstr(Buffer, data_off), line_break, Source);
      if( !new_info.HasData() )
        break;

      if( TotalRead == 0 )  // 1. previously restarted from 2
        info = new_info;
      else if( new_info != info  )  {  // have to restart...
        TBasicApp::NewLogEntry(logInfo, true) << "Restarted download: info mismatch old={"
          << '(' << info.contentMD5 << ',' << info.contentLength <<
          "}, new={" << new_info.contentMD5 << ',' << new_info.contentLength << '}';
        info = new_info;
        _DoTruncateFile(allocation_info);
        if( allocation_info.file == NULL )  {
          delete [] Buffer;
          return NULL;
        }
        TotalRead = 0;
        if( _OnReadFailed(info, TotalRead) )
          continue;  // 2. continue to the same block to 1 re-init the header, restarted is true
        break;
      }
      ThisRead = ThisRead-data_off-1;
      allocation_info.file->Write(&Buffer[data_off+1], ThisRead);
      Progress.SetPos(TotalRead+=ThisRead);
      OnProgress.Execute(this, &Progress);
      restarted = false;  //proceed to normal operation
      continue;
    }
    if( ThisRead <= 0 )  {
      if( _OnReadFailed(info, TotalRead) )  {
        restarted = true;
        continue;
      }
      break;
    }
    else  {
      if( this->Break )// user terminated
        break;
      allocation_info.file->Write(Buffer, ThisRead);
      Progress.SetPos(TotalRead+=ThisRead);
      OnProgress.Execute(this, &Progress);
    }
  }
  delete [] Buffer;
  allocation_info.file->SetPosition(0);
  if( !_DoValidate(info, *allocation_info.file) )  {  // premature completion?
    Progress.SetPos(0);
    OnProgress.Exit(this, &Progress);
    delete allocation_info.file;
    return NULL;
  }
  Progress.SetPos(info.contentLength);
  OnProgress.Exit(this, &Progress);
  return allocation_info.file;
}
//..............................................................................
int THttpFileSystem::_read(char* dest, size_t dest_sz) const {
  int rl = recv(Socket, dest, dest_sz, 0);
  if( rl <= 0 || (size_t)rl == dest_sz )
    return rl;
  size_t total_sz = rl;
  while( total_sz < dest_sz )  {
    rl = recv(Socket, &dest[total_sz], dest_sz-total_sz, 0);
    if( rl <= 0 )
      return total_sz;
    total_sz += rl;
  }
  return total_sz;
}
//..............................................................................
int THttpFileSystem::_write(const olxcstr& str)  {
  int sv = send(Socket, str.c_str(), str.Length(), 0);
  if( sv <= 0 || (size_t)sv == str.Length() )
    return sv;
  size_t total_sz = sv;
  while( total_sz < str.Length() )  {
    sv = send(Socket, &str.c_str()[total_sz], str.Length()-total_sz, 0);
    if( sv <= 0 )
      return total_sz;
    total_sz += sv;
  }
  return total_sz;
}
//..............................................................................
bool THttpFileSystem::_DoesExist(const olxstr& f, bool forced_check)  {
  if( Index != NULL )  {
    olxstr fn = TEFile::UnixPath(f);
    if( fn.StartsFrom(GetBase()) )
      return Index->GetRoot().FindByFullName(fn.SubStringFrom(GetBase().Length())) != NULL;
    else
      return Index->GetRoot().FindByFullName(fn) != NULL;
  }
  if( !forced_check )  return false;
  try  {
    DoConnect();
    const size_t BufferSize = 1024;
    char* Buffer = new char[BufferSize+1];
    _write(GenerateRequest("HEAD", f));
    int read = _read(Buffer, BufferSize);
    if( read <= 0 )  {
      delete [] Buffer;
      return false;
    }
    TCStrList toks;
    // make sure that \r\n or \n\n are treated properly
    toks.LoadFromTextArray(Buffer, read, true);
    return toks[0].EndsWith("200 OK");
  }
  catch(...)  {  return false;  }
}
//..............................................................................
