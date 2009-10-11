#ifndef emempoolH
#define emempoolH
#include "ebase.h"
/* This class is to be used for threads to "allocate" memory
The memory is preallocated and newly created objects instead of using new operator
using this memory
*/


BeginEsdlNamespace()
/*
class TMemoryPool  {
  void* Memory;
  long Size, RemainingSize;
  static TMemoryPool* Instance;
public:
  TMemoryPool(long size)  {
    try  {  Memory = new char[size];  }
    catch( ... )  {  throw TFunctionFailedException(__OlxSourceInfo, "failed to init");  }
  }
  template <class T>
    T* Alloc(int size)  {

    }
};
*/
EndEsdlNamespace()
#endif
