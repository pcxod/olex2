//----------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//
#ifndef gxbaseH
#define gxbaseH
//---------------------------------------------------------------------------

#define BeginGxlNamespace()  namespace gxlib {
#define EndGxlNamespace()  };\
  using namespace gxlib;
#define UseGxlNamespace()  using namespace gxlib;
#define GlobalGxlFunction( fun )     gxlib::fun
#define GxlObject( obj )     gxlib::obj

BeginGxlNamespace()

const short qaHigh    = 1,  // drawing quality
            qaMedium  = 2,
            qaLow     = 3,
            qaPict    = 4;

const short ddsDef       = 0, // default drawing style for primitives
            ddsDefAtomA  = 1,
            ddsDefAtomB  = 2,
            ddsDefRim    = 3,
            ddsDefSphere = 4;
            
EndGxlNamespace()
#endif

 
