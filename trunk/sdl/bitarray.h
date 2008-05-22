//---------------------------------------------------------------------------

#ifndef bitarrayH
#define bitarrayH
#include "ebase.h"

BeginEsdlNamespace()

class TEBitArray: public IEObject  {
  int *FData, FCount, FIntCount;
public:
  TEBitArray();
  TEBitArray( const TEBitArray& arr);
  TEBitArray(int size);
  virtual ~TEBitArray();
  void Clear();
  void SetSize(int newSize);
  inline int Count()  const        {  return FCount;  }
  bool Get(int index) const;
  void Set(int index, bool v);
  void SetTrue(int index)   {  Set(index, true);  }
  void SetFalse(int index)  {  Set(index, false);  }

  TEBitArray& operator = (const TEBitArray& arr );
  bool operator == (const TEBitArray& arr )  const;
  int Compare(const TEBitArray& arr )  const;

  virtual TIString ToString() const;
  olxstr FormatString( short bitsInSegment );
};

EndEsdlNamespace()
#endif
