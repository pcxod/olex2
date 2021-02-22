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
#include "wildcard.h"
#include "eset.h"
#include "efile.h"

using namespace exparse::parser_util;
using namespace cif_dp;

//..............................................................................
//..............................................................................
//..............................................................................
void TCifDP::Clear() {
  data.Clear();
  data_map.Clear();
}
//..............................................................................
void TCifDP::Format() {
  for (size_t i = 0; i < data.Count(); i++) {
    data[i].Format();
  }
}
//..............................................................................
void TCifDP::LoadFromStrings(const TStrList &lines) {
  LoadFromString(lines.Text('\n'));
}
//.............................................................................
void TCifDP::LoadFromStream(IInputStream & stream) {
  size_t sz = (size_t)stream.GetSize();
  olx_array_ptr<char> data = new char[sz];
  stream.Read(data, sz);
  LoadFromString(olxstr::FromUTF8(data, sz));
}
//..............................................................................
void TCifDP::LoadFromString(const olxstr &str_) {
  Clear();
  olxstr str = str_;
  if (str.StartsFrom("#\\#CIF_2.0")) {
    version = 2;
  }
  else {
    version = 1;
  }
  if (str.Contains('\r')) {
    if (str.Contains('\n')) {
      str = str.Replace("\r", "");
    }
    else {
      str = str.Replace("\r", "\n");
    }
  }
  TTypeList<CifToken> toks = TokenizeString(
    version == 2 ? str.SubStringFrom(10) : str, version);
  //TBasicApp::NewLogEntry() << toks;
  CifBlock *current_block = &Add(EmptyString());
  for (size_t i = 0; i < toks.Count(); i++) {
    const olxstr& line = toks[i].value;
    if (line.IsEmpty()) {
      continue;
    }
    if (line.CharAt(0) == '#') {
      current_block->Add(new cetComment(line.SubStringFrom(1)));
      continue;
    }
    if (line == "loop_") {
      TStrList comments, h_comments;
      olx_object_ptr<cetTable> t = new cetTable();
      while (++i < toks.Count() &&
        (toks[i].value.StartsFrom('_') || toks[i].value.StartsFrom('#')))
      {
        if (toks[i].value.StartsFrom('_')) {
          t->AddCol(toks[i].value);
        }
        else {
          h_comments.Add(toks[i].value);
        }
      }
      size_t st = i--;
      while (++i < toks.Count() && !IsLoopBreaking(toks[i].value)) {
        if (toks[i].value.StartsFrom('#')) {
          comments.Add(toks[i].value);
          toks.NullItem(i);
          continue;
        }
      }
      if (t->ColCount() == 0 || ((i - st - comments.Count()) % t->ColCount()) > 0) {
        throw ParsingException(__OlxSourceInfo,
          olxstr("invalid table ") << t->GetName(), st);
      }
      comments.AddAll(h_comments);
      for (size_t ii = st; ii < i;) {
        if (toks.IsNull(ii) || toks[ii].value.IsEmpty()) {
          ii++;
          continue;
        }
        CifRow &r = t->AddRow();
        for (size_t ij = 0; ij < t->ColCount(); ij++, ii++) {
          if (toks.IsNull(ii)) {
            ij--;
            continue;
          }
          r[ij] = ICifEntry::FromToken(toks[ii], version);
        }
      }
      if (t->RowCount() == 0) {
        TBasicApp::NewLogEntry(logWarning) << "Ignoring empty table "
          << t->GetName();
        continue;
      }
      current_block->Add(t.release());
      for (size_t ci = 0; ci < comments.Count(); ci++) {
        current_block->Add(new cetComment(comments[ci].SubStringFrom(1)));
      }
      i--;
      continue;
    }
    if (line.CharAt(0) == '_') {  // parameter
      if (i + 1 < toks.Count()) {
        // take first comment and skip others
        olxstr comment;
        if (toks[i + 1].value.StartsFrom('#')) {
          comment = toks[++i].value.SubStringFrom(1);
          while (++i < toks.Count() && toks[i].value.StartsFrom('#')) {
          }
          i--;
        }
        if (++i < toks.Count()) {
          ICifEntry *e = ICifEntry::FromToken(toks[i], version);
          e->SetName(line);
          if (!comment.IsEmpty()) {
            e->SetComment(comment);
          }
          current_block->Add(e);
          continue;
        }
      }
      throw ParsingException(__OlxSourceInfo,
        olxstr("missing value for ") << line, 0);
    }
    else if (line.StartsFromi("data_")) {
      olxstr dn = line.SubStringFrom(5);
      if (data_map.HasKey(dn)) {
        TBasicApp::NewLogEntry(logError) <<
          "Duplicate CIF data name '" << dn << '\'' << " auto renaming...";
        olxstr new_name = dn + '1';
        size_t idx = 1;
        while (data_map.HasKey(new_name)) {
          new_name = dn + olxstr(++idx);
        }
        dn = new_name;
        TBasicApp::NewLogEntry(logInfo) << "New name: " << dn;
      }
      if (current_block->GetName().IsEmpty() && current_block->params.IsEmpty()) {
        current_block->SetName(dn);
      }
      else {
        current_block = &Add(dn);
      }
    }
    else if (line.StartsFromi("save_")) {
      if (line.Length() > 5) {
        current_block = &Add(line.SubStringFrom(5), current_block);
      }  // close the block
      else if (current_block->parent != 0) {
        current_block = current_block->parent;
      }
      else {
        ; // should be error
      }
    }
    else if (line.StartsFrom(';')) {
      throw ParsingException(__OlxSourceInfo, "unnamed text block", toks[i].lineNumber);
    }
    else if (line.StartsFrom('\'')) {
      throw ParsingException(__OlxSourceInfo, "unnamed text string", toks[i].lineNumber);
    }
  }
  Format();
}
//..............................................................................
TStrList& TCifDP::SaveToStrings(TStrList& Strings) const {
  for (size_t i = 0; i < data.Count(); i++) {
    data[i].ToStrings(Strings);
  }
  return Strings;
}
//..............................................................................
size_t TCifDP::LineIndexer::GetLineNumber(size_t idx) {
  ln += str.SubString(lastPos, idx - lastPos).CharCount('\n');
  lastPos = idx;
  return ln;
};

TTypeList<CifToken>::const_list_type TCifDP::TokenizeString(const olxstr &str_,
  int version)
{
  olxstr str = str_;
  TTypeList<CifToken> toks;
  size_t start = 0;
  LineIndexer lni(str);
  for (size_t i = 0; i < str.Length(); i++) {
    olxch ch = str.CharAt(i);
    if (is_quote(ch) &&
      (i == 0 || olxstr::o_iswhitechar(str.CharAt(i - 1)) || str.CharAt(i - 1) == '\n'))
    {
      if (i + 2 < str.Length() && str.CharAt(i + 1) == ch && str.CharAt(i + 2) == ch) {
        size_t idx = str.FirstIndexOf(olxstr::CharStr(ch, 3), i + 3);
        if (idx == InvalidIndex) {
          throw ParsingException(__OlxSourceInfo, "unbalanced quotation",
            lni.GetLineNumber(i));
        }
        toks.Add(
          new CifToken(olxstr().quote(ch) << str.SubString(i + 3, idx - i - 3),
            lni.GetLineNumber(i)));
        i = idx + 3;
        start = i + 1;
      }
      else {
        size_t st = i + 1;
        while (++i < str.Length()) {
          if ((str.CharAt(i) == ch && ((i + 1) >= str.Length() ||
            olxstr::o_iswhitechar(str.CharAt(i + 1)) ||
            str.CharAt(i + 1) == '\n' ||
            (version == 2 && str.CharAt(i + 1) == ':'))))
          {
            break;
          }
        }
        if (str.CharAt(i) != ch) {
          i--;
        }
        toks.Add(
          new CifToken(olxstr().quote(ch) << str.SubString(st, i - st),
            lni.GetLineNumber(i)));
        if (version ==2 && (i + 1) < str.Length() &&
          str.CharAt(i) == ch && str.CharAt(i + 1) == ':')
        {
          i++;
        }
      }
      start = i + 1;
    }
    else if (olxstr::o_iswhitechar(ch) || ch == '\n') {
      if (start == i) { // white chars cannot define empty args
        start = i + 1;
        continue;
      }
      toks.Add(
        new CifToken(str.SubString(start, i - start).TrimWhiteChars(),
          lni.GetLineNumber(start)));
      start = i + 1;
    }
    else if (ch == ';' && (i == 0 || str.CharAt(i - 1) == '\n')) {
      if (i == 0) {
        throw ParsingException(__OlxSourceInfo, "invalid start of file",
          lni.GetLineNumber(i));
      }
      size_t idx = str.FirstIndexOf("\n;", i + 1);
      if (idx == InvalidIndex) {
        throw ParsingException(__OlxSourceInfo, "unbalanced quotation",
          lni.GetLineNumber(i));
      }
      toks.Add(
        new CifToken(str.SubString(i, idx - i + 2),
          lni.GetLineNumber(i)));
      i = idx + 2;
      start = i + 1;
    }
    else if (ch == '#' && (i==0 || olxstr::o_isoneof(str.CharAt(i-1), " \t\n"))) {
      size_t idx = str.FirstIndexOf('\n', i + 1);
      if (idx == InvalidIndex) {
        idx = str.Length();
      }
      toks.Add(
        new CifToken(str.SubString(start, idx - start),
          lni.GetLineNumber(start)));
      i = idx;
      start = i + 1;
    }
    else if (start == i && version == 2) {
      if (ch == '[') {
        toks.Add(
          new CifToken(ExtractBracketedData(str, '[', ']', i),
            lni.GetLineNumber(i)));

        start = i + 1;
      }
      else if (ch == '{') {
        toks.Add(
          new CifToken(ExtractBracketedData(str, '{', '}', i),
            lni.GetLineNumber(i)));
        start = i + 1;
      }
    }
  }
  if (start < str.Length()) {
    toks.Add(
      new CifToken(str.SubStringFrom(start).TrimWhiteChars(),
        lni.GetLineNumber(start)));
  }
  return toks;
}
//.............................................................................
olxstr TCifDP::ExtractBracketedData(const olxstr &str, olxch open, olxch close,
  size_t &i_)
{
  if (str.CharAt(i_) != open) {
    throw TInvalidArgumentException(__OlxSourceInfo, "data");
  }
  size_t oc = 0;
  olxch qch = 0;
  for (size_t i = i_; i < str.Length(); i++) {
    olxch ch = str.CharAt(i);
    if (is_quote(ch)) {
      if (qch == ch) {
        qch = 0;
      }
      else {
        qch = ch;
      }
      continue;
    }
    if (qch == 0) {
      if (ch == open) {
        oc++;
      }
      else if (ch == ';' && i > 0 && str.CharAt(i-1) == '\n') {
        while (++i < str.Length()) {
          if (str.CharAt(i) == ';' && str.CharAt(i - 1) == '\n') {
            break;
          }
        }
      }
      else if (ch == close) {
        if (--oc == 0) {
          olxstr rv = str.SubString(i_, i - i_+1);
          i_ = i;
          return rv;
        }
      }
    }
  }
  throw TInvalidArgumentException(__OlxSourceInfo, "data");
}
//.............................................................................
CifBlock& TCifDP::Add(const olxstr& name, CifBlock* parent) {
  const size_t i = data_map.IndexOf(name);
  if (i != InvalidIndex) {
    return *data_map.GetValue(i);
  }
  else {
    return *data_map.Add(name, &data.Add(new CifBlock(name, parent)));
  }
}
//.............................................................................
size_t TCifDP::IndexOf(const CifBlock& cb) const {
  const CifBlock* cb_ptr = &cb;
  for (size_t i = 0; i < data.Count(); i++)
    if (&data[i] == cb_ptr) {
      return i;
    }
  return InvalidIndex;
}
//.............................................................................
void TCifDP::Rename(const olxstr& old_name, const olxstr& new_name) {
  if (old_name == new_name) {
    return;
  }
  if (data_map.HasKey(new_name)) {
    throw TInvalidArgumentException(__OlxSourceInfo,
      olxstr("Name already in use: ") << new_name);
  }
  const size_t cb_ind = data_map.IndexOf(old_name);
  if (cb_ind == InvalidIndex) {
    throw TInvalidArgumentException(__OlxSourceInfo,
      olxstr("Undefined block: ") << old_name);
  }
  CifBlock* cb = data_map.GetValue(cb_ind);
  cb->name = new_name;
  data_map.Delete(cb_ind);
  data_map.Add(new_name, cb);
}
//.............................................................................
//.............................................................................
//.............................................................................
cetTable::cetTable(const cetTable& v)
  : ICifEntry(v), data(v.data)
{
  for (size_t i = 0; i < data.RowCount(); i++) {
    for (size_t j = 0; j < data.ColCount(); j++) {
      data[i][j] = v.data[i][j]->Replicate();
    }
  }
}
//.............................................................................
ICifEntry& cetTable::Set(size_t i, size_t j, ICifEntry* v)  {
  delete data[i][j];
  return *(data[i][j] = v);
}
//.............................................................................
void cetTable::AddCol(const olxstr& col_name) {
  if (data.ColIndex(col_name) != InvalidIndex) {
    return;
  }
  data.AddCol(col_name);
  if (data.ColCount() == 1) {
    ICifEntry::SetName(col_name);
  }
  else {
    olxstr nn = data.ColName(0).CommonSubString(data.ColName(1));
    size_t min_len = olx_min(data.ColName(0).Length(), data.ColName(1).Length());
    for (size_t i = 2; i < data.ColCount(); i++) {
      olxstr n = data.ColName(i).CommonSubString(nn);
      if (n.Length() > 1) {
        if (data.ColName(i).Length() < min_len) {
          min_len = data.ColName(i).Length();
        }
        nn = n;
      }
    }
    if (nn.Length() != min_len) {  // line _geom_angle and geom_angle_etc
      const size_t u_ind = nn.LastIndexOf('_');
      if (u_ind != InvalidIndex && u_ind != 0) {
        nn.SetLength(u_ind);
      }
      else {
        const size_t d_ind = nn.LastIndexOf('.');
        if (d_ind != InvalidIndex && d_ind != 0) {
          nn.SetLength(d_ind);
        }
      }
    }
    if (nn.IsEmpty()) {
      throw TFunctionFailedException(__OlxSourceInfo, "mismatching loop columns");
    }
    ICifEntry::SetName(nn);
  }
}
//.............................................................................
void cetTable::DelRow(size_t idx) {
  if (idx >= data.RowCount()) {
    return;
  }
  for (size_t i = 0; i < data.ColCount(); i++) {
    delete data[idx][i];
  }
  data.DelRow(idx);
}
//.............................................................................
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
//.............................................................................
cetTable::cetTable(const olxstr& cols, size_t row_count)  {
  const TStrList toks(cols, ',');
  for (size_t i = 0; i < toks.Count(); i++) {
    AddCol(toks[i]);
  }
  if (row_count != InvalidSize) {
    data.SetRowCount(row_count);
  }
}
//.............................................................................
void cetTable::Clear() {
  for (size_t i = 0; i < data.RowCount(); i++) {
    data[i].DeleteItems();
  }
  data.Clear();
}
//.............................................................................
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
  list.AddAll(out);
  list.Add();  // add an empty string after loop for better formating
}
//.............................................................................
int cetTable::TableSorter::Compare_(const CifRow &r1, const CifRow &r2) const {
  const size_t sz = r1.Count();
  size_t cmpb_cnt = 0;
  for (size_t i = 0; i < sz; i++) {
    if (r1.GetItem(i)->GetCmpHash() != InvalidIndex) {
      cmpb_cnt++;
    }
  }
  if (cmpb_cnt == 3) {  // special case for sorting angles...
    int cmps[3] = { 0, 0, 0 };
    cmpb_cnt = 0;
    for (size_t i = 0; i < sz; i++) {
      size_t h1 = r1.GetItem(i)->GetCmpHash();
      if (h1 == InvalidIndex) {
        continue;
      }
      cmps[cmpb_cnt] = olx_cmp(h1, r2.GetItem(i)->GetCmpHash());
      if (++cmpb_cnt >= 3) {
        break;
      }
    }
    if (cmps[1] == 0) {
      if (cmps[0] == 0) {
        return cmps[2];
      }
      return cmps[0];
    }
    return cmps[1];
  }
  else {
    for (size_t i = 0; i < sz; i++) {
      size_t h1 = r1.GetItem(i)->GetCmpHash();
      h1 = (h1 == InvalidIndex) ? 0 : h1;
      size_t h2 = r2.GetItem(i)->GetCmpHash();
      h2 = (h2 == InvalidIndex) ? 0 : h2;
      if (h1 < h2) {
        return -1;
      }
      if (h1 > h2) {
        return 1;
      }
    }
  }
  return 0;
}
//.............................................................................
void cetTable::Sort() {
  if (data.RowCount() == 0) {
    return;
  }
  bool update = false;
  for (size_t i = 0; i < data.ColCount(); i++) {
    if (data[0][i]->GetCmpHash() != InvalidIndex) {
      update = true;
      break;
    }
  }
  if (!update) {
    return;
  }
  data.SortRows(TableSorter());
}
void cetTable::SetName(const olxstr& nn) {
  for (size_t i = 0; i < data.ColCount(); i++) {
    data.ColName(i) = nn + data.ColName(i).SubStringFrom(name->Length());
  }
  this->name = nn;
}
//.............................................................................
//.............................................................................
//.............................................................................
void cetString::SetValue_(const olxstr &val) {
  value = val;
  if (val.Length() > 1) {
    const olxch ch = val[0];
    if ((ch == '\'' || ch == '"') && val.EndsWith(ch)) {
      value = val.SubStringFrom(1, 1);
      quoted = true;
    }
  }
  if (!quoted && (value.IsEmpty() || value.ContainAnyOf(" \t"))) {
    quoted = true;
  }
}
//.............................................................................
void cetString::ToStrings(TStrList& list) const {
  size_t qsz = quoted ? 3 : 0;
  if (HasName()) {
    list.Add(GetName()).RightPadding(34, ' ', true);
  }
  if (value.Contains('\n')) {
    qsz = 7;
  }
  olxstr& line =
    (list.IsEmpty() ||
     (list.GetLastString().Length() + value.Length() + qsz > 80) ||
     list.GetLastString().StartsFrom(';')) ?
    list.Add(' ') : (list.GetLastString() << ' ');
  if (quoted) {
    if (qsz == 3) {
      size_t qidx = InvalidIndex;
      bool use_dq = false, use_ml = false;
      // check if ' will be valid in this case
      while ((qidx = value.FirstIndexOf('\'', qidx)) != InvalidIndex) {
        if (++qidx < value.Length() &&
          olxstr::o_isoneof(value.CharAt(qidx), " \t"))
        {
          use_dq = true;
          break;
        }
      }
      // check if " will be valid in this case
      if (use_dq) {
        while ((qidx = value.FirstIndexOf('"', qidx)) != InvalidIndex) {
          if (++qidx < value.Length() &&
            olxstr::o_isoneof(value.CharAt(qidx), " \t"))
          {
            use_ml = true;
            break;
          }
        }
      }
      if (use_ml) {
        if (line.Length() == 1) {
          line[0] = ';';
          if (value.StartsFrom(';')) {
            list.Add(' ') << value;
          }
          else {
            list.Add(value);
          }
          list.Add(';');
        }
      }
      else if (use_dq) {
        line << '"' << value << '"';
      }
      else {
        line << '\'' << value << '\'';
      }
    }
    else {
      line << "'''" << value << "'''";
    }
  }
  else {
    line << (value.IsEmpty() ? GetEmptyValue() : value);
  }
  if (HasComment()) {
    line << " # " << *comment;
  }
}
//..............................................................................
//..............................................................................
//..............................................................................
void cetStringList::ToStrings(TStrList& list) const {
  if (comment.ok()) {
    list.Add("#") << *comment;
  }
  if (name.ok()) {
    list.Add(*name);
  }
  list.Add(';');
  list.AddAll(lines);
  list.Add(';');
}
//..............................................................................
olxstr cetStringList::GetStringValue() const {
  return olxstr(NewLineSequence()).Join(lines);
}
//..............................................................................
//..............................................................................
//..............................................................................
CifBlock::CifBlock(const CifBlock& v) {
  for (size_t i = 0; i < v.params.Count(); i++) {
    param_map.Add(v.params[i], v.params.GetObject(i));
    params.Add(v.params[i], v.params.GetObject(i));
    if (v.params.GetObject(i)->Is<cetTable>()) {
      table_map.Add(v.params[i], (cetTable*)v.params.GetObject(i));
    }
  }
}
//..............................................................................
CifBlock::~CifBlock() {
  for (size_t i = 0; i < params.Count(); i++) {
    delete params.GetObject(i);
  }
}
//..............................................................................
ICifEntry& CifBlock::Add(ICifEntry* p) {
  // only comments are allowed to have not name
  if (!p->HasName() || p->GetName().IsEmpty()) {
    if (!p->Is<cetComment>()) {
      throw TInvalidArgumentException(__OlxSourceInfo, "name");
    }
    return *params.Add(EmptyString(), p).Object;
  }
  const olxstr& pname = p->GetName();
  const size_t i = param_map.IndexOf(pname);
  if (i == InvalidIndex) {
    param_map.Add(pname, p);
    params.Add(pname, p);
    if (p->Is<cetTable>()) {
      table_map.Add(pname, (cetTable*)p);
    }
  }
  else {
    const size_t ti = table_map.IndexOf(pname);
    if (ti != InvalidIndex) {
      if (p->Is<cetTable>()) {
        table_map.GetValue(ti) = (cetTable*)p;
      }
      else {
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
//..............................................................................
bool CifBlock::Delete(size_t idx) {
  if (idx == InvalidIndex) {
    return false;
  }
  const size_t ti = table_map.IndexOf(param_map.GetKey(idx));
  if (ti != InvalidIndex) {
    table_map.Delete(ti);
  }
  const size_t oi = params.IndexOfi(param_map.GetKey(idx));
  delete params.GetObject(oi);
  params.Delete(oi);
  param_map.Delete(idx);
  return true;
}
//..............................................................................
void CifBlock::Rename(const olxstr& old_name, const olxstr& new_name,
  bool replace_if_exists)
{
  size_t st_idx = old_name.IndexOf('*');
  if (st_idx != InvalidIndex) {
    if (old_name.Length() < 2 ||
      !new_name.EndsWith('*') || new_name.Length() < 2)
    {
      return;
    }
    olxset<olxstr, olxstrComparator<true> > to_skip;
    TStrList toks(old_name, ",");
    for (size_t i = 1; i < toks.Count(); i++) {
      to_skip.Add(toks[i]);
    }
    olxstr oname = toks[0].SubString(0, toks[0].Length() - 1),
      nname = new_name.SubString(0, new_name.Length() - 1);
    olxstr_dict<olxstr> names;
    for (size_t i = 0; i < param_map.Count(); i++) {
      if (param_map.GetKey(i).StartsFrom(oname) &&
        !to_skip.Contains(param_map.GetKey(i)))
      {
        names.Add(param_map.GetKey(i),
          nname + param_map.GetKey(i).SubStringFrom(oname.Length()));
      }
    }
    for (size_t i = 0; i < names.Count(); i++) {
      Rename(names.GetKey(i), names.GetValue(i));
    }
    return;
  }
  const size_t idx = param_map.IndexOf(old_name);
  if (idx == InvalidIndex) {
    return;
  }
  ICifEntry* val = param_map.GetValue(idx);
  if (!val->HasName()) {
    return;
  }
  const size_t ni = param_map.IndexOf(new_name);
  if (ni != InvalidIndex) {
    if (!replace_if_exists) {
      Delete(idx);
      return;
    }
    else {
      Delete(ni);
    }
  }
  try {
    val->SetName(new_name);
  }
  catch (...) { // read only name?
    if (val->Is<cetTable>()) {
      cetTable* t = dynamic_cast<cetTable *>(val);
      cetTable *nt = new cetTable();
      for (size_t i = 0; i < t->ColCount(); i++) {
        nt->AddCol(new_name + t->ColName(i).SubStringFrom(old_name.Length()));
      }
      for (size_t i = 0; i < t->RowCount(); i++) {
        CifRow &r = nt->AddRow();
        for (size_t j = 0; j < t->ColCount(); j++) {
          r[j] = (*t)[i][j];
          (*t)[i][j] = 0;
        }
      }
      table_map.Delete(table_map.IndexOf(old_name));
      delete t;
      table_map.Add(new_name, nt);
      param_map.Delete(idx);
      param_map.Add(new_name, nt);
      const size_t oi = params.IndexOfi(old_name);
      params[oi] = new_name;
      params.GetObject(oi) = nt;
    }
    return;
  }
  param_map.Delete(idx);
  param_map.Add(new_name, val);
  const size_t oi = params.IndexOfi(old_name);
  params[oi] = new_name;
}
//..............................................................................
void CifBlock::ToStrings(TStrList& list) const {
  if (!GetName().IsEmpty()) {
    (parent != 0 ? list.Add("save_") : list.Add("data_")) << GetName();
  }
  for (size_t i = 0; i < params.Count(); i++) {
    params.GetObject(i)->ToStrings(list);
  }
  if (parent != 0 && !GetName().IsEmpty()) {
    list.Add("save_");
  }
}
//..............................................................................
void CifBlock::Format() {
  for (size_t i = 0; i < params.Count(); i++) {
    params.GetObject(i)->Format();
  }
}
//..............................................................................
void CifBlock::Sort(const TStrList& pivots, const TStrList& endings) {
  TTypeList<CifBlock::EntryGroup> groups;
  for (size_t i = 0; i < params.Count(); i++) {
    CifBlock::EntryGroup& eg = groups.AddNew();
    while (i < params.Count() &&
      params.GetObject(i)->Is<cetComment>())
    {
      eg.items.Add(params.GetObject(i++));
    }
    if (i < params.Count()) {
      eg.items.Add(params.GetObject(i));
      eg.name = params.GetObject(i)->GetName();
    }
  }
  QuickSorter::Sort(groups, CifSorter(pivots, endings));
  params.Clear();
  for (size_t i = 0; i < groups.Count(); i++) {
    for (size_t j = 0; j < groups[i].items.Count() - 1; j++) {
      params.Add(EmptyString(), groups[i].items[j]);
    }
    params.Add(groups[i].name, groups[i].items.GetLast());
  }
}
//.............................................................................
int CifBlock::CifSorter::Compare_(const CifBlock::EntryGroup &e1,
  const CifBlock::EntryGroup &e2) const
{
  size_t c1 = InvalidIndex, c2 = InvalidIndex, c1_l = 0, c2_l = 0;
  for (size_t i = 0; i < pivots.Count(); i++) {
    olxstr p = pivots[i];
    bool table = p.EndsWith('#');
    if (table) {
      p.SetLength(p.Length() - 1);
    }
    if (e1.name.StartsFromi(p)) {
      if (!table || (table && e1.items.GetLast()->Is<cetTable>())) {
        // use original - it is longer for tables!
        if (pivots[i].Length() > c1_l) {
          c1 = i;
          c1_l = pivots[i].Length();
        }
      }
    }
    if (e2.name.StartsFromi(p)) {
      if (!table || (table && e2.items.GetLast()->Is<cetTable>())) {
        // use original - it is longer for tables!
        if (pivots[i].Length() > c2_l) {
          c2 = i;
          c2_l = pivots[i].Length();
        }
      }
    }
  }
  if (c1 == c2) {
    if (e1.name.Length() == e2.name.Length()) {
      size_t s1 = InvalidIndex, s2 = InvalidIndex, s1_l = 0, s2_l = 0;
      for (size_t i = 0; i < endings.Count(); i++) {
        if (s1 == InvalidIndex && e1.name.EndsWithi(endings[i])) {
          if (endings[i].Length() > s1_l) {
            s1 = i;
            s1_l = endings[i].Length();
          }
        }
        if (s2 == InvalidIndex && e2.name.EndsWithi(endings[i])) {
          if (endings[i].Length() > s2_l) {
            s2 = i;
            s2_l = endings[i].Length();
          }
        }
      }
      if (s1 == s2) {
        return e1.name.Comparei(e2.name);
      }
      return olx_cmp(s1, s2);
    }
    else {
      return e1.name.Comparei(e2.name);
    }
  }
  return olx_cmp(c1, c2);
}
//.............................................................................
//.............................................................................
//.............................................................................
cetList::cetList(const cetList &l) : ICifEntry(l) {
  data.SetCount(l.data.Count());
  for (size_t i = 0; i < l.data.Count(); i++) {
    data[i] = l.data[i]->Replicate();
  }
}
//.............................................................................
cetList::~cetList() {
  data.DeleteItems();
}
//.............................................................................
void cetList::ToStrings(TStrList& list) const {
  if (comment.ok()) {
    list.Add("#") << *comment;
  }
  if (name.ok()) {
    list.Add(*name);
  }
  if (data.IsEmpty()) {
    if (list.IsEmpty() || (list.GetLastString().Length() + 3) > 80) {
      list.Add("");
    }
    else {
      list.GetLastString() << ' ';
    }
    list.GetLastString() << '[' << ']';
    return;
  }
  if (list.IsEmpty() || (list.GetLastString().Length() + 2) > 80) {
    list.Add("");
  }
  else {
    list.GetLastString() << ' ';
  }
  list.GetLastString() << '[';
  for (size_t i = 0; i < data.Count(); i++) {
    data[i]->ToStrings(list);
  }
  if (list.GetLastString().Length() + 2 > 80) {
    list.Add("");
  }
  else {
    list.GetLastString() << ' ';
  }
  list.GetLastString() << "]";
}
//.............................................................................
void cetList::FromToken(const CifToken &token) {
  TTypeList<CifToken> toks = TCifDP::TokenizeString(
    token.value.SubStringFrom(1, 1), 2);
  for (size_t i = 0; i < toks.Count(); i++) {
    data.Add(ICifEntry::FromToken(toks[i], 2));
  }
}
//.............................................................................
//.............................................................................
//.............................................................................
cetDict::cetDict(const cetDict &d) : ICifEntry(d) {
  data.SetCapacity(d.data.Count());
  for (size_t i = 0; i < d.data.Count(); i++) {
    data.Add(new cetDict::dict_item_t(
     (IStringCifEntry *)d.data[i].a->Replicate(), d.data[i].b->Replicate()));
  }
}
//.............................................................................
cetDict::~cetDict() {
  for (size_t i = 0; i < data.Count(); i++) {
    delete data[i].a;
    delete data[i].b;
  }
}
//.............................................................................
void cetDict::ToStrings(TStrList& list) const {
  if (comment.ok()) {
    list.Add("#") << *comment;
  }
  if (name.ok()) {
    list.Add(name);
  }
  if (data.IsEmpty()) {
    if (list.IsEmpty() || (list.GetLastString().Length() + 3) > 80) {
      list.Add("");
    }
    list.GetLastString() << ' ' << '{' << '}';
    return;
  }
  olxstr& line =
    (list.IsEmpty() || (list.GetLastString().Length() + 2 > 80)) ?
    list.Add(' ') : (list.GetLastString() << ' ');
  line << '{';
  for (size_t i = 0; i < data.Count(); i++) {
    data[i].a->ToStrings(list);
    list.GetLastString() << ':';
    data[i].b->ToStrings(list);
  }
  if (list.GetLastString().Length() + 1 > 80) {
    list.Add("");
  }
  list.GetLastString() << '}';
}
//.............................................................................
void cetDict::FromToken(const CifToken &token) {
  TTypeList<CifToken> toks = TCifDP::TokenizeString(
    token.value.SubStringFrom(1,1), 2);
  if ((toks.Count() % 2) != 0) {
    throw ParsingException(__OlxSourceInfo,
      "even number of items is expected for a map", token.lineNumber);
  }
  for (size_t i = 0; i < toks.Count(); i+=2) {
    data.Add(
      new cetDict::dict_item_t(
        new cetString(toks[i].value, true),
        ICifEntry::FromToken(toks[i+1], 2))
    );
  }
}
//.............................................................................
//.............................................................................
//.............................................................................
ICifEntry *ICifEntry::FromToken(const CifToken &t, int version) {
  if (t.value.IsEmpty()) {
    return new cetString(EmptyString());
  }
  if (version == 1) {
    switch (t.value.CharAt(0)) {
    case ';':
    {
      if (t.value.Length() > 1 && t.value.EndsWith(';')) {
        olx_object_ptr<cetStringList> l = new cetStringList();
        l->lines.Strtok(t.value.SubStringFrom(1, 1), '\n', false);
        if (t.value.CharAt(1) == '\n') {
          l->lines.Delete(0);
        }
        return l.release();
      }
      else {
        return new cetString(t.value, true);
      }
    }
    case '#':
    {
      cetComment *c = new cetComment(t.value.SubStringFrom(1));
      return c;
    }
    default:
      return new cetString(t.value);
    }
  }
  else {
    switch (t.value.CharAt(0)) {
    case '[':
    {
      olx_object_ptr<cetList> l = new cetList();
      l->FromToken(t);
      return l.release();
    }
    case '{':
    {
      olx_object_ptr<cetDict> d = new cetDict();
      d->FromToken(t);
      return d.release();
    }
    case ';':
    {
      if (t.value.Length() > 1 && t.value.EndsWith(';')) {
        olx_object_ptr<cetStringList> l = new cetStringList();
        l->lines.Strtok(t.value.SubStringFrom(1, 1), '\n', false);
        if (t.value.CharAt(1) == '\n') {
          l->lines.Delete(0);
        }
        return l.release();
      }
      else {
        return new cetString(t.value, true);
      }
    }
    case '#':
    {
      cetComment *c = new cetComment(t.value.SubStringFrom(1));
      return c;
    }
    default:
      return new cetString(t.value);
    }
  }
}
