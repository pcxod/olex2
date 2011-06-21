#include "evaluable.h"

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
olxdict<std::type_info const*, IEvaluable::cast_operator, TPointerPtrComparator> 
  ANumberEvaluator::cast_operators( 
    ANumberEvaluator::cast_operators_table, 
    sizeof(ANumberEvaluator::cast_operators_table)/sizeof(ANumberEvaluator::cast_operators_table[0]) 
  );
