/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "mainform.h"
#include "dgenopt.h"
#include "matprop.h"
#include "scenep.h"
#include "primtvs.h"
#include "ptable.h"
#include "xatom.h"
#include "xbond.h"
#include "xplane.h"
#include "xlattice.h"
#include "xgrid.h"
#include "xblob.h"
#include "glbackground.h"
#include "edit.h"

void TMainForm::OnHtmlPanel(wxCommandEvent& event)  {
  processMacro("htmlpanelvisible");
  if (!this->FHtmlMinimized)
    processMacro("html.update");
}
//..............................................................................
void TMainForm::OnGenerate(wxCommandEvent& WXUNUSED(event))  {
//  TBasicApp::GetLog()->Info("generate!");;
  TdlgGenerate *G = new TdlgGenerate(this);
  if( G->ShowModal() == wxID_OK )  {
    processMacro(olxstr("pack ").stream(' ') <<
      olxstr::FormatFloat(1, G->GetAFrom()) <<
      olxstr::FormatFloat(1, G->GetATo()) <<
      olxstr::FormatFloat(1, G->GetBFrom()) <<
      olxstr::FormatFloat(1, G->GetBTo()) <<
      olxstr::FormatFloat(1, G->GetCFrom()) <<
      olxstr::FormatFloat(1, G->GetCTo()));
  }
  G->Destroy();
}
//..............................................................................
void TMainForm::OnFileOpen(wxCommandEvent& event)  {
  if( event.GetId() >= ID_FILE0 && event.GetId() <= (ID_FILE0+FRecentFilesToShow) )
    processMacro(olxstr("reap \"") << FRecentFiles[event.GetId() - ID_FILE0] << '\"');
}
//..............................................................................
void TMainForm::OnDrawStyleChange(wxCommandEvent& event)  {
  switch( event.GetId() )  {
    case ID_DSBS: processMacro("pers", __OlxSrcInfo);  break;
    case ID_DSES: processMacro("telp", __OlxSrcInfo);  break;
    case ID_DSSP: processMacro("sfil", __OlxSrcInfo);  break;
    case ID_DSWF: processMacro("proj", __OlxSrcInfo);  break;
    case ID_DSST: processMacro("tubes", __OlxSrcInfo);  break;
    case ID_SceneProps:
      TdlgSceneProps *Dlg = new TdlgSceneProps(this);
      if( Dlg->ShowModal() == wxID_OK )  {
        FBgColor = FXApp->GetRender().LightModel.GetClearColor();
      }
      Dlg->Destroy();
    break;
  }
  TimePerFrame = FXApp->Draw();
}
//..............................................................................
void TMainForm::OnViewAlong(wxCommandEvent& event) {
  processMacro(olxstr("matr ") << pmView->GetLabel(event.GetId()));
}
//..............................................................................
void TMainForm::OnAtomOccuChange(wxCommandEvent& event)  {
  TXAtom *XA = dynamic_cast<TXAtom*>(FObjectUnderMouse);
  if (XA == NULL) return;
  olxstr tmp;
  if (event.GetId() == ID_AtomOccuCustom) {
    TdlgEdit *dlg = new TdlgEdit(this, false);
    dlg->SetTitle(wxT("Please input the occupancy to fix to"));
    double val = XA->CAtom().GetChemOccu();
    dlg->SetText(val);
    if (dlg->ShowModal() == wxID_OK) {
      try { val = dlg->GetText().ToDouble(); }
      catch (...) {}
    }
    dlg->Destroy();
    tmp << "fix occu " << val;
  }
  else {
    tmp = ((event.GetId() == ID_AtomOccuFree) ? "free " : "fix ");
    tmp << "occu ";
    double val = 0;
    switch (event.GetId()) {
    case ID_AtomOccu1:   val = 1;  break;
    case ID_AtomOccu34:  val = 0.75;  break;
    case ID_AtomOccu12:  val = 0.5;  break;
    case ID_AtomOccu13:  val = 1. / 3;  break;
    case ID_AtomOccu14:  val = 0.25;  break;
    case ID_AtomOccuFix:   break;
    case ID_AtomOccuFixCurrent:
      tmp = "fvar 1";  break;
    case ID_AtomOccuFree:  break;
    }
    if (val != 0)
      tmp << val;
  }
  if (XA->IsSelected())
    tmp << " sel";
  else
    tmp << " #c" << XA->CAtom().GetId();
  processMacro(tmp);
  TimePerFrame = FXApp->Draw();
}
//..............................................................................
void TMainForm::OnAtomConnChange(wxCommandEvent& event)  {
  TXAtom *XA = (TXAtom*)FObjectUnderMouse;
  if (XA == NULL) return;
  olxstr tmp = "conn ";
  if (event.GetId() == ID_AtomConnChangeLast) {
    TdlgEdit dlg(this, false);
    dlg.SetTitle(wxT("Please input the maximum number of bonds ")
      wxT("and optionally the bonding radius"));
    olxstr txt = XA->CAtom().GetConnInfo().maxBonds;
    txt << ' ' << XA->CAtom().GetConnInfo().r;
    dlg.SetText(txt);
    if (dlg.ShowModal() == wxID_OK) {
      tmp << dlg.GetText();
    }
    else {
      return;
    }
  }
  else {
    tmp << TStrList(pmAtomConn->GetLabel(event.GetId()), ' ')[0];
  }
  if (!XA->IsSelected())
    tmp << " #c" << XA->CAtom().GetId();
  processMacro(tmp);
  TimePerFrame = FXApp->Draw();
}
//..............................................................................
void TMainForm::OnAtomPolyChange(wxCommandEvent& event)  {
  TXAtom *XA = (TXAtom*)FObjectUnderMouse;
  if( XA == NULL )  return;
  olxstr Tmp("poly ");
  if( !XA->IsSelected() )
    Tmp << "#c" << XA->CAtom().GetId() << ' ';
  switch( event.GetId() )  {
    case ID_AtomPolyNone: Tmp << "none";  break;
    case ID_AtomPolyAuto: Tmp << "auto";  break;
    case ID_AtomPolyRegular: Tmp << "regular";  break;
    case ID_AtomPolyPyramid: Tmp << "pyramid";  break;
    case ID_AtomPolyBipyramid: Tmp << "bipyramid";  break;
  }
  processMacro(Tmp);
  TimePerFrame = FXApp->Draw();
}
//..............................................................................
void TMainForm::OnAtomPartChange(wxCommandEvent& event)  {
  TXAtom *XA = (TXAtom*)FObjectUnderMouse;
  if (XA == NULL) return;
  olxstr cmd;
  if (event.GetId() == ID_AtomPartChangeLast) {
    TdlgEdit dlg(this, false);
    dlg.SetTitle(wxT("Please input the part number"));
    int val=XA->CAtom().GetPart();
    dlg.SetText(val);
    if (dlg.ShowModal() == wxID_OK) {
      try { val = dlg.GetText().ToInt(); }
      catch(...) {}
    }
    if (val != XA->CAtom().GetPart()) {
      cmd << "part " << val;
    }
    else {
      return;
    }
  }
  else {
    cmd << "part " << pmAtomPart->GetLabel(event.GetId());
  }
  if (!cmd.IsEmpty()) {
    if( !XA->IsSelected() )
      cmd << " #c" << XA->CAtom().GetId();
    processMacro(cmd);
  }
  TimePerFrame = FXApp->Draw();
}
//..............................................................................
void TMainForm::OnAtomUisoChange(wxCommandEvent& event)  {
  TXAtom *XA = (TXAtom*)FObjectUnderMouse;
  if( XA == NULL )  return;
  olxstr cmd;
  switch( event.GetId() )  {
    case ID_AtomUiso15:
      cmd << "fix uiso -1.5";  break;
    case ID_AtomUiso12:
      cmd << "fix uiso -1.2";  break;
    case ID_AtomUisoFree:
      cmd << "free uiso";  break;
    case ID_AtomUisoCustom:
    case ID_AtomUisoFix:
      {
        TdlgEdit *dlg = new TdlgEdit(this, false);
        dlg->SetTitle(wxT("Please give the Uiso value to fix"));
        dlg->SetText(XA->CAtom().GetUiso());
        double val=0;
        if (dlg->ShowModal() == wxID_OK) {
          try { val = dlg->GetText().ToDouble(); }
          catch(...) {}
        }
        dlg->Destroy();
        if (val != 0) {
          if (event.GetId() == ID_AtomUisoFix)
            cmd << "fix uiso " << val;
          else {
            try {
              TXAtomPList atoms = FXApp->GetSelection().Extract<TXAtom>();
              for (size_t i = 0; i < atoms.Count(); i++) {
                FXApp->SetAtomUiso(*atoms[i], val);
              }
            }
            catch (const TExceptionBase &e) {
              TBasicApp::NewLogEntry(logException) << e;
            }
          }
        }
      }
      break;
  }
  if (!cmd.IsEmpty()) {
    if( !XA->IsSelected() )
      cmd << " #c" << XA->CAtom().GetId();
    processMacro(cmd);
  }
  TimePerFrame = FXApp->Draw();
}
//..............................................................................
void TMainForm::OnDrawQChange(wxCommandEvent& event)  {
  switch( event.GetId() )  {
    case ID_DQH:  processMacro("qual 3");  break;
    case ID_DQM:  processMacro("qual 2");  break;
    case ID_DQL:  processMacro("qual 1");  break;
  }
}
//..............................................................................
void TMainForm::CellVChange()  {
  TStateRegistry::GetInstance().SetState(FXApp->stateCellVisible,
    FXApp->IsCellVisible(), EmptyString(), true);
  pmModel->SetLabel(ID_CellVisible, (!FXApp->IsCellVisible() ? wxT("Show cell")
    : wxT("Hide cell")));
}
//..............................................................................
void TMainForm::BasisVChange()  {
  TStateRegistry::GetInstance().SetState(FXApp->stateBasisVisible,
    FXApp->IsBasisVisible(), EmptyString(), true);
  pmModel->SetLabel(ID_BasisVisible,
    (FXApp->IsBasisVisible() ? wxT("Hide basis") : wxT("Show basis")));
}
//..............................................................................
void TMainForm::GridVChange()  {
  TStateRegistry::GetInstance().SetState(FXApp->stateXGridVisible,
    FXApp->IsGridVisible(), EmptyString(), true);
}
//..............................................................................
void TMainForm::FrameVChange()  {
  TStateRegistry::GetInstance().SetState(FXApp->stateWBoxVisible,
    FXApp->Get3DFrame().IsVisible(), EmptyString(), true);
}
//..............................................................................
void TMainForm::OnCellVisible(wxCommandEvent& event)  {
  FXApp->SetCellVisible(!FXApp->IsCellVisible());
}
//..............................................................................
void TMainForm::OnBasisVisible(wxCommandEvent& event)  {
  FXApp->SetBasisVisible(!FXApp->IsBasisVisible());
}
//..............................................................................
void TMainForm::OnGraphics(wxCommandEvent& event)  {
  if (FObjectUnderMouse == NULL) return;

  if (event.GetId() == ID_GraphicsHide) {
    if (FObjectUnderMouse->IsSelected())
      processMacro("hide sel");
    else
      FXApp->GetUndo().Push(FXApp->SetGraphicsVisible(FObjectUnderMouse, false));
    TimePerFrame = FXApp->Draw();
  }
  else if (event.GetId() == ID_GraphicsKill) {
    if( FObjectUnderMouse->IsSelected() )
      processMacro("kill sel");
    else  {
      AGDObjList l;
      l.Add(FObjectUnderMouse);
      FXApp->GetUndo().Push(FXApp->DeleteXObjects(l));
    }
    TimePerFrame = FXApp->Draw();
  }
  else if (event.GetId() == ID_GraphicsEdit) {
    if( LabelToEdit != NULL )  {
      olxstr Tmp = "getuserinput(1, \'Please, enter new label\', \'";
      Tmp << LabelToEdit->GetLabel() << "\')";
      if ( processFunction(Tmp.Replace('$', "\\$")) && !Tmp.IsEmpty()) {
        LabelToEdit->SetLabel(Tmp);
        FXApp->Draw();
      }
      LabelToEdit = NULL;
    }
  }
  else if (event.GetId() == ID_GraphicsDS) {
    TGlGroup& Sel = FXApp->GetSelection();
    TdlgMatProp* MatProp = new TdlgMatProp(this, *FObjectUnderMouse);
    if (EsdlInstanceOf(*FObjectUnderMouse, TGlGroup))
      MatProp->SetCurrent(((TGlGroup*)FObjectUnderMouse)->GetGlM());
    if (MatProp->ShowModal() == wxID_OK)  {
      if (EsdlInstanceOf(*FObjectUnderMouse, TXAtom))
        ;//FXApp->SynchroniseBonds((TXAtom*)FObjectUnderMouse);
    }
    MatProp->Destroy();
    TimePerFrame = FXApp->Draw();
  }
  else if (event.GetId() == ID_GraphicsSelect) {
    if (FObjectUnderMouse->IsSelected()) {
      sorted::PointerPointer<TGPCollection> colls;
      TGlGroup& sel = FXApp->GetSelection();
      for (size_t i=0; i < sel.Count(); i++) {
        TGPCollection& gpc = sel.GetObject(i).GetPrimitives();
        if (colls.AddUnique(&gpc).b) {
          for (size_t j = 0; j < gpc.ObjectCount(); j++) {
            AGDrawObject &go = gpc.GetObject(j);
            if (go.IsVisible())
              FXApp->GetRender().Select(go, true);
          }
        }
      }
    }
    else {
      for (size_t i = 0; i < FObjectUnderMouse->GetPrimitives().ObjectCount(); i++) {
        AGDrawObject &go = FObjectUnderMouse->GetPrimitives().GetObject(i);
        if (go.IsVisible())
          FXApp->GetRender().Select(go, true);
      }
    }
    TimePerFrame = FXApp->Draw();
  }
  else if (event.GetId() == ID_GraphicsP) {
    TStrList Ps;
    FObjectUnderMouse->ListPrimitives(Ps);
    if (Ps.IsEmpty())  {
      TBasicApp::NewLogEntry() << "The object does not support requested function...";
      return;
    }
    TdlgPrimitive* Primitives = new TdlgPrimitive(this, *FObjectUnderMouse);
    if (Primitives->ShowModal() == wxID_OK) {
      if (EsdlInstanceOf(*FObjectUnderMouse, TXBond)) {
        TXBondPList bonds;
        if (FObjectUnderMouse->IsSelected())  {
          for (size_t i = 0; i < FXApp->GetSelection().Count(); i++) {
            if (EsdlInstanceOf(FXApp->GetSelection()[i], TXBond))
              bonds.Add((TXBond&)FXApp->GetSelection()[i]);
          }
        }
        else
          bonds.Add((TXBond*)FObjectUnderMouse);
        FXApp->Individualise(bonds, Primitives->Level, Primitives->Mask);
      }
      else if (EsdlInstanceOf(*FObjectUnderMouse, TXAtom)) {
        TXAtomPList atoms;
        if (FObjectUnderMouse->IsSelected())  {
          for (size_t i = 0; i < FXApp->GetSelection().Count(); i++) {
            if (EsdlInstanceOf(FXApp->GetSelection()[i], TXAtom))
              atoms.Add((TXAtom&)FXApp->GetSelection()[i]);
          }
        }
        else
          atoms.Add((TXAtom*)FObjectUnderMouse);
        FXApp->Individualise(atoms, Primitives->Level, Primitives->Mask);
      }
      else  {
        olxstr TmpStr = "mask ";
        TmpStr << FObjectUnderMouse->GetPrimitives().GetName() << ' ' << Primitives->Mask;
        processMacro(TmpStr);
      }
    }
    Primitives->Destroy();
    TimePerFrame = FXApp->Draw();
  }
  else if (event.GetId() == ID_FixLattice) {
    if (EsdlInstanceOf(*FObjectUnderMouse, TXLattice))
      ((TXLattice*)FObjectUnderMouse)->SetFixed(true);
  }
  else if (event.GetId() == ID_FreeLattice) {
    if (EsdlInstanceOf(*FObjectUnderMouse, TXLattice))
      ((TXLattice*)FObjectUnderMouse)->SetFixed(false);
  }
  else if (event.GetId() == ID_GridMenuCreateBlob) {
    TXBlob* xb = FXApp->XGrid().CreateBlob(MousePositionX, MousePositionY);
    if (xb != NULL)
      FXApp->AddObjectToCreate(xb);
    xb->Create();
  }
  else if (event.GetId() == ID_GraphicsCollectivise) {
    if (FObjectUnderMouse->IsSelected()) {
      if (EsdlInstanceOf(*FObjectUnderMouse, TXAtom)) {
        FXApp->Collectivise(
          FXApp->GetSelection().Extract<TXAtom>().GetObject());
      }
      else if (EsdlInstanceOf(*FObjectUnderMouse, TXBond)) {
        FXApp->Collectivise(
          FXApp->GetSelection().Extract<TXBond>().GetObject());
      }
    }
    else {
      if (EsdlInstanceOf(*FObjectUnderMouse, TXAtom)) {
        FXApp->Collectivise(*(TXAtom*)FObjectUnderMouse);
      }
      else if (EsdlInstanceOf(*FObjectUnderMouse, TXBond)) {
        FXApp->Collectivise(*(TXBond*)FObjectUnderMouse);
      }
    }
  }
  else if (event.GetId() == ID_GraphicsIndividualise) {
    if (FObjectUnderMouse->IsSelected()) {
      if (EsdlInstanceOf(*FObjectUnderMouse, TXAtom)) {
        FXApp->Individualise(
          FXApp->GetSelection().Extract<TXAtom>().GetObject());
      }
      else if (EsdlInstanceOf(*FObjectUnderMouse, TXBond)) {
        FXApp->Individualise(
          FXApp->GetSelection().Extract<TXBond>().GetObject());
      }
    }
    else {
      if (EsdlInstanceOf(*FObjectUnderMouse, TXAtom)) {
        FXApp->Individualise(*(TXAtom*)FObjectUnderMouse);
      }
      else if (EsdlInstanceOf(*FObjectUnderMouse, TXBond)) {
        FXApp->Individualise(*(TXBond*)FObjectUnderMouse);
      }
    }
  }
}
//..............................................................................
void TMainForm::ObjectUnderMouse(AGDrawObject *G)  {
  FObjectUnderMouse = G;
  FCurrentPopup = NULL;
  if (G == NULL)  return;
  FCurrentPopup = NULL;
  if (EsdlInstanceOf(*G, TXAtom)) {
    TXAtom *XA = (TXAtom*)G;
    // bang
    TStrList SL = FXApp->BangList(*XA);
    pmBang->Clear();
    for (size_t i = 0; i < SL.Count(); i++)
      pmBang->Append(-1, SL[i].u_str());
    pmAtom->Enable(ID_MenuBang, SL.Count() != 0);
    olxstr T = XA->GetLabel();
    // label + basic peak/occu
    T << ':' << ' ' << XA->GetType().name;
    if (XA->GetType() == iQPeakZ) {
      T << ": " << olxstr::FormatFloat(3, XA->CAtom().GetQPeak());
    }
    else {
      T << " Occu: " << TEValueD(XA->CAtom().GetOccu(),
        XA->CAtom().GetOccuEsd()).ToString();
    }
    pmAtom->SetLabel(ID_AtomInfo, T.u_str());
    pmAtom->Enable(ID_AtomGrow, !XA->IsGrown());
    pmAtom->Enable(ID_Selection, G->IsSelected() &&
      EsdlInstanceOf(*G->GetParentGroup(), TGlGroup));
    pmAtom->Enable(ID_SelGroup, false);
    {
      int level = TXAtom::LegendLevel(XA->GetPrimitives().GetName());
      pmAtom->Enable(ID_GraphicsCollectivise, level > 0);
      pmAtom->Enable(ID_GraphicsIndividualise, level < 2);
    }
    // occu
    {
      double occu = XA->CAtom().GetChemOccu();
      XVarReference *sof = XA->CAtom().GetVarRef(catom_var_name_Sof);
      if (sof != 0 && sof->relation_type == relation_None) {
        pmAtomOccu->SetLabel(ID_AtomOccuFix, wxString("Fix"));
        if (olx_feq(occu, 1.0)) {
          pmAtomOccu->Check(ID_AtomOccu1, true);
        }
        else if (olx_feq(occu, 0.75)) {
          pmAtomOccu->Check(ID_AtomOccu34, true);
        }
        else if (olx_feq(occu, 0.5)) {
          pmAtomOccu->Check(ID_AtomOccu12, true);
        }
        else if (olx_feq(occu, 1. / 3)) {
          pmAtomOccu->Check(ID_AtomOccu13, true);
        }
        else if (olx_feq(occu, 0.25)) {
          pmAtomOccu->Check(ID_AtomOccu14, true);
        }
        else {
          pmAtomOccu->SetLabel(ID_AtomOccuFix, wxString("Fixed at: ") << occu);
          pmAtomOccu->Check(ID_AtomOccuFix, true);
        }
        pmAtomOccu->Enable(ID_AtomOccuFix, false);
        pmAtomOccu->Enable(ID_AtomOccuFixCurrent, false);
        pmAtomOccu->Enable(ID_AtomOccuFree, true);
        pmAtomOccu->SetLabel(ID_AtomOccuFree, wxString("Free"));
      }
      else {
        pmAtomOccu->Enable(ID_AtomOccuFix, true);
        pmAtomOccu->SetLabel(ID_AtomOccuFix, wxString("Fix"));
        pmAtomOccu->Enable(ID_AtomOccuFixCurrent, true);

        pmAtomOccu->Check(ID_AtomOccuFree, true);
        pmAtomOccu->Enable(ID_AtomOccuFree, false);
        pmAtomOccu->SetLabel(ID_AtomOccuFree, wxString("Free: ") << occu);
      }
    }
    // conn
    {
      short bonds = XA->CAtom().GetConnInfo().maxBonds;
      pmAtom->SetLabel(ID_AtomConnChangeLast, wxString("Custom..."));
      if (bonds >= 0 && bonds <= 4) {
        pmAtomConn->Check(ID_AtomConnChange+bonds, true);
      }
      else if (bonds == 12) {
        pmAtomConn->Check(ID_AtomConnChange+5, true);
      }
      else if (bonds == 24) {
        pmAtomConn->Check(ID_AtomConnChange+6, true);
      }
      else {
        pmAtomConn->Check(ID_AtomConnChangeLast, true);
        pmAtom->SetLabel(ID_AtomConnChangeLast,
          wxString("Custom: ") << bonds);
      }
    }
    // poly
    size_t bound_cnt = 0;
    for (size_t i = 0; i < XA->NodeCount(); i++) {
      if (XA->Node(i).IsDeleted() || XA->Node(i).GetType().z < 2)  // H,D,Q
        continue;
      bound_cnt++;
    }
    pmAtom->Enable(ID_MenuAtomPoly, bound_cnt > 3);
    if (bound_cnt > 3)
      pmAtom->Check(ID_AtomPolyNone + XA->GetPolyhedronType(), true);
    if (XA->CAtom().GetPart() >= -2 && XA->CAtom().GetPart() <= 2) {
      pmAtom->Check(ID_AtomPartChange + XA->CAtom().GetPart()+2, true);
      pmAtom->SetLabel(ID_AtomPartChangeLast, wxT("Custom..."));
    }
    else {
      pmAtom->Check(ID_AtomPartChangeLast, true);
      pmAtom->SetLabel(ID_AtomPartChangeLast, (olxstr("Custom: ") <<
        (int)XA->CAtom().GetPart()).u_str());
    }
    // Uiso
    if (XA->CAtom().GetEllipsoid() == NULL) {
      size_t ac = 0;
      for (size_t i = 0; i < XA->CAtom().AttachedSiteCount(); i++) {
        TCAtom &a = XA->CAtom().GetAttachedAtom(i);
        if (!a.IsDeleted() && a.GetType() != iQPeakZ)
          ac++;
      }
      pmAtom->Enable(ID_AtomUiso12, ac == 1);
      pmAtom->Enable(ID_AtomUiso15, ac == 1);
      pmAtom->SetLabel(ID_AtomUisoCustom, wxT("Custom..."));
      pmAtom->SetLabel(ID_AtomUisoFree, wxT("Free"));
      pmAtom->SetLabel(ID_AtomUisoFix, wxT("Fix"));
      pmAtom->Enable(ID_MenuAtomUiso, true);
      if (XA->CAtom().GetUisoOwner() != NULL) {
        if (XA->CAtom().GetUisoScale() == 1.5)
          pmAtom->Check(ID_AtomUiso15, true);
        else if (XA->CAtom().GetUisoScale() == 1.2)
          pmAtom->Check(ID_AtomUiso12, true);
        else {
          pmAtom->Check(ID_AtomUisoCustom, true);
          pmAtom->SetLabel(ID_AtomUisoCustom,
            (olxstr("Custom: ") << XA->CAtom().GetUisoScale() << " x U(" <<
            XA->CAtom().GetUisoOwner()->GetLabel() << ')').u_str());
        }
      }
      else if (XA->CAtom().GetVarRef(catom_var_name_Uiso) == NULL) {
        pmAtom->Check(ID_AtomUisoFree, true);
        pmAtom->SetLabel(ID_AtomUisoFree, (olxstr("Free: ") <<
          olxstr::FormatFloat(4, XA->CAtom().GetUiso())).u_str());
      }
      else if (XA->CAtom().GetVarRef(catom_var_name_Uiso) != NULL &&
        XA->CAtom().GetVarRef(catom_var_name_Uiso)->relation_type == relation_None)
      {
        pmAtom->SetLabel(ID_AtomUisoFix, (olxstr("Fixed: ") <<
          XA->CAtom().GetUiso()).u_str());
        pmAtom->Check(ID_AtomUisoFix, true);
      }
    }
    else {
      pmAtom->Enable(ID_MenuAtomUiso, false);
    }
    FCurrentPopup = pmAtom;
  }
  else if (EsdlInstanceOf(*G, TXBond))  {
    olxstr T;
    TXBond *XB = (TXBond*)G;
    {
      int level = TXAtom::LegendLevel(XB->GetPrimitives().GetName());
      pmBond->Enable(ID_GraphicsCollectivise, level > 0);
      pmBond->Enable(ID_GraphicsIndividualise, level < 2);
    }
    TStrList SL = FXApp->TangList(XB);
    pmTang->Clear();
    for (size_t i = 0; i < SL.Count(); i++)
      pmTang->Append(-1, SL[i].u_str());

    pmBond->Enable(ID_MenuTang, SL.Count() != 0);
    T = XB->A().GetLabel();
    T << '-' << XB->B().GetLabel() << ':' << ' '
      << olxstr::FormatFloat(3, XB->Length()) << 'A';
    pmBond->SetLabel(ID_BondInfo, T.u_str());
    pmBond->Enable(ID_Selection, G->IsSelected() &&
      EsdlInstanceOf(*G->GetParentGroup(), TGlGroup));
    pmBond->SetLabel(ID_BondRadius, wxString("Radius: ") << XB->GetRadius());
    FCurrentPopup = pmBond;
  }
  else if (EsdlInstanceOf(*G, TXPlane))  {
    pmPlane->Enable(ID_Selection, G->IsSelected() &&
      EsdlInstanceOf(*G->GetParentGroup(), TGlGroup));
    FCurrentPopup = pmPlane;
  }
  if (FCurrentPopup != NULL)  {
    FCurrentPopup->Enable(ID_SelGroup, false);
    FCurrentPopup->Enable(ID_SelUnGroup, false);
    if (FXApp->GetSelection().Count() > 1)  {
      FCurrentPopup->Enable(ID_SelGroup, true);
    }
    if (FXApp->GetSelection().Count() == 1)  {
      if (EsdlInstanceOf(FXApp->GetSelection().GetObject(0), TGlGroup))  {
        FCurrentPopup->Enable(ID_SelUnGroup, true);
      }
    }
    olxstr tt = FXApp->GetObjectInfoAt(MousePositionX, MousePositionY);
    FCurrentPopup->Enable(ID_SelLabel, !tt.IsEmpty());
  }
  if (EsdlInstanceOf(*G, TGlGroup))  {
    pmSelection->Enable(ID_SelGroup, G->IsSelected() && FXApp->GetSelection().Count() > 1);
    pmSelection->Enable(ID_SelUnGroup, true);
    FCurrentPopup = pmSelection;
  }
  else if (EsdlInstanceOf(*G, TGlBackground))  {
    FCurrentPopup = pmMenu;
  }
  else if (EsdlInstanceOf(*G, TXGlLabel))  {
    FCurrentPopup = pmLabel;
    LabelToEdit = (TXGlLabel*)G;
  }
  else if (EsdlInstanceOf(*G, TXLattice))  {
    FCurrentPopup = pmLattice;
  }
#ifdef _DEBUG
  else if (EsdlInstanceOf(*G, TXGrid))  {
    FCurrentPopup = pmGrid;
  }
#endif
}
//..............................................................................
void TMainForm::OnAtomTypeChange(wxCommandEvent& event)  {
  TXAtom *XA = (TXAtom*)FObjectUnderMouse;
  if (XA == NULL)  return;
  olxstr Tmp("name ");
  if (XA->IsSelected())
    Tmp << "sel";
  else
    Tmp << "#s" << XA->GetOwnerId();
  Tmp << ' ';
  if (event.GetId() == ID_AtomTypeChangeLast) {
    TPTableDlg Dlg(this);
    if (Dlg.ShowModal() == wxID_OK)  {
      Tmp << Dlg.GetSelected()->symbol;
    }
    else {
      return;
    }
  }
  else {
    Tmp << pmAtomType->GetLabel(event.GetId());
  }
  processMacro(Tmp);
  TimePerFrame = FXApp->Draw();
}
//..............................................................................
size_t TMainForm::GetFragmentList(TNetPList& res)  {
  if( FObjectUnderMouse == NULL )  return 0;
  if( FObjectUnderMouse->IsSelected() )  {
    TGlGroup& glg = FXApp->GetSelection();
    for( size_t i=0; i < glg.Count(); i++ )  {
      if( EsdlInstanceOf(glg[i], TXAtom) )
        res.Add(((TXAtom&)glg[i]).GetNetwork());
      else if( EsdlInstanceOf(glg[i], TXBond) )
        res.Add(((TXBond&)glg[i]).GetNetwork());
    }
    for( size_t i=0; i < res.Count(); i++ )
      res[i]->SetTag(i);
    for( size_t i=0; i < res.Count(); i++ )
      if( res[i]->GetTag() != i )
        res[i] = NULL;
    res.Pack();
  }
  else  {
    if( EsdlInstanceOf(*FObjectUnderMouse, TXAtom) )
      res.Add(((TXAtom*)FObjectUnderMouse)->GetNetwork());
    else if( EsdlInstanceOf(*FObjectUnderMouse, TXBond) )
      res.Add(((TXBond*)FObjectUnderMouse)->GetNetwork());
  }
  return res.Count();
}
//..............................................................................
void TMainForm::OnFragmentHide(wxCommandEvent& event)  {
  if( FObjectUnderMouse == NULL )  return;
  TNetPList L;
  if( GetFragmentList(L) == 0 )
    return;
  FXApp->FragmentsVisible(L, false);
  //FXApp->CenterView();
  TimePerFrame = FXApp->Draw();
}
//..............................................................................
void TMainForm::OnFragmentShowOnly(wxCommandEvent& event)  {
  if( FObjectUnderMouse == NULL )  return;
  TNetPList L;
  if( GetFragmentList(L) == 0 )  return;
  FXApp->FragmentsVisible(FXApp->InvertFragmentsList(L), false);
  FXApp->CenterView(true);
  TimePerFrame = FXApp->Draw();
}
//..............................................................................
void TMainForm::OnFragmentSelectAtoms(wxCommandEvent& event)  {
  TNetPList L;
  if( GetFragmentList(L) == 0 )  return;
  FXApp->SelectFragmentsAtoms(L, true);
  TimePerFrame = FXApp->Draw();
}
//..............................................................................
void TMainForm::OnFragmentSelectBonds(wxCommandEvent& event)  {
  TNetPList L;
  if( GetFragmentList(L) == 0 )  return;
  FXApp->SelectFragmentsBonds(L, true);
  TimePerFrame = FXApp->Draw();
}
//..............................................................................
void TMainForm::OnFragmentSelectAll(wxCommandEvent& event)  {
  TNetPList L;
  if( GetFragmentList(L) == 0 )  return;
  FXApp->SelectFragments(L, true);
  TimePerFrame = FXApp->Draw();
}
//..............................................................................
void TMainForm::OnShowAll(wxCommandEvent& event)  {
  processMacro("fmol");
}
//..............................................................................
void TMainForm::OnModelCenter(wxCommandEvent& event)  {
  processMacro("center");
  TimePerFrame = FXApp->Draw();
}
//..............................................................................
void TMainForm::OnAtom(wxCommandEvent& event)  {
  if( FObjectUnderMouse == NULL )  return;
  TXAtom *XA = (TXAtom*)FObjectUnderMouse;
  if( event.GetId() == ID_AtomGrow )
    processMacro(olxstr("grow #s") << XA->GetOwnerId());
  else if( event.GetId() == ID_AtomSelRings )
    processMacro(olxstr("sel rings *#s") << XA->GetOwnerId());
  else if( event.GetId() == ID_AtomCenter )  {
    if( !XA->IsSelected() )
      processMacro(olxstr("center #s") << XA->GetOwnerId());
    else
      processMacro("center");  // center of the selection
  }
  TimePerFrame = FXApp->Draw();
}
//..............................................................................
void TMainForm::OnPlane(wxCommandEvent& event)  {
  TXPlane *XP = dynamic_cast<TXPlane*>(FObjectUnderMouse);
  if( XP == NULL )  return;
  if( event.GetId() == ID_PlaneActivate )  {
    const vec3d& n = XP->GetNormal();
    const vec3d& c = XP->GetCenter();
    processMacro(olxstr("SetView -c ") << n[0] << ' ' << n[1] << ' ' << n[2]
     << ' ' << c[0] << ' ' << c[1] << ' ' << c[2]);
  }
}
//..............................................................................
void TMainForm::OnBond(wxCommandEvent& event)  {
  TXBond *xb = dynamic_cast<TXBond*>(FObjectUnderMouse);
  if (xb == NULL) return;
  if (event.GetId() == ID_BondViewAlong) {
    const vec3d n = (xb->B().crd()-xb->A().crd()).Normalise();
    const vec3d c = (xb->B().crd()+xb->A().crd())/2;
    processMacro(olxstr("SetView -c ") << n[0] << ' ' << n[1] << ' ' << n[2]
     << ' ' << c[0] << ' ' << c[1] << ' ' << c[2]);
  }
  else if (event.GetId() == ID_BondRadius) {
    TdlgEdit *dlg = new TdlgEdit(this, false);
    dlg->SetTitle(wxT("Please input the bond radius. By default it will be applied")
      wxT(" to the current bond group [by atom type], use 'individualise' and ")
      wxT("'collectivise' change this."));
    olxstr txt = xb->GetRadius();
    dlg->SetText(txt);
    if (dlg->ShowModal() == wxID_OK) {
      double r = dlg->GetText().ToDouble();
      TXBondPList bl;
      if (xb->IsSelected()) {
        bl = FXApp->GetSelection().Extract<TXBond>();
      }
      else {
        bl << xb;
      }
      FXApp->BondRad(r, &bl);
      FXApp->Draw();
    }
    dlg->Destroy();
  }
}
//..............................................................................
void TMainForm::OnSelection(wxCommandEvent& m)  {
  if( m.GetId() == ID_SelGroup )
    processMacro("group");
  else if( m.GetId() == ID_SelUnGroup )  {
    TGlGroup *GlR = NULL;
    if (FObjectUnderMouse != NULL && EsdlInstanceOf(*FObjectUnderMouse, TGlGroup))
      FXApp->Ungroup(*((TGlGroup*)FObjectUnderMouse));
    else
      FXApp->UngroupSelection();
  }
  else if( m.GetId() == ID_SelLabel )  {
    vec3d cent;
    size_t cnt = 0;
    TGlGroup& gl = FXApp->GetSelection();
    for( size_t i=0; i < gl.Count(); i++ )  {
      if( EsdlInstanceOf(gl[i], TXAtom) )  {
        cent += ((TXAtom&)gl[i]).crd();
        cnt++;
      }
      else if( EsdlInstanceOf(gl[i], TXBond) ) {
        cent += ((TXBond&)gl[i]).GetCenter();
        cnt++;
      }
      else if( EsdlInstanceOf(gl[i], TXPlane) ) {
        cent += ((TXPlane&)gl[i]).GetCenter();
        cnt++;
      }
    }
    if( cnt != 0 )
      cent /= cnt;
    FXApp->CreateLabel(cent,
      FXApp->GetObjectInfoAt(MousePositionX, MousePositionY), 4);
    TimePerFrame = FXApp->Draw();
  }
}
//..............................................................................
void TMainForm::OnGraphicsStyle(wxCommandEvent& event)  {
  if( event.GetId() == ID_GStyleSave )  {
    olxstr FN = PickFile("Drawing style",
    "Drawing styles|*.glds", StylesDir, EmptyString(), false);
    if( !FN.IsEmpty() )
      processMacro(olxstr("save style ") << FN);
  }
  if( event.GetId() == ID_GStyleOpen )  {
    olxstr FN = PickFile("Drawing style",
    "Drawing styles|*.glds", StylesDir, EmptyString(), true);
    if( !FN.IsEmpty() )
      processMacro(olxstr("load style ") << FN);
  }
}
//..............................................................................
