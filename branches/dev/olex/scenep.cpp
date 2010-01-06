//----------------------------------------------------------------------------//
// scene properties dialog
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//
#include "scenep.h"
#include "wx/fontdlg.h"
#include "wx/colordlg.h"

#include "gxapp.h"

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
TdlgSceneProps::TdlgSceneProps(TMainFrame *ParentFrame) :
  TDialog(ParentFrame, wxT("Scene Parameters"), EsdlClassName(TdlgSceneProps).u_str() )
{
  AActionHandler::SetToDelete(false);
    
  short Border = 2;
    
  wxStaticText *stX = new wxStaticText(this, -1, wxT("X"), wxDefaultPosition);
  tbX = new TTrackBar(this, wxDefaultSize);  tbX->OnChange.Add(this);  tbX->SetRange(-100,100);
  teX = new TTextEdit(this);  teX->SetReadOnly(true);
  wxStaticText *stY = new wxStaticText(this, -1, wxT("Y"), wxDefaultPosition);
  tbY = new TTrackBar(this, wxDefaultSize);  tbY->OnChange.Add(this);  tbY->SetRange(-100,100);
  teY = new TTextEdit(this);  teY->SetReadOnly(true);
  wxStaticText *stZ = new wxStaticText(this, -1, wxT("Z"), wxDefaultPosition);
  tbZ = new TTrackBar(this, wxDefaultSize);  tbZ->OnChange.Add(this);  tbZ->SetRange(-100,100);
  teZ = new TTextEdit(this);  teZ->SetReadOnly(true);
  wxStaticText *stR = new wxStaticText(this, -1, wxT("R"), wxDefaultPosition);
  tbR  = new TTrackBar(this, wxDefaultSize); tbR->OnChange.Add(this);  tbR->SetRange(-3,3);
  teR = new TTextEdit(this);  teR->SetReadOnly(true);
    
  //Light Position frame
  wxStaticBox *PBox = new wxStaticBox(this, -1, wxT("Light position"));
  wxStaticBoxSizer *PSizer = new wxStaticBoxSizer(PBox, wxVERTICAL );
  wxFlexGridSizer *LightPosGridSizer = new wxFlexGridSizer(4, 3, Border, Border);
    
  LightPosGridSizer->Add( stX, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 2 );
  LightPosGridSizer->Add( tbX, 1, wxEXPAND | wxALL, 1 );
  LightPosGridSizer->Add( teX, 0, wxALIGN_CENTER_VERTICAL | wxALL, 1 );

  LightPosGridSizer->Add( stY, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 2 );
  LightPosGridSizer->Add( tbY, 1, wxEXPAND | wxALL, 1 );
  LightPosGridSizer->Add( teY, 0, wxALIGN_CENTER_VERTICAL | wxALL, 1 );

  LightPosGridSizer->Add( stZ, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 2 );
  LightPosGridSizer->Add( tbZ, 1, wxEXPAND | wxALL, 1 );
  LightPosGridSizer->Add( teZ, 0, wxALIGN_CENTER_VERTICAL | wxALL, 1 );

  LightPosGridSizer->Add( stR, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 2 );
  LightPosGridSizer->Add( tbR, 1, wxEXPAND | wxALL, 1 );
  LightPosGridSizer->Add( teR, 0, wxALIGN_CENTER_VERTICAL | wxALL, 1 );
  LightPosGridSizer->AddGrowableCol(1);
  PSizer->Add( LightPosGridSizer, 1, wxEXPAND | wxALL, 1 );
  //End Light Position frame

  wxStaticText *stcolour = new wxStaticText(this, -1, wxT("Colour"), wxDefaultPosition);
  wxStaticText *sttransparency = new wxStaticText(this, -1, wxT("Transparency"), wxDefaultPosition);

  wxStaticText *stAmb = new wxStaticText(this, -1, wxT("Ambient"), wxDefaultPosition);
  teAmb = new TTextEdit(this); teAmb->SetReadOnly(true);  teAmb->OnClick.Add(this);
  scAmbA = new TSpinCtrl(this); 

  wxStaticText *stDiff = new wxStaticText(this, -1, wxT("Diffuse"), wxDefaultPosition);
  teDiff = new TTextEdit(this); teDiff->SetReadOnly(true);  teDiff->OnClick.Add(this);
  //teDiff = new wxColourPickerCtrl(this, -1); //teDiff->OnClick.Add(this);
  scDiffA = new TSpinCtrl(this);

  wxStaticText *stSpec = new wxStaticText(this, -1, wxT("Specular"), wxDefaultPosition);
  teSpec  = new TTextEdit(this); teSpec->SetReadOnly(true); teSpec->OnClick.Add(this);
  scSpecA = new TSpinCtrl(this);

  wxStaticText *stSExp = new wxStaticText(this, -1, wxT("Spot exponent"), wxDefaultPosition);
  scSExp = new TSpinCtrl(this); scSExp->SetRange(0, 128);

  //Light frame
  wxFlexGridSizer *GridSizer = new wxFlexGridSizer(4, 3, Border, Border);
  GridSizer->Add( -1,10, 0, wxALIGN_CENTER | wxRIGHT, 2 );
  GridSizer->Add( stcolour, 0, wxALIGN_CENTER | wxALL, 1 );
  GridSizer->Add( sttransparency, 0, wxALIGN_CENTER | wxALL, 1 );

  GridSizer->Add( stAmb, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 2 );
  GridSizer->Add( teAmb, 0, wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 1 );
  GridSizer->Add( scAmbA, 0, wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 1 );

  GridSizer->Add( stDiff, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 2 );
  GridSizer->Add( teDiff, 0, wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 1 );
  GridSizer->Add( scDiffA, 0, wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 1 );

  GridSizer->Add( stSpec, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 2 );
  GridSizer->Add( teSpec, 0, wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 1 );
  GridSizer->Add( scSpecA, 0, wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 1 );

  GridSizer->Add( stSExp, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 2 );  
  GridSizer->Add( scSExp, 0, wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 1 );
  GridSizer->AddGrowableCol(1);
  GridSizer->AddGrowableCol(2);
  GridSizer->SetSizeHints(this);
  
  wxStaticBox *LBox = new wxStaticBox(this, -1, wxT("Light colours"));
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
  cbLights->SetValue(cbLights->GetItem(0).u_str());
  cbLights->OnChange.Add(this);
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
  scSCO = new TSpinCtrl(this); scSCO->SetRange(1, 180);  scSCO->OnChange.Add(this);

  wxStaticBox *BoxSC = new wxStaticBox(this, -1, wxT("Spot cutoff"));
  wxStaticBoxSizer *SizerSC = new wxStaticBoxSizer(BoxSC, wxHORIZONTAL );
  SizerSC->Add( scSCO, 0, wxEXPAND | wxALL, Border );
  SizerSC->Add( cbUniform, 0, wxEXPAND | wxALL, Border );
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
  SizerSD->Add( stSCX, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 2  );
  SizerSD->Add( teSCX, 1, wxRIGHT, 15 );
  SizerSD->Add( stSCY, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 2 );
  SizerSD->Add( teSCY, 1, wxRIGHT, 15 );
  SizerSD->Add( stSCZ, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 2 );
  SizerSD->Add( teSCZ, 1, wxRIGHT, 2 );
  //end spot direction frame

  //spot direction frame + spot cut off frame
  wxBoxSizer *SizerS = new wxBoxSizer(wxHORIZONTAL );
  SizerS->Add( SizerSC, 1, wxEXPAND | wxALL, Border );
  SizerS->Add( SizerSD, 1, wxEXPAND | wxALL, Border );
  //end spot direction frame + spot cut off frame

  teAA = new TTextEdit(this);  //GL_QUADRATIC_ATTENUATION
  wxStaticText *stAA = new wxStaticText(this, -1, wxT("Quadratic"), wxDefaultPosition);
  teAB = new TTextEdit(this);  //GL_LINEAR_ATTENUATION
  wxStaticText *stAB = new wxStaticText(this, -1, wxT("Linear"), wxDefaultPosition);
  teAC = new TTextEdit(this);  //GL_CONSTANT_ATTENUATION
  wxStaticText *stAC = new wxStaticText(this, -1, wxT("Constant"), wxDefaultPosition);

  //frame attenuation
  wxStaticBox *BoxA = new wxStaticBox(this, -1, wxT("Attenuation"));
  wxStaticBoxSizer *SizerA = new wxStaticBoxSizer(BoxA, wxHORIZONTAL );
  SizerA->Add( stAA, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 2 );
  SizerA->Add( teAA, 1, wxRIGHT, 15 );
  SizerA->Add( stAB, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 2 );
  SizerA->Add( teAB, 1, wxRIGHT, 15 );
  SizerA->Add( stAC, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 2 );
  SizerA->Add( teAC, 1, wxRIGHT, 2 );
  //end frame attenuation
  
  wxStaticBox *Box1 = new wxStaticBox(this, -1, wxT("Light sources"));
  wxStaticBoxSizer *TSizer1 = new wxStaticBoxSizer(Box1, wxVERTICAL );
  TSizer1->Add( SizerLt, 0, wxEXPAND | wxALL, Border );//Light dropdown menu
  TSizer1->Add( TSizer0, 0, wxEXPAND | wxALL, Border );//Light frame + Light position frame
  TSizer1->Add( SizerS, 0, wxEXPAND | wxALL, Border );//spot cut off frame
  TSizer1->Add( SizerA, 0, wxEXPAND | wxALL, Border );//frame attenuation

  //light model frame
  cbFonts = new TComboBox(this);
  AGlScene& ascene = TGXApp::GetInstance().GetRender().GetScene();
  for( size_t i=0; i < ascene.FontCount(); i++ )
    cbFonts->AddObject( ascene.GetFont(i)->GetName(), ascene.GetFont(i) );
  cbFonts->SetSelection(0);
  cbFonts->OnChange.Add(this);
  tbEditFont = new TButton(this);  tbEditFont->SetCaption("Edit Font");  tbEditFont->OnClick.Add(this);

  cbLocalV = new wxCheckBox(this, -1, wxT("Local viewer"), wxDefaultPosition);
  cbTwoSide = new wxCheckBox(this, -1, wxT("Two side"), wxDefaultPosition);
  cbSmooth = new wxCheckBox(this, -1, wxT("Smooth shade"), wxDefaultPosition);
  tcAmbLM = new TTextEdit(this); tcAmbLM->SetReadOnly(true);  
    tcAmbLM->OnClick.Add(this);
  wxStaticText *stAmbLM = new wxStaticText(this, -1, wxT("Ambient color"), wxDefaultPosition);
  tcBgClr = new TTextEdit(this); tcBgClr->SetReadOnly(true);  
    tcBgClr->OnClick.Add(this);
  wxStaticText *stBgClr = new wxStaticText(this, -1, wxT("Background color"), wxDefaultPosition);

  wxStaticBox *Box2 = new wxStaticBox(this, -1, wxT("Light model"));
  wxStaticBoxSizer *SizerLM = new wxStaticBoxSizer(Box2, wxHORIZONTAL );

  wxGridSizer *SizerLM1 = new wxGridSizer(2, 2 );
  wxFlexGridSizer *SizerLM2 = new wxFlexGridSizer(2, 2 );

  SizerLM1->Add( cbLocalV, 0, wxALIGN_CENTER_VERTICAL | wxALL, 0 ); //light model second column
  SizerLM1->Add( cbTwoSide, 0, wxALIGN_CENTER_VERTICAL | wxALL, 0 );
  SizerLM1->Add( cbSmooth, 0, wxALIGN_CENTER_VERTICAL | wxALL, 0 );

  SizerLM2->Add( stAmbLM, 0, wxALL, 2 );//light model first column
  SizerLM2->Add( tcAmbLM, 0, wxEXPAND | wxALL, 1 );
  SizerLM2->Add( stBgClr, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 2 );
  SizerLM2->Add( tcBgClr, 0, wxEXPAND | wxALL, 1 );
  
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
  ButtonsSizer->Add( new wxButton( this, wxID_OPEN, wxT("Open") ), 0, wxEXPAND | wxALL, Border);
  ButtonsSizer->Add( new wxButton( this, wxID_SAVE, wxT("Save") ),     0, wxEXPAND | wxALL, Border );
  ButtonsSizer->Add( new wxButton( this, wxID_APPLY, wxT("Apply") ),     0, wxEXPAND | wxALL, Border );
  ButtonsSizer->Add( new wxButton( this, wxID_HELP, wxT("Help") ),     0, wxEXPAND | wxALL, Border );
  ButtonsSizer->SetSizeHints(this);
  //end right buttons list

  wxBoxSizer *TSizer3 = new wxBoxSizer(wxHORIZONTAL );
  TSizer3->Add( TSizer2, 1, wxALL, 5 );
  TSizer3->Add( ButtonsSizer, 0, wxALL, 5 );
  SetSizer(TSizer3);
  TSizer3->SetSizeHints(this);  

  Center();

  FCurrentLight = 0;
  FLightModel = TGXApp::GetInstance().GetRender().LightModel;
  FOriginalModel = TGXApp::GetInstance().GetRender().LightModel;
  InitLight(FLightModel.GetLight(0));
  InitLightModel(FLightModel);
}
//..............................................................................
TdlgSceneProps::~TdlgSceneProps()  {
  tbX->OnChange.Clear();
  tbY->OnChange.Clear();
  tbZ->OnChange.Clear();
  tbR->OnChange.Clear();
  teAmb->OnClick.Clear();
  teDiff->OnClick.Clear();
  teSpec->OnClick.Clear();
  scSCO->OnChange.Clear();
  cbLights->OnChange.Clear();
  tcAmbLM->OnClick.Clear();
  tcBgClr->OnClick.Clear();
  cbFonts->OnChange.Clear();
  tbEditFont->OnClick.Clear();
}
//..............................................................................
bool TdlgSceneProps::Execute(const IEObject* Sender, const IEObject* Data)  {
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
    UpdateLight(FLightModel.GetLight(FCurrentLight));
    int i = cbLights->FindString(cbLights->GetValue());
    if( i >= 0 )  {
      FCurrentLight = i;
      InitLight(FLightModel.GetLight(FCurrentLight));
    }
  }
  if( (TSpinCtrl*)Sender == scSCO )  {
    if( scSCO->GetValue() > 90 )  cbUniform->SetValue(true);
    else                       cbUniform->SetValue(false);
  }
  if( (TButton*)(AOlxCtrl*)Sender == tbEditFont )  {
    int sel = cbFonts->GetSelection();
    if( sel == -1 )  return false;
    TGXApp::GetInstance().GetRender().GetScene().ShowFontDialog( (TGlFont*)cbFonts->GetObject(sel) );
  }

  return true;
}
//..............................................................................
void TdlgSceneProps::InitLightModel(TGlLightModel& GlLM)  {
  cbLocalV->SetValue(GlLM.IsLocalViewer());
  cbTwoSide->SetValue(GlLM.IsTwoSides());
  cbSmooth->SetValue(GlLM.IsSmoothShade());
  tcAmbLM->WI.SetColor(GlLM.GetAmbientColor().GetRGB());
  tcBgClr->WI.SetColor(GlLM.GetClearColor().GetRGB());
}
//..............................................................................
void TdlgSceneProps::UpdateLightModel(TGlLightModel& GlLM)  {
  GlLM.SetLocalViewer(cbLocalV->GetValue());
  GlLM.SetTwoSides(cbTwoSide->GetValue());
  GlLM.SetSmoothShade(cbSmooth->GetValue());
  GlLM.SetAmbientColor(tcAmbLM->WI.GetColor());
  GlLM.SetClearColor(tcBgClr->WI.GetColor());
}
//..............................................................................
void TdlgSceneProps::InitLight(TGlLight& L)  {
  tbX->SetValue((int)L.GetPosition()[0]);  teX->SetText((int)L.GetPosition()[0]);
  tbY->SetValue((int)L.GetPosition()[1]);  teY->SetText((int)L.GetPosition()[1]);
  tbZ->SetValue((int)L.GetPosition()[2]);  teZ->SetText((int)L.GetPosition()[2]);
  tbR->SetValue((int)L.GetPosition()[3]);  teR->SetText((int)L.GetPosition()[3]);

  teAmb->WI.SetColor(L.GetAmbient().GetRGB());
  scAmbA->SetValue( (int)L.GetAmbient()[3]*100);
  teDiff->WI.SetColor(L.GetDiffuse().GetRGB());
  /*teDiff->SetColour(wxColour(GetRValue(L.Diffuse().GetRGB()), 
    GetGValue(L.Diffuse().GetRGB()), 
    GetBValue(L.Diffuse().GetRGB())));*/
  scDiffA->SetValue(L.GetDiffuse()[3]*100);
  teSpec->WI.SetColor(L.GetSpecular().GetRGB());
  scSpecA->SetValue((int)L.GetSpecular()[3]*100);
  teAA->SetText( L.GetAttenuation()[2] );
  teAB->SetText( L.GetAttenuation()[1] );
  teAC->SetText( L.GetAttenuation()[0] );
  cbEnabled->SetValue( L.IsEnabled() );
  scSExp->SetValue( L.GetSpotExponent() );
  scSCO->SetValue( L.GetSpotCutoff() );
  cbUniform->SetValue(L.GetSpotCutoff() == 180);
  teSCX->SetText( L.GetSpotDirection()[0] );
  teSCY->SetText( L.GetSpotDirection()[1] );
  teSCZ->SetText( L.GetSpotDirection()[2] );
}
//..............................................................................
void TdlgSceneProps::UpdateLight(TGlLight& L)  {
  L.SetPosition(TGlOption(tbX->GetValue(), tbY->GetValue(), tbZ->GetValue(), tbR->GetValue()));
  L.SetAmbient(teAmb->WI.GetColor() | (uint32_t)((256*scAmbA->GetValue()/100) << 24));
  L.SetDiffuse(teDiff->WI.GetColor() | (uint32_t)((256*scDiffA->GetValue()/100) << 24));
  L.SetSpecular(teSpec->WI.GetColor() | (uint32_t)((256*scSpecA->GetValue()/100) << 24));
  L.SetAttenuation(TGlOption(teAC->GetText().ToDouble(), teAB->GetText().ToDouble(), teAA->GetText().ToDouble()));
  L.SetEnabled(cbEnabled->GetValue());
  L.SetSpotExponent(scSExp->GetValue());
  L.SetSpotCutoff(cbUniform->GetValue() ? 180 : scSCO->GetValue());
  L.SetSpotDirection(TGlOption(teSCX->GetText().ToDouble(), teSCY->GetText().ToDouble(), teSCZ->GetText().ToDouble()));
}
//..............................................................................
void TdlgSceneProps::OnApply(wxCommandEvent& event)  {
  UpdateLight(FLightModel.GetLight(FCurrentLight));
  UpdateLightModel(FLightModel);
  TGXApp::GetInstance().GetRender().LightModel = FLightModel;
  TGXApp::GetInstance().GetRender().LoadIdentity();
  TGXApp::GetInstance().GetRender().InitLights();
  TGXApp::GetInstance().Draw();
}
//..............................................................................
void TdlgSceneProps::OnCancel(wxCommandEvent& event)  {
  TGXApp::GetInstance().GetRender().LightModel = FOriginalModel;
  TGXApp::GetInstance().GetRender().LoadIdentity();
  TGXApp::GetInstance().GetRender().InitLights();
  EndModal(wxID_OK);
}
//..............................................................................
void TdlgSceneProps::OnOK(wxCommandEvent& event)  {
  OnApply(event);
  EndModal(wxID_OK);
}
//..............................................................................
void TdlgSceneProps::OnOpen(wxCommandEvent& event)  {
  olxstr FN = Parent->PickFile("Load scene parameters",
    "Scene parameters|*.glsp", Parent->GetScenesFolder(), true);
  if( !FN.IsEmpty() )  {
    LoadFromFile(FLightModel, FN);
    Parent->SetScenesFolder(TEFile::ExtractFilePath(FN));
    InitLight(FLightModel.GetLight(FCurrentLight));
    InitLightModel(FLightModel);
  }
}
//..............................................................................
void TdlgSceneProps::OnSave(wxCommandEvent& event)  {
  olxstr FN = Parent->PickFile("Save scene parameters",
  "Scene parameters|*.glsp", Parent->GetScenesFolder(), false);
  if( !FN.IsEmpty() )  {
    UpdateLight(FLightModel.GetLight(FCurrentLight));
    Parent->SetScenesFolder(TEFile::ExtractFilePath(FN));
    UpdateLightModel(FLightModel);
    SaveToFile(FLightModel, FN);
  }
}
//..............................................................................
void TdlgSceneProps::LoadFromFile(TGlLightModel &FLM, const olxstr &FN)  {
  TDataFile F;
  F.LoadFromXLFile(FN, NULL);
  Parent->LoadScene(F.Root(), FLM);
}
//..............................................................................
void TdlgSceneProps::SaveToFile(TGlLightModel &FLM, const olxstr &FN)  {
  TDataFile DF;
  Parent->SaveScene(DF.Root(), FLM);
  try{  DF.SaveToXLFile(FN); }
  catch(...){  TBasicApp::GetLog().Error("Failed to save scene parameters!"); }
}
//..............................................................................

