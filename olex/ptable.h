//---------------------------------------------------------------------------

#ifndef ptableH
#define ptableH
#include "eobjects.h"
#include "actions.h"
#include "atominfo.h"
#include "wx/wx.h"
#include "ctrls.h"
//---------------------------------------------------------------------------
class TPTableDlg: public TDialog, public AActionHandler
{
protected:
  void CreateButton(int i, int j, int offset);
  TEList *ButtonsList;
  TAtomsInfo *FAI;
  TBasicAtomInfo *FSelected;
  class TMainFrame *FParent;

public:
  TPTableDlg(class TMainFrame *Parent, TAtomsInfo *AI);
  virtual ~TPTableDlg();
  TBasicAtomInfo *Selected(){  return FSelected; }
  bool Execute(const IEObject *Sender, const IEObject *Data);
};
#endif
