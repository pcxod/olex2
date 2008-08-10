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


using namespace std;

void ParseShelxIns(TStrList& ins)  {
  ins.CombineLines('=');
  for( int i=0; i < ins.Count(); i++ )  {
    
  }
}

int main(int argc, char* argv[])  {
  XModel xm;
  TAtomsInfo ai;
  double defs [] = {0.02, 0.1, 0.01, 0.04, 1};
  XScattererData& sc_h = xm.NewScattererData("H");
  XScattererData& sc_c = xm.NewScattererData("C");
  xm.CHIV.Add( *(new Restraint_Chiv(xm, defs, 0)) );
  xm.NewScatterer(0.0, 0.0, 0.0).AddScatterer("C1", &ai.GetAtomInfo(iCarbonIndex), &sc_c, 1);
  xm.NewScatterer(0.5, 0.5, 0.5).AddScatterer("H1", &ai.GetAtomInfo(iHydrogenIndex), &sc_h, 1);
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

