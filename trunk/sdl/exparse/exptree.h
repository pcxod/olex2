#ifndef __olx_exp_tree_H
#define __olx_exp_tree_H
#include "../tptrlist.h"
#include "../estrlist.h"

BeginEsdlNamespace()

namespace exparse  {
  namespace parser_util  {
    static const olxstr control_chars("+-*/&^|:!?=%<>");
    static const olxstr operators[]= {
      '+', '-', '*', '/', '%',  // arithmetic
      '&', '^', '|', // bitwise
      "==", "!=", ">=", "<=", '<', '>', // comparison
      ">>", "<<", // directional
      ':', '?', '!', "&&", "||", // logical
      '=', "+=", "-=", "/=", "*=", "&=", "|=", "^=", "<<=" // assignment
    };
    static bool parse_string(const olxstr& exp, olxstr& dest, int& ind);
    static bool parse_brackets(const olxstr& exp, olxstr& dest, int& ind);
    static bool is_operator(const olxstr& exp);
    static bool parse_control_chars(const olxstr& exp, olxstr& dest, int& ind);
    static bool is_expandable(const olxstr& exp);
    static void split_args(const olxstr& exp, TStrList& res);
    static inline bool is_bracket(const olxch& ch)  {
      return ch == '(' || ch == '[' || ch == '{';
    }
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
