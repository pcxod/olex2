/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "comboext.h"
#include "frameext.h"
#include "olxvar.h"

using namespace ctrl_ext;
IMPLEMENT_CLASS(TComboBox, wxComboBox)

BEGIN_EVENT_TABLE(TComboBox, wxComboBox)
  EVT_COMBOBOX(-1, TComboBox::ChangeEvent)
  EVT_TEXT_ENTER(-1, TComboBox::EnterPressedEvent)
  EVT_KILL_FOCUS(TComboBox::LeaveEvent)
  EVT_SET_FOCUS(TComboBox::EnterEvent)
END_EVENT_TABLE()

TComboBox::~TComboBox()  {
  for( unsigned int i=0; i < GetCount(); i++ )  {
    TDataObj* d_o = (TDataObj*)GetClientData(i);
    if( d_o != NULL )  {
      if( d_o->Delete )
        delete (IEObject*)d_o->Data;
      delete d_o;
    }
  }
}
//..............................................................................
void TComboBox::SetText(const olxstr& T) {
  olxstr actual_val = T;
  bool found = false;
  for (unsigned int i=0; i < GetCount(); i++ )  {
    TDataObj* res = (TDataObj*)GetClientData(i);
    if (res == NULL || !res->Delete) {
      continue;
    }
    if (T == GetString(i)) {
      found = true;
    }
    olxstr sv = res->Data->ToString();
    if (sv == T) {
      actual_val = GetString(i);
      break;
    }
  }
  if (!IsReadOnly() || found) {
    StrValue = actual_val;
    SetValue(StrValue.u_str());
  }
  else {
    if (GetCount() != 0) {
      wxComboBox::SetSelection(0);
    }
  }
}
//..............................................................................
void TComboBox::Clear() {
  StrValue.SetLength(0);
  if (GetCount() == 0) return;
  for( unsigned int i=0; i < GetCount(); i++ )  {
    TDataObj* d_o = (TDataObj*)GetClientData(i);
    if( d_o != NULL )  {
      if( d_o->Delete )
        delete (IEObject*)d_o->Data;
      delete d_o;
    }
  }
#if defined(__MAC__) && wxCHECK_VERSION(2,9,0)
  wxComboBox::DoClear();
#else
  wxComboBox::Clear();
#endif
}
//..............................................................................
void TComboBox::_AddObject(const olxstr &Item, IEObject* Data, bool Delete)  {
  Append(Item.u_str());
  if( Data != NULL )  {
    TDataObj* d_o = new TDataObj;
    d_o->Data = Data;
    d_o->Delete = Delete;
    SetClientData(GetCount()-1, d_o);
  }
  else
    SetClientData(GetCount()-1, NULL);
}
//..............................................................................
void TComboBox::AddObject(const olxstr &Item, IEObject* Data)  {
  _AddObject(Item, Data, false);
}
//..............................................................................
void TComboBox::EnterPressedEvent(wxCommandEvent &event)  {
  if( !Data.IsEmpty() )
    TOlxVars::SetVar(Data, GetText());
  OnReturn.Execute(this);
  event.Skip();
}
//..............................................................................
void TComboBox::ChangeEvent(wxCommandEvent& event) {
  olxstr v = GetValue();
  if (IsOnChangeAlways() || v != StrValue) {
    StrValue = v;
    if (!Data.IsEmpty())
      TOlxVars::SetVar(Data, GetText());
    OnChange.Execute(this);
  }
  event.Skip();
}
//..............................................................................
void TComboBox::LeaveEvent(wxFocusEvent& event)  {
  if (--entered_counter == 0)
    HandleOnLeave();
  event.Skip();
}
//..............................................................................
void TComboBox::EnterEvent(wxFocusEvent& event)  {
  if (++entered_counter == 1)
    HandleOnEnter();
  event.Skip();
}
//..............................................................................
const IEObject* TComboBox::GetObject(int i) const {
  TDataObj* res = (TDataObj*)GetClientData(i);
  return (res != NULL && !res->Delete) ? res->Data : NULL;
}
//..............................................................................
olxstr TComboBox::GetText() const {
  if (!HasValue())
    return EmptyString();
  int sel = GetSelection();
  if (sel == -1) {
    return IsReadOnly() ? EmptyString() : olxstr(GetValue());
  }
  TDataObj* res = (TDataObj*)GetClientData(sel);
  return (res == NULL || !res->Delete) ? olxstr(GetValue())
    : res->Data->ToString();
}
//..............................................................................
olxstr TComboBox::ItemsToString(const olxstr &sep)  {
  olxstr rv;
  for( unsigned int i=0; i < GetCount(); i++ )  {
    rv << GetString(i);
    TDataObj* res = (TDataObj*)GetClientData(i);
    if( res != NULL && res->Delete )
      rv << "<-" << res->Data->ToString();
    if( (i+1) < GetCount() )
      rv << sep;
  }
  return rv;
}
//..............................................................................
void TComboBox::AddItems(const TStrList& EL) {
  for( size_t i=0; i < EL.Count(); i++ )  {
    size_t ind = EL[i].IndexOf( "<-" );
    if(  ind != InvalidIndex )  {
      olxstr tmp = EL[i].SubStringFrom(ind + 2);
      _AddObject(EL[i].SubStringTo(ind), tmp.Replicate(), true);
    }
    else
      _AddObject(EL[i], NULL, false);
  }
}
//..............................................................................
void TComboBox::HandleOnLeave() {
  if (OnLeave.IsEnabled()) {
    olxstr v = GetValue();
    bool changed = (v != StrValue);
    if (changed) {
      StrValue = v;
      OnChange.Execute(this);
    }
    OnLeave.Execute(this);
    OnLeave.SetEnabled(false);
    OnEnter.SetEnabled(true);
  }
}
//..............................................................................
void TComboBox::HandleOnEnter()  {
  if (OnEnter.IsEnabled()) {
    OnEnter.Execute(this);
    OnEnter.SetEnabled(false);
    OnLeave.SetEnabled(true);
  }
}
//..............................................................................
