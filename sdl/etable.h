/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_etable_H
#define __olx_sdl_etable_H
#include "ebase.h"
#include "estrlist.h"
#include "typelist.h"
#include "evector.h"
BeginEsdlNamespace()

template <class T> class TTTable : public IOlxObject {
  TTypeList<T> Rows;
  TStrList ColNames, RowNames;
public:
  TTTable() {}
  TTTable(const TTTable& t)
    : Rows(t.Rows), ColNames(t.ColNames), RowNames(t.RowNames) {}
  TTTable(size_t RowCnt, size_t ColCnt) { Resize(RowCnt, ColCnt); }

  virtual ~TTTable() { Clear(); }

  void Clear() {
    Rows.Clear();
    ColNames.Clear();
    RowNames.Clear();
  }

  template <class T1> TTTable& Assign(const TTTable<T1>& Table) {
    Resize(Table.RowCount(), Table.ColCount());
    ColNames.Assign(Table.GetColNames());
    RowNames.Assign(Table.GetRowNames());
    for (size_t i = 0; i < RowCount(); i++)
      Rows[i].Assign(Table[i]);
    return *this;
  }
  TTTable& operator = (const TTTable& Table) { return Assign(Table); }
  template <class T1> TTTable& operator = (const TTTable<T1>& Table) {
    return Assign(Table);
  }

  const TStrList& GetColNames() const { return ColNames; }
  const TStrList& GetRowNames() const { return RowNames; }
  size_t RowCount() const { return Rows.Count(); }
  size_t ColCount() const { return ColNames.Count(); }
  olxstr& ColName(size_t index) const { return ColNames[index]; }
  olxstr& RowName(size_t index) const { return RowNames[index]; }
  template <typename Str> size_t ColIndex(const Str& N) const {
    return ColNames.IndexOf(N);
  }
  template <typename Str> size_t ColIndexi(const Str& N) const {
    return ColNames.IndexOfi(N);
  }
  template <typename Str> size_t RowIndex(const Str& N) const {
    return RowNames.IndexOf(N);
  }
  template <typename Str> size_t RowIndexi(const Str& N) const {
    return RowNames.IndexOfi(N);
  }

  void Resize(size_t RowCnt, size_t ColCnt) {
    if (RowCnt != Rows.Count()) {
      if (RowCnt < Rows.Count()) {
        Rows.Shrink(RowCnt);
        RowNames.SetCount(RowCnt);
      }
      else {
        Rows.SetCapacity(RowCnt);
        RowNames.SetCount(RowCnt);
        while (Rows.Count() < RowCnt)
          Rows.AddNew();
      }
    }
    if (ColCnt != ColNames.Count()) {
      ColNames.SetCount(ColCnt);
    }
    for (size_t i = 0; i < Rows.Count(); i++)
      Rows[i].SetCount(ColCnt);
  }

  void SetColCount(size_t NCC) { Resize(RowCount(), NCC); }
  void SetRowCount(size_t NRC) { Resize(NRC, ColNames.Count()); }
  /* sets capacity for rows */
  void SetRowCapacity(size_t row_cap) {
    Rows.SetCapacity(row_cap);
    RowNames.SetCapacity(row_cap);
  }
  void InsertCol(size_t index, const olxstr& Caption = EmptyString()) {
    for (size_t i = 0; i < RowCount(); i++)
      Rows[i].Insert(index);
    ColNames.Insert(index, Caption);
  }

  T& InsertRow(size_t index, const olxstr& Caption = EmptyString()) {
    T& SL = Rows.InsertNew(index);
    for (size_t i = 0; i < ColNames.Count(); i++)
      SL.Add();
    RowNames.Insert(index, Caption);
    return SL;
  }

  void AddCol(const olxstr& Caption = EmptyString()) {
    for (size_t i = 0; i < RowCount(); i++)
      Rows[i].Add();
    ColNames.Add(Caption);
  }

  T& AddRow(const olxstr& Caption = EmptyString()) {
    T& SL = Rows.AddNew();
    for (size_t i = 0; i < ColNames.Count(); i++)
      SL.Add();
    RowNames.Add(Caption);
    return SL;
  }

  void DelRow(size_t index) {
    Rows.Delete(index);
    RowNames.Delete(index);
  }

  void DelCol(size_t index) {
    ColNames.Delete(index);
    for (size_t i = 0; i < RowCount(); i++)
      Rows[i].Delete(index);
  }


  bool Find(const olxstr& What, size_t& row, size_t& col) const {
    for (size_t i = 0; i < Rows.Count(); i++) {
      col = Rows[i].IndexOf(What);
      if (col != InvalidIndex) {
        row = i;
        return true;
      }
    }
    return false;
  }
  // finds a row for the column value
  bool FindCol(const olxstr& What, size_t& col) const {
    for (size_t i = 0; i < Rows.Count(); i++)
      if (Rows[i][col] == What) {
        col = i;
        return true;
      }
    return false;
  }
  // finds a column in the row
  bool HasRow(const olxstr& What, size_t row) const {
    return Rows[row].IndexOf(What) != InvalidIndex;
  }

  TStrList CreateHTMLList(const olxstr& Title,
    bool colNames, bool rowNames,
    bool Format = true) const
  {
    TStrList l;
    return CreateHTMLList(l, Title, colNames, rowNames, Format);
  }
  TStrList& CreateHTMLList(TStrList& L, const olxstr& Title,
    bool colNames, bool rowNames,
    bool Format = true) const
  {
    olxstr Tmp;
    if (!Title.IsEmpty())
      L.Add(olxstr("<p><b>") << Title << olxstr("</b></p>"));
    if (Format)
      L.Add("<table border=\"1\" width = \"100%\" cellpadding=\"0\" "
        "cellspacing=\"0\" style=\"border-collapse: collapse\">");
    if (colNames) {
      L.Add("<tr>");
      if (rowNames)  Tmp = "<td></td>";
      for (size_t i = 0; i < ColNames.Count(); i++)
        Tmp << "<td>" << ColNames[i] << "</td>";
      L.Add((Tmp << "</tr>"));
    }
    for (size_t i = 0; i < Rows.Count(); i++) {
      Tmp = "<tr>";
      if (rowNames)
        Tmp << "<td>" << RowNames[i] << "</td>";
      for (size_t j = 0; j < ColNames.Count(); j++)
        Tmp << "<td>" << Rows[i][j] << "</td>";
      L.Add((Tmp << "</tr>"));
    }
    if (Format)  L.Add("</table>");
    return L;
  }

  const_strlist CreateHTMLList(const olxstr &Title,
    const olxstr& footer,
    bool colNames, bool rowNames,
    const olxstr& titlePAttr,
    const olxstr& footerPAttr,
    const olxstr& tabAttr,
    const olxstr& rowAttr,
    const TStrList& thAttr,
    const TStrList& clAttr,
    bool Format = true,
    unsigned short colCount = 1,
    const olxstr& colSepAttr = EmptyString()) const
  {
    TStrList L;
    return CreateHTMLList(L, Title, footer, colNames, rowNames, titlePAttr,
      footerPAttr, tabAttr, rowAttr, thAttr, clAttr, Format, colCount,
      colSepAttr);
  }
  TStrList& CreateHTMLList(TStrList &L, const olxstr &Title,
    const olxstr& footer,
    bool colNames, bool rowNames,
    const olxstr& titlePAttr,
    const olxstr& footerPAttr,
    const olxstr& tabAttr,
    const olxstr& rowAttr,
    const TStrList& thAttr,
    const TStrList& clAttr,
    bool Format = true,
    unsigned short colCount = 1,
    const olxstr& colSepAttr = EmptyString(),
    bool across = false // if colCount > 1 - horisontal or vertical
  ) const
  {
    if (Format) {
      L.Add("<table ") << tabAttr << '>';
      if (!Title.IsEmpty()) {
        L.Add("<caption ") << titlePAttr << '>' << Title << "</caption>";
      }
    }
    else if (!Title.IsEmpty()) {
      L.Add("<p ") << titlePAttr << '>' << Title << "</p>";
    }

    if (colNames) {
      olxstr Tmp = "<thead><tr>";
      if (rowNames) {
        Tmp << "<th>" << thAttr[0] << "</th>";
      }
      for (size_t i = 0; i < colCount; i++) {
        for (size_t j = 0; j < ColNames.Count(); j++) {
          Tmp << "<th " << thAttr[j + 1] << '>' << ColNames[j] << "</th>";
        }
        if ((i + 1) < colCount) {
          Tmp << "<td " << colSepAttr << ">&nbsp;</td>";
        }
      }
      L.Add(Tmp << "</tr></thead>");
    }
    size_t inc = (Rows.Count() % colCount) != 0 ? colCount : 0,
      l_l = (Rows.Count() + inc) / colCount;
    for (size_t i = 0; i < l_l; i++) {
      olxstr Tmp = "<tr ";
      Tmp << rowAttr << '>';
      for (size_t j = 0; j < colCount; j++) {
        size_t index = across ? i * colCount + j : i + j * l_l;
        if (index >= RowCount()) {
          if (rowNames) {
            Tmp << "<td " << clAttr[0] << ">&nbsp;</td>";
          }
          for (size_t k = 0; k < ColNames.Count(); k++) {
            Tmp << "<td " << clAttr[k + 1] << ">&nbsp;</td>";
          }
          continue;
        }
        if (rowNames) {
          Tmp << "<td " << clAttr[0] << '>' << RowNames[index] << "</td>";
        }
        for (size_t k = 0; k < ColNames.Count(); k++) {
          Tmp << "<td " << clAttr[k + 1] << '>' << Rows[index][k] << "</td>";
        }
        if ((j + 1) < colCount) {
          Tmp << "<td " << colSepAttr << ">&nbsp;</td>";
        }
      }
      L.Add(Tmp << "</tr>");
    }
    if (Format) {
      L.Add("</table>");
    }
    if (!footer.IsEmpty()) {
      L.Add("<p ") << footerPAttr << '>' << footer << "</p>";
    }
    return L;
  }

  const_strlist CreateTXTList(const olxstr &Title, bool colNames, bool rowNames,
    const olxstr& Sep) const
  {
    TStrList L;
    return CreateTXTList(L, Title, colNames, rowNames, Sep);
  }
  TStrList& CreateTXTList(TStrList &L, const olxstr &Title, bool colNames,
    bool rowNames, const olxstr& Sep) const
  {
    evecsz rowV(ColCount() + 1);
    L.Add(Title);
    for (size_t i = 0; i < Rows.Count(); i++)
      if (RowNames[i].Length() > rowV[0]) {
        rowV[0] = RowNames[i].Length();
      }

    for (size_t i = 0; i < ColNames.Count(); i++) {
      for (size_t j = 0; j < Rows.Count(); j++)
        if (Rows[j][i].Length() > rowV[i + 1]) {
          rowV[i + 1] = Rows[j][i].Length();
        }
      if (ColNames[i].Length() > rowV[i + 1]) {
        rowV[i + 1] = ColNames[i].Length();
      }
    }
    rowV += Sep.Length();

    for (size_t i = 0; i < ColNames.Count(); i++) {
      rowV[i + 1] += rowV[i];
    }

    if (colNames) {
      olxstr Tmp;
      if (rowNames) {
        Tmp.RightPadding(rowV[0], ' ');
        Tmp << Sep;
      }
      for (size_t i = 0; i < ColCount(); i++) {
        Tmp << ColNames[i];
        Tmp.RightPadding(rowV[i + 1], ' ');
        if ((i + 1) < ColNames.Count()) {
          Tmp << Sep;
        }
      }
      L.Add(Tmp);
    }
    for (size_t i = 0; i < RowCount(); i++) {
      olxstr Tmp;
      if (rowNames) {
        Tmp = RowNames[i];
        Tmp.RightPadding(rowV[0], ' ');
        Tmp << Sep;
      }
      for (size_t j = 0; j < ColCount(); j++) {
        Tmp << Rows[i][j];
        Tmp.RightPadding(rowV[j + 1], ' ');
        if ((j + 1) < ColCount()) {
          Tmp << Sep;
        }
      }
      L.Add(Tmp);
    }
    return L;
  }

  inline T& operator [] (size_t index) { return Rows[index]; }
  inline T const & operator [] (size_t index) const { return Rows[index]; }

  template <class Comparator> void SortRows(const Comparator& cmp) {
    QuickSorter::Sort(Rows, cmp, SyncSortListener::MakeSingle(RowNames));
  }
  void SwapRows(size_t r1, size_t r2) {
    Rows.Swap(r1, r2);
    RowNames.Swap(r1, r2);
  }
  // note that the void* passed to the functions
  // are of the TEStrListData type !! String = Row(row)->Col, Data = Row
  template <class comparator>
  void SortCols(int row) {
    throw TNotImplementedException(__OlxSourceInfo);
  }

};

  typedef TTTable<TStrList> TTable;
  typedef TTTable<TStringToList<olxstr,void*> > TETable;

EndEsdlNamespace()
#endif
