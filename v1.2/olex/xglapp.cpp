//----------------------------------------------------------------------------//
// TGlApp implementation
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "xglapp.h"
#include "xglcanv.h"
#include "mainform.h"
#include "wx/progdlg.h"
#include "wx/datetime.h"
#include "wx/tooltip.h"

#include "ins.h"
#include "mol.h"
#include "cif.h"
#include "xyz.h"
#include "p4p.h"
#include "crs.h"
#include "pdb.h"
#include "xdmas.h"
#include "datafile.h"

#include "efile.h"
#ifndef __WIN32__
#include "icons/olex2.xpm"
#endif

TGlXApp* TGlXApp::Instance = NULL;

class TProgress: public AActionHandler  {
  wxProgressDialog *Progress;
  int Max, Start;
  wxDateTime StartTime;
  bool ShowDialog;
public:
  TProgress()  {
    Progress = NULL;
    Start = Max = 0;
    ShowDialog = false;
    return;
  }
  virtual ~TProgress()  {  if( Progress )    delete Progress;  }
  bool Enter(const IEObject *Sender, const IEObject *Data)  {
    StartTime = wxDateTime::UNow();
    Start = 0;
    return false;
  }
  bool Exit(const IEObject *Sender, const IEObject *Data)  {
    if( Progress )  {
      delete Progress;
      Progress = NULL;
      Max = 0;
    }                                    
    return false;
  }
  bool Execute(const IEObject *Sender, const IEObject *Data)  {
    if( !EsdlInstanceOf(*Data, TOnProgress) )
      return false;
    IEObject* p_d = const_cast<IEObject*>(Data);
    TOnProgress *A = dynamic_cast<TOnProgress*>(p_d);

    if( (Progress == NULL) && ShowDialog )  {
      Progress = new wxProgressDialog(wxT("Progress"), wxT(""), (int)A->GetMax() );
      Max = (int)A->GetMax(); // remember to recreate the dialog when it changes, ...
      Start = (int)A->GetPos();
    }
    if( A->GetMax() != Max )  {
      if( Progress != NULL )
        delete Progress;
      if( ShowDialog )
        Progress = new wxProgressDialog(wxT("Progress"), wxT(""), (int)A->GetMax());
      Max = (int)A->GetMax(); // remember to recreate the dialog when it changes, ...
      StartTime = wxDateTime::UNow();
    }
    if( A->GetPos() > (Start + A->GetMax()*0.001) )  {  // if more than 5 seconds
      if( (wxDateTime::UNow() - StartTime).GetSeconds() > 1 )  {
        ShowDialog = true;
        if( Progress == NULL )
          Progress = new wxProgressDialog(wxT("Progress"), wxT(""), (int)A->GetMax());
        Progress->Show();
      }
      Start = (int)A->GetMax();
    }
    if( Progress != NULL )  {
      wxSize ds = Progress->GetSize();
      ds.SetWidth(400);
      Progress->SetSize( ds );

      // this is to prevent wxWidgets popping an assertion  dialogue
      if( A->GetPos() > A->GetMax() )
        A->SetPos( A->GetMax() );

      Progress->Update((int)A->GetPos(), uiStr(A->GetAction()) );
    }
    return false;
  }
};

//----------------------------------------------------------------------------//
// TGlApp function bodies
//----------------------------------------------------------------------------//
//..............................................................................
BEGIN_EVENT_TABLE(TGlXApp, wxApp)
    EVT_IDLE(TGlXApp::OnIdle)
END_EVENT_TABLE()

//..............................................................................
bool TGlXApp::OnInit()  {

  Instance = this;
//  wxToolTip::Enable(true);
  int ScreenW = wxSystemSettings::GetMetric(wxSYS_SCREEN_X),
      ScreenH = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y);
  wxPoint wTop(0, 0);
  wxSize wSize(ScreenW, ScreenH);

  MainForm = new TMainForm(this, ScreenW, ScreenH);
#ifdef __WIN32__
  MainForm->SetIcon(wxIcon(wxT("MAINICON")));
#else
  MainForm->SetIcon(wxIcon(mainicon_xpm));
#endif

  /*
  glEnable(GL_POINT_SMOOTH);
  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_POLYGON_SMOOTH);
  glHint(GL_POINT_SMOOTH_HINT,GL_NICEST);
  glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
  glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
  */

  MainForm->GlCanvas(new TGlCanvas(MainForm, wxID_ANY, wTop, wxDefaultSize, 0, wxT("GL_CANVAS") ) );
  // cratea an instance of the XApplication
  olxstr BaseDir = argv[0], Tmp;
  if( !TEFile::IsAbsolutePath(BaseDir) )
    BaseDir = TEFile::AddTrailingBackslash( TEFile::CurrentDir() );
  try  {
   //asm{  int 3 }
    #if defined __APPLE__ && defined __MACH__
//      BaseDir = TEFile::ExtractFilePath(BaseDir);
//      BaseDir = TEFile::ParentDir(BaseDir);
//      BaseDir << "Resources/";
    #endif
    XApp = new TGXApp( BaseDir );
    //XApp = new TGXApp( TEFile::UNCFileName(BaseDir) );
  }
  catch( TExceptionBase& exc )  {
    TStrList out;
    exc.GetException()->GetStackTrace(out);
    ::wxMessageBox( uiStr(out.Text('\n')) += wxT('\n'),
      uiStrT("Exception: ") += uiStrT(EsdlObjectNameT(exc)), wxOK|wxICON_ERROR);
    throw;
  }
  // asseble whole command line
  for( int i=1; i < argc; i++ )
    Tmp << argv[i] <<  ' ';
  TParamList::StrtokParams(Tmp, ' ', XApp->Arguments);
  for( int i=0; i < XApp->Arguments.Count(); i++ )  {
    if( XApp->Arguments.String(i).FirstIndexOf('=') != -1 )  {
      XApp->ParamList.FromString(XApp->Arguments.String(i), '=');
      XApp->Arguments.Delete(i);
      i--;
    }
  }
  TProgress *P = new TProgress;
  XApp->OnProgress->Add(P);

  TCif *Cif = new TCif(XApp->AtomsInfo());  // the objects will be automatically removed by the XApp
  XApp->RegisterXFileFormat(Cif, "cif");
  XApp->RegisterXFileFormat(Cif, "fcf");
  XApp->RegisterXFileFormat(Cif, "fco");
  TMol *Mol = new TMol(XApp->AtomsInfo());  // the objects will be automatically removed by the XApp
  XApp->RegisterXFileFormat(Mol, "mol");
  TIns *Ins = new TIns(XApp->AtomsInfo());
  XApp->RegisterXFileFormat(Ins, "ins");
  XApp->RegisterXFileFormat(Ins, "res");
  TXyz *Xyz = new TXyz(XApp->AtomsInfo());
  XApp->RegisterXFileFormat(Xyz, "xyz");
  XApp->RegisterXFileFormat(new TP4PFile(), "p4p");
  XApp->RegisterXFileFormat(new TCRSFile(), "crs");
  XApp->RegisterXFileFormat(new TPdb(XApp->AtomsInfo()), "pdb");
  XApp->RegisterXFileFormat(new TXDMas(XApp->AtomsInfo()), "mas");

  // set backgrownd color of the GlRender
  XApp->ClearColor(0x3f3f3f);
  MainForm->SetToolTip(wxT("\n")); // force multiline ttoltips with (&#10;)
  try  {
    MainForm->XApp(XApp);  // his sets XApp for the canvas as well
  }
  catch( TExceptionBase& exc )  {
    TStrList out;
    exc.GetException()->GetStackTrace(out);
    ::wxMessageBox( uiStr(out.Text('\n')) += wxT('\n'),
      uiStrT("Exception: ") += uiStrT(EsdlObjectNameT(exc)), wxOK|wxICON_ERROR);
  }
  SetTopWindow(MainForm);
  MainForm->Show(true);
  MainForm->Maximize(true);
  return true;
}
//..............................................................................
int TGlXApp::OnExit()  {
  delete XApp;
  return 0;
}
//..............................................................................
bool TGlXApp::Dispatch()  {
  return wxApp::Dispatch();
}
//..............................................................................
//int TGlXApp::MainLoop()  {
//  while( wxApp::Pending() )  wxApp::Dispatch();
  //TBasicApp::GetInstance()->OnTimer->Clear();
//  return wxApp::MainLoop();
//}
//..............................................................................
void TGlXApp::OnIdle(wxIdleEvent& event)  {
  wxApp::OnIdle( event );
}
//..............................................................................

IMPLEMENT_APP(TGlXApp)
