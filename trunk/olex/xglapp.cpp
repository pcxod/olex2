/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

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
#include "oxmfile.h"
#include "mol2.h"
#include "datafile.h"
#include "wxzipfs.h"
#include "shellutil.h"
#include "patchapi.h"
#include "cdsfs.h"
#include "efile.h"
#include "wxzipfs.h"
#ifndef __WIN32__
  #include "icons/olex2.xpm"
  #include <unistd.h>
#else
  #include <process.h>
#endif

#ifdef __linux__
#include <signal.h>
#endif

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
  bool Enter(const IEObject *Sender, const IEObject *Data, TActionQueue *)  {
    StartTime = wxDateTime::UNow();
    Start = 0;
    return false;
  }
  bool Exit(const IEObject *Sender, const IEObject *Data, TActionQueue *)  {
    if( Progress )  {
      delete Progress;
      Progress = NULL;
      Max = 0;
    }
    return false;
  }
  bool Execute(const IEObject *Sender, const IEObject *Data, TActionQueue *)  {
    return false; // not good for now...
    if( !EsdlInstanceOf(*Data, TOnProgress) )
      return false;
    if( Sender != NULL && EsdlInstanceOf(*Data, TwxZipFileSystem) )
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
      Progress->SetSize(ds);

      // this is to prevent wxWidgets popping an assertion  dialogue
      if( A->GetPos() > A->GetMax() )
        A->SetPos(A->GetMax());

      Progress->Update((int)A->GetPos(), A->GetAction().u_str());
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
  EVT_CHAR(TGlXApp::OnChar)
  EVT_KEY_DOWN(TGlXApp::OnKeyDown)
  EVT_NAVIGATION_KEY(TGlXApp::OnNavigation)
END_EVENT_TABLE()

//..............................................................................
bool TGlXApp::OnInit()  {
  setlocale(LC_NUMERIC, "C");
  wxApp::SetAppName(wxT("olex2"));
  Instance() = this;
//  wxToolTip::Enable(true);
  int ScreenW = wxSystemSettings::GetMetric(wxSYS_SCREEN_X),
      ScreenH = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y);
  wxPoint wTop(0, 0);
  wxSize wSize(ScreenW, ScreenH);
  TSocketFS::SetUseLocalFS(true);
  MainForm = new TMainForm(this);
  TEGC::Initialise();  // prepare Olex2 API...
#ifdef __WIN32__
  MainForm->SetIcon(wxIcon(wxT("MAINICON")));
#else
  MainForm->SetIcon(wxIcon(mainicon_xpm));
#endif
  // register wxWidgets to handle ZIP files
  TwxZipFileSystem::RegisterFactory();
  // create an instance of the XApplication
  olxstr BaseDir(argv[0]);
  // 2008.09.29
  // see if the system variable OLEX2_DIR is defined to override the default basedir
  try  {
    #if defined __APPLE__ && defined __MACH__
//      BaseDir = TEFile::ExtractFilePath(BaseDir);
//      BaseDir = TEFile::ParentDir(BaseDir);
//      BaseDir << "Resources/";
    #endif
    olxstr base_dir_x = TBasicApp::GuessBaseDir(BaseDir, "OLEX2_DIR");
    olxstr base_dir = TEFile::AddPathDelimeter(
      TEFile::ExtractFilePath(base_dir_x));
  olx_setenv("PATH", TEFile::TrimPathDelimeter(base_dir) << olx_env_sep()
    << olx_getenv("PATH"));
#if defined(_WIN64) && defined(_DEBUG)
    XApp = new Olex2App(TBasicApp::GuessBaseDir(BaseDir, "OLEX2_DEBUG_DIR"));
#else
    XApp = new Olex2App(TBasicApp::GuessBaseDir(BaseDir, "OLEX2_DIR"));
#endif
    XApp->SetSharedDir(patcher::PatchAPI::GetSharedDir());
    XApp->SetInstanceDir(patcher::PatchAPI::GetInstanceDir());
    olxstr config_dir = olx_getenv("OLEX2_CONFIGDIR");
    if (!config_dir.IsEmpty())
      XApp->SetConfigdDir(config_dir);
    XApp->ReadOptions(XApp->GetConfigDir() + ".options");
    XApp->CleanupLogs();
    XApp->CreateLogFile(XApp->GetOptions().FindValue("log_name", "olex2"));
  }
  catch(const TExceptionBase& e)  {
    TMainFrame::ShowAlert(e);
    throw;
  }
  // write PID file
  try {
    int pid = getpid();
    pid_file = new TEFile(olxstr(XApp->GetInstanceDir()) << pid << '.' <<
      patcher::PatchAPI::GetOlex2PIDFileExt(),
     "w+b");
  }
  catch(const TExceptionBase &e) {
    TBasicApp::NewLogEntry(logException) << e;
  }
  XApp->InitArguments<wxChar>(argc, argv);
  XApp->InitOlex2App();
  TProgress *P = new TProgress;
  XApp->OnProgress.Add(P);

  TCif *Cif = new TCif;  // the objects will be automatically removed by the XApp
  XApp->RegisterXFileFormat(Cif, "cif");
  XApp->RegisterXFileFormat(Cif, "cmf");
  XApp->RegisterXFileFormat(Cif, "fcf");
  XApp->RegisterXFileFormat(Cif, "fco");
  XApp->RegisterXFileFormat(new TMol, "mol");
  TIns *Ins = new TIns;
  XApp->RegisterXFileFormat(Ins, "ins");
  XApp->RegisterXFileFormat(Ins, "res");
  XApp->RegisterXFileFormat(new TXyz, "xyz");
  XApp->RegisterXFileFormat(new TP4PFile, "p4p");
  XApp->RegisterXFileFormat(new TCRSFile, "crs");
  XApp->RegisterXFileFormat(new TPdb, "pdb");
  XApp->RegisterXFileFormat(new TXDMas, "mas");
  XApp->RegisterXFileFormat(new TOXMFile(*XApp), "oxm");
  XApp->RegisterXFileFormat(new TMol2, "mol2");

  // set backgrownd color of the GlRender
  XApp->ClearColor(0x3f3f3f);
#ifdef __WIN32__  // on LInux they are multiline by default...
  MainForm->SetToolTip(wxT("\n")); // force multiline ttoltips with (&#10;)
#endif
  olxstr str_glAttr = olx_getenv("OLEX2_GL_DEFAULT"),
    str_glStereo = olx_getenv("OLEX2_GL_STEREO"),
    str_glDepth = olx_getenv("OLEX2_GL_DEPTH_BITS");
  short depth_bits = str_glDepth.IsInt() ? str_glDepth.ToInt() : 24;
  olx_array_ptr<int> gl_attr = TGlCanvas::GetGlAttributes(
    !str_glAttr.IsEmpty() && str_glAttr.Equalsi("TRUE"),
    XApp->GetOptions().FindValue(
    "gl_stereo", str_glStereo.IsEmpty() ? TrueString() : str_glStereo).ToBool(),
    XApp->GetOptions().FindValue(
    "gl_multisample", TrueString()).ToBool(),
    depth_bits
    );
  MainForm->GlCanvas(new TGlCanvas(
    MainForm, gl_attr, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0,
    wxT("GL_CANVAS")));
  try  { MainForm->XApp(XApp); }  // his sets XApp for the canvas as well
  catch(const TExceptionBase& e)  {
    TMainForm::ShowAlert(e);
  }
  SetTopWindow(MainForm);
  //MainForm->Maximize(true);
  MainForm->Show(true);
  return true;
}
//..............................................................................
int TGlXApp::OnExit()  {
  // do all operations before TEGC is deleted
  if( pid_file != NULL )  {
    pid_file->Delete();
    delete pid_file;
    pid_file = NULL;
  }
  TStrList pid_files;
  olxstr conf_dir = XApp->GetInstanceDir(); 
  TEFile::ListDir(conf_dir, pid_files, olxstr("*.") <<
    patcher::PatchAPI::GetOlex2PIDFileExt(), sefAll);
#ifdef __linux__
    size_t ext_len = olxstr::o_strlen(patcher::PatchAPI::GetOlex2PIDFileExt())+1;
#endif
  for (size_t i=0; i < pid_files.Count(); i++) {
#ifdef __linux__
    if (ext_len >= pid_files[i].Length()) continue;
    olxstr spid = pid_files[i].SubStringTo(pid_files[i].Length()-ext_len);
    if (spid.IsInt()) {
       int pid = spid.ToInt();
       if (kill(pid, 0) == 0)
         continue;
    }
#endif
    TEFile::DelFile(conf_dir+pid_files[i]);
  }
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
  //TBasicApp::GetInstance().OnTimer->Clear();

//  return wxApp::MainLoop();
//}
//..............................................................................
void TGlXApp::OnChar(wxKeyEvent& event)  {
  //MainForm->OnChar(event);
  event.Skip(); // pass it to the controls...
}
//..............................................................................
void TGlXApp::OnKeyDown(wxKeyEvent& event)  {
  MainForm->OnKeyDown(event);
}
//..............................................................................
void TGlXApp::OnNavigation(wxNavigationKeyEvent& event)  {
  MainForm->OnNavigation(event);
}
//..............................................................................
void TGlXApp::OnIdle(wxIdleEvent& event)  {
  event.Skip();
}
//..............................................................................

IMPLEMENT_APP(TGlXApp)
