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

int main(int argc, char* argv[])  {
  XModel xm;
  double defs [] = {0.02, 0.1, 0.01, 0.04, 1};
  xm.CHIV.Add( *(new Restraint_Chiv(xm, defs, 0)) );
  xm.Scatterers.Add("C1", new XScatterer("C1", NULL) );
  xm.LinearEquations.Add( xm.Scatterers[0].Occupancy, -1.5);
  return 0;
}

