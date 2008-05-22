//---------------------------------------------------------------------------

#ifdef __BORLANDC__
#pragma hdrstop
#endif
#include "bitarray.h"
#include "exception.h"
#include "egc.h"

UseEsdlNamespace()

const int IntBitSize = sizeof(int)*8;

TEBitArray::TEBitArray()  {  FData = NULL;  FCount = FIntCount = 0;  }
//..............................................................................
TEBitArray::TEBitArray(int size)  {
  FData = NULL;
  SetSize( size );
}
//..............................................................................
TEBitArray::TEBitArray( const TEBitArray& arr)  {
  FCount = arr.FCount;
  FIntCount = arr.FIntCount;
  FData = new int [FIntCount];
  for( int i=0; i < FIntCount; i++ )  FData[i] = arr.FData[i];
}
//..............................................................................
TEBitArray::~TEBitArray()  {
  delete [] FData;
}
//..............................................................................
TEBitArray& TEBitArray::operator = (const TEBitArray& arr )  {
  Clear();
  FCount = arr.FCount;
  FIntCount = arr.FIntCount;
  FData = new int [FIntCount];
  for( int i=0; i < FIntCount; i++ )  FData[i] = arr.FData[i];
  return *this;
}
//..............................................................................
void TEBitArray::SetSize(int newSize)  {
  if( FData )  delete [] FData;
  FCount = newSize;
  FIntCount = newSize/IntBitSize + 1;
  FData = new int [FIntCount];
  for( int i=0; i < FIntCount; i++ )  FData[i] = 0;
}
//..............................................................................
void TEBitArray::Clear()  {
  if( FData != NULL )  {
    delete [] FData;
    FData = NULL;
  }
  FCount = FIntCount = 0;
}
//..............................................................................
bool TEBitArray::Get(int index) const  {
  int intIndex = index/IntBitSize;
  int bitIndex = 1 << index%IntBitSize;

  TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, intIndex, 0, FIntCount);

  return (FData[intIndex] & bitIndex) != 0;
}
//..............................................................................
void TEBitArray::Set(int index, bool v)  {
  int intIndex = index/IntBitSize;
  int bitIndex = 1 << index%IntBitSize;

  TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, intIndex, 0, FIntCount);

  if( !v )  {
    if( FData[intIndex] & bitIndex )
      FData[intIndex] ^= bitIndex;
  }
  else
    FData[intIndex] |= bitIndex;
}
//..............................................................................
bool TEBitArray::operator == (const TEBitArray& arr )  const  {
  if( arr.Count() != Count() )  return false;
  for( int i=0; i < Count(); i++ )
    if( Get(i) != arr.Get(i) )  return false;
  return true;
}
//..............................................................................
int TEBitArray::Compare(const TEBitArray& arr )  const {
  if( arr.Count() < Count() )  {
    // check for a high bit
    for( int i=Count()-1; i > arr.Count(); i-- )
      if( Get(i) )  return 1;
    // comparison
    for( int i=arr.Count()-1; i >= 0; i-- )
      if( Get(i) && !arr.Get(i) )  return 1;
      else if( !Get(i) && arr.Get(i) )  return -1;

    return 0;
  }
  else if( arr.Count() == Count() )  {
    for( int i=Count()-1; i >= 0; i-- )
      if( Get(i) && !arr.Get(i) )  return 1;
      else if( !Get(i) && arr.Get(i) )  return -1;

    return 0;
  }
  else  // reverse te order
    return arr.Compare(*this);
}
//..............................................................................
TIString TEBitArray::ToString() const  {
  olxstr StrRepr(EmptyString, FCount+1);
  StrRepr.SetCapacity( Count() );
  for( int i=0; i < Count(); i++ )  {
    if( Get(i) )  StrRepr <<  '1';
    else          StrRepr <<  '0';
  }        
  return StrRepr;
}
//..............................................................................
olxstr TEBitArray::FormatString( short bitsInSegment )  {
  if( bitsInSegment <= 0 )
    throw TInvalidArgumentException(__OlxSourceInfo, "bitsInSegment" );
  olxstr StrRepr;
  StrRepr.SetCapacity( Count() + Count()/bitsInSegment + 1);
//  for( int i=0; i < Count(); i++ )  {
//    if( Get(i) )  StrRepr <<  '1';
//    else          StrRepr <<  '0';
//    if( !(i%bitsInSegment) && i )  StrRepr << ' ';
//  }
  int mask = 0;
  int strlen=0;
  for( int i=0; i < bitsInSegment/8+1; i++ )  {
    if( !(i%3) )  strlen += 3;
    else          strlen += 2;
  }
  for( int i=0; i < Count(); i++ )  {
    if( !(i%bitsInSegment) )  {
      if( i )  {
        olxstr str(mask);
        str.Format(strlen, false, '0');
        StrRepr << str;// << ' ';
      }
      mask = 0;
    }
    if( Get(i) )  mask |=  1 << (i%bitsInSegment);
  }
  if( Count() % bitsInSegment ) StrRepr << mask;
  return StrRepr;
}
//..............................................................................

