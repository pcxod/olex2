//----------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//
#ifndef paramlistH
#define paramlistH
//---------------------------------------------------------------------------
#include "ebase.h"
#include "string.h"
#include "estrlist.h"

BeginEsdlNamespace()

//---------------------------------------------------------------------------
class TParamList: protected TStrStrList  {
public:
  TParamList();
  TParamList(const TParamList &v);
  virtual ~TParamList();
  inline void Clear()                              {  TStrStrList::Clear(); }
  inline int Count()                      const {  return TStrStrList::Count();  };
  inline bool IsEmpty()                      const {  return TStrStrList::IsEmpty();  }
  inline const olxstr& GetValue(int index) const {  return Object(index);  }
  inline olxstr& Value(int index)                {  return Object(index);  }
  inline const olxstr& GetName(int index)  const {  return String(index);  }
  void FromString(const olxstr &S, char Sep); // -t=op
  void AddParam(const olxstr &Name, const olxstr &Param, bool Check = true);
  inline bool Contains(const olxstr &Name)  const {  return IndexOf(Name) != -1;  }
  const olxstr& FindValue(const olxstr &Name, const olxstr& defval=EmptyString) const;
  // these functions consider the folowing situation '"'
  template <class SC, class T>
    static int StrtokParams(const olxstr &Cmd, char Separator, TTStrList<SC,T>& Params)  {
      if( Separator == '\'' || Separator == '"' )
        throw TInvalidArgumentException(__OlxSourceInfo, "separator");
      int bc=0, pc=0, sc=0;
      olxch Sep='#';
      olxstr Param;
      for( int i=0; i < Cmd.Length(); i++ )  {
        if( (Cmd[i] == '\'' || Cmd[i] == '\"') && (Sep == '#') ) // identify separator
          Sep = Cmd[i];
        if( Cmd[i] == Sep && Sep !='#' )  sc++;  //count separators
        if( Cmd[i] == '(' )  bc++;  //count brackets
        if( Cmd[i] == ')' )  bc--;
        // check it is not between (,) or ',' or ","
        if( Cmd[i] == Separator && !bc && (sc%2) == 0 && Param.Length() != 0 )  { 
          ProcessStringParam(Param);  // skip double quotes '' or ""
          if( !Param.IsEmpty() )  {
            Params.Add(Param);
            pc++;
            Param = EmptyString;
          }
          Sep = '#';  // reset quotation character
          continue;
        }
        if( Cmd[i] == Separator )  {
          if( (sc%2) != 0 || bc != 0 )
            Param << Cmd[i];
          continue;
        }
        Param << Cmd[i];
      }
      if( !Param.IsEmpty() )  {
        ProcessStringParam(Param);
        pc++;
        Params.Add(Param);
      }
      return pc;
    }
  //this function removes the wrapping around the string 'str""'
  static bool ProcessStringParam(olxstr &Param);
  /* if the quation char is the same at the end and beginning of the string
     function returns true and initilises the Char argument */
  static bool GetQuotationChar( const olxstr &Param, olxch& Char );
};

EndEsdlNamespace()
#endif
