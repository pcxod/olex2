/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "groupobj.h"
UseEsdlNamespace()

AGOProperties& AGroupObject::SetProperties( const AGOProperties& C)  {
  return *(Properties = Parent.NewProps(*this, Properties, C));
}
//----------------------------------------------------------------------------//
// TGObjectGroup
//----------------------------------------------------------------------------//
TObjectGroup::TObjectGroup()
  : Objects(olx_reserve(0, 16)),
    Props(olx_reserve(0, 16))
{}
//..............................................................................
void TObjectGroup::Clear()  {
  Objects.Clear();
  Props.Clear();
}
//..............................................................................
void TObjectGroup::RemoveObjectsByTag(int Tag) {
  for (size_t i = 0; i < ObjectCount(); i++) {
    if (Objects[i]->GetTag() == Tag) {
      AGOProperties& P = Objects[i]->GetProperties();
      if (P.ObjectCount() == 1)
        P.SetObjectGroupId(InvalidIndex); // mark to remove
      P.RemoveObject(Objects[i]);
      delete Objects[i];
      Objects[i] = 0;
    }
  }
  Objects.Pack();
  for (size_t i = 0; i < Props.Count(); i++) {
    if (Props[i]->GetObjectGroupId() == InvalidIndex) {
      delete Props[i];
      Props[i] = 0;
    }
  }
  Props.Pack();
}
//..............................................................................
AGOProperties* TObjectGroup::FindProps(const AGOProperties& C) {
  for (size_t i = 0; i < Props.Count(); i++)
    if (C == *Props[i]) {
      return Props[i];
    }
  return 0;
}
//..............................................................................
AGOProperties* TObjectGroup::NewProps(AGroupObject& Sender, AGOProperties* OldProps,
  const AGOProperties& P)
{
  if (OldProps != 0) {
    if (*OldProps == P) {
      return OldProps;
    }
  }

  AGOProperties* Prop = FindProps(P);
  if (Prop == 0) {
    Prop = Sender.NewProperties();
    *Prop = P;
    Prop->AddObject(Sender);
    Props.Add(Prop);
    if (OldProps != 0) {
      OldProps->RemoveObject(Sender);
      // Id = 0 for default properties
      if (OldProps->ObjectCount() == 0 && OldProps->GetObjectGroupId() != 0) {
        Props.Remove(OldProps);
        delete OldProps;
      }
    }
    Prop->SetObjectGroupId(Props.Count() - 1);
    return Prop;
  }
  else {
    Prop->AddObject(Sender);
    if (OldProps) {
      OldProps->RemoveObject(Sender);
      // Id = 0 for default properties
      if (OldProps->ObjectCount() == 0 && OldProps->GetObjectGroupId() != 0) {
        Props.Remove(OldProps);
        delete OldProps;
      }
    }
    return Prop;
  }
}
//..............................................................................
