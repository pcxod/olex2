#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "winhttpfs.h"
#include "efile.h"
#include "bapp.h"

TWinHttpFileSystem::TWinHttpFileSystem(const TUrl& url): Url(url){
  WSADATA  WsaData;
  WSAStartup(0x0001, &WsaData);
  SetBase( url.GetPath() );
  Connect();
}
//..............................................................................
TWinHttpFileSystem::~TWinHttpFileSystem()  {
  if( Connected )
    Disconnect();

  WSACleanup();

  for( int i=0; i < TmpFiles.Count(); i++ )
    TEFile::DelFile( TmpFiles.String(i) );
}
//..............................................................................
void TWinHttpFileSystem::GetAddress(struct sockaddr* Result)  {
  struct hostent* Host;
  SOCKADDR_IN     Address;

  memset(Result, 0, sizeof(*Result));
  memset(&Address, 0, sizeof(Address));

  olxstr HostAdd = Url.HasProxy() ? Url.GetProxy().GetHost() : Url.GetHost();

  Host = gethostbyname( HostAdd.u_str() );
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
  return Connected;
}
//..............................................................................
IDataInputStream* TWinHttpFileSystem::OpenFile(const olxstr& Source)  {
  TOnProgress Progress;
  Progress.SetAction("Connecting to the server...");
  Progress.SetPos(0);
  Progress.SetMax(1);
  TBasicApp::GetInstance()->OnProgress->Execute(this, &Progress);
  if( Connected )  {
    Disconnect();
    Connect();
  }
  if( !Connected )
    throw TFunctionFailedException(__OlxSourceInfo, "could not connect");

  TEFile File;
  TEFile* File1 = new TEFile();

  char  Request[512], *Buffer;
  GetTempPath(512, Request);
  olxstr Tmp,
           FileName = TEFile::UnixPath( Source );

  Tmp = Request;
  GetTempFileName(Tmp.u_str(), "http", 0, Request);
  File.Open(Request, "w+b");

  GetTempFileName(Tmp.u_str(), "http", 0, Request);
  File1->Open(Request, "w+b");

  int i, parts,
    BufferSize = 1024*64,
    TotalRead = 0,
    ThisRead = 1,
    FileLength = -1;
  Buffer = new char[BufferSize];
  char LengthId[] = "Content-Length:", EndTagId[]="ETag:";

  bool FileAttached = false;

  Tmp = Url.GetFullHost();
  Tmp << '/' << FileName;

  Progress.SetAction(Source);
  TBasicApp::GetInstance()->OnProgress->Execute(this, &Progress);

  Tmp.Replace(" ", "%20");
  sprintf(Request, "GET %s HTTP/1.0\n\n", Tmp.u_str());
  send(Socket, Request, strlen(Request), 0);
  while( ThisRead )  {
    ThisRead = recv(Socket, Buffer, BufferSize, 0);

    if( !ThisRead )  break;

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
          if( !Tmp.IsEmpty() )  {
            FileLength = Tmp.ToInt();
            Progress.SetPos( 0 );
            Progress.SetMax( FileLength );
            TBasicApp::GetInstance()->OnProgress->Enter(this, &Progress);
          }
        }
      }
      TotalRead += ThisRead;
    }
    if( FileLength != -1 )  {
      Progress.SetPos(TotalRead);
      TBasicApp::GetInstance()->OnProgress->Execute(this, &Progress);
    }
  }
  if( (FileLength != -1) && (FileLength <= TotalRead) )  {
    if( FileAttached )  {
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
    }
    File.Delete();
    File1->Seek(0, SEEK_SET);
    Progress.SetAction("Download complete");
    Progress.SetPos(FileLength);
    TBasicApp::GetInstance()->OnProgress->Exit(this, &Progress);
    return File1;
  }
  else  {
    Progress.SetPos(0);
    Progress.SetAction("Download failed");
    TBasicApp::GetInstance()->OnProgress->Execute(this, &Progress);
    File1->Delete();
    File.Delete();
    delete File1;
    return NULL;
  }
}
//..............................................................................

