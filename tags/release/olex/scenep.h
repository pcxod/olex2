//---------------------------------------------------------------------------
#ifndef _xl_scenepH
#define _xl_scenepH
#include "wx/wx.h"
#include "ctrls.h"
#include "globj.h"
#include "xapp.h"
#include "mainform.h"

class TdlgSceneProps: public TDialog, public AActionHandler
{
private:
  wxCheckBox *cbEnabled, *cbUniform;
  TSpinCtrl *scSCO, *scAmbA, *scDiffA, *scSpecA, *scSExp;
  TTrackBar *tbX, *tbY, *tbZ, *tbR;
  TTextEdit *teAmb, *teDiff, *teSpec, *teSCX, *teSCY, *teSCZ, *teAA, *teAB, *teAC;
  TTextEdit *teX, *teY, *teZ, *teR;
  // light model
  wxCheckBox *cbLocalV, *cbTwoSide, *cbSmooth;
  TTextEdit *tcAmbLM, *tcBgClr;
  TButton* tbEditFont;
  TComboBox *cbLights, *cbFonts;
  TGlOption ConsoleFontColor, NotesFontColor, LabelsFontColor, XColor;
protected:
  void OnOK(wxCommandEvent& event);
  void OnCancel(wxCommandEvent& event);
  void OnOpen(wxCommandEvent& event);
  void OnSave(wxCommandEvent& event);
  void OnApply(wxCommandEvent& event);
  bool Execute(const IEObject *Sender, const IEObject *Data=NULL);
  TGXApp *FXApp;
  TGlLightModel FLightModel, FOriginalModel;
  int FCurrentLight;
  void InitLight( TGlLight &GlL );
  void UpdateLight( TGlLight &GlL );
  void InitLightModel( TGlLightModel &GlLM );
  void UpdateLightModel( TGlLightModel &GlLM );
  void Update(){  wxWindow::Update(); };
public:
  TdlgSceneProps(TMainForm *ParentForm, TGXApp *XApp);
  virtual ~TdlgSceneProps();
  void LoadFromFile(TGlLightModel &FLM, const olxstr &FN);
  void SaveToFile(TGlLightModel &FLM, const olxstr &FN);
//..............................................................................
// properties

//..............................................................................
// interface
//..............................................................................
  DECLARE_EVENT_TABLE()
};
#endif
