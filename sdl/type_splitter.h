/******************************************************************************
* Copyright (c) 2004-2024 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/
#pragma once
namespace esdl {

  /* Allows to call different functions on primitive functions and objects.
  See string implemenation for usage. The main rationale behind is that it was not
  possible to split IOlxObject from the generic template for primitive types...
  */
  struct primitive_type_splitter {
    template <class functor_t>
    struct primitive_type_splitter_ {
      const functor_t& functor;
      primitive_type_splitter_(const functor_t& functor)
        : functor(functor)
      {}

      template <typename x_t>
      void call(const x_t& v) const {
        functor.c_functor(v);
      }
      void call(const bool& v) const { functor.p_functor(v); }
      void call(const char& v) const { functor.p_functor(v); }
      void call(const short& v) const { functor.p_functor(v); }
      void call(const int& v) const { functor.p_functor(v); }
      void call(const long& v) const { functor.p_functor(v); }
      void call(const long long& v) const { functor.p_functor(v); }
      void call(const unsigned char& v) const { functor.p_functor(v); }
      void call(const unsigned short& v) const { functor.p_functor(v); }
      void call(const unsigned int& v) const { functor.p_functor(v); }
      void call(const unsigned long& v) const { functor.p_functor(v); }
      void call(const unsigned long long& v) const { functor.p_functor(v); }
      void call(const float& v) const { functor.p_functor(v); }
      void call(const double& v) const { functor.p_functor(v); }
      void call(const long double& v) const { functor.p_functor(v); }
    };

    template <class functor_t>
    static primitive_type_splitter_<functor_t> make(const functor_t& f) {
      return primitive_type_splitter_<functor_t>(f);
    }
  };

  template <typename T>  struct olx_is_float {  const static bool is = false; };

  template <> struct olx_is_float<float> { const static bool is = true; };
  template <> struct olx_is_float<double> { const static bool is = true; };
  template <> struct olx_is_float<long double> { const static bool is = true; };

  template <typename T>  struct olx_is_primitive { const static bool is = false; };

  template <> struct olx_is_primitive<bool> { const static bool is = true; };
  template <> struct olx_is_primitive<char> { const static bool is = true; };
  template <> struct olx_is_primitive<short> { const static bool is = true; };
  template <> struct olx_is_primitive<int> { const static bool is = true; };
  template <> struct olx_is_primitive<long> { const static bool is = true; };
  template <> struct olx_is_primitive<long long> { const static bool is = true; };

  template <> struct olx_is_primitive<unsigned char> { const static bool is = true; };
  template <> struct olx_is_primitive<unsigned short> { const static bool is = true; };
  template <> struct olx_is_primitive<unsigned int> { const static bool is = true; };
  template <> struct olx_is_primitive<unsigned long> { const static bool is = true; };
  template <> struct olx_is_primitive<unsigned long long> { const static bool is = true; };

  template <> struct olx_is_primitive<float> { const static bool is = true; };
  template <> struct olx_is_primitive<double> { const static bool is = true; };
  template <> struct olx_is_primitive<long double> { const static bool is = true; };


} // end of namespace esdl
