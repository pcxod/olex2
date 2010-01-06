#ifndef glbaseH
#define glbaseH

#define BeginGlNamespace()  namespace glObj {
#define EndGlNamespace()  };\
  using namespace glObj;
#define UseGlNamespace()  using namespace glObj;
#define GlobalGlFunction( fun )     glObj::fun

#include "ebase.h"
#include "gldefs.h"

#if defined __APPLE__ && defined __MACH__
  #include <OpenGL/gl.h>
  #include <OpenGL/glu.h>
#else
  #include <GL/gl.h>
  #include <GL/glu.h>
#endif
#endif
