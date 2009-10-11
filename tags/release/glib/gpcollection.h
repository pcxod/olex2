//---------------------------------------------------------------------------
#ifndef gpcollectionH
#define gpcollectionH
#include "glbase.h"
#include "elist.h"
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
  TGPCollection(TGlRenderer& parent, const olxstr& name=EmptyString) : 
      Parent(parent), Name(name), Style(NULL) {}
  virtual ~TGPCollection() {}
  // clears the content of the colelction and removes primitives
  // use it to change the content of the collection
  void ClearPrimitives();

  inline int PrimitiveCount() const {  return Primitives.Count();  }
  inline TGlPrimitive& GetPrimitive(int index) const {  return *Primitives[index];  }
  TGlPrimitive* FindPrimitiveByName(const olxstr& Name) const;

  TGlPrimitive& NewPrimitive(const olxstr& Name, short type);
  void AddPrimitive(TGlPrimitive& P)  {  Primitives.Add(P);  }

  void RemovePrimitive(int index) {  Primitives.Delete(index); }
  void RemovePrimitive(TGlPrimitive& GlP)  {  Primitives.Remove(GlP);  }

  bool ContainsPrimitive(TGlPrimitive& GlP);

  inline int ObjectCount() const          {  return GObjects.Count();  }
  class AGDrawObject& GetObject(int index) const {  return *GObjects[index];  }
  void AddObject(AGDrawObject& Obj);

  void ClearObjects()                  {  GObjects.Clear();  }
  void RemoveObject(AGDrawObject& G)   {  GObjects.Remove(G); }
  void DeleteObject(int i)             {  GObjects.Delete(i); }

  void Draw();

  void ListParams(TStrList& List, TGlPrimitive* Primitive);

  inline TGlRenderer& GetParent()  const {  return Parent; }
  DefPropC(olxstr, Name)

  virtual void SetStyle(TGraphicsStyle* S);
  // it might be NULL
  inline TGraphicsStyle& GetStyle() const {  return *Style; }
};


EndGlNamespace()
#endif

