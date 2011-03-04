#include "index_range.h"
namespace test {

void IndexRangeTest(OlxTests& t)  {
  t.description = __FUNC__;
  olxstr range = "1-6,9,12-15,20,999-1000,10-20";
  TSizeList rmatch;
  IndexRange::Builder builder;
  for( size_t i=1; i <= 6; i++ )  {
    rmatch << i;
    builder << i;
  }
  rmatch << 9 << 12 << 13 << 14 << 15 << 20 << 999 << 1000;
  builder << 9 << 12 << 13 << 14 << 15 << 20 << 999 << 1000;
  for( size_t i=10; i <= 20; i++ )  {
    rmatch << i;
    builder << i;
  }
  TSizeList testr = IndexRange::FromString(range);
  if( testr.Count() != rmatch.Count() )
    throw TFunctionFailedException(__OlxSourceInfo, "size mismatch");
  for( size_t i=0; i < rmatch.Count(); i++ )
    if( rmatch[i] != testr[i] )
      throw TFunctionFailedException(__OlxSourceInfo, "results mismatch");
  IndexRange::RangeItr itr = IndexRange::GetIterator(range);
  if( rmatch.Count() != itr.CalcSize() )
    throw TFunctionFailedException(__OlxSourceInfo, "CalcSize");
  size_t cnt = 0;
  while( itr.HasNext() )  {
    if( cnt >= rmatch.Count() )
      throw TFunctionFailedException(__OlxSourceInfo, "itr size mismatch");
    const size_t v = rmatch[cnt],
      v1 = itr.Next();
    if( rmatch[cnt++] != v1 )
      throw TFunctionFailedException(__OlxSourceInfo, "itr results mismatch");
  }
  olxstr s = IndexRange::ToString(rmatch);
  if( range != s )
    throw TFunctionFailedException(__OlxSourceInfo, "str results mismatch");
  if( range != builder.GetString() )
    throw TFunctionFailedException(__OlxSourceInfo, "builder str results mismatch");
}
//...................................................................................................
};  //namespace test
