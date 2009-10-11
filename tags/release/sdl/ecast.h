#ifndef eCastH
#define eCastH
#include "typelist.h"
#include "tptrlist.h"
BeginEsdlNamespace()
/*
  T - type
  C - const
  F - function
  P - pointer
  O - operator
  L - list, if prefixes the name - the firts argumen is the list type, not the content!,
    this is done for borland compiler, as it cannot handle some things, read comments in xapp.h

  TT - type 2 type, simple casting  A = (A)B, A* = (A*)B*
  TP - type 2 pointer - simply takes the address A* = &A
  TTP - type 2 type pointer - A* = (A*)&B
  POP - pointer operator to pointer  A* = (A*)(*B*), the casting operator must be implemented
  TOP - type operator to pointer  A = (A)(B)
  PFCP - pointer function const to pointer  A* = B*->*f() const
  PFP - pointer function to pointer  A* = B*->*f()
  TFCP - type function to pointer  A* = B.*f() const
  TFP - type function to pointer  A* = B.*f()

  many more posibilities are possible, but let's stop!

  ::: casting operator :::
    operator A* () [const]

*/

class TListCaster  {

public:

  /*   TT - type 2 type, simple casting  A = (A)B, A* = (A*)B*  */
  template <class A, class ADT, class B, class BDT>
    static TTypeListExt<B, BDT>& TT( const TTypeListExt<A,ADT>& from, TTypeListExt<B, BDT> &to )  {
      to.SetCapacity( to.Count() + from.Count() );
      for( int i=0; i < from.Count(); i++ )
        to.AddACopy( (B)from[i] );
      return to;
  }
  template <class A, class B>
    static TPtrList<B>& TT( const TPtrList<A>& from, TPtrList<B> &to )  {
      long sz = to.Count();
      to.SetCount( to.Count() + from.Count() );
      for( long i=0; i < from.Count(); i++ )
        to[sz+i] = (B*)from[i];
      return to;
  }
  /*   TTP - type 2 type pointer, A* = (A*)&B  */
  template <class A, class ADT, class B, class BDT>
    static TTypeListExt<B,BDT>& TTP( const TTypeListExt<A,ADT>& from, TTypeListExt<B,BDT> &to )  {
      to.SetCapacity( to.Count() + from.Count() );
      for( int i=0; i < from.Count(); i++ )
        to.AddACopy( (B)&from[i] );
      return to;
  }
  template <class A, class ADT, class B>
    static TPtrList<B>& TTP( const TTypeListExt<A,ADT>& from, TPtrList<B> &to )  {
      long sz = to.Count();
      to.SetCount( to.Count() + from.Count() );
      for( long i=0; i < from.Count(); i++ )
        to[sz+i] = (B*)&from[i];
      return to;
  }
  /*   TP - type 2 pointer, A* = &B */
  template <class A, class ADT, class PDT>
    static TTypeListExt<A*, PDT*>& TP( const TTypeListExt<A,ADT>& from, TTypeListExt<A*,PDT*> &to )  {
      to.SetCapacity( to.Count() + from.Count() );
      for( int i=0; i < from.Count(); i++ )
        to.AddACopy( &from[i] );
      return to;
  }
  /*   POP - pointer operator to pointer  A = (A)(*B), the casting operator must be implemented */
  template <class A, class ADT, class B, class BDT>
    static TTypeListExt<B,BDT>& POP( const TTypeListExt<A,ADT>& from, TTypeListExt<B,BDT> &to )  {
      to.SetCapacity( to.Count() + from.Count() );
      for( int i=0; i < from.Count(); i++ )
        to.AddACopy( (B)(*from[i]) );
      return to;
  }
  template <class A, class B>
    static TPtrList<B>& POP( const TPtrList<A>& from, TPtrList<B> &to )  {
      long sz = to.Count();
      to.SetCount( to.Count() + from.Count() );
      for( long i=0; i < from.Count(); i++ )
        to[sz+i] = (B*)(*from[i]);
      return to;
  }
  /*   TOP - instance operator to pointer  A* = (A*)(*B*), the casting operator must be implemented */
  template <class A, class ADT, class B, class BDT>
    static TTypeListExt<B,BDT>& TOP( const TTypeListExt<A,ADT>& from, TTypeListExt<B,BDT> &to )  {
      to.SetCapacity( to.Count() + from.Count() );
      for( int i=0; i < from.Count(); i++ )
        to.AddACopy( (B)(from[i]) );
      return to;
  }
  template <class A, class ADT, class B>
    static TPtrList<B>& TOP( const TTypeListExt<A,ADT>& from, TPtrList<B> &to )  {
      long sz = to.Count();
      to.SetCount( to.Count() + from.Count() );
      for( long i=0; i < from.Count(); i++ )
        to[sz+i] = (B*)from[i];
      return to;
  }
  /* PFCP - pointer function const to pointer  A* = B*->*f() const  */
  template <class A, class ADT, class B, class BDT, class AType>
    static TTypeListExt<B,BDT>& PFCP( const TTypeListExt<A,ADT>& from, TTypeListExt<B,BDT> &to,
      B (AType::*f)() const )  {
      to.SetCapacity( to.Count() + from.Count() );
      for( int i=0; i < from.Count(); i++ )
        to.AddACopy( (from[i]->*f)() );
      return to;
  }
  /* TFCP - type function const to pointer  A* = B.*f() const  */
  template <class A, class ADT, class B, class BDT>
    static TTypeListExt<B,BDT>& TFCP( const TTypeListExt<A,ADT>& from, TTypeListExt<B,BDT> &to,
      B (A::*f)() const )  {
      to.SetCapacity( to.Count() + from.Count() );
      for( int i=0; i < from.Count(); i++ )
        to.AddACopy( (from[i].*f)() );
      return to;
  }
  /* PFP - pointer function to pointer  A* = B*->*f() const  */
  template <class A, class ADT, class B, class BDT, class AType>
    static TTypeListExt<B,BDT>& PFP( const TTypeListExt<A,ADT>& from, TTypeListExt<B,BDT> &to,
      B (AType::*f)() )  {
      to.SetCapacity( to.Count() + from.Count() );
      for( int i=0; i < from.Count(); i++ )
        to.AddACopy( (from[i]->*f)() );
      return to;
  }
  /* TFP - type function const to pointer  A* = B.*f() const  */
  template <class A, class ADT, class B, class BDT>
    static TTypeListExt<B,BDT>& TFP( const TTypeListExt<A,ADT>& from, TTypeListExt<B,BDT> &to,
      B (A::*f)() )  {
      to.SetCapacity( to.Count() + from.Count() );
      for( int i=0; i < from.Count(); i++ )
        to.AddACopy( (from[i].*f)() );
      return to;
  }

  static void Tests();
};

EndEsdlNamespace()
#endif

