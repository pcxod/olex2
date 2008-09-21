// sym_bark.cpp : Defines the entry point for the console application.
//

#include "exception.h"
#include "efile.h"
#include "estrlist.h"
#include "xapp.h"
#include "log.h"

#include "ins.h"
#include "asymmunit.h"
#include "catom.h"

#include "symmlib.h"
#include "library.h"
#include "outstream.h"
#include "estlist.h"
#include "fastsymm.h"
#include "etime.h"

#include <iostream>
#include "simple_math.h"

#include "restraints.h"
#include "scat_it.h"
#include "chemdata.h"
#include "outstream.h"


using namespace std;


class ISF_calc {
public:
  virtual vec3d Calc(const TArrayList<vec3d>& positions) = 0;
};

template <class sg> class SF_calc : public ISF_calc {
public:
  virtual vec3d Calc(const TArrayList<vec3d>& positions) {
    vec3d pos;
    TArrayList<vec3d> rv(sg::size);
    for( int i=0; i < positions.Count(); i++ )  {
      sg::GenPos(positions[i], rv);
      for( int j=0; j < sg::size; j++ ) // this is important, not rv.count()!
        pos += rv[j];
    }
    return pos;
  }
};

class NT_calc  {
public:
  static vec3d Calc(const smatd_list& symop, const TArrayList<vec3d>& positions) {
    vec3d pos;
    for( int i=0; i < symop.Count(); i++ )  {
      for( int j=0; j < positions.Count(); j++ )  {
        pos += symop[i]*positions[j];
      }
    }
    return pos;
  }
};

DefineFSFactory(ISF_calc,SF_calc)


void ParseShelxIns(TStrList& ins)  {
  ins.CombineLines('=');
  for( int i=0; i < ins.Count(); i++ )  {
    
  }
}

int main(int argc, char* argv[])  {
  TSymmLib sl;
  TBasicApp bapp(argv[0]);
  bapp.GetLog().AddStream( new TOutStream, true );
  //TSpaceGroup* sg = sl.FindGroup("P21/c");
  //ISF_calc* sf_calc = fs_factory_ISF_calc("P21/c");
  //if( sf_calc != NULL && sg != NULL )  {  // acording to the test it gives 4 times speed up
  //  TArrayList<vec3d> positions(10000000);
  //  positions[0] = vec3d(0.5, -0.5, 0.5);  
  //  positions[1] = vec3d(-0.5, 0.5, -0.5);  
  //  smatd_list ml;
  //  sg->GetMatrices(ml, mattAll);
  //  time_t n = TETime::msNow();
  //  vec3d p1 = sf_calc->Calc(positions);
  //  TBasicApp::GetLog() << "FastSymm: " << TETime::msNow() - n << '\n';
  //  n = TETime::msNow();
  //  vec3d p2 = NT_calc::Calc(ml, positions);
  //  TBasicApp::GetLog() << "Conventional: " << TETime::msNow() - n << '\n';
  //  TBasicApp::GetLog() << p1.ToString() << '\n';
  //  TBasicApp::GetLog() << p2.ToString() << '\n';
  //}
  XModel xm;
  TAtomsInfo ai;
  double defs [] = {0.02, 0.1, 0.01, 0.04, 1};
  XScattererData& sc_h = xm.NewScattererData("H");
  XScattererData& sc_c = xm.NewScattererData("C");
  xm.CHIV.Add( *(new Restraint_Chiv(xm, defs, 0)) );
  xm.NewScatterer("C1", 0.0, 0.0, 0.0).AddScatterer(&ai.GetAtomInfo(iCarbonIndex), &sc_c, 1);
  xm.NewScatterer("H1", 0.5, 0.5, 0.5).AddScatterer(&ai.GetAtomInfo(iHydrogenIndex), &sc_h, 1);
  xm.Scatterers[0].Occupancy = 1;
  XVar& var = xm.Variables.AddNew(0.5);
  xm.Scatterers[1].Occupancy.options.SetVar( &var );
  xm.Scatterers[1].Occupancy.options.SetRefinable(false);
  xm.Scatterers[1].TDP.SetUani(NULL, &xm.Scatterers[0], 1.2);
  double uiso = xm.Scatterers[1].TDP.GetUiso();
  XLinearEquation& eq = xm.LinearEquations.AddNew(0, 0);
  // occu(H1) = 1.5 occu(C1)
//  eq.Add(1, *xm.Scatterers[1].Occupancy);
//  eq.Add(-1.5, *xm.Scatterers[0].Occupancy);
  return 0;
}

