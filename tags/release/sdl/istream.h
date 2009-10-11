//----------------------------------------------------------------------------//
// namespace TEObjects: Stream
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//
#ifndef __OLX_IStream_H
#define __OLX_IStream_H
//#include "ebase.h"
//#include "estrlist.h"
//TODO: fix that header and then re-include it #include "edlist.h"
//---------------------------------------------------------------------------

BeginEsdlNamespace()

class OlxIStream;
class IInputStream;
class IOutputStream;

class OlxIStream  {
public:
  virtual ~OlxIStream()  {  ;  }
  // these functions must throw an exception if an error happens
  virtual size_t GetSize() const = 0;
  virtual size_t GetPosition() const = 0;
  virtual void SetPosition(size_t newPos) = 0;
  inline void IncPosition() {  SetPosition(GetPosition()+1);  }
  inline void DecPosition() {  SetPosition(GetPosition()-1);  }
};

class IInputStream: public OlxIStream  {
protected:
  inline IInputStream& ValidateRead(size_t amount)  {
    if( (GetPosition() + amount) <= GetSize() )  return *this;
    TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "inalid stream position");
    return *this;  /// just to avoid bloody warnings
  }
public:
  virtual ~IInputStream() {  ;  }
//...................................................................................
  virtual void Read(void *Data, size_t size) = 0;
//...................................................................................
  // return the number of actual bytes read
  size_t SafeRead(void *Data, size_t size)  {
    size_t toread = olx_min(GetSize()-GetPosition(), size);
    if( toread == 0 )  return 0;
    Read(Data, toread);
    return toread;
  }
//...................................................................................
  inline char ReadCc() {
    char cc;
    ValidateRead(sizeof(char)).Read(&cc, sizeof(char));
    return cc;
  }
//...................................................................................
  inline wchar_t ReadWc()  {  
    wchar_t wc;
    ValidateRead(sizeof(wchar_t)).Read(&wc, sizeof(wchar_t));
    return wc;
  }
//...................................................................................
  inline olxch ReadCh()  {  
    olxch ch;
    ValidateRead(sizeof(olxch)).Read(&ch, sizeof(olxch));
    return ch;
  }
//...................................................................................
  void operator >> (IOutputStream &os);
//...................................................................................
  // beware not to pass classes here!
  template <class T> inline void operator >> (T& v)  {
    ValidateRead(sizeof(T)).Read( &v, sizeof(T) );
  }
};

class IOutputStream: public OlxIStream  {
public:
  virtual ~IOutputStream() {  ;  }
//...................................................................................
  virtual size_t Write(const void *Data, size_t size) = 0;
//...................................................................................
  virtual inline size_t Writenl(const void *Data, size_t size)  {
    size_t w = Write(Data, size);
    w += Write(NewLineSequence, NewLineSequenceLength);
    return w;
  }
//...................................................................................
  virtual void Flush()  { }
//...................................................................................
  // these functions must return pointer *this
  IOutputStream& operator << (IInputStream &is);
//...................................................................................
  // beware not to pass classes here
};

EndEsdlNamespace()
#endif
