#include "exptree.h"

using namespace esdl::exparse;

//...........................................................................
bool parser_util::skip_string(const olxstr& exp, int& ind)  {
  const olxch qc = exp.CharAt(ind);
  const int start = ind;
  while( ++ind < exp.Length() )  {
    if( exp.CharAt(ind) == qc && !is_escaped(exp, ind) )
      return true;
  }
  ind = start;
  return false;
}
//...........................................................................
bool parser_util::parse_string(const olxstr& exp, olxstr& dest, int& ind)  {
  const olxch qc = exp.CharAt(ind);
  bool end_found = false;
  const int start = ind+1;
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
//...........................................................................
bool parser_util::parse_escaped_string(const olxstr& exp, olxstr& dest, int& ind)  {
  const olxch qc = exp.CharAt(ind);
  bool end_found = false;
  const int start = ind+1;
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
//...........................................................................
bool parser_util::skip_brackets(const olxstr& exp, int& ind)  {
  const olxch oc = exp.CharAt(ind), 
    cc = (oc == '(' ? ')' : (oc == '[' ? ']' : (oc == '{' ? '}' : (oc == '<' ? '>' : '#'))));
  if( cc == '#' )
    throw TInvalidArgumentException(__OlxSourceInfo, olxstr("Invalid bracket char: ") << oc );
  int bc = 1;
  const int start = ind+1;
  while( ++ind < exp.Length() && bc != 0 )  {
    const olxch ch = exp.CharAt(ind);
    if( ch == cc && (--bc == 0) )  return true;
    else if( ch == oc )  bc++;
  }
  ind = start;
  return false;
}
//...........................................................................
bool parser_util::parse_brackets(const olxstr& exp, olxstr& dest, int& ind)  {
  const olxch oc = exp.CharAt(ind), 
    cc = (oc == '(' ? ')' : (oc == '[' ? ']' : (oc == '{' ? '}' : (oc == '<' ? '>' : '#'))));
  if( cc == '#' )
    throw TInvalidArgumentException(__OlxSourceInfo, olxstr("Invalid bracket char: ") << oc );
  int bc = 1;
  const int start = ind+1;
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
//...........................................................................
bool parser_util::is_operator(const olxstr& exp)  {
  static size_t sz = sizeof(operators)/sizeof(operators[0]);
  for( size_t i=0; i < sz; i++ )  {
    if( operators[i] == exp )
      return true;
  }
  return false;
}
//...........................................................................
bool parser_util::parse_control_chars(const olxstr& exp, olxstr& dest, int& ind)  {
  while( ind < exp.Length() && control_chars.IndexOf(exp.CharAt(ind)) != -1 )  {
    dest << exp.CharAt(ind++);
    if( !is_operator(dest) )  {
      dest.SetLength(dest.Length()-1);
      ind--;
      break;
    }
  }
  return !dest.IsEmpty();
}
//...........................................................................
bool parser_util::is_expandable(const olxstr& exp)  {
  for( int i=0; i < exp.Length(); i++ )  {
    const olxch ch = exp.CharAt(i);
    if( ch == '(' )
      return true;
    if( is_quote(ch) && !is_escaped(exp, i) )  {  // skip strings
      while( ++i < exp.Length() && exp.CharAt(i) != ch && exp.CharAt(i-1) != '\\' )
        ;
      continue;
    }
    if( control_chars.IndexOf(ch) != -1 )
      return true;
  }
  return false;
}
//...........................................................................
olxstr parser_util::unescape(const olxstr& exp)  {
  //return exp;
  olxstr out( EmptyString, exp.Length());
  for( int i=0; i < exp.Length(); i++ )  {
    if( exp.CharAt(i) == '\\' )  {
      if( ++i >= exp.Length() )  break;
      switch( exp.CharAt(i) )  {
        case 'r':   out << '\r';  break;
        case 'n':   out << '\n';  break;
        case 't':   out << '\t';  break;
        case '\\':  out << '\\';  break;
        case '\'':  out << '\'';  break;
        case '"':   out << '"';  break;
        default:    out << '\\' << exp.CharAt(i);  break;
      }
    }
    else
      out << exp.CharAt(i);
  }
  return out;
}
//...........................................................................
//...........................................................................
//...........................................................................
void expression_tree::expand()  {
  data.TrimWhiteChars();
  if( !parser_util::is_expandable(data) )  return;
  olxstr dt;
  for( int i=0; i < data.Length(); i++ )  {
    olxch ch = data.CharAt(i);
    if( olxstr::o_iswhitechar(ch) )  continue;
    else if( ch == '(' )  {  // parse out brackets
      olxstr arg;
      if( !parser_util::parse_brackets(data, arg, i) ) 
        throw TInvalidArgumentException(__OlxSourceInfo, "problem with brackets");
      arg.TrimWhiteChars();
      dt.TrimWhiteChars();
      if( arg.IsEmpty() ) // empty arg list
        evator = new evaluator<expression_tree>(dt);
      else  {
        TStrList args;
        parser_util::split_args(arg, args);
        if( args.Count() == 1 )  {
          left = new expression_tree(this, arg);
          left->expand();
          if( !dt.IsEmpty() )  { // ()
            evator = new evaluator<expression_tree>(dt);
            evator->args.Add(left);
            left = NULL;
          }
          else if( i+1 >= data.Length() )  {  // nothing else? then move one level up...
            expression_tree* eta = left;
            left = eta->left;
            right = eta->right;
            evator = eta->evator;
            data = eta->data;
            eta->left = eta->right = NULL;
            eta->evator = NULL;
            delete eta;
            priority = true;
          }
        }
        else  {
          evator = new evaluator<expression_tree>(dt);
          for( int ai=0; ai < args.Count(); ai++ )
            evator->args.Add(new expression_tree(this, args[ai]))->expand();
        }
      }
      dt.SetLength(0);
    }
    else if( parser_util::is_quote(ch) && !parser_util::is_escaped(data, i) )  { // parse out the strings
      if( !parser_util::parse_string(data, dt, i) ) 
        throw TInvalidArgumentException(__OlxSourceInfo, "problem with quotations");
    }
    else  { 
      olxstr opr;
      if( parser_util::parse_control_chars(data, opr, i) )  {
        if( opr == '.' )  {  // treat floating point values...
          bool number = false;
          if( i == 1 || (i+1) >= data.Length() )  // .1 or 1.
            number = true;
          else if( (i > 1 && olxstr::o_isdigit(data.CharAt(i-2))) ||
              (i < data.Length() && olxstr::o_isdigit(data.CharAt(i))) )
            number = true;
          if( number )  {
            dt << '.';
            i--;
            while( ++i < data.Length() && olxstr::o_isdigit(data.CharAt(i)) )
              dt << data.CharAt(i);
            i--;
            continue;
          }
        }

        if( !dt.IsEmpty() )  {
          if( left != NULL )
            throw TInvalidArgumentException(__OlxSourceInfo, "invalid expression");
          left = new expression_tree(this, dt);
          left->expand();
        }
        right = new expression_tree(this, data.SubStringFrom(i));
        right->expand();
        data = opr;
      }
      else
        dt << ch;
    }
  }
}
