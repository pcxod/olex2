#ifndef __olx_xl_cifdp_H
#define __olx_xl_cifdp_H
#include "estrlist.h"
#include "etable.h"
#include "edict.h"

namespace cif_dp {
  struct ICifEntry  {
    ICifEntry()  {}
    virtual ~ICifEntry()  {}
    virtual void ToStrings(TStrList& list) const = 0;
    virtual void Format()  {}
    //virtual bool IsMultiline
    // virtual size_t const StringCount
    // virtual const olxstr& GetString(size_t)
    // virtual void AddString()
    // virtual void Clear()
    // virtual void ...
  };
  struct cetComment : public ICifEntry {
    olxstr value;
    cetComment(const olxstr& _value) : value(_value)  {}
    virtual void ToStrings(TStrList& list) const {
      if( !value.IsEmpty() )
        list.Add(value);
    }
  };
  struct cetString : public ICifEntry  {
    olxstr value;
    bool quoted;
    cetString(const olxstr& _val);
    virtual void ToStrings(TStrList& list) const;
  };
  struct cetNamedString : public cetString {
    olxstr name;
    cetNamedString(const olxstr& _name, const olxstr& _val) : cetString(_val), name(_name)  {}
    virtual void ToStrings(TStrList& list) const;
  };
  struct cetCommentedString : public cetNamedString {
    olxstr comment;
    cetCommentedString(const olxstr& _name, const olxstr& _val, const olxstr& _comment)
      : cetNamedString(_name, _val), comment(_comment)  {}
    virtual void ToStrings(TStrList& list) const {
      cetNamedString::ToStrings(list);
      list.Last().String << " #" << comment;
    }
  };
  struct cetStringList : public ICifEntry {
    TStrList lines;
    virtual void ToStrings(TStrList& list) const {
      list.Add(';');
      list.AddList(lines);
      list.Add(';');
    }
    virtual void Format()  {
      if( lines.Count() > 1 )
        lines.TrimWhiteCharStrings();
    }
  };
  struct cetNamedStringList : public cetStringList {
    olxstr name;
    cetNamedStringList(const olxstr& _name) : name(_name)  {}
    virtual void ToStrings(TStrList& list) const {
      list.Add(name);
      cetStringList::ToStrings(list);
    }
  };
  struct cetCommentedNamedStringList : public cetNamedStringList {
    olxstr comment;
    cetCommentedNamedStringList(const olxstr& _name, const olxstr& _comment)
      : cetNamedStringList(_name), comment(_comment)  {}
    virtual void ToStrings(TStrList& list) const {
      list.Add(name) << " #" << comment;
      cetStringList::ToStrings(list);
    }
  };
  struct cetTable : public ICifEntry {
    TTTable<TPtrList<ICifEntry> > data;
    virtual ~cetTable();
    virtual void ToStrings(TStrList& list) const;
    virtual void Format()  {}
    olxstr GetName() const;
    void DataFromStrings(TStrList& lines);
  };

  struct CifBlock : public ICifEntry {
    olxstr name;
    olxdict<olxstr, cetTable*, olxstrComparator<true> > table_map;
    olxdict<olxstr, ICifEntry*, olxstrComparator<true> > param_map;
    TStrPObjList<olxstr, ICifEntry*> params;
    CifBlock(const olxstr& _name) : name(_name)  {}
    virtual ~CifBlock();
    ICifEntry& Add(const olxstr& pname, ICifEntry* p);
    virtual void ToStrings(TStrList& list) const;
    virtual void Format();
  };

  class TCifDP : public IEObject  {
  public:
  private:
    TTypeList<CifBlock> data;
    olxdict<olxstr, CifBlock*, olxstrComparator<true> > data_map;
    void Format();
    struct parse_context  {
      CifBlock* current_block;
      TStrList& lines;
      parse_context(TStrList& _lines) : lines(_lines), current_block(NULL) {}
    };
    bool ExtractLoop(size_t& start, parse_context& context);
  public:
    TCifDP()  {}
    virtual ~TCifDP()  {  Clear();  }
    void Clear();
    //............................................................................
    //Load the object from a file.
    virtual void LoadFromStrings(const TStrList& Strings);
    //Saves the data to a file and returns true if successful and false in the case of failure
    virtual void SaveToStrings(TStrList& Strings);
    //Finds a value by name
    inline size_t Count() const {  return data.Count();  }
    CifBlock& operator [] (size_t i) const {  return data[i];  }
    CifBlock& Add(const olxstr& name)  {
      size_t i = data_map.IndexOf(name);
      if( i != InvalidIndex )
        return *data_map.GetValue(i);
      else
        return *data_map.Add(name, &data.AddNew(name));
    }
    CifBlock* Find(const olxstr& data_name) const {
      size_t i = data_map.IndexOf(data_name);
      return i == InvalidIndex ? NULL : data_map.GetValue(i);
    }
    // specific CIF tokeniser to tackle 'dog's life'...
    static size_t CIFToks(const olxstr& str, TStrList& toks);
  };
}; // end cif_dp namespace

#endif
