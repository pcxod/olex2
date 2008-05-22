#ifndef __SDL_outstream
#define __SDL_outstream
#include "exception.h"
#include "datastream.h"
#include <stdio.h>

BeginEsdlNamespace()

class TOutStream : public IDataOutputStream  {
protected:
  virtual size_t GetSize() const  {  return 1;  }
  virtual size_t GetPosition() const  {  return 1;  }
  virtual void SetPosition(size_t newPos)  {
    throw TNotImplementedException(__OlxSourceInfo);
  }
  virtual size_t Write(const void* data, size_t len)  {
    throw TNotImplementedException(__OlxSourceInfo);
  }
public:
  TOutStream()  {  }
  virtual ~TOutStream()  {}
  virtual size_t Write(const olxstr& str);
  virtual size_t Writenl(const olxstr& str);
};

EndEsdlNamespace()

#endif
 
