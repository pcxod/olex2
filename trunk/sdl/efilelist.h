/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_efilelist_H
#define __olx_sdl_efilelist_H
#include "ebase.h"
#include "typelist.h"
#include "etime.h"
#include "emath.h"

BeginEsdlNamespace()
  class TFileListItem  {
  protected:
    unsigned short Attributes;
    uint64_t CreationTime, ModificationTime, LastAccessTime;
    uint64_t Size;
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
    DefPropP(uint64_t, CreationTime)
    DefPropP(uint64_t, ModificationTime)
    DefPropP(uint64_t, LastAccessTime)
    DefPropP(uint64_t, Size)
    DefPropP(unsigned short, Attributes)
    DefPropC(olxstr, Name)

    template <short field> class TFileListItemSorter  {
      static olxstr ExtractFileExt(const olxstr& fn)  {
        size_t ind = fn.LastIndexOf('.');
        return (ind != InvalidIndex) ?  fn.SubStringFrom(ind+1) : EmptyString();
      }
    public:
      template <class item_a_t, class item_b_t>
      static int Compare(const item_a_t &i1_, const item_b_t &i2_) {
        const TFileListItem &i1 = olx_ref::get(i1_),
          &i2 = olx_ref::get(i2_);
        if( field == 0 )
          return i1.GetName().Compare(i2.GetName());
        if( field == 1 ) {
          return ExtractFileExt(i1.GetName()).Compare(
            ExtractFileExt(i2.GetName()));
        }
        if( field == 2 )
          return olx_cmp(i1.GetCreationTime(), i2.GetCreationTime());
        if( field == 3 )
          return olx_cmp(i1.GetModificationTime(), i2.GetModificationTime());
        if( field == 4 )
          return olx_cmp(i1.GetLastAccessTime(), i2.GetLastAccessTime());
        if( field == 5 )
          return olx_cmp(i1.GetSize(), i2.GetSize());
        return 0;
      }
    };

    static void SortListByName(TTypeList<TFileListItem>& list)  {
      QuickSorter::Sort(list, TFileListItemSorter<0>());
    }
    static void SortListByExt(TTypeList<TFileListItem>& list)  {
      QuickSorter::Sort(list, TFileListItemSorter<1>());
    }
    static void SortListByAge(TTypeList<TFileListItem>& list)  {
      QuickSorter::Sort(list, TFileListItemSorter<2>());
    }
    static void SortListByModificationTime(TTypeList<TFileListItem>& list)  {
      QuickSorter::Sort(list, TFileListItemSorter<3>());
    }
    static void SortListByLastAccessTime(TTypeList<TFileListItem>& list)  {
      QuickSorter::Sort(list, TFileListItemSorter<4>());
    }
    static void SortListBySize(TTypeList<TFileListItem>& list)  {
      QuickSorter::Sort(list, TFileListItemSorter<5>());
    }
  };

  typedef TTypeList<TFileListItem> TFileList;

EndEsdlNamespace()
#endif
