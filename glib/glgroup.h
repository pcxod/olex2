#ifndef __olx_gl_group_H
#define __olx_gl_group_H
#include "glbase.h"
#include "gdrawobject.h"
#include "tptrlist.h"

BeginGlNamespace()

class TGlGroup: public AGDrawObject {
  TPtrList<AGDrawObject> FObjects;   // a list of grouped objects
  class TGlMaterial GlM;
  bool DefaultColor;
protected:
  void InitMaterial() const;
public:
  TGlGroup(TGlRenderer& R, const olxstr& collectionName);
  virtual void Create(const olxstr& cName = EmptyString, const ACreationParams* cpar = NULL);
  virtual ~TGlGroup();
  void Clear();
  void Draw(bool SelectPrimitives, bool SelectObjects) const;
  void Draw() const {  Draw(false, false); }
  // Adds an object to the list if it is not there and removes it otherwise
  // returns true if the object is added and false if it is removed
  bool Add(AGDrawObject& G);
  void Remove(AGDrawObject& G);
  void RemoveDeleted();

  inline bool Contains(AGDrawObject& G) const {  return  (FObjects.IndexOf(G) == InvalidIndex) ? false : true;  }
  inline size_t Count() const {  return FObjects.Count(); }
  bool IsEmpty() const {  return FObjects.IsEmpty();  }
  inline AGDrawObject& GetObject(size_t i) const {  return *FObjects[i]; }
  inline AGDrawObject& operator [] (size_t i) const {  return *FObjects[i]; }

  bool OnMouseDown(const IEObject *Sender, const class TMouseData *Data);
  bool OnMouseUp(const IEObject *Sender, const TMouseData *Data);
  bool OnMouseMove(const IEObject *Sender, const TMouseData *Data);

  inline bool Orient(TGlPrimitive& P){  return false; };
  inline bool GetDimensions(vec3d& Max, vec3d& Min){  return false;  }

  void SetVisible(bool On);
  void SetSelected(bool On);

  bool IsDefaultColor() const {  return DefaultColor;  }

  const TGlMaterial& GetGlM() {  return GlM; }
  void SetGlM(const TGlMaterial& m);
};


EndGlNamespace()
#endif
