/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "updateoptions.h"
#include "bapp.h"
#include "efile.h"
#include "etime.h"

//..............................................................................
TdlgUpdateOptions::TdlgUpdateOptions(TMainFrame *ParentFrame) :
  TDialog(ParentFrame, wxT("Update options"), wxT("dlgAutoUpdate"))
{
  short Border = 0;

  stProxy = new wxStaticText(this, -1,
    wxT("Proxy [if required, URL or user:password@URL]"),
    wxDefaultPosition, wxDefaultSize);
  stRepository = new wxStaticText(this, -1, wxT("Repository URL"),
    wxDefaultPosition, wxDefaultSize);
  stLastUpdated = new wxStaticText(this, -1, wxT("Last updated: unknown"),
    wxDefaultPosition, wxDefaultSize);

  if (uapi.GetSettings().last_updated != 0) {
    stLastUpdated->SetLabel(
      wxString(wxT("Last updated: ")) +
      TETime::FormatDateTime(uapi.GetSettings().last_updated).u_str());
  }
  else {
    stLastUpdated->SetLabel(wxT("Last updated: Never"));
  }
  TStrList repos;
  uapi.GetAvailableMirrors(repos);
  tcProxy = new wxTextCtrl(this, -1, uapi.GetSettings().proxy.u_str(),
    wxDefaultPosition, wxSize(400, -1), 0);
  cbRepository = new wxComboBox(this, -1, wxEmptyString, wxDefaultPosition,
    wxSize(400,-1), 0, NULL, wxTE_READONLY);
  for (size_t i=0; i < repos.Count(); i++)
    cbRepository->Append(uapi.AddTagPart(repos[i], true).u_str());
  cbRepository->SetValue(
    uapi.AddTagPart(uapi.GetSettings().repository, true).u_str());

  cbQueryUpdate = new wxCheckBox(this, -1,
    wxT("Download updates automatically"));
  cbQueryUpdate->SetValue(!uapi.GetSettings().ask_for_update);

  wxString options[] = {
    wxT("Always"), wxT("Daily"), wxT("Weekly"), wxT("Monthly"), wxT("Never")
  };
  rbUpdateInterval = new wxRadioBox(this, -1, wxT("Update Frequency"),
    wxDefaultPosition, wxDefaultSize, 5, options);
  int selIndex = rbUpdateInterval->FindString(
    uapi.GetSettings().update_interval.u_str());
  if (selIndex >= 0)
    rbUpdateInterval->SetSelection(selIndex);

#if !wxCHECK_VERSION(2,9,0)
  wxFlexGridSizer *ASizer = new wxFlexGridSizer(1, 7);
#else
  wxFlexGridSizer *ASizer = new wxFlexGridSizer(1, 7, 0);
#endif
  //wxBoxSizer *ASizer = new wxBoxSizer(wxVERTICAL);
  ASizer->Add(stProxy, 0, wxALL, Border);
  ASizer->Add(tcProxy, 1, wxALL, Border);
  ASizer->Add(stRepository, 0, wxALL, Border);
  ASizer->Add(cbRepository, 0, wxALL, Border);
  ASizer->Add(rbUpdateInterval, 0, wxALL, Border);
  ASizer->Add(cbQueryUpdate, 0, wxALL, Border);
  ASizer->Add(stLastUpdated, 0, wxALL, Border);
  wxBoxSizer *ButtonsSizer = new wxBoxSizer(wxHORIZONTAL);
  ButtonsSizer->Add(new wxButton(this, wxID_OK, wxT("OK")), 0, wxALL, Border);
  ButtonsSizer->Add(new wxButton(this, wxID_CANCEL, wxT("Cancel")), 0, wxALL, Border);
  ButtonsSizer->Add(new wxButton(this, wxID_HELP, wxT("Help")), 0, wxALL, Border);

  wxBoxSizer *TopSiser = new wxBoxSizer(wxVERTICAL);
  TopSiser->Add(ASizer, 0, wxALL, 0);
  TopSiser->Add(ButtonsSizer, 0, wxALL, 10);
  SetSizer(TopSiser); // use the sizer for layout
  TopSiser->SetSizeHints(this);   // set size hints to honour minimum size
  Center();
  Bind(wxEVT_BUTTON, &TdlgUpdateOptions::OnOK, this, wxID_OK);
}
//..............................................................................
TdlgUpdateOptions::~TdlgUpdateOptions() {}
//..............................................................................
void TdlgUpdateOptions::OnOK(wxCommandEvent& event)  {
  uapi.GetSettings().proxy = tcProxy->GetValue();
  uapi.GetSettings().repository = uapi.TrimTagPart(cbRepository->GetValue());
  uapi.GetSettings().update_interval = rbUpdateInterval->GetStringSelection();
  uapi.GetSettings().ask_for_update = !cbQueryUpdate->GetValue();
  uapi.GetSettings().Save();
  EndModal(wxID_OK);
}
//..............................................................................
