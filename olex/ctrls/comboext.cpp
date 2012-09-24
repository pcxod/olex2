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
#ifndef __WIN32__
IMPLEMENT_CLASS(TComboBox, wxComboBox)
#else
IMPLEMENT_CLASS(TComboBox, wxOwnerDrawnComboBox)
#endif

#ifndef __WIN32__
BEGIN_EVENT_TABLE(TComboBox, wxComboBox)
#else
BEGIN_EVENT_TABLE(TComboBox, wxOwnerDrawnComboBox)
#endif
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
void TComboBox::SetText(const olxstr& T)  {
  StrValue = T;
  SetValue(StrValue.u_str());
#ifdef __WIN32__
  if( GetTextCtrl() != NULL )
    GetTextCtrl()->SetInsertionPoint(0);
#endif		
}
//..............................................................................
void TComboBox::Clear() {
  StrValue.SetLength(0);
  for( unsigned int i=0; i < GetCount(); i++ )  {
    TDataObj* d_o = (TDataObj*)GetClientData(i);
    if( d_o != NULL )  {
      if( d_o->Delete )
        delete (IEObject*)d_o->Data;
      delete d_o;
    }
  }
#ifndef __WIN32__
  wxComboBox::Clear();
#else	
  wxOwnerDrawnComboBox::Clear();
#endif	
}
//..............................................................................
void TComboBox::_AddObject( const olxstr &Item, IEObject* Data, bool Delete)  {
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
void TComboBox::AddObject( const olxstr &Item, IEObject* Data)  {
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
void TComboBox::ChangeEvent(wxCommandEvent& event)  {
  StrValue = GetValue();
  if( !Data.IsEmpty() )
    TOlxVars::SetVar(Data, GetText());
  OnChange.Execute(this);
  event.Skip();
}
//..............................................................................
void TComboBox::LeaveEvent(wxFocusEvent& event)  {
  olxstr v = GetText();
  bool changed = (v != StrValue);
  if( changed )  {
    OnChange.Execute(this);
    StrValue = v;
  }
  OnLeave.Execute(this);
  event.Skip();
}
//..............................................................................
void TComboBox::EnterEvent(wxFocusEvent& event)  {
  OnEnter.Execute(this);
  event.Skip();
}
//..............................................................................
const IEObject* TComboBox::GetObject(int i)  {
  TDataObj* res = (TDataObj*)GetClientData(i);
  return (res != NULL && !res->Delete) ? res->Data : NULL;
}
//..............................................................................
olxstr TComboBox::GetText() const {
  olxstr cv = GetValue();
  for( unsigned int i=0; i < GetCount(); i++ )  {
    if( cv == GetString(i) )  {
      TDataObj* res = (TDataObj*)GetClientData(i);
      return (res != NULL && res->Delete) ? res->Data->ToString() : cv;
    }
  }
  return cv;
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
void TComboBox::HandleOnLeave()  {
  olxstr v = GetValue();
  bool changed = (v != StrValue);
  if( changed )  {
    OnChange.Execute(this);
    StrValue = v;
  }
  OnLeave.Execute(this);
}
//..............................................................................
void TComboBox::HandleOnEnter()  {
  OnEnter.Execute(this);
}
//..............................................................................
#ifdef __WIN32__
void TComboBox::OnDrawItem( wxDC& dc, const wxRect& rect, int item,
  int flags ) const
{
  wxOwnerDrawnComboBox::OnDrawItem(dc, rect, item, flags);
  return;
}
//..............................................................................
wxCoord TComboBox::OnMeasureItem( size_t item ) const {
  return this->GetCharHeight();
}
//..............................................................................
wxCoord TComboBox::OnMeasureItemWidth( size_t item ) const {
  return wxOwnerDrawnComboBox::OnMeasureItem(item);
}
//..............................................................................
void TComboBox::OnDrawBg(wxDC& dc, const wxRect& rect, int item,
  int flags) const
{
  wxOwnerDrawnComboBox::OnDrawBackground(dc, rect, item, flags);
}
#endif
