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
  TTypeList< T > Rows;
  TStrList ColNames, RowNames;
public:
  TTTable()  {  }

  TTTable(int RowCnt, int ColCnt)  {  Resize(RowCnt, ColCnt);  }

  virtual ~TTTable()  {  Clear();  }

  void Clear()  {
    Rows.Clear();
    ColNames.Clear();
    RowNames.Clear();
  }

  template <class T1>
    void Assign( const TTTable<T1>& Table)  {
      Resize(Table.RowCount(), Table.ColCount());
      ColNames.Assign(Table.GetColNames());
      RowNames.Assign(Table.GetRowNames());
      for( int i=0; i < RowCount(); i++ )
        Rows[i].Assign( *Table.Row(i) );
    }

  inline const TStrList& GetColNames() const  {  return ColNames;  }
  inline const TStrList& GetRowNames() const  {  return RowNames;  }
  inline int RowCount()               const {  return Rows.Count();  }
  inline int ColCount()               const {  return ColNames.Count();  }
  inline olxstr& ColName(int index) const { return ColNames.String(index);  }
  inline olxstr& RowName(int index) const {  return RowNames.String(index);  }
  inline int ColIndex( const olxstr& N) const  {  return ColNames.IndexOf(N);  }
  inline int RowIndex( const olxstr& N) const  {  return RowNames.IndexOf(N);  }

  void Resize(int  RowCnt, int ColCnt)  {
    if( RowCnt < 0 || ColCnt < 0 )
      throw TInvalidArgumentException(__OlxSourceInfo, "size");
    if( RowCnt != RowCount() )  {
      if( RowCnt < RowCount() )  {
        Rows.Shrink(RowCnt);
        RowNames.SetCount(RowCnt);
      }
      else  {
        int rc = RowCount();  // row count changes
        for( int i=0; i < RowCnt-rc; i++ )  {
          Rows.AddNew();
          RowNames.Add(EmptyString);
        }
      }
    }
    if( ColCnt != ColNames.Count() )  {
      for( int i=0; i < RowCount(); i++ )  {
        if(  Rows[i].Count() < ColCnt )  {  // can happen if RowCnt changed
          int cc = Rows[i].Count();  // count changes
          for( int j=0; j < ColCnt - cc; j++ )
            Rows[i].Add(EmptyString);
        }
        else
          Rows[i].SetCount(ColCnt);
      }
      if(  ColNames.Count() < ColCnt )  {  // can happen if RowCnt changed
        int cc = ColNames.Count();  // count changes
        for( int j=0; j < ColCnt - cc; j++ )
          ColNames.Add(EmptyString);
      }
      else
        ColNames.SetCount(ColCnt);
    }
    for( int i=0; i < Rows.Count(); i++ )  {
      int c = Rows[i].Count();
      if(  c < ColCnt )  {  // can happen if RowCnt changed
        for( int j=0; j < ColCnt - c; j++ )
          Rows[i].Add(EmptyString);
      }
      else
        Rows[i].SetCount(ColCnt);
    }
  }

  void SetColCount(int NCC)  {  Resize(RowCount(), NCC);  }

  void SetRowCount(int NRC)  {  Resize(NRC, ColNames.Count() );  }
  
  void InsertCol(int index, const olxstr &Caption)  {
    for( int i=0; i < RowCount(); i++ )
      Rows[i].Insert(index, EmptyString);
    ColNames.Insert(index, Caption);
  }

  T& InsertRow(int index, const olxstr &Caption)  {
    T& SL = Rows.InsertNew(index);
    for( int i=0; i < ColNames.Count(); i++ )
      SL.Add(EmptyString);
    RowNames.Insert(index, Caption);
    return SL;
  }

  void AddCol(const olxstr &Caption)  {
    for( int i=0; i < RowCount(); i++ )
      Rows[i].Add( EmptyString );
    ColNames.Add(Caption);
  }

  T& AddRow(const olxstr &Caption)  {
    T& SL = Rows.AddNew();
    for( int i=0; i < ColNames.Count(); i++ )
      SL.Add(EmptyString);
    RowNames.Add(Caption);
    return SL;
  }

  void EmptyContent(bool EmptyCaptions)  {
    for( int i=0; i < Rows.Count(); i++ )  {
      for( int j=0; j < Rows[i].Count(); j++ )
        Rows[i].String(j) = EmptyString;
    }
    if( EmptyCaptions )  {
      for( int j=0; j < ColNames.Count(); j++ )
        ColNames.String(j) = EmptyString;
      for( int j=0; j < RowNames.Count(); j++ )
        RowNames.String(j) = EmptyString;
    }
  }

  void EmptyRow(int index, bool EmptyCaption)  {
    for( int j=0; j < Rows[index].Count(); j++ )
      Rows[index].String(j) = EmptyString;
    if( EmptyCaption )
      RowNames.String(index) = EmptyString;
  }

  void EmptyCol(int index, bool EmptyCaption)  {
    for( int i=0; i < RowCount(); i++ )
      Rows[i].String(index) = EmptyString;
    if( EmptyCaption )
      ColNames.String(index) = EmptyString;
  }

  void DelRow(int index)  {
    Rows.Delete(index);
    RowNames.Delete(index);
  }

  void DelCol( int index)  {
    ColNames.Delete(index);
    for( int i=0; i < RowCount(); i++ )
      Rows[i].Delete(index);
  }


  bool Find(const olxstr &What, int &row, int &col ) const  {
    for( int i=0; i < Rows.Count(); i++ )  {
      col = Rows[i].IndexOf(What);
      if(  col >= 0 )  {  row = i;  return true;  }
    }
    return false;
  }

  bool FindCol(const olxstr &What, int &col ) const  {
    for( int i=0; i < RowCount(); i++ )
      if( Rows[i].String(col) == What )  {  col = i;  return true;  }
    return false;
  }

  bool FindRow(const olxstr &What, int &row ) const  {
    return (Rows[row].IndexOf(What) > 0) ? true : false;
  }

  void CreateHTMLList(TStrList &L, const olxstr &Title,
                      bool colNames, bool rowNames,
                      bool Format=true) const  {
    olxstr Tmp;
    if( !Title.IsEmpty() )
      L.Add(olxstr("<p><b>") << Title << olxstr("</b></p>"));
    if( Format )  L.Add("<table border=\"1\" width = \"100%\" cellpadding=\"0\" cellspacing=\"0\" style=\"border-collapse: collapse\">");
    if( colNames )  {
      L.Add("<tr>");
      if( rowNames )  Tmp = "<td></td>";
      for( int i=0; i < ColNames.Count(); i++ )
        Tmp << "<td>" << ColNames.String(i) << "</td>";
      L.Add( (Tmp << "</tr>") );
    }
    for( int i=0; i < RowCount(); i++ )  {
      Tmp = "<tr>";
      if( rowNames )
        Tmp << "<td>" << RowNames.String(i) << "</td>";
      for( int j=0; j < ColNames.Count(); j++ )
        Tmp << "<td>" << Rows[i].String(j) << "</td>";
      L.Add( (Tmp << "</tr>") );
    }
    if( Format )  L.Add("</table>");
  }
  
  void CreateHTMLList(TStrList &L, const olxstr &Title,
                      const olxstr& footer,
                      bool colNames, bool rowNames,
                      const olxstr& titlePAttr,
                      const olxstr& footerPAttr,
                      const olxstr& colTitleRowAttr,
                      const olxstr& tabAttr,
                      const olxstr& rowAttr,
                      const TStrList& colAttr,
                      bool Format=true,
                      unsigned short colCount = 1) const  {

    olxstr Tmp;
    if( !Title.IsEmpty() )  L.Add(olxstr("<p ") << titlePAttr << '>' << Title << "</p>" );
    if( Format )    L.Add( olxstr("<table ") << tabAttr << '>' );
    if( colNames )  {
      L.Add( olxstr("<tr ") << colTitleRowAttr << '>');
      if( rowNames )  Tmp = "<td></td>";
      for( int i=0; i < colCount; i++ )  {
        for( int j=0; j < ColNames.Count(); j++ )  {
          if( colAttr.Count() < j )
            Tmp << "<td " << colAttr.String(j+1) << '>';
          else
            Tmp << "<td>";
          Tmp << ColNames.String(j) << "</td>";
        }
      }
      L.Add( (Tmp << "</tr>") );
    } //!ColNames
    int inc = (RowCount()%colCount)!=0 ? colCount : 0;
    for( int i=0; i < (RowCount()+inc)/colCount; i++ )  {
      Tmp = "<tr ";
      Tmp << rowAttr << '>';
      for( int j=0; j < colCount; j++ )  {
        int index = i+ j*(RowCount()+inc)/colCount;
        if( index >= RowCount() )  {
          if( rowNames )
            Tmp << "<td " << colAttr.String(0) << '>' << "&nbsp;" << "</td>";
          for( int k=0; k < ColNames.Count(); k++ )
            Tmp << "<td " << colAttr.String(k+1) << '>' << "&nbsp;" << "</td>";
          continue;
        }
        if( rowNames )
          Tmp << "<td " << colAttr.String(0) << '>' << RowNames.String(index) << "</td>";
        for( int k=0; k < ColNames.Count(); k++ )
          Tmp << "<td " << colAttr.String(k+1) << '>' << Rows[index].String(k) << "</td>";
      }
      L.Add( (Tmp << "</tr>") );
    }
    if( Format )  L.Add("</table>");
    if( !footer.IsEmpty() )  L.Add(olxstr("<p ") << footerPAttr << '>' << footer << "</p>" );
  }
  void CreateTXTList(TStrList &L, const olxstr &Title, bool ColNames, bool RowNames, const olxstr& Sep) const {
    TVector<int> rowV(ColCount()+1);
    olxstr Tmp;
    L.Add(Title);
    for( int i=0; i < RowCount(); i++ )
      if( RowName(i).Length() > rowV[0] )
        rowV[0] = RowName(i).Length();

    for( int i=0; i < ColCount(); i++ )  {
      for( int j=0; j < RowCount(); j++ )
        if( Row(j)->String(i).Length() > rowV[i+1] )
          rowV[i+1] = Row(j)->String(i).Length();
      if( ColName(i).Length() > rowV[i+1] )
        rowV[i+1] = ColName(i).Length();
    }

    rowV += Sep.Length();

    for( int i=0; i < ColCount(); i++ )  rowV[i+1] += rowV[i];

    if( ColNames )  {
      Tmp = EmptyString;
      if( RowNames )  Tmp.Format(rowV[0], true, ' ');
      Tmp << Sep;
      for( int i=0; i < ColCount(); i++ )  {
        Tmp << ColName(i);
        Tmp.Format(rowV[i+1], true, ' ');
        if( (i+1) < ColCount() )  Tmp << Sep;
      }
      L.Add(Tmp);
    }
    for( int i=0; i < RowCount(); i++ )  {
      if( RowNames )  {
        Tmp = RowName(i);
        Tmp.Format(rowV[0], true, ' ');
        Tmp << Sep;
      }
      else
        Tmp = EmptyString;

      for( int j=0; j < ColCount(); j++ )  {
        Tmp << Row(i)->String(j);
        Tmp.Format(rowV[j+1], true, ' ');
        if( (j+1) < ColCount() )  Tmp << Sep;
      }
      L.Add(Tmp);
    }
  }

  inline T& operator [] (int index)  {  return Rows[index];  }
  inline T* Row(int index)    const  {  return &Rows[index];  }

  // note that the void* passed to the functions
  // are of the TEStrListData type !! String = Row->Col(col), Data = Row
  template <class comparator>
    void SortRows(int col)  {
      TStrPObjList<olxstr,int> SL;

      for( int i=0; i < RowCount(); i++ )
        SL.Add(Rows[i].String(col), i);

      SL.QuickSort<comparator>();
      TIntList indexes( RowCount() );
      for( int i=0; i < RowCount(); i++ )
        indexes[i] = SL.Object(i);
      RowNames.Rearrange( indexes );
      Rows.Rearrange( indexes );
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
