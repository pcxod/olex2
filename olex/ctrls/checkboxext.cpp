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
IMPLEMENT_CLASS(TCheckBox, wxCheckBox)

BEGIN_EVENT_TABLE(TCheckBox, wxCheckBox)
  EVT_CHECKBOX(-1, TCheckBox::ClickEvent)
  EVT_ENTER_WINDOW(TCheckBox::MouseEnterEvent)
END_EVENT_TABLE()

//..............................................................................
void TCheckBox::MouseEnterEvent(wxMouseEvent& event)  {
  event.Skip();
  SetCursor(wxCursor(wxCURSOR_HAND));
}
//..............................................................................
void TCheckBox::SetActionQueue(TActionQueue& q, const olxstr& dependMode)  {
  ActionQueue = &q;
  DependMode = dependMode;
  ActionQueue->Add(this);
}
bool TCheckBox::Execute(const IOlxObject *Sender, const IOlxObject *Data,
  TActionQueue *)
{
  if( Data && EsdlInstanceOf(*Data, TModeChange) )  {
    const TModeChange* mc = (const TModeChange*)Data;
    SetChecked(TModeRegistry::CheckMode(DependMode));
  }
  OnClick.Execute((AOlxCtrl*)this);
  if( IsChecked() )
    OnCheck.Execute((AOlxCtrl*)this);
  else
    OnUncheck.Execute((AOlxCtrl*)this);
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
