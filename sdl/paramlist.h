/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_paramlist_H
#define __olx_sdl_paramlist_H
#include "ebase.h"
#include "exparse/exptree.h"
#undef GetObject
BeginEsdlNamespace()

class TParamList: protected TStrStrList  {
public:
  TParamList() {}
  TParamList(const TParamList& v);
  virtual ~TParamList() {}
  void Clear()  {  TStrStrList::Clear(); }
  size_t Count() const {  return TStrStrList::Count();  };
  bool IsEmpty() const {  return TStrStrList::IsEmpty();  }
  const olxstr& GetValue(size_t index) const {  return GetObject(index);  }
  olxstr& GetValue(size_t index)  {  return GetObject(index);  }
  const olxstr& GetName(size_t index) const {  return GetString(index);  }
  void FromString(const olxstr& S, char Sep); // -t=op
  void AddParam(const olxstr& Name, const olxstr& Param, bool Check = true);
  template <class T>
  bool Contains(const T& Name) const {
    return TStrStrList::IndexOf(Name) != InvalidIndex;
  }
  template <class T>
  const olxstr& FindValue(const T& Name,
    const olxstr& defval=EmptyString()) const
  {
    size_t i = IndexOf(Name);
    return (i != InvalidIndex) ? GetObject(i) : defval;
  }
  // special evaluation of a boolean option
  template <class T>
  bool GetBoolOption(const T& Name,
    bool if_empty=true,
    bool if_does_not_exist=false) const
  {
    size_t i = IndexOf(Name);
    if (i == InvalidIndex) return if_does_not_exist;
    const olxstr &v = GetObject(i);
    return v.IsEmpty() ? if_empty : v.ToBool();
  }
  template <class T>
  const olxstr& operator [] (const T& Name) const {  return FindValue(Name);  }
  // this function considers the folowing situations: '"', '('')' and '\''
  template <class StrLst>
  static size_t StrtokParams(const olxstr& exp, olxch sep, StrLst& out,
    bool do_unquote=true)
  {
    using namespace exparse::parser_util;
    if( is_quote(sep) )
      throw TInvalidArgumentException(__OlxSourceInfo, "separator");
    const size_t pc = out.Count();
    size_t start = 0;
    for( size_t i=0; i < exp.Length(); i++ )  {
      const olxch ch = exp.CharAt(i);
      if( is_quote(ch) && !is_escaped(exp, i) )  {
        if( !skip_string(exp, i) )  {
          out.Add(exp.SubStringFrom(start).TrimWhiteChars());
          start = exp.Length();
          break;
        }
      }
      else if (is_bracket(ch) && !is_escaped(exp, i)) {
        if( !skip_brackets(exp, i) )  {
          out.Add(exp.SubStringFrom(start).TrimWhiteChars());
          start = exp.Length();
          break;
        }
      }
      else if( ch == sep )  {
        // white spaces cannot define empty args
        if( sep == ' ' && start == i )  {
          start = i+1;
          continue;
        }
        if( do_unquote )
          out.Add(unquote(exp.SubString(start, i-start).TrimWhiteChars()));
        else
          out.Add(exp.SubString(start, i-start).TrimWhiteChars());
        start = i+1;
      }
    }
    if( start < exp.Length() )  {
      if( do_unquote )
        out.Add(unquote(exp.SubStringFrom(start).TrimWhiteChars()));
      else
        out.Add(exp.SubStringFrom(start).TrimWhiteChars());
    }
    return out.Count() - pc;
  }
  /* this function considers the folowing situations: '"', '('')'
  to be used to tokenise strings like "cmd1>>if something then 'icmd1>>icmd2'"
  in this case the quoted >> will not be a separator
  */
  static const_strlist StrtokLines(const olxstr& exp, const olxstr &sep,
    bool do_unquote=true)
  {
    using namespace exparse::parser_util;
    if (sep.IsEmpty() ||(sep.Length() == 1 && is_quote(sep[0])))
      throw TInvalidArgumentException(__OlxSourceInfo, "separator");
    TStrList rv;
    size_t start = 0;
    for (size_t i=0; i < exp.Length(); i++) {
      const olxch ch = exp.CharAt(i);
      if (is_quote(ch) && !is_escaped(exp, i)) {
        if (!skip_string(exp, i)) {
          rv.Add(exp.SubStringFrom(start).TrimWhiteChars());
          start = exp.Length();
          break;
        }
      }
      else if (is_bracket(ch) && !is_escaped(exp, i)) {
        if (!skip_brackets(exp, i) ) {
          rv.Add(exp.SubStringFrom(start).TrimWhiteChars());
          start = exp.Length();
          break;
        }
      }
      else if (exp.IsSubStringAt(sep, i)) {
        // white spaces cannot define empty args
        if (sep == ' ' && start == i) {
          start = i+1;
          continue;
        }
        if (do_unquote)
          rv.Add(unquote(exp.SubString(start, i-start).TrimWhiteChars()));
        else
          rv.Add(exp.SubString(start, i-start).TrimWhiteChars());
        start = i+sep.Length();
        i += (sep.Length()-1);
      }
    }
    if (start < exp.Length()) {
      if (do_unquote)
        rv.Add(unquote(exp.SubStringFrom(start).TrimWhiteChars()));
      else
        rv.Add(exp.SubStringFrom(start).TrimWhiteChars());
    }
    return rv;
  }
};

EndEsdlNamespace()
#endif
