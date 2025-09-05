/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#define this_InitFunc(funcName, argc) \
  Library.Register( new TFunction<TMainForm>(this, &TMainForm::fun##funcName, #funcName, argc))
#define this_InitMacro(macroName, validOptions, argc)\
  Library.Register( new TMacro<TMainForm>(this, &TMainForm::mac##macroName, #macroName, #validOptions, argc))
#define this_InitMacroA(realMacroName, macroName, validOptions, argc)\
  Library.Register( new TMacro<TMainForm>(this, &TMainForm::mac##realMacroName, #macroName, #validOptions, argc))
#define this_InitFuncD(funcName, argc, desc) \
  Library.Register( new TFunction<TMainForm>(this, &TMainForm::fun##funcName, #funcName, argc, desc))
#define this_InitMacroD(macroName, validOptions, argc, desc)\
  Library.Register( new TMacro<TMainForm>(this, &TMainForm::mac##macroName, #macroName, (validOptions), argc, desc))
#define this_InitMacroAD(realMacroName, macroName, validOptions, argc, desc)\
  Library.Register( new TMacro<TMainForm>(this, &TMainForm::mac##realMacroName, #macroName, (validOptions), argc, desc))

#include "mainform.h"
#include "xglcanv.h"
#include "xglapp.h"
#include "wxglscene.h"

#include "wx/utils.h"
#include "wx/wxhtml.h"
#include "wx/image.h"
#include "wx/panel.h"
#include "wx/fontdlg.h"
#include "wx/tooltip.h"
#include "wx/clipbrd.h"
#include "wx/dynlib.h"

#include "gpcollection.h"
#include "glgroup.h"
#include "glbackground.h"
#include "glcursor.h"

#include "dbasis.h"
#include "dunitcell.h"
#include "xatom.h"
#include "xbond.h"
#include "xplane.h"
#include "xgrowline.h"
#include "xgrowpoint.h"
#include "symmparser.h"
#include "xreflection.h"
#include "xline.h"
#include "xangle.h"

#include "ins.h"
#include "cif.h"
#include "xyz.h"

#include "efile.h"

#include "html/htmlmanager.h"

#include "httpfs.h"
#include "pyext.h"

#include "olxstate.h"
#include "glbitmap.h"
#include "log.h"
#include "cmdline.h"

#include "shellutil.h"
#include "fsext.h"
#include "etime.h"

#include "egc.h"
#include "gllabel.h"
#include "gllabels.h"
#include "xlattice.h"
#include "xgrid.h"

#include "olxvar.h"
#include "edit.h"

#include "xmacro.h"
#include "utf8file.h"
#include "py_core.h"
#include "updateth.h"
#include "msgbox.h"
#include "updateapi.h"
#include "patchapi.h"
#include "hkl_py.h"
#include "filetree.h"
#include "md5.h"
#include "olxth.h"
#include "lcells.h"
#include "label_corrector.h"
#include "dusero.h"
#include "exparse/exptree.h"
#include "math/libmath.h"
#include "libfile.h"
#include "libstr.h"
#include "gxmacro.h"
#include "auto.h"
#include "py_reg.h"

#ifdef _CUSTOM_BUILD_
  #include "custom_base.h"
#endif

#include "refinement_listener.h"

#ifdef __GNUC__
  #undef Bool
#endif

#if !wxCHECK_VERSION(2,9,0)
#ifndef WXK_RAW_CONTROL
# define WXK_RAW_CONTROL WXK_CONTROL
#endif

#ifndef wxMOD_RAW_CONTROL
#define wxMOD_RAW_CONTROL wxMOD_CONTROL
#endif
#endif

  IMPLEMENT_CLASS(TMainForm, TMainFrame)

const olxstr ProcessOutputCBName("procout");
const olxstr OnStateChangeCBName("statechange");
const olxstr OnLogCBName("onlog");

class TObjectVisibilityChange : public AActionHandler {
  TMainForm *FParent;
public:
  TObjectVisibilityChange(TMainForm *Parent) { FParent = Parent; }
  virtual ~TObjectVisibilityChange() { ; }
  bool Execute(const IOlxObject *Sender, const IOlxObject *Obj, TActionQueue *) {
    if (Obj == 0) {
      return false;
    }
    if (Obj->Is<TDBasis>()) {
      FParent->BasisVChange();
    }
    else if (Obj->Is<TDUnitCell>()) {
      FParent->CellVChange();
    }
    else  if (Obj == FParent->FInfoBox) {
      FParent->processMacro("showwindow info false");
    }
    else  if (Obj == FParent->FHelpWindow) {
      FParent->processMacro("showwindow help false");
    }
    else if (Obj->Is<TXGrid>()) {
      FParent->GridVChange();
    }
    else if (Obj->Is<T3DFrameCtrl>()) {
      FParent->FrameVChange();
    }
    return true;
  }
};
/******************************************************************************/
// TMainForm function bodies
//..............................................................................
TMainForm::TMainForm(TGlXApp *Parent)
  : TMainFrame(wxT("Olex2"), wxPoint(0,0), wxDefaultSize, wxT("MainForm")),
  olex2::OlexProcessorImp(NULL),
  HtmlManager(*(new THtmlManager(this))),
  _ProcessHandler(*this)
{
#ifdef _WIN32
  void InitCommonControls();
#endif // _WIN32

  //Bindings
  {
    Bind(wxEVT_SIZE, &TMainForm::OnSize, this);
    Bind(wxEVT_MOVE, &TMainForm::OnMove, this);
    Bind(wxEVT_CLOSE_WINDOW, &TMainForm::OnCloseWindow, this);

    Bind(wxEVT_MENU, &TMainForm::OnHtmlPanel, this, ID_HtmlPanel);

    Bind(wxEVT_MENU, &TMainForm::OnGenerate, this, ID_StrGenerate);

    Bind(wxEVT_MENU, &TMainForm::OnDrawStyleChange, this, ID_DSBS); // drawing styles
    Bind(wxEVT_MENU, &TMainForm::OnDrawStyleChange, this, ID_DSWF);
    Bind(wxEVT_MENU, &TMainForm::OnDrawStyleChange, this, ID_DSSP);
    Bind(wxEVT_MENU, &TMainForm::OnDrawStyleChange, this, ID_DSES);
    Bind(wxEVT_MENU, &TMainForm::OnDrawStyleChange, this, ID_DSST);
    Bind(wxEVT_MENU, &TMainForm::OnDrawStyleChange, this, ID_SceneProps);

    Bind(wxEVT_MENU, &TMainForm::OnDrawQChange, this, ID_DQH);// drawing quality
    Bind(wxEVT_MENU, &TMainForm::OnDrawQChange, this, ID_DQM);
    Bind(wxEVT_MENU, &TMainForm::OnDrawQChange, this, ID_DQL);

    Bind(wxEVT_MENU, &TMainForm::OnCellVisible, this, ID_CellVisible); // model menu
    Bind(wxEVT_MENU, &TMainForm::OnBasisVisible, this, ID_BasisVisible);
    Bind(wxEVT_MENU, &TMainForm::OnShowAll, this, ID_ShowAll);
    Bind(wxEVT_MENU, &TMainForm::OnModelCenter, this, ID_ModelCenter);

    Bind(wxEVT_MENU, &TMainForm::OnGraphics, this, ID_GraphicsHide);
    Bind(wxEVT_MENU, &TMainForm::OnGraphics, this, ID_GraphicsKill);
    Bind(wxEVT_MENU, &TMainForm::OnGraphics, this, ID_GraphicsDS);
    Bind(wxEVT_MENU, &TMainForm::OnGraphics, this, ID_GraphicsP);
    Bind(wxEVT_MENU, &TMainForm::OnGraphics, this, ID_GraphicsEdit);
    Bind(wxEVT_MENU, &TMainForm::OnGraphics, this, ID_GraphicsSelect);
    Bind(wxEVT_MENU, &TMainForm::OnGraphics, this, ID_GraphicsCollectivise);
    Bind(wxEVT_MENU, &TMainForm::OnGraphics, this, ID_GraphicsIndividualise);
    Bind(wxEVT_MENU, &TMainForm::OnGraphics, this, ID_FixLattice);
    Bind(wxEVT_MENU, &TMainForm::OnGraphics, this, ID_FreeLattice);
    Bind(wxEVT_MENU, &TMainForm::OnGraphics, this, ID_GridMenuCreateBlob);

    Bind(wxEVT_MENU, &TMainForm::OnFragmentHide, this, ID_FragmentHide);
    Bind(wxEVT_MENU, &TMainForm::OnFragmentShowOnly, this, ID_FragmentShowOnly);
    Bind(wxEVT_MENU, &TMainForm::OnFragmentSelectAtoms, this, ID_FragmentSelectAtoms);
    Bind(wxEVT_MENU, &TMainForm::OnFragmentSelectBonds, this, ID_FragmentSelectBonds);
    Bind(wxEVT_MENU, &TMainForm::OnFragmentSelectAll, this, ID_FragmentSelectAll);

    Bind(wxEVT_MENU, &TMainForm::OnAtomTypeChange, this, ID_AtomTypeChangeLast);
    Bind(wxEVT_MENU, &TMainForm::OnAtom, this, ID_AtomGrow);
    Bind(wxEVT_MENU, &TMainForm::OnAtom, this, ID_AtomCenter);
    Bind(wxEVT_MENU, &TMainForm::OnAtom, this, ID_AtomSelRings);
    Bind(wxEVT_MENU, &TMainForm::OnAtom, this, ID_AtomExploreEnvi);

    Bind(wxEVT_MENU, &TMainForm::OnPlane, this, ID_PlaneActivate);
    Bind(wxEVT_MENU, &TMainForm::OnBond, this, ID_BondViewAlong);
    Bind(wxEVT_MENU, &TMainForm::OnBond, this, ID_BondRadius);
    Bind(wxEVT_MENU, &TMainForm::OnBond, this, ID_BondInfo);
    Bind(wxEVT_MENU, &TMainForm::OnAtomOccuChange, this, ID_AtomOccuCustom);
    Bind(wxEVT_MENU, &TMainForm::OnAtomOccuChange, this, ID_AtomOccu1);
    Bind(wxEVT_MENU, &TMainForm::OnAtomOccuChange, this, ID_AtomOccu34);
    Bind(wxEVT_MENU, &TMainForm::OnAtomOccuChange, this, ID_AtomOccu12);
    Bind(wxEVT_MENU, &TMainForm::OnAtomOccuChange, this, ID_AtomOccu13);
    Bind(wxEVT_MENU, &TMainForm::OnAtomOccuChange, this, ID_AtomOccu14);
    Bind(wxEVT_MENU, &TMainForm::OnAtomOccuChange, this, ID_AtomOccuFree);
    Bind(wxEVT_MENU, &TMainForm::OnAtomOccuChange, this, ID_AtomOccuFix);
    Bind(wxEVT_MENU, &TMainForm::OnAtomOccuChange, this, ID_AtomOccuFixCurrent);
    Bind(wxEVT_MENU, &TMainForm::OnAtomConnChange, this, ID_AtomConnChangeLast);
    Bind(wxEVT_MENU, &TMainForm::OnAtomConnChange, this, ID_AtomFree);
    Bind(wxEVT_MENU, &TMainForm::OnAtomConnChange, this, ID_AtomBind);
    Bind(wxEVT_MENU, &TMainForm::OnAtomPolyChange, this, ID_AtomPolyNone);
    Bind(wxEVT_MENU, &TMainForm::OnAtomPolyChange, this, ID_AtomPolyAuto);
    Bind(wxEVT_MENU, &TMainForm::OnAtomPolyChange, this, ID_AtomPolyRegular);
    Bind(wxEVT_MENU, &TMainForm::OnAtomPolyChange, this, ID_AtomPolyPyramid);
    Bind(wxEVT_MENU, &TMainForm::OnAtomPolyChange, this, ID_AtomPolyBipyramid);
    Bind(wxEVT_MENU, &TMainForm::OnAtomPartChange, this, ID_AtomPartChangeLast);
    Bind(wxEVT_MENU, &TMainForm::OnAtomUisoChange, this, ID_AtomUisoCustom);
    Bind(wxEVT_MENU, &TMainForm::OnAtomUisoChange, this, ID_AtomUiso12);
    Bind(wxEVT_MENU, &TMainForm::OnAtomUisoChange, this, ID_AtomUiso15);
    Bind(wxEVT_MENU, &TMainForm::OnAtomUisoChange, this, ID_AtomUisoFree);
    Bind(wxEVT_MENU, &TMainForm::OnAtomUisoChange, this, ID_AtomUisoFix);
    Bind(wxEVT_MENU, &TMainForm::OnSelection, this, ID_SelGroup);
    Bind(wxEVT_MENU, &TMainForm::OnSelection, this, ID_SelUnGroup);
    Bind(wxEVT_MENU, &TMainForm::OnSelection, this, ID_SelLabel);
    Bind(wxEVT_MENU, &TMainForm::OnGraphicsStyle, this, ID_GStyleSave);
    Bind(wxEVT_MENU, &TMainForm::OnGraphicsStyle, this, ID_GStyleOpen);
  }
  idle_time = idle_start = 0;
  TEGC::AddP(&HtmlManager);
  nui_interface = 0;
  _UpdateThread = 0;
  ActionProgress = UpdateProgress = 0;
  SkipSizing = false;
  Destroying = false;
#if defined(__WIN32__) || defined(__linux__)
  _UseGlTooltip = false;  // Linux and Mac set tooltips after have been told to do so...
#else
  _UseGlTooltip = true;
#endif
  StartupInitialised = RunOnceProcessed = false;
  wxInitAllImageHandlers();
  /* a singleton - will be deleted in destructor, we cannot use GC as the Py_DecRef
   would be called after finalising python
  */
  PythonExt::Init(this).Register(
    TMainForm::ModuleName(), &TMainForm::PyInit);
  PythonExt::GetInstance()->Register(
    OlexPyCore::ModuleName(), &OlexPyCore::PyInit);
  PythonExt::GetInstance()->Register(
    hkl_py::ModuleName(), &hkl_py::PyInit);
  PythonExt::GetInstance()->Register(
    TXGrid::ModuleName(), &TXGrid::PyInit);
#if defined(_WIN32)
  PythonExt::GetInstance()->Register(
    py_reg::ModuleName(), &py_reg::PyInit);
#endif
  //TOlxVars::Init().OnVarChange->Add(this, ID_VarChange);
  FGlCanvas = 0;
  FXApp = 0;
  FGlConsole = 0;
  FInfoBox = 0;
  GlTooltip = 0;

  MousePositionX = MousePositionY = -1;

  LabelToEdit = 0;

  TimePerFrame = 50;
  DrawSceneTimer = 0;

  MouseMoveTimeElapsed  = 0;
  FBitmapDraw = false;
  FMode = 0;
  FRecentFilesToShow = 9;
  FHtmlPanelWidth = 0.25;
  FHtmlWidthFixed = false;
  FHtmlMinimized = false;

  InfoWindowVisible = HelpWindowVisible = true;
  CmdLineVisible = false;

  FLastSettingsFile = "last.osp";

  Modes = new TModeRegistry();

  FParent = Parent;
  ObjectUnderMouse(0);
  FHelpItem = 0;
  FTimer = new TTimer;
  HelpFontColorCmd.SetFlags(sglmAmbientF);
  HelpFontColorTxt.SetFlags(sglmAmbientF);
  HelpFontColorCmd.AmbientF = 0x00ffff;
  HelpFontColorTxt.AmbientF = 0x00ffff00;

  ExecFontColor.SetFlags(sglmAmbientF);
  ExecFontColor.AmbientF = 0x00ffff;

  InfoFontColor.SetFlags(sglmAmbientF);
  InfoFontColor.AmbientF = 0x007fff;

  WarningFontColor.SetFlags(sglmAmbientF);
  WarningFontColor.AmbientF = 0x007fff;

  ErrorFontColor.SetFlags(sglmAmbientF);
  ErrorFontColor.AmbientF = 0x007fff;

  ExceptionFontColor.SetFlags(sglmAmbientF);
  ExceptionFontColor.AmbientF = 0x0000ff;
}
//..............................................................................
bool TMainForm::Destroy() {
  Destroying = true;
  tensor::tensor_rank_2::cleanup();
  tensor::tensor_rank_3::cleanup();
  tensor::tensor_rank_4::cleanup();
  SaveVFS(plGlobal);  // save virtual db to file
  SaveVFS(plStructure);
  FXApp->OnObjectsDestroy.Remove(this);
  processMacro("onexit");
  {
    for (size_t i = 0; i < loadedDll.Count(); i++) {
      loadedDll[i]->Finalise();
    }
  }
  SaveSettings(FXApp->GetConfigDir() + FLastSettingsFile);
  HtmlManager.Destroy();
  if (_UpdateThread != 0) {
    _UpdateThread->OnTerminate.Remove(this);
    _UpdateThread->Join(true);
    delete _UpdateThread;
  }
  if (UpdateProgress != 0) {
    delete UpdateProgress;
  }
  // clean up it here
  FXApp->GetStatesRegistry().OnChange.Clear();
  FXApp->XFile().OnFileLoad.Remove(this);
  FXApp->XFile().OnFileClose.Remove(this);
  FXApp->XFile().GetRM().OnSetBadReflections.Remove(this);
  FXApp->XFile().GetRM().OnCellDifference.Remove(this);

  delete Modes;
  FTimer->OnTimer.Clear();
  delete FTimer;
  delete pmGraphics;
  delete pmFragment;
  delete pmMenu;
  delete pmAtom;
  delete pmBond;
  delete pmPlane;
  delete pmSelection;
  delete pmLabel;
  delete pmLattice;
  delete pmGrid;

  // leave it for the end
  delete _ProcessManager;
#ifdef _CUSTOM_BUILD_
  CustomCodeBase::Finalise();
#endif
  // the order is VERY important!
  TOlxVars::Finalise();
  PythonExt::Finilise();
  return wxFrame::Destroy();
}
//..............................................................................
TMainForm::~TMainForm() {
  GetInstance() = 0;
}
//..............................................................................
void TMainForm::XApp(Olex2App *XA)  {
  olex2::OlexProcessorImp::SetLibraryContainer(*XA);
  FXApp = XA;

  _ProcessManager = new ProcessManager(_ProcessHandler);
  FXApp->SetCifTemplatesDir(XA->GetBaseDir() + "etc/CIF/");
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
  Macros.Init();
  TLibrary &Library = XA->GetLibrary();
  this_InitMacroAD(Reap, @reap,
    "b-blind&;"
    "r-prevent resetting the style to default&;"
    "*-read and overlay&;"
    "check_loaded-[True] checks if the file has been loaded in another Olex2 instance&;"
    "check_crashed-[True] do not load file if crash detected&;"
    ,
    fpAny,
    "This macro loads a file if provided as an argument. If no argument is "
    "provided, it shows a file open dialog");
  this_InitMacroD(Pict,
    "c-embossed output in color&;"
    "bw-embossed output in b&w&;"
    "pq-highest (picture) quality&;"
    "nbg-mask the background with 0 alpha (an rgb colour may be gives)&;"
    "dpi-the physical resolution to be written to the file&;"
    ,
    fpOne|fpTwo,
    "Outputs a raster picture. Output file name is required, if a second "
    "numerical parameter is provided, it is considered to be image resolution "
    "in range [0.1-10] in screen sizes, anythin greater than 10 is considered "
    "as the desired picture width.");
  this_InitMacroD(Picta,
    "pq-picture quality&;"
    "nbg-mask the background with 0 alpha (an rgb colour may be gives)&;"
    "dpi-the physical resolution to be written to the file&;"
    ,
    fpOne|fpTwo,
    "A portable version of pict with limited resolution (OS/graphics card "
    "dependent). Not stable on some graphics cards");
  this_InitMacroD(Echo,
    "m-the printing color (info, warning, error or exceptiion)&;"
    "c-copy printed information to clipboard",
    fpAny,
    "Prints provided string, functions are evaluated before printing");
  this_InitMacroD(PictPS,
    "color_line-lines&;"
    "color_fill-ellipses are filled&;"
    "color_bond-bonds are colored&;"
    "color_plane-planes are colored&;"
    "div_pie-number [4] of stripes in the octant&;"
    "lw_pie-line width [0.5] of the octant stripes&;"
    "lw_octant-line width [0.5] of the octant arcs&;"
    "lw_font-line width [1] for the vector font&;"
    "lw_ellipse-line width [0.5] of the ellipse&;"
    "scale_hb-scale for H-bonds [0.5]&;"
    "p-perspective &;"
    "octants-comma separated atom types/names ADP's of which to be rendered "
    "with octants [-$C]&;"
    "bond_outline_color-bond outline color[0xffffff]&;"
    "bond_outline_oversize-the extra size of the outline in percents [10]&;"
    "atom_outline_color-atom outline color[0xffffff]&;"
    "atom_outline_oversize- the extra size of outline in percents[5]&;"
    "stipple_disorder- render stippled bonds for atoms in non 0 part [true]&;"
    "multiple_bond_width- if 0 double and triple bonds are rendered as a "
    "fraction of their real width. If a values is not 0 - it specifies the "
    "width of the strips&;"
    "div_ellipse-the number of ellipse segments [36]"
    ,
    fpOne|psFileLoaded,
    "Postscript rendering");
  this_InitMacroD(PictTEX,
    "color_line-lines&;"
    "color_fill-ellipses are filled",
    fpOne|psFileLoaded,
    "Experimental tex/pgf rendering");
  this_InitMacroD(PictS,
    "a-view angle [6]&;"
    "s-separation between the images in % [10]&;"
    "h-output image height [screen*resolution]",
    fpOne|fpTwo|psFileLoaded,
    "Stereoscopic picture rendering");
  this_InitMacroD(PictPR, EmptyString(), fpOne|psFileLoaded,
    "PovRay output");
  // contains an accumulation buffer. prints only when '\n' is encountered
  this_InitMacroD(Post, EmptyString(), fpAny,
    "Prints a string, but only after a new line character is encountered");

  this_InitMacroD(Clear, EmptyString(), fpNone|fpOne,
    "When no arguments is given, clears console buffer (text). If 'groups' is"
    " the first argument - all objects are put back to the default groups.");
  this_InitMacroD(Rota, EmptyString(), fpTwo|fpFive,
    "For two arguments the first one specifies axis of rotation (1,2,3 or "
    "x,y,z) and the second one the rotation angle in degrees. For five "
    "arguments the first three arguments specify the rotation vector [x,y,z] "
    "the forth parameter is the rotation angle and the fifth one is the "
    "increment - the rotation will be continuous");

  this_InitMacroD(Listen, EmptyString(), fpAny,
    "Listens for changes in a file provided as argument. If the file content "
    "changes it is automatically reloaded in Olex2. If no arguments provided "
    "prints current status of the mode");
  this_InitMacroD(ListenCmd, EmptyString(), fpAny,
    "Listens for changes in a file provided as argument. If the file content "
    "changes its first line is ran as a command. If no arguments provided "
    "prints current status of the mode");
  #ifdef __WIN32__
  this_InitMacroD(WindowCmd, EmptyString(), fpAny^(fpNone|fpOne),
    "Windows specific command which send a command to a process with GUI "
    "window. First argument is the window name, the second is the command. "
    "'nl' is considered as a new line char and 'sp' as white space char");
  #endif
  this_InitMacroD(ProcessCmd, EmptyString(), fpAny^(fpNone|fpOne),
    "Send a command to current process. 'nl' is translated to the new line "
    "char and 'sp' to the white space char");
  this_InitMacroD(Wait, "r-during wait all events are processed", fpOne,
    "Forces Olex2 and calling process to sleep for provided number of "
    "milliseconds");

  this_InitMacroD(SwapBg, EmptyString(), fpNone,
    "Swaps current background to white or vice-versa");
  this_InitMacroD(Silent, EmptyString(), fpNone|fpOne,
    "If no argument is provided, prints out current mode status. Takes 'on' "
    "and 'off' values to turn Olex2 log on and off");
  this_InitMacroD(Stop, EmptyString(), fpOne,
    "Switches specified mode off");

  this_InitMacroD(Exit, EmptyString(), fpNone, "Exits Olex2");
  this_InitMacroAD(Exit, quit, EmptyString(), fpNone, "Exits Olex2");

  this_InitMacroD(Capitalise, "", (fpAny|psFileLoaded)^fpNone,
    "Changes atom labels capitalisation for all/given/selected atoms. The"
    " first argument is the template like Aaaa");


  this_InitMacroD(SetEnv, EmptyString(), fpTwo,
"Sets an environmental variable");

  this_InitMacroD(Help, "c-specifies commands category", fpAny,
    "Prints available information. If no arguments provided prints available "
    "commands");
  this_InitMacroD(AddLabel,
    EmptyString(),
    fpThree|fpFive,
    "Adds a new label to the collection named by the first argument.\n"
    "For 3 arguments: [collection_name] 'x y z' label, where 'x y z' may be "
    "obtained by a call to crd() function\n"
    "For 5 arguments: [collection_name] x y z label");

  this_InitMacroD(Hide,
    "b-also hides all bonds atatched to the selected atoms",
    fpAny,
    "Hides selected objects or provided atom names (no atom related objects as"
    " bonds are hidden automatically)");

  this_InitMacroD(Exec,
    "s-synchronise&;"
    "o-detached&;"
    "d-output dub file name&;"
    "t-a list of commands to be run when the process is terminated&;"
    "q-do not post output to console",
    fpAny^fpNone,
    "Executes external process");
  this_InitMacroD(Shell, EmptyString(), fpAny,
    "If no arguments launches a new interactive shell, otherwise runs provided"
    " file in the interactive shell (on windows ShellExecute is used to avoid "
    "flickering console)");
  this_InitMacroD(Save, EmptyString(), fpAny^fpNone,
    "Saves style/scene/view/gview/model/ginfo");
  GetLibrary().Register(
    new TMacro<TMainForm>(this, &TMainForm::macLoad, "Load",
      "c-when loading style clears current model customisation [false]",
      fpAny^fpNone,
      "Loads style/scene/view/gview/model/radii/ginfo. For radii accepts sfil, vdw, pers"),
    libChain
  );
  this_InitMacro(Link, , fpNone|fpOne);
  this_InitMacroD(Style, "s-shows a file open dialog", fpNone|fpOne,
    "Prints default style or sets it (none resets)");
  this_InitMacroD(Scene, "s-shows a file open dialog", fpNone|fpOne,
    "Prints default scene parameters or sets it (none resets)");

  this_InitMacroD(Lines, EmptyString(), fpOne,
    "Sets the number of visible text lines in the console. Use -1 to display "
    "all lines");

  this_InitMacro(Ceiling, , fpOne);
  this_InitMacro(Fade, , fpThree);

  this_InitMacro(WaitFor, , fpOne);

  this_InitMacroD(HtmlPanelSwap, EmptyString(), fpNone|fpOne,
    "Swaps or sets the position of the HTML GUI panel. If no arguments given -"
    " swaps the panel position. The position can be specified by left and "
    "right");
  this_InitMacro(HtmlPanelWidth, , fpNone|fpOne);
  this_InitMacroD(HtmlPanelVisible, EmptyString(), fpNone|fpOne|fpTwo,
    "Swaps visibility of the HTML GUI panel or sets it to a given state"
    " (true/false)");

  this_InitMacroD(QPeakScale, EmptyString(), fpNone|fpOne,
    "Prints/sets the scale of dependency of the Q-peak transparency vs "
    "height");
  this_InitMacroD(QPeakSizeScale, EmptyString(), fpNone|fpOne,
    "Prints/sets the scale the Q-peak size relative to other atoms, default is"
    " 1");

  this_InitMacroD(Focus, EmptyString(), fpNone,
    "Sets input focus to the console");
  this_InitMacroD(Refresh, EmptyString(), fpNone,
    "Refreshes the GUI");

  this_InitMacroD(Mode,
    "a-[name] autocomplete; [grow] grow (rebuild) asymmetric unit only; [fit] "
    "afix&;"
    "p-[name] prefix; [grow] inserts the new atoms into the AU with given [-1]"
    " part value&;"
    "s-[grow] short interactions; [name] suffix;"
    " [fit] split, atoms to split offset [0]&;"
    "t-[name] type&;temperature correction for distance [hfix]"
    "c-[grow] covalent bonds; [move] copy fragments instead of moving&;"
    "r-[split] a restraint/constraint for split atoms; [grow] show radial "
    "bonds between the same atoms; [fit] rotation angle increment (smooth "
    "rotation by default); [name] synchronise names in the residues&;"
    "v-[grow] use user provided delta for connectivity analysis, default 2A; "
    "[fit] variable index&;"
    "shells-[grow] grow atom shells vs fragments&;"
    "l-[name] lock atom types after naming;[grow] list contacts&;"
    "e-a macro to run on mode exit&;"
    "d-[hfix] distance&;"
    "i-[fit, false] use only the selected atoms for 1-3 distances&;"
    ,
    (fpAny^fpNone)|psFileLoaded,
    "Turns specified mode on. Valid mode: fixu, fixc, grow, himp, match, move,"
    " name, occu, pack, part, split, fit");

  Library.Register(new TMacro<TMainForm>(this, &TMainForm::macFlush,
    "Flush",
    EmptyString(), fpNone|fpOne|fpTwo,
    "An extension to 'flush log' to 'flush output' to flush console buffer "
    "into DataDir()/[output.txt] file, the file name can follow the command"),
    libChain);
  this_InitMacroD(ShowStr, EmptyString(), fpNone|fpOne|psFileLoaded,
    "Shows/hides structure and console buffer");
  // not implemented
  this_InitMacro(Bind, , fpTwo);

  this_InitMacroD(Grad,
    "i-toggles gradient mode and the user/white background&;"
    "p-sets/removes the gradient picture, the picture is assumed to have "
    "power of 2 dimensions (like 512x256), it is stretched if needed",
    fpNone|fpOne|fpFour,
    "Sets options for the background gradient. No options - showsn the "
    "gradient dialog where the user can choose the gradient colors. One "
    "parameter is expected to be a boolean - shows/hides the gradient. Four "
    "parameters specify the gradient colours explicetly.");

  this_InitMacroD(EditAtom, "cs-do not clear the selection",
    fpAny|psFileLoaded,
    "Shows information for the given atom and all of its dependents");
  this_InitMacro(EditIns, , fpNone|psFileLoaded);
  this_InitMacro(HklEdit, , fpNone|fpOne|fpThree);
  this_InitMacro(HklView, , fpNone|fpOne);

  // not implemented
  this_InitMacro(HklExtract, , fpOne|psFileLoaded);

  this_InitMacro(ViewGrid, , fpNone|fpOne);

  this_InitMacroD(Popup,
    "w-width&;"
    "h-height&;"
    "t-title&;"
    "b-border[trscaip],t-caption bar, r-sizeable border, s-system menu, "
    "c-close box, a-maximise box, i-minimise box, p-window should stay on the"
    " top of others&;x-left position&;y-top position&;ondblclick-a macro or "
    "commands to execute when window is double clicked&;onsize-a macro to be "
    "executed when the popup is resized&;s-do show the window after the creation",
    fpTwo,
    "Creates a popup HTML window. Usage: popup popup_name html_source");


  this_InitMacroAD(Python, @py,
    "i-shows a text input box&;"
    "l-loads a file into a text input box",
    fpAny,
    "Runs provided python lines '\\n' is used as new line separator or shows "
    "a text input window");

  this_InitMacro(CreateMenu, c&;s&;r&;m, fpOne|fpTwo|fpThree);
  this_InitMacro(DeleteMenu, , fpOne);
  this_InitMacro(EnableMenu, , fpOne);
  this_InitMacro(DisableMenu, , fpOne);
  this_InitMacro(CheckMenu, , fpOne);
  this_InitMacro(UncheckMenu, , fpOne);

  this_InitMacro(CreateShortcut, , fpTwo);

  this_InitMacro(SetCmd, , fpAny);

  this_InitMacroD(UpdateOptions, EmptyString(), fpNone,
    "Shows the Update Options dialog");
  this_InitMacroD(Update,
    "f-force [true]&;"
    "reinstall-reinstalls Olex2 completely&;"
    ,
    fpNone,
    "Does check for the updates");
  this_InitMacro(Reload, , fpOne);
  this_InitMacro(StoreParam, , fpTwo|fpThree);
  this_InitMacro(SelBack, a&;o&;x, fpNone);

  this_InitMacro(CreateBitmap, r, fpTwo);
  this_InitMacro(DeleteBitmap, , fpOne);
  this_InitMacro(Tref, ,fpOne|fpTwo|psCheckFileTypeIns);
  this_InitMacro(Patt, ,fpNone|psCheckFileTypeIns);

  this_InitMacro(InstallPlugin,
    "l-local installation from a zip file, which must contains index.ind",
    fpOne);
  this_InitMacro(SignPlugin, ,fpAny^(fpOne|fpNone));
  this_InitMacro(UninstallPlugin, ,fpOne);
  this_InitMacro(UpdateFile, f,fpOne);
  this_InitMacro(NextSolution, ,fpNone);

  this_InitMacro(ShowWindow, ,fpOne|fpTwo);

  this_InitMacroD(Schedule,
    "r-repeatable&;"
    "g-requires GUI"
    ,
    fpAny^(fpNone),
    "Schedules a particular macro (second argument) to be executed within "
    "provided interval (first argument). If the interval is not specified the "
    "requested macro is called when the program is idle");

  this_InitMacro(Test, , fpAny);

  this_InitMacroD(IT,
    "o-orients basis according to principle axes of inertia",
    fpAny,
    "Calculates tensor of inertia");

  this_InitMacroD(StartLogging, "c-empties the file if exists", fpOne,
    "Creates/opens for appending a log file, where all screen output is "
    "saved");
  this_InitMacroD(ViewLattice, EmptyString(), fpOne,
    "Loads cell information from provided file and displays it on screen as "
    "lattice points/grid");
  this_InitMacroD(AddObject,
    "g-grow for spheres using current model",
    fpAny^(fpNone|fpOne|fpTwo),
    "Adds a new user defined object to the graphical scene");
  this_InitMacroD(DelObject, EmptyString(), fpOne,
    "Deletes graphical object by name");

  this_InitMacroD(OnRefine, EmptyString(), fpAny,
    "Internal procedure");
  this_InitMacroD(TestMT, EmptyString(), fpAny,
    "Testing multithreading");
  this_InitMacroD(SetFont,
    "ps-point size&;"
    "b-bold&;"
    "i-italic",
    fpAny^(fpNone|fpOne),
    "Sets font for specified control");
  this_InitMacroD(EditMaterial, EmptyString(), fpOne,
    "Brings up material properties dialog for specified object");

  GetLibrary().Register(
    new TMacro<TMainForm>(this, &TMainForm::macSetMaterial, "SetMaterial",
    EmptyString(),
    fpTwo | fpThree,
    "Assigns provided value to specified material. Special materials are: "
    "helpcmd, helptxt, execout, error, exception"),
    libChain
    );

  this_InitMacroD(ShowSymm, EmptyString(), fpNone|fpOne,
    "Shows symmetry elements of the unitcell");
  this_InitMacroD(Textm, EmptyString(), fpOne,
    "Runs subsequent commands stored in a text file");
  this_InitMacroD(TestStat, EmptyString(), fpOne,
    "Test: runs statistical tests on structures in current folder. Expects a "
    "file name");
  this_InitMacroD(ExportFont, EmptyString(), fpTwo,
    "Exports given fonts into Olex2 portable format. At maximum two fonts a "
    "file are supported: a fixed and a proportional font. Example: ExportFont "
    "ChooseFont()&ChooseFont test.fnt");
  this_InitMacroD(ImportFont, EmptyString(), fpTwo, "");
  this_InitMacroD(ImportFrag,
    "p-part to assign&;"
    "d-generate DFIX for 1-2 and 1-3 distances&;"
    "a-set specified AFIX to the imported fragment&;"
    "o-set specified occupancy to the imported fragment atoms&;"
    "c-take the content from the clipboard (XYZ like)&;"
    "rr-replace particular RESI with the given content (auto-fit)&;"
    "i-invert the provided atom coordinates",
    fpNone|fpOne|psFileLoaded,
    "Import a fragment into current structure");
  this_InitMacroD(ExportFrag, EmptyString(), fpNone|psFileLoaded,
    "Exports selected fragment to an external file");
  this_InitMacroD(UpdateQPeakTable, EmptyString(), fpNone|psFileLoaded,
    "Internal routine for synchronisation");
  this_InitMacroD(FlushFS, EmptyString(),
    (fpOne|fpNone),
    "Saves current content of the virtual file system. If no parameters is "
    "given - the global state is saved. Possible arguments: global, "
    "structure");
  this_InitMacroD(Elevate, EmptyString(),
    fpNone|fpOne,
    "Runs Olex2 in elevated/desktop mode [true]/false- only available on "
    "Windows");
  this_InitMacroD(Restart,
    EmptyString(),
    fpNone,
    "Restarts Olex2");
  this_InitMacroD(ADPDisp, EmptyString(),
    fpTwo|fpNone,
    "Compares two structures in the terms of atomsic dispacement after the "
    "structure optimisation and the experimental ADP. First structure is the "
    "XRay experimental structure and the second is the optimised one. The "
    "structures are expected to have identical labelling scheme. If no "
    "arguments is given - the procedure prints lengths cut by ADPs on the "
    "bonds.");
  this_InitMacroD(RegisterFonts,
    EmptyString(),
    fpOne,
    "Registers fonts in the given folder for the application use");

  // FUNCTIONS _________________________________________________________________

  this_InitFunc(FileLast, fpNone|fpOne);
  this_InitFunc(FileSave, fpThree|fpFour);
  this_InitFunc(FileOpen, fpThree|fpFour);
  this_InitFuncD(ChooseDir, fpNone|fpOne|fpTwo,
    "Shows a dialog to pick a folder. Arguments [title=Choose directory],"
    " [default path=current directory].");

  this_InitFunc(Strcat, fpTwo);
  this_InitFunc(Strcmp, fpTwo);

  this_InitFuncD(GetEnv, fpNone|fpOne,
    "Prints all variables if no arguments is given or returns the given "
    "veariable value");

  this_InitFunc(Atoms, fpOne|psFileLoaded);

  this_InitFuncD(Sel, fpNone|fpOne|psFileLoaded,
    "Returns current selection. By default expands bonds and planes into the "
    "list of atoms. If the 'a' argument is given, returns only selected atoms."
  );
  this_InitFunc(FPS, fpNone);

  this_InitFunc(Cursor, fpNone|fpOne|fpTwo|fpThree);
  this_InitFunc(RGB, fpThree|fpFour);
  this_InitFunc(Color, fpNone|fpOne|fpTwo);

  this_InitFunc(HtmlPanelWidth, fpNone|fpOne);

  this_InitFunc(LoadDll, fpOne);

  this_InitFuncD(Alert, fpTwo|fpThree|fpFour,
    "title message [flags YNCO-yes,no,cancel,ok "
    "XHEIQ-icon:exclamation,hand,eror,information,question "
    "R-show checkbox] [checkbox message]");

  this_InitFunc(IsPluginInstalled, fpOne);
  this_InitFunc(ValidatePlugin, fpOne);

  // number of lines, caption, def value
  this_InitFunc(GetUserInput, fpThree);
  this_InitFunc(GetUserStyledInput, fpThree);

  this_InitFuncD(TranslatePhrase, fpOne,
"Translates provided phrase into current language");
  this_InitFuncD(IsCurrentLanguage, fpOne,
"Checks current language");
  this_InitFuncD(CurrentLanguageEncoding, fpNone,
"Returns current language encoding, like: ISO8859-1");

  this_InitFunc(ChooseElement, fpNone);

  this_InitFuncD(ChooseFont, fpNone|fpOne|fpTwo,
    "Brings up a font dialog. If font information provided, initialises the "
    "dialog with that font; the first argument may be just 'olex2' or 'system'"
    " to enforce choosing the Olex2/System font (the font information can be "
    "provided in the second argument then)");
  this_InitFuncD(GetFont, fpOne,
    "Returns specified font");

  GetLibrary().Register(
    new TFunction<TMainForm>(this, &TMainForm::funGetMaterial, "GetMaterial",
    fpOne | fpTwo,
    "Returns material of specified object. Special materials are: "
    "helpcmd, helptxt, execout, error, exception"),
    libChain
    );

  this_InitFuncD(ChooseMaterial, fpNone|fpOne,
    "Brings up a dialog to edit default or provided material");

  this_InitFuncD(GetMouseX, fpNone,
    "Returns current mouse X position");
  this_InitFuncD(GetMouseY, fpNone,
    "Returns current mouse Y position");
  this_InitFuncD(GetWindowSize, fpNone|fpOne|fpThree,
    "Returns size of the requested window, main window by default");
  this_InitFuncD(IsOS, fpOne,
    "Returns true if current system Windows [win], Linux/GTK [linux], Mac "
    "[mac]");
  this_InitFuncD(HasGUI, fpNone,
    "Returns if true if Olex2 is built with GUI");
  this_InitFuncD(CheckState, fpOne|fpTwo,
    "Returns if true if given program state is active");
  this_InitFuncD(GlTooltip, fpNone|fpOne,
    "Returns state of/sets OpenGL tooltip implementation for the main window "
    "(some old platforms do not have proper implementation of tooltips)");
  this_InitFuncD(CurrentLanguage, fpNone|fpOne,
    "Returns/sets current language");
  this_InitFuncD(GetMAC, fpNone|fpOne,
    "Returns simicolon separated list of computer MAC addresses. If 'full' is "
    "provided as argument, the adoptor names are also returned as "
    "adapter=""MAC;..");
  this_InitFuncD(ThreadCount, fpNone|fpOne,
    "Returns/sets the number of simultaneous tasks");
  this_InitFuncD(FullScreen, fpNone|fpOne,
    "Returns/sets full screen mode (true/false/swap)");
  this_InitFuncD(Freeze, fpNone|fpOne,
    "Gets/Sets display update status");
  Library.AttachLibrary(FXApp->ExportLibrary());
  Library.AttachLibrary(LibFile::ExportLibrary());
  Library.AttachLibrary(LibStr::ExportLibrary());
  Library.AttachLibrary(LibMath::ExportLibrary());
  //Library.AttachLibrary(olxstr::ExportLibrary("str"));
  Library.AttachLibrary(PythonExt::GetInstance()->ExportLibrary());
  Library.AttachLibrary(TETime::ExportLibrary());
  Library.AttachLibrary(lcells::IndexManager::ExportLibrary());
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
  Library.AttachLibrary(XA->XFile().ExportLibrary());
  Library.AttachLibrary(XA->GetFader().ExportLibrary());
  Library.AttachLibrary(XA->XGrid().ExportLibrary());
  Library.AttachLibrary(XA->XFile().DUnitCell->ExportLibrary());
  Library.AttachLibrary(TFileHandlerManager::ExportLibrary());
  Library.AttachLibrary(TAutoDB::GetInstance(false).ExportLibrary());

  TOlxVars::ExportLibrary(EmptyString(), &Library);

#ifdef _CUSTOM_BUILD_
  CustomCodeBase::Initialise(Library);
#endif
  {
    olxstr f = "GetCompilationInfo(full)";
    if (processFunction(f, EmptyString(), true)) {
      FXApp->NewLogEntry(logInfo, true) << "Welcome to Olex2-"
        << patcher::PatchAPI::ReadRepositoryTag() << ' ' << f;
    }
  }
  // menu initialisation
  MenuBar = new wxMenuBar;

  MenuFile = new TMenu();
  TMenu *MenuView = new TMenu();
  TMenu *MenuHelp = new TMenu();
  TMenu *MenuStructure = new TMenu();

  pmMenu = new TMenu();
  pmDrawStyle = new TMenu();
  pmDrawQ = new TMenu();
  pmModel = new TMenu();
  pmView = new TMenu();
  pmAtom = new TMenu();
    pmBang = new TMenu();
    pmAtomType = new TMenu();
    pmAtomOccu = new TMenu();
    pmAtomConn = new TMenu();
    pmAtomPoly = new TMenu();
    pmAtomPart = new TMenu();
    pmAtomUiso = new TMenu();
  pmFragment = new TMenu();
  pmSelection = new TMenu();
  pmGraphics = new TMenu();
  pmBond = new TMenu();
    pmTang = new TMenu();
  pmPlane = new TMenu();

  MenuBar->Append(MenuFile, wxT("&File"));
  for (int i = 0; i < 10; i++) {
    Bind(wxEVT_COMMAND_MENU_SELECTED,
      &TMainForm::OnFileOpen, this, ID_FILE0 + i);
  }
  MenuBar->Append(MenuView, wxT("&View"));
  MenuBar->Append(MenuStructure, wxT("&Structure"));
  MenuBar->Append(MenuHelp, wxT("&Help"));

  miHtmlPanel = new wxMenuItem(MenuView, ID_HtmlPanel, wxT("&Html panel"),
    wxT("Show/hide html panel"), wxITEM_CHECK, NULL);
  MenuView->Append(miHtmlPanel);

  MenuStructure->Append(ID_StrGenerate, wxT("&Generate..."));

// statusbar initialisation
  StatusBar = CreateStatusBar();
  SetStatusText( wxT("Welcome to OLEX2!"));
// toolbar initialisation
  ToolBar = NULL;
//  ToolBar = CreateToolBar(wxTB_FLAT | wxTB_HORIZONTAL | wxTB_TEXT  , -1, "MainToolBar");
//  olxstr S = TEFile::ExtractFilePath(wxGetApp().argv[0]);
//  S += "toolbar\\copy.bmp";
//  wxBitmap Bmp( wxBITMAP(COPY));
//  int w = Bmp.GetWidth(), h = Bmp.GetHeight();

//  ToolBar->SetToolBitmapSize(wxSize(w, h));
//  ToolBar->AddTool(ID_DSSP, "SP", Bmp, "Sphere packing");
//  ToolBar->AddTool(ID_DSES, "ES", Bmp, "Ellipsoids & sticks");
//  ToolBar->AddTool(ID_DSBS, "BS", Bmp, "Balls & sticks");
//  ToolBar->AddTool(ID_DSWF, "WF", Bmp, "Wireframe");
//  ToolBar->Realize();

// setting popup menu

  pmMenu->Append(ID_MenuDrawStyle, wxT("&Draw style"), pmDrawStyle);
  pmMenu->Append(ID_MenuDrawQ, wxT("&Draw quality"), pmDrawQ);
  pmMenu->Append(ID_MenuModel, wxT("&Model"), pmModel);
  pmMenu->Append(ID_MenuView, wxT("&View"), pmView);

  pmDrawStyle->Append(ID_DSSP, wxT("Sphere packing"));
  pmDrawStyle->Append(ID_DSBS, wxT("Balls && sticks"));
  pmDrawStyle->Append(ID_DSES, wxT("Ellipses && sticks"));
  pmDrawStyle->Append(ID_DSST, wxT("Sticks"));
  pmDrawStyle->Append(ID_DSWF, wxT("Wireframe"));
  pmDrawStyle->AppendSeparator();
  pmDrawStyle->Append(ID_SceneProps, wxT("Scene Properties..."));
  pmDrawStyle->AppendSeparator();
  pmDrawStyle->Append(ID_GStyleOpen, wxT("Load style..."));
  pmDrawStyle->Append(ID_GStyleSave, wxT("Save style..."));

  pmDrawQ->Append(ID_DQH, wxT("High"));
  pmDrawQ->Append(ID_DQM, wxT("Medium"));
  pmDrawQ->Append(ID_DQL, wxT("Low"));

  pmModel->Append(ID_CellVisible,  wxT("Cell"));
  pmModel->Append(ID_BasisVisible, wxT("Basis"));
  pmModel->Append(ID_ModelCenter,  wxT("Center"));
// setting fragment menu
  pmFragment->Append(ID_FragmentHide, wxT("Hide"));
  pmFragment->Append(ID_FragmentShowOnly, wxT("Show this only"));
  pmFragment->Append(ID_ShowAll, wxT("Show all"));
  pmFragment->Append(ID_FragmentSelectAtoms, wxT("Select atoms"));
  pmFragment->Append(ID_FragmentSelectBonds, wxT("Select bonds"));
  pmFragment->Append(ID_FragmentSelectAll, wxT("Select"));
// setting selection menu
  pmSelection->Append(ID_SelGroup, wxT("Group"));
  pmSelection->Append(ID_SelUnGroup, wxT("Ungroup"));
  pmSelection->Append(ID_SelLabel, wxT("Label"));
//  pmSelection->AppendSeparator();
//  pmSelection->Append(ID_MenuGraphics, "Graphics", pmGraphics->Clone());
// setting graphics menu
  pmGraphics->Append(ID_GraphicsHide, wxT("Hide"));
  pmGraphics->Append(ID_GraphicsDS, wxT("Draw style..."));
  pmGraphics->Append(ID_GraphicsP, wxT("Primitives..."));
  pmGraphics->Append(ID_GraphicsSelect, wxT("Select the group(s)"));
// setting label menu
  pmLabel = pmGraphics->Clone();
  pmLabel->Append(ID_GraphicsEdit, wxT("Edit..."));
// setting label menu
  pmGrid = pmGraphics->Clone();
  pmGrid->Append(ID_GridMenuCreateBlob, wxT("Create blob"));
// setting Lattice menu
  pmLattice = pmGraphics->Clone();
  pmLattice->Append(ID_FixLattice, wxT("Fix"));
  pmLattice->Append(ID_FreeLattice, wxT("Free"));
// extra graphics bits
  pmGraphics->Append(ID_GraphicsCollectivise,
    "Join parent group")->Enable(false);
  pmGraphics->Append(ID_GraphicsIndividualise,
    "Remove from parent group")->Enable(false);
  // setting atom menu
  pmAtom->Append(ID_AtomInfo, wxT("?"));
  pmAtom->AppendSeparator();
  pmAtom->Append(ID_MenuBang, wxT("BANG"), pmBang);
  TStrList at("C;N;O;F;H;S", ';');
  for (size_t i = 0; i < at.Count(); i++) {
    pmAtomType->Append(ID_AtomTypeChange + i, at[i].u_str());
    Bind(wxEVT_COMMAND_MENU_SELECTED,
      &TMainForm::OnAtomTypeChange, this, ID_AtomTypeChange + i);
  }
    pmAtomType->Append(ID_AtomTypeChangeLast, wxT("More..."));
    pmAtomOccu->AppendRadioItem(ID_AtomOccuCustom, wxT("Custom"));
    pmAtomOccu->AppendRadioItem(ID_AtomOccu1, wxT("1"));
    pmAtomOccu->AppendRadioItem(ID_AtomOccu34, wxT("3/4"));
    pmAtomOccu->AppendRadioItem(ID_AtomOccu12, wxT("1/2"));
    pmAtomOccu->AppendRadioItem(ID_AtomOccu13, wxT("1/3"));
    pmAtomOccu->AppendRadioItem(ID_AtomOccu14, wxT("1/4"));
    pmAtomOccu->AppendRadioItem(ID_AtomOccuFix, wxT("Fix"));
    pmAtomOccu->AppendRadioItem(ID_AtomOccuFree, wxT("Free"));
    pmAtomOccu->Append(ID_AtomOccuFixCurrent, wxT("Fix as is"));

    pmAtomConn->AppendRadioItem(ID_AtomConnChangeLast, wxT("Custom"));
    TStrList bc("0;1;2;3;4;12 [Default];24", ';');
    for (size_t i = 0; i < bc.Count(); i++) {
      pmAtomConn->AppendRadioItem(ID_AtomConnChange+i, bc[i].u_str());
      Bind(wxEVT_COMMAND_MENU_SELECTED,
        &TMainForm::OnAtomConnChange, this, ID_AtomConnChange + i);
    }
    pmAtomConn->AppendSeparator();
    pmAtomConn->Append(ID_AtomBind, wxT("Create bond"));
    pmAtomConn->Append(ID_AtomFree, wxT("Remove bond"));

    pmAtomPoly->AppendRadioItem(ID_AtomPolyNone, wxT("None"));
    pmAtomPoly->AppendRadioItem(ID_AtomPolyAuto, wxT("Auto"));
    pmAtomPoly->AppendRadioItem(ID_AtomPolyRegular, wxT("Regular"));
    pmAtomPoly->AppendRadioItem(ID_AtomPolyPyramid, wxT("Pyramid"));
    pmAtomPoly->AppendRadioItem(ID_AtomPolyBipyramid, wxT("Bipyramid"));

    pmAtomPart->AppendRadioItem(ID_AtomPartChangeLast, wxT("X"));
    TStrList pn("-2;-1;0;1;2", ';');
    for (size_t i = 0; i < pn.Count(); i++) {
      pmAtomPart->AppendRadioItem(ID_AtomPartChange+i, pn[i].u_str());
      Bind(wxEVT_COMMAND_MENU_SELECTED,
        &TMainForm::OnAtomPartChange, this, ID_AtomPartChange + i);
    }

    pmAtomUiso->AppendRadioItem(ID_AtomUisoCustom, wxT("X"));
    pmAtomUiso->AppendRadioItem(ID_AtomUiso12, wxT("1.2x"));
    pmAtomUiso->AppendRadioItem(ID_AtomUiso15, wxT("1.5x"));
    pmAtomUiso->AppendRadioItem(ID_AtomUisoFree, wxT("Free"));
    pmAtomUiso->AppendRadioItem(ID_AtomUisoFix, wxT("Fix"));

  pmAtom->Append(ID_MenuAtomType, wxT("Type"), pmAtomType);
  pmAtom->Append(ID_MenuAtomConn, wxT("Bonds"), pmAtomConn);
  pmAtom->Append(ID_MenuAtomOccu, wxT("Chemical occupancy"), pmAtomOccu);
  pmAtom->Append(ID_MenuAtomPart, wxT("Part"), pmAtomPart);
  pmAtom->Append(ID_MenuAtomUiso, wxT("Uiso"), pmAtomUiso);
  pmAtom->Append(ID_MenuAtomPoly, wxT("Polyhedron"), pmAtomPoly);
  pmAtom->AppendSeparator();
  pmAtom->Append(ID_AtomGrow, wxT("Grow"));
  pmAtom->Append(ID_AtomCenter, wxT("Center"));
  pmAtom->Append(ID_AtomSelRings, wxT("Select ring(s)"));
  pmAtom->AppendSeparator();
  pmAtom->Append(ID_GraphicsKill, wxT("Delete"));
  pmAtom->AppendSeparator();
  pmAtom->Append(ID_MenuFragment, wxT("Fragment"), pmFragment->Clone());
  pmAtom->Append(ID_MenuGraphics, wxT("Graphics"), pmGraphics->Clone());
  pmAtom->Append(ID_Selection, wxT("Selection"), pmSelection->Clone());
// setting bond menu
  pmBond->Append(ID_BondInfo, wxT("?"));
  pmBond->Append(ID_BondRadius, wxT("?"));
  pmBond->AppendSeparator();
  pmBond->Append(ID_MenuTang, wxT("TANG"), pmTang);
  pmBond->AppendSeparator();
  pmBond->Append(ID_BondViewAlong, wxT("View along"));
  pmBond->Append(ID_GraphicsKill, wxT("Delete"));
  pmBond->AppendSeparator();
  pmBond->Append(ID_MenuFragment, wxT("Fragment"), pmFragment->Clone());
  pmBond->Append(ID_MenuGraphics, wxT("Graphics"), pmGraphics->Clone());
  pmBond->Append(ID_Selection, wxT("Selection"), pmSelection->Clone());
// setting plane menu
  pmPlane->Append(ID_PlaneActivate, wxT("View along normal"));
  pmPlane->Append(ID_GraphicsKill, wxT("Delete"));
  pmPlane->Append(1, wxT("Graphics"), pmGraphics->Clone());
  pmPlane->Enable(ID_GraphicsCollectivise, false);
  pmPlane->Enable(ID_GraphicsIndividualise, false);
  pmPlane->Append(ID_Selection, wxT("Selection"), pmSelection->Clone());
  pmPlane->AppendSeparator();
// setting view menu
  TStrList vn("100;010;001;110;101;011;111", ';');
  for (size_t i = 0; i < vn.Count(); i++) {
    pmView->Append(ID_ViewAlong+i, vn[i].u_str());
    Bind(wxEVT_COMMAND_MENU_SELECTED,
      &TMainForm::OnViewAlong, this, ID_ViewAlong + i);
  }
// update to selection menu - need to add graphics
  pmSelection->AppendSeparator();
  pmSelection->Append(ID_MenuGraphics, wxT("Graphics"), pmGraphics->Clone());
  pmSelection->Enable(ID_GraphicsCollectivise, false);
  pmSelection->Enable(ID_GraphicsIndividualise, false);

  Menus.Add("File", MenuFile);
  Menus.Add("View", MenuView);
  Menus.Add("Structure", MenuStructure);
  Menus.Add("Help", MenuHelp);

  SetMenuBar(MenuBar);
//////////////////////////////////////////////////////////////
  FXApp->GetRenderer().OnDraw.Add(this, ID_GLDRAW, msiExit);
  TObjectVisibilityChange* VC = new TObjectVisibilityChange(this);
  XA->OnGraphicsVisible.Add(VC);
  // put correct captions to the menu
  pmModel->SetLabel(ID_CellVisible, FXApp->IsCellVisible() ?
    wxT("Hide cell") : wxT("Show cell"));
  pmModel->SetLabel(ID_BasisVisible, FXApp->IsBasisVisible() ?
    wxT("Hide basis") : wxT("Show basis"));
  TutorialDir = XA->GetBaseDir()+"etc/";
//  DataDir = TutorialDir + "Olex_Data\\";
  FHtmlIndexFile = TutorialDir+"index.htm";

  TFileHandlerManager::AddBaseDir(TutorialDir);
  TFileHandlerManager::AddBaseDir(FXApp->GetInstanceDir());

  SetStatusText(XA->GetBaseDir().u_str());

  TBasicApp::GetLog().OnInfo.Add(this, ID_INFO, msiEnter);
  TBasicApp::GetLog().OnWarning.Add(this, ID_WARNING, msiEnter);
  TBasicApp::GetLog().OnError.Add(this, ID_ERROR, msiEnter);
  TBasicApp::GetLog().OnException.Add(this, ID_EXCEPTION, msiEnter);
  TBasicApp::GetLog().OnPost.Add(this, ID_LOG, msiExecute);
  FXApp->OnObjectsDestroy.Add(this, ID_XOBJECTSDESTROY, msiEnter);
  XLibMacros::OnDelIns().Add(this, ID_DELINS, msiExit);
  XLibMacros::OnAddIns().Add(this, ID_ADDINS, msiExit);
  LoadVFS(plGlobal);
  {
    long flags = wxBORDER_NONE | wxCLIP_CHILDREN;
#if wxMAJOR_VERSION == 3 && wxMINOR_VERSION == 0
    flags |= wxVSCROLL;
#else
    flags |= wxALWAYS_SHOW_SB;
#endif
    HtmlManager.InitialiseMain(flags);
  }
  GetLibrary().AttachLibrary(HtmlManager.ExportLibrary());

  HtmlManager.OnLink.Add(this, ID_ONLINK);
  HtmlManager.main->OnKey.Add(this, ID_HTMLKEY);

  FXApp->SetLabelsVisible(false);
  FXApp->GetRenderer().LightModel.SetClearColor(0x0f0f0f0f);

  FGlConsole = new TGlConsole(FXApp->GetRenderer(), "Console");
  // the commands are posted from in Dispatch, SkipPosting is controlling the output
  FXApp->GetLog().AddStream(FGlConsole, false);
  FGlConsole->OnCommand.Add(this, ID_COMMAND);
  FGlConsole->OnPost.Add(this, ID_TEXTPOST);
  FXApp->AddObjectToCreate(FGlConsole);
////////////////////////////////////////////////////////////////////////////////
  Library.AttachLibrary(FGlConsole->ExportLibrary());
  Library.AttachLibrary(FXApp->GetRenderer().ExportLibrary());
////////////////////////////////////////////////////////////////////////////////
  FCmdLine = new TCmdLine(this, wxNO_BORDER);
//  wxWindowDC wdc(this);
//  FCmdLine->WI.SetHeight(wdc.GetTextExtent(wxT("W")).GetHeight());
  FCmdLine->OnChar.Add(this, ID_CMDLINECHAR);
  FCmdLine->OnKeyDown.Add(this, ID_CMDLINEKEYDOWN);
  FCmdLine->OnCommand.Add( this, ID_COMMAND);

  FHelpWindow = new TGlTextBox(FXApp->GetRenderer(), "HelpWindow");
  FXApp->AddObjectToCreate(FHelpWindow);
  FHelpWindow->SetVisible(false);

  FInfoBox = new TGlTextBox(FXApp->GetRenderer(), "InfoBox");
  FXApp->AddObjectToCreate(FInfoBox);

  GlTooltip = new TGlTextBox(FXApp->GetRenderer(), "Tooltip");
  FXApp->AddObjectToCreate(GlTooltip);
  GlTooltip->SetVisible(false);
  GlTooltip->SetZ(4.9);

  FTimer->OnTimer.Add(&TBasicApp::GetInstance().OnTimer);
  TBasicApp::GetInstance().OnTimer.Add(this, ID_TIMER);
  FXApp->XFile().OnFileLoad.Add(this, ID_FileLoad);
  FXApp->XFile().OnFileClose.Add(this, ID_FileClose);
  FXApp->XFile().GetRM().OnSetBadReflections.Add(this, ID_BadReflectionSet);
  FXApp->XFile().GetRM().OnCellDifference.Add(this, ID_CellChanged);
  TStateRegistry &states = FXApp->GetStatesRegistry();
  stateHtmlVisible = states.Register("htmlvis",
    new TStateRegistry::Slot(
      states.NewGetter<TMainForm>(this, &TMainForm::CheckState),
      new TStateRegistry::TMacroSetter("HtmlPanelVisible")
    )
  );
  stateInfoWidnowVisible = states.Register("infovis",
    new TStateRegistry::Slot(
      TStateRegistry::NewGetter<TGlTextBox>(FInfoBox, &TGlTextBox::IsVisible),
      new TStateRegistry::TMacroSetter("ShowWindow info")
    )
  );
  stateHelpWindowVisible = states.Register("helpvis",
    new TStateRegistry::Slot(
      TStateRegistry::NewGetter<TGlTextBox>(FHelpWindow, &TGlTextBox::IsVisible),
      new TStateRegistry::TMacroSetter("ShowWindow help")
    )
  );
  stateCmdLineVisible = states.Register("cmdlinevis",
    new TStateRegistry::Slot(
      states.NewGetter<TMainForm>(this, &TMainForm::CheckState),
      new TStateRegistry::TMacroSetter("ShowWindow cmdline")
    )
  );
  stateGlTooltips = states.Register("GLTT",
    new TStateRegistry::Slot(
    states.NewGetter<TMainForm>(this, &TMainForm::CheckState),
    new TStateRegistry::TMacroSetter("GlTooltip")
    )
  );


  // synchronise if value is different in settings file...
  miHtmlPanel->Check(!FHtmlMinimized);
#if defined(__WIN32__) || defined(__MAC__)
  StartupInit();
  if (Destroying) {
    Close(true);
    return;
  }
#endif
  try  {
    nui_interface = olx_nui::Initialise();
    if (nui_interface != 0) {
      nui_interface->InitProcessing(olx_nui::INUI::processSkeleton
        | olx_nui::INUI::processVideo);
    }
  }
  catch(const TExceptionBase &e)  {
    FXApp->NewLogEntry(logError) << e.GetException()->GetError();
  }
  TBasicApp::GetInstance().NewActionQueue(olxappevent_UPDATE_GUI)
    .Add(this, ID_UPDATE_GUI);
}
//..............................................................................
void TMainForm::StartupInit() {
  if (StartupInitialised) {
    return;
  }
  StartupInitialised = true;
  if (FGlCanvas != 0) {
    FGlCanvas->XApp(FXApp);
  }
  wxFont Font(10, wxMODERN, wxNORMAL, wxNORMAL);//|wxFONTFLAG_ANTIALIASED);
  TGlMaterial glm("2049;0.698,0.698,0.698,1.000");
  AGlScene& gls = FXApp->GetRenderer().GetScene();
  TGlFont &fnt_def = gls.CreateFont("Default", Font.GetNativeFontInfoDesc());
  FHelpWindow->SetFontIndex(
    gls.CreateFont("Help", Font.GetNativeFontInfoDesc()).GetId());
  FInfoBox->SetFontIndex(
    gls.CreateFont("Notes", Font.GetNativeFontInfoDesc()).GetId());
  TGlFont& fnt_lb = gls.CreateFont("Labels", Font.GetNativeFontInfoDesc());
  gls.RegisterFontForType<TXAtom>(
    gls.CreateFont("AtomLabels", Font.GetNativeFontInfoDesc()));
  gls.RegisterFontForType<TXBond>(
    gls.CreateFont("BondLabels", Font.GetNativeFontInfoDesc()));
  gls.RegisterFontForType<TXAngle>(
    gls.CreateFont("AngleLabels", Font.GetNativeFontInfoDesc()));
  GlTooltip->SetFontIndex(
    gls.CreateFont("Tooltip", Font.GetNativeFontInfoDesc()).GetId());
  gls.RegisterFontForType<TDBasis>(fnt_lb);
  gls.RegisterFontForType<TDUnitCell>(fnt_lb);
  gls.RegisterFontForType<TXGlLabels>(fnt_lb);
  gls.RegisterFontForType<TGlConsole>(fnt_def);
  gls.RegisterFontForType<TGlCursor>(fnt_def);

  for (size_t i = 0; i < gls.FontCount(); i++) {
    gls._GetFont(i).SetMaterial(glm);
  }


  olxstr T(FXApp->GetConfigDir());
  T << FLastSettingsFile;
  if (!TEFile::Exists(T)) {
    T = TBasicApp::GetBaseDir();
    TEFile::AddPathDelimeterI(T);
    T << FLastSettingsFile;
  }
  AGlScene &sc = FXApp->GetRenderer().GetScene();
  sc.materials.Add("Help_txt", &HelpFontColorTxt);
  sc.materials.Add("Help_cmd", &HelpFontColorCmd);
  sc.materials.Add("Exec", &ExecFontColor);
  sc.materials.Add("Info", &InfoFontColor);
  sc.materials.Add("Warning", &WarningFontColor);
  sc.materials.Add("Error", &ErrorFontColor);
  sc.materials.Add("Exception", &ExceptionFontColor);
  try  {
    LoadSettings(T);
    olxstr hfn = FXApp->GetConfigDir() + "history.txt";
    if (TEFile::Exists(hfn)) {
      TStrList h = TEFile::ReadLines(hfn);
      FGlConsole->SetCommands(h);
    }
   }
  catch(const TExceptionBase& e)  {
    TBasicApp::NewLogEntry(logExceptionTrace) << e;
    FXApp->CreateObjects(false);
    ShowAlert(e);
    //throw;
  }
  FXApp->Init(); // initialise the gl after styles reloaded
  if (!GradientPicture.IsEmpty()) { // need to call it after all objects are created
    processMacro(olxstr("grad ") << " -p=\'" << GradientPicture << '\'');
  }

  processMacro(olxstr("showwindow help ") << HelpWindowVisible);
  processMacro(olxstr("showwindow info ") << InfoWindowVisible);
  processMacro(olxstr("showwindow cmdline ") << CmdLineVisible);
  FGlConsole->ShowBuffer(true);  // this should be on :)
  processMacro("reload macro", __OlxSrcInfo);
  processMacro("reload help", __OlxSrcInfo);

  FTimer->Start(15);

  if (TEFile::Exists(FXApp->GetBaseDir() + "settings.xld")) {
    TDataFile settings;
    settings.LoadFromXLFile(FXApp->GetBaseDir() + "settings.xld", 0);
    settings.Include(0);
    TDataItem* sh = settings.Root().FindItemi("shortcuts");
    if (sh != 0) {
      try {
        olxstr cmd;
        for (size_t i=0; i < sh->ItemCount(); i++) {
          TDataItem& item = sh->GetItemByIndex(i);
          AccShortcuts.AddAccell(
            TranslateShortcut(item.FindField("key")),
            item.FindField("macro"));
          // cannot execute it through a macro - functions get evaluated...
          //Macros.ProcessMacro(cmd, MacroError);
        }
      }
      catch (TExceptionBase& exc) {
        TBasicApp::NewLogEntry(logException) <<
          exc.GetException()->GetFullMessage();
      }
    }
    sh = settings.Root().FindItemi("menus");
    ABasicFunction *cm_macro = GetLibrary().FindMacro("CreateMenu");
    if (sh != 0 && cm_macro != 0) {
      TMacroData me;
      me.SetLocation(__OlxSrcInfo);
      try {
        for (size_t i=0; i < sh->ItemCount(); i++) {
          TDataItem& item = sh->GetItemByIndex(i);
          TStrObjList params;
          TParamList opts;
          params << item.FindField("title") << item.FindField("macro");
          olxstr bf = item.FindField("before");
          if (!bf.IsEmpty()) {
            params << bf;
          }

          olxstr modeDep = item.FindField("modedependent");
          if (!modeDep.IsEmpty()) {
            opts.AddParam('m', modeDep);
          }
          olxstr stateDep = item.FindField("statedependent");
          if (!stateDep.IsEmpty()) {
            opts.AddParam('s', stateDep);
          }
          if (item.GetName().Equalsi("radio")) {
            opts.AddParam('r', EmptyString());
          }
          if (item.GetName().Equalsi("sep")) {
            opts.AddParam('#', EmptyString());
          }
          if (item.GetName().Equalsi("check")) {
            opts.AddParam('c', EmptyString());
          }
          cm_macro->Run(params, opts, me);
        }
      }
      catch (TExceptionBase& exc) {
        TBasicApp::NewLogEntry(logExceptionTrace) << exc;
      }
    }
  }

  // set the variables
  for (size_t i=0; i < StoredParams.Count(); i++) {
    processMacro(olxstr("setvar(") << StoredParams.GetKey(i) << ",\"" <<
      StoredParams.GetValue(i) << "\")");
  }

  olxstr textures_dir = FXApp->GetBaseDir() + "etc/Textures";
  if (TEFile::IsDir(textures_dir)) {
    FXApp->LoadTextures(textures_dir);
  }

  olxstr load_file;
  bool is_arg = false;
  if (FXApp->GetArguments().Count() >= 2) {
    // do the iterpreters job if needed
    if (FXApp->GetArguments().GetLastString().EndsWith(".py")) {
      TStrList in = TEFile::ReadLines(FXApp->GetArguments().GetLastString());
      PythonExt::GetInstance()->RunPython(in.Text('\n'));
    }
    else {
      load_file = FXApp->GetArguments().Text(' ', 1);
      if (!TEFile::Exists(load_file)) {
        load_file.SetLength(0);
      }
      else {
        TOlxVars::UnsetVar("startup");
        is_arg = true;
      }
    }
  }
  else {
    load_file = TOlxVars::FindValue("startup");
    if (!load_file.IsEmpty()) {
      load_file = TEFile::ExpandRelativePath(load_file);
    }
  }

  bool do_load_file = true;
  if (!load_file.IsEmpty() && !load_file.Equalsi("none") &&
    TEFile::Exists(load_file))
  {
#ifdef _WIN32
    ListOlex2OpenedFiles();
    size_t idx = LoadedFileIdx(load_file);
    olxstr res = "Y";
    if (idx != InvalidIndex) {
      olxstr opt_n = "on_loaded_file";
      res = TBasicApp::GetInstance().GetOptions().FindValue(opt_n);
      if (res.IsEmpty()) {
        res = TdlgMsgBox::Execute(this, olxstr("The file \n'") <<
          TEFile::ChangeFileExt(load_file, EmptyString()) << '\'' <<
          "\nhas been loaded in another instance of Olex2."
          "\nWould you like to open it in a this instance of Olex2?",
          "Please confirm",
          "Remember my decision",
          wxYES_NO | wxCANCEL | wxICON_QUESTION,
          true);
        if (res.StartsFrom('R')) {
          TBasicApp::GetInstance().UpdateOption(opt_n, res.SubStringFrom(1));
          TBasicApp::GetInstance().SaveOptions();
        }
      }
    }
    if (res.Contains('N')) {
      do_load_file = false;
    }
    else if (res.Contains('C')) {
      Destroying = true;
      HWND wnd = loadedFiles.GetObject(idx);
      TGlXApp::ActivateWindow(wnd);
      return;
    }
    loadedFiles.Clear();
#endif
  }
  else {
    do_load_file = false;
  }
  processMacro("onstartup", __OlxSrcInfo);
  processMacro("user_onstartup", __OlxSrcInfo);
  if (do_load_file) {
    olxstr reap = "reap -check_loaded";
    if (is_arg) { // if parsed as an argument - do not check for crash
      reap << ' ' << "-check_crashed";
    }
    reap << ' ' << '"' << load_file << '\"';
    processMacro(reap, __OlxSrcInfo);
  }

  // load html in last call - it might call some destructive functions on uninitialised data
  HtmlManager.main->LoadPage(FHtmlIndexFile.u_str());
  HtmlManager.main->SetHomePage(FHtmlIndexFile);
  FileDropTarget* dndt = new FileDropTarget(*this);
  this->SetDropTarget(dndt);
  processMacro("schedule 'update -f=false' -g");
  TStateRegistry::GetInstance().RepeatAll();
  try {
    TEFile f(olxstr(FXApp->GetInstanceDir()) << getpid() << ".ready", "wb");
    f.Writeln(TETime::FormatDateTime(TETime::msNow()));
  }
  catch (...) {
  }
}
//..............................................................................
bool TMainForm::CreateUpdateThread(bool force, bool reinstall, bool cleanup) {
  volatile olx_scope_cs cs(TBasicApp::GetCriticalSection());
  if (_UpdateThread != NULL || !updater::UpdateAPI().WillUpdate(force))
    return false;
  {
    olxstr tfn = TBasicApp::GetSharedDir() + "app.token";
    if (!TEFile::Exists(tfn)) {
      int answer = wxMessageBox(wxString("The program usage statistics can "
        "provide very helpful information to its developers. Would you like to "
        "contribute into the Olex2 development by providing us with"
        " anonymous information regarding its use?"),
        wxT("Help needed!"),
        wxYES_NO | wxCANCEL | wxYES_DEFAULT | wxICON_QUESTION);
      TCStrList sl;
      if (answer == wxYES) {
        sl.Add("at=") << MD5::Digest(
          olxcstr(rand()) << TETime::msNow() << rand());
      }
      if (answer != wxCANCEL) {
        TEFile::WriteLines(tfn, sl); // NO creates empty file
      }
    }
  }
#ifndef __WIN32__ // do updates on non-Win only if the folder is writable
  if( FXApp->IsBaseDirWriteable() )  {
#endif
    _UpdateThread = new UpdateThread(FXApp->GetInstanceDir() +
      patcher::PatchAPI::GetPatchFolder(),
      force,
      reinstall,
      cleanup);
    _UpdateThread->OnTerminate.Add(this, ID_UpdateThreadTerminate);
    _UpdateThread->OnDownload.Add(this, ID_UpdateThreadDownload);
    _UpdateThread->OnAction.Add(this, ID_UpdateThreadAction);
    _UpdateThread->Start();
    return true;
#ifndef __WIN32__
  }
  else
    return false;
#endif
}
//..............................................................................
bool TMainForm::Dispatch(int MsgId, short MsgSubId, const IOlxObject *Sender,
  const IOlxObject *Data, TActionQueue *)
{
  if (Destroying) {
    FMode = 0;  // to release waitfor
    return false;
  }

  if (MsgId == ID_TIMER && wxThread::IsMain() &&
    StartupInitialised && Py_IsInitialized() && PyEval_ThreadsInitialized())
  {
    size_t tc = OlexPyCore::GetRunningPythonThreadsCount();
    if (tc > 0) {
      PyGILState_STATE st = PyGILState_Ensure();
      Py_BEGIN_ALLOW_THREADS;
      olx_sleep(5);
      Py_END_ALLOW_THREADS;
      PyGILState_Release(st);
    }
  }

  bool res = true, Silent = (FMode & mSilent) != 0, Draw = false;
  static bool actionEntered = false, downloadEntered = false;
  if (MsgId == ID_GLDRAW && !IsIconized()) {
    if (!FBitmapDraw) {
      //glLoadIdentity();
      //glRasterPos3d(-0.5,-0.5,-1.6);
      //wxMemoryDC dc;
      //const wxSize glsz = FGlCanvas->GetSize();
      //wxBitmap bmp(glsz.GetWidth(), glsz.GetHeight());
      //dc.SelectObject(bmp);
      //GLfloat rgba[4];
      //olx_gl::get(GL_COLOR_CLEAR_VALUE, rgba);
      //dc.SetBackground(wxBrush(RGBA(rgba[0]*255,rgba[1]*255,rgba[2]*255,rgba[3]*255)));
      //dc.Clear();
      //dc.SetPen(*wxBLACK_PEN);
      //wxFont wxf(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_LIGHT, false, wxT(""), wxFONTENCODING_ISO8859_5);
      //dc.SetFont(wxf);
      //const wxSize tsz = dc.GetTextExtent(wxT("X"));
      //int y=glsz.GetHeight()-tsz.GetY()*2;
      //for( int i=FGlConsole->Buffer().Count()-1; i >=0; i-- )  {
      //  dc.DrawText( FGlConsole->Buffer()[i].u_str(), 0, y);
      //  y -= tsz.GetY();
      //  if( y < 0 )  break;
      //}
      //dc.SelectObject(wxNullBitmap);
      //wxImage img = bmp.ConvertToImage();
      //char* bf = new char[glsz.GetWidth()*glsz.GetHeight()*4];
      //const unsigned char* data = img.GetData();
      //for( int i=0; i < glsz.GetWidth(); i++ )  {
      //  for( int j=0; j < glsz.GetHeight(); j++ )  {
      //    const int ind1 = (j*glsz.GetWidth()+i)*3;
      //    const int ind2 = ((glsz.GetHeight()-j-1)*glsz.GetWidth()+i)*4;
      //    bf[ind2+0] = data[ind1+0];
      //    bf[ind2+1] = data[ind1+1];
      //    bf[ind2+2] = data[ind1+2];
      //    bf[ind2+3] = 0xff; //(data[ind1+0] == 0xFF && data[ind1+1] == 0xFF && data[ind1+2] == 0xFF) ? 0 : 0xFF;
      //  }
      //}
      //olx_gl::drawBuffer(GL_BACK);
      //olx_gl::rasterPos(0,0,0);
      //olx_gl::drawPixels(glsz.GetWidth(), glsz.GetHeight(), GL_RGBA, GL_UNSIGNED_BYTE, bf);
      //delete [] bf;
      FGlCanvas->SwapBuffers();
      OnNonIdle();
    }
    //wxClientDC dc(FGlCanvas);
    //dc.DrawText(wxT("RRRRRRRRRRRR"), 0, 0);
  }
  //else if( MsgId == ID_VarChange )  {
  //  if( Data != NULL && EsdlInstanceOf(*Data, TOlxVarChangeData) && FGlConsole != NULL )  {
  //    TOlxVarChangeData& vcd = *(TOlxVarChangeData*)Data;
  //    if( GlConsoleBlendVarName.Comparei(vcd.var_name) == 0 )  {
  //      if( !vcd.str_val.IsEmpty() )
  //        FGlConsole->SetBlend(vcd.str_val.ToBool());
  //    }
  //  }
  //}
  else if (MsgId == ID_UpdateThreadTerminate) {
    volatile olx_scope_cs cs(TBasicApp::GetCriticalSection());
    _UpdateThread = 0;

    if (UpdateProgress != 0) {
      delete UpdateProgress;
      UpdateProgress = 0;
    }
  }
  else if (MsgId == ID_UpdateThreadDownload) {
    volatile olx_scope_cs cs(TBasicApp::GetCriticalSection());
    if (MsgSubId == msiEnter) {
      if (UpdateProgress == 0)
        UpdateProgress = new TOnProgress;
    }
    if (MsgSubId == msiExecute && Data != 0 && Data->Is<TOnProgress>()) {
      TOnProgress& pg = *(TOnProgress*)Data;
      if (UpdateProgress != 0) {
        *UpdateProgress = pg;
      }
      downloadEntered = true;
      AOlxThread::Yield();
    }
    else if (MsgSubId == msiExit) {
      if (UpdateProgress != 0) {
        delete UpdateProgress;
        UpdateProgress = 0;
      }
    }
  }
  else if (MsgId == ID_UpdateThreadAction) {
    volatile olx_scope_cs cs(TBasicApp::GetCriticalSection());
    if (MsgSubId == msiEnter) {
      if (ActionProgress == 0) {
        ActionProgress = new TOnProgress;
      }
    }
    else if (MsgSubId == msiExecute && Data != 0 && Data->Is<TOnProgress>()) {
      TOnProgress& pg = *(TOnProgress*)Data;
      if (ActionProgress != 0) {
        *ActionProgress = pg;
      }
      actionEntered = true;
      AOlxThread::Yield();
    }
    else if (MsgSubId == msiExit) {
      if (ActionProgress != 0) {
        delete ActionProgress;
        ActionProgress = 0;
      }
    }
  }
  else if (MsgId == ID_TIMER) {
    FTimer->OnTimer.SetEnabled(false);
    if (nui_interface != 0) {
      nui_interface->DoProcessing();
    }
    // execute tasks ...
    for (size_t i = 0; i < Tasks.Count(); i++) {
      if (Tasks[i].NeedsGUI && !this->IsShownOnScreen())
        continue;
      if ((TETime::Now() - Tasks[i].LastCalled) > Tasks[i].Interval) {
        olxstr tmp = Tasks[i].Task;
        if (!Tasks[i].Repeatable) {
          Tasks.Delete(i);
          i--;
        }
        else {
          Tasks[i].LastCalled = TETime::Now();
        }
        processMacro(tmp, "Scheduled task");
      }
    }
    for (size_t i = 0; i < RunWhenVisibleTasks.Count(); i++) {
      RunWhenVisibleTasks[i]->Run();
    }
    RunWhenVisibleTasks.DeleteItems().Clear();
    // end tasks ...
    FTimer->OnTimer.SetEnabled(true);
    if ((FMode & mListen) != 0 && TEFile::Exists(FListenFile)) {
      static time_t FileMT = TEFile::FileAge(FListenFile);
      time_t FileT = TEFile::FileAge(FListenFile);
      if (FileMT != FileT) {
        FObjectUnderMouse = 0;
        processMacro((olxstr("reap_listen \"") << FListenFile) + '\"', "OnListen");
        FileMT = FileT;
      }
    }
    if ((FMode & mListenCmd) != 0 && TEFile::Exists(FListenCmdFile)) {
      static time_t FileMT = TEFile::FileAge(FListenCmdFile);
      time_t FileT = TEFile::FileAge(FListenCmdFile);
      if (FileMT != FileT) {
        FObjectUnderMouse = 0;
        const_cstrlist file_lines = TEFile::ReadCLines(FListenCmdFile);
        if (!file_lines.IsEmpty()) {
          processMacro(file_lines[0], "OnCmdListen");
          FXApp->Draw();
        }
        FileMT = FileT;
      }
    }
    if ((FMode & mRota) != 0) {
      FXApp->GetRenderer().GetBasis().RotateX(
        FXApp->GetRenderer().GetBasis().GetRX() + FRotationIncrement*FRotationVector[0]);
      FXApp->GetRenderer().GetBasis().RotateY(
        FXApp->GetRenderer().GetBasis().GetRY() + FRotationIncrement*FRotationVector[1]);
      FXApp->GetRenderer().GetBasis().RotateZ(
        FXApp->GetRenderer().GetBasis().GetRZ() + FRotationIncrement*FRotationVector[2]);
      FRotationAngle -= olx_abs(FRotationVector.Length()*FRotationIncrement);
      if (FRotationAngle < 0) {
        FMode ^= mRota;
      }
      Draw = true;
    }
    if ((FMode & mFade) != 0) {
      Draw = true;
      if (FFadeVector[0] == FFadeVector[1])
      {
        FMode ^= mFade;
      }//FXApp->GetRenderer().Ceiling()->Visible(false);  }

      FFadeVector[0] += FFadeVector[2];
      if (FFadeVector[2] > 0) {
        if (FFadeVector[0] > FFadeVector[1]) {
          FFadeVector[0] = FFadeVector[1];
          FMode ^= mFade;
        }
      }
      else {
        if (FFadeVector[0] < FFadeVector[1]) {
          FFadeVector[0] = FFadeVector[1];
          FMode ^= mFade;
        }
      }
      if ((FMode & mFade) != 0) {
        TGlOption glO;
        glO = FXApp->GetRenderer().Ceiling()->LT();  glO[3] = FFadeVector[0];
        FXApp->GetRenderer().Ceiling()->LT(glO);

        glO = FXApp->GetRenderer().Ceiling()->RT();  glO[3] = FFadeVector[0];
        FXApp->GetRenderer().Ceiling()->RT(glO);

        glO = FXApp->GetRenderer().Ceiling()->LB();  glO[3] = FFadeVector[0];
        FXApp->GetRenderer().Ceiling()->LB(glO);

        glO = FXApp->GetRenderer().Ceiling()->RB();  glO[3] = FFadeVector[0];
        FXApp->GetRenderer().Ceiling()->RB(glO);
        Draw = true;
      }
    }
    if (FXApp->GetFader().IsVisible()) {
      if (!FXApp->GetFader().Increment()) {
        FXApp->GetFader().SetVisible(false);
      }
      Draw = true;
    }
    if (MouseMoveTimeElapsed < 2500) {
      MouseMoveTimeElapsed += FTimer->GetInterval();
    }
    if (MouseMoveTimeElapsed > 500 && MouseMoveTimeElapsed < 5000) {
      olxstr tt = this->FXApp->GetObjectInfoAt(MousePositionX, MousePositionY);
      if (!_UseGlTooltip) {
        FGlCanvas->SetToolTip(tt.u_str());
      }
      else if (GlTooltip != 0) {
        if (tt.IsEmpty()) {
          if (GlTooltip->IsVisible()) {
            GlTooltip->SetVisible(false);
            Draw = true;
          }
        }
        else {
          GlTooltip->Clear();
          GlTooltip->PostText(tt);
          GlTooltip->Fit();
          int x = MousePositionX - GlTooltip->GetWidth() / 2,
            y = MousePositionY - GlTooltip->GetHeight() - 4;
          if (x < 0) {
            x = 0;
          }
          if ((size_t)(x + GlTooltip->GetWidth()) > (size_t)FXApp->GetRenderer().GetWidth()) {
            x = FXApp->GetRenderer().GetWidth() - GlTooltip->GetWidth();
          }
          if (y < 0) {
            y = 0;
          }
          GlTooltip->SetLeft(x); // put it off the mouse
          GlTooltip->SetTop(y);
          GlTooltip->SetZ(FXApp->GetRenderer().GetMaxRasterZ());
          GlTooltip->SetVisible(true);
          Draw = true;
        }
      }
      if (DrawSceneTimer > 0 && !Draw) {
        if (DrawSceneTimer < FTimer->GetInterval()) {
          TimePerFrame = FXApp->Draw();
        }
        else {
          DrawSceneTimer -= FTimer->GetInterval();
        }
      }
      MouseMoveTimeElapsed = 5000;
    }
    if (Draw) {
      TimePerFrame = FXApp->Draw();
    }
    // here it cannot be done with scope_cs - GTK would freese the main loop...
    TBasicApp::EnterCriticalSection();
    if (_UpdateThread != 0 && _UpdateThread->GetUpdateSize() != 0) {
      TBasicApp::LeaveCriticalSection();
      if (wxApp::IsMainLoopRunning()) {
        FTimer->OnTimer.SetEnabled(false);
        DoUpdateFiles();
        FTimer->OnTimer.SetEnabled(true);
      }
    }
    else {
      TBasicApp::LeaveCriticalSection();
    }
    // deal with updates
    if (wxIsMainThread()) {
      static bool UpdateExecuted = false;
      volatile olx_scope_cs cs(TBasicApp::GetCriticalSection());
      if (actionEntered && ActionProgress != 0) {
        StatusBar->SetStatusText((olxstr("Processing ") <<
          ActionProgress->GetAction()).u_str());
        actionEntered = false;
      }
      if (downloadEntered && UpdateProgress != 0) {
        downloadEntered = false;
        UpdateExecuted = true;
        StatusBar->SetStatusText(
          (olxstr("Downloading ") << UpdateProgress->GetAction() << ' ' <<
            olxstr::FormatFloat(2, UpdateProgress->GetPos() * 100 /
            (UpdateProgress->GetMax() + 1)) << '%').u_str()
        );
      }
      if (UpdateExecuted && _UpdateThread == 0) {
        StatusBar->SetStatusText(TBasicApp::GetBaseDir().u_str());
        UpdateExecuted = false;
      }
    }
  }
  else if (MsgId == ID_XOBJECTSDESTROY) {
    FObjectUnderMouse = 0;
    if (Modes->GetCurrent() != 0) {
      Modes->GetCurrent()->OnGraphicsDestroy();
    }
  }
  else if (MsgId == ID_FileLoad) {
    if (MsgSubId == msiExit) {
      olxstr title = "Olex2";
      if (FXApp->XFile().HasLastLoader()) {
        title << ": " << TEFile::ExtractFileName(FXApp->XFile().GetFileName())
          << ", " << FXApp->XFile().GetFileName();
      }
      this->SetTitle(title.u_str());
    }
  }
  else if (MsgId == ID_FileClose) {
    if (MsgSubId == msiExit) {
      UpdateRecentFile(EmptyString());
      UpdateInfoBox();
      SetTitle(wxT("Olex2"));
    }
  }
  else if (MsgId == ID_CMDLINECHAR) {
    if (Data != 0 && Data->Is<TKeyEvent>()) {
      this->OnChar(((TKeyEvent*)Data)->GetEvent());
    }
  }
  else if (MsgId == ID_CMDLINEKEYDOWN) {
    if (Data != 0 && Data->Is<TKeyEvent>()) {
      this->OnKeyDown(((TKeyEvent*)Data)->GetEvent());
    }
  }
  else if ((MsgId == ID_INFO || MsgId == ID_WARNING || MsgId == ID_ERROR ||
    MsgId == ID_EXCEPTION) && (MsgSubId == msiEnter))
  {
    if (Data != 0) {
      TGlMaterial *glm = 0;
      if (MsgId == ID_INFO) {
        glm = &InfoFontColor;
      }
      else if (MsgId == ID_WARNING) {
        glm = &WarningFontColor;
      }
      else if (MsgId == ID_ERROR) {
        glm = &ErrorFontColor;
      }
      else if (MsgId == ID_EXCEPTION) {
        glm = &ExceptionFontColor;
      }
      if (!((FMode&mSilent) != 0 && (MsgId == ID_INFO)) ||
        (MsgId == ID_WARNING || MsgId == ID_ERROR || MsgId == ID_EXCEPTION))
      {
        // the proporgation will happen after we return false
        FGlConsole->OnPost.SetEnabled(false);
        FGlConsole->PrintText(Data->ToString(), glm, true);
        FGlConsole->PrintText(EmptyString());
        FGlConsole->OnPost.SetEnabled(true);
        TimePerFrame = FXApp->Draw();
      }
      FGlConsole->SetSkipPosting(true);
      res = false;  // propargate to other streams, logs in particular
    }
  }
  else if (MsgId == ID_LOG && (MsgSubId == msiExecute)) {
    if (Data != 0) {
      callCallbackFunc(OnLogCBName, TStrList() << Data->ToString());
    }
  }
  else if (MsgId == ID_ONLINK) {
    if (Data != 0 && Data->Is<olxstr>()) {
      TStrList Toks = TParamList::StrtokLines(*(olxstr*)Data, ">>");
      //GetHtml()->LockPageLoad();
      /* the page, if requested, will beloaded on time event. The timer is disabled
      in case if a modal window appears and the timer event can be called */
      FTimer->OnTimer.SetEnabled(false);
      for (size_t i = 0; i < Toks.Count(); i++) {
        if (!processMacro(olxstr::DeleteSequencesOf<char>(Toks[i], ' '), "OnLink")) {
          break;
        }
      }
      TimePerFrame = FXApp->Draw();
      // enabling the timer back
      // retrun fucus to the main window, but let typing in the comboboxes
      if (Sender != 0) {
        const std::type_info &ti = typeid(*Sender);
        if (((olxstr*)Data)->IsEmpty()) {
          ;
        }
        else if (ti == typeid(TComboBox) &&
          !(dynamic_cast<const TComboBox *>(Sender))->IsReadOnly())
        {
        }
        else if (ti == typeid(TTreeView))
          ;
        else if (ti == typeid(TTextEdit))
          ;
        else if (ti == typeid(TSpinCtrl))
          ;
        else if (ti == typeid(TDateCtrl))
          ;
        else {
          if (CmdLineVisible) {
            FCmdLine->SetFocus();
          }
          else {
            FGlCanvas->SetFocus();
          }
        }
      }
      else {
        if (CmdLineVisible) {
          FCmdLine->SetFocus();
        }
        else {
          FGlCanvas->SetFocus();
        }
      }
      FTimer->OnTimer.SetEnabled(true);
    }
  }
  else if (MsgId == ID_HTMLKEY) {
    if (CmdLineVisible) {
      FCmdLine->SetFocus();
    }
    else {
      FGlCanvas->SetFocus();
    }
    OnChar(((TKeyEvent*)Data)->GetEvent());
  }
  else if (MsgId == ID_TEXTPOST) {
    if (Data != 0) {
      FGlConsole->SetSkipPosting(true);
      TBasicApp::NewLogEntry() << olxstr(Data->ToString());
      FGlConsole->SetSkipPosting(false);
    }
  }
  else if (MsgId == ID_COMMAND) {
    olxstr tmp;
    if (CmdLineVisible && Sender->Is<TCmdLine>()) {
      tmp = FCmdLine->GetCommand();
    }
    else if (Sender->Is<TGlConsole>()) {
      tmp = FGlConsole->GetCommand();
    }
    if (!tmp.IsEmpty()) {
      if (_ProcessManager->GetRedirected() != 0) {
        _ProcessManager->GetRedirected()->Write(tmp);
        _ProcessManager->GetRedirected()->Writenl();
        TimePerFrame = FXApp->Draw();
      }
      else {
        FHelpWindow->SetVisible(false);
        olxstr FullCmd(tmp);
        if (tmp.StartsFrom('!')) {
          if (CmdLineVisible && Sender->Is<TCmdLine>()) {
            tmp = FCmdLine->GetLastCommand(tmp.SubStringFrom(1));
          }
          else if (Sender->Is<TGlConsole>()) {
            tmp = FGlConsole->GetLastCommand(tmp.SubStringFrom(1));
          }
          if (!tmp.IsEmpty()) {
            FullCmd = tmp;
            TBasicApp::NewLogEntry() << FullCmd;
          }
        }
        processMacro(FullCmd, "Console");
      }
      if (CmdLineVisible && Sender->Is<TCmdLine>()) {
        FCmdLine->SetCommand(EmptyString());
      }
      else {
        FGlConsole->SetCommand(EmptyString());
      }
    }
  }
  else if (MsgId == ID_DELINS) {
    if (Data != 0 && Data->Is<olxstr>()) {
      if (((olxstr*)Data)->Equalsi("OMIT")) {
        BadReflectionsTable(false);
        processMacro("html.update");
      }
    }
  }
  else if (MsgId == ID_ADDINS) {
    if (Data != 0 && Data->Is<olxstr>()) {
      if (((olxstr*)Data)->Equalsi("OMIT")) {
        BadReflectionsTable(false);
        processMacro("html.update");
      }
    }
  }
  else if (MsgId == ID_BadReflectionSet) {
    if (MsgSubId == msiExit) {
      BadReflectionsTable(false);
    }
  }
  else if (MsgId == ID_UPDATE_GUI) {
    processMacro("html.update");
  }
  else if (MsgId == ID_CellChanged) {
    if (Data != 0 && Data->Is<TIns>()) {
      const TIns *hf = dynamic_cast<const TIns*>(Data);
      RunWhenVisibleTasks.Add(
        new CellChangeTask(FXApp->XFile().GetRM().GetHKLSource(),
          hf->GetAsymmUnit()));
    }
  }
  return res;
}
//..............................................................................
void TMainForm::PreviewHelp(const olxstr& Cmd)  {
  if( !HelpWindowVisible )  return;
  //if( !Cmd.IsEmpty() )  {
  //  TPtrList<macrolib::TEMacro> macros;
  //  Macros.FindSimilar(Cmd, macros);
  //  if( !macros.IsEmpty() )  {
  //    FHelpWindow->Clear();
  //    FHelpWindow->SetVisible(HelpWindowVisible);
  //    FGlConsole->ShowBuffer(!HelpWindowVisible);
  //    FHelpWindow->SetTop(
  //      InfoWindowVisible ? FInfoBox->GetTop() + FInfoBox->GetHeight() + 5 : 1);
  //    FHelpWindow->SetMaxStringLength((uint16_t)(
  //      FHelpWindow->GetFont().MaxTextLength(FXApp->GetRenderer().GetWidth())));
  //    FHelpWindow->SetZ(FXApp->GetRenderer().CalcRasterZ(0.1));
  //    for( size_t i=0; i < macros.Count(); i++ )  {
  //      FHelpWindow->PostText(macros[i]->GetName(), &HelpFontColorCmd);
  //      if( !macros[i]->GetDescription().IsEmpty() )  {
  //        FHelpWindow->PostText(macros[i]->GetDescription(), &HelpFontColorTxt);
  //        //Cat = Item->FindItem("category");
  //        //if( Cat != NULL  )  {
  //        //  olxstr Categories;
  //        //  for( size_t j=0; j < Cat->ItemCount(); j++ )  {
  //        //    Categories << Cat->GetItem(j).GetName();
  //        //    if( (j+1) < Cat->ItemCount() )  Categories << ", ";
  //        //  }
  //        //  if( !Categories.IsEmpty() )  {
  //        //    Categories.Insert("\t", 0);
  //        //    FHelpWindow->PostText("\tCategory", &HelpFontColorCmd);
  //        //    FHelpWindow->PostText(Categories, &HelpFontColorTxt);
  //        //  }
  //        //}
  //      }
  //    }
  //    FHelpWindow->Fit();
  //  }
  //  else  {
  //    FHelpWindow->SetVisible(false);
  //    FGlConsole->ShowBuffer(true);
  //  }
  //}
  //else  {
  //  FHelpWindow->SetVisible(false);
  //  FGlConsole->ShowBuffer(true);
  //}
}
//..............................................................................
bool TMainForm::ImportFrag(const olxstr& line) {
  if (!FXApp->XFile().HasLastLoader()) {
    return false;
  }
  olxstr trimmed_content = line;
  trimmed_content.Trim(' ').Replace('\r', '\n').Trim('\n').DeleteSequencesOf('\n');
  if (!trimmed_content.StartsFromi("FRAG") || !trimmed_content.EndsWithi("FEND")) {
    return false;
  }
  TStrList lines(trimmed_content, '\n');
  if (lines.Count() < 4) {
    return false;
  }
  lines.Delete(lines.Count() - 1);
  lines.Delete(0);
  for (size_t i = 0; i < lines.Count(); i++) {
    TStrList toks(lines[i].Trim('\r'), ' ');
    if (toks.Count() != 5) {
      lines[i].SetLength(0);
      continue;
    }
    toks.Delete(1);
    lines[i] = toks.Text(' ');
  }
  try {
    TXyz xyz;
    xyz.LoadFromStrings(lines);
    if (xyz.GetAsymmUnit().AtomCount() == 0) {
      return false;
    }
    processMacro("mode fit -a=6");
    TXAtomPList xatoms;
    TXBondPList xbonds;
    LabelCorrector lc(FXApp->XFile().GetAsymmUnit(), TXApp::GetMaxLabelLength(),
      TXApp::DoRenameParts());
    FXApp->AdoptAtoms(xyz.GetAsymmUnit(), xatoms, xbonds);
    for (size_t i = 0; i < xatoms.Count(); i++) {
      FXApp->XFile().GetRM().Vars.FixParam(
        xatoms[i]->CAtom(), catom_var_name_Sof);
      lc.Correct(xatoms[i]->CAtom());
      xatoms[i]->CAtom().SetPart(-1);
    }
    FXApp->CenterView(true);
    AMode *md = Modes->GetCurrent();
    if (md != 0) {
      md->AddAtoms(xatoms);
      for (size_t i = 0; i < xbonds.Count(); i++) {
        FXApp->GetRenderer().Select(*xbonds[i], true);
      }
    }
    if (FXApp->XFile().GetLattice().IsGenerated()) {
      Modes->OnModeExit.Add("fuse");
    }
    return true;
  }
  catch (...) {
    return false;
  }
}
//..............................................................................
bool TMainForm::ProcessTab() {
  olxstr FullCmd;
  olxstr Cmd = CmdLineVisible? FCmdLine->GetCommand()
    : FGlConsole->GetCommand();
  size_t spi = Cmd.LastIndexOf(' ');
  if (spi != InvalidIndex)  {
    FullCmd = ExpandCommand(Cmd.SubStringFrom(spi + 1), true);
    if (FullCmd != Cmd.SubStringFrom(spi + 1)) {
      FullCmd = Cmd.SubStringTo(spi + 1) << FullCmd;
    }
    else {
      FullCmd.SetLength(0);
    }
  }
  else {
    FullCmd = ExpandCommand(Cmd, false);
  }
  bool res = false;
  if (!FullCmd.IsEmpty() && (FullCmd != Cmd)) {
    if (CmdLineVisible) {
      FCmdLine->SetCommand(FullCmd);
    }
    else {
      FGlConsole->SetCommand(FullCmd);
    }
    res = true;
  }
  TimePerFrame = FXApp->Draw();
  return res;
}
//..............................................................................
void TMainForm::OnChar(wxKeyEvent& m) {
  OnNonIdle();
  m.Skip(false);
  short Fl = 0;
  if (m.GetModifiers() & wxMOD_ALT) {
    Fl |= sssAlt;
  }
  if (m.GetModifiers() & wxMOD_RAW_CONTROL) {
    Fl |= sssCtrl;
  }
  if (m.GetModifiers() & wxMOD_SHIFT) {
    Fl |= sssShift;
  }
  // Alt + Up,Down,Left, Right - rotation, +Shift - speed
  if (((Fl & sssShift)) || (Fl & sssAlt)) {
    int inc = 3;
    double zoom_inc = 0.01;
    if ((Fl & sssShift)) {
      inc *= 3;
      zoom_inc *= 3;
    }
    if (m.m_keyCode == WXK_UP) {
      FXApp->GetRenderer().RotateX(FXApp->GetRenderer().GetBasis().GetRX() + inc);
      TimePerFrame = FXApp->Draw();
      return;
    }
    if (m.m_keyCode == WXK_DOWN) {
      FXApp->GetRenderer().RotateX(FXApp->GetRenderer().GetBasis().GetRX() - inc);
      TimePerFrame = FXApp->Draw();
      return;
    }
    if (m.m_keyCode == WXK_LEFT) {
      FXApp->GetRenderer().RotateY(FXApp->GetRenderer().GetBasis().GetRY() - inc);
      TimePerFrame = FXApp->Draw();
      return;
    }
    if (m.m_keyCode == WXK_RIGHT) {
      FXApp->GetRenderer().RotateY(FXApp->GetRenderer().GetBasis().GetRY() + inc);
      TimePerFrame = FXApp->Draw();
      return;
    }
    if (m.m_keyCode == WXK_END) {
      if (FXApp->GetRenderer().GetZoom() + zoom_inc < 100) {
        FXApp->GetRenderer().SetZoom(FXApp->GetRenderer().GetZoom() + zoom_inc);
        TimePerFrame = FXApp->Draw();
        return;
      }
    }
    if (m.m_keyCode == WXK_HOME) {
      double z = FXApp->GetRenderer().GetZoom() - zoom_inc;
      if (z <= 0) z = 0.001;
      FXApp->GetRenderer().SetZoom(z);
      TimePerFrame = FXApp->Draw();
      return;
    }
  }
  // Ctrl + Up, Down - browse solutions
  if ((Fl & sssCtrl) != 0) {
    if (m.m_keyCode == WXK_UP && ((FMode&mSolve) == mSolve)) {
      ChangeSolution(CurrentSolution - 1);
      return;
    }
    if (m.m_keyCode == WXK_DOWN && ((FMode&mSolve) == mSolve)) {
      ChangeSolution(CurrentSolution + 1);
      return;
    }
  }
  if ((Fl&sssCtrl) && m.GetKeyCode() == 'c' - 'a' + 1) {  // Ctrl+C
    if (_ProcessManager->GetRedirected() != 0) {
      if (_ProcessManager->GetRedirected()->Terminate()) {
        TBasicApp::NewLogEntry(logInfo) <<
          "Process has been successfully terminated...";
      }
      else {
        TBasicApp::NewLogEntry(logInfo) <<
          "Could not terminate the process...";
      }
      TimePerFrame = FXApp->Draw();
    }
    return;
  }
  if (m.GetKeyCode() == WXK_RETURN) {
    if (FMode & mSolve) {
      FMode ^= mSolve;
      TBasicApp::NewLogEntry(logInfo) << "Model is set to current solution";
    }
  }
  if (m.GetKeyCode() == WXK_ESCAPE) {  // escape
    if (Modes->GetCurrent() != 0) {
      if (Modes->GetCurrent()->OnKey(m.GetKeyCode(), Fl))
        return;
      else
        processMacro("mode off");
    }
    processMacro("sel -u");
    TimePerFrame = FXApp->Draw();
  }
  if (m.GetKeyCode() == WXK_TAB) {  // tab
    ProcessTab();
    return;
  }

  if (!CmdLineVisible) {
    if (FGlConsole->ProcessKey(m.GetKeyCode(), Fl)) {
      PreviewHelp(FGlConsole->GetCommand());
      TimePerFrame = FXApp->Draw();
      return;
    }
  }
  else {
    if (FCmdLine->ProcessKey(m)) {
      PreviewHelp(FCmdLine->GetCommand());
      TimePerFrame = FXApp->Draw();
      return;
    }
    else {
      if (!FGlConsole->ProcessKey(m.GetKeyCode(), Fl)) {
        m.Skip();
      }
      else {
        TimePerFrame = FXApp->Draw();
      }
    }
  }

  if (_ProcessManager->GetRedirected() != 0) {
    FHelpWindow->SetVisible(false);
    FGlConsole->ShowBuffer(true);
    TimePerFrame = FXApp->Draw();
    return;
  }

  if (m.GetKeyCode() == WXK_RETURN) {
    TimePerFrame = FXApp->Draw();
  }
  else {
    if (!CmdLineVisible) {
      TimePerFrame = FXApp->Draw();
    }
  }
  if (FindFocus() != FCmdLine) {
    m.Skip();
  }
}
//..............................................................................
void TMainForm::OnKeyUp(wxKeyEvent& m)  {
  m.Skip();
}
//..............................................................................
void TMainForm::OnKeyDown(wxKeyEvent& m) {
  if (m.m_keyCode == WXK_RAW_CONTROL ||
    m.m_keyCode == WXK_MENU ||
    m.m_keyCode == WXK_SHIFT ||
    m.m_keyCode == WXK_ALT)
  {
    m.Skip();
    return;
  }
  m.Skip(false);
  wxWindow* wxw = FindFocus();
  if (wxw != FGlCanvas && wxw != FCmdLine &&
    (wxw->GetParent() == HtmlManager.main || wxw == HtmlManager.main))
  {
    bool process = false;
    const std::type_info &ti = typeid(*wxw);
    if (ti == typeid(TComboBox) ||
      ti == typeid(TChoice) ||
      ti == typeid(TTreeView) ||
      ti == typeid(TTextEdit) ||
      ti == typeid(TSpinCtrl) ||
      ti == typeid(TDateCtrl))
    {}
    else {
      if (CmdLineVisible) {
        wxw = FCmdLine;
        FCmdLine->SetFocus();
      }
      else {
        wxw = FGlCanvas;
        FGlCanvas->SetFocus();
      }
      if (m.GetKeyCode() >= 'A' && m.GetKeyCode() <= 'Z') {
        if (m.GetModifiers() == wxMOD_NONE || m.GetModifiers() == wxMOD_SHIFT) {
          if (m.GetModifiers() == wxMOD_NONE) {
            m.m_keyCode += 'a' - 'A';
          }
          OnChar(m);
          return;
        }
      }
      // else: look through shortcuts etc
    }
  }
  if (wxw != this && wxw != FGlCanvas && wxw != FCmdLine) {
    m.Skip();
    return;
  }
  static bool special_drawing_set = false;
  if (m.m_controlDown && m.m_keyCode == WXK_SPACE) {
    special_drawing_set = true;
    TGXApp::AtomIterator ai = FXApp->GetAtoms();
    while (ai.HasNext()) {
      ai.Next().SetSpecialDrawing(true);
    }
    FXApp->Draw();
  }
  else if (special_drawing_set) {
    special_drawing_set = false;
    TGXApp::AtomIterator ai = FXApp->GetAtoms();
    while (ai.HasNext()) {
      ai.Next().SetSpecialDrawing(false);
    }
    FXApp->Draw();
  }

  short Fl = 0;
  if (CmdLineVisible && wxw == FGlCanvas) {
    if (m.m_keyCode != WXK_BACK && m.m_keyCode != WXK_DELETE) {
      FCmdLine->EmulateKeyPress(m);
      return;
    }
  }

  if (m.GetModifiers() & wxMOD_ALT) {
    Fl |= sssAlt;
  }
  if (m.GetModifiers() & wxMOD_RAW_CONTROL) {
    Fl |= sssCtrl;
  }
  if (m.GetModifiers() & wxMOD_SHIFT) {
    Fl |= sssShift;
  }
  // process built-ins first
  if (m.GetModifiers() == wxMOD_CMD) {
    // paste, Cmd+V, Ctrl+V
    if (m.GetKeyCode() == 'V') {
      // avoid duplication
      olxstr content;
      if (wxTheClipboard->Open()) {
        if (wxTheClipboard->IsSupported(wxDF_UNICODETEXT)) {
          wxTextDataObject data;
          wxTheClipboard->GetData(data);
          content = data.GetText();
        }
        wxTheClipboard->Close();
      }
      olxstr cmdl;
      if (CmdLineVisible) {
        cmdl = FCmdLine->GetCommand();
      }
      else {
        cmdl = FGlConsole->GetCommand();
      }
      if (!ImportFrag(content)) {
        olxstr trimmed_content = content;
        trimmed_content.Trim(' ').Trim('\n').Trim('\r');
        size_t ip;
        if (CmdLineVisible) {
          ip = FCmdLine->GetInsertionPoint() -
            FCmdLine->GetPromptStr().Length();
        }
        else {
          ip = FGlConsole->GetCmdInsertPosition();
        }
        if (ip >= cmdl.Length()) {
          cmdl << content;
        }
        else {
          cmdl.Insert(content, ip);
        }
        if (CmdLineVisible) {
          FCmdLine->SetCommand(cmdl);
        }
        else {
          FGlConsole->SetCommand(cmdl);
        }
      }
      TimePerFrame = FXApp->Draw();
      return;
    }
    //undo, Cmd+Z, Ctrl+Z
    else if (m.GetKeyCode() == 'Z') {
      processMacro("undo");
      TimePerFrame = FXApp->Draw();
      return;
    }
    else if (m.GetKeyCode() == WXK_INSERT) {
      FXApp->CopySelection();
      return;
    }
  }
  else if (m.GetModifiers() == wxMOD_SHIFT) {
    if (m.GetKeyCode() == WXK_INSERT) {
      FXApp->PasteSelection();
      return;
    }
  }
  if (!AccShortcuts.ValueExists(Fl << 16 | m.m_keyCode)) {
    m.Skip();
    return;
  }
  if (CmdLineVisible) {
    if (FCmdLine->WillProcessKey(m)) {
      m.Skip();
      return;
    }
  }
  else if (FGlConsole->WillProcessKey(m.GetKeyCode(), Fl)) {
    m.Skip();
    return;
  }
  olxstr Cmd = AccShortcuts.GetValue(Fl << 16 | m.m_keyCode);
  if (!Cmd.IsEmpty()) {
    processMacro(Cmd, __OlxSrcInfo);
    TimePerFrame = FXApp->Draw();
    return;
  }
  m.Skip();
}
//..............................................................................
void TMainForm::OnNavigation(wxNavigationKeyEvent& event) {
  wxWindow *wxw = wxWindow::FindFocus();
  if (wxw == this || wxw == FGlCanvas || wxw == FCmdLine) {
    ProcessTab();
    event.Skip(false);
  }
  else {
    event.Skip(true);
  }
}
//..............................................................................
void TMainForm::OnMove(wxMoveEvent& evt) {
  if (Destroying || FXApp == 0 || FGlConsole == 0 || FInfoBox == 0 ||
    !StartupInitialised)
  {
    return;
  }
  wxPoint p = FGlCanvas->GetScreenPosition();
  FXApp->GetRenderer().SetAbsoluteTop(p.y);
}
//..............................................................................
void TMainForm::OnSize(wxSizeEvent& event) {
  wxFrame::OnSize(event);
  if (SkipSizing || Destroying) {
    return;
  }
  if (FXApp == 0 || FGlConsole == 0 || FInfoBox == 0 || !StartupInitialised) {
    return;
  }
  OnResize();
}
//..............................................................................
void TMainForm::OnResize() {
  if (!wxIsMainThread()) {
    return;
  }
  int w = 0, h = 0, l = 0;
  int dheight = InfoWindowVisible ? FInfoBox->GetHeight() : 1;
  GetClientSize(&w, &h);
  if (FHtmlMinimized) {
    if (FHtmlOnLeft) {
      HtmlManager.main->SetSize(0, 0, 10, h);
      l = 10;
      w = w - l;
    }
    else {
      HtmlManager.main->SetSize(w - 10, 0, 10, h);
      w = w - 10;
    }
  }
  else {
    HtmlManager.main->Freeze();
    if (FHtmlOnLeft) {
      const int cw = (FHtmlWidthFixed ? (int)FHtmlPanelWidth
        : (int)(w * FHtmlPanelWidth));
      HtmlManager.main->SetClientSize(cw, -1);
      HtmlManager.main->SetSize(-1, h);
      HtmlManager.main->Move(0, 0);
      l = HtmlManager.main->GetSize().GetWidth();
      w -= l;
    }
    else {
      const int cw = (FHtmlWidthFixed ? (int)FHtmlPanelWidth
        : (int)(w * FHtmlPanelWidth));
      HtmlManager.main->SetClientSize(cw, -1);
      HtmlManager.main->SetSize(-1, h);
      HtmlManager.main->Move((int)(w - HtmlManager.main->GetSize().GetWidth()), 0);
      w -= HtmlManager.main->GetSize().GetWidth();
    }
    HtmlManager.main->Refresh();
    HtmlManager.main->Update();
    HtmlManager.main->Thaw();
  }
  if (CmdLineVisible) {
    FCmdLine->WI.SetWidth(w);
    FCmdLine->WI.SetLeft(l);
    FCmdLine->WI.SetTop(h - FCmdLine->WI.GetHeight());
  }
  if (w <= 0) {
    w = 5;
  }
  if (h <= 0) {
    h = 5;
  }
  FGlCanvas->SetSize(l, 0, w, h - (CmdLineVisible ? FCmdLine->WI.GetHeight() : 0));
  FGlCanvas->GetClientSize(&w, &h);
  FXApp->GetRenderer().Resize(0, 0, w, h, 1);
  FGlConsole->Resize(0, dheight, w, h - dheight);
  if (FInfoBox->IsCreated()) {
    FInfoBox->SetTop(1);
    FInfoBox->SetWidth(w);
    FInfoBox->SetLeft(0);
  }
}
//..............................................................................
olxstr TMainForm::ExpandCommand(const olxstr &Cmd, bool inc_files) {
  if (FHelpWindow->IsVisible()) {
    FHelpWindow->Clear();
  }
  if (Cmd.IsEmpty()) {
    return Cmd;
  }
  olxstr FullCmd(Cmd.ToLowerCase());
  TStrList all_cmds;
  if (inc_files) {
    TStrList names;
    olxstr path = TEFile::ExpandRelativePath(Cmd, TEFile::CurrentDir());
    if (!path.IsEmpty() && TEFile::IsAbsolutePath(path)) {
      size_t lsi = path.LastIndexOf(TEFile::GetPathDelimeter());
      if (lsi != InvalidIndex) {
        olxstr dir_name = path.SubStringTo(lsi + 1);
        if (TEFile::Exists(dir_name)) {
          TEFile::ListDir(dir_name, names, olxstr(path.SubStringFrom(lsi + 1)) <<
            '*', sefReadWrite);
          for (size_t i = 0; i < names.Count(); i++) {
            all_cmds.Add(dir_name + names[i]);
          }
        }
      }
    }
    else {
      TEFile::ListCurrentDir(all_cmds, olxstr(Cmd) << '*', sefReadWrite);
    }
  }
  TBasicLibraryPList libs;
  GetLibrary().FindSimilarLibraries(Cmd, libs);
  TBasicFunctionPList bins;  // builins
  GetLibrary().FindSimilarMacros(Cmd, bins);
  GetLibrary().FindSimilarFunctions(Cmd, bins);
  for (size_t i = 0; i < bins.Count(); i++) {
    all_cmds.Add(bins[i]->GetQualifiedName());
  }
  for (size_t i = 0; i < libs.Count(); i++) {
    all_cmds.Add(libs[i]->GetQualifiedName());
  }
  if (all_cmds.Count() > 1) {
    if (FHelpWindow->IsVisible())  // console buffer is hidden then...
      FHelpWindow->Clear();
    olxstr cmn_str = all_cmds[0].ToLowerCase();
    olxstr line(all_cmds[0], 80);
    for (size_t i = 1; i < all_cmds.Count(); i++) {
      cmn_str = all_cmds[i].ToLowerCase().CommonString(cmn_str);
      if (line.Length() + all_cmds[i].Length() > 79) {  // expects no names longer that 79!
        if (FHelpWindow->IsVisible()) {
          FHelpWindow->PostText(line);
        }
        else {
          FXApp->NewLogEntry() << line;
        }
        line.SetLength(0);
      }
      else {
        line << ' ' << all_cmds[i];
      }
    }
    FullCmd = cmn_str;
    if (!line.IsEmpty()) {
      if (FHelpWindow->IsVisible()) {
        FHelpWindow->PostText(line);
      }
      else {
        FXApp->NewLogEntry() << line;
      }
    }
    FHelpWindow->Fit();
  }
  else if (all_cmds.Count() == 1) {
    return all_cmds[0];
  }
  return FullCmd;
}
//..............................................................................
void TMainForm::PostCmdHelp(const olxstr &Cmd, bool Full)  {
  ABasicFunction *MF = FXApp->GetLibrary().FindMacro(Cmd);
  bool printed = false;
  if (MF != 0) {
    FGlConsole->PrintText(olxstr("Built in macro ") << MF->GetName());
    FGlConsole->PrintText(olxstr(" Signature: ") << MF->GetSignature());
    FGlConsole->PrintText((olxstr(" Description: ") <<
      MF->GetDescription()).Replace('\t', "  "));
    if (!MF->GetOptions().IsEmpty()) {
      FGlConsole->PrintText(" Options: ");
      for (size_t i=0; i < MF->GetOptions().Count(); i++) {
        FGlConsole->PrintText(olxstr("   ") << MF->GetOptions().GetKey(i) << " - "
          << MF->GetOptions().GetValue(i));
      }
    }
    printed = true;
  }
  MF = FXApp->GetLibrary().FindFunction(Cmd);
  if (MF != 0) {
    FGlConsole->PrintText(olxstr("Built in function ") << MF->GetName());
    FGlConsole->PrintText(olxstr(" Signature: ") << MF->GetSignature());
    FGlConsole->PrintText((olxstr(" Description: ") <<
      MF->GetDescription()).Replace('\t', "  "));
    printed = true;
  }
  if (printed) {
    TBasicApp::NewLogEntry() << NewLineSequence();
  }
}
//..............................................................................
void TMainForm::SaveSettings(const olxstr& FN) {
  TDataFile DF;
  TDataItem* I = &DF.Root().AddItem("Folders");
  I->AddField("Styles",
    olxstr().quote('"') << TEFile::CreateRelativePath(StylesDir));
  I->AddField("Scenes",
    olxstr().quote('"') << TEFile::CreateRelativePath(ScenesDir));
  I->AddField("Current",
    olxstr().quote('"') << TEFile::CreateRelativePath(XLibMacros::CurrentDir()));

  I = &DF.Root().AddItem("HTML");
  I->AddField("Minimized", FHtmlMinimized);
  I->AddField("OnLeft", FHtmlOnLeft);
  if (!FHtmlWidthFixed)
    I->AddField("Width", olxstr(FHtmlPanelWidth) << '%');
  else
    I->AddField("Width", FHtmlPanelWidth);
  I->AddField("Tooltips", HtmlManager.main->GetShowTooltips());
  I->AddField("Borders", HtmlManager.main->GetBorders());
  {
    olxstr normal, fixed;
    HtmlManager.main->GetFonts(normal, fixed);
    I->AddField("NormalFont", normal);
    I->AddField("FixedFont", fixed);
  }
  if (!IsIconized()) {  // otherwise left and top are -32000 causing all sort of problems...
    I = &DF.Root().AddItem("Window");
    if (IsMaximized()) {
      I->AddField("Maximized", true);
    }
    int w_w = 0, w_h = 0;
    GetSize(&w_w, &w_h);
    I->AddField("Width", w_w);

    I->AddField("Height", w_h);
    GetPosition(&w_w, &w_h);
    I->AddField("X", w_w);
    I->AddField("Y", w_h);
  }

  I = &DF.Root().AddItem("Windows");
  I->AddField("Help", HelpWindowVisible);
  I->AddField("Info", InfoWindowVisible);
  I->AddField("CmdLine", CmdLineVisible);

  I = &DF.Root().AddItem("Defaults");
  I->AddField("Style", TEFile::CreateRelativePath(DefStyle));
  I->AddField("Scene", TEFile::CreateRelativePath(DefSceneP));

  I->AddField("BgColor", FBgColor.ToString());
  I->AddField("WhiteOn",
    (FXApp->GetRenderer().LightModel.GetClearColor().GetRGB() == 0xffffffff));
  I->AddField("Gradient", FXApp->GetRenderer().Background()->IsVisible());
  I->AddField("GradientPicture", TEFile::CreateRelativePath(GradientPicture));
  I->AddField("language", FXApp->Dictionary.GetCurrentLanguage());
  I->AddField("ExtraZoom", FXApp->GetExtraZoom());
  I->AddField("GlTooltip", _UseGlTooltip);
  I->AddField("ThreadCount", FXApp->GetMaxThreadCount());

  I = &DF.Root().AddItem("Recent_files");
  for (size_t i = 0; i < olx_min(FRecentFilesToShow, FRecentFiles.Count()); i++) {
    olxstr x = TEFile::CreateRelativePath(FRecentFiles[i]);
    if (x.Length() > FRecentFiles[i].Length()) {
      x = FRecentFiles[i];
    }
    I->AddField(olxstr("file") << i, olxstr().quote('"') << x);
  }

  I = &DF.Root().AddItem("Stored_params");
  for (size_t i = 0; i < StoredParams.Count(); i++) {
    TDataItem& it = I->AddItem(StoredParams.GetKey(i));
    it.AddField("value", olxstr().quote('"') << StoredParams.GetValue(i));
  }
  FXApp->GetRenderer().GetScene().ToDataItem(
    DF.Root().AddItem("Scene"));
  try {
    FXApp->GetRenderer().GetStyles().ToDataItem(DF.Root().AddItem("Styles"));
  }
  catch (const TExceptionBase& e) {
    TBasicApp::NewLogEntry(logExceptionTrace) << e;
    FXApp->GetRenderer().GetStyles().Clear();
  }
  DF.SaveToXLFile(FN + ".tmp");
  TEFile::Rename(FN + ".tmp", FN);
  /* check if the stereo buffers are available and if not disable - this way the
  multisampling can be enabled!
  */
  {
    GLboolean stereo_supported = GL_FALSE;
    olx_gl::get(GL_STEREO, &stereo_supported);
    if (stereo_supported == GL_FALSE) {
      try {
        olxstr str_glStereo = olx_getenv("OLEX2_GL_STEREO");
        bool stereo_enabled = FXApp->GetOptions().FindValue("gl_stereo",
          str_glStereo.IsEmpty() ? TrueString() : str_glStereo).ToBool();
        if (stereo_enabled) {
          this->UpdateUserOptions("gl_stereo", FalseString());
        }
      }
      catch (const TExceptionBase& e) {
        TBasicApp::NewLogEntry(logExceptionTrace) << e;
      }
    }
  }
}
//..............................................................................
void TMainForm::LoadSettings(const olxstr& FN) {
  if (!TEFile::Exists(FN)) {
    return;
  }
  // compatibility check...
#ifdef __WIN32__
  {
    if (!TEFile::OSPath(FN).StartsFrom(TBasicApp::GetBaseDir())) {
      TEFile f(FN, "rb");
      olxcstr l = f.ReadLine(), ds("\\\\");
      f.Close();
      if (l.Contains('\\') && !l.Contains(ds)) { // no escapes, needs converting
        TCStrList t = TEFile::ReadCLines(FN);
        for (size_t i = 0; i < t.Count(); i++)
          t[i].Replace('\\', ds);
        try {
          TEFile::WriteLines(FN, t);
        }
        catch (const TExceptionBase& e) {
          TBasicApp::NewLogEntry(logException) << e;
        }
      }
    }
  }
#endif
  TDataFile DF;
  TStrList Log;
  olxstr Tmp;
  DF.LoadFromXLFile(FN, &Log);

  TDataItem* I = DF.Root().FindItem("Folders");
  if (I == 0) {
    return;
  }
  StylesDir = TEFile::ExpandRelativePath(
    exparse::parser_util::unquote(I->FindField("Styles")));
  processFunction(StylesDir);
  ScenesDir = TEFile::ExpandRelativePath(
    exparse::parser_util::unquote(I->FindField("Scenes")));
  processFunction(ScenesDir);
  XLibMacros::CurrentDir() = TEFile::ExpandRelativePath(
    exparse::parser_util::unquote(I->FindField("Current")));
  if (!TEFile::IsAbsolutePath(XLibMacros::CurrentDir()))
    processFunction(XLibMacros::CurrentDir());

  I = DF.Root().FindItem("HTML");
  if (I != 0) {
    Tmp = I->FindField("Minimized");
    FHtmlMinimized = Tmp.IsEmpty() ? false : Tmp.ToBool();
    Tmp = I->FindField("OnLeft");
    FHtmlOnLeft = Tmp.IsEmpty() ? true : Tmp.ToBool();

    Tmp = I->FindField("Width");
    if (!Tmp.IsEmpty()) {
      FHtmlWidthFixed = !Tmp.EndsWith('%');
      FHtmlPanelWidth = ((!FHtmlWidthFixed) ? Tmp.SubStringTo(Tmp.Length() - 1).ToDouble()
        : Tmp.ToDouble());
      if (!FHtmlWidthFixed && FHtmlPanelWidth >= 0.5) {
        FHtmlPanelWidth = 0.25;
      }
    }
    else {
      FHtmlPanelWidth = 0.25;
    }

    Tmp = I->FindField("Tooltips", EmptyString());
    if (!Tmp.IsEmpty())
      HtmlManager.main->SetShowTooltips(Tmp.ToBool());

    Tmp = I->FindField("Borders");
    if (!Tmp.IsEmpty() && Tmp.IsNumber())
      HtmlManager.main->SetBorders(Tmp.ToInt());

    olxstr nf(I->FindField("NormalFont", EmptyString()));
    olxstr ff(I->FindField("FixedFont", EmptyString()));
    HtmlManager.main->SetFonts(nf, ff);
  }

  SkipSizing = true;
  I = DF.Root().FindItem("Window");
  if (I != 0) {
    if (I->FindField("Maximized", FalseString()).ToBool()) {
      int l = I->FindField("X", "0").ToInt(),
        t = I->FindField("Y", "0").ToInt();
      Move(l, t);
      Maximize();
    }
    else {
      int w = I->FindField("Width", "100").ToInt(),
        h = I->FindField("Height", "100").ToInt(),
        l = I->FindField("X", "0").ToInt(),
        t = I->FindField("Y", "0").ToInt();
      SetSize(l, t, w, h);
    }
  }
  else {
    Maximize();
  }
  SkipSizing = false;

  I = DF.Root().FindItem("Windows");
  if (I != 0) {
    HelpWindowVisible = I->FindField("Help", TrueString()).ToBool();
    InfoWindowVisible = I->FindField("Info", TrueString()).ToBool();
    CmdLineVisible = I->FindField("CmdLine", FalseString()).ToBool();
  }
  TEFile::ChangeDir(XLibMacros::CurrentDir());

  I = DF.Root().FindItem("Recent_files");
  if (I != 0) {
    MenuFile->AppendSeparator();
    int i = 0;
    TStrList uniqNames;
    olxstr T = TEFile::ExpandRelativePath(I->FindField(olxstr("file") << i));
    while (!T.IsEmpty()) {
      if (T.EndsWithi(".ins") || T.EndsWithi(".res")) {
        T = TEFile::ChangeFileExt(T, EmptyString());
      }
      TEFile::OSPathI(T);
      if (uniqNames.IndexOf(T) == InvalidIndex)
        uniqNames.Add(T);
      i++;
      T = I->FindField(olxstr("file") << i);
    }
    for (size_t j = 0; j < olx_min(uniqNames.Count(), FRecentFilesToShow); j++) {
      processFunction(uniqNames[j]);
      MenuFile->AppendCheckItem((int)(ID_FILE0 + j), uniqNames[j].u_str());
      FRecentFiles.Add(uniqNames[j],
        MenuFile->FindItemByPosition(MenuFile->GetMenuItemCount() - 1));
    }
  }
  try {
    I = &DF.Root().GetItemByName("Defaults");
  }
  catch (const TExceptionBase& e) {
    FXApp->CreateObjects(false);
    ShowAlert(e, "Failed to load settings, reseting to the defaults...");
    processMacro("default");
    return;
  }
  DefStyle = TEFile::ExpandRelativePath(I->FindField("Style"));
  processFunction(DefStyle);
  DefSceneP = TEFile::ExpandRelativePath(I->FindField("Scene"));
  processFunction(DefSceneP);
  // loading default style if provided ?
  if (TEFile::Exists(DefStyle)) {
    TDataFile SDF;
    SDF.LoadFromXLFile(DefStyle, &Log);
    FXApp->GetRenderer().GetStyles().FromDataItem(
      *SDF.Root().FindItem("style"), false);
  }
  else {
    TDataItem& last_saved_style = DF.Root().GetItemByName("Styles");
    int l_version = TGraphicsStyles::ReadStyleVersion(last_saved_style);
    // old style override, let's hope it is newer!
    if (l_version < TGraphicsStyles::CurrentVersion()) {
      olxstr new_set = FXApp->GetBaseDir() + "last.osp";
      if (TEFile::Exists(new_set)) {
        TDataFile LF;
        try {
          LF.LoadFromXLFile(new_set);
          TDataItem& distributed_style = LF.Root().GetItemByName("Styles");
          int d_version = TGraphicsStyles::ReadStyleVersion(distributed_style);
          // it would be weird if distributed version is not current... but might happen
          FXApp->GetRenderer().GetStyles().FromDataItem(
            (d_version <= l_version) ? last_saved_style : distributed_style,
            false);
        }
        catch (...) {  // recover...
          FXApp->GetRenderer().GetStyles().FromDataItem(last_saved_style,
            false);
        }
      }
    }
    else  // up-to-date then...
      FXApp->GetRenderer().GetStyles().FromDataItem(last_saved_style, false);
  }
  // default scene properties provided?
  if (TEFile::Exists(DefSceneP)) {
    TDataFile SDF;
    SDF.LoadFromXLFile(DefSceneP, &Log);
    FXApp->GetRenderer().GetScene().FromDataItem(SDF.Root());
  }
  else {
    FXApp->GetRenderer().GetScene().FromDataItem(
      DF.Root().GetItemByName("Scene"));
  }
  FBgColor = FXApp->GetRenderer().LightModel.GetClearColor();
  // restroring language or setting default
  try {
    FXApp->SetCurrentLanguage(
      I->FindField("language", EmptyString()));
  }
  catch (const TExceptionBase& e) {
    ShowAlert(e, "Failed loading/processing dictionary file");
  }
  FXApp->SetExtraZoom(I->FindField("ExtraZoom", "1.25").ToDouble());
#ifdef __WIN32__
  const olxstr& defGlTVal = FalseString();
#else
  const olxstr& defGlTVal = TrueString();
#endif
  UseGlTooltip(I->FindField("GlTooltip", defGlTVal).ToBool());
  if (I->FieldExists("ThreadCount")) {
    FXApp->SetMaxThreadCount(I->FindField("ThreadCount", "1").ToInt());
  }
  else {
    int cpu_cnt = wxThread::GetCPUCount();
    if (cpu_cnt > 0) {
      FXApp->SetMaxThreadCount(cpu_cnt);
    }
  }
  if (FBgColor.GetRGB() == 0xffffffff) {  // only if the information got lost
    olxstr T(I->FindField("BgColor"));
    if (!T.IsEmpty()) {
      FBgColor.FromString(T);
    }
  }
  bool whiteOn = I->FindField("WhiteOn", FalseString()).ToBool();
  FXApp->GetRenderer().LightModel.SetClearColor(
    whiteOn ? 0xffffffff : FBgColor.GetRGB());

  GradientPicture = TEFile::ExpandRelativePath(
    I->FindField("GradientPicture", EmptyString()));
  if (!TEFile::Exists(GradientPicture)) {
    GradientPicture.SetLength(0);
  }
  olxstr T = I->FindField("Gradient", EmptyString());
  if (!T.IsEmpty()) {
    processMacro(olxstr("grad ") << T);
  }

  I = DF.Root().FindItem("Stored_params");
  if (I != 0) {
    for (size_t i = 0; i < I->ItemCount(); i++) {
      TDataItem& pd = I->GetItemByIndex(i);
      olxstr v = pd.FindField("value");
      processFunction(v, EmptyString(), true);
      StoredParams.Add(pd.GetName(), v);
    }
  }
}
//..............................................................................
void TMainForm::UpdateRecentFile(const olxstr& fn)  {
  if (fn.IsEmpty()) {
    for (size_t i = 0; i < FRecentFiles.Count(); i++) { // change item captions
      FRecentFiles.GetObject(i)->Check(false);
    }
    return;
  }
  TPtrList<wxMenuItem> Items;
  olxstr FN = (fn.EndsWithi(".ins") || fn.EndsWithi(".res")) ?
    TEFile::ChangeFileExt(fn, EmptyString()) : fn;
  TEFile::OSPathI(FN);
  olxstr x = TEFile::CreateRelativePath(FN);
  if (x.Length() < FN.Length()) {
    FN = x;
  }
  size_t index = FRecentFiles.IndexOf(FN);
  wxMenuItem* mi=NULL;
  if( index == InvalidIndex )  {
    if( (FRecentFiles.Count()+1) < FRecentFilesToShow )  {
      for( size_t i=0; i < MenuFile->GetMenuItemCount(); i++ )  {
        wxMenuItem* item = MenuFile->FindItemByPosition(i);
          if( item->GetId() >= ID_FILE0 && item->GetId() <= (ID_FILE0+FRecentFilesToShow))
            index = i;
      }
      if( index != InvalidIndex ) {
        mi = MenuFile->InsertCheckItem(index + 1,
          (int)(ID_FILE0+FRecentFiles.Count()), wxT("tmp"));
      }
      else {
        mi = MenuFile->AppendCheckItem(
          (int)(ID_FILE0+FRecentFiles.Count()), wxT("tmp"));
      }
      FRecentFiles.Insert(0, FN, mi);
    }
    else  {
      FRecentFiles.Insert(0, FN, FRecentFiles.GetLast().Object);
      FRecentFiles.Delete(FRecentFiles.Count()-1);
    }
  }
  else
    FRecentFiles.Move(index, 0);

  for( size_t i=0; i < FRecentFiles.Count(); i++ )
    Items.Add( FRecentFiles.GetObject(i) );
  for( size_t i=0; i < FRecentFiles.Count(); i++ )  { // put items in the right position
    FRecentFiles.GetObject(Items[i]->GetId()-ID_FILE0) = Items[i];
    Items[i]->SetItemLabel(FRecentFiles[Items[i]->GetId()-ID_FILE0].u_str());
    Items[i]->Check(false);
  }
  FRecentFiles.GetObject(0)->Check( true );
  if( FRecentFiles.Count() >= FRecentFilesToShow )
    FRecentFiles.SetCount(FRecentFilesToShow);
}
//..............................................................................
bool TMainForm::UpdateRecentFilesTable(bool TableDef)  {
  const olxstr RecentFilesFile("recent_files.htm");
  TTTable<TStrList> Table;
  TStrList Output;
  int tc=0;
  olxstr Tmp;
  if( FRecentFiles.Count()%3 )  tc++;
  Table.Resize(FRecentFiles.Count()/3+tc, 3);
  for( size_t i=0; i < FRecentFiles.Count(); i++ )  {
    Tmp = "<a href=\'reap \"";
    Tmp << TEFile::OSPath(FRecentFiles[i]) << "\"\'>";
    Tmp << TEFile::ExtractFileName(FRecentFiles[i]) << "</a>";
    Table[i/3][i%3] = Tmp;
  }
  Table.CreateHTMLList(Output, EmptyString(), false, false, false);
  olxcstr cst = TUtf8::Encode(Output.Text('\n'));
  TFileHandlerManager::AddMemoryBlock(RecentFilesFile, cst.c_str(), cst.Length(), plGlobal);
  if( TEFile::Exists(FXApp->GetInstanceDir()+RecentFilesFile) )
    TEFile::DelFile(FXApp->GetInstanceDir()+RecentFilesFile);
  //TUtf8File::WriteLines( RecentFilesFile, Output, false );
  return true;
}
//..............................................................................
void TMainForm::QPeakTable(bool TableDef, bool Create)  {
  static const olxstr QPeakTableFile("qpeaks.htm");
  if( !Create )  {
    TFileHandlerManager::AddMemoryBlock(QPeakTableFile, NULL, 0, plStructure);
    return;
  }
  TTTable<TStrList> Table;
  TPtrList<const TCAtom> atoms;
  const TAsymmUnit& au = FXApp->XFile().GetAsymmUnit();
  for( size_t i=0; i < au.AtomCount(); i++ )  {
    if( !au.GetAtom(i).IsDeleted() && au.GetAtom(i).GetType() == iQPeakZ )
      atoms.Add(au.GetAtom(i));
  }
  if( atoms.IsEmpty() )  {
    Table.Resize(1, 3);
    Table[0][0] = "N/A";
    Table[0][1] = "N/A";
    if( FXApp->CheckFileType<TIns>() )
      Table[0][2] = "No Q-Peaks";
    else
      Table[0][1] = "N/A in this file format";
  }
  else  {
    double max_peak = 0; // there are negative peaks too!
    for (size_t i=0; i < atoms.Count(); i++) {
      double pv = olx_abs(atoms[i]->GetQPeak());
      if (pv > max_peak) max_peak = pv;
    }
    QuickSorter::SortSF(atoms, &GXLibMacros::QPeakSortD);
    Table.Resize(olx_min(10, atoms.Count()), 3);
    size_t rowIndex = 0;
    for( size_t i=0; i < atoms.Count(); i++, rowIndex++ )  {
      if( i > 8 )  i = atoms.Count() -1;
      Table[rowIndex][0] = atoms[i]->GetLabel();
      Table[rowIndex][1] = olxstr::FormatFloat(3, atoms[i]->GetQPeak());
      olxstr Tmp = "<a href=\"sel -i ";
      if( i > rowIndex )
        Tmp << atoms[rowIndex]->GetLabel() << " to ";
      Tmp << atoms[i]->GetLabel();
      olxstr image_name;
      if (atoms[i]->GetQPeak() < 0)
        image_name = "purple";
      else if (atoms[i]->GetQPeak() > 3.5)
        image_name = "red";
      else if (atoms[i]->GetQPeak() > 1.5)
        image_name = "orange";
      else
        image_name = "green";
      Tmp << "\"><zimg border=\"0\" src=\"bar_" << image_name <<
        ".png\" height=\"10\" width=\"";
      Tmp << olxstr::FormatFloat(1, olx_abs(atoms[i]->GetQPeak()*100/max_peak));
      Tmp << "%\"></a>";
      Table[rowIndex][2] = Tmp;
    }
  }
  TStrList Output = Table.CreateHTMLList(EmptyString(), false, false, TableDef);
  olxcstr cst = TUtf8::Encode(Output.Text('\n'));
  TFileHandlerManager::AddMemoryBlock(QPeakTableFile, cst.c_str(),
    cst.Length(), plStructure);
}
//..............................................................................
void TMainForm::BadReflectionsTable(bool TableDef, bool Create)  {
  static const olxstr BadRefsFile("badrefs.htm");
  if (!Create ||
     (!FXApp->CheckFileType<TIns>() &&
      !(FXApp->XFile().HasLastLoader() &&
        FXApp->XFile().LastLoader()->IsNative())))
  {
    TFileHandlerManager::AddMemoryBlock(BadRefsFile, NULL, 0, plStructure);
    return;
  }

  smatd_list matrices;
  FXApp->GetSymm(matrices);
  bool sort_abs = TOlxVars::FindValue(
    "olex2.disagreeable.sort_abs", TrueString()).ToBool();
  double sort_th = TOlxVars::FindValue(
    "olex2.disagreeable.threshold", "10").ToDouble();
  TTypeList<RefinementModel::BadReflection> bad_refs =
    FXApp->XFile().GetRM().GetBadReflectionList();
  if (bad_refs.IsEmpty()) {
    return;
  }
  if (!sort_abs) {
    QuickSorter::Sort(bad_refs,
      ReverseComparator::Make(&RefinementModel::BadReflection::CompareDirect));
  }
  bool editable = (FXApp->XFile().GetRM().GetHKLF() <= 4);
  THklFile Hkl;
  if (editable) {
    Hkl.Append(FXApp->XFile().GetRM().GetReflections());
  }
  TTTable<TStrList> Table;
  Table.Resize(bad_refs.Count(), editable ? 7 : 6);
  Table.ColName(0) = "H";
  Table.ColName(1) = "K";
  Table.ColName(2) = "L";
  Table.ColName(3) = "Error/esd";
  Table.ColName(4) = "d/A";
  for (size_t i=0; i < bad_refs.Count(); i++) {
    for (int j = 0; j < 3; j++) {
      Table[i][j] = bad_refs[i].index[j];
    }
    if (olx_abs(bad_refs[i].factor) >= sort_th) {
      Table[i][3] << "<font color=\'red\'>" <<
        olxstr::FormatFloat(2, bad_refs[i].factor) << "</font>";
    }
    else {
      Table[i][3] << olxstr::FormatFloat(2, bad_refs[i].factor);
    }
    double ds = TReflection(bad_refs[i].index).ToCart(
      FXApp->XFile().GetRM().aunit.GetHklToCartesian()).Length();
    Table[i][4] << olxstr::FormatFloat(2, 1. / ds);

    if (FXApp->XFile().GetRM().GetOmits().Contains(bad_refs[i].index)) {
      Table[i][5] << "Omitted";
    }
    else {
      Table[i][5] << "<a href='omit " << bad_refs[i].index[0] <<  ' ' <<
        bad_refs[i].index[1] << ' ' << bad_refs[i].index[2] << "\'>" <<
        "omit" << "</a>";
    }
    if (editable) {
      if (Hkl.AllRefs(bad_refs[i].index, matrices).Count() > 1) {
        Table[i][6].stream(' ') << "<a href='HklEdit" << bad_refs[i].index[0]
          << bad_refs[i].index[1] << bad_refs[i].index[2] << "'>Edit...</a>";
      }
      else {
        Table[i][6] = "---";
      }
    }
  }
  TStrList Output = Table.CreateHTMLList(EmptyString(), true, false, TableDef);
  Output.Add("Error = sig(D)"
    "(wD<sup>2</sup>/&lt;wD<sup>2</sup>&gt)<sup>1/2</sup>,"
    " where D=Fc<sup>2</sup>-Fo<sup>2</sup> (Shelx)"
    " or just D (olex2.refine)");
  olxcstr cst = TUtf8::Encode(Output.Text('\n'));
  TFileHandlerManager::AddMemoryBlock(BadRefsFile, cst.c_str(), cst.Length(),
    plStructure);
  if (TEFile::Exists(BadRefsFile))
    TEFile::DelFile(BadRefsFile);
  //TUtf8File::WriteLines( BadRefsFile, Output, false );
}
//..............................................................................
void TMainForm::RefineDataTable(bool TableDef, bool Create)  {
  static const olxstr RefineDataFile("refinedata.htm");
  if( !Create || !FXApp->CheckFileType<TIns>() )  {
    TFileHandlerManager::AddMemoryBlock(RefineDataFile, NULL, 0, plStructure);
    return;
  }
  TTTable<TStrList> Table(8, 4);
  TStrList Output;

  const TLst& Lst = FXApp->XFile().GetLastLoader<TIns>().GetLst();
  olxstr m1 = "-1";
  double v[7] = {
    Lst.params.Find("R1", m1).ToDouble(),
    Lst.params.Find("R1all", m1).ToDouble(),
    Lst.params.Find("wR2", m1).ToDouble(),
    Lst.params.Find("S", m1).ToDouble(),
    Lst.params.Find("rS", m1).ToDouble(),
    Lst.params.Find("peak", m1).ToDouble(),
    Lst.params.Find("hole", m1).ToDouble(),
  },
  ev[7] = {0.1, 0.1, 0.2, -1, -1, 1.5, 1.5};
  const char* vl[7] = {
  "R1(Fo > 4sig(Fo))",
  "R1(all data)",
  "wR2",
  "GooF",
  "GooF(restr)",
  "Highest peak",
  "Deepest hole"
  };
  size_t coli=0, rowi=0;
  for (size_t i=0; i < 7; i++) {
    Table[rowi][coli] = vl[i];
    if ((ev[i] >= 0 && v[i] > ev[i]) ||
        ((ev[i] < 0 && olx_abs(v[i]+ev[i]) > 0.5)) )
    {
      Table[rowi][coli+1] << "<font color=\'red\'>" <<
        olxstr::FormatFloat(4,v[i]) << "</font>";
    }
    else
      Table[rowi][coli+1] = olxstr::FormatFloat(4, v[i]);
    if (coli == 2) {
      rowi++;
      coli = 0;
    }
    else
      coli = 2;
  }
  Table[3][2] = "Params";
    Table[3][3] = Lst.params.Find("param_n", m1);
  Table[4][0] = "Refs(total)";
    Table[4][1] = Lst.params.Find("ref_total", m1);
  Table[4][2] = "Refs(uniq)";
    Table[4][3] = Lst.params.Find("ref_unique", m1);
  Table[5][0] = "Refs(Fo > 4sig(Fo))";
    Table[5][1] = Lst.params.Find("ref_4sig", m1);
  Table[5][2] = "R(int)";
    Table[5][3] = Lst.params.Find("Rint", m1);
  Table[6][0] = "R(sigma)";
    Table[6][1] = Lst.params.Find("Rsig", m1);
  Table[6][2] = "F000";
    Table[6][3] = Lst.params.Find("F000", m1);
  Table[7][0] = "&rho;/g*cm<sup>-3</sup>";
    Table[7][1] = Lst.params.Find("Rho", m1);
  Table[7][2] = "&mu;/mm<sup>-1</sup>";
    Table[7][3] = Lst.params.Find("Mu", m1);

  Table.CreateHTMLList(Output, EmptyString(), false, false, TableDef);
  olxcstr cst = TUtf8::Encode(Output.Text('\n'));
  TFileHandlerManager::AddMemoryBlock(RefineDataFile, cst.c_str(),
    cst.Length(), plStructure);
  if( TEFile::Exists(RefineDataFile) )
    TEFile::DelFile(RefineDataFile);
//TUtf8File::WriteLines( RefineDataFile, Output, false );
}
//..............................................................................
void TMainForm::OnMouseWheel(int x, int y, double delta) {
  size_t ind = Bindings.IndexOf("wheel");
  if (ind == InvalidIndex) {
    return;
  }
  olxstr cmd = Bindings.GetValue(ind);
  ind = TOlxVars::VarIndex("core_wheel_step");
  const olxstr& step(ind == InvalidIndex ? EmptyString()
    : TOlxVars::GetVarStr(ind));
  if (step.IsNumber()) {
    delta *= step.ToDouble();
  }
  cmd << delta;
  uint8_t ll = Macros.GetLogLevel();
  Macros.SetLogLevel(0);
  processMacro(cmd);
  Macros.SetLogLevel(ll);
}
//..............................................................................
void TMainForm::OnMouseMove(int x, int y) {
  if (MousePositionX == x && MousePositionY == y)
    return;
  else {
    MouseMoveTimeElapsed = 0;
    MousePositionX = x;
    MousePositionY = y;
    if (!_UseGlTooltip) {
      if (FGlCanvas->GetToolTip() != 0 &&
        !FGlCanvas->GetToolTip()->GetTip().IsEmpty())
      {
        FGlCanvas->SetToolTip(wxT(""));
      }
    }
    else if (GlTooltip != NULL && GlTooltip->IsVisible()) {
      GlTooltip->SetVisible(false);
      TimePerFrame = FXApp->Draw();
    }
  }
}
//..............................................................................
bool TMainForm::OnMouseDown(int x, int y, short Flags, short Buttons) {
  MousePositionX = x;
  MousePositionY = y;
  MouseMoveTimeElapsed = 5000;
  return false;
}
//..............................................................................
bool TMainForm::OnMouseUp(int x, int y, short Flags, short Buttons)  {
  // HKL "grid snap on mouse release
  if( FXApp->XFile().HasLastLoader() && FXApp->IsHklVisible() && false )  {
    mat3d cellM, M;
    vec3d N(0, 0, 1), Z;
    TAsymmUnit *au = &FXApp->XFile().GetAsymmUnit();
    cellM = au->GetHklToCartesian();
    cellM *= FXApp->GetRenderer().GetBasis().GetMatrix();
    cellM.Transpose();
    // 4x4 -> 3x3 matrix
    Z = cellM[0];    M[0] = Z;
    Z = cellM[1];    M[1] = Z;
    Z = cellM[2];    M[2] = Z;
    Z = FXApp->GetRenderer().GetBasis().GetMatrix()[2];
    olxstr  Tmp="current: ";
      Tmp << Z.ToString();
      TBasicApp::NewLogEntry() << Tmp;
    Z.Null();
    Z = mat3d::GaussSolve(M, N);
    Z.Normalise();
    double H = Z[0]*Z[0];
    double K = Z[1]*Z[1];
    double L = Z[2]*Z[2];
    if( H > 0.07 )  H = 1./H;
    if( K > 0.07 )  K = 1./K;
    if( L > 0.07 )  L = 1./L;
    int iH = olx_round(H), iK = olx_round(K), iL = olx_round(L);
    double diff = sqrt(olx_abs(H + K + L - iH - iK - iL)/(H + K + L));
    if( diff < 0.25 )  {
      cellM = au->GetHklToCartesian();
      Z.Null();
      if( iH ) Z += (cellM[0]/sqrt((double)iH));
      if( iK ) Z += (cellM[1]/sqrt((double)iK));
      if( iL ) Z += (cellM[2]/sqrt((double)iL));
      Z.Normalise();

      Tmp="orienting to: ";
      Tmp << Z.ToString() << "; HKL ";
      Tmp << iH << ' ' << iK << ' ' << iL << ' ';
      TBasicApp::NewLogEntry() << Tmp;
      N = FXApp->GetRenderer().GetBasis().GetMatrix()[2];
        double ca = N.CAngle(Z);
        if( ca < -1 )  ca = -1;
        if( ca > 1 )   ca = 1;
        vec3d V = Z.XProdVec(N);
        FXApp->GetRenderer().GetBasis().Rotate(V, acos(ca));
      N = FXApp->GetRenderer().GetBasis().GetMatrix()[2];
      Tmp="got: ";
      Tmp << N.ToString();
      TBasicApp::NewLogEntry() << Tmp;

      FXApp->Draw();
    }
  }
  MousePositionX = x;
  MousePositionY = y;
  MouseMoveTimeElapsed = 5000;
  return false;
}
//..............................................................................
bool TMainForm::CheckMode(size_t mode, const olxstr& modeData) {
  if (Modes->GetCurrent() == 0) {
    return false;
  }
  return mode == Modes->GetCurrent()->GetId();
}
//..............................................................................
bool TMainForm::CheckState(size_t state, const olxstr& stateData) const {
  if (state == stateHtmlVisible) {
    if (stateData.IsEmpty()) {
      return FHtmlMinimized;
    }
    THtmlManager::TPopupData* pp = HtmlManager.Popups.Find(stateData, NULL);
    return (pp != NULL) ? pp->Dialog->IsShown() : false;
  }
  if (state == stateCmdLineVisible) {
    return CmdLineVisible;
  }
  if (state == stateGlTooltips) {
    return _UseGlTooltip;
  }
  return false;
}
//..............................................................................
void TMainForm::OnIdle() {
  if (Destroying) {
    return;
  }
#if !defined(__WIN32__)
  if (!StartupInitialised && IsShownOnScreen() && FGlCanvas->IsShownOnScreen()) {
    StartupInit();
    if (Destroying) {
      Close(true);
      return;
    }
  }
#endif
    TBasicApp::GetInstance().OnIdle.Execute((AEventsDispatcher*)this, NULL);
  // runonce business...
  if (!RunOnceProcessed && TBasicApp::IsBaseDirWriteable()) {
    RunOnceProcessed = true;
    TStrList rof;
    TEFile::ListDir(FXApp->GetBaseDir(), rof, "runonce*.*", sefFile);
    TStrList macros;
    for (size_t i=0; i < rof.Count(); i++) {
      rof[i] = FXApp->GetBaseDir()+rof[i];
      try {
        TEFile::ReadLines(rof[i], macros);
        macros.CombineLines('\\');
        for (size_t j=0; j < macros.Count(); j++) {
          processMacro(macros[j]);
#ifdef _DEBUG
          FXApp->NewLogEntry() << TEFile::ExtractFileName(rof[i]) << ": " <<
            macros[j];
#endif
        }
      }
      catch (const TExceptionBase& e) {
        ShowAlert(e);
      }
      time_t fa = TEFile::FileAge(rof[i]);
      // Null the file
      try  {  TEFile ef(rof[i], "wb+");  }
      catch(TIOException&)  {}
      TEFile::SetFileTimes(rof[i], fa, fa);
    }
  }
}
//..............................................................................
void TMainForm::SetUserCursor(const olxstr& param, const olxstr& mode)  {
  wxBitmap bmp(32, 32);
  wxMemoryDC memDC(bmp);
  wxBrush Brush = memDC.GetBrush();
  Brush.SetColour(*wxWHITE);
  memDC.SetBrush(Brush);
  wxPen Pen = memDC.GetPen();
  Pen.SetColour(*wxRED);
  memDC.SetPen(Pen);
  wxFont Font = memDC.GetFont();
  Font.SetFamily(wxSWISS);
#if defined(__WIN32__)
  Font.SetPointSize(10);
#else
  Font.SetPointSize(10);
#endif

  memDC.SetFont(Font);
  memDC.Clear();
  Brush.SetColour(*wxGREEN);
  memDC.SetBrush(Brush);
  memDC.DrawCircle(2, 2, 2);
  memDC.SetTextForeground(*wxRED);
  memDC.DrawText(param.u_str(), 0, 4);
  memDC.DrawLine(0, 18, 32, 18);
  memDC.SetTextForeground(*wxGREEN);
  memDC.SetPen(Pen);
  memDC.DrawText(mode.u_str(), 0, 18);
  wxImage img(bmp.ConvertToImage());
  img.SetMaskColour(255, 255, 255);
  img.SetMask(true);
  wxCursor cr(img);
  SetCursor(cr);
  FGlCanvas->SetCursor(cr);
}
//..............................................................................
bool TMainForm::ProcessEvent(wxEvent& evt) {
  if (evt.GetEventType() == wxEVT_COMMAND_MENU_SELECTED &&
    AccMenus.ValueExists(evt.GetId()))
  {
    olxstr macro(AccMenus.GetValue(evt.GetId())->GetCommand());
    if (!macro.IsEmpty()) {
      TStrList sl = TParamList::StrtokLines(macro, ">>");
      for (size_t i = 0; i < sl.Count(); i++) {
        if (!processMacro(sl[i])) {
          break;
        }
      }
      // restore state if failed
      if (AccMenus.GetValue(evt.GetId())->IsCheckable()) {
        AccMenus.GetValue(evt.GetId())->ValidateState();
      }

      FXApp->Draw();
      return true;
    }
  }
  evt.Skip();
  return wxFrame::ProcessEvent(evt);
}
//..............................................................................
int TMainForm::TranslateShortcut(const olxstr& sk) {
  if (sk.IsEmpty()) {
    return -1;
  }
  TStrList toks(sk, '+');
  if (!toks.Count()) {
    return -1;
  }
  short Shift = 0, Char = 0;
  for (size_t i = 0; i < toks.Count() - 1; i++) {
    if (toks[i].Equalsi("Ctrl")) {
      Shift |= sssCtrl;
    }
    else if (toks[i].Equalsi("Shift")) {
      Shift |= sssShift;
    }
    else if (toks[i].Equalsi("Alt")) {
      Shift |= sssAlt;
    }
    else if (toks[i].Equalsi("Cmd")) {
      Shift |= sssCmd;
    }
  }
  olxstr charStr = toks.GetLastString();
  // a char
  if (charStr.Length() == 1) {
    Char = charStr[0];
    if (Char >= 'a' && Char <= 'z') {
      Char -= ('a' - 'A');
    }
    return ((Shift << 16) | Char);
  }
  if (charStr.CharAt(0) == 'F' && charStr.SubStringFrom(1).IsNumber()) {
    Char = WXK_F1 + charStr.SubStringFrom(1).ToInt() - 1;
    return ((Shift << 16) | Char);
  }
  charStr.UpperCase();
  if (charStr == "TAB")       Char = WXK_TAB;
  else if (charStr == "HOME") Char = WXK_HOME;
  else if (charStr == "PGUP") Char = WXK_PAGEUP;
  else if (charStr == "PGDN") Char = WXK_PAGEDOWN;
  else if (charStr == "END")  Char = WXK_END;
  else if (charStr == "DEL")  Char = WXK_DELETE;
  else if (charStr == "INS")  Char = WXK_INSERT;
  else if (charStr == "BREAK") Char = WXK_PAUSE;
  else if (charStr == "BACK") Char = WXK_BACK;

  return Char != 0 ? ((Shift << 16) | Char) : -1;
}
//..............................................................................
bool TMainForm::OnMouseDblClick(int x, int y, short Flags, short Buttons) {
  AGDrawObject *G = FXApp->SelectObject(x, y, false);
  if (Modes->GetCurrent() != 0 && Modes->GetCurrent()->OnDblClick()) {
    return true;
  }
  if (G == 0) {
    processMacro("sel -u");
    return true;
  }
  if (G->Is<TGlBitmap>()) {
    TGlBitmap* glB = (TGlBitmap*)G;
    if (!(glB->GetLeft() > 0)) {
      int Top = InfoWindowVisible ? FInfoBox->GetTop() + FInfoBox->GetHeight() : 1;
      for (size_t i = 0; i < FXApp->GlBitmapCount(); i++) {
        TGlBitmap* b = &FXApp->GlBitmap(i);
        if (b == glB) {
          break;
        }
        Top += (b->GetHeight() + 2);
      }
      glB->SetTop(Top);
      glB->SetLeft(FXApp->GetRenderer().GetWidth() - glB->GetWidth());
    }
    else {
      glB->SetLeft(0);
      glB->SetTop(InfoWindowVisible ? FInfoBox->GetTop() + FInfoBox->GetHeight() : 1);
    }
  }
  else if (G->Is<TXGlLabel>()) {
    olxstr label = "getuserinput(1, \'Please, enter new label\', \'";
    label << ((TXGlLabel*)G)->GetLabel() << "\')";
    if (processFunction(label.Replace('$', "\\$")) && !label.IsEmpty())
      ((TXGlLabel*)G)->SetLabel(label);

  }
  else if (G->Is<TXAtom>()) {
    TXAtom * xa = (TXAtom*)G;
    if (xa->CAtom().GetExyzGroup() != 0) {
      TGXApp::AtomIterator ai = FXApp->GetAtoms();
      while (ai.HasNext()) {
        TXAtom &a = ai.Next();
        if (a.CAtom().GetExyzGroup() == xa->CAtom().GetExyzGroup()) {
          a.SetSpecialDrawing(!a.IsSpecialDrawing());
        }
      }
      FXApp->SelectAll(false);
    }
    else if (FXApp->GetMouseHandler().IsSelectionEnabled()) {
      TNetwork& n = xa->GetNetwork();
      size_t sel_cnt = 0, cnt = 0;
      for (size_t i = 0; i < n.NodeCount(); i++) {
        TXAtom& a = (TXAtom&)n.Node(i);
        if (!a.IsVisible()) {
          continue;
        }
        cnt++;
        if (a.IsSelected()) {
          sel_cnt++;
        }
      }
      for (size_t i = 0; i < n.BondCount(); i++) {
        TXBond& b = (TXBond&)n.Bond(i);
        if (!b.IsVisible()) {
          continue;
        }
        cnt++;
        if (b.IsSelected()) {
          sel_cnt++;
        }
      }
      if (cnt > 0) {
        FXApp->SelectFragments(TNetPList() << &n, ((double)sel_cnt / cnt) < .75);
      }
    }
  }
  FXApp->Draw();
  return true;
}
//..............................................................................
bool TMainForm::Show(bool v) {
  bool res = wxFrame::Show(v);
  FXApp->SetMainFormVisible(v);
  if (v) {
    FXApp->GetRenderer().GetScene().SetEnabled(true);
  }
  if (CmdLineVisible) {
    FCmdLine->SetFocus();
  }
  else {
    FGlCanvas->SetFocus();
  }
  return res;
}
//..............................................................................
void TMainForm::UseGlTooltip(bool v) {
  if (v == _UseGlTooltip) {
    return;
  }
  TStateRegistry::GetInstance().SetState(stateGlTooltips, v, EmptyString(), true);
  _UseGlTooltip = v;
  if (v) {
    FGlCanvas->SetToolTip(wxT(""));
  }
}
//..............................................................................
//..............................................................................
//..............................................................................
void TMainForm::SaveVFS(short persistenceId) {
  try {
    olxstr dbFN;
    if (persistenceId == plStructure) {
      if (!FXApp->XFile().HasLastLoader()) {
        return;
      }
      dbFN = FXApp->XFile().GetStructureDataFolder();
      dbFN << TEFile::ChangeFileExt(
        TEFile::ExtractFileName(FXApp->XFile().GetFileName()) , "odb");
    }
    else if (persistenceId == plGlobal) {
      dbFN << FXApp->GetInstanceDir() << "global.odb";
    }
    else {
      throw TFunctionFailedException(__OlxSourceInfo,
        "undefined persistence level");
    }

    TEFile dbf(dbFN + ".tmp", "wb");
    TFileHandlerManager::SaveToStream(dbf, persistenceId);
    dbf.Close();
    TEFile::Rename(dbFN + ".tmp", dbFN);
  }
  catch (const TExceptionBase &e) {
    TBasicApp::NewLogEntry(logInfo) << "Failed to save VFS: " << e;
  }
}
//..............................................................................
void TMainForm::LoadVFS(short persistenceId) {
  try {
    olxstr dbFN;
    if (persistenceId == plStructure) {
      if (!FXApp->XFile().HasLastLoader()) {
        return;
      }
      dbFN = FXApp->XFile().GetStructureDataFolder();
      dbFN << TEFile::ChangeFileExt(
        TEFile::ExtractFileName(FXApp->XFile().GetFileName()) , "odb");
    }
    else if (persistenceId == plGlobal) {
      dbFN << FXApp->GetInstanceDir() << "global.odb";
    }
    else {
      throw TFunctionFailedException(__OlxSourceInfo,
        "undefined persistence level");
    }

    if (!TEFile::Exists(dbFN)) {
      return;
    }
    try {
      TEFile dbf(dbFN, "rb");
      TFileHandlerManager::LoadFromStream(dbf, persistenceId);
    }
    catch (const TExceptionBase &e) {
      TEFile::DelFile(dbFN);
      throw TFunctionFailedException(__OlxSourceInfo, e, "failed to read VFS");
    }
  }
  catch (const TExceptionBase &e) {
    TBasicApp::NewLogEntry(logInfo) << "Failed to read VFS";
  }
}
//..............................................................................
bool TMainForm::FindXAtoms(const TStrObjList &Cmds, TXAtomPList& xatoms,
  bool GetAll, bool unselect)
{
  size_t cnt = xatoms.Count();
  xatoms.AddAll(FXApp->FindXAtoms(Cmds, GetAll, unselect));
  return (xatoms.Count() != cnt);
}
//..............................................................................
bool TMainForm::IsControl(const olxstr& _cname) const {
  size_t di = _cname.IndexOf("->");
  olxstr pname = (di == InvalidIndex ? EmptyString() : _cname.SubStringTo(di));
  olxstr cname = (di == InvalidIndex ? _cname : _cname.SubStringFrom(di+2));
  THtml* html = HtmlManager.main;
  if (!pname.IsEmpty()) {
    THtmlManager::TPopupData *pd = HtmlManager.Popups.Find(pname, NULL);
    if (pd != NULL)
      html = pd->Html;
  }
  return html == NULL ? false : (html->FindObject(cname) != NULL);
}
//..............................................................................
void TMainForm::DoUpdateFiles()  {
  TBasicApp::EnterCriticalSection();
  if (_UpdateThread == NULL) {
    TBasicApp::LeaveCriticalSection();
    return;
  }
  uint64_t sz = _UpdateThread->GetUpdateSize();
  if (sz > 0 && !TBasicApp::IsBaseDirWriteable()
#ifdef __WIN32__
    && !TShellUtil::IsAdmin()
#endif
    )
  {
    TBasicApp::LeaveCriticalSection();
    wxMessageBox(wxT("There are new updates available, please run Olex2 with ")
      wxT("an elevated account.\nNote that if this message keeps appearing ")
      wxT("even after a successful update, type ")
      wxT("'shell DataDir()' in the Olex2 command line and remove\n")
      wxT("__location.update and __cmd.update files and the patch directory."),
      wxT("Updates available"), wxICON_INFORMATION, this);
    TBasicApp::EnterCriticalSection();
    if (_UpdateThread == NULL) {
      TBasicApp::LeaveCriticalSection();
      return;
    }
    _UpdateThread->SendTerminate();
    _UpdateThread = NULL;
    TBasicApp::LeaveCriticalSection();
    return;
  }
  updater::SettingsFile sf(updater::UpdateAPI::GetSettingsFileName());
  if (sf.ask_for_update) {
    // this is essential as it can freeze the GTK GUI
    TBasicApp::LeaveCriticalSection();
    TdlgMsgBox* msg_box = new TdlgMsgBox( this,
      olxstr("There are new updates available (") <<
        olxstr::FormatFloat(3, (double)sz/(1024*1024)) << "Mb)\n" <<
      "Updates will be downloaded in the background during this session and\n"
      "will take effect with the next restart of Olex2",
      "Automatic Updates",
      "Do not show this message again",
      wxYES|wxNO|wxICON_QUESTION,
      true);
    int res = msg_box->ShowModal();
    bool checked = msg_box->IsChecked();
    msg_box->Destroy();
    TBasicApp::EnterCriticalSection();
    if (_UpdateThread == 0) {
      TBasicApp::LeaveCriticalSection();
      return;
    }
    if (res == wxID_YES) {
      _UpdateThread->DoUpdate();
      if (checked) {
        sf.ask_for_update = false;
        sf.Save();
      }
    }
    else  {
      _UpdateThread->SendTerminate();
      _UpdateThread = 0;
      if (checked) {
        sf.update_interval = "Never";
        sf.Save();
      }
    }
  }
  else {
    _UpdateThread->DoUpdate();
  }
  if (_UpdateThread != 0) {
    _UpdateThread->ResetUpdateSize();
  }
  TBasicApp::LeaveCriticalSection();
}
//..............................................................................
size_t TMainForm::DownloadFiles(const TStrList& files, const olxstr& dest) {
  if (files.IsEmpty()) {
    return 0;
  }
  size_t cnt = 0;
  updater::SettingsFile sf(updater::UpdateAPI::GetSettingsFileName());
  TUrl proxy(sf.proxy);
  for (size_t i = 0; i < files.Count(); i++) {
    TUrl url(TEFile::UnixPath(files[i]));
    if (!proxy.GetHost().IsEmpty()) {
      url.SetProxy(proxy);
    }
    olx_object_ptr<AFileSystem> fs = HttpFSFromURL(url);
    if (!fs.ok()) {
      TBasicApp::NewLogEntry(logWarning) << "failed to get FS for "
        << url.GetFullAddress();
      continue;
    }
    try {
      olx_object_ptr<IInputStream> ins = fs->OpenFile(url.GetPath());
      if (ins.ok()) {
        if (!TEFile::Exists(dest)) {
          TEFile::MakeDir(dest);
        }
        const olxstr dest_fn = TEFile::AddPathDelimeter(dest) <<
          TEFile::ExtractFileName(url.GetPath());
        TEFile dest(dest_fn, "wb+");
        dest << *ins;
        dest.Close();
        cnt++;
      }
    }
    catch (const TExceptionBase& e) {
      TBasicApp::NewLogEntry(logError) << e.GetException()->GetFullMessage();
    }
  }
  return cnt;
}
//..............................................................................
void TMainForm::UpdateUserOptions(const olxstr &option, const olxstr &value) {
  FXApp->UpdateOption(option, value);
  FXApp->SaveOptions();
}
//..............................................................................
void TMainForm::OnCloseWindow(wxCloseEvent &evt) {
  olxstr v = FXApp->GetOptions().FindValue("confirm_on_close", TrueString());
  if (!v.IsBool()) v = FalseString();
  if (v.ToBool()) {
    olxstr rv = TdlgMsgBox::Execute(this, "Would you like to exit Olex2?",
      "Question",
      "Always exit without asking",
      wxYES|wxCANCEL|wxICON_QUESTION, true);
    bool do_exit = rv.Contains('Y');
    if (do_exit && rv.Containsi('R'))
      UpdateUserOptions("confirm_on_close", false);
    if (do_exit)
      Destroy();
    else {
      if (!evt.CanVeto())
        Destroy();
      else
        evt.Veto();
    }
  }
  else
    Destroy();
}
//..............................................................................
//..............................................................................
//..............................................................................

//..............................................................................
PyObject* pyIsControl(PyObject* self, PyObject* args)  {
  olxstr cname, pname;  // control and popup (if any) name
  if( !PythonExt::ParseTuple(args, "w|w", &cname, &pname) )
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "w|w");
  if (!pname.IsEmpty()) {
    cname = pname << "->" << cname;
  }
  return Py_BuildValue("b", TGlXApp::GetMainForm()->IsControl(cname));
}
//..............................................................................
PyObject* pyGetIdleTime(PyObject* self, PyObject* args)  {
  time_t t = TGlXApp::GetMainForm()->idle_time;
  return Py_BuildValue("d", (double)t/1000);
}
//..............................................................................
PyObject* pyResetIdleTime(PyObject* self, PyObject* args)  {
  TGlXApp::GetMainForm()->idle_time = 0;
  return PythonExt::PyNone();
}
//..............................................................................
PyObject* pyGetUserInput(PyObject* self, PyObject* args) {
  olxstr title, str;
  int flags = 0;
  if (!PythonExt::ParseTuple(args, "iww", &flags, &title, &str)) {
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "iww");
  }
  const bool MultiLine = (flags != 1);
  TdlgEdit* dlg = new TdlgEdit(TGlXApp::GetMainForm(), MultiLine);
  dlg->SetTitle(title.u_str());
  dlg->SetText(str);

  PyObject* rv;
  if (dlg->ShowModal() == wxID_OK)
    rv = PythonExt::BuildString(dlg->GetText());
  else {
    rv = Py_None;
    Py_IncRef(rv);
  }
  dlg->Destroy();
  return rv;
}
//..............................................................................
PyObject* pyGetUserStyledInput(PyObject* self, PyObject* args) {
  olxstr title, str;
  int lexer_code;
  int flags = 0;
  if (!PythonExt::ParseTuple(args, "iwwi", &flags, &title, &str, &lexer_code)) {
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "iwwi");
  }
  const bool MultiLine = (flags != 1);
  auto* dlg = new TdlgStyledEdit(TGlXApp::GetMainForm(), MultiLine);
  dlg->SetTitle(title.u_str());
  dlg->SetText(str);
  dlg->SetLexer(lexer_code);

  PyObject* rv;
  if (dlg->ShowModal() == wxID_OK)
    rv = PythonExt::BuildString(dlg->GetText());
  else {
    rv = Py_None;
    Py_IncRef(rv);
  }
  dlg->Destroy();
  return rv;
}
//..............................................................................
PyObject* pyPPI(PyObject* self, PyObject* args) {
  wxWindowDC wx_dc(TGlXApp::GetMainForm());
  wxSize ppi = wx_dc.GetPPI();
  return Py_BuildValue("(ii)", ppi.GetX(), ppi.GetY());
}
//..............................................................................
PyObject* pyPreprocessHtml(PyObject* self, PyObject* args) {
  olxstr html;
  if (!PythonExt::ParseTuple(args, "w", &html)) {
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "w");
  }
  THtmlPreprocessor htmlp;
  return PythonExt::BuildString(htmlp.Preprocess(html));
}
//..............................................................................
#ifdef _WIN32
PyObject* pyCheckPAROpened(PyObject* self, PyObject* args) {
  olxstr fn;
  if (!PythonExt::ParseTuple(args, "w", &fn)) {
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "w");
  }
  return PythonExt::PyBool(TGlXApp::FindWindow("CrysAlisPro", fn) != 0);
}
//..............................................................................
PyObject* pyActivateCAP(PyObject* self, PyObject* args) {
  olxstr fn;
  if (!PythonExt::ParseTuple(args, "w", &fn)) {
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "w");
  }
  HWND wnd = TGlXApp::FindWindow("CrysAlisPro", fn);
  if (wnd != 0) {
    TGlXApp::ActivateWindow(wnd);
    return PythonExt::PyTrue();
  }
  return PythonExt::PyFalse();
}
#endif
//..............................................................................
static PyMethodDef CORE_Methods[] = {
  {"GetUserInput", pyGetUserInput, METH_VARARGS,
  "Shows a dialog, where user can type some text. Takes three agruments: flags"
  ", title and content. If flags not equal to 1, a muliline dialog sis created"
  },
    {"GetUserStyledInput", pyGetUserStyledInput, METH_VARARGS,
  "Shows a dialog, where user can type some text. Takes four agruments: flags"
  ", title, content and lexer_code. If flags not equal to 1, a muliline dialog sis created. lexer_code defines the "
  "programming language. It uses wxSTC_LEX compatible ones."
  },
  {"IsControl", pyIsControl, METH_VARARGS,
  "Takes HTML element name and optionaly popup name. Returns true/false if "
  "given control exists"
  },
  {"GetPPI", pyPPI, METH_VARARGS, "Returns screen PPI"},
  { "GetIdleTime", pyGetIdleTime, METH_VARARGS, "Returns idle time" },
  { "ResetIdleTime", pyResetIdleTime, METH_VARARGS, "Resets idle time to 0" },
  { "PreprocessHtml", pyPreprocessHtml, METH_VARARGS, "Preprocesses given HTML as in the GUI" },
#ifdef _WIN32
  { "CheckPAROpened", pyCheckPAROpened, METH_VARARGS, "Checks if PAR file is loaded in CAP" },
  { "ActivateCAP", pyActivateCAP, METH_VARARGS, "Brings CAP to the front if PAR file is loaded" },
#endif
  { NULL, NULL, 0, NULL }
};
//..............................................................................
olxcstr &TMainForm::ModuleName() {
  static olxcstr mn = "olex_gui";
  return mn;
}
//..............................................................................
PyObject *TMainForm::PyInit() {
  return PythonExt::init_module(ModuleName(), CORE_Methods);
}
//..............................................................................
//..............................................................................
//..............................................................................
bool TMainForm::FileDropTarget::OnDropFiles(wxCoord x, wxCoord y,
  const wxArrayString& filenames)
{
  if( filenames.Count() != 1 )  return false;
  const olxstr fn = filenames[0];
  try  {
    if( parent.FXApp->XFile().FindFormat(TEFile::ExtractFileExt(fn)) == NULL )
      return false;
  }
  catch(...)  {  return false;  }
  parent.processMacro(olxstr("reap \"") << fn << '\"');
  return true;
}
//..............................................................................
bool TMainForm::PopupMenu(wxMenu* menu, const wxPoint& p)  {
  if( GlTooltip != NULL && _UseGlTooltip )
    GlTooltip->SetVisible(false);
  return wxWindow::PopupMenu(menu, p);
}
//..............................................................................
void TMainForm::UpdateInfoBox()  {
  FInfoBox->Clear();
  if (FXApp->XFile().HasLastLoader()) {
    FInfoBox->PostText(olxstr("\\-") <<
      TEFile::ExtractFilePath(FXApp->XFile().GetFileName()));
    FInfoBox->PostText(TEFile::ExtractFileName(FXApp->XFile().GetFileName()));
    FInfoBox->PostText(FXApp->XFile().LastLoader()->GetTitle());
    FInfoBox->Fit();
  }
  else  {
    FInfoBox->PostText("No file is loaded");
  }
}
//..............................................................................
void TMainForm::beforeCall(const olxstr &cmd) {
  OnNonIdle();
}
//..............................................................................
void TMainForm::afterCall(const olxstr &cmd) {
  OnNonIdle();
}
//..............................................................................
void TMainForm::OnNonIdle() {
  if (idle_start != 0) {
    idle_start = TETime::msNow() - idle_start;
    if (idle_start > 10000) {
      idle_time += idle_start;
    }
  }
  idle_start = 0;
}
//..............................................................................
//..............................................................................
//..............................................................................
void TMainForm::ProcessHandler::BeforePrint() {
  parent.FGlConsole->SetPrintMaterial(&parent.ExecFontColor);
  printed = false;
}
//..............................................................................
void TMainForm::ProcessHandler::Print(const olxstr& line) {
  if (!line.IsEmpty()) {
    TBasicApp::GetLog() << line;
     printed = true;
    parent.callCallbackFunc(ProcessOutputCBName, TStrList() << line);
  }
}
//..............................................................................
void TMainForm::ProcessHandler::AfterPrint() {
  parent.FGlConsole->SetPrintMaterial(0);
  if (printed) {
    parent.FXApp->Draw();
    printed = false;
  }
}
//..............................................................................
void TMainForm::ProcessHandler::OnWait() {
  parent.FParent->Yield();
  TBasicApp::GetInstance().OnTimer.Execute(&(AEventsDispatcher&)parent);
}
//..............................................................................
void TMainForm::ProcessHandler::OnTerminate(const AProcess& p) {
  TMacroData err;
  for (size_t i = 0; i < p.OnTerminateCmds().Count(); i++) {
    const olxstr& cmd = p.OnTerminateCmds()[i];
    parent.processMacro(cmd, olxstr("OnTerminate of: ") << p.GetCmdLine());
    if (!err.IsSuccessful()) {
      break;
    }
  }
  TBasicApp::NewLogEntry(logInfo) << "The process '" << p.GetCmdLine() <<
    "' has been terminated...";
  parent.TimePerFrame = parent.FXApp->Draw();
}
//..............................................................................
#ifdef _WIN32
WXLRESULT TMainForm::MSWWindowProc(WXUINT msg, WXWPARAM wParam, WXLPARAM lParam) {
  int msg_id = GetFileQueryEvtId();
  if (msg == msg_id && (HWND)wParam != GetHWND()) {
    const size_t max_sz = MAX_PATH * sizeof(olxch);
    if (lParam == 0) {
      static olxstr fn = TXApp::GetInstance().XFile().GetFileName();
      HANDLE hMapFile = CreateFileMapping(
        INVALID_HANDLE_VALUE,    // use paging file
        NULL,                    // default security
        PAGE_READWRITE,          // read/write access
        0,                       // maximum object size (high-order DWORD)
        max_sz,                  // maximum object size (low-order DWORD)
        GetFileQueryFileName().u_str());
      if (hMapFile != 0) {
        LPCTSTR pBuf = (LPTSTR)MapViewOfFile(hMapFile,
          FILE_MAP_ALL_ACCESS, // read/write permission
          0,
          0,
          max_sz);
        if (pBuf != 0) {
          CopyMemory((PVOID)pBuf, fn.u_str(), olx_min((fn.Length() + 1) * sizeof(olxch), max_sz));
          SendMessage((HWND)wParam, msg_id, (WPARAM)GetHWND(), 1);
          //PostThreadMessage(wParam, msg_id, (WPARAM)GetHWND(), 1);
          UnmapViewOfFile(pBuf);
        }
        else {
          TBasicApp::NewLogEntry(logError) << "Null view: " << GetLastError();
        }
        CloseHandle(hMapFile);
      }
      else {
        TBasicApp::NewLogEntry(logError) << "Null mapping: " << GetLastError();
      }
      return true;
    }
    else {
      HANDLE hMapFile = OpenFileMapping(
        FILE_MAP_ALL_ACCESS,
        FALSE,
        GetFileQueryFileName().u_str());
      if (hMapFile != 0) {
        LPCTSTR pBuf = (LPTSTR)MapViewOfFile(hMapFile,
          FILE_MAP_ALL_ACCESS,  // read/write permission
          0,
          0,
          max_sz);
        if (pBuf != 0) {
          loadedFiles.Add(pBuf, (HWND)wParam);
          UnmapViewOfFile(pBuf);
        }
        else {
          TBasicApp::NewLogEntry(logError) << "Null view: " << GetLastError();
        }
        CloseHandle(hMapFile);
      }
      else {
        TBasicApp::NewLogEntry(logError) << "Null mapping: " << GetLastError();
      }
      return true;
    }
  }
  return wxFrame::MSWWindowProc(msg, wParam, lParam);
}
//..............................................................................
const olxstr& TMainForm::GetFileQueryFileName() {
  static olxstr en("Olex2OpenedFiles");
  return en;
}
//..............................................................................
const olxstr& TMainForm::GetFileQueryEvtName() {
  static olxstr en("Olex2_FILE_QUERY_MSG");
  return en;
}
//..............................................................................
UINT TMainForm::GetFileQueryEvtId() {
  static UINT WM_QUERY_FILE = RegisterWindowMessage(GetFileQueryEvtName().u_str());
  return WM_QUERY_FILE;
}
//..............................................................................
struct QueryParams {
  HWND hwnd;
  int evt;
  QueryParams(HWND hwnd, int evt) : hwnd(hwnd), evt(evt)  {}
};

BOOL CALLBACK TMainForm::QueryOlex2Windows(HWND w, LPARAM p) {
  size_t max_sz = 256;
  olx_array_ptr<wchar_t> title(max_sz);
  int sz = GetWindowText(w, &title, max_sz - 1);
  if (sz >= 0) {
    olxstr t = olxstr::FromExternal(title.release(), sz, max_sz);
    QueryParams* q = (QueryParams*)p;
    if (t.StartsFrom("Olex2") && q->hwnd != w) {
      SendMessageTimeout(w, q->evt, (WPARAM)q->hwnd, 0,
        SMTO_ABORTIFHUNG, 1000, 0); // 1 sec timeout
    }
  }
  return TRUE;
}
//..............................................................................
void TMainForm::ListOlex2OpenedFiles() {
  loadedFiles.Clear();
  QueryParams q(GetHWND(), GetFileQueryEvtId());
  EnumDesktopWindows(0, &TMainForm::QueryOlex2Windows, (LPARAM)&q);
}
#endif //_WIN32
//..............................................................................
size_t TMainForm::LoadedFileIdx(const olxstr& fn_) {
#ifndef _WIN32
  return InvalidIndex;
#else
  olxstr ext = TEFile::ExtractFileExt(fn_).ToLowerCase();
  if (ext != "ins" && ext != "res") {
    return InvalidIndex;
  }
  const olxstr fn = TEFile::ChangeFileExt(fn_, EmptyString()).ToLowerCase();
  for (size_t i = 0; i < loadedFiles.Count(); i++) {
    olxstr ext = TEFile::ExtractFileExt(loadedFiles[i]).ToLowerCase();
    if (ext != "ins" && ext != "res") {
      continue;
    }
    if (fn == TEFile::ChangeFileExt(loadedFiles[i], EmptyString()).ToLowerCase()) {
      return i;
    }
  }
  return InvalidIndex;
#endif
}
//..............................................................................
