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
namespace olx_array {
  template <typename item_t>
  struct array_1d {
    item_t *data;
    size_t width;
    array_1d()
      : data(0), width(0)
    {}
    array_1d(const array_1d &a)
    : data(0), width(0)
    {
      (*this) = a;
    }
    array_1d(size_t width_, bool init = true)
      : data(0)
    {
      resize(width_, init);
    }
    ~array_1d() { free_(); }
    void free_() {
      if (data != 0) {
        delete[] data;
      }
    }
    void clear() {
      free_();
      data = 0;
      width = 0;
    }
    array_1d &resize(size_t w, bool init) {
      free_();
      width = w;
      data = new item_t[width];
      if (init) {
        memset(data, 0, width*sizeof(item_t));
      }
      return *this;
    }
    item_t &operator [] (size_t i) { return data[i]; }
    const item_t &operator [] (size_t i) const { return data[i]; }
    array_1d &operator = (const array_1d<item_t> &a) {
      if (width != a.width) {
        resize(a.width);
      }
      memcpy(data, a.data, sizeof(item_t)*width);
      return *this;
    }
    template <class T> array_1d &operator = (const array_1d<T> &a) {
      if (width != a.width) {
        resize(a.width, false);
      }
      for (size_t i = 0; i < width; i++) {
        data[i] = a.data[i];
      }
      return *this;
    }
    size_t Count() const { return width; }
  public:
    typedef item_t list_item_type;
  };
  // array [0..width][0..height]
  template <typename item_t>
  struct array_2d {
    item_t **data;
    size_t width, height;
    array_2d()
      : data(0), width(0), height(0)
    {}
    array_2d(const array_2d &a)
      : data(0), width(0), height(0)
    {
      (*this) = a;
    }
    array_2d(size_t width_, size_t height_, bool init = true)
      : data(0)
    {
      resize(width_, height_, init);
    }
    ~array_2d() { free_(); }
    void free_() {
      if (data != 0) {
        for (size_t i = 0; i < width; i++) {
          delete[] data[i];
        }
        delete[] data;
      }
    }
    void clear() {
      free_();
      data = 0;
      width = height = 0;
    }
    array_2d &resize(size_t w, size_t h, bool init) {
      free_();
      width = w;
      height = h;
      data = new item_t*[width];
      for (size_t i = 0; i < width; i++) {
        data[i] = new item_t[height];
        if (init) {
          memset(data[i], 0, height*sizeof(item_t));
        }
      }
      return *this;
    }
    item_t *operator [] (size_t i) { return data[i]; }
    const item_t *operator [] (size_t i) const { return data[i]; }
    array_2d &operator = (const array_2d<item_t> &a) {
      if (width != a.width || height != a.height) {
        resize(a.width, a.height, false);
      }
      for (size_t i = 0; i < width; i++) {
        memcpy(data[i], a.data[i], sizeof(item_t)*height);
      }
      return *this;
    }
    template <class T> array_2d &operator = (const array_2d<T> &a) {
      if (width != a.width || height != a.height) {
        resize(a.width, a.height, false);
      }
      for (size_t i = 0; i < width; i++) {
        for (size_t j = 0; j < height; j++) {
          data[i][j] = a.data[i][j];
        }
      }
      return *this;
    }
  };

  template <typename item_t>
  struct array_3d {
    item_t ***data;
    size_t width, height, depth;
    array_3d()
      : data(0), width(0), height(0), depth(0)
    {}
    array_3d(const array_3d &a)
      : data(0), width(0), height(0), depth(0)
    {
      (*this) = a;
    }
    array_3d(size_t width_, size_t height_, size_t depth_, bool init = true)
      : data(0)
    {
      resize(width_, height_, depth_, init);
    }
    ~array_3d() { free_(); }
    void free_() {
      if (data != 0) {
        for (size_t i = 0; i < width; i++) {
          for (size_t j = 0; j < height; j++) {
            delete[] data[i][j];
          }
          delete[] data[i];
        }
        delete[] data;
      }
    }
    void clear() {
      free_();
      data = 0;
      width = height = depth = 0;
    }
    array_3d & resize(size_t w, size_t h, size_t d, bool init) {
      free_();
      width = w;
      height = h;
      depth = d;
      data = new item_t**[width];
      for (size_t i = 0; i < width; i++) {
        data[i] = new item_t*[height];
        for (size_t j = 0; j < height; j++) {
          data[i][j] = new item_t[depth];
          if (init) {
            memset(data[i][j], 0, depth*sizeof(item_t));
          }
        }
      }
      return *this;
    }
    vec3s dim() const { return vec3s(width, height, depth); }
    item_t **operator [] (size_t i) { return data[i]; }
    item_t * const *operator [] (size_t i) const { return data[i]; }
    array_3d &operator = (const array_3d<item_t> &a) {
      if (width != a.width || height != a.height || depth != a.depth) {
        resize(a.width, a.height, a.depth, false);
      }
      for (size_t i = 0; i < width; i++) {
        for (size_t j = 0; j < height; j++) {
          memcpy(data[i][j], a.data[i][j], sizeof(item_t)*depth);
        }
      }
      return *this;
    }
    template <class T> array_3d &operator = (const array_3d<T> &a) {
      if (width != a.width || height != a.height || depth != a.depth) {
        resize(a.width, a.height, a.depth, false);
      }
      for (size_t i = 0; i < width; i++) {
        for (size_t j = 0; j < height; j++) {
          for (size_t k = 0; k < depth; k++) {
            data[i][j][k] = a.data[i][j][k];
          }
        }
      }
      return *this;
    }
  };
  ////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////
  template <class AE> class TArray1D : public IOlxObject {
    index_t MinIndex;
  public:
    // the indexes provided are inclusive
    TArray1D(index_t minIndex, index_t maxIndex)
      : MinIndex(minIndex)
    {
      if (minIndex > maxIndex) {
        throw TInvalidArgumentException(__OlxSourceInfo, "size");
      }
      Data.resize(maxIndex - minIndex + 1, true);
    }

    void InitWith(const AE &val) {
      for (size_t i = 0; i < Data.width; i++) {
        Data.data[i] = val;
      }
    }

    void FastInitWith(int val) {
      memset(Data, val, Data.width*sizeof(AE));
    }

    array_1d<AE> Data;

    size_t Length() const { return Data.width; }
    size_t Count() const { return Length(); }
    index_t GetMin() const { return MinIndex; }
    bool IsInRange(index_t ind) const {
      return ind >= MinIndex && ((ind - MinIndex) < Length());
    }
    AE& operator [] (index_t index) const { return Value(index); }
    AE& Value(index_t index) const {
#ifdef _DEBUG
      TIndexOutOfRangeException::ValidateRange(
        __POlxSourceInfo, index - MinIndex, 0, Length());
#endif
      return Data[index - MinIndex];
    }
  public:
    typedef AE list_item_type;
  };

  // we do not use TArray1D< TArray1D > for performance reasons...
  template <class AE> class TArray2D : public IOlxObject {
    const index_t MinWidth;
    const index_t MinHeight;
  public:
    // the indexes provided are inclusive
    TArray2D(index_t minWidth, index_t maxWidth,
      index_t minHeight, index_t maxHeight)
      : MinWidth(minWidth), MinHeight(minHeight)
    {
      if (minWidth > maxWidth || minHeight > maxHeight) {
        throw TInvalidArgumentException(__OlxSourceInfo, "size");
      }
      Data.resize(maxWidth - minWidth + 1, maxHeight - minHeight + 1, true);
    }

    void InitWith(const AE& val) {
      for (size_t i = 0; i < Data.height; i++) {
        for (size_t j = 0; j < Data.width; j++) {
          Data.data[i][j] = val;
        }
      }
    }

    void FastInitWith(const int val) {
      for (int i = 0; i < Data.width; i++) {
        memset(Data.data[i], val, Data.height*sizeof(AE));
      }
    }

    bool IsInRange(index_t x, index_t y) const {
      return (x >= MinWidth && ((x - MinWidth) < Data.width)) &&
        (y >= MinHeight && ((y - MinHeight) < Data.height));
    }
    size_t GetWidth() const { return Data.width; }
    size_t Length1() const { return GetWidth(); }
    index_t GetMin1() const { return MinWidth; }
    size_t GetHeight() const { return Data.height; }
    size_t Length2() const { return GetHeight(); }
    index_t GetMin2() const { return MinHeight; }

    array_2d<AE> Data;

    const AE& Value(index_t x, index_t y) const {
#ifdef _DEBUG
      TIndexOutOfRangeException::ValidateRange(
        __POlxSourceInfo, x - MinWidth, 0, GetWidth());
      TIndexOutOfRangeException::ValidateRange(
        __POlxSourceInfo, y - MinHeight, 0, GetHeight());
#endif
      return Data[x - MinWidth][y - MinHeight];
    }
    const AE& operator () (index_t x, index_t y) const {
      return Value(x, y);
    }

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
  template <class AE> class TArray3D : public IOlxObject {
    const index_t MinWidth;
    const index_t MinHeight;
    const index_t MinDepth;
  public:
    template <typename vec_t>
    TArray3D(const vec_t& mind, const vec_t& maxd)
      : MinWidth(mind[0]), MinHeight(mind[1]), MinDepth(mind[2])
      {
        if (mind[0] > maxd[0] || mind[1] > maxd[1] || mind[2] > maxd[2]) {
          throw TInvalidArgumentException(__OlxSourceInfo, "size");
        }
        Data.resize(maxd[0] - mind[0] + 1,
          maxd[1] - mind[1] + 1,
          maxd[2] - mind[2] + 1, true);
      }
      // the indexes provided are inclusive
        TArray3D(index_t minWidth, index_t maxWidth, index_t minHeight,
          index_t maxHeight, index_t minDepth, index_t maxDepth)
        : MinWidth(minWidth), MinHeight(minHeight), MinDepth(minDepth)
      {
        if (minWidth >= maxWidth || minHeight >= maxHeight || minDepth >= maxDepth) {
          throw TInvalidArgumentException(__OlxSourceInfo, "size");
        }
        Data.resize(maxWidth - minWidth + 1,
          maxHeight - minHeight + 1,
          maxDepth - minDepth + 1, true);
      }

      void InitWith(const AE& val) {
        for (size_t i = 0; i < Data.width; i++) {
          for (size_t j = 0; j < Data.height; j++) {
            for (size_t k = 0; k < Data.depth; k++) {
              Data.data[i][j][k] = val;
            }
          }
        }
      }

      void FastInitWith(const int val) {
        for (size_t i = 0; i < Data.width; i++) {
          for (size_t j = 0; j < Data.height; j++) {
            memset(Data.data[i][j], val, Data.depth*sizeof(AE));
          }
        }
      }

      bool IsInRange(index_t x, index_t y, index_t z) const {
        return (x >= MinWidth && ((x - MinWidth) < (index_t)GetWidth())) &&
          (y >= MinHeight && ((y - MinHeight) < (index_t)GetHeight())) &&
          (z >= MinDepth && ((z - MinDepth) < (index_t)GetDepth()));
      }
      template <class vec> bool IsInRange(const vec& ind) const {
        return IsInRange((index_t)ind[0], (index_t)ind[1], (index_t)ind[2]);
      }
      vec3s GetSize() const { return vec3s(GetWidth(), GetHeight(), GetDepth()); }
      size_t GetWidth() const { return Data.width; }
      size_t Length1() const { return GetWidth(); }
      index_t GetMin1() const { return MinWidth; }
      size_t GetHeight() const { return Data.height; }
      size_t Length2() const { return GetHeight(); }
      index_t GetMin2() const { return MinHeight; }
      size_t GetDepth() const { return Data.depth; }
      size_t Length3() const { return GetDepth(); }
      index_t GetMin3() const { return MinDepth; }

      // direct access to private member
      array_3d<AE> Data;

      const AE& Value(index_t x, index_t y, index_t z) const {
#ifdef _DEBUG
        TIndexOutOfRangeException::ValidateRange(
          __POlxSourceInfo, x - MinWidth, 0, GetWidth());
        TIndexOutOfRangeException::ValidateRange(
          __POlxSourceInfo, y - MinHeight, 0, GetHeight());
        TIndexOutOfRangeException::ValidateRange(
          __POlxSourceInfo, z - MinDepth, 0, GetDepth());
#endif
        return Data[x - MinWidth][y - MinHeight][z - MinDepth];
      }
      AE& Value(index_t x, index_t y, index_t z) {
#ifdef _DEBUG
        TIndexOutOfRangeException::ValidateRange(
          __POlxSourceInfo, x - MinWidth, 0, GetWidth());
        TIndexOutOfRangeException::ValidateRange(
          __POlxSourceInfo, y - MinHeight, 0, GetHeight());
        TIndexOutOfRangeException::ValidateRange(
          __POlxSourceInfo, z - MinDepth, 0, GetDepth());
#endif
        return Data[x - MinWidth][y - MinHeight][z - MinDepth];
      }
      const AE& operator () (index_t x, index_t y, index_t z) const {
        return Value(x, y, z);
      }
      AE& operator () (index_t x, index_t y, index_t z) {
        return Value(x, y, z);
      }

      template <class VC> const AE& Value(const VC& ind) const {
        return Value((index_t)ind[0], (index_t)ind[1], (index_t)ind[2]);
      }
      template <class VC> AE& Value(const VC& ind) {
        return Value((index_t)ind[0], (index_t)ind[1], (index_t)ind[2]);
      }
      template <class VC> const AE& operator () (const VC& ind) const {
        return Value((index_t)ind[0], (index_t)ind[1], (index_t)ind[2]);
      }
      template <class VC> AE& operator () (const VC& ind) {
        return Value((index_t)ind[0], (index_t)ind[1], (index_t)ind[2]);
      }
  };

  /* suitable for pimitive types only
  */
  template <class T>
  void copy_map_segment_3(const array_3d<T> &src, array_3d<T> &dst,
    const vec3s &start, const vec3s &end)
  {
    if (start[0] > end[0] || start[1] > end[1] || start[2] > end[2] ||
      src.width + 1 < end[0] || src.height + 1 < end[1] || src.depth + 1 < end[2] ||
      dst.width + 1 < end[0] || dst.height + 1 < end[1] || dst.depth + 1 < end[2])
    {
      throw TInvalidArgumentException(__OlxSrcInfo, "map dimensions/segment");
    }
    for (size_t x = start[0]; x < end[0]; x++) {
      for (size_t y = start[1]; y < end[1]; y++) {
        memcpy(&dst[x][y][start[2]], &src[x][y][start[2]],
          sizeof(T)*(end[2]-start[2]));
      }
    }

  }
} // namespace olx_array
EndEsdlNamespace()
#endif
