#ifndef __olx_cmdline_H
#define __olx_cmdline_H
#include "ctrls.h"

class TCmdLine : public TTextEdit, public AActionHandler  {
private:
  TActionQList Actions;
protected:
  olxstr PromptStr;
  TStrList Commands;
  int CmdIndex;
  virtual bool Execute(const IEObject *Sender, const IEObject *Data=NULL);
public:
  TCmdLine(wxWindow *parent, int flags);
  virtual ~TCmdLine();

  bool ProcessKey(wxKeyEvent& evt);

  DefPropC(olxstr, PromptStr)

  olxstr GetCommand()  {
    return GetText().Length() > PromptStr.Length() ? GetText().SubStringFrom( PromptStr.Length() ) :
                                                  EmptyString();
  }
  void SetCommand(const olxstr& cmd);

  TActionQueue &OnCommand;
};

#endif
