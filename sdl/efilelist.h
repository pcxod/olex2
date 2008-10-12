#ifndef efilelistH
#define efilelistH
#include "ebase.h"
#include "typelist.h"
#include "etime.h"

BeginEsdlNamespace()
  class TFileListItem  {
  protected:
    unsigned short Attributes;
    time_t CreationTime, ModificationTime, LastAccessTime;
    long Size;
    olxstr Name;
  public:
    TFileListItem()  { 
      Size = 0;  
      CreationTime = ModificationTime = LastAccessTime = 0;   
      Attributes = 0;
    }
    TFileListItem( const TFileListItem& item)  {
      Name = item.Name;
      Size = item.Size;
      Attributes = item.Attributes;
      CreationTime = item.CreationTime;
      ModificationTime = item.ModificationTime;
      LastAccessTime = item.LastAccessTime;
    }
    DefPropP(time_t, CreationTime)
    DefPropP(time_t, ModificationTime)
    DefPropP(time_t, LastAccessTime)
    DefPropP(long, Size)
    DefPropP(unsigned short, Attributes)
    DefPropC(olxstr, Name)

    template <short field> class TFileListItemSorter  {
      static olxstr ExtractFileExt(const olxstr& fn)  {
        int ind = fn.LastIndexOf('.');
        if( ind >=0 )  return fn.SubStringFrom(ind+1);
        return EmptyString;
      }
    public:
      static int Compare( const TFileListItem& i1, const TFileListItem& i2 )  {
        if( field == 0 )  return i1.GetName().Compare(i2.GetName());
        if( field == 1 )  return ExtractFileExt(i1.GetName()).Compare(ExtractFileExt(i2.GetName()));
        if( field == 2 )  return TPrimitiveComparator::Compare<time_t>(i1.GetCreationTime(), i2.GetCreationTime());
        if( field == 3 )  return TPrimitiveComparator::Compare<time_t>(i1.GetModificationTime(), i2.GetModificationTime());
        if( field == 4 )  return TPrimitiveComparator::Compare<time_t>(i1.GetLastAccessTime(), i2.GetLastAccessTime());
        if( field == 5 )  return TPrimitiveComparator::Compare<long>(i1.GetSize(), i2.GetSize());
        return 0;
      }
    };

    static void SortListByName(TTypeList<TFileListItem>& list)  {
      TTypeList<TFileListItem>::QuickSorter.Sort< TFileListItemSorter<0> >( list );
    }
    static void SortListByExt(TTypeList<TFileListItem>& list)  {
      TTypeList<TFileListItem>::QuickSorter.Sort< TFileListItemSorter<1> >( list );
    }
    static void SortListByAge(TTypeList<TFileListItem>& list)  {
      TTypeList<TFileListItem>::QuickSorter.Sort< TFileListItemSorter<2> >( list );
    }
    static void SortListByModificationTime(TTypeList<TFileListItem>& list)  {
      TTypeList<TFileListItem>::QuickSorter.Sort< TFileListItemSorter<3> >( list );
    }
    static void SortListByLastAccessTime(TTypeList<TFileListItem>& list)  {
      TTypeList<TFileListItem>::QuickSorter.Sort< TFileListItemSorter<4> >( list );
    }
    static void SortListBySize(TTypeList<TFileListItem>& list)  {
      TTypeList<TFileListItem>::QuickSorter.Sort< TFileListItemSorter<5> >( list );
    }
  };

  typedef TTypeList<TFileListItem> TFileList;

EndEsdlNamespace()

#endif
