//----------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "groupobj.h"

UseEsdlNamespace()
//----------------------------------------------------------------------------//
// TGOproperties
//----------------------------------------------------------------------------//
AGOProperties::AGOProperties()  {
  Id = -1;
  Objects.SetIncrement( 512 );
}
//..............................................................................
AGOProperties::~AGOProperties()  {  }
//..............................................................................
/*
void TGOProperties::ReplaceObjects(TEList *CurObj, TEList *NewObj)
{
  int i;
  TGroupObject *GO;
  for( i=0; i < ObjectCount(); i++ )
  {
    GO = Object(i);
    if( GO->Tag() >=0 ) // replace objects
    {
      FObjects->Item(GO->Tag()) = NewObj->Item(GO->Tag());
    }
  }
}
*/
//----------------------------------------------------------------------------//
// TGroupObject
//----------------------------------------------------------------------------//
AGroupObject::AGroupObject(TObjectGroup *Group)  {
  FParent = Group;
  FProperties = NULL;
}
//..............................................................................
AGOProperties * AGroupObject::SetProperties( const AGOProperties *C)  {
  FProperties = FParent->NewProps(this, FProperties, C);
  return FProperties;
}
//----------------------------------------------------------------------------//
// TGObjectGroup
//----------------------------------------------------------------------------//
TObjectGroup::TObjectGroup()  {
  Objects.SetIncrement( 512 );
  Props.SetIncrement( 512 );
}
//..............................................................................
TObjectGroup::~TObjectGroup() {   }
//..............................................................................
void TObjectGroup::Clear()  {
  Objects.Clear();
  Props.Clear();
}
//..............................................................................
/*
void TObjectGroup::ReplaceObjects(TEList *CurObj, TEList *NewObj )
{
  if( CurObj->Count() != NewObj->Count() )
    BasicApp->Log->Exception("TObjectGroup:: lists count does not much!", true);
  AGroupObject *GO;
  AGOProperties *P;
  int i;
  for( i=0; i < ObjectCount(); i++ )  Object(i)->Tag(-1);
  
  for( i=0; i < CurObj->Count(); i++ )
  {
    GO = (AGroupObject*)CurObj->Item(i);
    GO->Tag(i);
//    GO = (AGroupObject*)NewObj->Item(i);
//    GO->Tag(i);
  }
  for( i=0; i < ObjectCount(); i++ )
  {
    GO = Object(i);
    if( GO->Tag() >=0 ) // replace objects
    {
      FObjects->Item(GO->Tag()) = NewObj->Item(GO->Tag());
    }
  }
  for( i=0; i < PropCount(); i++ )
  {
    P = Properties(i);
    P->ReplaceObjects(CurObj, NewObj);
  }
  FProps->Pack();
}
*/
//..............................................................................
void TObjectGroup::RemoveObjectsByTag(int Tag)  {
  for( int i=0; i < ObjectCount(); i++ )  {
    if( Objects[i]->GetTag() == Tag )  {
      AGOProperties* P = const_cast<AGOProperties*>(Objects[i]->GetProperties());
      if( P->ObjectCount() == 1 )   P->SetId(-1); // mark to remove
      P->RemoveObject( Objects[i] );
      delete Objects[i];
      Objects[i] = NULL;
    }
  }
  Objects.Pack();
  for( int i=0; i < Props.Count(); i++ )  {
    if( Props[i]->GetId() == -1 )  {
      delete Props[i];
      Props[i] = NULL;
    }
  }
  Props.Pack();
}
//..............................................................................
AGOProperties *TObjectGroup::GetProps( const AGOProperties &C)  {
  for( int i = 0; i < Props.Count(); i++ )
    if( C == *Props[i] )
      return Props[i];
  return NULL;
}
//..............................................................................
AGOProperties * TObjectGroup::NewProps(AGroupObject *Sender, AGOProperties *OldProps, const AGOProperties *P)  {
  if( OldProps != NULL )
    if( *OldProps == *P )
      return OldProps;

  AGOProperties *Prop = GetProps(*P);
  if( Prop == NULL )  {
    Prop = Sender->NewProperties();
    *Prop = *P;
    Prop->AddObject(Sender);
    Props.Add(Prop);
    if( OldProps != NULL )  {
      OldProps->RemoveObject(Sender);
      if( !OldProps->ObjectCount() && OldProps->GetId() )  {  // Id = 0 for default properties
        Props.Remove(OldProps);
        delete OldProps;
      }
    }
    Prop->SetId( Props.Count()-1 );
    return Prop;
  }
  else  {
    Prop->AddObject(Sender);
    if( OldProps )  {
      OldProps->RemoveObject(Sender);
      if( !OldProps->ObjectCount() && OldProps->GetId() )  {  // Id = 0 for default properties
        Props.Remove(OldProps);
        delete OldProps;
      }
    }
    return Prop;
  }
}
//..............................................................................

