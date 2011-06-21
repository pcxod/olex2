#ifndef __olx_cl_base_H
#define __olx_cl_base_H

#define BeginChemNamespace()  namespace chemlib {
#define EndChemNamespace()  };\
  using namespace chemlib;
#define UseChemNamespace()  using namespace chemlib;
#define GlobalChemFunction( fun )     chemlib::fun
#define ChemObject( obj )     chemlib::obj

#endif

 
