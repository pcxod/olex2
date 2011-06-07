#ifndef __olx_xlib_lcells_H
#define __olx_xlib_lcells_H
#include "xbase.h"
#include "cifdp.h"
#include "unitcell.h"
#include "cell_reduction.h"
#include "library.h"
#include "filetree.h"

BeginXlibNamespace()
namespace lcells {
  struct CellInfo {
    double volume, niggle_volume;
    evecd cell;
    short lattice;
    CellInfo() : cell(6), volume(0), niggle_volume(0), lattice(1)  {}
    IDataOutputStream &ToStream(IDataOutputStream &out) const {
      out << volume << niggle_volume;
      cell.ToStream(out);
      return out;
    }
    IDataInputStream& FromStream(IDataInputStream &in)  {
      in >> volume;
      in >> niggle_volume;
      cell.FromStream(in);
      return in;
    }
    struct VolumeComparator {
      static int Compare(const CellInfo &a, const CellInfo &b)  {
        return olx_cmp(a.volume, b.volume);
      }
      static int Compare(const CellInfo *a, const CellInfo *b)  {
        return olx_cmp(a->volume, b->volume);
      }
    };
    struct ReducedVolumeComparator {
      static int Compare(const CellInfo &a, const CellInfo &b)  {
        return olx_cmp(a.niggle_volume, b.niggle_volume);
      }
      static int Compare(const CellInfo *a, const CellInfo *b)  {
        return olx_cmp(a->niggle_volume, b->niggle_volume);
      }
    };
  };

  struct CellReader {
    static olx_object_ptr<TTypeList<CellInfo> > read(const olxstr &fn);
    static olxstr GetCifParamAsString(const cif_dp::CifBlock &block, const olxstr &Param);
    static int ExtractLattFromSymmetry(const cif_dp::CifBlock &block);
    static double ToDouble(const olxstr &str);
  };

  struct Index {
    struct Entry {
      olxstr name;
      uint64_t modified;
      Entry *parent;
      Entry(Entry *parent_=NULL) : parent(parent_), modified(~0)  {}
      Entry(Entry *parent_, const olxstr &name_, uint64_t modified_)
        : parent(parent_), name(name_), modified(modified_)  {}
      IDataOutputStream &ToStream(IDataOutputStream &out) const {
        return (out << name << modified);
      }
      IDataInputStream& FromStream(IDataInputStream &in)  {
        in >> name;
        in >> modified;
        return in;
      }
      struct NameComparator {
        static int Compare(const Entry &a, const Entry &b) {
          return olxstrComparator<false>::Compare(a.name, b.name);
        }
        static int Compare(const Entry *a, const Entry *b) {
          return olxstrComparator<false>::Compare(a->name, b->name);
        }
        static int Compare(const Entry &a, const olxstr &b) {
          return olxstrComparator<false>::Compare(a.name, b);
        }
      };
      olxstr FullName() const {
        olxstr_buf res = name;
        Entry *p = parent;
        olxstr ss = '.';
        while( p != NULL )  {
          res << ss << p->name;
          p = p->parent;
        }
        return olxstr::FromExternal(
          res.ReverseRead(olx_malloc<olxch>(res.Length()+1)), res.Length());
      }
    };
    struct ResultEntry : public CellInfo {
      ResultEntry(const olxstr &_file_name, const CellInfo &c)
        : file_name(_file_name), CellInfo(c)
      {}
      int Compare(const ResultEntry &e) const {  return file_name.Compare(e.file_name);  }
      olxstr file_name;
    };
    struct FolderEntry;
    struct FileEntry : public Entry {
      TArrayList<CellInfo> cells;
      FileEntry(FolderEntry &p) : Entry(&p)  {}
      FileEntry(FolderEntry &p, const olxstr &name, uint64_t modified)
        : Entry(&p, name, modified)  {}
      IDataOutputStream &ToStream(IDataOutputStream &out) const {
        out << (uint32_t)cells.Count();
        for( size_t i=0; i < cells.Count(); i++ )
          cells[i].ToStream(out);
        return Entry::ToStream(out);
      }
      FileEntry &FromStream(IDataInputStream &in)  {
        uint32_t sz;
        in >> sz;
        cells.SetCount(sz);
        for( uint32_t i=0; i < sz; i++ )
          cells[i].FromStream(in);
        Entry::FromStream(in);
        return *this;
      }
      void Expand(TTypeList<ResultEntry> &all) const {
        for( size_t i=0; i < cells.Count(); i++ )
          all.Add(new ResultEntry(FullName(), cells[i]));
      }
    };
    struct FolderEntry : public Entry {
      TTypeList<FileEntry> entries;
      //SortedTypeList<FileEntry, Entry::NameComparator> entries;
      TTypeList<FolderEntry> folders;
      FolderEntry(FolderEntry *parent=NULL) : Entry(parent)  {}
      FolderEntry(FolderEntry *parent, const olxstr &name, uint64_t modified)
        : Entry(parent, name, modified)  {}
      void Init(const TFileTree::Folder &folder);
      size_t Update(const TFileTree::Folder &folder);
      IDataOutputStream &ToStream(IDataOutputStream &out) const {
        out << (uint32_t)entries.Count();
        for( size_t i=0; i < entries.Count(); i++ )
          entries[i].ToStream(out);
        out << (uint32_t)folders.Count();
        for( size_t i=0; i < folders.Count(); i++ )
          folders[i].ToStream(out);
        return Entry::ToStream(out);
      }
      FolderEntry& FromStream(IDataInputStream &in)  {
        entries.Clear();
        folders.Clear();
        uint32_t sz;
        in >> sz;
        entries.SetCapacity(sz);
        for( uint32_t i=0; i < sz; i++ )
          entries.Add((new FileEntry(*this))->FromStream(in));
        in >> sz;
        folders.SetCapacity(sz);
        for( uint32_t i=0; i < sz; i++ )
          folders.Add(new FolderEntry(this)).FromStream(in);
        Entry::FromStream(in);
        return *this;
      }
      size_t TotalCount() const {
        size_t cnt = entries.Count();
        for( size_t i=0; i < folders.Count(); i++ )
          cnt += folders[i].TotalCount();
        return cnt;
      }
      void Expand(TTypeList<ResultEntry> &all) const {
        for( size_t i=0; i < entries.Count(); i++ )
          entries[i].Expand(all);
        for( size_t i=0; i < folders.Count(); i++ )
          folders[i].Expand(all);
      }
    };
    FolderEntry root;
    uint64_t LastUpdated;
    Index();
    static SortedObjectList<olxstr, olxstrComparator<false> > masks;
    static char file_sig[];
    static bool ConsiderFile(const olxstr &file_name) {
      return masks.IndexOf(
        TEFile::ExtractFileExt(file_name).ToLowerCase()) != InvalidIndex;
    }
    void Create(const olxstr &folder, const olxstr& index_name);
    void PrintInfo() const;
    static void PrintResults(const TTypeList<ResultEntry> & results);
    static void Update(const olxstr& index_name);
    static olxstr DefaultIndex()  {  return TBasicApp::GetSharedDir() + "lcells.ind";  }
    void SaveToFile(const olxstr &file_name) const;
    Index &LoadFromFile(const olxstr &file_name);
    TTypeList<ResultEntry> Search(const CellInfo &cell, double vol_diff,
      bool filter_by_dimensions) const;
    static TTypeList<ResultEntry> Search(const olxstr &index_name,
      const TStrObjList &Params, double vol_diff);
    static void Search(TStrObjList &Params, const TParamList &Options, TMacroError &E);
    static void Search(const TStrObjList &Params, TMacroError &E);
    static void Update(const TStrObjList &Params, TMacroError &E);
    static TLibrary* ExportLibrary(const olxstr &name=EmptyString());
  };
};
EndXlibNamespace()
#endif
