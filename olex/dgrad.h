//---------------------------------------------------------------------------

#ifndef dgradH
#define dgradH
//---------------------------------------------------------------------------

#include "wx/wx.h"
#include "ctrls.h"
#include "mainform.h"
//---------------------------------------------------------------------------

class TdlgGradient: public TDialog, AActionHandler  {
private:
  TTextEdit *tcA, *tcB, *tcC, *tcD;
  wxStaticText *stcA, *stcB, *stcC, *stcD;
protected:
  void OnOK(wxCommandEvent& event);
  bool Execute(const IEObject *Sender, const IEObject *Data=NULL);
  int A, B, C, D;
  void Init();
public:
  TdlgGradient(TMainForm *ParentFrame);
  virtual ~TdlgGradient();
  DefPropP(int, A)
  DefPropP(int, B)
  DefPropP(int, C)
  DefPropP(int, D)
  DECLARE_EVENT_TABLE()
};
#endif
