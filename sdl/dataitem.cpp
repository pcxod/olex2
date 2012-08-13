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

UseEsdlNamespace()
using namespace exparse::parser_util;

TDataItem::TDataItem(TDataItem* Prnt, const olxstr& nm, const olxstr& val)
  : Name(nm), Value(val)
{
  if( Prnt != NULL )  {
    Level = Prnt->GetLevel()+1;
    Index = (int)Prnt->ItemCount();
  }
  else  {
    Level = 0;
    Index = -1;
  }
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
  if( GetParent() == NULL )  {
    bf << Name;
  }
  else  {
    TPtrList<TDataItem> tmp;
    TDataItem *P = GetParent();
    while( P->GetParent() != NULL )  {
      tmp.Add(P);
      P = P->GetParent();
    }
    for( size_t i=tmp.Count(); i > 0; i-- )
      bf << tmp[i-1]->GetName() << '.';
    bf << Name;
  }
  return bf;
}
//..............................................................................
olxstr TDataItem::GetFullName()  {
  if( GetParent() == NULL )
    return GetName();
  olxstr_buf res = GetName();
  olxstr ds = '.';
  TDataItem *p = GetParent();
  while( p != NULL )  {
    res << ds << p->GetName();
    p = p->GetParent();
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
TDataItem *TDataItem::GetAnyItem(const olxstr& Name) const {
  TDataItem *DI = Items.FindObject(Name);
  if( DI == NULL )  {
    for( size_t i=0; i < ItemCount(); i++ )  {
      DI = GetItem(i).GetAnyItem(Name);
      if( DI != NULL )  return DI;
    }
  }
  return DI;
}
//..............................................................................
TDataItem *TDataItem::GetAnyItemCI(const olxstr &Name) const {
  TDataItem *DI = Items.FindObjecti(Name);
  if( DI == NULL )  {
    for( size_t i=0; i < ItemCount(); i++ )  {
      DI = GetItem(i).GetAnyItemCI(Name);
      if( DI != NULL )  return DI;
    }
  }
  return DI;
}
//..............................................................................
void TDataItem::AddContent(TDataItem& DI, bool extend)  {
  if( extend )  {
    for( size_t i=0; i < DI.ItemCount(); i++ )  {
      TDataItem *di = FindItem(DI.GetItem(i).GetName());
      if( di != NULL )  {
        for( size_t j=0; j < DI.GetItem(i).ItemCount(); j++ )
          di->AddItem(DI.GetItem(i).GetItem(j)).SetParent(di);
      }
      else
        AddItem(DI.GetItem(i)).SetParent(this);
    }
  }
  else  {
    for( size_t i=0; i < DI.ItemCount(); i++ )  {
      TDataItem *di = FindItem(DI.GetItem(i).GetName());
      if( di != NULL )
        DeleteItem(di);
      AddItem(DI.GetItem(i)).SetParent(this);
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
size_t TDataItem::LoadFromString(size_t start, const olxstr &Data, TStrList* Log)  {
  const size_t sl = Data.Length();
  for( size_t i=start; i < sl; i++ )  {
    olxch ch = Data[i];
    if (ch == '<') {
      const size_t name_start_i = i+1;
      while (++i < sl) {
        ch = Data[i];
        if (olxstr::o_is_oneof(ch, "<>\"") || olxstr::o_iswhitechar(ch))
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
        if (olxstr::o_is_oneof(ch, "=><\"\'") || olxstr::o_iswhitechar(ch))
          break;
        i++;
      }
      FieldName = Data.SubString(fn_start, i - fn_start);
      if ((skip_whitechars(Data, i)+1) >= sl)
        return sl+1;
      if (Data[i] == '=') {  // extract field value
        i++;
        if ((skip_whitechars(Data, i)+1) >= sl)
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
void TDataItem::ResolveFields(TStrList* Log)  {  // resolves referenced fields
  for( size_t i=0; i < FieldCount(); i++ )  {
    const olxstr& Tmp = Fields[i];
    if( Tmp.IndexOf('.') != InvalidIndex )  {  // a reference to an item
      TDataItem *DI = DotItem(Tmp, Log);
      if( DI != NULL )  {
        AddItem(*DI);
        if( Log != NULL )
          Log->Add(olxstr("Resolved: ") << Tmp);
        Fields.Delete(i);  // might be very slow !!!
        i--;
      }
      else  {
        olxstr RefFieldName(EmptyString());
        olxstr *RefFieldValue = DotField(Tmp, RefFieldName);
        if( RefFieldValue == NULL )  {
          if( Log != NULL )
            Log->Add(olxstr("UNRESOLVED: ") << Tmp);
        }
        else  {
          if( Log )  Log->Add(olxstr("Resolved field: ") << Tmp);
          Fields[i] = RefFieldName;
          Fields.GetObject(i) = *RefFieldValue;
        }
      }
    }
  }
  for( size_t i=0; i < ItemCount(); i++ )  {
    if( GetItem(i).GetParent() == this )
      GetItem(i).ResolveFields(Log);
  }
}
//..............................................................................
void TDataItem::SaveToStrBuffer(TEStrBuffer &Data) const {
  bool itemsadded = false;
  if( GetParent() != NULL )  {
    if( Data.Length() != 0 )
      Data << NewLineSequence();
    for( int i=0; i < Level-1; i++ )    Data << ' ';
    Data << '<' << Name << ' ';
    if( !Value.IsEmpty() )  {
      Data << '\"' << escape(Value) << "\" ";
    }
  }
  const size_t fc = FieldCount();
  for( size_t i=0; i < fc; i++ )  {
    Data << Fields.GetString(i) << '=';
    Data << '\"' << escape(Fields.GetObject(i)) << '\"' << ' ';
  }
  const size_t ic = ItemCount();
  for( size_t i=0; i < ic; i++ )  {
    if( GetItem(i).GetParent() != this && GetItem(i).GetParent() != NULL )  {  // dot operator
      GetItem(i).writeFullName(Data) << ' ';
    }
  }
  for( size_t i=0; i < ic; i++ )  {
    if( GetItem(i).GetParent() == this || GetItem(i).GetParent() == NULL )  {
      GetItem(i).SaveToStrBuffer(Data);
      if( !itemsadded ) itemsadded = true;
    }
  }
  if( GetParent() != NULL )  {
    if( itemsadded )  {
      Data << NewLineSequence();
      for( int i=0; i < Level-1; i++ )
        Data << ' ';
      Data << '>';
    }
    else  {
      Data << '>';
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
void TDataItem::Sort()  {
  Items.QSort(false);
  Fields.QSort(false);
}
//..............................................................................
void TDataItem::Validate(TStrList& Log)  {
  olxstr Tmp;
  for( size_t i=0; i < ItemCount(); i++ )  {
    if( GetItem(i).GetParent() != NULL && (GetItem(i).GetParent() != this) )  {
      Tmp = "Referred item from ";
      Tmp << this->GetFullName() << "->" << GetItem(i).GetFullName();
      Log.Add(Tmp);
    }
    if( GetItem(i).GetParent() == NULL )  {
      Tmp = "Item with null Parent: ";
      Tmp << GetItem(i).GetFullName();
      Log.Add(Tmp);
    }
  }
}
