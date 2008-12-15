//---------------------------------------------------------------------------
#ifndef gpcollectionH
#define gpcollectionH
#include "glbase.h"
#include "elist.h"
#include "glprimitive.h"
#include "tptrlist.h"

BeginGlNamespace()

class TGPCollection: public ACollectionItem  {
  class TGlRender *FParent;
  TPtrList<class TGlPrimitive> Primitives; // a list of primitives in the collection
  TPtrList<AGDrawObject> GObjects;   // a list of all objects using this collection
  olxstr FName;
  class TGraphicsStyle *FStyle;
public:
  TGPCollection(TGlRender *Parent);
  virtual ~TGPCollection();
  // clears the content of the colelction and removes primitives
  // use it to change the content of the collection
  void ClearPrimitives();

  inline int PrimitiveCount() const        {  return Primitives.Count();  }
  inline TGlPrimitive *Primitive(int index) const {  return Primitives[index];  }
  TGlPrimitive* PrimitiveByName(const olxstr &Name);

  TGlPrimitive* NewPrimitive(const olxstr &Name);

  inline const olxstr& PrimitiveName(int index) const {  return Primitives[index]->Name();  }
  void AddPrimitive(TGlPrimitive *P)                    {  Primitives.Add(P);  }

  void RemovePrimitive(int index) {  Primitives.Delete(index); }
  void RemovePrimitive(TGlPrimitive *GlP)  {  Primitives.Remove(GlP);  }

  bool ContainsPrimitive(TGlPrimitive *GlP);

  inline int ObjectCount() const       {  return GObjects.Count();  }
  class AGDrawObject *Object(int index){  return GObjects[index];  }
  void AddObject(AGDrawObject *Obj);

  void ClearObjects()                  {  GObjects.Clear();  }
  void RemoveObject(AGDrawObject *G)   {  GObjects.Remove(G); }
  void DeleteObject(int i)             {  GObjects.Delete(i); }

  void Draw();

  void ListParams(TStrList &List, TGlPrimitive *Primitive);

  inline TGlRender *Parent()              {  return FParent; }
  inline const olxstr& Name() const     {  return FName; }
  inline void Name(const olxstr &name)  {  FName = name;}

  virtual void Style(TGraphicsStyle *S);
  inline TGraphicsStyle* Style() const {  return FStyle; }
};


EndGlNamespace()
#endif

