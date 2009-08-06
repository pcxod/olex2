// olex2c.cpp : Defines the entry point for the console application.
//

#include <iostream>

#ifndef __WIN32__
  #include "wx/app.h"
#endif
using namespace std;

#include "con_in.h"

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
#include "settingsfile.h"
#include "py_core.h"
#include "olxth.h"
#include "egc.h"

#ifndef __WIN32__
  #include <readline/readline.h>
  #include <readline/history.h>
#endif

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

class TOlex: public AEventsDispatcher, public olex::IOlexProcessor, public ASelectionOwner  {
  TXApp XApp;
  TLst Lst;
  olxstr DataDir;
  TCSTypeList<olxstr, ABasicFunction*> CallbackFuncs;
  TStrList FOnTerminateMacroCmds; // a list of commands called when a process is terminated
  TStrList FOnAbortCmds;           // a "stack" of macroses, called when macro terminated
  TStrList FOnListenCmds;  // a list of commands called when a file is changed by another process
  TDataItem* FMacroItem;
  AProcess* FProcess;
  TEMacroLib Macros;
  TSAtomPList Selection;
  TDataFile PluginFile;
  TDataItem* Plugins;
  ConsoleInterface conint;
  bool Silent;
  TOutStream* OutStream;
  static void* TimerThreadFunction() {
    while( true )  {
      if( !TBasicApp::HasInstance() )  return 0;
      TBasicApp::GetInstance().OnTimer->Execute(NULL);
      olx_sleep(50);
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
  // slection owner interface
  virtual void ExpandSelection(TCAtomGroup& atoms)  {
    for( int i=0; i < Selection.Count(); i++ )
      atoms.AddNew( &Selection[i]->CAtom() );
    if( GetDoClearSelection() )
      Selection.Clear();
  }
  bool LocateAtoms(TStrObjList &Cmds, TSAtomPList& atoms, bool all)  {
    XApp.FindSAtoms(Cmds.Text(' '), atoms, true);
    UnifyAtomList(atoms);
    return !atoms.IsEmpty();
  }
public:
  TOlex( const olxstr& basedir) : XApp(basedir, this), Macros(*this) {
    XApp.SetCifTemplatesDir( XApp.GetBaseDir() + "etc/CIF/" );
    OlexInstance = this;
    Silent = true;
    OutStream = new TOutStream();
    XApp.GetLog().AddStream(OutStream , false );
    XApp.GetLog().OnInfo->Add(this, ID_INFO);
    XApp.GetLog().OnWarning->Add(this, ID_WARNING);
    XApp.GetLog().OnError->Add(this, ID_ERROR);
    XApp.GetLog().OnException->Add(this, ID_EXCEPTION);

    TLibrary &Library = XApp.GetLibrary();
    PythonExt::Init(this).Register(&OlexPyCore::PyInit);
    Library.AttachLibrary( TEFile::ExportLibrary() );
    Library.AttachLibrary( PythonExt::GetInstance()->ExportLibrary() );
    Library.AttachLibrary( TETime::ExportLibrary() );
    Library.AttachLibrary( XApp.XFile().ExportLibrary() );
    Library.AttachLibrary( TFileHandlerManager::ExportLibrary() );

    DataDir = TShellUtil::GetSpecialFolderLocation(fiAppData);
		cout << DataDir.c_str() << '\n';
    AOlxThread::RunThread(&TOlex::TimerThreadFunction);
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

    TCif *Cif = new TCif;  // the objects will be automatically removed by the XApp
    XApp.XFile().RegisterFileFormat(Cif, "cif");
    XApp.XFile().RegisterFileFormat(Cif, "fcf");
    XApp.XFile().RegisterFileFormat(Cif, "fco");
    TMol *Mol = new TMol;  // the objects will be automatically removed by the XApp
    XApp.XFile().RegisterFileFormat(Mol, "mol");
    TIns *Ins = new TIns;
    XApp.XFile().RegisterFileFormat(Ins, "ins");
    XApp.XFile().RegisterFileFormat(Ins, "res");
    TXyz *Xyz = new TXyz;
    XApp.XFile().RegisterFileFormat(Xyz, "xyz");
    XApp.XFile().RegisterFileFormat(new TP4PFile, "p4p");
    XApp.XFile().RegisterFileFormat(new TCRSFile, "crs");
    XApp.XFile().RegisterFileFormat(new TPdb, "pdb");
    XApp.XFile().RegisterFileFormat(new TXDMas, "mas");
    XApp.XFile().GetLattice().OnStructureUniq->Add(this, ID_STRUCTURECHANGED);
    XApp.XFile().GetLattice().OnStructureGrow->Add(this, ID_STRUCTURECHANGED);
    XApp.XFile().OnFileLoad->Add(this, ID_STRUCTURECHANGED);

    this_InitMacroD(Silent, "", fpOne, "Changes silent mode");
    this_InitMacroD(IF, "", fpAny, "if...");
    this_InitMacroD(Exec, "s&;o&;d&;q", fpAny^fpNone, "exec" );
    this_InitMacroD(Echo, "", fpAny, "echo" );
    this_InitMacroDA(Reap, @reap, "", fpAny^fpNone, "reap" );
    this_InitMacroD(Name, "", fpAny^(fpNone|fpOne)|psFileLoaded, "name" );
    this_InitMacroD(Info, "", fpAny, "info" );
    this_InitMacroDA(Python, @py, "", fpAny^fpNone, "Runs python script" );
    this_InitMacroD(Clear, "", fpNone, "" );
    this_InitMacroD(Stop, "", fpOne, "" );
    this_InitMacroD(Reload, "", fpOne, "" );
    this_InitMacroD(Reset, "s&;c&;f", fpAny|psFileLoaded, "" );
    this_InitMacroD(WaitFor, "", fpOne, "" );
    this_InitMacroD(Kill, "", (fpAny^fpNone)|psFileLoaded, "" );
    this_InitMacroD(Sel, "i&;a&;u", fpAny|psFileLoaded, "" );
    this_InitMacroD(Quit, "", fpNone, "" );
    
    this_InitFuncD(User, fpNone|fpOne, "reap" );
    this_InitFuncD(SetVar, fpTwo, "setvar" );
    this_InitFuncD(UnsetVar, fpOne, "" );
    this_InitFuncD(GetVar, fpOne|fpTwo, "" );
    this_InitFuncD(IsVar, fpOne, "" );
    this_InitFuncD(DataDir, fpNone, "" );
    this_InitFuncD(StrDir, fpNone, "" );
    this_InitFuncD(GetCompilationInfo, fpNone|fpOne, "" );
    this_InitFuncD(IsPluginInstalled, fpOne, "" );
    this_InitFuncD(CurrentLanguageEncoding, fpNone, "" );
    this_InitFuncD(StrCmp, fpTwo, "" );
    this_InitFuncD(Lst, fpOne|psFileLoaded, "" );
    this_InitFuncD(And, fpAny^(fpNone|fpOne), "" );
    this_InitFuncD(Or, fpAny^(fpNone|fpOne), "" );
    this_InitFuncD(Not, fpOne, "" );
    this_InitFuncD(HasGUI, fpNone, "" );
    this_InitFuncD(Sel, fpNone, "" );
    
    olxstr pluginsFile( XApp.GetBaseDir() + "plugins.xld" );
    Plugins = NULL;
    if( TEFile::Exists( pluginsFile ) )  {
      PluginFile.LoadFromXLFile( pluginsFile, NULL );
      Plugins = PluginFile.Root().FindItem("Plugin");
    }
    else  
      Plugins = &PluginFile.Root().AddItem("Plugin");
  
    olxstr macroFile( XApp.GetBaseDir() );
    macroFile << "macrox.xld";
    if( TEFile::Exists(macroFile) )  {
      TDataFile df;
      df.LoadFromXLFile( macroFile );
      df.Include(NULL);
      TDataItem* di = df.Root().FindItem("xl_macro");
      if( di != NULL )
        Macros.Load( *di );
    }
    executeMacro("onstartup");
    TBasicApp::GetInstance().OnTimer->Add(this, ID_TIMER);
  }
  ~TOlex()  {
    TBasicApp::GetInstance().OnTimer->Clear();
    executeMacro("onexit");
    for( int i=0; i < CallbackFuncs.Count(); i++ )
      delete CallbackFuncs.GetObject(i);
    if( FProcess )  {
      FProcess->OnTerminate->Clear();
      FProcess->Terminate();
      delete FProcess;
    }
    TOlxVars::Finalise();
    PythonExt::Finilise();
    delete OutStream;
    OlexInstance = NULL;
  }
  virtual bool executeMacroEx(const olxstr& cmdLine, TMacroError& er)  {
    str_stack stack;
    er.SetStack( stack );
    Macros.ProcessMacro( cmdLine, er, false );
    AnalyseError(er);
    return er.IsSuccessful();
  }
  //..........................................................................................
  virtual void print(const olxstr& msg, const short MessageType = olex::mtNone)  {
    if( Silent && MessageType == olex::mtInfo ) return;
    conint.Push();
    switch( MessageType )  {
      case olex::mtError:
      case olex::mtException:
        conint.SetTextForeground( fgcRed, true);
        break;
      case olex::mtWarning:
        conint.SetTextForeground( fgcRed, false);
        break;
      case olex::mtInfo:
        conint.SetTextForeground( fgcBlue, true);
        break;
      default:
        conint.SetTextForeground( fgcReset, false);
    }
    TBasicApp::GetLog() << msg << '\n';
    //SetConsoleTextAttribute(conout, FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE);
    conint.Pop();
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
  virtual TStrList GetPluginList() const {
    TStrList rv;
    if( Plugins != NULL )  {
      for( int i=0; i < Plugins->ItemCount(); i++ )
        rv.Add(Plugins->GetItem(i).GetName());
    }
    return rv;
  }
  // do not translate for now...
  virtual olxstr TranslateString(const olxstr& str) const {  return str;  }
  // no GUI
  virtual bool IsControl(const olxstr& cname) const {  return false;  }

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
      if( XApp.XFile().HasLastLoader() )  {
        olxstr sfn( TEFile::ExtractFilePath(XApp.XFile().GetFileName()) + "autochem.stop");
        if( TEFile::Exists(sfn) )  {
          olx_sleep(100);
          TEFile::DelFile(sfn);
//          TOlex::~TOlex();
          TerminateSignal = true;
//          exit(0);
        }
      }
      TBasicApp::GetInstance().OnTimer->SetEnabled( false );
      // execute tasks ...
      // end tasks ...
      if( FProcess != NULL )  {
        while( FProcess->StrCount() != 0 )  {
          conint.SetTextForeground(fgcGreen, false);
          TBasicApp::GetLog() << FProcess->GetString(0) << '\n';
          conint.SetTextForeground(fgcReset, false);  // was setting the first read value!
          CallbackFunc(ProcessOutputCBName, FProcess->GetString(0));
          FProcess->DeleteStr(0);
        }
      }
      //    if( (FMode & mListen) != 0 )  {
      TBasicApp::GetInstance().OnTimer->SetEnabled( true );
    }
    else if( MsgId == ID_PROCESSTERMINATE )  SetProcess(NULL);
    else if( MsgId == ID_INFO || MsgId == ID_WARNING || MsgId == ID_ERROR || MsgId == ID_EXCEPTION )  {
      if( MsgSubId == msiEnter )  {
        if( Data != NULL )  {
          if( MsgId == ID_INFO )           
            conint.SetTextForeground(fgcBlue, true);
          else if( MsgId == ID_WARNING )   
            conint.SetTextForeground(fgcRed, false);
          else if( MsgId == ID_ERROR || MsgId == ID_EXCEPTION )     
            conint.SetTextForeground(fgcRed, true);
          OutStream->SetSkipPost(Silent && MsgId == ID_INFO);
          res = false;  // propargate to other streams, logs in particular if not Silent
        }
      }
      else  if( MsgSubId == msiExit )  {
        conint.SetTextForeground(fgcReset);
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
  void macQuit(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
    TerminateSignal = true;
  }
  //..............................................................................
  void macSilent(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
    if( Cmds[0].Equalsi("on") )
      Silent = true;
    else if( Cmds[0].Equalsi("off") )
      Silent = false;
    else
      Silent = Cmds[0].ToBool();
  }
  //..............................................................................
  void funSel(const TStrObjList& Params, TMacroError &E) {
    olxstr rv;
    for( int i=0; i < Selection.Count(); i++ )
      rv << Selection[i]->GetLabel() << ' ';
    E.SetRetVal( rv );
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
      if( Cmds.Count() > 1 && Cmds[0].Equalsi("satoms") )  {
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
  void macStop(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
    return;
  }
  //..............................................................................
  void macWaitFor(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
    if( Cmds[0].Equalsi("process") )  {
      while( FProcess != NULL )  {
        olx_sleep(50);
      }
    }
  }
  //..............................................................................
  void macDelIns(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
    TIns& Ins = XApp.XFile().GetLastLoader<TIns>();
    if( Cmds[0].IsNumber() )  {
      int insIndex = Cmds[0].ToInt();
      Ins.DelIns(insIndex);
      return;
    }
    for( int i=0; i < Ins.InsCount(); i++ )  {
      if(  Ins.InsName(i).Equalsi(Cmds[0]) )  {
        Ins.DelIns(i--);  
        continue;
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
      TSpaceGroup* sg = XApp.XFile().GetLastLoader<TCRSFile>().GetSG();
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
    if( Ins->GetSfac().IsEmpty() )  {
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
      olxstr titl( TEFile::ChangeFileExt(TEFile::ExtractFileName(XApp.XFile().GetFileName()), EmptyString) );
      Ins->SetTitle( titl << " in " << sg->GetName() << " #" << sg->GetNumber());
    }
    if( fileName.IsEmpty() )
      fileName = XApp.XFile().GetFileName();
    olxstr FN( TEFile::ChangeFileExt(fileName, "ins") );
    olxstr lstFN( TEFile::ChangeFileExt(fileName, "lst") );

    Ins->SaveToRefine(FN, Cmds.Text(' '), newSg);
    if( TEFile::Exists(lstFN) )  {
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
    else if( Cmds[0].Equalsi("rint") )
      E.SetRetVal( Lst.Rint() );
    else if( Cmds[0].Equalsi("rsig") )
      E.SetRetVal( Lst.Rsigma() );
    else if( Cmds[0].Equalsi("r1") )
      E.SetRetVal( Lst.R1() );
    else if( Cmds[0].Equalsi("r1a") )
      E.SetRetVal( Lst.R1a() );
    else if( Cmds[0].Equalsi("wr2") )
      E.SetRetVal( Lst.wR2() );
    else if( Cmds[0].Equalsi("s") )
      E.SetRetVal( Lst.S() );
    else if( Cmds[0].Equalsi("rs") )
      E.SetRetVal( Lst.RS() );
    else if( Cmds[0].Equalsi("params") )
      E.SetRetVal( Lst.Params() );
    else if( Cmds[0].Equalsi("rtotal") )
      E.SetRetVal( Lst.TotalRefs() );
    else if( Cmds[0].Equalsi("runiq") )
      E.SetRetVal( Lst.UniqRefs() );
    else if( Cmds[0].Equalsi("r4sig") )
      E.SetRetVal( Lst.Refs4sig() );
    else if( Cmds[0].Equalsi("peak") )
      E.SetRetVal( Lst.Peak() );
    else if( Cmds[0].Equalsi("hole") )
      E.SetRetVal( Lst.Hole() );
    else if( Cmds[0].Equalsi("flack") )  {
      if( Lst.HasFlack() )
        E.SetRetVal( Lst.Flack().ToString() );
      else
        E.SetRetVal( NAString );
    }
    else
      E.SetRetVal<olxstr>( NAString );
  }
  //..............................................................................
  void macIF(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
    if( Cmds.Count() < 2 || !Cmds[1].Equalsi("then"))  {
      E.ProcessingError(__OlxSrcInfo, "incorrect syntax - two commands or a command and \'then\' are expected" );
      return;
    }
    olxstr Condition = Cmds[0];
    if( !Macros.ProcessMacroFunc(Condition, E) )  {
      return;
    }
    if( Condition.ToBool() )  {
      if( !Cmds[2].Equalsi("none") )
        Macros.ProcessMacro(Cmds[2], E);
      return;
    }
    else  {
      if( Cmds.Count() == 5 )  {
        if( Cmds[3].Equalsi("else") )  {
          if( !Cmds[4].Equalsi("none") )
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
        Cmd = FProcess->OnTerminateCmds()[0];
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
    bool Asyn = !Options.Contains('s'), // synchroniusly
      Cout = !Options.Contains('o'),    // catch output
      quite = Options.Contains('q');

    olxstr dubFile( Options.FindValue('s',EmptyString) );

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
#ifdef __WIN32__
    TWinProcess* Process  = new TWinProcess;
#else
    TWxProcess* Process = new TWxProcess;
#endif		
    Process->OnTerminateCmds().Assign( FOnTerminateMacroCmds );
    FOnTerminateMacroCmds.Clear();
    if( (Cout && Asyn) || Asyn )  {  // the only combination
      if( !Cout )  {
        SetProcess(Process);
        if( !Process->Execute(Tmp, quite ? spfQuite : 0) )  {
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
        if( !Process->Execute(Tmp, quite ? spfRedirected|spfQuite : spfRedirected) )  {
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
    if( Cmds[0].Equalsi("macro") )  {
      olxstr macroFile( XApp.GetBaseDir() + "macrox.xld" );
      if( TEFile::Exists(macroFile) )  {
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
    if( !TEFile::Exists(ofn) )  {
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
    if( Params.IsEmpty() )
      E.SetRetVal( olxstr(__DATE__) << ' ' << __TIME__ );
    else  {
      time_t date, time;
      try {  
        date = TETime::ParseDate( __DATE__ );
        time = TETime::ParseTime( __TIME__ );
        E.SetRetVal<olxstr>( TETime::FormatDateTime(Params[0], date+time) );
      }
      catch( TExceptionBase& ) {
        E.SetRetVal( olxstr(__DATE__) << ' ' << __TIME__ );
      }
    }
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
    XApp.XFile().LoadFromFile( Cmds.Text(' ') );
    olxstr lstfn( TEFile::ChangeFileExt(Cmds[0], "lst") );
    if( TEFile::Exists(lstfn) )
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
    TSAtomPList satoms;
    LocateAtoms(Cmds, satoms, true);
    TListCaster::POP(satoms, atoms);
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
  static bool TerminateSignal;
  static TOlex* OlexInstance;
};
bool TOlex::TerminateSignal = false;
TOlex* TOlex::OlexInstance;
////////////////////////////////////////////////////////////////////////////////////////

class TTerminationListener : public AActionHandler {
public:
  virtual bool Execute(const IEObject *Sender, const IEObject *Data=NULL)  {  
    if( TOlex::TerminateSignal )  {
      TBasicApp::GetLog() << "terminate\n";
      exit(0);
      //DWORD w=0;
      //WriteConsole(TOlex::OlexInstance->GetConin(),
      //  olx_T("\n"), 1, &w, NULL); 
    }
    return false; 
  }
};
#ifndef __WIN32__  // dummy stuff for wxWidgets...
class MyApp: public wxAppConsole {
  virtual bool OnInit() {  return true;  }
	virtual int OnRun() {  return 0;  }
};
IMPLEMENT_APP_NO_MAIN(MyApp)
#endif
int main(int argc, char* argv[])  {
  TEGC::Initialise();
#ifndef __WIN32__  // dummy stuff for wxWidgets...
  MyApp wx_app;
  wxAppConsole::SetInstance(&wx_app);
	rl_readline_name = argv[0];
//  struct termios new_settings, stored_settings;
//  tcgetattr(0,&stored_settings);
//  new_settings = stored_settings;
//  new_settings.c_lflag &= (~(ICANON|ECHO));
//  new_settings.c_cc[VTIME] = 0;
//  tcgetattr(0,&stored_settings);
//  new_settings.c_cc[VMIN] = 1;
//  tcsetattr(0,TCSANOW,&new_settings);
#endif
  
  olxstr bd( TBasicApp::GuessBaseDir(argv[0], "OLEX2_DIR"));
  TOlex olex(bd);
#ifdef __WIN32__
  SetConsoleTitle(olx_T("Olex2 Console"));
#endif	
  TLibrary &Library = olex.GetLibrary();
  cout << "Welcome to Olex2 console\n";
  cout << "GUI basedir is: " << TBasicApp::GetBaseDir().c_str() << '\n';
  cout << "Compilation information: " << __DATE__ << ' ' << __TIME__ << '\n';
#ifdef __WIN32__  // readline prints one itself...
  cout << ">>";
#endif
  if( argc > 1 )  {
    olxstr arg_ext( TEFile::ExtractFileExt(argv[1]) );
    if( arg_ext.Equalsi("autochem") )  {
      olxstr sf ( TEFile::ChangeFileExt(argv[1], EmptyString) );
      olex.executeMacro( olxstr("start_autochem '") << sf << '\'' );
    }
  }
  else  {
#ifdef __WIN32__
    char _cmd[512];
    _cmd[0] = '\0';
#endif		
    olxstr cmd;
    //TBasicApp::GetInstance().OnTimer->Add( new TTerminationListener );
    while( true )  {
      TBasicApp::GetInstance().OnIdle->Execute(NULL);
#ifdef __WIN32__
      cin.getline(_cmd, 512);
      cmd = _cmd;
#else
  char* _cmd = readline(">>");
	if( _cmd == NULL )
	  continue;
	add_history(_cmd);
	cmd = _cmd;
	delete _cmd;
#endif
//      int ch = getchar();
//      if( ch >= 'a' && ch <= 'z' )
//        cout << (char)ch;
//      else if( ch == 27 )  {
//        ch = getchar();
//        if( ch == '[' )  {
//          ch = getchar();  
//          if( ch == 'A' )
//            TBasicApp::GetLog() << "up\n";
//          if( ch == 'B' )
//            TBasicApp::GetLog() << "down\n";
//          if( ch == 'C' )  {
//            TBasicApp::GetLog() << "right\n";
//          }
//          if( ch == 'D' )  {
//            TBasicApp::GetLog() << "left\n";
//            cout << "\r";
//            continue;
//          }
//        }
//      }
      if( cmd.Equalsi("quit") )  break;
      else  {
        try { olex.executeMacro(cmd);  }
        catch( TExceptionBase& exc )  {
          TBasicApp::GetLog() << exc.GetException()->GetError();
        }
      }
      if( olex.TerminateSignal )  break;
#ifdef __WIN32__			
      cout << ">>";
#endif			
    }
  }
  TBasicApp::GetInstance().OnIdle->Execute(NULL);
  return 0;
}

