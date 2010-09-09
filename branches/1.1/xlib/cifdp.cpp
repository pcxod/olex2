//---------------------------------------------------------------------------//
// CIF data provider
// (c) Oleg V. Dolomanov, 2010
//---------------------------------------------------------------------------//
#include "cifdp.h"
#include "bapp.h"
#include "log.h"
#include "etime.h"
#include "bitarray.h"

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
    context.current_block = &Add(EmptyString);
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
    if( Lines[start].IsEmpty() )  continue;
    if( Lines[start].CharAt(0) != '_' )  {  start--;  break;  }
    bool param_found = false;  // in the case loop header is mixed up with loop data...
    if( Lines[start].IndexOf(' ') == InvalidIndex )
      table.AddCol(Lines[start]);
    else  {
      TStrList toks;
      CIFToks(Lines[start], toks);
      for( size_t i=0; i < toks.Count(); i++ )  {
        if( param_found || toks[i].CharAt(0) != '_' )  {
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
  
  try  {  table.DataFromStrings(loop_data);  }
  catch(const TExceptionBase& e)  {
    throw ParsingException(__OlxSourceInfo, e);
  }
  context.current_block->Add(table);
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
        context.current_block = &Add(EmptyString);
      context.current_block->Add(new cetComment(line.SubStringFrom(1)));
      continue;
    }
    if( ExtractLoop(i, context) )  continue;
    const size_t src_line = i;
    if( line.CharAt(0) == '_' )  {  // parameter
      if( context.current_block == NULL )
        context.current_block = &Add(EmptyString);
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
          context.current_block->Add(new cetComment(line));
          while( Lines[i].IsEmpty() && ++i < Lines.Count() )  continue;
          if( i >= Lines.Count() )  break;
          Char = Lines[i].CharAt(0);
        }
        if( Char == ';' )  {
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
    else if( line.StartsFromi("data_") )
      context.current_block = &Add(line.SubStringFrom(5));
    else if( line.StartsFromi("save_" ) )  {
      if( line.Length() > 5 )  {
        context.current_block = &Add(line.SubStringFrom(5),
          context.current_block == NULL ? &Add(EmptyString) : context.current_block);
      }  // close the block
      else if( context.current_block != NULL && context.current_block->parent != NULL )
        context.current_block = context.current_block->parent;
      else
        ; // should be error
    }
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
      name = data.ColName(i).CommonSubString(name);
      if( data.ColName(i).Length() < min_len )
        min_len = data.ColName(i).Length();
    }
    if( name.IsEmpty() )
      throw TFunctionFailedException(__OlxSourceInfo, "mismatching loop columns");
    if( name.Length() != min_len )  {  // lihe _geom_angle and geom_angle_etc
      const size_t u_ind = name.LastIndexOf('_');
      if( u_ind != InvalidIndex )
        name.SetLength(u_ind);
    }
  }
}
void cetTable::Clear()  {
  for( size_t i=0; i < data.RowCount(); i++ )
    data[i].Delete();
  data.Clear();
}
void cetTable::ToStrings(TStrList& list) const {
  if( data.RowCount() == 0 )  return;
  TStrList out;
  out.Add("loop_");
  for( size_t i=0; i < data.ColCount(); i++ )  // loop header
    out.Add("  ") << data.ColName(i);
  for( size_t i=0; i < data.RowCount(); i++ ) {  // loop content
    bool saveable = true;
    for( size_t j=0; j < data.ColCount(); j++ )  {
      if( !data[i][j]->IsSaveable() )  {
        saveable = false;
        break;
      }
    }
    if( !saveable )  continue;
    out.Add();
    for( size_t j=0; j < data.ColCount(); j++ )
      data[i][j]->ToStrings(out);
  }
  if( out.Count() == data.ColCount() +1 )  // no content is added
    return;
  list.AddList(out);
  list.Add();  // add an empty string after loop for better formating
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
      cells.Add(new cetString(toks[j]));
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
int cetTable::TableSorter::Compare(const CifRow* r1, const CifRow* r2)  {
  const size_t sz = r1->Count();
  for( size_t i=0; i < sz; i++ )  {
    size_t h1 = r1->Item(i)->GetCmpHash();
    h1 = (h1 == InvalidIndex) ? 0 : h1;
    size_t h2 = r2->Item(i)->GetCmpHash();
    h2 = (h2 == InvalidIndex) ? 0 : h2;
    if( h1 < h2 )  return -1;
    if( h1 > h2 )  return 1;
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
  if( !p->HasName() || p->GetName().IsEmpty() )  {  // only comments are allowed to have not name
    if( !EsdlInstanceOf(*p, cetComment) )
      throw TInvalidArgumentException(__OlxSourceInfo, "name");
    return *params.Add(EmptyString, p).Object;
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
bool CifBlock::Remove(const olxstr& pname)  {
  const size_t i = param_map.IndexOf(pname);
  if( i == InvalidIndex )  return false;
  param_map.Delete(i);
  const size_t ti = table_map.IndexOf(pname);
  if( ti != InvalidIndex )
    table_map.Delete(ti);
  const size_t oi = params.IndexOf(pname);
  delete params.GetObject(oi);
  params.Delete(oi);
  return true;
}
void CifBlock::Rename(const olxstr& old_name, const olxstr& new_name)  {
  const size_t i = param_map.IndexOf(old_name);
  if( i == InvalidIndex )  return;
  ICifEntry* val = param_map.GetValue(i);
  param_map.Delete(i);
  param_map.Add(new_name, val);
  const size_t ti = table_map.IndexOf(old_name);
  if( ti != InvalidIndex )  {
    table_map.Delete(ti);
    table_map.Add(new_name, (cetTable*)val);
  }
  const size_t oi = params.IndexOf(old_name);
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
void CifBlock::Sort(const TStrList& pivots)  {
  static TStrList def_pivots(
    "_audit_creation,_publ,_chemical_name,_chemical_formula,_chemical,_atom_type,"
    "_space_group,_space_group_symop,_symmetry,"
    "_cell_length,_cell_angle,_cell_volume,_cell_formula,_cell,"
    "_exptl_,"
    "_diffrn,"
    "_reflns,"
    "_computing,"
    "_refine,"
    "_atom_sites,_atom_site,_atom_site_aniso,"
    "_geom_special,_geom_bond,_geom_angle,_geom,"
    "_smtbx"
    , 
    ',');
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
  groups.QuickSorter.Sort(groups, CifSorter(pivots.IsEmpty() ? def_pivots : pivots));
  params.Clear();
  for( size_t i=0; i < groups.Count(); i++ )  {
    for( size_t j=0; j < groups[i].items.Count()-1; j++ )
      params.Add(EmptyString, groups[i].items[j]);
    params.Add(groups[i].name, groups[i].items.Last());
  }
}
//.............................................................................
int CifBlock::CifSorter::Compare(const CifBlock::EntryGroup* e1, const CifBlock::EntryGroup* e2) const {
  size_t c1=InvalidIndex, c2=InvalidIndex, c1_l=0, c2_l=2;
  for( size_t i=0; i < pivots.Count(); i++ )  {
    if( c1 == InvalidIndex && e1->name.StartsFromi(pivots[i]) )  {
      if( pivots[i].Length() > c1_l )  {
        c1 = i;
        c1_l = pivots[i].Length();
      }
    }
    if( c2 == InvalidIndex && e2->name.StartsFromi(pivots[i]) )  {
      if( pivots[i].Length() > c2_l )  {
        c2 = i;
        c2_l = pivots[i].Length();
      }
    }
  }
  if( c1 == c2 )
    return e1->name.Comparei(e2->name);
  return olx_cmp(c1, c2);
}
//.............................................................................
