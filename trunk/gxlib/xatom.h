#ifndef xatomH
#define xatomH
#include "gxbase.h"

#include "actions.h"

#include "glrender.h"
//#include "gdrawobject.h"
#include "glmouselistener.h"
#include "styles.h"

#include "satom.h"

BeginGxlNamespace()

const short adsSphere    = 0x0001,  // atom draw styles
            adsEllipsoid = 0x0002,
            adsEllipsoidNPD = 0x0003;

const short darPers     = 0x0001, // default atom radii
            darIsot     = 0x0002,
            darPack     = 0x0004,
            darBond     = 0x0008;

class TXAtomStylesClear: public AActionHandler  {
public:
  TXAtomStylesClear(TGlRender *Render)  {  Render->OnStylesClear->Add(this);  }
  virtual ~TXAtomStylesClear()  {  ;  }
  bool Enter(const IEObject *Sender, const IEObject *Data=NULL);
  bool Exit(const IEObject *Sender, const IEObject *Data=NULL);
};

class TXAtom: public TGlMouseListener  {
private:
  TSAtom *FAtom;
  short FDrawStyle;
  friend class TXAtomStylesClear;
protected:
  TStrList* FindPrimitiveParams(TGlPrimitive *P);
  static TEList FPrimitiveParams;
  void ValidateRadius(TGraphicsStyle *GS);
  void ValidateDS(TGraphicsStyle *GS);
  static void ValidateAtomParams();
  static TXAtomStylesClear *FXAtomStylesClear;
protected:
  static float FTelpProb, FQPeakScale;
  static TGraphicsStyle *FAtomParams;
public:
  TXAtom(const olxstr& collectionName, TSAtom& A, TGlRender *Render);
  virtual ~TXAtom();
  void Create(const olxstr& cName = CEmptyString);

  static TStrPObjList<olxstr,TGlPrimitive*> FStaticObjects;
  void CreateStaticPrimitives();

  static olxstr GetLegend(const TSAtom& A, const short Level=2); // returns full legend with symm code
  // returns level of the given legend (number of '.', basically)
  static short LegendLevel(const olxstr& legend);
  static olxstr GetLabelLegend(TSAtom *A);
  // returns full legend for the label. e.g. "Q.Q1"

  static void GetDefSphereMaterial(const TSAtom& A, TGlMaterial &M);
  static void GetDefRimMaterial(const TSAtom& A, TGlMaterial &M);

  static void DefRad(short V);
  static void DefDS(short V);
  static void DefSphMask(int V);
  static void DefElpMask(int V);
  static void DefNpdMask(int V);
  static void TelpProb(float V);
  static void DefZoom(float V);

  static short DefRad(); // default radius
  static short DefDS();     // default drawing style
  static int   DefSphMask();  // default mask for spherical atoms
  static int   DefElpMask();  // default mask for elliptical atoms
  static int   DefNpdMask();  // default mas for elliptical atoms eith NPD ellipsoid
  static float TelpProb();    // to use with ellipsoids
  static float DefZoom();    // to use with ellipsoids

  static float QPeakScale();    // to use with q-peaks
  static void QPeakScale(float V);    // to use with q-peaks

  void CalcRad(short DefAtomR);

  inline operator TSAtom* () const {  return FAtom;  }
  
  inline TSAtom& Atom() const  {  return *FAtom;  }
  void ApplyStyle(TGraphicsStyle *S);
  void UpdateStyle(TGraphicsStyle *S);

  void Zoom(float Z);
  inline double Zoom()  {  return Params()[1]; }

  bool Orient(TGlPrimitive *P);
  bool GetDimensions(vec3d &Max, vec3d &Min);

  void ListParams(TStrList &List, TGlPrimitive *Primitive);
  // for parameters of a specific primitive
  void ListParams(TStrList &List);
  void ListPrimitives(TStrList &List) const;
  // fills the list with proposal primitives to construct object
  TGraphicsStyle* Style();
  void UpdatePrimitives(int32_t Mask);

  bool OnMouseDown(const IEObject *Sender, const TMouseData *Data);
  bool OnMouseUp(const IEObject *Sender, const TMouseData *Data);
  bool OnMouseMove(const IEObject *Sender, const TMouseData *Data);

  inline bool Deleted()  const {  return AGDrawObject::Deleted(); }
  void Deleted(bool v)         {  AGDrawObject::Deleted(v);  FAtom->SetDeleted(v); }
  void ListDrawingStyles(TStrList &List);
  inline short DrawStyle() const {  return FDrawStyle; }
  void DrawStyle(short V);

  void UpdatePrimitiveParams(TGlPrimitive *GlP);
  void OnPrimitivesCleared();
  void Quality(const short Val);

  static void Init(TGlRender* glr)  {
    if( FXAtomStylesClear == NULL ) 
      FXAtomStylesClear = new TXAtomStylesClear(glr);
  }
};

EndGxlNamespace()
#endif
