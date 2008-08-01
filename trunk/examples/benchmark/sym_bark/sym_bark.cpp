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
  xm.CHIV.Add( *(new Restraint_Chiv(xm, defs, 0)) );
  xm.Scatterers.Add( new XScatterer() ).AddScatterer("C1", &ai.GetAtomInfo(iCarbonIndex), 1);
  xm.Scatterers.Add( new XScatterer ).AddScatterer("H1", &ai.GetAtomInfo(iHydrogenIndex), 1 );
  xm.Sites.AddNew( vec3d(0, 0, 0) );
  xm.Sites.AddNew( vec3d(0.5, 0.5, 0.5) );
  xm.Scatterers[0].SetSite( xm.Sites[0] );
  xm.Scatterers[0].AllocateOccupancy().Value = 1;
  xm.Scatterers[1].SetSite( xm.Sites[1] );
  xm.Scatterers[1].Occupancy->Refinable = false;
  XLinearEquation& eq = xm.LinearEquations.AddNew(0, 0);
  // occu(H1) = 1.5 occu(C1)
  eq.Add(1, *xm.Scatterers[1].Occupancy);
  eq.Add(-1.5, *xm.Scatterers[0].Occupancy);
  return 0;
}

