//---------------------------------------------------------------------------

#ifndef glbaseH
#define glbaseH

#define BeginGlNamespace()  namespace glObj {
#define EndGlNamespace()  };\
  using namespace glObj;
#define UseGlNamespace()  using namespace glObj;
#define GlobalGlFunction( fun )     glObj::fun
// for other platforms the necessary file has to be inluded
// if this file is not inluded, gl.h cannot be processed
#ifdef _WIN32
  #include <windows.h>
#endif
#if defined __APPLE__ && defined __MACH__
  #include <OpenGL/gl.h>
  #include <OpenGL/glu.h>
#else
  #include <GL/gl.h>
  #include <GL/glu.h>
#endif

#include "gldefs.h"
#include "ebase.h"

//BeginGlNamespace()
//EndGlNamespace()
#endif
