#ifndef xbondH
#define xbondH
#include "gxbase.h"

#include "glrender.h"
#include "gdrawobject.h"
#include "glprimitive.h"
#include "styles.h"

#include "sbond.h"
#include "talist.h"


BeginGxlNamespace()

class TXBondStylesClear: public AActionHandler  {
public:
  TXBondStylesClear(TGlRenderer *Render)  {  Render->OnStylesClear->Add(this);  }
  virtual ~TXBondStylesClear();
  bool Enter(const IEObject *Sender, const IEObject *Data=NULL);
  bool Exit(const IEObject *Sender, const IEObject *Data=NULL);
};

class TXBond: public AGDrawObject  {
private:
  TSBond *FBond;
  short FDrawStyle;
  friend class TXBondStylesClear;
protected:
  void GetDefSphereMaterial(TGlMaterial &M);
  void GetDefRimMaterial(TGlMaterial &M);
  //TEStringList* FindPrimitiveParams(TGlPrimitive *P);
  static TArrayList<TGlPrimitiveParams> FPrimitiveParams;
  static void ValidateBondParams();
  static TGraphicsStyle *FBondParams;
  static TXBondStylesClear *FXBondStylesClear;
  virtual bool IsMaskSaveable() const {  return false;  }
  virtual bool IsStyleSaveable() const {  return false; }
  virtual bool IsRadiusSaveable() const {  return false; }
public:
  TXBond(TGlRenderer& Render, const olxstr& collectionName, TSBond& B);
  void Create(const olxstr& cName = EmptyString, const ACreationParams* cpar = NULL);
  virtual ACreationParams* GetACreationParams() const;
  virtual ~TXBond();

  static TStrPObjList<olxstr,TGlPrimitive*> FStaticObjects;
  void CreateStaticPrimitives();

  // creates legend up three levels (0 to 2)
  static olxstr GetLegend(const TSBond& B, const short level);

  // beware - for objects, having no wrapped bond this might fail
  inline TSBond& Bond() const {  return *FBond; }
  
  void SetRadius(float V);
  inline double GetRadius() const {  return FParams[4]; }

  bool Orient(TGlPrimitive& P);
  bool GetDimensions(vec3d &Max, vec3d &Min){  return false; };

  void ListParams(TStrList &List, TGlPrimitive *Primitive);
  // for parameters of a specific primitive
  void ListParams(TStrList &List);
  // for internal object parameters
  void ListPrimitives(TStrList &List) const;
  // fills the list with proposal primitives to construct object
  TGraphicsStyle* Style();

  bool OnMouseDown(const IEObject *Sender, const TMouseData *Data);
  bool OnMouseUp(const IEObject *Sender, const TMouseData *Data);
  bool OnMouseMove(const IEObject *Sender, const TMouseData *Data);

  inline bool IsDeleted() const {  return AGDrawObject::IsDeleted(); }
  void SetDeleted(bool v)  {  AGDrawObject::SetDeleted(v);  FBond->SetDeleted(v); }
  void ListDrawingStyles(TStrList &List);

  uint32_t GetPrimitiveMask() const;
  static void DefMask(int V);
  static int  DefMask();
  inline short DrawStyle() const {  return FDrawStyle; }

  void UpdatePrimitiveParams(TGlPrimitive *Primitive);
  void OnPrimitivesCleared();
  void Quality(const short Val);
  void BondUpdated();

  static void Init(TGlRenderer* glr)  {
    if( FXBondStylesClear == NULL ) 
      FXBondStylesClear = new TXBondStylesClear(glr);
  }

  static TGraphicsStyle* GetParamStyle() {  return FBondParams;  }
};

struct BondCreationParams : public ACreationParams {
  class TXAtom &a1, &a2;
  BondCreationParams(TXAtom& xa1, TXAtom& xa2) : a1(xa1), a2(xa2) { }
};

EndGxlNamespace()
#endif
