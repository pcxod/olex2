#ifndef _xl_matpropH
#define _xl_matpropH
#include "wx/wx.h"
#include "ctrls.h"
#include "globj.h"
#include "gxapp.h"

class TdlgMatProp: public TDialog, public AActionHandler  {
private:
  wxCheckBox *cbAmbF, *cbAmbB, *cbDiffF, *cbDiffB, *cbEmmF, *cbEmmB,
      *cbSpecF, *cbSpecB, *cbShnF, *cbShnB, *cbTrans, *cbIDraw;
  TTextEdit *tcAmbF, *tcAmbB, *tcDiffF, *tcDiffB, *tcEmmF, *tcEmmB,
      *tcSpecF, *tcSpecB, *tcShnF, *tcShnB;
  TSpinCtrl *scAmbF, *scAmbB, *scDiffF, *scDiffB, *scEmmF, *scEmmB,
      *scSpecF, *scSpecB, *scTrans;
  wxCheckBox *cbApplyToGroup;
  wxButton* bEditFont;
  TComboBox *cbPrimitives;
  TGPCollection *GPCollection;
  TEList *FSpinCtrls;
protected:
  void OnOK(wxCommandEvent& event);
  bool Execute(const IEObject *Sender, const IEObject *Data=NULL);
  void OnRemove()  {  ;  }
  TGlMaterial *FMaterials;
  TSAtom *FAtom;
  int FCurrentMaterial;
  TGXApp *FXApp;
  void Init( const TGlMaterial &GlM );
  void Update( TGlMaterial &GlM );
  void Update(){  wxWindow::Update(); };

  static TGlMaterial MaterialCopy;
  void OnCopy(wxCommandEvent& event);
  void OnPaste(wxCommandEvent& event);
  void OnEditFont(wxCommandEvent& event);
public:
  TdlgMatProp(TMainFrame *ParentFrame, TGPCollection *GPC, TGXApp *XApp);
  virtual ~TdlgMatProp();
  void ApplyToGroupEnabled(bool v){  if( cbApplyToGroup )  cbApplyToGroup->Enable(v); }
  void ApplyToGroupChecked(bool v){  if( cbApplyToGroup )  cbApplyToGroup->SetValue(v); }

  const TGlMaterial& GetCurrent()  const {  return FMaterials[FCurrentMaterial];  }
  void SetCurrent(const TGlMaterial& m)  {
    FMaterials[FCurrentMaterial] = m;
    Init(m);
  }
//..............................................................................
// properties

//..............................................................................
// interface
//..............................................................................
  DECLARE_EVENT_TABLE()
};

#endif
