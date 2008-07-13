	//----------------------------------------------------------------------------//
// main frame of the application
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "wx/protocol/http.h"

#include "mainform.h"
#include "xglcanv.h"
#include "xglapp.h"

#include "wx/clipbrd.h"
#include "wx/filesys.h"
#include "wx/cursor.h"
#include "wx/colordlg.h"
#include "wx/fontdlg.h"

#include "wx/image.h"
#include "wx/dcps.h"
#include "evaln.h"
#include "imagep.h"

#include "dgrad.h"
#include "edit.h"
#include "updateoptions.h"
#include "ptable.h"

#include "symmlib.h"
#include "bitarray.h"
#include "glbackground.h"
#include "glgroup.h"
#include "gpcollection.h"

#include "idistribution.h"
#include "ipattern.h"
#include "chnexp.h"

#include "xatom.h"
#include "xbond.h"
#include "xplane.h"
#include "xline.h"
#include "xreflection.h"
#include "gllabel.h"
#include "gllabels.h"
#include "dunitcell.h"

#include "symmparser.h"

#include "efile.h"

#include "ins.h"
#include "cif.h"
#include "hkl.h"
#include "p4p.h"
#include "mol.h"
#include "crs.h"

#include "fsext.h"
#include "htmlext.h"

#include <iostream>

#include "pyext.h"

#include "ellipsoid.h"

#include "obase.h"
#include "glbitmap.h"
#include "bitarray.h"
#include "arrays.h"
#include "sgtest.h"
#include "network.h"

#include "filesystem.h"
#include "wxhttpfs.h"

#include "ecast.h"
#include "sls.h"

#include "cmdline.h"

#include "xlcongen.h"

#include "tls.h"
#include "ecast.h"

#include "scat_it.h"
#include "arrays.h"
#include "estrbuffer.h"
#include "unitcell.h"
#include "xgrid.h"
#include "symmtest.h"
#include "xlattice.h"

#include "matprop.h"
#include "utf8file.h"

#include "msgbox.h"

#include "ebtree.h"
#include "dusero.h"
//#include "gl2ps.h"

#ifdef __GNUC__
  #ifdef Bool
    #undef Bool
  #endif
  #ifdef Success
    #undef Success
  #endif
#endif

#include "olxmps.h"
#include "ecomplex.h"
#include "sr_fft.h"

#include "olxvar.h"

#include "ecast.h"
#include "atomref.h"
#include "wxglscene.h"

using namespace _xl_Controls;

static const olxstr NAString("n/a");
static const olxstr StartMatchCBName("startmatch");
static const olxstr OnMatchCBName("onmatch");
static const olxstr NoneString("none");

int CalcL( int v )  {
  int r = 0;
  while( (v/=2) > 2 )  r++;
  return r+2;
}

//olex::IBasicMacroProcessor *olex::OlexPort::MacroProcessor;

//..............................................................................
void TMainForm::funFileLast(const TStrObjList& Params, TMacroError &E)  {
  int index = 0;
  if( FXApp->XFile().GetLastLoader() )  index = 1;
  if( FRecentFiles.Count() <= index )  {
    E.ProcessingError(__OlxSrcInfo, "no last file");
    return;
  }
  if( !TEFile::FileExists(FRecentFiles[index]) )  {
    E.ProcessingError(__OlxSrcInfo, "file does not exists anymore");
    return;
  }
  E.SetRetVal( FRecentFiles[index] );
}
//..............................................................................
void TMainForm::funCell(const TStrObjList& Params, TMacroError &E)  {
  if( !Params[0].Comparei("a") )
    E.SetRetVal( FXApp->XFile().GetAsymmUnit().Axes()[0].GetV() );
  else if( !Params[0].Comparei("b") )
    E.SetRetVal( FXApp->XFile().GetAsymmUnit().Axes()[1].GetV() );
  else if( !Params[0].Comparei("c") )
    E.SetRetVal( FXApp->XFile().GetAsymmUnit().Axes()[2].GetV() );
  else if( !Params[0].Comparei("alpha") )
    E.SetRetVal( FXApp->XFile().GetAsymmUnit().Angles()[0].GetV() );
  else if( !Params[0].Comparei("beta") )
    E.SetRetVal( FXApp->XFile().GetAsymmUnit().Angles()[1].GetV() );
  else if( !Params[0].Comparei("gamma") )
    E.SetRetVal( FXApp->XFile().GetAsymmUnit().Angles()[2].GetV() );
  else if( !Params[0].Comparei("volume") )
    E.SetRetVal( olxstr::FormatFloat(2, FXApp->XFile().GetUnitCell().CalcVolume()) );
  else
    E.ProcessingError(__OlxSrcInfo, "invalid argument: ") << Params[0];
}
//..............................................................................
void TMainForm::funTitle(const TStrObjList& Params, TMacroError &E)  {
  if( !FXApp->XFile().GetLastLoader() )  {
    if( Params.IsEmpty() )
      E.SetRetVal( olxstr("File is not loaded") );
    else
      E.SetRetVal( Params[0] );
  }
  else
    E.SetRetVal( FXApp->XFile().GetLastLoader()->GetTitle() );
}
//..............................................................................
void TMainForm::funCif(const TStrObjList& Params, TMacroError &E)  {
  TCif* cf = (TCif*)FXApp->XFile().GetLastLoader();
  if( cf->ParamExists( Params[0] ) )
    E.SetRetVal( cf->GetSParam(Params[0]) );
  else
    E.SetRetVal( NAString );
}
//..............................................................................
void TMainForm::funP4p(const TStrObjList& Params, TMacroError &E)  {
}
//..............................................................................
void TMainForm::funHasGUI(const TStrObjList& Params, TMacroError &E)  {
  E.SetRetVal( true );
}
//..............................................................................
void TMainForm::funExtraZoom(const TStrObjList& Params, TMacroError &E)  {
  if( Params.IsEmpty() )
    E.SetRetVal( FXApp->GetExtraZoom() );
  else
    FXApp->SetExtraZoom( Params[0].ToDouble() );
}
//..............................................................................
void TMainForm::funIsOS(const TStrObjList& Params, TMacroError &E)  {
#if defined(__WXMSW__)
  E.SetRetVal( Params[0].Comparei("win") == 0 );
#elif defined(__WXMAC__) || defined(__MAC__)
  E.SetRetVal( Params[0].Comparei("mac") == 0 );
#elif defined(__WXGTK__)
  E.SetRetVal(Params[0].Comparei("linux") == 0);
#else
  E.SetRetVal(false);
#endif
}
//..............................................................................
void TMainForm::funCrs(const TStrObjList& Params, TMacroError &E)  {
  TCRSFile* cf = (TCRSFile*)FXApp->XFile().GetLastLoader();
  if( Params[0].Comparei("sg") == 0 )  {
    TSpaceGroup* sg = cf->GetSG();
    if( sg != NULL )
      E.SetRetVal( sg->GetName() );
    else
      E.SetRetVal( EmptyString );
  }
  else
    E.SetRetVal( NAString );
}
//..............................................................................
void TMainForm::funIns(const TStrObjList& Params, TMacroError &E)  {
  TIns *I = (TIns*)FXApp->XFile().GetLastLoader();
  olxstr tmp;
  if( Params[0].Comparei("weight") == 0 || Params[0].Comparei("wght") == 0 )  {
    for( int j=0; j < I->Wght().Count(); j++ )  {
      tmp << I->Wght()[j];
      if( (j+1) < I->Wght().Count() )  tmp << ' ';
    }
    E.SetRetVal( tmp );
    return;
  }
  if( !Params[0].Comparei("weight1") )  {
    for( int j=0; j < I->Wght1().Count(); j++ )  {
      tmp << I->Wght1()[j];
      if( (j+1) < I->Wght1().Count() )  tmp << ' ';
    }
    E.SetRetVal( tmp );
    return;
  }
  if( (Params[0].Comparei("L.S.") == 0) || (Params[0].Comparei("CGLS") == 0) )  {
    for( int i=0; i < I->GetLSV().Count(); i++ )  {
      tmp << I->GetLSV()[i];
      if( (i+1) < I->GetLSV().Count() )  tmp << ' ';
    }
    E.SetRetVal( tmp );
    return;
  }
  if( Params[0].Comparei("ls") == 0 )  {
    E.SetRetVal( I->GetIterations() );
    return;
  }
  if( Params[0].Comparei("plan") == 0)  {
    for( int i=0; i < I->GetPlanV().Count(); i++ )  {
      tmp << ((i < 1) ? Round(I->GetPlanV()[i]) : I->GetPlanV()[i]);
      if( (i+1) < I->GetPlanV().Count() )  tmp << ' ';
    }
    E.SetRetVal( tmp );
    return;
  }
  if( Params[0].Comparei("qnum") == 0)  {
    E.SetRetVal( I->GetPlan() );
    return;
  }
  if( !I->InsExists(Params[0]) )  {
    E.SetRetVal( NAString );
    return;
  }
//  FXApp->XFile().UpdateAsymmUnit();
//  I->UpdateParams();

  TInsList* insv = I->FindIns( Params[0] );
  if( insv != 0 )
    E.SetRetVal( insv->Text(' ') );
  else
    E.SetRetVal( EmptyString );
}
//..............................................................................
void TMainForm::funDataDir(const TStrObjList& Params, TMacroError &E)  {
  E.SetRetVal( DataDir.SubStringFrom(0, 1) );
}
//..............................................................................
void TMainForm::funStrcat(const TStrObjList& Params, TMacroError &E)  {
  E.SetRetVal( Params[0] + Params[1] );
}
//..............................................................................
void TMainForm::funStrcmp(const TStrObjList& Params, TMacroError &E)  {
  E.SetRetVal( Params[0] == Params[1] );
}
//..............................................................................
void TMainForm::funGetEnv(const TStrObjList& Params, TMacroError &E)  {
  wxString Val;
  if( !wxGetEnv( uiStr(Params[0]), &Val) )  {
    E.ProcessingError(__OlxSrcInfo,  "undefined variable: ") << Params[0];
    return;
  }
  E.SetRetVal<olxstr>( Val.c_str() );
}
//..............................................................................
void TMainForm::funFileSave(const TStrObjList& Params, TMacroError &E)  {
  E.SetRetVal( PickFile(Params[0], Params[1], Params[2], false) );
  if( E.GetRetVal().IsEmpty() )
    E.ProcessingError(__OlxSrcInfo, "operation canceled" );
}
//..............................................................................
void TMainForm::funFileOpen(const TStrObjList& Params, TMacroError &E)  {
  E.SetRetVal( PickFile(Params[0], Params[1], Params[2], true) );
  if( E.GetRetVal().IsEmpty() )
    E.ProcessingError(__OlxSrcInfo, "operation canceled" );
}
//..............................................................................
void TMainForm::funUnsetVar(const TStrObjList& Params, TMacroError &E)  {
  TOlxVars::UnsetVar(Params[0]);
}
//..............................................................................
void TMainForm::funSetVar(const TStrObjList& Params, TMacroError &E)  {
  TOlxVars::SetVar(Params[0], Params[1]);
}
//..............................................................................
void TMainForm::funGetVar(const TStrObjList& Params, TMacroError &E)  {
  int ind = TOlxVars::VarIndex(Params[0]);
  if( ind == -1 )  {
    if( Params.Count() == 2 )
      E.SetRetVal( Params[1] );
    else  
      E.ProcessingError(__OlxSrcInfo, "Could not locate specified attribute: '") << Params[0] << '\'';
    return;
  }
  E.SetRetVal( TOlxVars::GetVarStr(ind) );
  return;
}
//..............................................................................
void TMainForm::funIsVar(const TStrObjList& Params, TMacroError &E)  {
  E.SetRetVal( TOlxVars::IsVar(Params[0]));
}
//..............................................................................
void TMainForm::funVVol(const TStrObjList& Params, TMacroError &E)  {
  TSStrPObjList<olxstr,double, true> *Volumes = NULL;
  if( Params.Count() == 1 )  {
    if( !TEFile::FileExists(Params[0]) )  {
      E.ProcessingError(__OlxSrcInfo, "the volumes file does not exist: ") << Params[0];
      return;
    }
    TStrList SL, toks;
    SL.LoadFromFile( Params[0] );
    Volumes = new TSStrPObjList<olxstr,double, true>;
    for(int i=0; i < SL.Count(); i++ )  {
      toks.Clear();
      toks.Strtok(SL[i], ' ');
      if( toks.Count() != 2 )  {
        E.ProcessingError(__OlxSrcInfo, "wrong file line: ") << SL[i];
        delete Volumes;
        return;
      }
      Volumes->Add(toks[0], toks[1].ToDouble());
    }
  }
  olxstr report;
  E.SetRetVal( FXApp->CalcVolume(Volumes, report) );
  FGlConsole->PrintText( olxstr("Please note that this is a highly approximate procedure. \
Volume of current fragment is calculated using a maximum two overlaping spheres, to calculate packing indexes, use calcvoid instead"), &ErrorFontColor);
  TBasicApp::GetLog() << report;
  if( Volumes != NULL )
    delete Volumes;
}
//..............................................................................
void TMainForm::funSel(const TStrObjList& Params, TMacroError &E)  {
  TSAtomPList atoms;
  TGlGroup* sel = FXApp->Selection();
  for( int i=0; i < sel->Count(); i++ )  {
    AGDrawObject* gdo = sel->Object(i);
    if( EsdlInstanceOf(*gdo, TXAtom) )
      atoms.Add( &((TXAtom*)gdo)->Atom() );
    else if( EsdlInstanceOf(*gdo, TXBond) )  {
      atoms.Add( &((TXBond*)gdo)->Bond().A() );
      atoms.Add( &((TXBond*)gdo)->Bond().B() );
    }
  }
  olxstr tmp;
  for( int i=0; i < atoms.Count(); i++ )  {
    tmp << atoms[i]->GetLabel();
    if( (i+1) < atoms.Count() )  tmp << ' ';
  }
  E.SetRetVal( tmp );
}
//..............................................................................
void TMainForm::funAtoms(const TStrObjList& Params, TMacroError &E)
{
  throw TNotImplementedException(__OlxSourceInfo);
}
//..............................................................................
void TMainForm::funCrd(const TStrObjList& Params, TMacroError &E) {
  TXAtomPList Atoms;
  if( !FindXAtoms(Params, Atoms, true, true) ) {
    E.ProcessingError(__OlxSrcInfo, "could not find any atoms" );
    return;
  }
  TVPointD center;
  for( int i=0; i < Atoms.Count(); i++ )
    center += Atoms[i]->Atom().Center();

  center /= Atoms.Count();
  olxstr tmp = olxstr::FormatFloat(3, center[0]);
  tmp << ' ' << olxstr::FormatFloat(3, center[1]) << ' ' <<
         olxstr::FormatFloat(3, center[2]);
  E.SetRetVal( tmp );
}
//..............................................................................
void TMainForm::funCCrd(const TStrObjList& Params, TMacroError &E)  {
  TXAtomPList Atoms;
  if( !FindXAtoms(Params, Atoms, true, true) )  {
    E.ProcessingError(__OlxSrcInfo, "no atoms provided" );
    return;
  }
  TVPointD ccenter;
  for( int i=0; i < Atoms.Count(); i++ )
    ccenter += Atoms[i]->Atom().CCenter();

  ccenter /= Atoms.Count();
  olxstr tmp= olxstr::FormatFloat(3, ccenter[0]);
  tmp << ' ' << olxstr::FormatFloat(3, ccenter[1]) << ' ' <<
         olxstr::FormatFloat(3, ccenter[2]); 

  E.SetRetVal( tmp );
}
//..............................................................................
void TMainForm::funEnv(const TStrObjList& Params, TMacroError &E)  {
  TXAtom *XA = FXApp->GetXAtom(Params[0], false);
  if( XA == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "wrong atom: ") << Params[0];
    return;
  }
  olxstr tmp;
  for(int i=0; i < XA->Atom().NodeCount(); i++ )  {
    tmp << XA->Atom().Node(i).GetLabel() << ' ';
  }
  E.SetRetVal( tmp );
}
//..............................................................................
void TMainForm::funFPS(const TStrObjList& Params, TMacroError &E) {
  TimePerFrame = 0;
  for( int i=0; i < 10; i++ )
    TimePerFrame += FXApp->Draw();
  if( TimePerFrame != 0 )
    E.SetRetVal( 10*(1000./TimePerFrame) );
}
//..............................................................................
void TMainForm::funCursor(const TStrObjList& Params, TMacroError &E)  {
  if( Params.IsEmpty() )  {
    wxCursor cr(wxCURSOR_ARROW);
    SetCursor( cr );
    FGlCanvas->SetCursor( cr );
    SetStatusText(wxT(""));
  }
  else  {
    if( !Params[0].Comparei("busy") )  {
      wxCursor cr(wxCURSOR_WAIT);
      SetCursor( cr );
      FGlCanvas->SetCursor( cr );
      if( Params.Count() == 2 )
        SetStatusText( uiStr(Params[1]) );
    }
    else if( !Params[0].Comparei("brush") )  {
      wxCursor cr(wxCURSOR_PAINT_BRUSH);
      SetCursor( cr );
      FGlCanvas->SetCursor( cr );
    }
    else if( !Params[0].Comparei("hand") )  {
      wxCursor cr(wxCURSOR_HAND);
      SetCursor( cr );
      FGlCanvas->SetCursor( cr );
    }
    else  {
      if( TEFile::FileExists(Params[0]) )  {
        wxImage img;
        img.LoadFile( uiStr(Params[0]) );
        img.SetMaskColour(254, 254, 254);
        img.SetMask(true);
        wxCursor cr(img);
        SetCursor( cr );
        FGlCanvas->SetCursor( cr );
      }
    }
  }
}
//..............................................................................
void TMainForm::funRGB(const TStrObjList& Params, TMacroError &E)  {
  if( Params.Count() == 3 )  {
    E.SetRetVal( (int)RGB(Params[0].ToInt(), Params[1].ToInt(), Params[2].ToInt()) );
    return;
  }
  if( Params.Count() == 4 )  {
    E.SetRetVal( (int)RGBA( Params[0].ToInt(), Params[1].ToInt(), Params[2].ToInt(), Params[3].ToInt()) );
    return;
  }
}
//..............................................................................
void TMainForm::funHtmlPanelWidth(const TStrObjList &Cmds, TMacroError &E)  {
  if( FHtmlMinimized )
    E.SetRetVal( olxstr("-1") );
  else
    E.SetRetVal( GetHtml()->WI.GetWidth() );
}
void TMainForm::funColor(const TStrObjList& Params, TMacroError &E)  {
  wxColourDialog CD(this);
  wxColor wc;
  if( Params.Count() == 2 )  {
     wxColor wxdef(Params[1].u_str());
     wc = wxdef;
  }
  CD.GetColourData().SetColour(wc);
  if( CD.ShowModal() == wxID_OK )  {
    wc = CD.GetColourData().GetColour();
    if( Params.IsEmpty() )  {
      E.SetRetVal( (int)RGB(wc.Red(), wc.Green(), wc.Blue()) );
    }
    else if( Params[0].Comparei("hex") == 0 )  {
      char* bf = new char [35];
      sprintf( bf, "#%.2x%.2x%.2x", wc.Red(), wc.Green(), wc.Blue() );
      E.SetRetVal( olxstr(bf) );
      delete [] bf;
    }

  }
  else
    E.ProcessingError(__OlxSrcInfo, "operation canceled" );
}
//..............................................................................
void TMainForm::funLst(const TStrObjList &Cmds, TMacroError &E)  {
  if( !Lst.IsLoaded() )  {
    E.SetRetVal( NAString );
  }
  else if( !Cmds[0].Comparei("rint") )
    E.SetRetVal( Lst.Rint() );
  else if( !Cmds[0].Comparei("rsig") )
    E.SetRetVal( Lst.Rsigma() );
  else if( !Cmds[0].Comparei("r1") )
    E.SetRetVal( Lst.R1() );
  else if( !Cmds[0].Comparei("r1a") )
    E.SetRetVal( Lst.R1a() );
  else if( !Cmds[0].Comparei("wr2") )
    E.SetRetVal( Lst.wR2() );
  else if( !Cmds[0].Comparei("s") )
    E.SetRetVal( Lst.S() );
  else if( !Cmds[0].Comparei("rs") )
    E.SetRetVal( Lst.RS() );
  else if( !Cmds[0].Comparei("params") )
    E.SetRetVal( Lst.Params() );
  else if( !Cmds[0].Comparei("rtotal") )
    E.SetRetVal( Lst.TotalRefs() );
  else if( !Cmds[0].Comparei("runiq") )
    E.SetRetVal( Lst.UniqRefs() );
  else if( !Cmds[0].Comparei("r4sig") )
    E.SetRetVal( Lst.Refs4sig() );
  else if( !Cmds[0].Comparei("peak") )
    E.SetRetVal( Lst.Peak() );
  else if( !Cmds[0].Comparei("hole") )
    E.SetRetVal( Lst.Hole() );
  else
    E.SetRetVal( NAString );
}
//..............................................................................
void TMainForm::funZoom(const TStrObjList &Cmds, TMacroError &E)  {
  if( Cmds.IsEmpty() )
    E.SetRetVal( FXApp->GetRender().GetZoom() );
  else  {
    double zoom = FXApp->GetRender().GetZoom() + Cmds[0].ToDouble();
    if( zoom < 0.001 )  zoom = 0.001;
    FXApp->GetRender().SetZoom( zoom );
  }
}
#ifdef __WIN32__
//..............................................................................
void TMainForm::funLoadDll(const TStrObjList &Cmds, TMacroError &E)  {
  if( !TEFile::FileExists( Cmds[0] ) )  {
    E.ProcessingError(__OlxSrcInfo, "could not locate specified file" );
    return;
  }
  HINSTANCE lib_inst = LoadLibrary( uiStr(Cmds[0]) );
  if( !lib_inst )  {
    E.ProcessingError(__OlxSrcInfo, "could not load the library" );
    return;
  }
  FARPROC initPoint = GetProcAddress(lib_inst, "@olex@OlexPort@GetOlexRunnable$qv");
  if( !initPoint )
    initPoint = GetProcAddress(lib_inst, "?GetOlexRunnable@OlexPort@olex@@SGPAVIOlexRunnable@2@XZ");
  if( !initPoint )  {
    E.ProcessingError(__OlxSrcInfo, "could not locate initialisation point" );
    FreeLibrary( lib_inst );
    return;
  }
  typedef olex::IOlexRunnable* (*GetOlexRunnable)();
  olex::IOlexRunnable* runnable = ((GetOlexRunnable)initPoint)();

  if( !runnable )  {
    E.ProcessingError(__OlxSrcInfo, "NULL runnable" );
    FreeLibrary( lib_inst );
    return;
  }
  runnable->Run( *this );
  //FreeLibrary( lib_inst );
}
#endif // __WIN32__
//..............................................................................
bool TMainForm::ProcessMacroFunc(olxstr &Cmd)  {
  if( Cmd.IndexOf('(') == -1 )  return true;

  int specialFunctionIndex = Cmd.IndexOf('$');
  if( specialFunctionIndex != -1 )  {
    olxstr spFunction;
    int i=specialFunctionIndex, bc = 0;
    bool funcStarted = false;
    while( i++ < Cmd.Length() )  {
      if( Cmd.CharAt(i) == '(' )  {  bc++;  funcStarted = true;  }
      if( Cmd.CharAt(i) == ')' )  bc--;
      if( bc == 0 && funcStarted )  {
        spFunction << ')';
        if( ProcessMacroFunc( spFunction ) )  {
          Cmd.Delete( specialFunctionIndex, i - specialFunctionIndex + 1 );
          Cmd.Insert( spFunction, specialFunctionIndex );
        }
        else  {
          Cmd.Delete( specialFunctionIndex, i - specialFunctionIndex + 1 );
        }
        specialFunctionIndex = Cmd.IndexOf('$');
        if( specialFunctionIndex == -1 )  return true;
        i = specialFunctionIndex;
        spFunction = EmptyString;
        funcStarted = false;
        continue;
      }
      spFunction << Cmd[i];
    }
    return false;
  }

  int bc=0, fstart=0, fend, astart = 0, aend = 0;
  olxstr Func, ArgV;
  TStrObjList Params;
  TMacroError E;
  ABasicFunction *Function;
  for( int i=0; i < Cmd.Length(); i++ )  {
    if( Cmd.CharAt(i) == '(' && !Func.IsEmpty())  {
      if( Func.EndsWith('.') )  {  Func = EmptyString;  continue;  }
      bc++;
      i++;
      fend = -1;
      astart = aend = i;
      while( i < Cmd.Length() )  {
        if( Cmd.CharAt(i) == '(' )  bc++;
        if( Cmd.CharAt(i) == ')' )  {
          bc--;
          if( bc == 0 )  {  fend = i+1;  break; }
        }
        ArgV << Cmd.CharAt(i);
        i++;
        aend++;
      }
      if( ArgV.Length() != 0 )  { // arecursive call to all inner functions
        Params.Clear();
        TParamList::StrtokParams(ArgV, ',', Params);
        // evaluation will be called from within the functions
        if( Func.Comparei("or") != 0 && Func.Comparei("and") != 0 )  {
          olxstr localArg;
          for( int j=0; j < Params.Count(); j++ )  {
            if( !ProcessMacroFunc(Params[j]) )  {
              if( !Func.Comparei("eval") ) // put the function back
                Params[j] = ArgV;
              else  return false;
            }
          }
        }
        if( Params.Count() )
          ArgV = Params[0];
      }
      if( fend == -1 )  {
        TBasicApp::GetLog().Error(olxstr("Number of brackets do not match: ") << Cmd);
        return false;
      }
      if( Func.IsEmpty() )  {  // in case arithmetic ()
        Cmd.Delete(fstart+1, fend-fstart-2);  // have to leave ()
        Cmd.Insert(ArgV, fstart+1);
        return true;
      }
      E.Reset();
      Function = GetLibrary().FindFunction(Func);//, Params.Count() );
      //TODO: proper function management is needed ...
      if( Function == NULL )  {
        if( !Func.IsEmpty() )  {
          E.NonexitingMacroError( Func );
          AnalyseError( E );
        }
        return true;
      }
      Cmd.Delete(fstart, fend-fstart);
      Function->Run(Params, E);
      AnalyseError( E );
      if( !E.IsSuccessful() )  {
        if( (FMode & mSilent) == 0 )
          TBasicApp::GetLog() << ( olxstr("Last function call: '")  << Function->GetRuntimeSignature() << '\'' << '\n');
        return false;
      }
      Cmd.Insert(E.GetRetVal(), fstart);
      i = fstart + E.GetRetVal().Length();
    }
    if( i >= Cmd.Length() )  return true;
    olxch ch = Cmd.CharAt(i);
    if( ch == ' ' || ch == '+' || ch == '-' || ch == '/' ||
        ch == '*' || ch == '~' || ch == '&' || ch == '\'' ||
        ch == '\\' || ch == '|' || ch == '!' || ch == '"' ||
        ch == '$' || ch == '%' || ch == '^' || ch == '#'  ||
        ch == '=' || ch == '[' || ch == ']' || ch == '{' ||
        ch == '}' || ch == ':' || ch == ';' || ch == '?' ||
        ch == '(' || ch == ')')
    {
      Func = EmptyString;
      fstart = i+1;
      Params.Clear();
      ArgV = EmptyString;
      continue;
    }
    Func << ch;
  }
  return true;
}
//..............................................................................
void TMainForm::SubstituteArguments(olxstr &Cmd, TStrList &PName, TStrList &PVal)  {
  int index, iindex, pindex;
  olxstr ArgS, ArgV, CmdName, Tmp;
  index = Cmd.FirstIndexOf('%');  // argument by index
  while( index >= 0 && index < (Cmd.Length()-1) )  {
    ArgS = EmptyString;
    iindex = index;
    while( olxstr::o_isdigit(Cmd.CharAt(iindex+1)) )  {  // extract argument number
      ArgS << Cmd.CharAt(iindex+1);
      iindex ++;
      if( iindex >= (Cmd.Length()-1) )  break;
    }
    if( !ArgS.IsEmpty() )  {
      pindex = ArgS.ToInt()-1;  // index of the parameter
      if( pindex < PVal.Count() && pindex >= 0 )  {  // check if valid argument index
        Cmd.Delete(index, ArgS.Length()+1); // delete %xx value
        ArgV = PVal[pindex];
        Cmd.Insert(ArgV, index);  // insert value parameter
      }
      else  {
        TBasicApp::GetLog().Error(olxstr("Wrong argument index: ") << (pindex+1) << '\n');
      }
    }
    if( index++ < Cmd.Length() )
      index = Cmd.FirstIndexOf('%', index);  // next argument by index
    else
      index = -1;
  }
}
//..............................................................................
void TMainForm::DecodeParams(TStrObjList &Cmds, const olxstr &Cmd)  {
  if( !Cmd.Length() || !FMacroItem )  return;
  int index;
  TStrList Toks, Args, ArgP, Toks1;
  olxstr Tmp, ArgS, ArgV;
  TParamList::StrtokParams(Cmd, ' ' , Toks);
  if( !Toks.Count() )  return;
  TDataItem *DI = FMacroItem->FindItem(Toks[0]), *ArgsI, *CmdI, *TmpI;
  if( DI == NULL )  {
    TBasicApp::GetLog().Error(olxstr("Undefined macro: ") << Toks[0]);
    return;
  }
  DI = DI->FindItem("body");
  if( DI == NULL )  {
    TBasicApp::GetLog().Error(olxstr("Macro body is not defined: ") << Toks[0]);
    return;
  }
  CmdI = DI->FindItem("cmd");
  if( CmdI == NULL )  {
    TBasicApp::GetLog().Warning(olxstr("Empty macro: ") << Toks[0]);
    return;
  }
  ArgsI = DI->FindItem("args");
  if( ArgsI != NULL )  {
    if( ArgsI->ItemCount() < (Toks.Count()-1) )  {
      TBasicApp::GetLog().Error(olxstr("Macro has too many parameters: ") << Toks[0]);
      return;
    }
  }
  else   {  // if no arguments defined, then nothing to do with this function
    for( int i=0; i < CmdI->ItemCount(); i++ )  {
      TmpI = &CmdI->Item(i);
      Tmp = TmpI->GetValue();
      if( Tmp.IsEmpty() )
        Tmp = TmpI->GetFieldValue("cmd", EmptyString);
      if( Tmp.IsEmpty() )
        TBasicApp::GetLog().Warning(olxstr("Command has no body: ") << TmpI->GetName());
      else
        Cmds.Add(Tmp);
    }
    return;
  }
  Args.SetCount(ArgsI->ItemCount());
  ArgP.SetCount(ArgsI->ItemCount());
  for( int i=0; i < ArgsI->ItemCount(); i++ )  {
    TmpI = &ArgsI->Item(i);
    Tmp = TmpI->GetFieldValue("name", EmptyString); // name of the argument
    if( Tmp.IsEmpty() )
      TBasicApp::GetLog().Error(olxstr("Unnamed argument: ") << TmpI->GetName());
    else
      Args[i] = Tmp;
    Tmp = TmpI->GetFieldValue("def");    // defualt value
    if( !Tmp.IsEmpty() )  {
      ProcessMacroFunc(Tmp);
      ArgP[i] = Tmp;
    }
  }
  for( int i=1; i < Toks.Count(); i++ )  {
    if( Toks[i].FirstIndexOf('=') != -1 )  {  // argument with name and value
      Toks1.Clear();
      Toks1.Strtok(Toks[i], '=');
      if( Toks1.Count() < 2 )  {
        TBasicApp::GetLog().Error(olxstr("Parameter value missing for: ") << Toks1[0]);
        continue;
      }
      index = Args.IndexOf(Toks1[0]);
      if( index == -1 )
        TBasicApp::GetLog().Error(olxstr("Wrong parameter name: ") << Toks1[0]);
      else
        ArgP[index] = Toks1[1];
    }
    else  // argument by position
      ArgP[i-1] = Toks[i];
  }
  for( int i=0; i < CmdI->ItemCount(); i++ )  {
    TmpI = &CmdI->Item(i);
    Tmp = TmpI->GetValue();
    if( Tmp.IsEmpty() )
      Tmp = TmpI->GetFieldValue("cmd", EmptyString);
    if( Tmp.IsEmpty() )  {
      TBasicApp::GetLog().Warning(olxstr("Command has no body: ") << TmpI->GetName());
      continue;
    }
    SubstituteArguments(Tmp, Args, ArgP);
    Cmds.Add(Tmp);
  }
  CmdI = DI->FindItem("onterminate");
  if( CmdI != NULL )  {
    FOnTerminateMacroCmds.Clear();
    for( int i=0; i < CmdI->ItemCount(); i++ )  {
      TmpI = &CmdI->Item(i);
      Tmp = TmpI->GetValue();
      if( Tmp.IsEmpty() )
        Tmp = TmpI->GetFieldValue("cmd", EmptyString);
      if( Tmp.IsEmpty() )  {
        TBasicApp::GetLog().Warning(olxstr("Command has no body: ") << TmpI->GetName());
        continue;
      }
      SubstituteArguments(Tmp, Args, ArgP);
      FOnTerminateMacroCmds.Add(Tmp);
    }
  }
  CmdI = DI->FindItem("onlisten");
  FOnListenCmds.Clear();
  if( CmdI != NULL )  {
    for( int i=0; i < CmdI->ItemCount(); i++ )  {
      TmpI = &CmdI->Item(i);
      Tmp = TmpI->GetValue();
      if( Tmp.IsEmpty() )
        Tmp = TmpI->GetFieldValue("cmd", EmptyString);
      if( Tmp.IsEmpty() )  {
        TBasicApp::GetLog().Warning(olxstr("Command has no body: ") << TmpI->GetName());
        continue;
      }
      SubstituteArguments(Tmp, Args, ArgP);
      FOnListenCmds.Add(Tmp);
    }
  }
  CmdI = DI->FindItem("onabort");
  FOnAbortCmds.Clear();
  if( CmdI != NULL )  {
    for( int i=0; i < CmdI->ItemCount(); i++ )  {
      TmpI = &CmdI->Item(i);
      Tmp = TmpI->GetValue();
      if( Tmp.IsEmpty() )
        Tmp = TmpI->GetFieldValue("cmd", EmptyString);
      if( Tmp.IsEmpty() )  {
        TBasicApp::GetLog().Warning(olxstr("Command has no body: ") << TmpI->GetName());
        continue;
      }
      SubstituteArguments(Tmp, Args, ArgP);
      FOnAbortCmds.Add(Tmp);
    }
  }
}
//..............................................................................
void TMainForm::ProcessXPMacro(const olxstr &Cmd, TMacroError &Error, bool ProcessFunctions, bool ClearMacroError)  {
  if( ClearMacroError )  Error.Reset();
/*
  the Cmd string is constructed in the following way:
  command parameter1 parameter2 ...  -option1=option_value -option2=option_value ...
  the Cmd string is splitinto the lists: Cmds, Options and OptionParams
  Options and OptionParams always contain the same number of items
*/
  if( !Cmd.Length() )  return;

  if( (FMode & mSilent) == 0 )  {
    olxstr s(Cmd.ToLowerCase());
    if( (!s.StartsFrom("echo") && !s.StartsFrom("post")) )
      TBasicApp::GetLog() << Cmd << '\n';
  }
  else
    TBasicApp::GetLog().Info(Cmd);

  TStrObjList Cmds;
  TStrList Output;
  TParamList Options;
  olxstr Tmp, Command(Cmd), EnvName;
  // processing environment variables
  wxString EnvVal;
  int ind = Command.FirstIndexOf('|'), ind1;
  while( ind >= 0 )  {
    if( ind+1 >= Command.Length() )  break;
    ind1 = Command.FirstIndexOf('|', ind+1);
    if( ind1 == -1 )  break;
    if( ind1 == ind+1 )  { // %%
      Command.Delete(ind1, 1);
      ind = Command.FirstIndexOf('|', ind1);
      continue;
    }
    EnvName = Command.SubString(ind+1, ind1-ind-1);
    if( wxGetEnv( wxString(EnvName.raw_str(), EnvName.Length()), &EnvVal) )  {
      Command.Delete( ind, ind1-ind+1);
      Command.Insert(EnvVal.c_str(), ind );
      ind1 = ind + EnvVal.Length();
    }
    else  // variable is not found - leave as it is
      ind1 = ind + EnvName.Length();

    if( ind1+1 >= Command.Length() )  break;
    ind = Command.FirstIndexOf('|', ind1+1);
  }
  // end processing environment variables
  // special treatment of pyhton commands
  TParamList::StrtokParams(Command, ' ', Cmds);
//  CommandCS = Cmds[0];
//  Command = CommandCS.LowerCase();
  Command = Cmds[0];
  Cmds.Delete(0);
  for( int i = 0; i < Cmds.Count(); i++ )  {
    if( !Cmds[i].Length() )  continue;

    if( Cmds[i].CharAt(0) == '-' && !Cmds[i].IsNumber() )  {  // an option
      if( Cmds[i].Length() > 1 &&
          ((Cmds[i].CharAt(1) >= '0' && Cmds[i].CharAt(1) <= '9') || Cmds[i].CharAt(1) == '-') )  // cannot start from number
        continue;
      if( Cmds[i].Length() > 1 )  {
        Options.FromString(Cmds[i].SubStringFrom(1), '=');
        // 18.04.07 added - !!!
        ProcessMacroFunc( Options.Value(Options.Count()-1) );
      }
      Cmds.Delete(i);  i--;
      continue;
    }
  }
  ABasicFunction *MF = GetLibrary().FindMacro(Command);//, Cmds.Count());
  if( MF != NULL )  {
    if( !Command.Comparei("if") )  {
      ABasicFunction *MF = GetLibrary().FindMacro(Command);
      if( MF )  {
        MF->Run(Cmds, Options, Error);
        if( ProcessFunctions )  AnalyseError( Error );
      }
      return;
    }
    for( int i=0; i < Cmds.Count(); i++ )  {
      if( !ProcessMacroFunc(Cmds[i]) )
        return;
    }
    MF->Run(Cmds, Options, Error);
    AnalyseError( Error );
    return;
  }
  //..............................................................................
  // macro processing
  if( FMacroItem && FMacroItem->FindItem(Command) == NULL )  {  // macro does not exist
    if( Command.FirstIndexOf('(') != -1 )  {
      if( !ProcessMacroFunc(Command) )
        return;
    }
    else  {
      Error.NonexitingMacroError( Command );
      AnalyseError( Error );
      return;
    }
    //Error.NonexitingMacroError( Command );
    //if( ProcessFunctions )  AnalyseError( Error );
    return;
  }
  for( int i=0; i < Cmds.Count(); i++ )  {
    Command << ' ' << '\"' << Cmds[i] << '\"';
  }
  Cmds.Clear();
  TStrList OldOnAbort;
  OldOnAbort.Assign(FOnAbortCmds);
  FOnAbortCmds.Clear();
  DecodeParams(Cmds, Command);

  if( !Cmds.Count() )  {
    Error.ProcessingError(__OlxSrcInfo, "empty macro: ") << Command;
    if( ProcessFunctions )  AnalyseError( Error );
    FOnAbortCmds.Assign(OldOnAbort);
    return;
  }
  for( int i=0; i < Cmds.Count(); i++ )  {
    Command = Cmds[i];
    if( !Command.Length() )  continue;
    ProcessXPMacro(Command, Error);
    if( !Error.IsSuccessful() )  {
      for( int j=0; j < FOnAbortCmds.Count(); j++ )
        ProcessXPMacro(FOnAbortCmds[j], Error);
      break;
    }
  }
  FOnAbortCmds.Assign(OldOnAbort);
}
//..............................................................................
void TMainForm::macIF(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Cmds.Count() < 2 || Cmds[1].Comparei("then"))  {
    E.ProcessingError(__OlxSrcInfo, "incorrect syntax - two commands or a command and \'then\' are expected" );
    return;
  }
  olxstr Condition = Cmds[0];
  if( !ProcessMacroFunc(Condition) )  {
    E.ProcessingError(__OlxSrcInfo, "error processing condition" );
    return;
  }
  if( Condition.ToBool() )  {
    if( Cmds[2].Comparei(NoneString) )
      ProcessXPMacro(Cmds[2], E);
    return;
  }
  else  {
    if( Cmds.Count() == 5 )  {
      if( !Cmds[3].Comparei("else") )  {
        if( Cmds[4].Comparei(NoneString) )
          ProcessXPMacro(Cmds[4], E);
        return;
      }
      else  {
        E.ProcessingError(__OlxSrcInfo, "no keyword 'else' found" );
        return;
      }
    }
  }
}
//..............................................................................
void TMainForm::macBasis(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
// the events are handled in void TMainForm::CellVChange()
  if( Cmds.IsEmpty() )  {
//    TStateChange sc(prsBasisVis, !FXApp->BasisVisible());
    FXApp->SetBasisVisible( !FXApp->IsBasisVisible() );
//    OnStateChange->Execute((AEventsDispatcher*)this, &sc);
  }
  else  {
    bool status = Cmds[0].ToBool();
//    TStateChange sc(prsBasisVis, status );
    FXApp->SetBasisVisible( status );
//    OnStateChange->Execute((AEventsDispatcher*)this, &sc);
  }
}
//..............................................................................
void TMainForm::macLines(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  FGlConsole->SetLinesToShow( Cmds[0].ToInt() );
}
//..............................................................................
void TMainForm::macPict(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
#ifdef __WIN32__
  bool Emboss = false, EmbossColour = false, PictureQuality = false;
  for( int i=0; i < Options.Count(); i++ )  {
    if( !Options.GetName(i).Comparei("c") )
    {  Emboss = true;  EmbossColour = true;  continue;  }
    if( !Options.GetName(i).Comparei("bw") )
    {  Emboss = true;  continue;  }
    if( !Options.GetName(i).Comparei("pq") )
    {  PictureQuality = true;  continue;  }
  }

  if( PictureQuality )  FXApp->Quality(qaPict);

  short bits = 24, extraBytes;
  float res = 2;

  if( Cmds.Count() == 2 )  res = Cmds[1].ToDouble();
  if( res <= 0 )  res = 2;
  if( res >= 10 )  res = 10;

  int vpLeft = FXApp->GetRender().GetLeft(),
      vpTop = FXApp->GetRender().GetTop(),
      vpWidth = FXApp->GetRender().GetWidth(),
      vpHeight = FXApp->GetRender().GetHeight();


  int BmpHeight = vpHeight*res, BmpWidth = vpWidth*res;

  extraBytes = (4-(BmpWidth*3)%4)%4;

  HDC hDC = wglGetCurrentDC();
  HGLRC glc = wglGetCurrentContext();
  HDC dDC = CreateCompatibleDC(NULL);

  BITMAPFILEHEADER BmpFHdr;
  // intialise bitmap header
  BITMAPINFO BmpInfo;
  BmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER) ;
  BmpInfo.bmiHeader.biWidth = BmpWidth;
  BmpInfo.bmiHeader.biHeight = BmpHeight;
  BmpInfo.bmiHeader.biPlanes = 1 ;
  BmpInfo.bmiHeader.biBitCount = (WORD) bits;
  BmpInfo.bmiHeader.biCompression = BI_RGB ;
  BmpInfo.bmiHeader.biSizeImage = (BmpWidth*3+extraBytes+1)*BmpHeight; //TODO: +1 check!!!
  BmpInfo.bmiHeader.biXPelsPerMeter = 0 ;
  BmpInfo.bmiHeader.biYPelsPerMeter = 0 ;
  BmpInfo.bmiHeader.biClrUsed = 0;
  BmpInfo.bmiHeader.biClrImportant = 0 ;

  BmpFHdr.bfType = 0x4d42;
  BmpFHdr.bfSize = sizeof(BmpFHdr)  + sizeof(BITMAPINFOHEADER) + (BmpWidth*3+extraBytes)*BmpHeight;
  BmpFHdr.bfReserved1 = BmpFHdr.bfReserved2 = 0;
  BmpFHdr.bfOffBits = sizeof(BmpFHdr) + sizeof(BmpInfo.bmiHeader) ;

  char *DIBits;  DIBits = NULL;
  HBITMAP DIBmp = CreateDIBSection(dDC, &BmpInfo, DIB_RGB_COLORS, (void **)&DIBits, NULL, 0);

  SelectObject(dDC, DIBmp);

  PIXELFORMATDESCRIPTOR pfd = {
    sizeof(PIXELFORMATDESCRIPTOR),
    1,
    PFD_SUPPORT_OPENGL | PFD_SUPPORT_GDI | PFD_DRAW_TO_BITMAP,
    PFD_TYPE_RGBA,
    bits, //bits
    0,0,0,0,0,0,
    0,0,
    0,0,0,0,0,
    32,
    0,
    0,
    PFD_MAIN_PLANE,
    0,
    0,0,
    };
    int PixelFormat = ChoosePixelFormat(dDC, &pfd);
    SetPixelFormat(dDC, PixelFormat, &pfd);
  HGLRC dglc = wglCreateContext(dDC);

  FXApp->GetRender().Resize(0, 0, BmpWidth, BmpHeight, res);
  wglMakeCurrent(dDC, dglc);
  FBitmapDraw = true;
  FGlConsole->Visible(false);
  FXApp->BeginDrawBitmap(res);
  int vp[] = {0,0,BmpWidth,BmpHeight};
//  TEFile pdfout("c:/olex2.pdf", "w+b");
/*  gl2psBeginPage( "Olex2 output", "Olex2",
                  vp,
                  GL2PS_PDF,
                  GL2PS_SIMPLE_SORT,
                  GL2PS_USE_CURRENT_VIEWPORT | GL2PS_SILENT |
                   GL2PS_SIMPLE_LINE_OFFSET | GL2PS_OCCLUSION_CULL | GL2PS_BEST_ROOT,
                  GL_RGBA, 0,
                  NULL,
                  0, 0, 0,
                  1024*1024*10,
                  pdfout.Handle(),
                  "pdfout");
*/
  FXApp->GetRender().EnableFog( FXApp->GetRender().IsFogEnabled() );

  FXApp->Draw();
  GdiFlush();
  FBitmapDraw = false;

//  gl2psEndPage();

  wglDeleteContext(dglc);
  DeleteDC(dDC);
  wglMakeCurrent(hDC, glc);
  FXApp->GetRender().Resize(vpLeft, vpTop, vpWidth, vpHeight, 1);
  FGlConsole->Visible(true);
  FXApp->FinishDrawBitmap();
  if( PictureQuality )  FXApp->Quality(qaMedium);
  FXApp->GetRender().EnableFog( FXApp->GetRender().IsFogEnabled() );

  if( Emboss )  {
    if( EmbossColour )  {
      TProcessImage::EmbossC((unsigned char *)DIBits, BmpWidth, BmpHeight, 3,
                       FXApp->GetRender().LightModel.ClearColor().GetRGB());
    }
    else  {
      TProcessImage::EmbossBW((unsigned char *)DIBits, BmpWidth, BmpHeight, 3,
                       FXApp->GetRender().LightModel.ClearColor().GetRGB());
    }
  }

  olxstr bmpFN;

  if( FXApp->XFile().GetLastLoader() && Cmds[0].FirstIndexOf(':') == -1 )
    bmpFN = TEFile::ExtractFilePath(FXApp->XFile().GetFileName()) << TEFile::ExtractFileName( Cmds[0] );
  else
    bmpFN = Cmds[0];
  TEFile BmpFile(bmpFN, "w+b");
  BmpFile.Write(&(BmpFHdr), sizeof(BITMAPFILEHEADER));
  BmpFile.Write(&(BmpInfo), sizeof(BITMAPINFOHEADER));

  char *PP = DIBits;
  BmpFile.Write(PP, (BmpWidth*3+extraBytes)*BmpHeight);
  DeleteObject(DIBmp);
  //check if the image is bmp
  if( !BmpFile.ExtractFileExt(Cmds[0]).Comparei("bmp") )
    return;
  wxImage image;
  image.LoadFile( uiStr(bmpFN), wxBITMAP_TYPE_BMP);
  if( !image.Ok() )  {
    Error.ProcessingError(__OlxSrcInfo, "could not process image conversion" );
    return;
  }
  image.SaveFile( uiStr(bmpFN) );
#else
  macPicta(Cmds, Options, Error);
#endif // __WIN32__
}
//..............................................................................
void TMainForm::macPicta(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  short res = 2;

  if( Cmds.Count() == 2 )  res = Cmds[1].ToInt();
  if( res <= 0 )  res = 2;
  if( res > 10 )  res = 10;

  int orgHeight = FXApp->GetRender().GetHeight(),
      orgWidth  = FXApp->GetRender().GetWidth();
  int ScrHeight = (orgHeight/(res*2)-1)*res*2,
      ScrWidth  = (orgWidth/(res*2)-1)*res*2;
  int BmpHeight = ScrHeight*res, BmpWidth = ScrWidth*res;

  FXApp->Quality(qaPict);

  FXApp->GetRender().Resize(ScrWidth, ScrHeight); 

  const int bmpSize = BmpHeight*BmpWidth*3;
  char* bmpData = (char*)malloc(bmpSize);
  FGlConsole->Visible(false);
  FXApp->GetRender().OnDraw->SetEnabled( false );
  if( res > 1 )  FXApp->GetRender().Scene()->ScaleFonts(res);
  for( int i=0; i < res; i++ )  {
    for( int j=0; j < res; j++ )  {
      FXApp->GetRender().LookAt(j, i, res);
      FXApp->GetRender().Draw();
      char *PP = FXApp->GetRender().GetPixels(false, 1);
      int mj = j*ScrWidth;
      int mi = i*ScrHeight;
      for(int k=0; k < ScrWidth; k++ )  {
        for(int l=0; l < ScrHeight; l++ )  {
          int indexA = (l*ScrWidth + k)*3;
//          int indexB = (i*BmpWidth*ScrHeight + l*BmpWidth + j*ScrWidth + k)*3;
//          int indexB = bmpSize - (i*BmpWidth*ScrHeight + l*BmpWidth + (BmpWidth-(j*ScrWidth + k - 1)) - 1)*3;
          int indexB = bmpSize - (BmpWidth*(mi + l + 1) - mj - k)*3;
          bmpData[indexB] = PP[indexA];
          bmpData[indexB+1] = PP[indexA+1];
          bmpData[indexB+2] = PP[indexA+2];
        }
      }
      delete [] PP;
    }
  }
  // x
  for( double i=0; i < res; i++ )  {
    for( double j=0.5; j < res-1; j++ )  {
      FXApp->GetRender().LookAt(j, i, res);
      FXApp->GetRender().Draw();
      char *PP = FXApp->GetRender().GetPixels(false, 1);
      int mj = j*ScrWidth;
      int mi = i*ScrHeight;
      for(int k=ScrWidth*0.25; k <= ScrWidth*0.75; k++ )  {
        for(int l=ScrHeight*0.25; l < ScrHeight; l++ )  {
          int indexA = (l*ScrWidth + k)*3;
          int indexB = bmpSize - (BmpWidth*(mi + l + 1) - mj - k)*3;
          bmpData[indexB] = PP[indexA];
          bmpData[indexB+1] = PP[indexA+1];
          bmpData[indexB+2] = PP[indexA+2];
        }
      }
      delete [] PP;
    }
  }
  // y
  for( double i=0.5; i < res-1; i++ )  {
    for( double j=0; j < res; j++ )  {
      FXApp->GetRender().LookAt(j, i, res);
      FXApp->GetRender().Draw();
      char *PP = FXApp->GetRender().GetPixels(false, 1);
      int mj = j*ScrWidth;
      int mi = i*ScrHeight;
      for(int k=ScrWidth*0.25; k < ScrWidth; k++ )  {
        for(int l=0.25*ScrHeight; l <= ScrHeight*0.75; l++ )  {
          int indexA = (l*ScrWidth + k)*3;
          int indexB = bmpSize - (BmpWidth*(mi + l + 1) - mj - k)*3;
          bmpData[indexB] = PP[indexA];
          bmpData[indexB+1] = PP[indexA+1];
          bmpData[indexB+2] = PP[indexA+2];
        }
      }
      delete [] PP;
    }
  }
  // x,y
  for( double i=0.5; i < res-1; i++ )  {
    for( double j=0.5; j < res-1; j++ )  {
      FXApp->GetRender().LookAt(j, i, res);
      FXApp->GetRender().Draw();
      char *PP = FXApp->GetRender().GetPixels(false, 1);
      int mj = j*ScrWidth;
      int mi = i*ScrHeight;
      for(int k=0.25*ScrWidth; k <= ScrWidth*0.75; k++ )  {
        for(int l=0.25*ScrHeight; l <= ScrHeight*0.75; l++ )  {
          int indexA = (l*ScrWidth + k)*3;
          int indexB = bmpSize - (BmpWidth*(mi + l + 1) - mj - k)*3;
          bmpData[indexB] = PP[indexA];
          bmpData[indexB+1] = PP[indexA+1];
          bmpData[indexB+2] = PP[indexA+2];
        }
      }
      delete [] PP;
    }
  }

  FXApp->Quality(qaMedium);

  FXApp->GetRender().OnDraw->SetEnabled( true );
  FGlConsole->Visible(true);
  if( res > 1 ) 
    FXApp->GetRender().Scene()->RestoreFontScale();
  // end drawing etc
  FXApp->GetRender().Resize(orgWidth, orgHeight); 
  FXApp->GetRender().LookAt(0,0,1);
  FXApp->GetRender().SetView();
  FXApp->Draw();
  olxstr bmpFN;
  if( FXApp->XFile().GetLastLoader() )
    bmpFN = TEFile::ExtractFilePath(FXApp->XFile().GetFileName()) << TEFile::ExtractFileName( Cmds[0] );
  else
    bmpFN = Cmds[0];
  wxImage image;
  image.SetData((unsigned char*)bmpData, BmpWidth, BmpHeight ); 
//  image.Mirror(false).SaveFile( bmpFN.u_str() );
  image.SaveFile( bmpFN.u_str() );
  
#ifdef __WIN32x__
  short bits = 24, extraBytes;
  float res = 2;

  if( Cmds.Count() == 2 )  res = Cmds[1].ToDouble();
  if( res <= 0 )  res = 2;
  if( res >= 10 )  res = 5;

  int ScrHeight = FXApp->GetRender().GetHeight(),
      ScrWidth = FXApp->GetRender().GetWidth();
  int BmpHeight = ScrHeight*res, BmpWidth = ScrWidth*res;

  extraBytes = (4-(BmpWidth*3)%4)%4;
  int extraGlBytes = (4-(ScrWidth*3)%4)%4;

  BITMAPFILEHEADER BmpFHdr;
  // intialise bitmap header
  BITMAPINFO BmpInfo;
  BmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER) ;
  BmpInfo.bmiHeader.biWidth = BmpWidth;
  BmpInfo.bmiHeader.biHeight = BmpHeight;
  BmpInfo.bmiHeader.biPlanes = 1 ;
  BmpInfo.bmiHeader.biBitCount = (WORD) bits;
  BmpInfo.bmiHeader.biCompression = BI_RGB ;
  BmpInfo.bmiHeader.biSizeImage = (BmpWidth*3+extraBytes)*BmpHeight;
  BmpInfo.bmiHeader.biXPelsPerMeter = 0 ;
  BmpInfo.bmiHeader.biYPelsPerMeter = 0 ;
  BmpInfo.bmiHeader.biClrUsed = 0;
  BmpInfo.bmiHeader.biClrImportant = 0 ;

  BmpFHdr.bfType = 0x4d42;
  BmpFHdr.bfSize = sizeof(BmpFHdr)  + sizeof(BITMAPINFOHEADER) + (BmpWidth*3+extraBytes)*BmpHeight;
  BmpFHdr.bfReserved1 = BmpFHdr.bfReserved2 = 0;
  BmpFHdr.bfOffBits = sizeof(BmpFHdr) + sizeof(BmpInfo.bmiHeader) ;

  TEFile *BmpFile = NULL;
  olxstr bmpFN;
  if( FXApp->XFile().GetLastLoader() )
    bmpFN = TEFile::ExtractFilePath(FXApp->XFile().GetFileName()) << TEFile::ExtractFileName( Cmds[0] );
  else
    bmpFN = Cmds[0];
  BmpFile = new TEFile(bmpFN, "w+b");

  BmpFile->Write(&(BmpFHdr), sizeof(BITMAPFILEHEADER));
  BmpFile->Write(&(BmpInfo), sizeof(BITMAPINFOHEADER));

  FGlConsole->Visible(false);
  FXApp->GetRender().OnDraw->SetEnabled( false );
  if( res > 1 )  
    FXApp->GetRender().Scene()->ScaleFonts(res);
  char *imageLayer = new char [(BmpWidth*3+extraBytes)*ScrHeight];
  int ires = res;
  if( ires == 0 )  res = 1;
  for( int i=0; i < res; i++ )  {
    for( int j=0; j < res; j++ )  {
      FXApp->GetRender().LookAt(j, i, res);
      FXApp->GetRender().Draw();
      char *PP = FXApp->GetRender().GetPixels();
      for(int k=0; k < ScrWidth*3; k+=3 )  {
        for(int l=0; l < ScrHeight; l++ )  {
//          int indexA = l*(ScrWidth*3 + 5) + k + 5;
          int indexA = l*(ScrWidth*3+extraGlBytes) + k;
          int indexB = l*(BmpWidth*3+extraBytes) + j*ScrWidth*3 + k;
          imageLayer[indexB] = PP[indexA+2];
          imageLayer[indexB+1] = PP[indexA+1];
          imageLayer[indexB+2] = PP[indexA];
        }
      }
      delete [] PP;
    }
    BmpFile->Write(imageLayer, (BmpWidth*3+extraBytes)*ScrHeight);
  }
  delete [] imageLayer;

  FXApp->GetRender().OnDraw->SetEnabled( true );
  FGlConsole->Visible(true);
  if( res > 1 ) 
    FXApp->GetRender().Scene()->RestoreFontScale();
  // end drawing etc
  FXApp->GetRender().LookAt(0,0,1);
  FXApp->GetRender().SetView();
  FXApp->Draw();

  //check if the image is bmp
  if( !BmpFile->ExtractFileExt(Cmds[0]).Comparei("bmp") )
  {  delete BmpFile;  return;  }
  delete BmpFile;
  wxImage image;
  image.LoadFile( uiStr(bmpFN), wxBITMAP_TYPE_BMP);
  if( !image.Ok() )  {
    Error.ProcessingError(__OlxSrcInfo, "could not process image conversion" );
    return;
  }
  image.SaveFile( uiStr(bmpFN) );
#elif defined(RUBISH) // just make a snapshot then
  FGlConsole->Visible(false);
  FXApp->Draw();
  FGlConsole->Visible(true);
  char* pixels = FXApp->GetRender().GetPixels(true, 1);  //use malloc
  olxstr bmpFN;
  if( FXApp->XFile().GetLastLoader() )
    bmpFN = TEFile::ExtractFilePath(FXApp->XFile().GetFileName()) << TEFile::ExtractFileName( Cmds[0] );
  else
    bmpFN = Cmds[0];
  wxImage image;
  image.SetData((unsigned char*)pixels, FXApp->GetRender().GetWidth(), FXApp->GetRender().GetHeight() ); 
  image.Mirror(false).SaveFile( bmpFN.u_str() );
#endif // __WIN32__
}
//..............................................................................
void TMainForm::macBang(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TTTable<TStrList> Table;
  olxstr Tmp = Cmds.Text(' '), clipbrd;
  TXAtomPList Atoms;
  if( !FindXAtoms(Cmds, Atoms, true, true) ) {
    Error.ProcessingError(__OlxSrcInfo, "could not find any atoms" );
    return;
  }
  TStrList Output;
  for( int i=0; i < Atoms.Count(); i++ )  {
    FXApp->BangTable(Atoms[i], Table);
    Output.Clear();
    Table.CreateTXTList(Output, EmptyString, true, true, ' ');
    FGlConsole->PrintText( Output, NULL, false );
    clipbrd << Output.Text('\n');
  }
  if( wxTheClipboard->Open() )  {
    if (wxTheClipboard->IsSupported(wxDF_TEXT) )
      wxTheClipboard->SetData( new wxTextDataObject( uiStr(clipbrd)) );
    wxTheClipboard->Close();
  }
  TBasicApp::GetLog() << ("The environment list was placed to clipboard\n");
}
//..............................................................................
void TMainForm::macGrow(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error) {
  bool GrowShells = false,
      GrowContent = false,
      Template=false;
  TStrList TemplList;
  TCAtomPList TemplAtoms;
  for( int i=0; i < Options.Count(); i++ )  {
    if( Options.GetName(i) == "s" )  GrowShells  = true;
    else if( Options.GetName(i) == "w" )  GrowContent = true;
    else if( Options.GetName(i) == "t" )  {
      Template = true;
      TemplList.Strtok(Options.GetValue(i), ',');
      FXApp->FindCAtoms(TemplList.Text(' '), TemplAtoms);
    }
  }
  if( TemplList.Count() != 0 )  Template = true;
  else                     Template = false;
  if( Cmds.IsEmpty() )  {  // grow fragments
    if( Template )  {
      if( GrowContent ) FXApp->GrowWhole(&TemplAtoms);
      else              FXApp->GrowFragments(GrowShells, &TemplAtoms);
    }
    else  {
      if( GrowContent ) FXApp->GrowWhole(NULL);
      else              FXApp->GrowFragments(GrowShells);
    }
  }
  else  {  // grow atoms
    if( Template )  {
      if( GrowContent ) FXApp->GrowWhole(&TemplAtoms);
      else              FXApp->GrowAtoms(Cmds.Text(' '), GrowShells, &TemplAtoms);
    }
    else  {
      if( GrowContent ) FXApp->GrowWhole(NULL);
      else              FXApp->GrowAtoms(Cmds.Text(' '), GrowShells);
    }
  }
}
//..............................................................................
void TMainForm::macUniq(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( Cmds.IsEmpty() ) {  ;  }
  else  {
    TNetPList L, L1;
    TXAtomPList Atoms;
    FXApp->FindXAtoms(Cmds.Text(' '), Atoms);
    for( int i=0; i < Atoms.Count(); i++ )
      L.Add( &Atoms[i]->Atom().GetNetwork());

    FXApp->InvertFragmentsList(L, L1);
    FXApp->FragmentsVisible(L1, false);
    FXApp->CenterView();
    FXApp->GetRender().Basis()->SetZoom( FXApp->GetRender().CalcZoom()*FXApp->GetExtraZoom() );

    TimePerFrame = FXApp->Draw();
  }
}
//..............................................................................
void TMainForm::macGroup(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  olxstr name = Options.FindValue("n");
  if( Cmds.Count() >= 1 )  {
    TXAtomPList xatoms;
    FXApp->FindXAtoms( Cmds.Text(' '), xatoms, false);
    TGlGroup* glg = FXApp->GetRender().Selection();
    for( int i=0; i < xatoms.Count(); i++ )
      xatoms[i]->SetTag(0);
    for( int i=0; i < glg->Count(); i++ )
      glg->Object(i)->SetTag(1);
    for( int i=0; i < xatoms.Count(); i++ )
      if( xatoms[i]->GetTag() == 0 )
        FXApp->GetRender().Select( xatoms[i] );
    if( name.IsEmpty() )  {
      name = "group";
      name << (FXApp->GetRender().GroupCount()+1);
    }
    FXApp->GroupSelection(name);
    FXApp->SelectAll(false);
  }
}
//..............................................................................
void TMainForm::macFmol(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  FXApp->AllVisible(true);
  FXApp->CenterView();
  FXApp->GetRender().Basis()->SetZoom( FXApp->GetRender().CalcZoom()*FXApp->GetExtraZoom() );
}
//..............................................................................
void TMainForm::macClear(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error) {
  FGlConsole->ClearBuffer();  
}
//..............................................................................
void TMainForm::macCell(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
// the events are handled in void TMainForm::CellVChange()
  if( Cmds.IsEmpty() )
    FXApp->SetCellVisible( !FXApp->IsCellVisible() );
  else
    FXApp->SetCellVisible( Cmds[0].ToBool() );
}
//..............................................................................
void TMainForm::macRota(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( Cmds.Count() == 2 )  {  // rota x 90 syntax
    double angle = Cmds[1].ToDouble();
    if( Cmds[0] == "1" || Cmds[0] == "x"  || Cmds[0] == "a" )
      FXApp->GetRender().Basis()->RotateX(FXApp->GetRender().GetBasis().GetRX()+angle);
    else if( Cmds[0] == "2" || Cmds[0] == "y"  || Cmds[0] == "b" )
      FXApp->GetRender().Basis()->RotateY(FXApp->GetRender().GetBasis().GetRY()+angle);
    else if( Cmds[0] == "3" || Cmds[0] == "z"  || Cmds[0] == "c" )
      FXApp->GetRender().Basis()->RotateZ(FXApp->GetRender().GetBasis().GetRZ()+angle);
  }
  else if( Cmds.Count() == 5 )  {  // rota x y z 90 1 syntax - rotation around (x,y,z) 90 degrees with 1 degree inc
    FRotationVector[0] = Cmds[0].ToDouble();
    FRotationVector[1] = Cmds[1].ToDouble();
    FRotationVector[2] = Cmds[2].ToDouble();
    FRotationAngle = Cmds[3].ToDouble();
    FRotationIncrement = Cmds[4].ToDouble();
    FMode = FMode | mRota;
  }
  else  {
    Error.ProcessingError(__OlxSrcInfo, "wrong parameters" );
  }
}
//..............................................................................
void TMainForm::macListen(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( Cmds.Count() != 0 )  {
    FMode |= mListen;
    FListenFile = TEFile::OSPath(Cmds.Text(' '));
    TBasicApp::GetLog() << (olxstr("Listening for: ") << FListenFile << '\n');
  }
  else  {
    if( FMode & mListen )
      TBasicApp::GetLog() << (olxstr("Listening for: ") << FListenFile << '\n');
    else
      TBasicApp::GetLog() << ("Not in a listening mode\n");
  }
}
//..............................................................................
#ifdef __WIN32__
void TMainForm::macWindowCmd(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  for( int i=1; i < Cmds.Count(); i++ )  {
    if( !Cmds[i].Comparei("nl") )    {  TWinWinCmd::SendWindowCmd(Cmds[0], '\r'); continue; }
    if( !Cmds[i].Comparei("sp") )    {  TWinWinCmd::SendWindowCmd(Cmds[0], ' ');  continue;}
    TWinWinCmd::SendWindowCmd(Cmds[0], Cmds[i]+' ');
  }
}
#endif
//..............................................................................
void TMainForm::macProcessCmd(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( FProcess == NULL )  {
    Error.ProcessingError(__OlxSrcInfo, "process does not exist" );
    return;
  }
  for( int i=0; i < Cmds.Count(); i++ )  {
    if( !Cmds[i].Comparei("nl") )
      FProcess->Writenl();
    else if( !Cmds[i].Comparei("sp") )
      FProcess->Write(' ');
    else
      FProcess->Write(Cmds[i]+' ');
  }
}
//..............................................................................
void TMainForm::macWait(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  FXApp->Sleep(Cmds[0].ToInt());
}
//..............................................................................
void TMainForm::macSwapBg(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)
{
  // hide the gradient background
  FXApp->GetRender().Background()->Visible(false);
  if( FXApp->GetRender().LightModel.ClearColor().GetRGB() == 0xffffffff )
    FXApp->GetRender().LightModel.ClearColor() = FBgColor;
  else
    FXApp->GetRender().LightModel.ClearColor() = 0xffffffff;
  FXApp->GetRender().LoadIdentity();
  FXApp->GetRender().InitLights();
}
//..............................................................................
void TMainForm::macSilent(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( Cmds.IsEmpty() )  {
    if( FMode & mSilent )  TBasicApp::GetLog() << ("Silent mode is on\n");
    else                   TBasicApp::GetLog() << ("Silent mode is off\n");
  }
  else if( Cmds[0] == "on" )  {
    FMode |= mSilent;
    TBasicApp::GetLog().Info("Silent mode is on");
  }
  else if( Cmds[0] == "off" )  {
    FMode &= ~mSilent;
    TBasicApp::GetLog().Info("Silent mode is off");
  }
}
//..............................................................................
void TMainForm::macStop(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( Cmds[0] == "listen" )  {
    if( FMode & mListen )  {
      FMode ^= mListen;
      TBasicApp::GetLog().Info( "Listen mode is off" );
    }
  }
  else if( Cmds[0] == "logging" )  {
    if( ActiveLogFile != NULL )  {
      delete ActiveLogFile;
      ActiveLogFile = NULL;
    }
  }
}
//..............................................................................
void TMainForm::macEcho(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TBasicApp::GetLog() << Cmds.Text(' ') << '\n';
  //FXApp->Draw();
}
//..............................................................................
void TMainForm::macPost(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( FXApp == NULL || FGlConsole == NULL )  return;
  TBasicApp::GetLog() << Cmds.Text(' ');
  FXApp->Draw();
  wxTheApp->Dispatch();
}
//..............................................................................
void TMainForm::macExit(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  Close(true);
}
//..............................................................................
void TMainForm::macPack(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TVPointD From( -1.0, -1.0, -1.0);
  TVPointD To( 1.5, 1.5, 1.5);

  int number_count = 0;
  for( int i=0; i < Cmds.Count(); i++ )
    if( Cmds[i].IsNumber() )
      number_count++;

  if( number_count != 0 && number_count != 6 )  {
    Error.ProcessingError(__OlxSrcInfo, "please provide 6 numbers" );
    return;
  }
  if( number_count == 6 )  {
    number_count = 0;
    for( int i=0; i < Cmds.Count(); i++ )  {
      if( Cmds[i].IsNumber() )  {
        if( !(number_count%2) )
          From[number_count/2] = Cmds[i].ToDouble();
        else
          To[number_count/2]= Cmds[i].ToDouble();
        number_count++;
        Cmds.Delete(i);
        i--;
      }
    }
  }

  bool ClearCont = !Options.Contains("c");
  bool IncludeQ = Options.Contains("q");
  TCAtomPList TemplAtoms;
  if( Cmds.Count() != 0 )
    FXApp->FindCAtoms(Cmds.Text(' '), TemplAtoms);

  int64_t st = TETime::msNow();
  if( TemplAtoms.Count() != 0 )
    FXApp->Generate(From, To, &TemplAtoms, ClearCont, IncludeQ);
  else
    FXApp->Generate(From, To, NULL, ClearCont, IncludeQ);

  TBasicApp::GetLog().Info( olxstr(FXApp->XFile().GetLattice().AtomCount()) << " atoms and " <<
     FXApp->XFile().GetLattice().BondCount() << " bonds generated in " <<
     FXApp->XFile().GetLattice().FragmentCount() << " fragments (" << (TETime::msNow()-st) << "ms)");
  // optimise drawing ...
  //FXApp->GetRender().Compile(true);
}
//..............................................................................
void TMainForm::macName(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  bool checkLabels = Options.Contains("c");
  bool changeSuffix = Options.Contains("s");
  if( changeSuffix )  {
    TXAtomPList xatoms;
    FXApp->FindXAtoms( Cmds.Text(' '), xatoms, !Options.Contains("cs") );
    if( xatoms.Count() != 0 )
      FUndoStack->Push( FXApp->ChangeSuffix(xatoms, Options.FindValue("s"), checkLabels) );
  }
  else  {
    FUndoStack->Push( FXApp->Name(Cmds[0], Cmds[1], checkLabels, !Options.Contains("cs")) );
  }
}
//..............................................................................
void TMainForm::macTelpV(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  FXApp->CalcProbFactor(Cmds[0].ToDouble());
}
//..............................................................................
void TMainForm::macLabels(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  short lmode = 0;
  for( int i=0; i < Options.Count(); i++ )  {
    if( !Options.GetName(i).Comparei("p") )       lmode |= lmPart;
    else if( !Options.GetName(i).Comparei("l") )  lmode |= lmLabels;
    else if( !Options.GetName(i).Comparei("v") )  lmode |= lmOVar;
    else if( !Options.GetName(i).Comparei("o") )  lmode |= lmOccp;
    else if( !Options.GetName(i).Comparei("ao") ) lmode |= lmAOcc;
    else if( !Options.GetName(i).Comparei("u") )  lmode |= lmUiso;
    else if( !Options.GetName(i).Comparei("r") )  lmode |= lmUisR;
    else if( !Options.GetName(i).Comparei("a") )  lmode |= lmAfix;
    else if( !Options.GetName(i).Comparei("h") )  lmode |= lmHydr;
    else if( !Options.GetName(i).Comparei("f") )  lmode |= lmFixed;
  }
  if( lmode == 0 )  {
    lmode |= lmLabels;
    FXApp->LabelsMode(lmode);
    FXApp->LabelsVisible( !FXApp->LabelsVisible() );
  }
  else  {
    FXApp->LabelsMode(lmode);
    FXApp->LabelsVisible(true);
  }
}
//..............................................................................
void TMainForm::macSetEnv(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( !wxSetEnv(uiStr(Cmds[0]), uiStr(Cmds[1])) )  {
    Error.ProcessingError(__OlxSrcInfo, "could not set the variable" );
    return;
  }
}
//..............................................................................
void TMainForm::macActivate(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXPlane *XP = FXApp->XPlane(Cmds[0]);
  if( XP != NULL )  {
    TVPointD V;
    V = XP->Plane().Normal();
    FXApp->Orient(V);
  }
  else  {
    Error.ProcessingError(__OlxSrcInfo, "could not find specified plane: ") << Cmds[0];
    return;
  }
}
//..............................................................................
void TMainForm::macInfo(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TStrList Output;
  if( Cmds.IsEmpty() )
    FXApp->InfoList(EmptyString, Output);
  else
    FXApp->InfoList(Cmds.Text(' '), Output);
  TBasicApp::GetLog() << Output;
}
//..............................................................................
void TMainForm::macHelp(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( FHelpItem == NULL )  {  // just print out built in functions if any
    PostCmdHelp(Cmds[0], true);
    return;
  }
  if( !Cmds.Count() )  {
    if( !Options.Count() )  {
      int period=6;
      olxstr Tmp;
      for(int i=0; i <= FHelpItem->ItemCount(); i+=period )  {
        Tmp = EmptyString;
        for( int j=0; j < period; j++ )  {
          if( (i+j) >= FHelpItem->ItemCount() )
            break;
          Tmp << FHelpItem->Item(i+j).GetName();
          Tmp.Format((j+1)*10, true, ' ');
        }
        FGlConsole->PrintText(Tmp);
      }
      return;
    }
    else  {
      if( Options.GetName(0)[0] ==  'c' )  {  // show categories
        TStrList Cats;
        TDataItem *Cat;
        for( int i=0; i < FHelpItem->ItemCount(); i++ )  {
          Cat = FHelpItem->Item(i).FindItemCI("category");
          if( Cat == NULL )  continue;
          for( int j=0; j < Cat->ItemCount(); j++ )  {
            if( Cats.IndexOf(Cat->Item(j).GetName()) == -1 )
              Cats.Add(Cat->Item(j).GetName());
          }
        }
        if( Cats.Count() )
          FGlConsole->PrintText("Macro categories: ");
        else
          FGlConsole->PrintText("No macro categories was found...");
        Cats.QSort(true);
        for( int i=0; i < Cats.Count(); i++ )
          FGlConsole->PrintText(Cats[i]);
      }
    }
  }
  else  {
    if( !Options.Count() )
      PostCmdHelp(Cmds[0], true);
    else  {
      if( Options.GetName(0)[0] ==  'c' )  {  // show categories
        FGlConsole->PrintText(olxstr("Macroses for category: ") << Cmds[0]);
        TDataItem *Cat;
        for( int i=0; i < FHelpItem->ItemCount(); i++ )  {
          Cat = FHelpItem->Item(i).FindItemCI("category");
          if( Cat == NULL )  continue;
          for( int j=0; j < Cat->ItemCount(); j++ )  {
            if( Cat->Item(j).GetName().Comparei(Cmds[0]) == 0 )  {
              FGlConsole->PrintText(FHelpItem->Item(i).GetName());
              break;
            }
          }
        }
      }
    }
  }
}
//..............................................................................
void TMainForm::macMatr(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( Cmds.IsEmpty() )  {
    TMatrixD Matr(3,3);
    Matr = FXApp->GetRender().GetBasis().GetMatrix();
    olxstr Tmp;
    for(int i=0; i < 3; i++ )  {
      Tmp = EmptyString;
      Tmp << olxstr::FormatFloat(3, Matr[0][i]);  Tmp.Format(7, true, ' ');
      Tmp << olxstr::FormatFloat(3, Matr[1][i]);  Tmp.Format(14, true, ' ');
      Tmp << olxstr::FormatFloat(3, Matr[2][i]);  Tmp.Format(21, true, ' ');
      TBasicApp::GetLog() << (Tmp << '\n');
    }
    return;
  }
  else  {
    if( Cmds.Count() == 1 )  {
      TMatrixD M;
      if( FXApp->HklVisible() )  M = FXApp->XFile().GetAsymmUnit().GetHklToCartesian();
      else                       M = FXApp->XFile().GetCell2Cartesian();
      if( Cmds[0] == "100" || Cmds[0] == "1" ){ FXApp->Orient( M[0] ); return; }
      if( Cmds[0] == "010" || Cmds[0] == "2"  ){ FXApp->Orient( M[1] ); return; }
      if( Cmds[0] == "001" || Cmds[0] == "3"  ){ FXApp->Orient( M[2] ); return; }

      if( Cmds[0] == "110" ){ FXApp->Orient( M[0] + M[1] ); return; }
      if( Cmds[0] == "101" ){ FXApp->Orient( M[0] + M[2] ); return; }
      if( Cmds[0] == "011" ){ FXApp->Orient( M[1] + M[2] ); return; }
      if( Cmds[0] == "111" ){ FXApp->Orient( M[0] + M[1] + M[2] ); return; }

      Error.ProcessingError(__OlxSrcInfo, "undefined arguments" );
      return;
    }
    if( Cmds.Count() == 9 )  {
      TMatrixD M(3,3);
      M[0][0] = Cmds[0].ToDouble();
      M[0][1] = Cmds[1].ToDouble();
      M[0][2] = Cmds[2].ToDouble();
      M[1][0] = Cmds[3].ToDouble();
      M[1][1] = Cmds[4].ToDouble();
      M[1][2] = Cmds[5].ToDouble();
      M[2][0] = Cmds[6].ToDouble();
      M[2][1] = Cmds[7].ToDouble();
      M[2][2] = Cmds[8].ToDouble();
      M.Transpose();
      M[0].Normalise();
      M[1].Normalise();
      M[2].Normalise();
      FXApp->GetRender().Basis()->Matrix(M);
      return;
    }
    Error.ProcessingError(__OlxSrcInfo, "wrong arguments" );
    return;
  }
}
//..............................................................................
void TMainForm::macQual(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( Options.IsEmpty() )  {
    Error.ProcessingError(__OlxSrcInfo, "wrong number of arguments" );
    return;
  }
  else  {
     if( Options.GetName(0)[0] == 'h' ) { FXApp->Quality(qaHigh);  return; }
     if( Options.GetName(0)[0] == 'm' ) { FXApp->Quality(qaMedium);  return; }
     if( Options.GetName(0)[0] == 'l' ) { FXApp->Quality(qaLow);  return; }

    Error.ProcessingError(__OlxSrcInfo, "wrong argument" );
    return;
  }
}
//..............................................................................
void TMainForm::macLine(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TVPointD V;
  TXAtomPList Atoms;
  olxstr name;
  if( Cmds.Count() == 3 )  {
   name = Cmds[0];
    Cmds.Delete(0);
  }
  FXApp->FindXAtoms(Cmds.Text(' '), Atoms);
  if( Atoms.Count() != 2 )  {
    Error.ProcessingError(__OlxSrcInfo, "a [name] and two atoms are expected" );
    return;
  }
  V = Atoms[0]->Atom().Center() - Atoms[1]->Atom().Center();
  FXApp->Orient(V);
  FXApp->AddLine(name, Atoms[0]->Atom().Center(), Atoms[1]->Atom().Center());
}
//..............................................................................
void TMainForm::macMpln(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TSPlane* plane = NULL;
  bool orientOnly = false,
       rectangular = false;
  int weightExtent = 0;
  olxstr planeName;
  for(int i=0; i < Options.Count(); i++ )  {
    if( !Options.GetName(i).Comparei("n") )
      orientOnly = true;
    else if( !Options.GetName(i).Comparei("r") )
      rectangular = true;
    else if( !Options.GetName(i).Comparei("we") )
      weightExtent = Options.GetValue(i).ToDouble();
  }
  TXAtomPList Atoms;

  if( Cmds.IsEmpty() )  {
    if( orientOnly )  {
      FXApp->FindXAtoms( EmptyString, Atoms );
      plane = FXApp->TmpPlane(NULL, weightExtent);
      if( plane != NULL )  {
        FXApp->Orient( plane->Normal() );
        planeName = "All atoms";
      }
    }
    else  {
      Error.ProcessingError(__OlxSrcInfo, "wrong arguments" );
      return;
    }
  }
  else  {
    FXApp->FindXAtoms(Cmds.Text(' '), Atoms);
    for( int i=0; i < Atoms.Count(); i++ )  {
      planeName << Atoms[i]->Atom().GetLabel();
      if( i+1 < Atoms.Count() )
        planeName << ' ';
    }

    if( Atoms.Count() < 3 )  {
      Error.ProcessingError(__OlxSrcInfo, "wrong atom count" );
      return;
    }
    if( orientOnly )  {
      plane = FXApp->TmpPlane(&Atoms, weightExtent);
      if( plane != NULL )  {
        FXApp->Orient( plane->Normal() );
      }
    }
    else  {
       TXPlane* xp = FXApp->AddPlane(Atoms, rectangular, weightExtent);
        if( xp != NULL )
          plane = &xp->Plane();
    }
  }
  if( plane != NULL )  {
    int colCount = 3;
    double summ = 0, v;
    TTTable<TStrList> tab( Atoms.Count()/colCount + (((Atoms.Count()%colCount)==0)?0:1), colCount*2);
    for( int i=0; i < colCount; i++ )  {
      tab.ColName(i*2) = "Label";
      tab.ColName(i*2+1) = "D/A";
    }
    for( int i=0; i < Atoms.Count(); i+=colCount )  {
      for( int j=0; j < colCount; j++ )  {
        if( i + j >= Atoms.Count() )
          break;
        tab.Row(i/colCount)->String(j*2) = Atoms[i+j]->Atom().GetLabel();
        v = plane->DistanceTo(Atoms[i+j]->Atom());
        tab.Row(i/colCount)->String(j*2+1) = olxstr::FormatFloat(3, v );
        summ += v;
      }
    }
    TStrList Output;
    tab.CreateTXTList(Output, olxstr("Atom-to-plane distances for ") << planeName, true, false, "  | ");
    TBasicApp::GetLog() << ( Output );
    TBasicApp::GetLog() << ( olxstr("Summ deviation: ") << olxstr::FormatFloat(3, summ) << '\n');

    TVPointD center;
    for( int i=0; i < Atoms.Count(); i++ )
      center += Atoms[i]->Atom().Center();
    center /= Atoms.Count();
    FXApp->SetGridDepth(center);
  }
}
//..............................................................................
void TMainForm::macCent(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXAtomPList Atoms;
  FXApp->FindXAtoms(Cmds.Text(' '), Atoms);
  FXApp->AddCentroid(Atoms);
}
//..............................................................................
void TMainForm::macFile(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  olxstr Tmp;
  if( Cmds.IsEmpty() )  {  // error
    // res -> Ins rotation if ins file
    if( EsdlInstanceOf(*FXApp->XFile().GetLastLoader(), TIns) )
      Tmp = TEFile::ChangeFileExt(FXApp->XFile().GetFileName(), "ins");
    else
      Tmp = FXApp->XFile().GetFileName();
  }
  else
    Tmp = Cmds[0];

  bool Sort = Options.Contains('s');

  if( !TEFile::ExtractFilePath(Tmp).Length() )
    Tmp = TEFile::AddTrailingBackslash(CurrentDir) + Tmp;

  FXApp->SaveXFile(Tmp, Sort);
  UpdateRecentFile(Tmp);
  FInfoBox->Clear();
  FInfoBox->PostText(FXApp->XFile().GetFileName());
  FInfoBox->PostText(FXApp->XFile().GetLastLoader()->GetTitle());
  OnResize();
  Tmp = TEFile::ExtractFilePath(Tmp);
  if( Tmp.Length() && (Tmp.Comparei(CurrentDir)) )
  {
    if( !TEFile::ChangeDir(Tmp) )
      TBasicApp::GetLog().Error("Cannot change current folder...");
    else
      CurrentDir = Tmp;
  }
}
//..............................................................................
void TMainForm::macUser(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( Cmds.IsEmpty() )  {
    TBasicApp::GetLog() << TEFile::CurrentDir() << '\n';
  }
  else if( !TEFile::ChangeDir(Cmds[0]) )  {
    Error.ProcessingError(__OlxSrcInfo, "could not change current folder" );
  }
  else
    CurrentDir = Cmds[0]; 
}
//..............................................................................
void TMainForm::macDir(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  olxstr Filter("*.*");
  TStrList Output;
  if( Cmds.Count() != 0 )  Filter = Cmds[0];
  TFileList fl;
  TEFile::ListCurrentDirEx(fl, Filter, sefFile|sefDir);
  TTTable<TStrList> tab(fl.Count(), 4);
  tab.ColName(0) = "Name";
  tab.ColName(1) = "Size";
  tab.ColName(2) = "Last Modified";
  tab.ColName(3) = "Attributes";

  TFileListItem::SortListByName(fl);

  for( int i=0; i < fl.Count(); i++ )  {
    tab.Row(i)->String(0) = fl[i].GetName();
    if( (fl[i].GetAttributes() & sefDir) != 0 )
      tab.Row(i)->String(1) = "Folder";
    else
      tab.Row(i)->String(1) = fl[i].GetSize();
    tab.Row(i)->String(2) = TETime::FormatDateTime("yyyy.MM.dd hh:mm:ss", fl[i].GetModificationTime());
    if( (fl[i].GetAttributes() & sefReadOnly) != 0 )
      tab.Row(i)->String(3) << 'r';
    if( (fl[i].GetAttributes() & sefWriteOnly) != 0 )
      tab.Row(i)->String(3) << 'w';
    if( (fl[i].GetAttributes() & sefHidden) != 0 )
      tab.Row(i)->String(3) << 'h';
    if( (fl[i].GetAttributes() & sefSystem) != 0 )
      tab.Row(i)->String(3) << 's';
    if( (fl[i].GetAttributes() & sefExecute) != 0 )
      tab.Row(i)->String(3) << 'e';
  }

  tab.CreateTXTList(Output, "Directory list", true, true, ' ');
  TBasicApp::GetLog() << Output;
}
//..............................................................................
void TMainForm::macAnis(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXAtomPList XAtoms;
  TCAtomPList CAtoms;
  bool ConsiderH = Options.Contains("h");
  if( !FindXAtoms(Cmds, XAtoms, true, !Options.Contains("cs")) )  {
    Error.ProcessingError(__OlxSrcInfo, "wrong atom names" );
    return;
  }
  CAtoms.SetCapacity( XAtoms.Count() );
  for( int i=0; i < XAtoms.Count(); i++ )  {
    if( (!ConsiderH && XAtoms[i]->Atom().GetAtomInfo() == iHydrogenIndex) ||
        XAtoms[i]->Atom().GetAtomInfo() == iQPeakIndex )
      continue;
    CAtoms.Add( &XAtoms[i]->Atom().CAtom() );
  }
  FXApp->XFile().GetLattice().SetAnis( CAtoms, true );
}
//..............................................................................
void TMainForm::macIsot(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXAtomPList XAtoms;
  TCAtomPList CAtoms;
  bool ConsiderH = Options.Contains("h");
  if( !FindXAtoms(Cmds, XAtoms, true, !Options.Contains("cs")) )  {
    Error.ProcessingError(__OlxSrcInfo, "wrong atom names" );
    return;
  }
  CAtoms.SetCapacity( XAtoms.Count() );
  for( int i=0; i < XAtoms.Count(); i++ )  {
    if( (!ConsiderH && XAtoms[i]->Atom().GetAtomInfo() == iHydrogenIndex) ||
        XAtoms[i]->Atom().GetAtomInfo() == iQPeakIndex )
      continue;
    CAtoms.Add( &XAtoms[i]->Atom().CAtom() );
  }
  FXApp->XFile().GetLattice().SetAnis( CAtoms, false );
}
//..............................................................................
void TMainForm::macMask(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error){
  if( Cmds[0] == "atoms" )  {
    int Mask = Cmds[1].ToInt();
    short ADS=0, AtomsStart=2;
    olxstr Tmp;
    TXAtomPList Atoms;
    if( Cmds.Count() >= 3 )  {  // a atom drawing style is specified
      if( Cmds[2] == "elp" )  {
        ADS |= adsEllipsoid;
        AtomsStart = 3;
        TXAtom::DefElpMask(Mask);
      }
      if( Cmds[2] == "sph" )  {
        ADS |= adsSphere;
        AtomsStart = 3;
        TXAtom::DefSphMask(Mask);
      }
      if( Cmds[2] == "npd" )  {
        ADS |= adsEllipsoidNPD;
        AtomsStart = 3;
        TXAtom::DefNpdMask(Mask);
      }
      for( int i=AtomsStart; i <  Cmds.Count(); i++ )
        Tmp << Cmds[i] << ' ';
      FXApp->FindXAtoms(Tmp, Atoms);
      if( ADS )  {
        TXAtom *XA;
        for( int i=0; i < Atoms.Count(); i++ )  {
          XA = (TXAtom*)Atoms[i];
          if( XA->DrawStyle() != ADS )  
            Atoms[i] = NULL;
        }
        Atoms.Pack();
      }
      FXApp->UpdateAtomPrimitives(Mask, &Atoms);
    }
    else  {
      FXApp->UpdateAtomPrimitives(Mask);
    }
    return;
  }
  if( Cmds[0] == "bonds" )  {
    int Mask = Cmds[1].ToInt();
    TXBond::DefMask(Mask);
    olxstr Tmp;
    TXBondPList Bonds;
    if( Cmds.Count() >= 3 )  {  // a atom drawing style is specified
      for( int i=2; i <  Cmds.Count(); i++ )  {
        Tmp << Cmds[i] << ' ';
      }
      FXApp->GetBonds(Tmp, Bonds);
      FXApp->UpdateBondPrimitives(Mask, &Bonds);
    }
    else  {
      FXApp->UpdateBondPrimitives(Mask);
    }
    return;
  }
  int Mask = Cmds[Cmds.Count()-1].ToInt();
  Cmds.Delete( Cmds.Count() - 1 );
  TGPCollection *GPC = FXApp->GetRender().FindCollection( Cmds.Text(' ') );
  if( GPC )  {
    if( GPC->ObjectCount() )  {
      GPC->Object(0)->UpdatePrimitives( Mask );
    }
  }
  else  {
    Error.ProcessingError(__OlxSrcInfo, "undefined graphis" );
    return;
  }
}
//..............................................................................
void TMainForm::macARad(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)
{
  FXApp->AtomRad(Cmds[0]);
}
//..............................................................................
void TMainForm::macADS(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXAtomPList Atoms;
  if( Cmds[0] == "elp" )  {
    Cmds.Delete(0);
    FXApp->FindXAtoms(Cmds.Text(' '), Atoms);
    FXApp->SetAtomDrawingStyle(adsEllipsoid, &Atoms);
    return;
  }
  if( Cmds[0] == "sph" )  {
    Cmds.Delete(0);
    FXApp->FindXAtoms(Cmds.Text(' '), Atoms);
    FXApp->SetAtomDrawingStyle(adsSphere, &Atoms);
    return;
  }
  Error.ProcessingError(__OlxSrcInfo, "unknown atom type (elp/sph) supported only" );
}
//..............................................................................
void TMainForm::macAZoom(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( Cmds.Count() == 1 )  FXApp->AtomZoom(Cmds[0].ToDouble());
  if( Cmds.Count() >= 2 )  {
    olxstr Tmp = Cmds[0];  Cmds.Delete(0);
    TXAtomPList Atoms;
    FXApp->FindXAtoms(Cmds.Text(' '), Atoms);
    FXApp->AtomZoom(Tmp.ToDouble(), &Atoms);
  }
}
//..............................................................................
void TMainForm::macBRad(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  FXApp->BondRad(Cmds[0].ToDouble());
}
//..............................................................................
void TMainForm::macKill(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( Cmds.Count() == 1 && !Cmds[0].Comparei("sel") )  {
    TPtrList<AGDrawObject> Objects;
    TGlGroup *sel = FXApp->Selection();
    for( int i=0; i < sel->Count(); i++ )  Objects.Add( (AGDrawObject*)sel->Object(i) );
    FUndoStack->Push( FXApp->DeleteXObjects( Objects ) );
    sel->RemoveDeleted();
  }
  else  {
    TXAtomPList Atoms;
    FXApp->FindXAtoms(Cmds.Text(' '), Atoms);
    if( !Atoms.Count() )  {
      Error.ProcessingError(__OlxSrcInfo, "wrong atom name(s)" );
      return;
    }
    FUndoStack->Push( FXApp->DeleteXAtoms(Atoms) );
  }
}
//..............................................................................
void TMainForm::macLS(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TIns* iF = (TIns*)FXApp->XFile().GetLastLoader();
  for( int i=0; i < Cmds.Count(); i++ )  {
    if( Cmds[i].IsNumber() )  {
      int ls = Cmds[i].ToInt();
      Cmds.Delete(i);
      if( ls != -1 )
        iF->SetIterations(ls);
      break;
    }
  }
  if( Cmds.Count() )
    iF->SetRefinementMethod( Cmds[0] );
}
//..............................................................................
void TMainForm::macUpdateWght(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( !FXApp->CheckFileType<TIns>() )  return;
  TIns *I = (TIns*)FXApp->XFile().GetLastLoader();
  if( I->Wght1().Count() == 0 )  return;
  if( Cmds.IsEmpty() )  { I->Wght() = I->Wght1();  }
  else  {
    I->Wght().Resize(Cmds.Count());
    for( int i=0; i < Cmds.Count(); i++ )  I->Wght()[i] = Cmds[i].ToDouble();
  }
}
//..............................................................................
void TMainForm::macPlan(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error) {
  int plan = Cmds[0].ToInt();
  if( plan == -1 )  return; // leave like it is

  TIns *IF = (TIns*)FXApp->XFile().GetLastLoader();
  IF->SetPlan( plan );
}
//..............................................................................
void TMainForm::macOmit(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)
{
  TIns *IF = (TIns*)FXApp->XFile().GetLastLoader();
  IF->AddIns("omit", Cmds );
}
//..............................................................................
void TMainForm::macExec(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  bool Asyn = true, // synchroniusly
    Cout = true;    // catch output

  olxstr dubFile;

  for( int i=0; i < Options.Count(); i++ )  {
    if( Options.GetName(i) == "s" )  Asyn  = false;
    else if( Options.GetName(i) == "o" )  Cout = false;
    else if( Options.GetName(i) == "d" )  dubFile = Options.GetValue(i);
  }
  olxstr Tmp;
  bool Space;
  for( int i=0; i < Cmds.Count(); i++ )  {
    Space =  (Cmds[i].FirstIndexOf(' ') != -1 );
    if( Space )  Tmp << '\"';
    Tmp << Cmds[i];
    if( Space ) Tmp << '\"';
    Tmp << ' ';
  }
  TBasicApp::GetLog() << (olxstr("EXEC: ") << Tmp << '\n');
#ifdef __WIN32__
  TWinProcess* Process  = new TWinProcess;
#elif defined(__WXWIDGETS__)
  TWxProcess* Process = new TWxProcess;
#endif

  Process->OnTerminateCmds().Assign( FOnTerminateMacroCmds );
  FOnTerminateMacroCmds.Clear();
  TBasicApp::GetLog() << "Please press Ctrl+C to terminate a process...\n";
  if( (Cout && Asyn) || Asyn )  {  // the only combination
    if( !Cout )  {
      SetProcess(Process);
      if( !Process->Execute(Tmp, 0) )  {
        Error.ProcessingError(__OlxSrcInfo, "failed to launch a new process" );
        return;
      }
      return;
    }
    else  {
      SetProcess(Process);
      if( !dubFile.IsEmpty() )  {
        TEFile* df = new TEFile(dubFile, "wb+");
        Process->SetDubStream( df );
      }
      if( !Process->Execute(Tmp, spfRedirected) )  {
        Error.ProcessingError(__OlxSrcInfo, "failed to launch a new process" );
        SetProcess(NULL);
        return;
      }
    }
    return;
  }
  if( !Process->Execute(Tmp, spfSynchronised) )  {
    Error.ProcessingError(__OlxSrcInfo, "failed to launch a new process" );
    return;
  }
  delete Process;
}
//..............................................................................
void TMainForm::macShell(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( Cmds.Count() == 0 )
    wxShell();
  else  {
#ifdef __WIN32__
    ShellExecute((HWND)this->GetHWND(), wxT("open"), uiStr(Cmds[0]), NULL, uiStr(TEFile::CurrentDir()), SW_SHOWNORMAL);
#else
    if( Cmds[0].EndsWith(".htm") || Cmds[0].EndsWith(".html") )
      ProcessXPMacro( olxstr("exec -o getvar(defbrowser) '") << Cmds[0] << '\'', Error);
    else
      ProcessXPMacro( olxstr("exec -o getvar(defexplorer) '") << Cmds[0] << '\'', Error);
    //wxShell( Cmds[0].u_str() );
#endif
  }
}
//..............................................................................
void TMainForm::macSave(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  olxstr Tmp = Cmds[0];
  Cmds.Delete(0);
  olxstr FN = Cmds.Text(' ');
  if( Tmp == "style" )  {
    if( !FN.Length() )
    {  FN = PickFile("Save drawing style", "Drawing styles|*.glds", StylesDir, false);  }
    if( FN.IsEmpty() )  {
      Error.ProcessingError(__OlxSrcInfo, "no file name is given" );
      return;
    }
    Tmp = TEFile::ExtractFilePath(FN);
    if( !Tmp.IsEmpty() )  {
      if( !(StylesDir.LowerCase() == Tmp.LowerCase()) )  {
        TBasicApp::GetLog().Info(olxstr("Styles folder is changed to: ") << Tmp);
        StylesDir = Tmp;
      }
    }
    else  {
      if( !StylesDir.IsEmpty() )  Tmp = StylesDir;
      else                        Tmp = FXApp->BaseDir();
      Tmp << FN;  FN = Tmp;
    }
    FN = TEFile::ChangeFileExt(FN, "glds");
    TDataFile F;
    FXApp->GetRender().Styles()->ToDataItem(F.Root().AddItem("style"));
    try  {  F.SaveToXLFile(FN); }
    catch( TExceptionBase& )  {
      Error.ProcessingError(__OlxSrcInfo, "failed to save file: " ) << FN;
    }
    return;
  }
  if( Tmp == "scene" )  {
    if( FN.IsEmpty() )
      FN = PickFile("Save scene parameters", "Scene parameters|*.glsp", SParamDir, false);
    if( FN.IsEmpty() )  {
      Error.ProcessingError(__OlxSrcInfo, "no file name is given" );
      return;
    }
    Tmp = TEFile::ExtractFilePath(FN);
    if( !Tmp.IsEmpty() )  {
      if( !(SParamDir.LowerCase() == Tmp.LowerCase()) )  {
        TBasicApp::GetLog().Info(olxstr("Scene parameters folder is changed to: ") << Tmp);
        SParamDir = Tmp;
      }
    }
    else  {
      if( SParamDir.Length() )  Tmp = SParamDir;
      else                      Tmp = FXApp->BaseDir();
      Tmp << FN;  FN = Tmp;
    }
    FN = TEFile::ChangeFileExt(FN, "glsp");
    TDataFile F;
    SaveScene(&F.Root());
    try  {  F.SaveToXLFile(FN);  }
    catch( TExceptionBase& )  {
      Error.ProcessingError(__OlxSrcInfo, "failed to save file: ") << FN ;
    }
    return;
  }
  if( Tmp == "view" )  {
    if( FXApp->XFile().GetLastLoader() != NULL )  {
      Tmp = (Cmds.Count() == 1) ? TEFile::ChangeFileExt(Cmds[0], "xlds") :
                                  TEFile::ChangeFileExt(FXApp->XFile().GetFileName(), "xlds");
      TDataFile DF;
      TDataItem *style = DF.Root().AddItem("style");
      FXApp->GetRender().Styles()->ToDataItem(style);
      TDataItem *basis = DF.Root().AddItem("basis");
      FXApp->GetRender().GetBasis().ToDataItem(basis);
      DF.SaveToXLFile(Tmp);
    }
  }
}
//..............................................................................
void TMainForm::macLoad(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)
{
  olxstr Tmp = Cmds[0];
  Cmds.Delete(0);
  olxstr FN = Cmds.Text(' ');
  if( Tmp == "style" )  {
    if( FN.IsEmpty() )
      FN = PickFile("Load drawing style", "Drawing styles|*.glds", StylesDir, true);
    if( FN.IsEmpty() )  {
      Error.ProcessingError(__OlxSrcInfo, "no file name is given" );
      return;
    }
    Tmp = TEFile::ExtractFilePath(FN);
    if( !Tmp.IsEmpty() )  {
      if( StylesDir.Comparei(Tmp) )  {
        TBasicApp::GetLog().Info(olxstr("Styles folder is changed to: ") + Tmp);
        StylesDir = Tmp;
      }
    }
    else  {
      if( !StylesDir.IsEmpty() )
        Tmp = StylesDir;
      else
        Tmp = FXApp->BaseDir();
      Tmp << FN;
      FN = Tmp;
    }
    FN = TEFile::ChangeFileExt(FN, "glds");
    TEFile::CheckFileExists(__OlxSourceInfo, FN);
    TDataFile F;
    F.LoadFromXLFile(FN, NULL);

    FXApp->GetRender().Styles()->FromDataItem(F.Root().FindItem("style"));
    FXApp->CreateObjects( true );
    FN = FXApp->GetRender().Styles()->LinkFile();
    if( !FN.IsEmpty() )  {
      if( TEFile::FileExists(FN) )  {
        F.LoadFromXLFile(FN, NULL);
        LoadScene(&F.Root());
      }
      else
        TBasicApp::GetLog().Error(olxstr("Load::style: link file does not exist: ") << FN );
    }
    return;
  }
  if( Tmp == "scene" )  {
    if( FN.IsEmpty() )
      FN = PickFile("Load scene parameters", "Scene parameters|*.glsp", SParamDir, true);
    if( FN.IsEmpty() )  {
      Error.ProcessingError(__OlxSrcInfo, "no file name is given" );
      return;
    }
    Tmp = TEFile::ExtractFilePath(FN);
    if( !Tmp.IsEmpty() )  {
      if( SParamDir.Comparei(Tmp) != 0 )  {
        TBasicApp::GetLog().Info(olxstr("Scene parameters folder is changed to: ") << Tmp);
        SParamDir = Tmp;
      }
    }
    else  {
      if( SParamDir.Length() )
        Tmp = SParamDir;
      else
        Tmp = FXApp->BaseDir();
      Tmp << FN;
      FN = Tmp;
    }
    FN = TEFile::ChangeFileExt(FN, "glsp");
    TEFile::CheckFileExists(__OlxSourceInfo, FN);
    TDataFile DF;
    DF.LoadFromXLFile(FN, NULL);
    LoadScene( &DF.Root() );
    return;
  }
  if( Tmp == "view" )  {
    if( FXApp->XFile().GetLastLoader() != NULL )  {
      if( Cmds.Count() == 1 )
        Tmp = TEFile::ChangeFileExt(Cmds[0], "xlds");
      else
        Tmp = TEFile::ChangeFileExt(FXApp->XFile().GetFileName(), "xlds");
    }
    if( TEFile::FileExists(Tmp) )  {
      TDataFile DF;
      TStrList log;
      DF.LoadFromXLFile(Tmp, &log);
      TDataItem *style = DF.Root().FindItem("style");
      if( style == NULL )
        FXApp->GetRender().Styles()->FromDataItem(DF.Root().FindItem("DStyle"));
      else  {
        FXApp->GetRender().Styles()->FromDataItem(style);
        TDataItem *basis = DF.Root().FindItem("basis");
        if( basis )  FXApp->GetRender().Basis()->FromDataItem(basis);
      }
      FXApp->Draw();
    }
    return;
  }
  Error.ProcessingError(__OlxSrcInfo, "undefined parameter" );
}
//..............................................................................
void TMainForm::macLink(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  olxstr FN, Tmp;
  if( Cmds.IsEmpty() )
    FN = PickFile("Load scene parameters", "Scene parameters|*.glsp", SParamDir, false);
  else
    FN = Cmds.Text(' ');

  if( FN.IsEmpty() )  {
    Error.ProcessingError(__OlxSrcInfo, "no file name is given" );
    return;
  }
  if( FN == "none" )  {
    FXApp->GetRender().Styles()->LinkFile(EmptyString);
    TBasicApp::GetLog().Info("The link has been removed...");
    return;
  }
  Tmp = TEFile::ExtractFilePath(FN);
  if( Tmp.IsEmpty() )  {
    if( !SParamDir.IsEmpty() )
      Tmp = SParamDir;
    else
      Tmp = FXApp->BaseDir();
    FN = (Tmp << FN );
  }
  FN = TEFile::ChangeFileExt(FN, "glsp");
  if( TEFile::FileExists(FN) )  { FXApp->GetRender().Styles()->LinkFile(FN); }
  else  {
    Error.ProcessingError(__OlxSrcInfo, "file does not exists : ") << FN;
    return;
  }
}
//..............................................................................
void TMainForm::macStyle(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  olxstr FN, Tmp;
  if( !Cmds.Count() && !Options.Count() )  {
    FN = "Default style: ";
    if( !DefStyle.IsEmpty() )
      FN << DefStyle;
    else
      FN << "none";
    TBasicApp::GetLog().Info(FN);
    return;
  }
  else  {
    if( Cmds.Count() != 0 )  {
      if( Cmds[0] == "none" )  {
        DefStyle = EmptyString;
        Error.ProcessingError(__OlxSrcInfo, "default style is cleared" );
        return;
      }
      FN = Cmds.Text(' ');
      Tmp = TEFile::ExtractFilePath(FN);
      if( Tmp.IsEmpty() )  {
        if( StylesDir.Length() )
          Tmp = StylesDir;
        else
          Tmp = FXApp->BaseDir();
        FN = (Tmp << FN);
      }
      if( TEFile::FileExists(FN) )
        DefStyle = FN;
      else  {
        Error.ProcessingError(__OlxSrcInfo, "specified file does not exists" );
        return;
      }
      return;
    }
    else  {
      if( Options.Count() != 0 )  {
        if( Options.GetName(0) == "s" )  {
          FN = PickFile("Load drawing style", "Drawing styles|*.glds", StylesDir, false);
          if( FN.Length() )
            DefStyle = FN;
          else  {
            Error.ProcessingError(__OlxSrcInfo, "no file name is given" );
            return;
          }
        }
        Error.ProcessingError(__OlxSrcInfo, "wrong option: ") << Options.GetName(0);
        return;
      }
    }
  }
}
//..............................................................................
void TMainForm::macScene(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  olxstr FN, Tmp;
  if( Cmds.IsEmpty()  && Options.IsEmpty() )  {
    TBasicApp::GetLog() << DefSceneP << '\n';
    return;
  }
  if( Cmds.Count() != 0 )  {
    if( Cmds[0] == "none" )  {
      Error.ProcessingError(__OlxSrcInfo, "default scene is cleared" );
      DefSceneP = EmptyString;
      return;
    }
    FN = Cmds.Text(' ');
    Tmp = TEFile::ExtractFilePath(FN);
    if( Tmp.IsEmpty() )  {
      if( !SParamDir.IsEmpty() )
        Tmp = SParamDir;
      else
        Tmp = FXApp->BaseDir();
      FN = (Tmp << FN);
    }
    if( TEFile::FileExists(FN) )
      DefSceneP = FN;
    else  {
      Error.ProcessingError(__OlxSrcInfo, "specified file does not exists" );
      return;
    }
    return;
  }
  else  {
    if( Options.Count() != 0 )  {
      if( Options.GetName(0) == "s" )  {
        FN = PickFile("Load scene parameters", "Scene parameters|*.glsp", SParamDir, false);
        if( FN.Length() )
          DefSceneP = FN;
        else  {
          Error.ProcessingError(__OlxSrcInfo, "no file name is given" );
          return;
        }
      }
    }
  }
}
//..............................................................................
void TMainForm::macSyncBC(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  FXApp->XAtomDS2XBondDS("Sphere");  
}
//..............................................................................
void TMainForm::macCeiling(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( Cmds.Count() == 1 )  {
    if( !Cmds[0].Comparei("on") )
      FXApp->GetRender().Ceiling()->Visible(true);
    else if( !Cmds[0].Comparei("off") )
      FXApp->GetRender().Ceiling()->Visible(false);
    else
      Error.ProcessingError(__OlxSrcInfo, "wrong arguments" );
    FXApp->Draw();
  }
}
//..............................................................................
void TMainForm::macFade(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  FFadeVector[0] = Cmds[0].ToDouble();
  FFadeVector[1] = Cmds[1].ToDouble();
  FFadeVector[2] = Cmds[2].ToDouble();
  if( FFadeVector[1] < 0 || FFadeVector[1] > 1 )  {
    Error.ProcessingError(__OlxSrcInfo, "wrong arguments" );
    return;
  }
  TGlBackground *C = FXApp->GetRender().Ceiling();
  TGlBackground *G = FXApp->GetRender().Background();
  if( !C->Visible() )  {
    if( !G->Visible() )  {
      C->LT( FXApp->GetRender().LightModel.ClearColor() );
      C->RT( FXApp->GetRender().LightModel.ClearColor() );
      C->LB( FXApp->GetRender().LightModel.ClearColor() );
      C->RB( FXApp->GetRender().LightModel.ClearColor() );
    }
    else  {
      C->LT( G->LT() );
      C->RT( G->RT() );
      C->LB( G->LB() );
      C->RB( G->RB() );
    }

    C->Visible(true);
  }
  FMode = FMode | mFade;
  return;
}
//..............................................................................
void TMainForm::macWaitFor(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  // we need to call the timer in case it is disabled ...
  if( !Cmds[0].Comparei("fade") )  {
    if( !IsVisible() )  return;
    while( FMode & mFade )  {
      FParent->Dispatch();
      Dispatch(ID_TIMER, -1, (AActionHandler*)this, NULL);
    }
  }
  if( !Cmds[0].Comparei("xfader") )  {
    if( !IsVisible() )  return;
    while( FXApp->GetFader().GetPosition() < 1 && FXApp->GetFader().Visible() )  {
      FParent->Dispatch();
      Dispatch(ID_TIMER, -1, (AActionHandler*)this, NULL);
    }
  }
  else if( !Cmds[0].Comparei("rota") )  {
    while( FMode & mRota )  {
      FParent->Dispatch();
      Dispatch(ID_TIMER, -1, (AActionHandler*)this, NULL);
    }
  }
  else if( !Cmds[0].Comparei("process") )  {
    while( FProcess != NULL )  {
      FParent->Dispatch();
      Dispatch(ID_TIMER, -1, (AActionHandler*)this, NULL);
    }
  }
}
//..............................................................................
void TMainForm::macOccu(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( !Cmds.LastStr().IsNumber() )  {
    Error.ProcessingError(__OlxSrcInfo, "wrong occupancy value" );
    return;
  }
  TCAtomPList CAtoms;
  double occp = Cmds.LastStr().ToDouble();
  Cmds.Delete(Cmds.Count()-1);
  FXApp->FindCAtoms(Cmds.Text(' '), CAtoms);
  for( int i=0; i < CAtoms.Count(); i++ )  {
    CAtoms[i]->SetOccp(occp);
    // reset the variable as it has priority when saving to a file
    CAtoms[i]->SetOccpVar(0);
  }
}
//..............................................................................
void TMainForm::macAddIns(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  // synchronise atom names
  FXApp->XFile().UpdateAsymmUnit();

  TIns * Ins = (TIns*)FXApp->XFile().GetLastLoader();
  olxstr InsName;
  // check for long instructions
  if( Cmds.Count() == 1 )  {
    int spindex = Cmds[0].IndexOf(' ');
    if( spindex != -1 )  {
      InsName = Cmds[0].SubStringTo(spindex).Trim(' ');
      Cmds[0].Delete(0, spindex+1);
    }
    else  {
      InsName = Cmds[0];
      Cmds.Delete(0);
    }
  }
  else  {
    InsName = Cmds[0];
    Cmds.Delete(0);
  }

  if( !Ins->AddIns(InsName, Cmds) )  {
    Error.ProcessingError(__OlxSrcInfo, "could not add instruction" );
    return;
  }
  // synchronise instruction params
  Ins->UpdateParams();
}
//..............................................................................
void TMainForm::macFixUnit(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)
{
  if( !FXApp->CheckFileType<TIns>() )  return;

  TIns * Ins = (TIns*)FXApp->XFile().GetLastLoader();
  FXApp->XFile().UpdateAsymmUnit();
  bool atoms = false;
  TCAtom *CA;
  for(int i=0; i < FXApp->XFile().GetAsymmUnit().AtomCount(); i++ )  {
    CA = &FXApp->XFile().GetAsymmUnit().GetAtom(i);
    if( CA->IsDeleted() )  continue;
    if( CA->GetAtomInfo() != iQPeakIndex )
    {  atoms = true;  break;  }
  }
  if( atoms )  {  Ins->FixUnit();  }
}
//..............................................................................
void TMainForm::macHtmlPanelSwap(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Cmds.IsEmpty() )  {
    FHtmlOnLeft = !FHtmlOnLeft;
    OnResize();
  }
  else  {
    bool changed = false;
    if( Cmds[0].Comparei("left") == 0 )  {
      if( !FHtmlOnLeft )  {
        changed = FHtmlOnLeft = true;
      }
    }
    else if( Cmds[0].Comparei("right") == 0 )  {
      if( FHtmlOnLeft )  {
        changed = true;
        FHtmlOnLeft = false;
      }
    }
    else  {
      E.ProcessingError( __OlxSrcInfo, olxstr("uncknown option '") << Cmds[0] << '\'');
      return;
    }
    if( changed )  OnResize();
  }
}
//..............................................................................
void TMainForm::macHtmlPanelVisible(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Cmds.Count() == 1 )  {
    FHtmlMinimized = !Cmds[0].ToBool();
    OnResize();
    miHtmlPanel->Check(!FHtmlMinimized);
  }
  else  {
    bool show = Cmds[0].ToBool();
    TPopupData *pd = GetPopup( Cmds[1] );
    if( pd != NULL )  {
      if( show && !pd->Dialog->IsShown() )  pd->Dialog->Show();
      if( !show && pd->Dialog->IsShown() )  pd->Dialog->Hide();
    }
    else  {
      E.ProcessingError(__OlxSrcInfo, "undefined popup name" );
    }
  }
}
//..............................................................................
void TMainForm::macHtmlPanelWidth(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Cmds.IsEmpty() )  {
    TBasicApp::GetLog().Info( olxstr("Current HTML panel width: ") << FHtmlPanelWidth);
    return;
  }
  if( Cmds.Count() == 1 )  {
    FHtmlWidthFixed = !Cmds[0].EndsWith('%');
    float width = (FHtmlWidthFixed ? Cmds[0].ToDouble() : Cmds[0].SubStringTo(Cmds[0].Length()-1).ToDouble());
    if( !FHtmlWidthFixed )  {
      width /= 100;
      if( width < 0.01 )
        FHtmlMinimized = true;
      else  {
        if( width > 0.75 )  width = 0.75;
        FHtmlMinimized = false;
        FHtmlPanelWidth = width;
      }
    }
    else  {
      if( width < 10 )
        FHtmlMinimized = true;
      else  {
        int w, h;
        GetClientSize(&w, &h);
        if( width > w*0.75 )  width = w*0.75;
        FHtmlMinimized = false;
        FHtmlPanelWidth = width;
      }
    }
    OnResize();
    return;
  }
  E.ProcessingError(__OlxSrcInfo, "wrong number of arguments" );
  return;
}
//..............................................................................
void TMainForm::macQPeakScale(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Cmds.IsEmpty() )  {
    TBasicApp::GetLog().Info( olxstr("Current Q-Peak scale: ") << FXApp->QPeakScale() );
    return;
  }
  if( Cmds.Count() == 1 )  {
    if( FXApp->XFile().GetLastLoader() == NULL )  {
      E.ProcessingError(__OlxSrcInfo, "file is not loaded" );
      return;
    }
    float scale = Cmds[0].ToDouble();
    FXApp->QPeakScale(scale);
    return;
  }
  E.ProcessingError(__OlxSrcInfo, "wrong number of arguments" );
  return;
}
//..............................................................................
void TMainForm::macLabel(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Cmds.Count() )  {
    TXAtomPList Atoms;
    FXApp->FindXAtoms(Cmds.Text(" "), Atoms);
    for( int i=0; i < Atoms.Count(); i++ )  {
      FXApp->CreateLabel( Atoms[i], fntPLabels )->Visible(true);
    }
    return;
  }
  E.ProcessingError(__OlxSrcInfo, "wrong number of arguments" );
  return;
}
//..............................................................................
void TMainForm::macCalcChn(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( !FXApp->XFile().GetLastLoader() && !Cmds.Count() )  {
    Error.ProcessingError(__OlxSrcInfo, "nor file is loaded neither formula is provided" );
    return;
  }
  TCHNExp chn(FXApp->XFile().GetAsymmUnit().GetAtomsInfo());
  double C=0, H=0, N=0, Mr=0;
  if( Cmds.Count() == 1 )  {
    chn.LoadFromExpression(Cmds[0]);
    chn.CHN(C, H, N, Mr);
    TBasicApp::GetLog() << (olxstr("Molecular weight: ") << Mr << '\n');
    olxstr Msg("C: ");   Msg << C*100./Mr  << " H: " << H*100./Mr << " N: " << N*100./Mr;
    TBasicApp::GetLog() << (Msg << '\n' << '\n');
    TBasicApp::GetLog() << (olxstr("Full composition:\n") << chn.Composition() << '\n');
    return;
  }
  chn.LoadFromExpression(FXApp->XFile().GetAsymmUnit().SummFormula(EmptyString));
  chn.CHN(C, H, N, Mr);
  TBasicApp::GetLog() << (olxstr("Molecular weight: ") << Mr << '\n');
  olxstr Msg("C: ");   Msg << C*100./Mr << " H: " << H*100./Mr << " N: " << N*100./Mr;
  TBasicApp::GetLog() << (Msg << '\n' << '\n');
  TBasicApp::GetLog() << (olxstr("Full composition:\n") << chn.Composition() << '\n');
}
//..............................................................................
void TMainForm::macCalcMass(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TIPattern *ip = NULL;
  if( !FXApp->XFile().GetLastLoader() && !Cmds.Count() )  {
    Error.ProcessingError(__OlxSrcInfo, "nor file is loaded neither formula is provided" );
    return;
  }
  ip = new TIPattern(FXApp->XFile().GetAsymmUnit().GetAtomsInfo());
  TSPoint *point;
  olxstr Msg;
  int yVal;
  if( Cmds.Count() == 1 )  {
    if( !ip->Calc(Cmds[0], Msg, true, 0.5) )  {
      Error.ProcessingError(__OlxSrcInfo, "could not parse the given expression: ") << Msg;
      delete ip;
      return;
    }
    for( int i=0; i < ip->PointCount(); i++ )  {
      point = ip->Point(i);
      if( point->Y < 0.001 )  break;
      Msg = point->X;
      Msg.Format(11, true, ' ');
      Msg << ": " << point->Y;
      TBasicApp::GetLog() << (Msg << '\n');
    }
    TBasicApp::GetLog() << ("    -- NOTE THAT NATURAL DISTRIBUTION OF ISOTOPES IS ASSUMED --    \n");
    TBasicApp::GetLog() << ("******* ******* SPECTRUM ******* ********\n");
    ip->SortDataByMolWeight();
    for( int i=0; i < ip->PointCount(); i++ )  {
      point = ip->Point(i);
      if( point->Y < 1 )  continue;
      Msg = point->X;
      Msg.Format(11, true, ' ');
      Msg << "|";
      yVal = Round(point->Y/2);
      for( int j=0; j < yVal; j++ )  Msg << '-';
      TBasicApp::GetLog() << (Msg << '\n');
    }
    delete ip;
    return;
  }
  if( !ip->Calc(FXApp->XFile().GetAsymmUnit().SummFormula(EmptyString), Msg, true, 0.5) )  {
    Error.ProcessingError(__OlxSrcInfo, "could not parse the given expression: ") << Msg;
    delete ip;
    return;
  }
  for( int i=0; i < ip->PointCount(); i++ )  {
    point = ip->Point(i);
      if( point->Y < 0.001 )  break;
    Msg = point->X;
    Msg.Format(11, true, ' ');
    Msg << ": " << point->Y;
    TBasicApp::GetLog() << (Msg << '\n');
  }
  TBasicApp::GetLog() << ("    -- NOTE THAT NATURAL DISTRIBUTION OF ISOTOPES IS ASSUMED --    \n");
  TBasicApp::GetLog() << ("******* ******* SPECTRUM ******* ********\n");
  ip->SortDataByMolWeight();
  for( int i=0; i < ip->PointCount(); i++ )  {
    point = ip->Point(i);
    if( point->Y < 1 )  continue;
    Msg = point->X;
    Msg.Format(11, true, ' ');
    Msg << "|";
    yVal = Round(point->Y/2);
    for( int j=0; j < yVal; j++ )  Msg << '-';
    TBasicApp::GetLog() << (Msg << '\n');
  }
  delete ip;
}
//..............................................................................
void TMainForm::macFocus(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  FGlCanvas->SetFocus();
}
//..............................................................................
void TMainForm::macRefresh(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  Dispatch(ID_TIMER, msiEnter, (AEventsDispatcher*)this, NULL);
  FHtml->Update();
  FXApp->Draw();
  wxTheApp->Dispatch();
}
//..............................................................................
void TMainForm::macMove(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  if( Cmds.IsEmpty() )  {
    FXApp->MoveToCenter();
    return;
  }
  bool clearSelection = !Options.Contains("cs");
  TXAtom *A = FXApp->GetXAtom(Cmds[0], clearSelection);
  TXAtom *B = FXApp->GetXAtom(Cmds[1], clearSelection);
  if( A == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "wrong atom - ") << Cmds[0];
    return;
  }
  if( !B )  {
    E.ProcessingError(__OlxSrcInfo, "wrong atom - " ) << Cmds[1];
    return;
  }
  bool copy = Options.Contains('c');
  FXApp->MoveFragment(A, B, copy);
}
//..............................................................................
void TMainForm::macShowH(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  if( Cmds.Count() == 2 )  {
    bool v = Cmds[1].ToBool();
    if( Cmds[0] == "a" )  {
      if( v && !FXApp->HydrogensVisible() )  {
        TStateChange sc(prsHVis, true);
        FXApp->HydrogensVisible( true );
        OnStateChange->Execute((AEventsDispatcher*)this, &sc);
      }
      else if( !v && FXApp->HydrogensVisible() )  {
        TStateChange sc(prsHVis, false);
        FXApp->HydrogensVisible( false );
        OnStateChange->Execute((AEventsDispatcher*)this, &sc);
      }
    }
    else if( Cmds[0] == "b" )  {
      if( v && !FXApp->HBondsVisible() )  {
        TStateChange sc(prsHBVis, true);
        FXApp->HBondsVisible( true );
        OnStateChange->Execute((AEventsDispatcher*)this, &sc);
      }
      else if( !v && FXApp->HBondsVisible() )  {
        TStateChange sc(prsHBVis, false);
        FXApp->HBondsVisible( false );
        OnStateChange->Execute((AEventsDispatcher*)this, &sc);
      }
    }
  }
  else  {
    if( FXApp->HydrogensVisible() && !FXApp->HBondsVisible() )  {
      TStateChange sc(prsHBVis, true);
      FXApp->HBondsVisible( true );
      OnStateChange->Execute((AEventsDispatcher*)this, &sc);
    }
    else if( FXApp->HydrogensVisible() && FXApp->HBondsVisible() )  {
      TStateChange sc(prsHBVis|prsHVis, false);
      FXApp->HBondsVisible( false );
      FXApp->HydrogensVisible( false );
      OnStateChange->Execute((AEventsDispatcher*)this, &sc);
    }
    else if( !FXApp->HydrogensVisible() && !FXApp->HBondsVisible() )  {
      TStateChange sc(prsHVis, true);
      FXApp->HydrogensVisible( true );
      OnStateChange->Execute((AEventsDispatcher*)this, &sc);
    }
  }
}
//..............................................................................
void TMainForm::macFvar(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( !FXApp->CheckFileType<TIns>() )  return;

  TIns *I = (TIns*)FXApp->XFile().GetLastLoader();
  if( !Cmds.Count() )  {
    olxstr tmp = "Free variables: ";
    for( int i=0; i < I->Vars().Count(); i++ )
      tmp << olxstr::FormatFloat(3, I->Vars()[i]) << ' ';
    TBasicApp::GetLog() << (tmp << '\n');
    return;
  }
  float fvar = 0;
  for( int i=0; i < Cmds.Count(); i++ )  {
    if( Cmds[i].IsNumber() )  {
      fvar = Cmds[i].ToDouble();
      Cmds.Delete(i);
      break;
    }
  }
  if( Cmds.IsEmpty() )  {
    E.ProcessingError(__OlxSrcInfo, "atom names are expected" );
    return;
  }
  TCAtomPList CAtoms;
  FXApp->FindCAtoms(Cmds.Text(' '), CAtoms);
  if( CAtoms.Count() == 2 && !fvar )  {
    I->AddVar(0.5);
    int num = I->Vars().Count();
    CAtoms.Item(0)->SetOccpVar(num*10+1);
    CAtoms.Item(1)->SetOccpVar(-num*10-1);
  }
  else  {
    if( fvar )  {
      for(int i=0; i < CAtoms.Count(); i++ )
        CAtoms[i]->SetOccpVar(fvar);
      int iv = (int)fabs(fvar/10);
      while( I->Vars().Count() < iv )  {
        I->AddVar(0.5);
      }
    }
  }
}
//..............................................................................
void TMainForm::macSump(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)
{
  if( !FXApp->CheckFileType<TIns>() )  return;

  TIns *I = (TIns*)FXApp->XFile().GetLastLoader();
  TCAtomPList CAtoms;
  FXApp->FindCAtoms(Cmds.Text(' '), CAtoms);
  if( CAtoms.Count() < 2 )  {
    E.ProcessingError(__OlxSrcInfo, "two atoms at least are expected" );
    return;
  }
  olxstr sump;
  for( int i=0; i < CAtoms.Count(); i++ )  {
    if( CAtoms[i]->GetOccpVar() != 0 )  {
      sump << ' ';
      sump << CAtoms[i]->Label();
    }
  }
  if( !sump.IsEmpty() )  {
    E.ProcessingError(__OlxSrcInfo, "these/this atom(s) already have assigned free variables -" ) << sump;
    return;
  }
  sump = "1 0.01";
  for( int i=0; i < CAtoms.Count(); i++ )  {
    I->AddVar(1./CAtoms.Count());
    CAtoms[i]->SetOccpVar( I->Vars().Count()*10+1 );
    sump << ' ';
    sump << "1.00 ";  // weight
    sump << I->Vars().Count();  // variable
  }
  TStrList SL;
  SL.Add(sump);
  I->AddIns("SUMP", SL);
}
//..............................................................................
void TMainForm::macPart(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( !FXApp->CheckFileType<TIns>() )  return;

  TIns *Ins = (TIns*)FXApp->XFile().GetLastLoader();
  short part = -1, partCount = 1;
  bool linkOccu = false;
  TXAtomPList Atoms;
  TSAtom *SA;
  for( int i=0; i < Cmds.Count(); i++ )  {
    if( Cmds[i].IsNumber() )  {
      part = Cmds[i].ToInt();
      Cmds.Delete(i);
      break ;
    }
  }

  if( Cmds.Count() )  FXApp->FindXAtoms(Cmds.Text(' '), Atoms);
  if( Atoms.IsEmpty() )  {
    E.ProcessingError(__OlxSrcInfo, "non zero number of atoms is expected" );
    return;
  }
  for( int i=0; i < Options.Count(); i++ )  {
    if( Options.GetName(i).Comparei("p") == 0 )  {
      partCount = Options.GetValue(i).ToInt();
      if( !partCount || partCount < 0 || (Atoms.Count()%partCount) )  {
        E.ProcessingError(__OlxSrcInfo, "wrong number of parts" );
        return;
      }
    }
    else if( Options.GetName(i).Comparei("lo") == 0 )
      linkOccu = true;
  }
  int startVar;
  if( linkOccu )  {
    // -21 -> 21
    if( partCount == 2 )  {
      Ins->AddVar(0.5);
      startVar = Ins->Vars().Count()*10+1;
    }
    // SUMP
    if( partCount > 2 )  {
      startVar = Ins->Vars().Count()*10+1;
      for( int i=0; i < partCount; i++ )  {  Ins->AddVar(1./partCount);  }
    }
  }

  if( part == -1 ) part = FXApp->XFile().GetLattice().GetAsymmUnit().GetMaxPart();

  for( int i=0; i < partCount; i++ )  {
    for( int j=(Atoms.Count()/partCount)*i; j < (Atoms.Count()/partCount)*(i+1); j++ )  {
      Atoms[j]->Atom().CAtom().SetPart( part );
      for( int k=0; k <  Atoms[j]->Atom().NodeCount(); k++ )  {
        SA = &Atoms[j]->Atom().Node(k);
        if( SA->GetAtomInfo() == iHydrogenIndex )
          SA->CAtom().SetPart(part);
      }
      if( linkOccu )  {
        if( partCount == 2 )  {
          if( i )  Atoms[j]->Atom().CAtom().SetOccpVar(startVar);
          else     Atoms[j]->Atom().CAtom().SetOccpVar(-startVar);
        }
        if( partCount > 2 )  {
          Atoms[j]->Atom().CAtom().SetOccpVar(startVar + partCount*10);
        }
      }
    }
    part++;
  }
  if( linkOccu )  {
    if( partCount > 2 )  {
      olxstr Sump = "1 0.01 ";
      for( int i=0; i < partCount; i++ )  {
        Sump << "1.00 " << (startVar + i*10) << ' ';
      }
      TStrList SL;
      SL.Add(Sump);
      Ins->AddIns("SUMP", SL);
    }
  }
}
void TMainForm::macAfix(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  short afix = -1;
  TXAtomPList Atoms;
  for( int i=0; i < Cmds.Count(); i++ )  {
    if( Cmds[i].IsNumber() )  {
      afix = Cmds[i].ToInt();
      Cmds.Delete(i);
      break ;
    }
  }
  if( afix == -1 )  {
    E.ProcessingError(__OlxSrcInfo, "afix should be specified" );
    return;
  }
  if( Cmds.Count() != 0 )
    FXApp->FindXAtoms(Cmds.Text(' '), Atoms);
  if( Atoms.IsEmpty() )  {
    E.ProcessingError(__OlxSrcInfo, "no atoms provided" );
    return;
  }
  for( int i=0; i < Atoms.Count(); i++ )
    Atoms[i]->Atom().CAtom().SetAfix( afix );
}
//..............................................................................
void TMainForm::macDegen(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TXAtom* XA = FXApp->GetXAtom(Cmds[0], !Options.Contains("cs"));
  if( XA == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "wrong atom: " ) << Cmds[0];
    return;
  }
  olxstr deg( XA->Atom().CAtom().GetDegeneracy() );
  TBasicApp::GetLog() << (deg << '\n');
}
//..............................................................................
void TMainForm::macSwapExyz(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  throw TNotImplementedException(__OlxSourceInfo);
}
//..............................................................................
void TMainForm::macAddExyz(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  throw TNotImplementedException(__OlxSourceInfo);
}
//..............................................................................
void TMainForm::macRRings(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TTypeList< TSAtomPList > rings;
  try  {  FXApp->FindRings(Cmds[0], rings);  }
  catch( const TExceptionBase& exc )  {  throw TFunctionFailedException(__OlxSourceInfo, exc);  }

  double l = 1.39, e = 0.001;

  if( Cmds.Count() > 1 && Cmds[1].IsNumber() )
    l = Cmds[1].ToDouble();
  if( Cmds.Count() > 2 && Cmds[2].IsNumber() )
    e = Cmds[2].ToDouble();

  for( int i=0; i < rings.Count(); i++ )  {
    TSimpleRestraint& dfix = FXApp->XFile().GetAsymmUnit().RestrainedDistances().AddNew();
    dfix.SetValue( l );
    dfix.SetEsd( e );
    TSimpleRestraint& flat = FXApp->XFile().GetAsymmUnit().RestrainedPlanarity().AddNew();
    flat.SetEsd( 0.1 );
    for( int j=0; j < rings[i].Count(); j++ )  {
      flat.AddAtom( rings[i][j]->CAtom(), &rings[i][j]->GetMatrix(0) );
      if( (j+1) < rings[i].Count() )
        dfix.AddAtomPair( rings[i][j]->CAtom(), &rings[i][j]->GetMatrix(0),
                          rings[i][j+1]->CAtom(), &rings[i][j+1]->GetMatrix(0));
      else
        dfix.AddAtomPair( rings[i][j]->CAtom(), &rings[i][j]->GetMatrix(0),
                          rings[i][0]->CAtom(), &rings[i][0]->GetMatrix(0));
    }
  }
}
//..............................................................................
void ParseResParam(TStrObjList &Cmds, double& esd, double* len = NULL, double* len1 = NULL, double* ang = NULL)  {
  for( int i=0; i < Cmds.Count(); i++ )  {
    if( Cmds[i].IsNumber() )  {
      double v = Cmds[i].ToDouble();
      if( v > 0.25 )  {
        if( v < 5 )  {
          if( len == NULL )
            throw TInvalidArgumentException(__OlxSourceInfo, "too many numerical arguments, length");
          else  {
            if( *len == 0 )  {
              *len = v;
              if( len1 != NULL )  *len1 = v;
            }
            else if( len1 != NULL )
              *len1 = v;
            else
              throw TInvalidArgumentException(__OlxSourceInfo, "too many numerical arguments, length");
          }
        }
        else if( v >= 15 && v <= 180 )  {  // looks line an angle?
          if( ang != NULL )  *ang = v;
          else
            throw TInvalidArgumentException(__OlxSourceInfo, "too many numerical arguments, angle");
        }
        else
          throw TInvalidArgumentException(__OlxSourceInfo, Cmds[i]);
      }
      else // v < 0.25
        esd = v;
      Cmds.Delete(i);
      i--;
    }
  }
}
//..............................................................................
void TMainForm::macDfix(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  double fixLen = 0, esd = 0.02;  // length and esd for dfix
  ParseResParam(Cmds, esd, &fixLen);
  if( fixLen == 0 )  {
    E.ProcessingError(__OlxSrcInfo, "please specify the distance to restrain to" );
    return;
  }

  TXAtomPList Atoms;
  FindXAtoms(Cmds, Atoms, false, false);
  if( Atoms.IsEmpty() )  {
    TGlGroup* sel = FXApp->Selection();
    
    if( sel->Count() > 1 ) {
      TSimpleRestraint *sr = &FXApp->XFile().GetAsymmUnit().RestrainedDistances().AddNew();
      sr->SetEsd( esd );
      sr->SetValue( fixLen );
      for( int i=0; i < sel->Count(); i++ )  {
        if( !EsdlInstanceOf(*sel->Object(i), TXBond) )  continue;
        const TXBond* xb = (TXBond*)sel->Object(i);
        sr->AddAtomPair(xb->Bond().A().CAtom(), &xb->Bond().A().GetMatrix(0),
          xb->Bond().B().CAtom(), &xb->Bond().B().GetMatrix(0));
      }
      FXApp->XFile().GetAsymmUnit().RestrainedDistances().ValidateRestraint( *sr );
    }
    else
      E.ProcessingError(__OlxSrcInfo, "no atoms or bonds provided" );
    if( !Options.Contains("cs") )  FXApp->SelectAll(false);
    return;
  }
  if( !Options.Contains("cs") )  FXApp->SelectAll(false);

  TSimpleRestraint* dfix = &FXApp->XFile().GetAsymmUnit().RestrainedDistances().AddNew();
  dfix->SetValue( fixLen );
  dfix->SetEsd( esd );
  if( Atoms.Count() == 1 )  {  // special case
    TXAtom* XA = Atoms[0];
    for( int i=0; i < XA->Atom().NodeCount(); i++ )  {
      TSAtom* SA = &XA->Atom().Node(i);
      if( SA->IsDeleted() )  continue;
      if( SA->GetAtomInfo() == iQPeakIndex )  continue;
      dfix->AddAtomPair( XA->Atom().CAtom(), &XA->Atom().GetMatrix(0), SA->CAtom(), &SA->GetMatrix(0));
    }
  }
  else if( Atoms.Count() == 3 )  {  // special case
    dfix->AddAtomPair( Atoms[0]->Atom().CAtom(), &Atoms[0]->Atom().GetMatrix(0), 
                       Atoms[1]->Atom().CAtom(), &Atoms[1]->Atom().GetMatrix(0));
    dfix->AddAtomPair( Atoms[1]->Atom().CAtom(), &Atoms[1]->Atom().GetMatrix(0), 
                       Atoms[2]->Atom().CAtom(), &Atoms[2]->Atom().GetMatrix(0));
  }
  else  {
    if( (Atoms.Count()%2) != 0 )  {
      E.ProcessingError(__OlxSrcInfo, "even number of atoms is expected" );
      return;
    }
    for( int i=0; i < Atoms.Count(); i += 2 )  {
      TXAtom* XA = Atoms[i];
      TXAtom* XA1 = Atoms[i+1];
      dfix->AddAtomPair( XA->Atom().CAtom(), &XA->Atom().GetMatrix(0),
        XA1->Atom().CAtom(), &XA1->Atom().GetMatrix(0));
      if( dfix->AtomCount() >= 12 )  {
        FXApp->XFile().GetAsymmUnit().RestrainedDistances().ValidateRestraint(*dfix);
        dfix = &FXApp->XFile().GetAsymmUnit().RestrainedDistances().AddNew();
        dfix->SetValue( fixLen );
        dfix->SetEsd( esd );
      }
    }
  }
  FXApp->XFile().GetAsymmUnit().RestrainedDistances().ValidateRestraint(*dfix);
}
//..............................................................................
void TMainForm::macDang(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  double fixLen = 0, esd = 0.04;  // length and esd for dang
  ParseResParam(Cmds, esd, &fixLen);
  if( fixLen == 0 )  {
    E.ProcessingError(__OlxSrcInfo, "please specify the distance to restrain to" );
    return;
  }

  TXAtomPList Atoms;
  FindXAtoms(Cmds, Atoms, false, !Options.Contains("cs"));
  if( Atoms.IsEmpty() )  {
    E.ProcessingError(__OlxSrcInfo, "no atoms or bonds provided" );
    return;
  }
  TSimpleRestraint* dang = &FXApp->XFile().GetAsymmUnit().RestrainedAngles().AddNew();
  dang->SetValue( fixLen );
  dang->SetEsd( esd );
  if( (Atoms.Count()%2) != 0 )  {
    E.ProcessingError(__OlxSrcInfo, "even number of atoms is expected" );
    return;
  }
  for( int i=0; i < Atoms.Count(); i += 2 )  {
    TXAtom* XA = Atoms[i];
    TXAtom* XA1 = Atoms[i+1];
    dang->AddAtomPair( XA->Atom().CAtom(), &XA->Atom().GetMatrix(0),
      XA1->Atom().CAtom(), &XA1->Atom().GetMatrix(0));
    if( dang->AtomCount() >= 12 )  {
      FXApp->XFile().GetAsymmUnit().RestrainedAngles().ValidateRestraint(*dang);
      dang = &FXApp->XFile().GetAsymmUnit().RestrainedAngles().AddNew();
      dang->SetValue( fixLen );
      dang->SetEsd( esd );
    }
  }
  FXApp->XFile().GetAsymmUnit().RestrainedAngles().ValidateRestraint(*dang);
}
//..............................................................................
void TMainForm::macTria(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  double dfixLenA = 0, dfixLenB = 0,
        esd = 0.02,  // esd for dfix, for dang will be doubled
        angle = 0;
  ParseResParam(Cmds, esd, &dfixLenA, &dfixLenB, &angle);
  if( angle == 0 )  {
    E.ProcessingError(__OlxSrcInfo, "please provide the angle to restrain to" );
    return;
  }
  TXAtomPList xatoms;
  TSAtomPList satoms;
  FindXAtoms(Cmds, xatoms, false, false);
  if( xatoms.IsEmpty() )  {
    TGlGroup* sel = FXApp->Selection();
    
    if( sel->Count() > 1 ) {
      for( int i=0; i < sel->Count(); i += 2 )  {
        if( !EsdlInstanceOf(*sel->Object(i), TXBond) || !EsdlInstanceOf(*sel->Object(i+1), TXBond) ) {
          E.ProcessingError(__OlxSrcInfo, "bonds only expected" );
          return;
        }
        const TXBond* xba = (TXBond*)sel->Object(i);
        const TXBond* xbb = (TXBond*)sel->Object(i+1);
        TSAtom* sa = (&xba->Bond().A() == &xbb->Bond().A() || &xba->Bond().B() == &xbb->Bond().A() ) ? &xbb->Bond().A() : 
                    ((&xba->Bond().A() == &xbb->Bond().B() || &xba->Bond().B() == &xbb->Bond().B()) ? &xbb->Bond().B() : 
                      NULL );
        if( sa == NULL )  {
          E.ProcessingError(__OlxSrcInfo, "some bonds do not share atom" );
          return;
        }
        satoms.Add( &xba->Bond().Another(*sa) );
        satoms.Add( sa );
        satoms.Add( &xbb->Bond().Another(*sa) );
      }
    }
    else
      E.ProcessingError(__OlxSrcInfo, "no atoms or bonds provided" );
  }
  else
    TListCaster::POP(xatoms, satoms);

  if( !Options.Contains("cs") )  FXApp->SelectAll(false);

  for( int i=0; i < satoms.Count(); i+=3 )  {
    if( dfixLenA == 0 )  {
      dfixLenA = FXApp->XFile().GetAsymmUnit().FindRestrainedDistance(satoms[i]->CAtom(), satoms[i+1]->CAtom());
      dfixLenB = FXApp->XFile().GetAsymmUnit().FindRestrainedDistance(satoms[i+1]->CAtom(), satoms[i+2]->CAtom());
      if( dfixLenA == -1 || dfixLenB == -1 )  {
        TBasicApp::GetLog().Error(olxstr("Tria: please fix or provided distance(s)") );
        dfixLenA = dfixLenB = 0;
        continue;
      }
    }
    TSimpleRestraint* dfix = &FXApp->XFile().GetAsymmUnit().RestrainedDistances().AddNew();
    dfix->SetValue( dfixLenA );
    dfix->SetEsd( esd );
    dfix->AddAtomPair(satoms[i]->CAtom(), &satoms[i]->GetMatrix(0),
                      satoms[i+1]->CAtom(), &satoms[i+1]->GetMatrix(0) );
    if( dfixLenB != dfixLenA )  {
      FXApp->XFile().GetAsymmUnit().RestrainedDistances().ValidateRestraint(*dfix);
      dfix = &FXApp->XFile().GetAsymmUnit().RestrainedDistances().AddNew();
    }
    dfix->AddAtomPair(satoms[i+1]->CAtom(), &satoms[i+1]->GetMatrix(0),
                      satoms[i+2]->CAtom(), &satoms[i+2]->GetMatrix(0) );
    FXApp->XFile().GetAsymmUnit().RestrainedDistances().ValidateRestraint(*dfix);
    

    TSimpleRestraint* dang = &FXApp->XFile().GetAsymmUnit().RestrainedAngles().AddNew();
    dang->SetValue( sqrt(QRT(dfixLenA) + QRT(dfixLenB) - 2*dfixLenA*dfixLenB*cos(angle*M_PI/180)) );
    dang->SetEsd(esd*2);
    dang->AddAtom(satoms[i]->CAtom(), &satoms[i]->GetMatrix(0) );
    //dang->AddAtom(Atoms[i+1]->Atom().CAtom(), &Atoms[i+1]->Atom().GetMatrix(0) );
    dang->AddAtom(satoms[i+2]->CAtom(), &satoms[i+2]->GetMatrix(0) );
    FXApp->XFile().GetAsymmUnit().RestrainedAngles().ValidateRestraint(*dang);
  }
}
//..............................................................................
void TMainForm::macSadi(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  double esd = 0.02;  // esd for sadi
  ParseResParam(Cmds, esd);
  TXAtomPList Atoms;
  FindXAtoms(Cmds, Atoms, false, false);
  if( Atoms.IsEmpty() )  {
    TGlGroup* sel = FXApp->Selection();
    if( sel->Count() > 1 ) {
      TSimpleRestraint *sr = &FXApp->XFile().GetAsymmUnit().SimilarDistances().AddNew();
      sr->SetEsd(esd);
      for( int i=0; i < sel->Count(); i++ )  {
        if( !EsdlInstanceOf( *sel->Object(i), TXBond) )  continue;
        const TXBond* xb = (TXBond*)sel->Object(i);
        sr->AddAtomPair(xb->Bond().A().CAtom(), &xb->Bond().A().GetMatrix(0),
          xb->Bond().B().CAtom(), &xb->Bond().B().GetMatrix(0));
      }
      FXApp->XFile().GetAsymmUnit().SimilarDistances().ValidateRestraint(*sr);
    }
    else
      E.ProcessingError(__OlxSrcInfo, "no atoms or bonds provided" );
    if( !Options.Contains("cs") )  FXApp->SelectAll(false);
    return;
  }
  if( !Options.Contains("cs") )  FXApp->SelectAll(false);

  TSimpleRestraint *sr = &FXApp->XFile().GetAsymmUnit().SimilarDistances().AddNew();
  sr->SetEsd(esd);

  if( Atoms.Count() == 1 )  {  // special case
    TSimpleRestraint *sr1 = &FXApp->XFile().GetAsymmUnit().SimilarDistances().AddNew();
    sr1->SetEsd(esd);
    sr->SetEsd(esd*2);
    TXAtom* XA = Atoms[0];
    double td = 0;
    for( int i=0; i < XA->Atom().NodeCount(); i++ )  {
      TSAtom& SA = XA->Atom().Node(i);
      if( SA.IsDeleted() )  continue;
      if( SA.GetAtomInfo() == iQPeakIndex )  continue;
      sr1->AddAtomPair(XA->Atom().CAtom(), &XA->Atom().GetMatrix(0), SA.CAtom(), &SA.GetMatrix(0));
      if( td == 0 )  // need this one to remove opposite atoms from restraint
        td = XA->Atom().GetCenter().DistanceTo( SA.GetCenter() ) * 2;
      for( int j=i+1; j < XA->Atom().NodeCount(); j++ )  {
        TSAtom& SA1 = XA->Atom().Node(j);
        if( SA1.IsDeleted() )  continue;
        if( SA1.GetAtomInfo() == iQPeakIndex )  continue;
        double d = SA.GetCenter().DistanceTo( SA1.GetCenter() ) ;
        if( d/td > 0.85 )  continue;
        sr->AddAtomPair(SA.CAtom(), &SA.GetMatrix(0), SA1.CAtom(), &SA1.GetMatrix(0));
        if( sr->AtomCount() >= 12 )  {
          FXApp->XFile().GetAsymmUnit().SimilarDistances().ValidateRestraint(*sr);
          sr = &FXApp->XFile().GetAsymmUnit().SimilarDistances().AddNew();
          sr->SetEsd( esd*2 );
        }
      }
    }
  }
  else if( Atoms.Count() == 3 )  {  // special case
    sr->AddAtomPair( Atoms[0]->Atom().CAtom(), &Atoms[0]->Atom().GetMatrix(0), 
                       Atoms[1]->Atom().CAtom(), &Atoms[1]->Atom().GetMatrix(0));
    sr->AddAtomPair( Atoms[1]->Atom().CAtom(), &Atoms[1]->Atom().GetMatrix(0), 
                       Atoms[2]->Atom().CAtom(), &Atoms[2]->Atom().GetMatrix(0));
  }
  else  {
    if( (Atoms.Count()%2) != 0 )  {
      E.ProcessingError(__OlxSrcInfo, "even number of atoms is expected" );
      return;
    }
    for( int i=0; i < Atoms.Count(); i += 2 )  {
      sr->AddAtomPair(Atoms[i]->Atom().CAtom(), &Atoms[i]->Atom().GetMatrix(0),
        Atoms[i+1]->Atom().CAtom(), &Atoms[i+1]->Atom().GetMatrix(0));
    }
  }
  FXApp->XFile().GetAsymmUnit().SimilarDistances().ValidateRestraint(*sr);
}
//..............................................................................
void TMainForm::macFlat(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  double esd = 0.1;  // esd for flat
  ParseResParam(Cmds, esd);
  TXAtomPList Atoms;
  if( !FindXAtoms(Cmds, Atoms, false, !Options.Contains("cs")) )  {
    E.ProcessingError(__OlxSrcInfo, "no atoms provided" );
    return;
  }

  TSimpleRestraint& sr = FXApp->XFile().GetAsymmUnit().RestrainedPlanarity().AddNew();
  sr.SetEsd(esd);

  for( int i=0; i < Atoms.Count(); i++ )
    sr.AddAtom(Atoms[i]->Atom().CAtom(), &Atoms[i]->Atom().GetMatrix(0));
  FXApp->XFile().GetAsymmUnit().RestrainedPlanarity().ValidateRestraint(sr);
}
//..............................................................................
void TMainForm::macEADP(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TXAtomPList Atoms;
  if( !FindXAtoms(Cmds, Atoms, false, !Options.Contains("cs")) )  {
    E.ProcessingError(__OlxSrcInfo, "no atoms provided" );
    return;
  }
  // validate that atoms of the same type
  bool allIso = (Atoms[0]->Atom().GetEllipsoid() == NULL);
  for( int i=1; i < Atoms.Count(); i++ )
    if( (Atoms[i]->Atom().GetEllipsoid() == NULL) != allIso )  {
      E.ProcessingError(__OlxSrcInfo, "mixed atoms types *aniso and iso)" );
      return;
    }
  TSimpleRestraint& sr = FXApp->XFile().GetAsymmUnit().EquivalentU().AddNew();
  for( int i=0; i < Atoms.Count(); i++ )
    sr.AddAtom(Atoms[i]->Atom().CAtom(), NULL);
  FXApp->XFile().GetAsymmUnit().EquivalentU().ValidateRestraint(sr);
}
//..............................................................................
void TMainForm::macSIMU(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  short setCnt = 0;
  double esd1 = 0.04, esd2=0.08, val = 1.7;  // esd
  for( int i=0; i < Cmds.Count(); i++ )  {
    if( Cmds[i].IsNumber() )  {
      if( setCnt == 0 )      {  esd1 = Cmds[i].ToDouble();  setCnt++;  }
      else if( setCnt == 1 ) {  esd2 = Cmds[i].ToDouble();  setCnt++;  }
      else if( setCnt == 2 ) {  val = Cmds[i].ToDouble();  setCnt++;  }
      else  {
        E.ProcessingError(__OlxSrcInfo, "too many numirical parameters" );
        return;
      }
      Cmds.Delete(i);
    }
  }
  if( setCnt == 1 )  esd2 = esd1 * 2;
  TXAtomPList Atoms;
  FindXAtoms(Cmds, Atoms, false, !Options.Contains("cs"));
  // validate that atoms of the same type
  TSimpleRestraint& sr = FXApp->XFile().GetAsymmUnit().SimilarU().AddNew();
  sr.SetAllNonHAtoms( Atoms.IsEmpty() );
  sr.SetEsd(esd1);
  sr.SetEsd1(esd2);
  sr.SetValue(val);
  for( int i=0; i < Atoms.Count(); i++ )
    sr.AddAtom(Atoms[i]->Atom().CAtom(), NULL);
  FXApp->XFile().GetAsymmUnit().SimilarU().ValidateRestraint(sr);
}
//..............................................................................
void TMainForm::macDELU(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  short setCnt = 0;
  double esd1 = 0.01, esd2=0.01;  // esd
  for( int i=0; i < Cmds.Count(); i++ )  {
    if( Cmds[i].IsNumber() )  {
      if( setCnt == 0 )      {  esd1 = Cmds[i].ToDouble();  setCnt++;  }
      else if( setCnt == 1 ) {  esd2 = Cmds[i].ToDouble();  setCnt++;  }
      else  {
        E.ProcessingError(__OlxSrcInfo, "too many numirical parameters" );
        return;
      }
      Cmds.Delete(i);
    }
  }
  if( setCnt == 1 )  esd2 = esd1;
  TXAtomPList Atoms;
  FindXAtoms(Cmds, Atoms, false, !Options.Contains("cs"));
  // validate that atoms of the same type
  TSimpleRestraint& sr = FXApp->XFile().GetAsymmUnit().RigidBonds().AddNew();
  sr.SetEsd(esd1);
  sr.SetEsd1(esd2);
  sr.SetAllNonHAtoms( Atoms.IsEmpty() );
  for( int i=0; i < Atoms.Count(); i++ )
    sr.AddAtom(Atoms[i]->Atom().CAtom(), NULL);
  FXApp->XFile().GetAsymmUnit().RigidBonds().ValidateRestraint(sr);
}
//..............................................................................
void TMainForm::macISOR(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  short setCnt = 0;
  double esd1 = 0.1, esd2=0.2;  // esd
  for( int i=0; i < Cmds.Count(); i++ )  {
    if( Cmds[i].IsNumber() )  {
      if( setCnt == 0 )      {  esd1 = Cmds[i].ToDouble();  setCnt++;  }
      else if( setCnt == 1 ) {  esd2 = Cmds[i].ToDouble();  setCnt++;  }
      else  {
        E.ProcessingError(__OlxSrcInfo, "too many numirical parameters" );
        return;
      }
      Cmds.Delete(i);
    }
  }
  if( setCnt == 1 )  esd2 = esd1 * 2;
  TXAtomPList Atoms;
  FindXAtoms(Cmds, Atoms, false, !Options.Contains("cs"));
  // validate that atoms of the same type
  TSimpleRestraint& sr = FXApp->XFile().GetAsymmUnit().RestranedUaAsUi().AddNew();
  sr.SetEsd(esd1);
  sr.SetEsd1(esd2);
  sr.SetAllNonHAtoms( Atoms.IsEmpty() );
  for( int i=0; i < Atoms.Count(); i++ )
    sr.AddAtom(Atoms[i]->Atom().CAtom(), NULL);
  FXApp->XFile().GetAsymmUnit().RestranedUaAsUi().ValidateRestraint(sr);
}
//..............................................................................
void TMainForm::macChiv(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  short setCnt = 0;
  double esd = 0.1, val=0;  // esd
  for( int i=0; i < Cmds.Count(); i++ )  {
    if( Cmds[i].IsNumber() )  {
      if( setCnt == 0 )      {  val = Cmds[i].ToDouble(); setCnt++;  }
      else if( setCnt == 1 ) {  esd = Cmds[i].ToDouble(); setCnt++;  }
      else  {
        E.ProcessingError(__OlxSrcInfo, "too many numirical parameters" );
        return;
      }
      Cmds.Delete(i);
    }
  }
  TXAtomPList Atoms;
  if( !FindXAtoms(Cmds, Atoms, false, !Options.Contains("cs")) )  {
    E.ProcessingError(__OlxSrcInfo, "no atoms provided" );
    return;
  }

  TSimpleRestraint& sr = FXApp->XFile().GetAsymmUnit().RestrainedVolumes().AddNew();
  sr.SetValue(val);
  sr.SetEsd(esd);
  for( int i=0; i < Atoms.Count(); i++ )
    sr.AddAtom(Atoms[i]->Atom().CAtom(), &Atoms[i]->Atom().GetMatrix(0));
  FXApp->XFile().GetAsymmUnit().RestrainedVolumes().ValidateRestraint(sr);
}
//..............................................................................
void TMainForm::macShowQ(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Cmds.Count() == 2 )  {
    bool v = Cmds[1].ToBool();
    if( Cmds[0] == "a" )  {
      if( v && !FXApp->QPeaksVisible() )  {
        TStateChange sc(prsQVis, true);
        FXApp->QPeaksVisible( true );
        OnStateChange->Execute((AEventsDispatcher*)this, &sc);
      }
      else if( !v && FXApp->QPeaksVisible() )  {
        TStateChange sc(prsQVis, false);
        FXApp->QPeaksVisible( false );
        OnStateChange->Execute((AEventsDispatcher*)this, &sc);
      }
    }
    else if( Cmds[0] == "b" )  {
      if( v && !FXApp->QPeakBondsVisible() )  {
        TStateChange sc(prsQBVis, true);
        FXApp->QPeakBondsVisible( true );
        OnStateChange->Execute((AEventsDispatcher*)this, &sc);
      }
      else if( !v && FXApp->QPeakBondsVisible() )  {
        TStateChange sc(prsQBVis, false);
        FXApp->QPeakBondsVisible( false );
        OnStateChange->Execute((AEventsDispatcher*)this, &sc);
      }
    }
  }
  else  {
    if( (!FXApp->QPeaksVisible() && !FXApp->QPeakBondsVisible()) )  {
      TStateChange sc(prsQVis, true);
      FXApp->QPeaksVisible(true);
      OnStateChange->Execute((AEventsDispatcher*)this, &sc);
      return;
    }
    if( FXApp->QPeaksVisible() && !FXApp->QPeakBondsVisible())  {
      TStateChange sc(prsQBVis, true);
      FXApp->QPeakBondsVisible(true);
      OnStateChange->Execute((AEventsDispatcher*)this, &sc);
      return;
    }
    if( FXApp->QPeaksVisible() && FXApp->QPeakBondsVisible() )  {
      TStateChange sc(prsQBVis|prsQVis, false);
      FXApp->QPeaksVisible(false);
      FXApp->QPeakBondsVisible(false);
      OnStateChange->Execute((AEventsDispatcher*)this, &sc);
      return;
    }
  }
}
//..............................................................................
void TMainForm::macMode(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  // this variable is set when the mode is changed from within this function
  static bool ChangingMode = false;
  if( ChangingMode )  return;
  AMode *md = Modes->SetMode( Cmds[0] );
  if( md != NULL )  {
    Cmds.Delete(0);
    md->Init(Cmds, Options);
  }
  ChangingMode = false;
}
//..............................................................................
void TMainForm::macReset(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)
{
  if( !(FXApp->CheckFileType<TIns>() ||
        FXApp->CheckFileType<TP4PFile>() ||
        FXApp->CheckFileType<TCRSFile>()  )  )  return;

  olxstr newSg(Options.FindValue('s')), 
         content( olxstr::DeleteChars(Options.FindValue('c'), ' ')),
         fileName(Options.FindValue('f') );

  TIns *Ins = (TIns*)FXApp->XFile().FindFormat("ins");
  if( FXApp->CheckFileType<TP4PFile>() )  {
    if( !newSg.Length() )  {
      E.ProcessingError(__OlxSrcInfo, "please specify a space group with -s=SG switch" );
      return;
    }
    Ins->Adopt( &FXApp->XFile() );
  }
  else if( FXApp->CheckFileType<TCRSFile>() )  {
    TSpaceGroup* sg = ((TCRSFile*)FXApp->XFile().GetLastLoader())->GetSG();
    if( newSg.IsEmpty() )  {
      if( sg == NULL )  {
        E.ProcessingError(__OlxSrcInfo, "please specify a space group with -s=SG switch" );
        return;
      }
      else  {
        TBasicApp::GetLog() << ( olxstr("The CRS file format space group is: ") << sg->GetName() << '\n');
      }
    }
    Ins->Adopt( &FXApp->XFile() );
  }
  if( content.Length() )  Ins->SetSfacUnit( content );
  if( Ins->GetSfac().Length() == 0)  {
    content = "getuserinput(1, \'Please, enter structure composition\', \'C1\')";
    ProcessMacroFunc(content);
    Ins->SetSfacUnit( content );
    if( Ins->GetSfac().Length() == 0 )  {
      E.ProcessingError(__OlxSrcInfo, "empty SFAC instruction, please use -c=Content to specify" );
      return;
    }
  }

  if( !newSg.IsEmpty() )  {
    TSpaceGroup* sg = TSymmLib::GetInstance()->FindGroup( newSg );
    if( !sg )  {
      E.ProcessingError(__OlxSrcInfo, "could not find space group: ") << newSg;
      return;
    }
    Ins->GetAsymmUnit().ChangeSpaceGroup( *sg );
    newSg = EmptyString;
    newSg <<  " reset to " << sg->GetName() << " #" << sg->GetNumber();
  }
  if( fileName.IsEmpty() )
    fileName = FXApp->XFile().GetFileName();
  olxstr FN( TEFile::ChangeFileExt(fileName, "ins") );
  olxstr lstFN( TEFile::ChangeFileExt(fileName, "lst") );

  Ins->SaveToRefine(FN, Cmds.Text(' '), newSg);
  if( TEFile::FileExists(lstFN) )  {
    olxstr lstTmpFN( lstFN );
    lstTmpFN << ".tmp";
    wxRenameFile( uiStr(lstFN), uiStr(lstTmpFN) );
  }
  ProcessXPMacro( olxstr("@reap \'") << FN << '\'', E);
  ProcessXPMacro("htmlreload", E);
}
//..............................................................................
void TMainForm::macLstIns(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  if( !FXApp->CheckFileType<TIns>() )  return;

  bool remarks = false;
  for( int i=0; i < Options.Count(); i++ )  {
    if( !Options.GetName(i).Comparei("r") )  {  remarks = true; break;  }
  }

  TIns *Ins = (TIns*)FXApp->XFile().GetLastLoader();
  TBasicApp::GetLog() << ("List of current instructions:\n");
  olxstr Tmp;
  for( int i=0; i < Ins->InsCount(); i++ )  {
    if( !remarks && !Ins->InsName(i).Comparei("REM") )  continue;
    Tmp = i;  Tmp.Format(3, true, ' ');
    Tmp << Ins->InsName(i) << ' ' << Ins->InsParams(i).Text(' ');
    TBasicApp::GetLog() << (Tmp << '\n');
  }
}
//..............................................................................
void TMainForm::macLstMac(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TBasicFunctionPList macros;
  GetLibrary().ListAllMacros( macros );
  olxstr line;
  for( int i=0; i < macros.Count(); i++ )  {
    ABasicFunction* func = macros[i];
    line = func->GetQualifiedName();
    TBasicApp::GetLog() << (line << " - " << func->GetSignature() << '\n');
  }
}
//..............................................................................
void TMainForm::macLstFun(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TBasicFunctionPList functions;
  GetLibrary().ListAllFunctions( functions );
  olxstr line;
  for( int i=0; i < functions.Count(); i++ )  {
    ABasicFunction* func = functions[i];
    line = func->GetQualifiedName();
    TBasicApp::GetLog() << (line << " - " << func->GetSignature() << '\n');
  }
}
//..............................................................................
void TMainForm::macLstVar(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( TOlxVars::VarCount() == 0 )  return;
  TTTable<TStrList> tab(TOlxVars::VarCount(), 3);
  tab.ColName(0) = "Name";
  tab.ColName(1) = "Value";
  tab.ColName(2) = "RefCnt";
  for( int i=0; i < TOlxVars::VarCount(); i++ )  {
    tab[i][0] = TOlxVars::GetVarName(i);
    tab[i][1] = TOlxVars::GetVarStr(i);
    if( TOlxVars::GetVarWrapper(i) != NULL )
      tab[i][2] = TOlxVars::GetVarWrapper(i)->ob_refcnt;
    else
      tab[i][2] = NAString;
  }
  TStrList Output;
  tab.CreateTXTList(Output, "Variables list", true, true, ' ');
  FGlConsole->PrintText( Output, NULL, false );
}
//..............................................................................
void TMainForm::macDelIns(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TIns *Ins = (TIns*)FXApp->XFile().GetLastLoader();
  if( Cmds[0].IsNumber() )  {
    int insIndex = Cmds[0].ToInt();
    Ins->DelIns(insIndex);
    return;
  }

  for( int i=0; i < Ins->InsCount(); i++ )  {
    if(  !Ins->InsName(i).Comparei(Cmds[0]) )  {
      Ins->DelIns(i);  i--;  continue;
    }
  }
}
//..............................................................................
void TMainForm::macText(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  olxstr FN = DataDir + "output.txt";
  TUtf8File::WriteLines( FN, FGlConsole->Buffer());
  ProcessXPMacro( olxstr("exec getvar('defeditor') -o \"") << FN << '\"' , E);
}
//..............................................................................
void TMainForm::macShowStr(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Cmds.IsEmpty() )
    FXApp->StructureVisible( !FXApp->StructureVisible() );
  else
    FXApp->StructureVisible( Cmds[0].ToBool() );

  TStateChange sc(prsStrVis, FXApp->StructureVisible());
  OnStateChange->Execute((AEventsDispatcher*)this, &sc);
}
//..............................................................................
void TMainForm::macBind(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  throw TNotImplementedException(__OlxSourceInfo);
}
//..............................................................................
void TMainForm::macFree(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  olxstr vars = Cmds[0];
  Cmds.Delete(0);
  TXAtomPList Atoms;
  FXApp->FindXAtoms(Cmds.Text(' '), Atoms, false);
  if( Atoms.IsEmpty() )  {
    E.ProcessingError(__OlxSrcInfo, "no atoms provided");
    return;
  }
  if( vars.Comparei( "XYZ" ) == 0 )  {
    for(int i=0; i < Atoms.Count(); i++ )  {
      TXAtom* XA = Atoms[i];
      for( int j=0; j < 3; j++ )
        XA->Atom().CAtom().FixedValues()[TCAtom::CrdFixedValuesOffset + j] = 0;
    }
  }
  else if( vars.Comparei( "UISO" ) == 0 )  {
    for(int i=0; i < Atoms.Count(); i++ )  {
      TXAtom* XA = Atoms[i];
      if( XA->Atom().GetEllipsoid() == NULL )  {  // isotropic atom
          XA->Atom().CAtom().SetUisoVar( 0 );
      }
      else  {
        for( int j=0; j < 6; j++ )
          XA->Atom().CAtom().FixedValues()[TCAtom::UisoFixedValuesOffset + j] = 0;
      }
    }
  }
  else if( vars.Comparei( "OCCU" ) == 0 )  {
    for(int i=0; i < Atoms.Count(); i++ )  {
      Atoms[i]->Atom().CAtom().SetOccpVar( 0 );
    }
  }
}
//..............................................................................
void TMainForm::macFix(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  olxstr vars = Cmds[0];
  Cmds.Delete(0);
  TXAtomPList Atoms;
  FXApp->FindXAtoms(Cmds.Text(' '), Atoms, false);
  if( Atoms.IsEmpty() )  {
    E.ProcessingError(__OlxSrcInfo, "no atoms provided");
    return;
  }
  if( vars.Comparei( "XYZ" ) == 0 )  {
    for(int i=0; i < Atoms.Count(); i++ )  {
      TXAtom* XA = Atoms[i];
      for( int j=0; j < 3; j++ )
        XA->Atom().CAtom().FixedValues()[TCAtom::CrdFixedValuesOffset + j] = 10;
    }
  }
  else if( vars.Comparei( "UISO" ) == 0 )  {
    double uiso = 0;
    if( Cmds.Count() > 1 && Cmds[0].IsNumber() )  {
      uiso = Cmds[0].ToDouble();
    }
    for(int i=0; i < Atoms.Count(); i++ )  {
      TXAtom* XA = Atoms[i];
      if( XA->Atom().GetEllipsoid() == NULL )  {  // isotropic atom
        if( uiso != 0 )  {
          if( uiso < 10 )  {
            XA->Atom().CAtom().SetUiso( uiso );
            XA->Atom().CAtom().SetUisoVar( 10 );
          }
          else  {
            int iv = (int) uiso;
            XA->Atom().CAtom().SetUiso( uiso-iv );
            XA->Atom().CAtom().SetUisoVar( iv*10 );
          }
        }
        else  if( XA->Atom().CAtom().GetUisoVar() == 0 )  {  // have to skip riding atoms
          XA->Atom().CAtom().SetUisoVar( 10 );
        }
      }
      else  {
        for( int j=0; j < 6; j++ )
          XA->Atom().CAtom().FixedValues()[TCAtom::UisoFixedValuesOffset + j] = 10;
      }
    }
  }
  else if( vars.Comparei( "OCCU" ) == 0 )  {
    for(int i=0; i < Atoms.Count(); i++ )  {
      if( Atoms[i]->Atom().CAtom().GetPart() == 0 )  {
        Atoms[i]->Atom().CAtom().SetOccp( 1./Atoms[i]->Atom().CAtom().GetDegeneracy() );
        Atoms[i]->Atom().CAtom().SetOccpVar( 10 );
      }
    }
  }
}
//..............................................................................
void TMainForm::macGrad(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  bool invert = Options.Contains('i');
  if( invert )  {
    TStateChange sc(prsGradBG, !FXApp->GetRender().Background()->Visible() );
    OnStateChange->Execute( (AEventsDispatcher*)this, &sc );
    FXApp->GetRender().Background()->Visible( !FXApp->GetRender().Background()->Visible() );
  }
  else if( Cmds.Count() == 1 )  {
    TStateChange sc(prsGradBG, Cmds[0].ToBool() );
    OnStateChange->Execute( (AEventsDispatcher*)this, &sc );
    FXApp->GetRender().Background()->Visible( Cmds[0].ToBool() );
  }
  else if( Cmds.IsEmpty() )  {
    TdlgGradient *G = new TdlgGradient(this);
    G->ShowModal();
    G->Destroy();
  }
  else if( Cmds.Count() == 4 )  {
    TGlOption v;
    v = Cmds[0].ToInt();  FXApp->GetRender().Background()->RB(v);
    v = Cmds[1].ToInt();  FXApp->GetRender().Background()->LB(v);
    v = Cmds[2].ToInt();  FXApp->GetRender().Background()->RT(v);
    v = Cmds[3].ToInt();  FXApp->GetRender().Background()->LT(v);
  }
  GradientPicture = Options.FindValue("p");
  if( GradientPicture.IsEmpty() )  {
    TGlTexture* glt = FXApp->GetRender().Background()->GetTexture();
    if( glt != NULL  )
      glt->SetEnabled(false);
  }
  else if( TEFile::FileExists(GradientPicture) )  {
    wxFSFile* inf = TFileHandlerManager::GetFSFileHandler( GradientPicture );
    if( inf == NULL )  {
      E.ProcessingError(__OlxSrcInfo, "Image file does not exist: ") << GradientPicture;
      return;
    }
    wxImage img( *inf->GetStream() );
    delete inf;
    if( !img.Ok() )  {
      E.ProcessingError(__OlxSrcInfo, "Invalid image file: ") << GradientPicture;
      return;
    }
    int owidth = img.GetWidth(), oheight = img.GetHeight();
    int l = CalcL( img.GetWidth() );
    int swidth = (int)pow((double)2, (double)l);
    l = CalcL( img.GetHeight() );
    int sheight = (int)pow((double)2, (double)l);

    if( swidth != owidth || sheight != oheight )
      img.Rescale( swidth, sheight );

    int cl = 3, bmpType = GL_RGB;
    if( img.HasAlpha() )  {
      cl ++;
      bmpType = GL_RGBA;
    }

    unsigned char* RGBData = new unsigned char[ swidth * sheight * cl];
    for( int i=0; i < sheight; i++ )  {
      for( int j=0; j < swidth; j++ )  {
        int indexa = (i*swidth + (swidth-j-1)) * cl;
        RGBData[indexa] = img.GetRed(j, i);
        RGBData[indexa+1] = img.GetGreen(j, i);
        RGBData[indexa+2] = img.GetBlue(j, i);
        if( cl == 4 )
          RGBData[indexa+3] = img.GetAlpha(j, i);
      }
    }
    TGlTexture* glt = FXApp->GetRender().Background()->GetTexture();
    if( glt != NULL  )
      FXApp->GetRender().GetTextureManager().Replace2DTexture(*glt, 0, swidth, sheight, 0, bmpType, RGBData);
    else  {
      int glti = FXApp->GetRender().GetTextureManager().Add2DTexture("grad", 0, swidth, sheight, 0, bmpType, RGBData);
      FXApp->GetRender().Background()->SetTexture( FXApp->GetRender().GetTextureManager().FindTexture(glti) );
      glt = FXApp->GetRender().Background()->GetTexture();
      glt->SetEnvMode( tpeDecal );
      glt->SetSCrdWrapping( tpCrdClamp );
      glt->SetTCrdWrapping( tpCrdClamp );

      glt->SetMagFilter( tpFilterNearest );
      glt->SetMinFilter( tpFilterLinear );
      glt->SetEnabled( true );
    }
    delete [] RGBData;
  }
}
//..............................................................................
void TMainForm::macSplit(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  bool found;
  TLstSplitAtom *SpA;
  TCAtomPList Atoms;
  olxstr lbl;
  olxstr tmp = Cmds.IsEmpty() ? olxstr("sel") : Cmds.Text(' ');
  FXApp->FindCAtoms(tmp, Atoms, true);
  if( Atoms.Count() == 0 )  return;
  TCAtomPList ProcessedAtoms;
  for( int i=0; i < Atoms.Count(); i++ )  {
    TCAtom* CA = Atoms[i];
    if( CA->GetEllipsoid() == NULL )  continue;
    found = false;
    for( int j=0; j < Lst.SplitAtomCount(); j++ )  {
      SpA = Lst.SplitAtom(j);
      if( !SpA->AtomName.Comparei(CA->Label()) )  {
        lbl = SpA->AtomName;
        if( lbl.Length() > 3 )  lbl.SetLength(3);
        TCAtom& CA1 = FXApp->XFile().GetAsymmUnit().NewAtom();
        CA1.Assign(*CA);
        CA1.SetLoaderId(liNewAtom);
        CA1.SetPart(1);
        ProcessedAtoms.Add( &CA1 );
        CA1.CCenter() = SpA->PositionA;
        CA1.Label() = lbl+'a';
        TCAtom& CA2 = FXApp->XFile().GetAsymmUnit().NewAtom();
        CA2.Assign(*CA);
        CA2.SetLoaderId(liNewAtom);
        CA2.SetPart(2);
        CA2.CCenter() = SpA->PositionB;
        CA2.Label() = lbl+'b';
        CA->SetDeleted(true);
        ProcessedAtoms.Add( &CA2 );
        found = true;
      }
    }
    if( !found )  {
      TVPointD direction;
      float Length = 0;
      lbl = CA->Label();
      if( CA->GetEllipsoid()->GetSX() > CA->GetEllipsoid()->GetSY() )  {
        if( CA->GetEllipsoid()->GetSX() > CA->GetEllipsoid()->GetSZ() )  {
          Length = CA->GetEllipsoid()->GetSX();
          direction = CA->GetEllipsoid()->GetMatrix()[0];
        }
        else  {
          Length = CA->GetEllipsoid()->GetSZ();
          direction = CA->GetEllipsoid()->GetMatrix()[2];
        }
      }
      else  {
        if( CA->GetEllipsoid()->GetSY() > CA->GetEllipsoid()->GetSZ() )  {
          Length = CA->GetEllipsoid()->GetSY();
          direction = CA->GetEllipsoid()->GetMatrix()[1];
        }
        else  {
          Length = CA->GetEllipsoid()->GetSZ();
          direction = CA->GetEllipsoid()->GetMatrix()[2];
        }
      }
      direction *= Length;
      direction /= 2;
      FXApp->XFile().GetAsymmUnit().CartesianToCell( direction );

      TCAtom* CA1 = &FXApp->XFile().GetAsymmUnit().NewAtom();
      CA1->Assign(*CA);
      CA1->SetLoaderId(liNewAtom);
      CA1->SetPart(1);
      CA1->CCenter() += direction;
      CA1->Label() = FXApp->XFile().GetAsymmUnit().CheckLabel(CA1, lbl+'a');
      ProcessedAtoms.Add( CA1 );
      CA1 = &FXApp->XFile().GetAsymmUnit().NewAtom();
      CA1->Assign(*CA);
      CA1->SetLoaderId(liNewAtom);
      CA1->SetPart(2);
      CA1->CCenter() -= direction;
      CA1->Label() = FXApp->XFile().GetAsymmUnit().CheckLabel(CA1, lbl+'b');
      ProcessedAtoms.Add( CA1 );
      CA->SetDeleted(true);
    }
  }
  for( int i=0; i < ProcessedAtoms.Count(); i++ )
    ProcessedAtoms[i]->AssignEllps(NULL);
  FXApp->XFile().EndUpdate();
  // copy new atoms to the XF AU
  FXApp->XFile().UpdateAsymmUnit();
}
//..............................................................................
void TMainForm::macShowP(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TIntList parts;
  if( Cmds.Count() )  {
    for(int i=0; i < Cmds.Count(); i++ )
      parts.Add( Cmds[i].ToInt() );
    FXApp->ShowPart(parts, true);
  }
  else
    FXApp->ShowPart(parts, true);
  FXApp->CenterView();
}
//..............................................................................
void TMainForm::macEditAtom(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TSAtomPList SAtoms;
  TCAtomPList CAtoms;
  TXAtomPList Atoms;
  TIns *Ins = (TIns*)FXApp->XFile().GetLastLoader();
  
  if( !FindXAtoms(Cmds, Atoms, true, !Options.Contains("cs")) )  {
    E.ProcessingError(__OlxSrcInfo, "wrong atom names" );
    return;
  }
  // synchronise atom names etc
  FXApp->XFile().UpdateAsymmUnit();
  Ins->UpdateParams();
  TSimpleRestraintPList restraints;

  for(int i=0; i < Atoms.Count(); i++ )  {
    TXAtom* XA = Atoms[i];
    SAtoms.Add( &XA->Atom() );
    for( int j=0; j < XA->Atom().NodeCount(); j++ )  {
      TSAtom* SA = &XA->Atom().Node(j);
      if( SA->GetAtomInfo() == iHydrogenIndex )
        SAtoms.Add( SA );
    }
  }
  // make sure that the list is unique
  for( int i=0; i < SAtoms.Count(); i++ )
    SAtoms[i]->SetTag(i);
  for( int i=0; i < SAtoms.Count(); i++ )
    if( SAtoms[i]->GetTag() != i )  SAtoms[i] = NULL;
  SAtoms.Pack();
  TListCaster::POP(SAtoms, CAtoms);
  FXApp->XFile().GetAsymmUnit().Sort( &CAtoms );
  TStrList SL, *InsParamsCopy, NewIns;
  TStrPObjList<olxstr, TStrList* > RemovedIns;
  const TInsList* InsParams;
  olxstr Tmp;
  bool found;
  // go through instructions
  for(int i=0; i < Ins->InsCount(); i++ )  {
    // do not process remarks
    if( Ins->InsName(i).Comparei("rem") != 0 )  {
      InsParams = &Ins->InsParams(i);
      found = false;
      for( int j=0; j < InsParams->Count(); j++ )  {
        if( !InsParams->Object(j) )  continue;
        TCAtom* CA1 = InsParams->Object(j);
        for( int k=0; k < CAtoms.Count(); k++ )  {
          TCAtom* CA = CAtoms[k];
          if( !CA->Label().Comparei(CA1->Label()) )  {
            found = true;  break;
          }
        }
        if( found )  break;
      }
      if( found )  {
        Tmp = Ins->InsName(i);  Tmp << ' ' << InsParams->Text(' ');
        SL.Add(Tmp);
        InsParamsCopy = new TStrList();
        InsParamsCopy->Assign(*InsParams);
        RemovedIns.Add(Ins->InsName(i), InsParamsCopy);
        Ins->DelIns(i);
        i--;
      }
    }
  }
    // add some remarks
  if( SL.Count() != 0 )  {
    SL.Insert(0, EmptyString);
    SL.Insert(0, "REM please do not modify atom names inside the instructions - they will be updated ");
    SL.Insert(0, "REM by Olex2 automatically, though you can change any parameters");
  }
  SL.Add(EmptyString);

  Ins->SaveAtomsToStrings(CAtoms, SL, &restraints);
  for( int i=0; i < restraints.Count(); i++ )
    restraints[i]->GetParent()->Release(*restraints[i]);

  TdlgEdit *dlg = new TdlgEdit(this, true);
  Tmp = EmptyString;
  for( int i=0; i < SL.Count(); i++ )  {
    Tmp << SL[i];
    if( (i+1) < SL.Count() )
      Tmp << '\n';
  }
  dlg->SetText( Tmp );
  try  {
    if( dlg->ShowModal() == wxID_OK )  {
      SL.Clear();
      SL.Strtok(dlg->GetText(), '\n');
      Ins->UpdateAtomsFromStrings(CAtoms, SL, NewIns);
      // add new instructions
      for( int i=0; i < NewIns.Count(); i++ )  {
        SL.Clear();
        SL.Strtok(NewIns[i], ' ');
        if( SL.IsEmpty() )  continue;
        Tmp = SL[0];
        SL.Delete(0);
        Ins->AddIns(Tmp, SL);
      }
      // synchronisation for new instructions
      FXApp->XFile().UpdateAsymmUnit();
      // update instruction parameters
      Ins->UpdateParams();
      // emulate loading a new file
      FXApp->XFile().EndUpdate();
    }
    else  {
      for( int i=0; i < RemovedIns.Count(); i++ )
        Ins->AddIns(RemovedIns[i], *RemovedIns.Object(i));
      for( int i=0; i < restraints.Count(); i++ )
        restraints[i]->GetParent()->Restore(*restraints[i]);
    }
  }
  catch(const TExceptionBase& exc )  {
    TBasicApp::GetLog().Exception( exc.GetException()->GetError() );
    for( int i=0; i < RemovedIns.Count(); i++ )
      delete (TStrList*)RemovedIns.Object(i);
    for( int i=0; i < restraints.Count(); i++ )
      restraints[i]->GetParent()->Restore(*restraints[i]);
  }
  dlg->Destroy();
}
//..............................................................................
void TMainForm::macEditIns(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TIns *Ins = (TIns*)FXApp->XFile().GetLastLoader();
  TStrList SL;
  TStrPObjList<olxstr, TStrList* > RemovedIns;
  olxstr Tmp;
  // go through instructions
  SL.Add("# instructions not processed by Olex2");
  for(int i=0; i < Ins->InsCount(); i++ )  {
    Tmp = Ins->InsName(i);  
    Tmp << ' ' << Ins->InsParams(i).Text(' ');
    SL.Add(Tmp);
  }
  SL.Add(EmptyString);
  Tmp = "WGHT";
  for( int i=0; i < Ins->Wght().Count(); i++ )
    Tmp << ' ' << Ins->Wght()[i];
  SL.Add(Tmp);
  olxstr& rm = SL.Add( Ins->GetRefinementMethod() );
  for( int i=0; i < Ins->GetLSV().Count(); i++ )
    rm << ' ' << Ins->GetLSV()[i];
  olxstr& pn = SL.Add( "PLAN" );
  for( int i=0; i < Ins->GetPlanV().Count(); i++ )
    pn << ' ' << ((i < 1) ? Round(Ins->GetPlanV()[i]) : Ins->GetPlanV()[i]);
  SL.Add("HKLF ") << Ins->Hklf();
  SL.Add("# instructions processed by Olex2");
  FXApp->XFile().UpdateAsymmUnit();  // synchronise au's
  Ins->SaveRestraints(SL, NULL, NULL, NULL);

  TdlgEdit *dlg = new TdlgEdit(this, true);
  dlg->SetText( SL.Text('\n') );
  try  {
    if( dlg->ShowModal() == wxID_OK )  {
      SL.Clear();
      SL.Strtok( dlg->GetText(), '\n' );
      Ins->ClearIns();
      Ins->GetAsymmUnit().ClearRestraints();
      FXApp->XFile().GetAsymmUnit().ClearRestraints();
      SL.CombineLines('=');
      Ins->ParseRestraints(SL, &FXApp->XFile().GetAsymmUnit());
      for( int i=0; i < SL.Count(); i++ )  {
        SL[i] = SL[i].Trim(' ');
        if( !SL[i].IsEmpty() && SL[i].CharAt(0) != '#' )  
          Ins->AddIns( SL[i] );
      }
      // synchronisation for new instructions
      FXApp->XFile().UpdateAsymmUnit();
      // update instruction parameters
      Ins->UpdateParams();
      // emulate loading a new file
      FXApp->XFile().EndUpdate();
    }
    else  {
    }
  }
  catch(const TExceptionBase& exc )  {
    TBasicApp::GetLog().Exception( exc.GetException()->GetError() );
  }
  dlg->Destroy();
}
//..............................................................................
//..............................................................................
void TMainForm::macEditHkl(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( !FXApp->CheckFileType<TIns>() )  return;

  TIns *Ins = (TIns*)FXApp->XFile().GetLastLoader();
  olxstr HklFN = Ins->GetHKLSource();
  if( !TEFile::FileExists(HklFN) )  {
    E.ProcessingError(__OlxSrcInfo, "could not locate the HKL file" );
    return;
  }
  THklFile Hkl;
  Hkl.LoadFromFile(HklFN);
  TRefPList Hkls;
  olxstr Tmp;
  TStrList SL;
  SL.Add("REM Please put \'-\' char in the front of reflections you wish to omit");
  SL.Add("REM and remove '-' char if you want the reflection to be used in the refinement");
  SL.Add(EmptyString);
  if( Cmds.Count() != 3 )  {
    if( Lst.IsLoaded() )  {
      for(int i=0; i < Lst.DRefCount(); i++ )  {
        Tmp = "REM    ";
        Tmp << Lst.DRef(i)->H << ' ';
        Tmp << Lst.DRef(i)->K << ' ';
        Tmp << Lst.DRef(i)->L << ' ';
        Tmp << "Delta(F^2)/esd=" << Lst.DRef(i)->DF;
        Tmp << " Resolution=" << Lst.DRef(i)->Res;
        SL.Add(Tmp);
        Hkls.Clear();
        Hkl.AllRefs(Lst.DRef(i)->H, Lst.DRef(i)->K, Lst.DRef(i)->L, FXApp->XFile().GetAsymmUnit(), Hkls);

        for( int j=0; j < Hkls.Count(); j++ )
          SL.Add( Hkls[j]->ToNString());

        SL.Add(EmptyString);
      }
    }
  }
  else  {
    TReflection Refl(Cmds[0].ToInt(), Cmds[1].ToInt(), Cmds[2].ToInt());
    Hkls.Clear();
    Hkl.AllRefs(Refl, FXApp->XFile().GetAsymmUnit(), Hkls);
    for( int i=0; i < Hkls.Count(); i++ )
      SL.Add( Hkls[i]->ToNString());
  }
  TdlgEdit *dlg = new TdlgEdit(this, true);
  dlg->SetText(SL.Text('\n'));
  if( dlg->ShowModal() == wxID_OK )  {
    Tmp = dlg->GetText();
    SL.Clear();
    SL.Strtok(Tmp, '\n');
    TReflection R(0, 0, 0);
    for( int i=0; i < SL.Count(); i++ )  {
      if( SL[i].ToUpperCase().StartsFrom("REM") )  continue;
      R.FromNString(SL[i]);
      Hkl.UpdateRef(R);
    }
    Hkl.SaveToFile(HklFN);
  }
  dlg->Destroy();
}
//..............................................................................
void TMainForm::macViewHkl(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( !Cmds.Count() )
    FXApp->HklVisible( !FXApp->HklVisible() );
  else
    FXApp->HklVisible( Cmds[0].ToBool() );
}
//..............................................................................
struct Main_3DIndex  {
  int x,y,z;
  Main_3DIndex(int a, int b, int c) : x(a), y(b), z(c)  {}
  Main_3DIndex()  { }
};
typedef TArrayList< Main_3DIndex > T3DIndexList;
bool InvestigateVoid(int x, int y, int z, TArray3D<short>& map, T3DIndexList& points)  {
  const int mapX = map.Length1(),
            mapY = map.Length2(),
            mapZ = map.Length3();
  short*** D = map.Data;
  const short refVal = D[x][y][z]-1;
  // skip the surface points
  if( refVal < 0 )  return false;
  D[x][y][z] = -D[x][y][z];
  for(int ii = -1; ii <= 1; ii++)  {
    for(int jj = -1; jj <= 1; jj++)  {
      for(int kk = -1; kk <= 1; kk++)  {
        if( ii == jj == kk == 0 )  continue;
        int iind = x+ii,
            jind = y+jj,
            kind = z+kk;
        if( iind < 0 )  iind += mapX;
        if( jind < 0 )  jind += mapY;
        if( kind < 0 )  kind += mapZ;
        if( iind >= mapX )  iind -= mapX;
        if( jind >= mapY )  jind -= mapY;
        if( kind >= mapZ )  kind -= mapZ;
        if( D[iind][jind][kind] <= 0 )  {
          continue;
        }
        else if( D[iind][jind][kind] >= refVal )  {
          points.Add( Main_3DIndex(iind, jind, kind) );
        }
      }
    }
  }
  return true;
}
//
void TMainForm::macCalcVoid(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  double surfdis = Options.FindValue("d", "0.25").ToDouble();
  bool invert = Options.Contains("i");
  int mapX = 100, mapY = 100, mapZ = 100;
  FXApp->XGrid().InitGrid(mapX, mapY, mapZ);
  double mapVol = mapX*mapY*mapZ;
  TArray3D<short> map(0, mapX-1, 0, mapY-1, 0, mapZ-1);
  short*** D = map.Data;
  long structureGridPoints = 0;
  int MaxLevel = FXApp->CalcVoid(map, surfdis, -101, &structureGridPoints);
  double vol = FXApp->XFile().GetLattice().GetUnitCell().CalcVolume();
  TBasicApp::GetLog() << ( olxstr("Cell volume (A^3) ") << olxstr::FormatFloat(3, vol) << '\n');
  TBasicApp::GetLog() << ( olxstr("Max level reached ") << MaxLevel << '\n');
  TBasicApp::GetLog() << ( olxstr("Largest spherical void is (A^3) ") << olxstr::FormatFloat(3, MaxLevel*MaxLevel*MaxLevel*4*M_PI/(3*mapVol)*vol) << '\n');
  TBasicApp::GetLog() << ( olxstr("Structure occupies (A^3) ") << olxstr::FormatFloat(3, structureGridPoints*vol/mapVol) << '\n');
  int minLevel = Round( pow( 6*mapVol*3/(4*M_PI*vol), 1./3) );
  TBasicApp::GetLog() << ( olxstr("6A^3 level is ") << minLevel << '\n');
  TIntList levels(MaxLevel+ 2);
  for( int i=0; i < levels.Count(); i++ )
    levels[i] = 0;
  //FGlConsole->PostText( olxstr("0.5A level is ") << minLevel1 );
  structureGridPoints = 0;
  T3DIndexList allPoints;
  allPoints.SetCapacity( mapVol );
  for(int i=0; i < mapX; i++ )  {
    for(int j=0; j < mapY; j++ )  {
      for(int k=0; k < mapZ; k++ )  {
        if( D[i][j][k] > minLevel )
          allPoints.Add( Main_3DIndex(i, j, k));
        if( D[i][j][k] >= 0 )
          levels[ D[i][j][k] ] ++;
      }
    }
  }
  for( int i=0; i < allPoints.Count(); i++ )  {
    if( InvestigateVoid(allPoints[i].x, allPoints[i].y, allPoints[i].z, map, allPoints) )
      structureGridPoints ++;
  }
  allPoints.Clear();
  TBasicApp::GetLog() << ( olxstr("Total solvent accessible area is (A^3) ") << olxstr::FormatFloat(3, structureGridPoints*vol/mapVol) << '\n');
  // set map to view voids
  if( invert )  {
    for(int i=0; i < mapX; i++ )  {
      for(int j=0; j < mapY; j++ )  {
        for(int k=0; k < mapZ; k++ )  {
          if( D[i][j][k] < 0 && D[i][j][k] != -101 )
            FXApp->XGrid().SetValue(i, j, k, D[i][j][k]);
          else
            FXApp->XGrid().SetValue(i, j, k, -D[i][j][k]);
        }
      }
    }
  }
  else  {
    for(int i=0; i < mapX; i++ )  {
      for(int j=0; j < mapY; j++ )  {
        for(int k=0; k < mapZ; k++ )  {
          if( D[i][j][k] < 0 && D[i][j][k] != -101 )
            FXApp->XGrid().SetValue(i, j, k, -D[i][j][k]);
          else
            FXApp->XGrid().SetValue(i, j, k, D[i][j][k]);
        }
      }
    }
  }
  double totalVol = 0;
  for( int i=MaxLevel; i >= 0; i-- )  {
    totalVol += levels[i];
    TBasicApp::GetLog() << ( olxstr("Level ") << i << " corresponds to " <<
      olxstr::FormatFloat(3, totalVol*vol/mapVol) << "(A^3)\n" );
  }
  FXApp->XGrid().InitIso();
  FXApp->ShowGrid(true, EmptyString);
}
//..............................................................................
void TMainForm::macViewGrid(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  olxstr gname;
  if( FXApp->XFile().GetLastLoader() != NULL && Cmds.IsEmpty() )  {
    gname = TEFile::ExtractFilePath(FXApp->XFile().GetFileName());
    TEFile::AddTrailingBackslashI(gname) << "map.txt";
  }
  else if( Cmds.Count() != 0 )
    gname = Cmds[0];
  FXApp->ShowGrid(true, gname);
}
//..............................................................................
void TMainForm::macExtractHkl(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TGlGroup* sel = FXApp->Selection();
  if( !sel )  {
    E.ProcessingError(__OlxSrcInfo, "please select some reflections" );
    return;
  }
  TRefPList Refs;
  AGDrawObject* obj;
  for(int i=0; i < sel->Count(); i++ )  {
    obj = sel->Object(i);
    if( EsdlInstanceOf(*obj, TXReflection) )
      Refs.Add( ((TXReflection*)obj)->Reflection() );
  }
  if( Refs.IsEmpty() )  {
    E.ProcessingError(__OlxSrcInfo, "please select some reflections" );
    return;
  }
  THklFile::SaveToFile(Cmds[0], Refs, true);
}
//..............................................................................
void TMainForm::macAppendHkl(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)
{
  throw TNotImplementedException(__OlxSourceInfo);
}
//..............................................................................
void TMainForm::macExcludeHkl(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)
{
  throw TNotImplementedException(__OlxSourceInfo);
}
//..............................................................................
void TMainForm::macDirection(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TVPointD Z;
  TMatrixD Basis;

  Basis =  FXApp->GetRender().GetBasis().GetMatrix();
  olxstr Tmp;
  if( FXApp->XFile().GetLastLoader() )  {
    TMatrixD cellM(3,3), M(3,3), m;
    TVPointD N(0, 0, 1);
    TAsymmUnit &au = FXApp->XFile().GetAsymmUnit();
    if( FXApp->HklVisible() ) m = au.GetHklToCartesian();
    else                      m = au.GetCellToCartesian();

    for( int i=0; i < 3; i++ )
      for( int j=0; j < 3; j++ )
        cellM[i][j] = m[i][j];

    cellM *= Basis;
    cellM.Transpose();
    // 4x4 -> 3x3 matrix
    Z = cellM[0];    M[0] = Z;
    Z = cellM[1];    M[1] = Z;
    Z = cellM[2];    M[2] = Z;
    Z.Null();
    TMatrixD::GauseSolve(M, N, Z);
    Z.Normalise();
    if( FXApp->HklVisible() )  {
      Tmp =  "Direction: (";
      Tmp << olxstr::FormatFloat(3, Z[0]) << "*H, " <<
             olxstr::FormatFloat(3, Z[1]) << "*K, " <<
             olxstr::FormatFloat(3, Z[2]) << "*L)";
      TBasicApp::GetLog() << (Tmp << '\n');
      double H = fabs(Z[0]);  H *= H;
      double K = fabs(Z[1]);  K *= K;
      double L = fabs(Z[2]);  L *= L;
      if( H > 0.01 )  H = 1./H;
      if( K > 0.01 )  K = 1./K;
      if( L > 0.01 )  L = 1./L;
      int iH = Round(H), iK = Round(K), iL = Round(L);
      double diff = fabs(H + K + L - iH - iK - iL)/3;
      if( diff < 0.15 )  {
        Tmp = "View along (";
        TBasicApp::GetLog() << ( Tmp << iH << ", " << iK << ", " << iL << ')' << '\n');
      }
    }
    else  {
      Tmp =  "Direction: (";
      Tmp << olxstr::FormatFloat(3, Z[0]) << "*A, " <<
             olxstr::FormatFloat(3, Z[1]) << "*B, " <<
             olxstr::FormatFloat(3, Z[2]) << "*C)";
      TBasicApp::GetLog() << (Tmp << '\n');
      const char *Dir[] = {"000", "100", "010", "001", "110", "101", "011", "111"};
      TTypeList<TVPointD > Points;
      TVPointD D;
      cellM.Transpose();
      Z.Null();                          Points.AddCCopy(Z);
      Z = cellM[0];                      Points.AddCCopy(Z);
      Z = cellM[1];                      Points.AddCCopy(Z);
      Z = cellM[2];                      Points.AddCCopy(Z);
      Z = cellM[0] + cellM[1];           Points.AddCCopy(Z);
      Z = cellM[0] + cellM[2];           Points.AddCCopy(Z);
      Z = cellM[1] + cellM[2];           Points.AddCCopy(Z);
      Z = cellM[0] + cellM[1] + cellM[2]; Points.AddCCopy(Z);
      for( int i=0; i < Points.Count(); i++ )  {
        N = Points[i];
        for( int j = i+1; j < Points.Count(); j++ )  {
          Z = Points[j];
          D = Z - N;
          D.Normalise();
          if( fabs( fabs(D[2])-1 ) < 0.02 )  {  // 98 % coincidence
            Tmp = "View along ";
            Tmp << Dir[i] <<  '-' <<  Dir[j] << ' ' <<
                   '(' << (int)(100.00-fabs(fabs(D[2])-1)*100) <<  '%' << ')';
            TBasicApp::GetLog() << (Tmp << '\n');
          }
        }
      }
    }
  }
  else  {
    Z = Basis[2];
    Tmp =  "Normal: (";
    Tmp << olxstr::FormatFloat(3, Z[0]) << ", " <<
           olxstr::FormatFloat(3, Z[1]) << ", " <<
           olxstr::FormatFloat(3, Z[2]) << ')';
    TBasicApp::GetLog() << (Tmp << '\n');
  }
}
//..............................................................................
void TMainForm::macUndo(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( !FUndoStack->isEmpty() )  {
    TUndoData* data = FUndoStack->Pop();
    data->Undo();
    delete data;
  }
}
//..............................................................................
void TMainForm::macIndividualise(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TXAtomPList Atoms;
  FXApp->FindXAtoms(Cmds.Text(' '), Atoms);
  for( int i=0; i < Atoms.Count(); i++ )  {
    FXApp->Individualise( Atoms[i] );
  }
}
//..............................................................................
void TMainForm::macCollectivise(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TXAtomPList Atoms;
  FXApp->FindXAtoms(Cmds.Text(' '), Atoms);
  for( int i=0; i < Atoms.Count(); i++ )  {
    FXApp->Collectivise( Atoms[i] );
  }
}
//..............................................................................
void TMainForm::macSel(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( Options.Count() == 0 )  {  // print labels of selected atoms
    olxstr Tmp("sel");
    int period=5;
    float v;
    TGlGroup *Sel = FXApp->Selection();
    TXAtomPList Atoms;
    TXAtom *XA;
    FXApp->FindXAtoms(Tmp, Atoms, false);
    for( int i=0; i <= Atoms.Count(); i+=period )  {
      Tmp = EmptyString;
      for( int j=0; j < period; j++ )  {
        if( (j+i) >= Atoms.Count() )  break;
        XA = Atoms.Item(i+j);
        Tmp << XA->Atom().GetLabel();
        if( !XA->Atom().GetMatrix(0).IsE() )  {
          Tmp << '.' << TSymmParser::MatrixToSymmCode(FXApp->XFile().GetLattice().GetUnitCell(), XA->Atom().GetMatrix(0));
          Tmp.Format((j+1)*14, true, ' ');
        }
        else
          Tmp.Format((j+1)*7, true, ' ');
      }
      if( !Tmp.IsEmpty() )
        TBasicApp::GetLog() << (Tmp << '\n');
    }
    if( Cmds.Count() != 0 )  {
      int whereIndex = Cmds.IndexOf(olxstr("where"));
      if( whereIndex >= 1 )  {
        Tmp = Cmds[whereIndex-1];
        while( whereIndex >= 0  )  {  Cmds.Delete(whereIndex);  whereIndex --;  }
        if( !Tmp.Comparei("atoms") )
          FXApp->SelectAtomsWhere(Cmds.Text(' '));
        else if( !Tmp.Comparei("bonds") )
          FXApp->SelectBondsWhere(Cmds.Text(' '));
        else
          Error.ProcessingError(__OlxSrcInfo, "undefined keyword: " ) << Tmp;
        return;
      }
      else  {
        int ringsIndex = Cmds.IndexOf(olxstr("rings"));
        if( ringsIndex != -1 )  {
          Cmds.Delete( ringsIndex );
          FXApp->SelectRings(Cmds.Text(' '));
        }
        else
          FXApp->SelectAtoms(Cmds.Text(' '));
        return;
      }
    }
    if( Sel->Count() == 2 )  {
      if( EsdlInstanceOf(*Sel->Object(0), TXAtom) &&
          EsdlInstanceOf(*Sel->Object(1), TXAtom) )  {
        Tmp = "Distance: ";
        v = ((TXAtom*)Sel->Object(0))->Atom().Center().DistanceTo(
              ((TXAtom*)Sel->Object(1))->Atom().Center());
        Tmp << olxstr::FormatFloat(3, v);
        TBasicApp::GetLog() << (Tmp << '\n');
        return;
      }
      if( EsdlInstanceOf(*Sel->Object(0), TXBond) &&
          EsdlInstanceOf(*Sel->Object(1), TXBond) )  {
        Tmp = "Angle (bond-bond): ";
        TVPointD V, V1;
        TXBond* A = (TXBond*)Sel->Object(0), *B =(TXBond*)Sel->Object(1);
        V = A->Bond().A().Center() - A->Bond().B().Center();
        V1 = B->Bond().A().Center() - B->Bond().B().Center();
        v = V.CAngle(V1);  v = acos(v)*180/M_PI;
        Tmp << olxstr::FormatFloat(3, v) << " (" <<
               olxstr::FormatFloat(3, 180-v) << ')';
        TBasicApp::GetLog() << (Tmp << '\n');

        Tmp = "Torsion angle (bond-bond, away from closest atoms): ";
        double distances[4];
        int minInd;
        distances[0] = A->Bond().A().Center().DistanceTo( B->Bond().A().Center() );
        distances[1] = A->Bond().A().Center().DistanceTo( B->Bond().B().Center() );
        distances[2] = A->Bond().B().Center().DistanceTo( B->Bond().A().Center() );
        distances[3] = A->Bond().B().Center().DistanceTo( B->Bond().B().Center() );

        TVectorD::ArrayMin(&distances[0], minInd, 4);
        // ceck if the adjustent bonds
        if( fabs(distances[minInd]) < 0.01 )  return;
        TVPointD V2, V3, V4, V5;
        switch( minInd )  {
          case 0:
            V = A->Bond().B().Center() - A->Bond().A().Center();
            V1 = B->Bond().A().Center() - A->Bond().A().Center();
            V2 = B->Bond().B().Center() - B->Bond().A().Center();
            V3 = A->Bond().A().Center() - B->Bond().A().Center();
            break;
          case 1:
            V = A->Bond().B().Center() - A->Bond().A().Center();
            V1 = B->Bond().B().Center() - A->Bond().A().Center();
            V2 = B->Bond().A().Center() - B->Bond().B().Center();
            V3 = A->Bond().A().Center() - B->Bond().B().Center();
            break;
          case 2:
            V = A->Bond().A().Center() - A->Bond().B().Center();
            V1 = B->Bond().A().Center() - A->Bond().B().Center();
            V2 = B->Bond().B().Center() - B->Bond().A().Center();
            V3 = A->Bond().B().Center() - B->Bond().A().Center();
            break;
          case 3:
            V = A->Bond().A().Center() - A->Bond().B().Center();
            V1 = B->Bond().B().Center() - A->Bond().B().Center();
            V2 = B->Bond().A().Center() - B->Bond().B().Center();
            V3 = A->Bond().B().Center() - B->Bond().B().Center();
            break;
        }
        V4 = V.XProdVec(V1);
        V5 = V2.XProdVec(V3);
        if( !V4.Length() || !V5.Length() )  return;
        v = V4.CAngle(V5);  v = acos(v)*180/M_PI;
        Tmp << olxstr::FormatFloat(3, v) << " (" << olxstr::FormatFloat(3, 180-v) << ')';
        TBasicApp::GetLog() << (Tmp << '\n');
        return;
      }
      if( EsdlInstanceOf(*Sel->Object(0), TXPlane) &&
          EsdlInstanceOf(*Sel->Object(1), TXAtom) )  {
        Tmp = "Distance (plane-atom): ";
        v = ((TXPlane*)Sel->Object(0))->Plane().DistanceTo(((TXAtom*)Sel->Object(1))->Atom());
        TBasicApp::GetLog() << ( Tmp << olxstr::FormatFloat(3, v) << '\n');
        Tmp = "Distance (plane centroid-atom): ";
        v = ((TXPlane*)Sel->Object(0))->Plane().Center().DistanceTo(((TXAtom*)Sel->Object(1))->Atom().Center());
        TBasicApp::GetLog() << ( Tmp << olxstr::FormatFloat(3, v) << '\n');
        return;
      }
      if( EsdlInstanceOf(*Sel->Object(0), TXAtom) &&
          EsdlInstanceOf(*Sel->Object(1), TXPlane) )  {
        Tmp = "Distance (plane-atom): ";
        v = ((TXPlane*)Sel->Object(1))->Plane().DistanceTo(((TXAtom*)Sel->Object(0))->Atom());
        TBasicApp::GetLog() << ( Tmp << olxstr::FormatFloat(3, v) << '\n');
        Tmp = "Distance (plane centroid-atom): ";
        v = ((TXPlane*)Sel->Object(1))->Plane().Center().DistanceTo(((TXAtom*)Sel->Object(0))->Atom().Center());
        TBasicApp::GetLog() << ( Tmp << olxstr::FormatFloat(3, v) << '\n');
        return;
      }
      if( EsdlInstanceOf(*Sel->Object(0), TXBond) &&
          EsdlInstanceOf(*Sel->Object(1), TXPlane) )  {
        Tmp = "Angle (plane-bond): ";
        v = ((TXPlane*)Sel->Object(1))->Plane().Angle(((TXBond*)Sel->Object(0))->Bond());
        TBasicApp::GetLog() << ( Tmp << olxstr::FormatFloat(3, v) << '\n');
        return;
      }
      if( EsdlInstanceOf(*Sel->Object(1), TXBond) &&
          EsdlInstanceOf(*Sel->Object(0), TXPlane) )  {
        Tmp = "Angle (plane-bond): ";
        v = ((TXPlane*)Sel->Object(0))->Plane().Angle(((TXBond*)Sel->Object(1))->Bond());
        TBasicApp::GetLog() << ( Tmp << olxstr::FormatFloat(3, v) << '\n');
        return;
      }
      if( EsdlInstanceOf(*Sel->Object(1), TXPlane) &&
          EsdlInstanceOf(*Sel->Object(0), TXPlane) )  {
        Tmp = "Angle (plane-plane): ";
        v = ((TXPlane*)Sel->Object(0))->Plane().Angle(((TXPlane*)Sel->Object(1))->Plane());
        TBasicApp::GetLog() << ( Tmp << olxstr::FormatFloat(3, v) << '\n');
        Tmp = "Distance (plane centroid-plane centroid): ";
        v = ((TXPlane*)Sel->Object(0))->Plane().Center().DistanceTo(((TXPlane*)Sel->Object(1))->Plane().Center());
        TBasicApp::GetLog() << ( Tmp << olxstr::FormatFloat(3, v) << '\n');
        return;
      }
    }
    if( Sel->Count() == 3 )  {
      if( EsdlInstanceOf(*Sel->Object(0), TXAtom) &&
          EsdlInstanceOf(*Sel->Object(1), TXAtom) &&
          EsdlInstanceOf(*Sel->Object(2), TXAtom) )  {
        Tmp = "Angle: ";
        TVPointD V, V1;
        V = ((TXAtom*)Sel->Object(0))->Atom().Center() - ((TXAtom*)Sel->Object(1))->Atom().Center();
        V1 = ((TXAtom*)Sel->Object(2))->Atom().Center() - ((TXAtom*)Sel->Object(1))->Atom().Center();
        v = V.CAngle(V1);  v = acos(v)*180/M_PI;
        TBasicApp::GetLog() << ( Tmp << olxstr::FormatFloat(3, v) << '\n');
        return;
      }
    }
    if( Sel->Count() == 4 )  {
      if( EsdlInstanceOf(*Sel->Object(0), TXAtom) &&
          EsdlInstanceOf(*Sel->Object(1), TXAtom) &&
          EsdlInstanceOf(*Sel->Object(2), TXAtom) &&
          EsdlInstanceOf(*Sel->Object(3), TXAtom) )  {
        Tmp = "Torsion angle: ";
        TVPointD V1, V2, V3, V4;
        V1 = ((TXAtom*)Sel->Object(0))->Atom().Center() - ((TXAtom*)Sel->Object(1))->Atom().Center();
        V2 = ((TXAtom*)Sel->Object(2))->Atom().Center() - ((TXAtom*)Sel->Object(1))->Atom().Center();
        V3 = ((TXAtom*)Sel->Object(3))->Atom().Center() - ((TXAtom*)Sel->Object(2))->Atom().Center();
        V4 = ((TXAtom*)Sel->Object(1))->Atom().Center() - ((TXAtom*)Sel->Object(2))->Atom().Center();

        V1 = V1.XProdVec(V2);
        V3 = V3.XProdVec(V4);
        if( !V1.Length() || !V3.Length() )  return;
        v = V1.CAngle(V3);  v = acos(v)*180/M_PI;
        Tmp << olxstr::FormatFloat(3, v) << " (" << olxstr::FormatFloat(3, 180-v) << ')';
        TBasicApp::GetLog() << (Tmp << '\n');
        return;
      }
    }
  }
  else  {
    for( int i=0; i < Options.Count(); i++ )  {
      if( Options.GetName(i)[0] == 'a' )
        FXApp->SelectAll(true);
      else if( Options.GetName(i)[0] == 'u' )
        FXApp->SelectAll(false);
      else if( Options.GetName(i)[0] == 'i' )  {
        if( Cmds.Count() == 0 )
          FXApp->GetRender().InvertSelection();
        else
          FXApp->SelectAtoms(Cmds.Text(' '), true);
      }
      else
        Error.ProcessingError(__OlxSrcInfo, "wrong options" );
    }
  }
}
//..............................................................................
void TMainForm::macReap(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  SetSGList(EmptyString);
  olxstr FN, Tmp;
  bool Blind = Options.Contains('b'); // a switch showing if the last file is remembered
  bool ReadStyle = !Options.Contains('r');
  bool OverlayXFile = Options.Contains('*');
  if( Cmds.Count() >= 1 )  {  // merge the file name if a long one...
    FN = Cmds.Text(' ');
#ifdef __WIN32__ // tackle short path names problem
    WIN32_FIND_DATAA wfd;
    ZeroMemory(&wfd, sizeof(wfd));
    HANDLE fsh = FindFirstFileA(FN.c_str(), &wfd);
    if( fsh != NULL )  {
      FN = TEFile::AddTrailingBackslash(TEFile::ExtractFilePath(FN)) + wfd.cFileName;
      FindClose(fsh);
    }
#endif // win32
    Tmp = TEFile::ExtractFilePath(FN);
    if( Tmp.IsEmpty() )  { FN = CurrentDir + FN; }
  }
  else  {
    FN = PickFile("Open File",
        olxstr("All supported files|*.ins;*.cif;*res;*.mol;*.xyz;*.p4p;*.mas;*.crs;*pdb;*.fco;*.fcf;*.hkl")  <<
          "|INS files (*.ins)|*.ins"  <<
          "|CIF files (*.cif)|*.cif" <<
          "|MDL MOL files (*.mol)|*.mol" <<
          "|XYZ files (*.xyz)|*.xyz" <<
          "|P4P files (*.p4p)|*.p4p" <<
          "|PDB files (*.pdb)|*.pdb" <<
          "|XD Master files (*.mas)|*.mas" <<
          "|CRS files (*.crs)|*.crs" <<
          "|FCO files (*.fco)|*.fco" <<
          "|FCF files (*.fcf)|*.fcf" <<
          "|HKL files (*.hkl)|*.hkl",
        CurrentDir, true);
  }
  
  if( !FN.IsEmpty() )  {  // the dialog has been successfully executed
    /* with some compilations Borland would bring program into an incorrect state
     if the NonExistenFile exception is thrown from XFile ... (MSVC is fine thought)
    */
    if( !TEFile::FileExists(FN) )  {
      Error.ProcessingError(__OlxSrcInfo, olxstr("Nonexisting file: ") << FN );
      return;
    }
    if( OverlayXFile )  {
      TXFile& xf = FXApp->NewOverlayedXFile();
      xf.LoadFromFile( FN );
      return;
    }
    Tmp = TEFile::ChangeFileExt(FN, "xlds");
    if( TEFile::FileExists(Tmp) )  {
      ProcessXPMacro(olxstr("load view '") << TEFile::ChangeFileExt(FN, EmptyString) << '\'', Error);
    }
    else  {
      if( TEFile::FileExists(DefStyle) && ReadStyle )
        FXApp->GetRender().Styles()->LoadFromFile(DefStyle);
    }
    // delete the Space groups infor mation file
    if( !(TEFile::ChangeFileExt(FN, EmptyString) == TEFile::ChangeFileExt(FXApp->XFile().GetFileName(), EmptyString)) )
      TEFile::DelFile(DataDir+"spacegroups.htm");
    // special treatment of the kl files
    if( TEFile::ExtractFileExt(FN).Comparei("hkl") == 0 )  {
      if( !TEFile::FileExists( TEFile::ChangeFileExt(FN, "ins") ) )  {
        THklFile hkl;
        TIns* ins = (TIns*)FXApp->XFile().FindFormat("ins");
        //ins->Clear();
        hkl.LoadFromFile(FN, ins);
        FXApp->XFile().SetLastLoader(ins);
        ins->SetHKLSource(FN);  // make sure tha SGE finds the related HKL
        TMacroError er;
        ProcessXPMacro(olxstr("SGE '") << TEFile::ChangeFileExt(FN, "ins") << '\'', er);
        if( !er.HasRetVal() || !er.GetRetObj< TEPType<bool> >()->GetValue()  )  {
          olxstr s_inp("getuserinput(1, \'Please, enter the spacegroup\', \'')"), s_sg(s_inp);
          TSpaceGroup* sg = NULL;
          while( sg == NULL )  {
            ProcessMacroFunc(s_sg);
            sg = TSymmLib::GetInstance()->FindGroup(s_sg);
            if( sg != NULL ) break;
            s_sg = s_inp;
          }
          ins->GetAsymmUnit().ChangeSpaceGroup(*sg);
          if( ins->GetSfac().IsEmpty() )  {
            s_inp = "getuserinput(1, \'Please, enter cell content\', \'C1')";
            ProcessMacroFunc(s_inp);
            ins->SetSfacUnit(s_inp);
          }
          else  {
            int sfac_count = ins->GetSfac().CharCount(' ');
            olxstr unit;
            for( int i=0; i < sfac_count; i++ )  
              unit << (sg->MatrixCount()+1)*(sg->GetLattice().VectorCount()+1) << ' ';
            ins->SetUnit(unit);
            ins->GetAsymmUnit().SetZ( (sg->MatrixCount()+1)*(sg->GetLattice().VectorCount()+1));
          }
          ins->SaveToRefine( TEFile::ChangeFileExt(FN, "ins"), EmptyString, EmptyString );
          ProcessXPMacro( olxstr("reap '") << TEFile::ChangeFileExt(FN, "ins") << '\'', Error);
          ProcessXPMacro("solve", Error);
        }  // sge, if succeseded will run reap and solve
        return;
      }
      else
        FN = TEFile::ChangeFileExt(FN, "ins");
    }
    try  {
      SaveVFS(plStructure); // save virtual fs file
      int64_t st = TETime::msNow();
      FXApp->LoadXFile(FN);
      st = TETime::msNow() - st;
      TBasicApp::GetLog().Info( olxstr("Structure loaded in ") << st << " ms\n");
      LoadVFS(plStructure);  // load virtual fs file
    }
    catch(TEmptyFileException)  {
      olxstr lstFileName = TEFile::ChangeFileExt(FN, "lst");
      if( TEFile::FileExists(lstFileName)  )  {
        try  {  Lst.LoadFromFile(lstFileName);  }
        catch( ... )  {  throw;  }

        for( int i=0; i < Lst.ErrMsgCount(); i++ )  {
          TBasicApp::GetLog().Error( olxstr("Failure instruction: ") << Lst.GetCause(i) );
          TBasicApp::GetLog().Error( olxstr("Failure message: ") << Lst.GetError(i) );
          Tmp = Lst.GetCause(i);
          Tmp.Replace( "TO", EmptyString );
          if( !Tmp.IsEmpty() )
            FXApp->SelectAtoms( Tmp );
          Tmp = Lst.GetError(i);
          Tmp.Replace( "TO", EmptyString );
          FXApp->SelectAtoms( Tmp );
        }
      }
      return;
    }
    if( FXApp->XFile().GetLastLoader() &&
        ( EsdlInstanceOf(*FXApp->XFile().GetLastLoader(), TP4PFile) ||
          EsdlInstanceOf(*FXApp->XFile().GetLastLoader(), TCRSFile)    ) )  
    {
      TMacroError er;
      if( TEFile::FileExists( TEFile::ChangeFileExt(FN, "ins") ) )
        ProcessXPMacro("SG", er);
      else
        ProcessXPMacro("SGE", er);
    }
    // automatic export for kappa cif
    if( FXApp->XFile().GetLastLoader() && (EsdlInstanceOf(*FXApp->XFile().GetLastLoader(), TCif)) )  {
      TBasicApp::GetLog() << ("Start importing cif ...\n");
      FXApp->Draw();
      olxstr hklFileName = TEFile::ChangeFileExt(FN, "hkl");
      olxstr insFileName = TEFile::ChangeFileExt(FN, "ins");
      TMacroError er;
      if( !TEFile::FileExists(hklFileName)  )  {
        TCif* C = (TCif*)FXApp->XFile().GetLastLoader();
        if( C->FindLoop("_refln") != NULL )  {
          ProcessXPMacro( olxstr("export ") << TEFile::ExtractFileName(hklFileName), er);
          if( !er.IsProcessingError() )  {
            if( !TEFile::FileExists(insFileName) )  {
              TIns ins( FXApp->AtomsInfo() );
              ins.Adopt( &FXApp->XFile() );
              ins.SaveToFile( insFileName );
              ProcessXPMacro( olxstr("@reap \'") << insFileName << '\'', er);
              if( !er.IsProcessingError() )  {
                TBasicApp::GetLog() << ("End importing cif ...\n");
                ProcessXPMacro("reset", er);
              }
              FXApp->Draw();
              return;
            }
          }
          else
            AnalyseError( er );
        }
      }
    }
    FUndoStack->Clear();
    FInfoBox->Clear();
    FInfoBox->PostText(FN);
    FInfoBox->PostText(FXApp->XFile().GetLastLoader()->GetTitle());
    // changes the loaded position of the box to left
    OnResize();

    Tmp = TEFile::ExtractFilePath(FN);
    if( Tmp.Length() && !(Tmp == CurrentDir) )  {
      if( !TEFile::ChangeDir(Tmp) )
        TBasicApp::GetLog() << ("Cannot change current folder...\n");
      else  {
        if( !Blind )  CurrentDir = Tmp;
      }
    }
    if( !Blind )
      UpdateRecentFile(FN);
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    QPeaksTable(DataDir+"qpeaks.htm", false);
    RecentFilesTable(DataDir+"recent_files.htm", false);
    olxstr lstFileName = TEFile::ChangeFileExt(FN, "lst");
    if( TEFile::FileExists(lstFileName)  )  {
      Lst.LoadFromFile(lstFileName);
      if( !BadReflectionsTable(&Lst, DataDir+"badrefs.htm", false) )
        TEFile::DelFile(DataDir+"badrefs.htm");
      if( !RefineDataTable(&Lst, DataDir+"refinedata.htm", false) )
        TEFile::DelFile(DataDir+"refinedata.htm");
      if( Lst.SplitAtomCount() )  {
        TLstSplitAtom *SpA;
        TBasicApp::GetLog() << ("The following atom(s) may be split: \n");
        for( int i=0; i < Lst.SplitAtomCount(); i++ )  {
          SpA = Lst.SplitAtom(i);
          Tmp = SpA->AtomName;  Tmp.Format(5, true, ' ');
          Tmp << olxstr::FormatFloat(3, SpA->PositionA[0]);  Tmp.Format(12, true, ' ');
          Tmp << olxstr::FormatFloat(3, SpA->PositionA[1]);  Tmp.Format(19, true, ' ');
          Tmp << olxstr::FormatFloat(3, SpA->PositionA[2]);  Tmp.Format(26, true, ' ');
          Tmp << "& ";
          Tmp << olxstr::FormatFloat(3, SpA->PositionB[0]);  Tmp.Format(35, true, ' ');
          Tmp << olxstr::FormatFloat(3, SpA->PositionB[1]);  Tmp.Format(42, true, ' ');
          Tmp << olxstr::FormatFloat(3, SpA->PositionB[2]);
          TBasicApp::GetLog() << (Tmp << '\n');
        }
      }
      if( Lst.TrefTryCount() )  {
        TBasicApp::GetLog() << ("TREF tries:\n");
        olxstr Tmp1;
        Tmp = "CFOM";  Tmp.Format(6, true, ' ');
        Tmp1 = Tmp;
        Tmp = "NQual";  Tmp1 << Tmp.Format(10, true, ' ');
        Tmp = "Try#";   Tmp1 << Tmp.Format(10, true, ' ');
        Tmp1 << "Semivariants";
        TBasicApp::GetLog() << (Tmp1 << '\n');
        int tcount = 0;
        for( int i=0; i < Lst.TrefTryCount(); i++ )  {
          if( i > 0 )  {
            if( Lst.TrefTry(i-1).CFOM == Lst.TrefTry(i).CFOM &&
                Lst.TrefTry(i-1).Semivariants == Lst.TrefTry(i).Semivariants &&
                Lst.TrefTry(i-1).NQual == Lst.TrefTry(i).NQual )
              continue;
          }
          Tmp = Lst.TrefTry(i).CFOM;  Tmp.Format(6, true, ' ');
          Tmp1 = Tmp;
          Tmp = Lst.TrefTry(i).NQual;  Tmp1 << Tmp.Format(10, true, ' ');
          Tmp = Lst.TrefTry(i).Try;    Tmp1 << Tmp.Format(10, true, ' ');
          Tmp1 << Lst.TrefTry(i).Semivariants.FormatString( 31 );
          //Tmp1 += Lst.TrefTry(i).Semivariants.FormatString( Lst.TrefTry(i).Semivariants.Count() );
          TBasicApp::GetLog() << (Tmp1 << '\n');
          tcount ++;
          if( tcount > 5 && ( (i+1) < Lst.TrefTryCount()) )  {
            TBasicApp::GetLog() << ( olxstr("There are ") << Lst.TrefTryCount() - i << " more tries\n");
            break;
          }
        }
      }
      if( Lst.PattSolutionCount() > 1 )  {
        TBasicApp::GetLog() << ( olxstr("There are ") << Lst.PattSolutionCount()
            << " possible patterson solutions in the listing file\n");
        TBasicApp::GetLog() << ( "To browse possible solutions press Ctrl+Up and Ctrl+Down buttons. Press Enter to choose a particular solution\n" );
        FMode |= mSolve;
        CurrentSolution = -1;
      }
    }
    else  {
      TEFile::DelFile(DataDir+"badrefs.htm");
      TEFile::DelFile(DataDir+"refinedata.htm");
    }
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//      FXApp->Draw();  // to update the scene just in case...
    FGlConsole->SetCommand( FGlConsole->GetCommand() );  // force th eupdate
    return;
  }
  else  {
    Error.ProcessingError(__OlxSrcInfo, "no file name is given" );
    return;
  }
}
//..............................................................................
void TMainForm::macPopup(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  int width = 100, height = 200, x=0, y=0;
  olxstr border, title, onDblClick;
  int iBorder = 0;
  for( int i=0; i < Options.Count(); i++ )  {
    if( !Options.GetName(i).Comparei("w") )  {  width = Options.GetValue(i).ToInt(); continue;  }
    if( !Options.GetName(i).Comparei("h") )  {  height = Options.GetValue(i).ToInt(); continue;  }
    if( !Options.GetName(i).Comparei("t") )  {  title = Options.GetValue(i); continue;  }
    if( !Options.GetName(i).Comparei("b") )  {  border = Options.GetValue(i); continue;  }
    if( !Options.GetName(i).Comparei("x") )  {  x = Options.GetValue(i).ToInt(); continue;  }
    if( !Options.GetName(i).Comparei("y") )  {  y = Options.GetValue(i).ToInt(); continue;  }
    if( !Options.GetName(i).Comparei("d") )  {  onDblClick = Options.GetValue(i); continue;  }
  }
  for( int i=0; i < border.Length(); i++ )  {
    if( border.CharAt(i) == 't' )  {  iBorder |= wxCAPTION;        continue;  }
    if( border.CharAt(i) == 'r' )  {  iBorder |= wxRESIZE_BORDER;  continue;  }
    if( border.CharAt(i) == 's' )  {  iBorder |= wxSYSTEM_MENU;    continue;  }
    if( border.CharAt(i) == 'c' )  {  iBorder |= wxCLOSE_BOX;     iBorder |= wxSYSTEM_MENU; continue;  }
    if( border.CharAt(i) == 'a' )  {  iBorder |= wxMAXIMIZE_BOX;  iBorder |= wxSYSTEM_MENU; continue;  }
    if( border.CharAt(i) == 'i' )  {  iBorder |= wxMINIMIZE_BOX;  iBorder |= wxSYSTEM_MENU; continue;  }
    if( border.CharAt(i) == 'p' )  {  iBorder |= wxSTAY_ON_TOP;   continue;  }
  }
  iBorder |= wxNO_BORDER;
  // check if the popup already exists
  TPopupData *pd = GetPopup( Cmds[0] );
  if( pd != NULL )  {
    //pd->Dialog->SetWindowStyle( iBorder );
    //pd->Dialog->SetSize(x, y, width, height, wxSIZE_USE_EXISTING);
    //pd->Dialog->SetTitle( title );
    pd->Html->LoadPage( uiStr(Cmds[1]) );
    pd->Html->SetHomePage(TutorialDir + Cmds[1]);
    if( !pd->Dialog->IsShown() )  pd->Dialog->Show();
    return;
  }

  wxDialog *dlg = new wxDialog(this, -1, uiStr(title), wxPoint(x,y), wxSize(width, height),
    iBorder, wxT("htmlPopupWindow") );
  THtml *html1 = new THtml(dlg, FXApp);
  html1->WebFolder( TutorialDir );
  html1->SetHomePage( TutorialDir + Cmds[1] );
  html1->Movable(true);

  pd = new TPopupData;
  pd->Dialog = dlg;
  pd->Html = html1;
  pd->OnDblClick = onDblClick;

  FPopups.Add(Cmds[0], pd);
  THtml* ph = FHtml;
  FHtml = html1;
  try  {
    html1->LoadPage( uiStr(Cmds[1]) );
  }
  catch( ... )  {}
  FHtml = ph;
  
  html1->OnLink->Add(this, ID_ONLINK);
  html1->OnKey->Add(this, ID_HTMLKEY);
  html1->OnDblClick->Add(this, ID_HTMLDBLCLICK);
  html1->OnCmd->Add(this, ID_HTMLCMD);
  dlg->GetClientSize(&width, &height);
  html1->SetSize(width, height);
  dlg->Show();
}
//..............................................................................
void TMainForm::macDelta(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Cmds.Count() == 1 )  {
    float delta = Cmds[0].ToDouble();
    if( delta < 0.1 || delta > 0.9 )  delta = 0.5;
    FXApp->XFile().GetLattice().SetDelta( delta );
    return;
  }
  TBasicApp::GetLog() << ( olxstr("Current delta (covalent bonds) is: ") << FXApp->XFile().GetLattice().GetDelta() << '\n' );
}
//..............................................................................
void TMainForm::macDeltaI(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Cmds.Count() == 1 )  {
    float deltai = Cmds[0].ToDouble();
    if( deltai < 0.9 || deltai > 1.7 )  deltai = 1.2;
    FXApp->XFile().GetLattice().SetDeltaI( deltai );
    return;
  }
  TBasicApp::GetLog() << ( olxstr("Current delta (short interactions bonds) is: ") << FXApp->XFile().GetLattice().GetDeltaI() << '\n' );
}
//..............................................................................
void TMainForm::macPython(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  olxstr tmp = Cmds.Text(' ');
  tmp.Replace("\\n", "\n");
  if( !tmp.EndsWith('\n') )  tmp << '\n';
  PythonExt::GetInstance()->RunPython(  tmp, false );
}
//..............................................................................
void TMainForm::funEval(const TStrObjList& Params, TMacroError &E)  {
  TStrList Vars;
  TStrPObjList<olxstr,TSOperation*> Funcs;
  TSOperation S(NULL, &Funcs, &Vars, NULL);
  if( S.LoadFromExpression(Params[0]) == 0 )
  {  E.SetRetVal( S.Evaluate() );  }
}
//..............................................................................
void TMainForm::macCreateMenu(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)
{
  int ind = Cmds[0].LastIndexOf(';');
  if( ind == -1 )  
    throw TInvalidArgumentException(__OlxSourceInfo, "menu name");
  olxstr menuName = Cmds[0].SubStringTo(ind);

  short itemType = mtNormalItem;
  olxstr modeDependent, stateDependent;
  for( int i=0; i < Options.Count(); i++ )
  {
    if( Options.GetName(i)[0] == 'r' )  {  itemType = mtRadioItem;  continue;  }
    if( Options.GetName(i)[0] == 'c' )  {  itemType = mtCheckItem;  continue;  }
    if( Options.GetName(i)[0] == '#' )  {  itemType = mtSeparator;  continue;  }
    if( Options.GetName(i)[0] == 's' )  {  stateDependent = Options.GetValue(i);  continue;  }
    if( Options.GetName(i)[0] == 'm' )  {  modeDependent = Options.GetValue(i);  continue;  }
  }
  TMenu* menu = Menus[menuName];
  if( menu == NULL )  {
    TStrList toks;
    toks.Strtok( Cmds[0], ';');
    int mi=0;
    while( (ind = menuName.LastIndexOf(';')) != -1 && ! menu )
    {
      menuName = menuName.SubStringTo(ind);
      menu = Menus[menuName];
      mi++;
      
      if( menu )  break;
    }

    if( ! menu )  mi = 0;
    else          mi = toks.Count() - 1 - mi;

    for( int i=mi; i < toks.Count(); i++ )
    {
      if( (i+1) == toks.Count() )
      {
        int accell = AccMenus.GetLastId();
        if( !accell ) accell = 1000;
        else          accell++;

        if( Cmds.Count() == 3 )  {
          int insindex = menu->FindItem( uiStr(Cmds[2]) );
          if( insindex == -1 )  insindex = 0;
          if( itemType == mtSeparator )  menu->InsertSeparator(insindex);
          else {
            TMenuItem* item = new TMenuItem(itemType, accell, menu, toks[i]);
            if( modeDependent.Length() )  item->ActionQueue( OnModeChange, modeDependent );
            if( stateDependent.Length() )  item->ActionQueue( OnStateChange, stateDependent );
            if( Cmds.Count() > 1 )  item->SetCommand( Cmds[1] );
            menu->Insert(insindex, item );
            AccMenus.AddAccell(accell, item );
          }
        }
        else  {
          if( itemType == mtSeparator )  menu->AppendSeparator();
          else {
            TMenuItem* item = new TMenuItem(itemType, accell, menu, toks[i]);
            if( modeDependent.Length() )  item->ActionQueue( OnModeChange, modeDependent );
            if( stateDependent.Length() )  item->ActionQueue( OnStateChange, stateDependent );
            if( Cmds.Count() > 1 )  item->SetCommand( Cmds[1] );
            menu->Append( item );
            AccMenus.AddAccell(accell, item );
          }
        }
      }
      else
      {
        TMenu* smenu = new TMenu();
        if( !menu )
        {
          if( Cmds.Count() == 3 )
          {
            int insindex = MenuBar->FindMenu( uiStr(Cmds[2]) );
            if( insindex == -1 )  insindex = 0;
            MenuBar->Insert( insindex, smenu, uiStr(toks[i]) );
          }
          else
            MenuBar->Append( smenu, uiStr(toks[i]) );
        }
        else
          menu->Append( -1, uiStr(toks[i]), (wxMenu*)smenu );
        olxstr addedMenuName;
        for( int j=0; j <= i; j++ )  {
          addedMenuName << toks[j];
          if( j < i )  addedMenuName << ';';
        }
        Menus.Add(addedMenuName, smenu);
        menu = smenu;
      }
    }
  }
  else  {
    int accell = AccMenus.GetLastId();
    if( !accell ) accell = 1000;
    else          accell++;
    menuName = Cmds[0].SubStringFrom(ind+1);
    if( menuName == '#' )  menu->AppendSeparator();
    else  {
      int insindex;
      if( (insindex=menu->FindItem( uiStr(menuName) )) != -1 )
        throw TFunctionFailedException(__OlxSourceInfo, "duplicated name");
      if( Cmds.Count() == 3 )  {
        if( insindex == -1 )  insindex = 0;

        if( itemType == mtSeparator )  menu->InsertSeparator(insindex);
        else  {
          TMenuItem* item = new TMenuItem(itemType, accell, menu, menuName);
          if( Cmds.Count() > 1 )  item->SetCommand( Cmds[1] );
          if( modeDependent.Length() )  item->ActionQueue( OnModeChange, modeDependent );
          if( stateDependent.Length() )  item->ActionQueue( OnStateChange, stateDependent );
          menu->Insert(insindex, item );
          AccMenus.AddAccell(accell, item );  }
        }
      else  {
        if( itemType == mtSeparator )  menu->AppendSeparator();
        else  {
          TMenuItem* item = new TMenuItem(itemType, accell, menu, menuName);
          if( modeDependent.Length() )  item->ActionQueue( OnModeChange, modeDependent );
          if( stateDependent.Length() )  item->ActionQueue( OnStateChange, stateDependent );
          if( Cmds.Count() > 1 )  item->SetCommand( Cmds[1] );
          menu->Append( item );
          AccMenus.AddAccell(accell, item );
        }
      }
    }
  }
}
//..............................................................................
void TMainForm::macDeleteMenu(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)
{
  TMenu* menu = Menus[Cmds[0]];
  if( !menu )
  {
    int ind = Cmds[0].LastIndexOf(';');
    if( ind == -1 )  return;
    olxstr menuName = Cmds[0].SubStringTo( ind );
    olxstr itemName =  Cmds[0].SubStringFrom( ind+1 );
    menu = Menus[menuName];
    if( !menu )  return;
    ind = menu->FindItem( uiStr(itemName) );
    if( ind == -1 )  return;
    menu->Destroy( ind );
  }
  else
  {   /*
    int ind = Cmds[0].LastIndexOf(';');
    if( ind == -1 )
    {
      ind = MenuBar->FindMenu( Cmds[0].c_str() );
      if( ind != -1 )  MenuBar
    }
    olxstr menuName = Cmds[0].SubStringTo( ind );
    olxstr itemName =  Cmds[0].SubStringFrom( ind+1 );
    menu = (TMenu*)Menus.ObjectByName( menuName );
    if( !menu )  return;
    ind = menu->FindItem( itemName.c_str() );
    if( ind == -1 )  return;
    menu->Destroy( ind );  */
  }
}
//..............................................................................
void TMainForm::macEnableMenu(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)
{
  int ind = Cmds[0].LastIndexOf(';');
  if( ind == -1 )  return;
  olxstr menuName = Cmds[0].SubStringTo( ind );
  olxstr itemName =  Cmds[0].SubStringFrom( ind+1 );
  TMenu* menu = Menus[menuName];
  if( !menu )  return;
  ind = menu->FindItem( uiStr(itemName) );
  if( ind == -1 )  return;
  menu->Enable( ind, true );
}
//..............................................................................
void TMainForm::macDisableMenu(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)
{
  int ind = Cmds[0].LastIndexOf(';');
  if( ind == -1 )  return;
  olxstr menuName = Cmds[0].SubStringTo( ind );
  olxstr itemName =  Cmds[0].SubStringFrom( ind+1 );
  TMenu* menu = Menus[menuName];
  if( !menu )  return;
  ind = menu->FindItem( uiStr(itemName) );
  if( ind == -1 )  return;
  menu->Enable( ind, false );
}
//..............................................................................
void TMainForm::macCheckMenu(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  int ind = Cmds[0].LastIndexOf(';');
  if( ind == -1 )  return;
  olxstr menuName = Cmds[0].SubStringTo( ind );
  olxstr itemName =  Cmds[0].SubStringFrom( ind+1 );
  TMenu* menu = Menus[menuName];
  if( !menu )  return;
  ind = menu->FindItem( uiStr(itemName) );
  if( ind == -1 )  return;
  menu->Check( ind, true );
}
//..............................................................................
void TMainForm::macUncheckMenu(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)
{
  int ind = Cmds[0].LastIndexOf(';');
  if( ind == -1 )  return;
  olxstr menuName = Cmds[0].SubStringTo( ind );
  olxstr itemName =  Cmds[0].SubStringFrom( ind+1 );
  TMenu* menu = Menus[menuName];
  if( !menu )  return;
  ind = menu->FindItem( uiStr(itemName) );
  if( ind == -1 )  return;
  menu->Check( ind, false );
}
//..............................................................................
void TMainForm::macCreateShortcut(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)
{
  AccShortcuts.AddAccell( TranslateShortcut( Cmds[0]), Cmds[1] );
}
//..............................................................................
void TMainForm::macSetCmd(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)
{
  FGlConsole->SetCommand( Cmds.Text(' ') );
}
//..............................................................................
void TMainForm::funCmdList(const TStrObjList &Cmds, TMacroError &E)
{
  if( !FGlConsole->GetCommandCount() ) return;

  int cc = FGlConsole->GetCommandIndex() + Cmds[0].ToInt();
  if( cc >= FGlConsole->GetCommandCount() )  cc = 0;
  if( cc < 0 )  cc = FGlConsole->GetCommandCount() - 1;

  E.SetRetVal( FGlConsole->GetCommandByIndex(cc) );
}
//..............................................................................
void TMainForm::macUpdateOptions(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)
{
  TdlgUpdateOptions* dlg = new TdlgUpdateOptions(this);
  dlg->ShowModal();
  dlg->Destroy();
}
//..............................................................................
void TMainForm::macReload(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( !Cmds[0].Comparei("macro") && TEFile::FileExists(FXApp->BaseDir() + "macro.xld") )  {
    TStrList SL;
    FMacroFile.LoadFromXLFile(FXApp->BaseDir() + "macro.xld", &SL);
    FMacroItem = FMacroFile.Root().FindItem("xl_macro");
    FMacroFile.Include(&SL);
    TBasicApp::GetLog() << (SL);
  }
  if( !Cmds[0].Comparei("macro") && TEFile::FileExists(FXApp->BaseDir() + "help.xld") )  {
    TStrList SL;
    FHelpFile.LoadFromXLFile(FXApp->BaseDir() + "help.xld", &SL);
    FHelpItem = FHelpFile.Root().FindItem("xl_help");
    TBasicApp::GetLog() << (SL);
  }
}
//..............................................................................
void TMainForm::funSG(const TStrObjList &Cmds, TMacroError &E)  {
  TSpaceGroup * sg = TSymmLib::GetInstance()->FindSG( FXApp->XFile().GetAsymmUnit() );
  if( sg != NULL )  {
    olxstr Tmp;
    if( Cmds.IsEmpty() )  {
      Tmp = sg->GetName();
      if( !sg->GetFullName().IsEmpty() )  {
        Tmp << " (" << sg->GetFullName() << ')';
      }
      Tmp << " #" << sg->GetNumber();
    }
    else  {
      Tmp = Cmds[0];
      Tmp.Replace("%#", olxstr(sg->GetNumber()) );
      Tmp.Replace("%n", sg->GetName());
      Tmp.Replace("%N", sg->GetFullName());
      Tmp.Replace("%H", sg->GetHallSymbol());
      if( Tmp.IndexOf("%h") != -1 )  {
        olxstr t = sg->GetFullName(), res;
        res.SetCapacity( t.Length() + 20 );
        for( int i=0; i < t.Length(); i++ )  {
          if( (i+1) < t.Length() )  {
            if( (t[i] >= '0' && t[i] <= '9')  &&  (t[i+1] >= '0' && t[i+1] <= '9') )  {
              res << t[i] << "<sub>" << t[i+1] << "</sub>";
              i++;
              continue;
            }
          }
          res << t[i];
        }
        Tmp.Replace("%h", res);
      }
    }
    E.SetRetVal( Tmp );
  }
  else  {
    E.ProcessingError(__OlxSrcInfo, "could not find space group for the file" );
    return;
  }
}
//..............................................................................
void TMainForm::macSelBack(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  FXApp->RestoreSelection();
}
//..............................................................................
void TMainForm::macStoreParam(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  int ind = StoredParams.IndexOfComparable(Cmds[0]);
  if( ind == -1 )
    StoredParams.Add( Cmds[0], Cmds[1] );
  else
    StoredParams.Object(ind) = Cmds[1];
  if( Cmds.Count() == 3 && Cmds[2].ToBool() )
    SaveSettings(DataDir + FLastSettingsFile);
}
//..............................................................................
void TMainForm::macCreateBitmap(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  wxFSFile* inf = TFileHandlerManager::GetFSFileHandler( Cmds[1] );
  if( inf == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "Image file does not exist: ") << Cmds[1];
    return;
  }
  wxImage img( *inf->GetStream() );
  delete inf;
  if( !img.Ok() )  {
    E.ProcessingError(__OlxSrcInfo, "Invalid image file: ") << Cmds[1];
    return;
  }
  bool resize = true;
  for( int i=0; i < Options.Count(); i++ )  {
    if( Options.GetName(i)[0] == 'r' )  resize = false;
  }

  int owidth = img.GetWidth(), oheight = img.GetHeight();
  int l = CalcL( img.GetWidth() );
  int swidth = (int)pow((double)2, (double)l);
  l = CalcL( img.GetHeight() );
  int sheight = (int)pow((double)2, (double)l);

  if( swidth != owidth || sheight != oheight )
    img.Rescale( swidth, sheight );

  int cl = 3, bmpType = GL_RGB;
  if( img.HasAlpha() )  {
    cl ++;
    bmpType = GL_RGBA;
  }

  unsigned char* RGBData = new unsigned char[ swidth * sheight * cl];
  for( int i=0; i < sheight; i++ )  {
    for( int j=0; j < swidth; j++ )  {
      int indexa = (i*swidth + (swidth-j-1)) * cl;
      RGBData[indexa] = img.GetRed(j, i);
      RGBData[indexa+1] = img.GetGreen(j, i);
      RGBData[indexa+2] = img.GetBlue(j, i);
      if( cl == 4 )
        RGBData[indexa+3] = img.GetAlpha(j, i);
    }
  }
  bool Created = (FXApp->FindGlBitmap(Cmds[0]) == NULL);

  TGlBitmap* glB = FXApp->CreateGlBitmap( Cmds[0], 0, 0, swidth, sheight, RGBData, bmpType);
  delete [] RGBData;

  int Top = FInfoBox->GetTop() + FInfoBox->GetHeight();
  if( Created )  {
    for( int i=0; i < FXApp->GlBitmapCount(); i++ )
    {
      TGlBitmap& b = FXApp->GlBitmap(i);
      if( &b == glB )  continue;
      Top += (b.GetHeight() + 2);
    }
  }

  if( Created ) {
    glB->SetWidth( owidth );
    glB->SetHeight( oheight );
  }
  glB->SetTop( Top );
  if( resize && Created ) {
    double r = ((double)FXApp->GetRender().GetWidth()/(double)owidth)/10.0;
    glB->Basis.Reset();
    glB->Basis.SetZoom(r);
  }
  glB->SetLeft( FXApp->GetRender().GetWidth() - glB->GetWidth() );
  FXApp->Draw();
}
//..............................................................................
void TMainForm::macDeleteBitmap(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  FXApp->DeleteGlBitmap( Cmds[0] );
}
//..............................................................................
void TMainForm::ChangeSolution( int sol )  {
  if( !Lst.PattSolutionCount() )  {
    if( !Solutions.Count() )  return;

    if( sol < 0 )  sol = Solutions.Count()-1;
    if( sol >= Solutions.Count() )  sol = 0;
    olxstr cf = olxstr(SolutionFolder) << Solutions[sol] << ".res";
    TEFile::Copy( cf, TEFile::ChangeFileExt(FXApp->XFile().GetFileName(), "res"), true);
    TEFile::Copy( TEFile::ChangeFileExt(cf, "lst"),
                  TEFile::ChangeFileExt(FXApp->XFile().GetFileName(), "lst"), true);
    FXApp->LoadXFile( TEFile::ChangeFileExt(FXApp->XFile().GetFileName(), "res") );
    CurrentSolution = sol;
    TBasicApp::GetLog() << ( olxstr("Current solution with try #") << Solutions[sol] << '\n' );
    FXApp->Draw();
  }
  else  {
    if( sol < 0 )  sol = Lst.PattSolutionCount()-1;
    if( sol >= Lst.PattSolutionCount() )  sol = 0;
    TIns *Ins = (TIns*)FXApp->XFile().GetLastLoader();
    Ins->SavePattSolution( TEFile::ChangeFileExt(FXApp->XFile().GetFileName(), "res"),
      Lst.PattSolution(sol), olxstr("Solution #") << (sol+1) );

    FXApp->LoadXFile( TEFile::ChangeFileExt(FXApp->XFile().GetFileName(), "res") );
    CurrentSolution = sol;
    TBasicApp::GetLog() << ( olxstr("Current patt solution #") << (sol+1) << '\n' );
    FXApp->Draw();
  }
}
//..............................................................................
void TMainForm::macTref(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( !Lst.TrefTryCount() )  {
    E.ProcessingError(__OlxSrcInfo, "lst file does not contain tries");
    return;
  }
  Solutions.Clear();
  CurrentSolution = -1;
  FMode |= mSolve;
  SolutionFolder = EmptyString;
  TIns *Ins = (TIns*)FXApp->XFile().GetLastLoader();

  int reps = Cmds[0].ToInt();
  for( int i=0; i < Lst.TrefTryCount(); i++ )  {
    if( i > 0 )  {
      if( Lst.TrefTry(i-1).CFOM == Lst.TrefTry(i).CFOM &&
        Lst.TrefTry(i-1).Semivariants == Lst.TrefTry(i).Semivariants &&
        Lst.TrefTry(i-1).NQual == Lst.TrefTry(i).NQual )
      continue;
    }
    Solutions.AddACopy( Lst.TrefTry(i).Try );
    reps --;
    if( reps <=0 )  break;
  }
  SolutionFolder = TEFile::ExtractFilePath(FXApp->XFile().GetFileName() );
  TEFile::AddTrailingBackslashI( SolutionFolder ) << "olex_sol\\";
  if( !TEFile::FileExists( SolutionFolder ) )
    TEFile::MakeDir( SolutionFolder );
  olxstr cinsFN = TEFile::ChangeFileExt( FXApp->XFile().GetFileName(), "ins" );
  olxstr cresFN = TEFile::ChangeFileExt( FXApp->XFile().GetFileName(), "res" );
  olxstr clstFN = TEFile::ChangeFileExt( FXApp->XFile().GetFileName(), "lst" );
  for( int i=0; i < Solutions.Count(); i++ )  {
    Ins->SaveToRefine( cinsFN, olxstr("TREF -") << Solutions[i], EmptyString);
    FXApp->LoadXFile( cinsFN );
    ProcessXPMacro( "solve", E );
    while( FProcess )  {
      FParent->Dispatch();
      //FTimer->OnTimer->Execute((AActionHandler*)this, NULL);
    }
    TEFile::Copy( cresFN, olxstr(SolutionFolder) <<  Solutions[i] << ".res" );
    TEFile::Copy( clstFN, olxstr(SolutionFolder) <<  Solutions[i] << ".lst" );
  }
  ChangeSolution(0);
//  ProcessXPMacro( olxstr("reap \'") << currentFile << '\'', E);
}
//..............................................................................
void TMainForm::macPatt(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {

  if( !Lst.PattSolutionCount() )  {
    E.ProcessingError(__OlxSrcInfo, "lst file does not contain Patterson solutions");
    return;
  }
  Solutions.Clear();
  CurrentSolution = -1;
  FMode |= mSolve;
  SolutionFolder = EmptyString;
//  TIns *Ins = (TIns*)FXApp->XFile().GetLastLoader();

}
//..............................................................................
void TMainForm::macExport(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  olxstr exName;
  if( Cmds.Count() != 0 )
    exName = Cmds[0];
  else
    exName = TEFile::ChangeFileExt( FXApp->XFile().GetFileName(), "hkl" );

  if( TEFile::FileExists(exName) )  {
    E.ProcessingError(__OlxSrcInfo, "the hkl file already exists");
    return;
  }

  TCif* C = (TCif*)FXApp->XFile().GetLastLoader();
  TCifLoop* hklLoop = C->FindLoop("_refln");
  if( hklLoop == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "no hkl loop found");
    return;
  }
  int hInd = hklLoop->Table().ColIndex("_refln_index_h");
  int kInd = hklLoop->Table().ColIndex("_refln_index_k");
  int lInd = hklLoop->Table().ColIndex("_refln_index_l");
  int mInd = hklLoop->Table().ColIndex("_refln_F_squared_meas");
  int sInd = hklLoop->Table().ColIndex("_refln_F_squared_sigma");

  if( hInd == -1 || kInd == -1 || lInd == -1 || mInd == -1 || sInd == -1 ) {
    E.ProcessingError(__OlxSrcInfo, "could not locate <h k l meas sigma> data");
    return;
  }

  THklFile file;
  for( int i=0; i < hklLoop->Table().RowCount(); i++ )  {
    TReflection* r = new TReflection( hklLoop->Table()[i][hInd].ToInt(),
                                      hklLoop->Table()[i][kInd].ToInt(),
                                      hklLoop->Table()[i][lInd].ToInt(),
                                      hklLoop->Table()[i][mInd].ToDouble(),
                                      hklLoop->Table()[i][sInd].ToDouble() );
    file.Append( *r );
  }
  file.SaveToFile( exName );
}
//..............................................................................
void TMainForm::macFixHL(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  FUndoStack->Push( FXApp->FixHL() );
}
//..............................................................................
void TMainForm::funAlert(const TStrObjList& Params, TMacroError &E) {
  olxstr msg( Params[1] );
  msg.Replace("\\n", "\n");
  if( Params.Count() == 2 )  {
    E.SetRetVal( TdlgMsgBox::Execute(this, msg, Params[0]) );
  }
  else if( Params.Count() == 3 || Params.Count() == 4 )  {
    int flags = 0;
    bool showCheckBox=false;
    for( int i=0; i < Params[2].Length(); i++ )  {
      if( Params[2].CharAt(i) == 'Y' )  flags |= wxYES;
      else if( Params[2].CharAt(i) == 'N' )  flags |= wxNO;
      else if( Params[2].CharAt(i) == 'C' )  flags |= wxCANCEL;
      else if( Params[2].CharAt(i) == 'O' )  flags |= wxOK;
      else if( Params[2].CharAt(i) == 'X' )  flags |= wxICON_EXCLAMATION;
      else if( Params[2].CharAt(i) == 'H' )  flags |= wxICON_HAND;
      else if( Params[2].CharAt(i) == 'E' )  flags |= wxICON_ERROR;
      else if( Params[2].CharAt(i) == 'I' )  flags |= wxICON_INFORMATION;
      else if( Params[2].CharAt(i) == 'Q' )  flags |= wxICON_QUESTION;
      else if( Params[2].CharAt(i) == 'R' )  showCheckBox = true;
    }
    olxstr tickBoxMsg(EmptyString);
    if( Params.Count() == 4 )
      tickBoxMsg = Params[3];
    E.SetRetVal( TdlgMsgBox::Execute(this, msg, Params[0], tickBoxMsg, flags, showCheckBox) );
  }
  FGlCanvas->SetFocus();
}
//..............................................................................
void TMainForm::macSGInfo(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( !Cmds.Count() )  {
    TPtrList<TSpaceGroup> sgList;
    for(int i=0; i < TSymmLib::GetInstance()->BravaisLatticeCount(); i++ )  {
      TBravaisLattice& bl = TSymmLib::GetInstance()->GetBravaisLattice(i);
      bl.FindSpaceGroups( sgList );
      TBasicApp::GetLog() << (olxstr("------------------- ") << bl.GetName() << " --- "  << sgList.Count() << '\n' );
      olxstr tmp, tmp1;
      TPSTypeList<int, TSpaceGroup*> SortedSG;
      for( int j=0; j < sgList.Count(); j++ )
        SortedSG.Add( sgList[j]->GetNumber(), sgList[j] );
      for( int j=0; j < SortedSG.Count(); j++ )  {
        tmp1 << SortedSG.GetObject(j)->GetName() << "(#" << SortedSG.GetComparable(j) << ')';
        tmp <<tmp1.Format(15, true, ' ');
        tmp1 = EmptyString;
        if( tmp.Length() > 60 )  {
          TBasicApp::GetLog() << ( tmp << '\n' );
          tmp = EmptyString;
        }
      }
      TBasicApp::GetLog() << ( tmp << '\n' );
      sgList.Clear();
    }
    return;
  }
  bool Identity = true, Centering = true;
  for( int i=0; i < Options.Count(); i++ )
  {
    if( Options.GetName(i)[0] == 'c' )  Centering = false;
    else
      if( Options.GetName(i)[0] == 'i' )  Identity = false;
  }
  TSpaceGroup* sg = TSymmLib::GetInstance()->FindGroup( Cmds[0] );
  bool LaueClassPG = false;
  if( !sg )  {
    sg = TSymmLib::GetInstance()->FindGroup( olxstr("P") << Cmds[0] );
    if( !sg )  {
      E.ProcessingError(__OlxSrcInfo, "Could not find specified space group/Laue class/Point group: ") << Cmds[0];
      return;
    }
    LaueClassPG = true;
  }
  if( LaueClassPG )  {
    TPtrList<TSpaceGroup> sgList;
    TPSTypeList<int, TSpaceGroup*> SortedSG;
    if( &sg->GetLaueClass() == sg )  {
      TBasicApp::GetLog() << ( olxstr("Space groups of the Laue class ") << sg->GetBareName() << '\n');
      TSymmLib::GetInstance()->FindLaueClassGroups( *sg, sgList);
      for( int j=0; j < sgList.Count(); j++ )
        SortedSG.Add( sgList[j]->GetNumber(), sgList[j] );
      olxstr tmp, tmp1;
      for( int j=0; j < SortedSG.Count(); j++ )  {
        tmp1 << SortedSG.GetObject(j)->GetName() << "(#" << SortedSG.GetComparable(j) << ')';
        tmp << tmp1.Format(15, true, ' ');
        tmp1 = EmptyString;
        if( tmp.Length() > 60 )  {
          TBasicApp::GetLog() << ( tmp << '\n' );
          tmp = EmptyString;
        }
      }
      TBasicApp::GetLog() << ( tmp << '\n' );
    }
    if( &sg->GetPointGroup() == sg )  {
      sgList.Clear();
      SortedSG.Clear();
      olxstr tmp, tmp1;
      TBasicApp::GetLog() << ( olxstr("Space groups of the point group ") << sg->GetBareName() << '\n');
      TSymmLib::GetInstance()->FindPointGroupGroups( *sg, sgList);
      TPSTypeList<int, TSpaceGroup*> SortedSG;
      for( int j=0; j < sgList.Count(); j++ )
        SortedSG.Add( sgList[j]->GetNumber(), sgList[j] );
      for( int j=0; j < SortedSG.Count(); j++ )  {
        tmp1 << SortedSG.GetObject(j)->GetName() << "(#" << SortedSG.GetComparable(j) << ')';
        tmp << tmp1.Format(15, true, ' ');
        tmp1 = EmptyString;
        if( tmp.Length() > 60 )  {
          TBasicApp::GetLog() << ( tmp << '\n' );
          tmp = EmptyString;
        }
      }
      TBasicApp::GetLog() << ( tmp << '\n' );
    }
    return;
  }
  TPtrList<TSpaceGroup> AllGroups;
  TMatrixDList SGMatrices;

  TBasicApp::GetLog() << ( sg->IsCentrosymmetric() ? "Centrosymmetric" : "Non centrocymmetric") << '\n';
  TBasicApp::GetLog() << ( olxstr("Hall symbol: ") << sg->GetHallSymbol() << '\n');

  TSymmLib::GetInstance()->GetGroupByNumber( sg->GetNumber(), AllGroups );
  if( AllGroups.Count() > 1 )  {
    TBasicApp::GetLog() << ("Alternative settings:\n");
    olxstr tmp;
    for( int i=0; i < AllGroups.Count(); i++ )  {
      if( AllGroups[i] == sg )  continue;
      tmp << AllGroups[i]->GetName() << '(' << AllGroups[i]->GetFullName() <<  ") ";
    }
    TBasicApp::GetLog() << (tmp << '\n');
  }
  TBasicApp::GetLog() << ( olxstr("Space group number: ") << sg->GetNumber() << '\n');
  TBasicApp::GetLog() << ( olxstr("Crystal system: ") << sg->GetBravaisLattice().GetName() << '\n');
  TBasicApp::GetLog() << ( olxstr("Laue class: ") << sg->GetLaueClass().GetBareName() << '\n');
  TBasicApp::GetLog() << ( olxstr("Point group: ") << sg->GetPointGroup().GetBareName() << '\n');
  short Flags = mattAll;
  if( !Centering )  Flags ^= (mattCentering|mattTranslation);
  if( !Identity )  Flags ^= mattIdentity;
  sg->GetMatrices( SGMatrices, Flags );

  TTTable<TStrList> tab( SGMatrices.Count(), 2 );

  for( int i=0; i < SGMatrices.Count(); i++ )
    tab.Row(i)->String(0) = TSymmParser::MatrixToSymm( SGMatrices[i] );

  for( int i=0; i < TSymmLib::GetInstance()->SymmElementCount(); i++ )  {
    TSymmElement& se = TSymmLib::GetInstance()->GetSymmElement(i);
    for( int j=0; j < SGMatrices.Count(); j++ )  SGMatrices[j].SetTag(0);
    if( TSpaceGroup::ContainsElement( SGMatrices, &se ) )  {
      for( int j=0; j < SGMatrices.Count(); j++ )  {
        if( SGMatrices[j].GetTag() != 0 )  {
          if( tab.Row(j)->String(1).Length() )  tab.Row(j)->String(1) << ", ";
          tab.Row(j)->String(1) << TSymmLib::GetInstance()->GetSymmElement(i).GetName();
        }
      }
    }
  }
  TStrList Output;
  tab.CreateTXTList(Output, EmptyString, true, true, ' ');
  TBasicApp::GetLog() << ( Output );
}
//..............................................................................
void TMainForm::macAddLabel(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  double x = 0, 
         y = 0, 
         z = 0;
  olxstr name, label;
  if( Cmds.Count() == 3 )  {
    name = Cmds[0];
    label = Cmds[2];
    TStrList toks;
    toks.Strtok( Cmds[1], ' ');
    if( toks.Count() == 3 )  {
      x = toks[0].ToDouble();
      y = toks[1].ToDouble();
      z = toks[2].ToDouble();
    }
  }
  else  if( Cmds.Count() == 5 ) {
    name = Cmds[0];
    x = Cmds[1].ToDouble();
    y = Cmds[2].ToDouble();
    z = Cmds[3].ToDouble();
    label = Cmds[4];
  }
  FXApp->AddLabel( name, 
                   TVPointD(x, y, z),
                   label);
}
//..............................................................................
//
  class TOnSync : public AActionHandler  {
    TGXApp* xa;
    olxstr BaseDir;
    public:
      TOnSync(TGXApp& xapp, const olxstr baseDir)  {
        xa = & xapp;
        BaseDir = baseDir;
      }
      bool Execute(const IEObject *Sender, const IEObject *Data)  {
        if( !EsdlInstanceOf(*Data, olxstr) )  return false;
        olxstr cpath = olxstr::CommonString(BaseDir, *(const olxstr*)Data);
        TBasicApp::GetLog() << ( olxstr("\rDownloading /~/") << ((olxstr*)Data)->SubStringFrom(cpath.Length()) );
        xa->Draw();
        wxTheApp->Dispatch();
        return true;
      }
  };
class TDownloadProgress: public AActionHandler  {
    TGXApp* xa;
public:
  TDownloadProgress(TGXApp& xapp) : xa(&xapp) {  }
  bool Enter(const IEObject *Sender, const IEObject *Data)  {
    if( Data != NULL && EsdlInstanceOf(*Data, TOnProgress) )
      TBasicApp::GetLog() <<  ((TOnProgress*)Data)->GetAction() << '\n';
    return true;
  }
  bool Exit(const IEObject *Sender, const IEObject *Data)  {
    TBasicApp::GetLog() <<  "\nDone\n";
    return true;
  }
  bool Execute(const IEObject *Sender, const IEObject *Data)  {
    if( !EsdlInstanceOf(*Data, TOnProgress) )
      return false;
    IEObject* p_d = const_cast<IEObject*>(Data);
    TOnProgress *A = dynamic_cast<TOnProgress*>(p_d);
    if( A->GetPos() <= 0 )  return false;
    if( A->GetMax() <= 0 )
      TBasicApp::GetLog() <<  (olxstr("\r") << A->GetPos()/1024 << "Kb");
    else
      TBasicApp::GetLog() <<  (olxstr("\r") << A->GetPos()*100/A->GetMax() << '%');
    xa->Draw();
    wxTheApp->Dispatch();
    return true;
  }
};
//
void TMainForm::macInstallPlugin(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( !FPluginItem->ItemExists(Cmds[0]) )  {
    TStateChange sc(prsPluginInstalled, true);
    olxstr local_file( Options.FindValue("l", EmptyString) );
    if( !local_file.IsEmpty() )  {
      if( !TEFile::FileExists(local_file) )  {
        E.ProcessingError(__OlxSrcInfo, "cannot find plugin archive");
        return;
      }
      TwxZipFileSystem zipFS( local_file, false );
      TOSFileSystem osFS;
      osFS.SetBase( TBasicApp::GetInstance()->BaseDir() );
      TFSIndex fsIndex( zipFS );
      TStrList properties;
      properties.Add(Cmds[0]);
      TOnSync* progressListener = new TOnSync(*FXApp, TBasicApp::GetInstance()->BaseDir() );
      osFS.OnAdoptFile->Add( progressListener );

      IEObject* Cause = NULL;
      try  {  fsIndex.Synchronise(osFS, properties);  }
      catch( const TExceptionBase& exc )  {
        Cause = exc.Replicate();
      }
      osFS.OnAdoptFile->Remove( progressListener );
      delete progressListener;
      if( Cause )
        throw TFunctionFailedException(__OlxSourceInfo, Cause);

      FPluginItem->AddItem( Cmds[0] );
      OnStateChange->Execute((AEventsDispatcher*)this, &sc);

      FPluginFile.SaveToXLFile( PluginFile );
      TBasicApp::GetLog() << ("Installation complete\n");
      FXApp->Draw();
    }
    else  {
      olxstr SettingsFile( TBasicApp::GetInstance()->BaseDir() + "usettings.dat" );
      TSettingsFile settings;
      if( TEFile::FileExists(SettingsFile) )  {
        olxstr Proxy, Repository;

        settings.LoadSettings( SettingsFile );
        if( settings.ParamExists("proxy") )        Proxy = settings.ParamValue("proxy");
        if( settings.ParamExists("repository") )   Repository = settings.ParamValue("repository");

        if( Repository.Length() && !Repository.EndsWith('/') )  Repository << '/';

        TUrl url(Repository);
        if( !Proxy.IsEmpty() )  url.SetProxy( TUrl(Proxy) );
        TwxHttpFileSystem httpFS( url );
        // plugin- = 7
        olxstr zip_fn( olxstr("/") << TEFile::UnixPath(TEFile::ParentDir(url.GetPath())) <<
          Cmds[0].SubStringFrom(7) << ".zip" );
        olxstr local_fn;
        TDownloadProgress* dp = new TDownloadProgress(*FXApp);
        TBasicApp::GetInstance()->OnProgress->Add( dp );
        try  { local_fn = httpFS.SaveFile( zip_fn );  }
        catch( ... )  {  }
        TBasicApp::GetInstance()->OnProgress->Remove(dp);
        delete dp;
        if( !local_fn.IsEmpty() )  {
          httpFS.SetZipFS( new TwxZipFileSystem(local_fn, false) );
        }
        TOSFileSystem osFS;
        osFS.SetBase( TBasicApp::GetInstance()->BaseDir() );
        TFSIndex fsIndex( httpFS );
        TStrList properties;
        properties.Add(Cmds[0]);
        TOnSync* progressListener = new TOnSync(*FXApp, TBasicApp::GetInstance()->BaseDir() );
        osFS.OnAdoptFile->Add( progressListener );

        IEObject* Cause = NULL;
        try  {  fsIndex.Synchronise(osFS, properties);  }
        catch( const TExceptionBase& exc )  {
          Cause = exc.Replicate();
        }
        osFS.OnAdoptFile->Remove( progressListener );
        delete progressListener;
        if( Cause )
          throw TFunctionFailedException(__OlxSourceInfo, Cause);

        FPluginItem->AddItem( Cmds[0] );
        OnStateChange->Execute((AEventsDispatcher*)this, &sc);

        FPluginFile.SaveToXLFile( PluginFile );
        TBasicApp::GetLog() << ("\rInstallation complete\n");
        FXApp->Draw();
      }
      else  {
        TBasicApp::GetLog() << ("Could not locate usettings.dat file\n");
      }
    }
  }
  else  {
    TDataItem* di = FPluginItem->FindItem( Cmds[0] );
    if( di != NULL )  {
      TStateChange sc(prsPluginInstalled, false);
      OnStateChange->Execute((AEventsDispatcher*)this, &sc);
      ProcessXPMacro( olxstr("uninstallplugin ") << Cmds[0], E );
    }
    else  {
      TBasicApp::GetLog() << ( olxstr("Specified plugin does not exist: ") << Cmds[0] << '\n' );
    }
  }
}
//..............................................................................
void TMainForm::macSignPlugin(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TDataItem* di = FPluginItem->FindItem( Cmds[0] );
  if( di )  di->AddField("signature", Cmds[1] );
  FPluginFile.SaveToXLFile( PluginFile );
}
//..............................................................................
// local linkage (for classes within functions) is not supported by Borland, though MSVC is fine
  class TFSTraverser {
    olxstr Property, BaseDir;
    TGXApp* xa;
    TPtrList<TFSItem> ToDelete;
    public:
      TFSTraverser(TGXApp& xapp, const olxstr& baseDir, const olxstr& prop)  {
        Property = prop;
        BaseDir = baseDir;
        xa = &xapp;
      }
      ~TFSTraverser()  {
        TPtrList<TFSItem> FoldersToDelete;
        for( int i=0; i < ToDelete.Count(); i++ )
          if( !ToDelete[i]->IsFolder() )  {
            if( FoldersToDelete.IndexOf( ToDelete[i]->GetParent() ) == -1 )
              FoldersToDelete.Add( ToDelete[i]->GetParent() );
            ToDelete[i]->GetParent()->Remove( *ToDelete[i] );
            ToDelete.Delete(i);
            i--;
          }
        while( true )  {
          bool deleted = false;
          for( int i=0; i < FoldersToDelete.Count(); i++ )  {
            if( FoldersToDelete[i]->IsEmpty() )  {
              olxstr path = FoldersToDelete[i]->GetFileSystem().GetBase() + FoldersToDelete[i]->GetFullName(),
              cpath = path.CommonString(path, BaseDir);
              TBasicApp::GetLog() << ( olxstr("\rDeleting folder /~/") << path.SubStringFrom(cpath.Length()) );
              xa->Draw();
              wxTheApp->Dispatch();
              TEFile::DelDir( path );
              deleted = true;
              FoldersToDelete[i]->GetParent()->Remove( *FoldersToDelete[i] );
              FoldersToDelete.Delete(i);
              i--;
            }
          }
          if( !deleted )  break;
        }
      }
      bool OnItem(TFSItem& it) {
        if( it.HasProperty(Property) )  {
          olxstr path = it.GetFileSystem().GetBase() + it.GetFullName(),
                   cpath = path.CommonString(path, BaseDir);
          TBasicApp::GetLog() << ( olxstr("\rDeleting /~/") << path.SubStringFrom(cpath.Length()) );
          xa->Draw();
          wxTheApp->Dispatch();
          TEFile::DelFile( path );
          ToDelete.Add( &it );
        }
        return true;
      }
  };
void TMainForm::macUninstallPlugin(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Cmds[0].StartsFrom("olex") )  {
    E.ProcessingError(__OlxSrcInfo, "cannot uninstall core components");
    return;
  }
  TDataItem* di = FPluginItem->FindItem( Cmds[0] );
  if( di != NULL )  {
    TStateChange sc(prsPluginInstalled, false);
    FPluginItem->DeleteItem( di );
    OnStateChange->Execute((AEventsDispatcher*)this, &sc);
    olxstr indexFile = TBasicApp::GetInstance()->BaseDir() + "index.ind";
    if( TEFile::FileExists(indexFile) )  {
      TOSFileSystem osFS;
      osFS.SetBase( TBasicApp::GetInstance()->BaseDir() );
      TFSIndex fsIndex( osFS );

      fsIndex.LoadIndex( indexFile );
      TFSTraverser* trav = new TFSTraverser(*FXApp, TBasicApp::GetInstance()->BaseDir(), Cmds[0]);
      TFSItem::Traverser.Traverse<TFSTraverser>(fsIndex.GetRoot(), *trav );
      delete trav;
      fsIndex.SaveIndex( indexFile );
      TBasicApp::GetLog() << "\rUninstallation complete\n";
      FXApp->Draw();
    }
  }
  FPluginFile.SaveToXLFile( PluginFile );
}
//..............................................................................
void TMainForm::funIsPluginInstalled(const TStrObjList& Params, TMacroError &E) {
  E.SetRetVal( FPluginItem->ItemExists(Params[0]) );
}
//..............................................................................
void TMainForm::funValidatePlugin(const TStrObjList& Params, TMacroError &E) {
  E.SetRetVal( true );
}
//..............................................................................
void TMainForm::macUpdateFile(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  
  olxstr SettingsFile( TBasicApp::GetInstance()->BaseDir() + "usettings.dat" );
  TEFile::CheckFileExists( __OlxSourceInfo, SettingsFile );
  TSettingsFile settings;
  olxstr Proxy, Repository;
  bool Force = false;
  for( int i=0; i < Options.Count(); i++ )  {
    if( Options.GetName(i)[0] == 'f' )  {
      Force = true;
      break;
    }
  }

  settings.LoadSettings( SettingsFile );
  if( settings.ParamExists("proxy") )        Proxy = settings.ParamValue("proxy");
  if( settings.ParamExists("repository") )   Repository = settings.ParamValue("repository");

  if( Repository.Length() && !Repository.EndsWith('/') )  Repository << '/';

  TUrl url(Repository);
  if( !Proxy.IsEmpty() ) url.SetProxy( Proxy );

  TwxHttpFileSystem httpFS( url );
  TOSFileSystem osFS;
  osFS.SetBase( TBasicApp::GetInstance()->BaseDir() );
  TFSIndex fsIndex( httpFS );

  IEObject* Cause = NULL;
  try  {
    if( fsIndex.UpdateFile(osFS, Cmds[0], Force) )
      TBasicApp::GetLog() << ( olxstr("Updated '") << Cmds[0] << '\'' << '\n');
    else
      TBasicApp::GetLog() << ( olxstr("Up-to-date '") << Cmds[0] << '\'' << '\n');
  }
  catch( const TExceptionBase& exc )  {  Cause = exc.Replicate();  }
  if( Cause != NULL )
    throw TFunctionFailedException(__OlxSourceInfo, Cause);
}
//..............................................................................
void TMainForm::macNextSolution(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( ((FMode&mSolve) == mSolve) )  {
    ChangeSolution( CurrentSolution - 1 );
    return;
  }
}
//..............................................................................
//..............................................................................
double MatchAtomPairsQT(const TTypeList< AnAssociation2<TSAtom*,TSAtom*> >& atoms,
                        TMatrixD& res, bool InversionPossible, bool& InversionUsed)  {
  if( atoms.Count() < 4 )  return -1;
  double rms = TNetwork::FindAlignmentMatrix(atoms, res, false), rms1;
  InversionUsed = false;
  if( InversionPossible )  {
    TMatrixD lr(3,4);
    rms1 = TNetwork::FindAlignmentMatrix(atoms, lr, true);
    if( (rms1 < rms && rms1 >= 0) || (rms < 0 && rms1 >= 0) )  {
      res = lr;
      InversionUsed = true;
    }
  }
  if( InversionUsed )  {
    TBasicApp::GetLog() << ( olxstr("Summ(distances)/N is ") << olxstr::FormatFloat(3, rms1) <<
                    " with inversion vs." << olxstr::FormatFloat(3, rms) << '\n');
    rms = rms1;
  }
  else
    TBasicApp::GetLog() << ( olxstr("Summ(distances)/N is ") << olxstr::FormatFloat(3, rms) << '\n');
  return rms;
}
//..............................................................................
void MatchAtomPairsULS(const TTypeList< AnAssociation2<TSAtom*,TSAtom*> >& atoms, TMatrixD& res)  {
  TMatrixD gs(4,4), lm(atoms.Count(), 4), lmt(4, atoms.Count());
  TVectorD gr(4), sol(4), b( atoms.Count() );
  res.Resize(3,4);
  for( int i=0; i < atoms.Count(); i++ )  {
    for( int j=0; j < 3; j++ )  {
      lm[i][j] = atoms[i].GetA()->Center()[j];
      lmt[j][i] = lm[i][j];
    }
    lm[i][3] = 1;
    lmt[3][i] = 1;
  }
  for( int i=0; i < 3; i++ )  {
    for( int j=0; j < atoms.Count(); j++ )  {
      b[j] = atoms[j].GetB()->Center()[i];
    }
    gs = lmt * lm;
    gr = lmt * b;
    TMatrixD::GauseSolve(gs, gr, sol);
    res[i] = sol;
    sol.Null();
  }
}
//..............................................................................
void TMainForm::CallMatchCallbacks(TNetwork& netA, TNetwork& netB, double RMS)  {
  olxstr arg;
  TStrObjList callBackArg;
  for( int i=0; i < netA.NodeCount(); i++ )  {
    arg << netA.Node(i).GetLabel();
    if( (i+1) < netA.NodeCount() )  arg << ',';
  }
  callBackArg.Add(RMS);
  callBackArg.Add(arg);
  arg = EmptyString;
  for( int i=0; i < netB.NodeCount(); i++ )  {
    arg << netB.Node(i).GetLabel();
    if( (i+1) < netB.NodeCount() )  arg << ',';
  }
  callBackArg.Add(arg);
  CallbackFunc(OnMatchCBName, callBackArg);
}
//..............................................................................
void TMainForm::macMatch(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  CallbackFunc(StartMatchCBName, EmptyString);
  // ivertion test
  bool TryInvert = Options.Contains("i"), Inverted;
/*
  if( TryInvert )  {
    TSpaceGroup* sg = TSymmLib::GetInstance()->FindSG( FXApp->XFile().GetLastLoader()->GetAsymmUnit() );
    if( sg == NULL )  {
      TBasicApp::GetLog() << ("No space group information is provided");
      TryInvert = false;
    }
    else  {
      TryInvert = sg->IsCentrosymmetric();
    }
  }
*/  
  // end invertion test
  TXAtomPList atoms;
  bool subgraph = Options.Contains("s");
  olxstr suffix = Options.FindValue("n");
  bool name = !suffix.IsEmpty();
  bool align = Options.Contains("a");
  if( Cmds.Count() )  {
    FXApp->FindXAtoms( Cmds.Text(' '), atoms);
    if( atoms.Count() == 2 )  {
      TTypeList< AnAssociation2<int, int> > res;
      TIntList sk;
      TNetwork &netA = atoms[0]->Atom().GetNetwork(),
               &netB = atoms[1]->Atom().GetNetwork();
      bool match = subgraph ? netA.IsSubgraphOf( netB, res, sk ) :
                              netA.DoMatch( netB, res );
      TBasicApp::GetLog() << ( olxstr("Graphs match: ") << match << '\n' );
      if( match )  {
        TTypeList< AnAssociation2<TSAtom*,TSAtom*> > satomp;
        TSAtomPList atomsToTransform;
        TBasicApp::GetLog() << ("Matching pairs:\n");
        olxstr tmp('=');
        for(int i=0; i < res.Count(); i++ )  {
          tmp << '{' << netA.Node( res[i].GetA()).GetLabel() <<
                 ',' << netB.Node( res[i].GetB()).GetLabel() << '}';

          if( atomsToTransform.IndexOf( &netB.Node(res[i].GetB()) ) == -1 )  {
            atomsToTransform.Add( &netB.Node( res[i].GetB()) );
            satomp.AddNew<TSAtom*,TSAtom*>(&netA.Node( res[i].GetA()), &netB.Node( res[i].GetB()));
          }
        }
        if( name )  {
          for(int i=0; i < res.Count(); i++ )
            netB.Node( res[i].GetB()).SetLabel( netA.Node( res[i].GetA()).GetLabel() + suffix );
        }
        TMatrixD S(3,4);
        double rms = MatchAtomPairsQT( satomp, S, false, Inverted);
        TBasicApp::GetLog() << ("Transformation matrix:\n");
        for( int i=0; i < 3; i++ )
          TBasicApp::GetLog() << S[i].ToString() << '\n' ;
        // execute callback function
        CallMatchCallbacks(netA, netB, rms);
        // ends execute callback
        if( align && rms >= 0 )  {
          TNetwork::DoAlignAtoms(satomp, atomsToTransform, S, Inverted);
          FXApp->UpdateBonds();
          FXApp->CenterView();
        }

        TBasicApp::GetLog() << (tmp << '\n');
        if( subgraph )  {
          sk.Add( res[0].GetB() );
          res.Clear();
          while( atoms[0]->Atom().GetNetwork().IsSubgraphOf( atoms[1]->Atom().GetNetwork(), res, sk ) )  {
            sk.Add( res[0].GetB() );
            tmp = '=';
            for(int i=0; i < res.Count(); i++ )  {
              tmp << '{' << atoms[0]->Atom().GetNetwork().Node( res[i].GetA()).GetLabel() <<
                     ',' << atoms[1]->Atom().GetNetwork().Node( res[i].GetB()).GetLabel() << '}';
            }
            TBasicApp::GetLog() << (tmp << '\n');
            res.Clear();
          }
        }
      }
      return;
    }

    if( atoms.Count() >= 8 )  {  // a full basis provided
      if( (atoms.Count()%2) != 0 )  {
        E.ProcessingError(__OlxSrcInfo, "even number of atoms is expected");
        return;
      }
      TTypeList< AnAssociation2<TSAtom*,TSAtom*> > satomp;
      TSAtomPList atomsToTransform;
      for(int i=0; i < atoms.Count()/2; i++ )
        satomp.AddNew<TSAtom*,TSAtom*>( &atoms[i]->Atom(), &atoms[i+atoms.Count()/2]->Atom() );
      TNetwork &netA = satomp[0].GetA()->GetNetwork(),
               &netB = satomp[0].GetB()->GetNetwork();
      for(int i=1; i < satomp.Count(); i++ )  {
        if( satomp[i].GetA()->GetNetwork() != netA ||
            satomp[i].GetB()->GetNetwork() != netB )  {
          if( (atoms.Count()%2) != 0 )  {
            E.ProcessingError(__OlxSrcInfo, "atoms should belong to two distinct fragments or the same fragment");
           return;
          }
        }
      }
      if( netA != netB )  {  // collect all atoms
        for( int i=0; i < netA.NodeCount(); i++ )
          atomsToTransform.Add( &netA.Node(i) );
      }
      else  {
        for(int i=0; i < atoms.Count()/2; i++ )  {
          atomsToTransform.Add( &atoms[i]->Atom() );
        }
      }
      TMatrixD S(3,4);
      double rms = MatchAtomPairsQT( satomp, S, false, Inverted);
      TNetwork::DoAlignAtoms(satomp, atomsToTransform, S, Inverted);
      FXApp->UpdateBonds();
      FXApp->CenterView();

      CallMatchCallbacks(netA, netB, rms);
    }
  }
  else  {
    TNetPList nets;
    FXApp->GetNetworks(nets);
    TTypeList< AnAssociation2<int, int> > res;
    TTypeList< AnAssociation2<TSAtom*,TSAtom*> > satomp;
    TSAtomPList atomsToTransform;
    TMatrixD S(3,4);
    for( int i=0; i < nets.Count(); i++ )  {
      if( i > 0 )  TryInvert = false;
      for( int j=i+1; j < nets.Count(); j++ )  {
        res.Clear();
        if( nets[i]->DoMatch( *nets[j], res ) )  {
          satomp.Clear();
          atomsToTransform.Clear();
          for(int k=0; k < res.Count(); k++ )  {
            if( atomsToTransform.IndexOf( &nets[j]->Node(res[k].GetB()) ) == -1 )  {
              atomsToTransform.Add( &nets[j]->Node( res[k].GetB()) );
              satomp.AddNew<TSAtom*,TSAtom*>(&nets[j]->Node( res[k].GetB()),
                                             &nets[i]->Node( res[k].GetA()));
            }
          }
          double rms = MatchAtomPairsQT( satomp, S, TryInvert, Inverted);
//          double rms = MatchAtomPairsQT( satomp, S, false, Inverted);
          CallMatchCallbacks(*nets[i], *nets[j], rms);
          if( rms >= 0 )
            TNetwork::DoAlignAtoms(satomp, atomsToTransform, S, Inverted);

        }
      }
    }
      
    FXApp->UpdateBonds();
    FXApp->CenterView();
    // do match all possible fragments with similar number of atoms
  }
}
//..............................................................................
void TMainForm::macShowWindow(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Cmds.Count() == 2 )  {
    if( Cmds[0].Comparei("help") == 0 )  {
      HelpWindowVisible = Cmds[1].ToBool();
      FHelpWindow->Visible( HelpWindowVisible );
      TStateChange sc(prsHelpVis, HelpWindowVisible);
      OnStateChange->Execute((AEventsDispatcher*)this, &sc);
    } else  if( Cmds[0].Comparei("info") == 0 )  {
      InfoWindowVisible = Cmds[1].ToBool();
      FInfoBox->Visible( InfoWindowVisible );
      TStateChange sc(prsInfoVis, InfoWindowVisible);
      OnStateChange->Execute((AEventsDispatcher*)this, &sc);
      OnResize();
      FXApp->Draw();
    } else if( Cmds[0].Comparei("cmdline") == 0 )  {
      CmdLineVisible = Cmds[1].ToBool();
      FCmdLine->Show( CmdLineVisible );
      if( CmdLineVisible )  FCmdLine->SetFocus();
      FGlConsole->SetPromptVisible( !CmdLineVisible );
      TStateChange sc(prsCmdlVis, CmdLineVisible);
      OnStateChange->Execute((AEventsDispatcher*)this, &sc);
      OnResize();
      FXApp->Draw();
    }
  }
  else  {
    if( Cmds[0].Comparei("help") == 0 )  {
      HelpWindowVisible = !HelpWindowVisible;
      FHelpWindow->Visible( HelpWindowVisible );
      TStateChange sc(prsHelpVis, HelpWindowVisible);
      OnStateChange->Execute((AEventsDispatcher*)this, &sc);
    } else if( Cmds[0].Comparei("info") == 0 )  {
      InfoWindowVisible = !InfoWindowVisible;
      FInfoBox->Visible( InfoWindowVisible );
      TStateChange sc(prsInfoVis, InfoWindowVisible);
      OnStateChange->Execute((AEventsDispatcher*)this, &sc);
      OnResize();
      FXApp->Draw();
    } else if( Cmds[0].Comparei("cmdline") == 0 )  {
      CmdLineVisible = !CmdLineVisible;
      FCmdLine->Show( CmdLineVisible );
      if( CmdLineVisible )  FCmdLine->SetFocus();
      FGlConsole->SetPromptVisible( !CmdLineVisible );
      TStateChange sc(prsCmdlVis, CmdLineVisible);
      OnStateChange->Execute((AEventsDispatcher*)this, &sc);
      OnResize();
      FXApp->Draw();
    }
  }
}
//..............................................................................
void TMainForm::funGetUserInput(const TStrObjList& Params, TMacroError &E) {
  bool MultiLine = Params[0].ToInt() != 1;

  TdlgEdit *dlg = new TdlgEdit(this, MultiLine);
  dlg->SetTitle( uiStr(Params[1]) );
  dlg->SetText( Params[2] );
  if( dlg->ShowModal() == wxID_OK )
    E.SetRetVal( dlg->GetText() );
  else
    E.SetRetVal( EmptyString );
  dlg->Destroy();
}
//..............................................................................
void TMainForm::funGetCompilationInfo(const TStrObjList& Params, TMacroError &E) {
  E.SetRetVal( olxstr(__DATE__) << ' ' << __TIME__ );
}
//..............................................................................
void TMainForm::macDelOFile(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  int ind = Cmds[0].ToInt();
  if( ind < 0 || ind >= FXApp->OverlayedXFileCount() )
    throw TInvalidArgumentException(__OlxSourceInfo, olxstr("index=") << ind );
  FXApp->DeleteOverlayedXFile( ind );
}
//..............................................................................

class TTetrahedron  {
  TVPointDList Points;
  olxstr Name;
  double Volume;
protected:
  double CalcVolume()  {
    TVPointD a,b,n;
    double d, caS, sa;
    a = Points[1] - Points[0];
    b = Points[2] - Points[0];
    caS = a.CAngle(b);
    sa = sqrt( 1- caS*caS);
    caS = a.Length() * b.Length() * sa / 2;
    n = a.XProdVec(b);
    d = n[2]*Points[0][2] + n[1]*Points[0][1] + n[0]*Points[0][0];
    d = n[2]*Points[3][2] + n[1]*Points[3][1] + n[0]*Points[3][0] - d;
    d /= n.Length();
    return fabs( caS*d/2 );
  }
public:
  TTetrahedron(const olxstr& name)  {
    Name = name;
    Volume = -1;
  }
  void AddPoint( const TVPointD& p )  {
    Points.AddNew( p );
    if( Points.Count() == 4 )
      Volume = CalcVolume();
  }
  const olxstr& GetName() const  {  return Name;  }
  const TVPointD& operator [] (int i)  const  {  return Points[i];  }

  double GetVolume()  const  {  return Volume;  }
};

int ThSort( const TTetrahedron& th1, const TTetrahedron& th2 )  {
  double v = th1.GetVolume() - th2.GetVolume();
  if( v < 0 )  return -1;
  if( v > 0 )  return 1;
  return 0;
}

void TMainForm::macCalcVol(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {

  TXAtom* xa = FXApp->GetXAtom( Cmds[0], !Options.Contains("cs") );
  if( xa == NULL )  {
    Error.ProcessingError(__OlxSrcInfo, "no atom provided");
    return;
  }
  TSAtomPList atoms;
  for( int i=0; i < xa->Atom().NodeCount(); i++ ) {
    TSAtom& A = xa->Atom().Node(i);
    if( A.IsDeleted() || (A.GetAtomInfo() == iQPeakIndex ) )
      continue;
    atoms.Add(&A);
  }
  if( atoms.Count() < 4 )  {
    Error.ProcessingError(__OlxSrcInfo, "an atom with at least four bonds is expected");
    return;
  }
  TTypeList<TTetrahedron> tetrahedra;
  // special case for 4 nodes
  if( atoms.Count() == 4 )  {
    TTetrahedron& th = tetrahedra.AddNew( olxstr(atoms[0]->GetLabel() ) << '-'
      << atoms[1]->GetLabel() << '-'
      << atoms[2]->GetLabel() << '-'
      << atoms[3]->GetLabel()               );
    th.AddPoint( atoms[0]->Center() );
    th.AddPoint( atoms[1]->Center() );
    th.AddPoint( atoms[2]->Center() );
    th.AddPoint( atoms[3]->Center() );
  }
  else  {
    for( int i=0; i < atoms.Count(); i++ ) {
      for( int j=i+1; j < atoms.Count(); j++ ) {
        for( int k=j+1; k < atoms.Count(); k++ ) {
          TTetrahedron& th = tetrahedra.AddNew( olxstr(xa->Atom().GetLabel() ) << '-'
                 << atoms[i]->GetLabel() << '-'
                 << atoms[j]->GetLabel() << '-'
                 << atoms[k]->GetLabel()               );
          th.AddPoint( xa->Atom().Center() );
          th.AddPoint( atoms[i]->Center() );
          th.AddPoint( atoms[j]->Center() );
          th.AddPoint( atoms[k]->Center() );
        }
      }
    }
  }
  int thc = (atoms.Count()-2)*2;

  TTypeList<TTetrahedron>::QuickSorter.SortSF( tetrahedra, &ThSort );
  bool removed = false;
  while(  tetrahedra.Count() > thc )  {
    TBasicApp::GetLog() << ( olxstr("Removing tetrahedra ") <<  tetrahedra[0].GetName() << " with volume " << tetrahedra[0].GetVolume() << '\n' );
    tetrahedra.Delete(0);
    removed = true;
  }
  double v = 0;
  for( int i=0; i < tetrahedra.Count(); i++ )
    v += tetrahedra[i].GetVolume();

  if( removed )
    TBasicApp::GetLog() << ( olxstr("The volume for remaining tetrahedra is ") << olxstr::FormatFloat(3,v) << '\n' );
  else
    TBasicApp::GetLog() << ( olxstr("The tetrahedra volume is ") << olxstr::FormatFloat(3,v) << '\n' );
}
//..............................................................................
void TMainForm::macChangeLanguage(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  Dictionary.SetCurrentLanguage(DictionaryFile, Cmds[0] );
}
//..............................................................................
void TMainForm::funTranslatePhrase(const TStrObjList& Params, TMacroError &E) {
  E.SetRetVal( TranslatePhrase(Params[0]) );
}
//..............................................................................
void TMainForm::funCurrentLanguageEncoding(const TStrObjList& Params, TMacroError &E) {
  E.SetRetVal( Dictionary.GetCurrentLanguageEncodingStr() );
}
//..............................................................................
void TMainForm::funIsCurrentLanguage(const TStrObjList& Params, TMacroError &E) {
  TStrList toks;
  toks.Strtok(Params[0], ';');
  for( int i=0; i < toks.Count(); i++ )  {
    if( toks[i] == Dictionary.GetCurrentLanguage() )  {
      E.SetRetVal( true );
      return;
    }
  }
  E.SetRetVal( false );
}
//..............................................................................
void TMainForm::macHAdd(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TIns *Ins = (TIns*)FXApp->XFile().GetLastLoader();
  TSAtomPList satoms;
  TXAtomPList xatoms;
  if( !FindXAtoms(Cmds, xatoms, true, !Options.Contains("cs")) )  {
    Error.ProcessingError(__OlxSrcInfo, "wrong atom names" );
    return;
  }
  TListCaster::POP(xatoms, satoms);
  TXlConGen xlcg( Ins );
  FXApp->XFile().GetLattice().AnalyseHAdd( xlcg, satoms );
  FXApp->XFile().EndUpdate();
  FXApp->XFile().UpdateAsymmUnit();
  FUndoStack->Push( FXApp->FixHL() );
  FUndoStack->Clear();
}
//..............................................................................
void TMainForm::macHklStat(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  olxstr hklSrc = FXApp->XFile().GetLastLoader()->GetHKLSource();
  if( hklSrc.IsEmpty() )
    hklSrc = TEFile::ChangeFileExt( FXApp->XFile().GetLastLoader()->GetFileName(), "hkl");
  if( !TEFile::FileExists( hklSrc ) )  {
    Error.ProcessingError(__OlxSrcInfo, "could not find hkl file: ") << hklSrc;
    return;
  }

  bool list = false;
  for( int i=0; i < Options.Count(); i++ )
    if( Options.GetName(i)[0] == 'l' )  {  list = true;  continue;  }

  TTypeList<olxstr> Cons;
  olxstr Tmp, strV;
  for( int i=0; i < Cmds.Count(); i++ )
    Cons.AddNew( Cmds[i] );

  THklFile Hkl;
  Hkl.LoadFromFile( hklSrc );
  TVectorDList con;

  for( int i=0; i < Cons.Count(); i++ )  {
    int obi = Cons[i].FirstIndexOf('[');
    if( obi == -1 || !Cons[i].EndsWith(']') )  {
      Error.ProcessingError(__OlxSrcInfo, "incorrect construct: ") << Cons[i];
      return;
    }
    con.AddNew(4);
    Tmp = Cons[i].SubStringTo(obi);
    con[i][3] = Tmp.ToInt();
    Tmp = Cons[i].SubString(obi+1, Cons[i].Length() - obi - 2);
    int hkli=-1;
    for( int j=Tmp.Length()-1; j >= 0; j-- ) {
      if( Tmp.CharAt(j) == 'l' )  hkli = 2;
      else if( Tmp.CharAt(j) == 'k' )  hkli = 1;
      else if( Tmp.CharAt(j) == 'h' )  hkli = 0;
      if( hkli == -1 )  {
        Error.ProcessingError(__OlxSrcInfo, "incorrect construct: ") << Cons[i];
        return;
      }
      j--;
      strV = EmptyString;
      while( j >= 0 && !(Tmp[j] >= 'a' && Tmp[j] <= 'z' ) )  {
        strV.Insert( (olxch)Tmp[j], 0 );
        j--;
      }
      if( !strV.IsEmpty() && !(strV == "+") && !(strV == "-") )
        con[i][hkli] = strV.ToDouble();
      else  {
        if( strV.Length() && strV == "-" )
          con[i][hkli] = -1.0;
        else
          con[i][hkli] = 1.0;
      }
      if( con[i][hkli] == 0 )  {
        Error.ProcessingError(__OlxSrcInfo, "illegal value: ") << Cons[i];
        return;
      }
      j++;
    }
  }

/*  TArray3D< TReflection* > Hkl3D;
  long minH = (long)Hkl.GetMinValues()[0];
  long maxH = (long)Hkl.GetMaxValues()[0];
  long minK = (long)Hkl.GetMinValues()[1];
  long maxK = (long)Hkl.GetMaxValues()[1];
  long minL = (long)Hkl.GetMinValues()[2];
  long maxL = (long)Hkl.GetMaxValues()[2];

  for( int i=0; i < Hkl.RefCount(); i++ )
    Hkl3D[ref->H()][ref->K()][ref->L()] = Hkl.Ref(i);
*/
  double SI = 0, SE = 0;
  int count = 0;
  for( int i=0; i < Hkl.RefCount(); i++ )  {
    bool fulfilled = true;
    for( int j=0; j < Cons.Count(); j ++ )  {
      int v = (int)(Hkl[i].GetH()*con[j][0] +
                    Hkl[i].GetK()*con[j][1] +
                    Hkl[i].GetL()*con[j][2] );
      if( con[j][3] == 0 )  {
        if( v != 0 ) {
          fulfilled = false;
          break;
        }
      }
      else if( con[j][3] < 0 )  {
        if( (v%(int)con[j][3]) == 0 )  {
          fulfilled = false;
          break;
        }
      }
      else if( con[j][3] > 0 )  {
        if( (v%(int)con[j][3]) != 0 )  {
          fulfilled = false;
          break;
        }
      }
    }
    if( !fulfilled )  continue;
    count ++;
    SI += Hkl[i].GetI();
    SE += QRT(Hkl[i].GetS());
    if( list )  {
      TBasicApp::GetLog() << Hkl[i].ToString()<< '\n';
    }
  }
  if( count == 0 )  {
    TBasicApp::GetLog() << ("Could not find any reflections fulfilling given condition\n");
    return;
  }
  SI /= count;
  SE = sqrt(SE)/count;

  TBasicApp::GetLog() << ( olxstr("Found " ) << count << " reflections fulfilling given condition\n");
  TBasicApp::GetLog() << ( olxstr("I/s is ") << olxstr::FormatFloat(3, SI) << '/' << olxstr::FormatFloat(3, SE) << '\n' );

}
//..............................................................................
void TMainForm::macSchedule(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {

  if( !Cmds[0].IsNumber() )  {
    Error.ProcessingError(__OlxSrcInfo, "invalid syntax: <interval 'task'> are expected ");
    return;
  }
  bool repeatable = false;
  for( int i=0; i < Options.Count(); i++ )
    if( Options.GetName(i)[0] == 'r' )  {  repeatable = true;  break;  }

  TScheduledTask& task = Tasks.AddNew();
  task.Repeatable= repeatable;
  task.Interval = Cmds[0].ToInt();
  task.Task = Cmds[1];
  task.LastCalled = TETime::Now();
}
//..............................................................................
void TMainForm::funSGList(const TStrObjList& Params, TMacroError &E) {
  E.SetRetVal( GetSGList() );
}
//..............................................................................
void TMainForm::funAnd(const TStrObjList& Params, TMacroError &E) {
  olxstr tmp;
  for(int i=0; i < Params.Count(); i++ )  {
    tmp = Params[i];
    if( !ProcessMacroFunc( tmp ) )  {
      E.ProcessingError(__OlxSrcInfo, "could not process: ") << tmp;
      return;
    }
    if( !tmp.ToBool() )  {
      E.SetRetVal( false );
      return;
    }
  }
  E.SetRetVal( true );
}
//..............................................................................
void TMainForm::funOr(const TStrObjList& Params, TMacroError &E) {
  olxstr tmp;
  for(int i=0; i < Params.Count(); i++ )  {
    tmp = Params[i];
    if( !ProcessMacroFunc( tmp ) )  {
      E.ProcessingError(__OlxSrcInfo, "could not process: ") << tmp;
      return;
    }
    if( tmp.ToBool() )  {
      E.SetRetVal( true );
      return;
    }
  }
  E.SetRetVal( false );
}
//..............................................................................
void TMainForm::funNot(const TStrObjList& Params, TMacroError &E) {
  olxstr tmp = Params[0];

  if( !ProcessMacroFunc( tmp ) )  {
    E.ProcessingError(__OlxSrcInfo, "could not process: ") << tmp;
    return;
  }
  E.SetRetVal( !tmp.ToBool() );
}
//..............................................................................
void TMainForm::macTls(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  double cellParameters[12];
  for( int i = 0; i < 3; i++ )  {
    cellParameters[i]= FXApp->XFile().GetAsymmUnit().Axes()[i].GetV();
    cellParameters[i+3]= FXApp->XFile().GetAsymmUnit().Angles()[i].GetV();
    cellParameters[i+6]= FXApp->XFile().GetAsymmUnit().Axes()[i].GetE();
    cellParameters[i+9]= FXApp->XFile().GetAsymmUnit().Angles()[i].GetE();
  }
  TSAtomPList satoms;
  TXAtomPList xatoms;
  FXApp->FindXAtoms(Cmds.Text(' '), xatoms);
  TListCaster::POP(xatoms, satoms);
  if( !satoms.Count() )
    throw TInvalidArgumentException(__OlxSourceInfo, "atom count");
  xlib::TLS tls(satoms, cellParameters);

  TTTable<TStrList> tab(12, 3);
  olxstr ttitle("TLS analysis for: ");
  for( int i=0; i < satoms.Count(); i++ )  {
    ttitle << satoms[i]->GetLabel();
    if( (i+1) < satoms.Count() )
      ttitle << ", ";
  }
  tab.Row(0)->String(0) = "T";
  for(int i=0; i < 3; i++ )  {
    tab.Row(1)->String(i) = olxstr::FormatFloat( -3, tls.GetT()[i][0] );
    tab.Row(2)->String(i) = olxstr::FormatFloat( -3, tls.GetT()[i][1] );
    tab.Row(3)->String(i) = olxstr::FormatFloat( -3, tls.GetT()[i][2] );
  }
  tab.Row(4)->String(0) = "L";
  for(int i=0; i < 3; i++ )  {
    tab.Row(5)->String(i) = olxstr::FormatFloat( -3, tls.GetL()[i][0] );
    tab.Row(6)->String(i) = olxstr::FormatFloat( -3, tls.GetL()[i][1] );
    tab.Row(7)->String(i) = olxstr::FormatFloat( -3, tls.GetL()[i][2] );
  }
  tab.Row(8)->String(0) = "S";
  for(int i=0; i < 3; i++ )  {
    tab.Row(9)->String(i) = olxstr::FormatFloat( -3, tls.GetS()[i][0] );
    tab.Row(10)->String(i) = olxstr::FormatFloat( -3, tls.GetS()[i][1] );
    tab.Row(11)->String(i) = olxstr::FormatFloat( -3, tls.GetS()[i][2] );
  }
  TStrList output;
  tab.CreateTXTList(output, ttitle, false, false, ' ');
  TBasicApp::GetLog() << ( output );
}
//..............................................................................
void TMainForm::funSfacList(const TStrObjList& Params, TMacroError &E) {
  TIns* ins = (TIns*)FXApp->XFile().GetLastLoader();
  olxstr tmp = ins->GetSfac();
  tmp.Replace(' ', ';');
  E.SetRetVal( tmp );
}
//..............................................................................
void TMainForm::funChooseElement(const TStrObjList& Params, TMacroError &E) {
  TPTableDlg *Dlg = new TPTableDlg(this, FXApp->AtomsInfo());
  if( Dlg->ShowModal() == wxID_OK )
    E.SetRetVal( Dlg->Selected()->GetSymbol() );
  else
    E.SetRetVal( EmptyString );
  Dlg->Destroy();
}
//..............................................................................
void TMainForm::funChooseDir(const TStrObjList& Params, TMacroError &E) {
  olxstr title = "Choose directory",
           defPath = TEFile::CurrentDir();
  if( Params.Count() > 0 )  title = Params[0];
  if( Params.Count() > 1 )  defPath = Params[1];
  wxDirDialog dlg( this, uiStr(title), uiStr(defPath) );
  if( dlg.ShowModal() == wxID_OK )
    E.SetRetVal<olxstr>( dlg.GetPath().c_str() );
}
//..............................................................................
void TMainForm::funStrDir(const TStrObjList& Params, TMacroError &E) {
  E.SetRetVal( GetStructureOlexFolder().SubStringFrom(0,1) );
}
//..............................................................................

class Test_BTreeTraverser {
  int Y, Z;
  bool ZSet, YSet;
public:
  Test_BTreeTraverser()  {  Z = Y = -1;  ZSet = YSet = false;  }
  void SetZ(int v)  {  Z = v;  ZSet = true;  }
  void SetY(int v)  {  Y = v;  YSet = true;  }
  bool OnItem(const BTree<int,int>::Entry* en)  {
    if( ZSet && YSet )  {
      TBasicApp::GetLog() << (olxstr("pair ") << '{' << en->key << ',' << Y << ',' 
        << Z << '}' << '=' << en->val << '\n');
    }
    else if( YSet )  {
      TBasicApp::GetLog() << (olxstr("pair ") << '{' << en->key << ',' << Y << '}' 
        << '=' << en->val << '\n');
    }
    else 
      TBasicApp::GetLog() << (olxstr("pair ") << '{' << en->key << '}' 
        << '=' << en->val << '\n');
    ZSet = YSet = false;
    return true;
  }
};

void TMainForm::macTest(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( !Cmds.IsEmpty() )  {
    TAtomReference ar(Cmds.Text(' '));      
    TCAtomGroup ag;
    int atomAGroup;
    olxstr unp(ar.Expand(FXApp->XFile().GetAsymmUnit(), ag, EmptyString, atomAGroup));
    TBasicApp::GetLog() << "Expanding " << ar.GetExpression() << " to atoms \n";
    for( int i=0; i < ag.Count(); i++ )
      TBasicApp::GetLog() << ag[i].GetFullLabel() << ' ';
    TBasicApp::GetLog() << "\nUnprocessed instructions " << (unp.IsEmpty() ? olxstr("none") : unp) << '\n';
    return;
  }
  BTree<int, int> tree;
  tree.Add(0,0);
  tree.Add(-1,-10);
  tree.Add(1,10);
  tree.Add(-2,-20);
  tree.Add(-2,-21);
  tree.Add(-2,-22);
  tree.Add(-2,-23);
  tree.Add(2,20);
  tree.Add(-3,-30);
  tree.Add(3,30);
  tree.Add(3,31);
  tree.Add(3,32);
  tree.Add(3,33);
  tree.Add(4,40);
  int* tv = NULL;
  tv = tree.Find(0);
  tv = tree.Find(1);
  tv = tree.Find(2);
  tv = tree.Find(3);
  tv = tree.Find(4);
  tv = tree.Find(-1);
  tv = tree.Find(-2);
  tv = tree.Find(-3);
#ifndef __BORLANC__
  Test_BTreeTraverser tt;
  tree.Traverser.Traverse(tree, tt);

  BTree2<int,int> tree2;
  tree2.Add(0,0,0);
  tree2.Add(0,1,1);
  tv = tree2.Find(0,1);
  tree2.Traverser.FullTraverse(tree2, tt);

  BTree3<int, int> tree3;
  tree3.Add(0, 0, 0, 0);
  tree3.Add(0, 1, 0, 1);
  tree3.Add(0, 0, 1, 2);
  tree3.Add(1, 0, 1, 3);
  tv = tree3.Find(0, 0, 1);
  tv = tree3.Find(0, 0, 0);
  tv = tree3.Find(0, 1, 0);
  tree3.Traverser.FullTraverse(tree3, tt);
#endif  
  if( Cmds.IsEmpty() )  return;
  const CString atom_s("ATOM");
  TCStrList lst, toks;
  char bf[256];
  char format[] = "%s";
  TIntList AtomF;
  AtomF.Add(6);  //"ATOM  "
  AtomF.Add(5);  //serial number
  AtomF.Add(5);  //name
  AtomF.Add(4);  //residue name
  AtomF.Add(2);  //chain ID      #21
  AtomF.Add(4);  //  residue sequence number #26
  AtomF.Add(12);  // x                        #38
  AtomF.Add(8);  // y
  AtomF.Add(8);  // z
  AtomF.Add(6);  // occupancy
  AtomF.Add(6);  // temperature factor  #66
  AtomF.Add(12);  // element             #78
  AtomF.Add(2);  // charge

  lst.LoadFromFile(Cmds[0]);
  for( int i=0; i < lst.Count(); i++ )  {
    if( lst[i].StartsFrom(atom_s) )  {
      toks.Clear();
      toks.Strtok(lst[i], ' ');
      memset(bf, 32, 255);
      toks.Delete( toks.Count()-2 );
      int offset = 0;
      for( int j=0; j < olx_min(AtomF.Count(),toks.Count()); j++ )  {
        if( j > 0 )  {
          offset += AtomF[j];
          memcpy(&bf[offset-toks[j].Length()], toks[j].c_str(), toks[j].Length() );
        }
        else if( j == 0 )  {
          memcpy(&bf[offset], toks[j].c_str(), toks[j].Length() );
          offset += AtomF[j];
        }
      }
      bf[offset] = '\0';
      lst[i] = bf;
    }
  }
  lst.Pack();
  lst.SaveToFile( Cmds[0] + ".out" );
  return;
/*
  TStrList lst, toks;
  TEFile sgFile( FXApp->BaseDir() + "sg.txt", "rb" );
  TDataFile df;
  df.LoadFromXLFile( FXApp->BaseDir() + "symmlib.xld" );
  lst.LoadFromStream( sgFile );
  for( int i=2; i < lst.Count(); i++ )  {
    toks.Clear();
    olxstr::Strtok( olxstr::RemoveMultiSpaces( lst[i], true ), ' ', toks);
    if( toks.Count() < 7 )  continue;
    olxstr axis, num;
    int si = toks[0].IndexOf(':');
    if( si != -1 )  {
      num = toks[0].SubStringTo(si);
      axis = toks[0].SubStringFrom(si+1);
    }
    else
      num = toks[0];

    for( int j=0; j < df.Root().ItemCount(); j++ )  {
      if( df.Root().Item(j).GetFieldValue( "#" ) == num &&
          df.Root().Item(j).GetFieldValue("AXIS") == axis )  {
        TBasicApp::GetLog() << ( olxstr("Found ") << df.Root().Item(j).GetName() );
        olxstr tmp = toks[1];
        tmp << ' ' << toks[2] << ' ' << toks[3] << ' ' << toks[4];
        df.Root().Item(j).SetFieldValue( "FULL", tmp );
        break;
      }
    }
  }
  // validating
  for( int i=0; i < df.Root().ItemCount(); i++ )  {
    olxstr tmp = df.Root().Item(i).GetFieldValue("FULL");
    if( tmp.IsEmpty() )  {
      if( df.Root().Item(i).GetName().Length() == 4 )  {
        tmp << df.Root().Item(i).GetName()[0] << ' ' <<
               df.Root().Item(i).GetName()[1] << ' ' <<
               df.Root().Item(i).GetName()[2] << ' ' <<
               df.Root().Item(i).GetName()[3];
        TBasicApp::GetLog() << ( olxstr("Empty, but patched for ") << df.Root().Item(i).GetName() );
      }
      else
        TBasicApp::GetLog() << ( olxstr("Empty val for ") << df.Root().Item(i).GetName() );
      continue;
    }
    toks.Clear();
    olxstr::Strtok( tmp, ' ', toks );
    if( toks.Count() != 4 )  {
      TBasicApp::GetLog() << ( olxstr("Wrong toks count for ") << df.Root().Item(i).GetName() );
      continue;
    }
  }
  // saving
  df.SaveToXLFile( FXApp->BaseDir() + "sg.xld" );
*/
/*
  if( !FXApp->CheckFileType<TIns>() )  return;

  TIns *Ins = (TIns*)FXApp->XFile().GetLastLoader();
  olxstr HklFN = Ins->GetHKLSource();
  if( !TEFile::FileExists(HklFN) )  {
    Error.ProcessingError(__OlxSrcInfo, "could not locate HKL file" );
    return;
  }
  TScattererItLib scatlib;

  THklFile Hkl;
  Hkl.LoadFromFile(HklFN);
  TVectorD vec(3);
  TMatrixD M( Ins->GetAsymmUnit().GetHklToCartesian() );

  double minV = 1000, maxV = 0;
  TEFile out( TEFile::ChangeFileExt(Ins->GetFileName(), "out"), "wb+" );
  olxstr tmp;
  Hkl.AnalyseReflections( *TSymmLib::GetInstance()->FindSG(Ins->GetAsymmUnit()) );
//  Hkl.AnalyseReflections( *TSymmLib::GetInstance()->FindGroup("P-1") );
  for( int i=0; i < Hkl.RefCount(); i++ )  {
    TReflection* ref = Hkl.Ref(i);
    tmp = ref->ToString();
    tmp << ' ' << ref->IsCentric() << ' ' << ref->GetDegeneracy();
    out.Writenl(tmp);
  }
  double cm = 0, am = 0, integ = 0, meanFs = 0;
  int cc = 0, ac = 0, arc = 0;
  double cd = 0, ad = 0;

  long minH = (long)Hkl.GetMinValues()[0];
  long maxH = (long)Hkl.GetMaxValues()[0];
  long minK = (long)Hkl.GetMinValues()[1];
  long maxK = (long)Hkl.GetMaxValues()[1];
  long minL = (long)Hkl.GetMinValues()[2];
  long maxL = (long)Hkl.GetMaxValues()[2];


  typedef AnAssociation4<double, double, int, double> refc;
  TArray3D<refc*> Hkl3D(minH, maxH, minK, maxK, minL, maxL);

  TTypeList<refc*> allRefs;


  for( int i=0; i < Hkl.RefCount(); i++ )  {
    TReflection* ref = Hkl.Ref(i);

    if( fabs(ref->Data()[0])/ref->Data()[1] > 3 )
      continue;

    if( ref->Data()[0] < 0 )
      continue;

    refc* ref1 = Hkl3D[ref->H()][ref->K()][ref->L()];
    if( ref1 != NULL )  {
      ref1->A() += ref->Data()[0];
      ref1->B() += QRT(ref->Data()[1]);
      ref1->C() ++;
      ref1->D() += QRT(ref->Data()[0]);
    }
    else
      Hkl3D[ref->H()][ref->K()][ref->L()] =
        new refc(ref->Data()[0], QRT(ref->Data()[1]), 1, QRT(ref->Data()[0]) );
  }

  allRefs.SetCapacity( Hkl.RefCount() );

  for( int i=minH; i < maxH; i++ )  {
    for( int j=minK; j < maxK; j++ )  {
      for( int k=minL; k < maxL; k++ )  {
        refc* ref1 = Hkl3D[i][j][k];
        if( ref1 != NULL )  {
          ref1->A() /= ref1->C();
          ref1->B() /= sqrt(ref1->GetB());
          ref1->D() /= ref1->C();
          ref1->D() -= QRT( ref1->GetA() );
          ref1->D() = sqrt( ref1->GetD() );
          allRefs.AddACopy( ref1 );
        }
      }
    }
  }

  integ = 0;
  meanFs = 0;
  for( int i=0; i < allRefs.Count(); i++ )  {
//    meanFs += allRefs[i]->GetA();
    meanFs += allRefs[i]->GetA()/olx_max(allRefs[i]->GetB(), allRefs[i]->GetD());
    //}
    delete allRefs[i];
  }
  if( allRefs.Count() != 0 )
    meanFs /= allRefs.Count();

  for( int i=0; i < allRefs.Count(); i++ )
    integ += fabs( (allRefs[i]->GetA()/olx_max(allRefs[i]->GetB(), allRefs[i]->GetD()))/meanFs -1 );
//    integ += fabs(allRefs[i]->GetA()/meanFs -1 );

  if( allRefs.Count() != 0 )
    integ /= allRefs.Count();
  TBasicApp::GetLog() << (olxstr("Calculated value: ") << integ );
*/
  // qpeak anlysis
  TPSTypeList<double, TCAtom*> SortedQPeaks;
  TDoubleList vals;
  int cnt = 0;
  TAsymmUnit& au = FXApp->XFile().GetAsymmUnit();
  for( int i=0; i < au.AtomCount(); i++ )  {
    if( au.GetAtom(i).GetAtomInfo() == iQPeakIndex )  {
      SortedQPeaks.Add( au.GetAtom(i).GetQPeak(), &au.GetAtom(i));
    }
  }
  vals.Add(0);
  for(int i=SortedQPeaks.Count()-1; i >=1; i-- )  {
    if( (SortedQPeaks.GetComparable(i) - SortedQPeaks.GetComparable(i-1))/SortedQPeaks.GetComparable(i) > 0.1 )  {
      TBasicApp::GetLog() << ( olxstr("Threshold here: ") << SortedQPeaks.GetObject(i)->GetLabel() << '\n' );
      vals.Last() += SortedQPeaks.GetComparable(i);
      cnt++;
      vals.Last() /= cnt;
      cnt = 0;
      vals.Add(0);
      continue;
    }
    vals.Last() += SortedQPeaks.GetComparable(i);
    cnt ++;
  }
  if( cnt != 0 )
    vals.Last() /= cnt;
  for( int i=0; i < vals.Count(); i++ )
    TBasicApp::GetLog() << vals[i] << '\n';
}
//..............................................................................
void TMainForm::macLineWidth(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  int lw = Cmds[0].ToInt();
  if( lw > 10 && lw < 500 )
    FGlConsole->SetLineWidth( lw );
}
//..............................................................................
void TMainForm::macLstFS(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  double tc = 0;
  TTTable<TStrList> tab(TFileHandlerManager::Count(), 4);
  tab.ColName(0) = "Name";
  tab.ColName(1) = "Size";
  tab.ColName(2) = "Timestamp";
  tab.ColName(3) = "Persistent";
  for(int i=0; i < TFileHandlerManager::Count(); i++ )  {
    tab.Row(i)->String(0) = TFileHandlerManager::GetBlockName(i);
    tab.Row(i)->String(1) = TFileHandlerManager::GetBlockSize(i);
    tab.Row(i)->String(2) = TFileHandlerManager::GetBlockDateTime(i);
    tab.Row(i)->String(3) = TFileHandlerManager::GetPersistenceId(i);
    tc += TFileHandlerManager::GetBlockSize(i);
  }
  tc /= (1024*1024);

  TStrList Output;
  tab.CreateTXTList(Output, olxstr("Virtual FS content"), true, false, "|");
  FGlConsole->PrintText( Output, NULL, false );
  FGlConsole->PrintText( olxstr("Content size is ") << olxstr::FormatFloat(3, tc)  << "Mb");
}
//..............................................................................
void TMainForm::macInv(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  bool Force = false;  // forces inversion for sg without center of inversion

  for( int i=0; i < Options.Count(); i++ )  {
    if( Options.GetName(i)[0] == 'f' )  {
      Force = true;
      break;
    }
  }

  if( FXApp->CheckFileType<TIns>() || FXApp->CheckFileType<TCif>() )  {
    TSpaceGroup * sg = TSymmLib::GetInstance()->FindSG( FXApp->XFile().GetAsymmUnit() );
    if( sg == NULL )  {
      Error.ProcessingError(__OlxSrcInfo, "unknown file space group" );
      return;
    }
    if( !sg->IsCentrosymmetric() &&  !Force )  {
      Error.ProcessingError(__OlxSrcInfo, "noncentrosymmetric space group, use -f to force" );
      return;
    }
  }
  TXAtomPList xatoms;
  FXApp->FindXAtoms(Cmds.Text(' '), xatoms, true);
  FXApp->InvertFragments( xatoms );
}
//..............................................................................
void TMainForm::macPush(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TVPointD pnt;
  int pc = 0;
  for( int i=0; i < Cmds.Count(); i++ )  {
    if( Cmds[i].IsNumber() )  {
      pnt[pc] = Cmds[i].ToDouble();
      Cmds.Delete(i);
      i--;
      pc++;
      if( pc > 3 )
        throw TInvalidArgumentException(__OlxSourceInfo, "too many numerical parameters");
    }
  }
  if( pc < 3 )
    throw TInvalidArgumentException(__OlxSourceInfo, "wrong number of numerical parameters");
  TXAtomPList xatoms;
  if( !FindXAtoms(Cmds, xatoms, true, true) )  {
    Error.ProcessingError(__OlxSrcInfo, "no atoms provided" );
    return;
  }
  FXApp->MoveFragments( xatoms, pnt );
}
//..............................................................................
void TMainForm::macTransform(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  throw TNotImplementedException(__OlxSourceInfo);
  TXAtomPList xatoms;
  FXApp->FindXAtoms(Cmds.Text(' '), xatoms, true);
  FXApp->InvertFragments( xatoms );
}
//..............................................................................
void TMainForm::macLstRes(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TStrList output;
  olxstr Tmp, Tmp1;
  TAsymmUnit& au = FXApp->XFile().GetAsymmUnit();
  // fixed distances
  if( au.RestrainedDistances().Count() != 0 )  {
    output.Add( olxstr("Restrained distances: ") << au.RestrainedDistances().Count() );
    for( int i=0; i < au.RestrainedDistances().Count(); i++ )  {
      TSimpleRestraint& sr = au.RestrainedDistances()[i];
      sr.Validate();
      if( sr.AtomCount() == 0 )  continue;
      Tmp = olxstr::FormatFloat(3, sr.GetValue());
      Tmp << ' ' << olxstr::FormatFloat(3, sr.GetEsd()) << ' ';
      for( int j=0; j < sr.AtomCount(); j+=2 )  {
        Tmp1 = EmptyString;
        Tmp1 << '[' << sr.GetAtom(j).GetFullLabel() << ',' << sr.GetAtom(j+1).GetFullLabel() << ']';
        Tmp << Tmp1.Format(11, true, ' ');
      }
      output.Add( Tmp );
    }
  }
  // similar distances
  if( au.SimilarDistances().Count() != 0 )  {
    output.Add( olxstr("Similar distances: ") << au.SimilarDistances().Count() );
    for( int i=0; i < au.SimilarDistances().Count(); i++ )  {
      TSimpleRestraint& sr = au.SimilarDistances()[i];
      sr.Validate();
      if( sr.AtomCount() == 0 )  continue;
      Tmp = olxstr::FormatFloat(3, sr.GetEsd());
      Tmp << ' ';
      for( int j=0; j < sr.AtomCount(); j+=2 )  {
        Tmp1 = EmptyString;
        Tmp1 << '[' << sr.GetAtom(j).GetFullLabel() << ',' << sr.GetAtom(j+1).GetFullLabel() << ']';
        Tmp << Tmp1.Format(11, true, ' ');
      }
      output.Add( Tmp );
    }
  }
  // fixed "angles"
  if( au.RestrainedAngles().Count() != 0 )  {
    output.Add( olxstr("Restrained angles: ") << au.RestrainedAngles().Count() );
    for( int i=0; i < au.RestrainedAngles().Count(); i++ )  {
      TSimpleRestraint& sr = au.RestrainedAngles()[i];
      sr.Validate();
      if( sr.AtomCount() == 0 )  continue;
      Tmp = olxstr::FormatFloat(3, sr.GetValue());
      Tmp << ' ' << olxstr::FormatFloat(3, sr.GetEsd()) << ' ';
      for( int j=0; j < sr.AtomCount(); j+=2 )  {
        Tmp1 = EmptyString;
        Tmp1 << '[' << sr.GetAtom(j).GetFullLabel() << ',' << sr.GetAtom(j+1).GetFullLabel() << ']';
        Tmp << Tmp1.Format(11, true, ' ');
      }
      output.Add( Tmp );
    }
  }
  // fixed chiral atomic volumes
  if( au.RestrainedVolumes().Count() != 0 )  {
    output.Add( olxstr("Restrained 'chiral' volumes: ") << au.RestrainedVolumes().Count() );
    for( int i=0; i < au.RestrainedVolumes().Count(); i++ )  {
      TSimpleRestraint& sr = au.RestrainedVolumes()[i];
      sr.Validate();
      if( sr.AtomCount() == 0 )  continue;
      Tmp = olxstr::FormatFloat(3, sr.GetValue());
      Tmp << ' ' << olxstr::FormatFloat(3, sr.GetEsd()) << ' ';
      for( int j=0; j < sr.AtomCount(); j++ )  {
        Tmp << sr.GetAtom(j).GetFullLabel();
        if( (j+1) < sr.AtomCount() )  Tmp << ", ";
      }
      output.Add( Tmp );
    }
  }
  // planar groups
  if( au.RestrainedPlanarity().Count() != 0 )  {
    output.Add( olxstr("Planar groups: ") << au.RestrainedPlanarity().Count() );
    for( int i=0; i < au.RestrainedPlanarity().Count(); i++ )  {
      TSimpleRestraint& sr = au.RestrainedPlanarity()[i];
      sr.Validate();
      if( sr.AtomCount() < 4 )  continue;
      Tmp = olxstr::FormatFloat(3, sr.GetEsd());
      Tmp << " [";
      for( int j=0; j < sr.AtomCount(); j++ )  {
        Tmp << sr.GetAtom(j).GetFullLabel();
        if( (j+1) < sr.AtomCount() )  Tmp << ", ";
      }
      output.Add( Tmp << ']' );
    }
  }
  if( au.RigidBonds().Count() != 0 )  {
    output.Add( olxstr("Rigind bond restrains: ") << au.RigidBonds().Count() );
  }
  if( au.SimilarU().Count() != 0 )  {
    output.Add( olxstr("Similar Uij restrains: ") << au.SimilarU().Count() );
  }
  if( au.RestranedUaAsUi().Count() != 0 )  {
    output.Add( olxstr("Uij restrains to Uiso: ") << au.RestranedUaAsUi().Count() );
  }
  if( au.EquivalentU().Count() != 0 )  {
    output.Add( olxstr("Equivalent Uij constraints: ") << au.EquivalentU().Count() );
  }
  
  TBasicApp::GetLog() << (output);
}
//..............................................................................
void TMainForm::funLSM(const TStrObjList& Params, TMacroError &E) {
  TIns *I = (TIns*)FXApp->XFile().GetLastLoader();
  E.SetRetVal( I->GetRefinementMethod() );
}
//..............................................................................
void TMainForm::funSSM(const TStrObjList& Params, TMacroError &E) {
  TIns *I = (TIns*)FXApp->XFile().GetLastLoader();
  if( I->GetSolutionMethod().IsEmpty() && Params.Count() == 1 )
    E.SetRetVal( Params[0] );
  else
    E.SetRetVal( I->GetSolutionMethod() );
}
//..............................................................................
void TMainForm::macLstSymm(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TUnitCell& uc = FXApp->XFile().GetLattice().GetUnitCell();

  TTTable<TStrList> tab(uc.MatrixCount(), 2);
  tab.ColName(0) = "Code";
  tab.ColName(1) = "Symm";
  for( int i=0; i < uc.MatrixCount(); i++ )  {
    tab[i][0] = TSymmParser::MatrixToSymmCode(uc, uc.GetMatrix(i));
    tab[i][1] = TSymmParser::MatrixToSymm(uc.GetMatrix(i));
  }
  TStrList output;
  tab.CreateTXTList(output, "Unit cell symmetry list", true, true, ' ');
  TBasicApp::GetLog() << (output);
  TimePerFrame = FXApp->Draw();
}
//..............................................................................
void TMainForm::macSgen(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXAtomPList Atoms;
  TMatrixDList symm;
  TMatrixD* matr;
  for( int i=0; i < Cmds.Count(); i++ )  {
    bool validSymm = true;
    matr = new TMatrixD(3,4);
    try  {  TSymmParser::SymmToMatrix( Cmds[i], *matr );  }
    catch( TExceptionBase& )  {
      validSymm = false;
    }
    if( !validSymm )  {
      try  {
        *matr = TSymmParser::SymmCodeToMatrixU(FXApp->XFile().GetLattice().GetUnitCell(), Cmds[i] );
        validSymm = true;
      }
      catch( TExceptionBase& )  {    }
    }
    if( !validSymm )
      delete matr;
    else  {
      Cmds.Delete(i);
      symm.Add( *matr );
      i--;
    }
  }
  if( symm.IsEmpty() )
    throw TInvalidArgumentException(__OlxSourceInfo, "no symm code(s) provided");

  if( !FindXAtoms(Cmds, Atoms, true, true) )  {
    Error.ProcessingError(__OlxSrcInfo, "no atoms provided" );
    return;
  }
  FXApp->Grow(Atoms, symm);
}
//..............................................................................
void TMainForm::macIT(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXAtomPList xatoms;
  if( !FindXAtoms(Cmds, xatoms, true, true) )  {
    Error.ProcessingError(__OlxSrcInfo, "no atoms provided" );
    return;
  }

  bool orient = Options.Contains("o");

  TMatrixD I(3,3), Iv(3,3);
  Iv.E();
  TVPointD cent, c;
  for( int i=0; i < xatoms.Count(); i++ )  {
    TSAtom& a = xatoms[i]->Atom();
    if( a.GetAtomInfo() == iQPeakIndex )  continue;
    cent += a.Center();
  }
  cent /= xatoms.Count();
  for( int i=0; i < xatoms.Count(); i++ )  {
    TSAtom& a = xatoms[i]->Atom();
    if( a.GetAtomInfo() == iQPeakIndex )  continue;
    c = a.Center();
    c -= cent;
    double w = a.GetAtomInfo().GetMr()*a.CAtom().GetOccp();
    I[0][0] += w*( QRT(c[1]) + QRT(c[2]));
    I[0][1] -= w*c[0]*c[1];
    I[0][2] -= w*c[0]*c[2];
    I[1][0] = I[0][1];
    I[1][1] += w*( QRT(c[0]) + QRT(c[2]));
    I[1][2] -= w*c[1]*c[2];
    I[2][0] = I[0][2];
    I[2][1] = I[1][2];
    I[2][2] += w*( QRT(c[0]) + QRT(c[1]));
  }
  TBasicApp::GetLog() << ("Inertion tensor:\n");
  for( int i=0; i < 3; i++ )
    TBasicApp::GetLog() << I[i].ToString() << '\n';
  TMatrixD::EigenValues(I, Iv);
  TBasicApp::GetLog() << ( olxstr("Ixx =  ") << olxstr::FormatFloat(3, I[0][0]) <<
                                 "  Iyy = "  << olxstr::FormatFloat(3, I[1][1]) <<
                                 "  Izz = "  << olxstr::FormatFloat(3, I[2][2]) << '\n'  );
  TBasicApp::GetLog() << ("Eigen vectors:\n");
  for( int i=0; i < 3; i++ )
    TBasicApp::GetLog() << Iv[i].ToString() << '\n';

  if( orient )  {
    TVPointD t =  FXApp->GetRender().GetBasis().GetCenter();
     FXApp->GetRender().Basis()->Orient( Iv, false );
     FXApp->GetRender().Basis()->SetCenter(t);
  }

}
//..............................................................................
void TMainForm::macStartLogging(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  bool clear = Options.Contains("c");
  if( clear )  {
    olxstr fn = ActiveLogFile->GetName();
    if( ActiveLogFile != NULL )  {
      ActiveLogFile->Delete();
      delete ActiveLogFile;
      ActiveLogFile = NULL;
    }
    else  {
      if( TEFile::FileExists(Cmds[0]) )  {
        TEFile::DelFile( Cmds[0] );
      }
    }
  }
  if( ActiveLogFile != NULL )
    return;
  if( TEFile::FileExists(Cmds[0]) )
    ActiveLogFile = new TEFile(Cmds[0], "a+b");
  else
    ActiveLogFile = new TEFile(Cmds[0], "w+b");
  ActiveLogFile->Writenl( EmptyString );
  ActiveLogFile->Writenl( olxstr("Olex2 log started on ") << TETime::FormatDateTime( TETime::Now()) );
}
//..............................................................................
void TMainForm::macViewLattice(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( !TEFile::FileExists( Cmds[0] )  )  {
    Error.ProcessingError(__OlxSrcInfo, "file does not exist" );
    return;
  }
  TBasicCFile* bcf = FXApp->XFile().FindFormat( TEFile::ExtractFileExt(Cmds[0]) );
  if( bcf == NULL )  {
    Error.ProcessingError(__OlxSrcInfo, "unknown file format" );
    return;
  }
  bcf = (TBasicCFile*)bcf->Replicate();
  try  {  bcf->LoadFromFile(Cmds[0]);  }
  catch(const TExceptionBase& exc)  {
    delete bcf;
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }
  TXLattice& xl = FXApp->AddLattice("xlatt", bcf->GetAsymmUnit().GetCellToCartesian() );
  delete bcf;
  return;
}
//..............................................................................
void main_GenerateCrd(const TVPointDList& p, const TMatrixDList& sm, TVPointDList& res) {
  TVPointD v, t;
  for( int i=0; i < sm.Count(); i++ )  {
    v = sm[i] * p[0];
    v[0] += sm[i][0][3];  v[1] += sm[i][1][3];  v[2] += sm[i][2][3];
    t.Null();
    for( int j=0; j < 3; j++ )  {
      while( v[j] > 1.01 )  {  v[j] -= 1;  t[j] -= 1;  }
      while( v[j] < -0.01 ) {  v[j] += 1;  t[j] += 1;  }
    }
    bool found = false;
    for( int j=0; j < res.Count(); j += p.Count() )  { 
      if( v.QDistanceTo( res[j] ) < 0.0001 )  {
        found = true;
        break;
      }
    }
    if( !found )  {
      res.AddCCopy(v);
      for( int j=1; j < p.Count(); j++ )  {
        v = sm[i] * p[j];
        v[0] += sm[i][0][3];  v[1] += sm[i][1][3];  v[2] += sm[i][2][3];
        v += t;
        res.AddCCopy(v);
      }
    }
  }
  // expand translation
  int rc = res.Count();
  for( int x=-1; x <= 1; x++ )  {
    for( int y=-1; y <= 1; y++ )  {
      for( int z=-1; z <= 1; z++ )  {
        if( x == 0 && y == 0 && z == 0 )  continue;
        for( int i=0; i < rc; i+= p.Count() )  {
          v = res[i];
          v[0] += x;  v[1] += y;  v[2] += z;
          t.Null();
          for( int j=0; j < 3; j++ )  {
            if( v[j] > 1.01 )        {  v[j] -= 1;  t[j] -= 1;  }
            else if( v[j] < -0.01 )  {  v[j] += 1;  t[j] += 1;  }
          }
          bool found = false;
          for( int j=0; j < res.Count(); j+=p.Count() )  {  
            if( v.QDistanceTo( res[j] ) < 0.0001 )  {
              found = true;
              break;
            }
          }
          if( !found )  {
            res.AddCCopy(v);
            for( int j=1; j < p.Count(); j++ )  {
              v = res[i+j];
              v[0] += x;  v[1] += y;  v[2] += z;
              v += t;
              res.AddCCopy(v);
            }
          }
        }
      }
    }
  }
}

void TMainForm::macAddObject(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( Cmds[0].Comparei("cell") == 0 && Cmds.Count() == 2 )  {
    if( !TEFile::FileExists( Cmds[1] )  )  {
      Error.ProcessingError(__OlxSrcInfo, "file does not exist" );
      return;
    }
    TBasicCFile* bcf = FXApp->XFile().FindFormat( TEFile::ExtractFileExt(Cmds[1]) );
    if( bcf == NULL )  {
      Error.ProcessingError(__OlxSrcInfo, "unknown file format" );
      return;
    }
    bcf = (TBasicCFile*)bcf->Replicate();
    try  {  bcf->LoadFromFile(Cmds[1]);  }
    catch(const TExceptionBase& exc)  {
      delete bcf;
      throw TFunctionFailedException(__OlxSourceInfo, exc);
    }
    TDUnitCell* duc = new TDUnitCell(olxstr("cell") << (UserCells.Count()+1), &FXApp->GetRender() );
    TVectorD cell(6);
    cell[0] = bcf->GetAsymmUnit().Axes()[0].GetV();
    cell[1] = bcf->GetAsymmUnit().Axes()[1].GetV();
    cell[2] = bcf->GetAsymmUnit().Axes()[2].GetV();
    cell[3] = bcf->GetAsymmUnit().Angles()[0].GetV();
    cell[4] = bcf->GetAsymmUnit().Angles()[1].GetV();
    cell[5] = bcf->GetAsymmUnit().Angles()[2].GetV();
    duc->Init( cell );
    FXApp->AddObjectToCreate( duc );
    TSpaceGroup* sg = TSymmLib::GetInstance()->FindSG( bcf->GetAsymmUnit() );
    UserCells.Add( AnAssociation2<TDUnitCell*, TSpaceGroup*>(duc, sg) );
    duc->Create();
    delete bcf;
  }
  else {
    TDUnitCell* uc = NULL;
    TSpaceGroup* sg = NULL;
    if( !Cmds[2].IsNumber() )  {
      Error.ProcessingError(__OlxSrcInfo, "invalid unit cell reference" );
      return;
    }
    int cr = Cmds[2].ToInt()-1;
    if( cr == -1 )  {
      uc = &FXApp->DUnitCell();
      sg = TSymmLib::GetInstance()->FindSG( FXApp->XFile().GetAsymmUnit() );
    }
    else if( cr >=0 && cr < UserCells.Count() )  {
      uc = UserCells[cr].A();
      sg = UserCells[cr].B();
    }
    else {
      Error.ProcessingError(__OlxSrcInfo, "invalid unit cell reference" );
      return;
    }
    TMatrixDList ml;
    sg->GetMatrices( ml, mattAll );
    TVPointDList p, allPoints;
    TVPointD v;

    if( Cmds[0].Comparei("sphere") == 0 )  {
      if( (Cmds.Count()-3)%3 != 0 )  {
        Error.ProcessingError(__OlxSrcInfo, "invalid number of arguments" );
        return;
      }
      for( int i=3; i < Cmds.Count(); i+= 3 ) 
        p.AddNew(Cmds[i].ToDouble(), Cmds[i+1].ToDouble(), Cmds[i+2].ToDouble());
      main_GenerateCrd(p, ml, allPoints);
      TMatrixD& data = *(new TMatrixD(3, allPoints.Count()));
      for( int i=0; i < allPoints.Count(); i++ )  {
        v = allPoints[i] * uc->GetCellToCartesian();
        data[0][i] = v[0];  data[1][i] = v[1]; data[2][i] = v[2];
      }
      TDUserObj* uo = new TDUserObj(sgloSphere, &data, Cmds[1], &FXApp->GetRender());
      FXApp->AddObjectToCreate( uo );
      uo->Create();
    }
    else if( Cmds[0].Comparei("line") == 0 )  {
      if( (Cmds.Count()-3)%6 != 0 )  {
        Error.ProcessingError(__OlxSrcInfo, "invalid number of arguments" );
        return;
      }
      for( int i=3; i < Cmds.Count(); i+= 6 )  {
        p.AddNew(Cmds[i].ToDouble(), Cmds[i+1].ToDouble(), Cmds[i+2].ToDouble());
        p.AddNew(Cmds[i+3].ToDouble(), Cmds[i+4].ToDouble(), Cmds[i+5].ToDouble());
      }
      main_GenerateCrd(p, ml, allPoints);
      TMatrixD& data = *(new TMatrixD(3, allPoints.Count() ));
      for( int i=0; i < allPoints.Count(); i++ )  {
        v = allPoints[i] * uc->GetCellToCartesian();
        data[0][i] = v[0];  data[1][i] = v[1]; data[2][i] = v[2];
      }
      TDUserObj* uo = new TDUserObj(sgloLines, &data, Cmds[1], &FXApp->GetRender());
      FXApp->AddObjectToCreate( uo );
      uo->Create();
    }
    else if( Cmds[0].Comparei("plane") == 0 )  {
      if( (Cmds.Count()-3)%12 != 0 )  {
        Error.ProcessingError(__OlxSrcInfo, "invalid number of arguments" );
        return;
      }
      for( int i=3; i < Cmds.Count(); i+= 12 )  {
        p.AddNew(Cmds[i].ToDouble(), Cmds[i+1].ToDouble(), Cmds[i+2].ToDouble());
        p.AddNew(Cmds[i+3].ToDouble(), Cmds[i+4].ToDouble(), Cmds[i+5].ToDouble());
        p.AddNew(Cmds[i+6].ToDouble(), Cmds[i+7].ToDouble(), Cmds[i+8].ToDouble());
        p.AddNew(Cmds[i+9].ToDouble(), Cmds[i+10].ToDouble(), Cmds[i+11].ToDouble());
      }
      main_GenerateCrd(p, ml, allPoints);
      TMatrixD& data = *(new TMatrixD(3, allPoints.Count() ));
      for( int i=0; i < allPoints.Count(); i++ )  {
        v = allPoints[i] * uc->GetCellToCartesian();
        data[0][i] = v[0];  data[1][i] = v[1]; data[2][i] = v[2];
      }
      TDUserObj* uo = new TDUserObj(sgloQuads, &data, "user_plane", &FXApp->GetRender());
      FXApp->AddObjectToCreate( uo );
      uo->Create();
    }
    else if( Cmds[0].Comparei("poly") == 0 )  {
    }
    else
      Error.ProcessingError(__OlxSrcInfo, "unknown object type");
  }
}
//..............................................................................
void TMainForm::macDelObject(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  throw TNotImplementedException(__OlxSourceInfo);
}
//..............................................................................
void TMainForm::macTextm(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( !TEFile::FileExists( Cmds[0] )  )  {
    Error.ProcessingError(__OlxSrcInfo, "file does not exist" );
    return;
  }
  TStrList sl;
  sl.LoadFromFile( Cmds[0] );
  for( int i=0; i < sl.Count(); i++ )  {
    ProcessXPMacro(sl[i], Error);
    if( !Error.IsSuccessful() )  break;
  }
}
//..............................................................................
void TMainForm::macOnRefine(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  throw TNotImplementedException(__OlxSourceInfo);
}
//..............................................................................
//..............................................................................

class TestListIteration {
  TTypeList<olxstr>& List;
public:
  TestListIteration( TTypeList<olxstr>& list ) : List(list)  {  }
  void Run(long index)  {
    for( long i=index+1; i < List.Count(); i++ )
      List[index].Compare(List[i]);
  }
  inline TestListIteration* Replicate() const {  return new TestListIteration(List);  }
};

void TMainForm::macTestMT(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  const long listSize = 1024*8;
  TTypeList< olxstr > testList;
  testList.SetCapacity( listSize );
  TestListIteration testListIteration(testList);
  for( long i=0; i < listSize; i++ )
    testList.AddCCopy( "Test string, lol" );
  TBasicApp::GetLog() << ("Testing multithreading compatibility...\n");
  int64_t singleTT;
  int pratio = 1;
  for( int i=1; i <=4; i++ )  {
    FXApp->SetMaxThreadCount(i);
    int64_t st = TETime::msNow();
    TListIteratorManager<TestListIteration> Test(testListIteration, listSize, tQuadraticTask, 0);
    st = TETime::msNow() - st;
    TBasicApp::GetLog() << ( olxstr(i) << " threads " << st << " ms\n");
    if( i == 1 )  {
      singleTT = st;
    }
    else  {
      int pr = Round((double)singleTT/(double)st);
      if( pr > pratio )  {
        pratio = pr;
      }
      else  {
        break;
      }
    }
  }
  TBasicApp::GetLog() << ( olxstr("Maximum number of threads is set to ") << pratio << '\n' );
  FXApp->SetMaxThreadCount( pratio );

}
//..............................................................................
void TMainForm::macSetFont(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TwxGlScene* scene = (TwxGlScene*)FXApp->GetRender().Scene();
  TGlFont* glf = scene->FindFont( Cmds[0] );
  if( glf == NULL )  {
    E.ProcessingError(__OlxSrcInfo, olxstr("undefined font ") << Cmds[0]);
    return;
  }
  TwxGlScene::MetaFont mf(Cmds[1]);
  olxstr ps(Options.FindValue("ps"));
  if( !ps.IsEmpty() )  {
    if( ps.CharAt(0) == '+' || ps.CharAt(0) == '-' )
      mf.SetSize( mf.GetSize() + ps.ToInt() );
    else 
      mf.SetSize( ps.ToInt() );
  }
  if( Options.Contains('i') )  mf.SetItalic( true );
  if( Options.Contains('b') )  mf.SetBold( true );
  scene->CreateFont(glf->GetName(), mf.GetIdString() );
}

//..............................................................................
void TMainForm::funChooseFont(const TStrObjList &Params, TMacroError &E)  {
  olxstr fntId(EmptyString);
  if( !Params.IsEmpty() && (Params[0].Comparei("olex2") == 0) )
    fntId = TwxGlScene::MetaFont::BuildOlexFontId("olex2.fnt", 12, true, false, false);
  olxstr rv( FXApp->GetRender().Scene()->ShowFontDialog(NULL, fntId) );
  if( rv.IsEmpty() )  {
    E.ProcessingError(__OlxSrcInfo, "operation canceled");
    return;
  }
  E.SetRetVal( rv );
}
//..............................................................................
void TMainForm::funGetFont(const TStrObjList &Params, TMacroError &E)  {
  TGlFont* glf = FXApp->GetRender().Scene()->FindFont( Params[0] );
  if( glf == NULL )  {
    E.ProcessingError(__OlxSrcInfo, olxstr("undefined font ") << Params[0]);
    return;
  }
  E.SetRetVal( glf->IdString() );
}
//..............................................................................
void TMainForm::macEditMaterial(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TGlMaterial* mat = NULL;
  TGPCollection* gpc = NULL;
  if( Cmds[0] == "helpcmd" )
    mat = &HelpFontColorCmd;
  else if( Cmds[0] == "helptxt" )
    mat = &HelpFontColorTxt;
  else if( Cmds[0] == "execout" )
    mat = &ExecFontColor;
  else if( Cmds[0] == "error" )
    mat = &ErrorFontColor;
  else if( Cmds[0] == "exception" )
    mat = &ExceptionFontColor;
  else  {
    gpc = FXApp->GetRender().FindCollection( Cmds[0] );
  }
  if( mat == NULL && gpc == NULL )  {
    E.ProcessingError(__OlxSrcInfo, olxstr("undefined material/control ") << Cmds[0]);
    return;
  }
  TdlgMatProp* MatProp = new TdlgMatProp(this, gpc, FXApp);
  if( mat != NULL )
    MatProp->SetCurrent( *mat );

  if( MatProp->ShowModal() == wxID_OK )  {
    if( mat != NULL )
      *mat = MatProp->GetCurrent();
  }
  MatProp->Destroy();

}
//..............................................................................
void TMainForm::macSetMaterial(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TGlMaterial* mat = NULL, *smat = NULL;
  TGPCollection* gpc = NULL;
  if( Cmds[0] == "helpcmd" )
    mat = &HelpFontColorCmd;
  else if( Cmds[0] == "helptxt" )
    mat = &HelpFontColorTxt;
  else if( Cmds[0] == "execout" )
    mat = &ExecFontColor;
  else if( Cmds[0] == "error" )
    mat = &ErrorFontColor;
  else if( Cmds[0] == "exception" )
    mat = &ExceptionFontColor;
  else {
    int di = Cmds[0].IndexOf('.');
    if( di != -1 )  {
      gpc = FXApp->GetRender().FindCollection(Cmds[0].SubStringTo(di));
      if( gpc != NULL )  {
        TGlPrimitive* glp = gpc->PrimitiveByName(Cmds[0].SubStringFrom(di+1));
        if( glp != NULL )  {
          mat = const_cast<TGlMaterial*>((const TGlMaterial*)glp->GetProperties());
          smat = const_cast<TGlMaterial*>(gpc->Style()->Material(Cmds[0].SubStringFrom(di+1)));
        }
      }
    }
  }
  if( mat == NULL )  {
    E.ProcessingError(__OlxSrcInfo, olxstr("undefined material ") << Cmds[0]);
    return;
  }
  mat->FromString( Cmds[1] );
  if( smat != NULL )
    *smat = *mat;
}
//..............................................................................
void TMainForm::funChooseMaterial(const TStrObjList &Params, TMacroError &E)  {
  TGlMaterial glm;
  if( Params.Count() == 1 )
    glm.FromString( Params[0] );
  TdlgMatProp* MatProp = new TdlgMatProp(this, NULL, FXApp);
  MatProp->SetCurrent( glm );

  if( MatProp->ShowModal() == wxID_OK )
    E.SetRetVal( MatProp->GetCurrent().ToString() );
  else
    E.ProcessingError(__OlxSrcInfo, "operation canceled");

  MatProp->Destroy();
}
//..............................................................................
void TMainForm::funGetMaterial(const TStrObjList &Params, TMacroError &E)  {
  TGlMaterial* mat = NULL;
  if( Params[0] == "helpcmd" )
    mat = &HelpFontColorCmd;
  else if( Params[0] == "helptxt" )
    mat = &HelpFontColorTxt;
  else if( Params[0] == "execout" )
    mat = &ExecFontColor;
  else if( Params[0] == "error" )
    mat = &ErrorFontColor;
  else if( Params[0] == "exception" )
    mat = &ExceptionFontColor;
  else  {
    int di = Params[0].IndexOf('.');
    if( di != -1 )  {
      TGPCollection* gpc = FXApp->GetRender().FindCollection(Params[0].SubStringTo(di));
      if( gpc != NULL )  {
        TGlPrimitive* glp = gpc->PrimitiveByName(Params[0].SubStringFrom(di+1));
        if( glp != NULL )
          mat = const_cast<TGlMaterial*>((const TGlMaterial*)glp->GetProperties());
      }
    }
  }
  if( mat == NULL )  {
    E.ProcessingError(__OlxSrcInfo, olxstr("undefined material ") << Params[0]);
    return;
  }
  else
    E.SetRetVal( mat->ToString() );
}
//..............................................................................
void TMainForm::macLstGO(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TStrList output;
  output.SetCapacity( FXApp->GetRender().CollectionCount() );
  for( int i=0; i < FXApp->GetRender().CollectionCount(); i++ )  {
    TGPCollection* gpc = FXApp->GetRender().Collection(i);
    output.Add( gpc->Name() ) << '[';
    for( int j=0; j < gpc->PrimitiveCount(); j++ )  {
      output.Last().String() << gpc->Primitive(j)->Name();
      if( (j+1) < gpc->PrimitiveCount() )
        output.Last().String() << ';';
    }
    output.Last().String() << ']';
  }
  TBasicApp::GetLog() << ( output );
}
//..............................................................................
struct Main_StrFPatt  {
  int h, k, l;
  double ps;
  TEComplex<double> v;
};
void TMainForm::macCalcPatt(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TRefList refs;
  TAsymmUnit& au = FXApp->XFile().GetAsymmUnit();
  // space group matrix list
  TSpaceGroup* sg = TSymmLib::GetInstance()->FindSG(au);
  if( sg == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "could not locate sapce group");
    return;
  }
  TMatrixDList ml;
  sg->GetMatrices(ml, mattAll);
  olxstr hklFileName = FXApp->LocateHklFile();
  if( !TEFile::FileExists(hklFileName) )  {
    E.ProcessingError(__OlxSrcInfo, "could not locate hkl file");
    return;
  }
  THklFile Hkl;
  Hkl.LoadFromFile(hklFileName);
  double av = 0;
  for( int i=0; i < Hkl.RefCount(); i++ )
    av += Hkl[i].GetI() < 0 ? 0 : Hkl[i].GetI();
  av /= Hkl.RefCount();

  THklFile::MergeStats ms = Hkl.Merge( *sg, true, refs);

  double vol = FXApp->XFile().GetLattice().GetUnitCell().CalcVolume();
  int minH = 100,  minK = 100,  minL = 100;
  int maxH = -100, maxK = -100, maxL = -100;

  TVPointD hkl;
  TArrayList<Main_StrFPatt> AllF(refs.Count()*ml.Count());
  int index = 0;
  for( int i=0; i < refs.Count(); i++ )  {
    const TReflection& ref = refs[i];
    for( int j=0; j < ml.Count(); j++, index++ )  {
      ref.MulHklT(hkl, ml[j]);
      if( hkl[0] < minH )  minH = hkl[0];
      if( hkl[1] < minK )  minK = hkl[1];
      if( hkl[2] < minL )  minL = hkl[2];
      if( hkl[0] > maxH )  maxH = hkl[0];
      if( hkl[1] > maxK )  maxK = hkl[1];
      if( hkl[2] > maxL )  maxL = hkl[2];
      AllF[index].h = hkl[0];
      AllF[index].k = hkl[1];
      AllF[index].l = hkl[2];
      AllF[index].ps = hkl[0]*ml[j][0][3] + hkl[1]*ml[j][1][3] + hkl[2]*ml[j][2][3];
      AllF[index].v = sqrt(refs[i].GetI());
      AllF[index].v *= TEComplex<double>::polar(1, 2*M_PI*AllF[index].ps);
    }
  }
// init map
  //const int mapX = 4*olx_max(maxH, ::abs(minH)),
		//	mapY = 4*olx_max(maxK, ::abs(minK)),
		//	mapZ = 4*olx_max(maxL, ::abs(minL));
  const int mapX =100, mapY = 100, mapZ = 100;
  FXApp->XGrid().InitGrid(mapX, mapY, mapZ);
//  TArray3D<double> map(0, mapX/ml.Count(), 
//////////////////////////////////////////////////////////////////////////////////////////
  TEComplex<double> ** S, *T;
  int kLen = maxK-minK+1, hLen = maxH-minH+1, lLen = maxL-minL+1;
  S = new TEComplex<double>*[kLen];
  for( int i=0; i < kLen; i++ )
    S[i] = new TEComplex<double>[lLen];
  T = new TEComplex<double>[lLen];
  const double T_PI = 2*M_PI;
// precalculations
  int maxDim = olx_max(mapX, mapY);
  if( mapZ > maxDim ) maxDim = mapZ;
  int minInd = olx_min(minH, minK);
  if( minL < minInd )  minInd = minL;
  int maxInd = olx_max(maxH, maxK);
  if( maxL > maxInd )  maxInd = maxL;
  int iLen = maxInd - minInd + 1;
  TEComplex<double>** sin_cos = new TEComplex<double>*[maxDim];
  for( int i=0; i < maxDim; i++ )  {
    sin_cos[i] = new TEComplex<double>[iLen];
    for( int j=minInd; j <= maxInd; j++ )  {
      double rv = (double)(i*j)/maxDim, ca, sa;
      rv *= T_PI;
      SinCos(-rv, &sa, &ca);
      sin_cos[i][j-minInd].SetRe(ca);
      sin_cos[i][j-minInd].SetIm(sa);
    }
  }
  TEComplex<double> R;
  for( int ix=0; ix < mapX; ix++ )  {
    for( int i=0; i < AllF.Count(); i++ )  {
      const Main_StrFPatt& sf = AllF[i];
      S[sf.k-minK][sf.l-minL] += sf.v*sin_cos[ix][sf.h-minInd];
    }
    for( int iy=0; iy < mapY; iy++ )  {
      for( int i=minK; i <= maxK; i++ )  {
        for( int j=minL; j <= maxL; j++ )  {
          T[j-minL] += S[i-minK][j-minL]*sin_cos[iy][i-minInd];
        }
      }
      for( int iz=0; iz < mapZ; iz++ )  {
        R.Null();
        for( int i=minL; i <= maxL; i++ )  {
          R += T[i-minL]*sin_cos[iz][i-minInd];
        }
        FXApp->XGrid().SetValue(ix, iy, iz, R.mod()/vol);
      }
      for( int i=0; i < lLen; i++ )  
        T[i].Null();
    }
    for( int i=0; i < kLen; i++ )  
      for( int j=0; j < lLen; j++ )  
        S[i][j].Null();
  }
  for( int i=0; i < kLen; i++ )
    delete [] S[i];
  delete [] S;
  delete [] T;
  for( int i=0; i < maxDim; i++ )
    delete [] sin_cos[i];
  delete [] sin_cos;

  FXApp->XGrid().InitIso(false);
  FXApp->ShowGrid(true, EmptyString);
}
//..............................................................................
//..............................................................................
void TMainForm::funGetMouseX(const TStrObjList &Params, TMacroError &E)  {
  E.SetRetVal( ::wxGetMousePosition().x );
}
//..............................................................................
void TMainForm::funGetMouseY(const TStrObjList &Params, TMacroError &E)  {
  E.SetRetVal( ::wxGetMousePosition().y );
}
//..............................................................................
struct main_peak  { 
  int x, y, z, count;  //center
  bool process;
  double summ;
  main_peak() : process(true), summ(0), count(0) {}
  main_peak(int _x, int _y, int _z, double val) : process(true),  
    summ(val), count(1), x(_x), y(_y), z(_z) {}
};
struct main_level {
  int x, y, z;
  main_level(int _x, int _y, int _z) : x(_x), y(_y), z(_z) {}
  bool operator == (const main_level& l) const {
    if( x == l.x && y == l.y && z == l.z )  return true;
    return false;
  }
};

void main_peak_search(const TArray3D<float>& Data, TArray3D<bool>& Mask, 
                      const TPtrList< TTypeList<main_level> >& SphereMask, TArrayList<main_peak>& maxima)  {
  const int maxX = Data.Length1(),
            maxY = Data.Length2(),
            maxZ = Data.Length3();
  bool*** mask = Mask.Data;
  float*** const data = Data.Data;
  int level = 1;
  bool done = false;
  while( !done )  {
    done = true;
    const TTypeList<main_level>& il = *SphereMask[level];
    for( int i=0; i < maxima.Count(); i++ )  {
      main_peak& peak = maxima[i];
      if( !peak.process )  continue;
      for( int j=0; j < il.Count(); j++ )  {
        int x = peak.x + il[j].x;
        if( x < 0 )     x += maxX;
        if( x >= maxX ) x -= maxX; 
        int y = peak.y + il[j].y;
        if( y < 0 )     y += maxY;
        if( y >= maxY ) y -= maxY; 
        int z = peak.z + il[j].z;
        if( z < 0 )     z += maxZ;
        if( z >= maxZ ) z -= maxZ; 
        if( mask[x][y][z] )  continue;
        if( peak.summ > 0 && data[x][y][z] <= 0 )  {
          peak.process = false;
          break;
        }
        if( peak.summ < 0 && data[x][y][z] >= 0 )  {
          peak.process = false;
          break;
        }
        peak.count++;
        peak.summ += data[x][y][z];
        done = false;
        mask[x][y][z] = true;
      }
    }
    if( ++level >= SphereMask.Count() )  break;
  }
}
struct Main_StrF  {
  int h, k, l;
  double ps;
  TEComplex<double> v;
};
void TMainForm::macCalcFourier(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
// scale type
  static const short stSimple     = 0x0001,
                     stRegression = 0x0002;
  TRefList refs;
  TArrayList<TEComplex<double> > F;
  TAsymmUnit& au = FXApp->XFile().GetAsymmUnit();
  const TUnitCell& uc = FXApp->XFile().GetUnitCell();
  // space group matrix list
  const TSpaceGroup* sg = TSymmLib::GetInstance()->FindSG(au);
  if( sg == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "could not locate sapce group");
    return;
  }
  TMatrixDList ml;
  sg->GetMatrices(ml, mattAll);
  bool diff_map = Options.Contains("diff"); // Fo-Fc
  bool tomc_map = Options.Contains("tomc"); // 2Fo-Fc
  bool abs_map = Options.Contains("abs"); // |Fo-Fc|
  bool obs_map = Options.Contains("obs"); // Fo
  short scaleType = stSimple;
  if( diff_map )  {
    olxstr st = Options.FindValue('s').ToLowerCase();
    if( !st.IsEmpty() )  {
      if( st.CharAt(0) == 'r' )
        scaleType = stRegression;
    }
  }
  if( Options.Contains("fcf") )  {
    olxstr fcffn = TEFile::ChangeFileExt(FXApp->XFile().GetFileName(), "fcf");
    if( !TEFile::FileExists(fcffn) )  {
      fcffn = TEFile::ChangeFileExt(FXApp->XFile().GetFileName(), "fco");
      if( !TEFile::FileExists(fcffn) )  {
        E.ProcessingError(__OlxSrcInfo, "please load fcf file or make sure the one exists in current folder");
        return;
      }
    }
    TCif cif( FXApp->AtomsInfo() );
    cif.LoadFromFile( fcffn );
    TCifLoop* hklLoop = cif.FindLoop("_refln");
    if( hklLoop == NULL )  {
      E.ProcessingError(__OlxSrcInfo, "no hkl loop found");
      return;
    }
    int hInd = hklLoop->Table().ColIndex("_refln_index_h");
    int kInd = hklLoop->Table().ColIndex("_refln_index_k");
    int lInd = hklLoop->Table().ColIndex("_refln_index_l");
    // list 3, F
    int mfInd = hklLoop->Table().ColIndex("_refln_F_meas");
    int sfInd = hklLoop->Table().ColIndex("_refln_F_sigma");
    int aInd = hklLoop->Table().ColIndex("_refln_A_calc");
    int bInd = hklLoop->Table().ColIndex("_refln_B_calc");

    if( hInd == -1 || kInd == -1 || lInd == -1 || 
        mfInd == -1 || sfInd == -1 || aInd == -1 || bInd == -1  ) {
      E.ProcessingError(__OlxSrcInfo, "list 3 fcf file is expected");
      return;
    }
    refs.SetCapacity( hklLoop->Table().RowCount() );
    F.SetCount( hklLoop->Table().RowCount() );
    for( int i=0; i < hklLoop->Table().RowCount(); i++ )  {
      TStrPObjList<olxstr,TCifLoopData*>& row = hklLoop->Table()[i];
      TReflection& ref = refs.AddNew(row[hInd].ToInt(), row[kInd].ToInt(), 
                                     row[lInd].ToInt(), row[mfInd].ToDouble(), row[sfInd].ToDouble());
      if( diff_map )  {
        const TEComplex<double> rv(row[aInd].ToDouble(), row[bInd].ToDouble());
        double dI = (ref.GetI() - rv.mod());
        F[i] = TEComplex<double>::polar(dI, rv.arg());
      }
      else if( tomc_map )  {
        const TEComplex<double> rv(row[aInd].ToDouble(), row[bInd].ToDouble());
        double dI = 2*ref.GetI() - rv.mod();
        F[i] = TEComplex<double>::polar(dI, rv.arg());
      }
      else if( obs_map ) {
        const TEComplex<double> rv(row[aInd].ToDouble(), row[bInd].ToDouble());
        F[i] = TEComplex<double>::polar(ref.GetI(), rv.arg());
      }
      else  {
        F[i].SetRe(row[aInd].ToDouble());
        F[i].SetIm(row[bInd].ToDouble());
      }
    }
  }
  else  {
    olxstr hklFileName = FXApp->LocateHklFile();
    if( !TEFile::FileExists(hklFileName) )  {
      E.ProcessingError(__OlxSrcInfo, "could not locate hkl file");
      return;
    }
    THklFile Hkl;
    Hkl.LoadFromFile(hklFileName);
    double av = 0;
    for( int i=0; i < Hkl.RefCount(); i++ )
      av += Hkl[i].GetI() < 0 ? 0 : Hkl[i].GetI();
    av /= Hkl.RefCount();

    THklFile::MergeStats ms = Hkl.Merge( *sg, true, refs);
    F.SetCount(refs.Count());
    FXApp->CalcSF(refs, F);
    if( diff_map || tomc_map || obs_map )  {
      // find a linear scale between F
      TMatrixD points(2, F.Count() );
      TVectorD line(2);
      double sF2o = 0, sF2c = 0;
      for( int i=0; i < F.Count(); i++ )  {
        points[0][i] = sqrt(refs[i].GetI());
        points[1][i] = F[i].mod();
        if( refs[i].GetI()/refs[i].GetS() < 3 )  continue;
        sF2o += refs[i].GetI();
        sF2c += F[i].qmod();
      }
      double simple_scale = sqrt(sF2o/sF2c);
      double rms = TMatrixD::PLSQ(points, line, 1);
      TBasicApp::GetLog() << olxstr("Trendline scale: ") << line.ToString() << '\n';
      TBasicApp::GetLog() << olxstr("Simple scale: ") << olxstr::FormatFloat(3,simple_scale) << '\n';
      
      for( int i=0; i < F.Count(); i++ )  {
        double dI = sqrt(refs[i].GetI());
        if( scaleType == stSimple )
          dI /= simple_scale;
        else if( scaleType == stRegression )  {
          dI *= line[1];
          dI += line[0];
        }
        if( diff_map )  {
          dI -= F[i].mod();
          F[i] = TEComplex<double>::polar(dI, F[i].arg());
        }
        else if( tomc_map )  {
          dI *= 2;
          dI -= F[i].mod();
          F[i] = TEComplex<double>::polar(dI, F[i].arg());
        }
        else if( obs_map )  {
          F[i] = TEComplex<double>::polar(dI, F[i].arg());
        }
      }
    }
  }
  double vol = FXApp->XFile().GetLattice().GetUnitCell().CalcVolume();
  int minH = 100,  minK = 100,  minL = 100;
  int maxH = -100, maxK = -100, maxL = -100;

  TVPointD hkl;
  TArrayList<Main_StrF> AllF(refs.Count()*ml.Count());
  int index = 0;
  for( int i=0; i < refs.Count(); i++ )  {
    const TReflection& ref = refs[i];
    for( int j=0; j < ml.Count(); j++, index++ )  {
      ref.MulHklT(hkl, ml[j]);
      if( hkl[0] < minH )  minH = hkl[0];
      if( hkl[1] < minK )  minK = hkl[1];
      if( hkl[2] < minL )  minL = hkl[2];
      if( hkl[0] > maxH )  maxH = hkl[0];
      if( hkl[1] > maxK )  maxK = hkl[1];
      if( hkl[2] > maxL )  maxL = hkl[2];
      AllF[index].h = hkl[0];
      AllF[index].k = hkl[1];
      AllF[index].l = hkl[2];
      AllF[index].ps = hkl[0]*ml[j][0][3] + hkl[1]*ml[j][1][3] + hkl[2]*ml[j][2][3];
      AllF[index].v = F[i];
      AllF[index].v *= TEComplex<double>::polar(1, 2*M_PI*AllF[index].ps);
    }
  }
// init map
  const int mapX = (int)au.Axes()[0].GetV()*5,
			mapY = (int)au.Axes()[1].GetV()*5,
			mapZ = (int)au.Axes()[2].GetV()*5;
//  const int mapX =100, mapY = 100, mapZ = 10;
  FXApp->XGrid().InitGrid(mapX, mapY, mapZ);
  FXApp->XGrid().SetMaxHole(0.49);
  FXApp->XGrid().SetMinHole(-0.49);
//  TArray3D<double> map(0, mapX/ml.Count(), 
//////////////////////////////////////////////////////////////////////////////////////////
  TEComplex<double> ** S, *T;
  int kLen = maxK-minK+1, hLen = maxH-minH+1, lLen = maxL-minL+1;
  S = new TEComplex<double>*[kLen];
  for( int i=0; i < kLen; i++ )
    S[i] = new TEComplex<double>[lLen];
  T = new TEComplex<double>[lLen];
  const double T_PI = 2*M_PI;
// precalculations
  int minInd = olx_min(minH, minK);
  if( minL < minInd )  minInd = minL;
  int maxInd = olx_max(maxH, maxK);
  if( maxL > maxInd )  maxInd = maxL;
  int iLen = maxInd - minInd + 1;
  int mapMax = olx_max(mapX, mapY);
  if( mapZ > mapMax )  mapMax = mapZ;
  TEComplex<double>** sin_cosX = new TEComplex<double>*[mapX],
                      **sin_cosY, **sin_cosZ;
  for( int i=0; i < mapX; i++ )  {
    sin_cosX[i] = new TEComplex<double>[iLen];
    for( int j=minInd; j <= maxInd; j++ )  {
      double rv = (double)(i*j)/mapX, ca, sa;
      rv *= T_PI;
      SinCos(-rv, &sa, &ca);
      sin_cosX[i][j-minInd].SetRe(ca);
      sin_cosX[i][j-minInd].SetIm(sa);
    }
  }
  if( mapX == mapY )  {
    sin_cosY = sin_cosX;
  }
  else  {
    sin_cosY = new TEComplex<double>*[mapY];
    for( int i=0; i < mapY; i++ )  {
      sin_cosY[i] = new TEComplex<double>[iLen];
      for( int j=minInd; j <= maxInd; j++ )  {
        double rv = (double)(i*j)/mapY, ca, sa;
        rv *= T_PI;
        SinCos(-rv, &sa, &ca);
        sin_cosY[i][j-minInd].SetRe(ca);
        sin_cosY[i][j-minInd].SetIm(sa);
      }
    }
  }
  if( mapX == mapZ )  {
    sin_cosZ = sin_cosX;
  }
  else if( mapY == mapZ )  {
    sin_cosZ = sin_cosY;
  }
  else  {
    sin_cosZ = new TEComplex<double>*[mapZ];
    for( int i=0; i < mapZ; i++ )  {
      sin_cosZ[i] = new TEComplex<double>[iLen];
      for( int j=minInd; j <= maxInd; j++ )  {
        double rv = (double)(i*j)/mapZ, ca, sa;
        rv *= T_PI;
        SinCos(-rv, &sa, &ca);
        sin_cosZ[i][j-minInd].SetRe(ca);
        sin_cosZ[i][j-minInd].SetIm(sa);
      }
    }
  }
  TEComplex<double> R;
  /* http://smallcode.weblogs.us/2006/11/27/calculate-standard-deviation-in-one-pass/
    for one pass calculation of the variance
  */
  TXGrid& grid = FXApp->XGrid();
  double maxMapV = -1000, minMapV = 1000, sum = 0, sq_sum = 0;
  for( int ix=0; ix < mapX; ix++ )  {
    for( int i=0; i < AllF.Count(); i++ )  {
      const Main_StrF& sf = AllF[i];
      S[sf.k-minK][sf.l-minL] += sf.v*sin_cosX[ix][sf.h-minInd];
    }
    for( int iy=0; iy < mapY; iy++ )  {
      for( int i=minK; i <= maxK; i++ )  {
        for( int j=minL; j <= maxL; j++ )  {
          T[j-minL] += S[i-minK][j-minL]*sin_cosY[iy][i-minInd];
        }
      }
      for( int iz=0; iz < mapZ; iz++ )  {
        R.Null();
        for( int i=minL; i <= maxL; i++ )  {
          R += T[i-minL]*sin_cosZ[iz][i-minInd];
        }
        double val = R.Re()/vol;
        sum += ((val < 0) ? -val : val);
        sq_sum += val*val;
        if( abs_map && val < 0 )  val = -val;
        if( val > maxMapV )  maxMapV = val;
        if( val < minMapV )  minMapV = val;
        grid.SetValue(ix, iy, iz, val);
      }
      for( int i=0; i < lLen; i++ )  
        T[i].Null();
    }
    for( int i=0; i < kLen; i++ )  
      for( int j=0; j < lLen; j++ )  
        S[i][j].Null();
  }
  double map_mean = sum/(mapX*mapY*mapZ);
  double map_var = sq_sum/(mapX*mapY*mapZ) - map_mean*map_mean;
  FXApp->XGrid().SetMaxHole(sqrt(map_var)*1.4);
  FXApp->XGrid().SetMinHole(-sqrt(map_var)*1.4);
  grid.SetScale( -sqrt(map_var)*4 );
  TBasicApp::GetLog() << (olxstr("Map max val ") << olxstr::FormatFloat(3, maxMapV) << " min val " << olxstr::FormatFloat(3, minMapV) << '\n');
  // map clean up
  //float*** gridData = grid.Data()->Data;
  //maxMapV *= 0.25; 
  //minMapV *= 0.25; 
  //for( int ix=0; ix < mapX; ix++ )  {
  //  for( int iy=0; iy < mapY; iy++ )  {
  //    for( int iz=0; iz < mapZ; iz++ )  {
  //      if( gridData[ix][iy][iz] < maxMapV && gridData[ix][iy][iz] > minMapV )
  //        gridData[ix][iy][iz] = 0;
  //    }
  //  }
  //}
  // end map clean up

  // map integration
/*
  TPtrList< TTypeList<main_level> > SphereMask;
  for( int level=0; level < 11; level++ )
    SphereMask.Add( new TTypeList<main_level> );
  
  for( int x=-10; x < 11; x++ )  {
    for( int y=-10; y < 11; y++ )  {
      for( int z=-10; z < 11; z++ )  {
        int r = Round(sqrt((double)(x*x + y*y + z*z)));
        if( r < 11 && r > 0 )  // skip 0
          SphereMask[r]->AddNew(x,y,z);
      }
    }
  }
  // eliminate duplicate indexes
  for( int i=0; i < 11; i++ )  {
    TTypeList<main_level>& l1 = *SphereMask[i];
    for( int j= i+1; j < 11; j++ )  {
      TTypeList<main_level>& l2 = *SphereMask[j];
      for( int k=0; k < l1.Count(); k++ )  {
        if( l1.IsNull(k) )  continue;
        for( int l=0; l < l2.Count(); l++ )  {
          if( l2[l] == l1[k] )  {
            l2.NullItem(l);
            break;
          }
        }
      }
    }
    l1.Pack();
  }
  const int s_level = 3;
  TArray3D<bool> Mask(0, mapX-1, 0, mapY-1, 0, mapZ-1);
  TArrayList<main_peak> Peaks;
  float*** gridData = grid.Data()->Data;
  bool*** maskData = Mask.Data;
  double pos_level = 0.5*maxMapV, neg_level = 0.8*minMapV; 
  for( int ix=0; ix < mapX; ix++ )  {
    for( int iy=0; iy < mapY; iy++ )  {
      for( int iz=0; iz < mapZ; iz++ )  {
        if( !maskData[ix][iy][iz] && ((gridData[ix][iy][iz] > pos_level) ||
                                      (gridData[ix][iy][iz] < neg_level)) )  {
          const double refval = gridData[ix][iy][iz];
          bool located = true;
          if( refval > 0 )  {
            for( int i=-s_level; i <= s_level; i++ )  {
              int x = ix+i;
              if( x < 0 )      x += mapX;
              if( x >= mapX )  x -= mapX;
              for( int j=-s_level; j <= s_level; j++ )  {
                int y = iy+j;
                if( y < 0 )      y += mapY;
                if( y >= mapY )  y -= mapY;
                for( int k=-s_level; k <= s_level; k++ )  {
                  if( i==0 && j==0 && k == 0 )  continue;
                  int z = iz+k;
                  if( z < 0 )      z += mapZ;
                  if( z >= mapZ )  z -= mapZ;
                  if( gridData[x][y][z] > refval )  {
                    located = false;
                    break;
                  }
                }
                if( !located )  break;
              }
              if( !located )  break;
            }
          }
          else  {
            for( int i=-s_level; i <= s_level; i++ )  {
              int x = ix+i;
              if( x < 0 )      x += mapX;
              if( x >= mapX )  x -= mapX;
              for( int j=-s_level; j <= s_level; j++ )  {
                int y = iy+j;
                if( y < 0 )      y += mapY;
                if( y >= mapY )  y -= mapY;
                for( int k=-s_level; k <= s_level; k++ )  {
                  if( i==0 && j==0 && k == 0 )  continue;
                  int z = iz+k;
                  if( z < 0 )      z += mapZ;
                  if( z >= mapZ )  z -= mapZ;
                  if( gridData[x][y][z] < refval )  {
                    located = false;
                    break;
                  }
                }
                if( !located )  break;
              }
              if( !located )  break;
            }
          }
          if( located )
            Peaks.Add( main_peak(ix, iy, iz, refval) );
        }
      }
    }
  }
  int PointCount = mapX*mapY*mapZ;
  main_peak_search(*grid.Data(), Mask, SphereMask, Peaks);
  for( int i=0; i < Peaks.Count(); i++ )  {
    const main_peak& peak = Peaks[i];
    if( peak.count >= 64 )  {
      TVPoint<double> cnt; 
      cnt[0] = (double)peak.x/mapX;
      cnt[1] = (double)peak.y/mapY;
      cnt[2] = (double)peak.z/mapZ;
      double pv = (double)peak.count*vol/PointCount;
      double ed = peak.summ/(pv*218);
      TCAtom* oa = uc.FindOverlappingAtom(cnt, 0.1);
      if( oa != NULL )  {
        if( oa->GetAtomInfo() != iQPeakIndex )  {
          TBasicApp::GetLog() << (olxstr("Atom type under consideration ") << oa->GetLabel() << '\n');
        }
        else  {
          oa->SetQPeak( oa->GetQPeak() + ed);
        }
        continue;
      }
      //au.CellToCartesian(cnt);
      //TBasicApp::GetLog() << (olxstr("Peak ") << olxstr::FormatFloat(3, pv) << " at " << cnt.ToWStr() << '\n');
      TCAtom& ca = au.NewAtom();
      ca.SetLabel(olxstr("Q") << olxstr((100+i)));
      ca.CCenter().Value(0) = cnt[0];
      ca.CCenter().Value(1) = cnt[1];
      ca.CCenter().Value(2) = cnt[2];
      ca.SetQPeak( ed );
      ca.SetLoaderId( liNewAtom );
    }
  }
  for( int level=0; level < 11; level++ )
    delete SphereMask[level];
  // end of integration 
  au.InitData();
  FXApp->XFile().EndUpdate();
  // end map clean up
*/
  for( int i=0; i < kLen; i++ )
    delete [] S[i];
  delete [] S;
  delete [] T;
  if( sin_cosY == sin_cosX )  sin_cosY = NULL;
  if( sin_cosZ == sin_cosX || sin_cosZ == sin_cosY )  sin_cosZ = NULL;
  for( int i=0; i < mapX; i++ )
    delete [] sin_cosX[i];
  delete [] sin_cosX;
  if( sin_cosY != NULL )  {
    for( int i=0; i < mapY; i++ )
      delete [] sin_cosY[i];
    delete [] sin_cosY;
  }
  if( sin_cosZ != NULL )  {
    for( int i=0; i < mapZ; i++ )
      delete [] sin_cosZ[i];
    delete [] sin_cosZ;
  }

  FXApp->XGrid().InitIso(false);
  FXApp->ShowGrid(true, EmptyString);
}
//..............................................................................
void TMainForm::macFlush(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TBasicApp::GetLog().Flush();
}
//..............................................................................
void TMainForm::macShowSymm(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  throw TNotImplementedException(__OlxSourceInfo);
}
//..............................................................................
void TMainForm::macSGE(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TPtrList<TSpaceGroup> sgs;
  TSpaceGroup* sg = NULL;
  bool cntro = false;
  E.SetRetVal(&sgs);
  ProcessXPMacro("SG", E, false, false);
  E.SetRetVal<bool>(false);
  if( sgs.Count() == 0 )  {
    TBasicApp::GetLog().Error( "Could not find any suitable spacegroup. Terminating ... " );
    return;
  }
  else if( sgs.Count() == 1 )  {
    sg = sgs[0];
    TBasicApp::GetLog() << "Univocal spacegroup choice: " << sg->GetName() << '\n';
  }
  else  {
    E.Reset();
    ProcessXPMacro("Wilson", E, false, false);
    bool centro = E.GetRetVal().ToBool();
    TBasicApp::GetLog() << "Searching for centrosymmetric group: " << centro << '\n';
    for( int i=0; i < sgs.Count(); i++ )  {
      if( centro )  {
        if( sgs[i]->IsCentrosymmetric() )  {
          sg = sgs[i];
          break;
        }
      }
      else  {
        if( !sgs[i]->IsCentrosymmetric() )  {
          sg = sgs[i];
          break;
        }
      }
    }
    if( sg == NULL )  {  // no match to centre of symmetry found
      sg = sgs[0];
      TBasicApp::GetLog() << "Could not match, choosing: " << sg->GetName() << '\n';
    }
    else  {
      TBasicApp::GetLog() << "Chosen: " << sg->GetName() << '\n';
    }
  }
  olxstr fn( Cmds.IsEmpty() ? TEFile::ChangeFileExt(FXApp->XFile().GetFileName(), "ins") : Cmds[0] );
  ProcessXPMacro(olxstr("reset -s=") << sg->GetName() << " -f='" << fn << '\'', E);
  if( E.IsSuccessful() )  {
    ProcessXPMacro(olxstr("reap '") << fn << '\'', E);
    if( E.IsSuccessful() )  ProcessXPMacro(olxstr("solve"), E);
    E.SetRetVal<bool>(E.IsSuccessful());
  }
}
//..............................................................................
class TestClass : public PyObject {
  olxstr* instance;
public:
  TestClass(olxstr* inst)  {  instance = inst;  }
  static PyObject* Call(PyObject* self, PyObject* args)  {
    TestClass* tc = (TestClass*)self;
    TBasicApp::GetLog().Error( *tc->instance );
    Py_IncRef(Py_None);
    return Py_None;
  }
};
PyMethodDef Test_Methods[] = {
  {"m", TestClass::Call, METH_VARARGS, "test"},
  {NULL, NULL, 0, NULL}
};

void TMainForm::macTestBinding(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TestClass* tc = new TestClass( new olxstr("python test") );
  //str->ob_refcnt = 0;
  Py_InitModule4( "TestClass", Test_Methods, "doc", tc, 0 );
}
//..............................................................................
double Main_FindClosestDistance(const TMatrixDList& ml, TVPointD& o_from, const TCAtom& a_to) {
  TVPointD V1, V2, from(o_from), to(a_to.GetCCenter());
  V2 = from-to;
  a_to.GetParent()->CellToCartesian(V2);
  double minD = V2.Length();
  for( int i=0; i < ml.Count(); i++ )  {
    V1 = ml[i] * from;
    V1[0] += ml[i][0][3];  V1[1] += ml[i][1][3];  V1[2] += ml[i][2][3];
    V2 = V1;
    V2 -=to;
    int iv = Round(V2[0]);
    V2[0] -= iv;  V1[0] -= iv;  // find closest distance
    iv = Round(V2[1]);
    V2[1] -= iv;  V1[1] -= iv;
    iv = Round(V2[2]);
    V2[2] -= iv;  V1[2] -= iv;
    a_to.GetParent()->CellToCartesian(V2);
    double D = V2.Length();
    if( D < minD )  {
      minD = D;
      o_from = V1;
    }
  }
  return minD;
}
class TestDistanceAnalysisIteration {
  TCif cif;
  const TStrList& files;
  TAsymmUnit& au;
  TMatrixDList ml;
public:
  TPSTypeList<int, int> XYZ, XY, XZ, YZ, XX, YY, ZZ;  // length, occurence 

  TestDistanceAnalysisIteration( const TStrList& f_list, TAtomsInfo* ai) : 
      files(f_list), cif(ai), au(cif.GetAsymmUnit())  
  {  
    //TBasicApp::GetInstance()->SetMaxThreadCount(1);  // reset for xfile
  }
  ~TestDistanceAnalysisIteration()  {   }
  int Run(long i)  {
    TBasicApp::GetLog() << files[i] << '\n';  
    try { cif.LoadFromFile(files[i]);  }
    catch( ... )  {
      TBasicApp::GetLog().Exception(olxstr("Failed on ") << files[i]);
      return 0;
    }
    TSpaceGroup* sg = TSymmLib::GetInstance()->FindSG(au);
    if( sg == NULL )  {
      TBasicApp::GetLog().Exception(olxstr("Unknown sg for ") << files[i]);    
      return 0;
    }
    int cc = 0;
    ml.Clear();
    sg->GetMatrices(ml, mattAll);
    TVPointD diff;
    for( int j=0; j < au.AtomCount(); j++ )  {
      TCAtom& a1 = au.GetAtom(j);
      if( a1.GetId() == -1 )  continue;
      if( a1.GetAtomInfo() == iHydrogenIndex )  continue;
      for( int k=j+1; k < au.AtomCount(); k++ )  {
        TCAtom& a2 = au.GetAtom(k);
        if( a2.GetId() == -1 )  continue;
        cc ++;
        if( a2.GetAtomInfo() == iHydrogenIndex )  continue;
        TVPointD from(a1.GetCCenter()), to(a2.GetCCenter());
        int d = Round(Main_FindClosestDistance(ml, from, a2)*100);
        if( d < 1 )  {  // symm eq
          a2.SetId(-1);
          continue;
        }
        // XYZ
        int ind = XYZ.IndexOfComparable(d);
        if( ind == -1 )  XYZ.Add(d, 1);
        else             XYZ.Object(ind)++;
        // XY
        diff[0] = from[0]-to[0];  diff[1] = from[1]-to[1]; diff[2] = 0;
        au.CellToCartesian(diff);
        d = Round(diff.Length()*100);  // keep two numbers
        ind = XY.IndexOfComparable(d);
        if( ind == -1 )  XY.Add(d, 1);
        else             XY.Object(ind)++;
        // XZ
        diff[0] = from[0]-to[0];  diff[2] = from[2]-to[2]; diff[1] = 0;
        au.CellToCartesian(diff);
        d = Round(diff.Length()*100);  // keep two numbers
        ind = XZ.IndexOfComparable(d);
        if( ind == -1 )  XZ.Add(d, 1);
        else             XZ.Object(ind)++;
        // YZ
        diff[1] = from[1]-to[1];  diff[2] = from[2]-to[2]; diff[0] = 0;
        au.CellToCartesian(diff);
        d = Round(diff.Length()*100);  // keep two numbers
        ind = YZ.IndexOfComparable(d);
        if( ind == -1 )  YZ.Add(d, 1);
        else             YZ.Object(ind)++;
        // XX
        diff[0] = from[0]-to[0];  diff[1] = 0; diff[2] = 0;
        au.CellToCartesian(diff);
        d = Round(diff.Length()*100);  // keep two numbers
        ind = XX.IndexOfComparable(d);
        if( ind == -1 )  XX.Add(d, 1);
        else             XX.Object(ind)++;
        // YY
        diff[1] = from[1]-to[1];  diff[0] = 0; diff[2] = 0;
        au.CellToCartesian(diff);
        d = Round(diff.Length()*100);  // keep two numbers
        ind = YY.IndexOfComparable(d);
        if( ind == -1 )  YY.Add(d, 1);
        else             YY.Object(ind)++;
        // ZZ
        diff[2] = from[2]-to[2];  diff[0] = 0; diff[1] = 0;
        au.CellToCartesian(diff);
        d = Round(diff.Length()*100);  // keep two numbers
        ind = ZZ.IndexOfComparable(d);
        if( ind == -1 )  ZZ.Add(d, 1);
        else             ZZ.Object(ind)++;
      }
    }
    return cc;
  }
  inline TestDistanceAnalysisIteration* Replicate() const {  return new TestDistanceAnalysisIteration(files, &cif.GetAtomsInfo());  }
};
void TMainForm::macTestStat(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TStrList files;
  TEFile::ListCurrentDir(files, "*.cif", sefFile);
  TCif cif(XApp()->AtomsInfo());
  TAsymmUnit& au = cif.GetAsymmUnit();

  TPSTypeList<TBasicAtomInfo*, double*> atomTypes;
  TVPointD v1;
  double tmp_data[601];
  TMatrixDList ml;
  for( int i=0; i < files.Count(); i++ )  {
    TBasicApp::GetLog() << files[i] << '\n';  
    try { cif.LoadFromFile(files[i]);  }
    catch( ... )  {
      TBasicApp::GetLog().Exception(olxstr("Failed on ") << files[i]);
      continue;
    }
    TSpaceGroup* sg = TSymmLib::GetInstance()->FindSG(au);
    if( sg == NULL )  {
      TBasicApp::GetLog().Exception(olxstr("Unknown sg for ") << files[i]);    
      continue;
    }
    ml.Clear();
    sg->GetMatrices(ml, mattAll);
    for( int j=0; j < au.AtomCount(); j++ )  {
      TCAtom& a1 = au.GetAtom(j);
      if( a1.GetId() == -1 )  continue;
      if( a1.GetAtomInfo() == iHydrogenIndex )  continue;
      int ai = atomTypes.IndexOfComparable(&a1.GetAtomInfo());
      double* data = ((ai == -1) ? new double[601] : atomTypes.Object(ai));
      if( ai == -1 )  {// new array, initialise
        memset(data, 0, sizeof(double)*600);
        atomTypes.Add( &a1.GetAtomInfo(), data);
      }
      TVPointD from(a1.GetCCenter());
      for( int k=j+1; k < au.AtomCount(); k++ )  {
        TCAtom& a2 = au.GetAtom(k);
        if( a2.GetId() == -1 )  continue;
        if( a2.GetAtomInfo() == iHydrogenIndex )  continue;
        TVPointD to(a2.GetCCenter());
        memset(tmp_data, 0, sizeof(double)*600);
        for( int l=0; l < ml.Count(); l++ )  {
          v1 = ml[l] * to;
          v1[0] += ml[l][0][3];  v1[1] += ml[l][1][3];  v1[2] += ml[l][2][3];
          v1 -= from;
          v1[0] -= Round(v1[0]);  v1[1] -= Round(v1[1]);  v1[2] -= Round(v1[2]);
          au.CellToCartesian(v1);
          double d = v1.Length();
          if( d <= 0.01 )  {
            a2.SetId(-1);
            break;
          }
          if( d < 6 )
            tmp_data[Round(d*100)] ++;
        }
        if( a2.GetId() != -1 )  {
          for( int l=0; l < 600; l++ )
            data[l] += tmp_data[l];
        }
      }
    }
    FXApp->Draw();
    wxTheApp->Dispatch();
  }
  memset(tmp_data, 0, sizeof(double)*600);
  TEFile out("c:\\tmp\\bin_r5.db", "w+b");
  for( int i=0; i < atomTypes.Count(); i++ )  {
    double* data = atomTypes.Object(i);
    TCStrList sl;
    sl.SetCapacity( 600 );
    double sq = 0;
    for( int j=0; j < 600; j++ )  {
      if( j < 598 )
        sq += ((data[j+1]+data[j])*0.01/2.0);
    }
    for( int j=0; j < 600; j++ )  {
      data[j] /= sq;
      tmp_data[j] += data[j]*100.0;
      sl.Add( (double)j/100 ) << '\t' << data[j]*1000.0;  // normalised by 1000 square
    }
    sl.SaveToFile( (Cmds[0]+'_') << atomTypes.GetComparable(i)->GetSymbol() << ".xlt");
    out << (int16_t) atomTypes.GetComparable(i)->GetIndex();
    out.Write(data, 600*sizeof(double));
    delete [] data;
  }
  TCStrList sl;
  sl.SetCapacity( 600 );
  for( int i=0; i < 600; i++ )
    sl.Add( (double)i/100 ) << '\t' << tmp_data[i];
  sl.SaveToFile( (Cmds[0]+"_all") << ".xlt");
  return;
// old procedure
  TestDistanceAnalysisIteration testdai(files, XApp()->AtomsInfo());
//  FXApp->SetMaxThreadCount(4);
//  TListIteratorManager<TestDistanceAnalysisIteration>* Test = new TListIteratorManager<TestDistanceAnalysisIteration>(testdai, files.Count(), tLinearTask, 0);
//  delete Test;
  if( files.Count() == 1 && TEFile::FileExists(Cmds[0]) )  {
    TStrList sl;
    sl.LoadFromFile( Cmds[0] );
    TPSTypeList<int, int> ref_data, &data = testdai.XYZ;
    for( int i=0; i < sl.Count(); i++ )  {
      int ind = sl[i].IndexOf('\t');
      if( ind == -1 )  {
        TBasicApp::GetLog() << (olxstr("could not parse '") << sl[i] << "\'\n");
        continue;
      }
      ref_data.Add( Round(sl[i].SubStringTo(ind).ToDouble()*100), sl[i].SubStringFrom(ind+1).ToInt() );
    }
    double R = 0;
    for( int i=0; i < data.Count(); i++ )  {
      int ind = ref_data.IndexOfComparable(data.GetComparable(i));
      if( ind == -1 )  {
        TBasicApp::GetLog() << (olxstr("undefined distance ") << data.GetComparable(i) << '\n');
        continue;
      }
      R += sqrt( (double)ref_data.Object(ind)*data.Object(i) );
    }
//    if( cc != 0 )
//      TBasicApp::GetLog() << (olxstr("Rc=") << R/cc);
  }
  else  {  // just save the result
    TStrPObjList<olxstr, TPSTypeList<int, int>* > data;
    data.Add("xyz", &testdai.XYZ);
    data.Add("xx", &testdai.XX);
    data.Add("xy", &testdai.XY);
    data.Add("xz", &testdai.XZ);
    data.Add("yy", &testdai.YY);
    data.Add("yz", &testdai.YZ);
    data.Add("zz", &testdai.ZZ);
    for( int i=0; i < data.Count(); i++ )  {
      TCStrList sl;
      TPSTypeList<int, int>& d = *data.Object(i);
      sl.SetCapacity( d.Count() );
      for( int j=0; j < d.Count(); j++ )  {
        sl.Add( (double)d.GetComparable(j)/100 ) << '\t' << d.GetObject(j);
      }
      sl.SaveToFile( (Cmds[0]+data[i]) << ".xlt");
    }
  }
}
//..............................................................................
void TMainForm::macExportFont(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TwxGlScene* wxs = dynamic_cast<TwxGlScene*>(FXApp->GetRender().Scene());
  if( wxs == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "invalid scene object");
    return;
  }
  wxs->ExportFont(Cmds[0], Cmds[1]);
}
//..............................................................................
void TMainForm::macImportFont(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TwxGlScene* wxs = dynamic_cast<TwxGlScene*>(FXApp->GetRender().Scene());
  TGlFont* glf = wxs->FindFont( Cmds[0] );
  if( glf == NULL )  {
    E.ProcessingError(__OlxSrcInfo, olxstr("undefined font ") << Cmds[0]);
    return;
  }
  wxs->ImportFont(Cmds[0], Cmds[1]);
}
