#ifndef __olx_dlg_matprop_H
#define __olx_dlg_matprop_H
#include "ctrls.h"
#include "gxapp.h"

class TdlgMatProp: public TDialog, public AActionHandler  {
private:
  wxCheckBox *cbAmbF, *cbAmbB, *cbDiffF, *cbDiffB, *cbEmmF, *cbEmmB,
      *cbSpecF, *cbSpecB, *cbShnF, *cbShnB, *cbTrans, *cbIDraw;
  TTextEdit *tcAmbF, *tcAmbB, *tcDiffF, *tcDiffB, *tcEmmF, *tcEmmB,
      *tcSpecF, *tcSpecB, *tcShnF, *tcShnB;
  TSpinCtrl *scAmbF, *scAmbB, *scDiffF, *scDiffB, *scEmmF, *scEmmB,
      *scSpecF, *scSpecB, *scTrans;
  wxButton* bEditFont;
  wxComboBox *cbApplyTo;
  TComboBox *cbPrimitives;
  TPtrList<TSpinCtrl> SpinCtrls;
protected:
  void OnOK(wxCommandEvent& event);
  bool Execute(const IEObject *Sender, const IEObject *Data=NULL);
  void OnRemove()  {}
  TGlMaterial *Materials, *Material;
  AGDrawObject* Object;
  int FCurrentMaterial;
  void Init(const TGlMaterial& GlM);
  void Update(TGlMaterial& GlM);
  void Update()  {  wxWindow::Update();  }

  static TGlMaterial MaterialCopy;
  void OnCopy(wxCommandEvent& event);
  void OnPaste(wxCommandEvent& event);
  void OnEditFont(wxCommandEvent& event);
  void Init();
public:
  TdlgMatProp(TMainFrame *ParentFrame, AGDrawObject &GPC);
  TdlgMatProp(TMainFrame *ParentFrame, TGlMaterial& mat);
  ~TdlgMatProp();
  const TGlMaterial& GetCurrent()  const {  return Materials[FCurrentMaterial];  }
  void SetCurrent(const TGlMaterial& m)  {
    Materials[FCurrentMaterial] = m;
    Init(m);
  }
  DECLARE_EVENT_TABLE()
};

#endif
