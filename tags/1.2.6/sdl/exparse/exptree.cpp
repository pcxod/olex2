/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "exptree.h"
#include "../paramlist.h"
using namespace esdl::exparse;

//.............................................................................
parser_util::operator_set::operator_set()
  : operators(operatorset_t::FromList(
  TStrList(
    "+ - * / %" // arithmetic
    " & ^ |" // bitwise
    " == != >= <= < >" // comparison
    " >> <<" // directional
    " ." // indirection
    " : ? ! && ||" // logical
    " = += -= /= *= &= |= ^= <<=", // assignment
    ' ')))
{
  update_charset();
}
//.............................................................................
void parser_util::operator_set::update_charset() {
  control_chars.Clear();
  for (size_t i=0; i < operators.Count(); i++) {
    for (size_t j=0; j < operators[i].Length(); j++)
      control_chars.AddUnique(operators[i].CharAt(j));
  }
}
//.............................................................................
bool parser_util::operator_set::parse_control_chars(const olxstr& exp,
  olxstr& dest, size_t& ind) const
{
  if (ind >= exp.Length()) return false;
  size_t st=ind;
  while (ind < exp.Length() && control_chars.Contains(exp.CharAt(ind))) {
    dest = exp.SubString(st, ++ind-st);
    if (!is_operator(dest)) {
      dest.SetLength(dest.Length()-1);
      ind--;
      break;
    }
  }
  return !dest.IsEmpty();
}
//.............................................................................
bool parser_util::operator_set::add_operator(const olxstr &opr) {
  if (!operators.AddUnique(opr).GetB()) return false;
  for (size_t i=0; i < opr.Length(); i++)
    control_chars.AddUnique(opr[i]);
  return true;
}
//.............................................................................
bool parser_util::operator_set::remove_operator(const olxstr &opr) {
  if (operators.Remove(opr)) {
    update_charset();
    return true;
  }
  return false;
}
//.............................................................................
//.............................................................................
//.............................................................................
bool parser_util::skip_string(const olxstr& exp, size_t& ind)  {
  const olxch qc = exp.CharAt(ind);
  const size_t start = ind;
  while( ++ind < exp.Length() )  {
    if( exp.CharAt(ind) == qc && !is_escaped(exp, ind) )
      return true;
  }
  ind = start;
  return false;
}
//.............................................................................
bool parser_util::parse_string(const olxstr& exp, olxstr& dest, size_t& ind)  {
  const olxch qc = exp.CharAt(ind);
  bool end_found = false;
  const size_t start = ind+1;
  while( ++ind < exp.Length() )  {
    if( exp.CharAt(ind) == qc && !is_escaped(exp, ind) )  {
      end_found = true;
      break;
    }
  }
  if( end_found )
    dest = exp.SubString(start, ind - start);
  return end_found;
}
//.............................................................................
bool parser_util::parse_escaped_string(const olxstr& exp, olxstr& dest,
  size_t& ind)
{
  const olxch qc = exp.CharAt(ind);
  bool end_found = false;
  const size_t start = ind+1;
  while( ++ind < exp.Length() )  {
    if( exp.CharAt(ind) == qc )  {
      end_found = true;
      break;
    }
  }
  if( end_found )
    dest = exp.SubString(start, ind - start);
  return end_found;
}
//.............................................................................
bool parser_util::skip_brackets(const olxstr& exp, size_t& ind)  {
  const olxch oc = exp.CharAt(ind),
    cc = get_closing_bracket(oc);
  int bc = 1;
  const size_t start = ind+1;
  while( ++ind < exp.Length() && bc != 0 )  {
    const olxch ch = exp.CharAt(ind);
    if( is_quote(ch) && !is_escaped(exp, ind) )  {
      if( !skip_string(exp, ind) )  return false;
      continue;
    }
    if( ch == cc && (--bc == 0) )  return true;
    else if( ch == oc )  bc++;
  }
  ind = start;
  return false;
}
//.............................................................................
size_t parser_util::skip_whitechars(const olxstr& exp, size_t& ind) {
  size_t el = exp.Length();
  if (ind >= el) return ind;
  while (olxstr::o_iswhitechar(exp[ind]) && ++ind < el) ;
  return ind;
}
//.............................................................................
bool parser_util::parse_brackets(const olxstr& exp, olxstr& dest,
  size_t& ind)
{
  const olxch oc = exp.CharAt(ind),
    cc = get_closing_bracket(oc);
  int bc = 1;
  const size_t start = ind+1;
  while( ++ind < exp.Length() && bc != 0 )  {
    const olxch ch = exp.CharAt(ind);
    if( ch == cc )       bc--;
    else if( ch == oc )  bc++;
  }
  if( bc == 0 )  {
    ind--;
    dest = exp.SubString(start, ind-start);
    return true;
  }
  return false;
}
//.............................................................................
bool parser_util::is_expandable(const olxstr& exp) {
  for (size_t i=0; i < exp.Length(); i++) {
    const olxch ch = exp.CharAt(i);
    if (is_bracket(ch)) return true;
    if (is_quote(ch)) {
      skip_string(exp, i);
      continue;
    }
    // macro style call 'cos 90'
    if (olxstr::o_iswhitechar(ch)) return true;
  }
  return false;
}
//.............................................................................
bool parser_util::is_expandable(const olxstr& exp, const operator_set &os) {
  for( size_t i=0; i < exp.Length(); i++ )  {
    const olxch ch = exp.CharAt(i);
    if (is_bracket(ch)) return true;
    if (is_quote(ch)) {
      skip_string(exp, i);
      continue;
    }
    if (os.is_control_char(ch)) return true;
    // macro style call 'cos 90'
    if (olxstr::o_iswhitechar(ch)) return true;
  }
  return false;
}
//.............................................................................
bool parser_util::is_escaped(const olxstr& exp, size_t ch_ind)  {
  size_t sc = 0;
  while( --ch_ind != InvalidIndex && exp.CharAt(ch_ind) == '\\' ) sc++;
  return (sc%2) != 0;
}
//.............................................................................
size_t parser_util::next_unescaped(olxch _what, const olxstr& _where,
  size_t _from)
{
  for( size_t i=_from; i < _where.Length(); i++ )
    if( _where.CharAt(i) == _what && !is_escaped(_where, i) )
      return i;
  return InvalidIndex;
}
//.............................................................................
olxstr parser_util::unescape(const olxstr& exp)  {
  //return exp;
  olxstr out(EmptyString(), exp.Length());
  for( size_t i=0; i < exp.Length(); i++ )  {
    if( exp.CharAt(i) == '\\' )  {
      if( ++i >= exp.Length() )  {
        out << '\\';
        break;
      }
      switch( exp.CharAt(i) )  {
        case 'r':   out << '\r';  break;
        case 'n':   out << '\n';  break;
        case 't':   out << '\t';  break;
        case '\\':  out << '\\';  break;
        case '\'':  out << '\'';  break;
        case '\"':  out << '\"';  break;
        default:    out << '\\' << exp.CharAt(i);  break;
      }
    }
    else
      out << exp.CharAt(i);
  }
  return out;
}
//.............................................................................
olxstr parser_util::escape(const olxstr& exp)  {
  //return exp;
  olxstr out(EmptyString(), exp.Length()+5);
  for( size_t i=0; i < exp.Length(); i++ )  {
    switch( exp.CharAt(i) )  {
      case '\r': case '\n': case '\t':case '\\': case '\'': case '\"':
        out << '\\' << exp.CharAt(i);
        break;
      default:
        out << exp.CharAt(i);
        break;
    }
  }
  return out;
}
//.............................................................................
//.............................................................................
//.............................................................................
void expression_tree::expand(const parser_util::operator_set &os)  {
  data.TrimWhiteChars();
  if( !parser_util::is_expandable(data, os) )  return;
  size_t dt_st=0;
  for( size_t i=0; i < data.Length(); i++ )  {
    olxch ch = data.CharAt(i);
    if( olxstr::o_iswhitechar(ch) &&
      !os.is_next_char_control(data, i))
    {
      if (dt_st == i) continue;
      olxstr dt = data.SubString(dt_st, i-dt_st);
      if ((parser_util::skip_whitechars(data, i)) >= data.Length())
        break;
      // check if 'def ttt ccc' syntax is used
      if( !os.is_next_char_control(data, i)) {
        TStrList args;
        TParamList::StrtokParams(data.SubStringFrom(i), ' ', args);
        evator = new evaluator<expression_tree>(dt);
        for( size_t ai=0; ai < args.Count(); ai++ )
          evator->args.Add(new expression_tree(this, args[ai]))->expand(os);
        break;
      }
    }
    else if( ch == '(' )  {  // parse out brackets
      olxstr dt = data.SubString(dt_st, i-dt_st).TrimWhiteChars();
      olxstr arg;
      if( !parser_util::parse_brackets(data, arg, i) ) {
        throw TInvalidArgumentException(__OlxSourceInfo,
          "problem with brackets");
      }
      arg.TrimWhiteChars();
      if( arg.IsEmpty() ) // empty arg list
        evator = new evaluator<expression_tree>(dt);
      else  {
        TStrList args;
        parser_util::split_args(arg, args);
        if( args.Count() == 1 )  {
          left = new expression_tree(this, arg);
          left->expand(os);
          if( !dt.IsEmpty() )  { // ()
            evator = new evaluator<expression_tree>(dt);
            evator->args.Add(left);
            left = NULL;
          }
          // nothing else? then move one level up...
          else if( i+1 >= data.Length() )  {
            expression_tree* eta = left;
            left = eta->left;
            right = eta->right;
            evator = eta->evator;
            if( left != NULL )  left->parent = this;
            if( right != NULL )  right->parent = this;
            data = eta->data;
            eta->left = eta->right = NULL;
            eta->evator = NULL;
            delete eta;
            priority = true;
          }
        }
        else  {
          evator = new evaluator<expression_tree>(dt);
          for( size_t ai=0; ai < args.Count(); ai++ )
            evator->args.Add(new expression_tree(this, args[ai]))->expand(os);
        }
      }
      dt_st = i+1;
    }
    else if( ch == '[' || ch == '{' )  {
      olxstr arg;
      size_t dt_e = i;
      if (!parser_util::parse_brackets(data, arg, i)) {
        throw TInvalidArgumentException(__OlxSourceInfo,
          "problem with brackets");
      }
      if ((dt_st < dt_e) || (parent != NULL && parent->data == '.'))  {
        if (dt_st < dt_e) {
          left = new expression_tree(this, data.SubString(dt_st, dt_e-dt_st));
          left->expand(os);
          right = new expression_tree(this, '.');
          right->evator = new evaluator<expression_tree>("_idx_");
          right->evator->args.Add(new expression_tree(this, arg))->expand(os);
          olxstr ra = data.SubStringFrom(i+1);
          data = '.';
          right->right = new expression_tree(right,
            ra.StartsFrom('.') ? ra.SubStringFrom(1) : ra);
          right->right->expand(os);
        }
        else {
          evator = new evaluator<expression_tree>("_idx_");
          evator->args.Add(new expression_tree(this, arg))->expand(os);
          olxstr ra = data.SubStringFrom(i+1);
          data = '.';
          right = new expression_tree(this,
            ra.StartsFrom('.') ? ra.SubStringFrom(1) : ra);
          right->expand(os);
        }
      }
      else {
        if (i+1 != data.Length())
          throw TInvalidArgumentException(__OlxSourceInfo, "list initialiser");
        else
          data = (olxstr(ch) << arg << (ch == '[' ? ']' : '}'));
      }
      break;
    }
    // parse out the strings
    else if( parser_util::is_quote(ch) && !parser_util::is_escaped(data, i) )  {
      dt_st = i;
      if (!parser_util::skip_string(data, i)) {
        throw TInvalidArgumentException(__OlxSourceInfo,
          "problem with quotations");
      }
    }
    else  {
      olxstr opr;
      size_t dt_e = i;
      if (os.parse_control_chars(data, opr, i))  {
        if (opr == '.')  {  // treat floating point values...
          bool number = false;
          size_t dec=1;
          if (i > 1 && olxstr::o_isdigit(data.CharAt(i-2))) { // 1.
            number = true;
            dec = 2;
          }
          if (!number && i < data.Length() && olxstr::o_isdigit(data.CharAt(i))) { // .1
            number = true;
          }
          if (number) {
            dt_st = (i-=dec);
            while (++i < data.Length() &&
              (olxstr::o_isdigit(data.CharAt(i)) || data.CharAt(i) == '.'))
            {
              ;
            }
            i--;
            continue;
          }
        }
        olxstr dt = data.SubString(dt_st, dt_e-dt_st).TrimWhiteChars();
        if (!dt.IsEmpty())  {
          if (left != NULL) {
            throw TInvalidArgumentException(__OlxSourceInfo,
              "invalid expression");
          }
          left = new expression_tree(this, dt);
          left->expand(os);
        }
        right = new expression_tree(this, data.SubStringFrom(i));
        data = opr;
        right->expand(os);
        break;
      }
    }
  }
}
//.............................................................................
void expression_tree::expand_cmd()  {
  data.TrimWhiteChars();
  data.Replace((olxch)1, ' ');
  if (parser_util::is_quoted(data)) {
    return;
  }
  else if (!parser_util::is_expandable(data)) {
    macro_call = true;
    return;
  }
  size_t dt_st=0;
  for (size_t i=0; i < data.Length(); i++) {
    olxch ch = data.CharAt(i);
    // only the first command can use the 'macro' syntax
    if (olxstr::o_iswhitechar(ch) && parent == NULL) {
      if (dt_st == i) continue;
      olxstr dt = data.SubString(dt_st, i-dt_st);
      if ((parser_util::skip_whitechars(data, i)) >= data.Length())
        break;
      TStrList args;
      TParamList::StrtokParams(data.SubStringFrom(i), ' ', args, false);
      evator = new evaluator<expression_tree>(dt);
      for (size_t ai=0; ai < args.Count(); ai++)
        evator->args.Add(new expression_tree(this, args[ai]))->expand_cmd();
      macro_call = true;
      dt_st = data.Length();
      data.SetLength(0);
      break;
    }
    else if (ch == '(') {  // parse out brackets
      olxstr dt = data.SubString(dt_st, i-dt_st);//.TrimWhiteChars();
      if (dt.IsNumber()) continue;
      olxstr arg;
      if( !parser_util::parse_brackets(data, arg, i) ) {
        throw TInvalidArgumentException(__OlxSourceInfo,
          "problem with brackets");
      }
      arg.TrimWhiteChars();
      size_t st=0, st_idx=InvalidIndex;
      while (++st_idx < dt.Length()) {
        if (!olxstr::o_isalphanumeric(dt.CharAt(st_idx)) &&
            !olxstr::o_isoneof(dt.CharAt(st_idx), '_', '.'))
        {
          st = st_idx+1;
        }
      }
      olxstr left_v = st > 0 ? dt.SubStringTo(st) : EmptyString();
      dt = dt.SubStringFrom(st);
      if (arg.IsEmpty()) // empty arg list
        evator = new evaluator<expression_tree>(dt);
      else {
        TStrList args;
        parser_util::split_args(arg, args);
        bool math_package = dt.StartsFromi("math.");
        if (args.Count() > 1 || math_package) {
          evator = new evaluator<expression_tree>(dt);
          for (size_t ai=0; ai < args.Count(); ai++) {
            expression_tree* et = evator->args.Add(
              new expression_tree(this, args[ai]));
            if (!math_package)
              et->expand_cmd();
          }
        }
        else { // 1 argument
          left = new expression_tree(this, arg);
          left->expand_cmd();
          if (!dt.IsEmpty())  { // !()
            evator = new evaluator<expression_tree>(dt);
            evator->args.Add(left);
            left = NULL;
          }
          // nothing else? then move one level up...
          else if (i+1 >= data.Length()) {
            expression_tree* eta = left;
            left = eta->left;
            right = eta->right;
            evator = eta->evator;
            if( left != NULL )  left->parent = this;
            if( right != NULL )  right->parent = this;
            data = eta->data;
            eta->left = eta->right = NULL;
            eta->evator = NULL;
            delete eta;
            priority = true;
          }
        }
      }
      if (!left_v.IsEmpty()) {
        if (left != NULL)
          throw TFunctionFailedException(__OlxSourceInfo, "invalid syntax");
        left = new expression_tree(this, left_v);
      }
      size_t r_start = ++i; // skip the closing bracket
      if ((parser_util::skip_whitechars(data, i)) < data.Length()) {
        if (right != NULL)
          TFunctionFailedException(__OlxSourceInfo, "invalid syntax");
        // preserve any white spaces if any
        olxstr dt = data.SubStringFrom(r_start);
        r_start = 0;
        while (r_start < dt.Length() &&
          olxstr::o_iswhitechar(dt.CharAt(r_start)))
        {
          dt[r_start] = (olxch)1;
          r_start++;
        }
        right = new expression_tree(this, dt);
        right->expand_cmd();
      }
      if (evator == NULL) {
        priority = true;
        data.SetLength(0);
      }
      dt_st = data.Length();
      // done here
      data.SetLength(0);
      break;
    }
    else if (ch == '=') {
      if (i+1 < data.Length() && olxstr::o_iswhitechar(data.CharAt(i+1)) ||
        data.CharAt(i+1) == '=')
      {
        continue;
      }
      olxstr dt = data.SubString(dt_st, i-dt_st).TrimWhiteChars();
      bool valid_name = true;
      for (size_t idx=0; idx < dt.Length(); idx++) {
        if (olxstr::o_isalpha(dt.CharAt(idx))) continue;
        if (olxstr::o_isoneof(dt.CharAt(idx), "_.") && idx != 0) continue;
        if (dt.CharAt(idx) == '-' && idx == 0) continue;
        valid_name = false;
        break;
      }
      if (!valid_name)
        continue;
      if (dt_st < i)  {
        if (left != NULL) {
          throw TInvalidArgumentException(__OlxSourceInfo,
            "invalid expression");
        }
        left = new expression_tree(this, dt);
        left->expand_cmd();
      }
      else {
        throw TInvalidArgumentException(__OlxSourceInfo,
          "invalid assignment");
      }
      right = new expression_tree(this, data.SubStringFrom(i+1));
      data = ch;
      right->expand_cmd();
      dt_st = data.Length();
      break;
    }
    // parse out the strings
    else if (parser_util::is_quote(ch) && !parser_util::is_escaped(data, i)) {
      if (!parser_util::skip_string(data, i)) {
        throw TInvalidArgumentException(__OlxSourceInfo,
          "problem with quotations");
      }
    }
  }
  if (dt_st != 0 && dt_st < data.Length()) {
    if (right != NULL)
      throw TFunctionFailedException(__OlxSourceInfo, "invalid syntax");
    right = new expression_tree(this, data.SubStringFrom(dt_st));
  }
}
//.............................................................................
olxstr_buf expression_tree::ToStringBuffer() const {
  olxstr_buf rv;
  if (priority) rv << '(';
  if (left != NULL)
    rv << left->ToStringBuffer();
  if (evator != NULL) {
    olxstr sp(macro_call ? ' ' : ',');
    rv << evator->name << (macro_call ? ' ' : '(');
    for (size_t i=0; i < evator->args.Count(); i++) {
      rv << evator->args[i]->ToStringBuffer();
      if (i+1 < evator->args.Count())
        rv << sp;
    }
    if (!macro_call) rv << ')';
  }
  rv << data;
  if (priority) rv << ')';
  if (right != NULL)
    rv << right->ToStringBuffer();
  return rv;
}
  //.............................................................................
