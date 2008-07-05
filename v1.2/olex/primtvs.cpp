//----------------------------------------------------------------------------//
// primitives dialog
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif
#include "primtvs.h"
#include "mainform.h"

//..............................................................................
BEGIN_EVENT_TABLE(TdlgPrimitive, TDialog)
  EVT_BUTTON(wxID_OK, TdlgPrimitive::OnOK)
END_EVENT_TABLE()
//..............................................................................
TdlgPrimitive::TdlgPrimitive(TStrList *L, int mask, TMainForm *P) :
  TDialog(P, wxT("Primitives"), wxT("dlgPrimitives"))
{
  wxBoxSizer *ButtonsSizer = new wxBoxSizer( wxHORIZONTAL );
  wxBoxSizer *TopSizer = new wxBoxSizer( wxVERTICAL );
  ButtonsSizer->Add( new wxButton( this, wxID_OK, wxT("OK") ), 0, wxALL, 1);
  ButtonsSizer->Add( new wxButton( this, wxID_CANCEL, wxT("Cancel") ), 0, wxALL, 1);
  ButtonsSizer->Add( new wxButton( this, wxID_HELP, wxT("Help") ),     0, wxALL, 1);

  int i, off;
  FPList = L;
  wxSize DefS(150, 21);
  wxCheckBox *Box;
  for(i=0; i < L->Count(); i++ )
  {
    Box = new wxCheckBox(this, -1, uiStr(L->String(i)), wxDefaultPosition, DefS);
    TopSizer->Add( Box,     0, wxALL, 1);
    FBoxes.Add(Box);
    off = 1;
    off <<= i;
    Box->SetValue( (mask & off) != 0 );  
  }

  TopSizer->Add( ButtonsSizer,     0, wxALL, 1);
  
  TopSizer->SetSizeHints( this );   // set size hints to honour minimum size
  SetSizer(TopSizer);
  Center();
}
//..............................................................................
TdlgPrimitive::~TdlgPrimitive()
{
}
//..............................................................................
void TdlgPrimitive::OnOK(wxCommandEvent& event)
{
  int i, off;
  Mask = 0;
  wxCheckBox *Box;
  for( i=0; i < FBoxes.Count(); i++ )
  {
    Box = (wxCheckBox *)FBoxes[i];
    if( Box->GetValue() )
    {
      off = 1;
      off <<= i;
      Mask |= off;
    }
  }
  EndModal(wxID_OK);
}
//..............................................................................

