/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_file_tree_H
#define __olx_file_tree_H
#include "efile.h"
#include "actions.h"
#include "utf8file.h"
#undef CopyFile

BeginEsdlNamespace()

class TFileTree  {
public:
  // expansion flags
  static const short
    efNone   = 0x0000, // exceptions will be re-thrown
    efSafe   = 0x0001, // exceptions will be trapped
    efReport = 0x0002; // exceptions will be reported

  typedef TPtrList<const TFileListItem> TFilePtrList;
  template <class FT> struct TDiffFolder  {
    const FT *Src, *Dest;
    // the size is the same but some items might be missing
    TFilePtrList SrcFiles, DestFiles;
    TTypeList< TDiffFolder<FT> > Folders;
    TDiffFolder(const FT* src=NULL, const FT* dest=NULL) : Src(src), Dest(dest)  {  }
  };
///////////////////////////////////////////////////////////////////////////////////////
  class Folder : public TFileListItem {
    TFileList Files;
    TFileTree& FileTree;
    TTypeList<Folder> Folders;
    Folder* Parent;
    olxstr FullPath;
    typedef TDiffFolder<Folder> DiffFolder;
#ifdef __WIN32__
    static int CompareFiles(const TFileListItem &i1, const TFileListItem &i2)
    {
      return i1.GetName().Comparei(i2.GetName());
    }
    static int CompareFolders(const Folder &i1, const Folder &i2)  {
      return i1.Name.Comparei(i2.Name);
    }
    template <class T> static size_t FindSortedIndexOf(const T&list,
      const olxstr& entity)
    {
      if( list.Count() == 0 )  return InvalidIndex;
      if( list.Count() == 1 )
        return (list[0].GetName().Comparei(entity) == 0) ? 0 : InvalidIndex;
      size_t from = 0, to = list.Count()-1;
      if( list[from].GetName().Comparei(entity) == 0  )  return from;
      if( list[from].GetName().Comparei(entity) > 0 )  return InvalidIndex;
      if( list[to].GetName().Comparei(entity) == 0 )  return to;
      if( list[to].GetName().Comparei(entity) < 0  )  return InvalidIndex;
      while( true ) {
        size_t index = (to+from)/2;
        if( index == from || index == to)
          return InvalidIndex;
        if( list[index].GetName().Comparei(entity) < 0 )  from = index;
        else
          if( list[index].GetName().Comparei(entity) > 0 )  to  = index;
          else
            if( list[index].GetName().Comparei(entity) == 0 )  return index;
      }
      return InvalidIndex;
    }
#else
    static int CompareFiles(const TFileListItem &i1, const TFileListItem &i2)
    {
      return i1.GetName().Compare(i2.GetName());
    }
    static int CompareFolders(const Folder &i1, const Folder &i2)  {
      return i1.Name.Compare(i2.Name);
    }
    template <class T> static size_t FindSortedIndexOf(const T&list,
      const olxstr& entity)
    {
      if( list.Count() == 0 )  return InvalidIndex;
      if( list.Count() == 1 )
        return (list[0].GetName() == entity) ? 0 : InvalidIndex;
      size_t from = 0, to = list.Count()-1;
      if( list[from].GetName() == entity )  return from;
      if( list[from].GetName().Compare(entity) > 0 )  return InvalidIndex;
      if( list[to].GetName() == entity )  return to;
      if( list[to].GetName().Compare(entity) < 0  )  return InvalidIndex;
      while( true ) {
        size_t index = (to+from)/2;
        if( index == from || index == to)
          return InvalidIndex;
        if( list[index].GetName().Compare(entity) < 0 )  from = index;
        else
          if( list[index].GetName().Compare(entity) > 0 )  to  = index;
          else
            if( list[index].GetName() == entity )  return index;
      }
      return InvalidIndex;
    }
#endif
    void _ExpandNewFolder(DiffFolder& df, bool is_src) const;
  public:
    Folder(TFileTree& fileTree, const TFileListItem& info,
      const olxstr& fullPath, Folder* parent = NULL)
      : TFileListItem(info), FileTree(fileTree), Parent(parent)
    {
      FullPath = TEFile::OSPath(fullPath);
      TEFile::AddPathDelimeterI(FullPath);
    }
    const olxstr& GetFullPath() const {  return FullPath;  }
    size_t FileCount() const {  return Files.Count();  }
    const TFileListItem& GetFile(size_t i) const {  return Files[i];  }
    size_t FolderCount() const {  return Folders.Count();  }
    const Folder& GetFolder(size_t i) const {  return Folders[i];  }
    void NullFileEntry(size_t i) const {  Files.NullItem(i);  }
    bool IsEmpty() const {  return Files.IsEmpty() && Folders.IsEmpty();  }
    // calculates total size of the tree
    uint64_t CalcSize() const;
    // returns total number of files in folders
    size_t CalcItemCount() const;
    // reads the folder structure, flags determine how exceptions is handled
    void Expand(TOnProgress& pg, short flags);
    /* recursive deletion of the folder, must be expanded beforehand! Returns
    true if the deletion was successful, otherwise use, ListContent to get the
    list of remained items
    */
    bool Delete(TOnProgress& pg, bool ContentOnly=false);
    // compares folders and returns the difference size
    uint64_t Compare(const Folder& f, TOnProgress& pg) const;
    // calculates the difference tree between the folders
    void CalcMergedTree(const Folder& f, DiffFolder& df) const;
    // merges the difference tree, possibly into a different location
    void Merge(const DiffFolder& df, TOnProgress& onSync,
      const olxstr& _dest_n=EmptyString()) const;
    // copies and overwrites existing files and timestamp
    bool CopyTo(const olxstr& _dest,
                TOnProgress& OnSync,
                bool do_throw,
                void (*AfterCopy)(const olxstr& src, const olxstr& dest)=NULL
                ) const;
    /* synchronises two folders - only updates files, does not delete anything
    (yet) */
    void Synchronise(Folder& f, TOnProgress& onSync);
    // exports the folder index to a file
    void ExportIndex(const olxstr& fileName, TStrList* list=NULL,
      size_t level=0) const;
    // the function fills the list with full file names (recursively)
    void ListFilesEx(TStrList& out,
      const TTypeList<TEFile::TFileNameMask>* _mask=NULL) const;
    void ListFiles(TStrList& out, const olxstr& _mask) const;
    size_t CountFilesEx(
      const TTypeList<TEFile::TFileNameMask>* _mask=NULL) const;
    size_t CountFiles(const olxstr& _mask) const;
    /* creates a list like
    file: aaa.txt
    folder ccc
    if annotate is false, the file and folder annotations are not added
    */
    TStrList& ListContent(TStrList& out, bool annotate=true) const;
    const_strlist ListContent(bool annotate=true) const {
      TStrList l;
      return ListContent(l, annotate);
    }
  };
///////////////////////////////////////////////////////////////////////////////
public:
  typedef TFileTree::TDiffFolder<TFileTree::Folder> DiffFolder;
  class TFileTreeException : public TIOException {
  public:
    TFileTreeException(const olxstr& location, const olxstr &msg)
    : TIOException(location, msg)
    {}
    TFileTreeException(const olxstr& location, const olxstr &msg,
      const TStrList &content)
      : TIOException(location, msg),
        items(content)
    {}
    const char* GetNiceName() const { return "File tree exception"; }
    IOlxObject* Replicate() const { return new TFileTreeException(*this); }
    olxstr GetFullMessage() const;
    TStrList items;
  };
protected:
  TActionQList Actions;
  Folder Root;
  volatile bool Break;
public:

  TActionQueue &OnExpand, // called when the OS folder structure is traversed
    &OnSynchronise, // called when fodlers are merged
    &OnFileCopy,   // called in file copy process
    &OnFileCompare, // called in the comparison process
    &OnCompare, // called when files are being compared
    &OnDelete;  // called when recursive folder deletion is executed

  TFileTree(const olxstr& root)
    : Root(*this, TFileListItem(), root),
      Break(false),
      OnExpand(Actions.New("ON_EXPAND")),
      OnSynchronise(Actions.New("ON_SYNC")),
      OnFileCopy(Actions.New("ON_FCOPY")),
      OnFileCompare(Actions.New("ON_FCCOMPARE")),
      OnCompare(Actions.New("ON_COMPARE")),
      OnDelete(Actions.New("ON_DELETE"))
  {}
  const Folder& GetRoot() const {  return Root;  }
  void DoBreak() {  Break = true;  }
  // copies files (platform dependent, uses win API to handle huge files)
  bool CopyFile(const olxstr& from, const olxstr& to) const;
  // compares two files (platform independent, may fail on huge files)
  bool CompareFiles(const olxstr& from, const olxstr& to) const;
  //...........................................................................
  // calculates the number of bytes to be transfered when merging
  static uint64_t CalcMergeSize(const DiffFolder& df);
  //...........................................................................
  void Merge(const DiffFolder& df, const olxstr& dest=EmptyString() ) const {
    TOnProgress onSync;
    onSync.SetMax(CalcMergeSize(df));
    OnSynchronise.Enter(NULL, &onSync);
    Root.Merge(df, onSync, dest);
    onSync.SetPos(onSync.GetMax());
    OnSynchronise.Exit(NULL, &onSync);
  }
  //...........................................................................
  /* if do_throw is true, the processes will be terminated on the first error
  else all what is possible will be copied */
  bool CopyTo(const olxstr& _dest,
    void (*AfterCopy)(const olxstr& src, const olxstr& dest)=NULL,
    bool do_throw=true)
  {
    TOnProgress OnSync;
    OnSync.SetMax(Root.CalcSize());
    OnSynchronise.Enter(NULL, &OnSync);
    bool res = Root.CopyTo(_dest, OnSync, do_throw, AfterCopy);
    OnSync.SetMax(OnSync.GetMax());
    OnSynchronise.Exit(NULL, &OnSync);
    return res;
  }
  //...........................................................................
  // if do_throw is false no exceptions will be throws, also see above
  static bool Copy(const olxstr& src, const olxstr& dest, bool do_throw)  {
    try  {
      TFileTree ft(src);
      ft.Expand();
      return ft.CopyTo(dest, NULL, do_throw);
    }
    catch(const TExceptionBase&)  {
      return false;
    }
  }
  //...........................................................................
  void Expand(short flags=efNone)  {
    TOnProgress onExp;
    OnExpand.Enter(NULL, &onExp);
    Root.Expand(onExp, flags);
    OnSynchronise.Exit(NULL, &onExp);
  }
  //...........................................................................
  // if ContentOnly is true, the top folder is not deleted
  void Delete(bool ContentOnly=false)  {
    TOnProgress onDel;
    onDel.SetMax(Root.CalcItemCount());
    OnDelete.Enter(NULL, &onDel);
    if (!Root.Delete(onDel, ContentOnly)) {
      throw TFileTreeException(__OlxSourceInfo, "Deleteion failed",
        Root.ListContent());
    }
    onDel.SetPos(onDel.GetMax());
    OnDelete.Exit(NULL, &onDel);
  }
  //...........................................................................
  static void Delete(const olxstr& fn, bool ContentOnly=false)  {
    TFileTree ft(fn);
    ft.Expand();
    ft.Delete(ContentOnly);
  }
  //...........................................................................
  uint64_t Compare(const TFileTree& ft) const {
    TOnProgress onCmp;
    OnCompare.Enter(NULL, &onCmp);
    uint64_t sz = Root.Compare(ft.Root, onCmp);
    OnCompare.Exit(NULL, &onCmp);
    return sz;
  }
  //...........................................................................
  void CalcMergedTree(const TFileTree& ft, DiffFolder& df) const {
    Root.CalcMergedTree(ft.Root, df);
  }
  //...........................................................................
  void Synchronise(TFileTree& ft)  {
    TOnProgress onSync;
    OnSynchronise.Enter(NULL, &onSync);
    Root.Synchronise(ft.Root, onSync);
    OnSynchronise.Exit(NULL, &onSync);
  }
  //...........................................................................
  friend class TFileTree::Folder;
};

EndEsdlNamespace()
#endif
