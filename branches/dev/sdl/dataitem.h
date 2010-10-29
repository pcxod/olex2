#ifndef __olx_sdl_dataitem_H
#define __olx_sdl_dataitem_H
#include "ebase.h"
#include "typelist.h"
#include "tptrlist.h"
#include "estrlist.h"
#include "exception.h"

BeginEsdlNamespace()

class TEStrBuffer;

class TDataItem: public AReferencible  {
  TStrPObjList<olxstr,TDataItem*> Items;
  TStrStrList  Fields;
  olxstr Name, Value;
  olxstr ExtractBlockName(olxstr &Str);

  olxstr CodeString(const olxstr& Str)  const;
  olxstr DecodeString(const olxstr& Str) const;

  TDataItem* Parent;
  int Level, Index;
  void* Data;
protected:
  TDataItem& Root();
  olxstr GetFullName();
  TDataItem *DotItem(const olxstr& DotName, TStrList* Log);
  olxstr *DotField(const olxstr& DotName, olxstr &RefFieldName);
  TDataItem& AddItem(TDataItem& Item);
  olxstr* FieldPtr(const olxstr &Name) {
    const size_t i = Fields.IndexOf(Name);
    return (i != InvalidIndex) ? &Fields.GetObject(i) : NULL;
  }
  // to be called from the parser
  void _AddField(const olxstr& name, const olxstr& val) {
    Fields.Add(name, DecodeString(val) );
  }
  inline void SetParent(TDataItem* p)  {  Parent = p;  }
  TEStrBuffer& writeFullName(TEStrBuffer& bf) const;
public:
  TDataItem(TDataItem *Parent, const olxstr& Name, const olxstr& value=EmptyString);
  virtual ~TDataItem() {  Clear();  }
  void Clear();
  void Sort();  // sorts fields and items - improve the access by name performance
  void ResolveFields(TStrList* Log); // resolves referenced fields
  size_t LoadFromString(size_t start, const olxstr &Data, TStrList* Log);
  void SaveToStrBuffer(TEStrBuffer &Data) const;

  TDataItem& AddItem(const olxstr& Name, const olxstr& value=EmptyString);
  // if extend is true the item's content is extended instead of being overwritten
  void AddContent(TDataItem& DI, bool extend=false);
  // implementation of the include instruction object.item
  TDataItem& AddItem(const olxstr &Name, TDataItem *Reference);
  void DeleteItem(TDataItem *Item);

  TDataItem* GetAnyItem(const olxstr& Name) const;
  TDataItem* GetAnyItemCI(const olxstr& Name) const;
  // returns an item by name using recursive search within subitems as well
  // as in the current item
  template <class T> TDataItem* FindItemi(const T& Name) const {  return Items.FindObjecti(Name);  }
  template <class T> TDataItem* FindItem(const T& Name)  const {  return Items.FindObject(Name);  }
  template <class T> TDataItem& FindRequiredItem(const T& Name)   const {  
    size_t i = Items.IndexOf(Name);
    if( i == InvalidIndex )
      throw TFunctionFailedException(__OlxSourceInfo, olxstr("Required item does not exist: ") << Name);
    return *Items.GetObject(i);  
  }

  TDataItem& GetItem(size_t index) {  return *Items.GetObject(index); }
  const TDataItem& GetItem(size_t index) const {  return *Items.GetObject(index); }
  void FindSimilarItems(const olxstr& StartsFrom, TPtrList<TDataItem>& List);
  inline size_t ItemCount() const {  return Items.Count(); }
  template <class T> bool ItemExists(const T &Name) const {  return Items.IndexOf(Name) != InvalidIndex;  }
  size_t IndexOf(TDataItem *I) const {  return Items.IndexOfObject(I); };

  TDataItem& AddField(const olxstr& Name, const olxstr& Data)  {
    Fields.Add(Name, Data);
    return *this;
  }
  inline size_t FieldCount() const                 {  return Fields.Count(); }

  template <class T> size_t FieldIndex(const T& Name) const {  return Fields.IndexOf(Name);  }
  template <class T> size_t FieldIndexi(const T& Name) const {  return Fields.IndexOfi(Name);  }

  const olxstr& GetField(size_t i) const {  return Fields.GetObject(i); }
  // the filed will not be decoded
  const olxstr& FieldName(size_t i) const {  return Fields.GetString(i); }
  // if field does not exist, a new one added
  void SetField(const olxstr& fieldName, const olxstr& newValue) {
    const size_t i = Fields.IndexOf(fieldName);
    if( i == InvalidIndex )
      Fields.Add(fieldName, newValue);
    else
      Fields.GetObject(i) = newValue;
  }
  // deletes field by index
  void DeleteField(size_t index)  {  Fields.Delete(index);  }
  // deletes field by name, only deletes the first one if there are several with the same name
  template <class T> void DeleteField(const T& Name) {
    const size_t fieldIndex = FieldIndex(Name);
    if( fieldIndex != InvalidIndex )  
      DeleteField(fieldIndex);
  }
  template <class T>
  const olxstr& GetFieldValue( const T& Name, const olxstr& Default=EmptyString ) const {
    const size_t i = Fields.IndexOf(Name);
    return (i==InvalidIndex) ? Default : Fields.GetObject(i);
  }
  template <class T>
  const olxstr& GetFieldValueCI( const T& Name, const olxstr& Default=EmptyString ) const {
    const size_t i = Fields.IndexOfi(Name);
    return (i==InvalidIndex) ? Default : Fields.GetObject(i);
  }
  template <class T> const olxstr& GetRequiredField(const T& Name) const  {
    const size_t i = Fields.IndexOf(Name);
    if( i == InvalidIndex )
      throw TFunctionFailedException(__OlxSourceInfo, olxstr("Required attribute is missing: ") << Name);
    return Fields.GetObject(i);
  }

  template <class T> bool FieldExists(const T& Name)   {  return Fields.IndexOf(Name) != InvalidIndex;  }

  TDataItem* GetParent() const {  return Parent; }
  inline int GetLevel() const {  return Level; }
  inline int GetIndex() const {  return Index; }
  DefPropC(olxstr, Name)
  olxstr GetValue() const {  return DecodeString(Value); }
  void SetValue(const olxstr &V)  {  Value = CodeString(V); }
  // for use with whatsoever, initialised twith NULL
  DefPropP(void*, Data)

  void Validate(TStrList& Log);

  class TNonexistingDataItemException: public TBasicException  {
  public:
    TNonexistingDataItemException(const olxstr& location, const olxstr &Msg):
      TBasicException(location, Msg)  {  ;  }
    virtual IEObject* Replicate() const {  return new TNonexistingDataItemException(*this);  }
  };
};

EndEsdlNamespace()
#endif



