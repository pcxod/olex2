#include "httpfs.h"
#include "efile.h"
#include "bapp.h"
#include "log.h"
#include <errno.h>

#ifdef __WIN32__
  bool THttpFileSystem::Initialised = false;
#endif
//..............................................................................
THttpFileSystem::THttpFileSystem(const TUrl& url) : Url(url){
#ifdef __WIN32__
  if( !Initialised )
    Initialise();
#endif
  Access = afs_ReadOnlyAccess;
  SetBase(url.GetPath());
  Connected = false;
}
//..............................................................................
THttpFileSystem::~THttpFileSystem()  {
  if( Connected )
    Disconnect();
}
//..............................................................................
void THttpFileSystem::Initialise()  {
#ifdef __WIN32__
  volatile olx_scope_cs _cs(TBasicApp::GetCriticalSection());
  if( !Initialised )  {
    WSADATA  WsaData;
    if( WSAStartup(0x0001, &WsaData) != 0 )
      throw TFunctionFailedException(__OlxSourceInfo, "Uninitialised WinSocks");
    Initialised = true;
    TEGC::AddP(new THttpFileSystem::Finaliser);
  }
#endif
}
//..............................................................................
void THttpFileSystem::Finalise()  {
#ifdef __WIN32__
  if( Initialised )  {  
    volatile olx_scope_cs _cs(TBasicApp::GetCriticalSection());
    WSACleanup();
    Initialised = false;
  }
#endif
}
//..............................................................................
void THttpFileSystem::DoConnect()  {
 if( IsConnected() )
    Disconnect();
  Connect();
}
//..............................................................................
void THttpFileSystem::GetAddress(struct sockaddr* Result)  {
  struct hostent* Host;
  sockaddr_in Address;
  memset(Result, 0, sizeof(*Result));
  memset(&Address, 0, sizeof(Address));
  olxstr HostAdd = Url.HasProxy() ? Url.GetProxy().GetHost() : Url.GetHost();
  Host = gethostbyname(olxcstr(HostAdd).c_str());  // c_str() on unicode is not thread safe!
  if( Host != NULL )  {
    Address.sin_family  = AF_INET;
    Address.sin_port    = htons((unsigned short)(Url.HasProxy() ? Url.GetProxy().GetPort() : Url.GetPort()));
    memcpy(&Address.sin_addr, Host->h_addr_list[0], Host->h_length);
    memcpy(Result, &Address, sizeof(Address));
   }
   else
     throw TFunctionFailedException(__OlxSourceInfo, olxstr("Can't map hostname: ") << Url.GetHost() );
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
    throw TFunctionFailedException(__OlxSourceInfo, olxstr("Failed to setup timeout: ") << errno);

  Status = connect(Socket, &SockAddr, sizeof(SockAddr));
  if( Status >= 0 )
    Connected = true;
  else
    throw TFunctionFailedException(__OlxSourceInfo, "connection failed");
  return Connected;
}
//..............................................................................
olxcstr THttpFileSystem::GenerateRequest(const TUrl& url, const olxcstr& cmd, const olxcstr& FileName)  {
  olxcstr request(cmd);
  request << ' ' << (url.GetFullHost() << '/' <<
    TEFile::UnixPath(FileName)).Replace(' ', "%20") << " HTTP/1.0\n";
  if( url.HasProxy() && !url.GetProxy().GetUser().IsEmpty() && !url.GetProxy().GetPassword().IsEmpty() )
    request << "Authorization: " << olxcstr(url.GenerateHTTPAuthString()) << '\n';
  request << "Platform: " << TBasicApp::GetPlatformString() << '\n';
  return request << '\n';
}
//..............................................................................
THttpFileSystem::ResponseInfo THttpFileSystem::ParseResponseInfo(const olxcstr& str, const olxcstr& sep)  {
  ResponseInfo rv;
  if( str.IsEmpty() )
    return rv;
  const TCStrList header_toks(str, sep);
  rv.status = header_toks[0];
  for( size_t i=1; i < header_toks.Count(); i++ )  {
    const size_t si = header_toks[i].IndexOf(':');
    if( si == InvalidIndex )  continue;
    rv.headers(header_toks[i].SubStringTo(si), header_toks[i].SubStringFrom(si+1).Trim(' '));
  }
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
  const size_t BufferSize = 1024*64;
  char* Buffer = new char[BufferSize+1];
  // read and extract headers
  _write(GenerateRequest(GetUrl(), "GET", Source));
  int ThisRead = _read(Buffer, BufferSize);
  // find the data start offset
  bool crlf = false;
  try  {  crlf = IsCrLf(Buffer, ThisRead);  }
  catch(...)  {
    delete [] Buffer;
    return NULL;
  }
  uint64_t TotalRead = 0, FileLength = ~0;
  size_t data_off = GetDataOffset(Buffer, ThisRead, crlf);
  const ResponseInfo info = ParseResponseInfo(olxcstr(Buffer, data_off), olxcstr(crlf ? "\r\n" : "\n"));
  try  {
    FileLength = info.headers.Find("Content-Length", "-1").ToInt();
  }
  catch(...)  {}  // toInt my throw an exception
  if( FileLength == ~0 || !info.status.EndsWithi("200 OK") || !info.headers.HasKey("ETag") )  {
    delete [] Buffer;
    return NULL;
  }
  const olxcstr server_name = info.headers.Find("Server", CEmptyString);
  const olxcstr etag = info.headers["ETag"];
  Progress.SetPos(0);
  Progress.SetAction(Source);
  Progress.SetMax(FileLength);
  OnProgress.Enter(this, &Progress);
  TEFile* File = TEFile::TmpFile();
  TotalRead = ThisRead-data_off-1;
  File->Write(&Buffer[data_off+1], TotalRead);
  bool restarted = false;
  while( TotalRead != FileLength )  {
    if( TotalRead > FileLength ) // could happen?
      break;
    ThisRead = _read(Buffer, BufferSize);
    if( restarted && ThisRead > 0 )  {
      data_off = GetDataOffset(Buffer, ThisRead, crlf);
      ThisRead = ThisRead-data_off-1;
      File->Write(&Buffer[data_off+1], ThisRead);
      Progress.SetPos(TotalRead+=ThisRead);
      OnProgress.Execute(this, &Progress);
      restarted = false;
      continue;
    }
    if( ThisRead <= 0 )  {
      if( _OnReadFailed(server_name, Source, TotalRead) )  {
        restarted = true;
        continue;
      }
      break;
    }
    else  {
      if( this->Break )// user terminated
        break;
      File->Write(Buffer, ThisRead);
      Progress.SetPos(TotalRead+=ThisRead);
      OnProgress.Execute(this, &Progress);
    }
  }
  delete [] Buffer;
  File->SetPosition(0);
  if( TotalRead != FileLength || !_DoValidate(server_name, etag, *File) )  {  // premature completion?
    Progress.SetPos(0);
    OnProgress.Exit(this, &Progress);
    delete File;
    return NULL;
  }
  File->Flush();
  File->SetPosition(0);
  Progress.SetPos(FileLength);
  OnProgress.Exit(this, &Progress);
  return File;
}
//..............................................................................
int THttpFileSystem::_read(char* dest, size_t dest_sz) const {
  int rl = recv(Socket, dest, dest_sz, 0);
  if( rl <= 0 )  return rl;
  else if( rl == dest_sz )
    return rl;
  size_t total_sz = rl;
  while( total_sz < dest_sz )  {
    rl = recv(Socket, &dest[total_sz], dest_sz-total_sz, 0);
    if( rl < 0 )  return rl;
    if( rl == 0 )
      return total_sz;
    total_sz += rl;
  }
  return total_sz;
}
//..............................................................................
int THttpFileSystem::_write(const olxcstr& str)  {
  return send(Socket, str.c_str(), str.Length(), 0);
}
//..............................................................................
bool THttpFileSystem::_DoesExist(const olxstr& f, bool forced_check)  {
  if( Index != NULL )
    return Index->GetRoot().FindByFullName(f) != NULL;
  if( !forced_check )  return false;  
  try  {
    DoConnect();
    const size_t BufferSize = 1024;
    char* Buffer = new char[BufferSize+1];
    _write(GenerateRequest(GetUrl(), "HEAD", f));
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

