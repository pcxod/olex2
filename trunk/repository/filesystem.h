#ifndef olx_filesysteH
#define olx_filesysteH

#include "estrlist.h"
#include "actions.h"

#include "url.h"
#include "etime.h"
#include "estlist.h"
#include "etraverse.h"
#include "efile.h"
#undef GetObject

class TFSIndex;
class TFSItem;

class AFileSystem  {
  olxstr FBase;
  TActionQList Actions;
public:
  AFileSystem() {  
    OnProgress = &Actions.NewQueue("ON_PROGRESS");
  }
  virtual ~AFileSystem()  {  ; }

  // called on progress
  TActionQueue* OnProgress;

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
  inline void SetBase(const olxstr& b)  {  FBase = TEFile::AddTrailingBackslash(b); }

  virtual bool AdoptStream(IInputStream& file, const olxstr& name) = 0;
};
//.............................................................................//
//.............................................................................//
//.............................................................................//
class TOSFileSystem: public AFileSystem, public IEObject {
  TActionQList Events;
  // this is used to pass file.dir names to the events ....
  static olxstr F__N;
public:
  TOSFileSystem(const olxstr& base);
  virtual ~TOSFileSystem()  {  ; }
  virtual bool DelFile(const olxstr& FN);
  virtual bool DelDir(const olxstr& DN);
  virtual bool AdoptFile(const TFSItem& Source);
  virtual bool AdoptStream(IInputStream& file, const olxstr& name);
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
    SkipOptions() : extsToSkip(NULL), filesToSkip(NULL) {  }
  };
private:
  olxstr Name;
  uint64_t Size, DateTime;
  bool Folder, Processed;
  TFSItem* Parent;
  TCSTypeList<olxstr, TFSItem*> Items;
  TStrList Properties, Actions;
  TFSIndex& Index;
protected:
  void DoSetProcessed(bool V);
  void DeleteItem(TFSItem* item);
public:
  TFSItem(TFSIndex& index, TFSItem* parent, const olxstr& name) :
    Index(index), 
    Parent(parent), 
    Folder(false), 
    Processed(false) ,
    DateTime(0), 
    Size(0), 
    Name(name) {  }
  virtual ~TFSItem()  {  Clear();  }
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
  inline bool IsEmpty()       const {  return Items.IsEmpty(); }
  TFSItem& NewItem(const olxstr& name);
  // recreates specified item in current context
  TFSItem& NewItem(TFSItem* item);
  // removes the item and deletes the file/folder
  static void Remove(TFSItem& item);

  inline int PropertyCount()  const  {  return Properties.Count();  }
  inline const olxstr& GetProperty(int ind)  const  {  return Properties[ind];  }
  inline void AddProperty(const olxstr& p)  {  Properties.Add(p);  }
  inline bool HasProperty( const olxstr& pn )  const {
    return Properties.IndexOf(pn) != -1;
  }
  inline bool ValidateProperties( const TStrList& prs )  const {
    if( Properties.IsEmpty() || prs.IsEmpty() )  return true;
    for( int i=0; i < prs.Count(); i++ )
      if( Properties.IndexOf(prs[i]) != -1 )
        return true;
    return false;
  }
  void ListUniqueProperties(TStrList& Properties);

  const TStrList& GetActions() const {  return Actions;  }

  DefPropBIsSet(Folder)

  inline const olxstr& GetName()  const      {  return Name;  }

  int GetLevel()  const;
  olxstr GetFullName() const;

  DefPropP(uint64_t, DateTime)
  DefPropP(uint64_t, Size)

  template <class SC> 
  TFSItem* FindByName(const SC& Name) const {
    int ind = Items.IndexOfComparable(Name);
    return (ind == -1) ? NULL : Items.GetObject(ind);
  }
	// does a search of /parent_folder/parent_folder/file_name
  TFSItem* FindByFullName(const olxstr& Name) const;

  AFileSystem& GetIndexFS() const;
  AFileSystem& GetDestFS() const;
  // calculates the update size
  uint64_t CalcDiffSize(TFSItem& Dest, const TStrList& properties);
  // caller must be NULL, when invoked externally
  double Synchronise(TFSItem& Dest, const TStrList& properties, TStrList* cmds=NULL, 
    TFSItem* Caller=NULL);
  TFSItem* UpdateFile(TFSItem& FN);
  /* deletes underlying physical object (file or folder). If the object is a folder
  the content of that folder will be removed completely */
  void DelFile();

  DefPropBIsSet(Processed)

  uint64_t CalcTotalItemsSize(const TStrList& props) const;

  static TGraphTraverser<TFSItem> Traverser;
};
//.............................................................................//
//.............................................................................//
//.............................................................................//
class TFSIndex: public IEObject  {
private:
  TFSItem *Root;
  bool IndexLoaded;
protected:
  olxstr Source,  Destination;
  TStrList Properties;
  TActionQList Actions;
  AFileSystem *DestFS;
  AFileSystem& IndexFS;
  TOnProgress Progress;
public:
  TFSIndex(AFileSystem& fs);
  virtual ~TFSIndex();
  
  // this is to be used for the oevral progress monitorring
  TActionQueue* OnProgress;

  void LoadIndex(const olxstr& IndexFile, const TFSItem::SkipOptions* toSkip=NULL);
  void SaveIndex(const olxstr& IndexFile);
  /* returns the number transfered bytes.  If the dest_fs is not NULL, the difference is adopted by that
  file syste. If cmds is not NULL, the rm commands are stored in it */
  double Synchronise(AFileSystem& To, const TStrList& properties, const TFSItem::SkipOptions* toSkip=NULL, 
    AFileSystem* dest_fs=NULL, TStrList* cmds=NULL, const olxstr& indexName="index.ind");
  uint64_t CalcDiffSize(AFileSystem& To, const TStrList& properties, const TFSItem::SkipOptions* toSkip=NULL,
    const olxstr& indexName="index.ind");
  // returns true if the file is updated (added) and false otherwise
  bool UpdateFile(AFileSystem& To, const olxstr& fileName, bool Force, const olxstr& indexName="index.ind");
  inline TFSItem& GetRoot()  const {  return *Root; }
  /* checks if the file actions specify to delete it, if a delete action is found return false
  if the timestamps of the items and size match and false in other cases */
  bool ShallAdopt(const TFSItem& src, const TFSItem& dest) const;
  bool ShouldExist(const TFSItem& src)  const {  return src.GetActions().IndexOfi("delete") == -1;  }
  void ProcessActions(TFSItem& item); 
  friend class TFSItem;
};
#endif
