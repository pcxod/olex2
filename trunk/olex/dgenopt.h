//---------------------------------------------------------------------------

#ifndef _xl_dgenoptH
#define _xl_dgenoptH
#include "wx/wx.h"
#include "ctrls.h"
//---------------------------------------------------------------------------

class TdlgGenerate: public TDialog, AActionHandler  {
private:
  wxStaticText *stAFrom, *stBFrom, *stCFrom, *stATo, *stBTo, *stCTo;
  wxTextCtrl *tcAFrom, *tcBFrom, *tcCFrom, *tcATo, *tcBTo, *tcCTo;
  TComboBox *cbA, *cbB, *cbC;
protected:
  void OnOK(wxCommandEvent& event);
  void OnLinkChanges(wxCommandEvent& event);
  void OnAChange();
  void OnBChange();
  void OnCChange();
  bool Execute(const IEObject *Sender, const IEObject *Data=NULL);

  float AFrom, BFrom, CFrom, ATo, BTo, CTo;
public:
  TdlgGenerate(TMainFrame *ParentFrame);
  virtual ~TdlgGenerate();
  DefPropP(float, AFrom)
  DefPropP(float, BFrom)
  DefPropP(float, CFrom)
  DefPropP(float, ATo)
  DefPropP(float, BTo)
  DefPropP(float, CTo)
//..............................................................................
  DECLARE_EVENT_TABLE()
};
#endif
