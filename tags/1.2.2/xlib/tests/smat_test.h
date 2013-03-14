/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "../symmat.h"

namespace test {

void symmat_Test(OlxTests& t)  {
  t.description = __OlxSrcInfo;
  const uint32_t id_1 = smatd::GenerateId(0, -56, -43, -21);
  const uint32_t id_2 = smatd::GenerateId(0, vec3i(-56, -43, -21));
  if( id_1 != id_2 ) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "ID generation mismatch");
  }
  if( smatd::GetContainerId(id_1) != 0 )
    throw TFunctionFailedException(__OlxSourceInfo, "invalid ID");
  if( smatd::GetT(id_1)[0] != -56 ||
      smatd::GetT(id_1)[1] != -43 ||
      smatd::GetT(id_1)[2] != -21 )
  {
    throw TFunctionFailedException(__OlxSourceInfo, "invalid T");
  }
  if( smatd::GetT(id_1)[0] != smatd::GetTx(id_1) ||
      smatd::GetT(id_1)[1] != smatd::GetTy(id_1) ||
      smatd::GetT(id_1)[2] != smatd::GetTz(id_1) )
  {
    throw TFunctionFailedException(__OlxSourceInfo, "invalid T");
  }
}
//...................................................................................................
void SymmMatTests(OlxTests& t)  {
  t.Add(test::symmat_Test);
}
};  //namespace test
