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


template <class T>
  class TEStack: public IEObject  {
  private:
    TTypeList<T>   Stack;
    int Position;
  public:
    TEStack()  {
      Position = 0;
    }
    virtual ~TEStack()  {  ;  }
    inline void Clear()  {
      Stack.Clear();
      Position = 0;
    }
    static int LoadFromExpression(TEStack<T>& stack,  const olxstr& Exp) {
      stack.Clear();
      int argc=0;
      char op;
      olxstr o;
      bool NewOperation = false;
      if( Exp.Length() )  {
        for( int i=0; i < Exp.Length(); i++ )  {
          switch( Exp[i] )  {
            case '-':    NewOperation = true;    op = '-';    break;
            case '+':    NewOperation = true;    op = '+';    break;
            case ' ':    break;
            case '*':    NewOperation = true;    op = '*';    break;
            case '/':    NewOperation = true;    op = '/';    break;
            default:
              if( NewOperation )  {
                if( o.Length() )  {
                  stack.Push(o);
                  argc++;
                }
                stack.Push(op);
                o = EmptyString;
                NewOperation = false;
              }
            o << Exp[i];
            break;
          }
        }
        stack.Push(o);
      }
      return argc;  }

    const T& Pop()  {
      Position--;
      if( Position < 0 )  {
        Position = 0;
        throw TFunctionFailedException(__OlxSourceInfo, "empty stack");
      }
      return Stack[Position];
    }
    inline void  Push(const T& val)  {
      Stack.AddCCopy( val );
      Position++;
    }

    inline int Capacity() const {  return Position;  }
    inline bool IsEmpty() const {  return Position <= 0;  }
  };

EndEsdlNamespace()
#endif

