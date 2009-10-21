#ifndef __olx_dlg_primtvs_H
#define __olx_dlg_primtvs_H
#include "ctrls.h"

class TdlgPrimitive: public TDialog  {
protected:
  void OnOK(wxCommandEvent& event);
  TPtrList<wxCheckBox> Boxes;
public:
  TdlgPrimitive(TMainFrame *P, const TStrList& L, int mask);
  virtual ~TdlgPrimitive()  {}
  int Mask;
  DECLARE_EVENT_TABLE()
};

#endif
