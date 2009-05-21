//---------------------------------------------------------------------------

#ifndef gllightH
#define gllightH
#include "glbase.h"
#include "datafile.h"
#include "gloption.h"

BeginGlNamespace()

class TGlLight: public IEObject
{
  TGlOption FAmbient;
  TGlOption FDiffuse;
  TGlOption FSpecular;
  TGlOption FPosition;
  TGlOption FSpotDirection;
  short FSpotCutoff;
  short FSpotExponent;
  TGlOption FAttenuation;
  bool FEnabled;
  int FIndex;
public:
  TGlLight();
  virtual ~TGlLight()                 {  return;  }

  inline int Index() const            {  return FIndex;  }
  inline void Index( int v )          {  FIndex = v;  }

  inline bool Enabled() const         {  return FEnabled;  }
  inline void Enabled( bool v )       {  FEnabled = v;  }

  inline short SpotCutoff() const     {  return FSpotCutoff;  }
  inline void SpotCutoff( short v )   {  FSpotCutoff = v;  }

  inline short SpotExponent() const   {  return FSpotExponent;  }
  inline void SpotExponent( short v ) {  FSpotExponent = v;  }

  inline TGlOption& Ambient()         {  return FAmbient;  }
  inline TGlOption& Diffuse()         {  return FDiffuse;  }
  inline TGlOption& Specular()        {  return FSpecular;  }
  inline TGlOption& Position()        {  return FPosition;  }
  inline TGlOption& SpotDirection()   {  return FSpotDirection;  }
  inline TGlOption& Attenuation()     {  return FAttenuation;  }

  inline const TGlOption& GetAmbient()   const      {  return FAmbient;  }
  inline const TGlOption& GetDiffuse()   const      {  return FDiffuse;  }
  inline const TGlOption& GetSpecular()  const      {  return FSpecular;  }
  inline const TGlOption& GetPosition()  const      {  return FPosition;  }
  inline const TGlOption& GetSpotDirection() const  {  return FSpotDirection;  }
  inline const TGlOption& GetAttenuation()    const {  return FAttenuation;  }

  void operator = (const TGlLight &S );
  void ToDataItem(TDataItem& Item) const;
  bool FromDataItem(const TDataItem& Item);
};

EndGlNamespace()
#endif
