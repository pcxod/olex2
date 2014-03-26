/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "dgrad.h"
#include "wx/colordlg.h"
#include "glbackground.h"
#include "gxapp.h"

// gradient colours properties dialog
BEGIN_EVENT_TABLE(TdlgGradient, TDialog)
  EVT_BUTTON(wxID_OK, TdlgGradient::OnOK)
END_EVENT_TABLE()

TdlgGradient::TdlgGradient(TMainFrame *ParentFrame):
  TDialog(ParentFrame, wxT("Gradient"), EsdlClassName(TdlgGradient).u_str())
{
  AActionHandler::SetToDelete(false);
  short Border = 3;

  wxStaticText *stcA = new wxStaticText(this, -1, wxT("Bottom left"), wxDefaultPosition);
  tcA = new TTextEdit(this);  tcA->SetReadOnly(true);  tcA->OnClick.Add(this);
  wxStaticText *stcB = new wxStaticText(this, -1, wxT("Bottom right"), wxDefaultPosition);
  tcB = new TTextEdit(this);  tcB->SetReadOnly(true);  tcB->OnClick.Add(this);
  wxStaticText *stcC = new wxStaticText(this, -1, wxT("Top right"), wxDefaultPosition);
  tcC = new TTextEdit(this);  tcC->SetReadOnly(true);  tcC->OnClick.Add(this);
  wxStaticText *stcD = new wxStaticText(this, -1, wxT("Top Left"), wxDefaultPosition);
  tcD = new TTextEdit(this);  tcD->SetReadOnly(true);  tcD->OnClick.Add(this);

  wxFlexGridSizer *GridSizer = new wxFlexGridSizer(2, 4, Border, Border);
  GridSizer->Add( stcD, 0, wxALIGN_CENTER_VERTICAL | wxRight, 2 );
  GridSizer->Add( tcD, 1, wxEXPAND | wxALL, 0 );
  GridSizer->Add( stcC, 0, wxALIGN_CENTER_VERTICAL | wxRight, 2 );
  GridSizer->Add( tcC, 1, wxEXPAND | wxALL, 0 );

  GridSizer->Add( stcA, 0, wxALIGN_CENTER_VERTICAL | wxRight, 2 );
  GridSizer->Add( tcA, 1, wxEXPAND | wxALL, 0 );
  GridSizer->Add( stcB, 0, wxALIGN_CENTER_VERTICAL | wxRight, 2 );
  GridSizer->Add( tcB, 1, wxEXPAND | wxALL, 0 );

  GridSizer->AddGrowableCol(1);
  GridSizer->AddGrowableCol(3);

  wxBoxSizer *ButtonsSizer = new wxBoxSizer( wxHORIZONTAL );
  ButtonsSizer->Add( new wxButton( this, wxID_OK, wxT("OK") ), 0, wxALL, Border);
  ButtonsSizer->Add( new wxButton( this, wxID_CANCEL, wxT("Cancel") ), 0, wxALL, Border);
//  ButtonsSizer->Add( new wxButton( this, wxID_HELP, wxT("Help") ),     0, wxALL, Border );

  wxBoxSizer *TopSiser = new wxBoxSizer( wxVERTICAL );
  TopSiser->Add(new wxStaticText(this, -1, wxT("Gradient colour of the background of the structure window"), wxDefaultPosition), 0, wxEXPAND | wxALL, 10);
  TopSiser->Add(GridSizer, 0, wxEXPAND | wxALL, 5);
  TopSiser->Add(ButtonsSizer, 0, wxALL, 10);

  SetSizer( TopSiser );      // use the sizer for layout
  TopSiser->SetSizeHints( this );   // set size hints to honour minimum size

  Center();
  Init();
}
//..............................................................................
TdlgGradient::~TdlgGradient()  {
  tcA->OnClick.Clear();
  tcB->OnClick.Clear();
  tcC->OnClick.Clear();
  tcD->OnClick.Clear();
}
//..............................................................................
bool TdlgGradient::Execute(const IEObject *Sender, const IEObject *Data,
  TActionQueue *)
{
  if( EsdlInstanceOf( *Sender, TTextEdit) )  {
    wxColourDialog *CD = new wxColourDialog(this);
    wxColor wc = ((TTextEdit*)Sender)->GetBackgroundColour();
    CD->GetColourData().SetColour(wc);
    if( CD->ShowModal() == wxID_OK )  {
      wc = CD->GetColourData().GetColour();
      ((TTextEdit*)Sender)->WI.SetColor(OLX_RGB(wc.Red(), wc.Green(), wc.Blue()) );
    }
    delete CD;
  }
  return true;
}
//..............................................................................
void TdlgGradient::Init()  {
  tcA->WI.SetColor(TGXApp::GetInstance().GetRender().Background()->LT().GetRGB());
  tcB->WI.SetColor(TGXApp::GetInstance().GetRender().Background()->RT().GetRGB());
  tcC->WI.SetColor(TGXApp::GetInstance().GetRender().Background()->RB().GetRGB());
  tcD->WI.SetColor(TGXApp::GetInstance().GetRender().Background()->LB().GetRGB());
}
//..............................................................................
void TdlgGradient::OnOK(wxCommandEvent& event)  {
  TGlOption opt;
  opt = tcA->WI.GetColor();  TGXApp::GetInstance().GetRender().Background()->LT(opt);
  opt = tcB->WI.GetColor();  TGXApp::GetInstance().GetRender().Background()->RT(opt);
  opt = tcC->WI.GetColor();  TGXApp::GetInstance().GetRender().Background()->RB(opt);
  opt = tcD->WI.GetColor();  TGXApp::GetInstance().GetRender().Background()->LB(opt);
  EndModal(wxID_OK);
}
//..............................................................................
