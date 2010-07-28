#ifndef __olx_gl_gdrawobject_H
#define __olx_gl_gdrawobject_H
#include "glbase.h"
#include "evector.h"
#include "emath.h"
#include "library.h"
#include "macroerror.h"
#include "gpcollection.h"
#include "glmaterial.h"

BeginGlNamespace()

const short
  sgdoVisible    = 0x0001, // TGDrawObject flags
  sgdoSelected   = 0x0002,
  sgdoGroupable  = 0x0004,
  sgdoGroup      = 0x0008,
  sgdoGrouped    = 0x0010,
  sgdoDeleted    = 0x0020,
  sgdoSelectable = 0x0040,
  sgdoCreated    = 0x0080;

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
  class TGlGroup *ParentGroup;  // parent collection
  class TGlRenderer& Parent;
  TGPCollection *Primitives;
  evecd FParams;
  olxstr CollectionName;
  inline void SetCollectionName(const olxstr& nn)  {  CollectionName = nn;  }
public:
  AGDrawObject(TGlRenderer& parent, const olxstr& collectionName);
  // create object within the specified collection, using provided parameters
  virtual void Create(const olxstr& newCollectionName=EmptyString, const ACreationParams* cpar = NULL)  {}
  // this should return object created with new in order to recreate the objecs as it was
  virtual ACreationParams* GetCreationParams() const {  return NULL;  }
  virtual ~AGDrawObject()  {}

  virtual const olxstr& GetPrimitiveMaskName() const {
    static const olxstr mn("PMask");
    return mn;
  }

  void  SetPrimitives(TGPCollection& GPC)  {  Primitives = &GPC;  }
  inline TGPCollection& GetPrimitives() const {  return *Primitives;  }

  inline const olxstr& GetCollectionName() const {  return CollectionName;  }

  inline evecd& Params()  {  return FParams;  }

  inline TGlRenderer& GetParent() const {  return Parent;  }
  virtual bool Orient(class TGlPrimitive& P) = 0;
//  inline virtual void OrientAfterDraw(TGlPrimitive *P){  return; };
  virtual bool GetDimensions(vec3d& Max, vec3d& Min) = 0;
  // mouse handlers, any object receives mouse down/up events; write appropriate
  //handlers to handle mouse; if the object returns true OnMouseDown, it receives
  //OnMouseMove as well; Objects must not change values of the Data!
  virtual bool OnMouseDown(const IEObject *Sender, const struct TMouseData& Data)  {  return false;  }
  virtual bool OnMouseUp(const IEObject *Sender, const struct TMouseData& Data)  {  return false;  }
  virtual bool OnMouseMove(const IEObject *Sender, const struct TMouseData& Data)  {  return false;  }
  virtual bool OnDblClick(const IEObject *Sender, const struct TMouseData& Data)  {  return false;  }
  virtual bool OnZoom(const IEObject *Sender, const struct TMouseData& Data)  {  return false;  }

  // need a virtual setters for these
  virtual void SetVisible(bool v)  {  olx_set_bit(v, Flags, sgdoVisible);  }
  inline bool IsVisible() const {  return ((Flags&sgdoVisible) != 0);  }
  virtual void SetSelected(bool v)  {  olx_set_bit(v, Flags, sgdoSelected);  }
  inline bool IsSelected() const {  return ((Flags&sgdoSelected) != 0);  }

  DefPropBFIsSet(Groupable, Flags, sgdoGroupable)
  DefPropBFIsSet(Grouped, Flags, sgdoGrouped)
  void SetDeleted(bool v)  {
    olx_set_bit(v, Flags, sgdoDeleted);
    if( v )
      olx_set_bit(false, Flags, sgdoVisible);
  }
  inline bool IsDeleted() const {  return ((Flags&sgdoDeleted) != 0);  }
  DefPropBFIsSet(Selectable, Flags, sgdoSelectable)
  // for internal use, may not reflect the real state of the object
  DefPropBFIsSet(Created, Flags, sgdoCreated)

  inline bool IsGroup() const {  return (Flags & sgdoGroup) == sgdoGroup; }

  short MaskFlags(short mask) const {  return (Flags&mask);  }

  virtual inline TGlGroup* GetParentGroup() const {  return ParentGroup;  }
  virtual void SetParentGroup(TGlGroup* P)  {
    SetGrouped((ParentGroup = P) != NULL);
    if( P == NULL )
      SetSelected(false);
  }
  
  virtual void ListDrawingStyles(TStrList& List)  {}
  virtual void UpdaterimitiveParams(TGlPrimitive* GlP)  {}
  /* a generic Update function, called when the model (like control points) has changed
  should be implemented by objects depending on coordinates of others in an indirect way
  */
  virtual void Update()  {}
  // should be implemented to update labels when font is changed 
  virtual void UpdateLabel()  {}

  virtual short DrawStyle() const {  return 0;  }
  /* is used to compile new created primitives without rebuilding entire model;
  use it when some object is added to existing scene */
  virtual void Compile(); 

  // for parameters of a specific primitive
  virtual void ListParams(TStrList& List, TGlPrimitive* Primitive)  {}
  // for internal object parameters
  virtual void ListParams(TStrList& List)  {}
  // fills the list with primitives from which the object can be constructed
  virtual void ListPrimitives(TStrList& List) const {}
  virtual void UpdatePrimitives(int32_t Mask, const ACreationParams* cpar=NULL);
  virtual void OnPrimitivesCleared()  {}

  void LibVisible(const TStrObjList& Params, TMacroError& E);
  void LibIsGrouped(const TStrObjList& Params, TMacroError& E);
  void LibIsSelected(const TStrObjList& Params, TMacroError& E);
  void LibGetName(const TStrObjList& Params, TMacroError& E);
  void ExportLibrary(TLibrary& lib);

  virtual void Individualize() {}
  virtual void Collectivize()  {}

  struct FlagsAnalyser  {
    const short ref_flags;
    FlagsAnalyser(short _ref_flags) : ref_flags(_ref_flags)  {}
    inline bool OnItem(const AGDrawObject& o) const {  return o.MaskFlags(ref_flags) != 0;  }
  };
  template <class Actor> struct FlagsAnalyserEx  {
    const short ref_flags;
    const Actor actor;
    FlagsAnalyserEx(short _ref_flags, const Actor& _actor) : ref_flags(_ref_flags), actor(_actor)  {}
    inline bool OnItem(AGDrawObject& o) const {
      if( o.MaskFlags(ref_flags) != 0 )
        return actor.OnItem(o);
      return false;
    }
  };
  struct PrimitivesAccessor  {
    static TGPCollection& Access(const AGDrawObject& o)  {  return o.GetPrimitives();  }
  };
};

typedef TPtrList<AGDrawObject> AGDObjList;

EndGlNamespace()
#endif
