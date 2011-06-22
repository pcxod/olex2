/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_estrbuffer_H
#define __olx_sdl_estrbuffer_H
#include "edlist.h"
#include "egc.h"
BeginEsdlNamespace()

class TEStrBuffer : public TDirectionalList<olxch> {
public:
  TEStrBuffer(size_t segmentSize=DefBufferSize) : TDirectionalList<olxch>(segmentSize) {}
  TEStrBuffer(const TEStrBuffer& bf) : TDirectionalList<olxch>(bf) {}
  TEStrBuffer(const olxstr& str)  {
    Write(str.raw_str(), str.Length());
  }

  TEStrBuffer(const olxch* str)  {  Write(str, olxstr::o_strlen(str));  }

  virtual ~TEStrBuffer()  {}

  inline TEStrBuffer& operator << (const olxch* str)  {
    Write(str, olxstr::o_strlen(str));
    return *this;
  }

  inline TEStrBuffer& operator << (const olxstr& str)  {
    Write(str.raw_str(), str.Length());
    return *this;
  }

  inline TEStrBuffer& operator << (olxch entity)  {
    Write(entity);
    return *this;
  }

  IOutputStream& operator >> (IOutputStream& os) const {
    TDirectionalListEntry<olxch>* en = GetHead();
    if( en == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "no entry at specified position");
    while( en != NULL )  {
      os.Write(en->GetData(), en->RawLen());
      en = en->GetNext();
    }
    return os;
  }
  inline size_t Length() const {  return GetLength();  }
  inline olxch& operator [] (size_t i)  {  return Get(i);  }
};

EndEsdlNamespace()
#endif
