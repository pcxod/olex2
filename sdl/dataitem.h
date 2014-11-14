/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_dataitem_H
#define __olx_sdl_dataitem_H
#include "ebase.h"
#include "typelist.h"
#include "tptrlist.h"
#include "estrlist.h"
#include "exception.h"
#include "exparse/exptree.h"
BeginEsdlNamespace()

class TEStrBuffer;

class TDataItem: public AReferencible  {
  TStringToList<olxstr, TDataItem*> Items;
  typedef olx_pair_t<olxstr, size_t> field_t;
  olxstr_dict<field_t, false> Fields;
  olxstr Name, Value;
  TDataItem* Parent;
  size_t Level;
  void* Data;
  void UpdateFieldIndices(size_t deleted);
protected:
  TDataItem& Root();
  TDataItem *DotItem(const olxstr& DotName, TStrList* Log);
  olxstr *DotField(const olxstr& DotName, olxstr &RefFieldName);
  TDataItem& AddItem(TDataItem& Item);
  olxstr* FieldPtr(const olxstr &Name) {
    const size_t i = Fields.IndexOf(Name);
    return (i != InvalidIndex) ? &Fields.GetValue(i).a : NULL;
  }
  // to be called from the parser
  void _AddField(const olxstr& name, const olxstr& val) {
    field_t & f = Fields.Add(name, exparse::parser_util::unescape(val));
    f.SetB(Fields.Count()-1);
  }
  void SetParent(TDataItem* p)  {  Parent = p;  }
  TEStrBuffer& writeFullName(TEStrBuffer& bf) const;
public:
  TDataItem(TDataItem *Parent, const olxstr& Name,
    const olxstr& value=EmptyString());
  virtual ~TDataItem();
  void Clear();
  // sorts fields and items - improve the access by name performance
  void Sort();
  void ResolveFields(TStrList* Log); // resolves referenced fields
  size_t LoadFromString(size_t start, const olxstr &Data, TStrList* Log);
  size_t LoadFromXMLString(size_t start, const olxstr &Data, TStrList* Log);
  void ValueFieldToValue();
  void SaveToStrBuffer(TEStrBuffer &Data) const;
  void SaveToXMLStrBuffer(TEStrBuffer &Data) const;

  TDataItem& AddItem(const olxstr& Name, const olxstr& value=EmptyString());
  /* if extend is true the item's content is extended instead of being
  overwritten
  */
  void AddContent(TDataItem& DI, bool extend=false);
  // implementation of the include instruction object.item
  TDataItem& AddItem(const olxstr &Name, TDataItem *Reference);
  void DeleteItem(TDataItem *Item);
  // does recursive search
  TDataItem* FindAnyItem(const olxstr& Name) const;
  // does recursive search
  TDataItem* FindAnyItemi(const olxstr& Name) const;
  // returns an item by name using recursive search within subitems as well
  // as in the current item
  template <class T> TDataItem* FindItemi(const T& Name,
    TDataItem *def=0) const
  {
    return Items.FindPointeri(Name, def);
  }
  template <class T> TDataItem* FindItem(const T& Name,
    TDataItem *def=0) const
  {
    return Items.FindPointer(Name, def);
  }
  /* finds and returns specified item, throws an exception if the items
  does not exist.
  */
  template <class T> TDataItem& GetItemByName(const T& Name) const {
    size_t i = Items.IndexOf(Name);
    if (i == InvalidIndex) {
      throw TFunctionFailedException(__OlxSourceInfo,
        olxstr("Required item does not exist: ").quote() << Name);
    }
    return *Items.GetObject(i);
  }

  TDataItem& GetItemByIndex(size_t index) const {
    return *Items.GetObject(index);
  }
  void FindSimilarItems(const olxstr& StartsFrom, TPtrList<TDataItem>& List);
  size_t ItemCount() const { return Items.Count(); }
  template <class T> bool ItemExists(const T &Name) const {
    return Items.IndexOf(Name) != InvalidIndex;
  }

  size_t IndexOf(TDataItem *I) const { return Items.IndexOfObject(I); }
  size_t FieldCount() const { return Fields.Count(); }

  template <class T> size_t FieldIndex(const T& Name) const {
    return Fields.IndexOf(Name);
  }
  template <typename T>
  TDataItem & AddField(const T& fieldName, const olxstr& newValue) {
    const size_t i = Fields.IndexOf(fieldName);
    if (i == InvalidIndex) {
      field_t &f = Fields.Add(fieldName, newValue);
      f.SetB(Fields.Count() - 1);
    }
    else {
      Fields.GetValue(i).SetA(newValue);
    }
    return *this;
  }
  const olxstr& GetFieldByIndex(size_t i) const {
    return Fields.GetValue(i).GetA();
  }
  template <class T> const olxstr& GetFieldByName(const T& Name) const {
    const size_t i = Fields.IndexOf(Name);
    if (i == InvalidIndex) {
      throw TFunctionFailedException(__OlxSourceInfo,
        olxstr("Required attribute is missing: ").quote() << Name);
    }
    return Fields.GetValue(i).GetA();
  }
  const olxstr& GetFieldName(size_t i) const { return Fields.GetKey(i); }
  // deletes field by index
  void DeleteFieldByIndex(size_t index, bool updateIndices = false) {
    Fields.Delete(index);
    if (updateIndices)
      UpdateFieldIndices(index);
  }
  /* deletes field by name, only deletes the first one if there are several
  with the same name. Returns true if the field is deleted.
  */
  template <class T> bool DeleteFieldByName(const T& Name,
    bool updateIndices = false)
  {
    const size_t fieldIndex = FieldIndex(Name);
    if (fieldIndex != InvalidIndex) {
      DeleteFieldByIndex(fieldIndex, updateIndices);
      return true;
    }
    return false;
  }
  template <class T> const olxstr& FindField(const T& Name,
    const olxstr& Default=EmptyString()) const
  {
    const size_t i = Fields.IndexOf(Name);
    return (i==InvalidIndex) ? Default : Fields.GetValue(i).GetA();
  }

  template <class T> bool FieldExists(const T& Name) {
    return Fields.IndexOf(Name) != InvalidIndex;
  }
  const_strstrlist GetOrderedFieldList() const;

  TDataItem* GetParent() const { return Parent; }
  size_t GetLevel() const { return Level; }
  olxstr GetFullName(const olxstr &sep='.', const TDataItem *upto=NULL) const;
  DefPropC(olxstr, Name)
  const olxstr &GetValue() const {  return Value; }
  void SetValue(const olxstr &V)  {  Value = V; }
  // for use with whatsoever, initialised twith NULL
  DefPropP(void*, Data)

  void Validate(TStrList& Log);

  class TNonexistingDataItemException: public TBasicException  {
  public:
    TNonexistingDataItemException(const olxstr& location, const olxstr &Msg):
      TBasicException(location, Msg)
      {}
    virtual IOlxObject* Replicate() const {
      return new TNonexistingDataItemException(*this);
    }
  };
};

EndEsdlNamespace()
#endif
