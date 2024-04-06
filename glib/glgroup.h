/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gl_group_H
#define __olx_gl_group_H
#include "glbase.h"
#include "sortedlist.h"
#include "gdrawobject.h"
BeginGlNamespace()

class TMouseData;

class TGlGroup : public AGDrawObject {
  AGDObjList Objects;  // a list of grouped objects
  TGlMaterial GlM;
  TGlOption BlendColor;
  bool DefaultColor, Blended;
protected:
  void InitMaterial() const;
  virtual void DoDraw(bool SelectPrimitives, bool SelectObjects);
  struct ObjectReleaser {
    template <typename item_t> static bool OnItem(item_t &o_, size_t) {
      AGDrawObject &o = olx_ref::get(o_);
      o.SetParentGroup(NULL);
      o.SetGrouped(false);
      return true;
    }
  };
  void OverrideMaterialDraw(bool SelectPrimitives, bool SelectObjects) const;
  void BlendMaterialDraw(bool SelectPrimitives, bool SelectObjects) const;
  TGlOption GetBlendColor() const;
  bool CheckBlended() const;
public:
  TGlGroup(class TGlRenderer& R, const olxstr& collectionName);
  virtual void Create(const olxstr& cName = EmptyString());
  virtual ~TGlGroup();
  virtual void Clear();
  void Draw(bool SelectPrimitives = false, bool SelectObjects = false) {
    DoDraw(SelectPrimitives, SelectObjects);
  }
  void IncCapacity(size_t v) { Objects.SetCapacity(Objects.Count() + v); }
  // Adds an object to the list if it is not there and removes it otherwise
  // returns true if the object is added and false if it is removed
  bool Add(AGDrawObject& G, bool remove = true);
  // a list to ADrawObjects/derived classes is expected
  template <class List> void AddObjects(const List& list) {
    Objects.SetCapacity(Objects.Count() + list.Count());
    for (size_t i = 0; i < list.Count(); i++) {
      Add(*list[i], false);
    }
  }
  template <class obj_t, class list_t> list_t& Extract(list_t& l) const {
    for (size_t i = 0; i < Objects.Count(); i++) {
      obj_t *a = dynamic_cast<obj_t*>(Objects[i]);
      if (a != 0) {
        l.Add(a);
      }
    }
    return l;
  }
  template <class obj_t> ConstPtrList<obj_t> Extract() const {
    TPtrList<obj_t> rv;
    return Extract<obj_t, TPtrList<obj_t> >(rv);
  }
  /* Returns true if th eobejct was removed and false if it was not a part of
  the group
  */
  bool Remove(AGDrawObject& G);
  void RemoveHidden();

  bool Contains(const AGDrawObject& G) const { return  Objects.Contains(G); }
  size_t Count() const { return Objects.Count(); }
  bool IsEmpty() const { return Objects.IsEmpty(); }
  AGDrawObject& GetObject(size_t i) const { return *Objects[i]; }
  AGDrawObject& operator [] (size_t i) const { return *Objects[i]; }
  void SortObjectsByTag();
  /* returns true if there are at least two groupable objects, moving the
  ungroupable ones to the provided list
  */
  bool TryToGroup(AGDObjList& ungroupable);

  bool Orient(TGlPrimitive&) { return false; }
  virtual vec3d CalcCenter() const;
  bool GetDimensions(vec3d&, vec3d&) { return false; }
  virtual bool OnMouseDown(const IOlxObject *Sender, const TMouseData& Data);
  virtual bool OnMouseUp(const IOlxObject *Sender, const TMouseData& Data);
  virtual bool OnMouseMove(const IOlxObject *Sender, const TMouseData& Data);

  virtual void SetVisible(bool On);
  virtual void SetSelected(bool On);

  bool IsDefaultColor() const { return DefaultColor; }
  bool IsBlended() const { return Blended; }
  void SetBlended(bool v);
  const TGlMaterial& GetGlM() const { return GlM; }
  void SetGlM(const TGlMaterial& m);
  // returns the actually rendered material
  virtual TGlMaterial GetActualMaterial(const TGlMaterial &) const;
  typedef AGDrawObject list_item_type;
};

EndGlNamespace()
#endif
