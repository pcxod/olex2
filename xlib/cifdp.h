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
  class ParsingException : public TBasicException {
    size_t LineNumber;
  public:
    ParsingException(const ParsingException& e)
      : TBasicException(e), LineNumber(e.LineNumber) {}
    ParsingException(const olxstr& location, const olxstr& msg, size_t lineNumber)
      : TBasicException(location, olxstr("Failed to parse CIF because ") <<
        msg << " at CIF#" << lineNumber),
      LineNumber(lineNumber)
    {}
    ParsingException(const olxstr& location, const TExceptionBase& cause,
      const olxstr& msg = EmptyString())
      : TBasicException(location, cause, msg)
    {}
    virtual const char* GetNiceName() const { return "CIF reading"; }
    virtual IOlxObject* Replicate() const {
      return new ParsingException(*this);
    }
    size_t GetLineNumber() {
      return LineNumber;
    }
  };
  /////////////////////////////////////////////////////////////////////////////
  struct CifToken {
    olxstr value;
    size_t lineNumber;
    CifToken(const olxstr &v, size_t ln)
      : value(v), lineNumber(ln)
    {}
  };
  /////////////////////////////////////////////////////////////////////////////
  struct ICifEntry : public IOlxObject {
    olx_object_ptr<olxstr> name, comment;
    ICifEntry() {}
    ICifEntry(const ICifEntry &e) {
      if (e.name.ok()) {
        name = new olxstr(*e.name);
      }
      if (e.comment.ok()) {
        comment = new olxstr(*e.comment);
      }
    }
    virtual ~ICifEntry() {}
    virtual void ToStrings(TStrList& list) const = 0;
    virtual void Format() {}
    bool HasName() const { return name.ok(); }
    bool HasComment() const { return comment.ok(); }
    virtual bool IsSaveable() const { return true; }
    // if the returned valus is InvalidIndex - the object is not comparable
    virtual size_t GetCmpHash() const { return InvalidIndex; }
    const olxstr& GetName() const {
      if (name.ok()) {
        return *name;
      }
      throw TNotImplementedException(__OlxSourceInfo);
    }
    virtual void SetName(const olxstr &n) {
      name = new olxstr(n);
    }
    const olxstr& GetComment() const {
      if (comment.ok()) {
        return *comment;
      }
      throw TNotImplementedException(__OlxSourceInfo);
    }
    void SetComment(const olxstr &c) {
      comment = new olxstr(c);
    }
    virtual ICifEntry* Replicate() const = 0;
    virtual olxstr GetStringValue() const = 0;
    static ICifEntry *FromToken(const CifToken &t, int version);
  };
  /////////////////////////////////////////////////////////////////////////////
  // string(s) value accessor interface
  struct IStringCifEntry : public ICifEntry {
    IStringCifEntry() {}
    IStringCifEntry(const ICifEntry &e)
      : ICifEntry(e)
    {}
    virtual ~IStringCifEntry() {}
    virtual size_t Count() const { return 0; }
    virtual const olxstr& operator [] (size_t) const {
      throw TNotImplementedException(__OlxSourceInfo);
    }
    virtual const olxstr& GetComment() const {
      throw TNotImplementedException(__OlxSourceInfo);
    }
  };
  /////////////////////////////////////////////////////////////////////////////
  struct cetComment : public IStringCifEntry {
    cetComment(const cetComment& v)
      : IStringCifEntry(v)
    {}
    cetComment(const olxstr& _value) {
      comment = new olxstr(_value);
    }
    virtual void ToStrings(TStrList& list) const {
      if (!comment->IsEmpty()) {
        list.Add('#') << *comment;
      }
    }
    virtual bool HasComment() const { return true; }
    virtual const olxstr& GetComment() const { return *comment; }
    virtual ICifEntry* Replicate() const { return new cetComment(*this); }
    virtual olxstr GetStringValue() const { return *comment; }
  };
  /////////////////////////////////////////////////////////////////////////////
  struct cetString : public IStringCifEntry {
    olxstr value;
    static const olxstr &GetEmptyValue() {
      static olxstr empty_value("''");
      return empty_value;
    }
    void SetValue_(const olxstr &val);
    bool quoted;
    cetString(const cetString& v)
      : IStringCifEntry(v),
      value(v.value), quoted(v.quoted)
    {}
    cetString(const olxstr& _val) : quoted(false) {
      SetValue_(_val);
    }
    cetString(const olxstr& _name, const olxstr &val)
      : quoted(false)
    {
      SetName(_name);
      SetValue_(val);
    }
    cetString(const olxstr& _val, bool quoted_)
      : value(_val), quoted(quoted_)
    {}
    virtual void ToStrings(TStrList& list) const;
    virtual size_t Count() const { return 1; }
    virtual const olxstr& operator [] (size_t) const { return value; }
    virtual ICifEntry* Replicate() const { return new cetString(*this); }
    virtual olxstr GetStringValue() const { return value; }
    static cetString *NewNamedString(const olxstr &name, const olxstr &val) {
      cetString *r = new cetString(val);
      r->SetName(name);
      return r;
    }
  };
  /////////////////////////////////////////////////////////////////////////////
  struct cetStringList : public IStringCifEntry {
    TStrList lines;
    cetStringList() {}
    cetStringList(const olxstr &name) {
      SetName(name);
    }
    cetStringList(const cetStringList& v)
      : IStringCifEntry(v),
      lines(v.lines)
    {}
    virtual void ToStrings(TStrList& list) const;
    virtual void Format() {
      if (lines.Count() > 1) {
        lines.TrimWhiteCharStrings();
      }
    }
    virtual size_t Count() const { return lines.Count(); }
    virtual const olxstr& operator [] (size_t i) const { return lines[i]; }
    virtual ICifEntry* Replicate() const { return new cetStringList(*this); }
    virtual olxstr GetStringValue() const;
  };
  /////////////////////////////////////////////////////////////////////////////
  struct cetList : public ICifEntry {
    TPtrList<ICifEntry> data;
    cetList(const cetList &l);
    cetList() {}
    ~cetList();
    void FromToken(const CifToken &token);
    virtual void ToStrings(TStrList& list) const;
    virtual bool HasName() const { return false; }
    virtual olxstr GetStringValue() const {
      throw TNotImplementedException(__OlxSourceInfo);
    }
    virtual ICifEntry* Replicate() const {
      return new cetList(*this);
    }
  };
  /////////////////////////////////////////////////////////////////////////////
  struct cetDict : public ICifEntry {
    typedef olx_pair_t<IStringCifEntry*, ICifEntry*> dict_item_t;
    TTypeList<dict_item_t> data;
    cetDict(const cetDict &l);
    cetDict() {}
    ~cetDict();
    void FromToken(const CifToken &token);
    void Add(const olxstr &name, ICifEntry * value) {
      data.Add(new dict_item_t(new cetString(name, true), value));
    }
    virtual void ToStrings(TStrList& list) const;
    virtual bool HasName() const { return false; }
    virtual olxstr GetStringValue() const {
      throw TNotImplementedException(__OlxSourceInfo);
    }
    virtual ICifEntry* Replicate() const {
      return new cetDict(*this);
    }
  };
  /////////////////////////////////////////////////////////////////////////////
  typedef TPtrList<ICifEntry> CifRow;
  typedef TTTable<CifRow> CifTable;
  struct cetTable : public ICifEntry {
  protected:
    CifTable data;
  public:
    cetTable() {}
    //takes comma separated list of column names
    cetTable(const olxstr& cols, size_t row_count = InvalidSize);
    cetTable(const cetTable& v);
    virtual ~cetTable() { Clear(); }
    virtual void SetName(const olxstr&);
    void Clear();
    void AddCol(const olxstr& col_name);
    template <class SC>
    bool RemoveCol(const SC& col_name) { return DelCol(ColIndex(col_name)); }
    void DelRow(size_t idx);
    bool DelCol(size_t idx);
    CifRow& AddRow() { return data.AddRow(); }
    ICifEntry& Set(size_t i, size_t j, ICifEntry* v);
    const ICifEntry& Get(size_t i, size_t j)  const { return *data[i][j]; }
    const CifTable& GetData() const { return data; }
    const CifRow& operator [] (size_t i) const { return data[i]; }
    size_t ColCount() const { return data.ColCount(); }
    const olxstr& ColName(size_t i) const { return data.ColName(i); }
    template <class Str>
    size_t ColIndex(const Str& name) const {
      return data.ColIndexi(name);
    }
    size_t RowCount() const { return data.RowCount(); }
    void SetRowCount(size_t sz) { data.SetRowCount(sz); }
    void SetRowCapacity(size_t sz) { data.SetRowCapacity(sz); }
    virtual void ToStrings(TStrList& list) const;
    virtual void Format() {}
    virtual ICifEntry* Replicate() const { return new cetTable(*this); }
    virtual olxstr GetStringValue() const {
      throw TNotImplementedException(__OlxSourceInfo);
    }
    template <class List> static olxstr GenerateName(const List& l) {
      if (l.IsEmpty()) {
        return EmptyString();
      }
      if (l.Count() == 1) {
        return l[0];
      }
      olxstr name = l[0].CommonSubString(l[1]);
      size_t min_len = olx_min(l[0].Length(), l[1].Length());
      for (size_t i = 2; i < l.Count(); i++) {
        name = l[i].CommonSubString(name);
        if (l[i].Length() < min_len) {
          min_len = l[i].Length();
        }
      }
      if (name.IsEmpty()) {
        throw TFunctionFailedException(__OlxSourceInfo,
          "Mismatching loop columns");
      }
      if (name.Length() != min_len) {  // lihe _geom_angle and geom_angle_etc
        const size_t u_ind = name.LastIndexOf('_');
        if (u_ind != InvalidIndex) {
          name.SetLength(u_ind);
        }
      }
      return name;
    }
    void Sort();
    // used in table sorting, not suitable for parallelisation
    struct TableSorter {
      int Compare_(const CifRow &r1, const CifRow &r2) const;
      template <class item_a_t, class item_b_t>
      int Compare(const item_a_t &r1, const item_b_t &r2) const {
        return Compare_(olx_ref::get(r1), olx_ref::get(r2));
      }
    };
  };
  /////////////////////////////////////////////////////////////////////////////
  struct CifBlock : public ICifEntry {
  protected:
    bool Delete(size_t idx);
  public:
    olxstr_dict<cetTable*, true> table_map;
    olxstr_dict<ICifEntry*, true> param_map;
    TStringToList<olxstr, ICifEntry*> params;
    CifBlock* parent;
    CifBlock(const CifBlock& v);
    // if parent is not NULL, creates save_ rather than data_
    CifBlock(const olxstr& _name, CifBlock* _parent = 0)
      : parent(_parent)
    {
      SetName(_name);
    }
    virtual ~CifBlock();
    ICifEntry& Add(ICifEntry* p);
    ICifEntry& Add(ICifEntry& p) { return Add(&p); }
    template <class SC>
    bool Remove(const SC &pname) { return Delete(param_map.IndexOf(pname)); }
    bool Remove(const ICifEntry& e) { return Remove(e.GetName()); }
    /*Renames an item, if new_name already exists, replace_if_exists controls
    what happens to it - if this flag is true, the old value will replace the
    new one, otherwise it will be deleted
    */
    void Rename(const olxstr& old_name, const olxstr& new_name,
      bool replace_if_exists = false);
    virtual void ToStrings(TStrList& list) const;
    virtual void Format();
    virtual ICifEntry* Replicate() const { return new CifBlock(*this); }
    virtual olxstr GetStringValue() const {
      throw TNotImplementedException(__OlxSourceInfo);
    }
    virtual bool HasName() const { return true; }
    virtual const olxstr& GetName() const { return name; }
    void Sort(const TStrList& pivots, const TStrList& endings);
    struct EntryGroup {
      TPtrList<ICifEntry> items;
      olxstr name;
    };
    struct CifSorter {
      const TStrList &pivots, &endings;
      CifSorter(const TStrList& _pivots, const TStrList& _endings) :
        pivots(_pivots), endings(_endings) {}
      int Compare_(const EntryGroup &e1, const EntryGroup &e2) const;
      template <class item_a_t, class item_b_t>
      int Compare(const item_a_t &e1, const item_b_t &e2) const {
        return Compare_(olx_ref::get(e1), olx_ref::get(e2));
      }
    };
  };
  /////////////////////////////////////////////////////////////////////////////
  class TCifDP : public IOlxObject {
  private:
    int version;
    TTypeList<CifBlock> data;
    olxstr_dict<CifBlock*, true> data_map;
    void Format();
    static bool IsLoopBreaking(const olxstr& v) {
      return v.StartsFrom('_') || v.StartsFromi("loop_") ||
        v.StartsFromi("data_") || v.StartsFromi("save_");
    }
    struct LineIndexer {
      const olxstr &str;
      size_t lastPos, ln;
      LineIndexer(const olxstr &str)
        : str(str), lastPos(0), ln(0)
      {}
      size_t GetLineNumber(size_t idx);
    };
  public:
    TCifDP()
      :version(2)
    {}
    virtual ~TCifDP() { Clear(); }
    void Clear();
    //..........................................................................
    //Load the object from a file.
    virtual void LoadFromStrings(const TStrList &lines);
    virtual void LoadFromStream(IInputStream &stream);
    virtual void LoadFromString(const olxstr &lines);
    //Saves the data to a strings
    virtual TStrList& SaveToStrings(TStrList& Strings) const;
    TStrList::const_list_type SaveToStrings() const {
      TStrList out;
      return SaveToStrings(out);
    }
    // number of data blocks
    size_t Count() const { return data.Count(); }
    CifBlock& operator [] (size_t i) const { return data[i]; }
    // if parent is not null - it is save_ rather than data_ block
    CifBlock& Add(const olxstr& name, CifBlock* parent = 0);
    //Finds a value by name
    CifBlock* Find(const olxstr& data_name) const {
      return data_map.Find(data_name, NULL);
    }
    int GetVersion() const {
      return version;
    }
    void SetVersion(int v) {
      version = v;
    }
    size_t IndexOf(const CifBlock& cb) const;
    void Rename(const olxstr& old_name, const olxstr& new_name);

    static TTypeList<CifToken>::const_list_type
      TokenizeString(const olxstr &str, int version);

    static olxstr ExtractBracketedData(const olxstr &str,
      olxch open, olxch close, size_t &i);
  };
}; // end cif_dp namespace

#endif
