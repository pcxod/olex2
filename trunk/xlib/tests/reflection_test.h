#include "reflection.h"

namespace test  {

void reflection_tests(OlxTests& t)  {
  t.description = __OlxSrcInfo;
  TReflection ref(0, 1, 2, 10, 1, 0);
  if( ref.GetBatch() != 0 )
    throw TFunctionFailedException(__OlxSourceInfo, "batch");
  ref.SetBatch(2);
  if( ref.GetBatch() != 2 )
    throw TFunctionFailedException(__OlxSourceInfo, "batch");
  ref.SetBatch(0);
  if( ref.GetBatch() != 0 )
    throw TFunctionFailedException(__OlxSourceInfo, "batch");

  if( ref.GetMultiplicity() != 1 )
    throw TFunctionFailedException(__OlxSourceInfo, "mult");
  ref.IncMultiplicity();
  if( ref.GetMultiplicity() != 2 )
    throw TFunctionFailedException(__OlxSourceInfo, "mult");
  ref.SetMultiplicity(16);
  if( ref.GetMultiplicity() != 16 )
    throw TFunctionFailedException(__OlxSourceInfo, "mult");
  ref.SetMultiplicity(0);
  if( ref.GetMultiplicity() != 0 )
    throw TFunctionFailedException(__OlxSourceInfo, "mult");
  
  if( ref.IsAbsent() )
    throw TFunctionFailedException(__OlxSourceInfo, "absent");
  ref.SetAbsent(true);
  if( !ref.IsAbsent() )
    throw TFunctionFailedException(__OlxSourceInfo, "absent");

  if( ref.IsCentric() )
    throw TFunctionFailedException(__OlxSourceInfo, "centric");
  ref.SetCentric(true);
  if( !ref.IsCentric() )
    throw TFunctionFailedException(__OlxSourceInfo, "centric");

  if( ref.IsOmitted() )
    throw TFunctionFailedException(__OlxSourceInfo, "omitted");
  ref.SetOmitted(true);
  if( !ref.IsOmitted() )
    throw TFunctionFailedException(__OlxSourceInfo, "omitted");

  if( ref.GetBatch() != 0 )
    throw TFunctionFailedException(__OlxSourceInfo, "batch");
  if( ref.GetMultiplicity() != 0 )
    throw TFunctionFailedException(__OlxSourceInfo, "mult");

  if( olxstr(ref.ToString()) != "   0   1   2   10.00    1.00   0" )
    throw TFunctionFailedException(__OlxSourceInfo, "ToString");
  if( olxstr(ref.ToNString()) != "-.1       0   1   2   10.00    1.00   0" )
    throw TFunctionFailedException(__OlxSourceInfo, "ToString");
  ref.SetOmitted(false);
  if( olxstr(ref.ToNString()) != "+.1       0   1   2   10.00    1.00   0" )
    throw TFunctionFailedException(__OlxSourceInfo, "ToString");
}
};  //namespace test
