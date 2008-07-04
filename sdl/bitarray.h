//---------------------------------------------------------------------------

#ifndef bitarrayH
#define bitarrayH
#include "ebase.h"
#include "istream.h"

BeginEsdlNamespace()

class TEBitArray: public IEObject  {
  uint32_t *FData, FCount, FIntCount;
  static const int IntBitSize;
public:
  TEBitArray();
  TEBitArray( const TEBitArray& arr);
  TEBitArray(int size);
  TEBitArray(const char* data, size_t size);
  virtual ~TEBitArray();
  void Clear();
  void SetSize(int newSize);
  inline int Count()  const        {  return (int)FCount;  }
  bool Get(int index) const  {
    int intIndex = index/IntBitSize;
    int bitIndex = 1 << index%IntBitSize;
#ifdef _OLX_DEBUG
    TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, intIndex, 0, FIntCount);
#endif
    return (FData[intIndex] & bitIndex) != 0;
  }
  inline void Set(int index, bool v)  {
    int intIndex = index/IntBitSize;
    int bitIndex = 1 << index%IntBitSize;
#ifdef _OLX_DEBUG
    TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, intIndex, 0, FIntCount);
#endif
    if( !v )  FData[intIndex] &= ~bitIndex;
    else      FData[intIndex] |= bitIndex;
  }
  inline void SetTrue(int index)   {  
    int intIndex = index/IntBitSize;
    int bitIndex = 1 << index%IntBitSize;
#ifdef _OLX_DEBUG
    TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, intIndex, 0, FIntCount);
#endif
    FData[intIndex] |= bitIndex;
  }
  inline void SetFalse(int index)  {  
    int intIndex = index/IntBitSize;
    int bitIndex = 1 << index%IntBitSize;
#ifdef _OLX_DEBUG
    TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, intIndex, 0, FIntCount);
#endif
    FData[intIndex] &= ~bitIndex;
  }

  inline const uint32_t* GetData() const {  return FData;  }
  inline int CharCount()           const {  return FIntCount*sizeof(uint32_t);  }
  inline int IntCount()            const {  return FIntCount;  }
  inline int IntSize()             const {  return sizeof(uint32_t);  }

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
