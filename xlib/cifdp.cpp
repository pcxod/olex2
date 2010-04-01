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
bool TCifDP::ExtractLoop(size_t& start, parse_context& context)  {
  if( !context.lines[start].StartsFromi("loop_") )  return false;
  TStrList& Lines = context.lines;
  if( context.current_block == NULL )
    context.current_block = &Add("anonymous");
  cetTable& table = *(new cetTable);
  TStrList loop_data;
  bool parse_header = true;
  if( Lines[start].IndexOf(' ') != InvalidIndex )  {
    TStrList toks;
    CIFToks(Lines[start], toks);
    for( size_t i=1; i < toks.Count(); i++ )  {
      if( !parse_header || toks[i].CharAt(0) != '_' )  {
        parse_header = false;
        loop_data.Add(toks[i]);
      }
      else
        table.data.AddCol(toks[i]);
    }
  }
  while( parse_header )  {  // skip loop definition
    if( ++start >= Lines.Count() )  {  // // end of file?
      context.current_block->Add(table.GetName(), &table);
      return true;
    }
    if( Lines[start].IsEmpty() )  continue;
    if( Lines[start].CharAt(0) != '_' )  {  start--;  break;  }
    if( table.data.ColCount() != 0 )  {
      if( olxstr::CommonString(Lines[start], table.data.ColName(0)).Length() == 1 )  {
        context.current_block->Add(table.GetName(), &table);
        start--;  // rewind
        return true;
      }
    }
    bool param_found = false;  // in the case loop header is mixed up with loop data...
    if( Lines[start].IndexOf(' ') == InvalidIndex )
      table.data.AddCol(Lines[start]);
    else  {
      TStrList toks;
      CIFToks(Lines[start], toks);
      for( size_t i=0; i < toks.Count(); i++ )  {
        if( param_found || toks[i].CharAt(0) != '_' )  {
          param_found = true;
          loop_data.Add(toks[i]);
        }
        else
          table.data.AddCol(toks[i]);
      }
    }
    if( param_found )  break;
  }
  size_t q_cnt = 0;
  while( true )  {  // skip loop data
    while( ++start < Lines.Count() && Lines[start].IsEmpty() )  continue;
    if( start >= Lines.Count() )  break;
    // a new loop or dataset started (not a part of a multi-string value)
    if( (q_cnt%2) == 0 && (Lines[start].StartsFrom('_') || 
      Lines[start].StartsFromi("loop_") || Lines[start].StartsFromi("data_")) )
      break;
    if( Lines[start].CharAt(0) == ';' )  q_cnt++;
    loop_data.Add(Lines[start]);
  }
  
  table.DataFromStrings(loop_data);
  context.current_block->Add(table.GetName(), &table);
  start--;
  return true;
}
//..............................................................................
void TCifDP::LoadFromStrings(const TStrList& Strings)  {
  Clear();
  TStrList Lines = Strings;
  for( size_t i=0; i < Lines.Count(); i++ )  {
    if( Lines[i].StartsFrom(';') )  {  // skip these things
      while( ++i < Lines.Count() && !Lines[i].StartsFrom(';') )  continue;
      continue;
    }
    Lines[i].DeleteSequencesOf<char>(' ').Trim(' ');
  }
  parse_context context(Lines);
  for( size_t i=0; i < Lines.Count(); i++ )  {
    const olxstr& line = Lines[i];
    if( line.IsEmpty() )  continue;
    if( line.CharAt(0) == '#')  {
      if( context.current_block == NULL )
        context.current_block = &Add("anonymous");
      context.current_block->Add(EmptyString, new cetComment(line));
      continue;
    }
    if( ExtractLoop(i, context) )  continue;
    if( line.CharAt(0) == '_' )  {  // parameter
      if( context.current_block == NULL )
        context.current_block = &Add("anonymous");
      TStrList toks;
      CIFToks(line, toks);
      if( toks.Count() >= 3 && toks[2].CharAt(0) == '#' )  {
        context.current_block->Add(toks[0],
          new cetCommentedString(toks[0], toks[1], toks[2].SubStringFrom(1)));
        toks.DeleteRange(0, 3);
      }
      else if( toks.Count() == 2 && toks[1].CharAt(0) != '#' )  {
        context.current_block->Add(toks[0], new cetNamedString(toks[0], toks[1]));
        toks.DeleteRange(0, 2);
      }
      else  {  // string list
        cetNamedStringList* list = NULL;
        if( toks.Count() == 2 && toks[1].CharAt(0) == '#' )  {
          list = new cetCommentedNamedStringList(toks[0], toks[1].SubStringFrom(1));
          toks.DeleteRange(0, 2);
        }
        else if( toks.Count() == 1 )  {
          list = new cetNamedStringList(toks[0]);
          toks.Delete(0);
        }
        else
          throw 1;
        context.current_block->Add(list->name, list);
        while( ++i < Lines.Count() && Lines[i].IsEmpty() )  continue;
        if( i >= Lines.Count() )  continue;
        olxch Char = Lines[i].CharAt(0);
        while( Char == '#' && ++i < Lines.Count() )  {
          context.current_block->Add(EmptyString, new cetComment(line));
          while( Lines[i].IsEmpty() && ++i < Lines.Count() )  continue;
          if( i >= Lines.Count() )  break;
          Char = Lines[i].CharAt(0);
        }
        if( Char == ';' )  {
          size_t sc_count = 1; 
          if( Lines[i].Length() > 1 )
            list->lines.Add(Lines[i].SubStringFrom(1));
          while( ++i < Lines.Count() )  {
            if( !Lines[i].IsEmpty() && Lines[i].CharAt(0) == ';' )  {
              break;
            }
            list->lines.Add(Lines[i]);
          }
        }
        else if( Char = '\'' || Char == '"' )  {
          list->lines.Add(Lines[i]);
          continue;
        }
      }
    }
    else if( line.StartsFrom("data_") )
      context.current_block = &Add(line.SubStringFrom(5));
  }
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
cetTable::~cetTable()  {
  for( size_t i=0; i < data.RowCount(); i++ )
    for( size_t j=0; j < data.ColCount(); j++ )
      delete data[i][j];
}
void cetTable::ToStrings(TStrList& list) const {
  list.Add("loop_");
  for( size_t i=0; i < data.ColCount(); i++ )  // loop header
    list.Add("  ") << data.ColName(i);
  for( size_t i=0; i < data.RowCount(); i++ ) {  // loop content
    list.Add(EmptyString);
    for( size_t j=0; j < data.ColCount(); j++ )
      data[i][j]->ToStrings(list);
  }
}
olxstr cetTable::GetName() const {
  if( data.ColCount() == 0 )  return EmptyString;
  if( data.ColCount() == 1 )  return data.ColName(0);
  olxstr C = olxstr::CommonString(data.ColName(0), data.ColName(1));
  for( size_t i=2; i < data.ColCount(); i++ )
    C = olxstr::CommonString(data.ColName(i), C);
  if( C.IsEmpty() )
    throw TFunctionFailedException(__OlxSourceInfo, "Mismatching loop columns");
  if( C.Last() == '_' )  C.SetLength(C.Length()-1);
  return C;
}
void cetTable::DataFromStrings(TStrList& lines)  {
  if( data.ColCount() == 0 )  return;
  TPtrList<ICifEntry> cells;
  TStrList toks;
  for( size_t i=0; i < lines.Count(); i++ )  {
    if( lines[i].IsEmpty() || lines[i].CharAt(0) == '#' )  continue;
    if( lines[i].StartsFrom(';') )  {
      cetStringList* cell = (cetStringList*)cells.Add(new cetStringList);
      if( lines[i].Length() > 1 )
        cell->lines.Add(lines[i].SubStringFrom(1));
      while( ++i < lines.Count() && !lines[i].StartsFrom(';') )
        cell->lines.Add(lines[i]);
      cell->lines.TrimWhiteCharStrings();
      continue;
    }
    toks.Clear();
    TCifDP::CIFToks(lines[i], toks);
    for( size_t j=0; j < toks.Count(); j++ )
      cells.Add( new cetString(toks[j]));
  }
  if( (cells.Count() % data.ColCount()) != 0 )  {
    for( size_t i=0; i < cells.Count(); i++ )  // clean up the memory
      delete cells[i];
    throw TFunctionFailedException(__OlxSourceInfo, 
      olxstr("Wrong number of parameters in '") << GetName() << "' loop");
  }
  const size_t ColCount = data.ColCount();
  const size_t RowCount = cells.Count()/ColCount;
  data.SetRowCount(RowCount);
  for( size_t i=0; i < RowCount; i++ )  {
    for( size_t j=0; j < ColCount; j++ )
      data[i][j] = cells[i*ColCount+j];
  }
}
//.............................................................................
cetString::cetString(const olxstr& _val) : value(_val), quoted(false)  {
  if( _val.Length() > 1 )  {
    const olxch ch = _val[0];
    if( (ch == '\'' || ch == '"') && _val.EndsWith(ch) )  {
      value = _val.SubStringFrom(1,1);
      quoted = true;
    }
  }
}
void cetString::ToStrings(TStrList& list) const {
  if( quoted )  {
    if( list.IsEmpty() || (list.Last().String.Length() + value.Length() + 3 > 80) )
      list.Add(" '") << value << '\'';
    else
      list.Last().String << " '" << value << '\'';
  }
  else  {
    if( list.IsEmpty() || (list.Last().String.Length() + value.Length() + 1 > 80) )
      list.Add(' ') << value;
    else
      list.Last().String << ' ' << value;
  }
}
//..............................................................................
void cetNamedString::ToStrings(TStrList& list) const {
  olxstr& tmp = list.Add(name);
  tmp.Format(34, true, ' ');
  if( quoted )
    tmp << '\'' << value << '\'';
  else
    tmp << value;
}
//..............................................................................
CifBlock::~CifBlock()  {
  for( size_t i=0; i < params.Count(); i++ )
    delete params.GetObject(i);
}
ICifEntry& CifBlock::Add(const olxstr& pname, ICifEntry* p)  {
  if( pname.IsEmpty() )  {  // only comments are allowed to have not name
    if( !EsdlInstanceOf(*p, cetComment) )
      throw TInvalidArgumentException(__OlxSourceInfo, "name");
    return *params.Add(pname, p).Object;
  }
  const size_t i = param_map.IndexOf(pname);
  if( i == InvalidIndex )  {
    param_map.Add(pname, p);
    params.Add(pname, p);
    if( EsdlInstanceOf(*p, cetTable) )
      table_map.Add(pname, (cetTable*)p); 
  }
  else  {
    const size_t ti = table_map.IndexOf(pname);
    if( ti != InvalidIndex )  {
      if( EsdlInstanceOf(*p, cetTable) )
        table_map.GetValue(ti) = (cetTable*)p;
      else  {
        table_map.Delete(ti);
        TBasicApp::GetLog().Warning(olxstr("Changing table type for ") << pname);
      }
    }
    const size_t oi = params.IndexOf(pname);
    delete params.GetObject(oi);
    params.GetObject(oi) = p;
    param_map.GetValue(i) = p;
  }
  return *p;
}
void CifBlock::ToStrings(TStrList& list) const {
  list.Add("data_") << name;
  for( size_t i=0; i < params.Count(); i++ )
    params.GetObject(i)->ToStrings(list);
}
void CifBlock::Format()  {
  for( size_t i=0; i < params.Count(); i++ )
    params.GetObject(i)->Format();    
}
//..............................................................................
//..............................................................................
//..............................................................................
//..............................................................................
//..............................................................................
