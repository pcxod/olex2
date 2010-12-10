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
  virtual uint64_t GetSize() const = 0;
  virtual uint64_t GetPosition() const = 0;
  virtual void SetPosition(uint64_t newPos) = 0;
  /* if do_throw is true throws an exception if size is larger than size_t (so that ne etc will fail)
  otherwise will return max size_t. Works ONLY for unsigned types! */
  template <typename IT> static IT CheckSize(uint64_t sz, bool do_throw=true)  {
    static const uint64_t _mask = ~(uint64_t)((IT)(~0));
    if( (sz&_mask) != 0 )  {
      if( do_throw )  {
        TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "invalid stream position");
        return 0;  // make the compiler happy
      }
      else
        return ~0;
    }
    else
      return static_cast<IT>(sz);
  }
  static size_t CheckSizeT(uint64_t sz, bool do_throw=true)  {
    return CheckSize<size_t>(sz, do_throw);
  }
  /* if do_throw is true throws an exception if size is larger than size_t (so that ne etc will fail)
  otherwise will return max size_t */
  size_t GetAvailableSizeT(bool do_throw=true) const {
    return CheckSizeT(GetSize() - GetPosition(), do_throw);
  }
};

class IInputStream: public OlxIStream  {
protected:
  inline IInputStream& ValidateRead(size_t amount)  {
    if( (GetPosition() + amount) <= GetSize() )  return *this;
    TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "invalid stream position");
    return *this;  /// just to avoid bloody warnings
  }
public:
  virtual ~IInputStream() {  ;  }
//...................................................................................
  virtual void Read(void *Data, size_t size) = 0;
//...................................................................................
  // return the number of actual bytes read
  size_t SafeRead(void *Data, size_t size)  {
    size_t toread = olx_min(GetAvailableSizeT(false), size);
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
    ValidateRead(sizeof(T)).Read(&v, sizeof(T) );
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
    w += Write(NewLineSequence.raw_str(), NewLineSequence.RawLen());
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
