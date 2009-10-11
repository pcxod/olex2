#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "updateoptions.h"
#include "bapp.h"
#include "efile.h"
#include "etime.h"

BEGIN_EVENT_TABLE(TdlgUpdateOptions, TDialog)
  EVT_BUTTON(wxID_OK, TdlgUpdateOptions::OnOK)
END_EVENT_TABLE()

//..............................................................................
TdlgUpdateOptions::TdlgUpdateOptions(TMainFrame *ParentFrame) :
  TDialog(ParentFrame, wxT("Update options"), wxT("dlgAutoUpdate"))
{
  short Border = 0;

  stProxy = new wxStaticText(this, -1, wxT("Proxy [if required, URL or user:password@URL]"), wxDefaultPosition, wxDefaultSize);
  stRepository = new wxStaticText(this, -1, wxT("Repository URL"), wxDefaultPosition, wxDefaultSize);
  stLastUpdated = new wxStaticText(this, -1, wxT("Last updated: unknown"), wxDefaultPosition, wxDefaultSize);

  if( uapi.GetSettings().last_updated != 0 )
    stLastUpdated->SetLabel( wxString(wxT("Last updated: ")) + TETime::FormatDateTime(uapi.GetSettings().last_updated).u_str() );
  else
    stLastUpdated->SetLabel(wxT("Last updated: Never"));
  TStrList repos;
  uapi.GetAvailableMirrors(repos);
  tcProxy = new wxTextCtrl(this, -1, uapi.GetSettings().proxy.u_str() , wxDefaultPosition, wxSize(320, 21), 0);
  cbRepository = new wxComboBox(this, -1, wxEmptyString, wxDefaultPosition, wxSize(320, 21), 0, NULL, wxTE_READONLY);
  for( int i=0; i < repos.Count(); i++ )
    cbRepository->Append( uapi.AddTagPart(repos[i], true).u_str() );
  cbRepository->SetValue( uapi.AddTagPart(uapi.GetSettings().repository, true).u_str() );
  
  cbQueryUpdate = new wxCheckBox(this, -1, wxT("Download updates automatically"));
  cbQueryUpdate->SetValue( !uapi.GetSettings().ask_for_update );

  wxString options[] = {wxT("Always"), wxT("Daily"), wxT("Weekly"), wxT("Monthly"), wxT("Never")};
  rbUpdateInterval = new wxRadioBox(this, -1, wxT("Update Frequency"),
                           wxDefaultPosition, wxDefaultSize, 5, options);
  int selIndex = rbUpdateInterval->FindString( uapi.GetSettings().update_interval.u_str() );
  if( selIndex >= 0 )  rbUpdateInterval->SetSelection( selIndex );

  wxBoxSizer *TopSiser = new wxBoxSizer( wxVERTICAL );

  wxBoxSizer *ASizer = new wxBoxSizer( wxHORIZONTAL );
  ASizer->Add( stProxy, 0, wxALL, Border );
  wxBoxSizer *AASizer = new wxBoxSizer( wxHORIZONTAL );
  AASizer->Add( tcProxy, 1, wxALL, Border );

  wxBoxSizer *BSizer = new wxBoxSizer( wxHORIZONTAL );
  BSizer->Add( stRepository, 0, wxALL, Border );
  wxBoxSizer *BBSizer = new wxBoxSizer( wxHORIZONTAL );
  BBSizer->Add( cbRepository, 0, wxALL, Border );

  wxBoxSizer *CSizer = new wxBoxSizer( wxHORIZONTAL );
  CSizer->Add( rbUpdateInterval, 0, wxALL, Border );

  wxBoxSizer *DSizer = new wxBoxSizer( wxHORIZONTAL );
  DSizer->Add( cbQueryUpdate, 0, wxALL, Border );

  wxBoxSizer *ESizer = new wxBoxSizer( wxHORIZONTAL );
  ESizer->Add( stLastUpdated, 0, wxALL, Border );

  wxBoxSizer *ButtonsSizer = new wxBoxSizer( wxHORIZONTAL );

  ButtonsSizer->Add( new wxButton( this, wxID_OK, wxT("OK") ), 0, wxALL, Border);
  ButtonsSizer->Add( new wxButton( this, wxID_CANCEL, wxT("Cancel") ), 0, wxALL, Border);
  ButtonsSizer->Add( new wxButton( this, wxID_HELP, wxT("Help") ),     0, wxALL, Border );

  TopSiser->Add(ASizer, 0, wxALL, 0);
  TopSiser->Add(AASizer, 0, wxALL, 0);
  TopSiser->Add(BSizer, 0, wxALL, 0);
  TopSiser->Add(BBSizer, 0, wxALL, 0);
  TopSiser->Add(CSizer, 0, wxALL, 0);
  TopSiser->Add(DSizer, 0, wxALL, 0);
  TopSiser->Add(ESizer, 0, wxALL, 0);
  TopSiser->Add(ButtonsSizer, 0, wxALL, 10);
  SetSizer( TopSiser );      // use the sizer for layout

  TopSiser->SetSizeHints( this );   // set size hints to honour minimum size

  Center();

  FParent->RestorePosition(this);
}
//..............................................................................
TdlgUpdateOptions::~TdlgUpdateOptions() {
  FParent->SavePosition(this);
}
//..............................................................................
void TdlgUpdateOptions::OnOK(wxCommandEvent& event)  {
  uapi.GetSettings().proxy = tcProxy->GetValue().c_str();
  uapi.GetSettings().repository = uapi.TrimTagPart(cbRepository->GetValue().c_str());
  uapi.GetSettings().update_interval = rbUpdateInterval->GetStringSelection().c_str();
  uapi.GetSettings().ask_for_update = !cbQueryUpdate->GetValue();
  uapi.GetSettings().Save();
  EndModal(wxID_OK);
}
//..............................................................................

