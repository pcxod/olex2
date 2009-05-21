//---------------------------------------------------------------------------

#pragma hdrstop

#include "fsindex.h"
#include <io.h>
#include <sys\stat.h>
#include <fcntl.h>
#include <dir.h>
#include "efile.h"
#include "bapp.h"
#include "log.h"

  EndIEObjectImplementation(TFSItem)
  EndIEObjectImplementation(TFSIndex)
//..............................................................................
//..............................................................................
//..............................................................................
bool TOSFS::DelFile(const TEString& FN)
{  
  //return TEFile::DelFile(FN);  
  return true;
}
//..............................................................................
bool TOSFS::DelDir(const TEString& DN)
{  
/*  TEString CD = TEFile::CurrentDir(), Dir;
  TEStringList SL;
  TEString::Strtok(DN, '\\', &SL);
  Dir = SL.String(SL.Count()-1);
  SL.Delete(SL.Count()-1);
  TEFile::ChangeDir(SL.Text('\\'));
  int res = rmdir(Dir.Data()); 
  TEFile::ChangeDir(CD);
  return !res; */
  //return !rmdir(DN.Data()); 
  return true;
}
//..............................................................................
bool TOSFS::AdoptFile(TFSItem *Src)
{  
  TEString FFN = Src->GetFullName();
  TEString SFN;
  TEString DFN = GetBase() + FFN;
  try
  {  
    SFN = Src->GetFS()->File(Src);
  }
  catch(...)
  {  
    return false;  
  }
  if( SFN.Length() )
  {
    if( CopyFile(SFN.Data(), DFN.Data(), FALSE) )
    {
      int fileHandle = open( DFN.Data(), O_RDWR);
      if( fileHandle != -1 )
      {
        struct ftime fileTime;
        short Year, Mon, Day, Hour, Min, Sec;
        TETime::DecodeDateTimeSec( Src->GetDateTime(), Year, Mon, Day, Hour, Min, Sec );
        fileTime.ft_tsec = Sec/2;
        fileTime.ft_min = Min;
        fileTime.ft_hour = Hour;
        fileTime.ft_day = Day;
        fileTime.ft_month = Mon;
        fileTime.ft_year = Year - 1980;
        setftime( fileHandle, &fileTime );
        close( fileHandle );
      }
    }
    return true;
  }
  return false;
}
//..............................................................................
bool TOSFS::ToLocalFile(TFSItem *Source, const TEString &Dest)
{
  return CopyFile((Source->GetFS()->GetBase()+Source->GetFullName()).Data(), Dest.Data(), TRUE);
}
//..............................................................................
bool TOSFS::NewDir(const TEString& DN)
{  return TEFile::MakeDir(DN);  }
//..............................................................................
bool TOSFS::FileExists(const TEString& FN)
{  return TEFile::FileExists(FN); }
//..............................................................................
TEString TOSFS::File(TFSItem *Src, const TEString &Name)
{  
  if( Src )  {  return GetBase()+Src->GetFullName();  }
  else return Name;
}
//..............................................................................
bool TOSFS::ChangeDir(const TEString &DN)
{  return TEFile::ChangeDir(DN);  }

#ifdef __FTPFS__
//..............................................................................
//..............................................................................
//..............................................................................
TFTPFS::TFTPFS(const TEString& Address, const TEString& User, const TEString& Pass)
{
  FTP = new TNMFTP(NULL);
  FTP->UserID = User.Data();
  FTP->Password = Pass.Data();
  FTP->Host = Address.Data();
  FTP->Port = 21;
  FTP->OnFailure = OnFailture;
  FTP->OnSuccess = OnSuccess;
  FTP->OnTransactionStop = OnTransactionStop;
  FURL = Address;
}
//..............................................................................
TFTPFS::~TFTPFS()
{
  int i;
  for( i=0; i < FTmpFiles.Count(); i++ )
  {  TEFile::DelFile(FTmpFiles.String(i));  }
  delete FTP;
}
//..............................................................................
void _fastcall TFTPFS::OnTransactionStop(TObject *Sender)
{  return; } //FStopped = true;  }
//..............................................................................
void _fastcall TFTPFS::OnFailture(bool &handled, TCmdType Trans_Type)
{  handled = true;  FError = true;  return;  }
//..............................................................................
void _fastcall TFTPFS::OnSuccess(TCmdType Trans_Type)
{  
  FError = false;  
//  if( (Trans_Type == cmdDownload) || (Trans_Type == cmdUpload) )
//  {  FStopped = true; }
  return;  
}
//..............................................................................
bool TFTPFS::Connect()
{  
  FTP->Connect();
  FTP->Mode(MODE_BYTE);
  if( !FTP->Connected )
  {  
    TBasicApp::GetLog()->Error(TEString("Could not connect to: ")+=FURL );
    return false;
  }
  return true;
}
//..............................................................................
bool TFTPFS::DelFile(const TEString& FN)
{  
  FTP->Delete(TEFile::UnixPath(FN).Data());  
  TBasicApp::GetLog()->Error(TEString("Could not delete file: ")+=FN );
  return !FError; 
}
//..............................................................................
bool TFTPFS::DelDir(const TEString& DN)
{  
  FTP->RemoveDir(TEFile::UnixPath(DN).Data()); 
  return !FError; 
}
//..............................................................................
bool TFTPFS::AdoptFile(TFSItem *Src)
{  
  if( !FTP->Connected )  return false;
  TEString FFN = Src->FullName();
  TEString SFN = Src->FS()->Base() + FFN;
  TEString DFN = Src->Name();
  SFN = Src->FS()->File(Src);
  if( SFN.Length() )
  {  FTP->Upload(SFN.Data(), TEFile::UnixPath(DFN).Data());   }
  else
  {  FError = true;  }
  if( FError )
  {  TBasicApp::GetLog()->Error(((TEString("Could not upload file: ")+=Src)+= " to: ")+=DFN );  }
  else
  {  TBasicApp::GetLog()->Info( (FFN+= " uploaded to: ")+=DFN );  }
  return !FError;  
}
//..............................................................................
bool TFTPFS::ToLocalFile(TFSItem *Source, const TEString &Dest)
{
  if( !FTP->Connected )  return "";
  char Bf[512];
  TEString FN = Base()+Source->FullName();
  FTP->Download(FN.Data(), Dest.Data());
  if( FError )
  {
    TBasicApp::GetLog()->Error(TEString("Could not create retrive: ")+=FN );
    return false;
  }
  return true;
}
//..............................................................................
bool TFTPFS::NewDir(const TEString& DN)
{  
  FTP->MakeDirectory(TEFile::UnixPath(DN).Data());  
  if( FError )
  {
    TBasicApp::GetLog()->Error(TEString("Could not create dir: ")+=DN );
  }
  return !FError; 
}
//..............................................................................
bool TFTPFS::FileExists(const TEString& FN)
{  
  return true; 
} 
//..............................................................................
TEString TFTPFS::File(TFSItem *Src, const TEString &Name)
{
  if( !FTP->Connected )  return "";
  char Bf[512];
  GetTempPath(512, Bf);
  TEString Tmp, FN;
  if( Src )  {  FN = Base()+Src->FullName(); }
  else  FN = Name;
  if( !FN.Length() )  return "";
  
  Tmp = FTP->CurrentDir.c_str();
  Tmp = Bf;
  GetTempFileName(Tmp.Data(), "olx", 0, Bf);
  FTP->Download(FN.Data(), Bf);
  if( FError )
  {
    TBasicApp::GetLog()->Error(TEString("Could not retrive: ")+=FN );
    return "";
  }
  Tmp = Bf;
  FTmpFiles.AddString(Tmp);
  return Tmp;
}
//..............................................................................
bool TFTPFS::ChangeDir(const TEString &DN)
{  
  FTP->ChangeDir(DN.Data());  
  if( FError )
  {
    TBasicApp::GetLog()->Error(TEString("Could not change dir to: ")+=DN );
  }
  return !FError; 
}
#endif
//..............................................................................
//..............................................................................
//..............................................................................
#ifdef __HTTPFS__
THTTPFS::THTTPFS()
{
  WSADATA  WsaData;
  WSAStartup(0x0001, &WsaData);
  FConnected = false;
}
//..............................................................................
THTTPFS::~THTTPFS()
{
  Disconnect();
  WSACleanup();
  int i;
  for( i=0; i < FTmpFiles.Count(); i++ )
  {  TEFile::DelFile(FTmpFiles.String(i));  }
}
//..............................................................................
int THTTPFS::GetAddress(struct sockaddr* Result)
{
  struct hostent* Host;
  SOCKADDR_IN     Address;

  memset(Result, 0, sizeof(*Result));
  memset(&Address, 0, sizeof(Address));

  TEString HostAdd = Url.GetHost();

  Host = gethostbyname( HostAdd.Data() );
  if(Host != NULL)
  {
    Address.sin_family  = AF_INET;
    Address.sin_port    = htons( (unsigned short)Url.GetPort() );
    memcpy(&Address.sin_addr, Host->h_addr_list[0], Host->h_length);
    memcpy(Result, &Address, sizeof(Address));
   }
   return Host != NULL;
}
//..............................................................................
void THTTPFS::Disconnect()
{
  if( FConnected )
  {
    FConnected = false;
    closesocket(FSocket);
  }
}
//..............................................................................
bool THTTPFS::Connect()
{
  if( FConnected ) Disconnect();
  struct sockaddr  SockAddr;
  FConnected = false;
  if( GetAddress(&SockAddr))
  {
    int  Status;
    FSocket = socket(AF_INET, SOCK_STREAM, 0);
    Status = connect(FSocket, &SockAddr, sizeof(SockAddr));
    if(Status >= 0)
    {  FConnected = true;  }
    else
    {  TBasicApp::GetLog()->Error(TEString("Connection failed: ") += WSAGetLastError()); }
  }
  else
  {  TBasicApp::GetLog()->Error(TEString("Can't map hostname ") += Url.GetFullAddress() ); }
  return FConnected;
}
//..............................................................................
bool THTTPFS::FileExists(const TEString& DN)
{
  return true;
}
//..............................................................................
bool THTTPFS::ToLocalFile(TFSItem *Source, const TEString &Dest)
{
  if( FConnected )  { Disconnect();  Connect();  }
  if( !FConnected )  return false;

  TEFile File, File1;

  char  Request[512], *Buffer;
  bool Res = false;
  GetTempPath(512, Request);
  TEString Tmp, FileName=TEFile::UnixPath( GetBase()+Source->GetFullName() );
  Tmp = Request;
  GetTempFileName(Tmp.Data(), "http", 0, Request);
  File.Open(Request, "w+b");

  File1.Open(Dest, "w+b");

  int i, parts,
    BufferSize = 1024*64, 
    TotalRead = 0,
    ThisRead = 1,
    FileLength = -1;
  try{  Buffer = new char[BufferSize];  }
  catch(...){  return ""; }
  char LengthId[] = "Content-Length:";

  Tmp = Url.GetPath() + FileName;
  // check if proxy is used, if not - have to add the leading slash
  if( Tmp.IndexOf("://") == -1 )
    if( !Tmp.StartsFrom('/') )  Tmp.Insert('/', 0);

  Tmp.Replace(" ", "%20");
  sprintf(Request, "GET \"%s\" HTTP/1.0\n\n", Tmp.Data());

  send(FSocket, Request, strlen(Request), 0);
  while( ThisRead )
  {
    ThisRead = recv(FSocket, Buffer, BufferSize, 0);
    if( ThisRead == SOCKET_ERROR || ThisRead == 0) 
    {  break;  }
    else
    {
      File.Write(Buffer, ThisRead);
      if( !TotalRead )
      {
        Tmp = "";
        int off = TEString::SearchArrayIC(Buffer, ThisRead, LengthId); 
        if( off != -1 )
        {
          off += strlen(LengthId)+1;
          while( (off < ThisRead) && Buffer[off] == ' ' ) {  off++; }  // skipp spaces
          while( (off < ThisRead) && Buffer[off] >= '0' && Buffer[off] <= '9') 
          {  
            Tmp += Buffer[off];
            off++; 
          }  
          if( Tmp.Length() )
          {  FileLength = Tmp.Int();  }
        }
        else
        {;}
      }
      TotalRead += ThisRead;
    }
  }
  if( (FileLength != -1) && (FileLength <= TotalRead) )
  {
    if( Source->GetSize() == FileLength )
    {
      File.Flush();                   
      File.Seek(TotalRead-FileLength, SEEK_SET);
      parts = FileLength/BufferSize;
      for( i=0; i < parts; i++ )
      {
        File.Read(Buffer, BufferSize);
        File1.Write(Buffer, BufferSize);
      }
      parts = FileLength%BufferSize;
      File.Read(Buffer, parts);
      File1.Write(Buffer, parts);
      if( Source )
      {
        int fileHandle = _fileno( File1.Handle() );
        if( fileHandle != -1 )
        {
          struct ftime fileTime;
          short Year, Mon, Day, Hour, Min, Sec;
          TETime::DecodeDateTimeSec( Source->GetDateTime(), Year, Mon, Day, Hour, Min, Sec );
          fileTime.ft_tsec = Sec/2;
          fileTime.ft_min = Min;
          fileTime.ft_hour = Hour;
          fileTime.ft_day = Day;
          fileTime.ft_month = Mon;
          fileTime.ft_year = Year - 1980;
          setftime( fileHandle, &fileTime );
        }
      }
      Res = true;
    }
  }
  File.Delete();
  delete [] Buffer;
  return Res;
}
//..............................................................................
TEString THTTPFS::File(TFSItem *Src, const TEString &Name)
{
  if( FConnected )  { Disconnect();  Connect();  }
  if( !FConnected )  return "";

  TEFile File, File1;

  char  Request[512], *Buffer;
  GetTempPath(512, Request);
  TEString Tmp, FileName;
  if( Src )  {  FileName = TEFile::UnixPath( GetBase()+Src->GetFullName() ); }
  else  {  FileName = Name; }
  if( !FileName.Length() )  return "";
  
  Tmp = Request;
  GetTempFileName(Tmp.Data(), "http", 0, Request);
  File.Open(Request, "w+b");

  GetTempFileName(Tmp.Data(), "http", 0, Request);
  File1.Open(Request, "w+b");

  int i, parts,
    BufferSize = 1024*64, 
    TotalRead = 0,
    ThisRead = 1,
    FileLength = -1;
  try{  Buffer = new char[BufferSize];  }
  catch(...){  return ""; }
  char LengthId[] = "Content-Length:", EndTagId[]="ETag:";

  bool FileAttached = false;

  Tmp = Url.GetPath() + FileName;
  // check if proxy is used, if not - have to add the leading slash
  if( Tmp.IndexOf("://") == -1 )
    if( !Tmp.StartsFrom('/') )  Tmp.Insert('/', 0);

  Tmp.Replace(" ", "%20");
  sprintf(Request, "GET %s HTTP/1.0\n\n", Tmp.Data());
  send(FSocket, Request, strlen(Request), 0);
  while( ThisRead )
  {
    ThisRead = recv(FSocket, Buffer, BufferSize, 0);

    if( !ThisRead )  break;

    if( ThisRead == SOCKET_ERROR )  break;
    else
    {
      File.Write(Buffer, ThisRead);
      if( !TotalRead )
      {
        Tmp = "";
        int off = TEString::SearchArrayIC(Buffer, ThisRead, EndTagId);
        if( off != -1 )  FileAttached = true;
        off = TEString::SearchArrayIC(Buffer, ThisRead, LengthId);
        if( off != -1 )
        {
          off += strlen(LengthId)+1;
          while( (off < ThisRead) && Buffer[off] == ' ' ) {  off++; }  // skipp spaces
          while( (off < ThisRead) && Buffer[off] >= '0' && Buffer[off] <= '9') 
          {  
            Tmp += Buffer[off];
            off++; 
          }  
          if( Tmp.Length() )
          {  FileLength = Tmp.Int();  }
        }
        else
        {;}
      }
      TotalRead += ThisRead;
    }
  }
  Tmp = "";
  if( (FileLength != -1) && (FileLength <= TotalRead) )
  {
//    if( (Src && (Src->Size() == FileLength)) || (!Src) )
    if( FileAttached )
    {
      File.Flush();                   
      File.Seek(TotalRead-FileLength, SEEK_SET);
      parts = FileLength/BufferSize;
      for( i=0; i < parts; i++ )
      {
        File.Read(Buffer, BufferSize);
        File1.Write(Buffer, BufferSize);
      }
      parts = FileLength%BufferSize;
      if( parts )
      {
        File.Read(Buffer, parts);
        File1.Write(Buffer, parts);
      }
      Tmp = File1.Name();
      FTmpFiles.AddString(Tmp);
      if( Src )
      {
        int fileHandle = _fileno( File1.Handle() );
        if( fileHandle != -1 )
        {
          struct ftime fileTime;
          short Year, Mon, Day, Hour, Min, Sec;
          TETime::DecodeDateTimeSec( Src->GetDateTime(), Year, Mon, Day, Hour, Min, Sec );
          fileTime.ft_tsec = Sec/2;
          fileTime.ft_min = Min;
          fileTime.ft_hour = Hour;
          fileTime.ft_day = Day;
          fileTime.ft_month = Mon;
          fileTime.ft_year = Year - 1980;
          setftime( fileHandle, &fileTime );
        }
      }
    }
  }
  else
  {
    File1.Delete();
  }
  File.Delete();
  delete [] Buffer;
  return Tmp;
}
#endif
//..............................................................................
//..............................................................................
//..............................................................................
TFSItem::TFSItem(TFSItem *Parent, TFS *FS)
{
  FParent = Parent;
  FItems = new TEList;
  FFS = FS;
  FFolder = false;
  FProcessed = false;
  FDateTime = FSize = 0;
}
//..............................................................................
TFSItem::~TFSItem()
{
  Clear();
  delete FItems;
}
//..............................................................................
void TFSItem::Clear()
{
  for( int i=0; i < ItemCount(); i++ )  delete Item(i);
  FItems->Clear();
  FName.SetLength(0);
  FDateTime = 0;
  FSize = 0;
}
//..............................................................................
void TFSItem::operator >> (TStrList& S) const  {
  TEString str( TEString::CharStr( '\t', GetLevel() ) );
  S.Add( str + GetName() );
  str += FDateTime;
  str += ',';  str += ' ';
  str += GetSize();
  S.Add( str );
  for( int i=0; i < ItemCount(); i++ )
    *Item(i) >> S;
}
//..............................................................................
int TFSItem::ReadStrings(int& index, TFSItem* caller, TStrList& strings)
{
  TStrList toks;
  while( (index + 2) < strings.Count() )
  {
    int level = strings.String(index).LeadingCharCount( '\t' );
    TFSItem* item = NewItem();
    item->SetName( strings.String(index).Trim('\t') );
    index++;
    TEString::Strtok( strings.String(index), ',', toks );
    if( toks.Count() < 2 )
      throw TInvalidArgumentException(__OlxSourceInfo, "wrong number of tokens");
    item->SetDateTime( toks.String(0).Long() );
    item->SetSize( toks.String(1).Long() );
    toks.Clear();
    index++;
    if( index < strings.Count() )
    {
      int nextlevel = strings.String(index).LeadingCharCount('\t');
      if( nextlevel > level )
      {
        item->Folder( true );
        int slevel = item->ReadStrings(index, this, strings);
        if( slevel != level )
          return slevel;
      }
      if( nextlevel < level )  return  nextlevel;
    }
  }
  return 0;
}
//..............................................................................
TFSItem* TFSItem::NewItem()
{
  TFSItem *I = new TFSItem(this, GetFS());
  FItems->Add(I);
  return I;
}
//..............................................................................
int TFSItem::_SortItemsByName(const void *I, const void *I1)
{
  return ((TFSItem*)I)->GetName().Compare( ((TFSItem*)I1)->GetName() );
}
//..............................................................................
void TFSItem::SortItemsByName()
{
  int i;
  FItems->Sort(_SortItemsByName);
  for( i=0; i < ItemCount(); i++ )
  {  Item(i)->SortItemsByName();  }
}
//..............................................................................
TFSItem* TFSItem::FindByName(const TEString &Name, int from, int to)
{
/*
  if( from == -1 ) from = 0;
  if( to == -1 )  to = ItemCount()-1;
  if( to < 0 )  return NULL;
  if( to == from )
  {  
    if( !Item(from)->Name().Compare(Name) )  return Item(from);
    return NULL;
  }
  if( !Item(from)->Name().Compare(Name) )  return Item(from);
  if( !Item(to)->Name().Compare(Name) )  return Item(to);
  if( (to-from) == 1 )  return NULL;
  if( Item(from)->Name().Compare(Name) < 0 && Item(to)->Name().Compare(Name) > 0 )
  {
    int index = (to+from)/2;
    int res = Item(index)->Name().Compare(Name);
    if( res < 0 )  {  return FindByName(Name, index, to);  }
    if( res > 0 )  {  return FindByName(Name, from, index);  }
    if( res == 0 )  {  return Item(index);  }
  }
  return NULL;
  */
  for( int i=0; i < ItemCount(); i++ )
  {
    if( !Item(i)->GetName().Compare(Name) )  return Item(i);
  }
  return NULL;
}
//..............................................................................
TEString TFSItem::GetFullName() const
{
// an alernative implementation can be done with TEString::Insert ... to be considered
  TFSItem *FI = const_cast<TFSItem*>(this);
  TEString Tmp;
  TEList L;
  int i;
  while( FI )
  {
    L.Add(FI);
    FI = FI->Parent();
    if( !FI->Parent() )  break;  //ROOT
  }
  for( i=L.Count()-1; i >= 0; i-- )
  {
    FI = (TFSItem*)L.Item(i);
    Tmp += FI->GetName();
    if( i > 0 )  Tmp += '\\';
  }
  return Tmp;
}
//..............................................................................
int TFSItem::GetLevel()  const
{
  int level = 0;
  TFSItem *FI = const_cast<TFSItem*>(this);
  while( FI && FI->Parent() )  {  FI = FI->Parent();  level++;  }
  return level;
}
//..............................................................................
void TFSItem::DoSetProcessed(bool V)
{
  SetProcessed(V);
  for( int i=0; i < ItemCount(); i++ )
  {  Item(i)->SetProcessed(V);  }
}
//..............................................................................
int TFSItem::TotalItemsCount(int &cnt)
{
  cnt += ItemCount();
  TFSItem *FS;
  for( int i=0; i < ItemCount(); i++ )
  {
    FS = Item(i);
    if( FS->IsFolder() )
    {  FS->TotalItemsCount(cnt);  }
  }
  return cnt;
}
//..............................................................................
long int TFSItem::TotalItemsSize(long int &cnt)
{
  TFSItem *FS;
  for( int i=0; i < ItemCount(); i++ )
  {
    FS = Item(i);
    if( FS->IsFolder() )
      FS->TotalItemsSize(cnt);
    else
      cnt += FS->GetSize();
  }
  return cnt;
}
//..............................................................................
int TFSItem::Synchronize(TFSItem *Dest, bool Count)
{
  static TOnProgress Progress;
  static bool Top=true;
  static long int fc=0;
  if( Top )  {
    Top = false;
    Progress.SetMax( Synchronize(Dest, true) );
    Progress.SetPos( 0.0 );
    TBasicApp::GetInstance()->OnProgress->Enter(NULL, &Progress);
    if( !fc )  return 0;  // nothing to do then ...
    fc = 0;
  }
  TFSItem *FI, *Res;
  SetProcessed(false);
  if( Dest )
  {
    for( int i=0; i < Dest->ItemCount(); i++ )
    {
      FI = Dest->Item(i);
      if( !Count )
      {
        Progress.SetAction( FI->GetFullName() );
        Progress.IncPos( FI->GetSize() );
        TBasicApp::GetInstance()->OnProgress->Execute(NULL, &Progress);
      }
      Res = FindByName( FI->GetName() );
      if( FI->IsFolder() && Res )
      {
        Res->Synchronize(FI, Count);
        Res->SetProcessed(true);
        continue;
      }
      if( !Res  )
      {
        if( !Count)
          FI->DelFile();
        fc += FI->GetSize();
      }
      else
      {
        Res->SetProcessed(true);
        if( Res->GetDateTime() != FI->GetDateTime() )
        {
          if( !Count )  FI->UpdateFile(Res);
          fc += FI->GetSize();
        }
      }
    }
  }
  for( int i=0; i < ItemCount(); i++ )
  {
    FI = Item(i);
    if( Count == 0 )  {
      Progress.SetAction( FI->GetFullName() );
      Progress.IncPos( FI->GetSize() );
      TBasicApp::GetInstance()->OnProgress->Execute(NULL, &Progress);
    }
    if( FI->IsProcessed() )  continue;
    if( FI->IsFolder() )
    {
      if( !Count)
      {
        Res = Dest->UpdateFile(FI);
        if( Res ) FI->Synchronize(Res);
      }
      else
        FI->TotalItemsSize(fc);
    }
    else
    {
      if( !Count)  Dest->UpdateFile(FI);
      fc += FI->GetSize();
    }
  }
  return fc;
}
//..............................................................................
TFSItem* TFSItem::UpdateFile(TFSItem *FN)
{
  if( FN->IsFolder() )
  {
    if( TEFile::FileExists(FN->GetFullName().Data()) || GetFS()->NewDir(FN->GetFullName()) )
    {
      TFSItem *FI = NewItem();
      *FI = *FN;
      return FI;
    }
    return NULL;
  }
  else
  {
    TEString Tmp = FN->GetFullName();
    if( !GetFS()->AdoptFile(FN) )
    {
      TBasicApp::GetLog()->Error(TEString("Could not copy: ") += Tmp);
    }
  }
  return NULL;
}
//..............................................................................
void TFSItem::DelFile()
{
  if( IsFolder() )
  {
    for( int i=0; i < ItemCount(); i++ )
      Item(i)->DelFile();

    GetFS()->DelDir( GetFS()->GetBase() + this->GetFullName() );
  }
  else
    GetFS()->DelFile( GetFS()->GetBase() + this->GetFullName());
}
//..............................................................................
void TFSItem::operator = (const TFSItem& FI)
{
  FName = FI.GetName();
  FSize = FI.GetSize();
  FDateTime = FI.GetDateTime();
}
//..............................................................................
//..............................................................................
//..............................................................................
bool TFSIndex::LoadFromFTP(const TEString &FTP)
{
  return false;
}
//..............................................................................
TFSIndex::TFSIndex()
{
  FRoot = new TFSItem(NULL, NULL);
}
//..............................................................................
TFSIndex::~TFSIndex()
{
  delete FRoot;
}
//..............................................................................
bool TFSIndex::CreateIndex(const TEString &DirName,
  TStrList *Mask,
  TStrList *Exceptions,
  TFSItem *II)
{
  bool Save = false;
  if( !II )
  {
    Save = true;
    FRoot->Clear();
    FRoot->SetName( "ROOT" );
    II = FRoot;
    if( !TEFile::ChangeDir(DirName) )
      throw TFunctionFailedException(__OlxSourceInfo, "cannot set initial folder");
  }
  TEString Tmp = DirName;
  TEFile::AddTrailingBackslashI(Tmp);
  TEStringList Folders;
  if( !TEFile::ChangeDir(Tmp) )
    throw TFunctionFailedException(__OlxSourceInfo, "cannot change folder");
  struct ffblk ffblk;
  TFSItem *FI;
  int done, i;
  if( Mask )
  {
    for( i=0; i < Mask->Count(); i++ )
    {
      done = findfirst(Mask->String(i).Data(),&ffblk, 0);
      while (!done)
      {
        if( ffblk.ff_attrib  & FA_DIREC )
        {  done = findnext(&ffblk);  continue;  }
        if( Exceptions )
        {
          if( Exceptions->IndexOfString(ffblk.ff_name) != -1 )
          {  done = findnext(&ffblk);  continue;  }
        }
        FI = II->NewItem();
        FI->SetName( ffblk.ff_name );
        FI->SetDateTime( ffblk.ff_fdate, ffblk.ff_ftime );
        FI->SetSize( ffblk.ff_fsize );
        done = findnext(&ffblk);
      }
      findclose( &ffblk );
    }
  }
  else
  {
    done = findfirst("*.*",&ffblk, 0);
    while (!done)
    {
      if( ffblk.ff_attrib  & FA_DIREC )
      {  done = findnext(&ffblk);  continue;  }
      if( Exceptions )
      {
        if( Exceptions->IndexOfString(ffblk.ff_name) != -1 )
        {  done = findnext(&ffblk);  continue;  }
      }
      FI = II->NewItem();
      FI->SetName( ffblk.ff_name );
      FI->SetDateTime( ffblk.ff_fdate, ffblk.ff_ftime );
      FI->SetSize( ffblk.ff_fsize );
      done = findnext(&ffblk);
    }
    findclose( &ffblk );
  }

  done = findfirst("*.*",&ffblk, FA_DIREC);
  while (!done)
  {
    if( ffblk.ff_attrib  & FA_DIREC )
    {
      if( !strcmp(ffblk.ff_name, "." ) || !strcmp(ffblk.ff_name, ".." ) )
      {  done = findnext(&ffblk);  continue;  }
      if( (ffblk.ff_attrib & FA_HIDDEN) )
      {  done = findnext(&ffblk);  continue;  }
      if( Exceptions )
      {
        if( Exceptions->IndexOfString(ffblk.ff_name) != -1 )
        {  done = findnext(&ffblk);  continue;  }
      }
      FI = II->NewItem();
      FI->SetName( ffblk.ff_name );
      FI->SetDateTime( ffblk.ff_fdate, ffblk.ff_ftime );
      FI->SetSize( ffblk.ff_fsize );
      FI->Folder(true);
      Folders.AddObject(ffblk.ff_name, FI);
    }
    done = findnext(&ffblk);
  }
  findclose( &ffblk );

  for( i=0; i < Folders.Count(); i++ )
  {
    FI = (TFSItem*)Folders.Object(i);
    CreateIndex(Folders.String(i), Mask, Exceptions, FI);
  }
  if( Save )
  {
    TEStringList SL;
    FRoot->SortItemsByName();
    *FRoot >> SL;
    SL.SaveToFile(SL, DirName + "index.ind");
  }
  TEFile::ChangeDir("..");
  return true;
}
//..............................................................................
void TFSIndex::LoadIndex(TEString &IndexFile)
{
  if( !IndexFile.Length() )  IndexFile = GetRoot()->GetFS()->GetBase() + "index.ind";

  FRoot->Clear();
  if( !GetRoot()->GetFS()->FileExists(IndexFile) )
    throw TFileDoesNotExistException(__OlxSourceInfo, IndexFile);

  TEString FN = GetRoot()->GetFS()->File(NULL, IndexFile);
  if( !FN.Length() )
    throw TFunctionFailedException(__OlxSourceInfo, "could not retrieve the index file");
  TEFile f( FN, "rb" );
  TEStringList strings;
  strings.LoadFromStream( f );
  int index = 0;
  FRoot->ReadStrings(index, NULL, strings);
  IndexFile = FN;
}
//..............................................................................
void TFSIndex::SaveIndex(const TEString &IndexFile)
{
  TEStringList strings;
  *GetRoot() >> strings;
  strings.SaveToFile(strings, IndexFile );
}
//..............................................................................
int TFSIndex::Local2Local(TOSFS &SrcFS, TOSFS &DestFS)
{
  TFSIndex DestI;
  TEString SrcInd;
  TEString DestInd;

  GetRoot()->SetFS(&SrcFS);
  DestI.GetRoot()->SetFS(&DestFS);

  LoadIndex(SrcInd);
  DestI.LoadIndex(DestInd);
  FFilesUpdated = this->GetRoot()->Synchronize( DestI.GetRoot() );

  if( !DestInd.Length() )
  {  DestInd = DestI.GetRoot()->GetFS()->GetBase() + "index.xind";  }
  CopyFile(SrcInd.Data(), DestInd.Data(), FALSE);

  return FFilesUpdated;
}
#ifdef __FTPFS__
//..............................................................................
bool TFSIndex::FTP2Local(TFTPFS &SrcFS, TOSFS &DestFS)
{
  TFSIndex DestI;
  TEString SrcInd;
  TEString DestInd;

  this->Root()->FS(&SrcFS);
  DestI.Root()->FS(&DestFS);

  if( !LoadIndex(SrcInd) )  return false;
  if( !DestI.LoadIndex(DestInd) )  return false;
  
  FFilesUpdated = this->Root()->Synchronize(DestI.Root());  
  TBasicApp::GetLog()->Info(TEString("Files updated: ")+= FFilesUpdated);

  if( !DestInd.Length() )
  {  DestInd = DestI.Root()->FS()->Base() + "index.xind";  }
  CopyFile(SrcInd.Data(), DestInd.Data(), FALSE);
  return true;
}
//..............................................................................
bool TFSIndex::Local2FTP(TOSFS &SrcFS, TFTPFS &DestFS)
{
  TFSIndex DestI;
  TEString SrcInd;
  TEString DestInd;

  this->Root()->FS(&SrcFS);
  DestI.Root()->FS(&DestFS);

  if( !LoadIndex(SrcInd) )  return false;
  if( !DestI.LoadIndex(DestInd) )
  { ; }// return false;
  
  FFilesUpdated = this->Root()->Synchronize(DestI.Root());  
  TBasicApp::GetLog()->Info(TEString("Files updated: ")+= FFilesUpdated);

  if( !DestInd.Length() )
  {  DestInd = DestI.Root()->FS()->Base() + "index.xind";  }
  TFSItem *Index = Root()->FindByName("index.xind");
  if( Index )
  { 
    if( DestFS.AdoptFile(Index) )  return true;
    return false;
  }
  return false;
}
//..............................................................................
#endif
//..............................................................................
#ifdef __HTTPFS__
int TFSIndex::HTTP2Local(THTTPFS &SrcFS, TOSFS &DestFS)
{
  TFSIndex DestI;
  TEString SrcInd;
  TEString DestInd;

  GetRoot()->SetFS( &SrcFS );
  DestI.GetRoot()->SetFS( &DestFS );

  LoadIndex(SrcInd);
  try  {
    DestI.LoadIndex(DestInd);  }
  catch( ... ) {  ;  }

  FFilesUpdated = GetRoot()->Synchronize( DestI.GetRoot() );

  chmod(DestInd.Data(), S_IWRITE);
  esdl::TEFile::Copy(SrcInd, DestInd);
  return FFilesUpdated;
}
//..............................................................................
#endif
//..............................................................................


