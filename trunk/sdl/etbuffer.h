//---------------------------------------------------------------------------
#ifndef etBufferH
#define etBufferH
#include "elist.h"
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
//---------------------------------------------------------------------------
template <typename T>
  class TTBuffer : public AReferencible  {
      size_t Size, Capacity;
    protected:
      T* _Data;
    public:
      TTBuffer( size_t capacity=DefBufferSize )  {
        Size = 0;  Capacity = olx_max( 1, capacity );
        _Data = (T*)malloc( capacity*sizeof(T) );
        if( !_Data )  throw TOutOfMemoryException(__OlxSourceInfo);
      }

      TTBuffer( T* memoryBlockToOwn, size_t size )  {
        Size = Capacity = size; _Data = memoryBlockToOwn;
      }

      // copy constructor - note that only the necessary amount of memory is allocated
      TTBuffer( const TTBuffer& entry )  {
        Capacity = Size = entry.GetSize();
        if( Size == 0 )  {
          _Data = NULL;
          return;
        }
        _Data = (T*)malloc( Size*sizeof(T) );
        if( !_Data )  throw TOutOfMemoryException(__OlxSourceInfo);
        memcpy( _Data, entry.GetData(), Size*sizeof(T) );
      }

      virtual ~TTBuffer()  {  if( _Data != NULL )  free(_Data);  }

      // this function must be used to allocate the memory provieded for the object
      static T* Alloc(size_t count) {
        T* memb = (T*)malloc( count*sizeof(T) );
        if( memb == NULL )  throw TOutOfMemoryException(__OlxSourceInfo);
        return memb;
      }
      // this function must be used to allocate the memory provieded for the object
      static TTBuffer* New(const T *bf, size_t count) {
        T* memb = (T*)malloc( count*sizeof(T) );
        if( memb == NULL )  throw TOutOfMemoryException(__OlxSourceInfo);
        return new TTBuffer(memb, count);
      }

      void SetCapacity( size_t newSize )  {
        if( (newSize == 0)  || (newSize <= Capacity) )  return;
        _Data = (T*)realloc(_Data, newSize*sizeof(T));
        if( _Data == NULL )  throw TOutOfMemoryException(__OlxSourceInfo);
        Capacity = newSize;
      }

      void SetSize(size_t newSize )  {
        if( newSize > Capacity ) {
          SetCapacity( newSize );
          Size = newSize;
        }  else  {
          Size = newSize;
        }
      }

      /* writes data starting from specified offset, if necessary expand the size
        to the required one plus the increment
         returns the new size of the buffer */
      inline size_t Insert(const T* arr, size_t offset, size_t count, unsigned int increment=0)  {
        if( (Capacity - Size) < count )
          SetCapacity( (Capacity - Size) + count + increment );
        // move the region to overwrite
        memcpy( &_Data[offset+count], &_Data[offset], count*sizeof(T) );
        // write the memory block
        memcpy( &_Data[offset], arr, count*sizeof(T) );
        Size += count;
        return Size;
      }

      /* writes data to the end of the buffer
         returns the number of written elements if capacity-size < count*/
      inline size_t Write(const T* arr, size_t count)  {
        size_t written = count;
        if( Capacity-Size < count )  {
          written = Capacity-Size;
          memcpy( &_Data[Size], arr, written*sizeof(T) );
          Size = Capacity;
        }
        else  {
          memcpy( &_Data[Size], arr, written*sizeof(T) );
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
          memcpy( &_Data[offset], arr, written*sizeof(T) );
          Size = Capacity;
        }
        else  {
          memcpy( &_Data[offset], arr, written*sizeof(T) );
          if( offset+count > Size )
            Size = offset+count;
        }
        return written;
      }
      // returns the number read elements
      inline size_t Read( T* arr, size_t offset, size_t count)  const  {
        size_t read = count;
        if( (Size-offset) < count )  {
          read = Size-offset;
          memcpy( arr, &_Data[offset], read*sizeof(T) );
        }
        else  {
          memcpy( arr, &_Data[offset], read*sizeof(T) );
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

      inline const T& Get(size_t ind )  const {
#ifdef _OLX_DEBUG
        TIndexOutOfRangeException::ValidateRange(__OlxSourceINfo, ind, 0, Size);
#endif
        return _Data[ind];
      }

      inline void Set(size_t ind, T& val )  {
#ifdef _OLX_DEBUG
        TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, ind, 0, Size);
#endif
        _Data[ind] = val;
      }

      inline T& Item(size_t ind )  {
#ifdef _OLX_DEBUG
        TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, ind, 0, Size);
#endif
        return _Data[ind];
      }

      inline T& operator [](size_t ind )  {
#ifdef _OLX_DEBUG
        TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, ind, 0, Size);
#endif
        return _Data[ind];
      }

      inline const T* GetData()   const {  return _Data;      }
      inline T* Data()                  {  return _Data;      }

      inline size_t GetCapacity() const {  return Capacity;  }
      inline size_t GetSize()     const {  return Size;      }
      inline size_t RawLen()      const {  return Size*sizeof(T); }
  };

EndEsdlNamespace()
#endif

