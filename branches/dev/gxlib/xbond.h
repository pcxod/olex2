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

class TXAtom;

class TXBondStylesClear: public AActionHandler  {
public:
  TXBondStylesClear(TGlRenderer *Render)  {  Render->OnStylesClear.Add(this);  }
  virtual ~TXBondStylesClear();
  bool Enter(const IEObject *Sender, const IEObject *Data=NULL);
  bool Exit(const IEObject *Sender, const IEObject *Data=NULL);
};

class TXBond: public TSBond, public AGDrawObject  {
private:
  TXGlLabel* Label;
  short FDrawStyle;
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
  TXBond(TNetwork* net, TGlRenderer& Render, const olxstr& collectionName);
  void Create(const olxstr& cName = EmptyString);
  virtual ~TXBond();

  // multiple inheritance...
  void SetTag(index_t v) {   TSBond::SetTag(v);  }
  index_t GetTag() const {  return TSBond::GetTag();  }
  index_t IncTag()  {  return TSBond::IncTag();  }
  index_t DecTag()  {  return TSBond::DecTag();  }

  TXAtom& A() const {  return (TXAtom&)TSBond::A();  }
  TXAtom& B() const {  return (TXAtom&)TSBond::B();  }

  TXGlLabel& GetGlLabel() const {  return *Label;  }
  void UpdateLabel()  {  GetGlLabel().Update();  }
  // creates legend up three levels (0 to 2)
  static olxstr GetLegend(const TSBond& B, const short level);

  void SetRadius(float V);
  inline double GetRadius() const {  return FParams[4]; }

  bool Orient(TGlPrimitive& P);
  bool GetDimensions(vec3d &Max, vec3d &Min)  {  return false; }

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

  void SetVisible(bool v)  {
    AGDrawObject::SetVisible(v);
    if( !v )
      Label->SetVisible(false);
  }
  virtual bool IsDeleted() const {  return TSBond::IsDeleted();  }
  void ListDrawingStyles(TStrList &List);

  uint32_t GetPrimitiveMask() const;
  static void DefMask(int V);
  static int  DefMask();
  inline short DrawStyle() const {  return FDrawStyle; }

  void UpdatePrimitiveParams(TGlPrimitive *Primitive);
  void OnPrimitivesCleared();
  static void Quality(const short Val);
  // should be called when atom coordinates have changed
  virtual void Update();

  static void Init(TGlRenderer* glr)  {
    if( FXBondStylesClear == NULL ) 
      FXBondStylesClear = new TXBondStylesClear(glr);
  }

  static TGraphicsStyle* GetParamStyle()  {  return FBondParams;  }
  static void CreateStaticObjects(TGlRenderer& parent);
  static void ClearStaticObjects()  {
    FStaticObjects.Clear();
  }
};

EndGxlNamespace()
#endif
