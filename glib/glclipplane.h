//---------------------------------------------------------------------------

#ifndef glclipplaneH
#define glclipplaneH

#include "glbase.h"
#include "evector.h"
#include "glrender.h"

BeginGlNamespace()

class TGlClipPlane  {
  evecd FEq;
  bool FEnabled;
  class TGlClipPlanes *FParent;
  int FId;  // GL_CLIPPLANE_i
public:
  TGlClipPlane( int Id, TGlClipPlanes *FParent, float A, float B, float C, float D );
  ~TGlClipPlane();
  inline bool Enabled() const     {  return FEnabled; }
  void Enabled(bool v);
  inline evecd& Equation()        {  return FEq; }
  inline TGlClipPlanes *Parent()  {  return FParent; }
  inline int Id() const           {  return FId; }
};

class TGlClipPlanes  {
  TPtrList<TGlClipPlane> FPlanes;
  class TGlRenderer *FParent;
public:
  TGlClipPlanes(TGlRenderer *R);
  ~TGlClipPlanes();
  inline TGlRenderer *Parent()  {  return FParent;  }
  inline TGlClipPlane *Plane(size_t i)  {  return FPlanes[i];  }
  inline size_t PlaneCount() const {  return FPlanes.Count();  }
  void Enable(bool v);
};


EndGlNamespace()
#endif

