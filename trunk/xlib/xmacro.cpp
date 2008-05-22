#ifdef __BORLANC__
  #pragma hdrstop
#endif

#include "xmacro.h"
#include "xapp.h"

#define xlib_InitMacro(macroName, validOptions, argc, desc)\
  lib.RegisterStaticMacro( new TStaticMacro(&XLibMacros::mac##macroName, #macroName, (validOptions), argc, desc))

void XLibMacros::Export(TLibrary& lib)  {
  xlib_InitMacro(BrushHkl, "f-consider Friedel law", fpAny, "for high redundancy\
 data sets, removes equivalents with high sigma");

  xlib_InitMacro(SG, "a", fpNone|fpOne, "suggest space group");

  xlib_InitMacro(GraphSR, "b-number of bins", fpNone|fpOne|psFileLoaded,
"Prints a scale vs resolution graph for current file (fcf file must exist in current folder)");

  xlib_InitMacro(Wilson, "b-number of bins", fpNone|fpOne|psFileLoaded, "Prints Wilson plot data");
  xlib_InitMacro(TestSymm, "e-tolerance limit", fpNone|psFileLoaded, "Tests current \
  structure for missing symmetry");
}


#ifdef __BORLANC__
  #pragma package(smart_init)
#endif
