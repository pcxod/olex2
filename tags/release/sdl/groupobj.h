//---------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//
#ifndef groupobjH
#define groupobjH
#include "ebase.h"
#include "tptrlist.h"
#undef GetObject
#undef AddObject

BeginEsdlNamespace()

class AGroupObject;
class TObjectGroup;
class AGOProperties;

class AGOProperties  {
  size_t ObjectGroupId;
  void SetObjectGroupId(size_t val) {  ObjectGroupId = val;  }
protected:
  TPtrList<AGroupObject> Objects;
  size_t GetObjectGroupId() const {  return ObjectGroupId;  }
public:
  AGOProperties() : ObjectGroupId(InvalidIndex) {  }// Objects.SetIncrement(512);  }
  virtual ~AGOProperties() {}
  // adds an object (reference or a pointer) to the group and returns it
  template <class AGO> 
  inline AGroupObject& AddObject(AGO& O)  {  return *Objects.Add(O); }
  inline AGroupObject& GetObject(size_t index) const {  return *Objects[index]; }
  inline size_t ObjectCount() const { return Objects.Count(); }
  // removes an object reference or a pointer
  template <class AGO> void RemoveObject(const AGO& GO)  {
    size_t index = Objects.IndexOf(GO);
    if( index != InvalidIndex )  
      Objects.Delete(index);
  }
  
  void SetObjectCapacity(size_t cap)  {  Objects.SetCapacity(cap);  }
  void SetObjectIncrement(size_t inc) {  Objects.SetIncrement(inc);  }

  virtual bool operator == (const AGOProperties& C) const = 0;
  virtual AGOProperties& operator = (const AGOProperties& C) = 0;

  friend class TObjectGroup; // to modify the ObjectGroupId
};

class AGroupObject: public ACollectionItem  {
protected:
  TObjectGroup& Parent;
  AGOProperties* Properties;
  friend class TObjectGroup;
  // the function is used to create a new instance of the properties
  virtual AGOProperties* NewProperties() const = 0;
public:
  AGroupObject(TObjectGroup& parent) : Parent(parent), Properties(NULL) { }
  virtual ~AGroupObject()  { }
  // this should be used carefull - any change will affect all objects of the group!
  inline AGOProperties& GetProperties()   {  return *Properties;  }
  inline const AGOProperties& GetProperties() const {  return *Properties;  }
  /* a copy of C is created and returned if the property does not exists
   otherwise a pointer to existing property is returned
  */
  virtual AGOProperties& SetProperties(const AGOProperties& C);
};

class TObjectGroup: public IEObject  {
protected:
  AGOProperties* FindProps(const AGOProperties& C);
protected:
  TPtrList<AGroupObject> Objects;
  TPtrList<AGOProperties> Props;
public:
  TObjectGroup();
  virtual ~TObjectGroup() { }
  void Clear();
  template <class AGO>
  AGroupObject& AddObject(AGO const& O)           {  return *Objects.Add(O);  }
  inline AGroupObject& GetObject(size_t index) const {  return *Objects[index]; }
  inline size_t ObjectCount() const {  return Objects.Count(); }
  template <class AGO>  size_t IndexOfObject(const AGO& G)  {  return Objects.IndexOf(G); }
  // used to remove objects form the collection; if an object->Tag ==Tag, it is removed
  void RemoveObjectsByTag(int Tag);
  //void ReplaceObjects(TEList *CurObj, TEList *NewObj );

  AGOProperties& GetProperties(size_t index) const {  return *Props[index]; }

  inline size_t PropertiesCount() const {  return Props.Count(); }
  template <class AGP>
  size_t IndexOfProperties(const AGP& P) const {  return Props.IndexOf(P); }
  // creates new properties if required, alwasy returns a valid pointer
  AGOProperties* NewProps(AGroupObject& Sender, // the sender
    AGOProperties *OldProps, // senders old properties
    const AGOProperties& NewProps);  //th eproperties to set
};

// helper class to avoid casts (well, too many of them)
template <class PC, class OC>  // property class, object class
class ObjectGroup : public TObjectGroup {
public:
  ObjectGroup() { }
  virtual ~ObjectGroup() { }
  template <class AGO> OC& AddObject(AGO const& O)  {  return *(OC*)Objects.Add(O);  }
  inline OC& GetObject(size_t index) const {  return *(OC*)Objects[index]; }
  inline PC& GetProperties(size_t index) const {  return *(PC*)Props[index]; }
};
EndEsdlNamespace()
#endif
