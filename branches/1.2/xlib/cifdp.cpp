/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "cifdp.h"
#include "bapp.h"
#include "log.h"
#include "etime.h"
#include "bitarray.h"

using namespace exparse::parser_util;
using namespace cif_dp;
const olxstr cetString::empty_value("''");

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
    context.current_block = &Add(EmptyString());
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
      else  {
        try  {  table.AddCol(toks[i]);  }
        catch(...)  {
          throw ParsingException(__OlxSourceInfo, "invalid table definition", start);
        }
      }
    }
  }
  while( parse_header )  {  // skip loop definition
    if( ++start >= Lines.Count() )  {  // end of file?
      context.current_block->Add(table);
      return true;
    }
    if (Lines[start].IsEmpty() || Lines[start].StartsFrom('#'))
      continue;
    if (!Lines[start].StartsFrom('_')) {
      start--;
      break;
    }
    bool param_found = false;  // in the case loop header is mixed up with loop data...
    if( Lines[start].IndexOf(' ') == InvalidIndex )
      table.AddCol(Lines[start]);
    else  {
      TStrList toks;
      CIFToks(Lines[start], toks);
      for( size_t i=0; i < toks.Count(); i++ )  {
        if (toks[i].CharAt(0) == '#') continue;
        if (param_found || !toks[i].StartsFrom('_')) {
          param_found = true;
          loop_data.Add(toks[i]);
        }
        else
          table.AddCol(toks[i]);
      }
    }
    if( param_found )  break;
  }
  size_t q_cnt = 0;
  while( true )  {  // skip loop data
    while( ++start < Lines.Count() && Lines[start].IsEmpty() )  continue;
    if( start >= Lines.Count() )  break;
    // a new loop or dataset started (not a part of a multi-string value)
    if( (q_cnt%2) == 0 && IsLoopBreaking(Lines[start]) )
      break;
    if( Lines[start].CharAt(0) == ';' )  q_cnt++;
    loop_data.Add(Lines[start]);
  }

  try  {
    table.DataFromStrings(loop_data);
    context.current_block->Add(table);
  }
  catch(const TExceptionBase& e)  {
    delete &table;
    throw ParsingException(__OlxSourceInfo, e, olxstr("starting at line #") << start);
  }
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
    Lines[i].Replace('\t', ' ').DeleteSequencesOf<char>(' ').Trim(' ');
  }
  parse_context context(Lines);
  for( size_t i=0; i < Lines.Count(); i++ )  {
    const olxstr& line = Lines[i];
    if( line.IsEmpty() )  continue;
    if( line.CharAt(0) == '#')  {
      if( context.current_block == NULL )
        context.current_block = &Add(EmptyString());
      context.current_block->Add(new cetComment(line.SubStringFrom(1)));
      continue;
    }
    if( ExtractLoop(i, context) )  continue;
    const size_t src_line = i;
    if( line.CharAt(0) == '_' )  {  // parameter
      if( context.current_block == NULL )
        context.current_block = &Add(EmptyString());
      TStrList toks;
      CIFToks(line, toks);
      if( toks.Count() >= 3 && toks[2].CharAt(0) == '#' )  {
        context.current_block->Add(new cetCommentedString(toks[0], toks[1], toks[2].SubStringFrom(1)));
        toks.DeleteRange(0, 3);
      }
      else if( toks.Count() == 2 && toks[1].CharAt(0) != '#' )  {
        context.current_block->Add(new cetNamedString(toks[0], toks[1]));
        toks.DeleteRange(0, 2);
      }
      else  {  // string or list
        while( ++i < Lines.Count() && Lines[i].IsEmpty() )  continue;
        if( i >= Lines.Count() )  continue;
        olxch Char = Lines[i].CharAt(0);
        while( Char == '#' && ++i < Lines.Count() )  {
          while( Lines[i].IsEmpty() && ++i < Lines.Count() )  continue;
          if( i >= Lines.Count() )  break;
          Char = Lines[i].CharAt(0);
          if( Char == '#' )
            context.current_block->Add(new cetComment(Lines[i].SubStringFrom(1)));
        }
        if( Char == ';' )  {
          cetNamedStringList* list = NULL;
          if( toks.Count() >= 2 && toks[1].CharAt(0) == '#' )  {
            list = new cetCommentedNamedStringList(toks[0], toks.Text(' ', 1).SubStringFrom(1));
            toks.DeleteRange(0, 2);
          }
          else if( toks.Count() == 1 )  {
            list = new cetNamedStringList(toks[0]);
            toks.Delete(0);
          }
          else
            throw ParsingException(__OlxSourceInfo, "invalid multi line parameter syntax", src_line);
          context.current_block->Add(list);
          if( Lines[i].Length() > 1 )
            list->lines.Add(Lines[i].SubStringFrom(1));
          while( ++i < Lines.Count() )  {
            if( !Lines[i].IsEmpty() && Lines[i].CharAt(0) == ';' )  {
              break;
            }
            list->lines.Add(Lines[i]);
          }
        }
        else if( Char == '\'' || Char == '"' )  {
          cetNamedString* str = NULL;
          if( toks.Count() == 2 && toks[1].CharAt(0) == '#' )  {
            str = new cetCommentedString(toks[0], Lines[i], toks[1].SubStringFrom(1));
            toks.DeleteRange(0, 2);
          }
          else if( toks.Count() == 1 )  {
            str = new cetNamedString(toks[0], Lines[i]);
            toks.Delete(0);
          }
          else
            throw ParsingException(__OlxSourceInfo, "invalid single line parameter syntax", src_line);
          context.current_block->Add(str);
        }
      }
    }
    else if( line.StartsFromi("data_") ) {
      olxstr dn = line.SubStringFrom(5);
      if (data_map.HasKey(dn)) {
        TBasicApp::NewLogEntry(logError) <<
          "Duplicate CIF data name '" << dn << '\'' << " auto renaming...";
        olxstr new_name = dn + '1';
        size_t idx=1;
        while (data_map.HasKey(new_name))
          new_name = dn + olxstr(++idx);
        dn = new_name;
        TBasicApp::NewLogEntry(logInfo) << "New name: " << dn;
      }
      context.current_block = &Add(dn);
    }
    else if( line.StartsFromi("save_" ) )  {
      if( line.Length() > 5 )  {
        context.current_block = &Add(line.SubStringFrom(5),
          context.current_block == NULL ? &Add(EmptyString()) : context.current_block);
      }  // close the block
      else if( context.current_block != NULL && context.current_block->parent != NULL )
        context.current_block = context.current_block->parent;
      else
        ; // should be error
    }
    else if( line.StartsFrom(';') )  {
      throw ParsingException(__OlxSourceInfo, "unnamed text block", i);
    }
    else if( line.StartsFrom('\'') )  {
      throw ParsingException(__OlxSourceInfo, "unnamed text string", i);
    }
  }
  Format();
}
//..............................................................................
TStrList& TCifDP::SaveToStrings(TStrList& Strings) const {
  for( size_t i=0; i < data.Count(); i++ )
    data[i].ToStrings(Strings);
  return Strings;
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
    else if( ch == '#' )  {
      out.Add(exp.SubStringFrom(start));
      start = exp.Length();
      break;
    }
  }
  if( start < exp.Length() )
    out.Add(exp.SubStringFrom(start).TrimWhiteChars());
  return out.Count() - toks_c;
}
//.............................................................................
cetTable::cetTable(const cetTable& v) : name(v.name), data(v.data) {
  for( size_t i=0; i < data.RowCount(); i++ )
    for( size_t j=0; j < data.ColCount(); j++ )
      data[i][j] = v.data[i][j]->Replicate();
}
ICifEntry& cetTable::Set(size_t i, size_t j, ICifEntry* v)  {
  delete data[i][j];
  return *(data[i][j] = v);
}
void cetTable::AddCol(const olxstr& col_name)  {
  data.AddCol(col_name);
  if( data.ColCount() == 1 )  {
    name = col_name;
  }
  else  {
    name = data.ColName(0).CommonSubString(data.ColName(1));
    size_t min_len = olx_min(data.ColName(0).Length(), data.ColName(1).Length());
    for( size_t i=2; i < data.ColCount(); i++ )  {
      olxstr n = data.ColName(i).CommonSubString(name);
      if( n.Length() > 1 )  {
        if( data.ColName(i).Length() < min_len )
          min_len = data.ColName(i).Length();
        name = n;
      }
    }
    if( name.Length() != min_len )  {  // line _geom_angle and geom_angle_etc
      const size_t u_ind = name.LastIndexOf('_');
      if( u_ind != InvalidIndex && u_ind != 0 )
        name.SetLength(u_ind);
      else  {
        const size_t d_ind = name.LastIndexOf('.');
        if( d_ind != InvalidIndex && d_ind != 0 )
          name.SetLength(d_ind);
      }
    }
    if (name.IsEmpty()) {
      throw TFunctionFailedException(__OlxSourceInfo, "mismatching loop columns");
    }
  }
}

void cetTable::DelRow(size_t idx) {
  if (idx >= data.RowCount()) {
    return;
  }
  for (size_t i = 0; i < data.ColCount(); i++) {
    delete data[idx][i];
  }
  data.DelRow(idx);
}

bool cetTable::DelCol(size_t idx) {
  if (idx >= data.ColCount()) {
    return false;
  }
  for (size_t i = 0; i < data.RowCount(); i++) {
    delete data[i][idx];
  }
  data.DelCol(idx);
  // shall we update the table name here?
  return true;
}

cetTable::cetTable(const olxstr& cols, size_t row_count)  {
  const TStrList toks(cols, ',');
  for (size_t i = 0; i < toks.Count(); i++) {
    AddCol(toks[i]);
  }
  if (row_count != InvalidSize) {
    data.SetRowCount(row_count);
  }
}
void cetTable::Clear()  {
  for (size_t i = 0; i < data.RowCount(); i++) {
    data[i].DeleteItems();
  }
  data.Clear();
}
void cetTable::ToStrings(TStrList& list) const {
  if (data.RowCount() == 0) {
    return;
  }
  TStrList out;
  out.Add("loop_");
  for (size_t i = 0; i < data.ColCount(); i++) { // loop header
    out.Add("  ") << data.ColName(i);
  }
  for (size_t i = 0; i < data.RowCount(); i++) {  // loop content
    bool saveable = true;
    for (size_t j = 0; j < data.ColCount(); j++) {
      if (!data[i][j]->IsSaveable()) {
        saveable = false;
        break;
      }
    }
    if (!saveable) {
      continue;
    }
    out.Add();
    for (size_t j = 0; j < data.ColCount(); j++) {
      data[i][j]->ToStrings(out);
    }
  }
  if (out.Count() == data.ColCount() + 1) { // no content is added
    return;
  }
  list.AddList(out);
  list.Add();  // add an empty string after loop for better formating
}
void cetTable::DataFromStrings(TStrList& lines)  {
  if (data.ColCount() == 0) {
    return;
  }
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
      if( i < lines.Count() && lines[i].Length() > 1 )  {
        lines[i] = lines[i].SubStringFrom(1);
        i--;
      }
      cell->lines.TrimWhiteCharStrings();
      continue;
    }
    toks.Clear();
    TCifDP::CIFToks(lines[i], toks);
    for( size_t j=0; j < toks.Count(); j++ )  {
      if( toks[j].CharAt(0) != '#' )
        cells.Add(new cetString(toks[j]));
    }
  }
  if( (cells.Count() % data.ColCount()) != 0 )  {
    for( size_t i=0; i < cells.Count(); i++ )  // clean up the memory
      delete cells[i];
    throw TFunctionFailedException(__OlxSourceInfo,
      olxstr("wrong number of parameters in '") << GetName() << "' loop");
  }
  const size_t ColCount = data.ColCount();
  const size_t RowCount = cells.Count()/ColCount;
  data.SetRowCount(RowCount);
  for( size_t i=0; i < RowCount; i++ )  {
    for( size_t j=0; j < ColCount; j++ )
      data[i][j] = cells[i*ColCount+j];
  }
}
int cetTable::TableSorter::Compare_(const CifRow &r1, const CifRow &r2) const {
  const size_t sz = r1.Count();
  size_t cmpb_cnt = 0;
  for( size_t i=0; i < sz; i++ )
    if( r1.GetItem(i)->GetCmpHash() != InvalidIndex )
      cmpb_cnt++;
  if( cmpb_cnt == 3 )  {  // special case for sorting angles...
    int cmps[3] = {0, 0, 0};
    cmpb_cnt = 0;
    for( size_t i=0; i < sz; i++ )  {
      size_t h1 = r1.GetItem(i)->GetCmpHash();
      if( h1 == InvalidIndex )  continue;
      cmps[cmpb_cnt] = olx_cmp(h1, r2.GetItem(i)->GetCmpHash());
      if( ++cmpb_cnt >= 3 )
        break;
    }
    if( cmps[1] == 0 )  {
      if( cmps[0] == 0 )
        return cmps[2];
      return cmps[0];
    }
    return cmps[1];
  }
  else  {
    for( size_t i=0; i < sz; i++ )  {
      size_t h1 = r1.GetItem(i)->GetCmpHash();
      h1 = (h1 == InvalidIndex) ? 0 : h1;
      size_t h2 = r2.GetItem(i)->GetCmpHash();
      h2 = (h2 == InvalidIndex) ? 0 : h2;
      if( h1 < h2 )  return -1;
      if( h1 > h2 )  return 1;
    }
  }
  return 0;
}
//.............................................................................
void cetTable::Sort()  {
  if( data.RowCount() == 0 )  return;
  bool update = false;
  for( size_t i=0; i < data.ColCount(); i++ )  {
    if( data[0][i]->GetCmpHash() != InvalidIndex )  {
      update = true;
      break;
    }
  }
  if( !update  )  return;
  data.SortRows(TableSorter());
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
  if( !quoted && value.IndexOf(' ') != InvalidIndex )
    quoted = true;
}
void cetString::ToStrings(TStrList& list) const {
  olxstr& line =
    (list.IsEmpty() ||
     (list.GetLastString().Length() + value.Length() + 3 > 80) ||
     list.GetLastString().StartsFrom(';')) ?
    list.Add(' ') : (list.GetLastString() << ' ');
  if( quoted )
    line << '\'' << value << '\'';
  else
    line << (value.IsEmpty() ? empty_value : value);
}
//..............................................................................
void cetNamedString::ToStrings(TStrList& list) const {
  if( olx_max(34, name.Length()) + value.Length() > 78 )  {
    list.Add(name);
    if( quoted )
      list.Add('\'') << value << '\'';
    else
      list.Add(value.IsEmpty() ? empty_value : value);
  }
  else  {
    olxstr& tmp = list.Add(name);
    tmp.RightPadding(34, ' ', true);
    if( quoted )
      tmp << '\'' << value << '\'';
    else
      tmp << (value.IsEmpty() ? empty_value : value);
  }
}
//..............................................................................
CifBlock::CifBlock(const CifBlock& v)  {
  for( size_t i=0; i < v.params.Count(); i++ )  {
    param_map.Add(v.params[i], v.params.GetObject(i));
    params.Add(v.params[i], v.params.GetObject(i));
    if( EsdlInstanceOf(*v.params.GetObject(i), cetTable) )
      table_map.Add(v.params[i], (cetTable*)v.params.GetObject(i));
  }
}
CifBlock::~CifBlock()  {
  for( size_t i=0; i < params.Count(); i++ )
    delete params.GetObject(i);
}
ICifEntry& CifBlock::Add(ICifEntry* p)  {
  // only comments are allowed to have not name
  if( !p->HasName() || p->GetName().IsEmpty() )  {
    if( !EsdlInstanceOf(*p, cetComment) )
      throw TInvalidArgumentException(__OlxSourceInfo, "name");
    return *params.Add(EmptyString(), p).Object;
  }
  const olxstr& pname = p->GetName();
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
        TBasicApp::NewLogEntry(logWarning) << "Changing table type for " <<
          pname;
      }
    }
    const size_t oi = params.IndexOfi(pname);
    delete params.GetObject(oi);
    params.GetObject(oi) = p;
    param_map.GetValue(i) = p;
  }
  return *p;
}
bool CifBlock::Delete(size_t idx)  {
  if( idx == InvalidIndex )  return false;
  const size_t ti = table_map.IndexOf(param_map.GetKey(idx));
  if( ti != InvalidIndex )
    table_map.Delete(ti);
  const size_t oi = params.IndexOfi(param_map.GetKey(idx));
  delete params.GetObject(oi);
  params.Delete(oi);
  param_map.Delete(idx);
  return true;
}
void CifBlock::Rename(const olxstr& old_name, const olxstr& new_name,
  bool replace_if_exists)
{
  const size_t i = param_map.IndexOf(old_name);
  if (i == InvalidIndex) return;
  ICifEntry* val = param_map.GetValue(i);
  if (!val->HasName()) return;
  const size_t ni = param_map.IndexOf(new_name);
  if (ni != InvalidIndex) {
    if (!replace_if_exists) {
      Delete(i);
      return;
    }
    else {
      Delete(ni);
    }
  }
  try { val->SetName(new_name); }
  catch(...)  {  return;  }  // read only name?
  param_map.Delete(i);
  param_map.Add(new_name, val);
  const size_t ti = table_map.IndexOf(old_name);
  if( ti != InvalidIndex )  {
    table_map.Delete(ti);
    table_map.Add(new_name, (cetTable*)val);
  }
  const size_t oi = params.IndexOfi(old_name);
  params[oi] = new_name;
}
void CifBlock::ToStrings(TStrList& list) const {
  if( !name.IsEmpty() )
    (parent != NULL ? list.Add("save_") : list.Add("data_")) << name;
  for( size_t i=0; i < params.Count(); i++ )
    params.GetObject(i)->ToStrings(list);
  if( parent != NULL && !name.IsEmpty() )
    list.Add("save_");
}
void CifBlock::Format()  {
  for( size_t i=0; i < params.Count(); i++ )
    params.GetObject(i)->Format();
}
//..............................................................................
void CifBlock::Sort(const TStrList& pivots, const TStrList& endings)  {
  TTypeList<CifBlock::EntryGroup> groups;
  for( size_t i=0; i < params.Count(); i++ )  {
    CifBlock::EntryGroup& eg = groups.AddNew();
    while( i < params.Count() && EsdlInstanceOf(*params.GetObject(i), cetComment) )  {
      eg.items.Add(params.GetObject(i++));
    }
    if( i < params.Count() )  {
      eg.items.Add(params.GetObject(i));
      eg.name = params.GetObject(i)->GetName();
    }
  }
  QuickSorter::Sort(groups, CifSorter(pivots, endings));
  params.Clear();
  for( size_t i=0; i < groups.Count(); i++ )  {
    for( size_t j=0; j < groups[i].items.Count()-1; j++ )
      params.Add(EmptyString(), groups[i].items[j]);
    params.Add(groups[i].name, groups[i].items.GetLast());
  }
}
//.............................................................................
int CifBlock::CifSorter::Compare_(const CifBlock::EntryGroup &e1,
  const CifBlock::EntryGroup &e2) const
{
  size_t c1=InvalidIndex, c2=InvalidIndex, c1_l=0, c2_l=0;
  for( size_t i=0; i < pivots.Count(); i++ )  {
    if( c1 == InvalidIndex && e1.name.StartsFromi(pivots[i]) )  {
      if( pivots[i].Length() > c1_l )  {
        c1 = i;
        c1_l = pivots[i].Length();
      }
    }
    if( c2 == InvalidIndex && e2.name.StartsFromi(pivots[i]) )  {
      if( pivots[i].Length() > c2_l )  {
        c2 = i;
        c2_l = pivots[i].Length();
      }
    }
  }
  if( c1 == c2 )  {
    if( e1.name.Length() == e2.name.Length() )  {
      size_t s1=InvalidIndex, s2=InvalidIndex, s1_l=0, s2_l=0;
      for( size_t i=0; i < endings.Count(); i++ )  {
        if( s1 == InvalidIndex && e1.name.EndsWithi(endings[i]) )  {
          if( endings[i].Length() > s1_l )  {
            s1 = i;
            s1_l = endings[i].Length();
          }
        }
        if( s2 == InvalidIndex && e2.name.EndsWithi(endings[i]) )  {
          if( endings[i].Length() > s2_l )  {
            s2 = i;
            s2_l = endings[i].Length();
          }
        }
      }
      if( s1 == s2 )
        return e1.name.Comparei(e2.name);
      return olx_cmp(s1, s2);
    }
    else
       return e1.name.Comparei(e2.name);
  }
  return olx_cmp(c1, c2);
}
//.............................................................................
