#ifndef __olx_sdl_composite_H
#define __olx_sdl_composite_H
#include "../emath.h"

/* a wrapper class to present a list of matrices as a single matrix. To be used with non empty
list and matrices of the same dimensions */
template <class List, typename NumT> class CompositeMatrix  {
  const List& matrices;
  const size_t row_sz, col_sz, m_row_sz, m_col_sz, col_cnt, row_cnt;
public:
  CompositeMatrix(const List& _matrices, size_t _row_sz, size_t _col_sz) :
    matrices(_matrices),
    row_sz(_row_sz), col_sz(_col_sz),
    m_row_sz(matrices[0].RowCount()), m_col_sz(matrices[0].RowCount()),
    col_cnt(col_sz*m_col_sz), row_cnt(row_sz*m_row_sz)
  {
    if( matrices.Count() != row_sz*col_sz )
      throw TInvalidArgumentException(__OlxSourceInfo, "size");
  }
  const NumT& Get(size_t i, size_t j) const {
    return matrices[(i/m_row_sz)*row_sz+j/m_row_sz][i%m_col_sz][j%m_row_sz];
  }
  NumT& Get(size_t i, size_t j)  {
    return matrices[(i/m_row_sz)*row_sz+j/m_row_sz][i%m_col_sz][j%m_row_sz];
  }
  void Set(size_t i, size_t j, const NumT& v)  {
    matrices[(i/m_row_sz)*row_sz+j/m_row_sz][i%m_col_sz][j%m_row_sz] = v;
  }
  size_t ColCount() const {  return col_cnt;  }
  size_t RowCount() const {  return row_cnt;  }
};
/* a wrapper class to represent a list of vectors as a single vector. To be used with non empty
list and all vectors of the same size */
template <class List, typename NumT> class CompositeVector  {
  const List& vertices;
  const size_t vec_sz;
public:
  CompositeVector(const List& _vertices) : vertices(_vertices), vec_sz(vertices[0].Count()) {}
  size_t Count() const {  return vertices.Count()*vec_sz;  }
  const NumT& Get(size_t i) const {  return  vertices[i/vec_sz][i%vec_sz];  }
  NumT& Get(size_t i)  {  return  vertices[i/vec_sz][i%vec_sz];  }
  void Set(size_t i, const NumT& v)  {  vertices[i/vec_sz][i%vec_sz] = v;  }
  NumT& operator [] (size_t i)  {  return Get(i);  }
  const NumT& operator [] (size_t i) const {  return Get(i);  }
};

/* vector based on a matrix */
template <class MatT, typename NumT> class MatrixVector  {
  const MatT& matrix;
  const size_t size;
public:
  MatrixVector(const MatT& _matrix) :
	 matrix(_matrix) , size(matrix.RowCount()*matrix.RowCount()) {}
  size_t Count() const {  return size;  }
  const NumT& Get(size_t i) const {  return  matrix[i/matrix.ColCout()][i%matrix.ColCout()];  }
  NumT& Get(size_t i)  {  return  matrix[i/matrix.ColCout()][i%matrix.ColCout()];  }
  void Set(size_t i, const NumT& v)  {  matrix[i/matrix.ColCout()][i%matrix.ColCout()] = v;  }
  NumT& operator [] (size_t i)  {  return Get(i);  }
  const NumT& operator [] (size_t i) const {  return Get(i);  }
};

/* vector const plain array */
template <typename NumT> class ConstPlainVector  {
  NumT const* data;
  const size_t size;
public:
  ConstPlainVector(const NumT* _data, size_t sz) :	data(_data) , size(sz) {}
  size_t Count() const {  return size;  }
  const NumT& Get(size_t i) const {  return  data[i];  }
  const NumT& operator [] (size_t i) const {  return Get(i);  }
};

/* vector plain array */
template <typename NumT> class PlainVector  {
  NumT const *data;
  const size_t size;
public:
  PlainVector(const NumT* _data, size_t sz) :	data(_data) , size(sz) {}
  size_t Count() const {  return size;  }
  const NumT& Get(size_t i) const {  return  data[i];  }
  NumT& Get(size_t i)  {  return  data[i];  }
  void Set(size_t i, const NumT& v)  {  data[i] = v;  }
  NumT& operator [] (size_t i)  {  return Get(i);  }
  const NumT& operator [] (size_t i) const {  return Get(i);  }
};

/* vector/list slice */
template <class VT, typename ItemT> class Slice {
  const VT& data;
  const size_t offset, size;
public:
  Slice(const VT& _data, size_t off, size_t sz) : data(_data), offset(off), size(sz)  {}
  size_t Count() const {  return size;  }
  const ItemT& Get(size_t i) const {  return  data[i+offset];  }
  ItemT& Get(size_t i)  {  return  data[i+offset];  }
  void Set(size_t i, const ItemT& v)  {  data[i+offset] = v;  }
  ItemT& operator [] (size_t i)  {  return Get(i);  }
  const ItemT& operator [] (size_t i) const {  return Get(i);  }
};
/* const vector/list slice */
template <class VT, typename ItemT> class ConstSlice {
  const VT& data;
  const size_t offset, size;
public:
  ConstSlice(const VT& _data, size_t off, size_t sz) : data(_data), offset(off), size(sz)  {}
  size_t Count() const {  return size;  }
  const ItemT& Get(size_t i) const {  return  data[i+offset];  }
  const ItemT& operator [] (size_t i) const {  return Get(i);  }
};

/* matrix based on a vector */
template <class VecT, typename NumT> class VectorMatrix  {
  const VecT& vector;
  const size_t row_sz, col_sz;
public:
  VectorMatrix(const VecT& _vector, size_t _row_sz, size_t _col_sz) :
    vector(_vector), row_sz(_row_sz), col_sz(_col_sz)  {}
  size_t ColCount() const {  return col_sz;  }
  size_t RowCount() const {  return row_sz;  }
  bool IsEmpty() const {  return coll_sz == 0 || row_sz == 0;  }
  const NumT& Get(size_t i, size_t j) const {  return  vector[i*col_sz+j];  }
  NumT& Get(size_t i, size_t j)  {  return  vector[i*col_sz+j];  }
  void Set(size_t i, size_t j, const NumT& v)  {  vector[i*col_sz+j] = v;  }
  NumT& operator () (size_t i, size_t j)  {  return Get(i,j);  }
  const NumT& operator () (size_t i, size_t j) const {  return Get(i,j);  }
};

template <typename NumT> class PlainMatrix : public VectorMatrix<NumT*,NumT>  {
public:
  PlainMatrix(NumT* _data, size_t _row_sz, size_t _col_sz) :
    VectorMatrix<NumT*,NumT>(_data, _row_sz, _col_sz)  {}
};

#endif
