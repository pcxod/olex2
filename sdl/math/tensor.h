/******************************************************************************
* Copyright (c) 2020-2020 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_tensor_H
#define __olx_sdl_tesor_H

namespace esdl { namespace tensor {
  namespace utils {
    static size_t calc_multiplicity(const size_t *idx, size_t sz) {
      size_t mult = 1;
      for (size_t i = 0; i < 3; i++) {
        mult *= olx_factorial_t<size_t>(idx[i]);
      }
      return olx_factorial_t<size_t>(sz) / mult;
    }
  } // tensor utils
  typedef TSizeList index_t;
  typedef TArrayList<index_t> index_list_t;

  template <class heir_t, typename FloatType>
  class tensor_base {
    heir_t &self() { return *(heir_t*)this; }
    const heir_t &self() const { return *(heir_t*)this; }
  protected:
    /* https://en.wikipedia.org/wiki/Heap%27s_algorithm
    initialises equivalent map indices
    */
    static void init_index(size_t k, index_t &idx, size_t linear_index) {
      if (k == 1) {
        heir_t::get_linear_index_(idx) = linear_index;
      }
      else {
        init_index(k - 1, idx, linear_index);
        for (size_t i = 0; i < k - 1; i++) {
          if ((k & 1) == 1) { // odd?
            idx.Swap(0, k - 1);
          }
          else {
            idx.Swap(i, k - 1);
          }
          init_index(k - 1, idx, linear_index);
        }
      }
    }
    /* initialises the rank-D index to linear index map and the indices
    multiplicity
    */
    static void init_map_m() {
      const index_list_t &indices = heir_t::get_indices();
      for (size_t i = 0; i < indices.Count(); i++) {
        index_t idx = indices[i];
        init_index(idx.Count(), idx, i);
        size_t mps[3] = { 0, 0, 0 };
        for (size_t j = 0; j < idx.Count(); j++) {
          mps[idx[j]]++;
        }
        get_multiplicity_()[i] = utils::calc_multiplicity(mps, 3);
      }
    }

    static index_t &get_multiplicity_() {
      static index_t multiplicity(heir_t::size());
      return multiplicity;
    }
  public:
    const FloatType & operator [](size_t i) const { return self().data_[i]; }

    FloatType & operator [](size_t i) { return self().data_[i]; }

    static size_t get_linear_idx(const index_t &idx) {
      return heir_t::get_linear_index_(idx);
    }

    size_t get_multiplicity(size_t i) const {
      return get_multiplicity_()[i];
    }
    /* this might be required in a multithreaded environment!
    */
    static void initialise() {
      heir_t::get_map();
    }

    FloatType sum_up(const vec3i &h) const {
      FloatType r = 0;
      const index_list_t &indices = heir_t::get_indices();
      for (size_t i = 0; i < indices.Count(); i++) {
        const index_t &idx = indices[i];
        FloatType prod_h = 1;
        for (size_t j = 0; j < heir_t::rank(); j++) {
          prod_h *= h[idx[j]];
        }
        r += prod_h * get_multiplicity(i) *
          self().data_[heir_t::get_linear_index_(idx)];
      }
      return r;
    }

    typename TArrayList<FloatType>::const_list_type
      gradient_coefficients(const vec3i &h) const
    {
      TArrayList<FloatType> r(heir_t::size());
      const index_list_t &indices = heir_t::get_indices();
      for (size_t i = 0; i < indices.Count(); i++) {
        const index_t &idx = indices[i];
        FloatType prod_h = 1;
        for (size_t j = 0; j < heir_t::rank(); j++) {
          prod_h *= h[idx[j]];
        }
        r[i] = prod_h * get_multiplicity(i);
      }
      return r;
    }

    const TArrayList<FloatType> &data() const {
      return self().data_;
    }
  };

  template <typename FloatType = double>
  class tensor_rank_2_t : public tensor_base<tensor_rank_2_t<FloatType>, FloatType> {
    typedef tensor_base<tensor_rank_2_t<FloatType>, FloatType> parent_t;
    friend class tensor_base<tensor_rank_2_t<FloatType>, FloatType>;
    TArrayList<FloatType> data_;
    /*
    should be to be identical with higher order
    0 0, 0 1, 0 2, 1 1, 1 2, 2 2
    but keep in cctbx format for simpler testing:
    0 0, 1 1, 2 2, 0 1, 0 2, 1 2
    */
    static size_t **& get_map_() {
      static size_t ** map = 0;
      return map;
    }

    static size_t **& get_map() {
      size_t **& r = get_map_();
      if (r == 0) {
        r = build_map();
        /* generic procedure
        parent_t::init_map_m();
        */
        // override to match shelxl
        r[0][0] = 0; r[0][1] = 3; r[0][2] = 5;
        r[1][0] = 3; r[1][1] = 1; r[1][2] = 4;
        r[2][0] = 5; r[2][1] = 4; r[2][2] = 2;
        parent_t::get_multiplicity_()[0] = 1;
        parent_t::get_multiplicity_()[1] = 1;
        parent_t::get_multiplicity_()[2] = 1;
        parent_t::get_multiplicity_()[3] = 2;
        parent_t::get_multiplicity_()[4] = 2;
        parent_t::get_multiplicity_()[5] = 2;
      }
      return r;
    }

    static size_t &get_linear_index_(const index_t &idx) {
      return get_map()[idx[0]][idx[1]];
    }

    static size_t ** build_map() {
      size_t ** map = new size_t *[3];
      for (size_t i = 0; i < 3; i++) {
        map[i] = new size_t[3];
      }
      return map;
    }
  public:
    tensor_rank_2_t()
      : data_(6, olx_list_init::zero())
    {}

    tensor_rank_2_t(const TArrayList<FloatType> &data)
      : data_(data)
    {
      if (data_.Count() != size()) {
        throw TInvalidArgumentException(__OlxSourceInfo, "data size");
      }
    }

    const FloatType & operator ()(size_t i, size_t j) const {
      return data_[get_map()[i][j]];
    }
    FloatType & operator ()(size_t i, size_t j) {
      return data_[get_map()[i][j]];
    }

    static const index_list_t &get_indices() {
      static index_list_t indices;
      if (indices.IsEmpty()) {
        indices.SetCount(size());
        for (size_t i = 0; i < size(); i++) {
          indices[i].SetCount(2);
        }
        indices[0][0] = 0; indices[0][1] = 0;
        indices[1][0] = 1; indices[1][1] = 1;
        indices[2][0] = 2; indices[2][1] = 2;
        indices[3][0] = 0; indices[3][1] = 1;
        indices[4][0] = 0; indices[4][1] = 2;
        indices[5][0] = 1; indices[5][1] = 2;

        /* overriding to match cctbx
          for (int i = 0, idx = 0; i < 3; i++) {
            for (int j = i; j < 3; j++, idx++) {
              indices[idx].SetCount(2);
              indices[idx][0] = i;
              indices[idx][1] = j;
            }
          }
          */
      }
      return indices;
    }

    static typename TArrayList<FloatType>::const_list_type
      get_transform(const index_t &idx, const mat3i &rm)
    {
      tensor_rank_2_t result;
      int i = idx[0], j = idx[1];
      for (int r = 0; r < 3; r++) {
        for (int s = 0; s < 3; s++) {
          result(s, r) += rm(i, r)*rm(j, s);
        }
      }
      return result.data_;
    }

    static size_t rank() { return 2; }
    static size_t size() { return 6; }

    static size_t linearise(size_t i, size_t j) {
      return get_map()[i][j];
    }

    static void cleanup() {
      size_t **map = get_map_();
      if (map != 0) {
        get_map_() = 0;
        for (size_t i = 0; i < 3; i++) {
          delete[] map[i];
        }
        delete map;
      }
    }
  }; // class scitbx::matrx::tensors::tensor_rank_2

  template <typename FloatType = double>
  class tensor_rank_3_t : public tensor_base<tensor_rank_3_t<FloatType>, FloatType> {
    typedef tensor_base<tensor_rank_3_t<FloatType>, FloatType> parent_t;
    friend class tensor_base<tensor_rank_3_t<FloatType>, FloatType>;
    TArrayList<FloatType> data_;
    /*
    0 0 0, 0 0 1, 0 0 2, 0 1 1, 0 1 2
    0 2 2, 1 1 1, 1 1 2, 1 2 2, 2 2 2
    */
  protected:
    static size_t ***& get_map_() {
      static size_t *** map = 0;
      return map;
    }

    static size_t ***& get_map() {
      size_t ***& r = get_map_();
      if (r == 0) {
        r = build_map();
        parent_t::init_map_m();
      }
      return r;
    }

    static size_t &get_linear_index_(const index_t &idx) {
      return get_map()[idx[0]][idx[1]][idx[2]];
    }

    static size_t *** build_map() {
      size_t *** map = new size_t **[rank()];
      for (size_t i = 0; i < 3; i++) {
        map[i] = new size_t*[3];
        for (size_t j = 0; j < 3; j++) {
          map[i][j] = new size_t[3];
        }
      }
      return map;
    }
  public:
    tensor_rank_3_t()
      : data_(10, olx_list_init::zero())
    {}

    tensor_rank_3_t(const TArrayList<FloatType> &data)
      : data_(data)
    {
      if (data_.Count() != size()) {
        throw TInvalidArgumentException(__OlxSourceInfo, "data size");
      }
    }

    const FloatType & operator ()(size_t i, size_t j, size_t k) const {
      return data_[get_map()[i][j][k]];
    }
    FloatType & operator ()(size_t i, size_t j, size_t k) {
      return data_[get_map()[i][j][k]];
    }

    static const index_list_t &get_indices() {
      static index_list_t indices;
      if (indices.IsEmpty()) {
        indices.SetCount(size());
        for (int i = 0, idx = 0; i < 3; i++) {
          for (int j = i; j < 3; j++) {
            for (int k = j; k < 3; k++, idx++) {
              indices[idx].SetCount(rank());
              indices[idx][0] = i;
              indices[idx][1] = j;
              indices[idx][2] = k;
            }
          }
        }
      }
      return indices;
    }

    static typename TArrayList<FloatType>::const_list_type
      get_transform(const index_t &idx, const mat3i &rm)
    {
      tensor_rank_3_t result;
      int i = idx[0], j = idx[1], k = idx[2];
      for (int r = 0; r < 3; r++) {
        for (int s = 0; s < 3; s++) {
          for (int t = 0; t < 3; t++) {
            result(r, s, t) += rm(i, r)*rm(j, s)*rm(k, t);
          }
        }
      }
      return result.data_;
    }

    static size_t rank() { return 3; }
    static size_t size() { return 10; }

    static size_t linearise(size_t i, size_t j, size_t k) {
      return get_map()[i][j][k];
    }

    static void cleanup() {
      size_t ***map = get_map_();
      if (map != 0) {
        get_map_() = 0;
        for (size_t i = 0; i < 3; i++) {
          for (size_t j = 0; j < 3; j++) {
            delete[] map[i][j];
          }
          delete[] map[i];
        }
        delete map;
      }
    }
  }; // class scitbx::matrix::tensors::tensor_rank_3_t

  template <typename FloatType = double>
  class tensor_rank_4_t : public tensor_base<tensor_rank_4_t<FloatType>, FloatType> {
    typedef tensor_base<tensor_rank_4_t<FloatType>, FloatType> parent_t;
    friend class tensor_base<tensor_rank_4_t<FloatType>, FloatType>;
    TArrayList<FloatType> data_;
    /*
    0 0 0 0, 0 0 0 1, 0 0 0 2, 0 0 1 1, 0 0 1 2
    0 0 2 2, 0 1 1 1, 0 1 1 2, 0 1 2 2, 0 2 2 2
    1 1 1 1, 1 1 1 2, 1 1 2 2, 1 2 2 2, 2 2 2 2
    */
    static size_t ****& get_map_() {
      static size_t **** map = 0;
      return map;
    }

    static size_t ****& get_map() {
      size_t ****& r = get_map_();
      if (r == 0) {
        r = build_map();
        parent_t::init_map_m();
      }
      return r;
    }

    static size_t &get_linear_index_(const index_t &idx) {
      return get_map()[idx[0]][idx[1]][idx[2]][idx[3]];
    }

    static size_t **** build_map() {
      size_t **** map = new size_t ***[3];
      for (size_t i = 0; i < 3; i++) {
        map[i] = new size_t **[3];
        for (size_t j = 0; j < 3; j++) {
          map[i][j] = new size_t *[3];
          for (size_t k = 0; k < 3; k++) {
            map[i][j][k] = new size_t[3];
          }
        }
      }
      return map;
    }
  public:
    tensor_rank_4_t()
      : data_(15, olx_list_init::zero())
    {}

    tensor_rank_4_t(const TArrayList<FloatType> &data)
      : data_(data)
    {
      if (data_.Count() != size()) {
        throw TInvalidArgumentException(__OlxSourceInfo, "data size");
      }
    }

    const FloatType & operator ()(size_t i, size_t j, size_t k, size_t l) const {
      return data_[get_map()[i][j][k][l]];
    }
    FloatType & operator ()(size_t i, size_t j, size_t k, size_t l) {
      return data_[get_map()[i][j][k][l]];
    }

    static const index_list_t &get_indices() {
      static index_list_t indices;
      if (indices.IsEmpty()) {
        indices.SetCount(size());
        for (int i = 0, idx = 0; i < 3; i++) {
          for (int j = i; j < 3; j++) {
            for (int k = j; k < 3; k++) {
              for (int l = k; l < 3; l++, idx++) {
                indices[idx].SetCount(4);
                indices[idx][0] = i;
                indices[idx][1] = j;
                indices[idx][2] = k;
                indices[idx][3] = l;
              }
            }
          }
        }
      }
      return indices;
    }

    static typename TArrayList <FloatType>::const_list_type
      get_transform(const index_t &idx, const mat3i &rm)
    {
      tensor_rank_4_t result;
      int i = idx[0], j = idx[1], k = idx[2], l = idx[3];
      for (int r = 0; r < 3; r++) {
        for (int s = 0; s < 3; s++) {
          for (int t = 0; t < 3; t++) {
            for (int u = 0; u < 3; u++) {
              result(r, s, t, u) += rm(i, r)*rm(j, s)*rm(k, t)*rm(l, u);
            }
          }
        }
      }
      return result.data_;
    }

    static size_t rank() { return 4; }
    static size_t size() { return 15; }

    static size_t linearise(size_t i, size_t j, size_t k, size_t l) {
      return get_map()[i][j][k][l];
    }

    static void cleanup() {
      size_t ****map = get_map_();
      if (map != 0) {
        get_map_() = 0;
        for (size_t i = 0; i < 3; i++) {
          for (size_t j = 0; j < 3; j++) {
            for (size_t k = 0; k < 3; k++) {
              delete[] map[i][j][k];
            }
            delete[] map[i][j];
          }
          delete[] map[i];
        }
        delete map;
      }
    }
  }; // class scitbx::matrix::tensors::tensor_rank_4_t

  typedef tensor_rank_2_t<double> tensor_rank_2;
  typedef tensor_rank_3_t<double> tensor_rank_3;
  typedef tensor_rank_4_t<double> tensor_rank_4;

}}  // end esdl::tensor

#endif
