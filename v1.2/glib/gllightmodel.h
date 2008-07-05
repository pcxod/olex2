//---------------------------------------------------------------------------

#ifndef gllightmodelH
#define gllightmodelH
#include "glbase.h"
#include "gllight.h"
#include "gloption.h"
#include "emath.h"

BeginGlNamespace()

const char cglmLocalViewer  = 0x01,     // light model properties
           cglmSmoothShade  = 0x02,
           cglmTwoSide      = 0x04,
           cglBack          = 0x01,
           cglFront         = 0x02;

class TGlLightModel: public IEObject
{
private:
  char FFlags;
  TGlOption FClearColor, FAmbient;
  TGlLight FLights[8];
public:

  TGlLightModel();

  void Init();

  TGlLight& Light(int i)    {  return FLights[i]; }
  TGlOption& ClearColor()   {  return FClearColor; };
  TGlOption& AmbientColor() {  return FAmbient; };
  inline int Flags() const  {  return FFlags; }
  inline bool LocalViewer() const  {  return (FFlags & cglmLocalViewer) ==cglmLocalViewer;  }
  void LocalViewer(bool v)  {  SetBit(v, FFlags, cglmLocalViewer);}

  inline bool SmoothShade() const  {  return (FFlags & cglmSmoothShade) == cglmSmoothShade; }
  void SmoothShade(bool v)  { SetBit(v, FFlags, cglmSmoothShade);}

  inline bool TwoSide() const      {  return  (FFlags & cglmTwoSide) == cglmTwoSide;  }
  void TwoSide(bool v)      {  SetBit(v, FFlags, cglmTwoSide); }

  TGlLightModel& operator = (TGlLightModel &M);

  void ToDataItem(TDataItem *Item);
  bool FromDataItem(TDataItem *Item);
};


EndGlNamespace()
#endif
