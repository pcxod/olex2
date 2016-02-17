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

#ifdef _CUSTOM_BUILD_
  #include "custom_base.h"
#endif

class TProgress : public AActionHandler {
  wxProgressDialog *Progress;
  uint64_t start, p_max;
public:
  TProgress() : Progress(NULL)
  {}
  virtual ~TProgress() {
    if (Progress != NULL)
      Progress->Destroy();
  }
  bool Enter(const IOlxObject *, const IOlxObject *, TActionQueue *) {
    start = TETime::msNow();
    return false;
  }
  bool Exit(const IOlxObject *Sender, const IOlxObject *Data, TActionQueue *) {
    if (Progress) {
      Progress->Destroy();
      Progress = NULL;
    }
    return false;
  }
  bool Execute(const IOlxObject *Sender, const IOlxObject *Data, TActionQueue *) {
    const TOnProgress *A = dynamic_cast<const TOnProgress*>(Data);
    if (A == NULL || A->GetPos() == 0) {
      return false;
    }
    if (Progress == NULL) {
      double tat = (double)A->GetPos()*(TETime::msNow() - start) / A->GetMax();
      if (tat*A->GetMax() < 30000) { // max time 30 sec
        return false;
      }
      p_max = A->GetMax();
      Progress = new wxProgressDialog(wxT("Progress"), wxT(""), (int)A->GetMax());
    }
    else if (A->GetMax() != p_max) {
      Progress->Destroy();
      Progress = new wxProgressDialog(wxT("Progress"), wxT(""), (int)A->GetMax());
      p_max = A->GetMax();
    }
    if (Progress != NULL) {
      //wxSize ds = Progress->GetSize();
      //ds.SetWidth(400);
      //Progress->SetSize(ds);
      if (A->GetPos() < A->GetMax()) {
        Progress->Update((int)A->GetPos(), A->GetAction().u_str());
      }
    }
    return false;
  }
};


//----------------------------------------------------------------------------//
// TGlApp function bodies
//----------------------------------------------------------------------------//
//..............................................................................
bool TGlXApp::OnInit() {
  setlocale(LC_NUMERIC, "C");
  wxApp::SetAppName(wxT("olex2"));
  Instance() = this;
  TEGC::Initialise();  // prepare Olex2 API...
  TSocketFS::SetUseLocalFS(true);
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
#ifdef _CUSTOM_BUILD_
  if (!CustomCodeBase::OnStartup())
#endif
    {
      XApp->SetSharedDir(patcher::PatchAPI::GetSharedDir());
      XApp->SetInstanceDir(patcher::PatchAPI::GetInstanceDir());
      olxstr config_dir = olx_getenv("OLEX2_CONFIGDIR");
      if (!config_dir.IsEmpty())
        XApp->SetConfigdDir(config_dir);
    }
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
  {
    olxstr cctbx_env = XApp->GetBaseDir() + "cctbx/cctbx_build/libtbx_env";
    if (!TEFile::Exists(cctbx_env) && !TBasicApp::IsBaseDirWriteable()) {
      TMainForm::ShowAlert("Please run Olex2 as administrator on the first run."
        "\nPyhton modules need to be precompiled.", "Error", wxOK | wxICON_ERROR);
      OnExit();
      return false;
    }
  }
  MainForm = new TMainForm(this);
  XApp->InitOlex2App();
  /* This generic progress box requires more thinking in particular considering
  the paralellalised tasks
  */
  //XApp->OnProgress.Add(new TProgress);
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
#ifdef __WIN32__
  MainForm->SetIcon(wxIcon(wxT("MAINICON")));
#else
  MainForm->SetIcon(wxIcon(mainicon_xpm));
#endif
#ifdef __WIN32__  // on LInux they are multiline by default...
  MainForm->SetToolTip(wxT("\n")); // force multiline tooltips with (&#10;)
#endif
  olxstr str_glAttr = olx_getenv("OLEX2_GL_DEFAULT"),
    str_glStereo = olx_getenv("OLEX2_GL_STEREO"),
    str_glMultisampling = olx_getenv("OLEX2_GL_MULTISAMPLE"),
    str_glDepth = olx_getenv("OLEX2_GL_DEPTH_BITS");
#if defined(__WIN32__) && 0
  olxstr str_glDefStereo = TrueString(),
    str_glDefMultisampling = TrueString();
#else
  olxstr str_glDefStereo = FalseString(),
    str_glDefMultisampling = FalseString();
#endif
  short depth_bits = str_glDepth.IsInt() ? str_glDepth.ToInt() : 24;
  olx_array_ptr<int> gl_attr = TGlCanvas::GetGlAttributes(
    !str_glAttr.IsEmpty() && str_glAttr.Equalsi("TRUE"),
    XApp->GetOptions().FindValue(
    "gl_stereo", str_glStereo.IsEmpty() ? str_glDefStereo : str_glStereo).ToBool(),
    XApp->GetOptions().FindValue(
    "gl_multisample", str_glMultisampling.IsEmpty() ? str_glDefMultisampling
    : str_glMultisampling).ToBool(),
    depth_bits
    );
  MainForm->GlCanvas(new TGlCanvas(
    MainForm, gl_attr, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0,
    wxT("GL_CANVAS")));
  try {
    MainForm->XApp(XApp);
  }  // his sets XApp for the canvas as well
  catch(const TExceptionBase& e)  {
    TMainForm::ShowAlert(e);
  }
  SetTopWindow(MainForm);
  //MainForm->Maximize(true);
  Bind(OLX_COMMAND_EVT, &TGlXApp::OnCmd, this);
  Bind(wxEVT_IDLE, &TGlXApp::OnIdle, this);
  Bind(wxEVT_CHAR, &TGlXApp::OnChar, this);
  Bind(wxEVT_KEY_DOWN, &TGlXApp::OnKeyDown, this);
  Bind(wxEVT_NAVIGATION_KEY, &TGlXApp::OnNavigation, this);
  MainForm->Show(true);
  return true;
}
//..............................................................................
int TGlXApp::OnExit() {
  // do all operations before TEGC is deleted
  if( pid_file != NULL )  {
    pid_file->Delete();
    delete pid_file;
    pid_file = NULL;
  }
  olxstr conf_dir = XApp->GetInstanceDir();
  TStrList pid_files = TEFile::ListDir(conf_dir, olxstr("*.") <<
    patcher::PatchAPI::GetOlex2PIDFileExt(), sefFile);
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
  if (TMainForm::HasInstance()) {
    MainForm->Destroy();
  }
  delete XApp;
  return 0;
}
//..............................................................................
bool TGlXApp::Dispatch() {
  return wxApp::Dispatch();
}
//..............................................................................
void TGlXApp::OnChar(wxKeyEvent& event)  {
  if (event.GetKeyCode() == 9) {
    wxComboBox *wnd = dynamic_cast<wxComboBox *>(MainForm->FindFocus());
    if (wnd != NULL) {
      event.Skip(false);
      wxWindow *nw = (event.GetModifiers() == wxMOD_SHIFT) ?
        wnd->GetPrevSibling() : wnd->GetNextSibling();
      if (nw != 0) {
        nw->SetFocus();
        return;
      }
    }
  }
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
  if (GetMainForm()->idle_start == 0)
    GetMainForm()->idle_start = TETime::msNow();
  GetMainForm()->OnIdle();
}
//..............................................................................
void TGlXApp::OnCmd(olxCommandEvent &evt) {
  TStrList toks(evt.GetCommand(), ">>");
  for (size_t i = 0; i < toks.Count(); i++) {
    if (!MainForm->processMacro(olxstr::DeleteSequencesOf(toks[i], ' '))) {
      break;
    }
  }
}
//..............................................................................
void TGlXApp::MacOpenFile(const wxString &fileName) {
  try {
    GetMainForm()->processMacro(olxstr("reap ").quote('"') << fileName);
  }
  catch (const TExceptionBase &e) {
    TBasicApp::NewLogEntry(logException) << e;
  }
}

IMPLEMENT_APP(TGlXApp)
