#ifndef __olx_xl_symspace_H
#define __olx_xl_symspace_H
#include "symmat.h"

template <class MatList>
class TSymSpace  {
  MatList ml;
  const mat3d &cart2cell, &cell2cart, &hkl2cart;
  bool centrosymmetric;
  size_t start;
protected:
  TSymSpace(const TSymSpace& sp, size_t _start) :
    ml(sp.ml), 
    cart2cell(sp.cart2cell),
    cell2cart(sp.cell2cart),
    hkl2cart(sp.hkl2cart),
    start(sp.start+_start),
    centrosymmetric(sp.centrosymmetric)  {}
public:
  TSymSpace(const MatList& _ml,
    const mat3d& _cart2cell,
    const mat3d& _cell2cart,
    const mat3d& _hkl2cart,
    bool _centrosymmetric) :
      ml(_ml), 
      cart2cell(_cart2cell),
      cell2cart(_cell2cart),
      hkl2cart(_hkl2cart),
      start(0),
      centrosymmetric(_centrosymmetric)  {}

  TSymSpace(const TSymSpace& sp) :
    ml(sp.ml), 
    cart2cell(sp.cart2cell),
    cell2cart(sp.cell2cart),
    hkl2cart(sp.hkl2cart),
    start(sp.start),
    centrosymmetric(sp.centrosymmetric)  {}
    
  inline const smatd& operator [] (size_t i) const {  return ml[i+start];  }
  inline size_t Count() const {  return ml.Count()-start;  }
  bool IsCentrosymmetric() const {  return centrosymmetric;  }
  template <typename vec_type> vec3d CellToCart(const vec_type& v) const {
    return vec3d(
      v[0]*cell2cart[0][0] + v[1]*cell2cart[1][0] + v[2]*cell2cart[2][0],
      v[1]*cell2cart[1][1] + v[2]*cell2cart[2][1],
      v[2]*cell2cart[2][2]);
  }
  template <typename vec_type> vec_type& CellToCartI(vec_type& v) const {
    v[0] = v[0]*cell2cart[0][0] + v[1]*cell2cart[1][0] + v[2]*cell2cart[2][0];
    v[1] = v[1]*cell2cart[1][1] + v[2]*cell2cart[2][1];
    v[2] = v[2]*cell2cart[2][2];
    return v;
  }
  template <typename vec_type> vec3d CartToCell(const vec_type& v) const {
    return vec3d(
      v[0]*cart2cell[0][0] + v[1]*cart2cell[1][0] + v[2]*cart2cell[2][0],
      v[1]*cart2cell[1][1] + v[2]*cart2cell[2][1],
      v[2]*cart2cell[2][2]);
  }
  template <typename vec_type> vec_type& CartToCellI(vec_type& v) const {
    v[0] = v[0]*cart2cell[0][0] + v[1]*cart2cell[1][0] + v[2]*cart2cell[2][0];
    v[1] = v[1]*cart2cell[1][1] + v[2]*cart2cell[2][1];
    v[2] = v[2]*cart2cell[2][2];
    return v;
  }
  template <typename vec_type> vec3d HklToCart(const vec_type& v) const {
    return vec3d(
      v[0]*hkl2cart[0][0],
      v[0]*hkl2cart[0][1] + v[1]*hkl2cart[1][1],
      v[0]*hkl2cart[0][2] + v[1]*hkl2cart[1][2] + v[2]*hkl2cart[2][2]
    );
  }

  TSymSpace<MatList> SubListFrom(size_t _start) const {  return TSymSpace<MatList>(*this, _start);  }
};

// the adaptor for complex classes having MatrixCount()/GetMatrix(size_t) methods
template <class MatrixContainer> class MatrixListAdaptor {
  const MatrixContainer& mc;
public:
  MatrixListAdaptor(const MatrixContainer& _mc) : mc(_mc)  {}
  MatrixListAdaptor(const MatrixListAdaptor& mla) : mc(mla.mc)  {}
  inline size_t Count() const {  return mc.MatrixCount();  }
  inline const smatd& operator [](size_t i) const {  return mc.GetMatrix(i);  }
};

#endif
