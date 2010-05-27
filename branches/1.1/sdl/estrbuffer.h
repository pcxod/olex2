//---------------------------------------------------------------------------
#ifndef estrBufferH
#define estrBufferH
#include "edlist.h"
#include "egc.h"
#include <string.h>

BeginEsdlNamespace()
//---------------------------------------------------------------------------
class TEStrBuffer : public TDirectionalList<olxch> {
public:
  TEStrBuffer(int segmentSize=DefBufferSize) : TDirectionalList<olxch>(segmentSize) {  }

  TEStrBuffer(const olxstr& str )  {
    Write( str.raw_str(), str.Length() );
  }

  TEStrBuffer(const olxch* str )  {  Write( str, olxstr::o_strlen(str) );  }

  virtual ~TEStrBuffer()  {  }

  inline TEStrBuffer& operator << (const olxch* str )  {
    Write( str, olxstr::o_strlen(str) );
    return *this;
  }

  inline TEStrBuffer& operator << (const olxstr& str )  {
    Write( str.raw_str(), str.Length() );
    return *this;
  }


  inline TEStrBuffer& operator << (olxch entity)  {
    Write( entity );
    return *this;
  }

  IOutputStream& operator >> (IOutputStream& os) const {
    TDirectionalListEntry<olxch>* en = GetHead();
    if( en == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "no entry at specified position");
    while( en != NULL )  {
      os.Write( en->GetData(), en->RawLen() );
      en = en->GetNext();
    }
    return os;
  }
  inline size_t Length() const  {  return GetLength();  }
  inline olxch& operator [] (size_t i)  {  return Get(i);  }

};

EndEsdlNamespace()
#endif

