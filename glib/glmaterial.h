//---------------------------------------------------------------------------

#ifndef glmaterialH
#define glmaterialH

#include "glbase.h"
#include "groupobj.h"
#include "gloption.h"
#include "datafile.h"
#include "emath.h"

BeginGlNamespace()

extern TGlOption BlackColor;

const short    sglmAmbientF   = 0x0001,    // material properties
               sglmAmbientB   = 0x0002,
               sglmAmbientFB  = 0x0003,
               
               sglmDiffuseF   = 0x0004,
               sglmDiffuseB   = 0x0008,
               sglmDiffuseFB  = 0x000C,

               sglmSpecularF  = 0x0010,
               sglmSpecularB  = 0x0020,
               sglmSpecularFB = 0x0030,

               sglmShininessF = 0x0040,
               sglmShininessB = 0x0080,
               sglmShininessFB= 0x00C0,
               
               sglmEmissionF =  0x0100,
               sglmEmissionB =  0x0200,
               sglmEmissionFB = 0x0300,
               
               sglmTransparent =0x0400,
               sglmIdentityDraw=0x0800,
//               sglmStaticDraw  =0x1000,

               sglmLighting   =0x2000,
               sglmColorMat   =0x4000;

class TGlMaterial: public AGOProperties, public IEObject  {
  short Flags;
  bool FMark;
public:
  TGlOption EmissionF;
  TGlOption SpecularF;
  short ShininessF;

  TGlOption EmissionB;
  TGlOption SpecularB;
  short ShininessB;

  TGlOption AmbientF;
  TGlOption DiffuseF;

  TGlOption AmbientB;
  TGlOption DiffuseB;
  
  TGlMaterial();
  void Init() const;
  const TGlMaterial& Intensity(TGlOption& ClearColor, double intensity) const;
  
  inline void  SetFlags(short v){  Flags = v; };
  inline short GetFlags() const {  return Flags; };
  inline bool Mark() const      {  return FMark; }
  inline void Mark(bool v)      {  FMark = v; }

  inline void SetAmbientF(bool On) {  SetBit(On, Flags, sglmAmbientF); }
  inline void SetDiffuseF(bool On) {  SetBit(On, Flags, sglmDiffuseF); }
  inline void SetAmbientB(bool On) {  SetBit(On, Flags, sglmAmbientB); }
  inline void SetDiffuseB(bool On) {  SetBit(On, Flags, sglmDiffuseB); }

  inline void SetEmissionF(bool On) {  SetBit(On, Flags, sglmEmissionF); }
  inline void SetSpecularF(bool On) {  SetBit(On, Flags, sglmSpecularF); }
  inline void SetShininessF(bool On){  SetBit(On, Flags, sglmShininessF); }
  inline void SetEmissionB(bool On) {  SetBit(On, Flags, sglmEmissionB); }
  inline void SetSpecularB(bool On) {  SetBit(On, Flags, sglmSpecularB); }
  inline void SetShininessB(bool On){  SetBit(On, Flags, sglmShininessB); }

  inline void SetLighting(bool On){  SetBit(On, Flags, sglmLighting); }
  inline void SetColorMaterial(bool On){  SetBit(On, Flags, sglmColorMat); }


  inline bool GetAmbientF() const {  return (Flags & sglmAmbientF) == sglmAmbientF; }
  inline bool GetDiffuseF() const {  return (Flags & sglmDiffuseF) == sglmDiffuseF;  }
  inline bool GetAmbientB() const {  return (Flags & sglmAmbientB) == sglmAmbientB;  }
  inline bool GetDiffuseB() const {  return (Flags & sglmDiffuseB) == sglmDiffuseB;  }

  inline bool GetEmissionF() const {  return (Flags & sglmEmissionF) == sglmEmissionF;  }
  inline bool GetSpecularF() const {  return (Flags & sglmSpecularF) ==  sglmSpecularF;  }
  inline bool GetShininessF()const {  return (Flags & sglmShininessF) ==  sglmShininessF;  }
  inline bool GetEmissionB() const {  return (Flags & sglmEmissionB) == sglmEmissionB;  }
  inline bool GetSpecularB() const {  return (Flags & sglmSpecularB) == sglmSpecularB;  }
  inline bool GetShininessB()const {  return (Flags & sglmShininessB) == sglmShininessB;  }

  inline bool GetLighting() const {  return (Flags & sglmLighting)  == sglmLighting;  }
  inline bool GetColorMaterial() const {  return (Flags & sglmColorMat) == sglmColorMat;  }

  inline void SetTransparent(bool On) {  SetBit(On, Flags, sglmTransparent);  }
  inline bool GetTransparent() const  {  return (Flags & sglmTransparent) == sglmTransparent;  }

  inline void SetIdentityDraw(bool On) {  SetBit(On, Flags, sglmIdentityDraw);  }
  inline bool GetIdentityDraw() const  {  return (Flags & sglmIdentityDraw) == sglmIdentityDraw;  }

  TGlMaterial& operator = (const TGlMaterial &G);
  AGOProperties& operator = (const AGOProperties &G);
  bool operator == (const TGlMaterial &G) const;
  bool operator == (const AGOProperties &G) const; // !!!

  void ToDataItem(TDataItem *Item);
  bool FromDataItem(TDataItem *Item);

  TIString ToString() const;
  void FromString( const olxstr& str );
};

EndGlNamespace()

#endif
