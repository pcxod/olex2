//----------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//
#ifndef chembaseH
#define chembaseH
//---------------------------------------------------------------------------

#define BeginChemNamespace()  namespace chemlib {
#define EndChemNamespace()  };\
  using namespace chemlib;
#define UseChemNamespace()  using namespace chemlib;
#define GlobalChemFunction( fun )     chemlib::fun
#define ChemObject( obj )     chemlib::obj

#endif

 
