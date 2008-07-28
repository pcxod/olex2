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
  TXBondStylesClear(TGlRender *Render)  {  Render->OnStylesClear->Add(this);  }
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
public:
  TXBond(const olxstr& collectionName, TSBond& B, TGlRender *Render);
  void Create(const olxstr& cName = EmptyString);
  virtual ~TXBond();

  static TStrPObjList<olxstr,TGlPrimitive*> FStaticObjects;
  void CreateStaticPrimitives();

  // this function shoul dbe with AtomALevel for B->A() and same for B()
  static olxstr GetLegend(const TSBond& B, const short AtomALevel, const short AtomBLevel);

  inline operator TSBond* () const {  return FBond;  }
  // beware - for objects, having not tdbond underneath this might fail
  inline TSBond& Bond()      const {  return *FBond; }
  void Radius(float V);
  inline double Radius()        {  return Params()[4]; }

  bool Orient(TGlPrimitive *P);
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

  inline bool Deleted()  {  return AGDrawObject::Deleted(); }
  void Deleted(bool v)   {  AGDrawObject::Deleted(v);  FBond->SetDeleted(v); }
  void ListDrawingStyles(TStrList &List);

  void UpdatePrimitives(int32_t Mask);
  static void DefMask(int V);
  static int  DefMask();
  inline short DrawStyle() const {  return FDrawStyle; }

  void UpdatePrimitiveParams(TGlPrimitive *Primitive);
  void OnPrimitivesCleared();
  void Quality(const short Val);
  void BondUpdated();
};


EndGxlNamespace()
#endif
