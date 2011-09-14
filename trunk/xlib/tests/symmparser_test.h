/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "symmparser.h"

namespace test  {

void symm_parser_tests(OlxTests& t)  {
  t.description = __OlxSrcInfo;
  const mat3d mres1(-1,0,0,-1,0,-1), 
    mres2(-1,1,0,0,-1,1,-1,0,-1),
    mres3(-1,1,-1,1,-1,1,-1,1,-1);
  const vec3d tres1(0,0,0),
    tres2(-1,-0.5,-1./3),
    tres3(-1, 1./6, -1./12),
    tres4(0,0,-1);
  smatd rv;
  TSymmParser::SymmToMatrix("-x,-y,-z", rv);
  if( rv.t.QDistanceTo(tres1) > 1e-10 || rv.r != mres1 )
    throw TFunctionFailedException(__OlxSourceInfo, "S2M:1");
  TSymmParser::SymmToMatrix("-x-1,-y-1/2,-z-1/3", rv);
  if( rv.t.QDistanceTo(tres2) > 1e-10 || rv.r != mres1 )
    throw TFunctionFailedException(__OlxSourceInfo, "S2M:2");
  TSymmParser::SymmToMatrix("-1-x,-1/2-y,-1/3-z", rv);
  if( rv.t.QDistanceTo(tres2) > 1e-10 || rv.r != mres1 )
    throw TFunctionFailedException(__OlxSourceInfo, "S2M:3");
  TSymmParser::SymmToMatrix("-1-x+y,-1/2-y+z,-1/3-z-x", rv);
  if( rv.t.QDistanceTo(tres2) > 1e-10 || rv.r != mres2 )
    throw TFunctionFailedException(__OlxSourceInfo, "S2M:4");
  TSymmParser::SymmToMatrix("-1-x+y-z,1/6+x-y+z,-1/12-x+y-z", rv);
  if( rv.t.QDistanceTo(tres3) > 1e-10 || rv.r != mres3 )
    throw TFunctionFailedException(__OlxSourceInfo, "S2M:5");
  TSymmParser::SymmToMatrix("+X,+Y,-1+Z", rv);
  if( rv.t.QDistanceTo(tres4) > 1e-10 || !rv.r.IsI() )
    throw TFunctionFailedException(__OlxSourceInfo, "S2M:6");
}
};  //namespace test
