#ifndef arraysH
#define arraysH

#ifdef __BORLANDC__
  #include <mem.h>
#else
  #include <stdlib.h>
  #include <string.h>
#endif
#include "evector.h"

BeginEsdlNamespace()
/*
  These arrays are only suitable for primitive types, as memset zeroes the initialised arrays.
  These arrays are just for convinience, as the Data attribute is publically exposed for the 
  performance issues
*/
template <class AE> class TArray1D : public IEObject  {
  const int _Length, MinIndex;
public:
// the indexes provided are inclusive
  TArray1D(int minIndex, int maxIndex) : MinIndex(minIndex), _Length(maxIndex - minIndex + 1)  {
    Data = NULL;
    if( _Length <=0 )  throw TInvalidArgumentException(__OlxSourceInfo, "size");
    Data = new AE[_Length];
    memset( Data, 0, _Length*sizeof(AE) );
  }

  virtual ~TArray1D()  {  if( Data != NULL )  delete [] Data;  }

  void InitWith( const AE& val)  {
    for( int i=0; i < _Length; i++ )
      Data[i] = val;
  }

  inline void FastInitWith( const int val)  {  memset( Data, val, _Length*sizeof(AE) );  }

  // direct access to private member
  AE* Data;

  inline int Length()  const  {  return _Length;  }
  inline bool IsInRange(int ind) const {  return ind >= MinIndex && ((ind-MinIndex) < _Length);  }
  inline AE& operator [] (int index)  const {  return Data[index-MinIndex];  }
  inline AE& Value(int index)  const {  return Data[index-MinIndex];  }
};

// we do not use TArray1D< TArray1D > for performance reasons...
template <class AE> class TArray2D : public IEObject {
  const int Width, MinWidth, Height, MinHeight;
public:
// the indexes provided are inclusive
  TArray2D(int minWidth, int maxWidth, int minHeight, int maxHeight) : 
      MinWidth(minWidth), Width(maxWidth - minWidth + 1), 
      MinHeight(minHeight), Height(maxHeight - minHeight + 1)  {
    Data = NULL;
    if( Width <=0 || Height <= 0 )  
      throw TInvalidArgumentException(__OlxSourceInfo, "size");
    Data = new AE*[Width];
    for( int i=0; i < Width; i++ )  {
      Data[i] = new AE[Height];
      memset( Data[i], 0, Height*sizeof(AE) );
    }
  }

  virtual ~TArray2D()  {
    if( Data == NULL )  return;  // if exception is thrown
    for( int i=0; i < Width; i++ )
      delete [] Data[i];
    delete [] Data;
  }

  void InitWith( const AE& val)  {
    for( int i=0; i < Width; i++ ) 
      for( int j=0; j < Height; j++ )
        Data[i][j] = val;
  }

  void FastInitWith( const int val)  {
    for( int i=0; i < Width; i++ ) 
      memset( Data[i], val, Height*sizeof(AE) );
  }

  inline bool IsInRange(int x, int y) const {  
    return (x >= MinWidth && ((x-MinWidth) < Width)) &&
           (y >= MinHeight && ((y-MinHeight) < Height));  
  }
  inline int GetWidth()  const  {  return Width;  }
  inline int Length1()   const  {  return Width;  }
  inline int GetHeight() const  {  return Height;  }
  inline int Length2()   const  {  return Height;  }
  
  // direct access to private member
  AE** Data;

  inline AE& Value(int x, int y)  {  return Data[x-MinWidth][y-MinHeight];  }
  inline AE& operator () (int x, int y)  {  return Data[x-MinWidth][y-MinHeight];  }
  
  template <class VC>
    inline AE& Value(const TVector<VC>& ind)  {  return Data[(int)(ind[0]-MinWidth)][(int)(ind[1]-MinHeight)];  }
  template <class VC>
    inline AE& operator () (const TVector<VC>& ind)  {  return Data[(int)(ind[0]-MinWidth)][(int)(ind[1]-MinHeight)];  }
};

// we do not use TArray1D<TArray2D<AE>*>* Data for performance reasons
template <class AE> class TArray3D : public IEObject {
  const int MinWidth, Width, MinHeight, Height, MinDepth, Depth;
public:
  // the indexes provided are inclusive
TArray3D(int minWidth, int maxWidth, int minHeight,
      int maxHeight, int minDepth, int maxDepth) : 
        MinWidth(minWidth), Width(maxWidth - minWidth + 1),
        MinHeight(minHeight), Height(maxHeight - minHeight + 1),
        MinDepth(minDepth), Depth(maxDepth - minDepth + 1)
        {
    Data = NULL;
    if( Width <= 0 || Height <= 0 || Depth <= 0)  
      throw TInvalidArgumentException(__OlxSourceInfo, "size");
    Data = new AE**[Width];
    for( int i=0; i < Width; i++ )  {
      Data[i] = new AE*[Height];
      for( int j=0; j < Height; j++ )  {
        Data[i][j] = new AE[Depth];
        memset( Data[i][j], 0, Depth*sizeof(AE) );
      }
    }
  }

  virtual ~TArray3D()  {
    for( int i=0; i < Width; i++ )  {
      for( int j=0; j < Height; j++ )
        delete [] Data[i][j];
      delete [] Data[i];
    }
    delete Data;
  }

  void InitWith( const AE& val)  {
    for( int i=0; i < Width; i++ )
      for( int j=0; j < Height; j++ )
        for( int k=0; k < Depth; k++ )
          Data[i][j][k] = val;
  }

  void FastInitWith( const int val)  {
    for( int i=0; i < Width; i++ )
      for( int j=0; j < Height; j++ )
        memset( Data[i][j], val, Depth*sizeof(AE) );
  }

  inline bool IsInRange(int x, int y, int z) const {  
    return (x >= MinWidth && ((x-MinWidth) < Width)) &&
           (y >= MinHeight && ((y-MinHeight) < Height)) &&
           (z >= MinDepth && ((z-MinDepth) < Depth));  
  }
  template <class vec> inline bool IsInRange(const vec& ind) const {  
    return (ind[0] >= MinWidth && ((ind[0]-MinWidth) < Width)) &&
           (ind[1] >= MinHeight && ((ind[1]-MinHeight) < Height)) &&
           (ind[2] >= MinDepth && ((ind[2]-MinDepth) < Depth));  
  }
  inline int GetWidth()  const  {  return Width;  }
  inline int Length1()   const  {  return Width;  }
  inline int GetHeight() const  {  return Height;  }
  inline int Length2()   const  {  return Height;  }
  inline int GetDepth()  const  {  return Depth;  }
  inline int Length3()   const  {  return Depth;  }

  // direct access to private member
  AE*** Data;

  inline AE& Value(int x, int y, int z)  {  return Data[x-MinWidth][y-MinHeight][z-MinDepth];  }
  inline const AE& Value(int x, int y, int z) const  {  
    return Data[x-MinWidth][y-MinHeight][z-MinDepth];  
  }
  inline AE& operator () (int x, int y, int z)  {  
    return Data[x-MinWidth][y-MinHeight][z-MinDepth];  
  }
  
  inline const AE& operator () (int x, int y, int z) const {  
    return Data[x-MinWidth][y-MinHeight][z-MinDepth];  
  }

  template <class VC> inline AE& Value(const VC& ind)  {  
    return Data[(int)(ind[0]-MinWidth)][(int)(ind[1]-MinHeight)][(int)(ind[2]-MinDepth)];  
  }
  template <class VC> inline const AE& Value(const VC& ind) const {  
    return Data[(int)(ind[0]-MinWidth)][(int)(ind[1]-MinHeight)][(int)(ind[2]-MinDepth)];  
  }
  template <class VC> inline AE& operator () (const VC& ind)  {  
    return Data[(int)(ind[0]-MinWidth)][(int)(ind[1]-MinHeight)][(int)(ind[2]-MinDepth)];  
  }
  template <class VC> inline const AE& operator () (const VC& ind) const {  
    return Data[(int)(ind[0]-MinWidth)][(int)(ind[1]-MinHeight)][(int)(ind[2]-MinDepth)];  
  }
};

class  TArraysTest  {
public:
  static void Test();
};

EndEsdlNamespace()
#endif
