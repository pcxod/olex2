//---------------------------------------------------------------------------

#ifndef glgroupH
#define glgroupH
#include "glbase.h"
#include "gdrawobject.h"

BeginGlNamespace()

class TGlGroup: public AGDrawObject {
  class TEList FObjects;   // a list of grouped objects
  class TGlMaterial *FGlM;
  bool DefaultColor;
protected:
  void InitMaterial() const;
public:
  TGlGroup(const olxstr& collectionName, TGlRender *Parent);
  virtual void Create(const olxstr& cName = EmptyString);
  virtual ~TGlGroup();
  void Clear();
  void Draw(bool SelectPrimitives, bool SelectObjects) const;
  void Draw() const {  Draw(false, false); }
  bool Add(AGDrawObject *G);
  // Adds an object to the list if it is not there and removes it otherwise
  // returns true if the object is added and false if it is removed
  void Remove(AGDrawObject *G);
  void RemoveDeleted();

  bool Contains(AGDrawObject *G);
  inline int Count()                 const {  return FObjects.Count(); }
  inline AGDrawObject *Object(int i) const {  return (AGDrawObject*)FObjects[i]; }

  bool OnMouseDown(const IEObject *Sender, const class TMouseData *Data);
  bool OnMouseUp(const IEObject *Sender, const TMouseData *Data);
  bool OnMouseMove(const IEObject *Sender, const TMouseData *Data);

  inline bool Orient(TGlPrimitive *P){  return false; };
  inline bool GetDimensions(TVPointD &Max, TVPointD &Min){  return false;  }

  inline bool Visible()  const {  return AGDrawObject::Visible();  }
  inline bool Selected() const {  return AGDrawObject::Selected();  }
  void Visible(bool On);
  void Selected(bool On);

  inline bool IsDefaultColor() const {  return DefaultColor;  }
  inline TGlMaterial* GlM()    const {  return FGlM; }
  void GlM(const TGlMaterial &G);
};


EndGlNamespace()
#endif
