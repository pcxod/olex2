//----------------------------------------------------------------------------//
// main frame of the application
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

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
  Library.RegisterMacro( new TMacro<TMainForm>(this, &TMainForm::mac##realMacroName, #macroName, #validOptions, argc, desc))

#include "wx/utils.h"
#include "wx/clipbrd.h"
#include "wx/wxhtml.h"
#include "wx/image.h"
#include "wx/panel.h"
#include "wx/fontdlg.h"
#include "wx/tooltip.h"
//#include "wx/msw/regconf.h"
#include "mainform.h"
#include "xglcanv.h"
#include "xglapp.h"
#include "dgenopt.h"
#include "matprop.h"
#include "ptable.h"
#include "scenep.h"
#include "wxglscene.h"
#include "primtvs.h"

#include "gpcollection.h"
#include "glgroup.h"
#include "glbackground.h"

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

#include "efile.h"

#include "htmlext.h"

//#include "ioext.h"
#include "pyext.h"

#include "obase.h"
#include "glbitmap.h"
#include "log.h"
#include "cmdline.h"

#include "shellutil.h"
#include "fsext.h"
#include "etime.h"

#include "egc.h"
#include "gllabel.h"
#include "xlattice.h"
#include "xgrid.h"

#include "olxvar.h"
#include "edit.h"

#include "xmacro.h"
#include "utf8file.h"

//#include <crtdbg.h>


#ifdef __GNUC__
  #undef Bool
#endif

  IMPLEMENT_CLASS(TMainForm, TMainFrame)

  const olxstr ProcessOutputCBName("procout");

enum
{
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

  ID_About, // help menu

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
  ID_AtomGrowShells,
  ID_AtomGrowFrags,
  
  ID_PlaneActivate,

  ID_FragmentHide,  // fragment menu
  ID_FragmentShowOnly,

  ID_View100,   // vew menu
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

  ID_Selection,
  ID_SelGroup,
  ID_SelUnGroup,

  ID_GraphicsHide,
  ID_GraphicsDS,
  ID_GraphicsP,
  ID_GraphicsEdit,

  ID_GStyleSave,
  ID_GStyleOpen,
  ID_FixLattice,
  ID_FreeLattice
};

class TObjectVisibilityChange: public AActionHandler
{
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
      FParent->ProcessXPMacro( olxstr("showwindow info false"), FParent->MacroError );
    else  if( Obj == FParent->FHelpWindow )
      FParent->ProcessXPMacro( olxstr("showwindow help false"), FParent->MacroError );
    return true;
  }
};
/******************************************************************************/
class TAtomModeUndo : public TUndoData {
  TXAtom* Atom;
  double Occu, Uiso;
  int Part;
  evecd FixedValues;
public:
  TAtomModeUndo(IUndoAction* action, TXAtom* XA) : TUndoData(action)  {
    Atom = XA;
    FixedValues = XA->Atom().CAtom().FixedValues();
    Occu = XA->Atom().CAtom().GetOccp();
    Uiso = XA->Atom().CAtom().GetUiso();
    Part = XA->Atom().CAtom().GetPart();
  }
  inline TXAtom* GetAtom() const {  return Atom;  }
  inline double GetUiso()  const {  return Uiso;  }
  inline double GetOccu() const  {  return Occu;  }
  inline int GetPart()    const  {  return Part;  }
  inline const evecd& GetFixedValues()  const  {  return FixedValues;  }
  
};

//----------------------------------------------------------------------------//
// TMainForm function bodies
//----------------------------------------------------------------------------//
BEGIN_EVENT_TABLE(TMainForm, wxFrame)  // basic interface
  EVT_SIZE(TMainForm::OnSize)
  EVT_MENU(ID_About, TMainForm::OnAbout)
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
  EVT_MENU(ID_GraphicsDS, TMainForm::OnGraphics)
  EVT_MENU(ID_GraphicsP, TMainForm::OnGraphics)
  EVT_MENU(ID_GraphicsEdit, TMainForm::OnGraphics)
  EVT_MENU(ID_FixLattice, TMainForm::OnGraphics)
  EVT_MENU(ID_FreeLattice, TMainForm::OnGraphics)

  EVT_MENU(ID_FragmentHide, TMainForm::OnFragmentHide)
  EVT_MENU(ID_FragmentShowOnly, TMainForm::OnFragmentShowOnly)

  EVT_MENU(ID_AtomTypeChangeC, TMainForm::OnAtomTypeChange)
  EVT_MENU(ID_AtomTypeChangeN, TMainForm::OnAtomTypeChange)
  EVT_MENU(ID_AtomTypeChangeO, TMainForm::OnAtomTypeChange)
  EVT_MENU(ID_AtomTypeChangeF, TMainForm::OnAtomTypeChange)
  EVT_MENU(ID_AtomTypeChangeH, TMainForm::OnAtomTypeChange)
  EVT_MENU(ID_AtomTypeChangeS, TMainForm::OnAtomTypeChange)
  EVT_MENU(ID_AtomTypePTable, TMainForm::OnAtomTypePTable)
  EVT_MENU(ID_AtomGrowShells, TMainForm::OnAtom)
  EVT_MENU(ID_AtomGrowFrags, TMainForm::OnAtom)

  EVT_MENU(ID_PlaneActivate, TMainForm::OnPlane)

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


  EVT_MENU(ID_SelGroup, TMainForm::OnSelection)
  EVT_MENU(ID_SelUnGroup, TMainForm::OnSelection)

  EVT_MENU(ID_GStyleSave, TMainForm::OnGraphicsStyle)
  EVT_MENU(ID_GStyleOpen, TMainForm::OnGraphicsStyle)

END_EVENT_TABLE()
//..............................................................................
TMainForm::TMainForm(TGlXApp *Parent, int Width, int Height):
  TMainFrame(wxT("Olex2"), wxPoint(0,0), wxSize(Width, Height), wxT("MainForm"))
{
//  _crtBreakAlloc = 5892;
  Destroying = false;
  StartupInitialised = RunOnceProcessed = false;
  wxInitAllImageHandlers();

  /* a singleton - will be deleted in destructor, we cannot use GC as the Py_DecRef
   would be called after finalising python
  */
  PythonExt::Init(this).Register( &TMainForm::PyInit );
  TOlxVars::Init();
  FGlCanvas = NULL;
  FXApp = NULL;
  FGlConsole = NULL;
  FInfoBox = NULL;
  GlTooltip = NULL;
  FHtml = NULL;
  ActiveLogFile = NULL;

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

  ProgramState = 0;

  OnModeChange = &FActionList.NewQueue("ONMODECHANGE");
  OnStateChange = &FActionList.NewQueue("ONSTATECHANGE");

  Modes = new TModes();

  FUndoStack = new TUndoStack();

  FParent = Parent;
  ObjectUnderMouse(NULL);
  FMacroItem = FHelpItem = NULL;
  FProcess = NULL;

  // FIOExt = new TIOExt();


  FTimer = new TTimer;

   HelpFontColorCmd.SetFlags(sglmAmbientF);  HelpFontColorTxt.SetFlags(sglmAmbientF);
   HelpFontColorCmd.AmbientF = 0x00ffff;     HelpFontColorTxt.AmbientF = 0x00ffff00;

//  ConsoleFontColor
//  NotesFontColor
//  LabelsFontColor

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

    FXApp->OnObjectsDestroy->Remove( this );
    ProcessXPMacro("onexit", MacroError);
    SaveSettings(DataDir + FLastSettingsFile);
    ClearPopups();
  }
  Destroying = true;
  return wxFrame::Destroy();
}
//..............................................................................
TMainForm::~TMainForm()  {
  delete Modes;
  for( int i=0; i < CallbackFuncs.Count(); i++ )
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
  FTimer->OnTimer()->Clear();
  delete FTimer;
//  delete FGlConsole;  // xapplication takes care !
  delete pmGraphics;
  delete pmFragment;
  delete pmMenu;
  delete pmAtom;
  delete pmBond;
  delete pmPlane;
  delete pmSelection;
  delete pmLabel;
  delete pmLattice;

  delete FUndoStack;
  // leave it fo last
  if( FProcess )  {
    FProcess->OnTerminate->Clear();
    FProcess->Terminate();
    delete FProcess;
  }
  // the order is VERY important!
  TOlxVars::Finalise();
  PythonExt::Finilise();
}
//..............................................................................
void TMainForm::XApp( TGXApp *XA)  {
  FXApp = XA;
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
  TLibrary &Library = XA->GetLibrary();

  this_InitMacroAD(Reap, @reap, b&;r&;*, fpAny,
"This macro loads a file if provided as an argument. If no argument is provided, it\
 shows a file open dialog");
  this_InitMacroD(Pict,
"c-embossed output in color&;bw-embossed output in b&w&;pq-highest (picture) quality",
  fpOne|fpTwo,
"Outputs a picture. Output file name is required, if a second numerical parameter is\
 provided, it is considered to be image resolution in range [0.1-10].");
  this_InitMacroD(Picta, "pq-picture quality", fpOne|fpTwo,
"A faster, but ditis version of pict. Not stable on some graphics cards");
  this_InitMacroD(Echo, EmptyString, fpAny,
"Prints provided string, functions are evaluated before printing");
  // contains an accumulation buffer. prints only when '\n' is encountered
  this_InitMacroD(Post, EmptyString, fpAny,
"Prints a string, but only after a new line crachter is encountered");

  this_InitMacroD(Bang, EmptyString, fpAny | psFileLoaded,
"Prints bonds and angles table for selected atoms");
  this_InitMacroD(Grow,
"s&;w-grows the rest of the structure, using already applied generators&;t-grows\
 only provided atoms/atom types", fpAny | psFileLoaded,
"Grows whole structure or provided atoms only");
  this_InitMacroD(Uniq, EmptyString, (fpAny ^ fpNone) | psFileLoaded,
"Shows only fragments specified by atom name(s) or selection");

  this_InitMacroD(Group, "n-a custom name can be provided", (fpAny ^ fpNone) | psFileLoaded,
"Groups curent selection");

  this_InitMacroD(Fmol, EmptyString, fpNone|psFileLoaded,
"Shows all fragments (as opposite to uniq)");
  this_InitMacroD(Clear, EmptyString, fpNone,
"Clears console buffer (text)");

  this_InitMacroD(Cell, EmptyString, fpNone|fpOne|psFileLoaded,
"If no arguments provided inverts visibility of unit cell, otherwise sets it to\
 the boolean value of the parameter");
  this_InitMacroD(Rota, EmptyString, fpTwo|fpFive,
"For two arguments the first one specifies axis of rotation (1,2,3 or x,y,z) and\
 the second one the rotation angle in degrees. For five arguments the first three\
 arguments specify the rotation vector [x,y,z] the forth parameter is the rotattion\
 angle and the fith one is the increment - the rotation will be continious");

  this_InitMacroD(Listen, EmptyString, fpAny,
"Listens for changes in a file provided as argument. If the file content changes\
 it is automatically reloaded in Olex2. If no arguments provided prints curent\
 status of the mode");
  #ifdef __WIN32__
  this_InitMacroD(WindowCmd, EmptyString, fpAny^(fpNone|fpOne),
"Windows specific command which send a command to a process with GUI window. First\
 argument is the window name, the second is the command. 'nl' is considered as a\
 new line char and 'sp' as wite space char");
  #endif
  this_InitMacroD(ProcessCmd, EmptyString, fpAny^(fpNone|fpOne),
"Send a command to current process. 'nl' is translated to the new line char and\
 'sp' to the white space char");
  this_InitMacroD(Wait, EmptyString, fpOne,
"Forces Olex2 to sleep for provided number of milliseconds");

  this_InitMacroD(SwapBg, EmptyString, fpNone,
"Swaps current background to white or vica-versa");
  this_InitMacroD(Silent, EmptyString, fpNone|fpOne,
"If no arguments is provided, prints out current mode status. Takes 'on' and 'off'\
 values to turn Olex2 log on and off");
  this_InitMacroD(Stop, EmptyString, fpOne,
"Switches specified mode off");

  this_InitMacroD(Exit, EmptyString, fpNone, "Exits Olex2");
  this_InitMacroAD(Exit, quit, EmptyString, fpNone, "Exits Olex2");

  this_InitMacroD(Pack,
"c-specifies if current lattice content should not be deleted&;q-includes Q-peaks to the atom list",
  fpAny|psFileLoaded,
"Packs structure within default or a volume provided as siz numbers. If atom\
 names/types are provided it only packs the provided atoms.");
  this_InitMacroD(Sel, "a-select all&;u-unselect all&;i-invert selection", fpAny,
"If no arguments provided, prints current selection. This includes distances, angles\
 and torsion angles. Selects atoms fulfilling provided conditions. An extended\
 syntax include keyword 'where' and 'rings' which allow selecting atoms and bonds\
 according to their properties, like type and length or rings of particular connectivity\
 like C6 or NC5. If the 'where' keyword is used, logical operators, like and (&&),\
 and or (||) can be used to refine the selection");

  this_InitMacroD(Name, "c-disables checking labels for dublications&;s-simply changes suffix\
  of provided atoms to the provided one (or none)&;cs-leaves current selection unchanged", fpOne | fpTwo,
"Names atoms. If the 'sel' keyword is used and a number is provided as second argument\
 the numbering will happen in the order the atoms were selected (make sure -c option\
 is added)");
  this_InitMacroD(TelpV, EmptyString, fpOne,
"Calculates ADPs for given thermal probability factor");
  this_InitMacroD(Labels,
"p-part&;l-label&;v-variables&;o-occupancy&;a-afix&;h-show hydrogen atom labels&;\
f-fixed parameters&;u-Uiso&;r-occupancy for riding atoms&;ao-actual accupancy\
 (as in the ins file)",  fpNone,
"Inverts visibility of atom labels on/off. Look at the options");

  this_InitMacroD(SetEnv, EmptyString, fpTwo,
"Sets an envomental variable");

  this_InitMacroD(Activate, EmptyString, fpOne,
"Sets current normal to the normal of the selected plane");
  this_InitMacroD(Info, EmptyString, fpAny,
"Prints out information on provided atoms ");
  this_InitMacroD(Help, "c-specifies commands category", fpAny,
"Prints available information. If no arguments provided prints available commands");
  this_InitMacroD(Matr, EmptyString, fpNone|fpOne|fpNine,
"Displays or sets current orientation matrix. For single argumen, 1,2,3 001, 111, etc\
 values are acceptable, nine values provide a full matrix ");
  this_InitMacroD(Qual, "h-Hight&;m-Medium&;l-Low", fpNone, "Sets drawings quality");

  this_InitMacro(Line, , fpOne|fpTwo|fpThree);
  this_InitMacro(AddLabel, , fpThree|fpFive);
  this_InitMacro(Mpln, n&;r&;we, fpAny);
  this_InitMacro(Cent, , fpAny ^ fpNone);
  this_InitMacro(File, s, fpNone|fpOne);
  this_InitMacro(User, , fpNone|fpOne);
  this_InitMacro(Dir, , fpNone|fpOne);
  this_InitMacroD(Isot,"h-adds hydrogen atoms&;cs-do not clear selection" , fpAny|psCheckFileTypeIns,
"makes provided atoms isotropic, if no arguments provieded, current selection all atoms become isotropic");
  this_InitMacroD(Anis,"h-adds hydrogen atoms&;cs-do not clear selection" , (fpAny) | psCheckFileTypeIns, 
"makes provided atoms anisotropic if no arguments provided current selection or all atoms are considered" );
  this_InitMacro(Mask, , fpAny );

  this_InitMacro(ARad, , fpOne );
  this_InitMacro(ADS, , fpAny^(fpNone) );
  this_InitMacro(AZoom, , fpAny^fpNone );
  this_InitMacro(BRad, , fpOne );

  this_InitMacro(Kill, h-kill hidden atoms, fpAny^fpNone );
  this_InitMacroAD(LS, LS, EmptyString, fpOne|fpTwo|psCheckFileTypeIns,
"Sets refinement method and/or the number of iterations.");
  this_InitMacroD(Plan, EmptyString, fpOne|psCheckFileTypeIns,
"Sets the number of Fuorier peaks to be found from the difference map");
  this_InitMacro(Omit, , fpOne|fpThree | psCheckFileTypeIns);
  this_InitMacro(UpdateWght, , fpAny | psCheckFileTypeIns );

  this_InitMacro(Exec, s&;o&;d, fpAny^fpNone );
  this_InitMacroD(Shell, "", fpNone|fpOne, "if no arguments launches a new interactive shell,\
  otherwise runs provided file in the interactive shell (on windows ShellExecute is\
  used to avoid flickering console)" );
  this_InitMacro(Save, , fpAny^fpNone );
  this_InitMacro(Load, , fpAny^fpNone );
  this_InitMacro(Link, , fpNone|fpOne );
  this_InitMacro(Style, s, fpNone|fpOne );
  this_InitMacro(Scene, s, fpNone|fpOne );

  this_InitMacro(SyncBC, , fpNone );

  this_InitMacro(IF, , fpAny );
  this_InitMacro(Basis, , fpNone|fpOne );
  this_InitMacro(Lines, , fpOne );
  this_InitMacro(LineWidth, , fpOne );

  this_InitMacro(Ceiling, , fpOne );
  this_InitMacro(Fade, , fpThree );

  this_InitMacro(WaitFor, , fpOne );
  this_InitMacro(Occu, , (fpAny^fpNone)^fpOne );
  this_InitMacro(FixUnit, , fpNone|psCheckFileTypeIns );
  this_InitMacro(AddIns, , (fpAny^fpNone)|psCheckFileTypeIns );

  this_InitMacro(HtmlPanelSwap, , fpNone|fpOne );
  this_InitMacro(HtmlPanelWidth, , fpNone|fpOne );
  this_InitMacro(HtmlPanelVisible, , fpNone|fpOne|fpTwo );

  this_InitMacro(QPeakScale, , fpNone|fpOne );
  this_InitMacro(Label, , fpAny^fpNone );
  this_InitMacro(CalcChn, , fpNone|fpOne );
  this_InitMacro(CalcMass, , fpNone|fpOne );

  this_InitMacro(Focus, , fpNone );
  this_InitMacro(Refresh, , fpNone );
  this_InitMacroD(Move,"cs-leaves selection unchanged&;c-copy moved atom", fpNone|fpTwo,
  "moves two atoms as close to each other as possible; if no atoms given, moves all fragments\
  as close to the cell center as possible" );

  this_InitMacro(ShowH, , fpNone|fpTwo|psFileLoaded );
  this_InitMacro(Fvar, , (fpAny^fpNone)|psCheckFileTypeIns );
  this_InitMacro(Sump, , (fpAny^fpNone)|psCheckFileTypeIns );
  this_InitMacro(Part, p&;lo, (fpAny^fpNone)|psCheckFileTypeIns );
  this_InitMacro(Afix, , (fpAny^fpNone)|psCheckFileTypeIns );
  this_InitMacro(Dfix, cs-do not clear selection&;e, fpAny|psCheckFileTypeIns );
  this_InitMacroD(Tria, "cs-do not clear selection", fpAny|psCheckFileTypeIns,
"Adds a distance restraint for bonds and 'angle' restraint for the angle");
  this_InitMacroD(Dang, "cs-do not clear selection", fpAny|psCheckFileTypeIns, 
"Adds a ShelX compatible angle restraint");
  this_InitMacroD(Sadi, EmptyString, fpAny|psCheckFileTypeIns,
"Similar distances restraint");
  this_InitMacroD(RRings,"s-esd&;cs-do not clear selection" , fpAny^fpNone,
"Makes all provided rings regular (flat and all distances suimilar)");
  this_InitMacroD(Flat, "cs-do not clear selection", fpAny|psCheckFileTypeIns,
"Forces flat group restraint for at least 4 provided atoms" );
  this_InitMacroD(Chiv, "cs- do not clear selection", fpAny|psCheckFileTypeIns,
"Forces chiral volume of atom(s) to '0' or provided value" );
  this_InitMacroD(EADP, "cs-do not clear selection", fpAny|psCheckFileTypeIns,
"Forces EADP/Uiso of provided atoms to be constrained the same" );
  this_InitMacroD(DELU, "cs-do not clear selection", fpAny|psCheckFileTypeIns,
"Rigid bond contstraint. If no atoms provided, all non-H atoms considered" );
  this_InitMacroD(SIMU, "cs-do not clear selection", fpAny|psCheckFileTypeIns,
"Forses simularity restraint for Uij of provided atoms. If no atoms provided, all non-H atoms considered" );
  this_InitMacroD(ISOR, "cs-do not clear selection", fpAny|psCheckFileTypeIns,
"Forses Uij of provided atoms to behave in isotropic manner. If no atoms provided, all non-H atoms considered" );

  this_InitMacro(Degen, cs, fpOne|psFileLoaded );
  // not implemented
  this_InitMacro(SwapExyz, , fpAny );
  // not implemented
  this_InitMacro(AddExyz, , fpAny );

  this_InitMacro(Reset, s&;c&;f, fpAny|psFileLoaded );
  this_InitMacro(ShowQ, , fpNone|fpTwo|psFileLoaded );
  this_InitMacro(LstMac, h-Shows help, fpNone );
  this_InitMacro(LstFun, h-Shows help, fpNone );
  this_InitMacro(LstIns, r, fpNone|psCheckFileTypeIns );
  this_InitMacroD(LstVar, EmptyString, fpNone, "lists all defined variables" );
  this_InitMacro(DelIns, , fpOne|psCheckFileTypeIns );

  this_InitMacro(Mode, p&;s&;t&;c, (fpAny^fpNone)|psFileLoaded );

  this_InitMacro(Text, , fpNone );
  this_InitMacro(ShowStr, , fpNone|fpOne|psFileLoaded );
  // not implemented
  this_InitMacro(Bind, , fpAny );

  this_InitMacro(Free, , fpAny^(fpNone|fpOne) );
  this_InitMacro(Fix, , fpAny^(fpNone|fpOne) );

  this_InitMacro(Grad, i&;p, fpNone|fpOne|fpFour );
  this_InitMacro(Split, , fpAny|psCheckFileTypeIns );
  this_InitMacro(ShowP, m-do not modify the display view, fpAny );

  this_InitMacro(EditAtom, cs-do not clear the selection,fpAny|psCheckFileTypeIns );
  this_InitMacro(EditIns, , fpNone|psCheckFileTypeIns );
  this_InitMacro(EditHkl, , fpNone|fpOne|fpThree );
  this_InitMacro(ViewHkl, , fpNone|fpOne );
  this_InitMacro(ExtractHkl, , fpOne|psFileLoaded );
  this_InitMacro(MergeHkl,f-do not merge Fiedel pairs , fpNone|fpOne|psFileLoaded );
  // not implemented
  this_InitMacroD(AppendHkl, "h&;k&;l&;c", fpAny, "moves reflection back into the refinement list\
 See excludeHkl for more details" );
  // not implemented
  this_InitMacroD(ExcludeHkl, "h-semicolumnt separated list of indexes&;k&;l&;c-true/false to use provided\
 indexes in any reflection. The default is in any one reflection" , fpAny, "excludes reflections with give indexes\
 from the hkl file -h=1;2 : all reflections where h=1 or 2. " );

  this_InitMacro(Direction, , fpNone );

  this_InitMacro(ViewGrid, , fpNone|fpOne );
  this_InitMacro(Undo, , fpNone );

  this_InitMacro(Individualise, , fpOne );
  this_InitMacro(Collectivise, , fpOne );

  this_InitMacro(Popup, w&;h&;t&;b&;x&;y&;d&;t&;r&;s&;c&;a&;i&;p, fpTwo );

  this_InitMacro(Delta, , fpNone|fpOne );
  this_InitMacro(DeltaI, , fpNone|fpOne );

  this_InitMacroA(Python, @py, , fpAny );

  this_InitMacro(CreateMenu, c&;s&;r&;m, fpOne|fpTwo|fpThree );
  this_InitMacro(DeleteMenu, , fpOne );
  this_InitMacro(EnableMenu, , fpOne );
  this_InitMacro(DisableMenu, , fpOne );
  this_InitMacro(CheckMenu, , fpOne );
  this_InitMacro(UncheckMenu, , fpOne );

  this_InitMacro(CreateShortcut, , fpTwo );

  this_InitMacro(SetCmd, , fpAny );

  this_InitMacro(UpdateOptions, , fpNone );
  this_InitMacro(Reload, , fpOne );
  this_InitMacro(StoreParam, , fpTwo|fpThree );
  this_InitMacro(SelBack, a&;o&;x, fpNone );

  this_InitMacro(CreateBitmap, r, fpTwo );
  this_InitMacro(DeleteBitmap, , fpOne );
  this_InitMacro(SGInfo, c&;i, fpNone|fpOne );
  this_InitMacro(Tref, ,fpOne|fpTwo|psCheckFileTypeIns );
  this_InitMacro(Patt, ,fpNone|psCheckFileTypeIns );
  this_InitMacro(Export, ,fpNone|fpOne|psCheckFileTypeCif );
  this_InitMacro(FixHL, ,fpNone );

  this_InitMacro(InstallPlugin,"l-local installation from a zip file, which must contains index.ind" ,fpOne );
  this_InitMacro(SignPlugin, ,fpAny^(fpOne|fpNone) );
  this_InitMacro(UninstallPlugin, ,fpOne );
  this_InitMacro(UpdateFile, f,fpOne );
  this_InitMacro(NextSolution, ,fpNone );

  this_InitMacro(Match, s&;n&;a&;i,fpNone|fpOne|fpTwo );
  this_InitMacro(ShowWindow, ,fpOne|fpTwo );

  this_InitMacro(DelOFile, ,fpOne );
  this_InitMacro(CalcVol, cs,fpOne );

  this_InitMacro(ChangeLanguage, ,fpOne );

  this_InitMacroD(HAdd, "cs-do not clear selection", fpAny|psCheckFileTypeIns, "Adds hydrogen atoms\
  to all or provided atoms" );
  this_InitMacro(HklStat, l,(fpAny^fpNone)|psFileLoaded );

  this_InitMacroD(Schedule, "r-repeatable", fpAny^(fpNone|fpOne),
"Schedules a particular macro (second argument) to be executed within provided\
 interval (first argument)" );

  this_InitMacro(Tls, , fpAny^(fpNone)|psFileLoaded );

  this_InitMacroD(LstFS, EmptyString, fpNone,
"Prints out detailed content of virtual file system");
  this_InitMacro(Test, , fpAny );
  // -f - force inversion even for nocentrosymmetric space group
  this_InitMacro(Inv, f, fpAny|psFileLoaded );
  this_InitMacro(Push, , (fpAny^(fpNone|fpOne|fpTwo))|psFileLoaded );
  this_InitMacro(Transform, , fpAny|psFileLoaded );

  this_InitMacroD(LstRes, EmptyString, fpNone|psFileLoaded,
"Prints all interpreted restrains for current structure" );
  this_InitMacroD(CalcVoid, "d-distance from Van-der-Waalse surface&;i-invert", fpNone|fpOne|psFileLoaded,
"Calculates solvent accessible void and packing parameters; optionally accepts a file with space \
separated values of Atom Type and radius, an entry a line" );
  this_InitMacroD(Sgen, EmptyString, (fpAny^fpNone)|psFileLoaded,
"Grows the structure using provided atoms (all if none provided) and symmetry code" );
  this_InitMacroD(LstSymm, EmptyString, fpNone|psFileLoaded,
"Prints symmetry codes of current unit cell" );
  this_InitMacroD(IT, "o-orients basis accordint to principle axes of inertion", fpAny,
"Calculates tensor of inertion" );

  this_InitMacroD(StartLogging, "c-empties the file if exists", fpOne,
"Creates/opens for appending a log file, where all screen output is saved" );
  this_InitMacroD(ViewLattice, EmptyString, fpOne,
"Loads cell information from provided file and dispalys it on screen as lattice points/grid" );
  this_InitMacroD(AddObject, EmptyString, fpAny^(fpNone|fpOne),
"Adds a new user defined object to the graphical scene" );
  this_InitMacroD(DelObject, EmptyString, fpOne,
"Deletes graphical object by name" );

  this_InitMacroD(OnRefine, EmptyString, fpAny,
"Internal procedure" );
  this_InitMacroD(TestMT, EmptyString, fpAny,
"Testing multithreading" );
  this_InitMacroD(SetFont, "ps-point size&;b-bold&;i-italic", fpAny^(fpNone|fpOne),
"Sets font for specified control" );
  this_InitMacroD(EditMaterial, EmptyString, fpOne,
"Brighs up material properties dialog for specified object" );
  this_InitMacroD(SetMaterial, EmptyString, fpTwo | fpThree,
"Assigns provided value to specified material" );
  this_InitMacroD(LstGO, EmptyString, fpNone,
"List current graphicl objects" );
  this_InitMacroD(CalcPatt, EmptyString, fpNone|psFileLoaded,
"Calculates patterson map" );
  this_InitMacroD(CalcFourier, "fcf-reads structure factors from a fcf file&;diff-calculates\
  difference map&;abs-calculates modulus of the electron density&;tomc-calculates 2Fo-Fc\
  map&;obs-calculates observer emap&;calc-calculates calculated emap&;scale-scle to use\
  for difference maps, corrently available simple(s) sum(Fo^2)/sum(Fc^2) for Fo^2/sigme > 3)\
  and regression(r)&;r-resolution in Angstrems", fpNone|psFileLoaded,
"Calculates fourier map" );
  this_InitMacroD(TestBinding, EmptyString, fpNone, "Internal tests" );
  this_InitMacroD(SGE, EmptyString, fpNone|fpOne|psFileLoaded, "Extended spacegroup determination. Internal use" );
  this_InitMacroD(Flush, EmptyString, fpNone|fpOne, "Flushes log streams" );
  this_InitMacroD(ShowSymm, EmptyString, fpNone|fpOne, "Shows symmetry elements of the unitcell" );
  this_InitMacroD(Textm, EmptyString, fpOne, "Runs subsequent commands stored in a text file" );
  this_InitMacroD(TestStat, EmptyString, fpOne, "Test: runs statistical tests on structures in current folder. Expects a file name" );
  this_InitMacroD(ExportFont, EmptyString, fpTwo, "" );
  this_InitMacroD(ImportFont, EmptyString, fpTwo, "" );
  // FUNCTIONS _________________________________________________________________

  this_InitFunc(FileLast, fpNone|fpOne);
  this_InitFunc(FileSave, fpThree);
  this_InitFunc(FileOpen, fpThree);
  this_InitFunc(ChooseDir, fpNone|fpOne|fpTwo);

  this_InitFunc(Cell, fpOne|psFileLoaded);

  this_InitFunc(Title, fpNone|fpOne);
  this_InitFuncD(Ins, fpOne|psCheckFileTypeIns,
"Returns instruction value (all data after the instruction). In case the instruction\
 does not exist it return 'n/a' string");
  this_InitFuncD(LSM, fpNone|psCheckFileTypeIns,
"Return current refinement method, L.S. or CGLS currently.");
  this_InitFuncD(SSM, fpNone|fpOne,
"Return current structure solution method, TREF or PATT currently. If current method is unknown\
 and an argument is provided, that argument is returned");
  this_InitFunc(DataDir, fpNone);
  this_InitFuncD(Cif, fpOne|psCheckFileTypeCif,
"Returns instruction value (all data after the instruction). In case the instruction\
 does not exist it return 'n/a' string");
  this_InitFuncD(P4p, fpOne|psCheckFileTypeP4P,
"Returns instruction value (all data after the instruction). In case the instruction\
 does not exist it return 'n/a' string");
  this_InitFuncD(Crs, fpOne|psCheckFileTypeCRS,
"Returns instruction value (all data after the instruction). In case the instruction\
 does not exist it return 'n/a' string");

  this_InitFunc(Strcat, fpTwo);
  this_InitFunc(Strcmp, fpTwo);

  this_InitFunc(GetEnv, fpOne);

  this_InitFunc(Eval, fpOne);

  this_InitFunc(UnsetVar, fpOne);
  this_InitFunc(SetVar, fpTwo);
  this_InitFunc(GetVar, fpOne|fpTwo);
  this_InitFunc(IsVar, fpOne);

  this_InitFunc(VVol, fpNone|fpOne|psFileLoaded);

  this_InitFunc(Env, fpOne|psFileLoaded);
  this_InitFunc(Crd, fpOne|psFileLoaded);
  this_InitFunc(CCrd, fpOne|psFileLoaded);
  this_InitFunc(Atoms, fpOne|psFileLoaded);

  this_InitFunc(Sel, fpNone|psFileLoaded);
  this_InitFunc(FPS, fpNone);

  this_InitFunc(Cursor, fpNone|fpOne|fpTwo);
  this_InitFunc(RGB, fpThree|fpFour);
  this_InitFunc(Color, fpNone|fpOne|fpTwo);

  this_InitFunc(Lst, fpOne|psFileLoaded);
  this_InitFunc(Zoom, fpNone|fpOne);
  this_InitFunc(HtmlPanelWidth, fpNone|fpOne);

  #ifdef __WIN32__
  this_InitFunc(LoadDll, fpOne);
  #endif

  this_InitFunc(CmdList, fpOne);
  this_InitFuncD(SG, fpNone|fpOne,
"Returns space group of currently loaded file. Also takes a string template, where\
 %# is replaced with SG number, %n - short name, %N - full name and %h - Hall symbol" );
  this_InitFunc(Alert, fpTwo|fpThree|fpFour);

  this_InitFunc(IsPluginInstalled, fpOne);
  this_InitFunc(ValidatePlugin, fpOne);

  // number of lines, caption, def value
  this_InitFunc(GetUserInput, fpThree);

  this_InitFunc(GetCompilationInfo, fpNone);

  this_InitFuncD(TranslatePhrase, fpOne,
"Translates provided phrase into current language");
  this_InitFuncD(IsCurrentLanguage, fpOne,
"Checks current language" );
  this_InitFuncD(CurrentLanguageEncoding, fpNone,
"Returns currentlanguage encoding, like: ISO8859-1" );

  this_InitFunc(SGList, fpNone );

  this_InitFunc(And, fpAny^(fpNone|fpOne) );
  this_InitFunc(Or, fpAny^(fpNone|fpOne) );
  this_InitFunc(Not, fpOne );

  this_InitFunc(ChooseElement, fpNone );
  this_InitFunc(SfacList, fpNone|psCheckFileTypeIns );

  this_InitFuncD(StrDir, fpNone|psFileLoaded, "Returns location of the folder, where\
  Olex stores structure related data" );

  this_InitFuncD(ChooseFont, fpNone|fpOne, "Brigns up a font dialog. If font\
  information provided, initialises the dialog with that font" );
  this_InitFuncD(GetFont, fpOne, "Returns specified font" );
  this_InitFuncD(GetMaterial, fpOne, "Returns specified material" );
  this_InitFuncD(ChooseMaterial, fpNone|fpOne, "Brings up a dialog to edit\
  default or provided material" );

  this_InitFuncD(GetMouseX, fpNone, "Returns current mouse X position" );
  this_InitFuncD(GetMouseY, fpNone, "Returns current mouse Y position" );
  this_InitFuncD(GetWindowSize, fpNone|fpOne|fpThree, "Returns size of the requested window, main window by default" );
  this_InitFuncD(IsOS, fpOne, "Returns true if current system Windows [win], Linux/GTK [linux], Mac [mac]" );
  this_InitFuncD(ExtraZoom, fpNone|fpOne, "Sets/reads current extra zoom (default zoom correction)" );
  this_InitFuncD(HasGUI, fpNone, "Returns if true if Olex2 is built with GUI" );

  Library.AttachLibrary( TEFile::ExportLibrary() );
  //Library.AttachLibrary( olxstr::ExportLibrary("str") );
  Library.AttachLibrary( PythonExt::GetInstance()->ExportLibrary() );
  Library.AttachLibrary( TETime::ExportLibrary() );
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
  Library.AttachLibrary( XA->XFile().ExportLibrary() );
  Library.AttachLibrary( XA->GetFader().ExportLibrary() );
  Library.AttachLibrary( XA->XGrid().ExportLibrary() );
  Library.AttachLibrary( TFileHandlerManager::ExportLibrary() );

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
  pmFragment = new TMenu();
  pmSelection = new TMenu();
  pmGraphics = new TMenu();
  pmBond = new TMenu();
    pmTang = new TMenu();
  pmPlane = new TMenu();

  MenuBar->Append(MenuFile, wxT("&File") );
  MenuBar->Append(MenuView, wxT("&View") );
  MenuBar->Append(MenuStructure, wxT("&Structure") );
  MenuBar->Append(MenuHelp, wxT("&Help") );

  miHtmlPanel = new wxMenuItem(MenuView, ID_HtmlPanel, wxT("&Html panel"),
    wxT("Show/hide html panel"), wxITEM_CHECK, NULL);
  MenuView->Append(miHtmlPanel);

  MenuStructure->Append(ID_StrGenerate, wxT("&Generate..."));

  MenuHelp->Append(ID_About, wxT("&About...") );

// statusbar initialisation
  StatusBar = CreateStatusBar();
  SetStatusText( wxT("Welcome to OLEX2!") );
// toolbar initialisation
  ToolBar = NULL;
//  ToolBar = CreateToolBar(wxTB_FLAT | wxTB_HORIZONTAL | wxTB_TEXT  , -1, "MainToolBar");
//  olxstr S = TEFile::ExtractFilePath(wxGetApp().argv[0]);
//  S += "toolbar\\copy.bmp";
//  wxBitmap Bmp( wxBITMAP(COPY) );
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
// setting selection menu
  pmSelection->Append(ID_SelGroup, wxT("Group"));
  pmSelection->Append(ID_SelUnGroup, wxT("Ungroup"));
  miGroupSel = pmSelection->FindItemByPosition(0);
  miUnGroupSel = pmSelection->FindItemByPosition(1);
//  pmSelection->AppendSeparator();
//  pmSelection->Append(ID_MenuGraphics, "Graphics", pmGraphics->Clone());
// setting graphics menu
  pmGraphics->Append(ID_GraphicsHide, wxT("Hide"));
  pmGraphics->Append(ID_GraphicsDS, wxT("Draw style..."));
  pmGraphics->Append(ID_GraphicsP, wxT("Primitives..."));
// setting label menu
  pmLabel = pmGraphics->Clone();
  pmLabel->Append(ID_GraphicsEdit, wxT("Edit..."));
// setting Lattice menu
  pmLattice = pmGraphics->Clone();
  pmLattice->Append(ID_FixLattice, wxT("Fix"));
  pmLattice->Append(ID_FreeLattice, wxT("Free"));
// setting atom menu
  pmAtom->Append(ID_MenuItemAtomInfo, wxT("?") );
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

  pmAtom->Append(ID_MenuAtomType, wxT("Type"), pmAtomType);
  pmAtom->Append(ID_MenuAtomOccu, wxT("Occupancy"), pmAtomOccu);
  pmAtom->AppendSeparator();
  pmAtom->Append(ID_AtomGrowShells, wxT("Grow Shell"));
    miAtomGrowShell = pmAtom->FindItemByPosition(pmAtom->GetMenuItemCount()-1);
  pmAtom->Append(ID_AtomGrowFrags, wxT("Grow Fragments"));
    miAtomGrowFrag = pmAtom->FindItemByPosition(pmAtom->GetMenuItemCount()-1);
  pmAtom->AppendSeparator();
  pmAtom->Append(ID_GraphicsHide, wxT("Delete"));
  pmAtom->AppendSeparator();
  pmAtom->Append(ID_MenuFragment, wxT("Fragment"), pmFragment->Clone());
  pmAtom->Append(ID_MenuGraphics, wxT("Graphics"), pmGraphics->Clone());
  pmAtom->Append(ID_Selection, wxT("Selection"), pmSelection->Clone());
// setting bond menu
  pmBond->Append(ID_MenuItemBondInfo, wxT("?") );
  pmBond->AppendSeparator();
  miBondInfo = pmBond->FindItemByPosition(0);
  pmBond->Append(ID_MenuTang, wxT("TANG"), pmTang);
  pmBond->AppendSeparator();
  pmBond->Append(ID_GraphicsHide, wxT("Delete"));
  pmBond->AppendSeparator();
  pmBond->Append(ID_MenuFragment, wxT("Fragment"), pmFragment->Clone());
  pmBond->Append(ID_MenuGraphics, wxT("Graphics"), pmGraphics->Clone());
  pmBond->Append(ID_Selection, wxT("Selection"), pmSelection->Clone());
// setting plane menu
  pmPlane->Append(ID_PlaneActivate, wxT("Activate") );
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

  SetMenuBar( MenuBar );
//////////////////////////////////////////////////////////////
  FXApp->GetRender().OnDraw->Add(this, ID_GLDRAW);
  TObjectVisibilityChange* VC = new TObjectVisibilityChange(this);
  XA->OnGraphicsVisible->Add(VC);
  // put correct captions to the menu
  if( FXApp->IsCellVisible() )   pmModel->SetLabel(ID_CellVisible, wxT("Hide cell"));
  else                           pmModel->SetLabel(ID_CellVisible, wxT("Show cell"));
  if( FXApp->IsBasisVisible() )  pmModel->SetLabel(ID_BasisVisible, wxT("Hide basis"));
  else                           pmModel->SetLabel(ID_BasisVisible, wxT("Show basis"));

  TutorialDir = XA->BaseDir()+"etc/";
//  DataDir = TutorialDir + "Olex_Data\\";
  DataDir = TShellUtil::GetSpecialFolderLocation(fiAppData);
#ifdef __WIN32__
  #ifdef _UNICODE
    DataDir << "Olex2u/";
  #else
    DataDir << "Olex2/";
  #endif
#endif

  BadRefsFile = DataDir + "badrefs.htm";
  RefineDataFile = DataDir + "refinedata.htm";
  DictionaryFile = XA->BaseDir() + "dictionary.txt";
  PluginFile =  XA->BaseDir() + "plugins.xld";
  FHtmlIndexFile = TutorialDir+"index.htm";

  TFileHandlerManager::AddBaseDir( TutorialDir );
  TFileHandlerManager::AddBaseDir( DataDir );

  SetStatusText( uiStr(XA->BaseDir()));

  if( !TEFile::FileExists(DataDir) )  {
    if( !TEFile::MakeDir(DataDir) )
      TBasicApp::GetLog().Error("Could not create data folder!");
  }
  // put log file to the user data folder
  try  {
    TBasicApp::GetLog().AddStream( TUtf8File::Create(DataDir + "olex2.log"), true );
  }
  catch( TExceptionBase& )  {
    TBasicApp::GetLog().Error("Could not create log file!");
  }

  TBasicApp::GetLog().OnInfo->Add(this, ID_INFO, msiEnter);
  TBasicApp::GetLog().OnWarning->Add(this, ID_WARNING, msiEnter);
  TBasicApp::GetLog().OnError->Add(this, ID_ERROR, msiEnter);
  TBasicApp::GetLog().OnException->Add(this, ID_EXCEPTION, msiEnter);
  FXApp->OnObjectsDestroy->Add(this, ID_XOBJECTSDESTROY, msiEnter);

  LoadVFS(plGlobal);

  FHtml = new THtml(this, FXApp);

  FHtml->OnLink->Add(this, ID_ONLINK);
  FHtml->OnKey->Add(this, ID_HTMLKEY);
  /*  people do not like it very much ....*/
//  FHtml->OnDblClick->Add(this, ID_HTMLDBLCLICK);
  FHtml->OnCmd->Add(this, ID_HTMLCMD);

  FXApp->LabelsVisible(false);
  FXApp->GetRender().LightModel.ClearColor() = 0x0f0f0f0f;

  FGlConsole = new TGlConsole("Console", &FXApp->GetRender() );
  // the commands are posted from in Dispatch, SkipPosting is controlling the output
  FXApp->GetLog().AddStream( FGlConsole, false );
  FGlConsole->OnCommand->Add( this, ID_COMMAND);
  FGlConsole->OnPost->Add( this, ID_TEXTPOST);
  FXApp->AddObjectToCreate(FGlConsole);
////////////////////////////////////////////////////////////////////////////////
  Library.AttachLibrary( FGlConsole->ExportLibrary() );
  Library.AttachLibrary( FXApp->GetRender().ExportLibrary() );
////////////////////////////////////////////////////////////////////////////////
  FCmdLine = new TCmdLine(this, wxNO_BORDER );
//  wxWindowDC wdc(this);
//  FCmdLine->WI.SetHeight( wdc.GetTextExtent(wxT("W")).GetHeight() );
  FCmdLine->OnChar->Add(this, ID_CMDLINECHAR);
  FCmdLine->OnKeyDown->Add(this, ID_CMDLINEKEYDOWN);
  FCmdLine->OnCommand->Add( this, ID_COMMAND);

  FHelpWindow = new TGlTextBox("HelpWindow", &FXApp->GetRender());
  FXApp->AddObjectToCreate(FHelpWindow);
  FHelpWindow->Visible(false);

  FInfoBox = new TGlTextBox("InfoBox", &FXApp->GetRender());
  FXApp->AddObjectToCreate( FInfoBox );

  GlTooltip = new TGlTextBox("Tooltip", &FXApp->GetRender());
  FXApp->AddObjectToCreate( GlTooltip );
  GlTooltip->Visible(false);
  GlTooltip->SetZ(4.9);

  FTimer->OnTimer()->Add( TBasicApp::GetInstance()->OnTimer );
  TBasicApp::GetInstance()->OnTimer->Add(this, ID_TIMER);
  // synchronise if value is different in settings file...
  miHtmlPanel->Check( !FHtmlMinimized );
#if defined(__WIN32__) || defined(__MAC__)
  StartupInit();
#endif
}
//..............................................................................
void TMainForm::StartupInit()  {
  if( StartupInitialised )  return;
  StartupInitialised = true;
  wxFont Font(10, wxMODERN, wxNORMAL, wxNORMAL);//|wxFONTFLAG_ANTIALIASED);
  // create 4 fonts
  
  TGlFont *fnt = FXApp->GetRender().Scene()->CreateFont("Console", Font.GetNativeFontInfoDesc().c_str());
  fnt->Material().SetFlags(sglmAmbientF|sglmEmissionF|sglmIdentityDraw);
  fnt->Material().AmbientF = 0x7fff7f;
  fnt->Material().EmissionF = 0x1f2f1f;

  fnt = FXApp->GetRender().Scene()->CreateFont("Help", Font.GetNativeFontInfoDesc().c_str());
  fnt->Material().SetFlags(sglmAmbientF|sglmIdentityDraw);
  fnt->Material().AmbientF = 0x7fff7f;

  fnt = FXApp->GetRender().Scene()->CreateFont("Notes", Font.GetNativeFontInfoDesc().c_str());
  fnt->Material().SetFlags(sglmAmbientF|sglmIdentityDraw);
  fnt->Material().AmbientF = 0x7fff7f;

  fnt = FXApp->GetRender().Scene()->CreateFont("Labels", Font.GetNativeFontInfoDesc().c_str());
  fnt->Material().SetFlags(sglmAmbientF|sglmIdentityDraw);
  fnt->Material().AmbientF = 0x7fff7f;

  fnt = FXApp->GetRender().Scene()->CreateFont("Picture_labels", Font.GetNativeFontInfoDesc().c_str());
  fnt->Material().SetFlags(sglmAmbientF|sglmIdentityDraw);
  fnt->Material().AmbientF = 0x7fff7f;

  fnt = FXApp->GetRender().Scene()->CreateFont("Tooltip", Font.GetNativeFontInfoDesc().c_str());
  fnt->Material().SetFlags(sglmAmbientF|sglmIdentityDraw);
  fnt->Material().AmbientF = 0x7fff7f;

  FXApp->LabelsFont( 3 );

  FGlConsole->SetFontIndex(0);
  FHelpWindow->SetFontIndex(1);
  FInfoBox->SetFontIndex( 2 );
  GlTooltip->SetFontIndex( 5 );

  FXApp->Init(); // initialise the gl

  olxstr T(DataDir);  
  T << FLastSettingsFile;
  if( !TEFile::FileExists(T) )  {
    T = TBasicApp::GetInstance()->BaseDir();
    TEFile::AddTrailingBackslashI(T);
    T << FLastSettingsFile;
  }
  try  {  LoadSettings(T);  }
  catch( TExceptionBase& exc )  {
    ::wxMessageBox( uiStr(exc.GetException()->GetError()) += wxT('\n'),
    uiStrT("Exception: ") += uiStr(EsdlObjectName(exc)), wxOK|wxICON_ERROR);
    throw;
  }

  FInfoBox->SetHeight(FXApp->GetRender().Scene()->Font(2)->TextHeight(EmptyString));
  
  ProcessXPMacro(olxstr("showwindow help ") << HelpWindowVisible, MacroError);
  ProcessXPMacro(olxstr("showwindow info ") << InfoWindowVisible, MacroError);
  ProcessXPMacro(olxstr("showwindow cmdline ") << CmdLineVisible, MacroError);
  ProcessXPMacro("reload macro", MacroError);
  ProcessXPMacro("reload help", MacroError);

  FTimer->Start(15);
  if( FGlCanvas != NULL )  FGlCanvas->XApp(FXApp);

  if( TEFile::FileExists(FXApp->BaseDir() + "settings.xld") )  {
    TDataFile settings;
    settings.LoadFromXLFile( FXApp->BaseDir() + "settings.xld", NULL );
    TDataItem* sh = settings.Root().FindItemCI("shortcuts");
    if( sh != NULL )  {
      try  {
        olxstr cmd;
        for( int i=0; i < sh->ItemCount(); i++ )  {
        TDataItem& item = sh->Item(i);
        AccShortcuts.AddAccell( TranslateShortcut( item.GetFieldValue("key")), item.GetFieldValue("macro") );
        // cannot execute it through a macro - functions get evaluated...
        //ProcessXPMacro(cmd, MacroError);
        }
      }
      catch( TExceptionBase& exc )  {
        TBasicApp::GetLog().Exception( exc.GetException()->GetFullMessage() );
      }
    }
    sh = settings.Root().FindItemCI("menus");
    if( sh != NULL )  {
    try  {
      olxstr cmd;
      for( int i=0; i < sh->ItemCount(); i++ )  {
      TDataItem& item = sh->Item(i);
      cmd = "createmenu \'";
      cmd << item.GetFieldValue("title") << "\' \'" <<
          item.GetFieldValue("macro") << '\'';

      olxstr before = item.GetFieldValue("before");
      if( before.Length() )  {
        cmd << " \'" << before <<'\'';
      }
      cmd << ' ';

      olxstr modeDep = item.GetFieldValue("modedependent");
      if( modeDep.Length() )  {
        cmd << " -m=\'" << modeDep << '\'';
      }
      cmd << ' ';

      olxstr stateDep = item.GetFieldValue("statedependent");
      if( stateDep.Length() )  {
        cmd << " -s=\'" << stateDep << '\'';
      }
      cmd << ' ';

      if( !item.GetName().Comparei("radio") )  cmd << "-r ";
      if( !item.GetName().Comparei("sep") )    cmd << "-# ";
      if( !item.GetName().Comparei("check") )  cmd << "-c ";

      ProcessXPMacro(cmd, MacroError);
      }
    }
    catch( TExceptionBase& exc )  {
      TBasicApp::GetLog().Exception( exc.GetException()->GetFullMessage() );
    }
    }
  }

  FPluginItem = NULL;
  if( TEFile::FileExists( PluginFile ) )  {
    FPluginFile.LoadFromXLFile( PluginFile, NULL );
    FPluginItem = FPluginFile.Root().FindItem("Plugin");
    TStateChange sc(prsPluginInstalled, true);
    // manually activate the event
    OnStateChange->Execute((AEventsDispatcher*)this, &sc);
  }
  else  {
    FPluginItem = FPluginFile.Root().AddItem("Plugin");
  }

  for( int i=0; i < StoredParams.Count(); i++ )  {
    // ser the variables
    ProcessXPMacro(olxstr("setvar(") << StoredParams.GetComparable(i)
                    << ",\'" << StoredParams.GetObject(i)
                    << "\')", MacroError);

  }

  ProcessXPMacro("onstartup", MacroError);

  // load html in last cal - it might call some destructive functions on uninitialised data

  FHtml->LoadPage( uiStr(FHtmlIndexFile));
  FHtml->SetHomePage( FHtmlIndexFile );

  if( FXApp->Arguments.Count() == 1 )  {
    olxstr openCmd = "reap \'";
    openCmd << FXApp->Arguments.String(0) << '\'';
    ProcessXPMacro(openCmd, MacroError);
  }
}
//..............................................................................
void TMainForm::SetProcess( AProcess *Process )  {
  if( FProcess != NULL && Process == NULL )  {
    while( FProcess->StrCount() != 0 )  {
      FGlConsole->PrintText(FProcess->GetString(0), &ExecFontColor);
      CallbackFunc(ProcessOutputCBName, FProcess->GetString(0));
      FProcess->DeleteStr(0);
    }
    FGlConsole->PrintText(EmptyString);

    if( FMode & mListen )
      Dispatch(ID_TIMER, msiEnter, (AEventsDispatcher*)this, NULL);

    FOnListenCmds.Clear();
    olxstr Cmd;
    TMacroError err;
    while( FProcess->OnTerminateCmds().Count() ) {
      Cmd = FProcess->OnTerminateCmds().String(0);
      FProcess->OnTerminateCmds().Delete(0);
      ProcessXPMacro(Cmd, err);
      if( !err.IsSuccessful() )  {
        AnalyseError( err );
        FProcess->OnTerminateCmds().Clear();
        break;
      }
    }
    TimePerFrame = FXApp->Draw();
  }
  if( Process )
    Process->OnTerminate->Add(this, ID_PROCESSTERMINATE);

  if( FProcess )  {  
    FProcess->OnTerminate->Clear();  
    FProcess->Detach();
    // will be deleted anyway :), detach puts it to the TEGC
    //delete FProcess;
  }
  FProcess = Process;
  if( FProcess == NULL )  {
  TBasicApp::GetLog().Info( "The process has been terminated..." );
    TimePerFrame = FXApp->Draw();
  }
}
//..............................................................................
// view menu
void TMainForm::OnHtmlPanel(wxCommandEvent& event)  {
  Dispatch(ID_HTMLDBLCLICK, 0, (AEventsDispatcher*)this, NULL);
}
//..............................................................................
void TMainForm::OnGenerate(wxCommandEvent& WXUNUSED(event))
{
//  TBasicApp::GetLog()->Info("generate!");;
  TdlgGenerate *G = new TdlgGenerate(this);
  if( G->ShowModal() == wxID_OK )  {
    olxstr T("pack ");
    T << olxstr::FormatFloat(1, G->AFrom()) << ' ' << olxstr::FormatFloat(1, G->ATo()) << ' ';
    T << olxstr::FormatFloat(1, G->BFrom()) << ' ' << olxstr::FormatFloat(1, G->BTo()) << ' ';
    T << olxstr::FormatFloat(1, G->CFrom()) << ' ' << olxstr::FormatFloat(1, G->CTo()) << ' ';
    ProcessXPMacro(T, MacroError);
  }
  G->Destroy();
}
//..............................................................................
void TMainForm::OnAbout(wxCommandEvent& WXUNUSED(event))  {
  wxMessageBox( wxT("OLEX 2\n(c) Oleg V. Dolomanov, Horst Puschmann, 2004-2008\nUniversity of Durham"),
    wxT("About"), wxOK | wxICON_INFORMATION, this);
}
//..............................................................................
void TMainForm::OnFileOpen(wxCommandEvent& event)  {
  if( event.GetId() >= ID_FILE0 && event.GetId() <= (ID_FILE0+9) )  {
    wxMenuItem *mi = FRecentFiles.Object(event.GetId() - ID_FILE0);
    if( TEFile::ExtractFileExt(FRecentFiles[event.GetId() - ID_FILE0]).IsEmpty() )  {
      olxstr fn( TEFile::ChangeFileExt(FXApp->XFile().GetFileName(), EmptyString) );
      TEFile::OSPathI(fn);
      if( fn != FRecentFiles[event.GetId() - ID_FILE0] )  {
        olxstr ins_fn( TEFile::ChangeFileExt(FRecentFiles[event.GetId() - ID_FILE0], "ins") );
        olxstr res_fn( TEFile::ChangeFileExt(FRecentFiles[event.GetId() - ID_FILE0], "res") );
        if( TEFile::FileExists(ins_fn) && TEFile::FileExists(res_fn) )  {
          if( TEFile::FileAge(ins_fn) > TEFile::FileAge(res_fn) )
            fn = ins_fn;
          else
            fn = res_fn;
        }
        else if( TEFile::FileExists(res_fn) )
          fn = res_fn;
        else if( TEFile::FileExists(ins_fn) )
          fn = ins_fn;
        else  {
          FXApp->GetLog().Error("Could not locate ins/res file");
          return;
        }
      }
      else  // just reopen
        fn = FXApp->XFile().GetFileName();
      ProcessXPMacro(olxstr("reap \'") << fn << '\'', MacroError);
    }
    else
      ProcessXPMacro(olxstr("reap \'") << FRecentFiles[event.GetId() - ID_FILE0] << '\'', MacroError);
  }
}
//..............................................................................
void TMainForm::OnDrawStyleChange(wxCommandEvent& event)  {
  switch( event.GetId() )  {
    case ID_DSBS: ProcessXPMacro("pers", MacroError);  break;
    case ID_DSES: ProcessXPMacro("telp", MacroError);  break;
    case ID_DSSP: ProcessXPMacro("sfil", MacroError);  break;
    case ID_DSWF: ProcessXPMacro("proj", MacroError);  break;
    case ID_DSST: ProcessXPMacro("sticks", MacroError);  break;
    case ID_SceneProps:
      TdlgSceneProps *Dlg = new TdlgSceneProps(this, FXApp);
      if( Dlg->ShowModal() == wxID_OK )  {
        FBgColor = FXApp->GetRender().LightModel.ClearColor();
      }
      TimePerFrame = FXApp->Draw();
      Dlg->Destroy();
    break;
  }
}
void TMainForm::OnViewAlong(wxCommandEvent& event) {
  switch( event.GetId() )  {
    case ID_View100:  ProcessXPMacro("matr 1", MacroError);  break;
    case ID_View010:  ProcessXPMacro("matr 2", MacroError);  break;
    case ID_View001:  ProcessXPMacro("matr 3", MacroError);  break;
    case ID_View110:  ProcessXPMacro("matr 110", MacroError);  break;
    case ID_View101:  ProcessXPMacro("matr 101", MacroError);  break;
    case ID_View011:  ProcessXPMacro("matr 011", MacroError);  break;
    case ID_View111:  ProcessXPMacro("matr 111", MacroError);  break;
  }
}
//..............................................................................
void TMainForm::OnAtomOccuChange(wxCommandEvent& event)  {
  TXAtom *XA = (TXAtom*)FObjectUnderMouse;
  if( XA == NULL )  return;
  olxstr Tmp("occu ");
  if( XA->Selected() )  Tmp << "sel";
  else                  Tmp << XA->Atom().GetLabel();
  Tmp << ' ';
  switch( event.GetId() )  {
    case ID_AtomOccu1:   Tmp << "11";  break;
    case ID_AtomOccu34:  Tmp << "10.75";  break;
    case ID_AtomOccu12:  Tmp << "10.5";  break;
    case ID_AtomOccu13:  Tmp << "10.33333";  break;
    case ID_AtomOccu14:  Tmp << "10.25";  break;
  }
  ProcessXPMacro(Tmp, MacroError);
}
//..............................................................................
void TMainForm::OnDrawQChange(wxCommandEvent& event)  {
  switch( event.GetId() )  {
    case ID_DQH:  ProcessXPMacro("qual -h", MacroError);     break;
    case ID_DQM:  ProcessXPMacro("qual -m", MacroError);     break;
    case ID_DQL:  ProcessXPMacro("qual -l", MacroError);     break;
  }
}
//..............................................................................
void TMainForm::CellVChange()  {
  TStateChange sc(prsCellVis, FXApp->IsCellVisible() );
  pmModel->SetLabel(ID_CellVisible, (FXApp->IsCellVisible() ? wxT("Show cell") : wxT("Hide cell")) );
  OnStateChange->Execute((AEventsDispatcher*)this, &sc);
}
//..............................................................................
void TMainForm::BasisVChange()  {
  TStateChange sc(prsBasisVis, FXApp->IsBasisVisible() );
  pmModel->SetLabel(ID_BasisVisible, (FXApp->IsBasisVisible() ? wxT("Hide basis") : wxT("Show basis")) );
  OnStateChange->Execute((AEventsDispatcher*)this, &sc);
}
//..............................................................................
void TMainForm::OnCellVisible(wxCommandEvent& event)  {
  FXApp->SetCellVisible( FXApp->IsCellVisible() );
}
//..............................................................................
void TMainForm::OnBasisVisible(wxCommandEvent& event)  {
  FXApp->SetBasisVisible( !FXApp->IsBasisVisible() );
}
//..............................................................................
void TMainForm::OnGraphics(wxCommandEvent& event)  {
  if( FObjectUnderMouse == NULL )  return;

  int i;
  olxstr PName, Tmp;
  TGlGroup *Sel;
  TdlgMatProp *MatProp;
  TdlgPrimitive *Primitives;
  TStrList Ps;
  olxstr TmpStr;

  switch( event.GetId() )  {
    case ID_GraphicsHide:
      if( FObjectUnderMouse->Selected() )
        ProcessXPMacro("kill sel", MacroError);
      else
        FUndoStack->Push( FXApp->SetGraphicsVisible(FObjectUnderMouse, false) );
      TimePerFrame = FXApp->Draw();
      break;
    case ID_GraphicsEdit:
      if( LabelToEdit != NULL )  {
        Tmp = "getuserinput(1, \'Please, enter new label\', \'";
        Tmp << LabelToEdit->GetLabel() << "\')";
        ProcessMacroFunc(Tmp);
        if( !Tmp.IsEmpty() ) {
          LabelToEdit->SetLabel(Tmp);
          FXApp->Draw();
        }
        LabelToEdit = NULL;
      }
      break;
    case ID_GraphicsDS:
      Sel = FXApp->Selection();
      MatProp = new TdlgMatProp(this, FObjectUnderMouse->Primitives(), FXApp);
      if( EsdlInstanceOf(*FObjectUnderMouse, TGlGroup) )
        MatProp->SetCurrent( *((TGlGroup*)FObjectUnderMouse)->GlM() );
      if( FObjectUnderMouse->Selected() )  {
        MatProp->ApplyToGroupEnabled(false);
        MatProp->ApplyToGroupChecked(false);
      }
      else  {
        MatProp->ApplyToGroupEnabled(true);
        MatProp->ApplyToGroupChecked(true);
      }
      if( MatProp->ShowModal() == wxID_OK )  {
        if( EsdlInstanceOf(*FObjectUnderMouse, TGlGroup) )
          ((TGlGroup*)FObjectUnderMouse)->GlM( MatProp->GetCurrent() );
        else if( EsdlInstanceOf( *FObjectUnderMouse, TXAtom) )  {
          FXApp->XAtomDS2XBondDS("Sphere");  
/*          OAtom = (TSAtom*)FObjectUnderMouse;
          if( FObjectUnderMouse->Selected() )
          {
            OAS =  OAtom->Style();
            for( i=0; i < Sel->Count(); i++ )
            {
              GO = Sel->Object(i);
              if( GO == FObjectUnderMouse )  continue;
              if( GO->ClassName() == "TSATOM" )
              {
                SA = (TSAtom*)GO;

                FXApp->CreateAtom(SA); // will create an atom if necessary
                Tmp = FXApp->GetRender().NameOfCollection(SA->Primitives());
                AS = FXApp->GetRender().Styles()->Style(Tmp, true);
                for( j=0; j < OAtom->PrimitiveCount(); j++ )
                {
                  PName = OAtom->PrimitiveName(j);
                  GlM = const_cast<TGlMaterial*>(OAS->Material(PName));
                  AS->PrimitiveMaterial(PName, GlM );
                  SA->Primitive(PName)->SetProperties(GlM);
                }
//                FXApp->UpdateAtomStyle(SA, AS);
              }
            }
          }*/
        }
      }
      MatProp->Destroy();
      TimePerFrame = FXApp->Draw();
      break;
    case ID_GraphicsP:
      FObjectUnderMouse->ListPrimitives(Ps);
      if( Ps.Count() < 2 )  {
        TBasicApp::GetLog().Info("The object does not support requested function...");
        return;
      }
      i = FObjectUnderMouse->Primitives()->Style()->ParameterValue("PMask", 0).ToInt();
      Primitives = new TdlgPrimitive(&Ps, i, this);
      if( Primitives->ShowModal() == wxID_OK )  {
        TmpStr = "mask ";
        TmpStr << FObjectUnderMouse->Primitives()->Name() << ' ' << Primitives->Mask;
        ProcessXPMacro(TmpStr, MacroError);
//        FObjectUnderMouse->UpdatePrimitives(Primitives->Mask);
      }
      Primitives->Destroy();
      break;
    case ID_FixLattice:
      if( EsdlInstanceOf(*FObjectUnderMouse, TXLattice) )  {
        ((TXLattice*)FObjectUnderMouse)->SetFixed(true);
      }
      break;
    case ID_FreeLattice:
      if( EsdlInstanceOf(*FObjectUnderMouse, TXLattice) )  {
        ((TXLattice*)FObjectUnderMouse)->SetFixed(false);
      }
      break;
  }
}
//..............................................................................
void TMainForm::ObjectUnderMouse( AGDrawObject *G)  {
  FObjectUnderMouse = G;
  FCurrentPopup = NULL;
  if( G == NULL )  return;
  FCurrentPopup = NULL;
  if( EsdlInstanceOf( *G, TXAtom) )  {
    TStrList SL;
    olxstr T;
    TXAtom *XA = (TXAtom*)G;
    FXApp->BangList(XA, SL);
    pmBang->Clear();
    for( int i=0; i < SL.Count(); i++ )
      pmBang->Append(-1, uiStr(SL[i]));
    pmAtom->Enable(ID_MenuBang, SL.Count() != 0 );
    T = XA->Atom().GetLabel();
    T << ':' << ' ' <<  XA->Atom().GetAtomInfo().GetName();
    if( XA->Atom().GetAtomInfo().GetIndex() == iQPeakIndex )  {
      T << ": " << olxstr::FormatFloat(3, XA->Atom().CAtom().GetQPeak());
    }
    else  {
      T << " Occu: " << olxstr::FormatFloat(3, XA->Atom().CAtom().GetOccp());
    }
    miAtomInfo->SetText( uiStr(T) );
    pmAtom->Enable(ID_AtomGrowShells, FXApp->AtomExpandable(XA));
    pmAtom->Enable(ID_AtomGrowFrags, FXApp->AtomExpandable(XA));
    pmAtom->Enable(ID_Selection, G->Selected());
    pmAtom->Enable(ID_SelGroup, false);
    FCurrentPopup = pmAtom;
  }
  else if( EsdlInstanceOf( *G, TXBond) )  {
    TStrList SL;
    olxstr T;
    TXBond *XB = (TXBond*)G;
    FXApp->TangList(XB, SL);
    pmTang->Clear();
    for( int i=0; i < SL.Count(); i++ )
      pmTang->Append(0, uiStr(SL[i]));

    pmBond->Enable(ID_MenuTang, SL.Count() != 0 );
    T = XB->Bond().GetA().GetLabel();
    T << '-' << XB->Bond().GetB().GetLabel() << ':' << ' '
      << olxstr::FormatFloat(3, XB->Bond().Length());
    miBondInfo->SetText( uiStr(T) );
    pmBond->Enable(ID_Selection, G->Selected());
    FCurrentPopup = pmBond;
  }
  else if( EsdlInstanceOf( *G, TXPlane) )  {
    FCurrentPopup = pmPlane;
  }
  if( FCurrentPopup != NULL )  {
    FCurrentPopup->Enable(ID_SelGroup, false);
    FCurrentPopup->Enable(ID_SelUnGroup, false);
    if( FXApp->Selection()->Count() > 1 )  {
      FCurrentPopup->Enable(ID_SelGroup, true);
    }
    if( FXApp->Selection()->Count() == 1 )  {
      if( EsdlInstanceOf( *FXApp->Selection()->Object(0), TGlGroup) )  {
        FCurrentPopup->Enable(ID_SelUnGroup, true);
      }
    }
  }
  if( EsdlInstanceOf( *G, TGlGroup) )  {
    pmSelection->Enable(ID_SelGroup, false);
    pmSelection->Enable(ID_SelUnGroup, true);
    FCurrentPopup = pmSelection;
  }
  else if( EsdlInstanceOf( *G, TGlBackground) )  {
    FCurrentPopup = pmMenu;
  }
  else if( EsdlInstanceOf( *G, TXGlLabel) )  {
    FCurrentPopup = pmLabel;
    LabelToEdit = (TXGlLabel*)G;
  }
  else if( EsdlInstanceOf( *G, TXLattice) )  {
    FCurrentPopup = pmLattice;
  }
}
//..............................................................................
void TMainForm::OnAtomTypeChange(wxCommandEvent& event)  {
  TXAtom *XA = (TXAtom*)FObjectUnderMouse;
  if( XA == NULL )  return;
  olxstr Tmp("name ");
  if( XA->Selected() )  Tmp << "sel";
  else                  Tmp << XA->Atom().GetLabel();
  Tmp << ' ';
  switch( event.GetId() )  {
    case ID_AtomTypeChangeC:
      Tmp << 'C';
      break;
    case ID_AtomTypeChangeN:
      Tmp << 'N';
      break;
    case ID_AtomTypeChangeO:
      Tmp << 'O';
      break;
    case ID_AtomTypeChangeF:
      Tmp << 'F';
      break;
    case ID_AtomTypeChangeH:
      Tmp << 'H';
      break;
    case ID_AtomTypeChangeS:
      Tmp << 'S';
      break;
  }
  Tmp << XA->Atom().GetLabel().SubStringFrom( XA->Atom().GetAtomInfo().GetSymbol().Length() );
  ProcessXPMacro(Tmp, MacroError);
  TimePerFrame = FXApp->Draw();
}
//..............................................................................
void TMainForm::OnAtomTypePTable(wxCommandEvent& event)  {
  TXAtom *XA = (TXAtom*)FObjectUnderMouse;
  if( !XA )  return;
  olxstr Tmp = "name ";
  if( XA->Selected() )  Tmp << "sel";
  else                  Tmp << XA->Atom().GetLabel();
  Tmp << ' ';
  TPTableDlg *Dlg = new TPTableDlg(this, FXApp->AtomsInfo());
  if( Dlg->ShowModal() == wxID_OK )  {
    Tmp << Dlg->Selected()->GetSymbol();
    Tmp << XA->Atom().GetLabel().SubStringFrom( XA->Atom().GetAtomInfo().GetSymbol().Length() );
    ProcessXPMacro(Tmp, MacroError);
  }
  Dlg->Destroy();
  TimePerFrame = FXApp->Draw();
}
//..............................................................................
void TMainForm::OnFragmentHide(wxCommandEvent& event)  {
  if( FObjectUnderMouse == NULL )  return;
  TNetPList L;
  if( EsdlInstanceOf(*FObjectUnderMouse, TXAtom) )
    L.Add( &((TXAtom*)FObjectUnderMouse)->Atom().GetNetwork() );
  else if( EsdlInstanceOf(*FObjectUnderMouse, TXBond) )
    L.Add( &((TXBond*)FObjectUnderMouse)->Bond().GetNetwork() );
  else
    return;
  FXApp->FragmentsVisible(L, false);
  FXApp->CenterView();
}
//..............................................................................
void TMainForm::OnShowAll(wxCommandEvent& event)  {
  ProcessXPMacro("fmol", MacroError);
}
//..............................................................................
void TMainForm::OnModelCenter(wxCommandEvent& event)  {
  FXApp->CenterModel();
  TimePerFrame = FXApp->Draw();
}
//..............................................................................
void TMainForm::OnFragmentShowOnly(wxCommandEvent& event)  {
  if( ! FObjectUnderMouse )  return;
  TSAtom *A;
  if( EsdlInstanceOf( *FObjectUnderMouse, TXAtom) )
    A = &((TXAtom*)FObjectUnderMouse)->Atom();
  else if( EsdlInstanceOf( *FObjectUnderMouse, TXBond) )
    A = &((TXBond*)FObjectUnderMouse)->Bond().A();
  else
    return;
  ProcessXPMacro(olxstr("uniq ") + A->GetLabel(), MacroError);
}
//..............................................................................
bool TMainForm::Dispatch( int MsgId, short MsgSubId, const IEObject *Sender, const IEObject *Data)  {
  bool res = true, Silent = (FMode & mSilent) != 0, Draw=false;
  if( Destroying )  {
    FMode = 0;  // to release waitfor 
    return false;
  }
  if( MsgId == ID_GLDRAW )  {
    if( !FBitmapDraw )  
      FGlCanvas->SwapBuffers();
  }
  else if( MsgId == ID_TIMER )  {
    FTimer->OnTimer()->SetEnabled( false );
    // execute tasks ...
    for( int i=0; i < Tasks.Count(); i++ )  {
      if(  (TETime::Now() - Tasks[i].LastCalled) > Tasks[i].Interval )  {
        olxstr tmp( Tasks[i].Task );
        if( !Tasks[i].Repeatable )  {
          Tasks.Delete(i);
          i--;
        }
        else
          Tasks[i].LastCalled = TETime::Now();
        ProcessXPMacro(tmp, MacroError);
        AnalyseError( MacroError );
      }
    }
    // end tasks ...
    if( GetHtml()->PageLoadRequested() && !GetHtml()->IsPageLocked() )
      GetHtml()->ProcessPageLoadRequest();
    FTimer->OnTimer()->SetEnabled( true );
    if( FProcess != NULL )  {
      //FTimer->OnTimer->Enabled = false;
      while( FProcess->StrCount() != 0 )  {
        FGlConsole->PrintText(FProcess->GetString(0), &ExecFontColor);
        CallbackFunc(ProcessOutputCBName, FProcess->GetString(0));
        FProcess->DeleteStr(0);
        Draw = true;
      }
      //FTimer->OnTimer->Enabled = true;
    }
    if( (FMode & mListen) != 0 && TEFile::FileExists(FListenFile) )  {
      static time_t FileMT = wxFileModificationTime( uiStr(FListenFile));
      time_t FileT = wxFileModificationTime( uiStr(FListenFile));
      if( FileMT != FileT )  {
        FObjectUnderMouse = NULL;
        ProcessXPMacro((olxstr("@reap -b -r \'") << FListenFile)+'\'', MacroError);
        // for debug purposes
        if( TEFile::FileExists(DefStyle) )  FXApp->GetRender().Styles()->LoadFromFile(DefStyle);
        for( int i=0; i < FOnListenCmds.Count(); i++ )  {
          ProcessXPMacro(FOnListenCmds.String(i), MacroError);
          if( !MacroError.IsSuccessful() )  break;
        }
        FileMT = FileT;
        if( FOnListenCmds.Count() )  Draw = true;
      }
    }
    if( (FMode & mRota) != 0  )  {
      FXApp->GetRender().Basis()->RotateX(FXApp->GetRender().GetBasis().GetRX()+FRotationIncrement*FRotationVector[0]);
      FXApp->GetRender().Basis()->RotateY(FXApp->GetRender().GetBasis().GetRY()+FRotationIncrement*FRotationVector[1]);
      FXApp->GetRender().Basis()->RotateZ(FXApp->GetRender().GetBasis().GetRZ()+FRotationIncrement*FRotationVector[2]);
      FRotationAngle -= fabs(FRotationVector.Length()*FRotationIncrement);
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
    if( FXApp->GetFader().Visible() )  {
      if( !FXApp->GetFader().Increment() )  {
       //FXApp->GetFader().Visible(false);
      }
      else
        Draw = true;
    }
    if( MouseMoveTimeElapsed < 2500 )
      MouseMoveTimeElapsed += FTimer->GetInterval();
    if( MouseMoveTimeElapsed > 500 && MouseMoveTimeElapsed < 5000 )  {
      AGDrawObject *G = FXApp->SelectObject(MousePositionX, MousePositionY, 0);
      olxstr Tip;
      if( G != NULL )  {
        if( EsdlInstanceOf( *G, TXAtom) )  {
          TXAtom &xa = *(TXAtom*)G;
          Tip = xa.Atom().GetLabel();
          if( xa.Atom().CAtom().GetResiId() != -1 )
            Tip << '_' <<
            xa.Atom().CAtom().GetParent()->GetResidue(xa.Atom().CAtom().GetResiId()).GetNumber();
          if( xa.Atom().GetAtomInfo() == iQPeakIndex )  {
            Tip << ':' << xa.Atom().CAtom().GetQPeak();
          }
        }
        else  if( EsdlInstanceOf( *G, TXBond) )  {
          Tip = ((TXBond*)G)->Bond().GetA().GetLabel();
          Tip << '-' << ((TXBond*)G)->Bond().GetB().GetLabel() << ": ";
          Tip << olxstr::FormatFloat(3, ((TXBond*)G)->Bond().Length());
        } else if( EsdlInstanceOf( *G, TXReflection) )  {
          Tip = ((TXReflection*)G)->Reflection()->GetH();
          Tip << ' ';
          Tip << ((TXReflection*)G)->Reflection()->GetK() << ' ';
          Tip << ((TXReflection*)G)->Reflection()->GetL() << ": ";
          Tip << ((TXReflection*)G)->Reflection()->GetI();
        }
        else if( EsdlInstanceOf( *G, TXLine) )  {
          Tip = olxstr::FormatFloat(3, ((TXLine*)G)->Length());
        }
        else if( EsdlInstanceOf( *G, TXGrowLine) )  {
          Tip = ((TXGrowLine*)G)->SAtom()->GetLabel();
          Tip << '-' << ((TXGrowLine*)G)->CAtom()->GetLabel() << ": " <<
            olxstr::FormatFloat(3, ((TXGrowLine*)G)->Length());
        }
        else if( EsdlInstanceOf( *G, TXGrowPoint) )  {
          Tip = TSymmParser::MatrixToSymm( ((TXGrowPoint*)G)->GetTransform() );
        }
#if defined (__WIN32__)
        FGlCanvas->SetToolTip( Tip.u_str());
        SetToolTip( Tip.u_str());
#else
        if( GlTooltip != NULL && !Tip.IsEmpty() )  {
          GlTooltip->Clear();
          GlTooltip->PostText( Tip );
          GlTooltip->SetLeft(MousePositionX+4); // put it off the mouse
          GlTooltip->SetTop(MousePositionY-GlTooltip->GetHeight()-4);
          GlTooltip->SetZ( FXApp->GetRender().GetMaxRasterZ() -0.1 );
          GlTooltip->Visible(true);
          Draw = true;
        }
        else if( GlTooltip != NULL )  {
          GlTooltip->Visible(false);
          Draw = true;
        }
#endif
      }
      if( DrawSceneTimer >= 0 && !Draw )  {
        DrawSceneTimer -= FTimer->GetInterval();
        if( DrawSceneTimer < 0 )  {  TimePerFrame = FXApp->Draw();  }
      }
      MouseMoveTimeElapsed = 5000;
    }
    if( Draw )  {
      TimePerFrame = FXApp->Draw();
    }
  }
  else if( MsgId == ID_XOBJECTSDESTROY )  {
    if( Modes->GetCurrent() != NULL ) Modes->GetCurrent()->OnGraphicsDestroy();
  }
  else if( MsgId == ID_CMDLINECHAR )  {
    if( Data != NULL && EsdlInstanceOf(*Data, TKeyEvent) )
      this->OnChar( ((TKeyEvent*)Data)->GetEvent() );
  }
  else if( MsgId == ID_CMDLINEKEYDOWN )  {
    if( Data != NULL && EsdlInstanceOf(*Data, TKeyEvent) )
      this->OnKeyDown( ((TKeyEvent*)Data)->GetEvent() );
  }
  else if( MsgId == ID_INFO || MsgId == ID_WARNING || MsgId == ID_ERROR || MsgId == ID_EXCEPTION && (MsgSubId == msiEnter))  {
    if( Data != NULL )  {
      TGlMaterial *glm = NULL;
      if( MsgId == ID_INFO )           glm = &InfoFontColor;
      else if( MsgId == ID_WARNING )   glm = &WarningFontColor;
      else if( MsgId == ID_ERROR )     glm = &ErrorFontColor;
      else if( MsgId == ID_EXCEPTION ) glm = &ExceptionFontColor;
      if( !( (FMode&mSilent) != 0 &&  (MsgId == ID_INFO || MsgId == ID_WARNING))
            || (MsgId == ID_ERROR || MsgId == ID_EXCEPTION) )  {
        FGlConsole->OnPost->SetEnabled(false); // the proporgation will happen after we return false
        FGlConsole->PrintText( Data->ToString(), glm, true);
        FGlConsole->PrintText( EmptyString );
        FGlConsole->OnPost->SetEnabled(true);
        TimePerFrame = FXApp->Draw();
      }
      FGlConsole->SetSkipPosting(true);
      res = false;  // propargate to other streams, logs in particular
    }
  }
  else if( MsgId == ID_ONLINK )  {
    if( Data != NULL )  {
      TStrList Toks( *(olxstr*)Data, ">>" );
      //GetHtml()->LockPageLoad();
      /* the page, if requested, will beloaded on time event. The timer is disabled
      in case if a modal window appears and the timer event can be called */
      FTimer->OnTimer()->SetEnabled( false );
      for( int i=0; i < Toks.Count(); i++ )  {
        ProcessXPMacro(olxstr::DeleteSequencesOf<char>(Toks[i], ' '), MacroError);
        if( !MacroError.IsSuccessful() )  {
          //AnalyseError( MacroError );  // it is already done in the ProcessMacro
          break;
        }
      }
      TimePerFrame = FXApp->Draw();
      // enabling the timer back
      // retrun fucus to the main window, but let typing in the comboboxes
      if( Sender != NULL )  {
        if( Data == NULL || ((olxstr*)Data)->Length() == 0 )
          ;
        else if( EsdlInstanceOf(*Sender, TComboBox) && !((TComboBox*)Sender)->IsReadOnly() )
          ;
        else if( EsdlInstanceOf(*Sender, TTreeView) )
          ;
        else if( EsdlInstanceOf(*Sender, TTextEdit) )
          ;
        else if( EsdlInstanceOf(*Sender, TSpinCtrl) )
          ;
        else
          FGlCanvas->SetFocus();
      }
      else
        FGlCanvas->SetFocus();
      FTimer->OnTimer()->SetEnabled( true );
    }
  }
  else if( MsgId == ID_HTMLCMD )  {
    if( Data != NULL )  {
      FMode |= mSilent;
      res = ProcessMacroFunc(*(olxstr*)Data);
      if( !Silent )  FMode ^= mSilent;
    }
  }
  else if( MsgId == ID_HTMLKEY )  {
    FGlCanvas->SetFocus();
    OnChar( ((TKeyEvent*)Data)->GetEvent() );
  }
  else if( MsgId == ID_HTMLDBLCLICK )  {
    TPopupData *pd = NULL;
    for( int i=0; i < FPopups.Count(); i++ )  {
      pd = FPopups.Object(i);
      if( dynamic_cast<const THtml*>(Sender) == pd->Html  )  break;
    }
    bool processed = false;
    if( pd != NULL && (dynamic_cast<const THtml*>(Sender) == (void*)pd->Html) )  {
      if( !pd->OnDblClick.IsEmpty() )  {
        ProcessXPMacro(pd->OnDblClick, MacroError);
        processed = true;
      }
    }
  }
  else if( MsgId == ID_PROCESSTERMINATE )  SetProcess(NULL);
  else if( MsgId == ID_TEXTPOST )  {
    if( Data != NULL )  {
      FGlConsole->SetSkipPosting(true);
      TBasicApp::GetLog() << Data->ToString() << '\n';
      FGlConsole->SetSkipPosting(false);
      if( ActiveLogFile != NULL )
        ActiveLogFile->Writenl( Data->ToString() );
    }
  }
  else if( MsgId == ID_COMMAND )  {
    olxstr tmp;
    if( CmdLineVisible && EsdlInstanceOf( *Sender, TCmdLine ) )
        tmp = FCmdLine->GetCommand();
    else if( EsdlInstanceOf( *Sender, TGlConsole ) )
        tmp = FGlConsole->GetCommand();
    if( !tmp.IsEmpty() )  {
      if( FProcess != NULL && FProcess->IsRedirected() )  {  // here we do not need to remember the command
        FProcess->Write(tmp);
        FProcess->Writenl();
        TimePerFrame = FXApp->Draw();
        FGlConsole->SetCommand(EmptyString);
      }
      else  {
        FHelpWindow->Visible(false);
        olxstr FullCmd(tmp);
        ProcessXPMacro(FullCmd, MacroError);
        // this is done in faivor of SetCmd macro, which supposed to modify the command ...
        if( !CmdLineVisible )
          if( FGlConsole->GetCommand() == tmp )
             FGlConsole->SetCommand(EmptyString);
          else
            FCmdLine->SetCommand( EmptyString );
      }
    }
  }
  return res;
}
//..............................................................................
void TMainForm::OnAtom(wxCommandEvent& event)  {
  TXAtom *XA = (TXAtom*)FObjectUnderMouse;
  if( XA == NULL )  return;
  olxstr Tmp;
  switch( event.GetId() )  {
    case ID_AtomGrowShells:
      Tmp = "grow -s "; Tmp << XA->Atom().GetLabel();
      ProcessXPMacro(Tmp, MacroError);
      break;
    case ID_AtomGrowFrags:
      Tmp = "grow ";   Tmp << XA->Atom().GetLabel();
      ProcessXPMacro(Tmp, MacroError);
      break;
  }
}
//..............................................................................
void TMainForm::OnPlane(wxCommandEvent& event)  {
  TXPlane *XP = (TXPlane*)FObjectUnderMouse;
  if( !XP )  return;
  switch( event.GetId() )  {
    case ID_PlaneActivate:
    ProcessXPMacro(olxstr("activate ") << XP->Primitives()->Name(), MacroError);
    break;
  }
}
//..............................................................................
void TMainForm::PreviewHelp(const olxstr& Cmd)  {
  if( !HelpWindowVisible )  return;
  olxstr Tmp;
  if( !Cmd.IsEmpty() && (FHelpItem != NULL))  {
    TPtrList<TDataItem> SL;
    TDataItem *Item, *Cat;
    FHelpItem->FindSimilarItems(Cmd, SL);
    if( FMacroItem != NULL )
      FMacroItem->FindSimilarItems(Cmd, SL);
    if( SL.Count() != 0 )  {
      FHelpWindow->Visible( HelpWindowVisible );
      FHelpWindow->Clear();
      FGlConsole->ShowBuffer( !HelpWindowVisible );
      FHelpWindow->SetTop( InfoWindowVisible ? FInfoBox->GetTop() + FInfoBox->GetHeight() + 5 : 1 );
      FHelpWindow->SetMaxStringLength( FHelpWindow->Font()->MaxTextLength(FXApp->GetRender().GetWidth()) );
      FHelpWindow->SetZ( FXApp->GetRender().GetMaxRasterZ()-0.1);
      for( int i=0; i < SL.Count(); i++ )  {
        Item = SL[i];
        Tmp = Item->GetName();
        FHelpWindow->PostText(Tmp, &HelpFontColorCmd);
        Tmp = Item->GetFieldValueCI("help");
        if( !Tmp.IsEmpty() )  {
          FHelpWindow->PostText(Tmp, &HelpFontColorTxt);
          Cat = Item->FindItem("category");
          if( Cat != NULL  )  {
            olxstr Categories;
            for( int j=0; j < Cat->ItemCount(); j++ )  {
              Categories << Cat->Item(j).GetName();
              if( (j+1) < Cat->ItemCount() )  Categories << ", ";
            }
            if( !Categories.IsEmpty() )  {
              Categories.Insert("\t", 0);
              FHelpWindow->PostText("\tCategory", &HelpFontColorCmd);
              FHelpWindow->PostText(Categories, &HelpFontColorTxt);
            }
          }
        }
      }
    }
    else  {
      FHelpWindow->Visible(false);
      FGlConsole->ShowBuffer(true);
    }
  }
  else  {
    FHelpWindow->Visible(false);
    FGlConsole->ShowBuffer(true);
  }
}
//..............................................................................
void TMainForm::OnChar( wxKeyEvent& m )  {

  short Fl=0, inc=3;
  olxstr Cmd, FullCmd;
  if( m.m_altDown )      Fl |= sssAlt;
  if( m.m_shiftDown )    Fl |= sssShift;
  if( m.m_controlDown )  Fl |= sssCtrl;
  // Alt + Up,Down,Left, Right - rotation, +Shift - speed
  if( ((Fl & sssShift)) || (Fl & sssAlt) )  {
    if( (Fl & sssShift) )  inc = 7;
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
      if( FXApp->GetRender().GetZoom()+inc/3 < 400 )  {
        FXApp->GetRender().SetZoom( FXApp->GetRender().GetZoom()+inc/3 );
        TimePerFrame = FXApp->Draw();
        return;
      }
    }
    if( m.m_keyCode == WXK_HOME )  {
      if( FXApp->GetRender().GetZoom()-inc/3 >= 0 )  {
        FXApp->GetRender().SetZoom(FXApp->GetRender().GetZoom()-inc/3);
        TimePerFrame = FXApp->Draw();
        return;
      }
    }
  }
  // Ctrl + Up, Down - browse solutions
  if( (Fl & sssCtrl) != 0  )  {
    if( m.m_keyCode == WXK_UP && ((FMode&mSolve) == mSolve) )  {
      ChangeSolution( CurrentSolution - 1 );
      return;
    }
    if( m.m_keyCode == WXK_DOWN  && ((FMode&mSolve) == mSolve) )  {
      ChangeSolution( CurrentSolution + 1 );
      return;
    }
  }
  if( (Fl&sssCtrl) && m.GetKeyCode() == 'v'-'a'+1 )  {  // paste
    if( wxTheClipboard->Open() )  {
      if (wxTheClipboard->IsSupported(wxDF_TEXT) )  {
        wxTextDataObject data;
        wxTheClipboard->GetData( data );
        olxstr Tmp = FGlConsole->GetCommand();
        FGlConsole->SetCommand(Tmp + data.GetText().c_str());
        TimePerFrame = FXApp->Draw();
      }
      wxTheClipboard->Close();
    }
    return;
  }
  if( (Fl&sssCtrl) && m.GetKeyCode() == 'z'-'a'+1 )  {  // Ctrl+Z
    ProcessXPMacro("undo", MacroError);
    TimePerFrame = FXApp->Draw();
    return;
  }
  if( (Fl&sssCtrl) && m.GetKeyCode() == 'c'-'a'+1 )  {  // Ctrl+C
    if( FProcess )  {
      FProcess->OnTerminate->Clear();
      if( FProcess->Terminate() )
        TBasicApp::GetLog().Info("Process has been successfully terminated...");
      else
        TBasicApp::GetLog().Info("Could not terminate the process...");
      FProcess->Detach();
      FProcess = NULL;
      TimePerFrame = FXApp->Draw();
      return;
    }
    return;
  }
  if( m.GetKeyCode() == WXK_RETURN )  {
    if( FMode & mSolve )  {
      FMode ^= mSolve;
      TBasicApp::GetLog().Info("Model is set to current solution");
    }
  }
  if( m.GetKeyCode() == WXK_ESCAPE )  {  // escape
    if( Modes->GetCurrent() != NULL )  {
      if( Modes->GetCurrent()->OnKey( m.GetKeyCode(), Fl) )
        return;
      else
        ProcessXPMacro("mode off", MacroError);
    }
    ProcessXPMacro("sel -u", MacroError);
    TimePerFrame = FXApp->Draw();
//    return;
  }
  if( m.GetKeyCode() == WXK_TAB )  {  // tab
    Cmd = FGlConsole->GetCommand();
    int spi = Cmd.LastIndexOf(' ');
    if( spi != -1 )  {
      FullCmd = ExpandCommand(Cmd.SubStringFrom(spi+1));
      if( FullCmd != Cmd.SubStringFrom(spi+1) )
        FullCmd = Cmd.SubStringTo(spi+1) << FullCmd;
      else
        FullCmd = EmptyString;
    }
    else
      FullCmd = ExpandCommand(Cmd);
    if( FullCmd.Length() && (FullCmd != Cmd) )
      FGlConsole->SetCommand(FullCmd);
    TimePerFrame = FXApp->Draw();
    return;
  }

  if( FGlConsole->ProcessKey(m.GetKeyCode(), Fl) )  {
    m.Skip(false);
    PreviewHelp( FGlConsole->GetCommand() );
    TimePerFrame = FXApp->Draw();
    return;
  }

  if( FProcess != NULL && FProcess->IsRedirected() )  {
    FHelpWindow->Visible(false);
    FGlConsole->ShowBuffer(true);
    TimePerFrame = FXApp->Draw();
    return;
  }
  if( !CmdLineVisible )
    Cmd = FGlConsole->GetCommand();
  else  {
    if( FCmdLine->ProcessKey( m ) )  {
      m.Skip(false);
    }
    else
      m.Skip(true);

    Cmd = FCmdLine->GetCommand();
    Cmd << (char)m.GetKeyCode();
  }
  //PreviewHelp( Cmd );

  // if we preview the help - the drawing should happen, which makes the external command line
  // much less usefull ... 

/*  if( FXApp->GetRender().GlImageChanged() )
  {
    FGlConsole->Visible(false);
    bool HV = FHelpWindow->Visible();
    FHelpWindow->Visible(false);
    FXApp->Draw();
    FXApp->GetRender().UpdateGlImage();
    FGlConsole->Visible(true);
    FHelpWindow->Visible(HV);
  }
  FXApp->GetRender().DrawObject(NULL, true);  
  if( FHelpWindow->Visible() )
  {  FXApp->GetRender().DrawObject(FHelpWindow);  }
  FXApp->GetRender().DrawObject(FGlConsole);
  FXApp->GetRender().DrawObject();  // causes OnDrawEvent
  if( FHelpWindow->Visible() )  {  FGlConsole->ShowBuffer(false);  }
  else
  {  FGlConsole->ShowBuffer(true);  }*/
  if( m.GetKeyCode() == WXK_RETURN ) {
    TimePerFrame = FXApp->Draw();
  }
  else  {
    //if( KeyEllapsedTime > TimePerFrame )
    if( !CmdLineVisible )
    {  TimePerFrame = FXApp->Draw();  }
//    else
    {  DrawSceneTimer = TimePerFrame;  }
  }
}
//..............................................................................
void TMainForm::OnKeyUp(wxKeyEvent& m)  {
  m.Skip();
}
//..............................................................................
void TMainForm::OnKeyDown(wxKeyEvent& m)  {
  if( CmdLineVisible )  {
    if( this->FindFocus() != (wxWindow*)FCmdLine )  {
      m.Skip(false);
      FCmdLine->EmulateKeyPress( m );
    }
  }
  short Fl = 0;
  if( m.m_keyCode == WXK_CONTROL || m.m_keyCode == WXK_MENU || m.m_keyCode == WXK_SHIFT )  {
    m.Skip();
    return;
  }
  if( m.m_altDown )      Fl |= sssAlt;
  if( m.m_shiftDown )    Fl |= sssShift;
  if( m.m_controlDown )  Fl |= sssCtrl;

  if( !AccShortcuts.ValueExists( Fl<<16 | m.m_keyCode ) )  {
    m.Skip();  return;
  }
  if( FGlConsole->WillProcessKey(m.GetKeyCode(), Fl) )  {
    m.Skip();
    return;
  }
  olxstr Cmd = AccShortcuts.GetValue( Fl<<16 | m.m_keyCode );
  if( Cmd.Length() )  {
    ProcessXPMacro(Cmd, MacroError );
    TimePerFrame = FXApp->Draw();
    return;
  }

  m.Skip();
}
//..............................................................................
void TMainForm::OnSelection(wxCommandEvent& m)
{
  TGlGroup *GlR = NULL;
  if( EsdlInstanceOf( *FObjectUnderMouse, TGlGroup) )
    GlR = (TGlGroup*)FObjectUnderMouse;
  switch( m.GetId() )
  {
    case ID_SelGroup:
      ProcessXPMacro("group sel", MacroError);
//      FXApp->GroupSelection();
      break;
    case ID_SelUnGroup:
      if( GlR ) FXApp->UnGroup(GlR);
      else      FXApp->UnGroupSelection();
      break;
  }
}
//..............................................................................
void TMainForm::OnGraphicsStyle(wxCommandEvent& event)
{
  if( event.GetId() == ID_GStyleSave )
  {
    olxstr FN = PickFile("Drawing style",
    "Drawing styles|*.glds", StylesDir, false);
    if( FN.Length() )
    { ProcessXPMacro(olxstr("save style ") << FN, MacroError);  }
  }
  if( event.GetId() == ID_GStyleOpen )
  {
    olxstr FN = PickFile("Drawing style",
    "Drawing styles|*.glds", StylesDir, true);
    if( FN.Length() )
    { ProcessXPMacro(olxstr("load style ") << FN, MacroError);  }
  }
}
//..............................................................................
void TMainForm::OnSize(wxSizeEvent& event)  {
  wxFrame::OnSize(event);
  if( FXApp == NULL || FGlConsole == NULL || FInfoBox == NULL || !StartupInitialised )  return;
  OnResize();
}
//..............................................................................
void TMainForm::OnResize()  {
  int w=0, h=0, l=0;
  int dheight = InfoWindowVisible ? FInfoBox->GetHeight() : 1;
  GetClientSize(&w, &h);

//#ifdef __X11__
//  h -= 100;
//#endif

  FInfoBox->SetTop(1);
  if( FHtmlMinimized )  {
    if( FHtmlOnLeft )  {
      FHtml->SetSize(0, 0, 10, h);
      l = 10;
      w = w - l;
    }
    else  {
      FHtml->SetSize(w-10, 0, 10, h);
      w = w-10;
    }
  }
  else  {
    FHtml->Freeze();
    int cw, ch;
    if( FHtmlOnLeft )  {
      FHtml->SetSize(0, 0, (int)FHtmlPanelWidth, h);
      FHtml->GetClientSize(&cw, &ch);
      cw = FHtmlWidthFixed ? (int)FHtmlPanelWidth : (int)(w*FHtmlPanelWidth);
      FHtml->SetClientSize(cw, h);
      l = FHtml->GetSize().GetWidth();  // new left
      FHtml->SetSize(0, 0, l, h);  // final iteration ....
      w -= l;  // new width
    }
    else  {
      FHtml->SetSize((int)(w-FHtmlPanelWidth), 0, (int)FHtmlPanelWidth, h);
      FHtml->GetClientSize(&cw, &ch);
      cw = FHtmlWidthFixed ? (int)FHtmlPanelWidth : (int)(w*FHtmlPanelWidth);
      FHtml->SetClientSize(cw, ch);
      FHtml->SetSize(w-FHtml->GetSize().GetWidth(), 0, FHtml->GetSize().GetWidth(), h);
      w -= FHtml->GetSize().GetWidth();
    }
    FHtml->Refresh();
    FHtml->Update();
    FHtml->Thaw();
  }
  if( CmdLineVisible )  {
    FCmdLine->WI.SetWidth( w );
    FCmdLine->WI.SetLeft( l );
    FCmdLine->WI.SetTop( h - FCmdLine->WI.GetHeight() );
  }
  FGlConsole->SetTop(dheight);
  FGlCanvas->SetSize(l, 0, w, h - (CmdLineVisible ? FCmdLine->WI.GetHeight() : 0) );
  FGlCanvas->GetClientSize(&w, &h);
  FXApp->GetRender().Resize(0, 0, w, h, 1);
  FGlConsole->SetLeft(0);
  FGlConsole->SetWidth(w);
  FGlConsole->SetHeight(h - dheight );
  FInfoBox->SetWidth(w);
  FInfoBox->SetLeft(0);
}
//..............................................................................
olxstr TMainForm::ExpandCommand(const olxstr &Cmd)  {
  olxstr FullCmd(Cmd);
  if( !Cmd.IsEmpty() && FMacroItem != NULL )  {
    TPtrList<TDataItem> SL;
    FMacroItem->FindSimilarItems(Cmd, SL);
    if( SL.Count() == 1 )
      FullCmd = SL[0]->GetName();
    else if( SL.IsEmpty() && FHelpItem != NULL )  {
      FHelpItem->FindSimilarItems(Cmd, SL);
      if( SL.Count() == 1 )
        FullCmd = SL[0]->GetName();
    }
  }
  if( FullCmd == Cmd )  {  // try buil-ins
    TBasicFunctionPList bins;  // builins
    GetLibrary().FindSimilarMacros(Cmd, bins);
    GetLibrary().FindSimilarFunctions(Cmd, bins);
    if( bins.Count() == 1 )  {
      FullCmd = bins[0]->GetQualifiedName();
    }
  }
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
      for(int i=0; i < MF->GetOptions().Count(); i++ )  {
        FGlConsole->PrintText( olxstr("   ") << MF->GetOptions().GetComparable(i) << " - "
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
  
  if( !Cmd.IsEmpty() && FHelpItem != NULL )  {
    TDataItem *Item;
    olxstr Tmp, FV;
    Item = FHelpItem->FindItem(Cmd);
    if( Item == NULL && FMacroItem != NULL )
      Item = FMacroItem->FindItem(Cmd);
    if( Item == NULL )  return;
    if( !Full)
      FGlConsole->PrintText(Item->GetFieldValue("help"));
    else  {
      int helpIndex = Item->FieldIndexCI("help");
      if( helpIndex == -1 )  return;
      TDataItem *Cat = Item->FindItemCI("category");
      FGlConsole->PrintText(Item->Field(helpIndex), &HelpFontColorTxt);
      FGlConsole->PrintText("Options:", &HelpFontColorCmd);
      for( int i=0; i < Item->FieldCount(); i++ )  {
        if( i == helpIndex )  continue;
        FGlConsole->PrintText((Item->FieldName(i) + ": ") + Item->Field(i));
      }
      if( Cat != NULL )  {
        olxstr Cats;
        for( int i=0; i < Cat->ItemCount(); i++ )  {
          Cats << Cat->Item(i).GetName();
          if( (i+1) < Cat->ItemCount() )
            Cats << ", ";
        }
        if( !Cats.IsEmpty() )
          FGlConsole->PrintText(olxstr("Macro category: ") << Cats, &HelpFontColorCmd );
        else
          FGlConsole->PrintText("Macro category is not assigned..", &HelpFontColorCmd);
      }
    }
  }
}
//..............................................................................
void TMainForm::SaveSettings(const olxstr &FN)  {
  TDataFile DF;
  TDataItem *I = DF.Root().AddItem("Folders");
  I->AddField("Styles", StylesDir);
  I->AddField("SceneP", SParamDir);
  I->AddField("Current", CurrentDir);
  I->AddField("CifTemplates", FXApp->GetCifTemplatesDir());

  I = DF.Root().AddItem("HTML");
  I->AddField("Minimized", FHtmlMinimized);
  I->AddField("OnLeft", FHtmlOnLeft);
  if( !FHtmlWidthFixed )
    I->AddField("Width", olxstr(FHtmlPanelWidth) << '%');
  else
    I->AddField("Width", FHtmlPanelWidth);
  I->AddField("Tooltips", FHtml->GetShowTooltips() );
  I->AddField("Borders", FHtml->GetBorders() );
  {
    olxstr normal, fixed;
    FHtml->GetFonts(normal, fixed);
    I->AddField("NormalFont", normal );
    I->AddField("FixedFont", fixed );
  }

  I = DF.Root().AddItem("Windows");
  I->AddField("Help", HelpWindowVisible);
  I->AddField("Info", InfoWindowVisible);
  I->AddField("CmdLine", CmdLineVisible);

  I = DF.Root().AddItem("Defaults");
  I->AddField("Style", DefStyle);
  I->AddField("SceneP", DefSceneP);

  I->AddField("BgColor", FBgColor.ToString());
  I->AddField("WhiteOn", (FXApp->GetRender().LightModel.ClearColor().GetRGB() == 0xffffffff) );
  I->AddField("Gradient", FXApp->GetRender().Background()->Visible() );
  I->AddField("GradientPicture", GradientPicture );
  I->AddField("language", Dictionary.GetCurrentLanguage() );
  I->AddField("ExtraZoom", FXApp->GetExtraZoom() );

  I = DF.Root().AddItem("Recent_files");
  for( int i=0; i < olx_min(FRecentFilesToShow, FRecentFiles.Count()); i++ )
    I->AddField(olxstr("file") << i, FRecentFiles[i]);

  I = DF.Root().AddItem("Stored_params");
  for( int i=0; i < StoredParams.Count(); i++ )  {
    TDataItem* it = I->AddItem( StoredParams.GetComparable(i) );
    it->AddField("value", StoredParams.GetObject(i) );
  }

  SaveScene(DF.Root().AddItem("Scene"));
  FXApp->GetRender().Styles()->ToDataItem(DF.Root().AddItem("Styles"));
  DF.SaveToXLFile(FN);
}
//..............................................................................
void TMainForm::LoadSettings(const olxstr &FN)  {

  if( !TEFile::FileExists(FN) ) return;

  TDataFile DF;
  TStrList Log;
  olxstr Tmp;
  DF.LoadFromXLFile(FN, &Log);

  TDataItem *I = DF.Root().FindItem("Folders");
  if( I == NULL )
    return;
  StylesDir = I->GetFieldValue("Styles");
    executeFunction(StylesDir, StylesDir);
  SParamDir = I->GetFieldValue("SceneP");
    executeFunction(SParamDir, SParamDir);
  CurrentDir = I->GetFieldValue("Current");
    executeFunction(CurrentDir, CurrentDir);
  olxstr CifTemplatesDir( I->GetFieldValue("CifTemplates", EmptyString) );
  if( CifTemplatesDir.IsEmpty() )
    CifTemplatesDir = TutorialDir + "CIF/";
  else
    executeFunction(CifTemplatesDir, CifTemplatesDir);
  // to fix old folder location at the basedir ... 
  if( !TEFile::FileExists(CifTemplatesDir) )
    CifTemplatesDir = TutorialDir + "CIF/";
  FXApp->SetCifTemplatesDir( CifTemplatesDir );

  I = DF.Root().FindItem("HTML");
  if( I != NULL )  {
    Tmp = I->GetFieldValue("Minimized");
    FHtmlMinimized = Tmp.IsEmpty() ? false : Tmp.ToBool();
    Tmp = I->GetFieldValue("OnLeft");
    FHtmlOnLeft = Tmp.IsEmpty() ? true : Tmp.ToBool();

    Tmp = I->GetFieldValue("Width");
    if( !Tmp.IsEmpty() )  {
      FHtmlWidthFixed = !Tmp.EndsWith('%');
      FHtmlPanelWidth = ((!FHtmlWidthFixed) ? Tmp.SubStringTo(Tmp.Length()-1).ToDouble() :
                                              Tmp.ToDouble());
      if( !FHtmlWidthFixed && FHtmlPanelWidth >= 0.5 )  FHtmlPanelWidth = 0.25;
    }
    else
      FHtmlPanelWidth = 0.25;

    Tmp = I->GetFieldValue("Tooltips", EmptyString);
    if( !Tmp.IsEmpty() )
      FHtml->SetShowTooltips( Tmp.ToBool() );

    Tmp = I->GetFieldValue("Borders");
    if( !Tmp.IsEmpty() && Tmp.IsNumber() )  FHtml->SetBorders( Tmp.ToInt() );

    olxstr nf( I->GetFieldValue("NormalFont", EmptyString) );
    olxstr ff( I->GetFieldValue("FixedFont", EmptyString) );
    FHtml->SetFonts(nf, ff);
  }
  I = DF.Root().FindItem("Windows");
  if( I != NULL )  {
    Tmp = I->GetFieldValue("Help");
    HelpWindowVisible = Tmp.IsEmpty() ? true : Tmp.ToBool();
    Tmp = I->GetFieldValue("Info");
    InfoWindowVisible = Tmp.IsEmpty() ? true : Tmp.ToBool();
    Tmp = I->GetFieldValue("CmdLine", EmptyString);
    CmdLineVisible = Tmp.IsEmpty() ? false : Tmp.ToBool();
  }
  TEFile::ChangeDir(CurrentDir);

  I = DF.Root().FindItem("Recent_files");
  if( I )  {
    MenuFile->AppendSeparator();
    wxMenuItem *mi;
    int i=0;
    TStrList uniqNames;
    olxstr T = I->GetFieldValue( olxstr("file") << i);
    while( !T.IsEmpty() )  {
      if( T.EndsWithi(".ins") || T.EndsWithi(".res") )  {
        T = TEFile::ChangeFileExt(T, EmptyString);
      }
      TEFile::OSPathI(T);
      if( uniqNames.IndexOf(T) == -1 )
        uniqNames.Add(T);
      i++;
      T = I->GetFieldValue(olxstr("file") << i);
    }
    for( int j=0; j < olx_min(uniqNames.Count(), FRecentFilesToShow); j++ )  {
      executeFunction(uniqNames[j], uniqNames[j]);
      MenuFile->AppendCheckItem(ID_FILE0+j, uniqNames[j].u_str());
      mi = MenuFile->FindItemByPosition(MenuFile->GetMenuItemCount()-1);
      FRecentFiles.Add(uniqNames[j], mi);
    }
  }
  if( TEFile::FileExists(DefSceneP) )  {
    TDataFile SDF;
    SDF.LoadFromXLFile(DefSceneP, &Log);
    LoadScene(&SDF.Root());
  }
  else
    LoadScene(DF.Root().FindItem("Scene"));

  if( TEFile::FileExists(DefStyle) )  {
    TDataFile SDF;
    SDF.LoadFromXLFile(DefStyle, &Log);
    FXApp->GetRender().Styles()->FromDataItem(SDF.Root().FindItem("style"));
  }
  else
    FXApp->GetRender().Styles()->FromDataItem(DF.Root().FindItem("Styles"));

  I = DF.Root().FindItem("Defaults");
  DefStyle = I->GetFieldValue("Style");
  DefSceneP = I->GetFieldValue("SceneP");
  // restroring language
  if( TEFile::FileExists( DictionaryFile ) )  {
      Dictionary.SetCurrentLanguage(DictionaryFile, I->GetFieldValue("language", EmptyString) );
  }
  FXApp->SetExtraZoom( I->GetFieldValue("ExtraZoom", "1.25").ToDouble() );

  olxstr T( I->GetFieldValue("BgColor") );
  if( !T.IsEmpty() )  FBgColor.FromString(T);
  bool whiteOn =  I->GetFieldValue("WhiteOn", FalseString).ToBool();
  FXApp->GetRender().LightModel.ClearColor() =  whiteOn ? 0xffffffff : FBgColor.GetRGB();

  T = I->GetFieldValue("Gradient", EmptyString);
  GradientPicture = I->GetFieldValue("GradientPicture", EmptyString);
  if( !T.IsEmpty() ) 
    ProcessXPMacro(olxstr("grad ") << T << " -p=\'" << GradientPicture << '\'', MacroError);

  I = DF.Root().FindItem("Stored_params");
  if( I )  {
    for( int i=0; i < I->ItemCount(); i++ )  {
      TDataItem& pd = I->Item(i);
      ProcessXPMacro( olxstr("storeparam ") << pd.GetName() << ' '
                        << '\'' << pd.GetFieldValue("value") << '\'' << ' '
                        << pd.GetFieldValue("process", EmptyString), MacroError );
    }
  }
}
//..............................................................................
void TMainForm::LoadScene(TDataItem *Root, TGlLightModel *FLM)  {
  if( Root == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "Root=NULL");
  TDataFile F;
  TDataItem *I;
  olxstr FntData;
  if( !FLM )  FLM = &(FXApp->GetRender().LightModel);
  I = Root->FindItem("Scene_Properties");
  if( I == NULL )  {
    TBasicApp::GetLog().Error("Wrong scene parameters file!");
    return;
  }
  FLM->FromDataItem(I);
  FBgColor = FLM->ClearColor();

  I = Root->FindItem("Fonts");
  if( I == NULL )  return;
  for( int i=0; i < I->ItemCount(); i++ )  {
    TDataItem& fi = I->Item(i);
    FXApp->GetRender().Scene()->CreateFont(fi.GetName(), fi.GetFieldValue("id") );
  }
  I = Root->FindItem("Materials");
  if( I != NULL )  {
    TDataItem *ci;
    ci = I->FindItem("Help_txt");
    if( ci != NULL )  HelpFontColorTxt.FromDataItem(ci);
    ci = I->FindItem("Help_cmd");
    if( ci != NULL ) HelpFontColorCmd.FromDataItem(ci);

    ci = I->FindItem("Exec");
    if( ci != NULL ) ExecFontColor.FromDataItem(ci);
    ci = I->FindItem("Info");
    if( ci != NULL ) InfoFontColor.FromDataItem(ci);
    ci =I->FindItem("Warning");
    if( ci != NULL ) WarningFontColor.FromDataItem(ci);
    ci = I->FindItem("Error");
    if( ci != NULL ) ErrorFontColor.FromDataItem(ci);
    ci = I->FindItem("Exception");
    if( ci != NULL ) ExceptionFontColor.FromDataItem(ci);
  }

//  FXApp->GetRender().LightModel = FLM;
  FXApp->GetRender().LoadIdentity();
  FXApp->GetRender().InitLights();
}
//..............................................................................
void TMainForm::SaveScene(TDataItem *Root, TGlLightModel *FLM)  {
  TDataItem *I;
  if( FLM )
    FLM->ToDataItem(Root->AddItem("Scene_Properties"));
  else
    FXApp->GetRender().LightModel.ToDataItem(Root->AddItem("Scene_Properties"));
  I = Root->AddItem("Fonts");
  for( int i=0; i < FXApp->GetRender().Scene()->FontCount(); i++ )  {
    TDataItem* fi = I->AddItem( FXApp->GetRender().Scene()->Font(i)->GetName());
    fi->AddField("id", FXApp->GetRender().Scene()->Font(i)->IdString() );
  }

  I = Root->AddItem("Materials");
  HelpFontColorTxt.ToDataItem(I->AddItem("Help_txt"));
  HelpFontColorCmd.ToDataItem(I->AddItem("Help_cmd"));
  ExecFontColor.ToDataItem(I->AddItem("Exec"));
  InfoFontColor.ToDataItem(I->AddItem("Exception"));
  WarningFontColor.ToDataItem(I->AddItem("Exception"));
  ErrorFontColor.ToDataItem(I->AddItem("Error"));
  ExceptionFontColor.ToDataItem(I->AddItem("Exception"));
}
//..............................................................................
void TMainForm::UpdateRecentFile(const olxstr& fn)  {
  TPtrList<wxMenuItem> Items;
  olxstr FN( (fn.EndsWithi(".ins") || fn.EndsWithi(".res")) ? 
    TEFile::ChangeFileExt(fn, EmptyString) : fn );
  TEFile::OSPathI(FN);
  int index = FRecentFiles.IndexOf(FN);
  wxMenuItem *mi=NULL;
  if( index == -1 )  {
    if( (FRecentFiles.Count()+1) < FRecentFilesToShow )  {
      MenuFile->AppendCheckItem(ID_FILE0+FRecentFiles.Count(), wxT("tmp"));
      mi = MenuFile->FindItemByPosition(MenuFile->GetMenuItemCount()-1);
    }
    FRecentFiles.Insert(0, FN);
    FRecentFiles.Object(0) = mi;
  }
  else  {
    mi = FRecentFiles.Object(index);
    FRecentFiles.Delete(index);
    FRecentFiles.Insert(0, FN);
    FRecentFiles.Object(0) = mi;
  }
  for( int i=0; i < FRecentFiles.Count(); i++ )
    Items.Add( FRecentFiles.Object(i) ); 
  for( int i=0; i < FRecentFiles.Count(); i++ )  {  // put items in the right position
    mi = Items[i];
    if( mi != NULL )  FRecentFiles.Object(mi->GetId()-ID_FILE0) = mi;
  }
  for( int i=0; i < FRecentFiles.Count(); i++ )  {  // change item captions
    mi = FRecentFiles.Object(i);
    if( mi != NULL )  {
      mi->SetText( FRecentFiles[i].u_str() ) ;
      mi->Check(false);
    }
  }
  FRecentFiles.Object(0)->Check( true );
  if( FRecentFiles.Count() >= FRecentFilesToShow )
    FRecentFiles.SetCount(FRecentFilesToShow);
}
//..............................................................................
bool TMainForm::RecentFilesTable(const olxstr &FN, bool TableDef)  {
  TTTable<TStrList> Table;
  TStrList Output;
  int tc=0;
  olxstr Tmp;
  if( FRecentFiles.Count()%3 )  tc++;
  Table.Resize(FRecentFiles.Count()/3+tc, 3);
  for( int i=0; i < FRecentFiles.Count(); i++ )  {
    Tmp = "<a href=\"reap \'";
    Tmp << TEFile::OSPath(FRecentFiles[i]) << "\'\">";
    Tmp << TEFile::ExtractFileName(FRecentFiles[i]) << "</a>";
    Table[i/3].String(i%3) = Tmp;
  }
  Table.CreateHTMLList(Output, EmptyString, false, false, false);
  TUtf8File::WriteLines( FN, Output, false );
  return true;
}
//..............................................................................
int SortQPeak( const TXAtom* a1, const TXAtom* a2)  {
  double v =  a2->Atom().CAtom().GetQPeak() - a1->Atom().CAtom().GetQPeak();
  if( v < 0 )  return -1;
  if( v > 0 )  return 1;
  return 0;
}
bool TMainForm::QPeaksTable(const olxstr &FN, bool TableDef)  {
  TTTable<TStrList> Table;
  TXAtomPList Atoms;
  olxstr Tmp;
  TStrList Output;
  FXApp->FindXAtoms("$Q", Atoms);
  if( Atoms.IsEmpty() )  {
    Table.Resize(1, 3);
    Table[0].String(0) = "N/A";
    Table[0].String(1) = "N/A";
    if( !EsdlInstanceOf(*FXApp->XFile().GetLastLoader(), TIns) )
      Table[0].String(2) = "No Q-Peaks";
    else
      Table[0].String(1) = "N/A in this file format";
    Table.CreateHTMLList(Output, EmptyString, false, false, TableDef);
    TUtf8File::WriteLines( FN, Output, false );
    return false;
  }
  Atoms.QuickSorter.SortSF(Atoms, SortQPeak);
  Table.Resize( olx_min(10, Atoms.Count()), 3);
  double LQP = olx_max(0.01, Atoms[0]->Atom().CAtom().GetQPeak() );
  int rowIndex = 0;
  for( int i=0; i < Atoms.Count(); i++, rowIndex++ )  {
    if( i > 8 )  i = Atoms.Count() -1;
    Table[rowIndex].String(0) = Atoms[i]->Atom().GetLabel();
    Table[rowIndex].String(1) = olxstr::FormatFloat(3, Atoms[i]->Atom().CAtom().GetQPeak());
    Tmp = "<a href=\"sel -i ";
    if( i > rowIndex )
      Tmp << Atoms[rowIndex]->Atom().GetLabel() << " to ";
    Tmp << Atoms[i]->Atom().GetLabel();
    if( Atoms[i]->Atom().CAtom().GetQPeak() < 2 )
      Tmp << "\"><img border=\"0\" src=\"gui/images/bar_small.gif\" height=\"10\" width=\"";
    else
      Tmp << "\"><img border=\"0\" src=\"gui/images/bar_large.gif\" height=\"10\" width=\"";
    Tmp << olxstr::FormatFloat(1, Atoms[i]->Atom().CAtom().GetQPeak()*100/LQP);
    Tmp << "%\"></a>";
    Table[rowIndex].String(2) = Tmp;
  }

  Table.CreateHTMLList(Output, EmptyString, false, false, TableDef);
  TUtf8File::WriteLines( FN, Output, false );
  return true;
}
//..............................................................................
void TMainForm::BadReflectionsTable(bool TableDef)  {
  if( FXApp->CheckFileType<TIns>() )
    Lst.SynchroniseOmits( (TIns*)FXApp->XFile().GetLastLoader() );

  TTTable<TStrList> Table;
  TStrList Output;
  Table.Resize(Lst.DRefCount(), 5);
  Table.ColName(0) = "H";
  Table.ColName(1) = "K";
  Table.ColName(2) = "L";
  Table.ColName(3) = "&Delta;(F<sup>2</sup>)/esd";
  for( int i=0; i < Lst.DRefCount(); i++ )  {
    TLstRef& Ref = Lst.DRef(i);
    Table[i][0] = Ref.H;
    Table[i][1] = Ref.K;
    Table[i][2] = Ref.L;
    if( Ref.DF >= 10 ) 
      Table[i][3] << "<font color=\'red\'>" << Ref.DF << "</font>";
    else
      Table[i][3] = Ref.DF;
    if( Ref.Deleted )
      Table[i][4] << "Omitted";
    else
      Table[i][4] << "<a href='omit " << Ref.H <<  ' ' << Ref.K << ' ' << Ref.L << "\'>" << "omit" << "</a>";
  }
  Table.CreateHTMLList(Output, EmptyString, true, false, TableDef);
  TUtf8File::WriteLines( BadRefsFile, Output, false );
}
//..............................................................................
void TMainForm::RefineDataTable(bool TableDef)  {
  TTTable<TStrList> Table;
  TStrList Output;

  Table.Resize(13, 4);

  Table[0][0] = "R1(Fo > 4sig(Fo))";
  if( Lst.R1() > 0.1 )
    Table[0][1] << "<font color=\'red\'>" << Lst.R1() << "</font>";
  else
    Table[0][1] = Lst.R1();

  Table[0][2] = "R1(all data)";
  if( Lst.R1a() > 0.1 )
    Table[0][3] << "<font color=\'red\'>" << Lst.R1a() << "</font>";
  else
   Table[0][3] = Lst.R1a();

  Table[1][0] = "wR2";
  if( Lst.wR2() > 0.2 )
     Table[1][1] << "<font color=\'red\'>" << Lst.wR2() << "</font>"; 
  else
    Table[1][1] = Lst.wR2();

  Table[1][2] = "GooF";
  if( fabs(Lst.S()-1) > 0.5 )
    Table[1][3] << "<font color=\'red\'>" << Lst.S() << "</font>";
  else
    Table[1][3] = Lst.S();  

  Table[2][0] = "GooF(Restr)";
  if( fabs(Lst.RS()-1) > 0.5 )
    Table[2][1] << "<font color=\'red\'>" << Lst.RS() << "</font>";
  else
    Table[2][1] = Lst.RS();

  Table[2][2] = "Highest peak";
  if( Lst.Peak() > 1.5 )
    Table[2][3] << "<font color=\'red\'>" << Lst.Peak() << "</font>";
  else
    Table[2][3] = Lst.Peak(); 

  Table[3][0] = "Deepest hole";
  if( fabs(Lst.Hole()) > 1.5 )
    Table[3][1] << "<font color=\'red\'>" << Lst.Hole() << "</font>";
  else
    Table[3][1] = Lst.Hole();

  Table[3][2] = "Params";             Table[3][3] = Lst.Params();
  Table[4][0] = "Refs(total)";        Table[4][1] = Lst.TotalRefs();
  Table[4][2] = "Refs(uni)";          Table[4][3] = Lst.UniqRefs();
  Table[5][0] = "Refs(Fo > 4sig(Fo))";Table[5][1] = Lst.Refs4sig();
  Table[5][2] = "R(int)";             Table[5][3] = Lst.Rint();
  Table[6][0] = "R(sigma)";           Table[6][1] = Lst.Rsigma();

  Table.CreateHTMLList(Output, EmptyString, false, false, TableDef);
  TUtf8File::WriteLines( RefineDataFile, Output, false );
}
//..............................................................................
void TMainForm::OnMouseMove(int x, int y)  {
  if( MousePositionX == x && MousePositionY == y )
    return;  
  else  {
    MouseMoveTimeElapsed = 0;
    MousePositionX = x;
    MousePositionY = y;
#if defined (__WIN32__)
    FGlCanvas->SetToolTip(wxT(""));
#else
    if( GlTooltip != NULL )  GlTooltip->Visible(false);
#endif
  }
}
//..............................................................................
bool TMainForm::OnMouseDown(int x, int y, short Flags, short Buttons) {
  MousePositionX = x;
  MousePositionY = y;
  return false;
}
bool TMainForm::OnMouseUp(int x, int y, short Flags, short Buttons)  {
  if( Modes->GetCurrent() != NULL )  {
    if( (abs(x-MousePositionX) < 3) && (abs(y-MousePositionY) < 3) )  {
      AGDrawObject *G = FXApp->SelectObject(x, y);
      if( G != NULL && Modes->GetCurrent()->OnObject(*G) )
        return true;
    }
  }
  // HKL "grid snap on mouse release
  if( FXApp->XFile().GetLastLoader() && FXApp->HklVisible() && false )
  {
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
      TBasicApp::GetLog() << Tmp;
    Z.Null();
    mat3d::GaussSolve(M, N, Z);
    Z.Normalise();
    double H = Z[0]*Z[0];
    double K = Z[1]*Z[1];
    double L = Z[2]*Z[2];
    if( H > 0.07 )  H = 1./H;
    if( K > 0.07 )  K = 1./K;
    if( L > 0.07 )  L = 1./L;
    int iH = Round(H), iK = Round(K), iL = Round(L);
    double diff = sqrt(fabs(H + K + L - iH - iK - iL)/(H + K + L));
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
      TBasicApp::GetLog() << Tmp;
      N = FXApp->GetRender().GetBasis().GetMatrix()[2];
        double ca = N.CAngle(Z);
        if( ca < -1 )  ca = -1;
        if( ca > 1 )   ca = 1;
        vec3d V = Z.XProdVec(N);
        FXApp->GetRender().Basis()->Rotate(V, acos(ca));
      N = FXApp->GetRender().GetBasis().GetMatrix()[2];
      Tmp="got: ";
      Tmp << N.ToString(); 
      TBasicApp::GetLog() << Tmp;

      FXApp->Draw();
    }
  }
  MousePositionX = x;
  MousePositionY = y;
  return false;
}
//..............................................................................
void TMainForm::ClearPopups()  {
  for( int i=0; i < FPopups.Count(); i++ )  {
    delete FPopups.Object(i)->Dialog;
    delete FPopups.Object(i);
  }
  FPopups.Clear();
}
//..............................................................................
TPopupData* TMainForm::GetPopup(const olxstr& name)  {
  return FPopups[name];
}
//..............................................................................
bool TMainForm::CheckMode(const unsigned short mode, const olxstr& modeData)  {
  if( Modes->GetCurrent() == NULL )  return false;
  return mode == Modes->GetCurrent()->GetId();
}
//..............................................................................
bool TMainForm::CheckState(const unsigned short state, const olxstr& stateData)
{
  if( !stateData.Length() )  return (ProgramState & state) != 0;

  if( state == prsHtmlVis )  {
    if( !stateData.Length() )  return FHtmlMinimized;

    TPopupData* pp = GetPopup( stateData );
    return (pp!=NULL) ? pp->Dialog->IsShown() : false;
  }
  if( state == prsHtmlTTVis )  {
    if( stateData.Length() == 0 )  return FHtml->GetShowTooltips();
    TPopupData* pp = GetPopup( stateData );
    return (pp!=NULL) ? pp->Html->GetShowTooltips() : false;
  }
  if( state == prsPluginInstalled )  {
    if( stateData.Length() == 0 )  return false;
    return FPluginItem->ItemExists( stateData );
  }
  if( state == prsHelpVis )  {
    return HelpWindowVisible;
  }
  if( state == prsInfoVis )  {
    return InfoWindowVisible;
  }
  if( state == prsCmdlVis )  {
    return CmdLineVisible;
  }
  if( state == prsGradBG )  {
    return FXApp->GetRender().Background()->Visible();
  }

  return false;
}
//..............................................................................
void TMainForm::OnInternalIdle()  {
  if( Destroying )  return;
  FParent->Yield();
#if !defined(__WIN32__)  
  if( !StartupInitialised )  StartupInit();
#endif
  TBasicApp::GetInstance()->OnIdle->Execute((AEventsDispatcher*)this, NULL);
  // runonce business...
  if( !RunOnceProcessed )  {
    RunOnceProcessed = true;
    TStrList rof;
    olxstr curd = TEFile::CurrentDir();
    TEFile::ChangeDir( FXApp->BaseDir() );
    TEFile::ListCurrentDir( rof, "runonce*.*", sefFile);
    TStrList macros;
    for( int i=0; i < rof.Count(); i++ )  {
      try  {
        macros.LoadFromFile( rof[i] );
        macros.CombineLines('\\');
        for( int i=0; i < macros.Count(); i++ )
          executeMacro( macros.String(i) );
      }
      catch( const TExceptionBase& exc )  {
        TBasicApp::GetLog().Exception( exc.GetException()->GetFullMessage() );
        ::wxMessageBox( uiStr(exc.GetException()->GetError()) += wxT('\n'),
          uiStrT("Exception: ") += uiStr(EsdlObjectName(exc)), wxOK|wxICON_ERROR);
      }
      long fa = TEFile::FileAge( rof.String(i) );
      // Null the file
      try  {  TEFile ef(rof.String(i), "wb+");  }
      catch( TIOExceptionBase& )  { ;  }
      TEFile::SetFileTimes(rof.String(i), fa, fa);
      //TEFile::DelFile(rof.String(i));
    }
    TEFile::ChangeDir( curd );
  }
  wxFrame::OnInternalIdle();
#ifdef __MAC__  // duno why otherwise it takes 100% of CPU time...
  wxMilliSleep(15);
#endif  
  return;
}
//..............................................................................
void TMainForm::SetUserCursor( const olxstr& param, const olxstr& mode )  {
  wxBitmap bmp(32, 32);
  wxMemoryDC memDC;
  wxBrush Brush = memDC.GetBrush();
  Brush.SetColour( *wxWHITE );
  memDC.SetBrush( Brush );
  wxPen Pen = memDC.GetPen();
  Pen.SetColour( *wxRED );
  memDC.SetPen( Pen );
  wxFont Font = memDC.GetFont();
  Font.SetFamily( wxSWISS );
#if defined(__WIN32__)
  Font.SetPointSize( 10 );
#else
  Font.SetPointSize( 10 );
#endif

  memDC.SetFont( Font );
  memDC.SelectObject( bmp );
  memDC.Clear();
  Brush.SetColour( *wxGREEN );
  memDC.SetBrush( Brush );
  memDC.DrawCircle( 2, 2, 2 );
  memDC.SetTextForeground( *wxRED );
  memDC.DrawText( uiStr(param), 0, 4 );
  memDC.DrawLine( 0, 18, 32, 18 );
  memDC.SetTextForeground( *wxGREEN );
  memDC.SetPen( Pen );
  memDC.DrawText( uiStr(mode), 0, 18 );
  wxImage img( bmp.ConvertToImage() );
  img.SetMaskColour( 255, 255, 255 );
  img.SetMask( true );
  wxCursor cr(img);
  SetCursor( cr );
  FGlCanvas->SetCursor(cr);
}
//..............................................................................
bool TMainForm::executeMacro(const olxstr& cmdLine)  {
  this->ProcessXPMacro( cmdLine, MacroError );
  return MacroError.IsSuccessful();
}
//..............................................................................
void TMainForm::print(const olxstr& output, const short MessageType)  {
//  TGlMaterial *glm = NULL;
  if( MessageType == olex::mtInfo )          TBasicApp::GetLog().Info(output);
  else if( MessageType == olex::mtWarning )   TBasicApp::GetLog().Warning(output);
  else if( MessageType == olex::mtError )     TBasicApp::GetLog().Error(output);
  else if( MessageType == olex::mtException ) TBasicApp::GetLog().Exception(output);
  // if need to foce printing - so go aroung the log
//  if( MessageType == 0 )  ;
//  else if( MessageType == olex::mtInfo )      glm = &InfoFontColor;
//  else if( MessageType == olex::mtWarning )   glm = &WarningFontColor;
//  else if( MessageType == olex::mtError )     glm = &ErrorFontColor;
//  else if( MessageType == olex::mtException ) glm = &ExceptionFontColor;
//  FGlConsole->PostText(output, glm);
}
//..............................................................................
bool TMainForm::executeFunction(const olxstr& function, olxstr& retVal)  {
  retVal = function;
  return ProcessMacroFunc( retVal );
}
//..............................................................................
IEObject* TMainForm::executeFunction(const olxstr& function)  {
  int ind = function.FirstIndexOf('(');
  if( (ind == -1) || (ind == (function.Length()-1)) || !function.EndsWith(')') )  {
    TBasicApp::GetLog().Error( olxstr("Incorrect function call: ") << function);
    return NULL;
  }
  olxstr funName = function.SubStringTo(ind);
  ABasicFunction* Fun = FXApp->GetLibrary().FindFunction( funName );
  if( Fun == NULL )  {
    TBasicApp::GetLog().Error( olxstr("Unknow function: ") << funName);
    return NULL;
  }
  TMacroError me;
  TStrObjList funParams;
  olxstr funArg = function.SubStringFrom(ind+1, 1);
  TParamList::StrtokParams(funArg, ',', funParams);
  try  {
    Fun->Run(funParams, me);
    if( !me.IsSuccessful() )  {
      AnalyseError( me );
      return NULL;
    }
  }
  catch( TExceptionBase& exc )  {
    me.ProcessingException(*Fun, exc);
    AnalyseError( me );
    return NULL;
  }
  return (me.HasRetVal()) ? me.RetObj()->Replicate() : NULL;
}
//..............................................................................
THtml* TMainForm::GetHtml(const olxstr& popupName)  {
  TPopupData* pd = FPopups[popupName];
  return pd ? pd->Html : NULL;
}
//..............................................................................
void TMainForm::AnalyseError( TMacroError& error )  {
  if( !error.IsSuccessful() )  {
    if( error.IsProcessingException() )  {
      TBasicApp::GetLog().Exception(olxstr(error.GetLocation()) << ": " <<  error.GetInfo());
    }
    else if( error.IsProcessingError() && !error.GetInfo().IsEmpty() )  {
      TBasicApp::GetLog().Error(olxstr(error.GetLocation()) << ": " <<  error.GetInfo());
    }
    else if( error.IsInvalidOption() )
      TBasicApp::GetLog().Error(error.GetInfo());
    else if( error.IsInvalidArguments() )
      TBasicApp::GetLog().Error(error.GetInfo());
    else if( error.IsIllegalState() )
      TBasicApp::GetLog().Error(error.GetInfo());
    else if( !error.DoesFunctionExist() && !(FMode&mSilent) )
      TBasicApp::GetLog().Warning(error.GetInfo());
  }
}
//..............................................................................
bool TMainForm::ProcessEvent( wxEvent& evt )  {
//  if( evt.GetId() ==
  if( evt.GetEventType() == wxEVT_COMMAND_MENU_SELECTED && AccMenus.ValueExists( evt.GetId() )  )  {
    olxstr macro( AccMenus.GetValue(evt.GetId())->GetCommand() );
    if( !macro.IsEmpty() )  {
      TStrList sl;
      bool checked = AccMenus.GetValue(evt.GetId())->IsChecked();
      sl.Strtok( macro, ">>");
      for( int i=0; i < sl.Count(); i++ )  {
        ProcessXPMacro( sl.String(i), MacroError );
        if( !MacroError.IsSuccessful() )  {
          AnalyseError(MacroError);
          break;
        }
      }
      // restore state if failed
      if( AccMenus.GetValue(evt.GetId())->IsCheckable() )
        AccMenus.GetValue(evt.GetId())->ValidateState();

      FXApp->Draw();
      return true;
    }
  }
  return wxFrame::ProcessEvent(evt);
}
//..............................................................................
int TMainForm::TranslateShortcut(const olxstr& sk)  {
  if( !sk.Length() )  return -1;
  TStrList toks(sk, '+');
  if( !toks.Count() )  return -1;
  short Shift=0, Char = 0;
  for( int i=0; i < toks.Count() - 1; i++ )  {
    if( ((Shift&sssCtrl)==0) && toks[i].Comparei("Ctrl")==0 )   {  Shift |= sssCtrl;  continue;  }
    if( ((Shift&sssShift)==0) && toks[i].Comparei("Shift")==0 )  {  Shift |= sssShift;  continue;  }
    if( ((Shift&sssAlt)==0) && toks[i].Comparei("Alt") )    {  Shift |= sssAlt;  continue;  }
  }
  olxstr charStr = toks.String( toks.Count() -1 );
  // a char
  if( charStr.Length() == 1 ) {
    Char = charStr[0];
    if( Char  >= 'a' && Char <= 'z' ) Char -= ('a'-'A');
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

  return Char!=0 ? ((Shift << 16)|Char) : -1;
}
//..............................................................................
void TMainForm::SetProgramState( bool val, unsigned short state )  {
  SetBit(val, ProgramState, state);
}
//..............................................................................
bool TMainForm::OnMouseDblClick(int x, int y, short Flags, short Buttons)  {
  AGDrawObject *G = FXApp->SelectObject(x, y);
  if( G == NULL )  return true;
  if( EsdlInstanceOf(*G, TGlBitmap) )  {
    TGlBitmap* glB = (TGlBitmap*)G;
    if( !(glB->GetLeft() > 0) )  {
      int Top = InfoWindowVisible ? FInfoBox->GetTop() + FInfoBox->GetHeight() : 1;
      for( int i=0; i < FXApp->GlBitmapCount(); i++ )  {
        TGlBitmap* b = &FXApp->GlBitmap(i);
        if( b == glB )  break;
        Top += (b->GetHeight() + 2);
      }
      glB->Basis.Reset();
      double r = ((double)FXApp->GetRender().GetWidth()/(double)glB->GetWidth()) / 10.0;
      glB->Basis.SetZoom(r);
      glB->SetTop( Top );
      glB->SetLeft( FXApp->GetRender().GetWidth() - glB->GetWidth() );
    }
    else  {
      glB->SetLeft(0);
      glB->SetTop( InfoWindowVisible ? FInfoBox->GetTop() + FInfoBox->GetHeight() : 1 );
      glB->Basis.Reset();

      glB->Basis.SetZoom(1.0);
    }
  }
  else if( EsdlInstanceOf(*G, TXGlLabel) )  {
    olxstr label = "getuserinput(1, \'Atom label\', \'";
    label << ((TXGlLabel*)G)->GetLabel() << "\')";
    if( ProcessMacroFunc(label) && !label.IsEmpty() )
      ((TXGlLabel*)G)->SetLabel(label);

  }
  FXApp->Draw();
  return true;
}
//..............................................................................
bool TMainForm::Show( bool v )  {
#ifdef __WXGTK__
  bool res = wxWindow::Show(v);
  //OnResize();
#else
  bool res = wxFrame::Show(v);
#endif
  if( res )  {
    FXApp->SetMainFormVisible( v );
    FGlCanvas->SetFocus();
  }
  return res;
}
//..............................................................................
const olxstr& TMainForm::TranslatePhrase(const olxstr& phrase)  {
  return Dictionary.Translate(phrase);
}
//..............................................................................
void TMainForm::TranslateString(olxstr& phrase)  {

//  bool translate = Dictionary.NeedsTranslating( CurrentLanguage );

  olxstr tp;
  int ind = phrase.FirstIndexOf('%'), ind1;
  while( ind >= 0 )  {
    if( ind+1 >= phrase.Length() )  return;
    ind1 = phrase.FirstIndexOf('%', ind+1);
    if( ind1 == -1 )  return;
    if( ind1 == ind+1 )  { // %%
      phrase.Delete(ind1, 1);
      ind = phrase.FirstIndexOf('%', ind1);
      continue;
    }

    tp = Dictionary.Translate( phrase.SubString(ind+1, ind1-ind-1) );
    phrase.Delete( ind, ind1-ind+1);
    phrase.Insert( tp, ind );
    ind1 = ind + tp.Length();
    if( ind1+1 >= phrase.Length() )  return;
    ind = phrase.FirstIndexOf('%', ind1+1);
  }
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
  int ind = CallbackFuncs.IndexOfComparable(cbEvent),
      i = ind;
  if( ind == -1 )  return;
  // go forward
  while( i < CallbackFuncs.Count() && (!CallbackFuncs.GetComparable(i).Compare(cbEvent)) )  {
    if( CallbackFuncs.GetObject(i)->GetName() == funcName )  {
      delete CallbackFuncs.GetObject(i);
      CallbackFuncs.Remove(i);
      return;
    }
  }
  // go backwards
  i = ind-1;
  while( i >= 0 && (!CallbackFuncs.GetComparable(i).Compare(cbEvent)) )  {
    if( CallbackFuncs.GetObject(i)->GetName() == funcName )  {
      delete CallbackFuncs.GetObject(i);
      CallbackFuncs.Remove(i);
      return;
    }
  }
}
//..............................................................................
const olxstr& TMainForm::getDataDir() const  {  return DataDir;  }
//..............................................................................
const olxstr& TMainForm::getVar(const olxstr &name, const olxstr &defval) const {
  int i = TOlxVars::VarIndex(name);
  if( i == -1 )  {
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
void TMainForm::CallbackFunc(const olxstr& cbEvent, TStrObjList& params)  {
  static TIntList indexes;
  static TMacroError me;
  indexes.Clear();

  CallbackFuncs.GetIndexes(cbEvent, indexes);
  for(int i=0; i < indexes.Count(); i++ )  {
    me.Reset();
    CallbackFuncs.GetObject( indexes[i] )->Run(params, me);
    AnalyseError( me );
  }
}
//..............................................................................
void TMainForm::CallbackFunc(const olxstr& cbEvent, const olxstr& param)  {
  static TIntList indexes;
  static TMacroError me;
  static TStrObjList sl;
  indexes.Clear();
  sl.Clear();
  sl.Add( param );

  CallbackFuncs.GetIndexes(cbEvent, indexes);
  for(int i=0; i < indexes.Count(); i++ )  {
    me.Reset();
    CallbackFuncs.GetObject( indexes[i] )->Run(sl, me);
    AnalyseError( me );
  }
}
//..............................................................................
//void TMainForm::CallbackMacro(const olxstr& cbEvent, TStrList& params, const TParamList &Options)  {
//}
//..............................................................................
//void TMainForm::CallbackMacro(const olxstr& cbEvent, olxstr& param, const TParamList &Options)  {
//}
//..............................................................................
void TMainForm::SaveVFS(short persistenceId)  {
  try  {
    olxstr dbFN;
    if( persistenceId == plStructure )  {
      if( FXApp->XFile().GetLastLoader() == NULL || FXApp->XFile().GetFileName().IsEmpty() )  return;
      dbFN = GetStructureOlexFolder();
      dbFN << TEFile::ChangeFileExt( TEFile::ExtractFileName(FXApp->XFile().GetFileName()) , "odb");
    }
    else if(persistenceId == plGlobal )
      dbFN << DataDir << "global.odb";
    else
      throw TFunctionFailedException(__OlxSourceInfo, "undefined persistence level");

    TEFile dbf(dbFN, "wb");
    TFileHandlerManager::SaveToStream(dbf, persistenceId);
  }
  catch( const TExceptionBase& exc )  {
    TBasicApp::GetLog().Exception(exc.GetException()->GetFullMessage());
    ::wxMessageBox( (uiStr(exc.GetException()->GetFullMessage()) += wxT('\n')),
      wxT("Failed to save VFS"), wxOK|wxICON_ERROR);
  }
}
//..............................................................................
void TMainForm::LoadVFS(short persistenceId)  {
  try  {
    olxstr dbFN;
    if( persistenceId == plStructure )  {
      if( FXApp->XFile().GetLastLoader() == NULL || FXApp->XFile().GetFileName().IsEmpty() )  return;
      dbFN = GetStructureOlexFolder();
      dbFN << TEFile::ChangeFileExt( TEFile::ExtractFileName(FXApp->XFile().GetFileName()) , "odb");
    }
    else if(persistenceId == plGlobal )
      dbFN << DataDir << "global.odb";
    else
      throw TFunctionFailedException(__OlxSourceInfo, "undefined persistence level");

    if( !TEFile::FileExists(dbFN ) )  return;

    try  {
      TEFile dbf(dbFN, "rb");
      TFileHandlerManager::LoadFromStream(dbf, persistenceId);
    }
    catch( const TExceptionBase& exc )  {
       try  {  TEFile::DelFile(dbFN);  }
      catch( ... )  {  throw TFunctionFailedException(__OlxSourceInfo, "faild to read VFS");  }
    }
  }
  catch( const TExceptionBase& exc )  {
    TBasicApp::GetLog().Exception(exc.GetException()->GetFullMessage());
    ::wxMessageBox( uiStr(exc.GetException()->GetFullMessage()) += wxT('\n'),
      wxT("Failed to save VFS"), wxOK|wxICON_ERROR);
  }
}
//..............................................................................
const olxstr& TMainForm::GetStructureOlexFolder()  {
  if( FXApp->XFile().GetLastLoader() != NULL )  {
    olxstr ofn = TEFile::ExtractFilePath(FXApp->XFile().GetFileName());
    TEFile::AddTrailingBackslashI(ofn) << ".olex/";
    if( !TEFile::FileExists(ofn) )  {
      if( !TEFile::MakeDir(ofn) )  {
        throw TFunctionFailedException(__OlxSourceInfo, "cannot create folder");
      }
#ifdef __WIN32__
      SetFileAttributes(uiStr(ofn), FILE_ATTRIBUTE_HIDDEN);
#endif
    }
    return TEGC::New<olxstr>(ofn);
  }
  return EmptyString;
}
//..............................................................................
void TMainForm::LockWindowDestruction(wxWindow* wnd)  {
  if( wnd == FHtml )
    FHtml->IncLockPageLoad();
}
//..............................................................................
void TMainForm::UnlockWindowDestruction(wxWindow* wnd)  {
  if( wnd == FHtml )  {
    FHtml->DecLockPageLoad();
  }
}
//..............................................................................
bool TMainForm::FindXAtoms(const TStrObjList &Cmds, TXAtomPList& xatoms, bool GetAll, bool unselect)  {
  int cnt = xatoms.Count();
  if( Cmds.IsEmpty() )  {
    FXApp->FindXAtoms("sel", xatoms, unselect);
    if( GetAll && xatoms.IsEmpty() )
      FXApp->FindXAtoms(EmptyString, xatoms, unselect);
  }
  else  {
    FXApp->FindXAtoms(Cmds.Text(' '), xatoms, unselect);
  }
  for( int i=0; i < xatoms.Count(); i++ )
    if( !xatoms[i]->Visible() )
      xatoms[i] = NULL;
  xatoms.Pack();
  return (xatoms.Count() != cnt);
}
//..............................................................................
const olxstr& TMainForm::GetSGList() const {
  int ind = TOlxVars::VarIndex(SGListVarName);
  return (ind != -1) ? TOlxVars::GetVarStr(ind) : EmptyString;
}
//..............................................................................
void TMainForm::SetSGList(const olxstr &sglist)  {
  TOlxVars::SetVar(SGListVarName, EmptyString);
}
//..............................................................................
//..............................................................................
//..............................................................................
//..............................................................................
//..............................................................................

PyObject* pyVarValue(PyObject* self, PyObject* args)  {
  olxstr varName;
  PyObject* defVal = NULL;
  if( !PythonExt::ParseTuple(args, "w|O", &varName, &defVal) )  {
    Py_INCREF(Py_None);
    return Py_None;
  }
  int i = TOlxVars::VarIndex(varName);
  if( i == -1 )  {
    if( defVal != NULL )  {
      TOlxVars::SetVar(varName, defVal);
      Py_IncRef( defVal );
      return TOlxPyVar::ObjectValue(defVal);
    }
    else  {
      PyErr_SetObject(PyExc_KeyError, PythonExt::BuildString("undefined key name"));
      Py_INCREF(Py_None);
      return Py_None;
    }
  }
  return TOlxVars::GetVarValue(i);
}
//..............................................................................
PyObject* pyVarObject(PyObject* self, PyObject* args)  {
  olxstr varName;
  PyObject* defVal = NULL;
  if( !PythonExt::ParseTuple(args, "w|O", &varName, &defVal) )  {
    Py_INCREF(Py_None);
    return Py_None;
  }
  int i = TOlxVars::VarIndex(varName);
  if( i == -1 )  {
    if( defVal != NULL )  {
      TOlxVars::SetVar(varName, defVal);
      Py_IncRef( defVal );
      return defVal;
    }
    else  {
      PyErr_SetObject(PyExc_KeyError, PythonExt::BuildString("undefined key name"));
      Py_INCREF(Py_None);
      return Py_None;
    }
  }
  PyObject *rv = TOlxVars::GetVarWrapper(i);
  if( rv == NULL )  rv = Py_None;
  Py_IncRef(rv);
  return rv;
}
//..............................................................................
PyObject* pyIsVar(PyObject* self, PyObject* args)  {
  olxstr varName;
  if( !PythonExt::ParseTuple(args, "w", &varName) )  {
    Py_INCREF(Py_None);
    return Py_None;
  }
  return Py_BuildValue("b", TOlxVars::IsVar(varName) );
}
//..............................................................................
PyObject* pyVarCount(PyObject* self, PyObject* args)  {
  return Py_BuildValue("i", TOlxVars::VarCount() );
}
//..............................................................................
PyObject* pyGetVar(PyObject* self, PyObject* args)  {
  int varIndex;
  if( !PyArg_ParseTuple(args, "i", &varIndex) )  {
    Py_INCREF(Py_None);
    return Py_None;
  }
  return TOlxVars::GetVarValue(varIndex);
}
//..............................................................................
PyObject* pyGetVarName(PyObject* self, PyObject* args)  {
  int varIndex;
  if( !PyArg_ParseTuple(args, "i", &varIndex) )  {
    Py_INCREF(Py_None);
    return Py_None;
  }
  return PythonExt::BuildString( TOlxVars::GetVarName(varIndex) );
}
//..............................................................................
PyObject* pyFindGetVarName(PyObject* self, PyObject* args)  {
  PyObject *val;
  if( !PyArg_ParseTuple(args, "O", &val) )  {
    Py_INCREF(Py_None);
    return Py_None;
  }
  return PythonExt::BuildString( TOlxVars::FindVarName(val) );
}
//..............................................................................
PyObject* pySetVar(PyObject* self, PyObject* args)  {
  olxstr varName;
  PyObject *varValue = NULL;
  if( !PythonExt::ParseTuple(args, "wO", &varName, &varValue) )  {
    Py_INCREF(Py_None);
    return Py_None;
  }
  TOlxVars::SetVar(varName, varValue);
  Py_INCREF(Py_None);
  return Py_None;
}
//..............................................................................
PyObject* pyExpFun(PyObject* self, PyObject* args)  {
  TBasicFunctionPList functions;
  TGlXApp::GetMainForm()->GetLibrary().ListAllFunctions( functions );
  PyObject* af = PyTuple_New( functions.Count() ), *f;
  for( int i=0; i < functions.Count(); i++ )  {
    ABasicFunction* func = functions[i];
    f = PyTuple_New(3);
    PyTuple_SetItem(af, i, f );

    PyTuple_SetItem(f, 0, PythonExt::BuildString(func->GetQualifiedName()) );
    PyTuple_SetItem(f, 1, PythonExt::BuildString(func->GetSignature()) );
    PyTuple_SetItem(f, 2, PythonExt::BuildString(func->GetDescription()) );
  }
  return af;
}
//..............................................................................
PyObject* pyExpMac(PyObject* self, PyObject* args)  {
  TBasicFunctionPList functions;
  TGlXApp::GetMainForm()->GetLibrary().ListAllMacros( functions );
  PyObject* af = PyTuple_New( functions.Count() ), *f, *s;
  for( int i=0; i < functions.Count(); i++ )  {
    ABasicFunction* func = functions[i];
    f = PyTuple_New(4);
    PyTuple_SetItem(af, i, f );
    PyTuple_SetItem(f, 0, PythonExt::BuildString(func->GetQualifiedName()) );
    PyTuple_SetItem(f, 1, PythonExt::BuildString(func->GetSignature()) );
    PyTuple_SetItem(f, 2, PythonExt::BuildString(func->GetDescription()) );
    s = PyDict_New();
    PyTuple_SetItem(f, 3, s );
    for(int j=0; j < func->GetOptions().Count(); j++ )  {
      PyDict_SetItem(s, PythonExt::BuildString(func->GetOptions().GetComparable(j)),
                        PythonExt::BuildString(func->GetOptions().GetObject(j)) );
    }
  }
  return af;
}
//..............................................................................
PyObject* pyTranslate(PyObject* self, PyObject* args)  {
  olxstr str;
  if( !PythonExt::ParseTuple(args, "w", &str) )  {
    Py_INCREF(Py_None);
    return Py_None;
  }
  TGlXApp::GetMainForm()->TranslateString(str);
  return PythonExt::BuildString(str);
}
//..............................................................................
PyObject* pyGetUserInput(PyObject* self, PyObject* args)  {
  olxstr title, str;
  int flags = 0;
  if( !PythonExt::ParseTuple(args, "iww", &flags, &title, &str) ||
      title.IsEmpty() || str.IsEmpty() )  {
    Py_INCREF(Py_None);
    return Py_None;
  }

  bool MultiLine = (flags != 1);

  TdlgEdit *dlg = new TdlgEdit(TGlXApp::GetMainForm(), MultiLine);
  dlg->SetTitle( uiStr(title) );
  dlg->SetText( str );

  PyObject* rv;
  if( dlg->ShowModal() == wxID_OK )
    rv = PythonExt::BuildString( dlg->GetText() );
  else  {
    rv = Py_None;
    Py_IncRef(rv);
  }
  dlg->Destroy();
  return rv;
}
//..............................................................................
PyObject* pyIsControl(PyObject* self, PyObject* args)  {
  olxstr cname, pname;  // control and popup (if any) name
  int flags = 0;
  if( !PythonExt::ParseTuple(args, "w|w", &cname, &pname) )  {
    Py_INCREF(Py_None);
    return Py_None;
  }
  THtml* html = (pname.IsEmpty() ) ? TGlXApp::GetMainForm()->GetHtml() : TGlXApp::GetMainForm()->GetHtml(pname);  
  if( html == NULL )
    return Py_BuildValue("b", false);
  return Py_BuildValue("b", html->FindObject(cname) != NULL);
}
//..............................................................................
static PyMethodDef CORE_Methods[] = {
  {"ExportFunctionList", pyExpFun, METH_VARARGS, "exports a list of olex functions and their description"},
  {"ExportMacroList", pyExpMac, METH_VARARGS, "exports a list of olex macros and their description"},
  {"IsVar", pyIsVar, METH_VARARGS, "returns boolean value if specified variable exists"},
  {"VarCount", pyVarCount, METH_VARARGS, "returns the number of variables"},
  {"VarValue", pyGetVar, METH_VARARGS, "returns specified variable value"},
  {"SetVar", pySetVar, METH_VARARGS, "sets value of specified variable"},
  {"FindValue", pyVarValue, METH_VARARGS, "returns value of specified variable or empty string"},
  {"FindObject", pyVarObject, METH_VARARGS, "returns value of specified variable as an object"},
  {"VarName", pyGetVarName, METH_VARARGS, "returns name of specified variable"},
  {"FindVarName", pyFindGetVarName, METH_VARARGS, "returns name of variable name corresponding to provided object"},
  {"Translate", pyTranslate, METH_VARARGS, "returns translated version of provided string"},
  {"IsControl", pyIsControl, METH_VARARGS, "Takes HTML element name and optionaly popup name. Returns true/false if given control exists"},
  {"GetUserInput", pyGetUserInput, METH_VARARGS, "shows a dialog, where user can type some text.\
   Takes three agruments: flags, title and content. If flags not equal to 1, a muliline dialog sis created"},
  {NULL, NULL, 0, NULL}
   };

void TMainForm::PyInit()  {
  Py_InitModule( "olex_core", CORE_Methods );
}


