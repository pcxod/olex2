/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "msgbox.h"
#include "icons/exception.xpm"
#include "icons/exclamation.xpm"
#include "icons/information.xpm"
#include "icons/question.xpm"
#include "icons/stop.xpm"

TdlgMsgBox::TdlgMsgBox(TMainFrame* Parent, const olxstr& msg, const olxstr& title,
                       const olxstr& tickBoxMsg, long flags, bool ShowRememberCheckBox) :
  TDialog(Parent, title.u_str(), EsdlClassName(TdlgMsgBox).u_str() )
{
  AActionHandler::SetToDelete(false);
  int Border = 5;
  wxStaticText* text = new wxStaticText(this, -1, msg.u_str());
  wxStaticBitmap* sbmp = NULL;
  cbRemember = (ShowRememberCheckBox ? new wxCheckBox(this, -1, tickBoxMsg.u_str()) : NULL);
  if( (flags & wxICON_EXCLAMATION) != 0 )
    sbmp = new wxStaticBitmap(this, -1, wxBitmap(exclamation_xpm));
  else if( (flags & wxICON_HAND) != 0 )
    sbmp = new wxStaticBitmap(this, -1, wxBitmap(stop_xpm));
  else if( (flags & wxICON_ERROR) != 0 )
    sbmp = new wxStaticBitmap(this, -1, wxBitmap(exception_xpm));
  else if( (flags & wxICON_INFORMATION) != 0 )
    sbmp = new wxStaticBitmap(this, -1, wxBitmap(information_xpm));
  else if( (flags & wxICON_QUESTION) != 0 )
    sbmp = new wxStaticBitmap(this, -1, wxBitmap(question_xpm));

  wxBoxSizer *ASizer = new wxBoxSizer( wxHORIZONTAL );
  if( sbmp != NULL )
    ASizer->Add( sbmp, 0, wxALL, Border );
  ASizer->Add( text, 0, wxALL, Border );
  wxBoxSizer *BSizer = NULL;
  if( ShowRememberCheckBox )  {
    BSizer = new wxBoxSizer( wxHORIZONTAL );
    BSizer->Add( cbRemember, 0, wxALL, Border );
  }

  wxBoxSizer *ButtonsSizer = new wxBoxSizer( wxHORIZONTAL );
  TButton* btn;
  if( (flags & wxOK) != 0 )  {
    btn = new TButton(this, wxID_OK, wxT("OK"));
    btn->OnClick.Add(this);
    ButtonsSizer->Add( btn, 0, wxALL, Border);
    buttons.Add(btn);
  }
  if( (flags & wxYES) != 0 )  {
    btn = new TButton(this, wxID_YES, wxT("Yes"));
    btn->OnClick.Add(this);
    ButtonsSizer->Add( btn, 0, wxALL, Border);
    buttons.Add(btn);
  }
  if( (flags & wxNO) != 0 )  {
    btn = new TButton(this, wxID_NO, wxT("No"));
    btn->OnClick.Add(this);
    ButtonsSizer->Add( btn, 0, wxALL, Border);
    buttons.Add(btn);
  }
  if( (flags & wxCANCEL) != 0 )  {
    btn = new TButton(this, wxID_CANCEL, wxT("Cancel"));
    btn->OnClick.Add(this);
    ButtonsSizer->Add( btn, 0, wxALL, Border);
    buttons.Add(btn);
  }

  wxBoxSizer *TopSiser = new wxBoxSizer( wxVERTICAL );
  TopSiser->Add(ASizer, 0, wxALL, 5);
  if( BSizer != NULL )
    TopSiser->Add(BSizer, 0, wxALL|wxALIGN_RIGHT, 5);
  TopSiser->Add(ButtonsSizer, 0, wxALL|wxALIGN_RIGHT, 5);

  SetSizer( TopSiser );      // use the sizer for layout

  TopSiser->SetSizeHints( this );   // set size hints to honour minimum size

  Center();
//  TDialog::Init();
}
//..............................................................................
TdlgMsgBox::~TdlgMsgBox()  {
  for( size_t i=0; i < buttons.Count(); i++ )
    buttons[i]->OnClick.Clear();
}
//..............................................................................
bool TdlgMsgBox::Execute(const IEObject *Sender, const IEObject *Data,
  TActionQueue *)
{
  if( EsdlInstanceOf( *Sender, TButton) )
    EndModal( ((TButton*)(AOlxCtrl*)Sender)->GetId() );
  return true;
}
//..............................................................................
olxstr TdlgMsgBox::Execute(TMainFrame* Parent, const olxstr& msg, const olxstr& title,
                           const olxstr& tickBoxMsg, long flags, bool ShowRememberCheckBox)  {
  TdlgMsgBox* dlg = new TdlgMsgBox(Parent, msg, title, tickBoxMsg, flags, ShowRememberCheckBox);
  int mv = dlg->ShowModal();
  olxstr rv;
  if( ShowRememberCheckBox && dlg->cbRemember->IsChecked() )
    rv = 'R';
  dlg->Destroy();
  if( mv == wxID_OK )  rv << 'O';
  else if( mv == wxID_YES )  rv << 'Y';
  else if( mv == wxID_NO )  rv << 'N';
  else if( mv == wxID_CANCEL )  rv << 'C';
  return rv;
}
