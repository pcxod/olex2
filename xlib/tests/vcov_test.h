#include "vcov.h"

namespace test {

void vcov_test(OlxTests& t)  {
  t.description = __OlxSrcInfo;
  TXApp& app = TXApp::GetInstance();
  if( app.XFile().GetLattice().GetObjects().atoms.IsEmpty() )
    throw TFunctionFailedException(__OlxSourceInfo, "could not complete test: at least 1 atom is needed");
  TAsymmUnit& au = app.XFile().GetAsymmUnit();
  VcoVContainer vcovc(au);
  try  {  app.InitVcoV(vcovc);  }
  catch(const TExceptionBase&)  {}
  TSAtom& src =  app.XFile().GetLattice().GetObjects().atoms[0];
  TSAtom a(NULL), b(NULL);
  a.CAtom(src.CAtom());
  a.AddMatrix(const_cast<smatd*>(&src.GetMatrix(0)));
  b.CAtom(src.CAtom());
  b.AddMatrix(const_cast<smatd*>(&src.GetMatrix(0)));
  // test cell axis esds
  for( size_t i=0; i < 3; i++ )  {
    vec3d tr;
    tr[i] = 1;
    a.crd() = au.Orthogonalise(src.ccrd()+tr);
    TEValueD e = vcovc.CalcDistance(src, a);
    if( olx_abs(e.GetV()-au.GetAxes()[i]) > 1e-6 || olx_abs(e.GetE()-au.GetAxisEsds()[i]) > 1e-6)
      throw TFunctionFailedException(__OlxSourceInfo, EmptyString());
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
      if( olx_abs(e.GetV()-au.GetAngles()[ai]) > 1e-4 || olx_abs(e.GetE()-au.GetAngleEsds()[ai]) > 1e-4)
        throw TFunctionFailedException(__OlxSourceInfo, EmptyString());
    }
  }
  return;
}
//..................................................................................
};  //end namespace test
