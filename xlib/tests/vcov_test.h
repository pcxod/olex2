/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "vcov.h"
#include "ins.h"

namespace test {

void vcov_test(OlxTests& t)  {
  t.description = __OlxSrcInfo;
  TXApp& app = TXApp::GetInstance();
  // trying to register the extension name throws an exception
  try  {  app.XFile().RegisterFileFormat(new TIns, "res"); }
  catch(const TExceptionBase& )  {}
  TEFile tf(app.GetBaseDir() + "vcov_test.res", "w+b");
  tf.Write(
    "CELL 0.7 12.3 10 4.7 90 123 78\n"
    "ZERR 2 0.01 0.03 0.008 0.1 0.13 0.17\n"
    "LATT 1\n"
    "SFAC C\n"
    "UNIT 1\n"
    "C1 1 0.33 0.25 0.66 1 0.25\n"
    "HKLF 4\n"
    "END"
    );
  tf.SetTemporary(true);
  tf.Flush();
  app.XFile().LoadFromFile(tf.GetName());
  if( app.XFile().GetLattice().GetObjects().atoms.IsEmpty() ) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "could not complete test: at least 1 atom is needed");
  }
  TAsymmUnit& au = app.XFile().GetAsymmUnit();
  VcoVContainer vcovc(au);
  try  {  app.InitVcoV(vcovc);  }
  catch(const TExceptionBase&)  {}
  TSAtom& src =  app.XFile().GetLattice().GetObjects().atoms[0];
  TSAtom a(NULL), b(NULL);
  a.CAtom(src.CAtom());
  a._SetMatrix(const_cast<smatd&>(src.GetMatrix()));
  b.CAtom(src.CAtom());
  b._SetMatrix(const_cast<smatd&>(src.GetMatrix()));
  // test cell axis esds
  for( size_t i=0; i < 3; i++ )  {
    vec3d tr;
    tr[i] = 1;
    a.crd() = au.Orthogonalise(src.ccrd()+tr);
    TEValueD e = vcovc.CalcDistance(src, a);
    if( olx_abs(e.GetV()-au.GetAxes()[i]) > 1e-6 ||
        olx_abs(e.GetE()-au.GetAxisEsds()[i]) > 1e-6)
    {
      throw TFunctionFailedException(__OlxSourceInfo, EmptyString());
    }
  }
  // test cell angle esds
  for( size_t i=0; i < 3; i++ )  {
    vec3d tra;
    tra[i] = 1;
    a.crd() = au.Orthogonalise(src.ccrd()+tra);
    for( size_t j=i+1; j < 3; j++ )  {
      vec3d trb;
      trb[j] = 1;
      b.crd() = au.Orthogonalise(src.ccrd()+trb);
      int ai = 0;
      if( j == 1 )
        ai = 2;
      else  {
        if( i == 0 )
          ai = 1;
        else
          ai = 0;
      }
      TEValueD e = vcovc.CalcAngle(a, src, b);
      if( olx_abs(e.GetV()-au.GetAngles()[ai]) > 1e-4 ||
          olx_abs(e.GetE()-au.GetAngleEsds()[ai]) > 1e-4)
      {
        throw TFunctionFailedException(__OlxSourceInfo, EmptyString());
      }
    }
  }
  return;
}
//..................................................................................
};  //end namespace test
