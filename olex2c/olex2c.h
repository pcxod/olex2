/******************************************************************************
* Copyright (c) 2004-2019 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* OlexSys Proprietary file                                                    *
******************************************************************************/
#ifndef __olexsys_olex2c_h
#define __olexsys_olex2c_h
#include "xapp.h"
#include "ipimp.h"
#include "olex2app.h"
#include "con_in.h"
#include "outstream.h"
#include "eprocess.h"

enum  {
  ID_TIMER = 1,
  ID_INFO,
  ID_ERROR,
  ID_WARNING,
  ID_EXCEPTION,
  ID_STRUCTURECHANGED,
  ID_LOG
};

// a scheduled macro
struct TScheduledTask {
  bool Repeatable;
  olxstr Task;
  long Interval, LastCalled;
};

class TOlex2c : public AEventsDispatcher, public olex2::OlexProcessorImp,
  public olex2::AOlex2App, public ASelectionOwner
{
  TXApp XApp;
  macrolib::TEMacroLib Macros;
  TTypeList<TScheduledTask> Tasks;
  TSAtomPList Selection;
  ConsoleInterface conint;
  bool Silent;
  TOutStream* OutStream;
#ifdef __WIN32__
#elif defined(__WXWIDGETS__)
  class TTimer : public wxTimer {
    void Notify() {
      TBasicApp::GetInstance().OnTimer.Execute(NULL);
    }
  public:
    TTimer() {}
  } timer;
#endif

  void UnifyAtomList(TSAtomPList atoms);
  // selection owner interface
  virtual void ExpandSelection(TCAtomGroup& atoms);
  virtual void ExpandSelectionEx(TSAtomPList& atoms);
  virtual ConstPtrList<TSObject<class TNetwork> > GetSelected();
  bool LocateAtoms(TStrObjList &Cmds, TSAtomPList& atoms, bool all);
  class ProcessHandler : public ProcessManager::IProcessHandler  {
    TOlex2c& parent;
  public:
    ProcessHandler(TOlex2c& _parent) : parent(_parent)  {}
    virtual void BeforePrint()  {
      parent.conint.SetTextForeground(fgcGreen, false);
    }
    virtual void Print(const olxstr& line)  {
      static const olxstr ProcessOutputCBName("procout");
      TBasicApp::GetLog() << line;
      parent.callCallbackFunc(ProcessOutputCBName, TStrList() << line);
    }
    virtual void AfterPrint()  {
      parent.conint.SetTextForeground(fgcReset, false);  // was setting the first read value!
    }
    virtual void OnWait()  {
      TBasicApp::GetInstance().OnTimer.Execute(NULL);
    }
    virtual void OnTerminate(const AProcess& p)  {
      TMacroData err;
      for (size_t i = 0; i < p.OnTerminateCmds().Count(); i++) {
        const olxstr& cmd = p.OnTerminateCmds()[i];
        parent.Macros.ProcessMacro(cmd, err);
        if (!err.IsSuccessful())
          break;
      }
      TBasicApp::NewLogEntry(logInfo) << "The process '" << p.GetCmdLine() <<
        "' has been terminated...";
    }
  };
  ProcessHandler _ProcessHandler;
  ProcessManager* _ProcessManager;
  TPtrList<olex2::IOlex2Runnable> loadedDll;
public:
  TOlex2c(const olxstr& basedir);
  ~TOlex2c();
  virtual bool processMacro(const olxstr& cmdLine,
    const olxstr &location = EmptyString(), bool quiet = false);
  virtual bool processMacroEx(const olxstr& cmdLine,
    TMacroData& er, const olxstr &location = EmptyString(),
    bool quiet = false);
  //..........................................................................................
  bool Dispatch(int MsgId, short MsgSubId, const IOlxObject *Sender,
    const IOlxObject *Data, TActionQueue *);
  void macQuit(TStrObjList &Cmds, const TParamList &Options, TMacroData &Error);
  //..............................................................................
  void macShell(TStrObjList &Cmds, const TParamList &Options, TMacroData &Error);
  //..............................................................................
  void macStat(TStrObjList &Cmds, const TParamList &Options, TMacroData &Error);
  //..............................................................................
  void macSilent(TStrObjList &Cmds, const TParamList &Options, TMacroData &Error);
  //..............................................................................
  void macSchedule(TStrObjList &Cmds, const TParamList &Options,
    TMacroData &Error);
  //..............................................................................
  void funSel(const TStrObjList& Params, TMacroData &E);
  //..............................................................................
  void macSel(TStrObjList &Cmds, const TParamList &Options, TMacroData &E);
  //..............................................................................
  void macStop(TStrObjList &Cmds, const TParamList &Options, TMacroData &E);
  //..............................................................................
  void macWaitFor(TStrObjList &Cmds, const TParamList &Options, TMacroData &E);
  //..............................................................................
  void macWait(TStrObjList &Cmds, const TParamList &Options, TMacroData &E);
  //..............................................................................
  void macDelIns(TStrObjList &Cmds, const TParamList &Options, TMacroData &E);
  //..............................................................................
  void macExec(TStrObjList &Cmds, const TParamList &Options, TMacroData &Error);
  //..............................................................................
  void macPython(TStrObjList &Cmds, const TParamList &Options, TMacroData &E);
  //..............................................................................
  void macReload(TStrObjList &Cmds, const TParamList &Options, TMacroData &Error);
  //..............................................................................
  void macClear(TStrObjList &Cmds, const TParamList &Options, TMacroData &Error);
  //..............................................................................
  void macEcho(TStrObjList &Cmds, const TParamList &Options, TMacroData &Error);
  //..............................................................................
  void funIsPluginInstalled(const TStrObjList& Params, TMacroData &E);
  //..............................................................................
  void funCurrentLanguageEncoding(const TStrObjList& Params, TMacroData &E);
  //..............................................................................
  void funHasGUI(const TStrObjList& Params, TMacroData &E);
  //..............................................................................
  void funStrCmp(const TStrObjList& Params, TMacroData &E);
  //..............................................................................
  void funUnsetVar(const TStrObjList& Params, TMacroData &E);
  //..............................................................................
  void funSetVar(const TStrObjList& Params, TMacroData &E);
  //..............................................................................
  void funGetVar(const TStrObjList& Params, TMacroData &E);
  //..............................................................................
  void funIsVar(const TStrObjList& Params, TMacroData &E);
  //..............................................................................
  void funGetEnv(const TStrObjList& Params, TMacroData &E);
  //..............................................................................
  void macReap(TStrObjList &Cmds, const TParamList &Options, TMacroData &Error);
  //..............................................................................
  void funUser(const TStrObjList &Cmds, TMacroData &Error);
  //..............................................................................
  void macInfo(TStrObjList &Cmds, const TParamList &Options, TMacroData &Error);
  //..............................................................................
  void macName(TStrObjList &Cmds, const TParamList &Options, TMacroData &Error);
  //..............................................................................
  void macKill(TStrObjList &Cmds, const TParamList &Options, TMacroData &Error);
  //..............................................................................
  void funGetMAC(const TStrObjList& Params, TMacroData &E);
  //..............................................................................
  void funExtractCell(const TStrObjList& Params, TMacroData &E);
  //..............................................................................
  void funReduceCell(const TStrObjList& Params, TMacroData &E);
  //..............................................................................
  void funImportMetadata(const TStrObjList& Params, TMacroData &E);
  //..............................................................................
  void funSuggestSGAndContent(const TStrObjList& Params, TMacroData &E);
  //..............................................................................
  void macUpdateModel(TStrObjList &Cmds, const TParamList &Options, TMacroData &E);
  //..............................................................................
  void macUpdateJSON(TStrObjList &Cmds, const TParamList &Options, TMacroData &E);
  //..............................................................................
  void funDeposit(const TStrObjList &Params, TMacroData &E);
  //..............................................................................
  void funLoadDll(const TStrObjList &Cmds, TMacroData &E);
  //..............................................................................
  void funTranslatePhrase(const TStrObjList& Params, TMacroData &E);
  //..............................................................................
  void funIsCurrentLanguage(const TStrObjList& Params, TMacroData &E);
    /////////////////////////////////////////////////////////////////////////////
  static bool TerminateSignal;

  static TOlex2c& GetInstance() {
    AOlex2App *inst = AOlex2App::GetInstance_();
    TOlex2c *i = dynamic_cast<TOlex2c *>(inst);
    if (i == 0) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "invalid instance");
    }
    return *i;
  }
};

#endif
