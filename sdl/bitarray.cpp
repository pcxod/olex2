/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "bitarray.h"
#include "exception.h"
#include "egc.h"
#include "encodings.h"
UseEsdlNamespace()

TEBitArray::TEBitArray()  {  FData = NULL;  FCount = FCharCount = 0;  }
//..............................................................................
TEBitArray::TEBitArray(size_t size)  {
  FData = NULL;
  FCharCount = FCount = 0;
  SetSize(size);
}
//..............................................................................
TEBitArray::TEBitArray(const TEBitArray& arr)  {
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
TEBitArray& TEBitArray::operator = (const TEBitArray& arr)  {
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
void TEBitArray::SetSize(size_t newSize)  {
  if( FCount == newSize )  return;
  size_t chcnt = FCharCount;
  unsigned char *data = FData;
  FCount = newSize;
  FCharCount = newSize/8 + 1;
  FData = new unsigned char [FCharCount];
  if (data != NULL) {
    memcpy(FData, data, olx_min(FCharCount, chcnt));
    if (FCharCount > chcnt)
      memset(&FData[chcnt], 0, FCharCount-chcnt);
    delete [] data;
  }
  else
    memset(FData, 0, FCharCount);
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
  memset(FData, v ? 0xff : 0, FCharCount);
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
void TEBitArray::operator >> (IOutputStream& out) const {
  out.Write(&FCount, sizeof(FCount) );
  if( FCount != 0 )
    out.Write(FData, FCharCount);
}
//..............................................................................
bool TEBitArray::operator == (const TEBitArray& arr ) const {
  if( arr.Count() != Count() )  return false;
  for( size_t i=0; i < FCharCount; i++ )
    if( FData[i] != arr.FData[i] )  return false;
  return true;
}
//..............................................................................
int TEBitArray::Compare(const TEBitArray& arr) const {
  if( arr.Count() < Count() )  {
    if( arr.IsEmpty() )  return 1;
    // check for a high bit
    for( size_t i=Count()-1; i > arr.Count(); i-- )  {
      if( Get(i) )  return 1;
    }
    // comparison
    for( size_t i=arr.Count()-1; i != InvalidIndex; i-- )  {
      if( Get(i) && !arr.Get(i) )  return 1;
      else if( !Get(i) && arr.Get(i) )  return -1;
    }
    return 0;
  }
  else if( arr.Count() == Count() )  {
    for( size_t i=Count()-1; i != InvalidIndex; i-- )  {
      if( Get(i) && !arr.Get(i) )  return 1;
      else if( !Get(i) && arr.Get(i) )  return -1;
    }
    return 0;
  }
  else  // reverse te order
    return arr.Compare(*this);
}
//..............................................................................
olxstr TEBitArray::ToBase64String() const {
  olxcstr rv = encoding::base64::encode(FData, FCharCount);
  rv << (char)('0' + FCount%8);
  return rv;
}
//..............................................................................
void TEBitArray::FromBase64String(const olxstr& str) {
  if( (str.Length()%4) != 1 ) 
    throw TInvalidArgumentException(__OlxSourceInfo, "representation");
  olxcstr cstr = encoding::base64::decode(olxcstr(str.SubStringTo(str.Length()-1)));
  SetSize(cstr.Length()*8 - (8-(str.GetLast()-'0'))); 
  memcpy(FData, cstr.raw_str(), cstr.Length());
}
//..............................................................................
TIString TEBitArray::ToString() const  {
  olxstr StrRepr(EmptyString(), FCount+1);
  StrRepr.SetCapacity( Count() );
  for( size_t i=0; i < Count(); i++ )
    StrRepr <<  (Get(i) ? '1': '0');
  return StrRepr;
}
//..............................................................................
olxstr TEBitArray::FormatString(uint16_t bitsInSegment) const {
  if( bitsInSegment == 0 || bitsInSegment > 64)
    throw TInvalidArgumentException(__OlxSourceInfo, "bitsInSegment");
  olxstr StrRepr;
  StrRepr.SetCapacity(Count() + Count()/bitsInSegment + 1);
  uint64_t mask = 0;
  size_t strlen=bitsInSegment/3+1;
  for( size_t i=0; i < Count(); i++ )  {
    if( (i%bitsInSegment) == 0 && i > 0)  {
      olxstr s(mask);
      StrRepr << s.LeftPadding(strlen, '0');
      mask = 0;
    }
    if( Get(i) )
      mask |=  (uint64_t)1 << (i%bitsInSegment);
  }
  if( mask != 0 ) {
    olxstr s(mask);
    StrRepr << s.LeftPadding(strlen, '0');
  }
  return StrRepr;
}
//..............................................................................
