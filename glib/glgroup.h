#ifndef __olx_gl_group_H
#define __olx_gl_group_H
#include "glbase.h"
#include "sortedlist.h"
#include "gdrawobject.h"

BeginGlNamespace()

class TGlGroup: public AGDrawObject {
  /* a sorted list, although would give some performace boost cannot be used here:
    SortedPtrList<AGDrawObject, TPointerPtrComparator> Objects;  // a list of grouped objects
    since the selection order influences a lot of functionality
  */
  AGDObjList Objects;  // a list of grouped objects
  TGlMaterial GlM;
  bool DefaultColor, Blended;
protected:
  void InitMaterial() const;
  virtual void DoDraw(bool SelectPrimitives, bool SelectObjects) const;
  struct ObjectReleaser  {
    static bool OnItem(AGDrawObject& o, size_t)  {
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
  virtual void Create(const olxstr& cName=EmptyString());
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
    Objects.SetCapacity(Objects.Count()+list.Count());
    for( size_t i=0; i < list.Count(); i++ )
      Add(*list[i], false);
  }
  void Remove(AGDrawObject& G);
  void RemoveHidden();

  inline bool Contains(const AGDrawObject& G) const {  return  Objects.IndexOf(&G) != InvalidIndex;  }
  inline size_t Count() const {  return Objects.Count();  }
  bool IsEmpty() const {  return Objects.IsEmpty();  }
  inline AGDrawObject& GetObject(size_t i) const {  return *Objects[i];  }
  inline AGDrawObject& operator [] (size_t i) const {  return *Objects[i];  }
  /* returns true if there are at least two groupable objects, moving the ungroupable
  ones to the provided list */
  bool TryToGroup(AGDObjList& ungroupable);

  inline bool Orient(TGlPrimitive& P)  {  return false;  }
  inline bool GetDimensions(vec3d& Max, vec3d& Min)  {  return false;  }
  virtual bool OnMouseDown(const IEObject *Sender, const struct TMouseData& Data);
  virtual bool OnMouseUp(const IEObject *Sender, const struct TMouseData& Data);
  virtual bool OnMouseMove(const IEObject *Sender, const struct TMouseData& Data);

  virtual void SetVisible(bool On);
  virtual void SetSelected(bool On);

  bool IsDefaultColor() const {  return DefaultColor;  }
  bool IsBlended() const {  return Blended;  }
  void SetBlended(bool v);
  const TGlMaterial& GetGlM()  {  return GlM; }
  void SetGlM(const TGlMaterial& m);
};

EndGlNamespace()
#endif
