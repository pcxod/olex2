#ifndef sysabsH
#define sysabsH

#include "xbase.h"
#include "elist.h"
#include "edlist.h"
#include "hkl.h"

BeginXlibNamespace()

/*
template <class DP>  class TRefList : public TDirectionalList<TReflection*>
{
	DP AssociatedData;
public:
	TRefList(const DP& dp, int segmentSize=DefBufferSize) : TDirectionalList<TReflection*>(segmentSize) {
		AssociatedData = dp;
	}
	virtual ~TRefList()  {  }
	inline TRefList& operator << (const TReflection* ref )  {
	    Write( ref );
	    return *this;
	}
	inline DP& GetAssociatedData()  {  return AssociatedData;  }
	double Summ(int index)  {
	double val = 0;
	for( int i=0; i < Length(); i++ )
		val += Item(i).Data()[index];
	return val;
	}
	inline int Length() const  {  return GetLength();  }
	inline TReflection& operator [] (int i)      {  return Get(i);  }
	inline const TReflection& Item(int i)  const {  return Get(i);  }
	BeginIEObjectImplementation()
};

template <class DP> class TRefStatistics  {
  DP Data;
  TEString Name;
  TRefList RefList;
public:
  TRefStatistics(const TEString name, const DP& data)  {
    Data = date;
    Name = name;
  }

};

class TSGAnalysis  {
public:
  void GetPresentSymmetryElements( TTypeList<> );
  void GetLattices( TTypeList<> );

};
  */
EndXlibNamespace()

#endif
