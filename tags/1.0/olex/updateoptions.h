#ifndef updateoptionsH
#define updateoptionsH

#include "wx/wx.h"
#include "ctrls.h"
#include "settingsfile.h"
//---------------------------------------------------------------------------

class TdlgUpdateOptions: public TDialog  {
  wxStaticText *stProxy, *stProxyUser, *stProxyPasswd, 
    *stRepository, *stLastUpdated;
  wxTextCtrl *tcProxy;
  wxComboBox* cbRepository;
  wxRadioBox *rbUpdateInterval;
protected:
  TSettingsFile SF;
  void OnOK(wxCommandEvent& event);
  olxstr SettingsFile;
public:

  TdlgUpdateOptions(TMainFrame *ParentFrame);
  virtual ~TdlgUpdateOptions();

  DECLARE_EVENT_TABLE()
};
#endif
