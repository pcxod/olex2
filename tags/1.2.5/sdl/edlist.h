/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_edlist_H
#define __olx_sdl_edlist_H
#include "exception.h"
#include "etbuffer.h"
BeginEsdlNamespace()

template <typename T>
class TDirectionalListEntry : public IEObject  {
  TTBuffer<T>* Data;
  TDirectionalListEntry<T>* NextEntry;
public:
  TDirectionalListEntry(const TDirectionalListEntry& entry)  {
    NextEntry = NULL;
    Data = new TTBuffer<T>(*entry.Data);
  }
  TDirectionalListEntry(size_t size = DefBufferSize)  {
    NextEntry = NULL;
    Data = new TTBuffer<T>(size);
  }
  TDirectionalListEntry(T* memoryBlockToOwn, size_t size)  {
    NextEntry = NULL;
    Data = new TTBuffer<T>(memoryBlockToOwn, size);
  }

  virtual ~TDirectionalListEntry()  {  delete Data;  }

  // returns the number of written elements
  size_t Write(const T* data, size_t count)  {
    return Data->Write(data, count);
  }

  // returns the number of written elements
  size_t Write(const T* data, size_t offset, size_t count)  {
    return Data->Write(data, offset, count);
  }

  // returns the number of read elements
  size_t Read(T* data, size_t offset, size_t count) const {
    return Data->Read(data, offset, count);
  }

  // returns 1 if written and 0 otherwise
  size_t Write(const T& entity)  {
    return Data->Write(entity);
  }

  TDirectionalListEntry* AddEntry(size_t size)  {
    if( NextEntry != NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "already initialised");
    TDirectionalListEntry<T>* e = new TDirectionalListEntry<T>(size);
    NextEntry = e;
    return e;
  }

  TDirectionalListEntry* AddEntry(T* memoryBlockToOwn, size_t size)  {
    if( NextEntry != NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "already initialised");
    return (NextEntry = new TDirectionalListEntry<T>(memoryBlockToOwn, size));
  }

  TDirectionalListEntry* AddEntry(const TDirectionalListEntry& entry)  {
    return (NextEntry = new TDirectionalListEntry<T>(entry));
  }

  TDirectionalListEntry* GetNext() const {  return NextEntry;  }
  const T& Get(size_t ind ) const {  return Data->Get(ind);  }
  T& Item(size_t ind )  {  return Data->Item(ind);  }
  void Set(size_t ind, T& val )  {  Data->Set(ind, val);  }
  const T* GetData() const {  return Data->GetData();  }
  size_t GetSize() const {  return Data->GetSize();  }
  size_t RawLen() const {  return Data->RawLen();  }
  size_t GetCapacity() const {  return Data->GetCapacity();  }
};

template <typename T>
  class TDirectionalList : public IEObject  {
    TDirectionalListEntry<T>* Head, *Tail;
    size_t Length;
    size_t SegmentSize;
  protected:
    void AddEntry(const TDirectionalListEntry<T>& entry)  {
      if( Head == NULL )
        Tail = Head = new TDirectionalListEntry<T>(entry);
      else
        Tail = Tail->AddEntry(entry);
      Length += entry.GetSize();
    }

    void CheckInitialised()  {
      if( Head == NULL )
        Tail = Head = new TDirectionalListEntry<T>(SegmentSize);
    }

    size_t GetSegmentSize() const {  return SegmentSize;  }
    // updates the tail and the length of this object
    void UpdateLength() {
      TDirectionalListEntry<T>* entry = Head;
      Length = 0;
      while( entry )  {
        Length += entry->GetSize();
        if( entry->GetNext() == NULL )  Tail = entry;
        entry = entry->GetNext();
      }
    }

  public:
  TDirectionalList(size_t segmentSize=DefBufferSize)  {
    SegmentSize = segmentSize;
    Length = 0;
    Head = Tail = NULL;
  }

  TDirectionalList(const TDirectionalList& list)  {
     Length = 0;
     Head = Tail = NULL;
     TDirectionalListEntry<T>* entry = list.GetHead();
     SegmentSize = list.GetSegmentSize();
     while( entry != NULL )  {
       AddEntry(*entry);
       entry = entry->GetNext();
     }
  }

  virtual ~TDirectionalList()  {  Clear();  }

  void Clear() {
    TDirectionalListEntry<T>* entry = Head;
    while( entry != NULL )  {
      TDirectionalListEntry<T>* en = entry->GetNext();
      delete entry;
      entry = en;
    }
    Head = Tail = NULL;
    Length = 0;
  }

  TDirectionalListEntry<T>* GetHead() const {  return Head;  }
  TDirectionalListEntry<T>* GetTail() const {  return Tail;  }
  /* returns the entry at specified position and updates position to in the found entry
     so that a caller can use this position to read directly from the entry */
  TDirectionalListEntry<T>* GetEntryAtPosition(size_t& pos) const {
    if( !Head )  return Head;
    TDirectionalListEntry<T> *entry = Head;

    while( pos > entry->GetSize() )  {
      pos -= entry->GetSize();
      entry = entry->GetNext();
    }
    return entry;
  }

  size_t GetLength() const {  return Length;  }

  bool IsEmpty() const {  return (Length == 0);  }

  T& Get(size_t index)  {
#ifdef _DEBUG
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, index, 0, Length+1);
#endif
    TDirectionalListEntry<T>* entry = Head;
    size_t ind = 0;
    while( (ind + entry->GetSize()) <= index )  {
      ind += entry->GetSize();
      entry = entry->GetNext();
      if( entry == NULL )
        throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, Length-1);
    }
    return entry->Item( index-ind );
  }
  // writes at the end od the list
  TDirectionalList& Write(const T* Data, size_t length)  {
    if( Tail == NULL )
      Tail = Head = new TDirectionalListEntry<T>(SegmentSize);
    size_t written=0;
    while( written < length )  {
      written += Tail->Write(&Data[written], length-written);
      if( written < length )  Tail = Tail->AddEntry(SegmentSize);
    }
    Length += length;
    return *this;
  }

  // writes starting from offset overwriting existing data
  TDirectionalList& Write(const T* Data, size_t offset, size_t length)  {
#ifdef _DEBUG
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, offset, 0, Length+1);
#endif
    if( Tail == NULL )
      Tail = Head = new TDirectionalListEntry<T>(SegmentSize);

    size_t firstOffset = offset;
    TDirectionalListEntry<T> *entry = GetEntryAtPosition(firstOffset);
    if( offset+length > Length )
      Length = offset + length;
    size_t written = entry->Write(Data, firstOffset, length);
    while( written < length )  {
      if( entry->GetNext() == NULL )  {
        // allocate one big chunk of memory
        entry = entry->AddEntry(olx_max(length-written, SegmentSize));
        Tail = entry;
      }
      else
        entry = entry->GetNext();
      written += entry->Write(&Data[written], 0, length-written);
    }
    return *this;
  }

  const TDirectionalList& Read(T* Data, size_t from, size_t length) const {
#ifdef _DEBUG
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, from, 0, Length+1);
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, from+length, 0, Length+1);
#endif
    size_t firstOffset = from;
    TDirectionalListEntry<T> *entry = GetEntryAtPosition(firstOffset);
    size_t read = entry->Read(Data, firstOffset, length);
    while( read < length )  {
      entry = entry->GetNext();
      read += entry->Read(&Data[read], 0, length-read);
    }
    return *this;
  }

  TDirectionalList& Write(const T& entity)  {
    if( Tail == NULL )
      Tail = Head = new TDirectionalListEntry<T>();
    while( Tail->Write(entity) == 0 )
      Tail = Tail->AddEntry(SegmentSize);
    Length += 1;
    return *this;
  }

  virtual TIString ToString() const {
    olxstr s(EmptyString(), Length);
    ToString(s);
    return s;
  }
  olxcstr& ToString(olxcstr& cstr) const {
    cstr.SetCapacity(cstr.Length() + Length);
    TDirectionalListEntry<T>* en = Head;
    while( en != NULL )  {
      cstr.Append(en->GetData(), en->GetSize());
      en = en->GetNext();
    }
    return cstr;
  }
  olxwstr& ToString(olxwstr& wstr) const {
    wstr.SetCapacity(wstr.Length() + Length);
    TDirectionalListEntry<T>* en = Head;
    while( en != NULL )  {
      wstr.Append(en->GetData(), en->GetSize());
      en = en->GetNext();
    }
    return wstr;
  }
public:
  typedef T list_item_type;
};
  
EndEsdlNamespace()
#endif
