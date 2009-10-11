//---------------------------------------------------------------------------

#ifndef olx_msgboxH
#define olx_msgboxH
//---------------------------------------------------------------------------

#include "wx/wx.h"
#include "wx/dialog.h"
#include "wx/textctrl.h"
#include "wx/checkbox.h"
#include "ctrls.h"
#include "tptrlist.h"
//---------------------------------------------------------------------------

class TdlgMsgBox: public TDialog, public AActionHandler  {
protected:
  wxCheckBox* cbRemember;
  bool Execute(const IEObject *Sender, const IEObject *Data=NULL);
  TPtrList<TButton> buttons;
public:
  TdlgMsgBox(TMainFrame* Parent, const olxstr& msg, const olxstr& title, 
    const olxstr& tickBoxMsg, long flags, bool ShowRememberCheckBox);
  virtual ~TdlgMsgBox();
  bool IsChecked()  {  return cbRemember == NULL ? false : cbRemember->IsChecked();  }
  // return a string YNCO for yes, no, cancel, or OK, R-remember choice
  static olxstr Execute(TMainFrame* Parent, const olxstr& msg, const olxstr& title, 
    const olxstr& tickBoxMsg ="Remember my decision", 
    long flags=wxID_OK|wxICON_INFORMATION, bool ShowRememberCheckBox=false);
};
#endif
