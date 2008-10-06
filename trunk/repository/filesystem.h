#ifndef olx_filesysteH
#define olx_filesysteH

#include "estrlist.h"
#include "actions.h"

#include "url.h"
#include "etime.h"
#include "estlist.h"
#include "etraverse.h"

class TFSIndex;
class TFSItem;

class AFileSystem  {
  olxstr FBase;
public:
  AFileSystem()           {  ; }
  virtual ~AFileSystem()  {  ; }
  // deletes a file
  virtual bool DelFile(const olxstr& FN)=0;
  // deletes a folder
  virtual bool DelDir(const olxstr& DN)=0;
  // puts a file to the file system
  virtual bool AdoptFile(const TFSItem& Source)=0;
  // creates a new folder
  virtual bool NewDir(const olxstr& DN)=0;
  // checks if the file exists
  virtual bool FileExists(const olxstr& DN)=0;
  // returns a stream for a specified stream, must be deleted
  virtual IInputStream* OpenFile(const olxstr& Source)=0;
  // changes a folder 
  virtual bool ChangeDir(const olxstr& DN)=0;
  // returns a base at which the file system is initalised
  inline const olxstr& GetBase() const  {  return FBase; }
  inline void SetBase(const olxstr& b)  {  FBase = b; }
};
//.............................................................................//
//.............................................................................//
//.............................................................................//
class TOSFileSystem: public AFileSystem, public IEObject {
  TActionQList Events;
  // this is used to pass file.dir names to the events ....
  static olxstr F__N;
public:
  TOSFileSystem();
  virtual ~TOSFileSystem()  {  ; }
  virtual bool DelFile(const olxstr& FN);
  virtual bool DelDir(const olxstr& DN);
  virtual bool AdoptFile(const TFSItem& Source);
  virtual bool NewDir(const olxstr& DN);
  virtual bool FileExists(const olxstr& DN);
  // not that the stream must be deleted
  virtual IInputStream* OpenFile(const olxstr& Source);
  virtual bool ChangeDir(const olxstr& DN);

  TActionQueue* OnRmFile;
  TActionQueue* OnRmDir;
  TActionQueue* OnMkDir;
  TActionQueue* OnChDir;
  TActionQueue* OnAdoptFile;
  TActionQueue* OnOpenFile;
};
//.............................................................................//
//.............................................................................//
//.............................................................................//
class TFSItem: public IEObject  {
public:
  struct SkipOptions  {
    TStrList *extsToSkip,  // extenstions to skip
             *filesToSkip; // file names to skip
  };
private:
  olxstr Name;
  long int Size, DateTime;
  bool Folder, Processed;
  TFSItem* Parent;
  TCSTypeList<olxstr, TFSItem*> Items;
  TCSTypeList<olxstr, void*> Properties;
  AFileSystem *FileSystem;
protected:
  void DoSetProcessed(bool V);
  void DeleteItem(TFSItem* item);
public:
  TFSItem(TFSItem* Parent, AFileSystem* FS, const olxstr& name);
  virtual ~TFSItem();
  void Clear();
  inline TFSItem* GetParent() const {  return Parent; }

  void operator >> (TStrList& strings) const;
  int ReadStrings(int& index, TFSItem* caller, TStrList& strings, const SkipOptions* toSkip=NULL);
  // removes empty folders recursively
  void ClearEmptyFolders();
  // removes nonexiting files recursively
  void ClearNonexisting();

  TFSItem& operator = (const TFSItem& FI);
  inline TFSItem& Item(int i) const {  return *Items.GetObject(i); }
  inline int Count()          const {  return Items.Count(); }
  inline bool IsEmpty()       const {  return (Items.Count() == 0); }
  TFSItem& NewItem(const olxstr& name);
  // recreates specified item in current context
  TFSItem& NewItem(TFSItem* item);
  // removes the item and deletes the file
  void Remove(TFSItem& item);

  inline int PropertyCount()  const  {  return Properties.Count();  }
  inline const olxstr& GetProperty(int ind)  const  {  return Properties.GetComparable(ind);  }
  inline void AddProperty(const olxstr& p)  {  Properties.Add(p, NULL);  }
  inline bool HasProperty( const olxstr& pn )  const {
    return Properties.IndexOfComparable(pn) != -1;
  }
  void ListUniqueProperties(TCSTypeList<olxstr, void*>& Properties);

  inline bool IsFolder()  const     {  return Folder; }
  inline void SetFolder(bool v)     {  Folder = v; }

  inline const olxstr& GetName()  const      {  return Name;  }

  int GetLevel()  const;
  olxstr GetFullName() const;

  inline void SetDateTime(long dt)  {  DateTime = dt;  }

  inline long GetDateTime() const  {  return DateTime; }

  inline long GetSize() const      {  return Size; }
  inline void SetSize(long s)      {  Size = s; }

  TFSItem* FindByName(const olxstr& Name)  const {
    int ind = Items.IndexOfComparable(Name);
    if( ind == -1 )  return NULL;
    return Items.GetObject(ind);
  }
	// does a search of /parent_folder/parent_folder/file_name
  TFSItem* FindByFullName(const olxstr& Name) const;

  inline AFileSystem& GetFileSystem()  const   {  return *FileSystem; }
  inline void SetFileSystem(AFileSystem& FS)   {  FileSystem = &FS; }
  // caller must be NULL, when invoked externally
  int Synchronize(TFSItem* Caller, TFSItem& Dest, const TStrList& properties, bool Count=false);
  TFSItem* UpdateFile(TFSItem& FN);
  void DelFile();

  inline bool IsProcessed()  const {  return Processed; }
  inline void SetProcessed(bool v) {  Processed = v; }

  int TotalItemsCount(int &cnt);
  long int TotalItemsSize(long int &cnt);

  static TGraphTraverser<TFSItem> Traverser;
};
//.............................................................................//
//.............................................................................//
//.............................................................................//
class TFSIndex: public IEObject  {
private:
  TFSItem *Root;
protected:
  olxstr Source,  Destination;
  int FilesUpdated;
  TCSTypeList<olxstr, void*> Properties;
public:
  TFSIndex(AFileSystem& fs);
  virtual ~TFSIndex();
  void LoadIndex(const olxstr& IndexFile, const TFSItem::SkipOptions* toSkip=NULL);
  void SaveIndex(const olxstr& IndexFile);
  // returns the number of updated files
  int Synchronise(AFileSystem& To, const TStrList& properties, const TFSItem::SkipOptions* toSkip=NULL);
  // returns true if the file is updated (added) and false otherwise
  bool UpdateFile(AFileSystem& To, const olxstr& fileName, bool Force);
  inline TFSItem& GetRoot()  const {  return *Root; }
};
#endif
