/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "primtvs.h"
#include "styles.h"
#include "xbond.h"
#include "xatom.h"

BEGIN_EVENT_TABLE(TdlgPrimitive, TDialog)
  EVT_BUTTON(wxID_OK, TdlgPrimitive::OnOK)
END_EVENT_TABLE()
//..............................................................................
TdlgPrimitive::TdlgPrimitive(TMainFrame *P, AGDrawObject& object) :
  TDialog(P, wxT("Primitives"), wxT("dlgPrimitives")),
  Object(object), cbApplyTo(NULL), Mask(-1), Level(-1)
{
  wxBoxSizer *TopSizer = new wxBoxSizer(wxVERTICAL);
  const int border = 1;
  if( (EsdlInstanceOf(Object,TXAtom) || EsdlInstanceOf(Object, TXBond)) )  {
    const uint16_t current_level =
      TXAtom::LegendLevel(Object.GetPrimitives().GetName());
    wxArrayString choices;
    if( EsdlInstanceOf(Object,TXAtom) )  {
      if( current_level == 0 )
        choices.Add(wxT("Atom Type"));
      if( current_level <= 1 )
        choices.Add(wxT("Atom Name"));
      choices.Add(wxT("Individual Atom"));
    }
    else {
      if( current_level == 0 )
        choices.Add(wxT("Bond by Atom Types"));
      if( current_level <= 1 )
        choices.Add(wxT("Bond by Atom Names"));
      choices.Add(wxT("Individual Bond"));
    }
    cbApplyTo = new wxComboBox(this, -1,
      choices[0], wxDefaultPosition, wxDefaultSize, choices, wxCB_READONLY);
    cbApplyTo->SetSelection(0);
    wxBoxSizer* Sizer0 = new wxBoxSizer(wxHORIZONTAL);
    Sizer0->Add(new wxStaticText(this, -1, wxT("Apply to: ")), 1, wxEXPAND|wxALL, border);
    Sizer0->Add(cbApplyTo, 1, wxFIXED_MINSIZE|wxALL, border);
    TopSizer->Add(Sizer0, 1, wxEXPAND|wxALL, border);
    TopSizer->Add(-1, 10);
  }
  TStrList L;
  Object.ListPrimitives(L);
  int mask = Object.GetPrimitives().GetStyle().GetParam(
    Object.GetPrimitiveMaskName(), "0").ToInt();
  for( size_t i=0; i < L.Count(); i++ )  {
    wxCheckBox* Box = Boxes.Add(
      new wxCheckBox(this, -1, L[i].u_str(), wxDefaultPosition));
    TopSizer->Add(Box, 0, wxALL, border);
    Box->SetValue((mask & (1 << i)) != 0);
  }
  wxBoxSizer *ButtonsSizer = new wxBoxSizer(wxHORIZONTAL);
  ButtonsSizer->Add(new wxButton(this, wxID_OK, wxT("OK")), 1, wxALL, 1);
  ButtonsSizer->Add(new wxButton(this, wxID_CANCEL, wxT("Cancel")), 1, wxALL, 1);
  ButtonsSizer->Add(new wxButton(this, wxID_HELP, wxT("Help")), 1, wxALL, 1);
  TopSizer->Add(-1, 10);
  TopSizer->Add(ButtonsSizer, 0, wxALL, border);
  TopSizer->SetSizeHints(this);   // set size hints to honour minimum size
  SetSizer(TopSizer);
  Center();
}
//..............................................................................
void TdlgPrimitive::OnOK(wxCommandEvent& event)  {
  Mask = 0;
  for( size_t i=0; i < Boxes.Count(); i++ )  {
    if( Boxes[i]->GetValue() )
      Mask |= (1 << i);
  }
  if( cbApplyTo != NULL )
    Level = cbApplyTo->GetSelection() + (3-cbApplyTo->GetCount());
  EndModal(wxID_OK);
}
//..............................................................................
