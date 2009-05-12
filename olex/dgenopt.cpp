//----------------------------------------------------------------------------//
// DlgGenerate implementation
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif
#include "dgenopt.h"
#include "mainform.h"
//----------------------------------------------------------------------------//
// TdlgGenerate function bodies
//----------------------------------------------------------------------------//
//enum { cbAChange, cbBChange, cbCChange };
BEGIN_EVENT_TABLE(TdlgGenerate, TDialog)
  EVT_BUTTON(wxID_OK, TdlgGenerate::OnOK)
END_EVENT_TABLE()

//..............................................................................
TdlgGenerate::TdlgGenerate(TMainFrame *ParentFrame)
:TDialog(ParentFrame, wxT("Generation options"), wxT("dlgGenerate"))

{
  FParent = ParentFrame;
  AActionHandler::SetToDelete(false);
  FAFrom = FBFrom = FCFrom = -1;
  FATo = FBTo = FCTo = 1;
  short Border = 3, i;
  TStrList EL;
  for( i=1; i < 9; i++ )    EL.Add(i);

  stAFrom = new wxStaticText(this, -1, wxT("A from"), wxDefaultPosition);
  stBFrom = new wxStaticText(this, -1, wxT("B from"), wxDefaultPosition);
  stCFrom = new wxStaticText(this, -1, wxT("C from"), wxDefaultPosition);
  stATo   = new wxStaticText(this, -1, wxT("to"), wxDefaultPosition);
  stBTo   = new wxStaticText(this, -1, wxT("to"), wxDefaultPosition);
  stCTo   = new wxStaticText(this, -1, wxT("to"), wxDefaultPosition);

  tcAFrom = new wxTextCtrl(this, -1, wxT("-1"), wxDefaultPosition);
  tcBFrom = new wxTextCtrl(this, -1, wxT("-1"), wxDefaultPosition);
  tcCFrom = new wxTextCtrl(this, -1, wxT("-1"), wxDefaultPosition);
  tcATo   = new wxTextCtrl(this, -1, wxT("1"), wxDefaultPosition);
  tcBTo   = new wxTextCtrl(this, -1, wxT("1"), wxDefaultPosition);
  tcCTo   = new wxTextCtrl(this, -1, wxT("1"), wxDefaultPosition);

  cbA = new TComboBox(this);  cbA->SetText("2"); cbA->WI.SetWidth(46); cbA->WI.SetHeight(21);  cbA->AddItems(EL);
  cbB = new TComboBox(this);  cbB->SetText("2"); cbB->WI.SetWidth(46); cbB->WI.SetHeight(21);  cbB->AddItems(EL);
  cbC = new TComboBox(this);  cbC->SetText("2"); cbC->WI.SetWidth(46); cbC->WI.SetHeight(21);  cbC->AddItems(EL);
  cbA->OnChange->Add(this);
  cbB->OnChange->Add(this);
  cbC->OnChange->Add(this);

  wxBoxSizer *TopSizer = new wxBoxSizer(wxVERTICAL );

  wxFlexGridSizer *GridSizer = new wxFlexGridSizer(3, 5, 0, 0);
  GridSizer->Add( stAFrom, 0, wxALL, Border );
  GridSizer->Add( tcAFrom, 0, wxALL, Border );
  GridSizer->Add( stATo,   0, wxALL, Border );
  GridSizer->Add( tcATo, 0, wxALL, Border );
  GridSizer->Add( cbA, 0, wxALL, Border );

  GridSizer->Add( stBFrom, 0, wxALL, Border );
  GridSizer->Add( tcBFrom, 0, wxALL, Border );
  GridSizer->Add( stBTo,   0, wxALL, Border );
  GridSizer->Add( tcBTo, 0, wxALL, Border );
  GridSizer->Add( cbB, 0, wxALL, Border );

  GridSizer->Add( stCFrom, 0, wxALL, Border );
  GridSizer->Add( tcCFrom, 0, wxALL, Border );
  GridSizer->Add( stCTo,   0, wxALL, Border );
  GridSizer->Add( tcCTo, 0, wxALL, Border );
  GridSizer->Add( cbC, 0, wxALL, Border );
  GridSizer->SetSizeHints(this);
  
  wxBoxSizer *ButtonsSizer = new wxBoxSizer( wxHORIZONTAL );

  ButtonsSizer->Add( new wxButton( this, wxID_OK, wxT("OK") ), 0, wxALL, Border);
  ButtonsSizer->Add( new wxButton( this, wxID_CANCEL, wxT("Cancel") ), 0, wxALL, Border);
  ButtonsSizer->Add( new wxButton( this, wxID_HELP, wxT("Help") ),     0, wxALL, Border );
  ButtonsSizer->SetSizeHints(this);
  
  TopSizer->Add(GridSizer, 0, wxALL, 5);
  TopSizer->Add(ButtonsSizer, 0, wxALL, 5);
  //TopSiser->Add(CSizer, 0, wxALL, 5);
  //TopSiser->Add(ButtonsSizer, 0, wxALL, 10);
  SetSizer( TopSizer );      // use the sizer for layout

  TopSizer->SetSizeHints( this );   // set size hints to honour minimum size

  Center();
  FParent->RestorePosition(this);
}
TdlgGenerate::~TdlgGenerate()
{
  cbA->OnChange->Clear();
  cbB->OnChange->Clear();
  cbC->OnChange->Clear();
  FParent->SavePosition(this);
}
bool TdlgGenerate::Execute(const IEObject *Sender, const IEObject *Data)  {
  if( (TComboBox*)Sender == cbA )  OnAChange();
  if( (TComboBox*)Sender == cbB )  OnBChange();
  if( (TComboBox*)Sender == cbC )  OnCChange();
  return true;
}
void TdlgGenerate::OnAChange()
{
  double v = 2;
  cbA->GetValue().ToDouble(&v);
  tcAFrom->SetValue( uiStr(olxstr(-v/2)) );
  tcATo->SetValue( uiStr(olxstr(v/2)) );
}
void TdlgGenerate::OnBChange()
{
  double v = 2;
  cbB->GetValue().ToDouble(&v);
  tcBFrom->SetValue( uiStr(olxstr(-v/2)) );
  tcBTo->SetValue( uiStr(olxstr(v/2)) );
}
void TdlgGenerate::OnCChange()
{
  double v = 2;
  cbC->GetValue().ToDouble(&v);
  tcCFrom->SetValue( uiStr(olxstr(-v/2)) );
  tcCTo->SetValue( uiStr(olxstr(v/2)) );
}
void TdlgGenerate::OnOK(wxCommandEvent& event)
{
  double v = 2;
  tcAFrom->GetValue().ToDouble(&v);  FAFrom = v;
  tcBFrom->GetValue().ToDouble(&v);  FBFrom = v;
  tcCFrom->GetValue().ToDouble(&v);  FCFrom = v;

  tcATo->GetValue().ToDouble(&v);  FATo = v;
  tcBTo->GetValue().ToDouble(&v);  FBTo = v;
  tcCTo->GetValue().ToDouble(&v);  FCTo = v;
  EndModal(wxID_OK);
}
