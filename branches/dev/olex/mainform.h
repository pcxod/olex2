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
#include "wx/wx.h"
#include "wx/dnd.h"
#include "wx/process.h"
#include "wx/thread.h"
#include "gxapp.h"
#include "ctrls.h"
#include "eprocess.h"
#include "undo.h"
#include "glconsole.h"
#include "datafile.h"
#include "gltextbox.h"
#include "integration.h"
#include "macroerror.h"
#include "library.h"
#include "eaccell.h"
#include "estlist.h"
#include "langdict.h"
#include "updateth.h"
#include "macrolib.h"
#include "exparse/exptree.h"
#include "nui/nui.h"

#define  ID_FILE0 100

enum  {
  ID_GLDRAW = 1000,
  ID_TIMER,
  ID_INFO,
  ID_WARNING,
  ID_ERROR,
  ID_EXCEPTION,
  ID_ONLINK,
  ID_HTMLKEY,
  ID_COMMAND,
  ID_XOBJECTSDESTROY,
  ID_CMDLINECHAR,
  ID_CMDLINEKEYDOWN,
  ID_TEXTPOST
};

enum  {
  ID_HtmlPanel=1,  // view menu

  ID_StrGenerate,  // structure menu

  ID_MenuTang,  // menu item ids
  ID_MenuBang,
  ID_MenuGraphics,
  ID_MenuModel,
  ID_MenuView,
  ID_MenuFragment,
  ID_MenuDrawStyle,
  ID_MenuDrawQ,
  ID_MenuItemAtomInfo,
  ID_MenuItemBondInfo,
  ID_MenuAtomType,
  ID_MenuAtomOccu,
  ID_MenuAtomConn,
  ID_MenuAtomPoly,

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

  ID_AtomTypeChangeC,
  ID_AtomTypeChangeN,
  ID_AtomTypeChangeO,
  ID_AtomTypeChangeF,
  ID_AtomTypeChangeH,
  ID_AtomTypeChangeS,
  ID_AtomTypePTable,
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

  ID_View100,   // view menu
  ID_View010,
  ID_View001,
  ID_View110,
  ID_View101,
  ID_View011,
  ID_View111,

  ID_AtomOccu1,
  ID_AtomOccu34,
  ID_AtomOccu12,
  ID_AtomOccu13,
  ID_AtomOccu14,
  ID_AtomOccuFix,
  ID_AtomOccuFree,

  ID_AtomConn0,
  ID_AtomConn1,
  ID_AtomConn2,
  ID_AtomConn3,
  ID_AtomConn4,
  ID_AtomConn12,

  ID_AtomPolyNone,
  ID_AtomPolyAuto,
  ID_AtomPolyRegular,
  ID_AtomPolyPyramid,
  ID_AtomPolyBipyramid,

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

  ID_GStyleSave,
  ID_GStyleOpen,
  ID_FixLattice,
  ID_FreeLattice,
  ID_DELINS,
  ID_ADDINS,
  ID_VarChange,

  ID_gl2ps,
  
  ID_PictureExport,
  ID_UpdateThreadTerminate,
  ID_UpdateThreadDownload,
  ID_UpdateThreadAction
};

//............................................................................//
const unsigned short
  mListen = 0x0001,    // modes
  mSilent = 0x0002,  // silent mode
  mPick   = 0x0020,  // pick mode, for a future use
  mFade   = 0x0080,  // structure fading ..
  mRota   = 0x0100,  // rotation
  mSolve  = 0x0200,  // structure solution
  mSGDet  = 0x0400;  // space group determination

// persistence level
const short
  plNone      = 0x0000,  // runtime data only - not saved at any moment
  plStructure = 0x0001,  // data saved/loaded when structure is un/loaded
  plGlobal    = 0x0002;  // data saved/loaded when olex is closed/executed

class TMainForm;
class TGlXApp;

//............................................................................//
struct TPopupData  {
  TDialog *Dialog;
  class THtml *Html;
};
//............................................................................//
struct TScheduledTask  {
  bool Repeatable;
  olxstr Task;
  long Interval, LastCalled;
};
//............................................................................//
class TMainForm: public TMainFrame, public AEventsDispatcher,
  public olex::IOlexProcessor
{
  //TFrameMaker FrameMaker;
public:
  virtual bool executeMacroEx(const olxstr& function, TMacroError& ar);
  virtual void print(const olxstr& function, const short MessageType = olex::mtNone);
  virtual bool executeFunction(const olxstr& function, olxstr& retVal);
  virtual IEObject* executeFunction(const olxstr& function);
  virtual bool registerCallbackFunc(const olxstr& cbEvent, ABasicFunction* fn);
  virtual void unregisterCallbackFunc(const olxstr& cbEvent, const olxstr& funcName);
  virtual const olxstr& getDataDir() const;
  virtual const olxstr& getVar(const olxstr& name, const olxstr& defval=EmptyString()) const;
  virtual void setVar(const olxstr& name, const olxstr& val) const;
  virtual TStrList GetPluginList() const;

  void CallbackFunc(const olxstr& cbEvent, const olxstr& param);
  void CallbackFunc(const olxstr& cbEvent, TStrObjList& params);
  TCSTypeList<olxstr, ABasicFunction*> CallbackFuncs;
protected:
  bool Destroying;
  TStack<AnAssociation2<wxCursor,wxString> > CursorStack;
  UpdateThread* _UpdateThread;
	TOnProgress* UpdateProgress, *ActionProgress;
  TEFile* ActiveLogFile;
  static void PyInit();
  TActionQList Action;
  TGlXApp* FParent;
  TArrayList< AnAssociation2<TDUnitCell*, TSpaceGroup*> > UserCells;
  TCSTypeList<olxstr, olxstr> StoredParams;

  TTypeList<TScheduledTask> Tasks;

  TSStrPObjList<olxstr,TPopupData*, true> FPopups;
  class TGlCanvas *FGlCanvas;
  TGXApp* FXApp;
  TDataFile FHelpFile, FMacroFile, FPluginFile;
  TDataItem *FHelpItem, *FPluginItem;
  
  TEMacroLib Macros;

  olxstr DictionaryFile, GradientPicture;
  TLangDict Dictionary;

  TGlConsole *FGlConsole;
  TGlTextBox *FHelpWindow, *FInfoBox, *GlTooltip;
  TStrList FOnTerminateMacroCmds; // a list of commands called when a process is terminated
  TStrList FOnAbortCmds;           // a "stack" of macroses, called when macro terminated
  TStrList FOnListenCmds;  // a list of commands called when a file is changed by another process
  TMacroError MacroError;
  
  olxstr Tooltip;
  void AquireTooltipValue();

  void ClearPopups();
  TPopupData* GetPopup(const olxstr& name);

  void PreviewHelp(const olxstr& Cmd);
  olxstr ExpandCommand(const olxstr &Cmd, bool inc_files);
  int MouseMoveTimeElapsed, MousePositionX, MousePositionY;
  // click-name states
  uint32_t ProgramState;

  class TModes *Modes;

   // solution mode variables
  TTypeList<long> Solutions;
  int CurrentSolution;
  olxstr SolutionFolder;
  void ChangeSolution(int sol);

  // helper functions ...
  void CallMatchCallbacks(TNetwork& netA, TNetwork& netB, double RMS);
  void UpdateInfoBox();
  olx_nui::INUI *nui_interface;
public:
  bool ProcessFunction(olxstr &cmd, const olxstr& location=EmptyString(), bool quiet=false) {  
    TMacroError err;
    err.SetLocation(location);
    //cmd = exparse::parser_util::unescape(cmd);
    const bool rv = Macros.ProcessFunction(cmd, err, false);  
    AnalyseErrorEx(err, quiet);
    return rv;
  }
  bool ProcessMacro(const olxstr& cmd, const olxstr& location=EmptyString())  {
    TMacroError err;
    err.SetLocation(location);
    Macros.ProcessTopMacro(cmd, err, *this, &TMainForm::AnalyseError);
    return err.IsSuccessful();
  }
  void OnMouseMove(int x, int y);
  void OnMouseWheel(int x, int y, double delta);
  bool OnMouseDown(int x, int y, short Flags, short Buttons);
  bool OnMouseUp(int x, int y, short Flags, short Buttons);
  bool OnMouseDblClick(int x, int y, short Flags, short Buttons);
  virtual bool Show( bool v );
  TActionQueue &OnModeChange, &OnStateChange;

  void SetUserCursor(const olxstr& param, const olxstr& mode);

  inline TUndoStack* GetUndoStack()  {  return FUndoStack;  }

  const TModes& GetModes() const {  return *Modes;  }

  void SetProgramState(bool val, uint32_t state, const olxstr& data );
  bool CheckMode(size_t mode, const olxstr& modeData);
  bool CheckState(uint32_t state, const olxstr& stateData);
  bool PopupMenu(wxMenu* menu, const wxPoint& p=wxDefaultPosition);
  bool PopupMenu(wxMenu* menu, int x, int y)  {  return PopupMenu(menu, wxPoint(x,y));  }
protected:
  void PostCmdHelp(const olxstr &Cmd, bool Full=false);
  void AnalyseErrorEx(TMacroError& error, bool queit=false);
  void AnalyseError(TMacroError& error)  {  AnalyseErrorEx(error);  }

  void OnSize(wxSizeEvent& event);

  void OnQuit(wxCommandEvent& event);
  void OnFileOpen(wxCommandEvent& event);
  void OnGenerate(wxCommandEvent& event);
  void OnDrawStyleChange(wxCommandEvent& event);
  void OnDrawQChange(wxCommandEvent& event);
  void OnViewAlong(wxCommandEvent& event);
  void OnInternalIdle();

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
  void OnAtomPolyChange(wxCommandEvent& event);
  void OnAtomTypePTable(wxCommandEvent& event);
  void OnAtom(wxCommandEvent& event); // general handler

  void OnBond(wxCommandEvent& event);
  void OnPlane(wxCommandEvent& event); // general handler

  void OnSelection(wxCommandEvent& event);
  void OnGraphicsStyle(wxCommandEvent& event);
  void OnPictureExport(wxCommandEvent& event);

  // view menu
  void OnHtmlPanel(wxCommandEvent& event);
  bool ImportFrag(const olxstr& line);
  // macro functions
private:

  DefMacro(Reap)
  DefMacro(Pict)
  DefMacro(Picta)
  DefMacro(PictPS)
  DefMacro(PictTEX)
  DefMacro(PictS)
  DefMacro(PictPR)
  DefMacro(Bang)
  DefMacro(Grow)
  DefMacro(Uniq)
  DefMacro(Group)
  DefMacro(Fmol)
  DefMacro(Clear)
  DefMacro(Cell)
  DefMacro(Rota)
  DefMacro(Listen)
  DefMacro(WindowCmd)
  DefMacro(ProcessCmd)
  DefMacro(Wait)
  DefMacro(SwapBg)
  DefMacro(Silent)
  DefMacro(Stop)
  DefMacro(Echo)
  DefMacro(Post)
  DefMacro(Exit)
  DefMacro(Pack)
  DefMacro(Sel)
  DefMacro(Esd)
  DefMacro(Name)
  DefMacro(TelpV)
  DefMacro(Labels)
  DefMacro(SetEnv)
  DefMacro(SetView)
  DefMacro(Info)
  DefMacro(Help)
  DefMacro(Matr)
  DefMacro(Qual)
  DefMacro(Line)
  DefMacro(AddLabel)
  DefMacro(Mpln)
  DefMacro(Cent)
  DefMacro(Mask)
  DefMacro(ARad)
  DefMacro(ADS)
  DefMacro(AZoom)
  DefMacro(BRad)
  DefMacro(Hide)
  DefMacro(Kill)
  DefMacro(UpdateWght)
  DefMacro(Exec)
  DefMacro(Shell)
  DefMacro(Save)
  DefMacro(Load)
  DefMacro(Link)
  DefMacro(Style)
  DefMacro(Scene)

  DefMacro(Basis)
  DefMacro(Lines)

  DefMacro(Ceiling)
  DefMacro(Fade)
  DefMacro(WaitFor)

  DefMacro(HtmlPanelSwap)
  DefMacro(HtmlPanelWidth)
  DefMacro(HtmlPanelVisible)
  DefMacro(QPeakScale)
  DefMacro(QPeakSizeScale)

  DefMacro(Label)

  DefMacro(Focus)
  DefMacro(Refresh)

  DefMacro(Move)

  DefMacro(ShowH)

  DefMacro(Fvar)
  DefMacro(Sump)
  DefMacro(Part)
  DefMacro(Afix)

  DefMacro(Dfix)
  DefMacro(Dang)
  DefMacro(Tria)
  DefMacro(Sadi)
  DefMacro(RRings)
  DefMacro(Flat)
  DefMacro(Chiv)
  DefMacro(SIMU)
  DefMacro(DELU)
  DefMacro(ISOR)

  DefMacro(ShowQ)
  DefMacro(Mode)

  DefMacro(Text)
  DefMacro(ShowStr)

  DefMacro(Bind)

  DefMacro(Grad)
  DefMacro(Split)
  DefMacro(ShowP)

  DefMacro(EditAtom)
  DefMacro(EditIns)
  DefMacro(HklEdit)
  DefMacro(HklView)
  DefMacro(HklExtract)
  DefMacro(Direction)

  DefMacro(ViewGrid)

  DefMacro(Undo)

  DefMacro(Individualise)
  DefMacro(Collectivise)

  DefMacro(Popup)

  DefMacro(Delta)
  DefMacro(DeltaI)

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
  DefMacro(Reload)
  DefMacro(StoreParam)
  DefMacro(SelBack)

  DefMacro(CreateBitmap)
  DefMacro(DeleteBitmap)
  DefMacro(Tref)
  DefMacro(Patt)
  DefMacro(Export)

  DefMacro(InstallPlugin)
  DefMacro(SignPlugin)
  DefMacro(UninstallPlugin)

  DefMacro(UpdateFile)
  DefMacro(NextSolution)

  DefMacro(Match)

  DefMacro(ShowWindow)

  DefMacro(OFileDel)
  DefMacro(OFileSwap)
  DefMacro(CalcVol)

  DefMacro(Schedule)
  DefMacro(Tls)
  DefMacro(Test)

  DefMacro(LstRes)
  DefMacro(CalcVoid)
  DefMacro(Sgen)
  DefMacro(LstSymm)
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
  DefMacro(LstGO)
  DefMacro(CalcPatt)
  DefMacro(CalcFourier)
  DefMacro(TestBinding)
  DefMacro(ShowSymm)
  DefMacro(Textm)
  DefMacro(TestStat)
  DefMacro(ExportFont)
  DefMacro(ImportFont)
  DefMacro(ProjSph)
  DefMacro(UpdateQPeakTable)
  DefMacro(Conn)
  DefMacro(AddBond)
  DefMacro(DelBond)
  DefMacro(SAME)
  DefMacro(RESI)
  DefMacro(WBox)
  DefMacro(Center)
  DefMacro(Capitalise)
////////////////////////////////////////////////////////////////////////////////
//////////////////////////////FUNCTIONS/////////////////////////////////////////
  DefFunc(FileLast)
  DefFunc(FileSave)
  DefFunc(FileOpen)
  DefFunc(ChooseDir)

  DefFunc(Cell)
  DefFunc(Cif)
  DefFunc(P4p)
  DefFunc(Crs)
  DefFunc(DataDir)
  DefFunc(Strcat)
  DefFunc(Strcmp)
  DefFunc(GetEnv)

  DefFunc(Eval)
  DefFunc(UnsetVar)
  DefFunc(SetVar)
  DefFunc(GetVar)
  DefFunc(IsVar)
  DefFunc(VVol)

  DefFunc(Sel)
  DefFunc(Atoms)
  DefFunc(Env)
  DefFunc(FPS)

  DefFunc(Cursor)
  DefFunc(RGB)
  DefFunc(Color)

  DefFunc(Zoom)
  DefFunc(HtmlPanelWidth)
  #ifdef __WIN32__
  DefFunc(LoadDll)
  #endif

  DefFunc(CmdList)
  DefFunc(Alert)

  DefFunc(ValidatePlugin)
  DefFunc(IsPluginInstalled)
  DefFunc(GetUserInput)
  DefFunc(GetCompilationInfo)
  DefFunc(TranslatePhrase)
  DefFunc(IsCurrentLanguage)
  DefFunc(CurrentLanguageEncoding)

  DefFunc(SGList)

  DefFunc(ChooseElement)
  DefFunc(StrDir)
  DefFunc(ChooseFont)
  DefFunc(GetFont)
  DefFunc(ChooseMaterial)
  DefFunc(GetMaterial)
  DefFunc(GetMouseX)
  DefFunc(GetMouseY)
  DefFunc(GetWindowSize)
  DefFunc(IsOS)
  DefFunc(ExtraZoom)
  DefFunc(HasGUI)
  DefFunc(CheckState)
  DefFunc(GlTooltip)
  DefFunc(CurrentLanguage)
  DefFunc(GetMAC)
  DefFunc(ThreadCount)
  DefFunc(FullScreen)
  DefFunc(MatchFiles)
  DefFunc(Freeze)

  TUndoStack *FUndoStack;
//..............................................................................
public:
  const olxstr&  TranslatePhrase(const olxstr& phrase);
  virtual TLibrary& GetLibrary()  {  return FXApp->GetLibrary();  }
  virtual olxstr TranslateString(const olxstr& str) const;
  virtual bool IsControl(const olxstr& cname) const;
  virtual void LockWindowDestruction(wxWindow* wnd, const IEObject* caller);
  virtual void UnlockWindowDestruction(wxWindow* wnd, const IEObject* caller);

  void OnKeyUp(wxKeyEvent& event);
  void OnKeyDown(wxKeyEvent& event);
  void OnChar(wxKeyEvent& event);
  void OnNavigation(wxNavigationKeyEvent& event);

  virtual bool ProcessEvent( wxEvent& evt );
  void OnResize();
  olxstr StylesDir, // styles folder
    ScenesDir, 
    DefStyle,         // default style file
    DefSceneP,        // default scene parameters file
    DataDir,
    TutorialDir,
    PluginFile;
  TGlMaterial
    HelpFontColorCmd, HelpFontColorTxt,
    ExecFontColor, InfoFontColor,
    WarningFontColor, ErrorFontColor, ExceptionFontColor;
private:
  bool Dispatch( int MsgId, short MsgSubId, const IEObject *Sender, const IEObject *Data=NULL);
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

  /* internal function - sets/gets list of proposed space groups  */
  const olxstr& GetSGList() const;
  void SetSGList(const olxstr &sglist);

  TStrPObjList<olxstr,wxMenuItem*> FRecentFiles;
  TSStrStrList<olxstr,true> Bindings;
  uint16_t FRecentFilesToShow;
  void UpdateRecentFile(const olxstr& FN);
  TGlOption FBgColor;
  THtml* FHtml;
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

  TSStrPObjList<olxstr,TMenu*, false> Menus;
  int32_t TranslateShortcut(const olxstr& sk);
  void SaveVFS(short persistenceId);
  void LoadVFS(short persistenceId);
  // this must be called at different times on GTK and windows
  void StartupInit();
  bool SkipSizing; // when size changed from the LoadSettings
  void DoUpdateFiles();
public:
  TMainForm(TGlXApp *Parent);
  virtual ~TMainForm();
  virtual bool Destroy();
  void LoadSettings(const olxstr &FN);
  void SaveSettings(const olxstr &FN);
  virtual const olxstr& GetScenesFolder() const {  return ScenesDir;  }
  virtual void SetScenesFolder(const olxstr &sf)  {  ScenesDir = sf;  }
  virtual void LoadScene(const TDataItem& Root, TGlLightModel &FLM);
  virtual void SaveScene(TDataItem& Root, const TGlLightModel &FLM) const;

  // fires the state change as well
  void UseGlTooltip(bool v);

  const olxstr& GetStructureOlexFolder();
  float GetHtmlPanelWidth() const  {  return FHtmlPanelWidth;  }
  inline THtml* GetHtml()  const {  return FHtml; }
  THtml* FindHtml(const olxstr& popupName) const;
  TPopupData* FindHtmlEx(const olxstr& popupName) const;
  inline const olxstr& GetCurrentLanguageEncodingStr() const {
    return Dictionary.GetCurrentLanguageEncodingStr();
  }
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
    wxMenuItem *miAtomInfo;
    wxMenuItem *miAtomGrow;
    TMenu    *pmBang;  // bonds angles
    TMenu    *pmAtomType;
    TMenu    *pmAtomOccu, 
             *pmAtomConn,
             *pmAtomPoly;
  TMenu    *pmBond;
    wxMenuItem *miBondInfo;
    TMenu    *pmTang;  // torsion angles
  TMenu    *pmFragment;
    wxMenuItem *miFragGrow;
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
  void XApp( TGXApp *XA);
  TGXApp *XApp()  {  return FXApp; }
  bool FindXAtoms(const TStrObjList &Cmds, TXAtomPList& xatoms, bool GetAll, bool unselect);
  ConstPtrList<TXAtom> FindXAtoms(const TStrObjList &Cmds,bool GetAll, bool unselect)  {
    TXAtomPList atoms;
    FindXAtoms(Cmds, atoms, GetAll, unselect);
    return atoms;
  }
//..............................................................................
// General interface
//..............................................................................
// actions
  void ObjectUnderMouse( AGDrawObject *G);
//..............................................................................
  DECLARE_CLASS(TMainForm)
  DECLARE_EVENT_TABLE()
};

#endif
