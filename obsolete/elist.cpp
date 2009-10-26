//---------------------------------------------------------------------------//
// namespace TEObjects
// TEList - list of void* pointers
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#include <mem.h>
#endif
#ifdef _MSC_VER
  #include <memory.h>
#else
  #include <string.h>
#endif

#include "elist.h"
#include "exception.h"
#include <stdarg.h>

UseEsdlNamespace()
//----------------------------------------------------------------------------//
// TEList function bodies
//----------------------------------------------------------------------------//
void TEList::init(int size)  {
  FCount = size;
  FIncrement = 5;
  FCapacity = FCount + FIncrement;
  Items = new void* [FCapacity];
  memset(Items, 0, FCapacity);
  for(int i=0; i < FCapacity; i++ )
    Items[i] = NULL;
}
//..............................................................................
TEList::TEList(int count, ...)  {
  if( count >= 0 )  {
    init( count );
    return;
  }
  count = -count;
  init( count );
  va_list arglist;
  va_start( arglist, count );
  for( int i=0; i < count; i++ )
    Items[i] = va_arg( arglist, void* );
  va_end( arglist );
}
//..............................................................................
TEList::~TEList()
{
  delete [] Items;
}
//..............................................................................
void TEList::Add(const TEList &E)
{
  int c = FCount + E.FCount;
  SetCapacity( c+1 );
  for( int i=0; i < E.FCount; i++ )
    Add( E.Items[i] );
  FCount = c;
}
//..............................................................................
void TEList::Assign(const TEList &E)
{
  Clear();
  int c = E.Count();
  SetCount( c );
  for( int i=0; i < c; i++ )
    Items[i] = E.Items[i];
  FCount = c;
}
//..............................................................................
#ifdef  _OLX_DEBUG
void * &TEList::operator [] (int index) const  {
  TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, index, 0, FCount);
  return Items[index];
}
//..............................................................................
void * &TEList::Item(int index) const  {
  TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, index, 0, FCount);
  return Items[index];
}
//..............................................................................
void * &TEList::Last(int index) const  {
  TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, FCount-1, 0, FCount);
  return Items[FCount-1];
}
//..............................................................................
#endif
int TEList::IndexOf(const void *D) const
{
  for( int i=0; i < FCount; i++ )
    if( Items[i] == D )  return i;
  return -1;
}
//..............................................................................
void TEList::Remove(const void *D)
{
  int i = IndexOf(D);
  if( i != -1 )  Delete(i);
  else
    throw TFunctionFailedException(__OlxSourceInfo, "could not locate specified object");
}
//..............................................................................
void TEList::Delete(int index)
{
#ifdef _OLX_DEBUG
  TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, index, 0, FCount);
#endif
  for( int i=index+1; i < FCount; i++ )
    Items[i-1] = Items[i];
  FCount --;
}
//..............................................................................
void TEList::DeleteRange(int From, int To)
{
#ifdef _OLX_DEBUG
  TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, From, 0, FCount);
  TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, To, 0, FCount);
#endif
  for( int i=To; i < FCount; i++ )
    Items[From+i-To] = Items[i];
  FCount -= (To-From);
}
//..............................................................................
void TEList::SetCapacity(int v)
{
  if( v < FCapacity )    return;
  FCapacity = v;
  void ** Bf = new void * [FCapacity];
  // copy items
  memcpy( Bf, Items, sizeof(void*)*FCount );
  //  for( int i=0; i < FCount; i++ )    Bf[i] = Items[i];
  // initialise the rest of items to NULL
  memset( &Bf[FCount], 0, FCapacity-FCount);
//  for( int i=FCount; i < FCapacity; i++ )    Bf[i] = NULL;
  delete [] Items;
  Items = Bf;
}
//..............................................................................
void TEList::Add(void *I)
{
  if( FCapacity == FCount )  SetCapacity((int)(1.5*FCount + FIncrement));
  Items[FCount] = I;
  FCount ++;
}
//..............................................................................
void TEList::Insert( int index, void *D)
{
#ifdef _OLX_DEBUG
/*
  insert can happen at position 0 and at position FCount, so we validate index-1
  and skip "0"
*/
  if( index )
    TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, index-1, 0, FCount);
#endif
  if( FCapacity == FCount )  SetCapacity((int)(1.5*FCount + FIncrement));
  for( int i=FCount-1; i >= index; i-- )
    Items[i+1] = Items[i];
  Items[index] = D;
  FCount++;
}
//..............................................................................
void TEList::Clear()
{
  SetCount(0);
}
//..............................................................................
void TEList::Pack()
{
  // count null pointers
  int nc = 0;
  for( int i=0; i < FCount; i++ )
    if( !Items[i] )  nc ++;
  if( !nc )  return;
  // this is the new capacity of the list
  int ns = FCount - nc + FCapacity;
  void **Bf = new void *[ns];
  for( int i=0, j=0; i < FCount; i++ )
  {
    if( Items[i] )
    {
      Bf[j] = Items[i];
      j++;
    }
  }
  delete [] Items;
  Items = Bf;
  FCount -= nc;
}
//..............................................................................
void TEList::SetCount(int v)
{
  if( v == FCount )  return;
#ifdef _OLX_DEBUG
 // TODO: check if v is valid
#endif
  if( v > FCount )
  {
    if( v > FCapacity )  SetCapacity(v + FIncrement);
  }
  else
  {
    void ** Bf = new void * [v+FIncrement];
    // copy items
    memcpy( Bf, Items, v*sizeof(void*) );
//    for( int i=0; i < v; i++ )    Bf[i] = Items[i];
    // initialise the rest of items to NULL
    memset( &Bf[v], 0, FIncrement);
//    for( int i=0; i < FIncrement; i++ )    Bf[i+v] = NULL;
    delete [] Items;
    Items = Bf;
    FCapacity = v + FIncrement;
  }
  FCount = v;
}
//..............................................................................
/* the cycle shifts could be implemented in other ways:
1. series of shifts by one
2. series of swaps:
  1 2 3 4 5 -> 1 4 5 2 3 -> 4 5 1 2 3
  3. like here - memory usage ..., not elegant, but simple
*/
void TEList::ShiftL(int cnt)  {
  if( FCount == 0 )  return;
  int sv = cnt%FCount;
  if( sv <= 0 )  return;

  if( sv == 1 )  {  // special case
    void *D = Items[0];
    for( int i=1; i <= FCount-1; i++ )
      Items[i-1] = Items[i];
    Items[FCount-1] = D;
  }
  else  {
    void** D = new void*[sv];
    //memcpy( &D[0], Items[0], sizeof(void*)*sv );
    for( int i=0; i < sv; i++ )
      D[i] = Items[i];
    for( int i=sv; i <= FCount-1; i++ )
      Items[i-sv] = Items[i];
//    memcpy( Items[FCount-sv], &D[0], sizeof(void*)*sv );
    for( int i=0; i < sv; i++ )
      Items[FCount-sv+i] = D[i];
    delete [] D;
  }
}
//..............................................................................
void TEList::ShiftR(int cnt)  {
  if( FCount == 0 )  return;
  int sv = cnt%FCount;
  if( sv <= 0 )  return;

  if( sv == 1 )  {  // special case
    void* D = Items[FCount-1];
    for( int i=FCount-2; i >= 0; i-- )
      Items[i+1] = Items[i];
    Items[0] = D;
  }
  else  {
    void** D = new void*[sv];
    memcpy( &D[0], Items[FCount-sv], sizeof(void*)*sv );
    for( int i=FCount-sv-1; i >= 0; i-- )
      Items[i+sv] = Items[i];
    memcpy( Items[0], &D[0], sizeof(void*)*sv );
    delete [] D;
  }
}
//..............................................................................
void TEList::Move(int from, int to)
{
#ifdef _OLX_DEBUG
  TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, from, 0, FCount);
  TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, to, 0, FCount);
#endif
  void *D = Items[from];
  if( from > to )  {
    for( int i=from-1; i >= to; i-- )
      Items[i+1] = Items[i];
  }
  else  {
    for( int i=from+1; i <= to; i++ )
      Items[i-1] = Items[i];
  }
  Items[to] = D;
}
//..............................................................................
void TEList::Swap(int i, int j)
{
#ifdef _OLX_DEBUG
  TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, i, 0, FCount);
  TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, j, 0, FCount);
#endif
  void *D = Items[i];
  Items[i] = Items[j];
  Items[j] = D;
}
//..............................................................................
void TEList::Sort(TListSort *LS)
{
  QuickSort(0, FCount - 1, LS);
}
//..............................................................................
void TEList::Sort(ListSort v)
{
  QuickSort(0, FCount - 1, v);
}
//..............................................................................
void TEList::QuickSort(int lo0, int hi0, ListSort v)
{
  int lo = lo0;
  int hi = hi0;
  if ( hi0 > lo0)
  {
    void * mid = Items[ ( lo0 + hi0 ) / 2 ];
    while( lo <= hi )
    {
      while( ( lo < hi0 ) && ( v(Items[lo], mid) < 0) )   lo++;
      while( ( hi > lo0 ) && ( v(Items[hi], mid) > 0) ) hi--;
      if( lo <= hi )
      {
        void *tmp = Items[lo];
        Items[lo] = Items[hi];
        Items[hi] = tmp;
        lo++;
        hi--;
      }
    }
    if( lo0 < hi )  QuickSort(lo0, hi, v);
    if( lo < hi0 )  QuickSort(lo, hi0, v);
  }
}
//..............................................................................
void TEList::QuickSort(int lo0, int hi0, TListSort *v)
{
  int lo = lo0;
  int hi = hi0;
  if ( hi0 > lo0)
  {
    void *mid = Items[ ( lo0 + hi0 ) / 2 ];
    while( lo <= hi )
    {
      while( ( lo < hi0 ) && ( v->Compare(Items[lo], mid) < 0) )   lo++;
      while( ( hi > lo0 ) && ( v->Compare(Items[hi], mid) > 0) ) hi--;
      if( lo <= hi )
      {
        void *tmp = Items[lo];
        Items[lo] = Items[hi];
        Items[hi] = tmp;
        lo++;
        hi--;
      }
    }
    if( lo0 < hi )  QuickSort(lo0, hi, v);
    if( lo < hi0 )  QuickSort(lo, hi0, v);
  }
}
//..............................................................................

