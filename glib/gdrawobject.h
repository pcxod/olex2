#ifndef gdrawobjectH
#define gdrawobjectH
#include "glbase.h"
#include "glrender.h"
#include "evector.h"
#include "emath.h"

#include "macroerror.h"

BeginGlNamespace()

const short  sgdoVisible   = 0x0001, // TGDrawObject flags
             sgdoSelected  = 0x0002,
             sgdoGroupable = 0x0004,
             sgdoGroup     = 0x0008,
             sgdoGrouped   = 0x0010,
             sgdoDeleted   = 0x0020;

/*
  defines basic functionality of a graphic object, accessible outside of
 the graphic core
*/
//---------------------------------------------------------------------------
struct ACreationParams {
  evecd params;
  TArrayList<TGlMaterial> materials;
  virtual ~ACreationParams() {}
};
class AGDrawObject: public ACollectionItem  {
protected:
  short FDrawStyle;
  short Flags;
  class TGlGroup *FParentGroup;  // parent collection
//  friend class TGlRender;
  class TGPCollection *FPrimitives;
  class TGlRender *FParent;   // initialised owhen the collection is assigned
  evecd FParams;
  olxstr CollectionName;
  inline void SetCollectionName(const olxstr& nn)  {  CollectionName = nn;  }
public:
  AGDrawObject(const olxstr& collectionName);
  // create object within the specified collection, using provided parameters
  virtual void Create(const olxstr& newCollectionName=EmptyString, const ACreationParams* cpar = NULL)  {  }
  // this should return object created with new in order to recreate the objecs as it was
  virtual ACreationParams* GetCreationParams() const {  return NULL;  }
  virtual ~AGDrawObject();

  void  Primitives( TGPCollection *GPC);
  inline TGPCollection* Primitives()  const {  return FPrimitives;  }

  inline const olxstr& GetCollectionName()  const  {  return CollectionName;  }

  inline evecd& Params()           {  return FParams;  }

  inline TGlRender *Parent()  const  {  return FParent;}
  inline void Parent(TGlRender *P)   {  FParent = P;}
  virtual bool Orient(class TGlPrimitive *P) = 0;
//  inline virtual void OrientAfterDraw(TGlPrimitive *P){  return; };
  virtual bool GetDimensions(vec3d &Max, vec3d &Min)=0;
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

  inline bool Visible()   const {  return (Flags & sgdoVisible) == sgdoVisible; }
  inline virtual void Visible(bool On) {  SetBit(On, Flags, sgdoVisible); }

  inline bool Selected()  const {  return (Flags & sgdoSelected) == sgdoSelected;  }
  inline virtual void Selected(bool On){  SetBit(On, Flags, sgdoSelected); }

  inline bool Groupable() const {  return (Flags & sgdoGroupable) == sgdoGroupable; }
  void Groupable(bool On){  SetBit(On, Flags, sgdoGroupable); }

  inline bool Group() const     {  return (Flags & sgdoGroup) == sgdoGroup; }

  inline bool Grouped() const   {  return (Flags & sgdoGrouped) == sgdoGrouped; }
  virtual void Grouped(bool On){ SetBit(On, Flags, sgdoGrouped); }

  inline bool Deleted()  const  {return (Flags & sgdoDeleted) == sgdoDeleted; }
  virtual void Deleted(bool On){ SetBit(On, Flags, sgdoDeleted); }

  short MaskFlags(short mask) const {  return (Flags&mask);  }

  virtual inline TGlGroup *ParentGroup() const {  return FParentGroup; }
  virtual void ParentGroup(TGlGroup *P);
  
  virtual void ListDrawingStyles(TStrList &List){  return; }

  virtual void UpdaterimitiveParams(TGlPrimitive *GlP){  return; }
  // the object should update its parameters from GlP

  virtual short DrawStyle() const {  return 0; }
  virtual void Compile(); // is used to compile new created primitives without rebuilding
  // entire model; use it when some object is added to existing scene

  virtual void ListParams(TStrList& List, TGlPrimitive *Primitive){  return; }
  // for parameters of a specific primitive
  virtual void ListParams(TStrList &List){  return; }
  // for internal object parameters
  virtual void ListPrimitives(TStrList &List) const {  return; }
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
