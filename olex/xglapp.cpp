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
  TProgress() : Progress(0)
  {}
  virtual ~TProgress() {
    if (Progress != 0) {
      Progress->Destroy();
    }
  }
  bool Enter(const IOlxObject *, const IOlxObject *, TActionQueue *) {
    start = TETime::msNow();
    return false;
  }
  bool Exit(const IOlxObject *Sender, const IOlxObject *Data, TActionQueue *) {
    if (Progress) {
      Progress->Destroy();
      Progress = 0;
    }
    return false;
  }
  bool Execute(const IOlxObject *Sender, const IOlxObject *Data, TActionQueue *) {
    const TOnProgress *A = dynamic_cast<const TOnProgress*>(Data);
    if (A == 0 || A->GetPos() == 0) {
      return false;
    }
    if (Progress == 0) {
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
    if (Progress != 0) {
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

#ifdef _WIN32
struct FileQuery {
  TStrList and_toks, or_toks;
  HWND wnd;

  FileQuery(const TStrList& and_toks, const TStrList or_toks)
  : and_toks(and_toks), or_toks(or_toks), wnd(0)
  {}
};

BOOL CALLBACK EnumWindowsFunc(HWND w, LPARAM p) {
  size_t max_sz = 256;
  olx_array_ptr<wchar_t> title(max_sz);
  int sz = GetWindowText(w, &title, max_sz - 1);
  FileQuery* q = (FileQuery*)p;
  if (sz >= 0) {
    olxstr t = olxstr::FromExternal(title.release(), sz, max_sz);
    for (size_t i = 0; i < q->and_toks.Count(); i++) {
      if (!t.Containsi(q->and_toks[i])) {
        return TRUE;
      }
    }
    if (q->or_toks.IsEmpty()) {
      q->wnd = w;
      return FALSE;
    }
    for (size_t i = 0; i < q->or_toks.Count(); i++) {
      if (t.Containsi(q->or_toks[i])) {
        q->wnd = w;
        return FALSE;
      }
    }
  }
  return TRUE;
}
#endif

//----------------------------------------------------------------------------//
// TGlApp function bodies
//----------------------------------------------------------------------------//
//..............................................................................
bool TGlXApp::OnInit() {
  wxLocale wx_lc(wxLANGUAGE_ENGLISH);
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
    {
      olxstr af_log = olx_getenv("OLEX2_AUTOFLUSH_LOG");
      if (af_log.IsBool()) {
        XApp->GetLog().SetAutoFlush(af_log.ToBool());
      }
    }
#ifdef _CUSTOM_BUILD_
  if (!CustomCodeBase::OnStartup())
#endif
    {
      XApp->SetSharedDir(patcher::PatchAPI::GetSharedDir());
      XApp->SetInstanceDir(patcher::PatchAPI::GetInstanceDir());
      olxstr config_dir = olx_getenv("OLEX2_CONFIGDIR");
      if (!config_dir.IsEmpty()) {
        XApp->SetConfigdDir(config_dir);
      }
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
        "\nPython modules need to be precompiled.", "Error", wxOK | wxICON_ERROR);
      OnExit();
      return false;
    }
  }
#ifdef WIN32
  if (XApp->GetArguments().Count() >= 2) {
    if (!XApp->GetArguments().GetLastString().EndsWith(".py") &&
      !XApp->GetArgOptions().GetBoolOption("-new"))
    {
      HWND wnd = FindWindow("Olex2", XApp->GetArguments().Text(' ', 1));
      if (wnd != 0) {
        TGlXApp::ActivateWindow(wnd);
        OnExit();
        return false;
      }
    }
  }
#endif
  MainForm = new TMainForm(this);
  XApp->InitOlex2App();
  /* This generic progress box requires more thinking in particular considering
  the paralellalised tasks
  */
  //XApp->OnProgress.Add(new TProgress);
  TCif *Cif = new TCif;  // the objects will be automatically removed by the XApp
  XApp->RegisterXFileFormat(Cif, "cif");
  XApp->RegisterXFileFormat(Cif, "cfx_LANA");
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
  GLboolean stereo_supported = GL_FALSE;
// causes segmentation fault!
#if !defined(__MAC__)
  olx_gl::get(GL_STEREO, &stereo_supported);
#endif
#if defined(__WIN32__)
  olxstr str_glDefStereo = stereo_supported ? TrueString() : FalseString(),
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
  /* need to set correct locale for Linux to deal with the file paths
   however not all of them like it - Suse crashes the program straight off
   disregarding the choice (enUS.UTF8, C or whatever), so making this upto the
   user
  */
  {
    olxstr lc = XApp->GetOptions().FindValue("locale.ctype", EmptyString());
    if (!lc.IsEmpty()) {
      olxcstr old = setlocale(LC_CTYPE, lc.c_str());
      TBasicApp::NewLogEntry(logInfo) << "Changing locale from '" << old
        << "' to " << lc;
    }
  }
  MainForm->Show(true);
  return true;
}
//..............................................................................
TGlXApp::~TGlXApp() {
}
//..............................................................................
int TGlXApp::OnExit() {
  // do all operations before TEGC is deleted
  if (pid_file != 0) {
    pid_file->Delete();
    delete pid_file;
    pid_file = NULL;
  }
  olxstr conf_dir = XApp->GetInstanceDir();
  TStrList pid_files = TEFile::ListDir(conf_dir, olxstr("*.") <<
    patcher::PatchAPI::GetOlex2PIDFileExt(), sefFile);
#ifdef __linux__
  size_t ext_len = olxstr::o_strlen(patcher::PatchAPI::GetOlex2PIDFileExt()) + 1;
#endif
  for (size_t i = 0; i < pid_files.Count(); i++) {
#ifdef __linux__
    if (ext_len >= pid_files[i].Length()) continue;
    olxstr spid = pid_files[i].SubStringTo(pid_files[i].Length() - ext_len);
    if (spid.IsInt()) {
      int pid = spid.ToInt();
      if (kill(pid, 0) == 0) {
        continue;
      }
    }
#endif
    TEFile::DelFile(conf_dir + pid_files[i]);
  }
  TStrList ready_files = TEFile::ListDir(conf_dir, "*.ready", sefFile);
  for (size_t i = 0; i < ready_files.Count(); i++) {
    TEFile::DelFile(conf_dir + ready_files[i]);
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
void TGlXApp::OnChar(wxKeyEvent& event) {
  if (event.GetKeyCode() == 9) {
    wxComboBox *wnd = dynamic_cast<wxComboBox *>(MainForm->FindFocus());
    if (wnd != 0) {
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
  if (GetMainForm()->idle_start == 0) {
    GetMainForm()->idle_start = TETime::msNow();
  }
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
//..............................................................................
#ifdef _WIN32
HWND TGlXApp::FindWindow(const TStrList& and_toks, const TStrList& or_toks) {
  FileQuery qp(and_toks, or_toks);
  EnumDesktopWindows(0, &EnumWindowsFunc, (LPARAM)&qp);
  return qp.wnd;
}
//..............................................................................
bool TGlXApp::ActivateWindow(HWND wnd) {
  if (IsIconic(wnd)) {
    // OpenIcon changes the window size to "normal" even if it was maximized before
    //if (!OpenIcon(wnd)) {
    //  return false;
    //}
    // force maximized as there seems to be no way to restore it properly
    ShowWindow(wnd, SW_MAXIMIZE);
  }
  return SetForegroundWindow(wnd);
}
#endif

IMPLEMENT_APP(TGlXApp)
