/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_arrays_H
#define __olx_sdl_arrays_H

#ifdef __BORLANDC__
  #include <mem.h>
#else
  #include <stdlib.h>
  #include <string.h>
#endif
#include "evector.h"
#include "threex3.h"
BeginEsdlNamespace()

/*
  These arrays are only suitable for primitive types, as memset zeroes the
 initialised arrays. These arrays are just for convinience, as the Data
 attribute is publically exposed for the performance issues
*/
template <class AE> class TArray1D : public IEObject  {
  index_t MinIndex;
  const size_t _Length;
public:
// the indexes provided are inclusive
  TArray1D(index_t minIndex, index_t maxIndex)
    : MinIndex(minIndex), _Length(maxIndex - minIndex + 1)
  {
    Data = NULL;
    if( minIndex > maxIndex )
      throw TInvalidArgumentException(__OlxSourceInfo, "size");
    Data = new AE[_Length];
    memset(Data, 0, _Length*sizeof(AE));
  }

  virtual ~TArray1D()  {  if( Data != NULL )  delete [] Data;  }

  void InitWith( const AE& val)  {
    for( size_t i=0; i < _Length; i++ )
      Data[i] = val;
  }

  void FastInitWith(const int val)  {
    memset( Data, val, _Length*sizeof(AE) );
  }

  // direct access to "private" member
  AE* Data;

  size_t Length() const {  return _Length;  }
  size_t Count() const { return _Length; }
  index_t GetMin() const {  return MinIndex;  }
  bool IsInRange(index_t ind) const {
    return ind >= MinIndex && ((ind-MinIndex) < _Length);
  }
  AE& operator [] (index_t index) const { return Value(index);  }
  AE& Value(index_t index) const {
#ifdef _DEBUG
    TIndexOutOfRangeException::ValidateRange(
          __POlxSourceInfo, index-MinIndex, 0, _Length);
#endif
    return Data[index-MinIndex];
  }
public:
  typedef AE list_item_type;
};

// we do not use TArray1D< TArray1D > for performance reasons...
template <class AE> class TArray2D : public IEObject {
  const index_t MinWidth;
  const size_t Width;
  const index_t MinHeight;
  const size_t Height;
public:
// the indexes provided are inclusive
  TArray2D(index_t minWidth, index_t maxWidth,
    index_t minHeight, index_t maxHeight)
    : MinWidth(minWidth), Width(maxWidth - minWidth + 1),
      MinHeight(minHeight), Height(maxHeight - minHeight + 1)  {
    Data = NULL;
    if( minWidth > maxWidth || minHeight > maxHeight )
      throw TInvalidArgumentException(__OlxSourceInfo, "size");
    Data = new AE*[Width];
    for( size_t i=0; i < Width; i++ )  {
      Data[i] = new AE[Height];
      memset(Data[i], 0, Height*sizeof(AE));
    }
  }

  virtual ~TArray2D()  {
    if( Data == NULL )  return;  // if exception is thrown
    for( size_t i=0; i < Width; i++ )
      delete [] Data[i];
    delete [] Data;
  }

  void InitWith(const AE& val)  {
    for( int i=0; i < Width; i++ )
      for( int j=0; j < Height; j++ )
        Data[i][j] = val;
  }

  void FastInitWith(const int val)  {
    for( int i=0; i < Width; i++ )
      memset( Data[i], val, Height*sizeof(AE) );
  }

  bool IsInRange(index_t x, index_t y) const {
    return (x >= MinWidth && ((x-MinWidth) < Width)) &&
           (y >= MinHeight && ((y-MinHeight) < Height));
  }
  size_t GetWidth() const {  return Width;  }
  size_t Length1() const {  return Width;  }
  index_t GetMin1() const {  return MinWidth;  }
  size_t GetHeight() const {  return Height;  }
  size_t Length2() const {  return Height;  }
  index_t GetMin2() const {  return MinHeight;  }
  // direct access to private member
  AE** Data;

  const AE& Value(index_t x, index_t y) const {
#ifdef _DEBUG
    TIndexOutOfRangeException::ValidateRange(
      __POlxSourceInfo, x-MinWidth, 0, Width);
    TIndexOutOfRangeException::ValidateRange(
      __POlxSourceInfo, y-MinHeight, 0, Height);
#endif
    return Data[x-MinWidth][y-MinHeight];
  }
  const AE& operator () (index_t x, index_t y) const {  return Value(x, y);  }

  template <class VC>
    AE& Value(const TVector<VC>& ind) const {
      return Value((index_t)ind[0], (index_t)ind[1]);
    }
  template <class VC>
    AE& operator () (const TVector<VC>& ind) const {
      return Value((index_t)ind[0], (index_t)ind[1]);
    }
};

// we do not use TArray1D<TArray2D<AE>*>* Data for performance reasons
template <class AE> class TArray3D : public IEObject {
  const index_t MinWidth;
  const size_t Width;
  const index_t MinHeight;
  const size_t Height;
  const index_t MinDepth;
  const size_t Depth;
  void Init()  {
    Data = new AE**[Width];
    for( size_t i=0; i < Width; i++ )  {
      Data[i] = new AE*[Height];
      for( size_t j=0; j < Height; j++ )  {
        Data[i][j] = new AE[Depth];
        memset(Data[i][j], 0, Depth*sizeof(AE));
      }
    }
  }
public:
  template <typename vec_t>
  TArray3D(const vec_t& mind, const vec_t& maxd)
    : MinWidth(mind[0]), Width(maxd[0] - mind[0] + 1),
      MinHeight(mind[1]), Height(maxd[1] - mind[1] + 1),
      MinDepth(mind[2]), Depth(maxd[2] - mind[2] + 1)
  {
    Data = NULL;
    if( mind[0] > maxd[0] || mind[1] > maxd[1] || mind[2] > maxd[2] )
      throw TInvalidArgumentException(__OlxSourceInfo, "size");
    Init();
  }
  // the indexes provided are inclusive
  TArray3D(index_t minWidth, index_t maxWidth, index_t minHeight,
    index_t maxHeight, index_t minDepth, index_t maxDepth)
    : MinWidth(minWidth), Width(maxWidth - minWidth + 1),
      MinHeight(minHeight), Height(maxHeight - minHeight + 1),
      MinDepth(minDepth), Depth(maxDepth - minDepth + 1)
  {
    Data = NULL;
    if( minWidth >= maxWidth || minHeight >= maxHeight || minDepth >= maxDepth )
      throw TInvalidArgumentException(__OlxSourceInfo, "size");
    Init();
  }

  virtual ~TArray3D()  {
    for( size_t i=0; i < Width; i++ )  {
      for( size_t j=0; j < Height; j++ )
        delete [] Data[i][j];
      delete [] Data[i];
    }
    delete [] Data;
  }

  void InitWith(const AE& val)  {
    for( size_t i=0; i < Width; i++ )
      for( size_t j=0; j < Height; j++ )
        for( size_t k=0; k < Depth; k++ )
          Data[i][j][k] = val;
  }

  void FastInitWith(const int val)  {
    for( size_t i=0; i < Width; i++ )
      for( size_t j=0; j < Height; j++ )
        memset(Data[i][j], val, Depth*sizeof(AE));
  }

  bool IsInRange(index_t x, index_t y, index_t z) const {
    return (x >= MinWidth && ((x-MinWidth) < (index_t)Width)) &&
           (y >= MinHeight && ((y-MinHeight) < (index_t)Height)) &&
           (z >= MinDepth && ((z-MinDepth) < (index_t)Depth));
  }
  template <class vec> bool IsInRange(const vec& ind) const {
    return IsInRange((index_t)ind[0], (index_t)ind[1], (index_t)ind[2]);
  }
  vec3s GetSize() const {  return vec3s(Width, Height, Depth);  }
  size_t GetWidth() const {  return Width;  }
  size_t Length1() const {  return Width;  }
  index_t GetMin1() const {  return MinWidth;  }
  size_t GetHeight() const {  return Height;  }
  size_t Length2() const {  return Height;  }
  index_t GetMin2() const {  return MinHeight;  }
  size_t GetDepth() const {  return Depth;  }
  size_t Length3() const {  return Depth;  }
  index_t GetMin3() const {  return MinDepth;  }

  // direct access to private member
  AE*** Data;

  AE& Value(index_t x, index_t y, index_t z) const {
#ifdef _DEBUG
    TIndexOutOfRangeException::ValidateRange(
      __POlxSourceInfo, x-MinWidth, 0, Width);
    TIndexOutOfRangeException::ValidateRange(
      __POlxSourceInfo, y-MinHeight, 0, Height);
    TIndexOutOfRangeException::ValidateRange(
      __POlxSourceInfo, z-MinDepth, 0, Depth);
#endif
    return Data[x-MinWidth][y-MinHeight][z-MinDepth];
  }
  AE& operator () (index_t x, index_t y, index_t z) const {
    return Value(x,y,z);
  }

  template <class VC> AE& Value(const VC& ind) const {
    return Value((index_t)ind[0], (index_t)ind[1], (index_t)ind[2]);
  }
  template <class VC> AE& operator () (const VC& ind) const {
    return Value((index_t)ind[0], (index_t)ind[1], (index_t)ind[2]);
  }
};

namespace simple {
  template <typename item_t>
  struct array_1d {
    item_t *data;
    const size_t width;
    array_1d()
      : data(NULL), width(0)
    {}
    array_1d(size_t width_, bool init=true)
      : width(width_)
    {
      data = new item_t[width];
      if (init)
        memset(data, 0, width*sizeof(item_t));
    }
    ~array_1d() {
      if (data == NULL) return;
      delete [] data;
    }
    item_t &operator [] (size_t i) { return data[i]; }
    const item_t &operator [] (size_t i) const { return data[i]; }
    size_t Count() const { return width; }
  public:
    typedef item_t list_item_type;
  };
  // array [0..height][0..width]
  template <typename item_t>
  struct array_2d {
    item_t **data;
    const size_t height, width;
    array_2d()
      : data(NULL), height(0), width(0)
    {}
    array_2d(size_t height_, size_t width_, bool init=true)
      : width(width_), height(height_)
    {
      data = new item_t*[height];
      for (size_t i=0; i < height; i++)  {
        data[i] = new item_t[width];
        if (init)
          memset(data[i], 0, width*sizeof(item_t));
      }
    }
    ~array_2d() {
      if (data == NULL) return;
      for( size_t i=0; i < height; i++ )
        delete [] data[i];
      delete [] data;
    }
    item_t *operator [] (size_t i) { return data[i]; }
    const item_t *operator [] (size_t i) const { return data[i]; }
  };

  template <typename item_t>
  struct array_3d {
    item_t ***data;
    const size_t depth, height, width;
    array_3d()
      : data(NULL), depth(0), height(0), width(0)
    {}
    array_3d(size_t depth_, size_t height_, size_t width_, bool init=true)
      : depth(depth_), height(height_), width(width_)
    {
      data = new item_t**[depth];
      for (size_t i=0; i < depth; i++)  {
        data[i] = new item_t*[height];
        for (size_t j=0; j < height; j++) {
          data[i][j] = new item_t[width];
          if (init)
            memset(data[i], 0, width*sizeof(item_t));
        }
      }
    }
    ~array_3d() {
      if (data == NULL) return;
      for( size_t i=0; i < depth; i++ ) {
        for( size_t j=0; j < height; j++ )
          delete [] data[i][j];
        delete [] data[i];
      }
      delete [] data;
    }
    item_t **operator [] (size_t i) { return data[i]; }
    const item_t **operator [] (size_t i) const { return data[i]; }
  };
} // namspace simple
EndEsdlNamespace()
#endif
