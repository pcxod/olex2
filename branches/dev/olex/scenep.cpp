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
    
  short Border = 2;
    
  wxStaticText *stX = new wxStaticText(this, -1, wxT("X"), wxDefaultPosition);
  tbX = new TTrackBar(this, wxDefaultSize);  tbX->OnChange->Add(this);  tbX->SetRange(-100,100);
  teX = new TTextEdit(this);  teX->SetReadOnly(true);
  wxStaticText *stY = new wxStaticText(this, -1, wxT("Y"), wxDefaultPosition);
  tbY = new TTrackBar(this, wxDefaultSize);  tbY->OnChange->Add(this);  tbY->SetRange(-100,100);
  teY = new TTextEdit(this);  teY->SetReadOnly(true);
  wxStaticText *stZ = new wxStaticText(this, -1, wxT("Z"), wxDefaultPosition);
  tbZ = new TTrackBar(this, wxDefaultSize);  tbZ->OnChange->Add(this);  tbZ->SetRange(-100,100);
  teZ = new TTextEdit(this);  teZ->SetReadOnly(true);
  wxStaticText *stR = new wxStaticText(this, -1, wxT("R"), wxDefaultPosition);
  tbR  = new TTrackBar(this, wxDefaultSize); tbR->OnChange->Add(this);  tbR->SetRange(-3,3);
  teR = new TTextEdit(this);  teR->SetReadOnly(true);
    
  //Light Position frame
  wxStaticBox *PBox = new wxStaticBox(this, -1, wxT("Light position"));
  wxStaticBoxSizer *PSizer = new wxStaticBoxSizer(PBox, wxVERTICAL );
  wxFlexGridSizer *LightPosGridSizer = new wxFlexGridSizer(4, 3, Border, Border);
    
  LightPosGridSizer->Add( stX, 0, wxALL, 0 );
  LightPosGridSizer->Add( tbX, 1, wxEXPAND | wxALL, 0 );
  LightPosGridSizer->Add( teX, 0, wxALL, 0 );

  LightPosGridSizer->Add( stY, 0, wxALL, 0 );
  LightPosGridSizer->Add( tbY, 1, wxEXPAND | wxALL, 0 );
  LightPosGridSizer->Add( teY, 0, wxALL, 0 );

  LightPosGridSizer->Add( stZ, 0, wxALL, 0 );
  LightPosGridSizer->Add( tbZ, 1, wxEXPAND | wxALL, 0 );
  LightPosGridSizer->Add( teZ, 0, wxALL, 0 );

  LightPosGridSizer->Add( stR, 0, wxALL, 0 );
  LightPosGridSizer->Add( tbR, 1, wxEXPAND | wxALL, 0 );
  LightPosGridSizer->Add( teR, 0, wxALL, 0 );
  LightPosGridSizer->AddGrowableCol(1);
  PSizer->Add( LightPosGridSizer, 1, wxEXPAND | wxALL, 1 );
  //End Light Position frame

  wxStaticText *stAmb = new wxStaticText(this, -1, wxT("Ambient"), wxDefaultPosition);
  teAmb = new TTextEdit(this); teAmb->SetReadOnly(true);  teAmb->OnClick->Add(this);
  scAmbA = new TSpinCtrl(this); 

  wxStaticText *stDiff = new wxStaticText(this, -1, wxT("Diffuse"), wxDefaultPosition);
  teDiff = new TTextEdit(this); teDiff->SetReadOnly(true);  teDiff->OnClick->Add(this);
  scDiffA = new TSpinCtrl(this);

  wxStaticText *stSpec = new wxStaticText(this, -1, wxT("Specular"), wxDefaultPosition);
  teSpec  = new TTextEdit(this); teSpec->SetReadOnly(true); teSpec->OnClick->Add(this);
  scSpecA = new TSpinCtrl(this);

  wxStaticText *stSExp = new wxStaticText(this, -1, wxT("Spot exponent"), wxDefaultPosition);
  scSExp = new TSpinCtrl(this); scSExp->SetRange(0, 128);

  //Light frame
  wxFlexGridSizer *GridSizer = new wxFlexGridSizer(4, 3, Border, Border);
  GridSizer->Add( stAmb, 1, wxEXPAND | wxALL, 0 );
  GridSizer->Add( teAmb, 1, wxEXPAND | wxALL, 0 );
  GridSizer->Add( scAmbA, 1, wxEXPAND | wxALL, 0 );

  GridSizer->Add( stDiff, 1, wxEXPAND | wxALL, 0 );
  GridSizer->Add( teDiff, 1, wxEXPAND | wxALL, 0 );
  GridSizer->Add( scDiffA, 1, wxEXPAND | wxALL, 0 );

  GridSizer->Add( stSpec, 1, wxEXPAND | wxALL, 0 );
  GridSizer->Add( teSpec, 1, wxEXPAND | wxALL, 0 );
  GridSizer->Add( scSpecA, 1, wxEXPAND | wxALL, 0 );

  GridSizer->Add( stSExp, 1, wxEXPAND | wxALL, 0 );  
  GridSizer->Add( scSExp, 1, wxEXPAND | wxALL, 0 );
  GridSizer->AddGrowableCol(1);
  GridSizer->AddGrowableCol(2);
  GridSizer->SetSizeHints(this);
  
  wxStaticBox *LBox = new wxStaticBox(this, -1, wxT("Light"));
  wxStaticBoxSizer *LSizer = new wxStaticBoxSizer(LBox, wxHORIZONTAL );
  LSizer->Add( GridSizer, 1, wxEXPAND | wxALL, 1 );
  //End Light frame
  
  wxBoxSizer *TSizer0 = new wxBoxSizer( wxHORIZONTAL );
  TSizer0->Add( LSizer, 1, wxEXPAND | wxALL, Border );//Light frame
  TSizer0->Add( PSizer, 1, wxEXPAND | wxALL, Border );//Light position frame

  //Light dropdown menu
  cbLights = new TComboBox(this); 
  for( int i=0; i < 8; i++ )
    cbLights->AddObject(olxstr("Light ") << (i + 1));
  cbLights->SetValue( uiStr(cbLights->GetItem(0)) );
  cbLights->OnChange->Add(this);
  //end Light dropdown menu

  // checkbox enabled
  cbEnabled = new wxCheckBox(this, -1, wxT("Enabled"), wxDefaultPosition);
  wxBoxSizer *SizerE = new wxBoxSizer( wxHORIZONTAL );
  SizerE->Add( cbEnabled, 1, wxEXPAND | wxALL, Border );
  //end checkbox enabled
  
  wxBoxSizer *SizerLt = new wxBoxSizer( wxHORIZONTAL );
  SizerLt->Add( cbLights, 1, wxEXPAND | wxALL, Border );//dropdown light menu
  SizerLt->Add( SizerE, 1, wxEXPAND | wxALL, Border );//checkbox enbaled

  //spot cut off frame
  cbUniform = new wxCheckBox(this, -1, wxT("Uniform"), wxDefaultPosition);
  scSCO = new TSpinCtrl(this); scSCO->SetRange(1, 180);  scSCO->OnChange->Add(this);

  wxStaticBox *BoxSC = new wxStaticBox(this, -1, wxT("Spot cutoff"));
  wxStaticBoxSizer *SizerSC = new wxStaticBoxSizer(BoxSC, wxHORIZONTAL );
  SizerSC->Add( scSCO, 1, wxEXPAND | wxALL, Border );
  SizerSC->Add( cbUniform, 1, wxEXPAND | wxALL, Border );
  //end spot cut off frame

  //spot direction frame
  wxStaticText *stSCX = new wxStaticText(this, -1, wxT("X"), wxDefaultPosition);
  teSCX = new TTextEdit(this);  
  wxStaticText *stSCY = new wxStaticText(this, -1, wxT("Y"), wxDefaultPosition);
  teSCY = new TTextEdit(this);  
  wxStaticText *stSCZ = new wxStaticText(this, -1, wxT("Z"), wxDefaultPosition);
  teSCZ = new TTextEdit(this);  

  wxStaticBox *BoxSD = new wxStaticBox(this, -1, wxT("Spot direction"));
  wxStaticBoxSizer *SizerSD = new wxStaticBoxSizer(BoxSD, wxHORIZONTAL );
  SizerSD->Add( stSCX, 0, wxEXPAND | wxALL, Border );
  SizerSD->Add( teSCX, 1, wxEXPAND | wxALL, Border );
  SizerSD->Add( stSCY, 0, wxEXPAND | wxALL, Border );
  SizerSD->Add( teSCY, 1, wxEXPAND | wxALL, Border );
  SizerSD->Add( stSCZ, 0, wxEXPAND | wxALL, Border );
  SizerSD->Add( teSCZ, 1, wxEXPAND | wxALL, Border );
  //end spot direction frame

  //spot direction frame + spot cut off frame
  wxBoxSizer *SizerS = new wxBoxSizer(wxHORIZONTAL );
  SizerS->Add( SizerSC, 1, wxEXPAND | wxALL, Border );
  SizerS->Add( SizerSD, 1, wxEXPAND | wxALL, Border );
  //end spot direction frame + spot cut off frame

  teAA = new TTextEdit(this);  
  wxStaticText *stAA = new wxStaticText(this, -1, wxT("x^2+"), wxDefaultPosition);
  teAB = new TTextEdit(this);  
  wxStaticText *stAB = new wxStaticText(this, -1, wxT("x+"), wxDefaultPosition);
  teAC = new TTextEdit(this);  

  //frame attenuation
  wxStaticBox *BoxA = new wxStaticBox(this, -1, wxT("Attenuation"));
  wxStaticBoxSizer *SizerA = new wxStaticBoxSizer(BoxA, wxHORIZONTAL );
  SizerA->Add( teAA, 1, wxEXPAND | wxALL, Border );
  SizerA->Add( stAA, 0, wxEXPAND | wxALL, Border );
  SizerA->Add( teAB, 1, wxEXPAND | wxALL, Border );
  SizerA->Add( stAB, 0, wxEXPAND | wxALL, Border );
  SizerA->Add( teAC, 1, wxEXPAND | wxALL, Border );
  //end frame attenuation
  
  wxStaticBox *Box1 = new wxStaticBox(this, -1, wxT("Light sources"));
  wxStaticBoxSizer *TSizer1 = new wxStaticBoxSizer(Box1, wxVERTICAL );
  TSizer1->Add( SizerLt, 0, wxEXPAND | wxALL, Border );//Light dropdown menu
  TSizer1->Add( TSizer0, 0, wxEXPAND | wxALL, Border );//Light frame + Light position frame
  TSizer1->Add( SizerS, 0, wxEXPAND | wxALL, Border );//spot cut off frame
  TSizer1->Add( SizerA, 0, wxEXPAND | wxALL, Border );//frame attenuation

  //light model frame
  cbFonts = new TComboBox(this);
  AGlScene& ascene = FXApp->GetRender().GetScene();
  for( int i=0; i < ascene.FontCount(); i++ )
    cbFonts->AddObject( ascene.GetFont(i)->GetName(), ascene.GetFont(i) );
  cbFonts->SetSelection(0);
  cbFonts->OnChange->Add(this);
  tbEditFont = new TButton(this);  tbEditFont->SetCaption("Edit Font");  tbEditFont->OnClick->Add(this);

  cbLocalV = new wxCheckBox(this, -1, wxT("Local viewer"), wxDefaultPosition);
  cbTwoSide = new wxCheckBox(this, -1, wxT("Two side"), wxDefaultPosition);
  cbSmooth = new wxCheckBox(this, -1, wxT("Smooth shade"), wxDefaultPosition);
  tcAmbLM = new TTextEdit(this); tcAmbLM->SetReadOnly(true);  
    tcAmbLM->OnClick->Add(this);
  wxStaticText *stAmbLM = new wxStaticText(this, -1, wxT("Ambient color"), wxDefaultPosition);
  tcBgClr = new TTextEdit(this); tcBgClr->SetReadOnly(true);  
    tcBgClr->OnClick->Add(this);
  wxStaticText *stBgClr = new wxStaticText(this, -1, wxT("Background color"), wxDefaultPosition);

  wxStaticBox *Box2 = new wxStaticBox(this, -1, wxT("Light model"));
  wxStaticBoxSizer *SizerLM = new wxStaticBoxSizer(Box2, wxHORIZONTAL );

  wxGridSizer *SizerLM1 = new wxGridSizer(2, 2 );
  wxFlexGridSizer *SizerLM2 = new wxFlexGridSizer(2, 2 );

  SizerLM1->Add( cbLocalV, 1, wxEXPAND | wxALL, 0 ); //light model second column
  SizerLM1->Add( cbTwoSide, 1, wxEXPAND | wxALL, 0 );
  SizerLM1->Add( cbSmooth, 1, wxEXPAND | wxALL, 0 );

  SizerLM2->Add( stAmbLM, 1, wxEXPAND | wxALL, 0 );//light model first column
  SizerLM2->Add( tcAmbLM, 1, wxEXPAND | wxALL, 0 );
  SizerLM2->Add( stBgClr, 1, wxEXPAND | wxALL, 0 );
  SizerLM2->Add( tcBgClr, 1, wxEXPAND | wxALL, 0 );
  
  SizerLM->Add( SizerLM2, 1, wxEXPAND | wxALL, Border );//light model frame
  SizerLM->Add( SizerLM1, 1, wxEXPAND | wxALL, Border );
  //light model frame
  
  wxStaticBox *Box2a = new wxStaticBox(this, -1, wxT("Fonts"));
  wxStaticBoxSizer *SizerFonts = new wxStaticBoxSizer(Box2a, wxHORIZONTAL );
  SizerFonts->Add( cbFonts, 1, wxEXPAND | wxALL, Border ); //fonts frame
  SizerFonts->Add( tbEditFont, 1, wxEXPAND | wxALL, Border );

  wxBoxSizer *TSizer2 = new wxBoxSizer(wxVERTICAL );
  TSizer2->Add( SizerFonts, 0, wxEXPAND | wxALL, Border );//fonts frame
  TSizer2->Add(-1,10);//spacer
  TSizer2->Add( SizerLM, 0, wxEXPAND | wxALL, Border );//light model frame
  TSizer2->Add(-1,10);
  TSizer2->Add( TSizer1, 1, wxEXPAND | wxALL, Border );//big light frame

  //right buttons list
  wxBoxSizer *ButtonsSizer = new wxBoxSizer( wxVERTICAL );

  ButtonsSizer->Add( new wxButton( this, wxID_OK, wxT("OK") ), 0, wxEXPAND | wxALL, Border);
  ButtonsSizer->Add( new wxButton( this, wxID_CANCEL, wxT("Cancel") ), 0, wxEXPAND | wxALL, Border);
  ButtonsSizer->Add( new wxButton( this, wxID_HELP, wxT("Help") ),     0, wxEXPAND | wxALL, Border );

  ButtonsSizer->Add( new wxButton( this, wxID_OPEN, wxT("Open") ), 0, wxEXPAND | wxALL, Border);
  ButtonsSizer->Add( new wxButton( this, wxID_SAVE, wxT("Save") ),     0, wxEXPAND | wxALL, Border );
  ButtonsSizer->Add( new wxButton( this, wxID_APPLY, wxT("Apply") ),     0, wxEXPAND | wxALL, Border );
  ButtonsSizer->SetSizeHints(this);
  //end right buttons list

  wxBoxSizer *TSizer3 = new wxBoxSizer(wxHORIZONTAL );
  TSizer3->Add( TSizer2, 1, wxALL, 5 );
  TSizer3->Add( ButtonsSizer, 0, wxALL, 5 );
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
    CD->Destroy();
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
    FXApp->GetRender().GetScene().ShowFontDialog( (TGlFont*)cbFonts->GetObject(sel) );
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

