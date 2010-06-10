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
    virtual bool HasName() const {  return false;  }
    virtual const olxstr& GetName() const {  throw TNotImplementedException(__OlxSourceInfo);  }
  };
  struct IStringCifEntry : public ICifEntry {  // string(s) value accessor interface
    IStringCifEntry()  {}
    virtual ~IStringCifEntry()  {}
    virtual size_t Count() const {  return 0;  }
    virtual bool HasValue() const {  return Count() != 0;  }
    virtual const olxstr& operator [] (size_t i) const {  throw TNotImplementedException(__OlxSourceInfo);  }
    virtual bool HasComment() const {  return false;  }
    virtual const olxstr& GetComment() const {  throw TNotImplementedException(__OlxSourceInfo);  }
  };
  struct cetComment : public IStringCifEntry {
    olxstr comment;
    cetComment(const olxstr& _value) : comment(_value)  {}
    virtual void ToStrings(TStrList& list) const {
      if( !comment.IsEmpty() )
        list.Add('#') << comment;
    }
    virtual bool HasComment() const {  return true;  }
    virtual const olxstr& GetComment() const {  return comment;  }
  };
  struct cetString : public IStringCifEntry  {
    olxstr value;
    bool quoted;
    cetString(const olxstr& _val);
    virtual void ToStrings(TStrList& list) const;
    virtual size_t Count() const {  return 1;  }
    virtual const olxstr& operator [] (size_t i) const {  return value;  }
  };
  struct cetNamedString : public cetString {
    olxstr name;
    cetNamedString(const olxstr& _name, const olxstr& _val) : cetString(_val), name(_name)  {}
    virtual void ToStrings(TStrList& list) const;
    virtual bool HasName() const {  return true;  }
    virtual const olxstr& GetName() const {  return name;  }
  };
  struct cetCommentedString : public cetNamedString  {
    olxstr comment;
    cetCommentedString(const olxstr& _name, const olxstr& _val, const olxstr& _comment)
      : cetNamedString(_name, _val), comment(_comment)  {}
    virtual void ToStrings(TStrList& list) const {
      cetNamedString::ToStrings(list);
      list.Last().String << " #" << comment;
    }
    virtual bool HasComment() const {  return true;  }
    virtual const olxstr& GetComment() const {  return comment;  }
  };
  struct cetStringList : public IStringCifEntry {
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
    virtual size_t Count() const {  return lines.Count();  }
    virtual const olxstr& operator [] (size_t i) const {  return lines[i];  }
  };
  struct cetNamedStringList : public cetStringList  {
    olxstr name;
    cetNamedStringList(const olxstr& _name) : name(_name)  {}
    virtual void ToStrings(TStrList& list) const {
      list.Add(name);
      cetStringList::ToStrings(list);
    }
    virtual bool HasName() const {  return true;  }
    virtual const olxstr& GetName() const {  return name;  }
  };
  struct cetCommentedNamedStringList : public cetNamedStringList {
   olxstr comment;
    cetCommentedNamedStringList(const olxstr& _name, const olxstr& _comment)
      : cetNamedStringList(_name), comment(_comment)  {}
    virtual void ToStrings(TStrList& list) const {
      list.Add(name) << " #" << comment;
      cetStringList::ToStrings(list);
    }
    virtual bool HasComment() const {  return true;  }
    virtual const olxstr& GetComment() const {  return comment;  }
  };
  struct cetTable : public ICifEntry {
    TTTable<TPtrList<ICifEntry> > data;
    virtual ~cetTable();
    virtual void ToStrings(TStrList& list) const;
    virtual void Format()  {}
    virtual const olxstr& GetName() const;
    virtual bool HasName() const {  return true;  }
    void DataFromStrings(TStrList& lines);
  };

  struct CifBlock : public ICifEntry {
    olxstr name;
    olxdict<olxstr, cetTable*, olxstrComparator<true> > table_map;
    olxdict<olxstr, ICifEntry*, olxstrComparator<true> > param_map;
    TStrPObjList<olxstr, ICifEntry*> params;
    CifBlock* parent;
    CifBlock(const olxstr& _name, CifBlock* _parent=NULL) : name(_name), parent(_parent)  {}
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
    //............................................................................
    //Load the object from a file.
    virtual void LoadFromStrings(const TStrList& Strings);
    //Saves the data to a file and returns true if successful and false in the case of failure
    virtual void SaveToStrings(TStrList& Strings);
    //Finds a value by name
    inline size_t Count() const {  return data.Count();  }
    CifBlock& operator [] (size_t i) const {  return data[i];  }
    CifBlock& Add(const olxstr& name, CifBlock* parent=NULL)  {
      size_t i = data_map.IndexOf(name);
      if( i != InvalidIndex )
        return *data_map.GetValue(i);
      else
        return *data_map.Add(name, &data.Add(new CifBlock(name, parent)));
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
