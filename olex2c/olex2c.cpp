// olex2c.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <windows.h>
#include <conio.h>
#include <process.h>

using namespace std;

#include "xapp.h"
#include "log.h"
#include "outstream.h"
#include "pyext.h"
#include "fsext.h"
#include "olxvar.h"
#include "shellutil.h"
#include "dataitem.h"
#include "datafile.h"
#include "eprocess.h"
#include "egc.h"
//
#include "ins.h"
#include "mol.h"
#include "cif.h"
#include "xyz.h"
#include "p4p.h"
#include "crs.h"
#include "pdb.h"
#include "xdmas.h"
#include "lst.h"
#include "macrolib.h"
#include "symmlib.h"
#include "xlcongen.h"
#include "seval.h"
#include "ecast.h"
#include "utf8file.h"
//

#define olx_Shift 25
#define olx_Ctrl 35
#define olx_Alt 45

enum {
  olx_TAB = 9,
  olx_F1 = 59,
  olx_F2,
  olx_F3,
  olx_F4,
  olx_F5,
  olx_F6,
  olx_F7,
  olx_F8,
  olx_F9,
  olx_F10,
  olx_F11=133,
  olx_F12,

  olx_HOME = 71,
  olx_UP = 72,
  olx_PGUP = 73, 
  olx_DOWN = 80,
  olx_PGDN = 81,
  olx_INS = 82,
  olx_DEL = 83,
  olx_LEFT = 75,
  olx_RIGHT = 77,
  olx_END = 79
};

#define this_InitMacroD(macroName, validOptions, argc, desc)\
  Library.RegisterMacro( new TMacro<TOlex>(this, &TOlex::mac##macroName, #macroName, (validOptions), argc, desc))
#define this_InitMacroDA(macroName, macroNameA, validOptions, argc, desc)\
  Library.RegisterMacro( new TMacro<TOlex>(this, &TOlex::mac##macroName, #macroNameA, (validOptions), argc, desc))
#define this_InitFuncD(funcName, argc, desc)\
  Library.RegisterFunction( new TFunction<TOlex>(this, &TOlex::fun##funcName, #funcName, argc, desc))

static const olxstr ProcessOutputCBName("procout");
static const olxstr NAString("n/a");
enum  {
  ID_PROCESSTERMINATE = 1,
  ID_TIMER,
  ID_INFO,
  ID_ERROR,
  ID_WARNING,
  ID_EXCEPTION,
  ID_STRUCTURECHANGED
};

class TOlex: public AEventsDispatcher, public olex::IOlexProcessor  {
  TXApp XApp;
  TLst Lst;
  HANDLE TimerThreadHandle;
  olxstr DataDir;
  TCSTypeList<olxstr, ABasicFunction*> CallbackFuncs;
  TStrList FOnTerminateMacroCmds; // a list of commands called when a process is terminated
  TStrList FOnAbortCmds;           // a "stack" of macroses, called when macro terminated
  TStrList FOnListenCmds;  // a list of commands called when a file is changed by another process
  TDataItem* FMacroItem;
  AProcess* FProcess;
  TEMacroLib Macros;
  TSAtomPList Selection;
  HANDLE conin, conout;
  TDataFile PluginFile;
  TDataItem* Plugins;
  CONSOLE_SCREEN_BUFFER_INFO TextAttrib;
  static unsigned long _stdcall TimerThreadRun(void* _instance) {
    while( true )  {
      if( TBasicApp::GetInstance()  == NULL )  return 0;
      TBasicApp::GetInstance()->OnTimer->Execute(NULL);
      Sleep(50);
    }
    return 0;
  }
 
  void UnifyAtomList(TSAtomPList atoms)  {
    // unify the selection
    for( int i=0; i < atoms.Count(); i++ )
      atoms[i]->CAtom().SetTag(i);
    for( int i=0; i < atoms.Count(); i++ )
      if( atoms[i]->CAtom().GetTag() != i || atoms[i]->CAtom().IsDeleted() )
        atoms[i] = NULL;
    atoms.Pack();
  }
  bool LocateAtoms(TStrObjList &Cmds, TSAtomPList& atoms, bool all)  {
    if( Cmds.IsEmpty() )  {
      if( !Selection.IsEmpty() )
        atoms.AddList( Selection );
      else if( all ) {
        TLattice& latt = XApp.XFile().GetLattice();
        for( int i=0; i < latt.AtomCount(); i++ )
          if( !latt.GetAtom(i).IsDeleted() )
            atoms.Add( &latt.GetAtom(i) );
      }
    }
    else  {
      for( int i=0; i < Cmds.Count(); i++ )  {
        if( Cmds[i].Comparei("sel") == 0 )  {
          atoms.AddList( Selection );
          Cmds.Delete(i);
          break;
        }
      }
      olxstr cnd(Cmds.Text(' '));
      if( !cnd.IsEmpty() )
        XApp.FindSAtoms(Cmds.Text(' '), atoms);
    }
    UnifyAtomList(atoms);
    return !atoms.IsEmpty();
  }
public:
  TOlex( const olxstr& basedir) : XApp(basedir), Macros(*this) {
    OlexInstance = this;
    XApp.GetLog().AddStream( new TOutStream(), true );
    XApp.GetLog().OnInfo->Add(this, ID_INFO);
    XApp.GetLog().OnWarning->Add(this, ID_WARNING);
    XApp.GetLog().OnError->Add(this, ID_ERROR);
    XApp.GetLog().OnException->Add(this, ID_EXCEPTION);

    TLibrary &Library = XApp.GetLibrary();
    PythonExt::Init(this).Register(&TOlex::PyInit);
    Library.AttachLibrary( TEFile::ExportLibrary() );
    Library.AttachLibrary( PythonExt::GetInstance()->ExportLibrary() );
    Library.AttachLibrary( TETime::ExportLibrary() );
    Library.AttachLibrary( XApp.XFile().ExportLibrary() );
    Library.AttachLibrary( TFileHandlerManager::ExportLibrary() );

    unsigned long thread_id;
    TimerThreadHandle = CreateThread(NULL, 0, TimerThreadRun, this, 0, &thread_id);
    conin = CreateFile(L"CONIN$", GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL); 
    conout  = CreateFile(L"CONOUT$", GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    GetConsoleScreenBufferInfo(conout, &TextAttrib); 
    DataDir = TShellUtil::GetSpecialFolderLocation(fiAppData);
#ifdef __WIN32__
  #ifdef _UNICODE
    DataDir << "Olex2u/";
  #else
    DataDir << "Olex2/";
  #endif
#endif
    XApp.GetLog().AddStream( TUtf8File::Create(DataDir + "olex2c.log"), true );
    FMacroItem = NULL;
    FProcess = NULL;

    TCif *Cif = new TCif(XApp.AtomsInfo());  // the objects will be automatically removed by the XApp
    XApp.XFile().RegisterFileFormat(Cif, "cif");
    XApp.XFile().RegisterFileFormat(Cif, "fcf");
    XApp.XFile().RegisterFileFormat(Cif, "fco");
    TMol *Mol = new TMol(XApp.AtomsInfo());  // the objects will be automatically removed by the XApp
    XApp.XFile().RegisterFileFormat(Mol, "mol");
    TIns *Ins = new TIns(XApp.AtomsInfo());
    XApp.XFile().RegisterFileFormat(Ins, "ins");
    XApp.XFile().RegisterFileFormat(Ins, "res");
    TXyz *Xyz = new TXyz(XApp.AtomsInfo());
    XApp.XFile().RegisterFileFormat(Xyz, "xyz");
    XApp.XFile().RegisterFileFormat(new TP4PFile(), "p4p");
    XApp.XFile().RegisterFileFormat(new TCRSFile(), "crs");
    XApp.XFile().RegisterFileFormat(new TPdb(XApp.AtomsInfo()), "pdb");
    XApp.XFile().RegisterFileFormat(new TXDMas(XApp.AtomsInfo()), "mas");
    XApp.XFile().GetLattice().OnStructureUniq->Add(this, ID_STRUCTURECHANGED);
    XApp.XFile().GetLattice().OnStructureGrow->Add(this, ID_STRUCTURECHANGED);
    XApp.XFile().OnFileLoad->Add(this, ID_STRUCTURECHANGED);

    this_InitMacroD(IF, "", fpAny, "if...");
    this_InitMacroD(Exec, "s&;o&;d", fpAny^fpNone, "exec" );
    this_InitMacroD(Echo, "", fpAny, "echo" );
    this_InitMacroDA(Reap, @reap, "", fpOne, "reap" );
    this_InitMacroD(File, "", fpNone|fpOne|psFileLoaded, "file" );
    this_InitMacroD(Name, "", fpAny^(fpNone|fpOne)|psFileLoaded, "name" );
    this_InitMacroD(Info, "", fpAny, "info" );
    this_InitMacroDA(Python, @py, "", fpAny^fpNone, "Runs python script" );
    this_InitMacroD(Clear, "", fpNone, "" );
    this_InitMacroD(Stop, "", fpOne, "" );
    this_InitMacroD(Reload, "", fpOne, "" );
    this_InitMacroD(Reset, "s&;c&;f", fpAny|psFileLoaded, "" );
    this_InitMacroD(WaitFor, "", fpOne, "" );
    this_InitMacroD(LstFS, "", fpNone, "" );
    this_InitMacroD(LstVar, "", fpNone, "" );
    this_InitMacroD(AddIns, "", fpAny|psCheckFileTypeIns, "" );
    this_InitMacroD(DelIns, "", fpOne|psCheckFileTypeIns, "" );
    this_InitMacroD(FixHL, "", fpNone|psFileLoaded, "" );
    this_InitMacroD(Kill, "", (fpAny^fpNone)|psFileLoaded, "" );
    this_InitMacroD(HAdd, "", fpAny|psCheckFileTypeIns, "" );
    this_InitMacroD(LS, "", fpOne|fpTwo|psCheckFileTypeIns, "" );
    this_InitMacroD(Plan, "", fpOne|psCheckFileTypeIns, "" );
    this_InitMacroD(Sel, "i&;a&;u", fpAny|psFileLoaded, "" );
    this_InitMacroD(Anis, "", fpAny|psCheckFileTypeIns, "" );
    this_InitMacroD(Isot, "", fpAny|psCheckFileTypeIns, "" );
    this_InitMacroD(Fix, "", (fpAny^fpNone)|psCheckFileTypeIns, "" );
    this_InitMacroD(LstMac, "", fpNone, "" );
    this_InitMacroD(LstFun, "", fpNone, "" );
    
    this_InitFuncD(User, fpNone|fpOne, "reap" );
    this_InitFuncD(SetVar, fpTwo, "setvar" );
    this_InitFuncD(UnsetVar, fpOne, "" );
    this_InitFuncD(GetVar, fpOne|fpTwo, "" );
    this_InitFuncD(IsVar, fpOne, "" );
    this_InitFuncD(DataDir, fpNone, "" );
    this_InitFuncD(StrDir, fpNone, "" );
    this_InitFuncD(GetCompilationInfo, fpNone, "" );
    this_InitFuncD(IsPluginInstalled, fpOne, "" );
    this_InitFuncD(CurrentLanguageEncoding, fpNone, "" );
    this_InitFuncD(StrCmp, fpTwo, "" );
    this_InitFuncD(SG, fpNone|fpOne|psFileLoaded, "" );
    this_InitFuncD(Ins, fpOne|psCheckFileTypeIns, "" );
    this_InitFuncD(Lst, fpOne|psFileLoaded, "" );
    this_InitFuncD(LSM, fpNone|psCheckFileTypeIns, "" );
    this_InitFuncD(And, fpAny^(fpNone|fpOne), "" );
    this_InitFuncD(Or, fpAny^(fpNone|fpOne), "" );
    this_InitFuncD(Not, fpOne, "" );
    this_InitFuncD(HasGUI, fpNone, "" );
    this_InitFuncD(Sel, fpNone, "" );
    
    olxstr pluginsFile( XApp.BaseDir() + "plugins.xld" );
    Plugins = NULL;
    if( TEFile::FileExists( pluginsFile ) )  {
      PluginFile.LoadFromXLFile( pluginsFile, NULL );
      Plugins = PluginFile.Root().FindItem("Plugin");
    }
    else  
      Plugins = PluginFile.Root().AddItem("Plugin");
  
    olxstr macroFile( XApp.BaseDir() );
    macroFile << "macrox.xld";
    if( TEFile::FileExists(macroFile) )  {
      TDataFile df;
      df.LoadFromXLFile( macroFile );
      df.Include(NULL);
      TDataItem* di = df.Root().FindItem("xl_macro");
      if( di != NULL )
        Macros.Load( *di );
    }
    executeMacro("onstartup");
    TBasicApp::GetInstance()->OnTimer->Add(this, ID_TIMER);
  }
  ~TOlex()  {
    TBasicApp::GetInstance()->OnTimer->Clear();
    executeMacro("onexit");
    for( int i=0; i < CallbackFuncs.Count(); i++ )
      delete CallbackFuncs.GetObject(i);
    if( FProcess )  {
      FProcess->OnTerminate->Clear();
      FProcess->Terminate();
      delete FProcess;
    }
    CloseHandle(TimerThreadHandle);
    TOlxVars::Finalise();
    PythonExt::Finilise();
    CloseHandle(conin);
    CloseHandle(conout);
    OlexInstance = NULL;
  }
  HANDLE GetConin()  {  return conin;  }
  HANDLE GetConout() {  return conout;  }
  virtual bool executeMacro(const olxstr& cmdLine)  {
    TMacroError ME;
    TEStack<olxstr> stack;
    ME.SetStack( stack );
    Macros.ProcessMacro( cmdLine, ME );
    AnalyseError(ME);
    return ME.IsSuccessful();
  }
  virtual void print(const olxstr& msg, const short MessageType = olex::mtNone)  {
    CONSOLE_SCREEN_BUFFER_INFO pi;
    GetConsoleScreenBufferInfo(conout, &pi); 
    switch( MessageType )  {
      case olex::mtError:
      case olex::mtException:
        SetConsoleTextAttribute(conout, FOREGROUND_RED|FOREGROUND_INTENSITY);
        break;
      case olex::mtWarning:
        SetConsoleTextAttribute(conout, FOREGROUND_RED);
        break;
      case olex::mtInfo:
        SetConsoleTextAttribute(conout, FOREGROUND_BLUE|FOREGROUND_INTENSITY);
        break;
      default:
        SetConsoleTextAttribute(conout, 0);
    }
    TBasicApp::GetLog() << msg << '\n';
    //SetConsoleTextAttribute(conout, FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE);
    SetConsoleTextAttribute(conout, pi.wAttributes);
  }
  virtual bool executeFunction(const olxstr& function, olxstr& retVal)  {
    retVal = function;
    TMacroError ME;
    Macros.ProcessMacroFunc( retVal, ME );
    AnalyseError(ME);
    return ME.IsSuccessful();
  }
  virtual IEObject* executeFunction(const olxstr& function)  {
    int ind = function.FirstIndexOf('(');
    if( (ind == -1) || (ind == (function.Length()-1)) || !function.EndsWith(')') )  {
      TBasicApp::GetLog().Error( olxstr("Incorrect function call: ") << function);
      return NULL;
    }
    olxstr funName = function.SubStringTo(ind);
    ABasicFunction* Fun = GetLibrary().FindFunction( funName );
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
  virtual bool registerCallbackFunc(const olxstr& cbEvent, ABasicFunction* fn)  {
    CallbackFuncs.Add(cbEvent, fn);
    return true;
  }
  virtual void unregisterCallbackFunc(const olxstr& cbEvent, const olxstr& funcName)  {
    int ind = CallbackFuncs.IndexOfComparable(cbEvent), i = ind;
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
  virtual const olxstr& getDataDir() const  {
    return DataDir;
  }
  virtual const olxstr& getVar(const olxstr &name, const olxstr &defval=NullString) const  {
    int i = TOlxVars::VarIndex(name);
    if( i == -1 )  {
      if( &defval == NULL )
        throw TInvalidArgumentException(__OlxSourceInfo, "undefined key");
      TOlxVars::SetVar(name, defval);
      return defval;
    }
    return TOlxVars::GetVarStr(i);
  }
  virtual void setVar(const olxstr &name, const olxstr &val) const  {
    TOlxVars::SetVar(name, val);
  }
  virtual TLibrary&  GetLibrary() {  return XApp.GetLibrary();  }
  bool Dispatch( int MsgId, short MsgSubId, const IEObject *Sender, const IEObject *Data=NULL)  {
    bool res = true;
    if( MsgId == ID_TIMER )  {
      if( XApp.XFile().GetLastLoader() != NULL )  {
        olxstr sfn( TEFile::ExtractFilePath(XApp.XFile().GetFileName()) + "autochem.stop");
        if( TEFile::FileExists(sfn) )  {
          XApp.Sleep(100);
          TEFile::DelFile(sfn);
//          TOlex::~TOlex();
          TerminateSignal = true;
//          exit(0);
        }
      }
      TBasicApp::GetInstance()->OnTimer->SetEnabled( false );
      // execute tasks ...
      // end tasks ...
      if( FProcess != NULL )  {
        while( FProcess->StrCount() != 0 )  {
          SetConsoleTextAttribute(conout, FOREGROUND_GREEN);
          TBasicApp::GetLog() << FProcess->GetString(0) << '\n';
          SetConsoleTextAttribute(conout, TextAttrib.wAttributes);
          CallbackFunc(ProcessOutputCBName, FProcess->GetString(0));
          FProcess->DeleteStr(0);
        }
      }
      //    if( (FMode & mListen) != 0 )  {
      TBasicApp::GetInstance()->OnTimer->SetEnabled( true );
    }
    else if( MsgId == ID_PROCESSTERMINATE )  SetProcess(NULL);
    else if( MsgId == ID_INFO || MsgId == ID_WARNING || MsgId == ID_ERROR || MsgId == ID_EXCEPTION )  {
      if( MsgSubId == msiEnter )  {
        if( Data != NULL )  {
          int mat = 0;
          if( MsgId == ID_INFO )           mat = FOREGROUND_BLUE|FOREGROUND_INTENSITY;
          else if( MsgId == ID_WARNING )   mat = FOREGROUND_RED;
          else if( MsgId == ID_ERROR )     mat = FOREGROUND_RED|FOREGROUND_INTENSITY;
          else if( MsgId == ID_EXCEPTION ) mat = FOREGROUND_RED|FOREGROUND_INTENSITY;
          SetConsoleTextAttribute(conout, mat);
          res = false;  // propargate to other streams, logs in particular
        }
      }
      else  if( MsgSubId == msiExit )  {
        SetConsoleTextAttribute(conout, TextAttrib.wAttributes);
      }
    }
    else if( MsgId == ID_STRUCTURECHANGED )
      Selection.Clear();
    return res;
  }
  ///////////////////////////////////////////////////////////////////////////////////////////////////
  void AnalyseError( TMacroError& error )  {
    if( !error.IsSuccessful() )  {
      if( error.GetStack() != NULL )  {
        while( !error.GetStack()->IsEmpty() )  {
          TBasicApp::GetLog() << error.GetStack()->Pop() << '\n';
        }
      }
      if( error.IsProcessingException() )  {
        print(olxstr(error.GetLocation()) << ": " <<  error.GetInfo(), olex::mtException);
      }
      else if( error.IsProcessingError() )  {
        print(olxstr(error.GetLocation()) << ": " <<  error.GetInfo(), olex::mtError);
      }
      else if( error.IsInvalidOption() )
        TBasicApp::GetLog().Error(error.GetInfo());
      else if( error.IsInvalidArguments() )
        TBasicApp::GetLog().Error(error.GetInfo());
      else if( error.IsIllegalState() )
        TBasicApp::GetLog().Error(error.GetInfo());
      else if( !error.DoesFunctionExist() )
        print(error.GetInfo(), olex::mtWarning);
    }
  }
  //..............................................................................
  void macLstFun(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
    TBasicFunctionPList functions;
    GetLibrary().ListAllFunctions( functions );
    olxstr line;
    for( int i=0; i < functions.Count(); i++ )  {
      ABasicFunction* func = functions[i];
      line = func->GetQualifiedName();
      TBasicApp::GetLog() << (line << " - " << func->GetSignature() << '\n');
    }
  }
  //..............................................................................
  void macLstMac(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
    TBasicFunctionPList macros;
    GetLibrary().ListAllMacros( macros );
    olxstr line;
    for( int i=0; i < macros.Count(); i++ )  {
      ABasicFunction* func = macros[i];
      line = func->GetQualifiedName();
      TBasicApp::GetLog() << (line << " - " << func->GetSignature() << '\n');
    }
  }
  //..............................................................................
  void macLstFS(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
    double tc = 0;
    TTTable<TStrList> tab(TFileHandlerManager::Count(), 4);
    tab.ColName(0) = "Name";
    tab.ColName(1) = "Size";
    tab.ColName(2) = "Timestamp";
    tab.ColName(3) = "Persistent";
    for(int i=0; i < TFileHandlerManager::Count(); i++ )  {
      tab.Row(i)->String(0) = TFileHandlerManager::GetBlockName(i);
      tab.Row(i)->String(1) = TFileHandlerManager::GetBlockSize(i);
      tab.Row(i)->String(2) = TFileHandlerManager::GetBlockDateTime(i);
      tab.Row(i)->String(3) = TFileHandlerManager::GetPersistenceId(i);
      tc += TFileHandlerManager::GetBlockSize(i);
    }
    tc /= (1024*1024);
    TStrList Output;
    tab.CreateTXTList(Output, olxstr("Virtual FS content"), true, false, "|");
    TBasicApp::GetLog() << Output << '\n';
    TBasicApp::GetLog() << (olxstr("Content size is ") << olxstr::FormatFloat(3, tc)  << "Mb");
  }
  //..............................................................................
  void macLstVar(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
    if( TOlxVars::VarCount() == 0 )  return;
    TTTable<TStrList> tab(TOlxVars::VarCount(), 3);
    tab.ColName(0) = "Name";
    tab.ColName(1) = "Value";
    tab.ColName(2) = "RefCnt";
    for( int i=0; i < TOlxVars::VarCount(); i++ )  {
      tab[i][0] = TOlxVars::GetVarName(i);
      tab[i][1] = TOlxVars::GetVarStr(i);
      if( TOlxVars::GetVarWrapper(i) != NULL )
        tab[i][2] = TOlxVars::GetVarWrapper(i)->ob_refcnt;
      else
        tab[i][2] = NAString;
    }
    TStrList Output;
    tab.CreateTXTList(Output, "Variables list", true, true, ' ');
    TBasicApp::GetLog() << Output << '\n';
  }
  //..............................................................................
  void funLSM(const TStrObjList& Params, TMacroError &E) {
    TIns *I = (TIns*)XApp.XFile().GetLastLoader();
    E.SetRetVal( I->GetRefinementMethod() );
  }
  //..............................................................................
  void funSel(const TStrObjList& Params, TMacroError &E) {
    olxstr rv;
    for( int i=0; i < Selection.Count(); i++ )
      rv << Selection[i]->GetLabel() << ' ';
    E.SetRetVal( rv );
  }
  //..............................................................................
  void macHAdd(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
    TSAtomPList satoms;
    XApp.FindSAtoms( Cmds.Text(' '), satoms );
    TXlConGen xlcg( (TIns*)XApp.XFile().GetLastLoader() );
    XApp.XFile().GetLattice().AnalyseHAdd( xlcg, satoms );
    XApp.XFile().EndUpdate();
    delete XApp.FixHL();
  }
  //..............................................................................
  void macSel(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
    TLattice& latt = XApp.XFile().GetLattice();
    if( Options.Contains("-u") )  {
      Selection.Clear();
    }
    else if( Options.Contains("-i") )  {
      for( int i=0; i < latt.AtomCount(); i++ )
        latt.GetAtom(i).SetTag(0);
      for( int i=0; i < Selection.Count(); i++ )
        Selection[i]->SetTag(1);
      Selection.Clear();
      for( int i=0; i < latt.AtomCount(); i++ )
        if( latt.GetAtom(i).GetTag() == 0 && !latt.GetAtom(i).IsDeleted() )
          Selection.Add(&latt.GetAtom(i));
    }
    else if( Options.Contains("-a" ) )  {
      Selection.Clear();
      Selection.SetCapacity( latt.AtomCount() );
      for( int i=0; i < latt.AtomCount(); i++ )
        if( !latt.GetAtom(i).IsDeleted() )
          Selection.Add(&latt.GetAtom(i));
    }
    else {
      if( Cmds.Count() > 1 && Cmds[0].Comparei("satoms") == 0 )  {
        int wi = Cmds.IndexOf("where");
        olxstr Where( Cmds.Text(' ', wi+1).LowerCase() );
        TSFactoryRegister rf;
        TTSAtom_EvaluatorFactory *satom = (TTSAtom_EvaluatorFactory*)rf.BindingFactory("satom");
        TSyntaxParser SyntaxParser(&rf, Where);
        if( SyntaxParser.Errors().Count() == 0 )  {
          for( int i=0; i < latt.AtomCount(); i++ )  {
            if( latt.GetAtom(i).IsDeleted() )  continue;
            satom->SetTSAtom_( &latt.GetAtom(i) );
            if( SyntaxParser.Evaluate() )  Selection.Add( &latt.GetAtom(i) );
          }
        }
        else
          XApp.GetLog().Error( SyntaxParser.Errors().Text('\n') );
      }
      else  {
        XApp.FindSAtoms(Cmds.Text(' '), Selection);
      }
      UnifyAtomList(Selection);
    }
  }
  //..............................................................................
  void macFixHL(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
    delete XApp.FixHL();
  }
  //..............................................................................
  void macPlan(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error) {
    int plan = Cmds[0].ToInt();
    if( plan == -1 )  return; // leave like it is
    ((TIns*)XApp.XFile().GetLastLoader())->SetPlan( plan );
  }
  //..............................................................................
  void macLS(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
    TIns* iF = (TIns*)XApp.XFile().GetLastLoader();
    for( int i=0; i < Cmds.Count(); i++ )  {
      if( Cmds[i].IsNumber() )  {
        int ls = Cmds[i].ToInt();
        Cmds.Delete(i);
        if( ls != -1 ) iF->SetIterations(ls);
        break;
      }
    }
    if( !Cmds.IsEmpty() )  iF->SetRefinementMethod( Cmds[0] );
  }
  //..............................................................................
  void macStop(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
    return;
  }
  //..............................................................................
  void macWaitFor(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
    if( Cmds[0].Comparei("process") == 0 )  {
      while( FProcess != NULL )  {
        XApp.Sleep(50);
        
      }
    }
  }
  //..............................................................................
  void macDelIns(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
    TIns *Ins = (TIns*)XApp.XFile().GetLastLoader();
    if( Cmds[0].IsNumber() )  {
      int insIndex = Cmds[0].ToInt();
      Ins->DelIns(insIndex);
      return;
    }
    for( int i=0; i < Ins->InsCount(); i++ )  {
      if(  !Ins->InsName(i).Comparei(Cmds[0]) )  {
        Ins->DelIns(i);  i--;  continue;
      }
    }
  }
  //..............................................................................
  void macReset(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
    if( !(XApp.CheckFileType<TIns>() ||
      XApp.CheckFileType<TP4PFile>() ||
      XApp.CheckFileType<TCRSFile>()  )  )  return;

    olxstr newSg(Options.FindValue('s')), 
      content( olxstr::DeleteChars(Options.FindValue('c'), ' ')),
      fileName(Options.FindValue('f') );

    TIns *Ins = (TIns*)XApp.XFile().FindFormat("ins");
    if( XApp.CheckFileType<TP4PFile>() )  {
      if( newSg.IsEmpty() )  {
        E.ProcessingError(__OlxSrcInfo, "please specify a space group with -s=SG switch" );
        return;
      }
      Ins->Adopt( &XApp.XFile() );
    }
    else if( XApp.CheckFileType<TCRSFile>() )  {
      TSpaceGroup* sg = ((TCRSFile*)XApp.XFile().GetLastLoader())->GetSG();
      if( newSg.IsEmpty() )  {
        if( sg == NULL )  {
          E.ProcessingError(__OlxSrcInfo, "please specify a space group with -s=SG switch" );
          return;
        }
        else 
          TBasicApp::GetLog() << ( olxstr("The CRS file format space group is: ") << sg->GetName() << '\n');
      }
      Ins->Adopt( &XApp.XFile() );
    }
    if( !content.IsEmpty() )  Ins->SetSfacUnit( content );
    if( Ins->GetSfac().Length() == 0)  {
      E.ProcessingError(__OlxSrcInfo, "empty SFAC instruction, please use -c=Content to specify" );
      return;
    }

    if( !newSg.IsEmpty() )  {
      TSpaceGroup* sg = TSymmLib::GetInstance()->FindGroup( newSg );
      if( sg == NULL )  {
        E.ProcessingError(__OlxSrcInfo, "could not find space group: ") << newSg;
        return;
      }
      Ins->GetAsymmUnit().ChangeSpaceGroup( *sg );
      newSg = EmptyString;
      newSg <<  " reset to " << sg->GetName() << " #" << sg->GetNumber();
    }
    if( fileName.IsEmpty() )
      fileName = XApp.XFile().GetFileName();
    olxstr FN( TEFile::ChangeFileExt(fileName, "ins") );
    olxstr lstFN( TEFile::ChangeFileExt(fileName, "lst") );

    Ins->SaveToRefine(FN, Cmds.Text(' '), newSg);
    if( TEFile::FileExists(lstFN) )  {
      olxstr lstTmpFN( lstFN );
      lstTmpFN << ".tmp";
      TEFile::Rename( lstFN, lstTmpFN );
    }
    executeMacro(olxstr("@reap \'") << FN << '\'');
  }
  //..............................................................................
  void funLst(const TStrObjList &Cmds, TMacroError &E)  {
    if( !Lst.IsLoaded() )  {
      E.SetRetVal<olxstr>( NAString );
    }
    else if( !Cmds[0].Comparei("rint") )
      E.SetRetVal( Lst.Rint() );
    else if( !Cmds[0].Comparei("rsig") )
      E.SetRetVal( Lst.Rsigma() );
    else if( !Cmds[0].Comparei("r1") )
      E.SetRetVal( Lst.R1() );
    else if( !Cmds[0].Comparei("r1a") )
      E.SetRetVal( Lst.R1a() );
    else if( !Cmds[0].Comparei("wr2") )
      E.SetRetVal( Lst.wR2() );
    else if( !Cmds[0].Comparei("s") )
      E.SetRetVal( Lst.S() );
    else if( !Cmds[0].Comparei("rs") )
      E.SetRetVal( Lst.RS() );
    else if( !Cmds[0].Comparei("params") )
      E.SetRetVal( Lst.Params() );
    else if( !Cmds[0].Comparei("rtotal") )
      E.SetRetVal( Lst.TotalRefs() );
    else if( !Cmds[0].Comparei("runiq") )
      E.SetRetVal( Lst.UniqRefs() );
    else if( !Cmds[0].Comparei("r4sig") )
      E.SetRetVal( Lst.Refs4sig() );
    else if( !Cmds[0].Comparei("peak") )
      E.SetRetVal( Lst.Peak() );
    else if( !Cmds[0].Comparei("hole") )
      E.SetRetVal( Lst.Hole() );
    else
      E.SetRetVal<olxstr>( NAString );
  }
  //..............................................................................
  void macAddIns(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
    XApp.XFile().UpdateAsymmUnit();
    TIns * Ins = (TIns*)XApp.XFile().GetLastLoader();
    olxstr InsName;
    if( Cmds.Count() == 1 )  {
      int spindex = Cmds[0].IndexOf(' ');
      if( spindex != -1 )  {
        InsName = Cmds[0].SubStringTo(spindex).Trim(' ');
        Cmds[0].Delete(0, spindex+1);
      }
      else  {
        InsName = Cmds[0];
        Cmds.Delete(0);
      }
    }
    else  {
      InsName = Cmds[0];
      Cmds.Delete(0);
    }
    if( !Ins->AddIns(InsName, Cmds) )  {
      Error.ProcessingError(__OlxSrcInfo, "could not add instruction" );
      return;
    }
    Ins->UpdateParams();
  }
  //..............................................................................
  void macIF(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
    if( Cmds.Count() < 2 || Cmds[1].Comparei("then"))  {
      E.ProcessingError(__OlxSrcInfo, "incorrect syntax - two commands or a command and \'then\' are expected" );
      return;
    }
    olxstr Condition = Cmds[0];
    if( !Macros.ProcessMacroFunc(Condition, E) )  {
      return;
    }
    if( Condition.ToBool() )  {
      if( Cmds[2].Comparei("none") )
        Macros.ProcessMacro(Cmds[2], E);
      return;
    }
    else  {
      if( Cmds.Count() == 5 )  {
        if( !Cmds[3].Comparei("else") )  {
          if( Cmds[4].Comparei("none") )
            Macros.ProcessMacro(Cmds[4], E);
          return;
        }
        else  {
          E.ProcessingError(__OlxSrcInfo, "no keyword 'else' found" );
          return;
        }
      }
    }
  }
  //..............................................................................
  void SetProcess(AProcess *Process)  {
    if( FProcess != NULL && Process == NULL )  {
      while( FProcess->StrCount() != 0 )  {
        TBasicApp::GetLog() << FProcess->GetString(0) << '\n';
        CallbackFunc(ProcessOutputCBName, FProcess->GetString(0));
        FProcess->DeleteStr(0);
      }
      TBasicApp::GetLog() << '\n';

//      if( FMode & mListen )
//        Dispatch(ID_TIMER, msiEnter, (AEventsDispatcher*)this, NULL);

      FOnListenCmds.Clear();
      olxstr Cmd;
      TMacroError err;
      while( FProcess->OnTerminateCmds().Count() ) {
        Cmd = FProcess->OnTerminateCmds().String(0);
        FProcess->OnTerminateCmds().Delete(0);
        Macros.ProcessMacro(Cmd, err);
        if( !err.IsSuccessful() )  {
          AnalyseError( err );
          FProcess->OnTerminateCmds().Clear();
          break;
        }
      }
    }
    if( Process )
      Process->OnTerminate->Add(this, ID_PROCESSTERMINATE);

    if( FProcess )  {  
      FProcess->OnTerminate->Clear();  
      FProcess->Detach();
    }
    FProcess = Process;
    if( FProcess == NULL )  {
      //TBasicApp::GetLog() << "\n>>";
    }
  }
  void macExec(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
    bool Asyn = true, // synchroniusly
      Cout = true;    // catch output

    olxstr dubFile;

    for( int i=0; i < Options.Count(); i++ )  {
      if( Options.GetName(i) == "s" )  Asyn  = false;
      else if( Options.GetName(i) == "o" )  Cout = false;
      else if( Options.GetName(i) == "d" )  dubFile = Options.GetValue(i);
    }
    olxstr Tmp;
    bool Space;
    for( int i=0; i < Cmds.Count(); i++ )  {
      Space =  (Cmds[i].FirstIndexOf(' ') != -1 );
      if( Space )  Tmp << '\"';
      Tmp << Cmds[i];
      if( Space ) Tmp << '\"';
      Tmp << ' ';
    }
    TBasicApp::GetLog() << (olxstr("EXEC: ") << Tmp << '\n');
    TWinProcess* Process  = new TWinProcess;
    Process->OnTerminateCmds().Assign( FOnTerminateMacroCmds );
    FOnTerminateMacroCmds.Clear();
//    TBasicApp::GetLog() << "Please press Ctrl+C to terminate a process...\n";
    if( (Cout && Asyn) || Asyn )  {  // the only combination
      if( !Cout )  {
        SetProcess(Process);
        if( !Process->Execute(Tmp, 0) )  {
          Error.ProcessingError(__OlxSrcInfo, "failed to launch a new process" );
          return;
        }
        return;
      }
      else  {
        SetProcess(Process);
        if( !dubFile.IsEmpty() )  {
          TEFile* df = new TEFile(dubFile, "wb+");
          Process->SetDubStream( df );
        }
        if( !Process->Execute(Tmp, spfRedirected) )  {
          Error.ProcessingError(__OlxSrcInfo, "failed to launch a new process" );
          SetProcess(NULL);
          return;
        }
      }
      return;
    }
    if( !Process->Execute(Tmp, spfSynchronised) )  {
      Error.ProcessingError(__OlxSrcInfo, "failed to launch a new process" );
      return;
    }
    delete Process;
  }
  //..............................................................................
  void macPython(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
    olxstr tmp = Cmds.Text(' ');
    tmp.Replace("\\n", "\n");
    if( !tmp.EndsWith('\n') )  tmp << '\n';
    PythonExt::GetInstance()->RunPython(  tmp, false );
  }
  //..............................................................................
  void macReload(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
    if( Cmds[0].Comparei("macro") == 0 )  {
      olxstr macroFile( XApp.BaseDir() + "macrox.xld" );
      if( TEFile::FileExists(macroFile) )  {
        TDataFile df;
        df.LoadFromXLFile( macroFile );
        df.Include(NULL);
        TDataItem* di = df.Root().FindItem("xl_macro");
        if( di != NULL )  Macros.Load( *di );
      }
    }
  }
  //..............................................................................
  void macClear(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
    return;
  }
  //..............................................................................
  void macEcho(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
    for( int i=0; i < Cmds.Count(); i++ )  {
      TBasicApp::GetLog() << Cmds[i].c_str() << (((i+1) < Cmds.Count()) ? ' ' : '\n');
    }
  }
  //..............................................................................
  void funAnd(const TStrObjList& Params, TMacroError &E) {
    olxstr tmp;
    TMacroError ME;
    for(int i=0; i < Params.Count(); i++ )  {
      tmp = Params[i];
      if( !Macros.ProcessMacroFunc(tmp, ME) )  {
        E.ProcessingError(__OlxSrcInfo, "could not process: ") << tmp;
        return;
      }
      if( !tmp.ToBool() )  {
        E.SetRetVal( false );
        return;
      }
    }
    E.SetRetVal( true );
  }
  //..............................................................................
  void funOr(const TStrObjList& Params, TMacroError &E) {
    olxstr tmp;
    TMacroError ME;
    for(int i=0; i < Params.Count(); i++ )  {
      tmp = Params[i];
      if( !Macros.ProcessMacroFunc(tmp, ME) )  {
        E.ProcessingError(__OlxSrcInfo, "could not process: ") << tmp;
        return;
      }
      if( tmp.ToBool() )  {
        E.SetRetVal( true );
        return;
      }
    }
    E.SetRetVal( false );
  }
  //..............................................................................
  void funNot(const TStrObjList& Params, TMacroError &E) {
    olxstr tmp = Params[0];
    TMacroError ME;
    if( !Macros.ProcessMacroFunc(tmp, ME) )  {
      E.ProcessingError(__OlxSrcInfo, "could not process: ") << tmp;
      return;
    }
    E.SetRetVal( !tmp.ToBool() );
  }
  //..............................................................................
  void funIsPluginInstalled(const TStrObjList& Params, TMacroError &E) {
    E.SetRetVal( Plugins->ItemExists(Params[0]) );
  }
  //..............................................................................
  void funCurrentLanguageEncoding(const TStrObjList& Params, TMacroError &E) {
    E.SetRetVal<olxstr>( "ISO8859-1" );
  }
  //..............................................................................
  void funHasGUI(const TStrObjList& Params, TMacroError &E) {
    E.SetRetVal( false );
  }
  //..............................................................................
  void funStrDir(const TStrObjList& Params, TMacroError &E) {
    olxstr ofn( TEFile::ExtractFilePath(XApp.XFile().GetFileName()) );
    TEFile::AddTrailingBackslashI(ofn) << ".olex";
    if( !TEFile::FileExists(ofn) )  {
      if( !TEFile::MakeDir(ofn) )  {
        throw TFunctionFailedException(__OlxSourceInfo, "cannot create folder");
      }
#ifdef __WIN32__
      SetFileAttributes(ofn.u_str(), FILE_ATTRIBUTE_HIDDEN);
#endif
    }
    E.SetRetVal( ofn );
  }
  //..............................................................................
  void funDataDir(const TStrObjList& Params, TMacroError &E)  {
    E.SetRetVal( DataDir.SubStringFrom(0, 1) );
  }
  //..............................................................................
  void funStrCmp(const TStrObjList& Params, TMacroError &E)  {
    E.SetRetVal( Params[0] == Params[1] );
  }
  //..............................................................................
  void funGetCompilationInfo(const TStrObjList& Params, TMacroError &E)  {
    E.SetRetVal( olxstr(__DATE__) << ' ' << __TIME__ );
  }
  //..............................................................................
  void funUnsetVar(const TStrObjList& Params, TMacroError &E)  {
    TOlxVars::UnsetVar(Params[0]);
  }
  //..............................................................................
  void funSetVar(const TStrObjList& Params, TMacroError &E)  {
    TOlxVars::SetVar(Params[0], Params[1]);
  }
  //..............................................................................
  void funIns(const TStrObjList& Params, TMacroError &E)  {
    TIns *I = (TIns*)XApp.XFile().GetLastLoader();

    olxstr tmp;
    if( Params[0].Comparei("weight") == 0 || Params[0].Comparei("wght") == 0 )  {
      for( int j=0; j < I->Wght().Count(); j++ )  {
        tmp << I->Wght()[j];
        if( (j+1) < I->Wght().Count() )  tmp << ' ';
      }
      E.SetRetVal( tmp );
      return;
    }
    if( !Params[0].Comparei("weight1") )  {
      for( int j=0; j < I->Wght1().Count(); j++ )  {
        tmp << I->Wght1()[j];
        if( (j+1) < I->Wght1().Count() )  tmp << ' ';
      }
      E.SetRetVal( tmp );
      return;
    }
    if( (Params[0].Comparei("L.S.") == 0) || (Params[0].Comparei("CGLS") == 0) )  {
      for( int i=0; i < I->GetLSV().Count(); i++ )  {
        tmp << I->GetLSV()[i];
        if( (i+1) < I->GetLSV().Count() )  tmp << ' ';
      }
      E.SetRetVal( I->GetLSV().Count() == 0 ? NAString : tmp );
      return;
    }
    if( Params[0].Comparei("ls") == 0 )  {
      E.SetRetVal( I->GetLSV().Count() == 0 ? NAString : olxstr(I->GetIterations()) );
      return;
    }
    if( Params[0].Comparei("plan") == 0)  {
      for( int i=0; i < I->GetPlanV().Count(); i++ )  {
        tmp << ((i < 1) ? Round(I->GetPlanV()[i]) : I->GetPlanV()[i]);
        if( (i+1) < I->GetPlanV().Count() )  tmp << ' ';
      }
      E.SetRetVal( I->GetPlanV().Count() == 0 ? NAString : tmp );
      return;
    }
    if( Params[0].Comparei("qnum") == 0)  {
      E.SetRetVal( I->GetPlanV().Count() == 0 ? NAString : olxstr(I->GetPlan()) );
      return;
    }
    if( Params[0].Comparei("R1") == 0)  {
      E.SetRetVal( I->GetR1() < 0 ? NAString : olxstr(I->GetR1()) );
      return;
    }
    if( !I->InsExists(Params[0]) )  {
      E.SetRetVal( NAString );
      return;
    }

  TInsList* insv = I->FindIns( Params[0] );
    E.SetRetVal( (insv != NULL) ? insv->Text(' ') : EmptyString );
  }
  //..............................................................................
  void funSG(const TStrObjList &Cmds, TMacroError &E)  {
    TSpaceGroup* sg = NULL;
    try  { sg = &FXApp->XFile().GetLastLoaderSG();  }
    catch(...)  {}
    if( sg != NULL )  {
      olxstr Tmp;
      if( Cmds.IsEmpty() )  {
        Tmp = sg->GetName();
        if( !sg->GetFullName().IsEmpty() )  {
          Tmp << " (" << sg->GetFullName() << ')';
        }
        Tmp << " #" << sg->GetNumber();
      }
      else  {
        Tmp = Cmds[0];
        Tmp.Replace("%#", olxstr(sg->GetNumber()) );
        Tmp.Replace("%n", sg->GetName());
        Tmp.Replace("%N", sg->GetFullName());
        Tmp.Replace("%H", sg->GetHallSymbol());
        if( Tmp.IndexOf("%h") != -1 )  {
          olxstr t(sg->GetFullName()), res;
          res.SetCapacity( t.Length() + 20 );
          for( int i=0; i < t.Length(); i++ )  {
            if( (i+1) < t.Length() )  {
              if( (t[i] >= '0' && t[i] <= '9')  &&  (t[i+1] >= '0' && t[i+1] <= '9') )  {
                res << t[i] << "<sub>" << t[i+1] << "</sub>";
                i++;
                continue;
              }
            }
            res << t[i];
          }
          Tmp.Replace("%h", res);
        }
      }
      E.SetRetVal( Tmp );
    }
    else  {
      E.ProcessingError(__OlxSrcInfo, "could not find space group for the file" );
      return;
    }
  }
  //..............................................................................
  void funGetVar(const TStrObjList& Params, TMacroError &E)  {
    int ind = TOlxVars::VarIndex(Params[0]);
    if( ind == -1 )  {
      if( Params.Count() == 2 )
        E.SetRetVal( Params[1] );
      else  
        E.ProcessingError(__OlxSrcInfo, "Could not locate specified variable: '") << Params[0] << '\'';
      return;
    }
    E.SetRetVal( TOlxVars::GetVarStr(ind) );
  }
  //..............................................................................
  void funIsVar(const TStrObjList& Params, TMacroError &E)  {
    E.SetRetVal( TOlxVars::IsVar(Params[0]));
  }
  //..............................................................................
  void macReap(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
    Lst.Clear();
    XApp.XFile().LoadFromFile( Cmds[0] );
    olxstr lstfn( TEFile::ChangeFileExt(Cmds[0], "lst") );
    if( TEFile::FileExists(lstfn) )
      Lst.LoadFromFile(lstfn);
  }
  //..............................................................................
  void funUser(const TStrObjList &Cmds, TMacroError &Error)  {
    if( Cmds.IsEmpty() )
      Error.SetRetVal( TEFile::CurrentDir() );
    else
      TEFile::ChangeDir( Cmds[0] );
  }
  //..............................................................................
  void macInfo(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
    TCAtomPList atoms;
    if( Cmds.IsEmpty() )  {
      for( int i=0; i < XApp.XFile().GetAsymmUnit().AtomCount(); i++ )  {
        TCAtom& ca = XApp.XFile().GetAsymmUnit().GetAtom(i);
        if( ca.IsDeleted() )  continue;
        atoms.Add( &ca );
      }
    }
    else  {
      TAtomReference ar(Cmds.Text(' '));      
      TCAtomGroup ag;
      int atomAGroup;
      olxstr unp(ar.Expand(XApp.XFile().GetAsymmUnit(), ag, EmptyString, atomAGroup));
      for( int i=0; i < ag.Count(); i++ )  
        atoms.Add( ag[i].GetAtom() );
    }
    TTTable<TStrList> Table(atoms.Count(), 7);
    Table.ColName(0) = "Atom";
    Table.ColName(1) = "Symb";
    Table.ColName(2) = "X";
    Table.ColName(3) = "Y";
    Table.ColName(4) = "Z";
    Table.ColName(5) = "Ueq";
    Table.ColName(6) = "Peak";
    for(int i = 0; i < atoms.Count(); i++ )  {
      Table[i][0] = atoms[i]->GetLabel();
      Table[i][1] = atoms[i]->GetAtomInfo().GetSymbol();
      Table[i][2] = olxstr::FormatFloat(3, atoms[i]->ccrd()[0]);
      Table[i][3] = olxstr::FormatFloat(3, atoms[i]->ccrd()[1]);
      Table[i][4] = olxstr::FormatFloat(3, atoms[i]->ccrd()[2]);
      Table[i][5] = olxstr::FormatFloat(3, atoms[i]->GetUiso());
      if( atoms[i]->GetQPeak() != -1 )
        Table[i][6] = olxstr::FormatFloat(3, atoms[i]->GetQPeak());
      else
        Table[i][0] = '-';
    }
    TStrList out;
    Table.CreateTXTList(out, "Atom information", true, true, ' ');
    TBasicApp::GetLog() << out;
  }
  //..............................................................................
  void macName(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
    TSAtomPList atoms;
    TStrObjList toks( Cmds.Text(' ', 0, Cmds.Count()-1), ' ');
    if( !LocateAtoms(toks, atoms, false) )  return;
    if( Cmds[Cmds.Count()-1].IsNumber() )  {
      int start = Cmds[1].ToInt();
      for( int i=0; i < atoms.Count(); i++ ) {
        atoms[i]->CAtom().Label() = atoms[i]->GetAtomInfo().GetSymbol();
        atoms[i]->CAtom().Label() << start++;
      }
    }
    else  {
      for( int i=0; i < atoms.Count(); i++ ) 
        atoms[i]->CAtom().SetLabel(Cmds[1]);
    }
  }
  //..............................................................................
  void macKill(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
    TSAtomPList atoms;
    if( !LocateAtoms(Cmds, atoms, false) )  return;
    for( int i=0; i < atoms.Count(); i++ ) 
      atoms[i]->CAtom().SetDeleted(true);
    XApp.XFile().EndUpdate();
    UnifyAtomList(Selection);
  }
  //..............................................................................
  void macAnis(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
    TSAtomPList atoms;
    TCAtomPList catoms;
    if( !LocateAtoms(Cmds, atoms, true) )  return;
    TListCaster::POP(atoms, catoms);
    for( int i=0; i < catoms.Count(); i++ )
      if( catoms[i]->GetAtomInfo() == iHydrogenIndex )
        catoms[i] = NULL;
    catoms.Pack();
    XApp.XFile().GetLattice().SetAnis(catoms, true);
  }
  //..............................................................................
  void macIsot(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
    TSAtomPList atoms;
    if( !LocateAtoms(Cmds, atoms, true) )  return;
    TCAtomPList catoms;
    TListCaster::POP(atoms, catoms);
    XApp.XFile().GetLattice().SetAnis(catoms, false);
  }
  //..............................................................................
  void macFix(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
    olxstr vars( Cmds[0] );
    Cmds.Delete(0);
    TSAtomPList atoms;
    if( !LocateAtoms(Cmds, atoms, true) )  return;
    if( vars.Comparei( "XYZ" ) == 0 )  {
      for(int i=0; i < atoms.Count(); i++ )  {
        for( int j=0; j < 3; j++ )
          atoms[i]->CAtom().FixedValues()[TCAtom::CrdFixedValuesOffset + j] = 10;
      }
    }
    else if( vars.Comparei( "UISO" ) == 0 )  {
      double uiso = 0;
      if( Cmds.Count() > 1 && Cmds[0].IsNumber() )  {
        uiso = Cmds[0].ToDouble();
      }
      for( int i=0; i < atoms.Count(); i++ )  {
        if( atoms[i]->GetEllipsoid() == NULL )  {  // isotropic atom
          if( uiso != 0 )  {
            if( uiso < 10 )  {
              atoms[i]->CAtom().SetUiso( uiso );
              atoms[i]->CAtom().SetUisoVar( 10 );
            }
            else  {
              int iv = (int) uiso;
              atoms[i]->CAtom().SetUiso( uiso-iv );
              atoms[i]->CAtom().SetUisoVar( iv*10 );
            }
          }
          else  if( atoms[i]->CAtom().GetUisoVar() == 0 )  {  // have to skip riding atoms
            atoms[i]->CAtom().SetUisoVar( 10 );
          }
        }
        else  {
          for( int j=0; j < 6; j++ )
            atoms[i]->CAtom().FixedValues()[TCAtom::UisoFixedValuesOffset + j] = 10;
        }
      }
    }
    else if( vars.Comparei( "OCCU" ) == 0 )  {
      for(int i=0; i < atoms.Count(); i++ )  {
        if( atoms[i]->CAtom().GetPart() == 0 )  {
          atoms[i]->CAtom().SetOccp( 1./atoms[i]->CAtom().GetDegeneracy() );
          atoms[i]->CAtom().SetOccpVar( 10 );
        }
      }
    }
  }
  //..............................................................................
  void macSGE(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
    TPtrList<TSpaceGroup> sgs;
    TSpaceGroup* sg = NULL;
    bool cntro = false;
    E.SetRetVal(&sgs);
    Macros.ProcessMacro( "SG", E );
    E.SetRetVal<bool>(false);
    if( sgs.Count() == 0 )  {
      TBasicApp::GetLog().Error( "Could not find any suitable spacegroup. Terminating ... " );
      return;
    }
    else if( sgs.Count() == 1 )  {
      sg = sgs[0];
      TBasicApp::GetLog() << "Univocal spacegroup choice: " << sg->GetName() << '\n';
    }
    else  {
      E.Reset();
      Macros.ProcessMacro("Wilson", E);
      bool centro = E.GetRetVal().ToBool();
      TBasicApp::GetLog() << "Searching for centrosymmetric group: " << centro << '\n';
      for( int i=0; i < sgs.Count(); i++ )  {
        if( centro )  {
          if( sgs[i]->IsCentrosymmetric() )  {
            sg = sgs[i];
            break;
          }
        }
        else  {
          if( !sgs[i]->IsCentrosymmetric() )  {
            sg = sgs[i];
            break;
          }
        }
      }
      if( sg == NULL )  {  // no match to centre of symmetry found
        sg = sgs[0];
        TBasicApp::GetLog() << "Could not match, choosing: " << sg->GetName() << '\n';
      }
      else  {
        TBasicApp::GetLog() << "Chosen: " << sg->GetName() << '\n';
      }
    }
    olxstr fn( Cmds.IsEmpty() ? TEFile::ChangeFileExt(XApp.XFile().GetFileName(), "ins") : Cmds[0] );
    executeMacro(olxstr("reset -s=") << sg->GetName() << " -f='" << fn << '\'');
    if( E.IsSuccessful() )  {
      executeMacro(olxstr("reap '") << fn << '\'');
      E.SetRetVal<bool>(E.IsSuccessful());
    }
  }
  //..............................................................................
  void macFile(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
    olxstr Tmp;
    XApp.XFile().UpdateAsymmUnit();
    if( Cmds.IsEmpty() )  {  // error
      // res -> Ins rotation if ins file
      if( XApp.CheckFileType<TIns>() )
        Tmp = TEFile::ChangeFileExt(XApp.XFile().GetFileName(), "ins");
      else
        Tmp = XApp.XFile().GetFileName();
    }
    else
      Tmp = Cmds[0];

    if( TEFile::ExtractFilePath(Tmp).IsEmpty() )
      Tmp = TEFile::AddTrailingBackslash(TEFile::CurrentDir()) + Tmp;

    XApp.XFile().SaveToFile(Tmp, Options.Contains('s'));
    Tmp = TEFile::ExtractFilePath(Tmp);
    if( !Tmp.IsEmpty() && Tmp.Comparei(TEFile::CurrentDir()) != 0 )
      TEFile::ChangeDir(Tmp);
  }
//////////////////////////////////////////////////////////////////////////////////////////////////
  void CallbackFunc(const olxstr& cbEvent, TStrObjList& params)  {
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
  void CallbackFunc(const olxstr& cbEvent, const olxstr& param)  {
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
  static void PyInit();
  static bool TerminateSignal;
  static TOlex* OlexInstance;
};
bool TOlex::TerminateSignal = false;
TOlex* TOlex::OlexInstance;
////////////////////////////////////////////////////////////////////////////////////////
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
  olex::IOlexProcessor::GetInstance()->GetLibrary().ListAllFunctions( functions );
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
  olex::IOlexProcessor::GetInstance()->GetLibrary().ListAllMacros( functions );
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
//  TGlXApp::GetMainForm()->TranslateString(str);
  return PythonExt::BuildString(str);
}
//..............................................................................
PyObject* pyIsControl(PyObject* self, PyObject* args)  {
  return Py_BuildValue("b", false);
}
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
  {NULL, NULL, 0, NULL}
   };

void TOlex::PyInit()  {  Py_InitModule( "olex_core", CORE_Methods );  }

class TTerminationListener : public AActionHandler {
public:
  virtual bool Execute(const IEObject *Sender, const IEObject *Data=NULL)  {  
    if( TOlex::TerminateSignal )  {
      TBasicApp::GetLog() << "terminate\n";
      exit(0);
      DWORD w=0;
      WriteConsole(TOlex::OlexInstance->GetConin(),
        L"\n", 1, &w, NULL); 
    }
    return false; 
  }
};

int main(int argc, char* argv[])  {
	TOlex olex(argv[0]);
  SetConsoleTitle(L"Olex2 Console");
  TLibrary &Library = olex.GetLibrary();
  cout << "Welcome to Olex2 console\n";
  cout << "Compilation information: " << __DATE__ << ' ' << __TIME__ << '\n';
  cout << ">>";
  if( argc > 1 )  {
    olxstr arg_ext( TEFile::ExtractFileExt(argv[1]) );
    if( arg_ext.Comparei("autochem") == 0 )  {
      olxstr sf ( TEFile::ChangeFileExt(argv[1], EmptyString) );
      olex.executeMacro( olxstr("start_autochem '") << sf << '\'' );
    }
  }
  char _cmd[512];
  olxstr cmd;
  TBasicApp::GetInstance()->OnTimer->Add( new TTerminationListener );
  while( true )  {
    TBasicApp::GetInstance()->OnIdle->Execute(NULL);
    cin.getline(_cmd, 512);
    if( olex.TerminateSignal )  break;
    cmd = _cmd;
    if( cmd.Comparei("quit") == 0 )  break;
    else  {
      try { olex.executeMacro(cmd);  }
      catch( TExceptionBase& exc )  {
        TBasicApp::GetLog() << exc.GetException()->GetError();
      }
    }
    cout << ">>";
  }
  TBasicApp::GetInstance()->OnIdle->Execute(NULL);
  return 0;
}

