//----------------------------------------------------------------------------//
// namespace TEObjects: Stream
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//
#ifndef ememstreamH
#define ememstreamH
#include "ebase.h"
#include "edlist.h"
#include "estrlist.h"
#include "datastream.h"
//---------------------------------------------------------------------------

BeginEsdlNamespace()

class TEMemoryStream: protected TDirectionalList<char>, 
                      public IDataInputStream,
                      public IDataOutputStream  {
  long Position;
protected:
  void Clear()  {  TDirectionalList<char>::Clear();  Position = 0;  }
public:
  TEMemoryStream(long BufferSize=DefBufferSize) : TDirectionalList<char>(BufferSize)  {  Position = 0;  }
  TEMemoryStream(IInputStream& is);
  virtual ~TEMemoryStream()  {  }
  //void operator >> (IEOutputStream *os);

  virtual inline size_t GetSize() const        {  return GetLength();  }
  virtual inline size_t GetPosition() const {  return Position;  }
  virtual void SetPosition(size_t pos)  {
#ifdef _OLX_DEBUG
    TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, pos, 0, GetLength()+1);
#endif
    Position = pos;
  }
  // returns updated position
  virtual size_t Write(const void *D, size_t count)  {
    if( Position == GetLength() )
        TDirectionalList<char>::Write((const char*)D, count);
    else
      TDirectionalList<char>::Write((const char*)D, Position, count);
    return (Position += count);
  }
  // returns updated position
  virtual void Read(void *D, size_t count)  {
    TDirectionalList<char>::Read((char*)D, Position, count);
    Position += count;
  }
  void operator >> (IOutputStream &os);
  IOutputStream& operator << (IInputStream &is);

  // functions rewind to zero position saves and then restores the position
  void SaveToFile(const olxstr& FN);
  // functions rewind to zero after reading
  void LoadFromFile(const olxstr& FN);
  template <class T> inline IDataInputStream& operator >> ( T& v )  {
    Read( &v, sizeof(T) );
    return *this;
  }
  inline IDataInputStream& operator >> ( olxstr& v )  {
    v.FromBinaryStream(*this);
    return *this;
  }
  IDataInputStream& operator >> ( TStrList& v )  {
    v << (IDataInputStream&)*this;
    return *this;
  }

  template <class T> inline IDataOutputStream& operator << ( const T& v )  {
    Write( &v, sizeof(T) );
    return *this;
  }
  inline IDataOutputStream& operator << ( const olxstr& v )  {
    v.ToBinaryStream(*this);
    return *this;
  }
  IDataOutputStream& operator << ( const TStrList& v )  {
    v >> *this;
    return *this;
  }
};
// a simple class to read from a memory array
class TEMemoryInputStream : public IDataInputStream {
  // borrowed pointed
  unsigned char const* Data;
  size_t Length;
  long Position;
public:
  TEMemoryInputStream(const void* data, size_t length) : 
      Data((unsigned char const*)data), 
      Position(0),
      Length(length) {  }
  
  virtual inline size_t GetSize() const        {  return Length;  }
  virtual inline size_t GetPosition() const {  return Position;  }
  virtual void SetPosition(size_t pos)  {
#ifdef _OLX_DEBUG
    TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, pos, 0, Length+1);
#endif
    Position = pos;
  }
  virtual void Read(void* to, size_t count)  {
#ifdef _OLX_DEBUG
    TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, Position+count, 0, Length+1);
#endif
    memcpy(to, &Data[Position], count);
    Position += count;
  }
};

EndEsdlNamespace()
#endif



