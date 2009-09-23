//............................................................................//
#ifndef _xl_mainformH
#define _xl_mainformH
//............................................................................//
#include "wx/wx.h"
#include "wx/process.h"
#include "wx/thread.h"

#include "globj.h"
#include "gxapp.h"
#include "ctrls.h"
#include "eprocess.h"
#include "undo.h"
//#include "framemaker.h"
#include "glconsole.h"
#include "datafile.h"
#include "gltextbox.h"

#include "lst.h"

#include "integration.h"
#include "macroerror.h"

#include "library.h"
#include "eaccell.h"
#include "estlist.h"
#include "langdict.h"
#include "updateth.h"

#define  ID_FILE0 100

#define  ID_GLDRAW            1000
#define  ID_TIMER             1001
#define  ID_INFO              1002
#define  ID_WARNING           1003
#define  ID_ERROR             1004
#define  ID_EXCEPTION         1005
#define  ID_ONLINK            1006
#define  ID_HTMLCMD           1007
#define  ID_HTMLDBLCLICK      1008
#define  ID_HTMLKEY           1009
#define  ID_PROCESSTERMINATE  1010
#define  ID_COMMAND           1011
#define  ID_XOBJECTSDESTROY   1012
#define  ID_CMDLINECHAR       1013
#define  ID_CMDLINEKEYDOWN    1014
#define  ID_TEXTPOST          1015

//............................................................................//
const unsigned short mListen = 0x0001,    // modes
                     mSilent = 0x0002,  // silent mode
                     mPick   = 0x0020,  // pick mode, for a future use
                     mFade   = 0x0080,  // structure fading ..
                     mRota   = 0x0100,  // rotation
                     mSolve  = 0x0200,  // structure solution
                     mSGDet  = 0x0400;  // space group determination

const int   fntConsole  = 0,
            fntHelp     = 1,
            fntNotes    = 2,
            fntLabels   = 3,
            fntPLabels  = 4;

// persistence level
const short plNone      = 0x0000,  // runtime data only - not saved at any moment
            plStructure = 0x0001,  // data saved/loaded when structure is un/loaded
            plGlobal    = 0x0002;  // data saved/loaded when olex is closed/executed
/*
const unsigned int   psFileLoaded        = pfSpecialCheckA,
                     psCheckFileTypeIns  = pfSpecialCheckB,
                     psCheckFileTypeCif  = pfSpecialCheckC,
*/
class TMainForm;
class TGlXApp;

//............................................................................//
struct TPopupData  {
  wxDialog *Dialog;
  class THtml *Html;
  olxstr OnDblClick;
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
  virtual const olxstr& getVar(const olxstr &name, const olxstr &defval=NullString) const;
  virtual void setVar(const olxstr &name, const olxstr &val) const;
  virtual TStrList GetPluginList() const;

  void CallbackFunc(const olxstr& cbEvent, const olxstr& param);
  void CallbackFunc(const olxstr& cbEvent, TStrObjList& params);
  TCSTypeList<olxstr, ABasicFunction*> CallbackFuncs;
protected:
  bool Destroying;
  UpdateThread* _UpdateThread;
	TOnProgress* UpdateProgress, *ActionProgress;
  TEFile* ActiveLogFile;
  static void PyInit();
  TActionQList FActionList;
  TGlXApp *FParent;
  TArrayList< AnAssociation2<TDUnitCell*, TSpaceGroup*> > UserCells;
  TCSTypeList<olxstr, olxstr> StoredParams;

  TTypeList<TScheduledTask> Tasks;

  TSStrPObjList<olxstr,TPopupData*, true> FPopups;
  class TGlCanvas *FGlCanvas;
  TGXApp *FXApp;
  TDataFile FHelpFile, FMacroFile, FPluginFile;
  TDataItem *FHelpItem, *FMacroItem, *FPluginItem;

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

  void ProcessXPMacro(const olxstr &Cmd, TMacroError &Error, bool ProcessFunctions=true, bool ClearMacroError = true);
  // decodes Cmd and creates a puts the list of commands to Cmds and also to
  //FOnTerminateCmds and FOnListenCmds
  void DecodeParams(TStrObjList &Cmds, const olxstr &Cmd);
  // substitutes arguments with values, also fields like $filename$ are substituted
  void SubstituteArguments(olxstr &Cmd, TStrList &PName, TStrList &PVal);
  void PreviewHelp(const olxstr& Cmd);
  olxstr ExpandCommand(const olxstr &Cmd);
  int MouseMoveTimeElapsed, MousePositionX, MousePositionY;
  // click-name states
  uint32_t ProgramState;

  class TModes *Modes;

   // solution mode variables
  TTypeList<long> Solutions;
  int CurrentSolution;
  olxstr SolutionFolder;
  void ChangeSolution( int sol );

  // helper functions ...
  void CallMatchCallbacks(TNetwork& netA, TNetwork& netB, double RMS);

  TLst Lst;
public:
  bool ProcessMacroFunc(olxstr &Cmd);
  void OnMouseMove(int x, int y);
  void OnMouseWheel(int x, int y, double delta);
  bool OnMouseDown(int x, int y, short Flags, short Buttons);
  bool OnMouseUp(int x, int y, short Flags, short Buttons);
  bool OnMouseDblClick(int x, int y, short Flags, short Buttons);
  virtual bool Show( bool v );
  TActionQueue *OnModeChange, *OnStateChange;

  void SetUserCursor( const olxstr& param, const olxstr& mode );

  inline TUndoStack* GetUndoStack()  {  return FUndoStack;  }

  void SetProgramState(bool val, uint32_t state, const olxstr& data );
  bool CheckMode(unsigned short mode, const olxstr& modeData);
  bool CheckState(uint32_t state, const olxstr& stateData);

protected:
  void PostCmdHelp(const olxstr &Cmd, bool Full=false);
  void AnalyseError( TMacroError& error );

  void OnSize(wxSizeEvent& event);

  void OnQuit(wxCommandEvent& event);
  void OnAbout(wxCommandEvent& event);
  void OnFileOpen(wxCommandEvent& event);

  void OnGenerate(wxCommandEvent& event);

  void OnDrawStyleChange(wxCommandEvent& event);

  void OnDrawQChange(wxCommandEvent& event);

  void OnViewAlong(wxCommandEvent& event);

  void OnInternalIdle();

  friend class TObjectVisibilityChange;
  void BasisVChange();
  void CellVChange();
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
  int GetFragmentList(TNetPList& res);

  void OnAtomTypeChange(wxCommandEvent& event);
  void OnAtomOccuChange(wxCommandEvent& event);
  void OnAtomConnChange(wxCommandEvent& event);
  void OnAtomPolyChange(wxCommandEvent& event);
  void OnAtomTypePTable(wxCommandEvent& event);
  void OnAtom(wxCommandEvent& event); // general handler

  void OnPlane(wxCommandEvent& event); // general handler

  void OnSelection(wxCommandEvent& event);

  void OnGraphicsStyle(wxCommandEvent& event);
  
  void OnPictureExport(wxCommandEvent& event);

  // view menu
  void OnHtmlPanel(wxCommandEvent& event);
  // macro functions
private:

  DefMacro(Reap)
  DefMacro(Pict)
  DefMacro(Picta)
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
  DefMacro(Activate)
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
  DefMacro(Omit)
  DefMacro(Exec)
  DefMacro(Shell)
  DefMacro(Save)
  DefMacro(Load)
  DefMacro(Link)
  DefMacro(Style)
  DefMacro(Scene)
  DefMacro(SyncBC)

  DefMacro(IF)
  DefMacro(Basis)
  DefMacro(Lines)

  DefMacro(Ceiling)
  DefMacro(Fade)
  DefMacro(WaitFor)

  DefMacro(HtmlPanelSwap)
  DefMacro(HtmlPanelWidth)
  DefMacro(HtmlPanelVisible)
  DefMacro(QPeakScale)

  DefMacro(CalcChn)
  DefMacro(CalcMass)

  DefMacro(Label)

  DefMacro(Focus)
  DefMacro(Refresh)

  DefMacro(Move)

  DefMacro(ShowH)

  DefMacro(Fvar)
  DefMacro(Sump)
  DefMacro(Part)
  DefMacro(Afix)

  DefMacro(Degen)
  DefMacro(SwapExyz)
  DefMacro(AddExyz)

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
  DefMacro(Reset)

  DefMacro(Text)
  DefMacro(ShowStr)

  DefMacro(Bind)

  DefMacro(Grad)
  DefMacro(Split)
  DefMacro(ShowP)

  DefMacro(EditAtom)
  DefMacro(EditIns)
  DefMacro(EditHkl)
  DefMacro(ViewHkl)
  DefMacro(ExtractHkl)
  DefMacro(AppendHkl)
  DefMacro(ExcludeHkl)
  DefMacro(MergeHkl)
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

  DefMacro(DelOFile)
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
  DefMacro(PictPS)
  DefMacro(PictTEX)
  DefMacro(UpdateQPeakTable)
  DefMacro(Conn)
  DefMacro(AddBond)
  DefMacro(DelBond)
  DefMacro(SAME)
  DefMacro(RESI)
  DefMacro(WBox)
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
  DefFunc(Crd)
  DefFunc(CCrd)
  DefFunc(Env)
  DefFunc(FPS)

  DefFunc(Cursor)
  DefFunc(RGB)
  DefFunc(Color)

  DefFunc(Lst)
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

  DefFunc(And)
  DefFunc(Or)
  DefFunc(Not)

  DefFunc(ChooseElement)
  DefFunc(SfacList)
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

  TUndoStack *FUndoStack;
//..............................................................................
public:
  const olxstr&  TranslatePhrase(const olxstr& phrase);
  virtual TLibrary& GetLibrary()  {  return FXApp->GetLibrary();  }
  virtual olxstr TranslateString(const olxstr& str) const;
  virtual bool IsControl(const olxstr& cname) const;
  virtual void LockWindowDestruction(wxWindow* wnd);
  virtual void UnlockWindowDestruction(wxWindow* wnd);
public:

  void OnKeyUp(wxKeyEvent& event);
  void OnKeyDown(wxKeyEvent& event);
  void OnChar(wxKeyEvent& event);
  void OnNavigation(wxNavigationKeyEvent& event);

  virtual bool ProcessEvent( wxEvent& evt );
  void OnResize();
  olxstr StylesDir, // styles folder
    SParamDir,        // scene parameters folder
    DefStyle,         // default style file
    DefSceneP,        // default scene parameters file
    DataDir,
    TutorialDir,
    PluginFile;
  TGlMaterial HelpFontColorCmd, HelpFontColorTxt,
              ExecFontColor, InfoFontColor,
              WarningFontColor, ErrorFontColor, ExceptionFontColor;
              

private:
  bool Dispatch( int MsgId, short MsgSubId, const IEObject *Sender, const IEObject *Data=NULL);
  olxstr FLastSettingsFile;
  AProcess* FProcess;
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
  short FRecentFilesToShow;
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
  void SetProcess( AProcess *Process );
  void LoadSettings(const olxstr &FN);
  void SaveSettings(const olxstr &FN);
  void LoadScene(TDataItem *Root, TGlLightModel *FLM=NULL);
  void SaveScene(TDataItem *Root, TGlLightModel *FLM=NULL);

  // fires the state change as well
  void UseGlTooltip(bool v);

  const olxstr& GetStructureOlexFolder();
  float GetHtmlPanelWidth() const  {  return FHtmlPanelWidth;  }
  inline THtml* GetHtml()  const {  return FHtml; }
  THtml* FindHtml(const olxstr& popupName) const;
  TPopupData* FindHtmlEx(const olxstr& popupName) const;
  inline const olxstr& GetCurrentLanguageEncodingStr()  const  {
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
    wxMenuItem *miAtomGrowShell;
    wxMenuItem *miAtomGrowFrag;
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
    wxMenuItem  *miGroupSel;
    wxMenuItem  *miUnGroupSel;
  TMenu    *pmView;
  TMenu    *pmPlane;
  TMenu    *pmGraphics;  // general menu for graphics
  TMenu*    pmLabel;
  TMenu*    pmLattice;
  class TXGlLabel* LabelToEdit;

  wxMenu  *FCurrentPopup;
public:
  wxMenu* CurrentPopupMenu()    {  return FCurrentPopup; }
  wxMenu* DefaultPopup()        {  return pmGraphics; }
  wxMenu* GeneralPopup()        {  return pmMenu; }
//..............................................................................
// TMainForm interface
  void GlCanvas( TGlCanvas *GC) {  FGlCanvas = GC;  }
  TGlCanvas * GlCanvas()        {  return FGlCanvas;  }
  void XApp( TGXApp *XA);
  TGXApp *XApp()         {  return FXApp; }
  bool FindXAtoms(const TStrObjList &Cmds, TXAtomPList& xatoms, bool GetAll, bool unselect);
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
