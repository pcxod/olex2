/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "exception.h"

void IInputStream::operator >> (IOutputStream &os)  {
  const size_t BufferSize = 64*1024;
  const uint64_t length = GetSize() - GetPosition();
  if( length < BufferSize )  {
    const size_t _length = static_cast<size_t>(length);
    char *Buffer = new char [_length];
    try  {
      Read(Buffer, _length);
      os.Write(Buffer, _length);
    }
    catch( const TExceptionBase& exc )  {
      delete [] Buffer;
      throw TFunctionFailedException(__OlxSourceInfo, exc);
    }
    delete [] Buffer;
    return;
  }
  char *Buffer = new char [BufferSize];
  size_t fullSize = static_cast<size_t>(length/BufferSize);
  size_t partSize = static_cast<size_t>(length%BufferSize);
  try  {
    for( size_t i=0; i < fullSize; i++ )  {
      Read(Buffer, BufferSize);
      os.Write(Buffer, BufferSize);
    }
    Read(Buffer, partSize);
    os.Write(Buffer, partSize);
  }
  catch( const TExceptionBase& exc )  {
   delete [] Buffer;
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }
  delete [] Buffer;
}

IOutputStream& IOutputStream::operator << (IInputStream &is)  {
  const size_t BufferSize = 64*1024;
  const uint64_t length = is.GetSize() - is.GetPosition();
  if( length == 0 )  return *this;
  if( length < BufferSize )  {
    const size_t _length = static_cast<size_t>(length);
    char *Buffer = new char [_length];
    try  {
      is.Read(Buffer, _length);
      Write(Buffer, _length);
    }
    catch( const TExceptionBase& exc )  {
      delete [] Buffer;
      throw TFunctionFailedException(__OlxSourceInfo, exc);
    }
    delete [] Buffer;
    return *this;
  }
  char *Buffer = new char [BufferSize];
  size_t fullSize = static_cast<size_t>(length/BufferSize);
  size_t partSize = static_cast<size_t>(length%BufferSize);
  try  {
    for( size_t i=0; i < fullSize; i++ )  {
      is.Read(Buffer, BufferSize);
      Write(Buffer, BufferSize);
    }
    is.Read(Buffer, partSize);
    Write(Buffer, partSize);
  }
  catch( const TExceptionBase& exc )  {
   delete [] Buffer;
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }
  delete [] Buffer;
  return *this;
}
