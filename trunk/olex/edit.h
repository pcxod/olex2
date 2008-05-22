//---------------------------------------------------------------------------

#ifndef editH
#define editH
//---------------------------------------------------------------------------

#include "wx/wx.h"
#include "ctrls.h"
#include "mainform.h"
//---------------------------------------------------------------------------

class TdlgEdit: public TDialog  {
private:
  TTextEdit *Text;
public:
  TdlgEdit(TMainForm *ParentFrame, bool MultiLine);
  ~TdlgEdit();
  void SetText(const olxstr& Text);
  olxstr GetText();
};
#endif
 
