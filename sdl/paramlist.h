//----------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//
#ifndef paramlistH
#define paramlistH
//---------------------------------------------------------------------------
#include "ebase.h"
#include "exparse/exptree.h"
#undef GetObject
BeginEsdlNamespace()

//---------------------------------------------------------------------------
class TParamList: protected TStrStrList  {
public:
  TParamList();
  TParamList(const TParamList& v);
  virtual ~TParamList();
  inline void Clear()                            {  TStrStrList::Clear(); }
  inline int Count()                       const {  return TStrStrList::Count();  };
  inline bool IsEmpty()                    const {  return TStrStrList::IsEmpty();  }
  inline const olxstr& GetValue(int index) const {  return GetObject(index);  }
  inline olxstr& Value(int index)                {  return GetObject(index);  }
  inline const olxstr& GetName(int index)  const {  return GetString(index);  }
  void FromString(const olxstr& S, char Sep); // -t=op
  void AddParam(const olxstr& Name, const olxstr& Param, bool Check = true);
  template <class T>
  inline bool Contains(const T& Name)  const {  return IndexOf(Name) != -1;  }
  template <class T>
  const olxstr& FindValue(const T& Name, const olxstr& defval=EmptyString) const {
    int i = IndexOf(Name);
    return (i >= 0) ? GetObject(i) : defval;
   }
  template <class T>
  const olxstr& operator [] (const T& Name) const {  return FindValue(Name);  }
  // these functions considers the folowing situations: '"', '('')' and '\''
  template <class StrLst>
    static int StrtokParams(const olxstr& exp, olxch sep, StrLst& out, bool do_unquote=true)  {
      using namespace exparse::parser_util;
      if( is_quote(sep) )
        throw TInvalidArgumentException(__OlxSourceInfo, "separator");
      const int pc = out.Count();
      int start = 0;
      for( int i=0; i < exp.Length(); i++ )  {
        const olxch ch = exp.CharAt(i);
        if( is_quote(ch) && !is_escaped(exp, i) )  {
          if( !skip_string(exp, i) )  {
            out.Add( exp.SubStringFrom(start).TrimWhiteChars() );
            start = exp.Length();
            break;
          }
        }
        else if( is_bracket(ch) )  {
          if( !skip_brackets(exp, i) )  {
            out.Add( exp.SubStringFrom(start).TrimWhiteChars() );
            start = exp.Length();
            break;
          }
        }
        else if( ch == sep )  {
          if( sep == ' ' && start == i )  { // white spaces cannot define empty args
            start = i+1;
            continue;
          }
          if( do_unquote )
            out.Add( unquote(exp.SubString(start, i-start).TrimWhiteChars()) );
          else 
            out.Add( exp.SubString(start, i-start).TrimWhiteChars() );
          start = i+1;
        }
      }
      if( start < exp.Length() )  {
        if( do_unquote )
          out.Add( unquote(exp.SubStringFrom(start).TrimWhiteChars()) );
        else 
          out.Add( exp.SubStringFrom(start).TrimWhiteChars() );
      }
      return out.Count() - pc;
    }
  //this function removes the wrapping around the string 'str""'
  //static olxstr ProcessStringParam(const olxstr& Param);
  /* if the quation char is the same at the end and beginning of the string
     function returns true and initilises the Char argument */
  //static bool GetQuotationChar(const olxstr& Param, olxch* Char);
};

EndEsdlNamespace()
#endif
