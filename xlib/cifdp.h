/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xl_cifdp_H
#define __olx_xl_cifdp_H
#include "estrlist.h"
#include "etable.h"
#include "edict.h"

namespace cif_dp {
  class ParsingException : public TBasicException  {
    size_t LineNumber;
  public:
    ParsingException(const ParsingException& e)
      : TBasicException(e), LineNumber(e.LineNumber)  {}
    ParsingException(const olxstr& location, const olxstr& msg, size_t lineNumber)
      : TBasicException(location, olxstr("Failed to parse CIF because ") <<
          msg << " at CIF#" << lineNumber),
        LineNumber(lineNumber)
    {}
    ParsingException(const olxstr& location, const TExceptionBase& cause,
      const olxstr& msg=EmptyString())
      : TBasicException(location, cause, msg )  {}
    virtual const char* GetNiceName() const {  return "CIF reading";  }
    virtual IEObject* Replicate() const {
      return new ParsingException(*this);
    }
  };
  struct ICifEntry  {
    ICifEntry()  {}
    virtual ~ICifEntry()  {}
    virtual void ToStrings(TStrList& list) const = 0;
    virtual void Format()  {}
    virtual bool HasName() const {  return false;  }
    virtual bool IsSaveable() const {  return true;  }
    // if the returned valus is InvalidIndex - the object is not comparable
    virtual size_t GetCmpHash() const {  return InvalidIndex;  }
    virtual const olxstr& GetName() const {
      throw TNotImplementedException(__OlxSourceInfo);
    }
    virtual void SetName(const olxstr&)  {
      throw TNotImplementedException(__OlxSourceInfo);
    }
    virtual ICifEntry* Replicate() const = 0;
    virtual olxstr GetStringValue() const = 0;
  };
  // string(s) value accessor interface
  struct IStringCifEntry : public ICifEntry {
    IStringCifEntry()  {}
    virtual ~IStringCifEntry()  {}
    virtual size_t Count() const {  return 0;  }
    virtual const olxstr& operator [] (size_t) const {
      throw TNotImplementedException(__OlxSourceInfo);
    }
    virtual bool HasComment() const {  return false;  }
    virtual const olxstr& GetComment() const {
      throw TNotImplementedException(__OlxSourceInfo);
    }
  };
  struct cetComment : public IStringCifEntry {
    olxstr comment;
    cetComment(const cetComment& v) : comment(v.comment)  {}
    cetComment(const olxstr& _value) : comment(_value)  {}
    virtual void ToStrings(TStrList& list) const {
      if( !comment.IsEmpty() )
        list.Add('#') << comment;
    }
    virtual bool HasComment() const {  return true;  }
    virtual const olxstr& GetComment() const {  return comment;  }
    virtual ICifEntry* Replicate() const {  return new cetComment(comment);  }
    virtual olxstr GetStringValue() const {  return comment;  }
  };
  struct cetString : public IStringCifEntry  {
    olxstr value;
    static const olxstr empty_value;
    bool quoted;
    cetString(const cetString& v) : value(v.value), quoted(v.quoted)  {}
    cetString(const olxstr& _val);
    virtual void ToStrings(TStrList& list) const;
    virtual size_t Count() const {  return 1;  }
    virtual const olxstr& operator [] (size_t) const {  return value;  }
    virtual ICifEntry* Replicate() const {  return new cetString(*this);  }
    virtual olxstr GetStringValue() const {  return value;  }
  };
  struct cetNamedString : public cetString {
    olxstr name;
    cetNamedString(const cetNamedString& v) : cetString(v), name(v.name)  {}
    cetNamedString(const olxstr& _name, const olxstr& _val)
      : cetString(_val), name(_name)  {}
    virtual void ToStrings(TStrList& list) const;
    virtual bool HasName() const {  return true;  }
    virtual const olxstr& GetName() const {  return name;  }
    virtual void SetName(const olxstr& _name)  {  name = _name;  }
    virtual ICifEntry* Replicate() const {  return new cetNamedString(*this);  }
  };
  struct cetCommentedString : public cetNamedString  {
    olxstr comment;
    cetCommentedString(const cetCommentedString& v)
      : cetNamedString(v), comment(v.comment)  {}
    cetCommentedString(const olxstr& _name, const olxstr& _val,
      const olxstr& _comment)
      : cetNamedString(_name, _val), comment(_comment)  {}
    virtual void ToStrings(TStrList& list) const {
      cetNamedString::ToStrings(list);
      list.GetLastString() << " #" << comment;
    }
    virtual bool HasComment() const {  return true;  }
    virtual const olxstr& GetComment() const {  return comment;  }
    virtual ICifEntry* Replicate() const {
      return new cetCommentedString(*this);
    }
  };
  struct cetStringList : public IStringCifEntry {
    TStrList lines;
    cetStringList()  {}
    cetStringList(const cetStringList& v) : lines(v.lines)  {}
    virtual void ToStrings(TStrList& list) const {
      list.Add(';');
      list.AddList(lines);
      list.Add(';');
    }
    virtual void Format()  {
      if( lines.Count() > 1 )
        lines.TrimWhiteCharStrings();
    }
    virtual size_t Count() const {  return lines.Count();  }
    virtual const olxstr& operator [] (size_t i) const {  return lines[i];  }
    virtual ICifEntry* Replicate() const {  return new cetStringList(*this);  }
    virtual olxstr GetStringValue() const {
      throw TNotImplementedException(__OlxSourceInfo);
    }
  };
  struct cetNamedStringList : public cetStringList  {
    olxstr name;
    cetNamedStringList(const cetNamedStringList& v)
      : cetStringList(v), name(v.name)  {}
    cetNamedStringList(const olxstr& _name) : name(_name)  {}
    virtual void ToStrings(TStrList& list) const {
      list.Add(name);
      cetStringList::ToStrings(list);
    }
    virtual bool HasName() const {  return true;  }
    virtual const olxstr& GetName() const {  return name;  }
    virtual void SetName(const olxstr& _name)  {  name = _name;  }
    virtual ICifEntry* Replicate() const {
      return new cetNamedStringList(*this);
    }
  };
  struct cetCommentedNamedStringList : public cetNamedStringList {
   olxstr comment;
    cetCommentedNamedStringList(const cetCommentedNamedStringList& v)
      : cetNamedStringList(v), comment(v.comment)  {}
    cetCommentedNamedStringList(const olxstr& _name, const olxstr& _comment)
      : cetNamedStringList(_name), comment(_comment)  {}
    virtual void ToStrings(TStrList& list) const {
      list.Add(name) << " #" << comment;
      cetStringList::ToStrings(list);
    }
    virtual bool HasComment() const {  return true;  }
    virtual const olxstr& GetComment() const {  return comment;  }
    virtual ICifEntry* Replicate() const {
      return new cetCommentedNamedStringList(*this);
    }
  };
  typedef TPtrList<ICifEntry> CifRow;
  typedef TTTable<CifRow> CifTable;
  struct cetTable : public ICifEntry {
  protected:
    olxstr name;
    CifTable data;
  public:
    cetTable()  {}
    //takes comma separated list of column names
    cetTable(const olxstr& cols, size_t row_count=InvalidSize);
    cetTable(const cetTable& v);
    virtual ~cetTable()  {  Clear();  }
    void Clear();
    void AddCol(const olxstr& col_name);
    template <class SC>
    bool RemoveCol(const SC& col_name) { return DelCol(ColIndex(col_name)); }
    bool DelCol(size_t idx);
    CifRow& AddRow()  {  return data.AddRow();  }
    ICifEntry& Set(size_t i, size_t j, ICifEntry* v);
    const ICifEntry& Get(size_t i, size_t j)  const {  return *data[i][j];  }
    const CifTable& GetData() const {  return data;  }
    const CifRow& operator [] (size_t i) const {  return data[i];  }
    size_t ColCount() const {  return data.ColCount();  }
    const olxstr& ColName(size_t i) const {  return data.ColName(i);  }
    template <class Str>
    size_t ColIndex(const Str& name) const {
      return data.ColIndex(name);
    }
    size_t RowCount() const {  return data.RowCount();  }
    void SetRowCount(size_t sz) { data.SetRowCount(sz); }
    virtual void ToStrings(TStrList& list) const;
    virtual void Format()  {}
    virtual const olxstr& GetName() const {  return name;  }
    virtual bool HasName() const {  return true;  }
    void DataFromStrings(TStrList& lines);
    virtual ICifEntry* Replicate() const {  return new cetTable(*this);  }
    virtual olxstr GetStringValue() const {
      throw TNotImplementedException(__OlxSourceInfo);
    }
    template <class List> static olxstr GenerateName(const List& l)  {
      if( l.IsEmpty() )  return EmptyString();
      if( l.Count() == 1 )  return l[0];
      olxstr name = l[0].CommonSubString(l[1]);
      size_t min_len = olx_min(l[0].Length(), l[1].Length());
      for( size_t i=2; i < l.Count(); i++ )  {
        name = l[i].CommonSubString(name);
        if( l[i].Length() < min_len )
          min_len = l[i].Length();
      }
      if( name.IsEmpty() ) {
        throw TFunctionFailedException(__OlxSourceInfo,
          "Mismatching loop columns");
      }
      if( name.Length() != min_len )  {  // lihe _geom_angle and geom_angle_etc
        const size_t u_ind = name.LastIndexOf('_');
        if( u_ind != InvalidIndex )
          name.SetLength(u_ind);
      }
      return name;
    }
    void Sort();
    // used in table sorting, not suitable for parallelisation
    struct TableSorter  {
      static int Compare(const CifRow &r1, const CifRow &r2);
    };
  };

  struct CifBlock : public ICifEntry {
  protected:
    bool Delete(size_t idx);
  public:
    olxstr name;
    olxdict<olxstr, cetTable*, olxstrComparator<true> > table_map;
    olxdict<olxstr, ICifEntry*, olxstrComparator<true> > param_map;
    TStrPObjList<olxstr, ICifEntry*> params;
    CifBlock* parent;
    CifBlock(const CifBlock& v);
    // if parent is not NULL, creates save_ rather than data_
    CifBlock(const olxstr& _name, CifBlock* _parent=NULL)
      : name(_name), parent(_parent)  {}
    virtual ~CifBlock();
    ICifEntry& Add(ICifEntry* p);
    ICifEntry& Add(ICifEntry& p)  {  return Add(&p);  }
    template <class SC>
    bool Remove(const SC &pname) { return Delete(param_map.IndexOf(pname)); }
    bool Remove(const ICifEntry& e)  {  return Remove(e.GetName());  }
    /*Renames an item, if new_name already exists, replace_if_exists controls
    what happens to it - if this flag is true, the old value will replace the
    new one, otherwise it will be deleted
    */
    void Rename(const olxstr& old_name, const olxstr& new_name,
      bool replace_if_exists=false);
    virtual void ToStrings(TStrList& list) const;
    virtual void Format();
    virtual ICifEntry* Replicate() const {  return new CifBlock(*this);  }
    virtual olxstr GetStringValue() const {
      throw TNotImplementedException(__OlxSourceInfo);
    }
    virtual bool HasName() const {  return true;  }
    virtual const olxstr& GetName() const {  return name;  }
    void Sort(const TStrList& pivots, const TStrList& endings);
    struct EntryGroup  {
      TPtrList<ICifEntry> items;
      olxstr name;
    };
    struct CifSorter  {
      const TStrList &pivots, &endings;
      CifSorter(const TStrList& _pivots, const TStrList& _endings) :
        pivots(_pivots), endings(_endings)  {}
      int Compare(const EntryGroup &e1, const EntryGroup &e2) const;
    };
  };

  class TCifDP : public IEObject  {
  public:
  private:
    TTypeList<CifBlock> data;
    olxdict<olxstr, CifBlock*, olxstrComparator<true> > data_map;
    void Format();
    struct parse_context  {
      TStrList& lines;
      CifBlock* current_block;
      parse_context(TStrList& _lines) : lines(_lines), current_block(NULL)  {}
    };
    bool ExtractLoop(size_t& start, parse_context& context);
    static bool IsLoopBreaking(const olxstr& v)  {
      return v.StartsFrom('_') || v.StartsFromi("loop_") ||
        v.StartsFromi("data_") || v.StartsFromi("save_");
    }
  public:
    TCifDP()  {}
    virtual ~TCifDP()  {  Clear();  }
    void Clear();
    //..........................................................................
    //Load the object from a file.
    virtual void LoadFromStrings(const TStrList& Strings);
    //Saves the data to a strings
    virtual TStrList& SaveToStrings(TStrList& Strings) const;
    TStrList SaveToStrings() const {
      TStrList out;
      return SaveToStrings(out);
    }
    // number of data blocks
    size_t Count() const {  return data.Count();  }
    CifBlock& operator [] (size_t i) const {  return data[i];  }
    // if parent is not null - it is save_ rather than data_ block
    CifBlock& Add(const olxstr& name, CifBlock* parent=NULL)  {
      const size_t i = data_map.IndexOf(name);
      if( i != InvalidIndex )
        return *data_map.GetValue(i);
      else
        return *data_map.Add(name, &data.Add(new CifBlock(name, parent)));
    }
    //Finds a value by name
    CifBlock* Find(const olxstr& data_name) const {
      return data_map.Find(data_name, NULL);
    }
    size_t IndexOf(const CifBlock& cb) const {
      const CifBlock* cb_ptr = &cb;
      for( size_t i=0; i < data.Count(); i++ )
        if( &data[i] == cb_ptr )
          return i;
      return InvalidIndex;
    }
    void Rename(const olxstr& old_name, const olxstr& new_name)  {
      if( old_name == new_name )  return;
      if( data_map.HasKey(new_name) ) {
        throw TInvalidArgumentException(__OlxSourceInfo,
          olxstr("Name already in use: ") << new_name);
      }
      const size_t cb_ind = data_map.IndexOf(old_name);
      if( cb_ind == InvalidIndex ) {
        throw TInvalidArgumentException(__OlxSourceInfo,
          olxstr("Undefined block: ") << old_name);
      }
      CifBlock* cb = data_map.GetValue(cb_ind);
      cb->name = new_name;
      data_map.Delete(cb_ind);
      data_map.Add(new_name, cb);
    }
    // specific CIF tokeniser to tackle 'dog's life'...
    static size_t CIFToks(const olxstr& str, TStrList& toks);
  };
}; // end cif_dp namespace

#endif
