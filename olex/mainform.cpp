/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#define this_InitFunc(funcName, argc) \
  Library.RegisterFunction( new TFunction<TMainForm>(this, &TMainForm::fun##funcName, #funcName, argc))
#define this_InitMacro(macroName, validOptions, argc)\
  Library.RegisterMacro( new TMacro<TMainForm>(this, &TMainForm::mac##macroName, #macroName, #validOptions, argc))
#define this_InitMacroA(realMacroName, macroName, validOptions, argc)\
  Library.RegisterMacro( new TMacro<TMainForm>(this, &TMainForm::mac##realMacroName, #macroName, #validOptions, argc))
#define this_InitFuncD(funcName, argc, desc) \
  Library.RegisterFunction( new TFunction<TMainForm>(this, &TMainForm::fun##funcName, #funcName, argc, desc))
#define this_InitMacroD(macroName, validOptions, argc, desc)\
  Library.RegisterMacro( new TMacro<TMainForm>(this, &TMainForm::mac##macroName, #macroName, (validOptions), argc, desc))
#define this_InitMacroAD(realMacroName, macroName, validOptions, argc, desc)\
  Library.RegisterMacro( new TMacro<TMainForm>(this, &TMainForm::mac##realMacroName, #macroName, (validOptions), argc, desc))

#include "wx/utils.h"
#include "wx/wxhtml.h"
#include "wx/image.h"
#include "wx/panel.h"
#include "wx/fontdlg.h"
#include "wx/tooltip.h"
#include "wx/clipbrd.h"
#include "mainform.h"
#include "xglcanv.h"
#include "xglapp.h"
#include "wxglscene.h"

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

#ifdef _CUSTOM_BUILD_
  #include "custom_base.h"
#endif

#ifdef __GNUC__
  #undef Bool
#endif

  IMPLEMENT_CLASS(TMainForm, TMainFrame)

static const olxstr ProcessOutputCBName("procout");
static const olxstr OnStateChangeCBName("statechange");

class TObjectVisibilityChange: public AActionHandler  {
  TMainForm *FParent;
public:
  TObjectVisibilityChange(TMainForm *Parent){  FParent = Parent; }
  virtual ~TObjectVisibilityChange()  {  ;  }
  bool Execute(const IEObject *Sender, const IEObject *Obj)  {
    if( !Obj )  return false;
    if( EsdlInstanceOf(*Obj, TDBasis) )
      FParent->BasisVChange();
    else if( EsdlInstanceOf(*Obj, TDUnitCell) )
      FParent->CellVChange();
    else  if( Obj == FParent->FInfoBox )
      FParent->processMacro("showwindow info false");
    else  if( Obj == FParent->FHelpWindow )
      FParent->processMacro("showwindow help false");
    else if( EsdlInstanceOf(*Obj, TXGrid) )
      FParent->GridVChange();
    else if( EsdlInstanceOf(*Obj, T3DFrameCtrl) )
      FParent->FrameVChange();
    return true;
  }
};
#ifdef __WIN32__
class SplashDlg : public wxDialog  {
  wxBitmap *bmp;
  int imgHeight, imgWidth, txtHeight, txtWidth;
  bool has_alpha;
public:
  SplashDlg(wxWindow *parent) :
      wxDialog(parent, -1, wxT("Initialising"), wxDefaultPosition, wxSize(100, 100),
        wxNO_BORDER|wxFRAME_SHAPED) 
  {
    wxDialog::SetTitle(wxT("Olex2 splash screen"));
    bmp = NULL;
    imgHeight = 0;
    imgWidth = 200;
    has_alpha = false;
    olxstr splash_img = TBasicApp::GetBaseDir() + "splash.jpg";
    if( TEFile::Exists(splash_img) )  {
      wxImage img;
      img.LoadFile(splash_img.u_str());
      has_alpha = img.HasAlpha();
      if (has_alpha)
        img.ConvertAlphaToMask();
      bmp = new wxBitmap(img);
      imgWidth = img.GetWidth();
      imgHeight = img.GetHeight();
      if (has_alpha) {
        wxRegion reg(*bmp);
        SetShape(reg);
      }
    }
    wxWindowDC dc(this);
    wxSize sz = dc.GetTextExtent(wxT("Olex2 is initialising"));
    int ScreenW = wxSystemSettings::GetMetric(wxSYS_SCREEN_X),
        ScreenH = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y);
    const int ch = imgHeight + sz.y; // combined height
    SetSize((ScreenW-imgWidth)/2, (ScreenH-ch)/2, imgWidth, ch);
    txtHeight = sz.y;
    txtWidth = sz.GetWidth();
  }
  ~SplashDlg()  {
    if( bmp != NULL )
      delete bmp;
  }
  void DoPaint()  {
    static size_t generation = 0;
    wxWindowDC dc(this);
    dc.SetBackgroundMode(wxTRANSPARENT);
    dc.SetBackground(*wxTRANSPARENT_BRUSH);//*wxWHITE_BRUSH);
    //dc.Clear();
    if( bmp != NULL )
      dc.DrawBitmap(*bmp, 0, 0, true);
    wxString str(wxT("Olex2 is initialising"));
    for( size_t i=0; i < generation; i++ )
      str += '.';
    wxColor cl(0x8b7566);
    //wxColor cl(0xb4);
    dc.SetBrush(wxBrush(cl));//*wxWHITE_BRUSH);
    dc.SetPen(wxPen(cl));//*wxWHITE_PEN);
    int h = has_alpha ? imgHeight-txtHeight : imgHeight;
    dc.DrawRectangle(0, h, imgWidth, h);
    dc.SetTextForeground(*wxWHITE);
    dc.DrawText(str, (imgWidth-olx_round(txtWidth*1.5))/2, h);
    if( ++generation > 20 )
      generation = 0;
  }
};
class RefreshTh : public AOlxThread  {
  SplashDlg& dlg;
public:
  RefreshTh(SplashDlg& _dlg) :dlg(_dlg)  {  Detached = false;  }
  int Run()  {
    while( true )  {
      if( Terminate )  return 1;
      dlg.DoPaint();
      //wxApp::GetInstance()->ProcessPendingEvents();
      olx_sleep(200);
    }
  }
};
#endif
/******************************************************************************/
//----------------------------------------------------------------------------//
// TMainForm function bodies
//----------------------------------------------------------------------------//
BEGIN_EVENT_TABLE(TMainForm, wxFrame)  // basic interface
  EVT_SIZE(TMainForm::OnSize)
  EVT_MOVE(TMainForm::OnMove)
  EVT_MENU(ID_FILE0, TMainForm::OnFileOpen)
  EVT_MENU(ID_FILE0+1, TMainForm::OnFileOpen)
  EVT_MENU(ID_FILE0+2, TMainForm::OnFileOpen)
  EVT_MENU(ID_FILE0+3, TMainForm::OnFileOpen)
  EVT_MENU(ID_FILE0+4, TMainForm::OnFileOpen)
  EVT_MENU(ID_FILE0+5, TMainForm::OnFileOpen)
  EVT_MENU(ID_FILE0+6, TMainForm::OnFileOpen)
  EVT_MENU(ID_FILE0+7, TMainForm::OnFileOpen)
  EVT_MENU(ID_FILE0+8, TMainForm::OnFileOpen)
  EVT_MENU(ID_FILE0+9, TMainForm::OnFileOpen)

  EVT_MENU(ID_HtmlPanel, TMainForm::OnHtmlPanel)

  EVT_MENU(ID_StrGenerate, TMainForm::OnGenerate)

  EVT_MENU(ID_DSBS, TMainForm::OnDrawStyleChange) // drawing styles
  EVT_MENU(ID_DSWF, TMainForm::OnDrawStyleChange)
  EVT_MENU(ID_DSSP, TMainForm::OnDrawStyleChange)
  EVT_MENU(ID_DSES, TMainForm::OnDrawStyleChange)
  EVT_MENU(ID_DSST, TMainForm::OnDrawStyleChange)
  EVT_MENU(ID_SceneProps, TMainForm::OnDrawStyleChange)

  EVT_MENU(ID_DQH, TMainForm::OnDrawQChange) // drawing quality
  EVT_MENU(ID_DQM, TMainForm::OnDrawQChange)
  EVT_MENU(ID_DQL, TMainForm::OnDrawQChange)

  EVT_MENU(ID_CellVisible, TMainForm::OnCellVisible)  // model menu
  EVT_MENU(ID_BasisVisible, TMainForm::OnBasisVisible)
  EVT_MENU(ID_ShowAll, TMainForm::OnShowAll)
  EVT_MENU(ID_ModelCenter, TMainForm::OnModelCenter)

  EVT_MENU(ID_GraphicsHide, TMainForm::OnGraphics)
  EVT_MENU(ID_GraphicsKill, TMainForm::OnGraphics)
  EVT_MENU(ID_GraphicsDS, TMainForm::OnGraphics)
  EVT_MENU(ID_GraphicsP, TMainForm::OnGraphics)
  EVT_MENU(ID_GraphicsEdit, TMainForm::OnGraphics)
  EVT_MENU(ID_GraphicsSelect, TMainForm::OnGraphics)
  EVT_MENU(ID_FixLattice, TMainForm::OnGraphics)
  EVT_MENU(ID_FreeLattice, TMainForm::OnGraphics)
  EVT_MENU(ID_GridMenuCreateBlob, TMainForm::OnGraphics)

  EVT_MENU(ID_FragmentHide, TMainForm::OnFragmentHide)
  EVT_MENU(ID_FragmentShowOnly, TMainForm::OnFragmentShowOnly)
  EVT_MENU(ID_FragmentSelectAtoms, TMainForm::OnFragmentSelectAtoms)
  EVT_MENU(ID_FragmentSelectBonds, TMainForm::OnFragmentSelectBonds)
  EVT_MENU(ID_FragmentSelectAll, TMainForm::OnFragmentSelectAll)

  EVT_MENU(ID_AtomTypeChangeC, TMainForm::OnAtomTypeChange)
  EVT_MENU(ID_AtomTypeChangeN, TMainForm::OnAtomTypeChange)
  EVT_MENU(ID_AtomTypeChangeO, TMainForm::OnAtomTypeChange)
  EVT_MENU(ID_AtomTypeChangeF, TMainForm::OnAtomTypeChange)
  EVT_MENU(ID_AtomTypeChangeH, TMainForm::OnAtomTypeChange)
  EVT_MENU(ID_AtomTypeChangeS, TMainForm::OnAtomTypeChange)
  EVT_MENU(ID_AtomTypePTable, TMainForm::OnAtomTypePTable)
  EVT_MENU(ID_AtomGrow, TMainForm::OnAtom)
  EVT_MENU(ID_AtomCenter, TMainForm::OnAtom)
  EVT_MENU(ID_AtomSelRings, TMainForm::OnAtom)

  EVT_MENU(ID_PlaneActivate, TMainForm::OnPlane)
  
  EVT_MENU(ID_BondViewAlong, TMainForm::OnBond)

  EVT_MENU(ID_View100, TMainForm::OnViewAlong)
  EVT_MENU(ID_View010, TMainForm::OnViewAlong)
  EVT_MENU(ID_View001, TMainForm::OnViewAlong)
  EVT_MENU(ID_View110, TMainForm::OnViewAlong)
  EVT_MENU(ID_View101, TMainForm::OnViewAlong)
  EVT_MENU(ID_View011, TMainForm::OnViewAlong)
  EVT_MENU(ID_View111, TMainForm::OnViewAlong)

  EVT_MENU(ID_AtomOccu1, TMainForm::OnAtomOccuChange)
  EVT_MENU(ID_AtomOccu34, TMainForm::OnAtomOccuChange)
  EVT_MENU(ID_AtomOccu12, TMainForm::OnAtomOccuChange)
  EVT_MENU(ID_AtomOccu13, TMainForm::OnAtomOccuChange)
  EVT_MENU(ID_AtomOccu14, TMainForm::OnAtomOccuChange)
  EVT_MENU(ID_AtomOccuFree, TMainForm::OnAtomOccuChange)
  EVT_MENU(ID_AtomOccuFix, TMainForm::OnAtomOccuChange)

  EVT_MENU(ID_AtomConn0, TMainForm::OnAtomConnChange)
  EVT_MENU(ID_AtomConn1, TMainForm::OnAtomConnChange)
  EVT_MENU(ID_AtomConn2, TMainForm::OnAtomConnChange)
  EVT_MENU(ID_AtomConn3, TMainForm::OnAtomConnChange)
  EVT_MENU(ID_AtomConn4, TMainForm::OnAtomConnChange)
  EVT_MENU(ID_AtomConn12, TMainForm::OnAtomConnChange)

  EVT_MENU(ID_AtomPolyNone, TMainForm::OnAtomPolyChange)
  EVT_MENU(ID_AtomPolyAuto, TMainForm::OnAtomPolyChange)
  EVT_MENU(ID_AtomPolyRegular, TMainForm::OnAtomPolyChange)
  EVT_MENU(ID_AtomPolyPyramid, TMainForm::OnAtomPolyChange)
  EVT_MENU(ID_AtomPolyBipyramid, TMainForm::OnAtomPolyChange)

  EVT_MENU(ID_AtomPartCustom, TMainForm::OnAtomPartChange)
  EVT_MENU(ID_AtomPart_2, TMainForm::OnAtomPartChange)
  EVT_MENU(ID_AtomPart_1, TMainForm::OnAtomPartChange)
  EVT_MENU(ID_AtomPart0, TMainForm::OnAtomPartChange)
  EVT_MENU(ID_AtomPart1, TMainForm::OnAtomPartChange)
  EVT_MENU(ID_AtomPart2, TMainForm::OnAtomPartChange)

  EVT_MENU(ID_AtomUisoCustom, TMainForm::OnAtomUisoChange)
  EVT_MENU(ID_AtomUiso12, TMainForm::OnAtomUisoChange)
  EVT_MENU(ID_AtomUiso15, TMainForm::OnAtomUisoChange)
  EVT_MENU(ID_AtomUisoFree, TMainForm::OnAtomUisoChange)
  EVT_MENU(ID_AtomUisoFix, TMainForm::OnAtomUisoChange)

  EVT_MENU(ID_SelGroup, TMainForm::OnSelection)
  EVT_MENU(ID_SelUnGroup, TMainForm::OnSelection)
  EVT_MENU(ID_SelLabel, TMainForm::OnSelection)

  EVT_MENU(ID_GStyleSave, TMainForm::OnGraphicsStyle)
  EVT_MENU(ID_GStyleOpen, TMainForm::OnGraphicsStyle)
END_EVENT_TABLE()
//..............................................................................
TMainForm::TMainForm(TGlXApp *Parent):
  TMainFrame(wxT("Olex2"), wxPoint(0,0), wxDefaultSize, wxT("MainForm")),
  Macros(*this),
  HtmlManager(*(new THtmlManager(this))),
  _ProcessHandler(*this)
{
  TEGC::AddP(&HtmlManager);
  ShowChemicalOccu = true;
  nui_interface = NULL;
  _UpdateThread = NULL;
  ActionProgress = UpdateProgress = NULL;
  SkipSizing = false;
  Destroying = false;
#ifdef __WIN32__
  _UseGlTooltip = false;  // Linux and Mac set tooltips after have been told to do so...
#else
  _UseGlTooltip = true;
#endif
  StartupInitialised = RunOnceProcessed = false;
  wxInitAllImageHandlers();
  /* a singleton - will be deleted in destructor, we cannot use GC as the Py_DecRef
   would be called after finalising python
  */
  PythonExt::Init(this).Register(&TMainForm::PyInit);
  PythonExt::GetInstance()->Register(&OlexPyCore::PyInit);
  PythonExt::GetInstance()->Register(&hkl_py::PyInit);
  //TOlxVars::Init().OnVarChange->Add(this, ID_VarChange);
  FGlCanvas = NULL;
  FXApp = NULL;
  FGlConsole = NULL;
  FInfoBox = NULL;
  GlTooltip = NULL;
  ActiveLogFile = NULL;

  MousePositionX = MousePositionY = -1;

  LabelToEdit = NULL;
  
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
  ObjectUnderMouse(NULL);
  FHelpItem = NULL;
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
bool TMainForm::Destroy()  {
  if( FXApp != NULL )  {
    SaveVFS(plGlobal);  // save virtual db to file
    SaveVFS(plStructure);
    FXApp->OnObjectsDestroy.Remove(this);
    processMacro("onexit");
    SaveSettings(FXApp->GetConfigDir() + FLastSettingsFile);
  }
  Destroying = true;
  HtmlManager.Destroy();
  return wxFrame::Destroy();
}
//..............................................................................
TMainForm::~TMainForm()  {
  if( _UpdateThread != NULL )  {
    _UpdateThread->OnTerminate.Remove(this);
    _UpdateThread->Join(true);
    delete _UpdateThread;
  }
  if( UpdateProgress != NULL )
    delete UpdateProgress;
  // clean up it here
  FXApp->GetStatesRegistry().OnChange.Clear();
  delete Modes;
  for( size_t i=0; i < CallbackFuncs.Count(); i++ )
    delete CallbackFuncs.GetObject(i);
  // delete FIOExt;

//   if( FXApp->XFile().GetLastLoader() ) // save curent settings
//   {
//     T = TEFile::ChangeFileExt(FXApp->XFile().FileName(), "xlds");
//     FXApp->Render()->Styles()->SaveToFile(T);
//   }
  if( ActiveLogFile != NULL )  {
    delete ActiveLogFile;
    ActiveLogFile = NULL;
  }
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
  // the order is VERY important!
  TOlxVars::Finalise();
  PythonExt::Finilise();
}
//..............................................................................
void TMainForm::XApp(TGXApp *XA)  {
  FXApp = XA;

  _ProcessManager = new ProcessManager(_ProcessHandler);
  FXApp->SetCifTemplatesDir(XA->GetBaseDir() + "etc/CIF/");
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
  Macros.Init();
  TLibrary &Library = XA->GetLibrary();
  this_InitMacroAD(Reap, @reap,
    "b-blind&;"
    "r-also relaod the default style&;"
    "*-read and overlay",
    fpAny,
    "This macro loads a file if provided as an argument. If no argument is "
    "provided, it shows a file open dialog");
  this_InitMacroD(Pict,
    "c-embossed output in color&;"
    "bw-embossed output in b&w&;"
    "pq-highest (picture) quality&;"
    "nbg-mask the background with 0 alpha (an rgb colour may be gives)",
    fpOne|fpTwo,
    "Outputs a raster picture. Output file name is required, if a second "
    "numerical parameter is provided, it is considered to be image resolution "
    "in range [0.1-10] in screen sizes, anythin greater than 10 is considered "
    "as the desired picture width.");
  this_InitMacroD(Picta,
    "pq-picture quality&;"
    "nbg-mask the background with 0 alpha (an rgb colour may be gives)",
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
    "atom_outline_oversize- the extra size of outline in percents[5]",
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

  this_InitMacroD(Bang, "c-copy info to the clipboard", fpAny|psFileLoaded,
    "Prints bonds and angles table for selected/given atoms");
  this_InitMacroD(Uniq, EmptyString(), fpAny | psFileLoaded,
    "Shows only fragments specified by atom name(s) or selection");

  this_InitMacroD(Group,
    "n-a custom name can be provided",
    fpNone|fpOne|psFileLoaded,
  "Groups current visible objects or selection");

  this_InitMacroD(Fmol, EmptyString(), fpNone|psFileLoaded,
    "Shows all fragments (as opposite to uniq)");
  this_InitMacroD(Clear, EmptyString(), fpNone,
    "Clears console buffer (text)");

  this_InitMacroD(Cell, "r-shows reciprocal cell",
    fpNone|fpOne|psFileLoaded,
    "If no arguments provided inverts visibility of unit cell, otherwise sets"
    " it to the boolean value of the parameter");
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

  this_InitMacroD(Sel,
    "a-select all&;"
    "u-unselect all&;"
    "l-consider the list of bonds as independent&;"
    "c-copies printed values to the clipboard&;"
    "i-invert selection&;",
    fpAny,
    "If no arguments provided, prints current selection. This includes "
    "distances, angles and torsion angles and other geometrical parameters. "
    "Selects atoms fulfilling provided conditions, like"
    "\nsel $type - selects any particular atom type; type can be one of the "
    "following shortcuts - * - for all atoms, M - for metals, X - for halogens"
    "\nsel $*,type - selects all but type atoms"
    "\n\n"
    "An extended syntax include keyword 'where' and 'rings' which "
    "allow selecting atoms and bonds according to their properties, like type "
    "and length or rings of particular connectivity like C6 or NC5. If the "
    "'where' keyword is used, logical operators, like and (&&), and or (||) "
    "can be used to refine the selection. For example:"
    "\nsel atoms where xatom.bai.z > 2 - to select all atoms heavier after H"
    "\nsel bonds where xbond.length > 2 - to select all bonds longer than 2 A"
    "\nsel bonds where xbond.b.bai.z == 1 - to select all bonds where the "
    "lightest atoms is H"
    );

  this_InitMacroD(Capitalise, "", (fpAny|psFileLoaded)^fpNone,
    "Changes atom labels capitalisation for all/given/selected atoms. The"
    " first argument is the template like Aaaa");

  this_InitMacroD(PiM,
    "l-display labels for the created lines",
    fpAny|psFileLoaded,
    "Creates an illustration of a pi-system to metal bonds");

  this_InitMacroD(Esd,
    "label-creates a graphics label&;"
    "l-consider the list of bonds as independent&;"
    "c-copies printed values to the clipboard",
    fpAny|psFileLoaded,
    "This procedure calculates possible parameters for the selection and "
    "evaluates their esd using the variance-covariance matrix coming from the "
    "ShelXL refinement with negative 'MORE' like 'MORE -1' option or from the "
    "olex2.refine");
  

  this_InitMacroD(SetEnv, EmptyString(), fpTwo,
"Sets an environmental variable");

  this_InitMacroD(SetView, "c-center", fpAny,
    "Sets view normal to the normal of the selected plane, to a bond or mean "
    "line");
  this_InitMacroD(Help, "c-specifies commands category", fpAny,
    "Prints available information. If no arguments provided prints available "
    "commands");
  this_InitMacroD(Matr, EmptyString(), fpNone|fpOne|fpTwo|fpThree|fpNine,
    "Displays or sets current orientation matrix. For single argument, 1,2,3 "
    "001, 111, etc values are acceptable, two values taken are of the klm "
    "form, which specify a view from k1*a+l1*b+m1*c to k2*a+l2*b+m2*c, three "
    "values pecify the view normal and nine values provide a full matrix");

  this_InitMacroD(Line,
    "n-just sets current view normal to the line without creating the object",
    fpAny,
    "Creates a line or best line for provided atoms");
  this_InitMacro(AddLabel, , fpThree|fpFive);
  this_InitMacroD(Mpln,
    "n-just orient, do not create plane&;"
    "r-create regular plane&;"
    "we-use weights proportional to the (atomic mass)^we", 
    fpAny, "Sets current view along the normal of the best plane");
  this_InitMacroD(Cent, EmptyString(), fpAny,
    "Creates a centroid for given/selected/all atoms");

  this_InitMacroD(Hide, EmptyString(), fpAny,
    "Hides selected objects or provided atom names (no atom related objects as"
    " bonds are hidden automatically)");
  this_InitMacroD(Kill,
    "au-kill atoms in the asymmetric unit (disregarding visibility/"
    "availability). This option is intended for the internal use - the model is"
    " not rebuilt after its execution",
    fpAny^fpNone,
    "Deletes provided/selected atoms, bonds and other objects");

  this_InitMacroD(Exec,
    "s-synchronise&;"
    "o-detached&;"
    "d-output dub file name&;"
    "q-do not post output to console",
    fpAny^fpNone,
    "Executes external process");
  this_InitMacroD(Shell, EmptyString(), fpAny,
    "If no arguments launches a new interactive shell, otherwise runs provided"
    " file in the interactive shell (on windows ShellExecute is used to avoid "
    "flickering console)");
  this_InitMacro(Save, , fpAny^fpNone);
  this_InitMacro(Load, , fpAny^fpNone);
  this_InitMacro(Link, , fpNone|fpOne);
  this_InitMacroD(Style, "s-shows a file open dialog", fpNone|fpOne,
    "Prints default style or sets it (none resets)");
  this_InitMacroD(Scene, "s-shows a file open dialog", fpNone|fpOne,
    "Prints default scene parameters or sets it (none resets)");

  this_InitMacroD(Basis, EmptyString(), fpNone|fpOne,
    "Shows/hides the orientation basis");
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
      "afix\n&;"
    "p-[name] prefix; [grow] inserts the new atoms into the AU with given [-1]"
      " part value\n&;"
    "s-[grow] short interactions; [name] suffix;"
      " [fit] split, atoms to split offset [0]&;"
    "t-[name] type\n&;"
    "c-[grow] covalent bonds; [move] copy fragments instead of moving\n&;"
    "r-[split] a restraint/constraint for split atoms; [grow] show radial "
    "bonds between the same atoms; [fit] rotation angle increment (smooth "
    "rotation by default)\n&;"
    "v-[grow] use user provided delta for connectivity analysis, default 2A\n&;"
    "shells-[grow] grow atom shells vs fragments", 
    (fpAny^fpNone)|psFileLoaded, 
    "Turns specified mode on. Valid mode: fixu, fixc, grow, himp, match, move,"
    " name, occu, pack, part, split, fit");

  this_InitMacroD(Text, EmptyString(), fpNone|fpOne,
    "Shows the console buffer in an external editor, defined by defeditor "
    "variable");
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
  this_InitMacroD(Split, "r-EADP,ISOR or SIMU to be placed for the split atoms",
    fpAny|psCheckFileTypeIns,
    "Splits provided atoms along the longest axis of the ADP");
  this_InitMacro(ShowP, m-do not modify the display view, fpAny);

  this_InitMacro(EditAtom, cs-do not clear the selection,fpAny|psCheckFileTypeIns);
  this_InitMacro(EditIns, , fpNone|psCheckFileTypeIns);
  this_InitMacro(HklEdit, , fpNone|fpOne|fpThree);
  this_InitMacro(HklView, , fpNone|fpOne);

  // not implemented
  this_InitMacro(HklExtract, , fpOne|psFileLoaded);

  this_InitMacroD(Direction, EmptyString(), fpNone,
    "Prints current orientation of the model in factional coordinates");
  this_InitMacro(ViewGrid, , fpNone|fpOne);
  this_InitMacro(Undo, , fpNone);

  this_InitMacroD(Individualise, EmptyString(), fpAny,
    "Moves provided atoms to individual collections, so that the atom "
    "properties, such as draw style and appearance can be changed separately "
    "of the group. The first call to this macro creates a group unique to the"
    " asymmetric unit, the second call makes the atom unique to the lattice");
  this_InitMacroD(Collectivise, EmptyString(), fpAny,
    "Does the opposite to the Individialise. If provided atoms are unique to "
    "the lattice a call to this function makes them uniq to the asymmetric "
    "unit, the following call makes the uniq to the element type");

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

  this_InitMacroD(Delta, EmptyString(), fpNone|fpOne,
    "Prints/sets current delta fir the covalent bonds");
  this_InitMacroD(DeltaI, EmptyString(), fpNone|fpOne,
    "Prints/sets current delta for short interactions");

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

  this_InitMacroD(UpdateOptions, EmptyString(), fpNone, "Shows the Update Options dialog");
  this_InitMacroD(Update, EmptyString(), fpNone, "Does check the for the updates");
  this_InitMacro(Reload, , fpOne);
  this_InitMacro(StoreParam, , fpTwo|fpThree);
  this_InitMacro(SelBack, a&;o&;x, fpNone);

  this_InitMacro(CreateBitmap, r, fpTwo);
  this_InitMacro(DeleteBitmap, , fpOne);
  this_InitMacro(Tref, ,fpOne|fpTwo|psCheckFileTypeIns);
  this_InitMacro(Patt, ,fpNone|psCheckFileTypeIns);
  this_InitMacro(Export, ,fpNone|fpOne|psCheckFileTypeCif);

  this_InitMacro(InstallPlugin,
    "l-local installation from a zip file, which must contains index.ind",
    fpOne);
  this_InitMacro(SignPlugin, ,fpAny^(fpOne|fpNone));
  this_InitMacro(UninstallPlugin, ,fpOne);
  this_InitMacro(UpdateFile, f,fpOne);
  this_InitMacro(NextSolution, ,fpNone);

  this_InitMacroD(Match,
    "s-subgraph match&;"
    "w-use Z as weights&;"
    "n-naming. If the value a symbol [or set of] this is appended to the "
    "label, '$xx' replaces the symbols after the atom type symbol with xx, "
    "leaving the ending, '-xx' - changes the ending of the label with xx&;"
    "a-align&;"
    "i-try inversion&;"
    "u-unmatch&;"
    "esd-calculate esd (works for pairs only)",
    fpNone|fpOne|fpTwo,
    "Fragment matching, alignment and label transfer routine");
  this_InitMacroD(Conn, EmptyString(), fpAny^fpNone,
    "Changes provided atom(s) connectivity (only until next connectivity "
    "modifying operation for now)."
    "\nUsage: conn max_bond bonding_radius [selection/atom(s)/$type]"
    "\nUsage: conn max_bond [selection/atom(s)/$type]"
    "\nUsage: conn bonding_radius [selection/atom(s)/$type] - note the radius"
    " should have floating point"
    );
  this_InitMacroD(AddBond, EmptyString(), fpAny,
    "Adds specified bond to the connectivity table");
  this_InitMacroD(DelBond, EmptyString(), fpAny,
    "Removes specified bond from the connectivity table");
  this_InitMacro(ShowWindow, ,fpOne|fpTwo);
  
  this_InitMacroD(OFileDel, EmptyString(), fpOne,
    "Deletes overlayed file specified by index");
  this_InitMacroD(OFileSwap, EmptyString(), fpOne,
    "Makes overlayed file, given by index the current file to which all "
    "commands are applied");
  this_InitMacroD(CalcVol,
    "n-normalises bonds before the calculation&;"
    "cs-do not clear the selection",
    fpNone|fpOne,
    "Calculates tetrahedron or bipyramidal shape volume for given (selected) "
    "atom");

  this_InitMacroD(Schedule, "r-repeatable", fpAny^(fpNone|fpOne),
    "Schedules a particular macro (second argument) to be executed within "
    "provided interval (first argument)");

  this_InitMacroD(Tls,
    "a-apply the TLS ADP to the atoms",
    fpAny|psFileLoaded,
    "TLS procedure. The TLS is calculated for the given atoms and then the "
    "matrices rotated to the L axes (making L diagonal) and shifted to make S "
    "symmetric. The printed R1 is calculated for ADPs in the L axes and is:\n"
    "R1=sum(i=1..3,j=i..3)(|Uobs_ij-Utls_ij|)/sum(i=1..3, j=i..3)(|Uobs_ij|)"
    "\nR2' is invariant under the rotation and is calculated as\n"
    "R2'=sum(i=1..3,j=1..3)((Uobs_ij-Utls_ij)^2)/sum(i=1..3,j=1..3)(Uobs_ij^2)"
    );

  this_InitMacro(Test, , fpAny);

  this_InitMacroD(CalcVoid,
    "d-distance from Van der Waals surface [0]&;r-resolution[0.2]&;"
    "p-precise calculation&;"
    "i-invert the map for rendering",
    fpNone|fpOne|psFileLoaded,
      "Calculates solvent accessible void and packing parameters; optionally "
      "accepts a file with space separated values of Atom Type and radius, an "
      "entry a line");
  this_InitMacroD(Sgen, EmptyString(), (fpAny^fpNone)|psFileLoaded,
    "Grows the structure using provided atoms (all if none provided) and "
    "symmetry code");
  this_InitMacroD(LstSymm, EmptyString(), fpNone|psFileLoaded,
    "Prints symmetry codes for current structure");
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
  this_InitMacroD(AddObject, EmptyString(), fpAny^(fpNone|fpOne),
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
  this_InitMacroD(SetMaterial, EmptyString(), fpTwo | fpThree,
    "Assigns provided value to specified material");
  this_InitMacroD(LstGO, EmptyString(), fpNone,
    "List current graphical objects");
  this_InitMacroD(TestBinding, EmptyString(), fpAny,
    "Internal tests");
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
    "a-set specified AFIX to the imported fragment",
    fpNone|psFileLoaded,
    "Import a fragment into current structure");
  this_InitMacroD(ExportFrag, EmptyString(), fpNone|psFileLoaded,
    "Exports selected fragment to an external file");
  this_InitMacroD(ProjSph,
    "g-sphere quality [6]",
    fpAny|psFileLoaded, 
    "Creates a projection from the selected atom onto a sphere, coloring each "
    "point on the sphere with a unique color corresponding to fragments.");
  this_InitMacroD(UpdateQPeakTable, EmptyString(), fpNone|psFileLoaded,
    "Internal routine for synchronisation");
  this_InitMacroD(SAME,
    "i-invert the graphs&;"
    "e-expand SAME into the list of SADI",
    fpAny|psFileLoaded,
    "Creates SAME instruction for two fragments (two selected atoms or two "
    "atoms provided) or number_of_groups and groups following each another "
    "(or selection)");
  this_InitMacroD(RESI, "a-alias", (fpAny^fpNone)|psFileLoaded,
    "Creates residue with given class name and optionally number and adds "
    "selected or provided atoms into the residue. If provided residue class "
    "name is 'none', provided atoms are removed from their residues");
  this_InitMacroD(WBox,
    "w-use atomic mass instead of unit weights for atoms&;"
    "s-create separate boxes for fragments", 
    (fpAny)|psFileLoaded,
    "Calculates wrapping box around provided box using the set of best, "
    "intermidiate and worst planes");
  this_InitMacroD(Center, "z-also recalculates the scene zoom", 
    (fpAny)|psFileLoaded, "Sets the centre of rotation to given point");
  this_InitMacroD(FlushFS, EmptyString(), 
    (fpOne|fpNone),
    "Saves current content of the virtual file system. If no parameters is "
    "given - the global state is saved. Possible arguments: global, "
    "structure");

  this_InitMacroD(ChemDraw, "", fpAny|psFileLoaded,
    "Currently only creates aromatic rings for Ph, Py and Cp rings");
  this_InitMacroD(Restrain, "", fpAny|psFileLoaded,
    "Creates a restraint");
  this_InitMacroD(Constrain, "", (fpAny^fpNone)|psFileLoaded,
    "Creates a constraint");

  this_InitMacroD(Tolman, "mpd-M to P distance", fpNone|fpFive|psFileLoaded,
    "Calculates Tolamn code angle for the selection (M P S1 S2 S3)");

  this_InitMacroD(Poly, EmptyString(), (fpAny^fpNone)|psFileLoaded,
    "Sets polyhedra type for all/selected/given atoms. Last argument specifies"
    " the type - none, auto, regular");

  // FUNCTIONS _________________________________________________________________

  this_InitFunc(FileLast, fpNone|fpOne);
  this_InitFunc(FileSave, fpThree);
  this_InitFunc(FileOpen, fpThree);
  this_InitFunc(ChooseDir, fpNone|fpOne|fpTwo);

  this_InitFunc(Cell, fpOne|psFileLoaded);

  this_InitFuncD(Cif, fpOne|psCheckFileTypeCif,
    "Returns instruction value (all data after the instruction). In case the "
    "instruction does not exist it return 'n/a' string");
  this_InitFuncD(P4p, fpOne|psCheckFileTypeP4P,
    "Returns instruction value (all data after the instruction). In case the "
    "instruction does not exist it return 'n/a' string");
  this_InitFuncD(Crs, fpOne|psCheckFileTypeCRS,
    "Returns instruction value (all data after the instruction). In case the "
    "instruction does not exist it return 'n/a' string");

  this_InitFunc(Strcat, fpTwo);
  this_InitFunc(Strcmp, fpTwo);

  this_InitFuncD(GetEnv, fpNone|fpOne,
    "Prints all variables if no arguments is given or returns the given "
    "veariable value");

  this_InitFunc(Eval, fpOne);

  this_InitFunc(VVol, fpNone|fpOne|psFileLoaded);

  this_InitFunc(Env, fpOne|psFileLoaded);
  this_InitFunc(Atoms, fpOne|psFileLoaded);

  this_InitFunc(Sel, fpNone|psFileLoaded);
  this_InitFunc(FPS, fpNone);

  this_InitFunc(Cursor, fpNone|fpOne|fpTwo|fpThree);
  this_InitFunc(RGB, fpThree|fpFour);
  this_InitFunc(Color, fpNone|fpOne|fpTwo);

  this_InitFunc(Zoom, fpNone|fpOne);
  this_InitFunc(HtmlPanelWidth, fpNone|fpOne);

  #ifdef __WIN32__
  this_InitFunc(LoadDll, fpOne);
  #endif

  this_InitFunc(CmdList, fpOne);
  this_InitFunc(Alert, fpTwo|fpThree|fpFour);

  this_InitFunc(IsPluginInstalled, fpOne);
  this_InitFunc(ValidatePlugin, fpOne);

  // number of lines, caption, def value
  this_InitFunc(GetUserInput, fpThree);

  this_InitFuncD(TranslatePhrase, fpOne,
"Translates provided phrase into current language");
  this_InitFuncD(IsCurrentLanguage, fpOne,
"Checks current language");
  this_InitFuncD(CurrentLanguageEncoding, fpNone,
"Returns current language encoding, like: ISO8859-1");

  this_InitFunc(SGList, fpNone);

  this_InitFunc(ChooseElement, fpNone);

  this_InitFuncD(StrDir, fpNone|psFileLoaded,
    "Returns location of the folder, where Olex2 stores structure related "
    "data");

  this_InitFuncD(ChooseFont, fpNone|fpOne|fpTwo,
    "Brings up a font dialog. If font information provided, initialises the "
    "dialog with that font; the first argument may be just 'olex2' or 'system'"
    " to enforce choosing the Olex2/System font (the font information can be "
    "provided in the second argument then)");
  this_InitFuncD(GetFont, fpOne,
    "Returns specified font");
  this_InitFuncD(GetMaterial, fpOne|fpTwo,
    "Returns material of specified object");
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
  this_InitFuncD(ExtraZoom, fpNone|fpOne,
    "Sets/reads current extra zoom (default zoom correction)");
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
  this_InitFuncD(MatchFiles, fpTwo|fpThree,
    "Matches given files");
  this_InitFuncD(Freeze, fpNone|fpOne,
    "Gets/Sets display update status");
  Library.AttachLibrary(FXApp->ExportLibrary());
  Library.AttachLibrary(TEFile::ExportLibrary());
  //Library.AttachLibrary(olxstr::ExportLibrary("str"));
  Library.AttachLibrary(PythonExt::GetInstance()->ExportLibrary());
  Library.AttachLibrary(TETime::ExportLibrary());
  Library.AttachLibrary(lcells::IndexManager::ExportLibrary());
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
  Library.AttachLibrary(XA->XFile().ExportLibrary());
  Library.AttachLibrary(XA->GetFader().ExportLibrary());
  Library.AttachLibrary(XA->XGrid().ExportLibrary());
  Library.AttachLibrary(TFileHandlerManager::ExportLibrary());
  TOlxVars::ExportLibrary(EmptyString(), &Library);
#ifdef _CUSTOM_BUILD_
  CustomCodeBase::Initialise(Library);
#endif
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
// setting atom menu
  pmAtom->Append(ID_MenuItemAtomInfo, wxT("?"));
  miAtomInfo = pmAtom->FindItemByPosition(0);
  pmAtom->AppendSeparator();
  pmAtom->Append(ID_MenuBang, wxT("BANG"), pmBang);
    pmAtomType->Append(ID_AtomTypeChangeC, wxT("C"));
    pmAtomType->Append(ID_AtomTypeChangeN, wxT("N"));
    pmAtomType->Append(ID_AtomTypeChangeO, wxT("O"));
    pmAtomType->Append(ID_AtomTypeChangeF, wxT("F"));
    pmAtomType->Append(ID_AtomTypeChangeH, wxT("H"));
    pmAtomType->Append(ID_AtomTypeChangeS, wxT("S"));
    pmAtomType->Append(ID_AtomTypePTable, wxT("More..."));
    pmAtomOccu->Append(ID_AtomOccu1,  wxT("1"));
    pmAtomOccu->Append(ID_AtomOccu34, wxT("3/4"));
    pmAtomOccu->Append(ID_AtomOccu12, wxT("1/2"));
    pmAtomOccu->Append(ID_AtomOccu13, wxT("1/3"));
    pmAtomOccu->Append(ID_AtomOccu14, wxT("1/4"));
    pmAtomOccu->Append(ID_AtomOccuFix, wxT("Fix"));
    pmAtomOccu->Append(ID_AtomOccuFree, wxT("Free"));
    pmAtomConn->Append(ID_AtomConn0, wxT("0"));
    pmAtomConn->Append(ID_AtomConn1, wxT("1"));
    pmAtomConn->Append(ID_AtomConn2, wxT("2"));
    pmAtomConn->Append(ID_AtomConn3, wxT("3"));
    pmAtomConn->Append(ID_AtomConn4, wxT("4"));
    pmAtomConn->Append(ID_AtomConn12, wxT("Default"));
    pmAtomPoly->AppendRadioItem(ID_AtomPolyNone, wxT("None"));
    pmAtomPoly->AppendRadioItem(ID_AtomPolyAuto, wxT("Auto"));
    pmAtomPoly->AppendRadioItem(ID_AtomPolyRegular, wxT("Regular"));
    pmAtomPoly->AppendRadioItem(ID_AtomPolyPyramid, wxT("Pyramid"));
    pmAtomPoly->AppendRadioItem(ID_AtomPolyBipyramid, wxT("Bipyramid"));
    miAtomPartCustom = pmAtomPart->AppendRadioItem(ID_AtomPartCustom, wxT("X"));
    pmAtomPart->AppendRadioItem(ID_AtomPart_2, wxT("-2"));
    pmAtomPart->AppendRadioItem(ID_AtomPart_1, wxT("-1"));
    pmAtomPart->AppendRadioItem(ID_AtomPart0, wxT("0"));
    pmAtomPart->AppendRadioItem(ID_AtomPart1, wxT("1"));
    pmAtomPart->AppendRadioItem(ID_AtomPart2, wxT("2"));
    miAtomUisoCustom = pmAtomUiso->AppendRadioItem(ID_AtomUisoCustom, wxT("X"));
    pmAtomUiso->AppendRadioItem(ID_AtomUiso12, wxT("1.2x"));
    pmAtomUiso->AppendRadioItem(ID_AtomUiso15, wxT("1.5x"));
    miAtomUisoFree = pmAtomUiso->AppendRadioItem(ID_AtomUisoFree, wxT("Free"));
    pmAtomUiso->AppendRadioItem(ID_AtomUisoFix, wxT("Fix"));

  pmAtom->Append(ID_MenuAtomType, wxT("Type"), pmAtomType);
  pmAtom->Append(ID_MenuAtomConn, wxT("Bonds"), pmAtomConn);
  pmAtom->Append(ID_MenuAtomOccu, wxT("Chemical occupancy"), pmAtomOccu);
  pmAtom->Append(ID_MenuAtomPart, wxT("Part"), pmAtomPart);
  pmAtom->Append(ID_MenuAtomUiso, wxT("Uiso"), pmAtomUiso);
  pmAtom->Append(ID_MenuAtomPoly, wxT("Polyhedron"), pmAtomPoly);
  pmAtom->AppendSeparator();
  pmAtom->Append(ID_AtomGrow, wxT("Grow"));
    miAtomGrow = pmAtom->FindItemByPosition(pmAtom->GetMenuItemCount()-1);
  pmAtom->Append(ID_AtomCenter, wxT("Center"));
  pmAtom->Append(ID_AtomSelRings, wxT("Select ring(s)"));
  pmAtom->AppendSeparator();
  pmAtom->Append(ID_GraphicsKill, wxT("Delete"));
  pmAtom->AppendSeparator();
  pmAtom->Append(ID_MenuFragment, wxT("Fragment"), pmFragment->Clone());
  pmAtom->Append(ID_MenuGraphics, wxT("Graphics"), pmGraphics->Clone());
  pmAtom->Append(ID_Selection, wxT("Selection"), pmSelection->Clone());
// setting bond menu
  pmBond->Append(ID_MenuItemBondInfo, wxT("?"));
  pmBond->AppendSeparator();
  miBondInfo = pmBond->FindItemByPosition(0);
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
  pmPlane->Append(ID_Selection, wxT("Selection"), pmSelection->Clone());
  pmPlane->AppendSeparator();
// setting view menu
  pmView->Append(ID_View100, wxT("100"));
  pmView->Append(ID_View010, wxT("010"));
  pmView->Append(ID_View001, wxT("001"));
  pmView->Append(ID_View110, wxT("110"));
  pmView->Append(ID_View101, wxT("101"));
  pmView->Append(ID_View011, wxT("011"));
  pmView->Append(ID_View111, wxT("111"));
// update to selection menu - need to add graphics
  pmSelection->AppendSeparator();
  pmSelection->Append(ID_MenuGraphics, wxT("Graphics"), pmGraphics->Clone());

  Menus.Add("File", MenuFile);
  Menus.Add("View", MenuView);
  Menus.Add("Structure", MenuStructure);
  Menus.Add("Help", MenuHelp);

  SetMenuBar(MenuBar);
//////////////////////////////////////////////////////////////
  FXApp->GetRender().OnDraw.Add(this, ID_GLDRAW, msiExit);
  TObjectVisibilityChange* VC = new TObjectVisibilityChange(this);
  XA->OnGraphicsVisible.Add(VC);
  // put correct captions to the menu
  pmModel->SetLabel(ID_CellVisible, FXApp->IsCellVisible() ?
    wxT("Hide cell") : wxT("Show cell"));
  pmModel->SetLabel(ID_BasisVisible, FXApp->IsBasisVisible() ?
    wxT("Hide basis") : wxT("Show basis"));
  TutorialDir = XA->GetBaseDir()+"etc/";
//  DataDir = TutorialDir + "Olex_Data\\";
  DictionaryFile = XA->GetBaseDir() + "dictionary.txt";
  PluginFile =  XA->GetBaseDir() + "plugins.xld";
  FHtmlIndexFile = TutorialDir+"index.htm";

  TFileHandlerManager::AddBaseDir(TutorialDir);
  TFileHandlerManager::AddBaseDir(FXApp->GetInstanceDir());

  SetStatusText(XA->GetBaseDir().u_str());

  // put log file to the user data folder
  XA->CleanupLogs();
  XA->CreateLogFile(XA->GetOptions().FindValue("log_name", "olex2"));

  TBasicApp::GetLog().OnInfo.Add(this, ID_INFO, msiEnter);
  TBasicApp::GetLog().OnWarning.Add(this, ID_WARNING, msiEnter);
  TBasicApp::GetLog().OnError.Add(this, ID_ERROR, msiEnter);
  TBasicApp::GetLog().OnException.Add(this, ID_EXCEPTION, msiEnter);
  FXApp->OnObjectsDestroy.Add(this, ID_XOBJECTSDESTROY, msiEnter);
  XLibMacros::OnDelIns.Add(this, ID_DELINS, msiExit);
  XLibMacros::OnAddIns.Add(this, ID_ADDINS, msiExit);
  LoadVFS(plGlobal);

  HtmlManager.InitialiseMain(4|wxVSCROLL|wxALWAYS_SHOW_SB);
  GetLibrary().AttachLibrary(HtmlManager.ExportLibrary());

  HtmlManager.OnLink.Add(this, ID_ONLINK);
  HtmlManager.main->OnKey.Add(this, ID_HTMLKEY);

  FXApp->SetLabelsVisible(false);
  FXApp->GetRender().LightModel.SetClearColor(0x0f0f0f0f);

  FGlConsole = new TGlConsole(FXApp->GetRender(), "Console");
  // the commands are posted from in Dispatch, SkipPosting is controlling the output
  FXApp->GetLog().AddStream(FGlConsole, false);
  FGlConsole->OnCommand.Add(this, ID_COMMAND);
  FGlConsole->OnPost.Add(this, ID_TEXTPOST);
  FXApp->AddObjectToCreate(FGlConsole);
////////////////////////////////////////////////////////////////////////////////
  Library.AttachLibrary(FGlConsole->ExportLibrary());
  Library.AttachLibrary(FXApp->GetRender().ExportLibrary());
////////////////////////////////////////////////////////////////////////////////
  FCmdLine = new TCmdLine(this, wxNO_BORDER);
//  wxWindowDC wdc(this);
//  FCmdLine->WI.SetHeight(wdc.GetTextExtent(wxT("W")).GetHeight());
  FCmdLine->OnChar.Add(this, ID_CMDLINECHAR);
  FCmdLine->OnKeyDown.Add(this, ID_CMDLINEKEYDOWN);
  FCmdLine->OnCommand.Add( this, ID_COMMAND);

  FHelpWindow = new TGlTextBox(FXApp->GetRender(), "HelpWindow");
  FXApp->AddObjectToCreate(FHelpWindow);
  FHelpWindow->SetVisible(false);

  FInfoBox = new TGlTextBox(FXApp->GetRender(), "InfoBox");
  FXApp->AddObjectToCreate(FInfoBox);

  GlTooltip = new TGlTextBox(FXApp->GetRender(), "Tooltip");
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
      states.NewGetter(*this, &TMainForm::CheckState),
      new TStateRegistry::TMacroSetter("HtmlPanelVisible")
    )
  );
  statePluginInstalled = states.Register("pluginInstalled",
    new TStateRegistry::Slot(
      states.NewGetter(*this, &TMainForm::CheckState),
      new TStateRegistry::TMacroSetter("HtmlPanelVisible")
    )
  );
  stateInfoWidnowVisible = states.Register("infovis",
    new TStateRegistry::Slot(
      TStateRegistry::NewGetter<TGlTextBox>(*FInfoBox, &TGlTextBox::IsVisible),
      new TStateRegistry::TMacroSetter("ShowWindow info")
    )
  );
  stateHelpWindowVisible = states.Register("helpvis",
    new TStateRegistry::Slot(
      TStateRegistry::NewGetter<TGlTextBox>(*FHelpWindow, &TGlTextBox::IsVisible),
      new TStateRegistry::TMacroSetter("ShowWindow help")
    )
  );
  stateCmdLineVisible = states.Register("cmdlinevis",
    new TStateRegistry::Slot(
      states.NewGetter(*this, &TMainForm::CheckState),
      new TStateRegistry::TMacroSetter("ShowWindow cmdline")
    )
  );
  //stateGlTooltips = States->Register("GLTT");

  // synchronise if value is different in settings file...
  miHtmlPanel->Check(!FHtmlMinimized);
#ifdef __WIN32__
  SplashDlg splash_dlg(this);
  RefreshTh rth(splash_dlg);
  splash_dlg.Show();
  rth.Start();
#endif
#if defined(__WIN32__) || defined(__MAC__)
  StartupInit();
#endif
#ifdef __WIN32__
  rth.Join(true);
#endif
  try  {
    nui_interface = olx_nui::Initialise();
    if( nui_interface != NULL )
      nui_interface->InitProcessing(olx_nui::INUI::processSkeleton
      |olx_nui::INUI::processVideo);
  }
  catch(const TExceptionBase &e)  {
    FXApp->NewLogEntry(logError) << e.GetException()->GetError();
  }
  try {
    ShowChemicalOccu = FXApp->GetOptions().FindValue(
      "tooltip_occu_chem", TrueString()).ToBool();
  }
  catch(...) {
    TBasicApp::NewLogEntry(logInfo) <<
      (olxstr("Invalid boolean value for ").quote() << "tooltip_occu_chem");
  }
  TBasicApp::GetInstance().NewActionQueue(olxappevent_UPDATE_GUI).Add(this, ID_UPDATE_GUI);
}
//..............................................................................
void TMainForm::StartupInit()  {
  if( StartupInitialised )  return;
  StartupInitialised = true;
  wxFont Font(10, wxMODERN, wxNORMAL, wxNORMAL);//|wxFONTFLAG_ANTIALIASED);
  TGlMaterial glm("2049;0.698,0.698,0.698,1.000");
  AGlScene& gls = FXApp->GetRender().GetScene();
  gls.CreateFont("Default", Font.GetNativeFontInfoDesc()).SetMaterial(glm);
  gls.CreateFont("Help", Font.GetNativeFontInfoDesc()).SetMaterial(glm);
  gls.CreateFont("Notes", Font.GetNativeFontInfoDesc()).SetMaterial(glm);
  gls.CreateFont("Labels", Font.GetNativeFontInfoDesc()).SetMaterial(glm);
  gls.RegisterFontForType<TXAtom>(
    gls.CreateFont("AtomLabels", Font.GetNativeFontInfoDesc())).SetMaterial(glm);
  gls.RegisterFontForType<TXBond>(
    gls.CreateFont("BondLabels", Font.GetNativeFontInfoDesc())).SetMaterial(glm);
  gls.CreateFont("Tooltip", Font.GetNativeFontInfoDesc()).SetMaterial(glm);
  gls.RegisterFontForType<TDBasis>(gls._GetFont(4));
  gls.RegisterFontForType<TDUnitCell>(gls._GetFont(4));
  gls.RegisterFontForType<TXGlLabels>(gls._GetFont(3));
  gls.RegisterFontForType<TGlConsole>(gls._GetFont(0));
  gls.RegisterFontForType<TGlCursor>(gls._GetFont(0));

  FHelpWindow->SetFontIndex(1);
  FInfoBox->SetFontIndex(2);
  GlTooltip->SetFontIndex(6);

  olxstr T(FXApp->GetConfigDir());
  T << FLastSettingsFile;
  if( !TEFile::Exists(T) )  {
    T = TBasicApp::GetBaseDir();
    TEFile::AddPathDelimeterI(T);
    T << FLastSettingsFile;
  }
  try  {  LoadSettings(T);  }
  catch(const TExceptionBase& e)  {
    ShowAlert(e);
    //throw;
  }

  FXApp->Init(); // initialise the gl after styles reloaded
  if( !GradientPicture.IsEmpty() )  // need to call it after all objects are created
    processMacro(olxstr("grad ") << " -p=\'" << GradientPicture << '\'');

  processMacro(olxstr("showwindow help ") << HelpWindowVisible);
  processMacro(olxstr("showwindow info ") << InfoWindowVisible);
  processMacro(olxstr("showwindow cmdline ") << CmdLineVisible);
  FGlConsole->ShowBuffer(true);  // this should be on :)
  processMacro("reload macro", __OlxSrcInfo);
  processMacro("reload help", __OlxSrcInfo);

  FTimer->Start(15);
  if( FGlCanvas != NULL )  FGlCanvas->XApp(FXApp);

  if( TEFile::Exists(FXApp->GetBaseDir() + "settings.xld") )  {
    TDataFile settings;
    settings.LoadFromXLFile(FXApp->GetBaseDir() + "settings.xld", NULL);
    settings.Include(NULL);
    TDataItem* sh = settings.Root().FindItemi("shortcuts");
    if( sh != NULL )  {
      try  {
        olxstr cmd;
        for( size_t i=0; i < sh->ItemCount(); i++ )  {
        TDataItem& item = sh->GetItem(i);
        AccShortcuts.AddAccell(
          TranslateShortcut(item.GetFieldValue("key")),
          item.GetFieldValue("macro"));
        // cannot execute it through a macro - functions get evaluated...
        //Macros.ProcessMacro(cmd, MacroError);
        }
      }
      catch( TExceptionBase& exc )  {
        TBasicApp::NewLogEntry(logException) << exc.GetException()->GetFullMessage();
      }
    }
    sh = settings.Root().FindItemi("menus");
    if( sh != NULL )  {
      try  {
        olxstr cmd;
        for( size_t i=0; i < sh->ItemCount(); i++ )  {
          TDataItem& item = sh->GetItem(i);
          cmd = "createmenu \'";
          cmd << item.GetFieldValue("title") << "\' \'" <<
            item.GetFieldValue("macro") << '\'';

          olxstr before = item.GetFieldValue("before");
          if( !before.IsEmpty() )
            cmd << " \'" << before <<'\'';
          cmd << ' ';

          olxstr modeDep = item.GetFieldValue("modedependent");
          if( !modeDep.IsEmpty() )
            cmd << " -m=\'" << modeDep << '\'';
          cmd << ' ';

          olxstr stateDep = item.GetFieldValue("statedependent");
          if( !stateDep.IsEmpty() )
            cmd << " -s=\'" << stateDep << '\'';
          cmd << ' ';

          if( item.GetName().Equalsi("radio") )  cmd << "-r ";
          if( item.GetName().Equalsi("sep") )    cmd << "-# ";
          if( item.GetName().Equalsi("check") )  cmd << "-c ";

          processMacro(cmd, __OlxSrcInfo);
        }
      }
      catch( TExceptionBase& exc )  {
        TBasicApp::NewLogEntry(logException) << exc.GetException()->GetFullMessage();
      }
    }
  }

  FPluginItem = NULL;
  if( TEFile::Exists( PluginFile ) )  {
    FPluginFile.LoadFromXLFile(PluginFile, NULL);
    FPluginItem = FPluginFile.Root().FindItem("Plugin");
    // manually activate the events
    for( size_t i=0; i < FPluginItem->ItemCount(); i++ )  {
      TStateRegistry::GetInstance().SetState(
        statePluginInstalled, true, FPluginItem->GetItem(i).GetName(), true);
    }
  }
  else
    FPluginItem = &FPluginFile.Root().AddItem("Plugin");

  // set the variables
  for( size_t i=0; i < StoredParams.Count(); i++ )  {
    processMacro(olxstr("setvar(") << StoredParams.GetKey(i) << ",\'" <<
      StoredParams.GetObject(i) << "\')");

  }

  if( Dictionary.GetCurrentLanguage().IsEmpty() )  {
    try  { Dictionary.SetCurrentLanguage(DictionaryFile, "English");  }
    catch(...) {}
  }
  // do the iterpreters job...
  if (FXApp->GetArguments().Count() >= 2) {
    if (FXApp->GetArguments().GetLastString().EndsWith(".py")) {
      TStrList in;
      in.LoadFromFile(FXApp->GetArguments().GetLastString());
      PythonExt::GetInstance()->RunPython(in.Text('\n'));
    }
    else // disable reading last file in
      TOlxVars::GetInstance()->SetVar("olx_reap_cmdl", FXApp->GetArguments()[1]);
  }
  processMacro("onstartup", __OlxSrcInfo);
  processMacro("user_onstartup", __OlxSrcInfo);
  if( FXApp->GetArguments().Count() >= 2 ) {
    processMacro(olxstr("reap \'") << FXApp->GetArguments().Text(' ', 1) <<
      '\'', __OlxSrcInfo);
  }  // load html in last call - it might call some destructive functions on uninitialised data
  HtmlManager.main->LoadPage(FHtmlIndexFile.u_str());
  HtmlManager.main->SetHomePage(FHtmlIndexFile);
  // must move it here since on Linux things will not get initialised at the previous position
  CreateUpdateThread();
  FileDropTarget* dndt = new FileDropTarget(*this);
  this->SetDropTarget(dndt);
}
//..............................................................................
bool TMainForm::CreateUpdateThread() {
  volatile olx_scope_cs cs(TBasicApp::GetCriticalSection());
  if (_UpdateThread != NULL) return false;
#ifndef __WIN32__ // do updates on non-Win only if the folder is writable
  if( FXApp->IsBaseDirWriteable() )  {
#endif
    _UpdateThread = new UpdateThread(FXApp->GetInstanceDir() +
      patcher::PatchAPI::GetPatchFolder());
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
void TMainForm::AquireTooltipValue()  {
  AGDrawObject *G = FXApp->SelectObject(MousePositionX, MousePositionY, 0);
  Tooltip.SetLength(0);
  if( G != NULL )  {
    if( G->IsSelected() )
      Tooltip = FXApp->GetSelectionInfo();
    else if( EsdlInstanceOf( *G, TXAtom) )  {
      const TXAtom &xa = *(TXAtom*)G;
      const TCAtom& ca = xa.CAtom();
      Tooltip = xa.GetGuiLabelEx();
      if( xa.GetType() == iQPeakZ )
        Tooltip << ':' << xa.CAtom().GetQPeak();
      double occu = (ShowChemicalOccu ? ca.GetChemOccu() : ca.GetOccu());
      Tooltip << (ShowChemicalOccu ? "\nChem occu(" : "\nXtal occu(");
      if( ca.GetVarRef(catom_var_name_Sof) != NULL )  {
        if( ca.GetVarRef(catom_var_name_Sof)->relation_type == relation_None )  {
          Tooltip << "fixed): ";
          if( olx_abs(occu-olx_round(occu)) < 1e-3 )
            occu = olx_round(occu);
        }
        else  {
          Tooltip << "linked to ";
          if( olx_abs(ca.GetVarRef(catom_var_name_Sof)->coefficient-1.0) > 1e-6 )
            Tooltip << olxstr(ca.GetVarRef(catom_var_name_Sof)->coefficient).TrimFloat() << 'x';
          if( ca.GetVarRef(catom_var_name_Sof)->relation_type == relation_AsVar )
            Tooltip << "[FVAR#";
          else
            Tooltip << "[1-FVAR #";
          Tooltip << (ca.GetVarRef(catom_var_name_Sof)->Parent.GetId()+1) << "]): ";
        }
      }
      else
        Tooltip << "free): ";
      Tooltip << TEValueD(occu, ca.GetOccuEsd()*ca.GetDegeneracy()).ToString();
      if( ca.GetEllipsoid() == NULL )  {
        Tooltip << "\nUiso (";
        if( ca.GetVarRef(catom_var_name_Uiso) != NULL && 
          ca.GetVarRef(catom_var_name_Uiso)->relation_type == relation_None &&
          ca.GetUisoOwner() == NULL)
        {
          Tooltip << "fixed): " << olxstr::FormatFloat(3, ca.GetUiso());
        }
        else if( ca.GetUisoOwner() != NULL )
          Tooltip << "riding): " << olxstr::FormatFloat(3, ca.GetUiso());
        else
          Tooltip << "free): " << TEValueD(ca.GetUiso(), ca.GetUisoEsd()).ToString();
      }
      else
        Tooltip << "\nUeq " << olxstr::FormatFloat(3, ca.GetEllipsoid()->GetUeq());
#ifdef _DEBUG
      Tooltip << "\nBonds: " << xa.BondCount() << ", nodes: " << xa.NodeCount();
      if( xa.GetEllipsoid() == NULL )  {
        Tooltip << "\nV: " << olxstr::FormatFloat(3,
          pow(xa.CAtom().GetUiso(), 3./2)*4*M_PI/3, true);
      }
      else {
        Tooltip << "\nV: " << olxstr::FormatFloat(3,
          xa.GetEllipsoid()->GetSX()*xa.GetEllipsoid()->GetSY()*
          xa.GetEllipsoid()->GetSZ()*4*M_PI/3, true);
      }
#endif
    }
    else  if( EsdlInstanceOf( *G, TXBond) )  {
      TXBond& xb = *(TXBond*)G;
      Tooltip = xb.A().GetLabel();
      Tooltip << '-' << xb.B().GetLabel() << ": ";
      if( FXApp->CheckFileType<TCif>() )  {
        ACifValue* cv = FXApp->XFile().GetLastLoader<TCif>()
          .GetDataManager().Match(xb.A(), xb.B());
        if( cv != NULL )
          Tooltip << cv->GetValue().ToString();
        else
          Tooltip << olxstr::FormatFloat(3, xb.Length());
      }
      else
        Tooltip << olxstr::FormatFloat(3, xb.Length());
#ifdef _DEBUG
      vec3d n = (xb.A().crd()-xb.B().crd()).Normalise();
      Tooltip << "\nn: " << olx_round(n[0], 1000) << ',' << olx_round(n[1], 1000) <<
        ',' << olx_round(n[2], 1000);
#endif
    } 
    else if( EsdlInstanceOf( *G, TXReflection) )  {
      Tooltip = ((TXReflection*)G)->GetHKL()[0];
      Tooltip << ' '
              << ((TXReflection*)G)->GetHKL()[1] << ' '
              << ((TXReflection*)G)->GetHKL()[2] << ": "
              << ((TXReflection*)G)->GetI();
    }
    else if( EsdlInstanceOf( *G, TXLine) )  {
      Tooltip = olxstr::FormatFloat(3, ((TXLine*)G)->Length());
    }
    else if( EsdlInstanceOf( *G, TXGrowLine) )  {
      Tooltip = ((TXGrowLine*)G)->XAtom().GetLabel();
      Tooltip << '-' << ((TXGrowLine*)G)->CAtom().GetLabel() << ": "
          << olxstr::FormatFloat(3, ((TXGrowLine*)G)->Length()) << '('
          << TSymmParser::MatrixToSymmEx(((TXGrowLine*)G)->GetTransform()) << ')';
    }
    else if( EsdlInstanceOf( *G, TXGrowPoint) )  {
      Tooltip = TSymmParser::MatrixToSymmEx(((TXGrowPoint*)G)->GetTransform());
    }
    else if( EsdlInstanceOf( *G, TXPlane) )  {
      Tooltip << "HKL direction: " <<
        ((TXPlane*)G)->GetCrystallographicDirection().ToString();
    }
  }
}
//..............................................................................
bool TMainForm::Dispatch( int MsgId, short MsgSubId, const IEObject *Sender, const IEObject *Data)  {
  bool res = true, Silent = (FMode & mSilent) != 0, Draw=false;
  static bool actionEntered = false, downloadEntered=false;
  if( Destroying )  {
    FMode = 0;  // to release waitfor 
    return false;
  }
  if( MsgId == ID_GLDRAW && !IsIconized() )  {
    if( !FBitmapDraw )  {
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
  else if( MsgId == ID_UpdateThreadTerminate )  {
    volatile olx_scope_cs cs(TBasicApp::GetCriticalSection());
    _UpdateThread = NULL;

     if( UpdateProgress != NULL )  {
       delete UpdateProgress;
       UpdateProgress = NULL;
     }
  }
  else if( MsgId == ID_UpdateThreadDownload )  {
    volatile olx_scope_cs cs( TBasicApp::GetCriticalSection());
    if( MsgSubId == msiEnter )  {
      if( UpdateProgress == NULL )
        UpdateProgress = new TOnProgress;
    }
    if( MsgSubId == msiExecute && Data != NULL && EsdlInstanceOf(*Data, TOnProgress) )  {
      TOnProgress& pg = *(TOnProgress*)Data;
      if( UpdateProgress != NULL )
        *UpdateProgress = pg;
      downloadEntered = true;
      AOlxThread::Yield();
    }
    else if( MsgSubId == msiExit )  {
      if( UpdateProgress != NULL )  {
        delete UpdateProgress;
        UpdateProgress = NULL;
      }
    }
  }
  else if( MsgId == ID_UpdateThreadAction )  {
    volatile olx_scope_cs cs( TBasicApp::GetCriticalSection());
    if( MsgSubId == msiEnter )  {
      if( ActionProgress == NULL )
        ActionProgress = new TOnProgress;
    }
    else if( MsgSubId == msiExecute && Data != NULL && EsdlInstanceOf(*Data, TOnProgress) )  {
      TOnProgress& pg = *(TOnProgress*)Data;
        if( ActionProgress != NULL )
          *ActionProgress = pg;
      actionEntered = true;
      AOlxThread::Yield();
    }
    else if( MsgSubId == msiExit )  {
       if( ActionProgress != NULL )  {
         delete ActionProgress;
         ActionProgress = NULL;
       }
    }
  }
  else if( MsgId == ID_TIMER )  {
    FTimer->OnTimer.SetEnabled(false);
    if( nui_interface != NULL )  {
      nui_interface->DoProcessing();
    }
    // execute tasks ...
    for( size_t i=0; i < Tasks.Count(); i++ )  {
      if(  (TETime::Now() - Tasks[i].LastCalled) > Tasks[i].Interval )  {
        olxstr tmp(Tasks[i].Task);
        if( !Tasks[i].Repeatable )  {
          Tasks.Delete(i);
          i--;
        }
        else
          Tasks[i].LastCalled = TETime::Now();
        processMacro(tmp, "Scheduled task");
      }
    }
    for (size_t i=0; i < RunWhenVisibleTasks.Count(); i++)
      RunWhenVisibleTasks[i]->Run();
    RunWhenVisibleTasks.DeleteItems().Clear();
    // end tasks ...
    HtmlManager.ProcessPageLoadRequests();
    FTimer->OnTimer.SetEnabled(true);
    if( (FMode & mListen) != 0 && TEFile::Exists(FListenFile) )  {
      static time_t FileMT = TEFile::FileAge(FListenFile);
      time_t FileT = TEFile::FileAge(FListenFile);
      if( FileMT != FileT )  {
        FObjectUnderMouse = NULL;
        processMacro((olxstr("@reap -b -r \'") << FListenFile)+'\'', "OnListen");
        for( size_t i=0; i < FOnListenCmds.Count(); i++ )  {
          if( !processMacro(FOnListenCmds[i], "OnListen") )
            break;
        }
        FileMT = FileT;
        if( !FOnListenCmds.IsEmpty() )  
          Draw = true;
      }
    }
    if( (FMode & mRota) != 0  )  {
      FXApp->GetRender().GetBasis().RotateX(
        FXApp->GetRender().GetBasis().GetRX()+FRotationIncrement*FRotationVector[0]);
      FXApp->GetRender().GetBasis().RotateY(
        FXApp->GetRender().GetBasis().GetRY()+FRotationIncrement*FRotationVector[1]);
      FXApp->GetRender().GetBasis().RotateZ(
        FXApp->GetRender().GetBasis().GetRZ()+FRotationIncrement*FRotationVector[2]);
      FRotationAngle -= olx_abs(FRotationVector.Length()*FRotationIncrement);
      if( FRotationAngle < 0 )  FMode ^= mRota;
      Draw = true;
    }
    if( (FMode & mFade) != 0 )  {
      Draw = true;
      if( FFadeVector[0] == FFadeVector[1] )
      {  FMode ^= mFade;  }//FXApp->GetRender().Ceiling()->Visible(false);  }

      FFadeVector[0] += FFadeVector[2];
      if( FFadeVector[2] > 0 )  {
        if( FFadeVector[0] > FFadeVector[1] )  {
          FFadeVector[0] = FFadeVector[1];
          FMode ^= mFade;
        }
      }
      else  {
        if( FFadeVector[0] < FFadeVector[1] )  {
          FFadeVector[0] = FFadeVector[1];
          FMode ^= mFade;       
        }
      }
      if( (FMode & mFade) != 0 )  {
        TGlOption glO;
        glO = FXApp->GetRender().Ceiling()->LT();  glO[3] = FFadeVector[0];
        FXApp->GetRender().Ceiling()->LT(glO);

        glO = FXApp->GetRender().Ceiling()->RT();  glO[3] = FFadeVector[0];
        FXApp->GetRender().Ceiling()->RT(glO);

        glO = FXApp->GetRender().Ceiling()->LB();  glO[3] = FFadeVector[0];
        FXApp->GetRender().Ceiling()->LB(glO);

        glO = FXApp->GetRender().Ceiling()->RB();  glO[3] = FFadeVector[0];
        FXApp->GetRender().Ceiling()->RB(glO);
        Draw = true;
      }
    }
    if( FXApp->GetFader().IsVisible() )  {
      if( !FXApp->GetFader().Increment() )
         FXApp->GetFader().SetVisible(false);
      Draw = true;
    }
    if( MouseMoveTimeElapsed < 2500 )
      MouseMoveTimeElapsed += FTimer->GetInterval();
    if( MouseMoveTimeElapsed > 500 && MouseMoveTimeElapsed < 5000 )  {
      AquireTooltipValue();
      if( !_UseGlTooltip )
        FGlCanvas->SetToolTip(Tooltip.u_str());
      else if( GlTooltip != NULL )  {
        if( Tooltip.IsEmpty() )  {
          if( GlTooltip->IsVisible() )  {
            GlTooltip->SetVisible(false);
            Draw = true;
          }
        }
        else  {
          GlTooltip->Clear();
          GlTooltip->PostText(Tooltip);
          GlTooltip->Fit();
          int x = MousePositionX-GlTooltip->GetWidth()/2,
            y = MousePositionY-GlTooltip->GetHeight()-4;
          if( x < 0 )  x = 0;
          if( (size_t)(x + GlTooltip->GetWidth()) > (size_t)FXApp->GetRender().GetWidth() )
            x = FXApp->GetRender().GetWidth() - GlTooltip->GetWidth();
          if( y < 0 )
            y  = 0;
          GlTooltip->SetLeft(x); // put it off the mouse
          GlTooltip->SetTop(y);
          GlTooltip->SetZ(FXApp->GetRender().GetMaxRasterZ());
          GlTooltip->SetVisible(true);
          Draw = true;
        }
      }
      if( DrawSceneTimer > 0 && !Draw )  {
        if( DrawSceneTimer < FTimer->GetInterval() )  
          TimePerFrame = FXApp->Draw();
        else
          DrawSceneTimer -= FTimer->GetInterval();
      }
      MouseMoveTimeElapsed = 5000;
    }
    if( Draw )  {
      TimePerFrame = FXApp->Draw();
    }
    // here it cannot be done with scope_cs - GTK would freese the main loop...
    TBasicApp::EnterCriticalSection();
    if( _UpdateThread != NULL && _UpdateThread->GetUpdateSize() != 0 )  {
      TBasicApp::LeaveCriticalSection();
      if( wxApp::IsMainLoopRunning() )  {
        FTimer->OnTimer.SetEnabled(false);
        DoUpdateFiles();
        FTimer->OnTimer.SetEnabled(true);
      }
    }
    else
      TBasicApp::LeaveCriticalSection();
  // deal with updates
    if( wxIsMainThread() )  {
      static bool UpdateExecuted = false;
      volatile olx_scope_cs cs( TBasicApp::GetCriticalSection());
      if( actionEntered && ActionProgress != NULL )  {
        StatusBar->SetStatusText( (olxstr("Processing ") <<
          ActionProgress->GetAction()).u_str() );
        actionEntered = false;
      }
      if( downloadEntered && UpdateProgress != NULL )  {
        downloadEntered = false;
        UpdateExecuted = true;
        StatusBar->SetStatusText( 
          (olxstr("Downloading ") << UpdateProgress->GetAction() << ' ' << 
          olxstr::FormatFloat(2, UpdateProgress->GetPos()*100/
            (UpdateProgress->GetMax()+1)) << '%').u_str()
        );
      }
      if( UpdateExecuted && _UpdateThread == NULL )  {
        StatusBar->SetStatusText(TBasicApp::GetBaseDir().u_str());
        UpdateExecuted = false;
      }
    }
  }
  else if( MsgId == ID_XOBJECTSDESTROY )  {
    FObjectUnderMouse = NULL;
    if( Modes->GetCurrent() != NULL )
      Modes->GetCurrent()->OnGraphicsDestroy();
  }
  else if( MsgId == ID_FileLoad )  {
  }
  else if( MsgId == ID_FileClose )  {
    if( MsgSubId == msiExit )  {
      UpdateRecentFile(EmptyString());
      UpdateInfoBox();
    }
  }
  else if( MsgId == ID_CMDLINECHAR )  {
    if( Data != NULL && EsdlInstanceOf(*Data, TKeyEvent) )
      this->OnChar(((TKeyEvent*)Data)->GetEvent());
  }
  else if( MsgId == ID_CMDLINEKEYDOWN )  {
    if( Data != NULL && EsdlInstanceOf(*Data, TKeyEvent) )
      this->OnKeyDown(((TKeyEvent*)Data)->GetEvent());
  }
  else if( MsgId == ID_INFO || MsgId == ID_WARNING || MsgId == ID_ERROR ||
           MsgId == ID_EXCEPTION && (MsgSubId == msiEnter))
  {
    if( Data != NULL )  {
      TGlMaterial *glm = NULL;
      if( MsgId == ID_INFO )           glm = &InfoFontColor;
      else if( MsgId == ID_WARNING )   glm = &WarningFontColor;
      else if( MsgId == ID_ERROR )     glm = &ErrorFontColor;
      else if( MsgId == ID_EXCEPTION ) glm = &ExceptionFontColor;
      if( !( (FMode&mSilent) != 0 &&  (MsgId == ID_INFO || MsgId == ID_WARNING))
            || (MsgId == ID_ERROR || MsgId == ID_EXCEPTION) )  {
        FGlConsole->OnPost.SetEnabled(false); // the proporgation will happen after we return false
        FGlConsole->PrintText(Data->ToString(), glm, true);
        FGlConsole->PrintText(EmptyString());
        FGlConsole->OnPost.SetEnabled(true);
        TimePerFrame = FXApp->Draw();
      }
      FGlConsole->SetSkipPosting(true);
      res = false;  // propargate to other streams, logs in particular
    }
  }
  else if( MsgId == ID_ONLINK )  {
    if( Data != NULL && EsdlInstanceOf(*Data, olxstr) )  {
      TStrList Toks(*(olxstr*)Data, ">>");
      //GetHtml()->LockPageLoad();
      /* the page, if requested, will beloaded on time event. The timer is disabled
      in case if a modal window appears and the timer event can be called */
      FTimer->OnTimer.SetEnabled(false);
      for( size_t i=0; i < Toks.Count(); i++ )  {
        if( !processMacro(olxstr::DeleteSequencesOf<char>(Toks[i], ' '), "OnLink") )
          break;
      }
      TimePerFrame = FXApp->Draw();
      // enabling the timer back
      // retrun fucus to the main window, but let typing in the comboboxes
      if( Sender != NULL )  {
        if( ((olxstr*)Data)->IsEmpty() )
          ;
        else if( EsdlInstanceOf(*Sender, TComboBox) && !((TComboBox*)Sender)->IsReadOnly() )
          ;
        else if( EsdlInstanceOf(*Sender, TTreeView) )
          ;
        else if( EsdlInstanceOf(*Sender, TTextEdit) )
          ;
        else if( EsdlInstanceOf(*Sender, TSpinCtrl) )
          ;
        else if( EsdlInstanceOf(*Sender, TDateCtrl) )
          ;
        else {
          if (CmdLineVisible)
            FCmdLine->SetFocus();
          else
            FGlCanvas->SetFocus();
        }
      }
      else {
          if (CmdLineVisible)
            FCmdLine->SetFocus();
          else
            FGlCanvas->SetFocus();
      }
      FTimer->OnTimer.SetEnabled(true);
    }
  }
  else if( MsgId == ID_HTMLKEY )  {
    if (CmdLineVisible)
      FCmdLine->SetFocus();
    else
      FGlCanvas->SetFocus();
    OnChar(((TKeyEvent*)Data)->GetEvent());
  }
  else if( MsgId == ID_TEXTPOST )  {
    if( Data != NULL )  {
      FGlConsole->SetSkipPosting(true);
      TBasicApp::NewLogEntry() << olxstr(Data->ToString());
      FGlConsole->SetSkipPosting(false);
      if( ActiveLogFile != NULL )
        ActiveLogFile->Writeln(Data->ToString());
    }
  }
  else if( MsgId == ID_COMMAND )  {
    olxstr tmp;
    if( CmdLineVisible && EsdlInstanceOf(*Sender, TCmdLine) )
      tmp = FCmdLine->GetCommand();
    else if( EsdlInstanceOf(*Sender, TGlConsole) )
      tmp = FGlConsole->GetCommand();
    if( !tmp.IsEmpty() )  {
      if( _ProcessManager->GetRedirected() != NULL )  {
        _ProcessManager->GetRedirected()->Write(tmp);
        _ProcessManager->GetRedirected()->Writenl();
        TimePerFrame = FXApp->Draw();
      }
      else  {
        FHelpWindow->SetVisible(false);
        olxstr FullCmd(tmp);
        if (tmp.StartsFrom('!')) {
          if (CmdLineVisible && EsdlInstanceOf(*Sender, TCmdLine))
            tmp = FCmdLine->GetLastCommand(tmp.SubStringFrom(1));
          else if (EsdlInstanceOf(*Sender, TGlConsole))
            tmp = FGlConsole->GetLastCommand(tmp.SubStringFrom(1));
          if (!tmp.IsEmpty()) {
            FullCmd = tmp;
            TBasicApp::NewLogEntry() << FullCmd;
          }
        }
        processMacro(FullCmd, "Console");
      }
      if( CmdLineVisible && EsdlInstanceOf(*Sender, TCmdLine) )
        FCmdLine->SetCommand(EmptyString());
      else
        FGlConsole->SetCommand(EmptyString());
    }
  }
  else if( MsgId == ID_DELINS )  {
    if( Data != NULL && EsdlInstanceOf(*Data, olxstr) )  {
      if( ((olxstr*)Data)->Equalsi("OMIT") )  {
        BadReflectionsTable(false);
        processMacro("html.update");
      }
    }
  }
  else if( MsgId == ID_ADDINS )  {
    if( Data != NULL && EsdlInstanceOf(*Data, olxstr) )  {
      if( ((olxstr*)Data)->Equalsi("OMIT") )  {
        BadReflectionsTable(false);
        processMacro("html.update");
      }
    }
  }
  else if (MsgId == ID_BadReflectionSet) {
    if (MsgSubId == msiExit)
      BadReflectionsTable(false);
  }
  else if (MsgId == ID_UPDATE_GUI) {
    processMacro("html.update");
  }
  else if (MsgId == ID_CellChanged) {
    if (Data != NULL && EsdlInstanceOf(*Data, THklFile) ) {
       const THklFile &hf = *dynamic_cast<const THklFile*>(Data);
      RunWhenVisibleTasks.Add(
        new CellChangeTask(FXApp->XFile().GetRM().GetHKLSource(),
          hf.GetCell(), hf.GetCellEsd()));
    }
  }
  return res;
}
//..............................................................................
void TMainForm::PreviewHelp(const olxstr& Cmd)  {
  if( !HelpWindowVisible )  return;
  if( !Cmd.IsEmpty() )  {
    TPtrList<TEMacro> macros;
    Macros.FindSimilar(Cmd, macros);
    if( !macros.IsEmpty() )  {
      FHelpWindow->Clear();
      FHelpWindow->SetVisible(HelpWindowVisible);
      FGlConsole->ShowBuffer(!HelpWindowVisible);
      FHelpWindow->SetTop(
        InfoWindowVisible ? FInfoBox->GetTop() + FInfoBox->GetHeight() + 5 : 1);
      FHelpWindow->SetMaxStringLength((uint16_t)(
        FHelpWindow->GetFont().MaxTextLength(FXApp->GetRender().GetWidth())));
      FHelpWindow->SetZ(FXApp->GetRender().CalcRasterZ(0.1));
      for( size_t i=0; i < macros.Count(); i++ )  {
        FHelpWindow->PostText(macros[i]->GetName(), &HelpFontColorCmd);
        if( !macros[i]->GetDescription().IsEmpty() )  {
          FHelpWindow->PostText(macros[i]->GetDescription(), &HelpFontColorTxt);
          //Cat = Item->FindItem("category");
          //if( Cat != NULL  )  {
          //  olxstr Categories;
          //  for( size_t j=0; j < Cat->ItemCount(); j++ )  {
          //    Categories << Cat->GetItem(j).GetName();
          //    if( (j+1) < Cat->ItemCount() )  Categories << ", ";
          //  }
          //  if( !Categories.IsEmpty() )  {
          //    Categories.Insert("\t", 0);
          //    FHelpWindow->PostText("\tCategory", &HelpFontColorCmd);
          //    FHelpWindow->PostText(Categories, &HelpFontColorTxt);
          //  }
          //}
        }
      }
      FHelpWindow->Fit();
    }
    else  {
      FHelpWindow->SetVisible(false);
      FGlConsole->ShowBuffer(true);
    }
  }
  else  {
    FHelpWindow->SetVisible(false);
    FGlConsole->ShowBuffer(true);
  }
}
//..............................................................................
bool TMainForm::ImportFrag(const olxstr& line)  {
  olxstr trimmed_content = line;
  trimmed_content.Trim(' ').Trim('\n').Trim('\r');
  if( !trimmed_content.StartsFromi("FRAG") || !trimmed_content.EndsWithi("FEND") )
    return false;
  TStrList lines(trimmed_content, '\n');
  lines.Delete(lines.Count()-1);
  lines.Delete(0);
  for( size_t i=0; i < lines.Count(); i++ )  {
    TStrList toks(lines[i].Trim('\r'), ' ');
    if( toks.Count() != 5 )  {
      lines[i].SetLength(0);
      continue;
    }
    toks.Delete(1);
    lines[i] = toks.Text(' ');
  }
  try {
    TXyz xyz;
    xyz.LoadFromStrings(lines);
    if( xyz.GetAsymmUnit().AtomCount() == 0 )
      return false;
    processMacro("mode fit -a=6");
    TXAtomPList xatoms;
    TXBondPList xbonds;
    LabelCorrector lc(FXApp->XFile().GetAsymmUnit());
    FXApp->AdoptAtoms(xyz.GetAsymmUnit(), xatoms, xbonds);
    for (size_t i=0; i < xatoms.Count(); i++) {
      FXApp->XFile().GetRM().Vars.FixParam(
        xatoms[i]->CAtom(), catom_var_name_Sof);
      lc.Correct(xatoms[i]->CAtom());
    }
    FXApp->CenterView(true);
    AMode *md = Modes->GetCurrent();
    if( md != NULL )  {
      md->AddAtoms(xatoms);
      for( size_t i=0; i < xbonds.Count(); i++ )
        FXApp->GetRender().Select(*xbonds[i], true);
    }
    return true;
  }
  catch(...)  {  return false;  }
}
//..............................................................................
void TMainForm::OnChar(wxKeyEvent& m)  {
  m.Skip(false);
  short Fl=0;
  olxstr Cmd, FullCmd;
  if( m.m_altDown )      Fl |= sssAlt;
  if( m.m_shiftDown )    Fl |= sssShift;
  if( m.m_controlDown )  Fl |= sssCtrl;
  // Alt + Up,Down,Left, Right - rotation, +Shift - speed
  if( ((Fl & sssShift)) || (Fl & sssAlt) )  {
    int inc = 3;
    double zoom_inc = 0.01;
    if( (Fl & sssShift) )  {
      inc *= 3;
      zoom_inc *= 3;
    }
    if( m.m_keyCode == WXK_UP )  {
      FXApp->GetRender().RotateX(FXApp->GetRender().GetBasis().GetRX()+inc);
      TimePerFrame = FXApp->Draw();
      return;
    }
    if( m.m_keyCode == WXK_DOWN )  {
      FXApp->GetRender().RotateX(FXApp->GetRender().GetBasis().GetRX()-inc);
      TimePerFrame = FXApp->Draw();
      return;
    }
    if( m.m_keyCode == WXK_LEFT )  {
      FXApp->GetRender().RotateY(FXApp->GetRender().GetBasis().GetRY()-inc);
      TimePerFrame = FXApp->Draw();
      return;
    }
    if( m.m_keyCode == WXK_RIGHT )  {
      FXApp->GetRender().RotateY(FXApp->GetRender().GetBasis().GetRY()+inc);
      TimePerFrame = FXApp->Draw();
      return;
    }
    if( m.m_keyCode == WXK_END )  {
      if( FXApp->GetRender().GetZoom()+zoom_inc < 100 )  {
        FXApp->GetRender().SetZoom(FXApp->GetRender().GetZoom()+zoom_inc);
        TimePerFrame = FXApp->Draw();
        return;
      }
    }
    if( m.m_keyCode == WXK_HOME )  {
      double z = FXApp->GetRender().GetZoom()-zoom_inc;
      if( z <= 0 ) z = 0.001;
      FXApp->GetRender().SetZoom(z);
      TimePerFrame = FXApp->Draw();
      return;
    }
  }
  // Ctrl + Up, Down - browse solutions
  if( (Fl & sssCtrl) != 0  )  {

    if( m.m_keyCode == WXK_UP && ((FMode&mSolve) == mSolve) )  {
      ChangeSolution(CurrentSolution - 1);
      return;
    }
    if( m.m_keyCode == WXK_DOWN  && ((FMode&mSolve) == mSolve) )  {
      ChangeSolution(CurrentSolution + 1);
      return;
    }
  }
  if( (Fl&sssCtrl) && m.GetKeyCode() == 'c'-'a'+1 )  {  // Ctrl+C
    if( _ProcessManager->GetRedirected() != NULL )  {
      if( _ProcessManager->GetRedirected()->Terminate() )
        TBasicApp::NewLogEntry(logInfo) << "Process has been successfully terminated...";
      else
        TBasicApp::NewLogEntry(logInfo) << "Could not terminate the process...";
      TimePerFrame = FXApp->Draw();
    }
    return;
  }
  if( m.GetKeyCode() == WXK_RETURN )  {
    if( FMode & mSolve )  {
      FMode ^= mSolve;
      TBasicApp::NewLogEntry(logInfo) << "Model is set to current solution";
    }
  }
  if( m.GetKeyCode() == WXK_ESCAPE )  {  // escape
    if( Modes->GetCurrent() != NULL )  {
      if( Modes->GetCurrent()->OnKey( m.GetKeyCode(), Fl) )
        return;
      else
        processMacro("mode off");
    }
    processMacro("sel -u");
    TimePerFrame = FXApp->Draw();
  }
  if( m.GetKeyCode() == WXK_TAB )  {  // tab
    if (CmdLineVisible)
      Cmd = FCmdLine->GetCommand();
    else
      Cmd = FGlConsole->GetCommand();
    size_t spi = Cmd.LastIndexOf(' ');
    if( spi != InvalidIndex )  {
      FullCmd = ExpandCommand(Cmd.SubStringFrom(spi+1), true);
      if( FullCmd != Cmd.SubStringFrom(spi+1) )
        FullCmd = Cmd.SubStringTo(spi+1) << FullCmd;
      else
        FullCmd.SetLength(0);
    }
    else
      FullCmd = ExpandCommand(Cmd, false);
    if( !FullCmd.IsEmpty() && (FullCmd != Cmd) ) {
      if (CmdLineVisible)
        FCmdLine->SetCommand(FullCmd);
      else {
        FGlConsole->SetCommand(FullCmd);
        TimePerFrame = FXApp->Draw();
      }
    }
    else
      TimePerFrame = FXApp->Draw();
    return;
  }

  if (!CmdLineVisible) {
    if( FGlConsole->ProcessKey(m.GetKeyCode(), Fl) )  {
      PreviewHelp(FGlConsole->GetCommand());
      TimePerFrame = FXApp->Draw();
      return;
    }
  }
  else {
    if( FCmdLine->ProcessKey(m) )  {
      PreviewHelp(FCmdLine->GetCommand());
      TimePerFrame = FXApp->Draw();
      return;
    }
    else {
      if( !FGlConsole->ProcessKey(m.GetKeyCode(), Fl))
        m.Skip();
      else
        TimePerFrame = FXApp->Draw();
    }
  }

  if( _ProcessManager->GetRedirected() != NULL )  {
    FHelpWindow->SetVisible(false);
    FGlConsole->ShowBuffer(true);
    TimePerFrame = FXApp->Draw();
    return;
  }

  if (m.GetKeyCode() == WXK_RETURN)
    TimePerFrame = FXApp->Draw();
  else {
    if (!CmdLineVisible)
      TimePerFrame = FXApp->Draw();
  }
  if (FindFocus() != FCmdLine)
    m.Skip();
}
//..............................................................................
void TMainForm::OnKeyUp(wxKeyEvent& m)  {
  m.Skip();
}
//..............................................................................
void TMainForm::OnKeyDown(wxKeyEvent& m)  {
  m.Skip(false);
  wxWindow* wxw = FindFocus();
  if (wxw != FGlCanvas && wxw != FCmdLine) {
    if (HtmlManager.main != NULL) {
      THtml* htw = HtmlManager.main;
      if ((wxw != NULL && EsdlInstanceOf(*wxw, THtml)))
        htw = (THtml*)wxw;
      else if (wxw != NULL && wxw->GetParent() != NULL &&
        EsdlInstanceOf(*wxw->GetParent(), THtml) )
      {
        htw = (THtml*)wxw->GetParent();
      }
      htw->OnKeyDown(m);
    }
    return;
  }
  short Fl = 0;
  if( m.m_keyCode == WXK_CONTROL ||
      m.m_keyCode == WXK_MENU ||
      m.m_keyCode == WXK_SHIFT ||
      m.m_keyCode == WXK_ALT)
  {
    m.Skip();
    return;
  }
  if (CmdLineVisible && wxw == FGlCanvas) {
    if (m.m_keyCode != WXK_BACK && m.m_keyCode != WXK_DELETE) {
      FCmdLine->EmulateKeyPress(m);
      return;
    }
  }
  if( m.m_altDown )      Fl |= sssAlt;
  if( m.m_shiftDown )    Fl |= sssShift;
  if( m.m_controlDown )  Fl |= sssCtrl;
  // process built-ins first
  if( m.GetModifiers() == wxMOD_CMD )  {
  // paste, Cmd+V, Ctrl+V
    if( m.GetKeyCode() == 'V' )  {
      // avoid duplication
      if (CmdLineVisible && FindFocus() != FCmdLine)
        return;
      olxstr content;
      if( wxTheClipboard->Open() )  {
        if (wxTheClipboard->IsSupported(wxDF_TEXT) )  {
          wxTextDataObject data;
          wxTheClipboard->GetData(data);
          content = data.GetText();
        }
        wxTheClipboard->Close();
      }
      olxstr cmdl;
      if (CmdLineVisible)
        cmdl = FCmdLine->GetCommand();
      else
        cmdl = FGlConsole->GetCommand();
      if( !ImportFrag(content) )  {
        olxstr trimmed_content = content;
        trimmed_content.Trim(' ').Trim('\n').Trim('\r');
        size_t ip;
        if (CmdLineVisible) {
          ip = FCmdLine->GetInsertionPoint() -
            FCmdLine->GetPromptStr().Length();
        }
        else
          ip = FGlConsole->GetCmdInsertPosition();
        if( ip >= cmdl.Length() )
          cmdl << content;
        else
          cmdl.Insert(content, ip);
        if (CmdLineVisible)
          FCmdLine->SetCommand(cmdl);
        else
          FGlConsole->SetCommand(cmdl);
      }
      TimePerFrame = FXApp->Draw();
      return;
    }
    //undo, Cmd+Z, Ctrl+Z
    else if( m.GetKeyCode() == 'Z' )  {
      processMacro("undo");
      TimePerFrame = FXApp->Draw();
      return;
    }
  }
  if( !AccShortcuts.ValueExists(Fl<<16 | m.m_keyCode) )  {
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
  olxstr Cmd = AccShortcuts.GetValue(Fl<<16 | m.m_keyCode);
  if( !Cmd.IsEmpty() )  {
    processMacro(Cmd, __OlxSrcInfo);
    TimePerFrame = FXApp->Draw();
    return;
  }
  m.Skip();
}
//..............................................................................
void TMainForm::OnNavigation(wxNavigationKeyEvent& event)  {
  if (FindFocus() != FGlCanvas) {
    if (HtmlManager.main != NULL) {
      THtml* htw = HtmlManager.main;
      wxWindow* wxw = FindFocus();
      if( (wxw != NULL && EsdlInstanceOf(*wxw, THtml)) )
        htw = (THtml*)wxw;
      else if(wxw != NULL && wxw->GetParent() != NULL &&
              EsdlInstanceOf(*wxw->GetParent(), THtml) )
      {
        htw = (THtml*)wxw->GetParent();
      }
      htw->OnNavigation(event);
    }
    return;
  }
  event.Skip();
}
//..............................................................................
void TMainForm::OnMove(wxMoveEvent& evt) {
  if( FXApp == NULL || FGlConsole == NULL || FInfoBox == NULL ||
      !StartupInitialised )
  {
    return;
  }
  wxPoint p = FGlCanvas->GetScreenPosition();
  FXApp->GetRender().SetAbsoluteTop(p.y);
}
//..............................................................................
void TMainForm::OnSize(wxSizeEvent& event)  {
  wxFrame::OnSize(event);
  if( SkipSizing )  return;
  if( FXApp == NULL || FGlConsole == NULL || FInfoBox == NULL ||
      !StartupInitialised )
  {
    return;
  }
  OnResize();
}
//..............................................................................
void TMainForm::OnResize()  {
  int w=0, h=0, l=0;
  int dheight = InfoWindowVisible ? FInfoBox->GetHeight() : 1;
  GetClientSize(&w, &h);

  FInfoBox->SetTop(1);
  if( FHtmlMinimized )  {
    if( FHtmlOnLeft )  {
      HtmlManager.main->SetSize(0, 0, 10, h);
      l = 10;
      w = w - l;
    }
    else  {
      HtmlManager.main->SetSize(w-10, 0, 10, h);
      w = w-10;
    }
  }
  else  {
    HtmlManager.main->Freeze();
    if( FHtmlOnLeft )  {
      const int cw = FHtmlWidthFixed ? (int)FHtmlPanelWidth
        : (int)(w*FHtmlPanelWidth);
      HtmlManager.main->SetClientSize(cw, -1);
      HtmlManager.main->SetSize(-1, h);
      HtmlManager.main->Move(0, 0);
      l = HtmlManager.main->GetSize().GetWidth();
      w -= l;
    }
    else  {
      const int cw = FHtmlWidthFixed ? (int)FHtmlPanelWidth
        : (int)(w*FHtmlPanelWidth);
      HtmlManager.main->SetClientSize(cw, -1);
      HtmlManager.main->SetSize(-1, h);
      HtmlManager.main->Move((int)(w-HtmlManager.main->GetSize().GetWidth()), 0);
      w -= HtmlManager.main->GetSize().GetWidth();
    }
    HtmlManager.main->Refresh();
    HtmlManager.main->Update();
    HtmlManager.main->Thaw();
  }
  if( CmdLineVisible )  {
    FCmdLine->WI.SetWidth(w);
    FCmdLine->WI.SetLeft(l);
    FCmdLine->WI.SetTop(h - FCmdLine->WI.GetHeight());
  }
  if( w <= 0 )  w = 5;
  if( h <= 0 )  h = 5;
  FGlCanvas->SetSize(l, 0, w, h - (CmdLineVisible ? FCmdLine->WI.GetHeight() : 0));
  FGlCanvas->GetClientSize(&w, &h);
  FXApp->GetRender().Resize(0, 0, w, h, 1);
  FGlConsole->Resize(0, dheight, w, h - dheight);
  FInfoBox->SetWidth(w);
  FInfoBox->SetLeft(0);
}
//..............................................................................
olxstr TMainForm::ExpandCommand(const olxstr &Cmd, bool inc_files)  {
  if( Cmd.IsEmpty() )  return Cmd;
  olxstr FullCmd(Cmd.ToLowerCase());
  TStrList all_cmds;
  if( inc_files )  {
    TStrList names;
    olxstr path = TEFile::ExpandRelativePath(Cmd, TEFile::CurrentDir());
    if( !path.IsEmpty() && TEFile::IsAbsolutePath(path) )  {
      size_t lsi = path.LastIndexOf(TEFile::GetPathDelimeter());
      if( lsi != InvalidIndex )  {
        olxstr dir_name = path.SubStringTo(lsi+1);
        if( TEFile::Exists(dir_name) )  {
          TEFile::ListDir(dir_name, names, olxstr(path.SubStringFrom(lsi+1)) <<
            '*', sefReadWrite);
          for( size_t i=0; i < names.Count(); i++ )
            all_cmds.Add(dir_name +names[i]);
        }
      }
    }
    else
      TEFile::ListCurrentDir(all_cmds, olxstr(Cmd) << '*', sefReadWrite);
  }
  if( !Cmd.IsEmpty() )
    Macros.FindSimilarNames(Cmd, all_cmds);
  TBasicLibraryPList libs;
  GetLibrary().FindSimilarLibraries(Cmd, libs);
  TBasicFunctionPList bins;  // builins
  GetLibrary().FindSimilarMacros(Cmd, bins);
  GetLibrary().FindSimilarFunctions(Cmd, bins);
  for( size_t i=0; i < bins.Count(); i++ )
    all_cmds.Add(bins[i]->GetQualifiedName());
  for( size_t i=0; i < libs.Count(); i++ )
    all_cmds.Add(libs[i]->GetQualifiedName());
  if( all_cmds.Count() > 1 )  {
    if( FHelpWindow->IsVisible() )  // console buffer is hidden then...
      FHelpWindow->Clear();
    olxstr cmn_str = all_cmds[0].ToLowerCase();
    olxstr line(all_cmds[0], 80);
    for( size_t i=1; i < all_cmds.Count(); i++ )  {
      cmn_str = all_cmds[i].ToLowerCase().CommonString(cmn_str);
      if( line.Length() + all_cmds[i].Length() > 79 )  {  // expects no names longer that 79!
        if( FHelpWindow->IsVisible() )
          FHelpWindow->PostText(line);
        else
          FXApp->NewLogEntry() << line;
        line.SetLength(0);
      }
      else
        line << ' ' << all_cmds[i];
    }
    FullCmd = cmn_str;
    if( !line.IsEmpty() )  {
      if( FHelpWindow->IsVisible() )
        FHelpWindow->PostText(line);
      else
        FXApp->NewLogEntry() << line;
    }
    FHelpWindow->Fit();
  }
  else if( all_cmds.Count() == 1 )
    return all_cmds[0];
  return FullCmd;
}
//..............................................................................
void TMainForm::PostCmdHelp(const olxstr &Cmd, bool Full)  {
  ABasicFunction *MF = FXApp->GetLibrary().FindMacro(Cmd);
  if( MF != NULL )  {
    FGlConsole->PrintText( olxstr("Built in macro ") << MF->GetName());
    FGlConsole->PrintText(olxstr(" Signature: ") << MF->GetSignature());
    FGlConsole->PrintText(olxstr(" Description: ") << MF->GetDescription());
    if( MF->GetOptions().Count() != 0 )  {
      FGlConsole->PrintText(" Switches: ");
      for( size_t i=0; i < MF->GetOptions().Count(); i++ )  {
        FGlConsole->PrintText( olxstr("   ") << MF->GetOptions().GetKey(i) << " - "
          << MF->GetOptions().GetObject(i) );
      }
    }
  }
  MF = FXApp->GetLibrary().FindFunction(Cmd);
  if( MF != NULL )  {
    FGlConsole->PrintText( olxstr("Built in function ") << MF->GetName());
    FGlConsole->PrintText(olxstr(" Signature: ") << MF->GetSignature());
    FGlConsole->PrintText(olxstr(" Description: ") << MF->GetDescription());
  }
  
  if( !Cmd.IsEmpty() )  {
    TEMacro* macro = Macros.FindMacro(Cmd);
    if( macro != NULL )  {
      FGlConsole->PrintText(macro->GetDescription(), &HelpFontColorTxt);
    }
  }
}
//..............................................................................
void TMainForm::SaveSettings(const olxstr &FN)  {
  TDataFile DF;
  TDataItem* I = &DF.Root().AddItem("Folders");
  I->AddField("Styles", TEFile::CreateRelativePath(StylesDir));
  I->AddField("Scenes", TEFile::CreateRelativePath(ScenesDir));
  I->AddField("Current", TEFile::CreateRelativePath(XLibMacros::CurrentDir));

  I = &DF.Root().AddItem("HTML");
  I->AddField("Minimized", FHtmlMinimized);
  I->AddField("OnLeft", FHtmlOnLeft);
  if( !FHtmlWidthFixed )
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
  if( !IsIconized() )  {  // otherwise left and top are -32000 causing all sort of problems...
    I = &DF.Root().AddItem("Window");
    if( IsMaximized() )  I->AddField("Maximized", true);
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
  I->AddField("WhiteOn", (FXApp->GetRender().LightModel.GetClearColor().GetRGB() == 0xffffffff));
  I->AddField("Gradient", FXApp->GetRender().Background()->IsVisible());
  I->AddField("GradientPicture", TEFile::CreateRelativePath(GradientPicture));
  I->AddField("language", Dictionary.GetCurrentLanguage());
  I->AddField("ExtraZoom", FXApp->GetExtraZoom());
  I->AddField("GlTooltip", _UseGlTooltip);
  I->AddField("ThreadCount", FXApp->GetMaxThreadCount());

  I = &DF.Root().AddItem("Recent_files");
  for( size_t i=0; i < olx_min(FRecentFilesToShow, FRecentFiles.Count()); i++ )
    I->AddField(olxstr("file") << i, TEFile::CreateRelativePath(FRecentFiles[i]));

  I = &DF.Root().AddItem("Stored_params");
  for( size_t i=0; i < StoredParams.Count(); i++ )  {
    TDataItem& it = I->AddItem(StoredParams.GetKey(i));
    it.AddField("value", StoredParams.GetObject(i));
  }

  SaveScene(DF.Root().AddItem("Scene"), FXApp->GetRender().LightModel);
  FXApp->GetRender().GetStyles().ToDataItem(DF.Root().AddItem("Styles"));
  DF.SaveToXLFile(FN+".tmp");
  TEFile::Rename(FN+".tmp", FN);
}
//..............................................................................
void TMainForm::LoadSettings(const olxstr &FN)  {
  if( !TEFile::Exists(FN) ) return;

  TDataFile DF;
  TStrList Log;
  olxstr Tmp;
  DF.LoadFromXLFile(FN, &Log);

  TDataItem *I = DF.Root().FindItem("Folders");
  if( I == NULL )
    return;
  StylesDir = TEFile::ExpandRelativePath(I->GetFieldValue("Styles"));
    processFunction(StylesDir);
  ScenesDir = TEFile::ExpandRelativePath(I->GetFieldValue("Scenes"));
    processFunction(ScenesDir);
  XLibMacros::CurrentDir = TEFile::ExpandRelativePath(I->GetFieldValue("Current"));
    processFunction(XLibMacros::CurrentDir);

  I = DF.Root().FindItem("HTML");
  if( I != NULL )  {
    Tmp = I->GetFieldValue("Minimized");
    FHtmlMinimized = Tmp.IsEmpty() ? false : Tmp.ToBool();
    Tmp = I->GetFieldValue("OnLeft");
    FHtmlOnLeft = Tmp.IsEmpty() ? true : Tmp.ToBool();

    Tmp = I->GetFieldValue("Width");
    if( !Tmp.IsEmpty() )  {
      FHtmlWidthFixed = !Tmp.EndsWith('%');
      FHtmlPanelWidth = ((!FHtmlWidthFixed) ? Tmp.SubStringTo(Tmp.Length()-1).ToDouble()
        : Tmp.ToDouble());
      if( !FHtmlWidthFixed && FHtmlPanelWidth >= 0.5 )
        FHtmlPanelWidth = 0.25;
    }
    else
      FHtmlPanelWidth = 0.25;

    Tmp = I->GetFieldValue("Tooltips", EmptyString());
    if( !Tmp.IsEmpty() )
      HtmlManager.main->SetShowTooltips(Tmp.ToBool());

    Tmp = I->GetFieldValue("Borders");
    if( !Tmp.IsEmpty() && Tmp.IsNumber() )
      HtmlManager.main->SetBorders(Tmp.ToInt());

    olxstr nf(I->GetFieldValue("NormalFont", EmptyString()));
    olxstr ff(I->GetFieldValue("FixedFont", EmptyString()));
    HtmlManager.main->SetFonts(nf, ff);
  }

  SkipSizing = true;
  I = DF.Root().FindItem("Window");
  if( I != NULL )  {
    if( I->GetFieldValue("Maximized", FalseString()).ToBool() )  {
      int l = I->GetFieldValue("X", "0").ToInt(), 
          t = I->GetFieldValue("Y", "0").ToInt();
        Move(l, t);
      Maximize();
    }
    else  {
      int w = I->GetFieldValue("Width", "100").ToInt(), 
        h = I->GetFieldValue("Height", "100").ToInt(), 
        l = I->GetFieldValue("X", "0").ToInt(), 
        t = I->GetFieldValue("Y", "0").ToInt();
      SetSize(l, t, w, h);
    }
  }
  else
    Maximize();
  SkipSizing = false;
  
  I = DF.Root().FindItem("Windows");
  if( I != NULL )  {
    HelpWindowVisible = I->GetFieldValue("Help", TrueString()).ToBool();
    InfoWindowVisible = I->GetFieldValue("Info", TrueString()).ToBool();
    CmdLineVisible = I->GetFieldValue("CmdLine", FalseString()).ToBool();
  }
  TEFile::ChangeDir(XLibMacros::CurrentDir);

  I = DF.Root().FindItem("Recent_files");
  if( I )  {
    MenuFile->AppendSeparator();
    int i=0;
    TStrList uniqNames;
    olxstr T = TEFile::ExpandRelativePath(I->GetFieldValue( olxstr("file") << i));
    while( !T.IsEmpty() )  {
      if( T.EndsWithi(".ins") || T.EndsWithi(".res") )  {
        T = TEFile::ChangeFileExt(T, EmptyString());
      }
      TEFile::OSPathI(T);
      if( uniqNames.IndexOf(T) == InvalidIndex )
        uniqNames.Add(T);
      i++;
      T = I->GetFieldValue(olxstr("file") << i);
    }
    for( size_t j=0; j < olx_min(uniqNames.Count(), FRecentFilesToShow); j++ )  {
      processFunction(uniqNames[j]);
      MenuFile->AppendCheckItem((int)(ID_FILE0+j), uniqNames[j].u_str());
      FRecentFiles.Add(uniqNames[j],
        MenuFile->FindItemByPosition(MenuFile->GetMenuItemCount()-1));
    }
  }

  I = DF.Root().FindItem("Defaults");
  DefStyle = TEFile::ExpandRelativePath(I->GetFieldValue("Style"));
    processFunction(DefStyle);
  DefSceneP = TEFile::ExpandRelativePath(I->GetFieldValue("Scene"));
    processFunction(DefSceneP);
  // loading default style if provided ?
  if( TEFile::Exists(DefStyle) )  {
    TDataFile SDF;
    SDF.LoadFromXLFile(DefStyle, &Log);
    FXApp->GetRender().GetStyles().FromDataItem(*SDF.Root().FindItem("style"), false);
  }
  else  {
    TDataItem& last_saved_style = DF.Root().FindRequiredItem("Styles");
    int l_version = TGraphicsStyles::ReadStyleVersion(last_saved_style);
    // old style override, let's hope it is newer!
    if( l_version < TGraphicsStyles::CurrentVersion )  {
      olxstr new_set = FXApp->GetBaseDir() + "last.osp";
      if( TEFile::Exists(new_set) )  {
        TDataFile LF;
        try  {  
          LF.LoadFromXLFile(new_set);
          TDataItem& distributed_style = LF.Root().FindRequiredItem("Styles");
          int d_version = TGraphicsStyles::ReadStyleVersion(distributed_style);
          // it would be weird if distributed version is not current... but might happen
          FXApp->GetRender().GetStyles().FromDataItem(
            (d_version <= l_version) ? last_saved_style : distributed_style, false);
        }
        catch(...)  {  // recover...
          FXApp->GetRender().GetStyles().FromDataItem(last_saved_style, false);
        }
      }
    }
    else  // up-to-date then...
      FXApp->GetRender().GetStyles().FromDataItem(last_saved_style, false);
  }
  // default scene properties provided?
  if( TEFile::Exists(DefSceneP) )  {
    TDataFile SDF;
    SDF.LoadFromXLFile(DefSceneP, &Log);
    LoadScene(SDF.Root(), FXApp->GetRender().LightModel);
  }
  else
    LoadScene(DF.Root().FindRequiredItem("Scene"), FXApp->GetRender().LightModel);
  // restroring language or setting default
  if( TEFile::Exists( DictionaryFile ) )  {
    try  {
      Dictionary.SetCurrentLanguage(DictionaryFile,
        I->GetFieldValue("language", EmptyString()));
    }
    catch(const TExceptionBase& e)  {
      ShowAlert(e, "Failed loading/processing dictionary file");
    }
  }
  FXApp->SetExtraZoom(I->GetFieldValue("ExtraZoom", "1.25").ToDouble());
#ifdef __WIN32__
  const olxstr& defGlTVal = FalseString();
#else
  const olxstr& defGlTVal = TrueString();
#endif
  UseGlTooltip( I->GetFieldValue("GlTooltip", defGlTVal).ToBool() );
  if( I->FieldExists("ThreadCount") ) 
    FXApp->SetMaxThreadCount(I->GetFieldValue("ThreadCount", "1").ToInt());
  else  {
    int cpu_cnt = wxThread::GetCPUCount();
    if( cpu_cnt > 0 )
      FXApp->SetMaxThreadCount(cpu_cnt);
  }
  if( FBgColor.GetRGB() == 0xffffffff )  {  // only if the information got lost
    olxstr T( I->GetFieldValue("BgColor") );
    if( !T.IsEmpty() )  FBgColor.FromString(T);
  }
  bool whiteOn =  I->GetFieldValue("WhiteOn", FalseString()).ToBool();
  FXApp->GetRender().LightModel.SetClearColor(
    whiteOn ? 0xffffffff : FBgColor.GetRGB());

  GradientPicture = TEFile::ExpandRelativePath(
    I->GetFieldValue("GradientPicture", EmptyString()));
  if( !TEFile::Exists(GradientPicture) )
    GradientPicture.SetLength(0);
  olxstr T = I->GetFieldValue("Gradient", EmptyString());
  if( !T.IsEmpty() ) 
    processMacro(olxstr("grad ") << T);

  I = DF.Root().FindItem("Stored_params");
  if( I )  {
    for( size_t i=0; i < I->ItemCount(); i++ )  {
      TDataItem& pd = I->GetItem(i);
      processMacro( olxstr("storeparam ") << pd.GetName() << ' '
                      << '\'' << pd.GetFieldValue("value") << '\'' << ' '
                      << pd.GetFieldValue("process", EmptyString()));
    }
  }
}
//..............................................................................
void TMainForm::LoadScene(const TDataItem& Root, TGlLightModel& FLM) {
  TDataFile F;
  olxstr FntData;
  FLM.FromDataItem(Root.FindRequiredItem("Scene_Properties"));
  FBgColor = FLM.GetClearColor();
  TDataItem *I = Root.FindItem("Fonts");
  if( I == NULL )  return;
  for( size_t i=0; i < I->ItemCount(); i++ )  {
    TDataItem& fi = I->GetItem(i);
    // compatibility conversion...
    if( fi.GetName() == "Console" )
      FXApp->GetRender().GetScene().CreateFont("Default", fi.GetFieldValue("id"));
    else
      FXApp->GetRender().GetScene().CreateFont(fi.GetName(), fi.GetFieldValue("id"));
  }
  I = Root.FindItem("Materials");
  if( I != NULL )  {
    TDataItem *ci;
    ci = I->FindItem("Help_txt");
    if( ci != NULL )  HelpFontColorTxt.FromDataItem(*ci);
    ci = I->FindItem("Help_cmd");
    if( ci != NULL ) HelpFontColorCmd.FromDataItem(*ci);

    ci = I->FindItem("Exec");
    if( ci != NULL ) ExecFontColor.FromDataItem(*ci);
    ci = I->FindItem("Info");
    if( ci != NULL ) InfoFontColor.FromDataItem(*ci);
    ci =I->FindItem("Warning");
    if( ci != NULL ) WarningFontColor.FromDataItem(*ci);
    ci = I->FindItem("Error");
    if( ci != NULL ) ErrorFontColor.FromDataItem(*ci);
    ci = I->FindItem("Exception");
    if( ci != NULL ) ExceptionFontColor.FromDataItem(*ci);
  }
  FXApp->GetRender().InitLights();
}
//..............................................................................
void TMainForm::SaveScene(TDataItem &Root, const TGlLightModel &FLM) const {
  FLM.ToDataItem(Root.AddItem("Scene_Properties"));
  TDataItem *I = &Root.AddItem("Fonts");
  for( size_t i=0; i < FXApp->GetRender().GetScene().FontCount(); i++ )  {
    TDataItem& fi = I->AddItem(FXApp->GetRender().GetScene()._GetFont(i).GetName());
    fi.AddField("id", FXApp->GetRender().GetScene()._GetFont(i).GetIdString());
  }
  I = &Root.AddItem("Materials");
  HelpFontColorTxt.ToDataItem(I->AddItem("Help_txt"));
  HelpFontColorCmd.ToDataItem(I->AddItem("Help_cmd"));
  ExecFontColor.ToDataItem(I->AddItem("Exec"));
  InfoFontColor.ToDataItem(I->AddItem("Info"));
  WarningFontColor.ToDataItem(I->AddItem("Warning"));
  ErrorFontColor.ToDataItem(I->AddItem("Error"));
  ExceptionFontColor.ToDataItem(I->AddItem("Exception"));
}
//..............................................................................
void TMainForm::UpdateRecentFile(const olxstr& fn)  {
  if( fn.IsEmpty() )  {
    for( size_t i=0; i < FRecentFiles.Count(); i++ )  // change item captions
      FRecentFiles.GetObject(i)->Check(false);
    return;
  }
  TPtrList<wxMenuItem> Items;
  olxstr FN( (fn.EndsWithi(".ins") || fn.EndsWithi(".res")) ? 
    TEFile::ChangeFileExt(fn, EmptyString()) : fn );
  TEFile::OSPathI(FN);
  size_t index = FRecentFiles.IndexOf(FN);
  wxMenuItem* mi=NULL;
  if( index == InvalidIndex )  {
    if( (FRecentFiles.Count()+1) < FRecentFilesToShow )  {
      for( size_t i=0; i < MenuFile->GetMenuItemCount(); i++ )  {
        wxMenuItem* item = MenuFile->FindItemByPosition(i);
          if( item->GetId() >= ID_FILE0 && item->GetId() <= (ID_FILE0+FRecentFilesToShow))
            index = i;
      }
      if( index != InvalidIndex )
        mi = MenuFile->InsertCheckItem(index + 1, (int)(ID_FILE0+FRecentFiles.Count()), wxT("tmp"));
      else
        mi = MenuFile->AppendCheckItem((int)(ID_FILE0+FRecentFiles.Count()), wxT("tmp"));
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
    Items[i]->SetText( FRecentFiles[Items[i]->GetId()-ID_FILE0].u_str() ) ;
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
    Tmp = "<a href=\"reap \'";
    Tmp << TEFile::OSPath(FRecentFiles[i]) << "\'\">";
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
int SortQPeak(const TCAtom &a1, const TCAtom &a2)  {
  return olx_cmp(a2.GetQPeak(), a1.GetQPeak());
}
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
    QuickSorter::SortSF(atoms, SortQPeak);
    Table.Resize( olx_min(10, atoms.Count()), 3);
    const double LQP = olx_max(0.01, atoms[0]->GetQPeak());
    size_t rowIndex = 0;
    for( size_t i=0; i < atoms.Count(); i++, rowIndex++ )  {
      if( i > 8 )  i = atoms.Count() -1;
      Table[rowIndex][0] = atoms[i]->GetLabel();
      Table[rowIndex][1] = olxstr::FormatFloat(3, atoms[i]->GetQPeak());
      olxstr Tmp = "<a href=\"sel -i ";
      if( i > rowIndex )
        Tmp << atoms[rowIndex]->GetLabel() << " to ";
      Tmp << atoms[i]->GetLabel();
      if( atoms[i]->GetQPeak() < 2 )
        Tmp << "\"><zimg border=\"0\" src=\"gui/images/bar_small.gif\" height=\"10\" width=\"";
      else
        Tmp << "\"><zimg border=\"0\" src=\"gui/images/bar_large.gif\" height=\"10\" width=\"";
      Tmp << olxstr::FormatFloat(1, atoms[i]->GetQPeak()*100/LQP);
      Tmp << "%\"></a>";
      Table[rowIndex][2] = Tmp;
    }
  }
  TStrList Output;
  Table.CreateHTMLList(Output, EmptyString(), false, false, TableDef);
  olxcstr cst = TUtf8::Encode(Output.Text('\n'));
  TFileHandlerManager::AddMemoryBlock(QPeakTableFile, cst.c_str(), cst.Length(), plStructure);
  if( TEFile::Exists(QPeakTableFile) )
    TEFile::DelFile(QPeakTableFile);
  //TUtf8File::WriteLines( FN, Output, false );
}
//..............................................................................
void TMainForm::BadReflectionsTable(bool TableDef, bool Create)  {
  static const olxstr BadRefsFile("badrefs.htm");
  if( !Create ||
    (!FXApp->CheckFileType<TIns>() &&
      !(FXApp->XFile().HasLastLoader() && FXApp->XFile().LastLoader()->IsNative())) )
  {
    TFileHandlerManager::AddMemoryBlock(BadRefsFile, NULL, 0, plStructure);
    return;
  }
  const TTypeList<RefinementModel::BadReflection> bad_refs =
    FXApp->XFile().GetRM().GetBadReflectionList();
  TTTable<TStrList> Table;
  TStrList Output;
  Table.Resize(bad_refs.Count(), 5);
  Table.ColName(0) = "H";
  Table.ColName(1) = "K";
  Table.ColName(2) = "L";
  Table.ColName(3) = "(Fc<sup>2</sup>-Fo<sup>2</sup>)/esd";
  for( size_t i=0; i < bad_refs.Count(); i++ )  {
    for (int j=0; j < 3; j++)
      Table[i][j] = bad_refs[i].index[j];
    double df = (bad_refs[i].Fc-bad_refs[i].Fo)/bad_refs[i].esd;
    if( olx_abs(df) >= 10 ) {
      Table[i][3] << "<font color=\'red\'>" <<
        olxstr::FormatFloat(2, df) << "</font>";
    }
    else  
      Table[i][3] << olxstr::FormatFloat(2, df);
    if( FXApp->XFile().GetRM().GetOmits().IndexOf(
      bad_refs[i].index) != InvalidIndex )
    {
      Table[i][4] << "Omitted";
    }
    else {
      Table[i][4] << "<a href='omit " << bad_refs[i].index[0] <<  ' ' <<
        bad_refs[i].index[1] << ' ' << bad_refs[i].index[2] << "\'>" <<
        "omit" << "</a>";
    }
  }
  Table.CreateHTMLList(Output, EmptyString(), true, false, TableDef);
  olxcstr cst = TUtf8::Encode(Output.Text('\n'));
  TFileHandlerManager::AddMemoryBlock(BadRefsFile, cst.c_str(), cst.Length(),
    plStructure);
  if( TEFile::Exists(BadRefsFile) )
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
  Table[7][0] = "&rho;/g*mm<sup>-3</sup>";
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
void TMainForm::OnMouseWheel(int x, int y, double delta)  {
  size_t ind = Bindings.IndexOf("wheel");
  if( ind == InvalidIndex )  return;
  olxstr cmd( Bindings.GetObject(ind) );
  ind = TOlxVars::VarIndex("core_wheel_step");
  const olxstr& step( ind == InvalidIndex ? EmptyString() : TOlxVars::GetVarStr(ind));
  if( step.IsNumber() )
    delta *= step.ToDouble();
  cmd << delta;
  processMacro(cmd);
}
//..............................................................................
void TMainForm::OnMouseMove(int x, int y)  {
  if( MousePositionX == x && MousePositionY == y )
    return;  
  else  {
    MouseMoveTimeElapsed = 0;
    MousePositionX = x;
    MousePositionY = y;
    if( !_UseGlTooltip )  {
      if( FGlCanvas->GetToolTip() != NULL && !FGlCanvas->GetToolTip()->GetTip().IsEmpty() )
        FGlCanvas->SetToolTip(wxT(""));
    }
    else if( GlTooltip != NULL && GlTooltip->IsVisible() )  {
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
bool TMainForm::OnMouseUp(int x, int y, short Flags, short Buttons)  {
  if( Modes->GetCurrent() != NULL && Buttons == smbLeft )  {
    if( (abs(x-MousePositionX) < 3) && (abs(y-MousePositionY) < 3) )  {
      AGDrawObject *G = FXApp->SelectObject(x, y);
      if( G != FObjectUnderMouse )
        return false;
      if( G != NULL && Modes->GetCurrent()->OnObject(*G) )
        return true;
    }
  }
  // HKL "grid snap on mouse release
  if( FXApp->XFile().HasLastLoader() && FXApp->IsHklVisible() && false )  {
    mat3d cellM, M;
    vec3d N(0, 0, 1), Z;
    TAsymmUnit *au = &FXApp->XFile().GetAsymmUnit();
    cellM = au->GetHklToCartesian();
    cellM *= FXApp->GetRender().GetBasis().GetMatrix();
    cellM.Transpose();
    // 4x4 -> 3x3 matrix
    Z = cellM[0];    M[0] = Z;
    Z = cellM[1];    M[1] = Z;
    Z = cellM[2];    M[2] = Z;
    Z = FXApp->GetRender().GetBasis().GetMatrix()[2];
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
      N = FXApp->GetRender().GetBasis().GetMatrix()[2];
        double ca = N.CAngle(Z);
        if( ca < -1 )  ca = -1;
        if( ca > 1 )   ca = 1;
        vec3d V = Z.XProdVec(N);
        FXApp->GetRender().GetBasis().Rotate(V, acos(ca));
      N = FXApp->GetRender().GetBasis().GetMatrix()[2];
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
bool TMainForm::CheckMode(size_t mode, const olxstr& modeData)  {
  if( Modes->GetCurrent() == NULL )  return false;
  return mode == Modes->GetCurrent()->GetId();
}
//..............................................................................
bool TMainForm::CheckState(size_t state, const olxstr& stateData) const {
  if (state == stateHtmlVisible) {
    if (stateData.IsEmpty()) return FHtmlMinimized;
    THtmlManager::TPopupData* pp = HtmlManager.Popups.Find(stateData, NULL);
    return (pp != NULL) ? pp->Dialog->IsShown() : false;
  }
  if (state == statePluginInstalled) {
    if( stateData.IsEmpty() ) return false;
    return FPluginItem->ItemExists(stateData);
  }
  if( state == stateCmdLineVisible )
    return CmdLineVisible;
  return false;
}
//..............................................................................
void TMainForm::OnInternalIdle()  {
  if( Destroying )  return;
  wxWindow::UpdateWindowUI();
#if !defined(__WIN32__)  
  if( !StartupInitialised )
    StartupInit();
#endif
  TBasicApp::GetInstance().OnIdle.Execute((AEventsDispatcher*)this, NULL);
  // runonce business...
  if( !RunOnceProcessed && TBasicApp::IsBaseDirWriteable() )  {
    RunOnceProcessed = true;
    TStrList rof;
    TEFile::ListDir(FXApp->GetBaseDir(), rof, "runonce*.*", sefFile);
    TStrList macros;
    for( size_t i=0; i < rof.Count(); i++ )  {
      rof[i] = FXApp->GetBaseDir()+rof[i];
      try  {
        macros.LoadFromFile( rof[i] );
        macros.CombineLines('\\');
        for( size_t j=0; j < macros.Count(); j++ )  {
          processMacro(macros[j]);
#ifdef _DEBUG
          FXApp->NewLogEntry() << TEFile::ExtractFileName(rof[i]) << ": " << macros[j];
#endif
        }
      }
      catch(const TExceptionBase& e)  {
        ShowAlert(e);
      }
      time_t fa = TEFile::FileAge( rof[i] );
      // Null the file
      try  {  TEFile ef(rof[i], "wb+");  }
      catch(TIOException&)  {}
      TEFile::SetFileTimes(rof[i], fa, fa);
      //TEFile::DelFile(rof.String(i));
    }
  }
  wxFrame::OnInternalIdle();
#ifdef __MAC__  // duno why otherwise it takes 100% of CPU time...
  wxMilliSleep(15);
#endif  
  return;
}
//..............................................................................
void TMainForm::SetUserCursor(const olxstr& param, const olxstr& mode)  {
  wxBitmap bmp(32, 32);
  wxMemoryDC memDC;
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
  memDC.SelectObject(bmp);
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
void TMainForm::print(const olxstr& output, const short MessageType)  {
//  TGlMaterial *glm = NULL;
  if( MessageType == olex::mtInfo )
    TBasicApp::NewLogEntry(logInfo) << output;
  else if( MessageType == olex::mtWarning )
    TBasicApp::NewLogEntry(logWarning) << output;
  else if( MessageType == olex::mtError )
    TBasicApp::NewLogEntry(logError) << output;
  else if( MessageType == olex::mtException )
    TBasicApp::NewLogEntry(logException) << output;
  // if need to foce printing - so go aroung the log
//  if( MessageType == 0 )  ;
//  else if( MessageType == olex::mtInfo )      glm = &InfoFontColor;
//  else if( MessageType == olex::mtWarning )   glm = &WarningFontColor;
//  else if( MessageType == olex::mtError )     glm = &ErrorFontColor;
//  else if( MessageType == olex::mtException ) glm = &ExceptionFontColor;
//  FGlConsole->PostText(output, glm);
}
//..............................................................................
void TMainForm::AnalyseErrorEx(TMacroError& error, bool quiet)  {
  if( !error.IsSuccessful() )  {
    if( error.IsProcessingException() ) {
      TBasicApp::NewLogEntry(logException) << error.GetLocation() << ": " <<
        error.GetInfo();
    }
    else if( !error.GetInfo().IsEmpty() )  {
      TBasicApp::NewLogEntry(quiet ? logInfo : logError) << error.GetLocation()
        << ": " <<  error.GetInfo();
    }
    while( !error.GetStack().IsEmpty() )  {
      TBasicApp::NewLogEntry(quiet ? logInfo : logDefault) << '\t'
        << error.GetStack().Pop().TrimWhiteChars();
    }
  }
}
//..............................................................................
bool TMainForm::ProcessEvent(wxEvent& evt)  {
  if( evt.GetEventType() == wxEVT_COMMAND_MENU_SELECTED &&
      AccMenus.ValueExists(evt.GetId()) )
  {
    olxstr macro( AccMenus.GetValue(evt.GetId())->GetCommand());
    if( !macro.IsEmpty() )  {
      TStrList sl;
      sl.Strtok( macro, ">>");
      for( size_t i=0; i < sl.Count(); i++ )  {
        if( !processMacro(sl[i]) )
          break;
      }
      // restore state if failed
      if( AccMenus.GetValue(evt.GetId())->IsCheckable() )
        AccMenus.GetValue(evt.GetId())->ValidateState();

      FXApp->Draw();
      return true;
    }
  }
  evt.Skip();
  return wxFrame::ProcessEvent(evt);
}
//..............................................................................
int TMainForm::TranslateShortcut(const olxstr& sk)  {
  if( sk.IsEmpty() )  return -1;
  TStrList toks(sk, '+');
  if( !toks.Count() )  return -1;
  short Shift=0, Char = 0;
  for( size_t i=0; i < toks.Count() - 1; i++ )  {
    if( toks[i].Equalsi("Ctrl") )
      Shift |= sssCtrl;
    else if( toks[i].Equalsi("Shift") )
      Shift |= sssShift;
    else if( toks[i].Equalsi("Alt") )
      Shift |= sssAlt;
    else if( toks[i].Equalsi("Cmd") )
      Shift |= sssCmd;
  }
  olxstr charStr = toks.GetLastString();
  // a char
  if( charStr.Length() == 1 ) {
    Char = charStr[0];
    if( Char >= 'a' && Char <= 'z' ) Char -= ('a'-'A');
    return ((Shift << 16)|Char);
  }
  if( charStr.CharAt(0) == 'F' && charStr.SubStringFrom(1).IsNumber() )  {
    Char = WXK_F1 + charStr.SubStringFrom(1).ToInt() - 1;
    return ((Shift << 16)|Char);
  }
  charStr.UpperCase();
  if( charStr == "TAB" )       Char = WXK_TAB;
  else if( charStr == "HOME" ) Char = WXK_HOME;
  else if( charStr == "PGUP" ) Char = WXK_PAGEUP;
  else if( charStr == "PGDN" ) Char = WXK_PAGEDOWN;
  else if( charStr == "END" )  Char = WXK_END;
  else if( charStr == "DEL" )  Char = WXK_DELETE;
  else if( charStr == "INS" )  Char = WXK_INSERT;
  else if( charStr == "BREAK" ) Char = WXK_PAUSE;
  else if( charStr == "BACK" ) Char = WXK_BACK;

  return Char!=0 ? ((Shift << 16)|Char) : -1;
}
//..............................................................................
bool TMainForm::OnMouseDblClick(int x, int y, short Flags, short Buttons)  {
  AGDrawObject *G = FXApp->SelectObject(x, y);
  if( G == NULL )  {
    processMacro("sel -u");
    return true;
  }
  if( EsdlInstanceOf(*G, TGlBitmap) )  {
    TGlBitmap* glB = (TGlBitmap*)G;
    if( !(glB->GetLeft() > 0) )  {
      int Top = InfoWindowVisible ? FInfoBox->GetTop() + FInfoBox->GetHeight() : 1;
      for( size_t i=0; i < FXApp->GlBitmapCount(); i++ )  {
        TGlBitmap* b = &FXApp->GlBitmap(i);
        if( b == glB )  break;
        Top += (b->GetHeight() + 2);
      }
      const double r = ((double)FXApp->GetRender().GetWidth()/(double)glB->GetWidth()) / 10.0;
      glB->SetZoom(r);
      glB->SetTop(Top);
      glB->SetLeft(FXApp->GetRender().GetWidth() - glB->GetWidth());
    }
    else  {
      glB->SetLeft(0);
      glB->SetTop(InfoWindowVisible ? FInfoBox->GetTop() + FInfoBox->GetHeight() : 1);
      glB->SetZoom(1.0);
    }
  }
  else if( EsdlInstanceOf(*G, TXGlLabel) )  {
    olxstr label = "getuserinput(1, \'Atom label\', \'";
    label << ((TXGlLabel*)G)->GetLabel() << "\')";
    if( processFunction(label) && !label.IsEmpty() )
      ((TXGlLabel*)G)->SetLabel(label);

  }
  else if( EsdlInstanceOf(*G, TXAtom) )  {
    TNetwork& n = ((TXAtom*)G)->GetNetwork();
    size_t sel_cnt=0, cnt = 0;
    for( size_t i=0; i < n.NodeCount(); i++ )  {
      TXAtom& a = (TXAtom&)n.Node(i);
      if( !a.IsVisible() )  continue;
      cnt++;
      if( a.IsSelected() )  sel_cnt++;
    }
    for( size_t i=0; i < n.BondCount(); i++ )  {
      TXBond& b = (TXBond&)n.Bond(i);
      if( !b.IsVisible() )  continue;
      cnt++;
      if( b.IsSelected() )  sel_cnt++;
    }
    if( cnt > 0 )  {
      FXApp->SelectFragments(TNetPList() << &n, ((double)sel_cnt/cnt) < .75);
    }
  }
  FXApp->Draw();
  return true;
}
//..............................................................................
bool TMainForm::Show(bool v)  {
#ifdef __WXGTK__
  bool res = wxWindow::Show(v);
  //OnResize();
#else
  bool res = wxFrame::Show(v);
#endif
  FXApp->SetMainFormVisible(v);
  if (CmdLineVisible)
    FCmdLine->SetFocus();
  else
    FGlCanvas->SetFocus();
  return res;
}
//..............................................................................
const olxstr& TMainForm::TranslatePhrase(const olxstr& phrase)  {
  return Dictionary.Translate(phrase);
}
//..............................................................................
olxstr TMainForm::TranslateString(const olxstr& str) const {
  olxstr phrase(str);
  size_t ind = phrase.FirstIndexOf('%');
  while( ind != InvalidIndex )  {
    if( ind+1 >= phrase.Length() )  return phrase;
    // analyse the %..
    size_t pi = ind;
    while( --pi != InvalidIndex &&
      (olxstr::o_isdigit(phrase.CharAt(pi)) || phrase.CharAt(pi) == '.') )
      ;
    if( pi != ind-1 && pi != InvalidIndex )  {
      if( phrase.CharAt(pi) == '\'' || phrase.CharAt(pi) == '\"' )  {
        if( ind < phrase.Length() && phrase.CharAt(pi) == phrase.CharAt(ind+1) )  {
          ind = phrase.FirstIndexOf('%', ind+1);
          continue;
        }
      }
      if( phrase.CharAt(pi) == '=' )  {
        ind = phrase.FirstIndexOf('%', ind+1);
        continue;
      }
    }
    size_t ind1 = phrase.FirstIndexOf('%', ind+1);
    if( ind1 == InvalidIndex )  return phrase;
    if( ind1 == ind+1 )  { // %%
      phrase.Delete(ind1, 1);
      ind = phrase.FirstIndexOf('%', ind1);
      continue;
    }
    olxstr tp( Dictionary.Translate( phrase.SubString(ind+1, ind1-ind-1) ) );
    phrase.Delete(ind, ind1-ind+1);
    phrase.Insert(tp, ind);
    ind1 = ind + tp.Length();
    if( ind1+1 >= phrase.Length() )  return phrase;
    ind = phrase.FirstIndexOf('%', ind1+1);
  }
  return phrase;
}
//..............................................................................
void TMainForm::UseGlTooltip(bool v)  {
  if( v == _UseGlTooltip )
    return;
  TStateRegistry::GetInstance().SetState(stateGlTooltips, v, EmptyString(), false);
  _UseGlTooltip = v;
  if( v )
    FGlCanvas->SetToolTip(wxT(""));
}
//..............................................................................
//..............................................................................
//..............................................................................
bool TMainForm::registerCallbackFunc(const olxstr& cbEvent, ABasicFunction* fn)  {
  CallbackFuncs.Add(cbEvent, fn);
  return true;
}
//..............................................................................
void TMainForm::unregisterCallbackFunc(const olxstr& cbEvent, const olxstr& funcName)  {
  size_t ind = CallbackFuncs.IndexOf(cbEvent),
    i = ind;
  if( ind == InvalidIndex )  return;
  // go forward
  while( i < CallbackFuncs.Count() && CallbackFuncs.GetKey(i).Equals(cbEvent) )  {
    if( CallbackFuncs.GetObject(i)->GetName() == funcName )  {
      delete CallbackFuncs.GetObject(i);
      CallbackFuncs.Delete(i);
      return;
    }
    i++;
  }
  // go backwards
  i = ind-1;
  while( i !=InvalidIndex && (!CallbackFuncs.GetKey(i).Compare(cbEvent)) )  {
    if( CallbackFuncs.GetObject(i)->GetName() == funcName )  {
      delete CallbackFuncs.GetObject(i);
      CallbackFuncs.Delete(i);
      return;
    }
    i--;
  }
}
//..............................................................................
const olxstr& TMainForm::getDataDir() const  {  return FXApp->GetInstanceDir();  }
//..............................................................................
const olxstr& TMainForm::getVar(const olxstr &name, const olxstr &defval) const {
  const size_t i = TOlxVars::VarIndex(name);
  if( i == InvalidIndex )  {
    if( &defval == NULL )
      throw TInvalidArgumentException(__OlxSourceInfo, "undefined key");
    TOlxVars::SetVar(name, defval);
    return defval;
  }
  return TOlxVars::GetVarStr(i);
}
//..............................................................................
void TMainForm::setVar(const olxstr &name, const olxstr &val) const {
  TOlxVars::SetVar(name, val);
}
//..............................................................................
void TMainForm::callCallbackFunc(const olxstr& cbEvent, const TStrList& params)
{
  TSizeList indexes;
  TMacroError me;
  CallbackFuncs.GetIndexes(cbEvent, indexes);
  for( size_t i=0; i < indexes.Count(); i++ )  {
    CallbackFuncs.GetObject(indexes[i])->Run(params, me);
    AnalyseError(me);
    me.Reset();
  }
}
//..............................................................................
void TMainForm::SaveVFS(short persistenceId)  {
  try  {
    olxstr dbFN;
    if( persistenceId == plStructure )  {
      if( !FXApp->XFile().HasLastLoader() )  return;
      dbFN = GetStructureOlexFolder();
      dbFN << TEFile::ChangeFileExt(
        TEFile::ExtractFileName(FXApp->XFile().GetFileName()) , "odb");
    }
    else if(persistenceId == plGlobal )
      dbFN << FXApp->GetInstanceDir() << "global.odb";
    else
      throw TFunctionFailedException(__OlxSourceInfo, "undefined persistence level");

    TEFile dbf(dbFN + ".tmp", "wb");
    TFileHandlerManager::SaveToStream(dbf, persistenceId);
    dbf.Close();
    TEFile::Rename(dbFN + ".tmp", dbFN);
  }
  catch(const TExceptionBase &e)  {
    ShowAlert(e, "Failed to save VFS");
  }
}
//..............................................................................
void TMainForm::LoadVFS(short persistenceId)  {
  try  {
    olxstr dbFN;
    if( persistenceId == plStructure )  {
      if( !FXApp->XFile().HasLastLoader() )  return;
      dbFN = GetStructureOlexFolder();
      dbFN << TEFile::ChangeFileExt(
        TEFile::ExtractFileName(FXApp->XFile().GetFileName()) , "odb");
    }
    else if(persistenceId == plGlobal )
      dbFN << FXApp->GetInstanceDir() << "global.odb";
    else  {
      throw TFunctionFailedException(__OlxSourceInfo,
        "undefined persistence level");
    }

    if( !TEFile::Exists(dbFN ) )  return;
    try  {
      TEFile dbf(dbFN, "rb");
      TFileHandlerManager::LoadFromStream(dbf, persistenceId);
    }
    catch(const TExceptionBase &e)  {
      TEFile::DelFile(dbFN);
      throw TFunctionFailedException(__OlxSourceInfo, e, "failed to read VFS");
    }
  }
  catch(const TExceptionBase &e)  {
    ShowAlert(e, "Failed to read VFS");
  }
}
//..............................................................................
const olxstr& TMainForm::GetStructureOlexFolder()  {
  if( FXApp->XFile().HasLastLoader() )  {
    olxstr ofn = TEFile::ExtractFilePath(FXApp->XFile().GetFileName());
    TEFile::AddPathDelimeterI(ofn) << ".olex/";
    if( !TEFile::Exists(ofn) )  {
      if( !TEFile::MakeDir(ofn) )  {
        throw TFunctionFailedException(__OlxSourceInfo, "cannot create folder");
      }
#ifdef __WIN32__
      SetFileAttributes(ofn.u_str(), FILE_ATTRIBUTE_HIDDEN);
#endif
    }
    return TEGC::New<olxstr>(ofn);
  }
  return EmptyString();
}
//..............................................................................
bool TMainForm::FindXAtoms(const TStrObjList &Cmds, TXAtomPList& xatoms,
  bool GetAll, bool unselect)
{
  size_t cnt = xatoms.Count();
  xatoms.AddList(FXApp->FindXAtoms(Cmds, GetAll, unselect));
  return (xatoms.Count() != cnt);
}
//..............................................................................
const olxstr& TMainForm::GetSGList() const {
  size_t ind = TOlxVars::VarIndex(SGListVarName);
  return (ind == InvalidIndex) ? EmptyString() : TOlxVars::GetVarStr(ind);
}
//..............................................................................
void TMainForm::SetSGList(const olxstr &sglist)  {
  TOlxVars::SetVar(SGListVarName, EmptyString());
}
//..............................................................................
TStrList TMainForm::GetPluginList() const {
  TStrList rv;
  if( FPluginItem != NULL )  {
    for( size_t i=0; i < FPluginItem->ItemCount(); i++ )
      rv.Add(FPluginItem->GetItem(i).GetName());
  }
  return rv;
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
  volatile olx_scope_cs cs(TBasicApp::GetCriticalSection());
  if( _UpdateThread == NULL )  return;
  uint64_t sz = _UpdateThread->GetUpdateSize();
  updater::SettingsFile sf(updater::UpdateAPI::GetSettingsFileName());
  TdlgMsgBox* msg_box = NULL;
  if( sf.ask_for_update )  {
    msg_box = new TdlgMsgBox( this, 
      olxstr("There are new updates available (") <<
        olxstr::FormatFloat(3, (double)sz/(1024*1024)) << "Mb)\n" <<
      "Updates will be downloaded in the background during this session and\n"
      "will take effect with the next restart of Olex2",
      "Automatic Updates",
      "Do not show this message again",
      wxYES|wxNO|wxICON_QUESTION,
      true);
    int res = msg_box->ShowModal();  
    if( res == wxID_YES )  {
      _UpdateThread->DoUpdate();
      if( msg_box->IsChecked() )  {
        sf.ask_for_update = false;
        sf.Save();
      }
    }
    else  {
      _UpdateThread->SendTerminate();
      _UpdateThread = NULL;
    }
    msg_box->Destroy();
  }
  else
    _UpdateThread->DoUpdate();
  if( _UpdateThread != NULL )
    _UpdateThread->ResetUpdateSize();
}
//..............................................................................
size_t TMainForm::DownloadFiles(const TStrList &files, const olxstr &dest) {
  if (files.IsEmpty()) return 0;
  THttpFileSystem fs;
  size_t cnt=0;
  updater::SettingsFile sf(updater::UpdateAPI::GetSettingsFileName());
  TUrl proxy(sf.proxy);
  for (size_t i=0; i < files.Count(); i++) {
    TUrl url(TEFile::UnixPath(files[i]));
    if (!proxy.GetHost().IsEmpty())
      url.SetProxy(proxy);
    fs.SetUrl(url);
    TEFile* file = NULL;
    try {
      file = fs.OpenFileAsFile(url.GetPath());
      if( file != NULL )  {
        if( !TEFile::Exists(dest) )
          TEFile::MakeDir(dest);
        const olxstr dest_fn = TEFile::AddPathDelimeter(dest) <<
          TEFile::ExtractFileName(url.GetPath());
        TEFile dest(dest_fn, "wb+");
        dest << *file;
        TEFile* df = file;
        file = NULL;
        delete df;
        dest.Close();
        cnt++;
      }
    }
    catch(const TExceptionBase& e)  {
      if( file != NULL )
        delete file;
      TBasicApp::NewLogEntry(logError) << e.GetException()->GetFullMessage();
    }
  }
  return cnt;
}
//..............................................................................
void TMainForm::UpdateUserOptions(const olxstr &option, const olxstr &value) {
  FXApp->UpdateOption(option, value);
  try {
    TSettingsFile st;
    olxstr fn = FXApp->GetInstanceDir() + ".options";
    if (TEFile::Exists(fn))
      st.LoadSettings(fn);
    st.SetParam(option, value);
    st.SaveSettings(fn);
  }
  catch (const TExceptionBase &e) {
    TBasicApp::NewLogEntry(logExceptionTrace) << e;
  }
}
//..............................................................................
//..............................................................................
//..............................................................................

//..............................................................................
PyObject* pyIsControl(PyObject* self, PyObject* args)  {
  olxstr cname, pname;  // control and popup (if any) name
  if( !PythonExt::ParseTuple(args, "w|w", &cname, &pname) )
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "w|w");
  if (!pname.IsEmpty())
    cname = pname << "->" << cname;
  return Py_BuildValue("b", TGlXApp::GetMainForm()->IsControl(cname));
}
//..............................................................................
PyObject* pyGetUserInput(PyObject* self, PyObject* args)  {
  olxstr title, str;
  int flags = 0;
  if( !PythonExt::ParseTuple(args, "iww", &flags, &title, &str) )
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "iww");
  const bool MultiLine = (flags != 1);
  TdlgEdit *dlg = new TdlgEdit(TGlXApp::GetMainForm(), MultiLine);
  dlg->SetTitle(title.u_str());
  dlg->SetText(str);

  PyObject* rv;
  if( dlg->ShowModal() == wxID_OK )
    rv = PythonExt::BuildString(dlg->GetText());
  else  {
    rv = Py_None;
    Py_IncRef(rv);
  }
  dlg->Destroy();
  return rv;
}
//..............................................................................
PyObject* pyPPI(PyObject* self, PyObject* args)  {
  wxWindowDC wx_dc( TGlXApp::GetMainForm() );
  wxSize ppi = wx_dc.GetPPI();
  return Py_BuildValue("(ii)", ppi.GetX(), ppi.GetY());
}
//..............................................................................
static PyMethodDef CORE_Methods[] = {
  {"GetUserInput", pyGetUserInput, METH_VARARGS, 
  "Shows a dialog, where user can type some text. Takes three agruments: flags"
  ", title and content. If flags not equal to 1, a muliline dialog sis created"
  },
  {"IsControl", pyIsControl, METH_VARARGS,
  "Takes HTML element name and optionaly popup name. Returns true/false if "
  "given control exists"
  },
  {"GetPPI", pyPPI, METH_VARARGS, "Returns screen PPI"},
  {NULL, NULL, 0, NULL}
};
//..............................................................................
void TMainForm::PyInit()  {
  Py_InitModule("olex_gui", CORE_Methods);
}
//..............................................................................
//..............................................................................
//..............................................................................
bool TMainForm::FileDropTarget::OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames)  {
  if( filenames.Count() != 1 )  return false;
  const olxstr fn = filenames[0];
  try  {
    if( parent.FXApp->XFile().FindFormat(TEFile::ExtractFileExt(fn)) == NULL )
      return false;
  }
  catch(...)  {  return false;  }
  parent.processMacro(olxstr("reap \'") << fn << '\'');
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
  if( FXApp->XFile().HasLastLoader() )  {
    FInfoBox->PostText(olxstr("\\-") << TEFile::ExtractFilePath(FXApp->XFile().GetFileName()));
    FInfoBox->PostText(TEFile::ExtractFileName(FXApp->XFile().GetFileName()));
    FInfoBox->PostText(FXApp->XFile().LastLoader()->GetTitle());
    FInfoBox->Fit();
  }
  else  {
    FInfoBox->PostText("No file is loaded");
  }
}
//..............................................................................
void TMainForm::ProcessHandler::BeforePrint() {
  parent.FGlConsole->SetPrintMaterial(&parent.ExecFontColor);
  printed = false;
}
//..............................................................................
void TMainForm::ProcessHandler::Print(const olxstr& line)  {
  if( !line.IsEmpty() )  {
    TBasicApp::GetLog() << line;
    parent.callCallbackFunc(ProcessOutputCBName, TStrList() << line);
    printed = true;
  }
}
//..............................................................................
void TMainForm::ProcessHandler::AfterPrint() {
  parent.FGlConsole->SetPrintMaterial(NULL);
  if( printed )  {
    parent.FXApp->Draw();
    printed = false;
  }
}
//..............................................................................
void TMainForm::ProcessHandler::OnWait() {
  parent.FParent->Dispatch();
  TBasicApp::GetInstance().OnTimer.Execute(&(AEventsDispatcher&)parent);
}
//..............................................................................
void TMainForm::ProcessHandler::OnTerminate(const AProcess& p)  {
  TMacroError err;
  for( size_t i=0; i < p.OnTerminateCmds().Count(); i++ ) {
    const olxstr& cmd = p.OnTerminateCmds()[i];
    parent.processMacro(cmd, olxstr("OnTerminate of: ") << p.GetCmdLine());
    if( !err.IsSuccessful() )
      break;
  }
  TBasicApp::NewLogEntry(logInfo) << "The process '" << p.GetCmdLine() <<
    "' has been terminated...";
  parent.TimePerFrame = parent.FXApp->Draw();
}
//..............................................................................
