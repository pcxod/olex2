/******************************************************************************
* Copyright (c) 2004-2026 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/
#include "ebase.h"
BeginEsdlNamespace()
template <size_t N>
struct FixedId {
  uint64_t data[N];

#ifdef __cpp_variadic_templates
  template <typename... Ts>
  FixedId(Ts... vs) {
    static_assert(sizeof...(Ts) == N, "FixedId: argument count must match N");
    uint64_t vals[N] = { (uint64_t)vs... };
    memcpy(data, vals, sizeof(data));
  }
#else
  // fixed-arity fallbacks for pre-C++11 variadic-template support,
  // mirroring complex_id_t's non-variadic branch -- add 2/3/4-arg
  // constructors here if this ever needs to compile without them
#endif

  int Compare(const FixedId<N>& o) const {
    return memcmp(&data[0], &o.data[0], sizeof(data));
  }
};

typedef FixedId<4> BigId; 

EndEsdlNamespace()