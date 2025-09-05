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
// Styled edit dialog
TdlgStyledEdit::TdlgStyledEdit(TMainFrame *ParentFrame, bool MultiLine):
  wxDialog(ParentFrame, -1,  wxT("Edit"), wxPoint(0, 0), wxDefaultSize, wxMAXIMIZE_BOX | wxRESIZE_BORDER | wxDEFAULT_DIALOG_STYLE, wxT("dlgEdit")), WI(this)
{
  int fontSize = 12, charNumber = 75;
  FParent = ParentFrame;

  int height = 25, width = fontSize*charNumber;
  if( MultiLine )  {
    height = 350;
  }
  Text = new wxStyledTextCtrl(this, -1, wxDefaultPosition, wxSize(width, height));
  const wxFont fnt(wxFontInfo(fontSize)
      .Family(wxFONTFAMILY_MODERN)
      .Style(wxFONTSTYLE_NORMAL)
      .Weight(wxFONTWEIGHT_NORMAL));

  Text->StyleSetFont(wxSTC_STYLE_DEFAULT, fnt);
  Text->SetMarginWidth(1, 0);

  wxBoxSizer *GlobalSizer = new wxBoxSizer(wxVERTICAL );
  GlobalSizer->Add(Text, 1, wxEXPAND | wxALL, 3);

  wxBoxSizer *ButtonsSizer = new wxBoxSizer( wxHORIZONTAL );
  ButtonsSizer->Add( new wxButton( this, wxID_OK, wxT("OK") ), 0, wxALL, 3);
  ButtonsSizer->Add( new wxButton( this, wxID_CANCEL, wxT("Cancel") ), 0, wxALL, 3);
  GlobalSizer->Add(ButtonsSizer,0, wxALL, 3);


  SetSizer(GlobalSizer);
  GlobalSizer->SetSizeHints(this);

  Center();
  FParent->RestorePosition(this);
}
//..............................................................................
TdlgStyledEdit::~TdlgStyledEdit()  {
  FParent->SavePosition(this);
}
//..............................................................................
void TdlgStyledEdit::SetText(const olxstr& text)  {
  Text->SetValue(text.u_str());
}
//..............................................................................
void TdlgStyledEdit::SetLexer(int style)
{
  Text->SetLexer(style);
  if (style == wxSTC_LEX_PYTHON)
  {
    Text->SetKeyWords(0, wxT("and as assert break class continue def del elif else except exec finally for from global if import in is lambda not or pass print raise return try while with yield"));
    // Comments
    Text->StyleSetSpec(wxSTC_P_COMMENTLINE, "fore:#007F00");
    Text->StyleSetSpec(wxSTC_P_COMMENTBLOCK, "fore:#007F00");
    // Numbers
    Text->StyleSetSpec(wxSTC_P_NUMBER, "fore:#007F7F");
    // Strings
    Text->StyleSetSpec(wxSTC_P_STRING, "fore:#7F007F");
    Text->StyleSetSpec(wxSTC_P_CHARACTER, "fore:#7F007F");
    // Keywords
    Text->StyleSetSpec(wxSTC_P_WORD, "fore:#00007F,bold");
    // Triple quotes
    Text->StyleSetSpec(wxSTC_P_TRIPLE, "fore:#7F0000");
    Text->StyleSetSpec(wxSTC_P_TRIPLEDOUBLE, "fore:#7F0000");
    // Class and function definitions
    Text->StyleSetSpec(wxSTC_P_CLASSNAME, "fore:#0000FF,bold");
    Text->StyleSetSpec(wxSTC_P_DEFNAME, "fore:#007F7F,bold");
    // Operators
    Text->StyleSetSpec(wxSTC_P_OPERATOR, "bold");
    Text->StyleSetSpec(wxSTC_P_STRINGEOL, "fore:#000000,back:#E0C0E0,eolfilled");
  }
  else if (style == wxSTC_LEX_PROPERTIES) // Use for TOML
  {
    // Basic styling for TOML-like syntax
    Text->StyleSetSpec(wxSTC_PROPS_DEFAULT, "fore:#000000");
    Text->StyleSetSpec(wxSTC_PROPS_COMMENT, "fore:#007F00");
    Text->StyleSetSpec(wxSTC_PROPS_SECTION, "fore:#0000FF,bold"); // [section]
    Text->StyleSetSpec(wxSTC_PROPS_ASSIGNMENT, "fore:#FF0000");   // =
    Text->StyleSetSpec(wxSTC_PROPS_DEFVAL, "fore:#7F007F");       // values
    Text->StyleSetSpec(wxSTC_PROPS_KEY, "fore:#000080,bold");     // keys
  }
}
//..............................................................................
olxstr TdlgStyledEdit::GetText()  {  return Text->GetValue();  }
//..............................................................................
