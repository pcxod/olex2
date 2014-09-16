/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "evaluable.h"
#include "context.h"
using namespace esdl::exparse;

const IEvaluable::operator_dict::Entry ANumberEvaluator::cast_operators_table[] = {
  IEvaluable::operator_dict::Entry(&typeid(bool), &ANumberEvaluator::bool_cast),
  IEvaluable::operator_dict::Entry(&typeid(olxstr), &ANumberEvaluator::str_cast),
  ANumberEvaluator::create_operator_entry<char>(),
  ANumberEvaluator::create_operator_entry<unsigned char>(),
  ANumberEvaluator::create_operator_entry<short>(),
  ANumberEvaluator::create_operator_entry<unsigned short>(),
  ANumberEvaluator::create_operator_entry<int>(),
  ANumberEvaluator::create_operator_entry<unsigned int>(),
  ANumberEvaluator::create_operator_entry<long int>(),
  ANumberEvaluator::create_operator_entry<unsigned long int>(),
  ANumberEvaluator::create_operator_entry<long long int>(),
  ANumberEvaluator::create_operator_entry<unsigned long long int>(),
  ANumberEvaluator::create_operator_entry<float>(),
  ANumberEvaluator::create_operator_entry<double>()
};

olxdict<std::type_info const*, IEvaluable::cast_operator, TPointerComparator> 
ANumberEvaluator::cast_operators( 
  ANumberEvaluator::cast_operators_table, 
  sizeof(ANumberEvaluator::cast_operators_table)/sizeof(ANumberEvaluator::cast_operators_table[0]) 
);

IEvaluable *IEvaluable::create_proxy_() const {
  return new VarProxy(const_cast<IEvaluable*>(this));
}

IEvaluable* creator<IEvaluable &>::create(
  const EvaluableFactory &f, IEvaluable &v)
{
  return f.create_ref(v);
}

