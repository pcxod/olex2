#include "mainform.h"
#include "dgenopt.h"
#include "matprop.h"
#include "scenep.h"
#include "primtvs.h"
#include "ptable.h"
#include "obase.h"
#include "xatom.h"
#include "xbond.h"
#include "xplane.h"
#include "xlattice.h"
#include "xgrid.h"
#include "xblob.h"
#include "glbackground.h"

void TMainForm::OnHtmlPanel(wxCommandEvent& event)  {
  ProcessMacro("htmlpanelvisible");
  ProcessMacro("html.updatehtml");
}
//..............................................................................
void TMainForm::OnGenerate(wxCommandEvent& WXUNUSED(event))  {
//  TBasicApp::GetLog()->Info("generate!");;
  TdlgGenerate *G = new TdlgGenerate(this);
  if( G->ShowModal() == wxID_OK )  {
    olxstr T("pack ");
    T << olxstr::FormatFloat(1, G->GetAFrom()) << ' ' << olxstr::FormatFloat(1, G->GetATo()) << ' ';
    T << olxstr::FormatFloat(1, G->GetBFrom()) << ' ' << olxstr::FormatFloat(1, G->GetBTo()) << ' ';
    T << olxstr::FormatFloat(1, G->GetCFrom()) << ' ' << olxstr::FormatFloat(1, G->GetCTo()) << ' ';
    ProcessMacro(T);
  }
  G->Destroy();
}
//..............................................................................
void TMainForm::OnFileOpen(wxCommandEvent& event)  {
  if( event.GetId() >= ID_FILE0 && event.GetId() <= (ID_FILE0+FRecentFilesToShow) )
    ProcessMacro(olxstr("reap \'") << FRecentFiles[event.GetId() - ID_FILE0] << '\'');
}
//..............................................................................
void TMainForm::OnDrawStyleChange(wxCommandEvent& event)  {
  switch( event.GetId() )  {
    case ID_DSBS: ProcessMacro("pers", __OlxSrcInfo);  break;
    case ID_DSES: ProcessMacro("telp", __OlxSrcInfo);  break;
    case ID_DSSP: ProcessMacro("sfil", __OlxSrcInfo);  break;
    case ID_DSWF: ProcessMacro("proj", __OlxSrcInfo);  break;
    case ID_DSST: ProcessMacro("tubes", __OlxSrcInfo);  break;
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
void TMainForm::OnViewAlong(wxCommandEvent& event) {
  switch( event.GetId() )  {
    case ID_View100:  ProcessMacro("matr 1");  break;
    case ID_View010:  ProcessMacro("matr 2");  break;
    case ID_View001:  ProcessMacro("matr 3");  break;
    case ID_View110:  ProcessMacro("matr 110");  break;
    case ID_View101:  ProcessMacro("matr 101");  break;
    case ID_View011:  ProcessMacro("matr 011");  break;
    case ID_View111:  ProcessMacro("matr 111");  break;
  }
}
//..............................................................................
void TMainForm::OnAtomOccuChange(wxCommandEvent& event)  {
  TXAtom *XA = (TXAtom*)FObjectUnderMouse;
  if( XA == NULL )  return;
  olxstr Tmp = ((event.GetId() == ID_AtomOccuFix) ? "fix " : 
                (event.GetId() == ID_AtomOccuFree) ? "free " : "fix ");
  Tmp << "occu ";
  switch( event.GetId() )  {
    case ID_AtomOccu1:   Tmp << "1";  break;
    case ID_AtomOccu34:  Tmp << "0.75";  break;
    case ID_AtomOccu12:  Tmp << "0.5";  break;
    case ID_AtomOccu13:  Tmp << "0.33333";  break;
    case ID_AtomOccu14:  Tmp << "0.25";  break;
    case ID_AtomOccuFix:   break;
    case ID_AtomOccuFree:  break;
  }
  if( XA->IsSelected() )  
    Tmp << " sel";
  else                  
    Tmp << " #c" << XA->Atom().CAtom().GetId();
  ProcessMacro(Tmp);
  TimePerFrame = FXApp->Draw();
}
//..............................................................................
void TMainForm::OnAtomConnChange(wxCommandEvent& event)  {
  TXAtom *XA = (TXAtom*)FObjectUnderMouse;
  if( XA == NULL )  return;
  olxstr Tmp("conn ");
  Tmp << ' ';
  switch( event.GetId() )  {
    case ID_AtomConn0:   Tmp << '0';  break;
    case ID_AtomConn1:   Tmp << '1';  break;
    case ID_AtomConn2:   Tmp << '2';  break;
    case ID_AtomConn3:   Tmp << '3';  break;
    case ID_AtomConn4:   Tmp << '4';  break;
    case ID_AtomConn12:  Tmp << def_max_bonds;  break;
  }
  if( !XA->IsSelected() )
    Tmp << " #c" << XA->Atom().CAtom().GetId();
  ProcessMacro(Tmp);
  TimePerFrame = FXApp->Draw();
}
//..............................................................................
void TMainForm::OnAtomPolyChange(wxCommandEvent& event)  {
  TXAtom *XA = (TXAtom*)FObjectUnderMouse;
  if( XA == NULL )  return;
  switch( event.GetId() )  {
    case ID_AtomPolyNone:       XA->SetPolyhedronType(polyNone);  break;
    case ID_AtomPolyAuto:       XA->SetPolyhedronType(polyAuto);  break;
    case ID_AtomPolyRegular:    XA->SetPolyhedronType(polyRegular);  break;
    case ID_AtomPolyPyramid:    XA->SetPolyhedronType(polyPyramid);  break;
    case ID_AtomPolyBipyramid:  XA->SetPolyhedronType(polyBipyramid);  break;
  }
  TimePerFrame = FXApp->Draw();
}
//..............................................................................
void TMainForm::OnDrawQChange(wxCommandEvent& event)  {
  switch( event.GetId() )  {
    case ID_DQH:  ProcessMacro("qual -h");     break;
    case ID_DQM:  ProcessMacro("qual -m");     break;
    case ID_DQL:  ProcessMacro("qual -l");     break;
  }
}
//..............................................................................
void TMainForm::CellVChange()  {
  TStateChange sc(prsCellVis, FXApp->IsCellVisible());
  pmModel->SetLabel(ID_CellVisible, (!FXApp->IsCellVisible() ? wxT("Show cell") : wxT("Hide cell")));
  OnStateChange.Execute((AEventsDispatcher*)this, &sc);
}
//..............................................................................
void TMainForm::BasisVChange()  {
  TStateChange sc(prsBasisVis, FXApp->IsBasisVisible());
  pmModel->SetLabel(ID_BasisVisible, (FXApp->IsBasisVisible() ? wxT("Hide basis") : wxT("Show basis")));
  OnStateChange.Execute((AEventsDispatcher*)this, &sc);
}
//..............................................................................
void TMainForm::GridVChange()  {
  TStateChange sc(prsGridVis, FXApp->XGrid().IsVisible());
  OnStateChange.Execute((AEventsDispatcher*)this, &sc);
}
//..............................................................................
void TMainForm::FrameVChange()  {
  TStateChange sc(prsGridVis, FXApp->Get3DFrame().IsVisible());
  OnStateChange.Execute((AEventsDispatcher*)this, &sc);
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
  if( FObjectUnderMouse == NULL )  return;

  if( event.GetId() == ID_GraphicsHide )  {
    if( FObjectUnderMouse->IsSelected() )
      ProcessMacro("hide sel");
    else
      FUndoStack->Push(FXApp->SetGraphicsVisible(FObjectUnderMouse, false));
    TimePerFrame = FXApp->Draw();
  }
  else if( event.GetId() == ID_GraphicsKill )  {
    if( FObjectUnderMouse->IsSelected() )
      ProcessMacro("kill sel");
    else  {
      AGDObjList l;
      l.Add(FObjectUnderMouse);
      FUndoStack->Push(FXApp->DeleteXObjects(l));
    }
    TimePerFrame = FXApp->Draw();
  }
  else if( event.GetId() == ID_GraphicsEdit )  {
    if( LabelToEdit != NULL )  {
      olxstr Tmp = "getuserinput(1, \'Please, enter new label\', \'";
      Tmp << LabelToEdit->GetLabel() << "\')";
      ProcessFunction(Tmp);
      if( !Tmp.IsEmpty() ) {
        LabelToEdit->SetLabel(Tmp);
        FXApp->Draw();
      }
      LabelToEdit = NULL;
    }
  }
  else if( event.GetId() == ID_GraphicsDS )  {
    TGlGroup& Sel = FXApp->GetSelection();
    TdlgMatProp* MatProp = new TdlgMatProp(this, *FObjectUnderMouse);
    if( EsdlInstanceOf(*FObjectUnderMouse, TGlGroup) )
      MatProp->SetCurrent(((TGlGroup*)FObjectUnderMouse)->GetGlM());
    if( MatProp->ShowModal() == wxID_OK )  {
      if( EsdlInstanceOf( *FObjectUnderMouse, TXAtom) )
        FXApp->XAtomDS2XBondDS("Sphere");  
    }
    MatProp->Destroy();
    TimePerFrame = FXApp->Draw();
  }
  else if( event.GetId() == ID_GraphicsSelect )  {
    if( FObjectUnderMouse->IsSelected() )  {
      SortedPtrList<TGPCollection, TPrimitiveComparator> colls;
      TGlGroup& sel = FXApp->GetSelection();
      for( size_t i=0; i < sel.Count(); i++ )  {
        TGPCollection& gpc = sel.GetObject(i).GetPrimitives();
        if( colls.AddUnique(&gpc) )  {
          for( size_t j=0; j < gpc.ObjectCount(); j++ )
            FXApp->GetRender().Select(gpc.GetObject(j), true);
        }
      }
    }
    else  {
      for( size_t i=0; i < FObjectUnderMouse->GetPrimitives().ObjectCount(); i++ )
        FXApp->GetRender().Select(FObjectUnderMouse->GetPrimitives().GetObject(i), true);
    }
    TimePerFrame = FXApp->Draw();
  }
  else if( event.GetId() == ID_GraphicsP )  {
    TStrList Ps;
    FObjectUnderMouse->ListPrimitives(Ps);
    if( Ps.IsEmpty() )  {
      TBasicApp::GetLog() << "The object does not support requested function...\n";
      return;
    }
    TdlgPrimitive* Primitives = new TdlgPrimitive(this, *FObjectUnderMouse);
    if( Primitives->ShowModal() == wxID_OK )  {
      if( EsdlInstanceOf(*FObjectUnderMouse, TXBond) )  {
        TXBondPList bonds;
        if( FObjectUnderMouse->IsSelected() )  {
          for( size_t i=0; i < FXApp->GetSelection().Count(); i++ )  {
            if( EsdlInstanceOf(FXApp->GetSelection()[i], TXBond) )
              bonds.Add((TXBond&)FXApp->GetSelection()[i]);
          }
        }
        else
          bonds.Add((TXBond*)FObjectUnderMouse);
        FXApp->Individualise(bonds, Primitives->Level, Primitives->Mask);
      }
      else if( EsdlInstanceOf(*FObjectUnderMouse, TXAtom) )  {
        TXAtomPList atoms;
        if( FObjectUnderMouse->IsSelected() )  {
          for( size_t i=0; i < FXApp->GetSelection().Count(); i++ )  {
            if( EsdlInstanceOf(FXApp->GetSelection()[i], TXAtom) )
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
        ProcessMacro(TmpStr);
      }
    }
    Primitives->Destroy();
    TimePerFrame = FXApp->Draw();
  }
  else if( event.GetId() == ID_FixLattice )  {
    if( EsdlInstanceOf(*FObjectUnderMouse, TXLattice) )
      ((TXLattice*)FObjectUnderMouse)->SetFixed(true);
  }
  else if( event.GetId() == ID_FreeLattice )  {
    if( EsdlInstanceOf(*FObjectUnderMouse, TXLattice) )
      ((TXLattice*)FObjectUnderMouse)->SetFixed(false);
  }
  else if( event.GetId() == ID_GridMenuCreateBlob )  {
    TXBlob* xb = FXApp->XGrid().CreateBlob(MousePositionX, MousePositionY);
    if( xb != NULL )
      FXApp->AddObjectToCreate(xb);
    xb->Create();
  }
}
//..............................................................................
void TMainForm::ObjectUnderMouse(AGDrawObject *G)  {
  FObjectUnderMouse = G;
  FCurrentPopup = NULL;
  if( G == NULL )  return;
  FCurrentPopup = NULL;
  if( EsdlInstanceOf(*G, TXAtom) )  {
    TStrList SL;
    TXAtom *XA = (TXAtom*)G;
    FXApp->BangList(XA, SL);
    pmBang->Clear();
    for( size_t i=0; i < SL.Count(); i++ )
      pmBang->Append(-1, SL[i].u_str());
    pmAtom->Enable(ID_MenuBang, SL.Count() != 0);
    olxstr T = XA->Atom().GetLabel();
    T << ':' << ' ' <<  XA->Atom().GetType().name;
    if( XA->Atom().GetType() == iQPeakZ )  {
      T << ": " << olxstr::FormatFloat(3, XA->Atom().CAtom().GetQPeak());
    }
    else 
      T << " Occu: " << TEValueD(XA->Atom().CAtom().GetOccu(), XA->Atom().CAtom().GetOccuEsd()).ToString();
    miAtomInfo->SetText(T.u_str());
    pmAtom->Enable(ID_AtomGrow, !XA->Atom().IsGrown());
    pmAtom->Enable(ID_Selection, G->IsSelected() && EsdlInstanceOf(*G->GetParentGroup(), TGlGroup));
    pmAtom->Enable(ID_SelGroup, false);
    size_t bound_cnt = 0;
    for( size_t i=0; i < XA->Atom().NodeCount(); i++ )  {
      if( XA->Atom().Node(i).IsDeleted() || XA->Atom().Node(i).GetType().GetMr() < 3.5 )  // H,D,Q
        continue;
      bound_cnt++;
    }
    pmAtom->Enable(ID_MenuAtomPoly, bound_cnt > 3);
    if( bound_cnt > 3 )
      pmAtom->Check(ID_AtomPolyNone + XA->GetPolyhedronType(), true);
    FCurrentPopup = pmAtom;
  }
  else if( EsdlInstanceOf(*G, TXBond) )  {
    TStrList SL;
    olxstr T;
    TXBond *XB = (TXBond*)G;
    FXApp->TangList(XB, SL);
    pmTang->Clear();
    for( size_t i=0; i < SL.Count(); i++ )
      pmTang->Append(0, SL[i].u_str());

    pmBond->Enable(ID_MenuTang, SL.Count() != 0);
    T = XB->Bond().A().GetLabel();
    T << '-' << XB->Bond().B().GetLabel() << ':' << ' '
      << olxstr::FormatFloat(3, XB->Bond().Length());
    miBondInfo->SetText(T.u_str());
    pmBond->Enable(ID_Selection, G->IsSelected() && EsdlInstanceOf(*G->GetParentGroup(), TGlGroup));
    FCurrentPopup = pmBond;
  }
  else if( EsdlInstanceOf(*G, TXPlane) )  {
    pmPlane->Enable(ID_Selection, G->IsSelected() && EsdlInstanceOf(*G->GetParentGroup(), TGlGroup));
    FCurrentPopup = pmPlane;
  }
  if( FCurrentPopup != NULL )  {
    FCurrentPopup->Enable(ID_SelGroup, false);
    FCurrentPopup->Enable(ID_SelUnGroup, false);
    if( FXApp->GetSelection().Count() > 1 )  {
      FCurrentPopup->Enable(ID_SelGroup, true);
    }
    if( FXApp->GetSelection().Count() == 1 )  {
      if( EsdlInstanceOf(FXApp->GetSelection().GetObject(0), TGlGroup) )  {
        FCurrentPopup->Enable(ID_SelUnGroup, true);
      }
    }
    AquireTooltipValue();
    FCurrentPopup->Enable(ID_SelLabel, !Tooltip.IsEmpty());
  }
  if( EsdlInstanceOf(*G, TGlGroup) )  {
    pmSelection->Enable(ID_SelGroup, G->IsSelected() && FXApp->GetSelection().Count() > 1);
    pmSelection->Enable(ID_SelUnGroup, true);
    FCurrentPopup = pmSelection;
  }
  else if( EsdlInstanceOf( *G, TGlBackground) )  {
    FCurrentPopup = pmMenu;
  }
  else if( EsdlInstanceOf( *G, TXGlLabel) )  {
    FCurrentPopup = pmLabel;
    LabelToEdit = (TXGlLabel*)G;
  }
  else if( EsdlInstanceOf( *G, TXLattice) )  {
    FCurrentPopup = pmLattice;
  }
#ifdef _DEBUG
  else if( EsdlInstanceOf(*G, TXGrid) )  {
    FCurrentPopup = pmGrid;
  }
#endif
}
//..............................................................................
void TMainForm::OnAtomTypeChange(wxCommandEvent& event)  {
  TXAtom *XA = (TXAtom*)FObjectUnderMouse;
  if( XA == NULL )  return;
  olxstr Tmp("name -c ");
  if( XA->IsSelected() )  
    Tmp << "sel";
  else                  
    Tmp << "#x" << XA->GetXAppId();
  Tmp << ' ';
  switch( event.GetId() )  {
    case ID_AtomTypeChangeC:
      Tmp << 'C';
      break;
    case ID_AtomTypeChangeN:
      Tmp << 'N';
      break;
    case ID_AtomTypeChangeO:
      Tmp << 'O';
      break;
    case ID_AtomTypeChangeF:
      Tmp << 'F';
      break;
    case ID_AtomTypeChangeH:
      Tmp << 'H';
      break;
    case ID_AtomTypeChangeS:
      Tmp << 'S';
      break;
  }
  ProcessMacro(Tmp);
  TimePerFrame = FXApp->Draw();
}
//..............................................................................
void TMainForm::OnAtomTypePTable(wxCommandEvent& event)  {
  TXAtom *XA = (TXAtom*)FObjectUnderMouse;
  if( XA == NULL )  return;
  olxstr Tmp = "name ";
  if( XA->IsSelected() )  
    Tmp << "sel";
  else                  
    Tmp << "#x" << XA->GetXAppId();
  Tmp << ' ';
  TPTableDlg *Dlg = new TPTableDlg(this);
  if( Dlg->ShowModal() == wxID_OK )  {
    Tmp << Dlg->GetSelected()->symbol;
    ProcessMacro(Tmp);
  }
  Dlg->Destroy();
  TimePerFrame = FXApp->Draw();
}
//..............................................................................
size_t TMainForm::GetFragmentList(TNetPList& res)  {
  if( FObjectUnderMouse == NULL )  return 0;
  if( FObjectUnderMouse->IsSelected() )  {
    TGlGroup& glg = FXApp->GetSelection();
    for( size_t i=0; i < glg.Count(); i++ )  {
      if( EsdlInstanceOf(glg[i], TXAtom) )
        res.Add(((TXAtom&)glg[i]).Atom().GetNetwork());
      else if( EsdlInstanceOf(glg[i], TXBond) )
        res.Add(((TXBond&)glg[i]).Bond().GetNetwork());
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
      res.Add(((TXAtom*)FObjectUnderMouse)->Atom().GetNetwork());
    else if( EsdlInstanceOf(*FObjectUnderMouse, TXBond) )
      res.Add(((TXBond*)FObjectUnderMouse)->Bond().GetNetwork());
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
  TNetPList L, L1;
  if( GetFragmentList(L) == 0 )  return;
  FXApp->InvertFragmentsList(L, L1);
  FXApp->FragmentsVisible(L1, false);
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
  ProcessMacro("fmol");
}
//..............................................................................
void TMainForm::OnModelCenter(wxCommandEvent& event)  {
  FXApp->CenterModel();
  TimePerFrame = FXApp->Draw();
}
//..............................................................................
void TMainForm::OnAtom(wxCommandEvent& event)  {
  if( FObjectUnderMouse == NULL )  return;
  TXAtom *XA = (TXAtom*)FObjectUnderMouse;
  if( event.GetId() == ID_AtomGrow )
    ProcessMacro(olxstr("grow #x") << XA->GetXAppId());
  else if( event.GetId() == ID_AtomSelRings )  {
    TTypeList<TSAtomPList> rings;
    XA->Atom().GetNetwork().FindAtomRings(XA->Atom(), rings);
    if( !rings.IsEmpty() )  {
      TXAtomPList xatoms;
      for( size_t i=0; i < rings.Count(); i++ )
        FXApp->SAtoms2XAtoms(rings[i], xatoms);
      for( size_t i=0; i < xatoms.Count(); i++ )  {
        if( !xatoms[i]->IsSelected() )
          FXApp->GetRender().Select(*xatoms[i]);
      }
      TimePerFrame = FXApp->Draw();
    }
  }
  else if( event.GetId() == ID_AtomCenter )  {
    if( !XA->IsSelected() )
      ProcessMacro(olxstr("center #x") << XA->GetXAppId());
    else
      ProcessMacro("center");  // center of the selection
    TimePerFrame = FXApp->Draw();
  }
}
//..............................................................................
void TMainForm::OnPlane(wxCommandEvent& event)  {
  TXPlane *XP = (TXPlane*)FObjectUnderMouse;
  if( !XP )  return;
  switch( event.GetId() )  {
    case ID_PlaneActivate:
    ProcessMacro(olxstr("activate ") << XP->GetPrimitives().GetName());
    break;
  }
}
//..............................................................................
void TMainForm::OnSelection(wxCommandEvent& m)  {
  if( m.GetId() == ID_SelGroup )
      ProcessMacro("group");
  else if( m.GetId() == ID_SelUnGroup )  {
  TGlGroup *GlR = NULL;
  if( FObjectUnderMouse != NULL && EsdlInstanceOf(*FObjectUnderMouse, TGlGroup) )
      FXApp->UnGroup(*((TGlGroup*)FObjectUnderMouse));
    else      
      FXApp->UnGroupSelection();
  }
  else if( m.GetId() == ID_SelLabel )  {
    vec3d cent;
    size_t cnt = 0;
    TGlGroup& gl = FXApp->GetSelection();
    for( size_t i=0; i < gl.Count(); i++ )  {
      if( EsdlInstanceOf(gl[i], TXAtom) )  {
        cent += ((TXAtom&)gl[i]).Atom().crd();
        cnt++;
      }
      else if( EsdlInstanceOf(gl[i], TXBond) ) {
        cent += ((TXBond&)gl[i]).Bond().GetCenter();
        cnt++;
      }
      else if( EsdlInstanceOf(gl[i], TXPlane) ) {
        cent += ((TXPlane&)gl[i]).Plane().GetCenter();
        cnt++;
      }
    }
    if( cnt != 0 )
      cent /= cnt;
    FXApp->CreateLabel(cent, Tooltip, 4);
    TimePerFrame = FXApp->Draw();
  }
}
//..............................................................................
void TMainForm::OnGraphicsStyle(wxCommandEvent& event)  {
  if( event.GetId() == ID_GStyleSave )  {
    olxstr FN = PickFile("Drawing style",
    "Drawing styles|*.glds", StylesDir, false);
    if( !FN.IsEmpty() )
      ProcessMacro(olxstr("save style ") << FN);
  }
  if( event.GetId() == ID_GStyleOpen )  {
    olxstr FN = PickFile("Drawing style",
    "Drawing styles|*.glds", StylesDir, true);
    if( !FN.IsEmpty() )
      ProcessMacro(olxstr("load style ") << FN);
  }
}
//..............................................................................
