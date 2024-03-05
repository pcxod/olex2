/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_outstream_H
#define __olx_sdl_outstream_H
#include "exception.h"
#include "datastream.h"
#include <stdio.h>
BeginEsdlNamespace()

class TOutStream : public IDataOutputStream  {
  static void outstream_putc(char ch)  {  putchar(ch);  }
  static void outstream_putc(wchar_t ch)  {  putwchar(ch);  }
protected:
  virtual uint64_t GetSize() const  {  return 1;  }
  virtual uint64_t GetPosition() const  {  return 1;  }
  virtual void SetPosition(uint64_t newPos)  {
    throw TNotImplementedException(__OlxSourceInfo);
  }
  virtual size_t Write(const void* data, size_t len)  {
    throw TNotImplementedException(__OlxSourceInfo);
  }
  bool SkipPost;
  template <class T> size_t write(const T& str)  {
    if( SkipPost )  {
      SkipPost = false;
      return 0;
    }
    for (size_t i = 0; i < str.Length(); i++) {
      outstream_putc(str[i]);
    }
    return str.Length();
  }
public:
  TOutStream() : SkipPost(false)  {}
  virtual ~TOutStream()  {}
  virtual size_t Write(const TIWString& str)  {  return  write(str);  }
  virtual size_t Write(const TICString& str)  {  return write(str);  }
  DefPropP(bool, SkipPost)
};

EndEsdlNamespace()
#endif
