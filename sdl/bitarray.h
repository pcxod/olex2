//---------------------------------------------------------------------------

#ifndef bitarrayH
#define bitarrayH
#include "ebase.h"
#include "istream.h"

BeginEsdlNamespace()

class TEBitArray: public IEObject  {
  unsigned char *FData; 
  uint32_t FCount, FCharCount;
  static inline olxch* ByteToHex(unsigned char bt, olxch* bf) {
    bf[0] = (bt&0x0F);
    bf[0] += bf[0] > 9 ? ('A'-10) : '0'; 
    bf[1] = (bt>>4);
    bf[1] += bf[1] > 9 ? ('A'-10) : '0'; 
    return bf;
  }
  static inline unsigned char ByteFromHex(const olxch* bf) {
    char rv = bf[0] - ((bf[0] >= '0' && bf[0] <= '9') ?  '0' : ('A'-10));
    rv |= ( (bf[1] - ((bf[1] >= '0' && bf[1] <= '9') ?  '0' : ('A'-10))) << 4 );
    return rv;
  }

public:
  TEBitArray();
  TEBitArray(const TEBitArray& arr);
  TEBitArray(uint32_t size);
  // if own is true, data [created with new!] will be deleted automatically 
  TEBitArray(unsigned char* data, uint32_t size, bool own);
  virtual ~TEBitArray();
  void Clear();
  void SetSize(uint32_t newSize);
  inline size_t Count() const {  return FCount;  }
  inline bool IsEmpty() const {  return FCount == 0;  }
  inline bool operator [] (size_t index) const  {
    size_t intIndex = index/8;
    size_t bitIndex = 1 << index%8;
#ifdef _OLX_DEBUG
    TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, intIndex, 0, FIntCount);
#endif
    return (FData[intIndex] & bitIndex) != 0;
  }
  bool Get(size_t index) const  {
    size_t intIndex = index/8;
    size_t bitIndex = 1 << index%8;
#ifdef _OLX_DEBUG
    TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, intIndex, 0, FIntCount);
#endif
    return (FData[intIndex] & bitIndex) != 0;
  }
  inline void Set(size_t index, bool v)  {
    size_t intIndex = index/8;
    size_t bitIndex = 1 << index%8;
#ifdef _OLX_DEBUG
    TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, intIndex, 0, FIntCount);
#endif
    if( !v )  FData[intIndex] &= ~bitIndex;
    else      FData[intIndex] |= bitIndex;
  }
  inline void SetTrue(size_t index)   {  
    size_t intIndex = index/8;
    size_t bitIndex = 1 << index%8;
#ifdef _OLX_DEBUG
    TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, intIndex, 0, FIntCount);
#endif
    FData[intIndex] |= bitIndex;
  }
  inline void SetFalse(size_t index)  {  
    size_t intIndex = index/8;
    size_t bitIndex = 1 << index%8;
#ifdef _OLX_DEBUG
    TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, intIndex, 0, FIntCount);
#endif
    FData[intIndex] &= ~bitIndex;
  }
  void SetAll(bool v);
  inline const unsigned char* GetData() const {  return FData;  }
  inline size_t CharCount() const {  return FCharCount;  }

  void operator << (IInputStream& in);
  void operator >> (IOutputStream& out) const;
  
  TEBitArray& operator = (const TEBitArray& arr);
  bool operator == (const TEBitArray& arr )  const;
  int Compare(const TEBitArray& arr )  const;

  olxstr ToHexString() const;
  void FromHexString(const olxstr& str);

  virtual TIString ToString() const;
  olxstr FormatString(uint16_t bitsInSegment);
};

EndEsdlNamespace()
#endif
