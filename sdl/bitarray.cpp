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

TEBitArray::TEBitArray()
  : FData(0), FCount(0), FCharCount(0)
{}
//..............................................................................
TEBitArray::TEBitArray(size_t size)
  : FData(0), FCount(0), FCharCount(0)
{
  SetSize(size);
}
//..............................................................................
TEBitArray::TEBitArray(const TEBitArray& arr) {
  FCount = arr.FCount;
  FCharCount = arr.FCharCount;
  if (arr.FData != 0) {
    FData = new unsigned char [FCharCount];
    memcpy(FData, arr.FData, FCharCount);
  }
  else
    FData = 0;
}
//..............................................................................
TEBitArray::TEBitArray(unsigned char* data, size_t size, bool own)
  : FData(0), FCount(0), FCharCount(0)
{
  if (own) {
    FData = data;
    FCharCount = size;
    FCount = size*8;
  }
  else {
    FData = 0;
    // set size allocates at least 1 byte to FData
    SetSize(size*8);
    memcpy(FData, data, size);
  }
}
//..............................................................................
TEBitArray::TEBitArray(const ConstBitArray &a) : FData(0) {
  TakeOver(a.Release(), true);
}
//..............................................................................
TEBitArray &TEBitArray::TakeOver(TEBitArray &l, bool do_delete) {
  olx_del_arr(FData);
  FCount = l.FCount;
  FCharCount = l.FCharCount;
  FData = l.FData;
  l.FData = 0;
  if (do_delete) {
    delete &l;
  }
  else {
    l.FCharCount = l.FCount = 0;
  }
  return *this;
}
TEBitArray& TEBitArray::operator = (const TEBitArray& arr) {
  Clear();
  FCount = arr.FCount;
  FCharCount = arr.FCharCount;
  if (arr.FData != 0) {
    FData = new unsigned char[FCharCount];
    memcpy(FData, arr.FData, FCharCount);
  }
  return *this;
}
//..............................................................................
TEBitArray& TEBitArray::operator = (const ConstBitArray& a) {
  return TakeOver(a.Release(), true);
}
//..............................................................................
void TEBitArray::SetSize(size_t newSize) {
  if (FCount == newSize) {
    return;
  }
  size_t chcnt = FCharCount;
  unsigned char* data = FData;
  FCount = newSize;
  FCharCount = newSize / 8 + 1;
  FData = new unsigned char[FCharCount];
  if (data != 0) {
    memcpy(FData, data, olx_min(FCharCount, chcnt));
    if (FCharCount > chcnt) {
      memset(&FData[chcnt], 0, FCharCount - chcnt);
    }
    delete[] data;
  }
  else {
    memset(FData, 0, FCharCount);
  }
}
//..............................................................................
void TEBitArray::Clear() {
  if (FData != 0) {
    delete[] FData;
    FData = 0;
  }
  FCount = FCharCount = 0;
}
//..............................................................................
void TEBitArray::SetAll(bool v) {
  if (FData == 0) {
    return;
  }
  memset(FData, v ? 0xff : 0, FCharCount);
}
//..............................................................................
void TEBitArray::operator << (IInputStream& in) {
  Clear();
  in.Read(&FCount, sizeof(FCount));
  if (FCount != 0) {
    FCharCount = FCount / 8 + 1;
    FData = new unsigned char[FCharCount];
    in.Read(FData, FCharCount);
  }
}
//..............................................................................
void TEBitArray::operator >> (IOutputStream& out) const {
  out.Write(&FCount, sizeof(FCount));
  if (FCount != 0) {
    out.Write(FData, FCharCount);
  }
}
//..............................................................................
bool TEBitArray::operator == (const TEBitArray& arr ) const {
  if (arr.Count() != Count()) {
    return false;
  }
  if (FCharCount == 0) {
    return true;
  }
  for (size_t i = 0; i < FCharCount - 1; i++) {
    if (FData[i] != arr.FData[i]) {
      return false;
    }
  }
  unsigned char last_bits = (1 << (FCount % 8)) -1;
  return (FData[FCharCount - 1] & last_bits) ==
         (arr.FData[FCharCount - 1] & last_bits);
}
//..............................................................................
int TEBitArray::Compare(const TEBitArray& arr) const {
  if (arr.Count() < Count()) {
    if (arr.IsEmpty()) {
      return 1;
    }
    // check for a high bit
    for (size_t i = Count() - 1; i > arr.Count(); i--) {
      if (Get(i)) {
        return 1;
      }
    }
    // comparison
    for (size_t i = arr.Count() - 1; i != InvalidIndex; i--) {
      if (Get(i) && !arr.Get(i)) {
        return 1;
      }
      else if (!Get(i) && arr.Get(i)) {
        return -1;
      }
    }
    return 0;
  }
  else if (arr.Count() == Count()) {
    for (size_t i = Count() - 1; i != InvalidIndex; i--) {
      if (Get(i) && !arr.Get(i)) {
        return 1;
      }
      else if (!Get(i) && arr.Get(i)) {
        return -1;
      }
    }
    return 0;
  }
  else { // reverse te order
    return arr.Compare(*this);
  }
}
//..............................................................................
olxstr TEBitArray::ToBase64String() const {
  olxcstr rv = encoding::base64::encode(FData, FCharCount);
  rv << (char)('0' + FCount%8);
  return rv;
}
//..............................................................................
void TEBitArray::FromBase64String(const olxstr& str) {
  if ((str.Length() % 4) != 1) {
    throw TInvalidArgumentException(__OlxSourceInfo, "representation");
  }
  olxcstr cstr = encoding::base64::decode(olxcstr(str.SubStringTo(str.Length()-1)));
  SetSize(cstr.Length()*8 - (8-(str.GetLast()-'0')));
  memcpy(FData, cstr.raw_str(), cstr.Length());
}
//..............................................................................
TIString TEBitArray::ToString() const {
  olxstr StrRepr(EmptyString(), FCount + 1);
  StrRepr.SetCapacity(Count());
  for (size_t i = 0; i < Count(); i++) {
    StrRepr << (Get(i) ? '1' : '0');
  }
  return StrRepr;
}
//..............................................................................
olxstr TEBitArray::FormatString(uint16_t bitsInSegment) const {
  if (bitsInSegment == 0 || bitsInSegment > 64) {
    throw TInvalidArgumentException(__OlxSourceInfo, "bitsInSegment");
  }
  olxstr StrRepr;
  StrRepr.SetCapacity(Count() + Count() / bitsInSegment + 1);
  uint64_t mask = 0;
  size_t strlen = bitsInSegment / 3 + 1;
  for (size_t i = 0; i < Count(); i++) {
    if ((i % bitsInSegment) == 0 && i > 0) {
      olxstr s(mask);
      StrRepr << s.LeftPadding(strlen, '0');
      mask = 0;
    }
    if (Get(i)) {
      mask |= (uint64_t)1 << (i % bitsInSegment);
    }
  }
  if (mask != 0) {
    olxstr s(mask);
    StrRepr << s.LeftPadding(strlen, '0');
  }
  return StrRepr;
}
//..............................................................................
size_t TEBitArray::CountTrue() const {
  size_t cnt = 0;
  for (size_t i = 0; i < Count(); i++) {
    if (Get(i)) {
      cnt++;
    }
  }
  return cnt;
}
//..............................................................................
