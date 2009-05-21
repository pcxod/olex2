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
    const int i = Fields.IndexOf(Name);
    return (i != -1) ? &Fields.GetObject(i) : NULL;
  }
  // to be called from the parser
  void _AddField(const olxstr& name, const olxstr& val) {
    Fields.Add(name, DecodeString(val) );
  }
  inline void SetParent(TDataItem* p)  {  Parent = p;  }
  TEStrBuffer& writeFullName(TEStrBuffer& bf) const;
public:
  TDataItem(TDataItem *Parent, const olxstr& Name, const olxstr& value=EmptyString);
  virtual ~TDataItem();
  void Clear();
  void Sort();  // sorts fields and items - improve the access by name performance
  void ResolveFields(TStrList* Log); // resolves referenced fields
  int LoadFromString( int start, olxstr &Data, TStrList* Log);
  void SaveToStrBuffer(TEStrBuffer &Data) const;

  TDataItem& AddItem(const olxstr& Name, const olxstr& value=EmptyString);
  void AddContent(TDataItem& DI);
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
    int i = Items.IndexOf(Name);
    if( i == -1 )
      throw TFunctionFailedException(__OlxSourceInfo, olxstr("Required item does not exist: ") << Name);
    return *Items.GetObject(i);  
  }

  TDataItem& GetItem(int index)                {  return *Items.GetObject(index); }
  const TDataItem& GetItem(int index)    const {  return *Items.GetObject(index); }
  void FindSimilarItems(const olxstr& StartsFrom, TPtrList<TDataItem>& List);
  inline int ItemCount() const                  {  return Items.Count(); }
  bool ItemExists(const olxstr &Name);
  int IndexOf(TDataItem *I) const               {  return Items.IndexOfObject(I); };

  TDataItem& AddField(const olxstr& Name, const olxstr& Data)  {
    Fields.Add(Name, Data);
    return *this;
  }
  inline int FieldCount() const                 {  return Fields.Count(); }

  template <class T> int FieldIndex(const T& Name)   const {  return Fields.IndexOf(Name);  }
  template <class T> int FieldIndexi(const T& Name)  const {  return Fields.IndexOfi(Name);  }

  const olxstr& GetField(int i)         const {  return Fields.GetObject(i); }
  // the filed will not be decoded
  const olxstr& FieldName(int i) const        {  return Fields.GetString(i); }
  // if field does not exist, a new one added
  void SetField(const olxstr& fieldName, const olxstr& newValue) {
    const int i = Fields.IndexOf(fieldName);
    if( i == -1 )
      Fields.Add(fieldName, newValue);
    else
      Fields.GetObject(i) = newValue;
  }
  // deletes field by index
  void DeleteField(int index)  {  Fields.Delete(index);  }
  // deletes field by name, only deletes the first one if there are several with the same name
  template <class T> void DeleteField(const T& Name) {
    int fieldIndex = FieldIndex(Name);
    if( fieldIndex != -1 )  
      DeleteField(fieldIndex);
  }
  template <class T>
  const olxstr& GetFieldValue( const T& Name, const olxstr& Default=EmptyString ) const {
    int i = Fields.IndexOf(Name);
    return (i==-1) ? Default : Fields.GetObject(i);
  }
  template <class T>
  const olxstr& GetFieldValueCI( const T& Name, const olxstr& Default=EmptyString ) const {
    int i = Fields.IndexOfi(Name);
    return (i==-1) ? Default : Fields.GetObject(i);
  }
  template <class T> const olxstr& GetRequiredField(const T& Name) const  {
    int i = Fields.IndexOf(Name);
    if( i == -1 )
      throw TFunctionFailedException(__OlxSourceInfo, olxstr("Required attribute is missing: ") << Name);
    return Fields.GetObject(i);
  }

  template <class T> bool FieldExists(const T& Name)   {  return Fields.IndexOf(Name) != -1;  }

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



