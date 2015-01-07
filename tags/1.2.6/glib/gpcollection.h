/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gl_gpcollection_H
#define __olx_gl_gpcollection_H
#include "glbase.h"
#include "tptrlist.h"
#include "estrlist.h"
BeginGlNamespace()

class TGlPrimitive;
class AGDrawObject;

class TGPCollection: public ACollectionItem  {
  class TGlRenderer& Parent;
  TPtrList<TGlPrimitive> Primitives; // a list of primitives in the collection
  TPtrList<AGDrawObject> GObjects;   // a list of all objects using this collection
  olxstr Name;
  class TGraphicsStyle* Style;
public:
  TGPCollection(TGlRenderer& parent, const olxstr& name=EmptyString()) :
      Parent(parent), Name(name), Style(NULL) {}
  virtual ~TGPCollection() {}
  // clears the content of the colelction and removes primitives
  // use it to change the content of the collection
  void ClearPrimitives();

  size_t PrimitiveCount() const {  return Primitives.Count();  }
  TGlPrimitive& GetPrimitive(size_t index) const {  return *Primitives[index];  }
  TGlPrimitive* FindPrimitiveByName(const olxstr& Name) const;
  const TPtrList<TGlPrimitive>& GetPrimitives() const {  return Primitives;  }

  TGlPrimitive& NewPrimitive(const olxstr& Name, short type);
  void AddPrimitive(TGlPrimitive& P)  {  Primitives.Add(P);  }

  void RemovePrimitive(size_t index) {  Primitives.Delete(index); }
  void RemovePrimitive(TGlPrimitive& GlP)  {  Primitives.Remove(GlP);  }

  bool ContainsPrimitive(TGlPrimitive& GlP);

  inline size_t ObjectCount() const {  return GObjects.Count();  }
  AGDrawObject& GetObject(size_t index) const {  return *GObjects[index];  }
  const TPtrList<AGDrawObject>& GetObjects() const {  return GObjects;  }
  void AddObject(AGDrawObject& Obj);
  bool IsEmpty() const {  return GObjects.IsEmpty();  }

  void ClearObjects()  {  GObjects.Clear();  }
  void RemoveObject(AGDrawObject& G)  {  GObjects.Remove(G);  }
  void RemoveObjects(const TPtrList<AGDrawObject>& objects)  {
    ACollectionItem::Exclude<>(GObjects, objects);
  }
  void DeleteObject(size_t i)  {  GObjects.Delete(i);  }

  void Draw();

  void ListParams(TStrList& List, TGlPrimitive* Primitive);

  inline TGlRenderer& GetParent() const {  return Parent;  }
  DefPropC(olxstr, Name)

  virtual void SetStyle(TGraphicsStyle* S);
  // it might be NULL
  inline TGraphicsStyle& GetStyle() const {  return *Style;  }
};


EndGlNamespace()
#endif
