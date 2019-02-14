/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

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

const uint16_t
  sgdoHidden     = 0x0100,
  sgdoSelected   = 0x0200,
  sgdoGroupable  = 0x0400,
  sgdoGroup      = 0x0800,
  sgdoGrouped    = 0x1000,
  sgdoSelectable = 0x2000,
  sgdoCreated    = 0x4000,
  sgdoPrintable  = 0x8000;

/*  defines basic functionality of a graphic object */
class AGDrawObject
  : public virtual ACollectionItem, public APerishable
{
  uint16_t sgdo_Flags;
protected:
  class TGlGroup *ParentGroup;  // parent collection
  class TGlRenderer& Parent;
  TGPCollection *Primitives;
  evecd FParams;
  olxstr CollectionName;
  void SetCollectionName(const olxstr& nn) { CollectionName = nn; }
  void SetTypeGroup() { sgdo_Flags |= sgdoGroup; }
  virtual bool SelfDraw(bool /*SelectPrimitives*/,
    bool /*SelectObjects*/)
  {
    return false;
  }
public:
  AGDrawObject(TGlRenderer& parent, const olxstr& collectionName);
  // create object within the specified collection, using provided parameters
  virtual void Create(const olxstr& newCollectionName = EmptyString()) {}
  // this should return object created with new in order to recreate the objecs as it was
  virtual ~AGDrawObject() {}

  virtual const olxstr& GetPrimitiveMaskName() const {
    static const olxstr mn("PMask");
    return mn;
  }

  void SetPrimitives(TGPCollection& GPC) { Primitives = &GPC; }
  TGPCollection& GetPrimitives() const { return *Primitives; }
  bool HasPrimitives() const { return Primitives != 0; }

  const olxstr& GetCollectionName() const { return CollectionName; }

  evecd& Params() { return FParams; }
  const evecd& Params() const { return FParams; }

  TGlRenderer& GetParent() const {  return Parent;  }
  virtual bool Orient(class TGlPrimitive& P) = 0;
//  virtual void OrientAfterDraw(TGlPrimitive *P){  return; };
  virtual bool GetDimensions(vec3d& Max, vec3d& Min) = 0;
  // mouse handlers, any object receives mouse down/up events; write appropriate
  //handlers to handle mouse; if the object returns true OnMouseDown, it receives
  //OnMouseMove as well; Objects must not change values of the Data!
  virtual bool OnMouseDown(const IOlxObject *, const struct TMouseData&) {
    return false;
  }
  virtual bool OnMouseUp(const IOlxObject *, const struct TMouseData&) {
    return false;
  }
  virtual bool OnMouseMove(const IOlxObject *, const struct TMouseData&) {
    return false;
  }
  virtual bool OnDblClick(const IOlxObject *, const struct TMouseData&) {
    return false;
  }
  virtual bool OnZoom(const IOlxObject *, const struct TMouseData&) {
    return false;
  }

  // need a virtual setters for these
  virtual void SetVisible(bool v) { olx_set_bit(!v, sgdo_Flags, sgdoHidden); }
  bool IsVisible() const { return ((sgdo_Flags&sgdoHidden) == 0); }
  virtual void SetSelected(bool v) { olx_set_bit(v, sgdo_Flags, sgdoSelected); }
  bool IsSelected() const { return ((sgdo_Flags&sgdoSelected) != 0); }
  /* returns true if the object is to end up in the picture output */
  virtual void SetPrintable(bool v) { olx_set_bit(v, sgdo_Flags, sgdoPrintable); }
  bool IsPrintable() const { return ((sgdo_Flags&sgdoPrintable) != 0); }

  virtual vec3d CalcCenter() const { return vec3d(0); }
  // by default projects the CalcCenter() and returns Z
  virtual double CalcZ() const;
  // cannot be const because some objects get update in the process
  virtual void DoDraw(bool SelectPrimitives, bool SelectObjects);

  DefPropBFIsSet(Groupable, sgdo_Flags, sgdoGroupable)
  DefPropBFIsSet(Grouped, sgdo_Flags, sgdoGrouped)
  DefPropBFIsSet(Selectable, sgdo_Flags, sgdoSelectable)
  // for internal use, may not reflect the real state of the object
  DefPropBFIsSet(Created, sgdo_Flags, sgdoCreated)

  bool IsGroup() const { return (sgdo_Flags&sgdoGroup) == sgdoGroup; }

  short MaskFlags(short mask) const { return (sgdo_Flags&mask); }

  virtual TGlGroup* GetParentGroup() const { return ParentGroup; }
  virtual void SetParentGroup(TGlGroup* P) {
    SetGrouped((ParentGroup = P) != 0);
    if (P == 0) {
      SetSelected(false);
    }
  }

  virtual void ListDrawingStyles(TStrList&) {}
  virtual void UpdaterimitiveParams(TGlPrimitive*) {}
  /* a generic Update function, called when the model (like control points) has changed
  should be implemented by objects depending on coordinates of others in an indirect way
  */
  virtual void Update() {}
  // should be implemented to update labels when font is changed
  virtual void UpdateLabel() {}

  /* is used to compile new created primitives without rebuilding entire model;
  use it when some object is added to existing scene */
  virtual void Compile();

  // for parameters of a specific primitive
  virtual void ListParams(TStrList&, TGlPrimitive*) {}
  // for internal object parameters
  virtual void ListParams(TStrList&) {}
  // fills the list with primitives from which the object can be constructed
  virtual void ListPrimitives(TStrList&) const {}
  virtual void UpdatePrimitives(int32_t Mask);
  virtual void OnPrimitivesCleared() {}
  virtual void OnStyleChange() {}

  void LibVisible(const TStrObjList& Params, TMacroData& E);
  void LibIsGrouped(const TStrObjList& Params, TMacroData& E);
  void LibIsSelected(const TStrObjList& Params, TMacroData& E);
  void LibGetName(const TStrObjList& Params, TMacroData& E);
  void ExportLibrary(TLibrary& lib);

  virtual void Individualize() {}
  virtual void Collectivize() {}

  struct FlagsAnalyser {
    const short ref_flags;
    FlagsAnalyser(short _ref_flags) : ref_flags(_ref_flags) {}
    template <typename item_t>
    bool OnItem(const item_t& o, size_t) const {
      return olx_ref::get(o).MaskFlags(ref_flags) != 0;
    }
  };
  template <class Actor> struct FlagsAnalyserEx {
    const short ref_flags;
    const Actor actor;
    FlagsAnalyserEx(short _ref_flags, const Actor& _actor)
      : ref_flags(_ref_flags), actor(_actor)
    {}
    template <typename item_t>
    bool OnItem(item_t& o_, size_t i) const {
      AGDrawObject &o = olx_ref::get(o_);
      if (o.MaskFlags(ref_flags) != 0)
        return actor.OnItem(o, i);
      return false;
    }
  };
};

typedef TPtrList<AGDrawObject> AGDObjList;

EndGlNamespace()
#endif
