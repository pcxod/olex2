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
  SettingsFile = TBasicApp::GetInstance()->BaseDir() + "usettings.dat";
  if( TEFile::Exists( SettingsFile ) )
    SF.LoadSettings( SettingsFile );
  short Border = 0;

  stProxy = new wxStaticText(this, -1, wxT("Proxy [if required, URL or user:password@URL]"), wxDefaultPosition, wxDefaultSize);
  stRepository = new wxStaticText(this, -1, wxT("Repository URL"), wxDefaultPosition, wxDefaultSize);
  stLastUpdated = new wxStaticText(this, -1, wxT("Last updated: unknown"), wxDefaultPosition, wxDefaultSize);

  olxstr lastUpdate = SF.ParamValue("lastupdate");
  if( lastUpdate.Length() )
  {
  // buggy wxWidgets will fail for timezone <> 0
//    wxDateTime dt( (time_t)lastUpdate.Long() );
//    wxString date = "Last updated: ";
//    date += dt.FormatISODate();
//    date += ' ';
//    date += dt.FormatISOTime();

    stLastUpdated->SetLabel( uiStr(TETime::FormatDateTime(lastUpdate.RadInt<time_t>())) );
  }
  wxString choices[] = {wxT("http://dimas.dur.ac.uk/olex-distro/update/"),
  wxT("http://dimas.dur.ac.uk/olex-distro-test/update/"),
  wxT("http://www.x-rayman.co.uk/olex2/olex-distro-test/update/")};
  tcProxy = new wxTextCtrl(this, -1, uiStr(SF.ParamValue("proxy")) , wxDefaultPosition, wxSize(320, 21), 0);
  cbRepository = new wxComboBox(this, -1, uiStr(SF.ParamValue("repository")), wxDefaultPosition, wxSize(320, 21), 3, choices);

  wxString options[] = {wxT("Always"), wxT("Daily"), wxT("Weekly"), wxT("Monthly"), wxT("Never")};
  rbUpdateInterval = new wxRadioBox(this, -1, wxT("Update Frequency"),
                           wxDefaultPosition, wxDefaultSize, 5, options);
  int selIndex = rbUpdateInterval->FindString( uiStr(SF.ParamValue("update")) );
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
  DSizer->Add( stLastUpdated, 0, wxALL, Border );

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
  SF.UpdateParam( "proxy", tcProxy->GetValue().c_str() );
  SF.UpdateParam( "repository", cbRepository->GetValue().c_str() );
  SF.UpdateParam( "update", rbUpdateInterval->GetStringSelection().c_str() );
  SF.SaveSettings( SettingsFile );
  EndModal(wxID_OK);
}
//..............................................................................

