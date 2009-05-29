//---------------------------------------------------------------------------

#ifndef editH
#define editH
//---------------------------------------------------------------------------

#include "wx/wx.h"
#include "ctrls.h"
#include "mainform.h"
//---------------------------------------------------------------------------

class TdlgEdit: public wxDialog  {
private:
  wxTextCtrl *Text;
  TMainFrame *FParent;
public:
  TdlgEdit(TMainForm *ParentFrame, bool MultiLine);
  ~TdlgEdit();
  void SetText(const olxstr& Text);
  olxstr GetText();
  TWindowInterface WI;
};
#endif
 
