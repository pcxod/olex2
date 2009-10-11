#ifndef __olx_exp_tree_H
#define __olx_exp_tree_H
#include "../tptrlist.h"
#include "../estrlist.h"

BeginEsdlNamespace()

namespace exparse  {
  namespace parser_util  {
    static const olxstr control_chars("+-*/&^|:!?=%<>.");
    static const olxstr operators[]= {
      '.',
      '+', '-', '*', '/', '%',  // arithmetic
      '&', '^', '|', // bitwise
      "==", "!=", ">=", "<=", '<', '>', // comparison
      ">>", "<<", // directional
      ':', '?', '!', "&&", "||", // logical
      '=', "+=", "-=", "/=", "*=", "&=", "|=", "^=", "<<=" // assignment
    };
    //leaves ind on the last quote or does not change it if there is no string
    bool skip_string(const olxstr& exp, int& ind);
    // leave the ind on the closing bracket char or does not change it if there is none
    bool skip_brackets(const olxstr& exp, int& ind);
    bool parse_string(const olxstr& exp, olxstr& dest, int& ind);
    bool parse_brackets(const olxstr& exp, olxstr& dest, int& ind);
    bool is_operator(const olxstr& exp);
    bool parse_control_chars(const olxstr& exp, olxstr& dest, int& ind);
    bool is_expandable(const olxstr& exp);
    // checks if the char is a bracket char
    static inline bool is_bracket(olxch ch)  {
      return ch == '(' || ch == '[' || ch == '{' || ch == '<';
    }
    // checks if the char is a quote char
    static inline bool is_quote(olxch ch)  {
      return ch == '"' || ch == '\'';
    }
    // checks if the char at ch_ind is ascaped (\')
    static bool is_escaped(const olxstr& exp, int ch_ind)  {
      int sc = 0;
      while( --ch_ind >=0 && exp.CharAt(ch_ind) == '\\' ) sc++;
      return (sc%2) != 0;
    }
    // splits expressions like ("",ddd(),"\""), leaves tokens quoted if quoted originally
    template <class StrLst> static void split_args(const olxstr& exp, StrLst& res)  {
      int start = 0;
      for( int i=0; i < exp.Length(); i++ )  {
        const olxch ch = exp.CharAt(i);
        if( ch == '(' )  {
          int bc = 1;
          while( ++i < exp.Length() && bc != 0 )  {
            if( exp.CharAt(i) == '(' )  bc++;
            else if( exp.CharAt(i) == ')' )  bc--;
          }
          i--;
        }
        else if( is_quote(ch) && !is_escaped(exp, i) )  {  // skip strings
          while( ++i < exp.Length() && exp.CharAt(i) != ch && !is_escaped(exp, i) )
            ;
        }
        else if( ch == ',' )  {
          res.Add( exp.SubString(start, i-start) ).TrimWhiteChars();
          start = i+1;
        }
      }
      if( start < exp.Length() )
        res.Add( exp.SubStringFrom(start) ).TrimWhiteChars();
    }
    // removes quotation from a string
    static inline olxstr unquote(const olxstr& exp)  {
      if( exp.Length() < 2 )  return exp;
      const olxch ch = exp.CharAt(0);
      if( is_quote(ch) && (exp.Last() == ch) && !is_escaped(exp, exp.Length()-1) )
        return exp.SubStringFrom(1, 1);
      return exp;
    }
    // replaces \t, \n, \r, \\, \", \' with corresponding values
    olxstr unescape(const olxstr& exp);
  };

  template <class T> struct evaluator  {
    olxstr name;
    TPtrList<T> args;
    evaluator(const olxstr& _name) : name(_name) {}
    ~evaluator()  {  
      for( int i=0; i < args.Count(); i++ )
        delete args[i];
    }
  };
  struct expression_tree  {
    olxstr data;
    expression_tree *parent, *left, *right;
    evaluator<expression_tree>* evator;
    bool priority;  // the expression is in brackets
    expression_tree(expression_tree* p, const olxstr& dt, 
      expression_tree* l, expression_tree* r, 
      evaluator<expression_tree>* e) : 
        parent(p), data(dt), left(l), right(r), evator(e), priority(false) {}
     //................................................................................
    expression_tree(expression_tree* p, const olxstr& dt) : 
     parent(p), data(dt), left(NULL), right(NULL), evator(NULL), priority(false) {}
     //................................................................................
    ~expression_tree()  {
      if( left != NULL )  delete left;
      if( right != NULL )  delete right;
      if( evator != NULL )  delete evator;
    }
    void expand();
  };

  struct expression_parser  {
    expression_tree* root;
    void expand()  {  root->expand();  }
    expression_parser(const olxstr& exp) {
      root = new expression_tree(NULL, exp);
    }
    ~expression_parser()  {  delete root;  }
  };

};  // end namespace exparse

EndEsdlNamespace()

#endif
