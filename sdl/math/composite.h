/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_composite_H
#define __olx_sdl_composite_H
#include "../emath.h"

/* a wrapper class to present a list of matrices as a single matrix. To be used
with non empty list and matrices of the same dimensions
*/
struct CompositeMatrix {
  template <class list_t> class CompositeMatrix_  {
    const list_t& matrices;
    typedef typename list_t::list_item_type::number_type number_type;
    const size_t row_sz, col_sz, m_row_sz, m_col_sz, col_cnt, row_cnt;
  public:
    CompositeMatrix_(const list_t& _matrices, size_t _row_sz, size_t _col_sz)
      : matrices(_matrices),
        row_sz(_row_sz), col_sz(_col_sz),
        m_row_sz(matrices[0].RowCount()), m_col_sz(matrices[0].RowCount()),
        col_cnt(col_sz*m_col_sz), row_cnt(row_sz*m_row_sz)
    {
      if( matrices.Count() != row_sz*col_sz )
        throw TInvalidArgumentException(__OlxSourceInfo, "size");
    }
    const number_type& operator ()(size_t i, size_t j) const {
      return matrices[(i/m_row_sz)*row_sz+j/m_row_sz][i%m_col_sz][j%m_row_sz];
    }
    number_type& operator ()(size_t i, size_t j)  {
      return matrices[(i/m_row_sz)*row_sz+j/m_row_sz][i%m_col_sz][j%m_row_sz];
    }
    size_t ColCount() const {  return col_cnt;  }
    size_t RowCount() const {  return row_cnt;  }
  };
  template <class list_t>
  static CompositeMatrix_<list_t> Make(const list_t &l,
    size_t row_sz, size_t col_sz)
  {
    return CompositeMatrix_<list_t>(l, row_sz, col_sz);
  }
};
/* a wrapper class to represent a list of vectors as a single vector. To be
used with non empty list and all vectors of the same size
*/
struct CompositeVector { 
  template <class list_t> class CompositeVector_  {
    const list_t& vertices;
    const size_t vec_sz;
    typedef typename list_t::list_item_type::list_item_type number_type;
    typedef typename list_t::list_item_type::list_item_type list_item_type;
  public:
    CompositeVector_(const list_t& _vertices)
      : vertices(_vertices), vec_sz(vertices[0].Count()) {}
    size_t Count() const {  return vertices.Count()*vec_sz;  }
    const number_type& Get(size_t i) const {
      return  vertices[i/vec_sz][i%vec_sz];
    }
    number_type& Get(size_t i)  {  return  vertices[i/vec_sz][i%vec_sz];  }
    void Set(size_t i, const number_type& v)  {
      vertices[i/vec_sz][i%vec_sz] = v;
    }
    number_type& operator [] (size_t i)  {  return Get(i);  }
    const number_type& operator [] (size_t i) const {  return Get(i);  }
    number_type& operator () (size_t i)  {  return Get(i);  }
    const number_type& operator () (size_t i) const {  return Get(i);  }
  };
  // convenience constructor
  template <typename list_t>
  static CompositeVector_<list_t> Make(const list_t &l) {
    return CompositeVector_<list_t>(l);
  }
};

/* vector based on a matrix */
template <class MatT> class MatrixVector  {
public:
  typedef typename MatT::number_type list_item_type;
  typedef typename MatT::number_type number_type;
private:
  const MatT& matrix;
  const size_t size;
public:
  MatrixVector(const MatT& _matrix) :
    matrix(_matrix) , size(matrix.RowCount()*matrix.RowCount()) {}
  size_t Count() const {  return size;  }
  const number_type& Get(size_t i) const {
    return  matrix[i/matrix.ColCout()][i%matrix.ColCout()];
  }
  number_type& Get(size_t i)  {
    return  matrix[i/matrix.ColCout()][i%matrix.ColCout()];
  }
  void Set(size_t i, const number_type& v)  {
    matrix[i/matrix.ColCout()][i%matrix.ColCout()] = v;
  }
  number_type& operator [] (size_t i)  {  return Get(i);  }
  const number_type& operator [] (size_t i) const {  return Get(i);  }
};

/* vector const plain array */
template <typename NumT> class ConstPlainVector  {
  NumT const* data;
  const size_t size;
public:
  ConstPlainVector(const NumT* _data, size_t sz)
    :	data(_data) , size(sz) {}
  size_t Count() const {  return size;  }
  const NumT& Get(size_t i) const {  return  data[i];  }
  const NumT& operator [] (size_t i) const {  return Get(i);  }
  const NumT& operator () (size_t i) const {  return Get(i);  }
public:
  typedef NumT list_item_type;
  typedef NumT number_type;
};

/* vector plain array */
template <typename NumT> class PlainVector  {
  NumT const *data;
  const size_t size;
public:
  PlainVector(const NumT* _data, size_t sz) : data(_data) , size(sz) {}
  size_t Count() const {  return size;  }
  const NumT& Get(size_t i) const {  return  data[i];  }
  NumT& Get(size_t i)  {  return  data[i];  }
  void Set(size_t i, const NumT& v)  {  data[i] = v;  }
  NumT& operator [] (size_t i)  {  return Get(i);  }
  const NumT& operator [] (size_t i) const {  return Get(i);  }
  NumT& operator () (size_t i)  {  return Get(i);  }
  const NumT& operator () (size_t i) const {  return Get(i);  }
public:
  typedef NumT number_type;
};

/* vector/list slice */
template <class VT> class Slice {
public:
  typedef typename VT::list_item_type list_item_type;
private:
  const VT& data;
  const size_t offset, size;
public:
  Slice(const VT& _data, size_t off, size_t sz)
    : data(_data), offset(off), size(sz)  {}
  size_t Count() const {  return size;  }
  const list_item_type& Get(size_t i) const {  return  data[i+offset];  }
  list_item_type& Get(size_t i)  {  return  data[i+offset];  }
  void Set(size_t i, const list_item_type& v)  {  data[i+offset] = v;  }
  list_item_type& operator [] (size_t i)  {  return Get(i);  }
  const list_item_type& operator [] (size_t i) const {  return Get(i);  }
  list_item_type& operator () (size_t i)  {  return Get(i);  }
  const list_item_type& operator () (size_t i) const {  return Get(i);  }
};
/* const vector/list slice */
template <class VT> class ConstSlice {
public:
  typedef typename VT::list_item_type list_item_type;
private:
  const VT& data;
  const size_t offset, size;
public:
  ConstSlice(const VT& _data, size_t off, size_t sz)
    : data(_data), offset(off), size(sz)  {}
  size_t Count() const {  return size;  }
  const list_item_type& Get(size_t i) const {  return  data[i+offset];  }
  const list_item_type& operator [] (size_t i) const {  return Get(i);  }
  const list_item_type& operator () (size_t i) const {  return Get(i);  }
};

/* matrix based on a vector */
template <class VecT> class VectorMatrix  {
public:
  typedef typename VecT::number_type number_type;
private:
  const VecT& vector;
  const size_t row_sz, col_sz;
public:
  VectorMatrix(const VecT& _vector, size_t _row_sz, size_t _col_sz) :
    vector(_vector), row_sz(_row_sz), col_sz(_col_sz)  {}
  size_t ColCount() const {  return col_sz;  }
  size_t RowCount() const {  return row_sz;  }
  bool IsEmpty() const {  return col_sz == 0 || row_sz == 0;  }
  const number_type& Get(size_t i, size_t j) const {
    return  vector[i*col_sz+j];
  }
  number_type& Get(size_t i, size_t j)  {  return  vector[i*col_sz+j];  }
  void Set(size_t i, size_t j, const number_type& v)  {
    vector[i*col_sz+j] = v;
  }
  number_type& operator () (size_t i, size_t j)  {  return Get(i,j);  }
  const number_type& operator () (size_t i, size_t j) const {
    return Get(i,j);
  }
};

template <typename NumT> class PlainMatrix
  : public VectorMatrix<PlainVector<NumT> >
{
  typedef PlainVector<NumT> vec_t;
  vec_t vec;
public:
  PlainMatrix(NumT* _data, size_t row_sz, size_t col_sz)
    : vec(_data, row_sz*col_sz),
      VectorMatrix<vec_t>(vec, row_sz, col_sz)  {}
};

#endif
