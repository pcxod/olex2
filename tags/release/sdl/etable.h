//----------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//
#ifndef etableH
#define etableH
//---------------------------------------------------------------------------
#include "ebase.h"
#include "estrlist.h"
#include "typelist.h"
#include "evector.h"

BeginEsdlNamespace()

//template argumets must be TTStrList or TTStringLists
template <class T>
class TTTable: public IEObject  {
  TTypeList<T> Rows;
  TStrList ColNames, RowNames;
public:
  TTTable()  {}
  TTTable(size_t RowCnt, size_t ColCnt)  {  Resize(RowCnt, ColCnt);  }

  virtual ~TTTable()  {  Clear();  }

  void Clear()  {
    Rows.Clear();
    ColNames.Clear();
    RowNames.Clear();
  }

  template <class T1> void Assign( const TTTable<T1>& Table)  {
    Resize(Table.RowCount(), Table.ColCount());
    ColNames.Assign(Table.GetColNames());
    RowNames.Assign(Table.GetRowNames());
    for( size_t i=0; i < RowCount(); i++ )
      Rows[i].Assign( Table[i] );
  }

  const TStrList& GetColNames() const  {  return ColNames;  }
  const TStrList& GetRowNames() const  {  return RowNames;  }
  size_t RowCount() const {  return Rows.Count();  }
  size_t ColCount() const {  return ColNames.Count();  }
  olxstr& ColName(size_t index) const { return ColNames[index];  }
  olxstr& RowName(size_t index) const {  return RowNames[index];  }
  size_t ColIndex(const olxstr& N) const  {  return ColNames.IndexOf(N);  }
  size_t RowIndex(const olxstr& N) const  {  return RowNames.IndexOf(N);  }

  void Resize(size_t  RowCnt, size_t ColCnt)  {
    if( RowCnt != Rows.Count() )  {
      if( RowCnt < Rows.Count() )  {
        Rows.Shrink(RowCnt);
        RowNames.SetCount(RowCnt);
      }
      else  {
        while( Rows.Count() < RowCnt )  {
          Rows.AddNew();
          RowNames.Add(EmptyString);
        }
      }
    }
    if( ColCnt != ColNames.Count() )  {
      if( ColNames.Count() < ColCnt )  {  // can happen if RowCnt changed
        while( ColNames.Count() < ColCnt )
          ColNames.Add(EmptyString);
      }
      else
        ColNames.SetCount(ColCnt);
    }
    for( size_t i=0; i < Rows.Count(); i++ )  {
      if( Rows[i].Count() < ColCnt )  { 
        while( Rows[i].Count() < ColCnt )
          Rows[i].Add(EmptyString);
      }
      else
        Rows[i].SetCount(ColCnt);
    }
  }

  void SetColCount(size_t NCC)  {  Resize(RowCount(), NCC);  }
  void SetRowCount(size_t NRC)  {  Resize(NRC, ColNames.Count() );  }
  void InsertCol(size_t index, const olxstr &Caption)  {
    for( size_t i=0; i < RowCount(); i++ )
      Rows[i].Insert(index, EmptyString);
    ColNames.Insert(index, Caption);
  }

  T& InsertRow(size_t index, const olxstr &Caption)  {
    T& SL = Rows.InsertNew(index);
    for( size_t i=0; i < ColNames.Count(); i++ )
      SL.Add(EmptyString);
    RowNames.Insert(index, Caption);
    return SL;
  }

  void AddCol(const olxstr &Caption)  {
    for( size_t i=0; i < RowCount(); i++ )
      Rows[i].Add( EmptyString );
    ColNames.Add(Caption);
  }

  T& AddRow(const olxstr &Caption)  {
    T& SL = Rows.AddNew();
    for( size_t i=0; i < ColNames.Count(); i++ )
      SL.Add(EmptyString);
    RowNames.Add(Caption);
    return SL;
  }

  void EmptyContent(bool EmptyCaptions)  {
    for( size_t i=0; i < Rows.Count(); i++ )  {
      for( size_t j=0; j < Rows[i].Count(); j++ )
        Rows[i][j] = EmptyString;
    }
    if( EmptyCaptions )  {
      for( size_t j=0; j < ColNames.Count(); j++ )
        ColNames[j] = EmptyString;
      for( size_t j=0; j < RowNames.Count(); j++ )
        RowNames[j] = EmptyString;
    }
  }

  void EmptyRow(size_t index, bool EmptyCaption)  {
    for( size_t j=0; j < Rows[index].Count(); j++ )
      Rows[index][j] = EmptyString;
    if( EmptyCaption )
      RowNames[index] = EmptyString;
  }

  void EmptyCol(size_t index, bool EmptyCaption)  {
    for( size_t i=0; i < RowCount(); i++ )
      Rows[i][index] = EmptyString;
    if( EmptyCaption )
      ColNames[index] = EmptyString;
  }

  void DelRow(size_t index)  {
    Rows.Delete(index);
    RowNames.Delete(index);
  }

  void DelCol(size_t index)  {
    ColNames.Delete(index);
    for( size_t i=0; i < RowCount(); i++ )
      Rows[i].Delete(index);
  }


  bool Find(const olxstr& What, size_t& row, size_t& col) const  {
    for( size_t i=0; i < Rows.Count(); i++ )  {
      col = Rows[i].IndexOf(What);
      if(  col != InvalidIndex )  {  
        row = i;  
        return true;  
      }
    }
    return false;
  }
  // finds a row for the column value
  bool FindCol(const olxstr& What, size_t& col) const  {
    for( size_t i=0; i < Rows.Count(); i++ )
      if( Rows[i][col] == What )  { 
        col = i;  
        return true;  
      }
    return false;
  }
  // finds a column in the row
  bool HasRow(const olxstr& What, size_t row ) const  {
    return (Rows[row].IndexOf(What) == InvalidIndex) ? false : true;
  }

  void CreateHTMLList(TStrList& L, const olxstr& Title,
                      bool colNames, bool rowNames,
                      bool Format=true) const  {
    olxstr Tmp;
    if( !Title.IsEmpty() )
      L.Add(olxstr("<p><b>") << Title << olxstr("</b></p>"));
    if( Format )  L.Add("<table border=\"1\" width = \"100%\" cellpadding=\"0\" cellspacing=\"0\" style=\"border-collapse: collapse\">");
    if( colNames )  {
      L.Add("<tr>");
      if( rowNames )  Tmp = "<td></td>";
      for( size_t i=0; i < ColNames.Count(); i++ )
        Tmp << "<td>" << ColNames[i] << "</td>";
      L.Add( (Tmp << "</tr>") );
    }
    for( size_t i=0; i < Rows.Count(); i++ )  {
      Tmp = "<tr>";
      if( rowNames )
        Tmp << "<td>" << RowNames[i] << "</td>";
      for( size_t j=0; j < ColNames.Count(); j++ )
        Tmp << "<td>" << Rows[i][j] << "</td>";
      L.Add( (Tmp << "</tr>") );
    }
    if( Format )  L.Add("</table>");
  }
  
  void CreateHTMLList(TStrList &L, const olxstr &Title,
                      const olxstr& footer,
                      bool colNames, bool rowNames,
                      const olxstr& titlePAttr,
                      const olxstr& footerPAttr,
                      const olxstr& tabAttr,
                      const olxstr& rowAttr,
                      const TStrList& thAttr,
                      const TStrList& clAttr,
                      bool Format=true,
                      unsigned short colCount = 1) const  {

    olxstr Tmp;
    if( Format )  {
      L.Add( olxstr("<table ") << tabAttr << '>' );
      if( !Title.IsEmpty() )  
        L.Add("<caption ") << titlePAttr << '>' << Title << "</caption>";
    }
    else if( !Title.IsEmpty() )  
      L.Add("<p ") << titlePAttr << '>' << Title << "</p>";

    if( colNames )  {
      if( rowNames )  Tmp << "<th>" << thAttr[0] << "</th>";
      for( size_t i=0; i < colCount; i++ )  {
        for( size_t j=0; j < ColNames.Count(); j++ )
          Tmp << "<th " << thAttr[j+1] << '>' << ColNames[j] << "</th>";
      }
      L.Add( (Tmp << "</tr>") );
    } //!ColNames
    size_t inc = (Rows.Count()%colCount)!=0 ? colCount : 0,
           l_l = (Rows.Count()+inc)/colCount;
    for( size_t i=0; i < l_l; i++ )  {
      Tmp = "<tr ";
      Tmp << rowAttr << '>';
      for( size_t j=0; j < colCount; j++ )  {
        size_t index = i+ j*(Rows.Count()+inc)/colCount;
        if( index >= RowCount() )  {
          if( rowNames )
            Tmp << "<td " << clAttr[0] << ">&nbsp;</td>";
          for( size_t k=0; k < ColNames.Count(); k++ )
            Tmp << "<td " << clAttr[k+1] << ">&nbsp;</td>";
          continue;
        }
        if( rowNames )
          Tmp << "<td " << clAttr[0] << '>' << RowNames[index] << "</td>";
        for( size_t k=0; k < ColNames.Count(); k++ )
          Tmp << "<td " << clAttr[k+1] << '>' << Rows[index][k] << "</td>";
      }
      L.Add( (Tmp << "</tr>") );
    }
    if( Format )  L.Add("</table>");
    if( !footer.IsEmpty() )  L.Add("<p ") << footerPAttr << '>' << footer << "</p>";
  }
  void CreateTXTList(TStrList &L, const olxstr &Title, bool colNames, bool rowNames, const olxstr& Sep) const {
    evecsz rowV(ColCount()+1);
    olxstr Tmp;
    L.Add(Title);
    for( size_t i=0; i < Rows.Count(); i++ )
      if( RowNames[i].Length() > rowV[0] )
        rowV[0] = RowNames[i].Length();

    for( size_t i=0; i < ColNames.Count(); i++ )  {
      for( size_t j=0; j < Rows.Count(); j++ )
        if( Rows[j][i].Length() > rowV[i+1] )
          rowV[i+1] = Rows[j][i].Length();
      if( ColNames[i].Length() > rowV[i+1] )
        rowV[i+1] = ColNames[i].Length();
    }

    rowV += Sep.Length();

    for( size_t i=0; i < ColNames.Count(); i++ )  
      rowV[i+1] += rowV[i];

    if( colNames )  {
      Tmp = EmptyString;
      if( rowNames )  
        Tmp.Format(rowV[0], true, ' ');
      Tmp << Sep;
      for( size_t i=0; i < ColCount(); i++ )  {
        Tmp << ColNames[i];
        Tmp.Format(rowV[i+1], true, ' ');
        if( (i+1) < ColNames.Count() )  Tmp << Sep;
      }
      L.Add(Tmp);
    }
    for( size_t i=0; i < RowCount(); i++ )  {
      if( rowNames )  {
        Tmp = RowNames[i];
        Tmp.Format(rowV[0], true, ' ');
        Tmp << Sep;
      }
      else
        Tmp = EmptyString;

      for( size_t j=0; j < ColCount(); j++ )  {
        Tmp << Rows[i][j];
        Tmp.Format(rowV[j+1], true, ' ');
        if( (j+1) < ColCount() )  Tmp << Sep;
      }
      L.Add(Tmp);
    }
  }

  inline T& operator [] (size_t index)  {  return Rows[index];  }
  inline T const & operator [] (size_t index) const {  return Rows[index];  }

  struct TableSort  {
    const T& data;
    size_t index;
    TableSort(const T& _data, size_t i) : data(_data), index(i)  {}
  };
  template <class comparator> void SortRows()  {
    TTypeList<TableSort> sl;
    for( size_t i=0; i < Rows.Count(); i++ )
      sl.AddNew(Rows[i], i);
    sl.QuickSorter.SortSF(sl, comparator::Compare);
    TSizeList indexes(Rows.Count());
    for( size_t i=0; i < Rows.Count(); i++ )
      indexes[i] = sl[i].index;
    RowNames.Rearrange(indexes);
    Rows.Rearrange(indexes);
  }
  void SwapRows(size_t r1, size_t r2)  {
    Rows.Swap(r1, r2);
    RowNames.Swap(r1, r2);
  }
  // note that the void* passed to the functions
  // are of the TEStrListData type !! String = Row(row)->Col, Data = Row
  template <class comparator>
    void SortCols(int row)  {
      throw TNotImplementedException(__OlxSourceInfo);
    }

};

  typedef TTTable< TStrList > TTable;
  typedef TTTable< TStrPObjList<olxstr,void*> > TETable;

EndEsdlNamespace()
#endif
