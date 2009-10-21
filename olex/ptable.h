#ifndef __olx_dlg_ptable_H
#define __olx_dlg_ptable_H
#include "ctrls.h"
#include "atominfo.h"

class TPTableDlg: public TDialog, public AActionHandler  {
protected:
  void CreateButton(int i, int j, int offset);
  TPtrList<TButton> ButtonsList;
  TBasicAtomInfo *Selected;
public:
  TPTableDlg(TMainFrame *Parent);
  virtual ~TPTableDlg();
  TBasicAtomInfo *GetSelected()  {  return Selected; }
  bool Execute(const IEObject *Sender, const IEObject *Data);
};
#endif
