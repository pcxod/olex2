#ifndef __olx_file_tree_H
#define __olx_file_tree_H

#include "efile.h"
BeginEsdlNamespace()
class TFileTree  {
public:
  struct Folder  {
    TFileList* Files;
    TTypeList<Folder> Folders;
    Folder* Parent;
    olxstr FullPath, Name;
    Folder(const olxstr& fullPath, Folder* parent = NULL) : 
    Parent(parent), Files(NULL)  {
      FullPath = TEFile::OSPath(fullPath);
      TEFile::AddTrailingBackslashI(FullPath);
      int ind = FullPath.LastIndexOf(TEFile::GetPathDelimeter(), FullPath.Length()-1);
      if( ind != -1 )
        Name = FullPath.SubStringFrom(ind+1, 1);
    }
    const olxstr& GetName() const {  return Name;  }
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

    size_t CalcSize()  {
      if( Files == NULL )  return 0;
      size_t res = 0;
      for( int i=0; i < Files->Count(); i++ )
        res += (*Files)[i].GetSize();
      for( int i=0; i < Folders.Count(); i++ )
        res += Folders[i].CalcSize();
      return res;
    }
    void Expand()  {
      if( Files != NULL )  
        Files->Clear();
      else
        Files = new TFileList;
      TEFile::ListDirEx(FullPath, *Files, "*", sefAll^sefRelDir);
      for( int i=0; i < Files->Count(); i++ )  {
        if( ((*Files)[i].GetAttributes() & sefDir) == sefDir )  {
          Folders.AddNew( FullPath + (*Files)[i].GetName(), this).Expand();
          Files->NullItem(i);
        }
      }
      Files->Pack();
      Files->QuickSorter.SortSF(*Files, &CompareFiles);
      Folders.QuickSorter.SortSF(Folders, &CompareFolders);
    }
    void Compare(const Folder& f, TStrList& out) const {
      if( Files != NULL && f.Files != NULL )  {
        for( int i=0; i < Files->Count(); i++ )  {
          int ind = FindSortedIndexOf( *f.Files, (*Files)[i].GetName() );
          if( ind == -1 )
            out.Add("New file: ") << FullPath << (*Files)[i].GetName();
          else
            if((*Files)[i].GetModificationTime() != (*f.Files)[ind].GetModificationTime() )
              out.Add("Changed file: ") << FullPath << (*Files)[i].GetName();
        }
      }
      for( int i=0; i < Folders.Count(); i++ )  {
        int ind = FindSortedIndexOf( f.Folders, Folders[i].Name );
        if( ind == -1 )
          out.Add("New folder: ") << FullPath << Folders[i].Name;
      }
    }
    void Synchronise(const Folder& f, TStrList& out) const {
      if( Files != NULL && f.Files != NULL )  {
        for( int i=0; i < Files->Count(); i++ )  {
          int ind = FindSortedIndexOf( *f.Files, (*Files)[i].GetName() );
          if( ind == -1 )
            out.Add("New file: ") << (*Files)[i].GetName();
          else
            if((*Files)[i].GetModificationTime() != (*f.Files)[ind].GetModificationTime() )
              out.Add("Changed file: ") << (*Files)[i].GetName();
        }
      }
      for( int i=0; i < Folders.Count(); i++ )  {
        int ind = FindSortedIndexOf( f.Folders, Folders[i].Name );
        if( ind == -1 )
          out.Add("New folder: ") << Folders[i].Name;
      }
    }
  };
///////////////////////////////////////////////////////////////////////////////////////
public:
  Folder Root;
  TFileTree(const olxstr& root) : Root(root)  {
  }
};


EndEsdlNamespace()
#endif

