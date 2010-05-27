#ifndef xatomH
#define xatomH
#include "gxbase.h"

#include "actions.h"

#include "glrender.h"
#include "glprimitive.h"
#include "glmouselistener.h"
#include "styles.h"

#include "satom.h"
#include "ellipsoid.h"
BeginGxlNamespace()

const short adsSphere       = 1,  // atom draw styles
            adsEllipsoid    = 2,
            adsStandalone   = 3,
            adsOrtep        = 4;

const short darPers     = 0x0001, // default atom radii
            darIsot     = 0x0002,
            darPack     = 0x0004,
            darBond     = 0x0008,
            darIsotH    = 0x0010,  // only affects H, others as darIsot
            darVdW      = 0x0020;
const short polyNone      = 0,
            polyAuto      = 1,  // polyhedron type
            polyRegular   = 2,
            polyPyramid   = 3,
            polyBipyramid = 4;

const int 
  xatom_PolyId        = 1,
  xatom_SphereId      = 2,
  xatom_SmallSphereId = 3,
  xatom_RimsId        = 4,
  xatom_DisksId       = 5,
  xatom_CrossId       = 6;

class TXAtomStylesClear: public AActionHandler  {
public:
  TXAtomStylesClear(TGlRenderer *Render)  {  Render->OnStylesClear->Add(this);  }
  virtual ~TXAtomStylesClear()  {  ;  }
  bool Enter(const IEObject *Sender, const IEObject *Data=NULL);
  bool Exit(const IEObject *Sender, const IEObject *Data=NULL);
};

class TXAtom: public TGlMouseListener  {
private:
  TSAtom *FAtom;
  short FDrawStyle, FRadius;
  size_t XAppId;
  static uint8_t PolyhedronIndex, 
    SphereIndex, 
    SmallSphereIndex, 
    RimsIndex, 
    DisksIndex,
    CrossIndex;
  friend class TXAtomStylesClear;
  struct Poly {
    TArrayList<vec3f> vecs;
    TTypeList<vec3f> norms;
    TTypeList<TVector3<size_t> > faces;
  };
  Poly* Polyhedron;
  void CreatePolyhedron(bool v);
  // returns the center of the created polyhedron
  vec3f TriangulateType2(Poly& p, const TSAtomPList& atoms);
  void CreateNormals(TXAtom::Poly& pl, const vec3f& cnt);
  void CreatePoly(const TSAtomPList& atoms, short type, 
    const vec3d* normal=NULL, const vec3d* center=NULL);
  // returns different names for isotropic and anisotropic atoms
protected:
  TStrList* FindPrimitiveParams(TGlPrimitive *P);
  static TTypeList<TGlPrimitiveParams> FPrimitiveParams;
  void ValidateRadius(TGraphicsStyle& GS);
  void ValidateDS(TGraphicsStyle& GS);
  static void ValidateAtomParams();
  static TXAtomStylesClear *FXAtomStylesClear;
  static int OrtepSpheres;  // 8 glLists
protected:
  static float FTelpProb, FQPeakScale, FQPeakSizeScale;
  static short FDefRad, FDefDS;
  static TGraphicsStyle *FAtomParams;
  static olxstr PolyTypeName;
public:
  TXAtom(TGlRenderer& Render, const olxstr& collectionName, TSAtom& A);
  virtual ~TXAtom();
  void Create(const olxstr& cName = EmptyString, const ACreationParams* cpar = NULL);
  virtual ACreationParams* GetCreationParams() const;

  DefPropP(size_t, XAppId)

  static TStrPObjList<olxstr,TGlPrimitive*> FStaticObjects;
  void CreateStaticPrimitives();

  static olxstr GetLegend(const TSAtom& A, const short Level=2); // returns full legend with symm code
  // returns level of the given legend (number of '.', basically)
  static uint16_t LegendLevel(const olxstr& legend);
  static olxstr GetLabelLegend(TSAtom *A);
  // returns full legend for the label. e.g. "Q.Q1"

  static void GetDefSphereMaterial(const TSAtom& A, TGlMaterial &M);
  static void GetDefRimMaterial(const TSAtom& A, TGlMaterial &M);

  static void DefRad(short V);
  static void DefDS(short V);
  static void DefMask(int V);
  static void TelpProb(float V);
  static void DefZoom(float V);

  static short DefRad(); // default radius
  static short DefDS();     // default drawing style
  static int   DefMask();  // default mas for elliptical atoms eith NPD ellipsoid
  static float TelpProb();    // to use with ellipsoids
  static float DefZoom();    // to use with ellipsoids

  static float GetQPeakScale();    // to use with q-peaks
  static void SetQPeakScale(float V);    // to use with q-peaks
  static float GetQPeakSizeScale();    // to use with q-peaks
  static void SetQPeakSizeScale(float V);    // to use with q-peaks

  void CalcRad(short DefAtomR);

  inline operator TSAtom* () const {  return FAtom;  }
  
  inline TSAtom& Atom() const  {  return *FAtom;  }
  void ApplyStyle(TGraphicsStyle& S);
  void UpdateStyle(TGraphicsStyle& S);

  void SetZoom(double Z);
  inline double GetZoom()  {  return Params()[1]; }
  double GetDrawScale() const {
    double scale = FParams[1];
    if( (FRadius & (darIsot|darIsotH)) != 0 )
      scale *= TelpProb();
    if( (FDrawStyle == adsEllipsoid || FDrawStyle == adsOrtep) && FAtom->GetEllipsoid() != NULL )
    {
      if( FAtom->GetEllipsoid()->IsNPD() )
        return (caDefIso*2*scale);
      return scale;
    }
    else
      return FParams[0]*scale;
  }
  bool Orient(TGlPrimitive& P);
  bool DrawStencil();
  bool GetDimensions(vec3d &Max, vec3d &Min);

  void ListParams(TStrList &List, TGlPrimitive *Primitive);
  // for parameters of a specific primitive
  void ListParams(TStrList &List);
  // fills the list with proposal primitives to construct object
  void ListPrimitives(TStrList &List) const;
  TGraphicsStyle& Style();
  void UpdatePrimitives(int32_t Mask, const ACreationParams* cpar=NULL);
  uint32_t GetPrimitiveMask() const;

  bool OnMouseDown(const IEObject *Sender, const TMouseData *Data);
  bool OnMouseUp(const IEObject *Sender, const TMouseData *Data);
  bool OnMouseMove(const IEObject *Sender, const TMouseData *Data);

  inline bool IsDeleted() const {  return AGDrawObject::IsDeleted(); }
  void SetDeleted(bool v)  {  AGDrawObject::SetDeleted(v);  FAtom->SetDeleted(v); }
  void ListDrawingStyles(TStrList &List);
  inline short DrawStyle() const {  return FDrawStyle; }
  void DrawStyle(short V);

  void UpdatePrimitiveParams(TGlPrimitive *GlP);
  void OnPrimitivesCleared();
  void Quality(const short Val);

  static void Init(TGlRenderer* glr)  {
    if( FXAtomStylesClear == NULL ) 
      FXAtomStylesClear = new TXAtomStylesClear(glr);
  }
  
  void SetPolyhedronType(short type);
  int GetPolyhedronType();

  static TGraphicsStyle* GetParamStyle() {  return FAtomParams;  }
};

struct AtomCreationParams : public ACreationParams {
};

EndGxlNamespace()
#endif
