#ifndef __olx_gxl_xbond_H
#define __olx_gxl_xbond_H
#include "glrender.h"
#include "gdrawobject.h"
#include "glprimitive.h"
#include "gllabel.h"
#include "styles.h"
#include "sbond.h"
#include "talist.h"

BeginGxlNamespace()

class TXBondStylesClear: public AActionHandler  {
public:
  TXBondStylesClear(TGlRenderer *Render)  {  Render->OnStylesClear.Add(this);  }
  virtual ~TXBondStylesClear();
  bool Enter(const IEObject *Sender, const IEObject *Data=NULL);
  bool Exit(const IEObject *Sender, const IEObject *Data=NULL);
};

class TXBond: public AGDrawObject  {
private:
  TSBond *FBond;
  TXGlLabel* Label;
  short FDrawStyle;
  size_t XAppId;
  friend class TXBondStylesClear;
protected:
  void GetDefSphereMaterial(TGlMaterial &M);
  void GetDefRimMaterial(TGlMaterial &M);
  //TEStringList* FindPrimitiveParams(TGlPrimitive *P);
  static TArrayList<TGlPrimitiveParams> FPrimitiveParams;
  static TStrPObjList<olxstr,TGlPrimitive*> FStaticObjects;
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

  DefPropP(size_t, XAppId)
  TXGlLabel& GetLabel() const {  return *Label;  }
  void UpdateLabel()  {  GetLabel().Update();  }
  // creates legend up three levels (0 to 2)
  static olxstr GetLegend(const TSBond& B, const short level);

  // beware - for objects, having no wrapped bond this might fail
  struct BondAccessor  {
    static inline TSBond& Access(TXBond& b)  {  return b.Bond();  }
    static inline TSBond& Access(TXBond* b)  {  return b->Bond();  }
  };
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

  bool OnMouseDown(const IEObject *Sender, const TMouseData& Data);
  bool OnMouseUp(const IEObject *Sender, const TMouseData& Data);
  bool OnMouseMove(const IEObject *Sender, const TMouseData& Data);

  void SetDeleted(bool v)  {
    AGDrawObject::SetDeleted(v);
    Label->SetDeleted(v);
    if( FBond != NULL )
      FBond->SetDeleted(v);
  }
  void SetVisible(bool v)  {
    AGDrawObject::SetVisible(v);
    if( !v )
      Label->SetVisible(false);
  }
  void ListDrawingStyles(TStrList &List);

  uint32_t GetPrimitiveMask() const;
  static void DefMask(int V);
  static int  DefMask();
  inline short DrawStyle() const {  return FDrawStyle; }

  void UpdatePrimitiveParams(TGlPrimitive *Primitive);
  void OnPrimitivesCleared();
  void Quality(const short Val);
  // should be called when atom coordinates have changed
  virtual void Update();

  static void Init(TGlRenderer* glr)  {
    if( FXBondStylesClear == NULL ) 
      FXBondStylesClear = new TXBondStylesClear(glr);
  }

  static TGraphicsStyle* GetParamStyle() {  return FBondParams;  }
  void CreateStaticObjects();
  static void ClearStaticObjects()  {
    FStaticObjects.Clear();
  }
};

struct BondCreationParams : public ACreationParams {
  class TXAtom &a1, &a2;
  BondCreationParams(TXAtom& xa1, TXAtom& xa2) : a1(xa1), a2(xa2) { }
};

EndGxlNamespace()
#endif
