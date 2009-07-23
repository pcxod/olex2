#ifndef updateoptionsH
#define updateoptionsH

#include "wx/wx.h"
#include "ctrls.h"
#include "updateapi.h"
//---------------------------------------------------------------------------

class TdlgUpdateOptions: public TDialog  {
  wxStaticText *stProxy, *stProxyUser, *stProxyPasswd, 
    *stRepository, *stLastUpdated;
  wxTextCtrl *tcProxy;
  wxComboBox* cbRepository;
  wxCheckBox* cbQueryUpdate;
  wxRadioBox *rbUpdateInterval;
protected:
  void OnOK(wxCommandEvent& event);
  updater::UpdateAPI uapi;
public:

  TdlgUpdateOptions(TMainFrame *ParentFrame);
  virtual ~TdlgUpdateOptions();

  DECLARE_EVENT_TABLE()
};
#endif
