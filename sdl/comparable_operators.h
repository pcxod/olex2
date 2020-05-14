/******************************************************************************
* Copyright (c) 2004-2020 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_comparable_operators
#define __olx_sdl_comparable_operators
#include "ebase.h"
BeginEsdlNamespace()

/* operator extensions for classes having Compare
*/
template<class heir_t>
class comparable_operators_base {
protected:
  heir_t& self() { return *static_cast<heir_t*>(this); }
  const heir_t& self() const {
    return *static_cast<const heir_t*>(this);
  }
public:
  bool operator > (const heir_t& v) const { return self().Compare(v) > 0; }
  bool operator >= (const heir_t& v) const { return self().Compare(v) >= 0; }
  bool operator < (const heir_t& v) const { return self().Compare(v) < 0; }
  bool operator <= (const heir_t& v) const { return self().Compare(v) <= 0; }
};

/* operator extensions for classes having Compare and ==
*/
template<class heir_t>
class comparable_operators :
  public comparable_operators_base<heir_t>
{
  typedef comparable_operators_base<heir_t> op_parent_t;
public:
  bool operator != (const heir_t& v) const {
    return !(op_parent_t::self().Compare(v) == 0);
  }
};

/* operator extensions for classes having Compare and no ==
*/
template<class heir_t>
class comparable_operators_full :
  public comparable_operators_base<heir_t>
{
  typedef comparable_operators_base<heir_t> op_parent_t;
public:
  bool operator == (const heir_t& v) const {
    return op_parent_t::self().Compare(v) == 0;
  }
  bool operator != (const heir_t& v) const {
    return op_parent_t::self().Compare(v) != 0;
  }
};
EndEsdlNamespace()
#endif
