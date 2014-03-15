/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "dataitem.h"
#include "estrbuffer.h"
#include "estack.h"
#include "xml.h"

UseEsdlNamespace()
using namespace exparse::parser_util;

TDataItem::TDataItem(TDataItem* Prnt, const olxstr& nm, const olxstr& val)
  : Name(nm), Value(val)
{
  Level = (Prnt == NULL ? 0 : Prnt->GetLevel() + 1);
  Parent = Prnt;
  Data = NULL;
}
//..............................................................................
TDataItem::~TDataItem() {
  Clear();
}
//..............................................................................
void TDataItem::Clear()  {
  Fields.Clear();
  for( size_t i=0; i < Items.Count(); i++ )  {
    if( Items.GetObject(i)->DecRef() == 0 )
      delete Items.GetObject(i);
    else  {
      if( Items.GetObject(i)->GetParent() == this )
        Items.GetObject(i)->SetParent(NULL);
    }
  }
  Items.Clear();
}
//..............................................................................
TDataItem& TDataItem::Root()  {
  if( GetParent() == NULL )  return *this;
  TDataItem *P = GetParent();
  while( P->GetParent() != NULL )
    P = P->GetParent();
  return *P;
}
//..............................................................................
TEStrBuffer& TDataItem::writeFullName(TEStrBuffer& bf) const {
  if (GetParent() == NULL) {
    bf << Name;
  }
  else {
    TStack<TDataItem*> tmp;
    TDataItem *P = GetParent();
    while (P->GetParent() != NULL) {
      tmp.Push(P);
      P = P->GetParent();
    }
    while (!tmp.IsEmpty())
      bf << tmp.Pop()->GetName() << '.';
    bf << Name;
  }
  return bf;
}
//..............................................................................
olxstr TDataItem::GetFullName(const olxstr &sep, const TDataItem *upto) const {
  if (GetParent() == NULL) return GetName();
  olxstr_buf res = GetName();
  TDataItem *p = GetParent();
  if (upto == NULL) {
    while (p != NULL && p->GetParent() != NULL) {
      res << sep << p->GetName();
      p = p->GetParent();
    }
  }
  else {
    while (p != NULL && p != upto) {
      res << sep << p->GetName();
      p = p->GetParent();
    }
  }
  return olxstr::FromExternal(
          res.ReverseRead(olx_malloc<olxch>(res.Length()+1)), res.Length());
}
//..............................................................................
TDataItem *TDataItem::DotItem(const olxstr &DotName, TStrList* Log)  {
  TDataItem* root = &Root();
  TStrList SL(DotName, '.');
  for( size_t i=0; i < SL.Count(); i++ )  {
    root = root->FindItem(SL[i]);
    if( root == NULL )  {
      if( Log != NULL )  
        Log->Add("Unresolved reference: ") << DotName;
      break;
    }
  }
  return root;
}
//..............................................................................
olxstr* TDataItem::DotField(const olxstr& DotName, olxstr& RefFieldName)  {
  TDataItem* root = &Root(), *PrevItem;
  TStrList SL(DotName, '.');
  olxstr *Str=NULL;
  for( size_t i=0; i < SL.Count(); i++ )  {
    PrevItem = root;
    root = root->FindItem(SL[i]);
    if( root == NULL )  {
      RefFieldName = SL[i];
      Str = PrevItem->FieldPtr(SL[i]);
      break;
    }
  }
  return Str;
}
//..............................................................................
TDataItem *TDataItem::FindAnyItem(const olxstr& Name) const {
  TDataItem *DI = Items.FindObject(Name, NULL);
  if (DI == NULL) {
    for (size_t i=0; i < ItemCount(); i++) {
      DI = GetItemByIndex(i).FindAnyItem(Name);
      if (DI != NULL) return DI;
    }
  }
  return DI;
}
//..............................................................................
TDataItem *TDataItem::FindAnyItemi(const olxstr &Name) const {
  TDataItem *DI = Items.FindObjecti(Name);
  if (DI == NULL) {
    for (size_t i=0; i < ItemCount(); i++) {
      DI = GetItemByIndex(i).FindAnyItemi(Name);
      if (DI != NULL) return DI;
    }
  }
  return DI;
}
//..............................................................................
void TDataItem::AddContent(TDataItem& DI, bool extend)  {
  if (extend) {
    for (size_t i=0; i < DI.ItemCount(); i++) {
      TDataItem *di = FindItem(DI.GetItemByIndex(i).GetName());
      if (di != NULL) {
        for (size_t j=0; j < DI.GetItemByIndex(i).ItemCount(); j++)
          di->AddItem(DI.GetItemByIndex(i).GetItemByIndex(j)).SetParent(di);
      }
      else
        AddItem(DI.GetItemByIndex(i)).SetParent(this);
    }
  }
  else {
    for (size_t i=0; i < DI.ItemCount(); i++) {
      TDataItem *di = FindItem(DI.GetItemByIndex(i).GetName());
      if (di != NULL)
        DeleteItem(di);
      AddItem(DI.GetItemByIndex(i)).SetParent(this);
    }
  }
}
//..............................................................................
void TDataItem::DeleteItem(TDataItem *Item)  {
  if( Item->GetParent() != this )  {
    if( Item->GetParent() != NULL )
      Item->GetParent()->DeleteItem(Item);
    return;
  }
  size_t index = Items.IndexOfObject(Item);
  if( index != InvalidIndex )
    Items.Delete(index);
  if( Item->DecRef() == 0 )
    delete Item;
  else
   Item->SetParent(NULL);
}
//..............................................................................
TDataItem& TDataItem::AddItem(const olxstr &Name, TDataItem *Reference)  {
  TDataItem& I = AddItem(Name);
  I.AddItem(*Reference);
  return I;
}
//..............................................................................
TDataItem& TDataItem::AddItem(TDataItem& Item)  {
  Items.Add(Item.GetName(), &Item);
  if( Item.GetParent() == NULL )
    Item.SetParent(this);
  Item.IncRef();
  return Item;
}
//..............................................................................
TDataItem& TDataItem::AddItem(const olxstr& name, const olxstr& val)  {
//  if( !(Name == "!--") )  // comment
//    if( ItemExists(Name) )
//      BasicApp->Log->Exception(olxstr("TDataItem: dublicate definition: ") + Name, false);
  return AddItem(*(new TDataItem(this, name, val)));
}
//..............................................................................
size_t TDataItem::LoadFromString(size_t start, const olxstr &Data,
  TStrList* Log)
{
  const size_t sl = Data.Length();
  for( size_t i=start; i < sl; i++ )  {
    olxch ch = Data[i];
    if (ch == '<') {
      const size_t name_start_i = i+1;
      while (++i < sl) {
        ch = Data[i];
        if (olxstr::o_isoneof(ch, "<>\"") || olxstr::o_iswhitechar(ch))
          break;
      }
      olxstr ItemName = Data.SubString(name_start_i, i-name_start_i);
      if( ItemName.IsEmpty() && Log != NULL )
        Log->Add((this->GetName() + ':') << " empty item name!");
      TDataItem* DI = NULL;
      try {  DI = &AddItem(ItemName);  }
      catch(...) {
        if( Log != NULL )
          Log->Add((this->GetName() + ':') << " cannot add item");
      }
      if( DI != NULL )  {
        i = DI->LoadFromString(i, Data, Log);
        continue;
      }
    }

    if (ch == '>')  return i;
    if (ch == '/' || ch == '\\') continue;
    if (!olxstr::o_iswhitechar(ch))  {
      olxstr FieldName, FieldValue;
      if (is_quote(ch)) {  // item value
        parse_string(Data, Value, i);
        Value = unescape(Value);
        if (i < sl && (Data[i] == '>'))
          return i;
        continue;
      }
      if( (i+1) >= sl ) return sl+1;
      const size_t fn_start = i;
      while (i < sl)  {  // extract field name
        ch = Data.CharAt(i);
        if (olxstr::o_isoneof(ch, "=><\"\'") || olxstr::o_iswhitechar(ch))
          break;
        i++;
      }
      FieldName = Data.SubString(fn_start, i - fn_start);
      if ((skip_whitechars(Data, i)) >= sl)
        return sl+1;
      if (Data[i] == '=') {  // extract field value
        i++;
        if ((skip_whitechars(Data, i)) >= sl)
          return sl+1;
        if (is_quote(Data[i]))  // field value
          parse_string(Data, FieldValue, i);
      }
      else  // the spaces can separate just two field names
        i--;
      if (FieldName.IndexOf('.') != InvalidIndex ) {  // a reference to an item
        TDataItem* DI = DotItem(FieldName, Log);
        if (DI != NULL)
          AddItem(*DI);
        else {  // unresolved so far if !DI
          olxstr RefFieldName;
          olxstr *RefField = DotField(FieldName, RefFieldName);
          if (RefField == NULL)
            _AddField(FieldName, FieldValue); 
          else
            _AddField(RefFieldName, *RefField);
        }
      }
      else {
        _AddField(FieldName, FieldValue);
      }
      if ((i+1) >= sl) return sl+1;
      if (Data[i] == '>') return i;
    }
  }
  return sl+1;
}
//..............................................................................
size_t TDataItem::LoadFromXMLString(size_t start, const olxstr &Data,
  TStrList* Log)
{
  static const olxch* wch = olxT(" \t\r\n");
  static size_t wch_l = olxstr::o_strlen(wch);
  bool can_have_fields = true;
  const size_t sl = Data.Length();
  if (Name.StartsFrom('!')) {
    size_t i = start, ob=1;
    while (i < sl) {
      if (Data[i] == '>' && --ob == 0) {
        SetValue(Data.SubString(start, i - start));
        return i;
      }
      else if (Data[i] == '<')
        ob++;
      i++;
    }
    return sl+1;
  }
  for (size_t i = start; i < sl; i++)  {
    olxch ch = Data[i];
    if (ch == '<') {
      const size_t name_start_i = i + 1;
      while (++i < sl) {
        ch = Data[i];
        if (ch == '>' || olxstr::o_isoneof(ch, wch, wch_l))
          break;
      }
      olxstr ItemName = Data.SubString(name_start_i, i - name_start_i);
      if (ItemName.IsEmpty() && Log != NULL)
        Log->Add((this->GetName() + ':') << " empty item name!");
      if (ItemName.StartsFrom('/') && ch == '>') {
        if (!ItemName.SubStringFrom(1).Equals(GetName())) {
          if (Log != NULL) {
            Log->Add((this->GetName() + ':') << " wrong closing token: '" <<
              ItemName << '\'');
          }
        }
        return i;
      }
      TDataItem* DI = NULL;
      try {
        if (ch == '>' && ItemName.EndsWith('/')) {
          AddItem(ItemName.SubStringTo(ItemName.Length()-1));
          continue;
        }
        DI = &AddItem(ItemName);
      }
      catch (...) {
        if (Log != NULL)
          Log->Add((this->GetName() + ':') << " cannot add item");
      }
      if (DI != NULL) {
        i = DI->LoadFromXMLString(i, Data, Log);
        continue;
      }
    }
    if (ch == '>') {
      can_have_fields = false;
      continue;
    }
    if (ch == '?' && (i + 1) < sl && Data[i + 1] == '>') {
      return i + 1;
    }
    if (ch == '/') {
      if (i + 1 < sl && Data[i + 1] == '>')
        return i + 1;
      else {
        if (Log != NULL)
          Log->Add((this->GetName() + ':') << " invalid / occurance");
      }
    }
    if (!olxstr::o_isoneof(ch, wch, wch_l)) {
      if (can_have_fields) {
        olxstr FieldName, FieldValue;
        const size_t fn_start = i;
        while (i < sl)  {  // extract field name
          ch = Data.CharAt(i);
          if (ch == '=' || olxstr::o_isoneof(ch, wch, wch_l))
            break;
          i++;
        }
        FieldName = Data.SubString(fn_start, i - fn_start);
        if ((skip_chars(Data, i, wch, wch_l)) >= sl)
          return sl + 1;
        if (Data[i] == '=') {  // extract field value
          i++;
          if ((skip_chars(Data, i, wch, wch_l)) >= sl)
            return sl + 1;
          if (is_quote(Data[i]))  // field value
            parse_string(Data, FieldValue, i);
        }
        else { // the spaces can separate just two field names
          i--;
        }
        Fields.Add(FieldName, XML::decode(FieldValue)).SetB(Fields.Count());
        if ((i + 1) >= sl) return sl + 1;
        if (Data[i] == '/' && Data[i + 1] == '>') return i + 1;
      }
      else {
        const size_t fn_start = i;
        while (i < sl)  {  // extract field name
          ch = Data.CharAt(i);
          if (ch == '<')
            break;
          i++;
        }
        Value = XML::decode(Data.SubString(fn_start, i-fn_start));
        i--;
      }
    }
  }
  return sl + 1;
}
//..............................................................................
void TDataItem::ResolveFields(TStrList* Log) {  // resolves referenced fields
  for (size_t i=0; i < FieldCount(); i++) {
    const olxstr& Tmp = Fields.GetKey(i);
    if (Tmp.Contains('.')) {  // a reference to an item
      TDataItem *DI = DotItem(Tmp, Log);
      if (DI != NULL) {
        AddItem(*DI);
        if (Log != NULL)
          Log->Add(olxstr("Resolved: ").quote() << Tmp);
        Fields.Delete(i--);  // might be very slow !!!
      }
      else {
        olxstr RefFieldName;
        olxstr *RefFieldValue = DotField(Tmp, RefFieldName);
        if (RefFieldValue == NULL) {
          if (Log != NULL)
            Log->Add(olxstr("UNRESOLVED: ").quote() << Tmp);
        }
        else {
          if (Log != NULL)
            Log->Add(olxstr("Resolved field: ").quote() << Tmp);
          size_t idx = Fields.GetValue(i).GetB();
          Fields.Delete(i);
          Fields.Add(RefFieldName, *RefFieldValue).SetB(idx);
        }
      }
    }
  }
  for (size_t i=0; i < ItemCount(); i++) {
    if (GetItemByIndex(i).GetParent() == this)
      GetItemByIndex(i).ResolveFields(Log);
  }
}
//..............................................................................
void TDataItem::SaveToStrBuffer(TEStrBuffer &Data) const {
  bool itemsadded = false;
  if (GetParent() != NULL ) {
    if (!Data.IsEmpty())
      Data << NewLineSequence();
    Data << olxstr::CharStr(' ', Level - 1);
    Data << '<' << Name;
    if (!Value.IsEmpty()) {
      Data << " \"" << escape(Value) << "\"";
    }
  }
  const size_t fc = FieldCount();
  for (size_t i=0; i < fc; i++) {
    Data << ' ' << GetFieldName(i) << '=';
    Data << '\"' << escape(GetFieldByIndex(i)) << '\"';
  }
  const size_t ic = ItemCount();
  for (size_t i=0; i < ic; i++) {
    // dot operator
    if( GetItemByIndex(i).GetParent() != this &&
      GetItemByIndex(i).GetParent() != NULL)
    {
      Data << ' ';
      GetItemByIndex(i).writeFullName(Data);
    }
  }
  for (size_t i=0; i < ic; i++) {
    if (GetItemByIndex(i).GetParent() == this ||
      GetItemByIndex(i).GetParent() == NULL)
    {
      GetItemByIndex(i).SaveToStrBuffer(Data);
      itemsadded = true;
    }
  }
  if (GetParent() != NULL) {
    if (itemsadded)  {
      Data << NewLineSequence() << olxstr::CharStr(' ', Level-1);
    }
    Data << '>';
  }
}
//..............................................................................
void TDataItem::SaveToXMLStrBuffer(TEStrBuffer &Data) const {
  olxstr ident = olxstr::CharStr(' ', Level);
  const size_t ic = ItemCount();
  size_t si = 0;
  for (size_t i = 0; i < ic; i++) {
    // dot operator
    if (GetItemByIndex(i).GetParent() != this &&
      GetItemByIndex(i).GetParent() != NULL)
    {
      Data << ident << "value=\"";
      GetItemByIndex(i).writeFullName(Data);
      Data << '"';
    }
    else {
      si++;
    }
  }
  if (si == 0) {
    if (GetParent() == NULL) return;
    if (Data.Length() != 0)
      Data << NewLineSequence();
    Data << ident.SubStringFrom(1);
    Data << '<' << Name;
    if (Name.StartsFrom('!')) {
      Data << XML::encode(GetValue()) << '>';
    }
    else {
      for (size_t i = 0; i < Fields.Count(); i++) {
        Data << ' ' << GetFieldName(i) << "=\"" <<
          XML::encode(GetFieldByIndex(i)) << '"';
      }
      if (!Value.IsEmpty()) {
        Data << '>' << XML::encode(Value) << "</" << Name << '>';
      }
      else {
        Data << (Name.StartsFrom('?') ? '?' : '/') << '>';
      }
    }
    return;
  }
  else {
    if (GetParent() != NULL) {
      if (Data.Length() != 0)
        Data << NewLineSequence();
      Data << ident.SubStringFrom(1);
      Data << '<' << Name;
      if (!Value.IsEmpty()) {
        Data << " value=\"" << XML::encode(Value) << '"';
      }
      for (size_t i = 0; i < Fields.Count(); i++) {
        Data << ' ' << GetFieldName(i) << "=\"" <<
          XML::encode(GetFieldByIndex(i)) << '"';
      }
      if (si > 0) {
        Data << '>';
      }
    }
    for (size_t i = 0; i < ic; i++) {
      if (GetItemByIndex(i).GetParent() == this ||
        GetItemByIndex(i).GetParent() == NULL)
      {
        GetItemByIndex(i).SaveToXMLStrBuffer(Data);
      }
    }
  }
  if (GetParent() != NULL) {
    if (si == 0) {
      Data << "/>";
    }
    else {
      Data << NewLineSequence() << ident.SubStringFrom(1);
      Data << "</" << Name << '>';
    }
  }
}
//..............................................................................
void TDataItem::FindSimilarItems(const olxstr &StartsFrom,
  TPtrList<TDataItem>& List)
{
  for( size_t i=0; i < ItemCount(); i++ )  {
    if( Items[i].StartsFromi(StartsFrom) )
      List.Add(Items.GetObject(i));
  }
}
//..............................................................................
// sorts fields and items - improve the access by name performance
void TDataItem::Sort() {
  Items.QSort(false);
}
//..............................................................................
void TDataItem::Validate(TStrList& Log)  {
  for (size_t i=0; i < ItemCount(); i++) {
    if (GetItemByIndex(i).GetParent() != NULL &&
        (GetItemByIndex(i).GetParent() != this))
    {
      Log.Add("Referred item from ") << this->GetFullName() << "->" <<
        GetItemByIndex(i).GetFullName();
    }
    if (GetItemByIndex(i).GetParent() == NULL) {
      Log.Add("Item with null Parent: ") <<
        GetItemByIndex(i).GetFullName();
    }
  }
}
//..............................................................................
const_strstrlist TDataItem::GetOrderedFieldList() const {
  TStrStrList rv(Fields.Count());
  for (size_t i = 0; i < Fields.Count(); i++) {
    size_t idx = Fields.GetValue(i).GetB();
    if (idx >= Fields.Count()) {
      throw TFunctionFailedException(__OlxSourceInfo, "field set is modified");
    }
    rv[idx] = Fields.GetKey(i);
    rv.GetObject(idx) = Fields.GetValue(i).GetA();
  }
  return rv;
}
//..............................................................................
void TDataItem::UpdateFieldIndices(size_t deleted) {
  for (size_t i = 0; i < Fields.Count(); i++) {
    if (Fields.GetValue(i).GetB() > deleted)
      Fields.GetValue(i).b--;
  }
}
//..............................................................................
void TDataItem::ValueFieldToValue() {
  size_t vidx = Fields.IndexOf("value");
  if (vidx != InvalidIndex) {
    SetValue(Fields.GetValue(vidx).GetA());
    DeleteFieldByIndex(vidx, true);
  }
  for (size_t i = 0; i < Items.Count(); i++) {
    Items.GetObject(i)->ValueFieldToValue();
  }
}
