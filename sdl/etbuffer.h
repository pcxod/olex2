/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_ttbuffer_H
#define __olx_sdl_ttbuffer_H
#include "exception.h"

#ifdef __BORLANDC__
  #include <alloc.h>
#else
  #include <stdlib.h>
  #include <string.h>
#endif

BeginEsdlNamespace()

// default buffer size, to be rational to avoid overhead with data associated with new
// int *i = new int takes more memory than just int!
const size_t DefBufferSize = 1024;

template <typename T>
class TTBuffer : public AReferencible  {
  size_t Size, Capacity;
protected:
  T* _Data;
public:
  TTBuffer(size_t capacity=DefBufferSize)  {
    Size = 0;  Capacity = olx_max(1, capacity);
    _Data = olx_malloc<T>(capacity);
  }

  TTBuffer(T* memoryBlockToOwn, size_t size)  {
    Size = Capacity = size;
    _Data = memoryBlockToOwn;
  }

  // copy constructor - note that only the necessary amount of memory is allocated
  TTBuffer(const TTBuffer& entry)  {
    Capacity = Size = entry.GetSize();
    if( Size == 0 )  {
      _Data = NULL;
      return;
    }
    _Data = olx_malloc<T>(Size);
    olx_memcpy(_Data, entry.GetData(), Size);
  }

  virtual ~TTBuffer()  {  olx_free(_Data);  }

  // this function must be used to allocate the memory provieded for the object
  static T* Alloc(size_t count) {  return olx_malloc<T>(count);  }
  // this function must be used to allocate the memory provided for the object
  static TTBuffer* New(const T *bf, size_t count) {
    return new TTBuffer(olx_malloc<T>(count), count);
  }

  void SetCapacity(size_t newSize)  {
    if( (newSize == 0)  || (newSize <= Capacity) )  return;
    _Data = olx_realloc(_Data, newSize);
    Capacity = newSize;
  }

  void SetSize(size_t newSize )  {
    if( newSize > Capacity ) {
      SetCapacity(newSize);
      Size = newSize;
    }
    else
      Size = newSize;
  }

  /* writes data starting from specified offset, if necessary expand the size
  to the required one plus the increment
  returns the new size of the buffer */
  inline size_t Insert(const T* arr, size_t offset, size_t count, size_t increment=0)  {
    if( (Capacity - Size) < count )
      SetCapacity((Capacity - Size) + count + increment);
    // move the region to overwrite
    olx_memcpy(&_Data[offset+count], &_Data[offset], count);
    // write the memory block
    olx_memcpy(&_Data[offset], arr, count);
    Size += count;
    return Size;
  }

  /* writes data to the end of the buffer
  returns the number of written elements if capacity-size < count*/
  inline size_t Write(const T* arr, size_t count)  {
    size_t written = count;
    if( Capacity-Size < count )  {
      written = Capacity-Size;
      olx_memcpy(&_Data[Size], arr, written);
      Size = Capacity;
    }
    else  {
      olx_memcpy(&_Data[Size], arr, written);
      Size += written;
    }
    return written;
  }

  /* writes data starting from specified offset
  returns the number of written elements if capacity-size < count*/
  inline size_t Write(const T* arr, size_t offset, size_t count)  {
    size_t written = count;
    if( Capacity-offset < count )  {
      written = Capacity-offset;
      olx_memcpy(&_Data[offset], arr, written);
      Size = Capacity;
    }
    else  {
      olx_memcpy(&_Data[offset], arr, written);
      if( offset+count > Size )
        Size = offset+count;
    }
    return written;
  }
  // returns the number read elements
  inline size_t Read( T* arr, size_t offset, size_t count) const {
    size_t read = count;
    if( (Size-offset) < count )  {
      read = Size-offset;
      olx_memcpy(arr, &_Data[offset], read);
    }
    else  {
      olx_memcpy(arr, &_Data[offset], read);
    }
    return read;
  }

  // returns 1 if written and 0 if not
  inline size_t Write(const T& entity)  {
    if( Size == Capacity )  return 0;
    _Data[Size] = entity;
    Size ++;
    return 1;
  }

  inline const T& Get(size_t ind ) const {
#ifdef _DEBUG
    TIndexOutOfRangeException::ValidateRange(__POlxSourceINfo, ind, 0, Size);
#endif
    return _Data[ind];
  }

  inline void Set(size_t ind, T& val )  {
#ifdef _DEBUG
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, ind, 0, Size);
#endif
    _Data[ind] = val;
  }

  inline T& Item(size_t ind )  {
#ifdef _DEBUG
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, ind, 0, Size);
#endif
    return _Data[ind];
  }

  inline T& operator [](size_t ind )  {
#ifdef _DEBUG
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, ind, 0, Size);
#endif
    return _Data[ind];
  }

  inline const T* GetData() const {  return _Data;  }
  inline T* Data()  {  return _Data;  }

  inline size_t GetCapacity() const {  return Capacity;  }
  inline size_t GetSize() const {  return Size;  }
  inline size_t RawLen() const {  return Size*sizeof(T);  }
};

EndEsdlNamespace()
#endif
