#ifndef __olx_file_tree_H
#define __olx_file_tree_H

#include "efile.h"
#include "actions.h"
#include "utf8file.h"

BeginEsdlNamespace()
class TFileTree  {
public:
  typedef TPtrList<const TFileListItem> TFilePtrList;

  template <class FT> struct TDiffFolder  {
    FT *Src, *Dest;
    // the size is the same but some items might be missing
    TFilePtrList SrcFiles, DestFiles;
    TTypeList< TDiffFolder<FT> > Folders;
    TDiffFolder(FT* src, FT* dest) : Src(src), Dest(dest)  {  }
  };
///////////////////////////////////////////////////////////////////////////////////////
  struct Folder  {
    TFileList Files;
    TFileTree& FileTree;
    TTypeList<Folder> Folders;
    Folder* Parent;
    olxstr FullPath, Name;
    typedef TDiffFolder<Folder> DiffFolder;
    Folder(TFileTree& fileTree, const olxstr& fullPath, Folder* parent = NULL) :
    Parent(parent), FileTree(fileTree)  {
      FullPath = TEFile::OSPath(fullPath);
      TEFile::AddTrailingBackslashI(FullPath);
      int ind = FullPath.LastIndexOf(TEFile::GetPathDelimeter(), FullPath.Length()-1);
      if( ind != -1 )
        Name = FullPath.SubStringFrom(ind+1, 1);
    }
    const olxstr& GetName() const {  return Name;  }
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
    uint64_t CalcSize()  {
      uint64_t res = 0;
      for( int i=0; i < Files.Count(); i++ )
        res += Files[i].GetSize();
      for( int i=0; i < Folders.Count(); i++ )
        res += Folders[i].CalcSize();
      return res;
    }
    // sends notification events when expanding
    void Expand(TOnProgress& pg)  {
      Files.Clear();
      pg.SetAction(FullPath);
      FileTree.OnExpand->Execute(NULL, &pg);
      TEFile::ListDirEx(FullPath, Files, "*", sefAll^sefRelDir);
      for( int i=0; i < Files.Count(); i++ )  {
        if( (Files[i].GetAttributes() & sefDir) == sefDir )  {
          Folders.Add( new Folder(FileTree, FullPath + Files[i].GetName(), this)).Expand(pg);
          Files.NullItem(i);
        }
      }
      Files.Pack();
      Files.QuickSorter.SortSF(Files, &CompareFiles);
      Folders.QuickSorter.SortSF(Folders, &CompareFolders);
    }
    // a quiet version of the above
    void Expand()  {
      Files.Clear();
      TEFile::ListDirEx(FullPath, Files, "*", sefAll^sefRelDir);
      for( int i=0; i < Files.Count(); i++ )  {
        if( (Files[i].GetAttributes() & sefDir) == sefDir )  {
          Folders.Add( new Folder(FileTree, FullPath + Files[i].GetName(), this)).Expand();
          Files.NullItem(i);
        }
      }
      Files.Pack();
      Files.QuickSorter.SortSF(Files, &CompareFiles);
      Folders.QuickSorter.SortSF(Folders, &CompareFolders);
    }
    // recursive deletion of the folder, must be expanded beforehand!
    bool Delete()  {
      try  {
        for( int i=0; i < Folders.Count(); i++ )  {
          Folders.NullItem(i);
          Folders[i].Delete();
        }
      }
      catch( const TExceptionBase& exc )  {
        Folders.Pack();
        throw TFunctionFailedException(__OlxSourceInfo, exc);
      }
      Folders.Clear();
      for( int i=0; i < Files.Count(); i++ )  {
        olxstr fn = FullPath + Files[i].GetName();
        if( !TEFile::DelFile(fn) )  {
          Files.Pack();
          throw TFunctionFailedException(__OlxSourceInfo, olxstr("Could not delete file: ") << fn);
        }
        else
          Files.NullItem(i);
      }
      Files.Pack();
      if( !TEFile::RmDir(FullPath) )
        throw TFunctionFailedException(__OlxSourceInfo, olxstr("Could not delete folder: ") << FullPath);
    }
    double Compare(const Folder& f, TOnProgress& pg) const {
      double total_size = 0;
      for( int i=0; i < Files.Count(); i++ )  {
        int ind = FindSortedIndexOf( f.Files, Files[i].GetName() );
        if( ind == -1 )  {
          total_size += Files[i].GetSize();
          pg.SetAction(olxstr("New file: ") << FullPath << Files[i].GetName());
          FileTree.OnCompare->Execute(NULL, &pg);
        }
        else if( Files[i].GetModificationTime() != f.Files[ind].GetModificationTime() ||
                 Files[i].GetSize() != f.Files[ind].GetSize() )  {
          pg.SetAction(olxstr("Changed file: ") << FullPath << Files[i].GetName());
          FileTree.OnCompare->Execute(NULL, &pg);
        }
      }
      for( int i=0; i < Folders.Count(); i++ )  {
        int ind = FindSortedIndexOf( f.Folders, Folders[i].Name );
        if( ind == -1 )  {
          total_size += Folders[i].CalcSize();
          pg.SetAction(olxstr("New folder: ") << Folders[i].FullPath);
          FileTree.OnCompare->Execute(NULL, &pg);
        }
        else
          total_size += Folders[i].Compare( f.Folders[ind], pg );
      }
      return total_size;
    }
    void _ExpandNewFolder(DiffFolder& df, bool is_src) const {
      TFilePtrList& dst = (is_src ? df.DestFiles : df.SrcFiles);
      TFilePtrList& src = (is_src ? df.SrcFiles : df.DestFiles);
      dst.SetCapacity( dst.Count() + Files.Count() );
      src.SetCapacity( src.Count() + Files.Count() );
      for( int i=0; i < Files.Count(); i++ )  {
        src.Add( &Files[i] );
        dst.Add( NULL );
      }
      for( int i=0; i < Folders.Count(); i++ )
        Folders[i]._ExpandNewFolder(
          df.Folders.Add(
            new DiffFolder(is_src ? &Folders[i] : NULL, is_src ? NULL : &Folders[i])),
          is_src
        );
    }
    void CalcMergedTree(const Folder& f, DiffFolder& df) const {
      for( int i=0; i < Files.Count(); i++ )  {
        int ind = FindSortedIndexOf( f.Files, Files[i].GetName() );
        df.SrcFiles.Add(&Files[i]);
        if( ind == -1 )
          df.DestFiles.Add(NULL);
        else
          df.DestFiles.Add(&f.Files[ind]);
      }
      // complete the tree
      for( int i=0; i < f.Files.Count(); i++ )  {
        int ind = FindSortedIndexOf( Files, f.Files[i].GetName() );
        if( ind == -1 )
          df.DestFiles.Add(&f.Files[ind]);
      }
      for( int i=0; i < Folders.Count(); i++ )  {
        int ind = FindSortedIndexOf( f.Folders, Folders[i].Name );
        if( ind == -1 )
          Folders[i]._ExpandNewFolder(
            df.Folders.Add( new DiffFolder(&Folders[i], NULL) ),
            true
          );
        else
          Folders[i].CalcMergedTree( f.Folders[ind], df.Folders.AddNew(&Folders[i], &f.Folders[ind]));
      }
      // complete the tree
      for( int i=0; i < f.Folders.Count(); i++ )  {
        int ind = FindSortedIndexOf( Folders, f.Folders[i].Name );
        if( ind == -1 )
          f.Folders[i]._ExpandNewFolder(
            df.Folders.Add( new DiffFolder(NULL, &f.Folders[i]) ),
            false
          );
      }
    }
    void Synchronise(Folder& f, TOnProgress& onSync, TOnProgress& onFileCopy) {
      if( FileTree.Stop )
        return;
      for( int i=0; i < Files.Count(); i++ )  {
        int ind = FindSortedIndexOf( f.Files, Files[i].GetName() );
        if( ind == -1 )  {
          olxstr nf(f.FullPath + Files[i].GetName());
          onSync.SetAction( olxstr("New file: ") << FullPath << Files[i].GetName() );
#ifdef __WIN32__
          if( FileTree.CopyFileX( olxstr(FullPath) << Files[i].GetName(), nf, onFileCopy) )  {
#else
          if( FileTree.CopyFile( olxstr(FullPath) << Files[i].GetName(), nf, onFileCopy) )  {
#endif
            bool res = TEFile::SetFileTimes(nf, Files[i].GetLastAccessTime(), Files[i].GetModificationTime());
            if( !res )
              throw TFunctionFailedException(__OlxSourceInfo, "settime");
            f.Files.AddCCopy(Files[i]);
            f.Files.QuickSorter.SortSF(f.Files, &CompareFiles);
            onSync.SetPos( onSync.GetPos() + Files[i].GetSize() );
            FileTree.OnSynchronise->Execute(NULL, &onSync);
          }
        }
        else if(Files[i].GetModificationTime() != f.Files[ind].GetModificationTime() )  {
          onSync.SetAction( olxstr("Changed file: ") << FullPath << Files[i].GetName() );
          FileTree.OnSynchronise->Execute(NULL, &onSync);
        }
      }
      for( int i=0; i < Folders.Count(); i++ )  {
        int ind = FindSortedIndexOf( f.Folders, Folders[i].Name );
        if( ind == -1 )  {
          onSync.SetAction( olxstr("New folder: ") << FullPath << Folders[i].Name );
          FileTree.OnSynchronise->Execute(NULL, &onSync);
          TEFile::MakeDir( olxstr(f.FullPath)  <<  Folders[i].Name);
          Folders[i].Synchronise(f.Folders.Add( new Folder(FileTree, olxstr(f.FullPath)  <<  Folders[i].Name, &f)), onSync, onFileCopy);
          f.Folders.QuickSorter.SortSF(f.Folders, &CompareFolders);
        }
        else
          Folders[i].Synchronise(f.Folders[ind], onSync, onFileCopy);
      }
    }
    void ExportIndex(const olxstr& fileName, TStrList* list = NULL, int level=0) const {
      TStrList* lst = (list == NULL) ? new TStrList : list;
      for( int i=0; i < Files.Count(); i++ )  {
        lst->Add( olxstr::CharStr('\t', level) ) << Files[i].GetName(); 
        lst->Add( olxstr::CharStr('\t', level) ) << Files[i].GetModificationTime() << ',' << Files[i].GetSize() << "{}"; 
      }
      for( int i=0; i < Folders.Count(); i++ )
        Folders[i].ExportIndex(fileName, lst, level+1);
      if( list == NULL )  {
        try { TUtf8File::WriteLines(fileName, *lst, false);  }
        catch( const TExceptionBase& exc )  {
          delete lst;
          throw TFunctionFailedException(__OlxSourceInfo, exc, "failed to save index");
        }
        delete lst;
      }
    }
    // the function fills the list with full file names
    void ListFilesEx(TStrList& out, const TTypeList<TEFile::TFileNameMask>* _mask=NULL) const {
      out.SetCapacity( out.Count() + Files.Count() );
      if( _mask != NULL && !_mask->IsEmpty() )   {
        for( int i=0; i < Files.Count(); i++ )  {
          for( int j=0; j < _mask->Count(); j++ )  {
            if( (*_mask)[j].DoesMatch( Files[i].GetName() ) )  {
              out.Add(FullPath) << Files[i].GetName();
              break;
            }
          }
        }
      }
      else  {
        for( int i=0; i < Files.Count(); i++ )  
          out.Add(FullPath) << Files[i].GetName();
      }
      for( int i=0; i < Folders.Count(); i++ )
        Folders[i].ListFilesEx(out, _mask);
    }
    void ListFiles(TStrList& out, const olxstr& _mask) const {
      TStrList toks(_mask, ";");
      if( !toks.IsEmpty() )  {
        TTypeList<TEFile::TFileNameMask> mask;
        for( int i=0; i < toks.Count(); i++ )
          mask.AddNew( toks[i] );
        ListFilesEx(out, &mask);
      }
      else
        ListFilesEx(out, NULL);
    }
  };
///////////////////////////////////////////////////////////////////////////////////////
  TActionQList Actions;
public:

  typedef TFileTree::TDiffFolder<TFileTree::Folder> DiffFolder;

  Folder Root;
  bool Stop;
  TActionQueue* OnExpand, *OnSynchronise, *OnFileCopy, *OnFileCompare, *OnCompare;
  TFileTree(const olxstr& root) : Root(*this, root)  {
    OnExpand = &Actions.NewQueue("ON_EXPAND");
    OnSynchronise = &Actions.NewQueue("ON_SYNC");
    OnFileCopy = &Actions.NewQueue("ON_FCopy");
    OnFileCompare = &Actions.NewQueue("ON_FCompare");
    OnCompare = &Actions.NewQueue("ON_Compare");
    Stop = false;
  }
#ifdef __WIN32__
  bool CopyFileX(const olxstr& from, const olxstr& to, TOnProgress& pg) const {
    HANDLE in = CreateFile(from.u_str(),
      GENERIC_READ, 0, NULL,
      OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if( in == INVALID_HANDLE_VALUE )
      throw TFunctionFailedException(__OlxSourceInfo, olxstr("Failed to open: ") << from );

    HANDLE out = CreateFile(to.u_str(),
      GENERIC_WRITE, 0, NULL,
      OPEN_ALWAYS, FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if( out == INVALID_HANDLE_VALUE )  {
      CloseHandle(in);
      throw TFunctionFailedException(__OlxSourceInfo, olxstr("Failed to create: ") << to );
    }
    DWORD fsHigh = 0;
    uint64_t fl = GetFileSize(in, &fsHigh);
    fl += MAXDWORD*fsHigh;
    const int bf_sz = 16*1024*1024;
    char* bf = new char[bf_sz];
    pg.SetMax( (double)fl );
    pg.SetPos(0);
    pg.SetAction( from );
    DWORD read = 1;
    while( true )  {
      if( ReadFile(in, bf, bf_sz, &read, NULL) == 0 )  {
        CloseHandle(in);
        CloseHandle(out);
        throw TFunctionFailedException(__OlxSourceInfo, "read failed" );
      }
      if( read == 0 )  break;
      if( Stop )  {
        delete [] bf;
        CloseHandle(in);
        CloseHandle(out);
        DeleteFile(to.u_str());
        return false;
      }
      DWORD written = 0;
      while( read != 0 )  {
        if( WriteFile(out, bf, read, &written, NULL) == 0 )  {
          CloseHandle(in);
          CloseHandle(out);
          throw TFunctionFailedException(__OlxSourceInfo, "write failed" );
        }
        read -= written;
        pg.SetPos( pg.GetPos() + written);
        OnFileCopy->Execute(NULL, &pg);
      }
    }
    pg.SetPos( (double)fl );
    OnFileCopy->Execute(NULL, &pg);
    pg.SetPos(0);
    delete [] bf;
    CloseHandle(in);
    CloseHandle(out);
    return true;
  }
#endif
  // will fails on huge files
  bool CopyFile(const olxstr& from, const olxstr& to, TOnProgress& pg)  {
    TEFile in(from, "rb"), out(to, "wb+");
    const int bf_sz = 16*1024*1024;
    char* bf = new char[bf_sz];
    size_t fl = in.Length();
    pg.SetMax( fl );
    pg.SetPos(0);
    pg.SetAction( from );
    size_t full = fl/bf_sz,
           part = fl%bf_sz;
    for( size_t i=0; i < full; i++ )  {
      in.Read(bf, bf_sz);
      out.Write(bf, bf_sz);
      pg.SetPos( (i+1)*bf_sz);
      OnFileCopy->Execute(NULL, &pg);
      if( Stop )  {
        delete [] bf;
        out.Delete();
        return false;
      }
    }
    in.Read(bf, part);
    out.Write(bf, part);
    pg.SetPos( fl );
    OnFileCopy->Execute(NULL, &pg);
    delete [] bf;
    return true;
  }
  bool CompareFiles(const olxstr& from, const olxstr& to, TOnProgress& pg)  {
    TEFile f1(from, "rb"), f2(to, "rb");
    if( f1.Length() != f2.Length() )  return false;
    const int bf_sz = 16*1024*1024;
    char* bf1 = new char[bf_sz];
    char* bf2 = new char[bf_sz];
    size_t fl = f1.Length();
    pg.SetMax( fl );
    pg.SetPos(0);
    pg.SetAction( from );
    size_t full = fl/bf_sz,
           part = fl%bf_sz;
    for( size_t i=0; i < full; i++ )  {
      f1.Read(bf1, bf_sz);
      f2.Read(bf2, bf_sz);
      if( olxstr::o_memcmp(bf1, bf2, bf_sz) != 0 )  {
        delete [] bf1;
        delete [] bf2;
        return false;
      }
      OnFileCompare->Execute(NULL, &pg);
    }
    f1.Read(bf1, part);
    f2.Read(bf1, part);
    bool res = olxstr::o_memcmp(bf1, bf2, part) != 0;
    pg.SetPos( fl );
    OnFileCompare->Execute(NULL, &pg);
    delete [] bf1;
    delete [] bf2;
    return res;
  }
  //............................................................................
  static uint64_t CalcMergeSize(const DiffFolder& df)  {
    if( df.Src == NULL )
      return 0;
    if( df.Dest == NULL )
      return df.Src->CalcSize();
    uint64_t sz = 0;
    for( int i=0; i < df.SrcFiles.Count(); i++ )  {
      if( df.SrcFiles[i] != NULL )
        sz += df.SrcFiles[i]->GetSize();
    }
    for( int i=0; i < df.Folders.Count(); i++ )
      sz += CalcMergeSize(df.Folders[i]);
    return sz;
  }
  //............................................................................
  void DoMerge(const DiffFolder& df, TOnProgress& onSync,
    TOnProgress& onFileCopy, const olxstr& _dest_n = EmptyString) const
  {
    if( df.Src == NULL )
      return;
    olxstr dest_n( df.Dest == NULL ? _dest_n : df.Dest->FullPath);
    if( df.Dest == NULL )
      TEFile::MakeDir(dest_n);

    for( int i=0; i < df.Src->Files.Count(); i++ )  {
      if( df.Src->Files.IsNull(i) )
        continue;
      olxstr nf(olxstr(dest_n) << df.Src->Files[i].GetName());
#ifdef __WIN32__
      if( CopyFileX( olxstr(df.Src->FullPath) << df.Src->Files[i].GetName(), nf, onFileCopy) )  {
#else
      if( CopyFile( olxstr(FullPath) << df.Src->Files[i].GetName(), nf, onFileCopy) )  {
#endif
        bool res = TEFile::SetFileTimes(nf, df.Src->Files[i].GetLastAccessTime(), df.Src->Files[i].GetModificationTime());
        if( !res )
          throw TFunctionFailedException(__OlxSourceInfo, "settime");
        onSync.SetPos( onSync.GetPos() + df.Src->Files[i].GetSize() );
        onSync.SetAction( df.Src->Files[i].GetName() );
        OnSynchronise->Execute(NULL, &onSync);
      }
      if( Stop )
        return;
    }
    for( int i=0; i < df.Folders.Count(); i++ )  {
      DiffFolder& f = df.Folders[i];
      if( f.Src == NULL )  continue;
      DoMerge(f, onSync, onFileCopy, (dest_n + f.Src->Name) << '/');
    }
  }
};


EndEsdlNamespace()
#endif

