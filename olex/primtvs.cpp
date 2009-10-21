//----------------------------------------------------------------------------//
// primitives dialog
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//
#include "primtvs.h"
//..............................................................................
BEGIN_EVENT_TABLE(TdlgPrimitive, TDialog)
  EVT_BUTTON(wxID_OK, TdlgPrimitive::OnOK)
END_EVENT_TABLE()
//..............................................................................
TdlgPrimitive::TdlgPrimitive(TMainFrame *P, const TStrList& L, int mask) :
  TDialog(P, wxT("Primitives"), wxT("dlgPrimitives"))
{
  wxBoxSizer *ButtonsSizer = new wxBoxSizer( wxHORIZONTAL );
  wxBoxSizer *TopSizer = new wxBoxSizer( wxVERTICAL );
  ButtonsSizer->Add( new wxButton( this, wxID_OK, wxT("OK") ), 0, wxALL, 1);
  ButtonsSizer->Add( new wxButton( this, wxID_CANCEL, wxT("Cancel") ), 0, wxALL, 1);
  ButtonsSizer->Add( new wxButton( this, wxID_HELP, wxT("Help") ),     0, wxALL, 1);

  wxSize DefS(150, 21);
  for( int i=0; i < L.Count(); i++ )  {
    wxCheckBox* Box = Boxes.Add( new wxCheckBox(this, -1, L[i].u_str(), wxDefaultPosition, DefS) );
    TopSizer->Add( Box, 0, wxALL, 1);
    Box->SetValue( (mask & (1 << i)) != 0 );  
  }

  TopSizer->Add( ButtonsSizer,     0, wxALL, 1);
  
  TopSizer->SetSizeHints( this );   // set size hints to honour minimum size
  SetSizer(TopSizer);
  Center();
}
//..............................................................................
void TdlgPrimitive::OnOK(wxCommandEvent& event)  {
  Mask = 0;
  for( int i=0; i < Boxes.Count(); i++ )  {
    if( Boxes[i]->GetValue() )
      Mask |= (1 << i);
  }
  EndModal(wxID_OK);
}
//..............................................................................

