#ifndef olx_editdlg_H
#define olx_editdlg_H
#include "ctrls.h"

class TdlgEdit: public wxDialog  {
private:
  wxTextCtrl *Text;
  TMainFrame *FParent;
public:
  TdlgEdit(TMainFrame *ParentFrame, bool MultiLine);
  ~TdlgEdit();
  void SetText(const olxstr& Text);
  olxstr GetText();
  TWindowInterface WI;
};
#endif
 
