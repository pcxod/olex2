/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_exp_tree_H
#define __olx_exp_tree_H
#include "../tptrlist.h"
#include "../estrlist.h"
#include "../edict.h"
BeginEsdlNamespace()

namespace exparse  {
  namespace parser_util  {
    struct operator_set {
      typedef SortedObjectList<olxch, TPrimitiveComparator> charset_t;
      typedef SortedObjectList<olxstr, olxstrComparator<false> > operatorset_t;
    protected:
      // a set of all chars unique to the operators
      charset_t control_chars;
      operatorset_t operators;
      void update_charset();
    public:
      operator_set();
      bool is_operator(const olxstr& exp) const {
        return operators.Contains(exp);
      }
      bool is_control_char(olxch ch) const { return control_chars.Contains(ch); }
      bool is_next_char_control(const olxstr& exp, size_t ind) const {
        return ind+1 < exp.Length() &&
          control_chars.Contains(exp.CharAt(ind+1));
      }
      bool parse_control_chars(const olxstr& exp, olxstr& dest,
        size_t& ind) const;
      bool add_operator(const olxstr &opr);
      bool remove_operator(const olxstr &opr);
    };
    //leaves ind on the last quote or does not change it if there is no string
    bool skip_string(const olxstr& exp, size_t& ind);
    /* leaves the ind on the closing bracket char or does not change it if
    there is none
    */
    bool skip_brackets(const olxstr& exp, size_t& ind);
    /* leaves the ind on the first non-white space/tab
    returns the updated index value
    */
    size_t skip_whitechars(const olxstr& exp, size_t& ind);
    /* more control over the chars to skip
    */
    template <typename seq_t>
    size_t skip_chars(const olxstr& exp, size_t& ind, const seq_t &seq, size_t seq_l) {
      const size_t el = exp.Length();
      if (ind >= el) return ind;
      while (olxstr::o_isoneof(exp[ind], seq, seq_l)) {
        if (++ind == el)
          break;
      }
      return ind;
    }
    template <typename seq_t>
    size_t skip_chars(const olxstr& exp, size_t& ind, const seq_t &seq) {
      return skip_chars(exp, ind, seq, olxstr::o_strlen(seq));
    }

    bool parse_string(const olxstr& exp, olxstr& dest, size_t& ind);
    // does not check for escaped quotes
    bool parse_escaped_string(const olxstr& exp, olxstr& dest, size_t& ind);
    bool parse_brackets(const olxstr& exp, olxstr& dest, size_t& ind);
    /* checks if there are any brackets or control chars or white spaces
    outside quoted strings
    */
    bool is_expandable(const olxstr& exp, const operator_set &os);
    /* as above, but does not check for the control chars */
    bool is_expandable(const olxstr& exp);
    // checks if the char is a bracket char
    inline bool is_bracket(olxch ch)  {
      return ch == '(' || ch == '[' || ch == '{' || ch == '<';
    }
    // returns the bracket counterpart
    inline olxch get_closing_bracket(olxch oc)  {
      const olxch cc = (oc == '(' ? ')' : (oc == '[' ? ']' : (oc == '{' ? '}'
        : (oc == '<' ? '>' : '#'))));
      if (cc == '#') {
        throw TInvalidArgumentException(__OlxSourceInfo,
          olxstr("bracket=").quote() << oc);
      }
      return cc;
    }
    // checks if the char is a quote char
    inline bool is_quote(olxch ch)  {
      return ch == '"' || ch == '\'';
    }
    // checks if the char at ch_ind is ascaped (\')
    bool is_escaped(const olxstr& exp, size_t ch_ind);
    // returns index of th next unescaped char...
    size_t next_unescaped(olxch _what, const olxstr& _where, size_t _from);
    /* splits expressions like ("",ddd(),"\""), leaves tokens quoted if quoted
    originally
    */
    template <class StrLst> void split_args(const olxstr& exp, StrLst& res)  {
      if (is_bracket(exp.CharAt(0))) {
        res.Add(exp);
        return;
      }
      size_t start = 0;
      for (size_t i=0; i < exp.Length(); i++) {
        const olxch ch = exp.CharAt(i);
        if (is_bracket(ch) && !is_escaped(exp, i)) {
          skip_brackets(exp, i);
        }
        else if (is_quote(ch) && !is_escaped(exp, i)) {  // skip strings
          skip_string(exp, i);
        }
        else if (ch == ',') {
          res.Add(exp.SubString(start, i-start)).TrimWhiteChars();
          start = i+1;
        }
      }
      if( start < exp.Length() )
        res.Add( exp.SubStringFrom(start) ).TrimWhiteChars();
    }
    // removes quotation from a string
    inline olxstr unquote(const olxstr& exp)  {
      if( exp.Length() < 2 )  return exp;
      const olxch ch = exp.CharAt(0);
      if( is_quote(ch) && (exp.GetLast() == ch) &&
        !is_escaped(exp, exp.Length()-1) )
      {
        return exp.SubStringFrom(1, 1);
      }
      return exp;
    }
    // checks if the string is quoted
    inline bool is_quoted(const olxstr& exp)  {
      if( exp.Length() < 2 )  return false;
      const olxch ch = exp.CharAt(0);
      return (is_quote(ch) && (exp.GetLast() == ch) &&
        !is_escaped(exp, exp.Length()-1));
    }
    // replaces \t, \n, \r, \\, \", \' with corresponding values
    olxstr unescape(const olxstr& exp);
    // opposite for the unescape
    olxstr escape(const olxstr& exp);
    // if find open_tag, initialises start_char_ind with position next after
    struct tag_parse_info  {
      int start_char_ind, end_string_ind, end_char_ind;
      tag_parse_info()
        : start_char_ind(-1), end_string_ind(-1), end_char_ind(-1)
      {}
    };
    template <class Lst>
    static tag_parse_info skip_tag(const Lst& list, const olxstr& open_tag,
      const olxstr& close_tag, size_t str_ind, size_t char_ind)
    {
      size_t oti = list[str_ind].FirstIndexOf(open_tag, char_ind);
      tag_parse_info rv;
      if( oti == InvalidIndex )  return rv;
      rv.start_char_ind = oti+open_tag.Length();
      int otc = 1;
      while( otc != 0 )  {
        const olxstr& line = list[str_ind];
        for( size_t i=0; i < line.Length(); i++ )  {
          if( line.IsSubStringAt(open_tag, i) )  otc++;
          else if( line.IsSubStringAt(close_tag, i) && --otc == 0)  {
            rv.end_string_ind = str_ind;
            rv.start_char_ind = i+close_tag.Length();
            return rv;
          }
        }
        if( ++str_ind >= list.Count() )  return rv;
      }
      return rv;
    }
  }

  template <class T> struct evaluator  {
    olxstr name;
    TPtrList<T> args;
    evaluator(const olxstr& _name) : name(_name) {}
    ~evaluator()  { args.DeleteItems(); }
  };
  struct expression_tree  {
    olxstr data;
    expression_tree *parent, *left, *right;
    evaluator<expression_tree>* evator;
    bool priority,  // the expression is in brackets
      macro_call; // call conversion - f(x) or 'f x'
    expression_tree(expression_tree* p, const olxstr& dt,
      expression_tree* l, expression_tree* r,
      evaluator<expression_tree>* e)
      : data(dt), parent(p), left(l), right(r), evator(e),
      priority(false), macro_call(false)
    {}
     //........................................................................
    expression_tree(expression_tree* p, const olxstr& dt)
      : data(dt), parent(p), left(NULL), right(NULL), evator(NULL),
      priority(false), macro_call(false)
    {}
     //........................................................................
    ~expression_tree()  {
      if( left != NULL )  delete left;
      if( right != NULL )  delete right;
      if( evator != NULL )  delete evator;
    }
    void expand(const parser_util::operator_set &os);
    /*this is for the expansion of the command lines rather than of the
    generic expressions - in this case no control chars are treated
    */
    void expand_cmd();
    olxstr_buf ToStringBuffer() const;
  };

  struct expression_parser  {
    expression_tree* root;
    void expand()  {  root->expand(parser_util::operator_set());  }
    void expand(const parser_util::operator_set &os)  {  root->expand(os);  }
    expression_parser(const olxstr& exp) {
      root = new expression_tree(NULL, exp);
    }
    ~expression_parser()  {  delete root;  }
  };

}  // end namespace exparse

EndEsdlNamespace()
#endif
