#ifndef gdrawobjectH
#define gdrawobjectH
#include "glbase.h"
#include "glrender.h"
#include "evector.h"
#include "emath.h"

#include "macroerror.h"
#include "gpcollection.h"

BeginGlNamespace()

const short  sgdoVisible   = 0x0001, // TGDrawObject flags
             sgdoSelected  = 0x0002,
             sgdoGroupable = 0x0004,
             sgdoGroup     = 0x0008,
             sgdoGrouped   = 0x0010,
             sgdoDeleted   = 0x0020;

class TlGroup;
/*
  defines basic functionality of a graphic object, accessible outside of
 the graphic core
*/
//---------------------------------------------------------------------------
struct ACreationParams {
  evecd* params;
  int tag;
  ACreationParams() : tag(-1), params(NULL) {}
  virtual ~ACreationParams() {}
};
class AGDrawObject: public ACollectionItem  {
protected:
  short FDrawStyle;
  short Flags;
  TGlGroup *ParentGroup;  // parent collection
  TGlRenderer& Parent;
  TGPCollection *Primitives;
  evecd FParams;
  olxstr CollectionName;
  inline void SetCollectionName(const olxstr& nn)  {  CollectionName = nn;  }
public:
  AGDrawObject(TGlRenderer& parent, const olxstr& collectionName);
  // create object within the specified collection, using provided parameters
  virtual void Create(const olxstr& newCollectionName=EmptyString, const ACreationParams* cpar = NULL)  {  }
  // this should return object created with new in order to recreate the objecs as it was
  virtual ACreationParams* GetCreationParams() const {  return NULL;  }
  virtual ~AGDrawObject();

  virtual const olxstr& GetPrimitiveMaskName() const {
    static const olxstr mn("PMask");
    return mn;
  }

  void  SetPrimitives(TGPCollection& GPC)  {  Primitives = &GPC;  }
  inline TGPCollection& GetPrimitives()  const {  return *Primitives;  }

  inline const olxstr& GetCollectionName()  const  {  return CollectionName;  }

  inline evecd& Params()           {  return FParams;  }

  inline TGlRenderer& GetParent()  const {  return Parent;  }
  virtual bool Orient(class TGlPrimitive& P) = 0;
//  inline virtual void OrientAfterDraw(TGlPrimitive *P){  return; };
  virtual bool GetDimensions(vec3d& Max, vec3d& Min)=0;
  // mouse handlers, any object receives mouse down/up events; write appropriate
  //handlers to handle mouse; if the object returns true OnMouseDown, it receives
  //OnMouseMove as well; Objects must not change values of the Data!
  virtual bool OnMouseDown(const IEObject *Sender, const class TMouseData *Data){  return false; }
  virtual bool OnMouseUp(const IEObject *Sender, const class TMouseData *Data){  return false; }
  virtual bool OnMouseMove(const IEObject *Sender, const class TMouseData *Data){  return false; }
  virtual bool OnDblClick(const IEObject *Sender, const class TMouseData *Data){  return false; }
  virtual bool OnZoom(const IEObject *Sender, const class TMouseData *Data){  return false; }

  // to be used in groups only
  virtual void Draw() const {  return;  }

  DefPropBFIsSet(Visible, Flags, sgdoVisible)
  DefPropBFIsSet(Selected, Flags, sgdoSelected)
  DefPropBFIsSet(Groupable, Flags, sgdoGroupable)
  DefPropBFIsSet(Grouped, Flags, sgdoGrouped)
  DefPropBFIsSet(Deleted, Flags, sgdoDeleted)

  inline bool IsGroup() const     {  return (Flags & sgdoGroup) == sgdoGroup; }

  short MaskFlags(short mask) const {  return (Flags&mask);  }

  virtual inline TGlGroup* GetParentGroup() const {  return ParentGroup;  }
  virtual void SetParentGroup(TGlGroup* P)  {  SetGrouped((ParentGroup = P) != NULL);  }
  
  virtual void ListDrawingStyles(TStrList& List){  return; }

  virtual void UpdaterimitiveParams(TGlPrimitive* GlP){  return; }
  // the object should update its parameters from GlP

  virtual short DrawStyle() const {  return 0; }
  virtual void Compile(); // is used to compile new created primitives without rebuilding
  // entire model; use it when some object is added to existing scene

  virtual void ListParams(TStrList& List, TGlPrimitive* Primitive){  return; }
  // for parameters of a specific primitive
  virtual void ListParams(TStrList& List){  return; }
  // for internal object parameters
  virtual void ListPrimitives(TStrList& List) const {  return; }
  // fills the list with proposal primitives to construct object
  virtual void UpdatePrimitives(int32_t Mask, const ACreationParams* cpar=NULL);
  virtual void OnPrimitivesCleared();

  void LibVisible(const TStrObjList& Params, TMacroError& E);
  void LibIsGrouped(const TStrObjList& Params, TMacroError& E);
  void LibIsSelected(const TStrObjList& Params, TMacroError& E);
  void LibGetName(const TStrObjList& Params, TMacroError& E);
  void ExportLibrary(class TLibrary& lib);

  virtual void Individualize() {  return; }
  virtual void Collectivize()  {  return; }

};

EndGlNamespace()
#endif
