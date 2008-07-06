//----------------------------------------------------------------------------//
// material properties dialog
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif
#include "matprop.h"

#include "wx/colordlg.h"
#include "wx/fontdlg.h"

#include "gpcollection.h"
#include "glgroup.h"
#include "xatom.h"
//..............................................................................
enum
{
  ID_COPY = 1,
  ID_PASTE,
  ID_EDITFONT
};
//..............................................................................
BEGIN_EVENT_TABLE(TdlgMatProp, TDialog)
  EVT_BUTTON(wxID_OK, TdlgMatProp::OnOK)
  EVT_BUTTON(ID_COPY, TdlgMatProp::OnCopy)
  EVT_BUTTON(ID_PASTE, TdlgMatProp::OnPaste)
  EVT_BUTTON(ID_EDITFONT, TdlgMatProp::OnEditFont)
END_EVENT_TABLE()
//..............................................................................
TGlMaterial TdlgMatProp::MaterialCopy;
//..............................................................................
TdlgMatProp::TdlgMatProp(TMainFrame *ParentFrame, TGPCollection *GPC, TGXApp *XApp)
:TDialog(ParentFrame, wxT("Material Parameters"), wxT("dlgMatProp"))
{
  AActionHandler::SetToDelete(false);
  bEditFont = NULL;
  if( GPC != NULL && GPC->ObjectCount() == 0 )
    throw TFunctionFailedException(__OlxSourceInfo, "empty collection");
  short Border = 3, SpW = 40;
  FSpinCtrls = new TEList;
  FParent = ParentFrame;
  GPCollection = GPC;
  FXApp = XApp;
  wxSize DefS(100, 21);
  if( GPC != NULL )  {
    if( EsdlInstanceOf( *GPC->Object(0), TXAtom) && false )  {
      FAtom = (TSAtom*)GPC->Object(0);
      cbApplyToGroup = new wxCheckBox(this, -1, wxT("Apply to Group"), wxDefaultPosition, DefS);
      cbApplyToGroup->SetValue(true);
    }
    else  {
      FAtom = NULL;
      cbApplyToGroup = NULL;
    }
  }
  else  {
    FAtom = NULL;
    cbApplyToGroup = NULL;
  }

  if( GPC != NULL && GPC->PrimitiveCount() )  {
    cbPrimitives = new TComboBox(this);
    TGlPrimitive *GlP;
    FMaterials = new TGlMaterial[GPC->PrimitiveCount()+1];
    for( int i=0; i < GPC->PrimitiveCount(); i++ )  {
      GlP = GPC->Primitive(i);
      cbPrimitives->AddObject(GlP->Name(), GlP);
      FMaterials[i] = *(TGlMaterial*)GlP->GetProperties();
    }
    cbPrimitives->SetValue( uiStr(GPC->Primitive(0)->Name()));
    cbPrimitives->OnChange->Add(this);
    FCurrentMaterial = 0;
  }
  else  {
    cbPrimitives = NULL;
    FMaterials = new TGlMaterial[1];
    if( GPC != NULL && EsdlInstanceOf(*GPC->Object(0), TGlGroup) )  {
      FMaterials[0] = *((TGlGroup*)GPC->Object(0))->GlM();
    }
    FCurrentMaterial = 0;
  }
  long flags = 0;wxCHK_3STATE|wxCHK_ALLOW_3RD_STATE_FOR_USER;
  cbAmbF = new wxCheckBox(this, -1, wxT("Ambient Front"), wxDefaultPosition, DefS, flags);
  scAmbF = new TSpinCtrl(this);  scAmbF->WI.SetWidth(SpW);  scAmbF->SetRange(0, 100);
  cbAmbB = new wxCheckBox(this, -1, wxT("Ambient Back"), wxDefaultPosition, DefS, flags);
  scAmbB = new TSpinCtrl(this);  scAmbB->WI.SetWidth(SpW);  scAmbB->SetRange(0, 100);
  cbDiffF = new wxCheckBox(this, -1, wxT("Diffuse Front"), wxDefaultPosition, DefS, flags);
  scDiffF = new TSpinCtrl(this);  scDiffF->WI.SetWidth(SpW);  scDiffF->SetRange(0, 100);
  cbDiffB = new wxCheckBox(this, -1, wxT("Diffuse Back"), wxDefaultPosition, DefS, flags);
  scDiffB = new TSpinCtrl(this);  scDiffB->WI.SetWidth(SpW);  scDiffB->SetRange(0, 100);
  cbEmmF = new wxCheckBox(this, -1, wxT("Emission Front"), wxDefaultPosition, DefS, flags);
  scEmmF = new TSpinCtrl(this);  scEmmF->WI.SetWidth(SpW);  scEmmF->SetRange(0, 100);
  cbEmmB = new wxCheckBox(this, -1, wxT("Emission Back"), wxDefaultPosition, DefS, flags);
  scEmmB = new TSpinCtrl(this);  scEmmB->WI.SetWidth(SpW);  scEmmB->SetRange(0, 100);
  cbSpecF = new wxCheckBox(this, -1, wxT("Specular Front"), wxDefaultPosition, DefS, flags);
  scSpecF = new TSpinCtrl(this);  scSpecF->WI.SetWidth(SpW);  scSpecF->SetRange(0, 100);
  cbSpecB = new wxCheckBox(this, -1, wxT("Specular Back"), wxDefaultPosition, DefS, flags);
  scSpecB = new TSpinCtrl(this);  scSpecB->WI.SetWidth(SpW);  scSpecB->SetRange(0, 100);
  cbShnF = new wxCheckBox(this, -1, wxT("Shininess Front"), wxDefaultPosition, DefS, flags);
  cbShnB = new wxCheckBox(this, -1, wxT("Shininess Back"), wxDefaultPosition, DefS, flags);

  FSpinCtrls->Add(scAmbF);  FSpinCtrls->Add(scAmbB);
  FSpinCtrls->Add(scDiffF);  FSpinCtrls->Add(scDiffB);
  FSpinCtrls->Add(scEmmF);  FSpinCtrls->Add(scEmmB);
  FSpinCtrls->Add(scSpecF);  FSpinCtrls->Add(scSpecB);

  cbTrans = new wxCheckBox(this, -1, wxT("Transluent"), wxDefaultPosition, DefS);
  scTrans = new TSpinCtrl(this);  scTrans->SetRange(5, 95);  scTrans->OnChange->Add(this);
  cbIDraw = new wxCheckBox(this, -1, wxT("Identity Draw"), wxDefaultPosition, DefS);

  tcAmbF = new TTextEdit(this);  tcAmbF->SetReadOnly(true);  tcAmbF->WI.SetWidth(34);  tcAmbF->WI.SetHeight(21);  tcAmbF->OnClick->Add(this);
  tcAmbB = new TTextEdit(this);  tcAmbB->SetReadOnly(true);  tcAmbB->WI.SetWidth(34);  tcAmbB->WI.SetHeight(21);  tcAmbB->OnClick->Add(this);
  tcDiffF = new TTextEdit(this); tcDiffF->SetReadOnly(true);  tcDiffF->WI.SetWidth(34);  tcDiffF->WI.SetHeight(21);  tcDiffF->OnClick->Add(this);
  tcDiffB = new TTextEdit(this); tcDiffB->SetReadOnly(true);  tcDiffB->WI.SetWidth(34);  tcDiffB->WI.SetHeight(21);  tcDiffB->OnClick->Add(this);
  tcEmmF = new TTextEdit(this);  tcEmmF->SetReadOnly(true);  tcEmmF->WI.SetWidth(34);  tcEmmF->WI.SetHeight(21);  tcEmmF->OnClick->Add(this);
  tcEmmB = new TTextEdit(this);  tcEmmB->SetReadOnly(true);  tcEmmB->WI.SetWidth(34);  tcEmmB->WI.SetHeight(21);  tcEmmB->OnClick->Add(this);
  tcSpecF = new TTextEdit(this); tcSpecF->SetReadOnly(true);  tcSpecF->WI.SetWidth(34);  tcSpecF->WI.SetHeight(21);  tcSpecF->OnClick->Add(this);
  tcSpecB = new TTextEdit(this); tcSpecB->SetReadOnly(true);  tcSpecB->WI.SetWidth(34);  tcSpecB->WI.SetHeight(21);  tcSpecB->OnClick->Add(this);
  tcShnF = new TTextEdit(this);  tcShnF->SetReadOnly(false);  tcShnF->WI.SetWidth(34);  tcShnF->WI.SetHeight(21);
  tcShnB = new TTextEdit(this);  tcShnB->SetReadOnly(false);  tcShnB->WI.SetWidth(34);  tcShnB->WI.SetHeight(21);

  wxBoxSizer *Sizer0 = NULL;
  if( cbPrimitives )  {
    Sizer0 = new wxBoxSizer( wxHORIZONTAL );
    Sizer0->Add( cbPrimitives, 0, wxALL, Border );
    if( cbApplyToGroup )
      Sizer0->Add( cbApplyToGroup, 0, wxALL, Border );
    Sizer0->Add( new wxButton( this, ID_COPY, wxT("Copy  Mat.") ), 0, wxALL, Border );
    Sizer0->Add( new wxButton( this, ID_PASTE, wxT("Paste Mat.") ), 0, wxALL, Border );
    bEditFont = new wxButton( this, ID_EDITFONT, wxT("Edit Font") );
    bEditFont->Enable( GPC->Primitive(0)->Font() != NULL );
    Sizer0->Add( bEditFont, 0, wxALL, Border );
  }

  wxBoxSizer *ASizer = new wxBoxSizer( wxHORIZONTAL );
  ASizer->Add( cbAmbF, 0, wxALL, Border );
  ASizer->Add( tcAmbF, 0, wxALL, Border );
  ASizer->Add( scAmbF, 0, wxALL, Border );
  ASizer->Add( cbAmbB, 0, wxALL, Border );
  ASizer->Add( tcAmbB, 0, wxALL, Border );
  ASizer->Add( scAmbB, 0, wxALL, Border );

  wxBoxSizer *BSizer = new wxBoxSizer( wxHORIZONTAL );
  BSizer->Add( cbDiffF, 0, wxALL, Border );
  BSizer->Add( tcDiffF, 0, wxALL, Border );
  BSizer->Add( scDiffF, 0, wxALL, Border );
  BSizer->Add( cbDiffB, 0, wxALL, Border );
  BSizer->Add( tcDiffB, 0, wxALL, Border );
  BSizer->Add( scDiffB, 0, wxALL, Border );

  wxBoxSizer *CSizer = new wxBoxSizer( wxHORIZONTAL );
  CSizer->Add( cbEmmF, 0, wxALL, Border );
  CSizer->Add( tcEmmF, 0, wxALL, Border );
  CSizer->Add( scEmmF, 0, wxALL, Border );
  CSizer->Add( cbEmmB, 0, wxALL, Border );
  CSizer->Add( tcEmmB, 0, wxALL, Border );
  CSizer->Add( scEmmB, 0, wxALL, Border );

  wxBoxSizer *DSizer = new wxBoxSizer( wxHORIZONTAL );
  DSizer->Add( cbSpecF, 0, wxALL, Border );
  DSizer->Add( tcSpecF, 0, wxALL, Border );
  DSizer->Add( scSpecF, 0, wxALL, Border );
  DSizer->Add( cbSpecB, 0, wxALL, Border );
  DSizer->Add( tcSpecB, 0, wxALL, Border );
  DSizer->Add( scSpecB, 0, wxALL, Border );

  wxBoxSizer *ESizer = new wxBoxSizer( wxHORIZONTAL );
  ESizer->Add( cbShnF, 0, wxALL, Border );
  ESizer->Add( tcShnF, 0, wxALL, Border );
  ESizer->Add( cbShnB, 0, wxALL, Border );
  ESizer->Add( tcShnB, 0, wxALL, Border );

  wxBoxSizer *FSizer = new wxBoxSizer( wxHORIZONTAL );
  FSizer->Add( cbTrans, 0, wxALL, Border );
  FSizer->Add( scTrans, 0, wxALL, Border );
  FSizer->Add( cbIDraw, 0, wxALL, Border );
  wxBoxSizer *ButtonsSizer = new wxBoxSizer( wxHORIZONTAL );

  ButtonsSizer->Add( new wxButton( this, wxID_OK, wxT("OK") ), 0, wxALL, Border);
  ButtonsSizer->Add( new wxButton( this, wxID_CANCEL, wxT("Cancel") ), 0, wxALL, Border);
  ButtonsSizer->Add( new wxButton( this, wxID_HELP, wxT("Help") ),     0, wxALL, Border );

  wxBoxSizer *TopSiser = new wxBoxSizer( wxVERTICAL );
  if( Sizer0 )  TopSiser->Add(Sizer0, 0, wxALL, 5);
  TopSiser->Add(ASizer, 0, wxALL, 5);
  TopSiser->Add(BSizer, 0, wxALL, 5);
  TopSiser->Add(CSizer, 0, wxALL, 5);
  TopSiser->Add(DSizer, 0, wxALL, 5);
  TopSiser->Add(ESizer, 0, wxALL, 5);
  TopSiser->Add(FSizer, 0, wxALL, 5);
  TopSiser->Add(ButtonsSizer, 0, wxALL, 10);

  SetSizer( TopSiser );      // use the sizer for layout

  TopSiser->SetSizeHints( this );   // set size hints to honour minimum size

  Center();

  Init(FMaterials[0]);

  FParent->RestorePosition(this);
}
//..............................................................................
TdlgMatProp::~TdlgMatProp()
{
  delete [] FMaterials;
  delete FSpinCtrls;
  if( cbPrimitives )  cbPrimitives->OnChange->Clear();
  scTrans->OnChange->Clear();
  tcAmbF->OnClick->Clear();
  tcAmbB->OnClick->Clear();
  tcDiffF->OnClick->Clear();
  tcDiffB->OnClick->Clear();
  tcEmmF->OnClick->Clear();
  tcEmmB->OnClick->Clear();
  tcSpecF->OnClick->Clear();
  tcSpecB->OnClick->Clear();
}
//..............................................................................
bool TdlgMatProp::Execute(const IEObject *Sender, const IEObject *Data)  {
  if( EsdlInstanceOf( *Sender, TTextEdit) )  {
    wxColourDialog *CD = new wxColourDialog(this);
    wxColor wc = ((TTextEdit*)Sender)->GetBackgroundColour();
    CD->GetColourData().SetColour(wc);
    if( CD->ShowModal() == wxID_OK )  {
      wc = CD->GetColourData().GetColour();
      ((TTextEdit*)Sender)->WI.SetColor(RGB(wc.Red(), wc.Green(), wc.Blue()) );
    }
    delete CD;
  }
  if( (TComboBox*)Sender == cbPrimitives )  {
    Update(FMaterials[FCurrentMaterial]);
    int i = cbPrimitives->FindString(cbPrimitives->GetValue());
    if( i >= 0 )  {
      FCurrentMaterial = i;
      Init(FMaterials[FCurrentMaterial]);
      const TGlPrimitive* glp = (const TGlPrimitive*)cbPrimitives->GetObject(i);
      bEditFont->Enable( glp->Font() != NULL );
    }
  }
  if( (TSpinCtrl*)Sender == scTrans )  {
    TSpinCtrl *SC;
    for( int i=0; i < FSpinCtrls->Count(); i++ )  {
      SC = (TSpinCtrl*)FSpinCtrls->Item(i);
      SC->SetValue(scTrans->GetValue());
    }
  }
  return true;
}
//..............................................................................
void TdlgMatProp::Init( const TGlMaterial &Glm )  {
  cbAmbF->SetValue(Glm.GetAmbientF());
  cbAmbB->SetValue(Glm.GetAmbientB());
  scAmbF->SetValue(Round(Glm.AmbientF.Data()[3]*100));
  scAmbB->SetValue( (int)Glm.AmbientB.Data()[3]*100);

  cbDiffF->SetValue(Glm.GetDiffuseF());
  cbDiffB->SetValue(Glm.GetDiffuseB());
  scDiffF->SetValue(Round(Glm.DiffuseF.Data()[3]*100));
  scDiffB->SetValue(Round(Glm.DiffuseB.Data()[3]*100));

  cbEmmF->SetValue(Glm.GetEmissionF());
  cbEmmB->SetValue(Glm.GetEmissionB());
  scEmmF->SetValue(Round(Glm.EmissionF.Data()[3]*100));
  scEmmB->SetValue(Round(Glm.EmissionB.Data()[3]*100));

  cbSpecF->SetValue(Glm.GetSpecularF());
  cbSpecB->SetValue(Glm.GetSpecularB());
  scSpecF->SetValue(Round(Glm.SpecularF.Data()[3]*100));
  scSpecB->SetValue(Round(Glm.SpecularB.Data()[3]*100));

  cbShnF->SetValue(Glm.GetShininessF());
  cbShnB->SetValue(Glm.GetShininessB());

  cbTrans->SetValue(Glm.GetTransparent());
  cbIDraw->SetValue(Glm.GetIdentityDraw());

  tcAmbF->WI.SetColor(Glm.AmbientF.GetRGB());
  tcAmbB->WI.SetColor(Glm.AmbientB.GetRGB());

  tcDiffF->WI.SetColor(Glm.DiffuseF.GetRGB());
  tcDiffB->WI.SetColor(Glm.DiffuseB.GetRGB());

  tcEmmF->WI.SetColor(Glm.EmissionF.GetRGB());
  tcEmmB->WI.SetColor(Glm.EmissionB.GetRGB());

  tcSpecF->WI.SetColor(Glm.SpecularF.GetRGB());
  tcSpecB->WI.SetColor(Glm.SpecularB.GetRGB());

  tcShnF->SetValue( uiStr(olxstr(Glm.ShininessF)));
  tcShnB->SetValue( uiStr(olxstr(Glm.ShininessB)));
}
//..............................................................................
void TdlgMatProp::Update(TGlMaterial &Glm)  {
  Glm.SetAmbientF(cbAmbF->GetValue());
  Glm.SetAmbientB(cbAmbB->GetValue());

  Glm.SetDiffuseF(cbDiffF->GetValue());
  Glm.SetDiffuseB(cbDiffB->GetValue());

  Glm.SetEmissionF(cbEmmF->GetValue());
  Glm.SetEmissionB(cbEmmB->GetValue());

  Glm.SetSpecularF(cbSpecF->GetValue());
  Glm.SetSpecularB(cbSpecB->GetValue());

  Glm.SetShininessF(cbShnF->GetValue());
  Glm.SetShininessB(cbShnB->GetValue());

  Glm.SetTransparent(cbTrans->GetValue());
  Glm.SetIdentityDraw(cbIDraw->GetValue());

  Glm.AmbientF = tcAmbF->WI.GetColor();
  Glm.AmbientF[3] = (double)scAmbF->GetValue()/100;
  Glm.AmbientB = tcAmbB->WI.GetColor();
  Glm.AmbientB[3] = (double)scAmbB->GetValue()/100;

  Glm.DiffuseF = tcDiffF->WI.GetColor();
  Glm.DiffuseF[3] = (double)scDiffF->GetValue()/100;
  Glm.DiffuseB = tcDiffB->WI.GetColor();
  Glm.DiffuseB[3] = (double)scDiffB->GetValue()/100;

  Glm.EmissionF = tcEmmF->WI.GetColor();
  Glm.EmissionF[3] = (double)scEmmF->GetValue()/100;
  Glm.EmissionB = tcEmmB->WI.GetColor();
  Glm.EmissionB[3] = (double)scEmmB->GetValue()/100;

  Glm.SpecularF = tcSpecF->WI.GetColor();
  Glm.SpecularF[3] = (double)scSpecF->GetValue()/100;
  Glm.SpecularB = tcSpecB->WI.GetColor();
  Glm.SpecularB[3] = (double)scSpecB->GetValue()/100;

  Glm.ShininessF = olxstr(tcShnF->GetValue().c_str()).ToInt();
  Glm.ShininessB = olxstr(tcShnB->GetValue().c_str()).ToInt();
}
//..............................................................................
void TdlgMatProp::OnOK(wxCommandEvent& event)
{
  Update(FMaterials[FCurrentMaterial]);
  TGraphicsStyle *GS;
  if( FAtom )
  {
/*    if( cbApplyToGroup->GetValue() )
    {
      AS = FXApp->GetStyle(FAtom, false);
      int i;
      TGlMaterial *GlM;
      for( i=0; i < FAtom->PrimitiveCount(); i++ )
      {
        AS->PrimitiveMaterial(FAtom->PrimitiveName(i), &FMaterials[i] );
        FAtom->Primitive(i)->SetProperties(&FMaterials[i]);
//        *((TGlMaterial*)FAtom->Primitive(i)->GetProperties()) = FMaterials[i];
      }
      FXApp->UpdateAtomStyle(FAtom, AS);
    }
    else
    {
      int i;
      FXApp->CreateAtom(FAtom); // will create an atom if necessary
      olxstr Tmp = FXApp->Render()->NameOfCollection(FAtom->Primitives());
      AS = FXApp->Render()->Styles()->Style(Tmp, true);
      for( i=0; i < FAtom->PrimitiveCount(); i++ )
      {
        AS->PrimitiveMaterial(FAtom->PrimitiveName(i), &FMaterials[i] );
        FAtom->Primitive(i)->SetProperties(&FMaterials[i]);
      }
      FXApp->UpdateAtomStyle(FAtom, AS);
    }  */
  }
  else  {
    if( GPCollection != NULL && EsdlInstanceOf(*GPCollection->Object(0), TGlGroup) )  {
      *((TGlGroup*)GPCollection->Object(0))->GlM() = FMaterials[0];
    }
    else  {
      if( GPCollection != NULL )  {
        GS = FXApp->GetRender().Styles()->Style( GPCollection->Name() );
        if( !GS )  GS = FXApp->GetRender().Styles()->NewStyle( GPCollection->Name() );
        for( int i=0; i < GPCollection->PrimitiveCount(); i++ )  {
          GPCollection->Primitive(i)->SetProperties(&FMaterials[i]);
          GS->PrimitiveMaterial( GPCollection->Primitive(i)->Name(), &FMaterials[i]);
        }
      }
    }
  }
//  FDrawObject->UpdatePrimitives(24);
  EndModal(wxID_OK);
}
//..............................................................................
void TdlgMatProp::OnCopy(wxCommandEvent& event)  {
  Update(FMaterials[FCurrentMaterial]);
  MaterialCopy = FMaterials[FCurrentMaterial];
}
//..............................................................................
void TdlgMatProp::OnPaste(wxCommandEvent& event)  {
//  FMaterials[FCurrentMaterial] = MaterialCopy;
  Init(MaterialCopy);
}
//..............................................................................
void TdlgMatProp::OnEditFont(wxCommandEvent& event)  {
  FXApp->GetRender().Scene()->ShowFontDialog( *GPCollection->Primitive(FCurrentMaterial)->Font() );
}
//..............................................................................

