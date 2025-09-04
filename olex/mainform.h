/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef _xl_mainformH
#define _xl_mainformH

#include "olex2app_imp.h"
#include "ctrls.h"
#include "eprocess.h"
#include "glconsole.h"
#include "datafile.h"
#include "gltextbox.h"
#include "ipimp.h"
#include "macroerror.h"
#include "library.h"
#include "eaccell.h"
#include "estlist.h"
#include "updateth.h"
#include "macrolib.h"
#include "exparse/exptree.h"
#include "nui/nui.h"
#include "tasks.h"
#include "olxstate.h"

#include "wx/wx.h"
#include "wx/dnd.h"
#include "wx/process.h"
#include "wx/thread.h"

enum  {
  ID_HtmlPanel=wxID_HIGHEST,  // view menu

  ID_StrGenerate,  // structure menu

  ID_MenuTang,  // menu item ids
  ID_MenuBang,
  ID_MenuGraphics,
  ID_MenuModel,
  ID_MenuView,
  ID_MenuFragment,
  ID_MenuDrawStyle,
  ID_MenuDrawQ,

  ID_DSBS,  // drawing style, balls and sticks
  ID_DSES,  // ellipsoids and sticks
  ID_DSSP,  // sphere packing
  ID_DSWF,  // wireframe
  ID_DSST,   // sticks
  ID_SceneProps,

  ID_DQH,  // drawing quality
  ID_DQM,
  ID_DQL,

  ID_CellVisible,  // model menu
  ID_BasisVisible,
  ID_ShowAll,
  ID_ModelCenter,

  ID_BondInfo,
  ID_BondRadius,

  ID_AtomInfo,
  ID_MenuAtomType,
  ID_AtomTypeChange,
  ID_AtomTypeChangeLast = ID_AtomTypeChange+120, // reserve some
  ID_AtomGrow,
  ID_AtomCenter,
  ID_AtomSelRings,

  ID_PlaneActivate,
  ID_BondViewAlong,

  ID_FragmentHide,  // fragment menu
  ID_FragmentShowOnly,
  ID_FragmentSelectAtoms,
  ID_FragmentSelectBonds,
  ID_FragmentSelectAll,
  ID_FileLoad,
  ID_FileClose,

  ID_ViewAlong,   // view menu
  ID_ViewAlongLast = ID_ViewAlong + 6,

  ID_MenuAtomOccu,
  ID_AtomOccuCustom,
  ID_AtomOccu1,
  ID_AtomOccu34,
  ID_AtomOccu12,
  ID_AtomOccu13,
  ID_AtomOccu14,
  ID_AtomOccuFix,
  ID_AtomOccuFixCurrent,
  ID_AtomOccuFree,

  ID_MenuAtomConn,
  ID_AtomConnChange,
  ID_AtomConnChangeLast = ID_AtomConnChange + 8,
  ID_AtomBind,
  ID_AtomFree,

  ID_MenuAtomUiso,
  ID_AtomUisoCustom,
  ID_AtomUiso15,
  ID_AtomUiso12,
  ID_AtomUisoFree,
  ID_AtomUisoFix,

  ID_MenuAtomPart,
  ID_AtomPartChange,
  ID_AtomPartChangeLast = ID_AtomPartChange + 6,

  ID_MenuAtomPoly,
  ID_AtomPolyNone,
  ID_AtomPolyAuto,
  ID_AtomPolyRegular,
  ID_AtomPolyPyramid,
  ID_AtomPolyBipyramid,

  ID_AtomExploreEnvi,

  ID_Selection,
  ID_SelGroup,
  ID_SelUnGroup,
  ID_SelLabel,

  ID_GridMenu,
  ID_GridMenuCreateBlob,

  ID_GraphicsKill,
  ID_GraphicsHide,
  ID_GraphicsDS,
  ID_GraphicsP,
  ID_GraphicsEdit,
  ID_GraphicsSelect,
  ID_GraphicsCollectivise,
  ID_GraphicsIndividualise,

  ID_GStyleSave,
  ID_GStyleOpen,
  ID_FixLattice,
  ID_FreeLattice,
  ID_DELINS,
  ID_ADDINS,
  ID_VarChange,
  ID_BadReflectionSet,
  ID_CellChanged,

  ID_UpdateThreadTerminate,
  ID_UpdateThreadDownload,
  ID_UpdateThreadAction,

  ID_FILE0,

  ID_GLDRAW = ID_FILE0+100,
  ID_TIMER,
  ID_INFO,
  ID_WARNING,
  ID_ERROR,
  ID_EXCEPTION,
  ID_LOG,
  ID_ONLINK,
  ID_HTMLKEY,
  ID_COMMAND,
  ID_XOBJECTSDESTROY,
  ID_CMDLINECHAR,
  ID_CMDLINEKEYDOWN,
  ID_TEXTPOST,
  ID_UPDATE_GUI
};

//............................................................................//
const unsigned short
  mListen = 0x0001,    // modes
  mSilent = 0x0002,  // silent mode
  mPick = 0x0020,  // pick mode, for a future use
  mFade = 0x0080,  // structure fading ..
  mRota = 0x0100,  // rotation
  mSolve = 0x0200,  // structure solution
  mSGDet = 0x0400,  // space group determination
  mListenCmd = 0x0800;    // listen to cmds file

// persistence level
const short
  plNone      = 0x0000,  // runtime data only - not saved at any moment
  plStructure = 0x0001,  // data saved/loaded when structure is un/loaded
  plGlobal    = 0x0002;  // data saved/loaded when olex is closed/executed

class TMainForm;
class TGlXApp;

//............................................................................//
class TMainForm: public TMainFrame, public AEventsDispatcher,
  public olex2::OlexProcessorImp
{
  //TFrameMaker FrameMaker;
protected:
  bool Destroying;
  TStack<olx_pair_t<wxCursor,wxString> > CursorStack;
  UpdateThread* _UpdateThread;
  TOnProgress* UpdateProgress, *ActionProgress;
  TStack<TEFile *> LogFiles;
  static olxcstr &ModuleName();
  static PyObject *PyInit();
  TActionQList Action;
  TGlXApp* FParent;
  TArrayList< olx_pair_t<TDUnitCell*, TSpaceGroup*> > UserCells;
  olxstr_dict<olxstr> StoredParams;

  TTypeList<TScheduledTask> Tasks;
  TPtrList<IOlxTask> RunWhenVisibleTasks;

  class TGlCanvas *FGlCanvas;
  Olex2App* FXApp;
  TDataFile FHelpFile, FMacroFile;
  TDataItem *FHelpItem;

  olxstr GradientPicture;
  TGlConsole *FGlConsole;
  TGlTextBox *FHelpWindow, *FInfoBox, *GlTooltip;
  TMacroData MacroError;

  void PreviewHelp(const olxstr& Cmd);
  olxstr ExpandCommand(const olxstr &Cmd, bool inc_files);
  int MouseMoveTimeElapsed, MousePositionX, MousePositionY;

  TModeRegistry *Modes;
  size_t
    stateHtmlVisible,
    stateInfoWidnowVisible,
    stateHelpWindowVisible,
    stateCmdLineVisible,
    stateGlTooltips;
   // solution mode variables
  TIntList Solutions;
  int CurrentSolution;
  olxstr SolutionFolder;
  void ChangeSolution(int sol);

  // helper functions ...
  void CallMatchCallbacks(TNetwork& netA, TNetwork& netB, double RMS);
  void UpdateInfoBox();
  olx_nui::INUI *nui_interface;
  class THtmlManager &HtmlManager;

  virtual void beforeCall(const olxstr &cmd);
  virtual void afterCall(const olxstr &cmd);
  TPtrList<olex2::IOlex2Runnable> loadedDll;
public:
  void OnMouseMove(int x, int y);
  void OnMouseWheel(int x, int y, double delta);
  bool OnMouseDown(int x, int y, short Flags, short Buttons);
  bool OnMouseUp(int x, int y, short Flags, short Buttons);
  bool OnMouseDblClick(int x, int y, short Flags, short Buttons);
  virtual bool Show(bool v);

  void SetUserCursor(const olxstr& param, const olxstr& mode);

  const TModeRegistry& GetModes() const {  return *Modes;  }

  bool CheckMode(size_t mode, const olxstr& modeData);
  bool CheckState(size_t state, const olxstr& stateData) const;
  bool PopupMenu(wxMenu* menu, const wxPoint& p=wxDefaultPosition);
  bool PopupMenu(wxMenu* menu, int x, int y)  {
    return PopupMenu(menu, wxPoint(x,y));
  }
protected:
  void PostCmdHelp(const olxstr &Cmd, bool Full=false);

  void OnSize(wxSizeEvent& event);
  void OnMove(wxMoveEvent& event);

  void OnQuit(wxCommandEvent& event);
  void OnFileOpen(wxCommandEvent& event);
  void OnGenerate(wxCommandEvent& event);
  void OnDrawStyleChange(wxCommandEvent& event);
  void OnDrawQChange(wxCommandEvent& event);
  void OnViewAlong(wxCommandEvent& event);
  void OnCloseWindow(wxCloseEvent &evt);
  friend class TObjectVisibilityChange;
  void BasisVChange();
  void CellVChange();
  void GridVChange();
  void FrameVChange();
  void OnBasisVisible(wxCommandEvent& event);
  void OnCellVisible(wxCommandEvent& event);

  AGDrawObject *FObjectUnderMouse;  //initialised when mouse clicked on an object on screen

  void OnGraphics(wxCommandEvent& event);
  void OnFragmentHide(wxCommandEvent& event);
  void OnShowAll(wxCommandEvent& event);
  void OnModelCenter(wxCommandEvent& event);
  void OnFragmentShowOnly(wxCommandEvent& event);
  void OnFragmentSelectAtoms(wxCommandEvent& event);
  void OnFragmentSelectBonds(wxCommandEvent& event);
  void OnFragmentSelectAll(wxCommandEvent& event);
  // helper function to get the list of fragments (if several selected)
  size_t GetFragmentList(TNetPList& res);

  void OnAtomTypeChange(wxCommandEvent& event);
  void OnAtomOccuChange(wxCommandEvent& event);
  void OnAtomConnChange(wxCommandEvent& event);
  void OnAtomPartChange(wxCommandEvent& event);
  void OnAtomUisoChange(wxCommandEvent& event);
  void OnAtomPolyChange(wxCommandEvent& event);
  void OnAtom(wxCommandEvent& event); // general handler

  void OnBond(wxCommandEvent& event);
  void OnPlane(wxCommandEvent& event); // general handler

  void OnSelection(wxCommandEvent& event);
  void OnGraphicsStyle(wxCommandEvent& event);

  // view menu
  void OnHtmlPanel(wxCommandEvent& event);
  bool ImportFrag(const olxstr& line);
  static size_t DownloadFiles(const TStrList &files, const olxstr &dest);
  static bool DownloadFile(const olxstr &url, const olxstr &dest) {
    return DownloadFiles(TStrList() << url, dest) != 0;
  }
  // tries to expand the command and returns the success status
  bool ProcessTab();
  TPtrList<olxCommandEvent> PostponedEvents;
#ifdef _WIN32
  TStringToList<olxstr, HWND> loadedFiles;
  WXLRESULT MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam);
  static BOOL CALLBACK QueryOlex2Windows(HWND w, LPARAM p);
  void ListOlex2OpenedFiles();
  static const olxstr& GetFileQueryEvtName();
  static const olxstr& GetFileQueryFileName();
  static UINT GetFileQueryEvtId();
#endif
  size_t LoadedFileIdx(const olxstr& fn);
private:
  // macro functions
  DefMacro(Reap)
  DefMacro(Pict)
  DefMacro(Picta)
  DefMacro(PictPS)
  DefMacro(PictTEX)
  DefMacro(PictS)
  DefMacro(PictPR)
  DefMacro(Group)
  DefMacro(Clear)
  DefMacro(Rota)
  DefMacro(Listen)
  DefMacro(ListenCmd)
  DefMacro(WindowCmd)
  DefMacro(ProcessCmd)
  DefMacro(Wait)
  DefMacro(SwapBg)
  DefMacro(Silent)
  DefMacro(Stop)
  DefMacro(Echo)
  DefMacro(Post)
  DefMacro(Exit)
  DefMacro(SetEnv)
  DefMacro(Help)
  DefMacro(AddLabel)
  DefMacro(Hide)
  DefMacro(Exec)
  DefMacro(Shell)
  DefMacro(Save)
  DefMacro(Load)
  DefMacro(Link)
  DefMacro(Style)
  DefMacro(Scene)

  DefMacro(Lines)

  DefMacro(Ceiling)
  DefMacro(Fade)
  DefMacro(WaitFor)

  DefMacro(HtmlPanelSwap)
  DefMacro(HtmlPanelWidth)
  DefMacro(HtmlPanelVisible)
  DefMacro(QPeakScale)
  DefMacro(QPeakSizeScale)

  DefMacro(Focus)
  DefMacro(Refresh)

  DefMacro(Mode)

  DefMacro(Flush)
  DefMacro(ShowStr)

  DefMacro(Bind)

  DefMacro(Grad)

  DefMacro(EditAtom)
  DefMacro(EditIns)
  DefMacro(HklEdit)
  DefMacro(HklView)
  DefMacro(HklExtract)

  DefMacro(ViewGrid)

  DefMacro(Popup)

  DefMacro(Python)

  DefMacro(CreateMenu)
  DefMacro(DeleteMenu)
  DefMacro(EnableMenu)
  DefMacro(DisableMenu)
  DefMacro(CheckMenu)
  DefMacro(UncheckMenu)

  DefMacro(CreateShortcut)
  DefMacro(SetCmd)

  DefMacro(UpdateOptions)
  DefMacro(Update)
  DefMacro(Reload)
  DefMacro(StoreParam)
  DefMacro(SelBack)

  DefMacro(CreateBitmap)
  DefMacro(DeleteBitmap)
  DefMacro(Tref)
  DefMacro(Patt)

  DefMacro(InstallPlugin)
  DefMacro(SignPlugin)
  DefMacro(UninstallPlugin)

  DefMacro(UpdateFile)
  DefMacro(NextSolution)

  DefMacro(ShowWindow)

  DefMacro(Schedule)
  DefMacro(Test)

  DefMacro(IT)
  DefMacro(StartLogging)
  DefMacro(ViewLattice)
  DefMacro(AddObject)
  DefMacro(DelObject)

  DefMacro(ImportFrag)
  DefMacro(ExportFrag)

  DefMacro(OnRefine)
  DefMacro(TestMT)
  DefMacro(SetFont)
  DefMacro(EditMaterial)
  DefMacro(SetMaterial)
  DefMacro(ShowSymm)
  DefMacro(Textm)
  DefMacro(TestStat)
  DefMacro(ExportFont)
  DefMacro(ImportFont)
  DefMacro(UpdateQPeakTable)
  DefMacro(Capitalise)
  DefMacro(FlushFS)
  DefMacro(Elevate)
  DefMacro(Restart)
  DefMacro(ADPDisp)
  DefMacro(RegisterFonts)
    ////////////////////////////////////////////////////////////////////////////////
//////////////////////////////FUNCTIONS/////////////////////////////////////////
  DefFunc(FileLast)
  DefFunc(FileSave)
  DefFunc(FileOpen)
  DefFunc(ChooseDir)

  DefFunc(Strcat)
  DefFunc(Strcmp)
  DefFunc(GetEnv)

  DefFunc(Sel)
  DefFunc(Atoms)
  DefFunc(FPS)

  DefFunc(Cursor)
  DefFunc(RGB)
  DefFunc(Color)

  DefFunc(HtmlPanelWidth)
  DefFunc(LoadDll)

  DefFunc(Alert)

  DefFunc(ValidatePlugin)
  DefFunc(IsPluginInstalled)
  DefFunc(GetUserInput)
  DefFunc(GetUserStyledInput)
  DefFunc(TranslatePhrase)
  DefFunc(IsCurrentLanguage)
  DefFunc(CurrentLanguageEncoding)

  DefFunc(ChooseElement)
  DefFunc(ChooseFont)
  DefFunc(GetFont)
  DefFunc(ChooseMaterial)
  DefFunc(GetMaterial)
  DefFunc(GetMouseX)
  DefFunc(GetMouseY)
  DefFunc(GetWindowSize)
  DefFunc(IsOS)
  DefFunc(HasGUI)
  DefFunc(CheckState)
  DefFunc(GlTooltip)
  DefFunc(CurrentLanguage)
  DefFunc(GetMAC)
  DefFunc(ThreadCount)
  DefFunc(FullScreen)
  DefFunc(Freeze)
//..............................................................................
public:
  bool IsControl(const olxstr& cname) const;
  //............................................................................

  void OnKeyUp(wxKeyEvent& event);
  void OnKeyDown(wxKeyEvent& event);
  void OnChar(wxKeyEvent& event);
  void OnNavigation(wxNavigationKeyEvent& event);
  void OnIdle();

  virtual bool ProcessEvent(wxEvent& evt);
  void OnResize();
  olxstr StylesDir, // styles folder
    ScenesDir,
    DefStyle,         // default style file
    DefSceneP,        // default scene parameters file
    TutorialDir;
  TGlMaterial
    HelpFontColorCmd, HelpFontColorTxt,
    ExecFontColor, InfoFontColor,
    WarningFontColor, ErrorFontColor, ExceptionFontColor;
private:
  bool Dispatch(int MsgId, short MsgSubId, const IOlxObject *Sender,
    const IOlxObject *Data, TActionQueue *);
  olxstr FLastSettingsFile;

  class ProcessHandler : public ProcessManager::IProcessHandler  {
    TMainForm& parent;
    bool printed;
  public:
    ProcessHandler(TMainForm& _parent) : parent(_parent), printed(false)  {}
    virtual void BeforePrint();
    virtual void Print(const olxstr& line);
    virtual void AfterPrint();
    virtual void OnWait();
    virtual void OnTerminate(const AProcess& p);
  };
  ProcessHandler _ProcessHandler;
  ProcessManager* _ProcessManager;
  // class TIOExt* FIOExt;
  TTimer *FTimer;
  unsigned short FMode;
  uint64_t TimePerFrame,    // this is evaluated by FXApp->Draw()
           DrawSceneTimer;  // this is set for onTimer to check when the scene has to be drawn

  double FRotationIncrement, FRotationAngle;
  vec3d FRotationVector;

  vec3d FFadeVector; // stores: current position, end and increment

  olxstr FListenFile;

  olxstr FListenCmdFile; // file to listen for commands

  TStringToList<olxstr,wxMenuItem*> FRecentFiles;
  olxstr_dict<olxstr,true> Bindings;
  uint16_t FRecentFilesToShow;
  void UpdateRecentFile(const olxstr& FN);
  TGlOption FBgColor;
  class TCmdLine* FCmdLine;
  olxstr FHtmlIndexFile;

  bool FHtmlMinimized, FHtmlOnLeft, FBitmapDraw, FHtmlWidthFixed,
       RunOnceProcessed,
       StartupInitialised;
  bool InfoWindowVisible, HelpWindowVisible, CmdLineVisible, _UseGlTooltip;

  float FHtmlPanelWidth;

  bool UpdateRecentFilesTable(bool TableDef=true);
  void QPeakTable(bool TableDef=true, bool Create=true);
  void BadReflectionsTable(bool TableDef=true, bool Create=true);
  void RefineDataTable(bool TableDef=true, bool Create=true);

  TAccellList<olxstr> AccShortcuts;
  TAccellList<TMenuItem*> AccMenus;

  olxstr_dict<TMenu*, false> Menus;
  int32_t TranslateShortcut(const olxstr& sk);
  void SaveVFS(short persistenceId);
  void LoadVFS(short persistenceId);
  // this must be called at different times on GTK and windows
  void StartupInit();
  bool SkipSizing; // when size changed from the LoadSettings
  void DoUpdateFiles();
  // returns true if the thread is created
  bool CreateUpdateThread(bool force, bool reinstall, bool cleanup);
public:
  TMainForm(TGlXApp *Parent);
  virtual ~TMainForm();
  virtual bool Destroy();
  void LoadSettings(const olxstr &FN);
  void SaveSettings(const olxstr &FN);
  virtual const olxstr& GetScenesFolder() const {  return ScenesDir;  }
  virtual void SetScenesFolder(const olxstr &sf)  {  ScenesDir = sf;  }
  void UpdateUserOptions(const olxstr &option, const olxstr &value);

  // fires the state change as well
  void UseGlTooltip(bool v);

  float GetHtmlPanelWidth() const {  return FHtmlPanelWidth;  }
  time_t idle_time, idle_start;
  void OnNonIdle();
  //..............................................................................
// properties
protected:
  wxToolBar   *ToolBar;
  wxStatusBar *StatusBar;
  wxMenuBar   *MenuBar;
  // file menu
  TMenu *MenuFile;
  // view menu
  wxMenuItem *miHtmlPanel;
  //popup menu
  TMenu      *pmMenu;
    TMenu      *pmDrawStyle,  // submenues
                *pmModel,
                *pmDrawQ;
  TMenu    *pmAtom;
    TMenu    *pmBang;  // bonds angles
    TMenu    *pmAtomType;
    TMenu    *pmAtomOccu,
             *pmAtomConn,
             *pmAtomPoly,
             *pmAtomPart,
             *pmAtomUiso
             ;
  TMenu    *pmBond;
    TMenu    *pmTang;  // torsion angles
  TMenu    *pmFragment;
  TMenu    *pmSelection;
  TMenu    *pmView;
  TMenu    *pmPlane;
  TMenu    *pmGraphics;  // generic menu for graphics
  TMenu    *pmGrid, *pmBlob;
  TMenu    *pmLabel;
  TMenu    *pmLattice;
  class TXGlLabel* LabelToEdit;
  wxMenu  *FCurrentPopup;

  class FileDropTarget : public wxFileDropTarget {
    TMainForm& parent;
  public:
    FileDropTarget(TMainForm& _parent) : parent(_parent)  {}
    virtual bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames);
    virtual wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult def)  {
      return wxFileDropTarget::OnDragOver(x,y,wxDragCopy);
    }
  };
public:
  wxMenu* CurrentPopupMenu()    {  return FCurrentPopup; }
  wxMenu* DefaultPopup()        {  return pmGraphics; }
  wxMenu* GeneralPopup()        {  return pmMenu; }
//..............................................................................
// TMainForm interface
  void GlCanvas(TGlCanvas *GC)  {  FGlCanvas = GC;  }
  TGlCanvas * GlCanvas()  {  return FGlCanvas;  }
  void XApp(Olex2App *XA);
  Olex2App *XApp()  {  return FXApp; }
  bool FindXAtoms(const TStrObjList &Cmds, TXAtomPList& xatoms, bool GetAll, bool unselect);
  ConstPtrList<TXAtom> FindXAtoms(const TStrObjList &Cmds,bool GetAll, bool unselect)  {
    TXAtomPList atoms;
    FindXAtoms(Cmds, atoms, GetAll, unselect);
    return atoms;
  }
//..............................................................................
// General interface
//..............................................................................
  static TMainForm *&GetInstance() {
    static TMainForm *i = 0;
    return i;
  }
  static bool HasInstance() {
    return GetInstance() != 0 && !GetInstance()->Destroying;
  }
  void ObjectUnderMouse(AGDrawObject *G);
//..............................................................................
  DECLARE_CLASS(TMainForm)
};

#endif
