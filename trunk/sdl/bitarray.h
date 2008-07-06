//---------------------------------------------------------------------------

#ifndef bitarrayH
#define bitarrayH
#include "ebase.h"
#include "istream.h"

BeginEsdlNamespace()

class TEBitArray: public IEObject  {
  unsigned char *FData; 
  uint32_t FCount, FCharCount;
public:
  TEBitArray();
  TEBitArray( const TEBitArray& arr);
  TEBitArray(int size);
  // if own is true, data [created with new!] will be deleted automatically 
  TEBitArray(unsigned char* data, size_t size, bool own);
  virtual ~TEBitArray();
  void Clear();
  void SetSize(int newSize);
  inline int Count()  const        {  return (int)FCount;  }
  inline bool operator [] (int index) const  {
    int intIndex = index/8;
    int bitIndex = 1 << index%8;
#ifdef _OLX_DEBUG
    TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, intIndex, 0, FIntCount);
#endif
    return (FData[intIndex] & bitIndex) != 0;
  }
  bool Get(int index) const  {
    int intIndex = index/8;
    int bitIndex = 1 << index%8;
#ifdef _OLX_DEBUG
    TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, intIndex, 0, FIntCount);
#endif
    return (FData[intIndex] & bitIndex) != 0;
  }
  inline void Set(int index, bool v)  {
    int intIndex = index/8;
    int bitIndex = 1 << index%8;
#ifdef _OLX_DEBUG
    TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, intIndex, 0, FIntCount);
#endif
    if( !v )  FData[intIndex] &= ~bitIndex;
    else      FData[intIndex] |= bitIndex;
  }
  inline void SetTrue(int index)   {  
    int intIndex = index/8;
    int bitIndex = 1 << index%8;
#ifdef _OLX_DEBUG
    TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, intIndex, 0, FIntCount);
#endif
    FData[intIndex] |= bitIndex;
  }
  inline void SetFalse(int index)  {  
    int intIndex = index/8;
    int bitIndex = 1 << index%8;
#ifdef _OLX_DEBUG
    TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, intIndex, 0, FIntCount);
#endif
    FData[intIndex] &= ~bitIndex;
  }
  void SetAll(bool v);
  inline const unsigned char* GetData() const {  return FData;  }
  inline int CharCount()                const {  return FCharCount;  }

  void operator << (IInputStream& in);
  void operator >> (IOutputStream& out) const;
  
  TEBitArray& operator = (const TEBitArray& arr );
  bool operator == (const TEBitArray& arr )  const;
  int Compare(const TEBitArray& arr )  const;

  virtual TIString ToString() const;
  olxstr FormatString( short bitsInSegment );
};

EndEsdlNamespace()
#endif
