#ifndef __olx_file_tree_H
#define __olx_file_tree_H

#include "efile.h"
#include "actions.h"

BeginEsdlNamespace()
class TFileTree  {
public:
  struct Folder  {
    TFileList Files;
    TFileTree& FileTree;
    TTypeList<Folder> Folders;
    Folder* Parent;
    olxstr FullPath, Name;
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
    size_t CalcSize()  {
      size_t res = 0;
      for( int i=0; i < Files.Count(); i++ )
        res += Files[i].GetSize();
      for( int i=0; i < Folders.Count(); i++ )
        res += Folders[i].CalcSize();
      return res;
    }
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
  };
///////////////////////////////////////////////////////////////////////////////////////
  TActionQList Actions;
public:
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
  bool CopyFileX(const olxstr& from, const olxstr& to, TOnProgress& pg)  {
    HANDLE in = CreateFile(from.u_str(),
      GENERIC_READ, 0, NULL,
      OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if( in == INVALID_HANDLE_VALUE )
      throw TFunctionFailedException(__OlxSourceInfo, olxstr("Failed to open: ") << from );

    HANDLE out = CreateFile(to.u_str(),
      GENERIC_WRITE, 0, NULL,
      CREATE_NEW, FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if( out == INVALID_HANDLE_VALUE )  {
      CloseHandle(in);
      throw TFunctionFailedException(__OlxSourceInfo, olxstr("Failed to create: ") << to );
    }
    DWORD fsHigh = 0;
    uint64_t fl = GetFileSize(in, &fsHigh);
    fl += MAXDWORD*fsHigh;
    const int bf_sz = 16*1024*1024;
    char* bf = new char[bf_sz];
    pg.SetMax( fl );
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
    pg.SetPos( fl );
    OnFileCopy->Execute(NULL, &pg);
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
    for( int i=0; i < full; i++ )  {
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
    for( int i=0; i < full; i++ )  {
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
};


EndEsdlNamespace()
#endif

