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

class TGlLightModel: public IEObject  {
private:
  char Flags;
  TGlOption ClearColor, AmbientColor;
  TGlLight Lights[8];
public:
  TGlLightModel();
  void Init();
  TGlLight& GetLight(int i)    {  return Lights[i]; }
  const TGlLight& GetLight(int i) const {  return Lights[i]; }
  DefPropC(TGlOption, ClearColor)
  DefPropC(TGlOption, AmbientColor)
  inline int GetFlags() const  {  return Flags; }
  DefPropBFIsSet(LocalViewer, Flags, cglmLocalViewer)
  DefPropBFIsSet(SmoothShade, Flags, cglmSmoothShade)
  DefPropBFIsSet(TwoSides, Flags, cglmTwoSide)

  TGlLightModel& operator = (TGlLightModel &M);

  void ToDataItem(TDataItem& Item) const;
  bool FromDataItem(const TDataItem& Item);
};


EndGlNamespace()
#endif
