/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_ememstream_H
#define __olx_ememstream_H
#include "ebase.h"
#include "edlist.h"
#include "estrlist.h"
#include "datastream.h"
BeginEsdlNamespace()

class TEMemoryStream: protected TDirectionalList<char>,
                      public IDataInputStream,
                      public IDataOutputStream  {
  size_t Position;
protected:
  void Clear()  {  TDirectionalList<char>::Clear();  Position = 0;  }
  // returns updated position
public:
  TEMemoryStream(long BufferSize=DefBufferSize)
    : TDirectionalList<char>(BufferSize), Position(0)
  {}
  TEMemoryStream(IInputStream& is);
  virtual ~TEMemoryStream()  {}
  //void operator >> (IEOutputStream *os);

  virtual size_t Write(const void *D, size_t count)  {
    if( Position == GetLength() )
        TDirectionalList<char>::Write((const char*)D, count);
    else
      TDirectionalList<char>::Write((const char*)D, Position, count);
    return (Position += count);
  }
  template <class T> inline size_t Write(const T& data) {
    return IDataOutputStream::Write(data);
  }
  virtual uint64_t GetSize() const {  return GetLength();  }
  virtual uint64_t GetPosition() const {  return Position;  }
  virtual void SetPosition(uint64_t pos)  {
#ifdef _DEBUG
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo,
      (size_t)pos, 0, GetLength()+1);
#endif
    Position = OlxIStream::CheckSizeT(pos);
  }
  // returns updated position
  virtual void Read(void *D, size_t count)  {
    TDirectionalList<char>::Read((char*)D, Position, count);
    Position += count;
  }
  void operator >> (IOutputStream &os);

  TEMemoryStream& operator << (IInputStream &is);

  // functions rewind to zero position saves and then restores the position
  void SaveToFile(const olxstr& FN);
  // functions rewind to zero after reading
  void LoadFromFile(const olxstr& FN);

  template <class T> TEMemoryStream& operator >> (T& v)  {
    Read(&v, sizeof(T));
    return *this;
  }

  TEMemoryStream& operator >> (TStrList& v)  {
    v << (IDataInputStream&)*this;
    return *this;
  }

  template <class T> TEMemoryStream& operator << (const T& v)  {
    Write(&v, sizeof(T));
    return *this;
  }

  TEMemoryStream& operator << (const TStrList& v)  {
    v >> *this;
    return *this;
  }
};
// a simple class to read from a memory array
class TEMemoryInputStream : public IDataInputStream {
  // borrowed pointer
  uint8_t const* Data;
  size_t Length;
  size_t Position;
public:
  TEMemoryInputStream(const void* data, size_t length) :
    Data((unsigned char const*)data),
    Length(length),
    Position(0)
    {}

  virtual inline uint64_t GetSize() const {  return Length;  }
  virtual inline uint64_t GetPosition() const {  return Position;  }
  virtual void SetPosition(uint64_t pos)  {
    Position = OlxIStream::CheckSizeT(pos);
  }
  const uint8_t *GetData() const { return &Data[Position]; }
  uint64_t Remaining() { return Length-Position; }
  virtual void Read(void* to, size_t count)  {
#ifdef _DEBUG
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, Position+count, 0, Length+1);
#endif
    memcpy(to, &Data[Position], count);
    Position += count;
  }
};

EndEsdlNamespace()
#endif
