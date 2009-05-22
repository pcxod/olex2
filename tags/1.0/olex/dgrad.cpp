//----------------------------------------------------------------------------//
// gradient colours properties dialog
// (c) Oleg V. Dolomanov, 2006
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif
#include "dgrad.h"
#include "wx/colordlg.h"
#include "glbackground.h"

BEGIN_EVENT_TABLE(TdlgGradient, TDialog)
  EVT_BUTTON(wxID_OK, TdlgGradient::OnOK)
END_EVENT_TABLE()

TdlgGradient::TdlgGradient(TMainForm *ParentFrame):
  TDialog(ParentFrame, wxT("Gradient"), uiStr(EsdlClassName(TdlgGradient)) )

{                 
  AActionHandler::SetToDelete(false);
  short Border = 3;
  FParent = ParentFrame;

  wxSize DefS(100, 21);

  tcA = new TTextEdit(this);  tcA->SetReadOnly(true);  tcA->WI.SetWidth(34);  tcA->WI.SetHeight(21);  tcA->OnClick->Add(this);
  tcB = new TTextEdit(this);  tcB->SetReadOnly(true);  tcB->WI.SetWidth(34);  tcB->WI.SetHeight(21);  tcB->OnClick->Add(this);
  tcC = new TTextEdit(this);  tcC->SetReadOnly(true);  tcC->WI.SetWidth(34);  tcC->WI.SetHeight(21);  tcC->OnClick->Add(this);
  tcD = new TTextEdit(this);  tcD->SetReadOnly(true);  tcD->WI.SetWidth(34);  tcD->WI.SetHeight(21);  tcD->OnClick->Add(this);

  wxBoxSizer *ASizer = new wxBoxSizer( wxHORIZONTAL );
  ASizer->Add( tcD, 0, wxALL, Border );
  ASizer->Add( tcC, 0, wxALL, Border );

  wxBoxSizer *BSizer = new wxBoxSizer( wxHORIZONTAL );
  BSizer->Add( tcA, 0, wxALL, Border );
  BSizer->Add( tcB, 0, wxALL, Border );

  wxBoxSizer *ButtonsSizer = new wxBoxSizer( wxHORIZONTAL );
  ButtonsSizer->Add( new wxButton( this, wxID_OK, wxT("OK") ), 0, wxALL, Border);
  ButtonsSizer->Add( new wxButton( this, wxID_CANCEL, wxT("Cancel") ), 0, wxALL, Border);
//  ButtonsSizer->Add( new wxButton( this, wxID_HELP, wxT("Help") ),     0, wxALL, Border );

  wxBoxSizer *TopSiser = new wxBoxSizer( wxVERTICAL );
  TopSiser->Add(ASizer, 0, wxALL, 5);
  TopSiser->Add(BSizer, 0, wxALL, 5);
  TopSiser->Add(ButtonsSizer, 0, wxALL, 10);

  SetSizer( TopSiser );      // use the sizer for layout

  TopSiser->SetSizeHints( this );   // set size hints to honour minimum size

  Center();
  Init();
  FParent->RestorePosition(this);
}
//..............................................................................
TdlgGradient::~TdlgGradient()  {
  tcA->OnClick->Clear();
  tcB->OnClick->Clear();
  tcC->OnClick->Clear();
  tcD->OnClick->Clear();
}
//..............................................................................
bool TdlgGradient::Execute(const IEObject *Sender, const IEObject *Data)  {
  if( EsdlInstanceOf( *Sender, TTextEdit) )  {
    wxColourDialog *CD = new wxColourDialog(this);
    wxColor wc = ((TTextEdit*)Sender)->GetBackgroundColour();
    CD->GetColourData().SetColour(wc);
    if( CD->ShowModal() == wxID_OK )  {
      wc = CD->GetColourData().GetColour();
      ((TTextEdit*)Sender)->WI.SetColor(RGB(wc.Red(), wc.Green(), wc.Blue()) );
    }
    delete CD;
  }
  return true;
}
//..............................................................................
void TdlgGradient::Init()  {
  tcA->WI.SetColor( ((TMainForm*)FParent)->XApp()->GetRender().Background()->LT().GetRGB());
  tcB->WI.SetColor( ((TMainForm*)FParent)->XApp()->GetRender().Background()->RT().GetRGB());
  tcC->WI.SetColor( ((TMainForm*)FParent)->XApp()->GetRender().Background()->RB().GetRGB());
  tcD->WI.SetColor( ((TMainForm*)FParent)->XApp()->GetRender().Background()->LB().GetRGB());
}
//..............................................................................
//..............................................................................
void TdlgGradient::OnOK(wxCommandEvent& event)
{
  TGlOption opt;
  opt = tcA->WI.GetColor();  ((TMainForm*)FParent)->XApp()->GetRender().Background()->LT(opt);
  opt = tcB->WI.GetColor();  ((TMainForm*)FParent)->XApp()->GetRender().Background()->RT(opt);
  opt = tcC->WI.GetColor();  ((TMainForm*)FParent)->XApp()->GetRender().Background()->RB(opt);
  opt = tcD->WI.GetColor();  ((TMainForm*)FParent)->XApp()->GetRender().Background()->LB(opt);
  EndModal(wxID_OK);
}
//..............................................................................

