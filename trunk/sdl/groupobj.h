//---------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//

#ifndef groupobjH
#define groupobjH
#include "ebase.h"
#include "tptrlist.h"
//---------------------------------------------------------------------------

BeginEsdlNamespace()

class AGroupObject;
class TObjectGroup;
class AGOProperties;

class AGOProperties  {
  int Id;
protected:
  TPtrList<AGroupObject> Objects;
public:
  AGOProperties();
  virtual ~AGOProperties();

  DefPropP(int, Id)
  // do not change the value of Id from the outside !!!

  inline void AddObject(AGroupObject *O)  {  Objects.Add(O); }
  inline AGroupObject* Object(size_t index) const {  return Objects[index]; }
  inline int ObjectCount() const { return Objects.Count(); }
  void RemoveObject(const AGroupObject *GO)  {
    int index = Objects.IndexOf(GO);
    if( index != -1 )  Objects.Delete(index);
  };
  virtual bool operator == (const AGOProperties &C) const = 0;
  virtual AGOProperties& operator = (const AGOProperties& C) = 0;
};

class AGroupObject: public ACollectionItem  {
protected:
  TObjectGroup* FParent;
  AGOProperties* FProperties;

  friend class TObjectGroup;
  // the function is used to create a new instance of the properties
  virtual AGOProperties * NewProperties() = 0;
public:
  AGroupObject(TObjectGroup *Group);
  virtual ~AGroupObject()  {  }
  inline const AGOProperties * GetProperties() const {  return FProperties;  }
  /* a copy of C is created and returned if the property does not exists
   therwise a pointer to existing proprty is returned
  */
  virtual AGOProperties * SetProperties(const AGOProperties* C);
};

class TObjectGroup: public IEObject  {
protected:
  AGOProperties *GetProps(const AGOProperties& C);
protected:
  TPtrList<AGroupObject> Objects;
  TPtrList<AGOProperties> Props;
public:
  TObjectGroup();
  virtual ~TObjectGroup();
  void Clear();

  void AddObject(AGroupObject* O)                 {  Objects.Add(O);  }
  inline AGroupObject* Object(size_t index) const {  return Objects[index]; }
  inline int ObjectCount() const                  {  return Objects.Count(); }
  int IndexOf(AGroupObject* G)                    {  return Objects.IndexOf(G); }
  // used to remove objects form the collection; if an object->Tag ==Tag, it is removed
  void RemoveObjectsByTag(int Tag);
  //void ReplaceObjects(TEList *CurObj, TEList *NewObj );

  AGOProperties * Properties(int index) const {  return Props[index]; }

  inline int PropCount()              const {  return Props.Count(); }
  int IndexOf(const AGOProperties *P) const {  return Props.IndexOf(P); }
  AGOProperties * NewProps(AGroupObject *Sender, AGOProperties *OldProps, const AGOProperties *NewProps);
};

EndEsdlNamespace()
#endif
