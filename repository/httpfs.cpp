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
  Host = gethostbyname( olxcstr(HostAdd).c_str() );  // c_str() on unicode is not thread safe!
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
  Status = connect(Socket, &SockAddr, sizeof(SockAddr));
  if(Status >= 0)
    Connected = true;
  else
    throw TFunctionFailedException(__OlxSourceInfo, "connection failed");
#ifdef __WIN32__
  int timeout = 10000; // ms ?
#else
  struct timeval timeout;
	memset(&timeout, 0, sizeof(timeout));
	timeout.tv_sec = 10;
#endif
  if( setsockopt(Socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) != 0 )
    throw TFunctionFailedException(__OlxSourceInfo, olxstr("Failed to setup timeout: ") << errno);
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

  const int BufferSize = 1024*64;

  int TotalRead = 0,
    ThisRead = 1,
    FileLength = -1;
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
    ThisRead = recv(Socket, Buffer, BufferSize, 0);
    if( ThisRead == 0 )  break;
    if( ThisRead == -1 )  break;
    else  {
      File->Write(Buffer, ThisRead);
      if( TotalRead == 0 )  {
        Tmp.SetLength(0);
        int off = olxstr::o_strposi(Buffer, ThisRead, EndTagId, olxstr::o_strlen(EndTagId));
        if( off != -1 )  FileAttached = true;
        off = olxstr::o_strposi(Buffer, ThisRead, LengthId, olxstr::o_strlen(LengthId));
        if( off != -1 )  {
          off += strlen(LengthId)+1;
          while( (off < ThisRead) && Buffer[off] == ' ' ) {  off++; }  // skip spaces
          while( (off < ThisRead) && Buffer[off] >= '0' && Buffer[off] <= '9')  {
            Tmp << Buffer[off];
            off++;
          }
          if( !Tmp.IsEmpty() && FileAttached )  {
            FileLength = Tmp.ToInt();
            Progress.SetPos( 0 );
            Progress.SetAction(Source);
            Progress.SetMax( FileLength );
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
  if( (FileLength != -1) && (FileLength <= TotalRead) && FileAttached )  {
    File->Flush();
    File->Seek(TotalRead-FileLength, SEEK_SET);
    int parts = FileLength/BufferSize;
    for( int i=0; i < parts; i++ )  {
      File->Read(Buffer, BufferSize);
      File1->Write(Buffer, BufferSize);
    }
    parts = FileLength%BufferSize;
    if( parts )  {
      File->Read(Buffer, parts);
      File1->Write(Buffer, parts);
    }
    File1->Seek(0, SEEK_SET);
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
bool THttpFileSystem::_DoesExist(const olxstr& f)  {  
  if( Index != NULL )
    return Index->GetRoot().FindByFullName(f) != NULL;
  return false;  
}
//..............................................................................

