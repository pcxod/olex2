#ifndef _xl_primtvsH
#define _xl_primtvsH
#include "ctrls.h"
#include "wx/listctrl.h"

class TdlgPrimitive: public TDialog
{
protected:
  void OnOK(wxCommandEvent& event);
  TPtrList<wxCheckBox> Boxes;
public:
  TdlgPrimitive(const TStrList& L, int mask, class TMainForm *P);
  virtual ~TdlgPrimitive();
  int Mask;
  DECLARE_EVENT_TABLE()
};

#endif
