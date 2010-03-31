#ifndef __olx_xl_cif_H
#define __olx_xl_cif_H
#include "estrlist.h"
#include "etable.h"
#include "edict.h"

namespace cif_dp  {
  struct ICifEntry  {
    virtual void ToStrings(TStrList& list) const = 0;
    virtual void Format()  {}
    virtual ~ICifEntry()  {}
    //virtual bool IsMultiline
  };
  struct cetString : public ICifEntry  {
    olxstr value;
    bool quoted;
    cetString(const olxstr& _val) : value(_val)  {
      if( _val.Length() > 1 )  {
        const olxch ch = _val[0];
        if( (ch == '\'' || ch == '"') && _val.EndsWith(ch) )  {
          value = _val.SubString(1,1);
          quoted = true;
        }
      }
    }
    virtual void ToStrings(TStrList& list) const {
      if( quoted )  {
        if( list.IsEmpty() || (list.Last().String.Length() + value.Length() + 3 < 80) )
          list.Add(" '") << value << '\'';
        else
          list.Last().String << " '" << value << '\'';
      }
      else  {
        if( list.IsEmpty() || (list.Last().String.Length() + value.Length() + 1 < 80) )
          list.Add(' ') << value;
        else
          list.Last().String << ' ' << value;
      }
    }
  };
  struct cetNamedString : public cetString {
    olxstr name;
    cetNamedString(const olxstr& _name, const olxstr& _val) : cetString(_val), name(_name)  {}
    virtual void ToStrings(TStrList& list) const {
      olxstr& tmp = list.Add(name);
      tmp.Format(34, true, ' ');
      if( quoted )
        tmp << '\'' << value << '\'';
      else
        tmp << value;
    }
  };
  struct cetCommentedString : public cetNamedString {
    olxstr comment;
    cetCommentedString(const olxstr& _name, const olxstr& _val, const olxstr& _comment)
      : cetNamedString(name, _val), comment(_comment)  {}
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
    virtual void ToStrings(TStrList& list) const {
      list.Add(name);
      cetStringList::ToStrings(list);
    }
  };
  struct cetCommentedNamedStringList : public cetStringList {
    olxstr name, comment;
    virtual void ToStrings(TStrList& list) const {
      list.Add(name) << " #" << comment;
      cetStringList::ToStrings(list);
    }
  };
  struct cetTable : public ICifEntry {
    TTTable<TStrPObjList<olxstr, ICifEntry*> > data;
  };

  struct CifBlock : public ICifEntry {
    olxstr name;
    olxdict<olxstr, cetTable*, olxstrComparator<true> > table_map;
    olxdict<olxstr, size_t, olxstrComparator<true> > param_map;
    TStrPObjList<olxstr,ICifEntry*> params;
    virtual void ToStrings(TStrList& list) const {
      list.Add("data_") << name;
      for( size_t i=0; i < params.Count(); i++ )
        params.GetObject(i)->ToStrings(list);
    }
    virtual void Format()  {
      for( size_t i=0; i < params.Count(); i++ )
        params.GetObject(i)->Format();    
    }
  };

class TCifDP : public IEObject  {
public:
private:
  TTypeList<CifBlock> data;
  olxdict<olxstr, CifBlock*, olxstrComparator<true> > data_map;
  void Format();
  bool ExtractLoop(size_t& start);
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
  CifBlock* Find(const olxstr& data_name) const {
    size_t i = data_map.IndexOf(data_name);
    return i == InvalidIndex ? NULL : data_map.GetValue(i);
  }
  // specific CIF tokeniser to tackle 'dog's life'...
  static size_t CIFToks(const olxstr& str, TStrList& toks);
};
}; // end cif_dp namespace

#endif
