//----------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//
#ifndef estackH
#define estackH
//---------------------------------------------------------------------------
#include "ebase.h"
#include "string.h"
#include "typelist.h"
#include "exception.h"

BeginEsdlNamespace()
// the simplet stack implementation
template <class T> class TStack  {
  struct item  {
    item* prev;
    T data;
    item(const T& v, item* _prev) : data(v), prev(_prev)  {}
  };
  item* cur;
  int _count;
public:
  TStack() : cur(NULL), _count(0)  {}
  virtual ~TStack()  {  Clear();  }
  void Clear()  {
    while( cur != NULL )  {
      item* p = cur->prev;
      delete cur;
      cur = p;
    }
    _count = 0;
  }
  inline T& Push(const T& v)  {
    item* ni = new item(v, cur);
    cur = ni;
    _count++;
    return cur->data;
  }
  inline T Pop()  { 
    if( cur != NULL )  { 
      item* i = cur->prev;
      T rv = cur->data;
      delete cur;
      cur = i;
      _count--;
      return rv;
    }
    throw TFunctionFailedException(__OlxSourceInfo, "stack is empty");
  }
  inline bool IsEmpty() const {  return cur == NULL;  }
  inline int Count() const {  return _count;  }
};

class str_stack : public TStack<olxstr>  {
public:
  str_stack(const olxstr& exp)  {  LoadFromExpression(exp);  }
  str_stack() {}

  int LoadFromExpression(const olxstr& Exp) {
    Clear();
    int argc=0;
    char op;
    olxstr o;
    bool NewOperation = false;
    if( !Exp.IsEmpty() )  {
      for( int i=0; i < Exp.Length(); i++ )  {
        switch( Exp.CharAt(i) )  {
            case '-':    NewOperation = true;    op = '-';    break;
            case '+':    NewOperation = true;    op = '+';    break;
            case ' ':    break;
            case '*':    NewOperation = true;    op = '*';    break;
            case '/':    NewOperation = true;    op = '/';    break;
            default:
              if( NewOperation )  {
                if( !o.IsEmpty() )  {
                  Push(o);
                  argc++;
                }
                Push(op);
                o = EmptyString;
                NewOperation = false;
              }
              o << Exp[i];
              break;
        }
      }
      Push(o);
    }
    return argc;  
  }
};

EndEsdlNamespace()
#endif

