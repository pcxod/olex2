#ifndef __olx_gl_group_H
#define __olx_gl_group_H
#include "glbase.h"
#include "tptrlist.h"
#include "gdrawobject.h"

BeginGlNamespace()

class TGlGroup: public AGDrawObject {
  TPtrList<AGDrawObject> FObjects;   // a list of grouped objects
  TGlMaterial GlM;
  bool DefaultColor;
protected:
  void InitMaterial() const;
  virtual void DoDraw(bool SelectPrimitives, bool SelectObjects) const;
public:
  TGlGroup(class TGlRenderer& R, const olxstr& collectionName);
  virtual void Create(const olxstr& cName = EmptyString, const ACreationParams* cpar = NULL);
  virtual ~TGlGroup();
  virtual void Clear();
  void Draw(bool SelectPrimitives=false, bool SelectObjects=false) const {
    DoDraw(SelectPrimitives, SelectObjects);
  }
  // Adds an object to the list if it is not there and removes it otherwise
  // returns true if the object is added and false if it is removed
  bool Add(AGDrawObject& G, bool remove=true);
  // a list to ADrawObjects/derived classes is expected
  template <class List> void AddObjects(const List& list)  {
    FObjects.SetCapacity(FObjects.Count()+list.Count());
    for( size_t i=0; i < list.Count(); i++ )
      Add(*list[i], false);
  }
  void Remove(AGDrawObject& G);
  void RemoveDeleted();

  inline bool Contains(const AGDrawObject& G) const {  return  FObjects.IndexOf(G) != InvalidIndex;  }
  inline size_t Count() const {  return FObjects.Count();  }
  bool IsEmpty() const {  return FObjects.IsEmpty();  }
  inline AGDrawObject& GetObject(size_t i) const {  return *FObjects[i];  }
  inline AGDrawObject& operator [] (size_t i) const {  return *FObjects[i];  }
  /* returns true if there are at least two groupable objects, moving the ungroupable
  ones to the provided list */
  bool TryToGroup(TPtrList<AGDrawObject>& ungroupable);

  inline bool Orient(TGlPrimitive& P)  {  return false;  }
  inline bool GetDimensions(vec3d& Max, vec3d& Min)  {  return false;  }

  virtual void SetVisible(bool On);
  virtual void SetSelected(bool On);

  bool IsDefaultColor() const {  return DefaultColor;  }

  const TGlMaterial& GetGlM()  {  return GlM; }
  void SetGlM(const TGlMaterial& m);
};

EndGlNamespace()
#endif
