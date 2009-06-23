#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "winhttpfs.h"

#ifdef __WIN32__ // nonportable code

#include "efile.h"
#include "bapp.h"

TWinHttpFileSystem::TWinHttpFileSystem(const TUrl& url): Url(url){
  Access = afs_ReadOnlyAccess;
  WSADATA  WsaData;
  Successful = (WSAStartup(0x0001, &WsaData) == 0);
  if( !Successful )
    throw TFunctionFailedException(__OlxSourceInfo, "could not initialise winsocks");
  SetBase( url.GetPath() );
  Connected = false;
  Connect();
}
//..............................................................................
TWinHttpFileSystem::~TWinHttpFileSystem()  {
  if( Connected )
    Disconnect();
  if( Successful )
    WSACleanup();

  for( int i=0; i < TmpFiles.Count(); i++ )
    TEFile::DelFile( TmpFiles[i] );
}
//..............................................................................
void TWinHttpFileSystem::GetAddress(struct sockaddr* Result)  {
  struct hostent* Host;
  SOCKADDR_IN     Address;

  memset(Result, 0, sizeof(*Result));
  memset(&Address, 0, sizeof(Address));

  olxstr HostAdd = Url.HasProxy() ? Url.GetProxy().GetHost() : Url.GetHost();
  Host = gethostbyname( HostAdd.c_str() );
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
void TWinHttpFileSystem::Disconnect()  {
  if( Connected )  {
    Connected = false;
    closesocket(Socket);
  }
}
//..............................................................................
bool TWinHttpFileSystem::Connect()  {
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
    throw TFunctionFailedException(__OlxSourceInfo, olxstr("connection failed: ") << WSAGetLastError());
  int timeout = 5000; // ms ?
  if( setsockopt(Socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(int)) != 0 )
    throw TFunctionFailedException(__OlxSourceInfo, olxstr("Failed to setup timeout: ") << WSAGetLastError());
  return Connected;
}
//..............................................................................
IInputStream* TWinHttpFileSystem::_DoOpenFile(const olxstr& Source)  {
  TOnProgress Progress;
  if( Connected )  {
    Disconnect();
    Connect();
  }
  if( !Connected )
    throw TFunctionFailedException(__OlxSourceInfo, "could not connect");

  TEFile File;
  TEFile* File1 = new TEFile();

  char  Request[512], *Buffer;
  olxch TmpFN[MAX_PATH];
  GetTempPath(MAX_PATH, TmpFN);
  olxstr Tmp,
         FileName = TEFile::UnixPath( Source ),
         fn_mask("http");

  Tmp = TmpFN;
  GetTempFileName(Tmp.u_str(), fn_mask.u_str(), 0, TmpFN);
  File.Open(TmpFN, "w+b");

  GetTempFileName(Tmp.u_str(), fn_mask.u_str(), 0, TmpFN);
  File1->Open(TmpFN, "w+b");

  int i, parts,
    BufferSize = 1024*64,
    TotalRead = 0,
    ThisRead = 1,
    FileLength = -1;
  Buffer = new char[BufferSize];
  static const char LengthId[] = "Content-Length:", EndTagId[]="ETag:";

  bool FileAttached = false;

  Tmp = Url.GetFullHost();
  Tmp << '/' << FileName;

  Tmp.Replace(' ', "%20");
  sprintf(Request, "GET %s HTTP/1.0\n\n", Tmp.c_str());
  if( Url.HasProxy() && !Url.GetProxy().GetUser().IsEmpty() && !Url.GetProxy().GetPassword().IsEmpty() )
    sprintf(Request, "Authorization: %s\n\n", Url.GenerateHTTPAuthString().c_str());

  send(Socket, Request, strlen(Request), 0);
  while( ThisRead )  {
    ThisRead = recv(Socket, Buffer, BufferSize, 0);
    if( ThisRead == 0 )  break;
    if( ThisRead == SOCKET_ERROR )  break;
    else  {
      File.Write(Buffer, ThisRead);
      if( TotalRead == 0 )  {
        Tmp.SetLength(0);
        int off = olxstr::o_strposi(Buffer, ThisRead, EndTagId, olxstr::o_strlen(EndTagId));
        if( off != -1 )  FileAttached = true;
        off = olxstr::o_strposi(Buffer, ThisRead, LengthId, olxstr::o_strlen(LengthId));
        if( off != -1 )  {
          off += strlen(LengthId)+1;
          while( (off < ThisRead) && Buffer[off] == ' ' ) {  off++; }  // skipp spaces
          while( (off < ThisRead) && Buffer[off] >= '0' && Buffer[off] <= '9')  {
            Tmp << Buffer[off];
            off++;
          }
          if( !Tmp.IsEmpty() && FileAttached )  {
            FileLength = Tmp.ToInt();
            Progress.SetPos( 0 );
            Progress.SetAction(Source);
            Progress.SetMax( FileLength );
            OnProgress->Enter(this, &Progress);
          }
        }
      }
      TotalRead += ThisRead;
    }
    if( FileLength != -1 && FileAttached )  {
      Progress.SetPos(TotalRead > Progress.GetMax() ? Progress.GetMax() : TotalRead);
      OnProgress->Execute(this, &Progress);
    }
  }
  if( (FileLength != -1) && (FileLength <= TotalRead) && FileAttached )  {
    File.Flush();
    File.Seek(TotalRead-FileLength, SEEK_SET);
    parts = FileLength/BufferSize;
    for( i=0; i < parts; i++ )  {
      File.Read(Buffer, BufferSize);
      File1->Write(Buffer, BufferSize);
    }
    parts = FileLength%BufferSize;
    if( parts )  {
      File.Read(Buffer, parts);
      File1->Write(Buffer, parts);
    }
    TmpFiles.Add( File1->GetName() );
    File.Delete();
    File1->Seek(0, SEEK_SET);
    Progress.SetPos(FileLength);
    OnProgress->Exit(this, &Progress);
    delete [] Buffer;
    return File1;
  }
  else  {
    Progress.SetPos(0);
    OnProgress->Exit(this, &Progress);
    File1->Delete();
    File.Delete();
    delete [] Buffer;
    delete File1;
    return NULL;
  }
}
//..............................................................................
bool TWinHttpFileSystem::_DoesExist(const olxstr& f)  {  
  if( Index != NULL )
    return Index->GetRoot().FindByFullName(f);
  return false;  
}
//..............................................................................
#endif  // __WIN32__

