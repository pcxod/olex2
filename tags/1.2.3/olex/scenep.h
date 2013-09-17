/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_dlg_scenep_H
#define __olx_dlg_scenep_H
#include "ctrls.h"
#include "gloption.h"
#include "gllightmodel.h"

class TdlgSceneProps: public TDialog, public AActionHandler  {
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
  bool Execute(const IEObject *Sender, const IEObject *Data, TActionQueue *);
  TGlLightModel FLightModel, FOriginalModel;
  int FCurrentLight;
  void InitLight( TGlLight &GlL );
  void UpdateLight( TGlLight &GlL );
  void InitLightModel( TGlLightModel &GlLM );
  void UpdateLightModel( TGlLightModel &GlLM );
  void Update(){  wxWindow::Update(); };
public:
  TdlgSceneProps(TMainFrame *ParentForm);
  virtual ~TdlgSceneProps();
  void LoadFromFile(TGlLightModel &FLM, const olxstr &FN);
  void SaveToFile(TGlLightModel &FLM, const olxstr &FN);
  DECLARE_EVENT_TABLE()
};
#endif
