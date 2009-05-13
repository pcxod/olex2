//----------------------------------------------------------------------------//
// edit text dialog dialog
// (c) Oleg V. Dolomanov, 2006
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif
#include "edit.h"
#include <wx/font.h>

TdlgEdit::TdlgEdit(TMainForm *ParentFrame, bool MultiLine):
  TDialog(ParentFrame, wxT("Edit"), wxT("dlgEdit"))

{
  int fontSize = 12, charNumber = 75;
  FParent = ParentFrame;

  int flags = 0;
  int height = 25, width = fontSize*charNumber;
  if( MultiLine )  {
    flags = wxTE_MULTILINE|wxTE_DONTWRAP;
    height = 350;
  }
  Text = new TTextEdit(this, flags);
  Text->SetSize(width, height);
  wxFont fnt(fontSize, wxMODERN, wxNORMAL, wxNORMAL);
  Text->SetFont(fnt);

  wxBoxSizer *ButtonsSizer = new wxBoxSizer( wxHORIZONTAL );
  ButtonsSizer->Add( new wxButton( this, wxID_OK, wxT("OK") ), 0, wxALL, 3);
  ButtonsSizer->Add( new wxButton( this, wxID_CANCEL, wxT("Cancel") ), 0, wxALL, 3);
  ButtonsSizer->SetDimension(0, height+1, width, 30);
  this->WI.SetWidth(width+5);
  this->WI.SetHeight(height+70);
  delete ButtonsSizer;
  Center();
  FParent->RestorePosition(this);
}
//..............................................................................
TdlgEdit::~TdlgEdit()  { }
//..............................................................................
void TdlgEdit::SetText(const olxstr& text)  {
  Text->SetValue( uiStr(text) );
}
//..............................................................................
olxstr TdlgEdit::GetText()  {
  return Text->GetValue().c_str();
}
//..............................................................................

