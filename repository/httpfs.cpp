#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "httpfs.h"
#include "efile.h"
#include "bapp.h"
#include "log.h"
#include <errno.h>

THttpFileSystem::THttpFileSystem(const TUrl& url): Url(url){
  Access = afs_ReadOnlyAccess;
#ifdef __WIN32__
  WSADATA  WsaData;
  Successful = (WSAStartup(0x0001, &WsaData) == 0);
  if( !Successful )
    throw TFunctionFailedException(__OlxSourceInfo, "could not initialise winsocks");
#endif
  SetBase( url.GetPath() );
  Connected = false;
  Connect();
}
//..............................................................................
THttpFileSystem::~THttpFileSystem()  {
  if( Connected )
    Disconnect();
#ifdef __WIN32__
  if( Successful )
    WSACleanup();
#endif
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
    Address.sin_port    = htons( (unsigned short)(Url.HasProxy() ? Url.GetProxy().GetPort() : Url.GetPort()) );
    memcpy(&Address.sin_addr, Host->h_addr_list[0], Host->h_length);
    memcpy(Result, &Address, sizeof(Address));
   }
   else
     throw TFunctionFailedException(__OlxSourceInfo,
       olxstr("Can't map hostname: ") << Url.GetHost() );
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
  if(Status >= 0)
    Connected = true;
  else
    throw TFunctionFailedException(__OlxSourceInfo, "connection failed");
  return Connected;
}
//..............................................................................
IInputStream* THttpFileSystem::_DoOpenFile(const olxstr& Source)  {
  TOnProgress Progress;
  if( Connected )  {
    Disconnect();
    Connect();
  }
  if( !Connected )
    throw TFunctionFailedException(__OlxSourceInfo, "could not connect");

  TEFile* File = TEFile::TmpFile();
  TEFile* File1 = TEFile::TmpFile();

  char  Request[512], *Buffer;
  olxstr FileName = TEFile::UnixPath( Source );

  const size_t BufferSize = 1024*64;

  uint64_t TotalRead = 0, FileLength = ~0;
  int ThisRead = 1;
  Buffer = new char[BufferSize+1];
  static const char LengthId[] = "Content-Length:", EndTagId[]="ETag:";
  bool FileAttached = false;

  olxcstr Tmp = Url.GetFullHost();
  Tmp << '/' << FileName;

  sprintf(Request, "GET %s HTTP/1.0\n\n", Tmp.Replace(' ', "%20").c_str());
  if( Url.HasProxy() && !Url.GetProxy().GetUser().IsEmpty() && !Url.GetProxy().GetPassword().IsEmpty() )
    sprintf(Request, "Authorization: %s\n\n", olxcstr(Url.GenerateHTTPAuthString()).c_str());

  send(Socket, Request, strlen(Request), 0);
  while( ThisRead )  {
    ThisRead = _read(Buffer, BufferSize);
    if( ThisRead <= 0 )
      break;
    else  {
      if( this->Break )  {  // premature termination
        Progress.SetPos(0);
        OnProgress.Exit(this, &Progress);
        delete [] Buffer;
        delete File1;
        delete File;
        return NULL;
      }
      File->Write(Buffer, ThisRead);
      if( TotalRead == 0 )  {
        Tmp.SetLength(0);
        size_t off = olxstr::o_strposi(Buffer, ThisRead, EndTagId, olxstr::o_strlen(EndTagId));
        if( off != InvalidIndex )  FileAttached = true;
        off = olxstr::o_strposi(Buffer, ThisRead, LengthId, olxstr::o_strlen(LengthId));
        if( off != InvalidIndex )  {
          off += olxstr::o_strlen(LengthId)+1;
          while( (off < (size_t)ThisRead) && Buffer[off] == ' ' ) {  off++; }  // skip spaces
          while( (off < (size_t)ThisRead) && olxstr::o_isdigit(Buffer[off]) )  {
            Tmp << Buffer[off];
            off++;
          }
          if( !Tmp.IsEmpty() && FileAttached )  {
            FileLength = Tmp.RadUInt<uint64_t>();
            Progress.SetPos(0);
            Progress.SetAction(Source);
            Progress.SetMax(FileLength);
            OnProgress.Enter(this, &Progress);
          }
        }
      }
      TotalRead += ThisRead;
    }
    if( FileLength != -1 && FileAttached )  {
      Progress.SetPos(TotalRead > Progress.GetMax() ? Progress.GetMax() : TotalRead);
      OnProgress.Execute(this, &Progress);
    }
  }
  if( olx_is_valid_size(FileLength) && (FileLength <= TotalRead) && FileAttached )  {
    File->Flush();
    File->SetPosition(TotalRead-FileLength);
    *File1 << *File;
    File1->SetPosition(0);
    delete File;
    Progress.SetPos(FileLength);
    OnProgress.Exit(this, &Progress);
    delete [] Buffer;
		return File1;
  }
  else  {
    Progress.SetPos(0);
    OnProgress.Exit(this, &Progress);
    delete [] Buffer;
    delete File1;
    delete File;
    return NULL;
  }
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
bool THttpFileSystem::_DoesExist(const olxstr& f, bool forced_check)  {
  if( Index != NULL )
    return Index->GetRoot().FindByFullName(f) != NULL;
  if( !forced_check )  return false;  

  if( Connected )  {
    Disconnect();
    Connect();
  }
  if( !Connected )
    throw TFunctionFailedException(__OlxSourceInfo, "could not connect");
  char  Request[512], *Buffer;
  olxstr FileName = TEFile::UnixPath(f);
  const size_t BufferSize = 1024;
  Buffer = new char[BufferSize+1];
  olxcstr Tmp = Url.GetFullHost();
  Tmp << '/' << FileName;
  sprintf(Request, "HEAD %s HTTP/1.0\n\n", Tmp.Replace(' ', "%20").c_str());
  if( Url.HasProxy() && !Url.GetProxy().GetUser().IsEmpty() && !Url.GetProxy().GetPassword().IsEmpty() )
    sprintf(Request, "Authorization: %s\n\n", olxcstr(Url.GenerateHTTPAuthString()).c_str());
  send(Socket, Request, strlen(Request), 0);
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
//..............................................................................

