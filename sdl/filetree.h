#ifndef __olx_file_tree_H
#define __olx_file_tree_H

#include "efile.h"
#include "actions.h"
#include "utf8file.h"
#undef CopyFile
BeginEsdlNamespace()
class TFileTree  {
public:
  typedef TPtrList<const TFileListItem> TFilePtrList;

  template <class FT> struct TDiffFolder  {
    const FT *Src, *Dest;
    // the size is the same but some items might be missing
    TFilePtrList SrcFiles, DestFiles;
    TTypeList< TDiffFolder<FT> > Folders;
    TDiffFolder(const FT* src=NULL, const FT* dest=NULL) : Src(src), Dest(dest)  {  }
  };
///////////////////////////////////////////////////////////////////////////////////////
  class Folder  {
    TFileList Files;
    TFileTree& FileTree;
    TTypeList<Folder> Folders;
    Folder* Parent;
    olxstr FullPath, Name;
    typedef TDiffFolder<Folder> DiffFolder;
#ifdef __WIN32__
    static int CompareFiles(const TFileListItem& i1, const TFileListItem& i2)  {
      return i1.GetName().Comparei(i2.GetName());
    }
    static int CompareFolders(const Folder& i1, const Folder& i2)  {
      return i1.Name.Comparei(i2.Name);
    }
    template <class T> static int FindSortedIndexOf(const T&list, const olxstr& entity) {
      if( list.Count() == 0 )  return -1;
      if( list.Count() == 1 )  return (list[0].GetName().Comparei(entity) == 0) ? 0 : -1;
      int from = 0, to = list.Count()-1;
      if( list[from].GetName().Comparei(entity) == 0  )  return from;
      if( list[from].GetName().Comparei(entity) > 0 )  return -1;
      if( list[to].GetName().Comparei(entity) == 0 )  return to;
      if( list[to].GetName().Comparei(entity) < 0  )  return -1;
      while( true ) {
        int index = (to+from)/2;
        if( index == from || index == to)
          return -1;
        if( list[index].GetName().Comparei(entity) < 0 )  from = index;
        else
          if( list[index].GetName().Comparei(entity) > 0 )  to  = index;
          else
            if( list[index].GetName().Comparei(entity) == 0 )  return index;
      }
      return -1;
    }
#else
    static int CompareFiles(const TFileListItem& i1, const TFileListItem& i2)  {
      return i1.GetName().Compare(i2.GetName());
    }
    static int CompareFolders(const Folder& i1, const Folder& i2)  {
      return i1.Name.Compare(i2.Name);
    }
    template <class T> static int FindSortedIndexOf(const T&list, const olxstr& entity) {
      if( list.Count() == 0 )  return -1;
      if( list.Count() == 1 )  return (list[0].GetName() == entity) ? 0 : -1;
      int from = 0, to = list.Count()-1;
      if( list[from].GetName() == entity )  return from;
      if( list[from].GetName().Compare(entity) > 0 )  return -1;
      if( list[to].GetName() == entity )  return to;
      if( list[to].GetName().Compare(entity) < 0  )  return -1;
      while( true ) {
        int index = (to+from)/2;
        if( index == from || index == to)
          return -1;
        if( list[index].GetName().Compare(entity) < 0 )  from = index;
        else
          if( list[index].GetName().Compare(entity) > 0 )  to  = index;
          else
            if( list[index].GetName() == entity )  return index;
      }
      return -1;
    }
#endif
    void _ExpandNewFolder(DiffFolder& df, bool is_src) const;
  public:
    Folder(TFileTree& fileTree, const olxstr& fullPath, Folder* parent = NULL) :
      Parent(parent), FileTree(fileTree)  
    {
      FullPath = TEFile::OSPath(fullPath);
      TEFile::AddTrailingBackslashI(FullPath);
      int ind = FullPath.LastIndexOf(TEFile::GetPathDelimeter(), FullPath.Length()-1);
      if( ind != -1 )
        Name = FullPath.SubStringFrom(ind+1, 1);
    }
    const olxstr& GetName() const {  return Name;  }
    const olxstr& GetFullPath() const {  return FullPath;  }
    int FileCount() const {  return Files.Count();  }
    const TFileListItem& GetFile(int i) const {  return Files[i];  }
    void NullFileEntry(int i) const {  Files.NullItem(i);  }
    bool IsEmpty() const {  return Files.IsEmpty() && Folders.IsEmpty();  }
    // calculates total size of the tree
    uint64_t CalcSize() const;
    // returns total number of files in folders
    size_t CalcItemCount() const;
    // reads the folder structure
    void Expand(TOnProgress& pg);
    // recursive deletion of the folder, must be expanded beforehand! throws TFunctionFailedException
    void Delete(TOnProgress& pg, bool ContentOnly=false);
    // compares folders and returns the difference size
    uint64_t Compare(const Folder& f, TOnProgress& pg) const;
    // calculates the difference tree between the folders
    void CalcMergedTree(const Folder& f, DiffFolder& df) const;
    // merges the difference tree, possibly into a different location
    void Merge(const DiffFolder& df, TOnProgress& onSync, const olxstr& _dest_n = EmptyString) const;
    // copies and overwrites existing files and timestamp
    void CopyTo(const olxstr& _dest,
                TOnProgress& OnSync,
                void (*AfterCopy)(const olxstr& src, const olxstr& dest) = NULL
                ) const;
    // syncronises two folders - only updates files, does not delete anything (yet)
    void Synchronise(Folder& f, TOnProgress& onSync);
    // exports the folder index to a file
    void ExportIndex(const olxstr& fileName, TStrList* list = NULL, int level=0) const;
    // the function fills the list with full file names (recursively)
    void ListFilesEx(TStrList& out, const TTypeList<TEFile::TFileNameMask>* _mask=NULL) const;
    void ListFiles(TStrList& out, const olxstr& _mask) const;
  };
///////////////////////////////////////////////////////////////////////////////////////
public:
  typedef TFileTree::TDiffFolder<TFileTree::Folder> DiffFolder;
protected:
  TActionQList Actions;
  Folder Root;
  volatile bool Break;
public:

  TActionQueue* OnExpand, // called when the OS folder structure is traversed
    *OnSynchronise, // called when fodlers are merged
    *OnFileCopy,   // called in file copy process
    *OnFileCompare, // called in the comparison process 
    *OnCompare, // called when files are being compared
    *OnDelete;  // called when recursive folder deletion is executed

  TFileTree(const olxstr& root) : Root(*this, root)  {
    OnExpand = &Actions.NewQueue("ON_EXPAND");
    OnSynchronise = &Actions.NewQueue("ON_SYNC");
    OnFileCopy = &Actions.NewQueue("ON_FCOPY");
    OnFileCompare = &Actions.NewQueue("ON_FCCOMPARE");
    OnCompare = &Actions.NewQueue("ON_COMPARE");
    OnDelete = &Actions.NewQueue("ON_DELETE");
    Break = false;
  }
  
  void DoBreak() {  Break = true;  }
  // copies files (platform dependent, uses win API to handle huge files)
  bool CopyFile(const olxstr& from, const olxstr& to) const;
  // compares two files (platform independent, may fail on huge files)
  bool CompareFiles(const olxstr& from, const olxstr& to) const;
  //............................................................................
  // calculates the number of bytes to be transfered when merging
  static uint64_t CalcMergeSize(const DiffFolder& df);
  //............................................................................
  void Merge(const DiffFolder& df, const olxstr& dest=EmptyString ) const {
    TOnProgress onSync;
    onSync.SetMax( (double)CalcMergeSize(df) );
    OnSynchronise->Enter(NULL, &onSync);
    Root.Merge(df, onSync, dest);
    onSync.SetPos( onSync.GetMax() );
    OnSynchronise->Exit(NULL, &onSync);
  }
  //............................................................................
  void CopyTo(const olxstr& _dest, void (*AfterCopy)(const olxstr& src, const olxstr& dest)=NULL)  {
    TOnProgress OnSync;
    OnSync.SetMax( (double)Root.CalcSize() );
    OnSynchronise->Enter(NULL, &OnSync);
    Root.CopyTo(_dest, OnSync, AfterCopy);
    OnSync.SetMax( OnSync.GetMax() );
    OnSynchronise->Exit(NULL, &OnSync);
  }
  //............................................................................
  void Expand()  {
    TOnProgress onExp;
    onExp.SetMax( -1 );
    OnExpand->Enter(NULL, &onExp);
    Root.Expand(onExp);
    OnSynchronise->Exit(NULL, &onExp);
  }
  //............................................................................
  // if ContentOnly is true, the top folder is not deleted
  void Delete(bool ContentOnly=false)  {
    TOnProgress onDel;
    onDel.SetPos(0);
    onDel.SetMax( Root.CalcItemCount() );
    OnDelete->Enter(NULL, &onDel);
    Root.Delete(onDel, ContentOnly);
    onDel.SetPos( onDel.GetMax() );
    OnDelete->Exit(NULL, &onDel);
  }
  //............................................................................
  static void Delete(const olxstr& fn, bool ContentOnly=false)  {
    TFileTree ft(fn);
    ft.Expand();
    ft.Delete(ContentOnly);
  }
  //............................................................................
  uint64_t Compare(const TFileTree& ft) const {
    TOnProgress onCmp;
    onCmp.SetMax(-1);
    OnCompare->Enter(NULL, &onCmp);
    uint64_t sz = Root.Compare(ft.Root, onCmp);
    OnCompare->Exit(NULL, &onCmp);
    return sz;
  }
  //............................................................................
  void CalcMergedTree(const TFileTree& ft, DiffFolder& df) const {
    Root.CalcMergedTree(ft.Root, df);
  }
  //............................................................................
  void Synchronise(TFileTree& ft)  {
    TOnProgress onSync;
    onSync.SetMax(-1);
    OnSynchronise->Enter(NULL, &onSync);
    Root.Synchronise(ft.Root, onSync);
    OnSynchronise->Exit(NULL, &onSync);
  }
  //............................................................................
  friend class TFileTree::Folder;
};


EndEsdlNamespace()
#endif

