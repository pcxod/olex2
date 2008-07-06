//----------------------------------------------------------------------------//
// scene properties dialog
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "scenep.h"
#include "wx/fontdlg.h"
#include "wx/colordlg.h"

#include "glfont.h"
#include "glscene.h"

#include "efile.h"

//..............................................................................
BEGIN_EVENT_TABLE(TdlgSceneProps, TDialog)
  EVT_BUTTON(wxID_OK, TdlgSceneProps::OnOK)
  EVT_BUTTON(wxID_CANCEL, TdlgSceneProps::OnCancel)
  EVT_BUTTON(wxID_OPEN, TdlgSceneProps::OnOpen)
  EVT_BUTTON(wxID_SAVE, TdlgSceneProps::OnSave)
  EVT_BUTTON(wxID_APPLY, TdlgSceneProps::OnApply)
END_EVENT_TABLE()
//..............................................................................

TdlgSceneProps::TdlgSceneProps(TMainForm *ParentFrame, TGXApp *XApp)
:TDialog(ParentFrame, wxT("Scene Parameters"), uiStrT(EsdlClassNameT(TdlgSceneProps)) )
{
  AActionHandler::SetToDelete(false);
  FXApp = XApp;

  wxSize DefS(100, 21);
  short Border = 1, TbW=100, SpW = 40;
  wxStaticText *stX = new wxStaticText(this, -1, wxT("X"), wxDefaultPosition, wxSize(21, 21));
  tbX = new TTrackBar(this);  tbX->WI.SetWidth(TbW);  tbX->OnChange->Add(this);  tbX->SetRange(-100,100);
  teX = new TTextEdit(this);  teX->WI.SetWidth(25);   teX->SetReadOnly(true);
  wxStaticText *stY = new wxStaticText(this, -1, wxT("Y"), wxDefaultPosition, wxSize(21, 21));
  tbY = new TTrackBar(this);  tbY->WI.SetWidth(TbW);  tbY->OnChange->Add(this);  tbY->SetRange(-100,100);
  teY = new TTextEdit(this);  teY->WI.SetWidth(25);   teY->SetReadOnly(true);
  wxStaticText *stZ = new wxStaticText(this, -1, wxT("Z"), wxDefaultPosition, wxSize(21, 21));
  tbZ = new TTrackBar(this);  tbZ->WI.SetWidth(TbW);  tbZ->OnChange->Add(this);  tbZ->SetRange(-100,100);
  teZ = new TTextEdit(this);  teZ->WI.SetWidth(25);   teZ->SetReadOnly(true);
  wxStaticText *stR = new wxStaticText(this, -1, wxT("R"), wxDefaultPosition, wxSize(21, 21));
  tbR  = new TTrackBar(this); tbR->WI.SetWidth(TbW);  tbR->OnChange->Add(this);  tbR->SetRange(-3,3);
  teR = new TTextEdit(this);  teR->WI.SetWidth(25);   teR->SetReadOnly(true);

  wxBoxSizer *PSizerX = new wxBoxSizer( wxHORIZONTAL );
  PSizerX->Add( stX, 0, wxALL, Border );
  PSizerX->Add( tbX, 0, wxALL, Border );
  PSizerX->Add( teX, 0, wxALL, Border );

  wxBoxSizer *PSizerY = new wxBoxSizer( wxHORIZONTAL );
  PSizerY->Add( stY, 0, wxALL, Border );
  PSizerY->Add( tbY, 0, wxALL, Border );
  PSizerY->Add( teY, 0, wxALL, Border );

  wxBoxSizer *PSizerZ = new wxBoxSizer( wxHORIZONTAL );
  PSizerZ->Add( stZ, 0, wxALL, Border );
  PSizerZ->Add( tbZ, 0, wxALL, Border );
  PSizerZ->Add( teZ, 0, wxALL, Border );

  wxBoxSizer *PSizerR = new wxBoxSizer( wxHORIZONTAL );
  PSizerR->Add( stR, 0, wxALL, Border );
  PSizerR->Add( tbR, 0, wxALL, Border );
  PSizerR->Add( teR, 0, wxALL, Border );

  wxStaticBox *PBox = new wxStaticBox(this, -1, wxT("Light position"));
  wxStaticBoxSizer *PSizer = new wxStaticBoxSizer(PBox, wxVERTICAL );
  PSizer->Add( PSizerX, 0, wxALL, 1 );
  PSizer->Add( PSizerY, 0, wxALL, 1 );
  PSizer->Add( PSizerZ, 0, wxALL, 1 );
  PSizer->Add( PSizerR, 0, wxALL, 1 );

  wxStaticText *stAmb = new wxStaticText(this, -1, wxT("Ambient"), wxDefaultPosition, wxSize(45, 21));
  teAmb = new TTextEdit(this); teAmb->SetReadOnly(true);  teAmb->WI.SetWidth(34);  teAmb->WI.SetHeight(21);  teAmb->OnClick->Add(this);
  scAmbA = new TSpinCtrl(this);  scAmbA->WI.SetWidth(SpW);

  wxStaticText *stDiff = new wxStaticText(this, -1, wxT("Diffuse"), wxDefaultPosition, wxSize(45, 21));
  teDiff = new TTextEdit(this); teDiff->SetReadOnly(true);  teDiff->WI.SetWidth(34);  teDiff->WI.SetHeight(21);  teDiff->OnClick->Add(this);
  scDiffA = new TSpinCtrl(this); scDiffA->WI.SetWidth(SpW);

  wxStaticText *stSpec = new wxStaticText(this, -1, wxT("Specular"), wxDefaultPosition, wxSize(45, 21));
  teSpec  = new TTextEdit(this); teSpec->SetReadOnly(true);  teSpec->WI.SetWidth(34);  teSpec->WI.SetHeight(21);  teSpec->OnClick->Add(this);
  scSpecA = new TSpinCtrl(this); scSpecA->WI.SetWidth(SpW);

  wxStaticText *stSExp = new wxStaticText(this, -1, wxT("Spot exponent"), wxDefaultPosition, wxSize(80, 21));
  scSExp = new TSpinCtrl(this); scSExp->WI.SetWidth(SpW+5);  scSExp->SetRange(0, 128);

  wxBoxSizer *LSizerA = new wxBoxSizer( wxHORIZONTAL );
  LSizerA->Add( stAmb, 0, wxALL, Border );
  LSizerA->Add( teAmb, 0, wxALL, Border );
  LSizerA->Add( scAmbA, 0, wxALL, Border );

  wxBoxSizer *LSizerB = new wxBoxSizer( wxHORIZONTAL );
  LSizerB->Add( stDiff, 0, wxALL, Border );
  LSizerB->Add( teDiff, 0, wxALL, Border );
  LSizerB->Add( scDiffA, 0, wxALL, Border );

  wxBoxSizer *LSizerC = new wxBoxSizer( wxHORIZONTAL );
  LSizerC->Add( stSpec, 0, wxALL, Border );
  LSizerC->Add( teSpec, 0, wxALL, Border );
  LSizerC->Add( scSpecA, 0, wxALL, Border );

  wxBoxSizer *LSizerD = new wxBoxSizer( wxHORIZONTAL );
  LSizerD->Add( stSExp, 0, wxALL, Border );
  LSizerD->Add( scSExp, 0, wxALL, Border );

  wxStaticBox *LBox = new wxStaticBox(this, -1, wxT("Light"));
  wxStaticBoxSizer *LSizer = new wxStaticBoxSizer(LBox, wxVERTICAL );
  LSizer->Add( LSizerA, 0, wxALL, 1 );
  LSizer->Add( LSizerB, 0, wxALL, 1 );
  LSizer->Add( LSizerC, 0, wxALL, 1 );
  LSizer->Add( LSizerD, 0, wxALL, 1 );

  wxBoxSizer *TSizer0 = new wxBoxSizer( wxHORIZONTAL );
  TSizer0->Add( LSizer, 0, wxALL, 1 );
  TSizer0->Add( PSizer, 0, wxALL, 1 );


  wxBoxSizer *ButtonsSizer = new wxBoxSizer( wxVERTICAL );

  ButtonsSizer->Add( new wxButton( this, wxID_OK, wxT("OK") ), 0, wxALL, Border);
  ButtonsSizer->Add( new wxButton( this, wxID_CANCEL, wxT("Cancel") ), 0, wxALL, Border);
  ButtonsSizer->Add( new wxButton( this, wxID_HELP, wxT("Help") ),     0, wxALL, Border );

  ButtonsSizer->Add( new wxButton( this, wxID_OPEN, wxT("Open") ), 0, wxALL, Border);
  ButtonsSizer->Add( new wxButton( this, wxID_SAVE, wxT("Save") ),     0, wxALL, Border );
  ButtonsSizer->Add( new wxButton( this, wxID_APPLY, wxT("Apply") ),     0, wxALL, Border );

  cbLights = new TComboBox(this); 
  for( int i=0; i < 8; i++ )
    cbLights->AddObject(olxstr("Light ") << (i + 1));
  cbLights->SetValue( uiStr(cbLights->GetItem(0)) );
  cbLights->OnChange->Add(this);
  wxBoxSizer *SizerLt = new wxBoxSizer( wxVERTICAL );
  SizerLt->Add( cbLights, 0, wxALL, 1 );


  cbUniform = new wxCheckBox(this, -1, wxT("Uniform"), wxDefaultPosition, wxSize(80, 21));
  scSCO = new TSpinCtrl(this); scSCO->WI.SetWidth(SpW);  scSCO->SetRange(1, 180);  scSCO->OnChange->Add(this);

  wxStaticBox *BoxSC = new wxStaticBox(this, -1, wxT("Spot cutoff"));
  wxStaticBoxSizer *SizerSC = new wxStaticBoxSizer(BoxSC, wxHORIZONTAL );
  SizerSC->Add( scSCO, 0, wxALL, 1 );
  SizerSC->Add( cbUniform, 0, wxALL, 1 );

  wxStaticText *stSCX = new wxStaticText(this, -1, wxT("X"), wxDefaultPosition, wxSize(10, 21));
  teSCX = new TTextEdit(this);  teSCX->WI.SetWidth(45);
  wxStaticText *stSCY = new wxStaticText(this, -1, wxT("Y"), wxDefaultPosition, wxSize(10, 21));
  teSCY = new TTextEdit(this);  teSCY->WI.SetWidth(45);
  wxStaticText *stSCZ = new wxStaticText(this, -1, wxT("Z"), wxDefaultPosition, wxSize(10, 21));
  teSCZ = new TTextEdit(this);  teSCZ->WI.SetWidth(45);

  wxStaticBox *BoxSD = new wxStaticBox(this, -1, wxT("Spot direction"));
  wxStaticBoxSizer *SizerSD = new wxStaticBoxSizer(BoxSD, wxHORIZONTAL );
  SizerSD->Add( stSCX, 0, wxALL, 1 );
  SizerSD->Add( teSCX, 0, wxALL, 1 );
  SizerSD->Add( stSCY, 0, wxALL, 1 );
  SizerSD->Add( teSCY, 0, wxALL, 1 );
  SizerSD->Add( stSCZ, 0, wxALL, 1 );
  SizerSD->Add( teSCZ, 0, wxALL, 1 );

  wxBoxSizer *SizerS = new wxBoxSizer(wxHORIZONTAL );
  SizerS->Add( SizerSC, 0, wxALL, 1 );
  SizerS->Add( SizerSD, 0, wxALL, 1 );


  teAA = new TTextEdit(this);  teAA->WI.SetWidth(45);
  wxStaticText *stAA = new wxStaticText(this, -1, wxT("x^2+"), wxDefaultPosition, wxSize(25, 21));
  teAB = new TTextEdit(this);  teAB->WI.SetWidth(45);
  wxStaticText *stAB = new wxStaticText(this, -1, wxT("x+"), wxDefaultPosition, wxSize(15, 21));
  teAC = new TTextEdit(this);  teAC->WI.SetWidth(45);

  wxStaticBox *BoxA = new wxStaticBox(this, -1, wxT("Attenuation"));
  wxStaticBoxSizer *SizerA = new wxStaticBoxSizer(BoxA, wxHORIZONTAL );
  SizerA->Add( teAA, 0, wxALL, 1 );
  SizerA->Add( stAA, 0, wxALL, 1 );
  SizerA->Add( teAB, 0, wxALL, 1 );
  SizerA->Add( stAB, 0, wxALL, 1 );
  SizerA->Add( teAC, 0, wxALL, 1 );

  cbEnabled = new wxCheckBox(this, -1, wxT("Enabled"), wxDefaultPosition, DefS);
  wxBoxSizer *SizerE = new wxBoxSizer( wxHORIZONTAL );
  SizerE->Add( cbEnabled, 0, wxALL, 1 );

  wxStaticBox *Box1 = new wxStaticBox(this, -1, wxT(""));
  wxStaticBoxSizer *TSizer1 = new wxStaticBoxSizer(Box1, wxVERTICAL );
  TSizer1->Add( SizerLt, 0, wxALL, 1 );
  TSizer1->Add( TSizer0, 0, wxALL, 1 );
  TSizer1->Add( SizerS, 0, wxALL, 1 );
  TSizer1->Add( SizerA, 0, wxALL, 1 );
  TSizer1->Add( SizerE, 0, wxALL, 1 );

  cbFonts = new TComboBox(this);
  AGlScene* ascene = FXApp->GetRender().Scene();
  for( int i=0; i < ascene->FontCount(); i++ )
    cbFonts->AddObject( ascene->Font(i)->GetName(), ascene->Font(i) );
  cbFonts->SetSelection(0);
  cbFonts->OnChange->Add(this);
  tbEditFont = new TButton(this);  tbEditFont->SetCaption("Edit Font");  tbEditFont->OnClick->Add(this);

  cbLocalV = new wxCheckBox(this, -1, wxT("Local viewer"), wxDefaultPosition, DefS);
  cbTwoSide = new wxCheckBox(this, -1, wxT("Two side"), wxDefaultPosition, DefS);
  cbSmooth = new wxCheckBox(this, -1, wxT("Smooth shade"), wxDefaultPosition, DefS);
  tcAmbLM = new TTextEdit(this); tcAmbLM->SetReadOnly(true);  tcAmbLM->WI.SetWidth(34);
    tcAmbLM->WI.SetHeight(21);  tcAmbLM->OnClick->Add(this);
  wxStaticText *stAmbLM = new wxStaticText(this, -1, wxT("Ambient color"), wxDefaultPosition, wxSize(75, 21));
  tcBgClr = new TTextEdit(this); tcBgClr->SetReadOnly(true);  tcBgClr->WI.SetWidth(34);
    tcBgClr->WI.SetHeight(21);  tcBgClr->OnClick->Add(this);
  wxStaticText *stBgClr = new wxStaticText(this, -1, wxT("Background color"), wxDefaultPosition, wxSize(100, 21));

  wxStaticBox *Box2 = new wxStaticBox(this, -1, wxT("Light model"));
  wxStaticBox *Box2a = new wxStaticBox(this, -1, wxT("Fonts"));
  wxStaticBoxSizer *SizerLM = new wxStaticBoxSizer(Box2, wxVERTICAL );
  wxStaticBoxSizer *SizerFonts = new wxStaticBoxSizer(Box2a, wxVERTICAL );

  wxBoxSizer *SizerLM0 = new wxBoxSizer(wxHORIZONTAL );
  wxBoxSizer *SizerLM1 = new wxBoxSizer(wxHORIZONTAL );
  wxBoxSizer *SizerLM2 = new wxBoxSizer(wxHORIZONTAL );

  SizerLM1->Add( cbLocalV, 0, wxALL, Border );
  SizerLM1->Add( cbTwoSide, 0, wxALL, Border );
  SizerLM1->Add( cbSmooth, 0, wxALL, Border );

  SizerLM2->Add( stAmbLM, 0, wxALL, Border );
  SizerLM2->Add( tcAmbLM, 0, wxALL, Border );
  SizerLM2->Add( stBgClr, 0, wxALL, Border );
  SizerLM2->Add( tcBgClr, 0, wxALL, Border );

  SizerLM->Add( SizerLM2, 0, wxALL, Border );
  SizerLM->Add( SizerLM1, 0, wxALL, Border );

  SizerFonts->Add( cbFonts, 0, wxALL, Border );
  SizerFonts->Add( tbEditFont, 0, wxALL, Border );

  SizerLM0->Add( SizerLM, 0, wxALL, Border );
  SizerLM0->Add( SizerFonts, 0, wxALL, Border );


  wxStaticBox *Box3 = new wxStaticBox(this, -1, wxT(""));
  wxStaticBoxSizer *TSizer2 = new wxStaticBoxSizer(Box3, wxVERTICAL );
  TSizer2->Add( SizerLM0, 0, wxALL, 1 );
  TSizer2->Add( TSizer1, 0, wxALL, 1 );

//  wxStaticBox *Box4 = new wxStaticBox(this, -1, "");
  wxBoxSizer *TSizer3 = new wxBoxSizer(wxHORIZONTAL );
  TSizer3->Add( TSizer2, 0, wxALL, 1 );
  TSizer3->Add( ButtonsSizer, 0, wxALL, 1 );
  SetSizer(TSizer3);
  TSizer3->SetSizeHints(this);

  Center();

  FCurrentLight = 0;
  FLightModel = FXApp->GetRender().LightModel;
  FOriginalModel = FXApp->GetRender().LightModel;
  InitLight(FLightModel.Light(0));
  InitLightModel(FLightModel);
  FParent->RestorePosition(this);

}
//..............................................................................
TdlgSceneProps::~TdlgSceneProps()  {
  tbX->OnChange->Clear();
  tbY->OnChange->Clear();
  tbZ->OnChange->Clear();
  tbR->OnChange->Clear();
  teAmb->OnClick->Clear();
  teDiff->OnClick->Clear();
  teSpec->OnClick->Clear();
  scSCO->OnChange->Clear();
  cbLights->OnChange->Clear();
  tcAmbLM->OnClick->Clear();
  tcBgClr->OnClick->Clear();
  cbFonts->OnChange->Clear();
  tbEditFont->OnClick->Clear();
}
//..............................................................................
bool TdlgSceneProps::Execute(const IEObject *Sender, const IEObject *Data)  {
  if( (TTrackBar*)Sender == tbX )    teX->SetText(tbX->GetValue());
  if( (TTrackBar*)Sender == tbY )    teY->SetText(tbY->GetValue());
  if( (TTrackBar*)Sender == tbZ )    teZ->SetText(tbZ->GetValue());
  if( (TTrackBar*)Sender == tbR )    teR->SetText(tbR->GetValue());
  if( EsdlInstanceOf( *Sender, TTextEdit) )  {
    wxColourDialog *CD = new wxColourDialog(this);
    wxColor wc = ((TTextEdit*)Sender)->GetBackgroundColour();
    CD->GetColourData().SetColour(wc);
    if( CD->ShowModal() == wxID_OK )  {
      wc = CD->GetColourData().GetColour();
      ((TTextEdit*)Sender)->WI.SetColor(RGB(wc.Red(), wc.Green(), wc.Blue()) );
    }
  }
  if( (TComboBox*)Sender == cbLights )  {
    UpdateLight(FLightModel.Light(FCurrentLight));
    int i = cbLights->FindString(cbLights->GetValue());
    if( i >= 0 )  {
      FCurrentLight = i;
      InitLight(FLightModel.Light(FCurrentLight));
    }
  }
  if( (TSpinCtrl*)Sender == scSCO )  {
    if( scSCO->GetValue() > 90 )  cbUniform->SetValue(true);
    else                       cbUniform->SetValue(false);
  }
  if( (TButton*)Sender == tbEditFont )  {
    int sel = cbFonts->GetSelection();
    if( sel == -1 )  return false;
    FXApp->GetRender().Scene()->ShowFontDialog( (TGlFont*)cbFonts->GetObject(sel) );
  }

  return true;
}
//..............................................................................
void TdlgSceneProps::InitLightModel( TGlLightModel &GlLM )  {
  cbLocalV->SetValue(GlLM.LocalViewer());
  cbTwoSide->SetValue(GlLM.TwoSide());
  cbSmooth->SetValue(GlLM.SmoothShade());
  tcAmbLM->WI.SetColor(GlLM.AmbientColor().GetRGB());
  tcBgClr->WI.SetColor(GlLM.ClearColor().GetRGB());
}
//..............................................................................
void TdlgSceneProps::UpdateLightModel( TGlLightModel &GlLM )  {
  GlLM.LocalViewer(cbLocalV->GetValue());
  GlLM.TwoSide(cbTwoSide->GetValue());
  GlLM.SmoothShade(cbSmooth->GetValue());
  GlLM.AmbientColor() = tcAmbLM->WI.GetColor();
  GlLM.ClearColor() = tcBgClr->WI.GetColor();
}
//..............................................................................
void TdlgSceneProps::InitLight( TGlLight &L )  {
  tbX->SetValue((int)L.Position()[0]);  teX->SetText((int)L.Position()[0]);
  tbY->SetValue((int)L.Position()[1]);  teY->SetText((int)L.Position()[1]);
  tbZ->SetValue((int)L.Position()[2]);  teZ->SetText((int)L.Position()[2]);
  tbR->SetValue((int)L.Position()[3]);  teR->SetText((int)L.Position()[3]);

  teAmb->WI.SetColor(L.Ambient().GetRGB());
  scAmbA->SetValue( (int)L.Ambient()[3]*100);
  teDiff->WI.SetColor(L.Diffuse().GetRGB());
  scDiffA->SetValue((int)L.Diffuse()[3]*100);
  teSpec->WI.SetColor(L.Specular().GetRGB());
  scSpecA->SetValue((int)L.Specular()[3]*100);
  teAA->SetText( L.Attenuation()[2] );
  teAB->SetText( L.Attenuation()[1] );
  teAC->SetText( L.Attenuation()[0] );
  cbEnabled->SetValue( L.Enabled() );
  scSExp->SetValue( L.SpotExponent() );
  scSCO->SetValue( L.SpotCutoff() );
  cbUniform->SetValue(L.SpotCutoff() == 180);
  teSCX->SetText( L.SpotDirection()[0] );
  teSCY->SetText( L.SpotDirection()[1] );
  teSCZ->SetText( L.SpotDirection()[2] );
}
//..............................................................................
void TdlgSceneProps::UpdateLight( TGlLight &L )  {
  L.Position()[0] = tbX->GetValue();
  L.Position()[1] = tbY->GetValue();
  L.Position()[2] = tbZ->GetValue();
  L.Position()[3] = tbR->GetValue();

  L.Ambient() = teAmb->WI.GetColor();
  L.Ambient()[3] = (float)scAmbA->GetValue()/100;

  L.Diffuse() = teDiff->WI.GetColor();
  L.Diffuse()[3] = (float)scDiffA->GetValue()/100;

  L.Specular() = teSpec->WI.GetColor();
  L.Specular()[3] = (float)scSpecA->GetValue()/100;

  L.Attenuation()[2] = teAA->GetText().ToDouble();
  L.Attenuation()[1] = teAB->GetText().ToDouble();
  L.Attenuation()[0] = teAC->GetText().ToDouble();

  L.Enabled( cbEnabled->GetValue() );

  L.SpotExponent( scSExp->GetValue() );

  if( cbUniform->GetValue() )
    L.SpotCutoff( 180 );
  else
    L.SpotCutoff( scSCO->GetValue() );

  L.SpotDirection()[0] = teSCX->GetText().ToDouble();
  L.SpotDirection()[1] = teSCY->GetText().ToDouble();
  L.SpotDirection()[2] = teSCZ->GetText().ToDouble();
}
//..............................................................................
void TdlgSceneProps::OnApply(wxCommandEvent& event)  {
  UpdateLight(FLightModel.Light(FCurrentLight));
  UpdateLightModel(FLightModel);
  FXApp->GetRender().LightModel = FLightModel;
  FXApp->GetRender().LoadIdentity();
  FXApp->GetRender().InitLights();
  FXApp->Draw();
}
//..............................................................................
void TdlgSceneProps::OnCancel(wxCommandEvent& event)  {
  FXApp->GetRender().LightModel = FOriginalModel;
  FXApp->GetRender().LoadIdentity();
  FXApp->GetRender().InitLights();
  EndModal(wxID_OK);
}
//..............................................................................
void TdlgSceneProps::OnOK(wxCommandEvent& event)
{
  OnApply(event);
  EndModal(wxID_OK);
}
//..............................................................................
void TdlgSceneProps::OnOpen(wxCommandEvent& event)
{
  olxstr FN = FParent->PickFile("Load scene parameters",
  "Scene parameters|*.glsp", ((TMainForm*)FParent)->SParamDir, true);
  if( FN.Length() )
  {
    LoadFromFile(FLightModel, FN);
    ((TMainForm*)FParent)->SParamDir = TEFile::ExtractFilePath(FN);
    InitLight(FLightModel.Light(FCurrentLight));
    InitLightModel(FLightModel);
  }
}
//..............................................................................
void TdlgSceneProps::OnSave(wxCommandEvent& event)
{
  olxstr FN = FParent->PickFile("Save scene parameters",
  "Scene parameters|*.glsp", ((TMainForm*)FParent)->SParamDir, false);
  if( FN.Length() )
  {
    UpdateLight(FLightModel.Light(FCurrentLight));
    ((TMainForm*)FParent)->SParamDir = TEFile::ExtractFilePath(FN);
    UpdateLightModel(FLightModel);
    SaveToFile(FLightModel, FN);
  }
}
//..............................................................................
void TdlgSceneProps::LoadFromFile(TGlLightModel &FLM, const olxstr &FN)
{
  TDataFile F;
  F.LoadFromXLFile(FN, NULL);
  ((TMainForm*)FParent)->LoadScene(&F.Root(), &FLM);
}
//..............................................................................
void TdlgSceneProps::SaveToFile(TGlLightModel &FLM, const olxstr &FN)
{
  TDataFile DF;
  ((TMainForm*)FParent)->SaveScene(&DF.Root(), &FLM);
  try{  DF.SaveToXLFile(FN); }
  catch(...){  TBasicApp::GetLog().Error("Failed to save scene parameters!"); }
}
//..............................................................................

