/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "checkboxext.h"
#include "frameext.h"
#include "olxstate.h"
#include "olxvar.h"

using namespace ctrl_ext;

//..............................................................................
TCheckBox::TCheckBox(wxWindow *Parent, wxWindowID id, const wxString& label,
  const wxPoint& pos, const wxSize& size, long style)
: AOlxCtrl(this),
  OnClick(AOlxCtrl::ActionQueue::New(Actions, evt_on_click_id)),
  OnCheck(AOlxCtrl::ActionQueue::New(Actions, evt_on_check_id)),
  OnUncheck(AOlxCtrl::ActionQueue::New(Actions, evt_on_uncheck_id)),
  ActionQueue(NULL)
{
  wxCheckBox::Create(Parent, id, label, pos, size, style);
  Bind(wxEVT_CHECKBOX, &TCheckBox::ClickEvent, this);
  Bind(wxEVT_ENTER_WINDOW, &TCheckBox::MouseEnterEvent, this);
  SetToDelete(false);
}
//..............................................................................
void TCheckBox::MouseEnterEvent(wxMouseEvent& event)  {
  event.Skip();
  SetCursor(wxCursor(wxCURSOR_HAND));
}
//..............................................................................
void TCheckBox::SetActionQueue(TActionQueue& q, const olxstr& dependMode)  {
  if (ActionQueue != 0) {
    ActionQueue->Remove(this);
  }
  ActionQueue = &q;
  DependMode = dependMode;
  ActionQueue->Add(this);
}
bool TCheckBox::Execute(const IOlxObject *Sender, const IOlxObject *Data,
  TActionQueue *)
{
  if (Data && EsdlInstanceOf(*Data, TModeChange)) {
    const TModeChange* mc = (const TModeChange*)Data;
    SetChecked(mc->GetStatus());
  }
  else if (Data && EsdlInstanceOf(*Data, TStateChange)) {
    const TStateChange* sc = (const TStateChange*)Data;
    SetChecked(sc->GetStatus());
  }
  else {
    OnClick.Execute((AOlxCtrl*)this);
    if (IsChecked())
      OnCheck.Execute((AOlxCtrl*)this);
    else
      OnUncheck.Execute((AOlxCtrl*)this);
  }
  return true;
}
//..............................................................................
void TCheckBox::ClickEvent(wxCommandEvent &event)  {
  event.Skip();
  OnClick.Execute((AOlxCtrl*)this);
  if( IsChecked() )
    OnCheck.Execute((AOlxCtrl*)this);
  else
    OnUncheck.Execute((AOlxCtrl*)this);
}
