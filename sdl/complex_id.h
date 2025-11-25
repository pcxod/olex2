/******************************************************************************
* Copyright (c) 2004-2025 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#pragma once
#include "ebase.h"
/* A simple complex ID representation to be used in dictionaries etc. For more
* elements use TArrayList or similar using list comparator. Like:
  typedef typename ListComparator<>::TListComparator<TPrimitiveComparator> cmp_t;
*/
class complex_id_t {
  char* data;
  size_t length;
#ifdef __cpp_variadic_templates
  size_t write_1(size_t offset) {
    return offset;
  }
  template <typename t1, typename... ts>
  size_t write_1(size_t offset, const t1& v1, ts... ids) {
    memcpy(&data[offset], &v1, sizeof(t1));
    return write_1(offset + sizeof(t1), ids...);
  }

  size_t get_size() {
    return 0;
  }
  template <typename t1, typename... ts>
  size_t get_size(const t1 &i, ts... ids) {
    return sizeof(t1) + get_size(ids...);
  }
#else
  template <typename t1>
  size_t write_1(size_t offset, const t1& v1) {
    memcpy(&data[offset], &v1, sizeof(t1));
    return offset + sizeof(t1);
  }
#endif
public:
  complex_id_t()
    : data(0), length(0)
  {}
  
  complex_id_t(const complex_id_t &a)
    : data(0), length(a.length)
  {
    if (length > 0) {
      data = new char[length];
      memcpy(&data[0], a.data, length);
    }
  }

#ifdef __cpp_variadic_templates
  template <typename t1, typename t2, typename... ts>
  complex_id_t(const t1 &i1, const t2 &i2, ts... ids)
  : length(get_size(i1, i2, ids...))
  {
    data = new char[length];
    write_1(0, i1, i2, ids...);
  }
#else
  template <typename t1, typename t2>
  complex_id_t(const t1& i1, const t2& i2)
    : length(sizeof(t1) + sizeof(t2))
  {
    data = new char[length];
    write_1(write_1(0, i1), i1);
  }

  template <typename t1, typename t2, typename t3>
  complex_id_t(const t1& i1, const t2& i2, const t3& i3)
    : length(sizeof(t1) + sizeof(t2) + sizeof(t3))
  {
    data = new char[length];
    write_1(write_1(write_1(0, i1), i2), i3);
  }

  template <typename t1, typename t2, typename t3, typename t4>
  complex_id_t(const t1& i1, const t2& i2, const t3& i3, const t4& i4)
    : length(sizeof(t1) + sizeof(t2) + sizeof(t3) + sizeof(t4))
  {
    data = new char[length];
    write_1(write_1(write_1(write_1(0, i1), i2), i3), i4);
  }
#endif

  ~complex_id_t() {
    if (data != 0) {
      delete data;
    }
  }

  int Compare(const complex_id_t &a) const {
    if (length != a.length) {
      return olx_cmp(length, a.length);
    }
    return memcmp(data, a.data, length);
  }
};
