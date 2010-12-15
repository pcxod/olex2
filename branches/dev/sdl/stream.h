//----------------------------------------------------------------------------//
// namespace TEObjects: Stream
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//
#ifndef estreamH
#define estreamH
//#include "ebase.h"
//#include "estrlist.h"
//TODO: fix that header and then re-include it #include "edlist.h"
//---------------------------------------------------------------------------

BeginEsdlNamespace()

class IEStream;
class IEInputStream;
class IEOutputStream;

class IEStream  {
public:
  virtual ~IEStream()  {  ;  }
  virtual long Size() const = 0;
  virtual long GetPosition() const = 0;
  virtual bool SetPosition(long newPos) = 0;
};

class IEInputStream: public IEStream  {
public:
  virtual ~IEInputStream() {  ;  }
  virtual long Read(void *Data, size_t size) = 0;
  virtual long Read(olxch *Data, size_t size)  {
    return Read( (void*)Data, size*sizeof(olxch) );
  }
  // this function normally returns pointer *this
  virtual IEInputStream& operator >> (IEOutputStream &os) = 0;
  template <class T> inline IEInputStream& operator >> ( T& v )  {
    Read( &v, sizeof(T) );
    return *this;
  }
};

class IEOutputStream: public IEStream  {
public:
  virtual ~IEOutputStream() {  ;  }
  virtual long Write(const void *Data, size_t size) = 0;
  // to let derived classes to handle unicode
  virtual long Write(const olxch *Data, size_t size)  {
    return Write((const void*)Data, size*sizeof(olxch));
  }
  virtual inline long Writenl(const void *Data, size_t size)  {
    Write(Data, size);
    return size+Write(NewLineSequence, NewLineSequenceLength);
  }
  virtual inline long Writenl(const olxch *Data, size_t size)  {
    Write((const void*)Data, size*sizeof(olxch));
    return size+Write(NewLineSequence, NewLineSequenceLength);
  }
  virtual void Flush()  { }
  // this function normally returns pointer *this
  virtual IEOutputStream& operator << (IEInputStream &is) = 0;
  template <class T> inline IEOutputStream& operator << ( const T& v )  {
    Write( &v, sizeof(T) );
    return *this;
  }
};

EndEsdlNamespace()
#endif
