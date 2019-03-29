/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "edit.h"
#include <wx/font.h>

// edit text dialog dialog
TdlgEdit::TdlgEdit(TMainFrame *ParentFrame, bool MultiLine):
  //TDialog(ParentFrame, -1, wxT("Edit"), wxT("dlgEdit"))
  wxDialog(ParentFrame, -1,  wxT("Edit"), wxPoint(0, 0), wxDefaultSize, wxMAXIMIZE_BOX | wxRESIZE_BORDER | wxDEFAULT_DIALOG_STYLE, wxT("dlgEdit")), WI(this)
{
  int fontSize = 12, charNumber = 75;
  FParent = ParentFrame;

  int flags = 0;
  int height = 25, width = fontSize*charNumber;
  if( MultiLine )  {
    flags = wxTE_MULTILINE|wxTE_DONTWRAP;
    height = 350;
  }
  Text = new wxTextCtrl(this, -1, wxT(""), wxDefaultPosition, wxSize(width, height), flags);
  //Text->SetSize(width, height);
  wxFont fnt(fontSize, wxMODERN, wxNORMAL, wxNORMAL);
  Text->SetFont(fnt);

  wxBoxSizer *GlobalSizer = new wxBoxSizer(wxVERTICAL );
  GlobalSizer->Add(Text, 1, wxEXPAND | wxALL, 3);

  wxBoxSizer *ButtonsSizer = new wxBoxSizer( wxHORIZONTAL );
  ButtonsSizer->Add( new wxButton( this, wxID_OK, wxT("OK") ), 0, wxALL, 3);
  ButtonsSizer->Add( new wxButton( this, wxID_CANCEL, wxT("Cancel") ), 0, wxALL, 3);
  //ButtonsSizer->SetDimension(0, height+1, width, 30);
  //this->WI.SetWidth(width+5);
  //this->WI.SetHeight(height+70);
  GlobalSizer->Add(ButtonsSizer,0, wxALL, 3);


  SetSizer(GlobalSizer);
  GlobalSizer->SetSizeHints(this);

  //delete ButtonsSizer;
  Center();
  FParent->RestorePosition(this);
}
//..............................................................................
TdlgEdit::~TdlgEdit()  {
  FParent->SavePosition(this);
}
//..............................................................................
void TdlgEdit::SetText(const olxstr& text)  {
  Text->SetValue(text.u_str());
}
//..............................................................................
olxstr TdlgEdit::GetText()  {  return Text->GetValue();  }
//..............................................................................
