//---------------------------------------------------------------------------

#ifndef dataitemH
#define dataitemH
//---------------------------------------------------------------------------
#include "ebase.h"
#include "typelist.h"
#include "tptrlist.h"
#include "estrlist.h"
#include "exception.h"

BeginEsdlNamespace()

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
  void AddItem(TDataItem& Item);
  olxstr* FieldPtr(const olxstr &Name);
  inline void SetParent(TDataItem* p)  {  Parent = p;  }
public:
  TDataItem(TDataItem *Parent, const olxstr& Name);
  virtual ~TDataItem();
  void Clear();
  void Sort();  // sorts fields and items - improve the access by name performance
  void ResolveFields(TStrList* Log); // resolves referenced fields
  int LoadFromString( int start, olxstr &Data, TStrList* Log);
  void SaveToString(olxstr &Data);
  void SaveToStrBuffer(class TEStrBuffer &Data);

  void AddItem(const olxstr &Name, const olxstr &Data);
  void AddContent(TDataItem& DI);
  TDataItem *AddItem(const olxstr &Name);
  // implementation of the include instruction object.item
  void AddItem(const olxstr &Name, TDataItem *Reference);
  void DeleteItem(TDataItem *Item);

  TDataItem* GetAnyItem(const olxstr& Name);
  TDataItem* GetAnyItemCI(const olxstr& Name);
  // returns an item by name using recursive search within subitems as well
  // as in the current item
  TDataItem* FindItemCI(const olxstr &Name) const {  return Items.FindObjectCI(Name);  }
  TDataItem* FindItem(const olxstr &Name)   const {  return Items.FindObject(Name);  }

  TDataItem& Item(int index)                   {  return *Items.Object(index); }
  void FindSimilarItems(const olxstr& StartsFrom, TPtrList<TDataItem>& List);
  inline int ItemCount() const                  {  return Items.Count(); }
  bool ItemExists(const olxstr &Name);
  int IndexOf(TDataItem *I) const               {  return Items.IndexOfObject(I); };

  void AddField(const olxstr& Name, const olxstr& Data);
  void AddCodedField(const olxstr& Name, const olxstr& Data);
  inline int FieldCount() const                 {  return Fields.Count(); }

  int FieldIndex(const olxstr& Name)    const {  return Fields.IndexOf(Name);  }
  int FieldIndexCI(const olxstr& Name)  const {  return Fields.CIIndexOf(Name);  }

  olxstr Field(int i)                   const {  return DecodeString(Fields.Object(i)); }
  // the filed will not be decoded
  const olxstr& RawField(int i)         const {  return Fields.Object(i); }
  const olxstr& FieldName(int i) const        {  return Fields.String(i); }
  // if field does not exist, a new one added
  void SetFieldValue(const olxstr& fieldName, const olxstr& newValue);
  void DeleteField(int index);
  void DeleteField(const olxstr& Name);

  const olxstr& GetFieldValue( const olxstr &Name, const olxstr& Default=EmptyString ) const;
  const olxstr& GetFieldValueCI( const olxstr &Name, const olxstr& Default=EmptyString ) const;

  bool FieldExists(const olxstr &Name);

  TDataItem* GetParent() const           {  return Parent; }
  inline int GetLevel() const            {  return Level; }
  inline int GetIndex() const            {  return Index; }
  DefPropC(olxstr, Name)
  olxstr GetValue()                const {  return DecodeString(Value); }
  void SetValue(const olxstr &V)         {  Value = CodeString(V); }
  // for use with whatsoever, initialised twith NULL
  DefPropP(void*, Data)

  void Validate(TStrList& Log);

  class TNonexistingDataItemException: public TBasicException  {
  public:
    TNonexistingDataItemException(const olxstr& location, const olxstr &Msg):
      TBasicException(location, Msg)  {  ;  }
    virtual IEObject* Replicate()  const  {  return new TNonexistingDataItemException(*this);  }
  };
};

EndEsdlNamespace()
#endif



