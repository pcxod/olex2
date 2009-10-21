#ifndef __olx_graddlg_H
#define __olx_graddlg_H
#include "ctrls.h"

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
  TdlgGradient(TMainFrame *ParentFrame);
  virtual ~TdlgGradient();
  DefPropP(int, A)
  DefPropP(int, B)
  DefPropP(int, C)
  DefPropP(int, D)
  DECLARE_EVENT_TABLE()
};
#endif
