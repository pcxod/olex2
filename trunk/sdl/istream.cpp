#include "exception.h"

void IInputStream::operator >> (IOutputStream &os)  {
  const size_t BufferSize = 64*1024;
  const size_t length = GetSize();
  if( length < BufferSize )  {
    char *Buffer = new char [length];
    try  {
      Read( Buffer, length );
      os.Write( Buffer, length );
    }
    catch( const TExceptionBase& exc )  {
      delete [] Buffer;
      throw TFunctionFailedException(__OlxSourceInfo, exc);
    }
    delete [] Buffer;
    return;
  }
  char *Buffer = new char [BufferSize];
  size_t fullSize = length / BufferSize;
  size_t partSize = length % BufferSize;
  try  {
    for( size_t i=0; i < fullSize; i++ )  {
      Read( Buffer, BufferSize );
      os.Write( Buffer, BufferSize );
    }
    Read( Buffer, partSize );
    os.Write( Buffer, partSize );
  }
  catch( const TExceptionBase& exc )  {
   delete [] Buffer;
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }
  delete [] Buffer;
}

IOutputStream& IOutputStream::operator << (IInputStream &is)  {
  const size_t BufferSize = 64*1024;
  const size_t length = is.GetSize();
  if( length == 0 )  return *this;
  if( length < BufferSize )  {
    char *Buffer = new char [length];
    try  {
      is.Read( Buffer, length );
      Write( Buffer, length );
    }
    catch( const TExceptionBase& exc )  {
      delete [] Buffer;
      throw TFunctionFailedException(__OlxSourceInfo, exc);
    }
    delete [] Buffer;
    return *this;
  }
  char *Buffer = new char [BufferSize];
  size_t fullSize = length / BufferSize;
  size_t partSize = length % BufferSize;
  try  {
    for( size_t i=0; i < fullSize; i++ )  {
      is.Read( Buffer, BufferSize );
      Write( Buffer, BufferSize );
    }
    is.Read( Buffer, partSize );
    Write( Buffer, partSize );
  }
  catch( const TExceptionBase& exc )  {
   delete [] Buffer;
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }
  delete [] Buffer;
  return *this;
}
