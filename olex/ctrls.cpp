/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "ctrls.h"
#include "integration.h"

wxDEFINE_EVENT(OLX_COMMAND_EVT, olxCommandEvent);
//..............................................................................
//..............................................................................
IMPLEMENT_CLASS(TDialog, wxDialog)
IMPLEMENT_CLASS(TTimer, wxTimer)
//..............................................................................
TDialog::TDialog(TMainFrame *Parent, const wxString &Title,
  const wxString &ClassName, const wxPoint& position, const wxSize& size,
  int style)
  : wxDialog(Parent, -1,  Title, position, size, style, ClassName),
    AOlxCtrl(this),
    OnResize(Actions.New("OnResize")),
    Parent(Parent)
{
  manage_parent = false;
  if (Parent != NULL)
    Parent->RestorePosition(this);
  Bind(wxEVT_SIZE, &TDialog::OnSizeEvt, this);
}
//..............................................................................
TDialog::TDialog(wxWindow *Parent, const wxString &Title,
  const wxString &ClassName, const wxPoint& position, const wxSize& size,
  int style)
  : wxDialog(Parent, -1,  Title, position, size, style, ClassName),
    AOlxCtrl(this),
    OnResize(Actions.New("OnResize")),
    Parent(NULL)
{
  manage_parent = false;
}
//..............................................................................
TDialog::~TDialog()  {
  if( Parent != NULL )
    Parent->SavePosition(this);
}
//..............................................................................
void TDialog::OnSizeEvt(wxSizeEvent& event)  {
  event.Skip();
  OnResize.Execute(this, NULL);
}
//..............................................................................
bool TDialog::Show(bool show) {
  if (!show && manage_parent) {
    manage_parent = false;
    GetParent()->Enable(true);
  }
  return wxDialog::Show(show);
}
//..............................................................................
int TDialog::ShowModalEx(bool manage_parent) {
  if (manage_parent && GetParent() != NULL) {
    this->manage_parent = true;
    GetParent()->Enable(false);
  }
  return wxDialog::ShowModal();
}
//..............................................................................
//..............................................................................
//..............................................................................
bool olxCommandAction::Run() {
  TStrList toks(cmd, ">>");
  olex2::IOlex2Processor *ip = olex2::IOlex2Processor::GetInstance();
  for (size_t i = 0; i < toks.Count(); i++) {
    if (!ip->processMacro(toks[i])) {
      return false;
    }
  }
  return true;
}

