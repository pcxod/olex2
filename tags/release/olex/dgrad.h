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
  int FA, FB, FC, FD;
  void Init();
public:
  TdlgGradient(TMainForm *ParentFrame);
  virtual ~TdlgGradient();
//..............................................................................

//..............................................................................
// interface
  int LT()  {  return FA;  }
  int RT()  {  return FB;  }
  int LB()  {  return FC;  }
  int RB()  {  return FD;  }
//..............................................................................
  DECLARE_EVENT_TABLE()
};
#endif
