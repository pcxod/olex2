//---------------------------------------------------------------------------//
// TDataItem
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "dataitem.h"
#include "estrbuffer.h"

  static olxstr DoubleQuoteCode("&2E;");

UseEsdlNamespace()

//..............................................................................
TDataItem::TDataItem(TDataItem* Prnt, const olxstr& nm, const olxstr& val) : Name(nm), Value(val)  {
  if( Prnt != NULL )  {
    Level = Prnt->GetLevel()+1;
    Index = Prnt->ItemCount();
  }
  else  {
    Level = 0;
    Index = -1;
  }
  Parent = Prnt;
  Data = NULL;
}
//..............................................................................
TDataItem::~TDataItem()  {
  Clear();
}
//..............................................................................
void TDataItem::Clear()  {
  Fields.Clear();
  for( int i=0; i < Items.Count(); i++ )  {
    if( Items.Object(i)->GetRefCount() < 1 ) delete Items.Object(i);
    else  {
      if( Items.Object(i)->GetParent() == this )
        Items.Object(i)->SetParent(NULL);
      Items.Object(i)->DecRef();
    }
  }
  Items.Clear();
}
//..............................................................................
bool TDataItem::ItemExists(const olxstr &Name)  {
  return (Items.IndexOf(Name) == -1) ? false : true;
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
TEStrBuffer& TDataItem::writeFullName(TEStrBuffer& bf)  {
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
    for( int i=tmp.Count()-1; i >= 0; i-- )
      bf << tmp[i]->GetName() << '.';
    bf << Name;
  }
  return bf;
}
//..............................................................................
olxstr TDataItem::GetFullName()  {
  if( GetParent() == NULL )
    return Name;
  TStrList SL;
  SL.Add( Name );
  TDataItem *P = GetParent();
  while( P->GetParent() != NULL )  {
    SL.Insert(0, P->GetName());
    P = P->GetParent();
  }
  return SL.Text('.');
}
//..............................................................................
olxstr* TDataItem::FieldPtr(const olxstr &Name)  {
  int i = Fields.IndexOf(Name);
  return (i != -1) ? &Fields.Object(i) : NULL;
}
//..............................................................................
TDataItem *TDataItem::DotItem(const olxstr &DotName, TStrList* Log)  {
  TDataItem* root = &Root();
  TStrList SL(DotName, '.');
  for( int i=0; i < SL.Count(); i++ )  {
    root = root->FindItem( SL[i] );
    if( root == NULL )  {
      if( Log != NULL )  Log->Add(olxstr("Unresolved reference: ") << DotName);
      break;
    }
  }
  return root;
}
//..............................................................................
olxstr* TDataItem::DotField(const olxstr &DotName, olxstr &RefFieldName)  {
  TDataItem* root = &Root(), *PrevItem;
  TStrList SL(DotName, '.');
  olxstr *Str=NULL;
  for( int i=0; i < SL.Count(); i++ )  {
    PrevItem = root;
    root = root->FindItem( SL[i] );
    if( root == NULL )  {
      RefFieldName = SL[i];
      Str = PrevItem->FieldPtr( SL[i] );
      break;
    }
  }
  return Str;
}
//..............................................................................
TDataItem *TDataItem::GetAnyItem(const olxstr &Name)  {
  TDataItem *DI = Items.FindObject(Name);
  if( DI == NULL )  {
    for( int i=0; i < ItemCount(); i++ )  {
      DI = Item(i).GetAnyItem(Name);
      if( DI != NULL )  return DI;
    }
  }
  return DI;
}
//..............................................................................
TDataItem *TDataItem::GetAnyItemCI(const olxstr &Name)  {
  TDataItem *DI = Items.FindObjectCI(Name);
  if( DI == NULL )  {
    for( int i=0; i < ItemCount(); i++ )  {
      DI = Item(i).GetAnyItemCI(Name);
      if( DI != NULL )  return DI;
    }
  }
  return DI;
}
//..............................................................................
void TDataItem::AddContent(TDataItem& DI)  {
  TDataItem *di;
  for( int i=0; i < DI.ItemCount(); i++ )  {
    di = FindItem( DI.Item(i).GetName() );
    if( di != NULL )
      DeleteItem(di);
    AddItem( DI.Item(i) );
  }
}
//..............................................................................
void TDataItem::DeleteItem(TDataItem *Item)  {
  if( Item->GetParent() != this )  {
    if( Item->GetParent() )
      Item->GetParent()->DeleteItem(Item);
    return;
  }
  int index = Items.IndexOfObject(Item);
  if( index >= 0 )  Items.Delete(index);
  if( Item->GetRefCount() < 1 )
    delete Item;
  else  {
   Item->DecRef();
   Item->SetParent(NULL);
  }
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
  if( Item.GetParent() == NULL)
      Item.SetParent(this);
  Item.IncRef();
  return Item;
}
//..............................................................................
TDataItem& TDataItem::AddItem(const olxstr& name, const olxstr& val)  {
//  if( !(Name == "!--") )  // comment
//    if( ItemExists(Name) )
//      BasicApp->Log->Exception(olxstr("TDataItem: dublicate definition: ") + Name, false);
  TDataItem *DI = new TDataItem(this, name, val);
  AddItem(*DI);
  DI->DecRef(); //!!!
  return *DI;
}
//..............................................................................
bool TDataItem::FieldExists(const olxstr &Name)  {
  return (Fields.IndexOf(Name) >=0 );
}
//..............................................................................
void TDataItem::AddField(const olxstr &Name, const olxstr &Data)  {
  AddCodedField(Name, CodeString(Data));
}
//..............................................................................
void TDataItem::SetFieldValue(const olxstr& fieldName, const olxstr &newValue)  {
  int i = Fields.IndexOf(fieldName);
  if( i == -1 )
    AddField(fieldName, newValue);
  else
  Fields.Object(i) = CodeString(newValue);
}
//..............................................................................
void TDataItem::DeleteField(int index)  {
  Fields.Delete(index);
}
//..............................................................................
void TDataItem::DeleteField(const olxstr& Name)
{
  int fieldIndex = FieldIndex(Name);
  if( fieldIndex != -1 )  DeleteField(fieldIndex);
}
//..............................................................................
const olxstr& TDataItem::GetFieldValue( const olxstr &Name, const olxstr &Default ) const  {
  int i = Fields.IndexOf(Name);
  return (i==-1) ? Default : Fields.Object(i);
}
//..............................................................................
const olxstr& TDataItem::GetFieldValueCI( const olxstr &Name, const olxstr &Default ) const  {
  int i = Fields.CIIndexOf(Name);
  return (i==-1) ? Default : Fields.Object(i);
}
//..............................................................................
void TDataItem::AddCodedField(const olxstr &Name, const olxstr &Data)  {
//  if( FieldExists(Name) )
//  {
//    BasicApp->Log->Exception(olxstr("TDataItem: dublicate field name: ") + Name, false);
//  }
  Fields.Add(Name, Data);
}
//..............................................................................
int TDataItem::LoadFromString( int start, olxstr &Data, TStrList* Log)  {
  int sl = Data.Length();
  TDataItem *DI;
  Value = EmptyString;
  olxstr ItemName, FieldName, FieldValue, *RefField, RefFieldName;
  olxch CloseChar;
  for( int i=start; i < sl; i++ )  {
    if( Data.CharAt(i) == '<' )  {
      ItemName = EmptyString;
      while( i < sl )  {
        i++;
        if( Data.CharAt(i) == ' ' || Data.CharAt(i) == '\"' )  break;
        ItemName << Data.CharAt(i);
      }
      if( ItemName.IsEmpty() && Log != NULL )
        Log->Add((this->GetName() + ':') << " empty item name!");
      DI = NULL;
      try {  DI = &AddItem(ItemName);  }
      catch(...) {
        if( Log != NULL )
          Log->Add((this->GetName() + ':') << " cannot add item");
      }
      if( DI != NULL )  {
//        BasicApp->Log->CriticalInfo(((olxstr("Loading: ") += this->Name()) += '.') += ItemName);
        i = DI->LoadFromString(i, Data, Log);
        continue;
      }
    }

    if( Data.CharAt(i) == '>' )  return i;
    if( Data.CharAt(i) == '/' )  continue;
    if( Data.CharAt(i) == '\\' ) continue;
    if( Data.CharAt(i) != ' ')   {
      FieldName = EmptyString;
      FieldValue = EmptyString;
      if( Data.CharAt(i) == '\'' || Data.CharAt(i) == '\"' )  {  // item value
        CloseChar = Data.CharAt(i);  // find closing character: space ' or "
        i++;
        while( i < sl )  {
          if( (Data.CharAt(i) == CloseChar) && (Data.CharAt(i-1) != '^') ){  i++; break; }
          Value << Data.CharAt(i);
          i++;
        }
        if( i < sl && (Data.CharAt(i) == '>') )  {  return i;  }
        continue;
      }
      if( (i+1) >= sl ) return sl+1;
      while( i < sl )  {  // extract field name
        if( (Data.CharAt(i) == ' ') || (Data.CharAt(i) == '=') ||
            (Data.CharAt(i) == '>') || (Data.CharAt(i) == '\"') ||
            (Data.CharAt(i) == '\''))  break;
        FieldName << Data.CharAt(i);
        i++;
      }
      if( (i+1) >= sl ) return sl+1;
      while( i < sl )  {  // skip spaces
        if( Data.CharAt(i) != ' ' )  break;
        i++;
      }
      if( (i+1) >= sl ) return sl+1;
      if( Data.CharAt(i) == '=' )  {  // extract field value
        i++;
        while( i < sl )  {  // skip spaces
          if( Data.CharAt(i) != ' ' )  break;
          i++;
        }
        if( (i+1) >= sl ) return sl+1;
        CloseChar = ' ';  // find closing character: space ' or "
        if( Data.CharAt(i) == '\'' ){  CloseChar = '\''; i++; }
        if( Data.CharAt(i) == '\"' ){  CloseChar = '\"'; i++; }
        while( i < sl )  {
          if( (Data.CharAt(i) == CloseChar) && (Data[i-1] != '^') ){  i++; break; }
//          if( (Data.CharAt(i) == CloseChar) || (Data.CharAt(i) == '>') ){  i++; break; }
          FieldValue << Data.CharAt(i);
          i++;
        }
      }
      else i--; // the spaces can separate just two field names
      if( FieldName.IndexOf('.') >= 0 )  {  // a reference to an item
        DI = DotItem(FieldName, Log);
        if( DI != NULL )  AddItem(*DI); // unresolved so far if !DI
        else  {
          RefField = DotField(FieldName, RefFieldName);
          if( RefField == NULL )  {
            try{ AddField(FieldName, FieldValue); }
            catch(...)  {  if( Log != NULL) Log->Add((this->GetName() + ':') << " cannot add field"); }
          }
          else  {
            try{ AddField(RefFieldName, *RefField); }
            catch(...){  if( Log != NULL )  Log->Add((this->GetName() + ':') << " cannot add field"); }
          }
        }
      }
      else  {
        try{ AddField(FieldName, FieldValue); }
        catch(...){  if( Log != NULL )  Log->Add((this->GetName() + ':') << " cannot add field"); }
      }
      if( (i+1) >= sl ) return sl+1;
      if( Data.CharAt(i) == '>' )  return i;
    }
  }
  return sl+1;
}
//..............................................................................
void TDataItem::ResolveFields(TStrList* Log)  {  // resolves referenced fields
  olxstr Tmp, *RefFieldValue, RefFieldName;
  TDataItem *DI;
  for( int i=0; i < FieldCount(); i++ )  {
    Tmp = Fields.String(i);
    if( Tmp.IndexOf('.') >= 0 )  {  // a reference to an item
      DI = DotItem(Tmp, Log);
      if( DI != NULL )  {
        AddItem(*DI);
        if( Log != NULL )  Log->Add( olxstr("Resolved: ") << Tmp);
        Fields.Delete(i);  // might be very slow !!!
        i--;
      }
      else  {
        RefFieldValue = DotField(Tmp, RefFieldName);
        if( RefFieldValue == NULL )  {
          if( Log != NULL )  Log->Add(olxstr("UNRESOLVED: ") << Tmp);
        }
        else
        {
          if( Log)  Log->Add(olxstr("Resolved field: ") << Tmp);
          Fields.String(i) = RefFieldName;
          Fields.Object(i) = *RefFieldValue;
        }
      }
    }
  }
  for( int i=0; i < ItemCount(); i++ )  {
    if( Item(i).GetParent() == this )
      Item(i).ResolveFields(Log);
  }
}
//..............................................................................
void TDataItem::SaveToString(olxstr &Data)  {
  int fc, ic;
  bool itemsadded = false;
  if( GetParent() != NULL )  {
    Data << '\r' << '\n';
    for( int i=0; i < Level-1; i++ )    Data << ' ';
    Data << '<' << Name << ' ';
    if( !Value.IsEmpty() )
      Data << '\"' << Value << '\"';
  }
  fc = FieldCount();
  ic = ItemCount();
  for( int i=0; i < fc; i++ )
    Data << Fields.String(i) << '=' << '\"' << RawField(i) << '\"' << ' ';
  for( int i=0; i < ic; i++ )  {
    if( Item(i).GetParent() != this )  // dot operator
      Data << Item(i).GetFullName()  << ' ';
  }
  for( int i=0; i < ic; i++ )  {
    if( Item(i).GetParent() == this )  {
      Item(i).SaveToString(Data);
      if( !itemsadded ) itemsadded = true;
    }
  }
  if( GetParent() != NULL )  {
    if( itemsadded )  {
      Data << '\r' << '\n';
      for( int i=0; i < Level-1; i++ )    Data << ' ';
      Data << '>';
    }
    else  {
      Data << '>';
    }
  }
}
//..............................................................................
void TDataItem::SaveToStrBuffer(TEStrBuffer &Data)  {
  int fc, ic;
  bool itemsadded = false;
  if( GetParent() != NULL )  {
    if( Data.Length() != 0 )
      Data << NewLineSequence;
    for( int i=0; i < Level-1; i++ )    Data << ' ';
    Data << '<' << Name << ' ';
    if( !Value.IsEmpty() )  {
      Data << '\"' << Value << '\"';
    }
  }
  fc = FieldCount();
  ic = ItemCount();
  for( int i=0; i < fc; i++ )  {
    Data << Fields.String(i) << '=';
    Data << '\"' << RawField(i) << '\"' << ' ';
  }
  for( int i=0; i < ic; i++ )  {
    if( Item(i).GetParent() != this )  {  // dot operator
      Item(i).writeFullName(Data) << ' ';
    }
  }
  for( int i=0; i < ic; i++ )  {
    if( Item(i).GetParent() == this )  {
      Item(i).SaveToStrBuffer(Data);
      if( !itemsadded ) itemsadded = true;
    }
  }
  if( GetParent() != NULL )  {
    if( itemsadded )  {
      Data << '\r' << '\n';
      for( int i=0; i < Level-1; i++ )    Data << ' ';
      Data << '>';
    }
    else  {
      Data << '>';
    }
  }
}
//..............................................................................
void TDataItem::FindSimilarItems(const olxstr &StartsFrom, TPtrList<TDataItem>& List)  {
  for(int i=0; i < ItemCount(); i++ )  {
    if( Items[i].StartsFrom(StartsFrom) )
      List.Add((TDataItem*)Items.Object(i));
  }
}
//..............................................................................
olxstr TDataItem::CodeString(const olxstr& Str) const  {
  if( Str.IndexOf('\"') == -1 )  return Str;
  olxstr rv(Str);
  rv.Replace('\"', DoubleQuoteCode);
  return rv;
}
//..............................................................................
olxstr TDataItem::DecodeString(const olxstr& Str) const  {
  if( Str.IndexOf(DoubleQuoteCode) == -1 )  return Str;
  olxstr Res(Str);
  Res.Replace(DoubleQuoteCode, '\"');
  return Res;
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
  for( int i=0; i < ItemCount(); i++ )  {
    if( Item(i).GetParent() != NULL && (Item(i).GetParent() != this) )  {
      Tmp = "Reffered item from ";
      Tmp << this->GetFullName() << "->" << Item(i).GetFullName();
      Log.Add( Tmp );
    }
    if( Item(i).GetParent() == NULL )  {
      Tmp = "Item with null Parent: ";
      Tmp << Item(i).GetFullName();
      Log.Add( Tmp );
    }
  }
}


