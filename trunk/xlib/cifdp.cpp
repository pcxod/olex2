//---------------------------------------------------------------------------//
// CIF data provider
// (c) Oleg V. Dolomanov, 2010
//---------------------------------------------------------------------------//
#include "cifdp.h"

#include "bapp.h"
#include "log.h"
#include "etime.h"

using namespace exparse::parser_util;
using namespace cif_dp;

void TCifDP::Clear()  {
  data.Clear();
  data_map.Clear();
}
//..............................................................................
void TCifDP::Format()  {
  for( size_t i=0; i < data.Count(); i++ )
    data[i].Format();
}
//..............................................................................
bool TCifDP::ExtractLoop(size_t& start)  {
//  if( !Lines[start].StartsFromi("loop_") )  return false;
//  TCifLoop& Loop = *(new TCifLoop);
//  Loops.Add(EmptyString, &Loop);
//  TStrList loop_data;
//  bool parse_header = true;
//  if( Lines[start].IndexOf(' ') != InvalidIndex )  {
//    TStrList toks;
//    CIFToks(Lines[start], toks);
//    for( size_t i=1; i < toks.Count(); i++ )  {
//      if( !parse_header || toks[i].CharAt(0) != '_' )  {
//        parse_header = false;
//        loop_data.Add(toks[i]);
//      }
//      else
//        Loop.GetTable().AddCol(toks[i]);
//    }
//  }
//  while( parse_header )  {  // skip loop definition
//    if( ++start >= Lines.Count() )  {  // // end of file?
//      Loops.Last().String = Loop.GetLoopName();
//      return true;
//    }
//    if( Lines[start].IsEmpty() )  continue;
//    if( Lines[start].CharAt(0) != '_' )  {  start--;  break;  }
//    if( Loop.GetTable().ColCount() != 0 )  {
//      /* check that the item actually belongs to the loop, this might happens in the case of empty loops */
//      if( olxstr::CommonString(Lines[start], Loop.GetTable().ColName(0)).Length() == 1 )  {
//        Loops.Last().String = Loop.GetLoopName();
//        start--;  // rewind
//        return true;
//      }
//    }
//    bool param_found = false;  // in the case loop header is mixed up with loop data...
//    if( Lines[start].IndexOf(' ') == InvalidIndex )
//      Loop.GetTable().AddCol(Lines[start]);
//    else  {
//      TStrList toks;
//      CIFToks(Lines[start], toks);
//      for( size_t i=0; i < toks.Count(); i++ )  {
//        if( param_found || toks[i].CharAt(0) != '_' )  {
//          param_found = true;
//          loop_data.Add(toks[i]);
//        }
//        else
//          Loop.GetTable().AddCol(toks[i]);
//      }
//    }
//    Lines[start] = EmptyString;
//    if( param_found )  break;
//  }
//  size_t q_cnt = 0;
//  while( true )  {  // skip loop data
//    while( ++start < Lines.Count() && Lines[start].IsEmpty() )  continue;
//    if( start >= Lines.Count() )  break;
//    // a new loop or dataset started (not a part of a multi-string value)
//    if( (q_cnt%2) == 0 && (Lines[start].StartsFrom('_') || 
//      Lines[start].StartsFromi("loop_") || Lines[start].StartsFromi("data_")) )
//      break;
//    if( Lines[start].CharAt(0) == ';' )  q_cnt++;
//    loop_data.Add(Lines[start]);
//    Lines[start] = EmptyString;
//  }
//  Loop.Format(loop_data);
//  Loops.Last().String = Loop.GetLoopName();
//  start--;
//  return true;
  return false;
}
//..............................................................................
void TCifDP::LoadFromStrings(const TStrList& Strings)  {
  Clear();
  //Lines = Strings;
  //for( size_t i=0; i < Lines.Count(); i++ )  {
  //  if( Lines[i].StartsFrom(';') )  {  // skip these things
  //    while( ++i < Lines.Count() && !Lines[i].StartsFrom(';') )  continue;
  //    continue;
  //  }
  //  Lines[i].DeleteSequencesOf<char>(' ').Trim(' ');
  //  if( Lines[i].IsEmpty() )  continue;
  //  size_t spindex = Lines[i].FirstIndexOf('#');  // a comment char
  //  if( spindex != InvalidIndex )  {
  //    if( spindex != 0 )  {
  //      olxstr Tmp = Lines[i];
  //      Lines[i] = Tmp.SubStringTo(spindex-1);  // remove the '#' character
  //      Lines.Insert(++i, Tmp.SubStringFrom(spindex));
  //    }
  //  }
  //}
  //for( size_t i=0; i < Lines.Count(); i++ )  {
  //  const olxstr& line = Lines[i];
  //  if( line.IsEmpty() )  continue;
  //  if( line.CharAt(0) == '#')  continue;
  //  if( ExtractLoop(i) )  continue;
  //  if( line.CharAt(0) == '_' )  {  // parameter
  //    olxstr Val, Param;
  //    size_t spindex = line.FirstIndexOf(' ');
  //    if( spindex != InvalidIndex )  {
  //      Param = line.SubStringTo(spindex);
  //      Val = line.SubStringFrom(spindex+1); // to remove the space
  //    }
  //    else
  //      Param = line;
  //    CifData *D = Lines.Set(i, Param, Parameters.Add(Param, new CifData(false)).Object).Object;
  //    if( !Val.IsEmpty() )
  //      D->data.Add(Val);
  //    else  {
  //      olxch Char;
  //      while( ++i < Lines.Count() && Lines[i].IsEmpty() )  continue;
  //      if( i >= Lines.Count() )  continue;
  //      Char = Lines[i].CharAt(0);
  //      while( Char == '#' && ++i < Lines.Count() )  {
  //        while( Lines[i].IsEmpty() && ++i < Lines.Count() )  continue;
  //        if( i >= Lines.Count() )  break;
  //        Char = Lines[i].CharAt(0);
  //      }
  //      if( Char == ';' )  {
  //        size_t sc_count = 1; 
  //        if( Lines[i].Length() > 1 )
  //          D->data.Add(Lines[i].SubStringFrom(1));
  //        Lines[i] = EmptyString;
  //        while( ++i < Lines.Count() )  {
  //          if( !Lines[i].IsEmpty() && Lines[i].CharAt(0) == ';' )  {
  //            Lines[i] = EmptyString;
  //            break;
  //          }
  //          D->data.Add(Lines[i]);
  //          Lines[i] = EmptyString;
  //        }
  //        D->quoted = true;
  //      }
  //      else if( Char = '\'' || Char == '"' )  {
  //        D->data.Add(Lines[i]);
  //        Lines[i] = EmptyString;
  //        continue;
  //      }
  //    }
  //  }
  //  else if( line.StartsFrom("data_") )  {
  //    if( FDataNameUpperCase )
  //      FDataName = line.SubStringFrom(5).UpperCase();
  //    else
  //      FDataName = line.SubStringFrom(5);
  //    FDataName.DeleteSequencesOf(' ');
  //    Lines[i] = "data_";
  //    Lines[i] << FDataName;
  //  }
  //}
  Format();
}
//..............................................................................
void TCifDP::SaveToStrings(TStrList& Strings)  {
  for( size_t i=0; i < data.Count(); i++ )
    data[i].ToStrings(Strings);
}
//..............................................................................
size_t TCifDP::CIFToks(const olxstr& exp, TStrList& out)  {
  size_t start = 0;
  const size_t toks_c = out.Count();
  for( size_t i=0; i < exp.Length(); i++ )  {
    const olxch ch = exp.CharAt(i);
    if( is_quote(ch) && (i==0 || olxstr::o_iswhitechar(exp[i-1])) )  {
      while( ++i < exp.Length() )  {
        if( exp[i] == ch && ((i+1) >= exp.Length() || olxstr::o_iswhitechar(exp[i+1])) )  {
          break;
        }
      }
    }
    else if( olxstr::o_iswhitechar(ch) )  {
      if( start == i )  { // white chars cannot define empty args
        start = i+1;
        continue;
      }
      out.Add(exp.SubString(start, i-start).TrimWhiteChars());
      start = i+1;
    }
  }
  if( start < exp.Length() )
    out.Add(exp.SubStringFrom(start).TrimWhiteChars());
  return out.Count() - toks_c;
}
//..............................................................................

