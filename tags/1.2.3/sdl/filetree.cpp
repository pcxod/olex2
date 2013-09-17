/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "filetree.h"
#undef CopyFile

#include "bapp.h"

uint64_t TFileTree::Folder::CalcSize() const {
  uint64_t res = 0;
  for( size_t i=0; i < Files.Count(); i++ )
    res += Files[i].GetSize();
  for( size_t i=0; i < Folders.Count(); i++ )
    res += Folders[i].CalcSize();
  return res;
}
//.............................................................................
size_t TFileTree::Folder::CalcItemCount() const {
  size_t res = Files.Count() + Folders.Count();
  for( size_t i=0; i < Folders.Count(); i++ )
    res += Folders[i].CalcItemCount();
  return res;
}
//.............................................................................
void TFileTree::Folder::Expand(TOnProgress& pg, short flags)  {
  Files.Clear();
  pg.SetAction(FullPath);
  FileTree.OnExpand.Execute(NULL, &pg);
  try {
    TEFile::ListDirEx(FullPath, Files, "*", sefAll^sefRelDir);
  }
  catch( const TExceptionBase &e)  {
    if( (flags& efReport) != 0 )  {
      TBasicApp::NewLogEntry(logException)
        << e.GetException()->GetFullMessage();
    }
    if( (flags&efSafe) == 0 )
      throw TFunctionFailedException(__OlxSourceInfo, e);
  }
  for( size_t i=0; i < Files.Count(); i++ )  {
    if( (Files[i].GetAttributes() & sefDir) == sefDir )  {
      Folders.Add(new Folder(FileTree, Files[i], FullPath + Files[i].GetName(),
        this)).Expand(pg, flags);
      Files.NullItem(i);
    }
  }
  Files.Pack();
  QuickSorter::SortSF(Files, &TFileTree::Folder::CompareFiles);
  QuickSorter::SortSF(Folders, &TFileTree::Folder::CompareFolders);
}
//.............................................................................
bool TFileTree::Folder::Delete(TOnProgress& pg, bool ContentOnly)  {
  try {
    for( size_t i=0; i < Files.Count(); i++ )  {
      olxstr fn = FullPath + Files[i].GetName();
      pg.SetAction(fn);
      pg.IncPos(1);
      FileTree.OnDelete.Execute(NULL, &pg);
      if (TEFile::DelFile(fn))
        Files.NullItem(i);
    }
    Files.Pack();
    for( size_t i=0; i < Folders.Count(); i++ )  {
      try  {
        pg.IncPos(1);
        pg.SetAction(Folders[i].FullPath);
        FileTree.OnDelete.Execute(NULL, &pg);
        if (Folders[i].Delete(pg))
          Folders.NullItem(i);
      }
      catch (const TExceptionBase& exc)  {
        Folders.Pack();
        throw TFunctionFailedException(__OlxSourceInfo, exc);
      }
    }
    Folders.Pack();
    const bool remove = (Parent == NULL ? !ContentOnly : true);
    if( remove && !TEFile::RmDir(FullPath) )
      return false;
    return IsEmpty();
  }
  catch (const TExceptionBase &) {
    Files.Pack();
    Folders.Pack();
    return IsEmpty();
  }
}
//.............................................................................
uint64_t TFileTree::Folder::Compare(const Folder& f, TOnProgress& pg) const {
  uint64_t total_size = 0;
  for( size_t i=0; i < Files.Count(); i++ )  {
    size_t ind = FindSortedIndexOf( f.Files, Files[i].GetName() );
    if( ind == InvalidIndex )  {
      total_size += Files[i].GetSize();
      pg.SetAction(olxstr("New file: ") << FullPath << Files[i].GetName());
      FileTree.OnCompare.Execute(NULL, &pg);
    }
    else if( Files[i].GetModificationTime() !=
              f.Files[ind].GetModificationTime() ||
            Files[i].GetSize() != f.Files[ind].GetSize() )
    {
      pg.SetAction(olxstr("Changed file: ") << FullPath << Files[i].GetName());
      FileTree.OnCompare.Execute(NULL, &pg);
    }
  }
  for( size_t i=0; i < Folders.Count(); i++ )  {
    size_t ind = FindSortedIndexOf( f.Folders, Folders[i].Name );
    if( ind == InvalidIndex )  {
      total_size += Folders[i].CalcSize();
      pg.SetAction(olxstr("New folder: ") << Folders[i].FullPath);
      FileTree.OnCompare.Execute(NULL, &pg);
    }
    else
      total_size += Folders[i].Compare( f.Folders[ind], pg );
  }
  return total_size;
}
//.............................................................................
void TFileTree::Folder::_ExpandNewFolder(DiffFolder& df, bool is_src) const {
  TFilePtrList& dst = (is_src ? df.DestFiles : df.SrcFiles);
  TFilePtrList& src = (is_src ? df.SrcFiles : df.DestFiles);
  dst.SetCapacity( dst.Count() + Files.Count() );
  src.SetCapacity( src.Count() + Files.Count() );
  for( size_t i=0; i < Files.Count(); i++ )  {
    src.Add( &Files[i] );
    dst.Add( NULL );
  }
  for( size_t i=0; i < Folders.Count(); i++ )
    Folders[i]._ExpandNewFolder(
      df.Folders.Add(
        new DiffFolder(is_src ? &Folders[i] : NULL, is_src ? NULL
          : &Folders[i])), is_src
    );
}
//.............................................................................
void TFileTree::Folder::CalcMergedTree(const Folder& f, DiffFolder& df) const {
  df.Src = this;
  df.Dest = &f;
  for( size_t i=0; i < Files.Count(); i++ )  {
    size_t ind = FindSortedIndexOf( f.Files, Files[i].GetName() );
    df.SrcFiles.Add(&Files[i]);
    if( ind == InvalidIndex )
      df.DestFiles.Add(NULL);
    else
      df.DestFiles.Add(&f.Files[ind]);
  }
  // complete the tree
  for( size_t i=0; i < f.Files.Count(); i++ )  {
    size_t ind = FindSortedIndexOf( Files, f.Files[i].GetName() );
    if( ind == InvalidIndex )
      df.DestFiles.Add(&f.Files[ind]);
  }
  for( size_t i=0; i < Folders.Count(); i++ )  {
    size_t ind = FindSortedIndexOf( f.Folders, Folders[i].Name );
    if( ind == InvalidIndex )
      Folders[i]._ExpandNewFolder(
      df.Folders.Add(new DiffFolder(&Folders[i], NULL)),
      true
      );
    else  {
      Folders[i].CalcMergedTree( f.Folders[ind], df.Folders.AddNew(&Folders[i],
        &f.Folders[ind]));
    }
  }
  // complete the tree
  for( size_t i=0; i < f.Folders.Count(); i++ )  {
    size_t ind = FindSortedIndexOf(Folders, f.Folders[i].Name);
    if( ind == InvalidIndex )
      f.Folders[i]._ExpandNewFolder(
      df.Folders.Add(new DiffFolder(NULL, &f.Folders[i])),
      false
      );
  }
}
//.............................................................................
void TFileTree::Folder::Merge(const DiffFolder& df, TOnProgress& onSync,
  const olxstr& _dest_n) const
{
  if( df.Src == NULL )  return;
  if( FileTree.Break )  return;
  olxstr dest_n( df.Dest == NULL ? _dest_n : df.Dest->GetFullPath());
  if( df.Dest == NULL && !TEFile::Exists(dest_n) )
    TEFile::MakeDirs(dest_n);

  for( size_t i=0; i < df.Src->Files.Count(); i++ )  {
    if( df.Src->Files.IsNull(i) )  continue;
    olxstr nf(olxstr(dest_n) << df.Src->Files[i].GetName());
    if( FileTree.CopyFile(
          olxstr(df.Src->FullPath) << df.Src->Files[i].GetName(), nf) )
    {
      bool res = TEFile::SetFileTimes(nf, df.Src->Files[i].GetLastAccessTime(),
        df.Src->Files[i].GetModificationTime());
      if( !res )
        throw TFunctionFailedException(__OlxSourceInfo, "settime");
      onSync.SetPos(onSync.GetPos() + df.Src->Files[i].GetSize());
      onSync.SetAction(df.Src->Files[i].GetName());
      FileTree.OnSynchronise.Execute(NULL, &onSync);
    }
    if( FileTree.Break )  return;
  }
  for( size_t i=0; i < df.Folders.Count(); i++ )  {
    DiffFolder& f = df.Folders[i];
    if( f.Src == NULL )  continue;
    Merge(f, onSync, (dest_n + f.Src->Name) << '/');
  }
}
//.............................................................................
bool TFileTree::Folder::CopyTo(const olxstr& _dest,
            TOnProgress& OnSync,
            bool do_throw,
            void (*AfterCopy)(const olxstr& src, const olxstr& dest)
            ) const
{
  if( FileTree.Break )  return false;
  olxstr dest = TEFile::AddPathDelimeter(
    _dest + (Parent == NULL ? EmptyString() : Name));
  if( !TEFile::Exists(dest) )  {
    if( !TEFile::MakeDir(dest) || !TEFile::MakeDirs(dest) )  {
      if( do_throw )  {
        throw TFunctionFailedException(__OlxSourceInfo,
          olxstr("Could not create folder: ").quote() << dest);
      }
      return false;
    }
  }
  bool final_res = true;
  for( size_t i=0; i < Files.Count(); i++ )  {
    if( FileTree.Break )  return false;
    olxstr src_fn(FullPath + Files[i].GetName());
    olxstr dest_fn(dest + Files[i].GetName());
    try  {
      FileTree.CopyFile(src_fn, dest_fn);
      OnSync.SetAction(dest_fn);
      OnSync.IncPos(Files[i].GetSize());
      FileTree.OnSynchronise.Execute(NULL, &OnSync);
      bool res = TEFile::SetFileTimes(dest_fn, Files[i].GetLastAccessTime(),
        Files[i].GetModificationTime());
      if( !res )
        throw TFunctionFailedException(__OlxSourceInfo, "settime");
    }
    catch( const TExceptionBase& e)  {
      if( do_throw )
        throw TFunctionFailedException(__OlxSourceInfo, e);
      final_res = false;
      continue;
    }
    if( AfterCopy != NULL )
      (*AfterCopy)(src_fn, dest_fn);
  }
  for( size_t i=0; i < Folders.Count(); i++ )  {
    if( !Folders[i].CopyTo(dest, OnSync, do_throw, AfterCopy) )
      final_res = false;
  }
  return final_res;
}
//.............................................................................
void TFileTree::Folder::Synchronise(TFileTree::Folder& f, TOnProgress& onSync)
{
  if( FileTree.Break )  return;
  for( size_t i=0; i < Files.Count(); i++ )  {
    if( FileTree.Break )  return;
    size_t ind = FindSortedIndexOf( f.Files, Files[i].GetName() );
    if( ind == InvalidIndex )  {
      olxstr nf(f.FullPath + Files[i].GetName());
      onSync.SetAction(olxstr("New file: ") << FullPath << Files[i].GetName());
      if( FileTree.CopyFile( olxstr(FullPath) << Files[i].GetName(), nf) )  {
        bool res = TEFile::SetFileTimes(nf, Files[i].GetLastAccessTime(),
          Files[i].GetModificationTime());
        if( !res )
          throw TFunctionFailedException(__OlxSourceInfo, "settime");
        f.Files.AddCopy(Files[i]);
        QuickSorter::SortSF(f.Files, &CompareFiles);
        onSync.SetPos( onSync.GetPos() + Files[i].GetSize() );
        FileTree.OnSynchronise.Execute(NULL, &onSync);
      }
    }
    else if( Files[i].GetModificationTime() !=
             f.Files[ind].GetModificationTime() )
    {
      onSync.SetAction(olxstr("Changed file: ") << FullPath
        << Files[i].GetName());
      FileTree.OnSynchronise.Execute(NULL, &onSync);
    }
  }
  for( size_t i=0; i < Folders.Count(); i++ )  {
    size_t ind = FindSortedIndexOf(f.Folders, Folders[i].Name);
    if( ind == InvalidIndex )  {
      onSync.SetAction(olxstr("New folder: ") << FullPath << Folders[i].Name);
      FileTree.OnSynchronise.Execute(NULL, &onSync);
      olxstr fn = olxstr(f.FullPath) << Folders[i].Name;
      TEFile::MakeDir(fn);
      bool res = TEFile::SetFileTimes(fn, Folders[i].GetLastAccessTime(),
        Folders[i].GetModificationTime());
      if( !res )
        throw TFunctionFailedException(__OlxSourceInfo, "settime");
      Folders[i].Synchronise(
        f.Folders.Add(
          new Folder(FileTree, Folders[i],
            olxstr(f.FullPath)  <<  Folders[i].Name, &f)), onSync);
      QuickSorter::SortSF(f.Folders, &CompareFolders);
    }
    else
      Folders[i].Synchronise(f.Folders[ind], onSync);
  }
}
//.............................................................................
void TFileTree::Folder::ExportIndex(const olxstr& fileName, TStrList* list,
  size_t level) const
{
  TStrList* lst = (list == NULL) ? new TStrList : list;
  for( size_t i=0; i < Files.Count(); i++ )  {
    lst->Add(olxstr::CharStr('\t', level) ) << Files[i].GetName(); 
    lst->Add(olxstr::CharStr('\t', level) ) << Files[i].GetModificationTime()
      << ',' << Files[i].GetSize() << "{}"; 
  }
  for( size_t i=0; i < Folders.Count(); i++ )
    Folders[i].ExportIndex(fileName, lst, level+1);
  if( list == NULL )  {
    try { TUtf8File::WriteLines(fileName, *lst, false);  }
    catch( const TExceptionBase& exc )  {
      delete lst;
      throw TFunctionFailedException(__OlxSourceInfo, exc,
        "failed to save index");
    }
    delete lst;
  }
}
//.............................................................................
void TFileTree::Folder::ListFilesEx(TStrList& out,
  const TTypeList<TEFile::TFileNameMask>* _mask) const
{
  out.SetCapacity( out.Count() + Files.Count() );
  if( _mask != NULL && !_mask->IsEmpty() )   {
    for( size_t i=0; i < Files.Count(); i++ )  {
      for( size_t j=0; j < _mask->Count(); j++ )  {
        if( (*_mask)[j].DoesMatch(Files[i].GetName()) )  {
          out.Add(FullPath) << Files[i].GetName();
          break;
        }
      }
    }
  }
  else  {
    for( size_t i=0; i < Files.Count(); i++ )  
      out.Add(FullPath) << Files[i].GetName();
  }
  for( size_t i=0; i < Folders.Count(); i++ )
    Folders[i].ListFilesEx(out, _mask);
}
//.............................................................................
void TFileTree::Folder::ListFiles(TStrList& out, const olxstr& _mask) const {
  TStrList toks(_mask, ";");
  if( !toks.IsEmpty() )  {
    TTypeList<TEFile::TFileNameMask> mask;
    for( size_t i=0; i < toks.Count(); i++ )
      mask.AddNew(toks[i]);
    ListFilesEx(out, &mask);
  }
  else
    ListFilesEx(out, NULL);
}
//.............................................................................
size_t TFileTree::Folder::CountFilesEx(
  const TTypeList<TEFile::TFileNameMask>* _mask) const
{
  size_t cnt = 0;
  if( _mask != NULL && !_mask->IsEmpty() )   {
    for( size_t i=0; i < Files.Count(); i++ )  {
      for( size_t j=0; j < _mask->Count(); j++ )  {
        if( (*_mask)[j].DoesMatch(Files[i].GetName()) )  {
          cnt++;
          break;
        }
      }
    }
  }
  else
    cnt = Files.Count();
  for( size_t i=0; i < Folders.Count(); i++ )
    cnt += Folders[i].CountFilesEx(_mask);
  return cnt;
}
//.............................................................................
size_t TFileTree::Folder::CountFiles(const olxstr& _mask) const {
  TStrList toks(_mask, ";");
  if( !toks.IsEmpty() )  {
    TTypeList<TEFile::TFileNameMask> mask;
    for( size_t i=0; i < toks.Count(); i++ )
      mask.AddNew(toks[i]);
    return CountFilesEx(&mask);
  }
  else
    return CountFilesEx(NULL);
}
//.............................................................................
TStrList &TFileTree::Folder::ListContent(TStrList &out, bool annotate) const {
  for (size_t i=0; i < Files.Count(); i++) {
    (annotate ? out.Add("File: ") : out.Add()) <<
      FullPath << Files[i].GetName();
  }
  for (size_t i=0; i < Folders.Count(); i++) {
    (annotate ? out.Add("Folder: ") : out.Add()) << Folders[i].GetFullPath();
    Folders[i].ListContent(out, annotate);
  }
  return out;
}
//_____________________________________________________________________________
//_____________________________________________________________________________
//_____________________________________________________________________________
#ifdef __WIN32__
bool TFileTree::CopyFile(const olxstr& from, const olxstr& to) const {
  HANDLE in = CreateFile(from.u_str(),
    GENERIC_READ, 0, NULL,
    OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
  if( in == INVALID_HANDLE_VALUE )  {
    throw TFunctionFailedException(__OlxSourceInfo,
      olxstr("Failed to open: ").quote() << from);
  }
  if( TEFile::Exists(to) )  { // delete the file, as CreateFile fails...
    if( !TEFile::DelFile(to) )  {
      throw TFunctionFailedException(__OlxSourceInfo,
        olxstr("Failed to delete: ").quote() << to);
    }
  }
  HANDLE out = CreateFile(to.u_str(),
    GENERIC_WRITE, 0, NULL,
    CREATE_ALWAYS, FILE_FLAG_BACKUP_SEMANTICS, NULL);
  if( out == INVALID_HANDLE_VALUE )  {
    CloseHandle(in);
    throw TFunctionFailedException(__OlxSourceInfo,
      olxstr("Failed to create: ").quote() << to);
  }
  DWORD fsHigh = 0;
  uint64_t fl = GetFileSize(in, &fsHigh);
  fl += MAXDWORD*fsHigh;
  const int bf_sz = 16*1024*1024;
  char* bf = new char[bf_sz];

  TOnProgress pg;
  pg.SetMax(fl);
  pg.SetPos(0);
  pg.SetAction( from );
  OnFileCopy.Enter(NULL, &pg);

  DWORD read = 1;
  while( true )  {
    if( ReadFile(in, bf, bf_sz, &read, NULL) == 0 )  {
      CloseHandle(in);
      CloseHandle(out);
      throw TFunctionFailedException(__OlxSourceInfo, "read failed");
    }
    if( read == 0 )  break;
    if( Break )  {
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
        throw TFunctionFailedException(__OlxSourceInfo, "write failed");
      }
      read -= written;
      pg.IncPos( written);
      OnFileCopy.Execute(NULL, &pg);
    }
  }
  pg.SetPos( fl );
  OnFileCopy.Exit(NULL, &pg);
  pg.SetPos(0);
  delete [] bf;
  CloseHandle(in);
  CloseHandle(out);
  return true;
}
//........................................................................
#else
// will fails on huge files
bool TFileTree::CopyFile(const olxstr& from, const olxstr& to) const {
  if( TEFile::Exists(to) )  { // delete the file, as CreateFile fails...
    if( !TEFile::DelFile(to) )  {
      throw TFunctionFailedException(__OlxSourceInfo,
        olxstr("Failed to delete: ").quote() << to);
    }
  }
  TEFile in(from, "rb"), out(to, "w+b");
  const int bf_sz = 16*1024*1024;
  char* bf = new char[bf_sz];
  size_t fl = in.Length();

  TOnProgress pg;
  pg.SetMax(fl);
  pg.SetPos(0);
  pg.SetAction(from);
  OnFileCopy.Enter(NULL, &pg);

  size_t full = fl/bf_sz,
    part = fl%bf_sz;
  for( size_t i=0; i < full; i++ )  {
    in.Read(bf, bf_sz);
    out.Write(bf, bf_sz);
    pg.SetPos( (i+1)*bf_sz);
    OnFileCopy.Execute(NULL, &pg);
    if( Break )  {
      delete [] bf;
      out.Delete();
      return false;
    }
  }
  in.Read(bf, part);
  out.Write(bf, part);
  pg.SetPos(fl);
  OnFileCopy.Exit(NULL, &pg);
  delete [] bf;
  return true;
}
#endif
//.............................................................................
bool TFileTree::CompareFiles(const olxstr& from, const olxstr& to) const {
  TEFile f1(from, "rb"), f2(to, "rb");
  if( f1.Length() != f2.Length() )  return false;
  const int bf_sz = 16*1024*1024;
  char* bf1 = new char[bf_sz];
  char* bf2 = new char[bf_sz];
  uint64_t fl = f1.Length();
  TOnProgress pg;
  pg.SetMax(fl);
  pg.SetPos(0);
  pg.SetAction(from);
  size_t full = static_cast<size_t>(fl/bf_sz),
    part = static_cast<size_t>(fl%bf_sz);
  for( size_t i=0; i < full; i++ )  {
    f1.Read(bf1, bf_sz);
    f2.Read(bf2, bf_sz);
    if( olxstr::o_memcmp(bf1, bf2, bf_sz) != 0 )  {
      delete [] bf1;
      delete [] bf2;
      return false;
    }
    OnFileCompare.Execute(NULL, &pg);
    if( Break )  {
      delete [] bf1;
      delete [] bf2;
      return false;
    }
  }
  f1.Read(bf1, part);
  f2.Read(bf1, part);
  bool res = olxstr::o_memcmp(bf1, bf2, part) != 0;
  pg.SetPos(fl);
  OnFileCompare.Execute(NULL, &pg);
  delete [] bf1;
  delete [] bf2;
  return res;
}
//.............................................................................
uint64_t TFileTree::CalcMergeSize(const DiffFolder& df)  {
  if( df.Src == NULL )
    return 0;
  if( df.Dest == NULL )
    return df.Src->CalcSize();
  uint64_t sz = 0;
  for( size_t i=0; i < df.SrcFiles.Count(); i++ )  {
    if( df.SrcFiles[i] != NULL )
      sz += df.SrcFiles[i]->GetSize();
  }
  for( size_t i=0; i < df.Folders.Count(); i++ )
    sz += CalcMergeSize(df.Folders[i]);
  return sz;
}
//.............................................................................
olxstr TFileTree::TFileTreeException::GetFullMessage() const {
  olxstr rv = GetNiceName();
  rv << ": " << GetError() << " at " << GetLocation() << NewLineSequence();
  rv << items.Text(NewLineSequence());
  return rv;
}
