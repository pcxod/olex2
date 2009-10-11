//---------------------------------------------------------------------------

#ifndef fsindexH
#define fsindexH
#include "estrlist.h"

#include "url.h"
#include "etime.h"

#ifdef __FTPFS__
  #include <NMFtp.hpp>
  #include <Psock.hpp>
#endif
#ifdef __HTTPFS__
  #include <winsock.h>
  #include <windows.h>
//  #pragma link "../..lib/psdk/mswsock.lib"
#endif

//---------------------------------------------------------------------------
class TFSIndex;
class TFSItem;

class TFS
{
  TEString FBase;
public:
  TFS()           {  ; }
  virtual ~TFS()  {  ; }
  virtual bool DelFile(const TEString& FN)=0;
  virtual bool DelDir(const TEString& DN)=0;
  virtual bool AdoptFile(TFSItem *Source)=0;
  virtual bool ToLocalFile(TFSItem *Source, const TEString &Dest)=0;
  virtual bool NewDir(const TEString& DN)=0;
  virtual bool FileExists(const TEString& DN)=0;
  virtual TEString File(TFSItem *Source, const TEString &Name="")=0;
  virtual bool ChangeDir(const TEString& DN)=0;

  const TEString& GetBase() const  {  return FBase; }
  void SetBase(const TEString& b)  {  FBase = b; }
};

class TOSFS: public TFS
{
public:
  TOSFS()           {  ; }
  virtual ~TOSFS()  {  ; }
  virtual bool DelFile(const TEString& FN);
  virtual bool DelDir(const TEString& DN);
  virtual bool AdoptFile(TFSItem *Source);
  virtual bool ToLocalFile(TFSItem *Source, const TEString &Dest);
  virtual bool NewDir(const TEString& DN);
  virtual bool FileExists(const TEString& DN);
  virtual TEString File(TFSItem *Source, const TEString& Name="");
  virtual bool ChangeDir(const TEString& DN);
};
#ifdef __FTPFS__
class TFTPFS: public TFS
{
  TNMFTP *FTP;
  TEString FURL;
  TEStringList FTmpFiles;
  bool FError;
protected:
  void _fastcall OnFailture(bool &handled, TCmdType Trans_Type);
  void _fastcall OnSuccess(TCmdType Trans_Type);
  void _fastcall OnTransactionStop(TObject *Sender);
public:
  TFTPFS(const TEString& Address, const TEString& User, const TEString& Pass);
  virtual ~TFTPFS();
  bool Connect();
  virtual bool DelFile(const TEString& FN);
  virtual bool DelDir(const TEString& DN);
  virtual bool AdoptFile(TFSItem *Source);
  virtual bool ToLocalFile(TFSItem *Source, const TEString &Dest);
  virtual bool NewDir(const TEString& DN);
  virtual bool FileExists(const TEString& DN);
  virtual TEString File(TFSItem *Source, const TEString &Name="");
  virtual bool ChangeDir(const TEString& DN);
};
#endif

#ifdef __HTTPFS__
class THTTPFS: public TFS
{
  TEStringList FTmpFiles;
  SOCKET  FSocket;
  bool FConnected;
  TUrl Url;
protected:
  int GetAddress(struct sockaddr* Result);
public:
  THTTPFS();
  virtual ~THTTPFS();
  bool Connect();
  void Disconnect();
  virtual bool DelFile(const TEString& FN) {  return false; };
  virtual bool DelDir(const TEString& DN)  {  return false; };
  virtual bool AdoptFile(TFSItem *Source)  {  return false; };
  virtual bool ToLocalFile(TFSItem *Source, const TEString &Dest);
  virtual bool NewDir(const TEString& DN)  { return false; };
  virtual bool FileExists(const TEString& DN);
  virtual TEString File(TFSItem *Source, const TEString &Name="");
  virtual bool ChangeDir(const TEString& DN){  return false; };

  void SetUrl(const TUrl& v)    {  Url = v; }
};
#endif

class TFSItem: public IEObject
{
  TEString FName;
  //short FTime, FDate;
  long int FSize, FDateTime;
  bool FFolder, FProcessed;
  TFSItem *FParent;
  TEList *FItems;
  TFS *FFS;
protected:
  static int _SortItemsByName(const void *I, const void *I1);
  void DoSetProcessed(bool V);
public:
  TFSItem(TFSItem *Parent, TFS *FS);
  virtual ~TFSItem();
  void Clear();
  TFSItem *Parent() const {  return FParent; }

  void operator >> (TStrList& strings) const;
  int ReadStrings(int& index, TFSItem* caller, TStrList& strings);

  void operator = (const TFSItem &FI);
  inline TFSItem *Item(int i) const {  return (TFSItem*)FItems->Item(i); }
  inline int ItemCount() const      {  return FItems->Count(); }
  TFSItem* NewItem();

  inline bool IsFolder()  const     {  return FFolder; }
  inline void Folder(bool v)        {  FFolder = v; }

  inline const TEString& GetName()  const      {  return FName;  }
  inline void SetName(const TEString& v)       {  FName = v;  }

  int GetLevel()  const;
  TEString GetFullName() const;

  inline void SetDateTime(short date, short time)
  {
    FDateTime = TETime::EncodeDateTimeSec(((date & 0xfe00) >> 9) + 1980, (date & 0x1e0) >> 5, (date & 0x1f),
                                           (time & 0xf800) >> 11, (time & 0x7e0) >> 5, (time & 0x1f)*2 );
  }
  inline void SetDateTime(long dt)  {  FDateTime = dt;  }
  
  inline long GetDateTime() const  {  return FDateTime; }

  inline long GetSize() const      {  return FSize; }
  inline void SetSize(long s)      {  FSize = s; }

  void SortItemsByName();

  TFSItem* FindByName(const TEString &Name, int from=-1, int to=-1);

  TFS *GetFS()            {  return FFS; }
  void SetFS(TFS *FS)     {  FFS = FS; }

  int Synchronize(TFSItem *Dest, bool Count=false);
  TFSItem* UpdateFile(TFSItem *FN);
  void DelFile();

  inline bool IsProcessed()  const {  return FProcessed; }
  inline void SetProcessed(bool v) {  FProcessed = v; }

  int TotalItemsCount(int &cnt);
  long int TotalItemsSize(long int &cnt);

  BeginIEObjectImplementation()
};

class TFSIndex: public IEObject
{
  bool LoadFromFTP(const TEString &FTP);
  TFSItem *FRoot;
protected:
  TEString FSrc,  FDest;
  int FFilesUpdated;
public:
  TFSIndex();
  virtual ~TFSIndex();
  void LoadIndex(TEString &IndexFile);
  void SaveIndex(const TEString &IndexFile);
  bool CreateIndex(const TEString &DirName,
    TStrList *Mask=NULL,
    TStrList *Exceptions=NULL,
    TFSItem *II=NULL);
  // returns the number of updated files
  int Local2Local(TOSFS &LocalFSSrc, TOSFS &LocalFSDest);
  int FilesUpdated(){  return FFilesUpdated; }
#ifdef __FTPFS__
  bool Local2FTP(TOSFS &LocalFS, TFTPFS &RemoteFS);
  bool FTP2Local(TFTPFS &RemoteFS, TOSFS &LocalFS);
#endif
#ifdef __HTTPFS__
  int HTTP2Local(THTTPFS &RemoteFS, TOSFS &LocalFS);
#endif
  TFSItem *GetRoot()  {  return FRoot; }

  BeginIEObjectImplementation()
};
//---------------------------------------------------------------------------
#endif
