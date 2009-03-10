//---------------------------------------------------------------------------

#ifdef __BORLANDC__
#pragma hdrstop
#endif
#include "bitarray.h"
#include "exception.h"
#include "egc.h"

UseEsdlNamespace()


TEBitArray::TEBitArray()  {  FData = NULL;  FCount = FCharCount = 0;  }
//..............................................................................
TEBitArray::TEBitArray(int size)  {
  FData = NULL;
  SetSize( size );
}
//..............................................................................
TEBitArray::TEBitArray( const TEBitArray& arr)  {
  FCount = arr.FCount;
  FCharCount = arr.FCharCount;
  if( arr.FData != NULL )  {
    FData = new unsigned char [FCharCount];
    memcpy(FData, arr.FData, FCharCount);
  }
  else
    FData = NULL;
}
//..............................................................................
TEBitArray::TEBitArray(unsigned char* data, size_t size, bool own)  {
  if( own )  {
    FData = data;
    FCharCount = size;
    FCount = size*8;
  }
  else  {
    FData = NULL;
    SetSize( size*8 );
    memcpy(FData, data, size);
  }
}
//..............................................................................
TEBitArray::~TEBitArray()  {  delete [] FData;  }
//..............................................................................
TEBitArray& TEBitArray::operator = (const TEBitArray& arr )  {
  Clear();
  FCount = arr.FCount;
  FCharCount = arr.FCharCount;
  if( arr.FData != NULL )  {
    FData = new unsigned char [FCharCount];
    memcpy(FData, arr.FData, FCharCount);
  }
  return *this;
}
//..............................................................................
void TEBitArray::SetSize(int newSize)  {
  if( FData )  delete [] FData;
  FCount = newSize;
  FCharCount = newSize/8 + 1;
  FData = new unsigned char [FCharCount];
  memset(FData, 0, FCharCount );
}
//..............................................................................
void TEBitArray::Clear()  {
  if( FData != NULL )  {
    delete [] FData;
    FData = NULL;
  }
  FCount = FCharCount = 0;
}
//..............................................................................
void TEBitArray::SetAll(bool v)  {
  if( FData == NULL )  return;
  memset(FData, v ? 0xff : 0, FCharCount );
}
//..............................................................................
void TEBitArray::operator << (IInputStream& in)  {
  Clear();
  in.Read(&FCount, sizeof(FCount));
  if( FCount != 0 )  {
    FCharCount = FCount/8 + 1;
    FData = new unsigned char [FCharCount];
    in.Read(FData, FCharCount);
  }
}
//..............................................................................
void TEBitArray::operator >> (IOutputStream& out) const  {
  out.Write(&FCount, sizeof(FCount) );
  if( FCount != 0 )
    out.Write(FData, FCharCount);
}
//..............................................................................
bool TEBitArray::operator == (const TEBitArray& arr )  const  {
  if( arr.Count() != Count() )  return false;
  for( uint32_t i=0; i < FCharCount; i++ )
    if( FData[i] != arr.FData[i] )  return false;
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
olxstr TEBitArray::ToHexString() const {
  olxstr rv(EmptyString, FCharCount*2+8);
  olxch bf[2];
  rv.Append( ByteToHex((char)FCount, bf), 2 );
  rv.Append( ByteToHex((char)(FCount>>8), bf), 2 );
  rv.Append( ByteToHex((char)(FCount>>16), bf), 2 );
  rv.Append( ByteToHex((char)(FCount>>24), bf), 2 );
  for( uint32_t i=0; i < FCharCount; i++ )
    rv.Append( ByteToHex(FData[i], bf), 2 );
  return rv;
}
//..............................................................................
void TEBitArray::FromHexString(const olxstr& str) {
  if( str.Length() < 8 ) 
    throw TInvalidArgumentException(__OlxSourceInfo, "representation");
  const olxch* s = str.raw_str();
  uint32_t cnt = ByteFromHex(s);
  cnt |= ( ByteFromHex(&s[2]) << 8 );
  cnt |= ( ByteFromHex(&s[4]) << 16 );
  cnt |= ( ByteFromHex(&s[6]) << 24 );
  if( str.Length() < (cnt/8+1)*2+8 )
    throw TInvalidArgumentException(__OlxSourceInfo, "representation");
  SetSize(cnt);
  for( uint32_t i=0; i < FCharCount; i++ )
    FData[i] = ByteFromHex( &s[8+i*2] );
}
//..............................................................................
TIString TEBitArray::ToString() const  {
  olxstr StrRepr(EmptyString, FCount+1);
  StrRepr.SetCapacity( Count() );
  for( uint32_t i=0; i < Count(); i++ )  {
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

