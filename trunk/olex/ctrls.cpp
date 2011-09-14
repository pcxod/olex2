/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "ctrls.h"

IMPLEMENT_CLASS(TDialog, wxDialog)
IMPLEMENT_CLASS(TTimer, wxTimer)

BEGIN_EVENT_TABLE(TDialog, wxDialog)
  EVT_SIZE(TDialog::OnSizeEvt)
END_EVENT_TABLE()

TDialog::TDialog(TMainFrame *Parent, const wxString &Title, const wxString &ClassName,
      const wxPoint& position, const wxSize& size, int style) :
  wxDialog(Parent, -1,  Title, position, size, style, ClassName),
  AOlxCtrl(this),
  OnResize(Actions.New("OnResIze")),
  Parent(Parent)
{
  if( Parent != NULL )
    Parent->RestorePosition(this);
}
TDialog::~TDialog()  {
  if( Parent != NULL )
    Parent->SavePosition(this);
}
void TDialog::OnSizeEvt(wxSizeEvent& event)  {
  event.Skip();
  OnResize.Execute(this, NULL);
}
