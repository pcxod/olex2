/******************************************************************************
* Copyright (c) 2004-2019 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* OlexSys Proprietary file                                                    *
******************************************************************************/
#include "olex2c.h"
#include "ebase.h"
#if !defined(__WIN32__) && defined(__WXWIDGETS__)
#include "wx/wx.h"
#include "wx/app.h"
#include "wx/dynlib.h"
#endif

#include "xmacro.h"
#include "log.h"
#include "pyext.h"
#include "fsext.h"
#include "httpfs.h"
#include "olxvar.h"
#include "shellutil.h"
#include "dataitem.h"
#include "datafile.h"
#include "eprocess.h"
#include "egc.h"
#include "file_filter.h"
#include "olxmps.h"
#include "filetree.h"
//
#include "ins.h"
#include "mol.h"
#include "cif.h"
#include "xyz.h"
#include "p4p.h"
#include "crs.h"
#include "pdb.h"
#include "unitcell.h"
#include "xdmas.h"
#include "macrolib.h"
#include "symmlib.h"
#include "xlcongen.h"
#include "seval.h"
#include "utf8file.h"
#include "settingsfile.h"
#include "auto.h"
#ifdef _PYTHON
#include "py_core.h"
#include "hkl_py.h"
#endif
#include "olxth.h"
#include "egc.h"
#include "patchapi.h"
#include "libfile.h"
#include "ipimp.h"
#include "cell_reduction.h"
#include "analysis.h"
#include "hkl.h"
#ifdef _CUSTOM_BUILD_
#include "custom_base.h"
#endif

#ifndef __WIN32__
#include <readline/readline.h>
#include <readline/history.h>
#endif

#define this_InitMacroD(macroName, validOptions, argc, desc)\
  Library.Register(new TMacro<TOlex2c>(this, &TOlex2c::mac##macroName, #macroName, (validOptions), argc, desc))
#define this_InitMacroDA(macroName, macroNameA, validOptions, argc, desc)\
  Library.Register(new TMacro<TOlex2c>(this, &TOlex2c::mac##macroName, #macroNameA, (validOptions), argc, desc))
#define this_InitFuncD(funcName, argc, desc)\
  Library.Register(new TFunction<TOlex2c>(this, &TOlex2c::fun##funcName, #funcName, argc, desc))

const olxstr OnLogCBName("onlog");

int main_thread_id;

#ifdef __WIN32__
UINT_PTR timer_id;
VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
  if (TBasicApp::HasInstance()) {
    TBasicApp::GetInstance().OnTimer.Execute(NULL);
  }
}
#endif

void TOlex2c::UnifyAtomList(TSAtomPList atoms) {
  // unify the selection
  atoms.ForEach(ACollectionItem::IndexTagSetter());
  for (size_t i = 0; i < atoms.Count(); i++) {
    if (atoms[i]->CAtom().GetTag() != i || atoms[i]->CAtom().IsDeleted()) {
      atoms[i] = 0;
    }
  }
  atoms.Pack();
}

void TOlex2c::ExpandSelection(TCAtomGroup& atoms) {
  atoms.SetCapacity(atoms.Count() + Selection.Count());
  for (size_t i = 0; i < Selection.Count(); i++) {
    atoms.AddNew(&Selection[i]->CAtom(), &Selection[i]->GetMatrix());
  }
  if (GetDoClearSelection()) {
    Selection.Clear();
  }
}

void TOlex2c::ExpandSelectionEx(TSAtomPList& atoms) {
  atoms.AddAll(Selection);
  if (GetDoClearSelection()) {
    Selection.Clear();
  }
}

ConstPtrList<TSObject<class TNetwork> > TOlex2c::GetSelected() {
  SObjectPtrList rv;
  return rv;
}

bool TOlex2c::LocateAtoms(TStrObjList &Cmds, TSAtomPList& atoms, bool all) {
  atoms = XApp.FindSAtoms(Cmds, all);
  UnifyAtomList(atoms);
  return !atoms.IsEmpty();
}

TOlex2c::TOlex2c(const olxstr& basedir)
  : OlexProcessorImp(NULL),
  XApp(basedir, NULL, this),
  Macros(*this),
  _ProcessHandler(*this)
{
  TStopWatch sw(__FUNC__);
#ifdef _CUSTOM_BUILD_
  if (!CustomCodeBase::OnStartup())
#endif
  {
    XApp.SetSharedDir(patcher::PatchAPI::GetSharedDir());
    XApp.SetInstanceDir(patcher::PatchAPI::GetInstanceDir());
  }
  XApp.ReadOptions(XApp.GetConfigDir() + ".options");
  XApp.CleanupLogs();
  XApp.CreateLogFile(XApp.GetOptions().FindValue("log_name", "olex2c"));
  olex2::OlexProcessorImp::SetLibraryContainer(XApp);
  AOlex2App::InitOlex2App();
  Macros.Init();
  XApp.SetCifTemplatesDir(XApp.GetBaseDir() + "etc/CIF/");
  Silent = true;
  OutStream = new TOutStream();
  XApp.GetLog().AddStream(OutStream, false);
  XApp.GetLog().OnInfo.Add(this, ID_INFO);
  XApp.GetLog().OnWarning.Add(this, ID_WARNING);
  XApp.GetLog().OnError.Add(this, ID_ERROR);
  XApp.GetLog().OnException.Add(this, ID_EXCEPTION);
  XApp.GetLog().OnPost.Add(this, ID_LOG, msiExecute);

  TLibrary &Library = XApp.GetLibrary();
#ifdef _PYTHON
  PythonExt::Init(this).Register(
    OlexPyCore::ModuleName(), &OlexPyCore::PyInit);
  PythonExt::GetInstance()->Register(
    hkl_py::ModuleName(), &hkl_py::PyInit);
  Library.AttachLibrary(PythonExt::GetInstance()->ExportLibrary());
#endif
  Library.AttachLibrary(LibFile::ExportLibrary());
  Library.AttachLibrary(TETime::ExportLibrary());
  Library.AttachLibrary(XApp.ExportLibrary());
  Library.AttachLibrary(XApp.XFile().ExportLibrary());
  Library.AttachLibrary(TFileHandlerManager::ExportLibrary());
  Library.AttachLibrary(TAutoDB::GetInstance(false).ExportLibrary());

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
  XApp.XFile().GetLattice().OnStructureUniq.Add(this, ID_STRUCTURECHANGED);
  XApp.XFile().GetLattice().OnStructureGrow.Add(this, ID_STRUCTURECHANGED);
  XApp.XFile().OnFileLoad.Add(this, ID_STRUCTURECHANGED);

  this_InitMacroD(Silent, "", fpOne, "Changes silent mode");
  this_InitMacroD(Exec, "s&;o&;d&;q", fpAny^fpNone, "exec");
  this_InitMacroD(Echo, "", fpAny, "echo");
  this_InitMacroDA(Reap, @reap, "", fpAny, "reap");
  this_InitMacroD(Name, "", fpAny ^ (fpNone) | psFileLoaded, "name");
  this_InitMacroD(Info, "", fpAny, "info");
  this_InitMacroDA(Python, @py, "", fpAny^fpNone, "Runs python script");
  this_InitMacroD(Clear, "", fpNone, "");
  this_InitMacroD(Stop, "", fpOne, "");
  this_InitMacroD(Reload, "", fpOne, "");
  this_InitMacroD(WaitFor, "", fpOne, "");
  this_InitMacroD(Wait, "", fpOne, "");
  this_InitMacroD(Kill, "au-dummy option here",
    fpAny | psFileLoaded, "");
  this_InitMacroD(Sel, "i&;a&;u", fpAny | psFileLoaded, "");
  this_InitMacroD(Quit, "", fpNone, "");
  this_InitMacroD(Shell, "", fpAny, "");
  this_InitMacroD(Stat, "s-self test", fpOne, "");
  this_InitMacroD(Schedule,
    "r-repeatable&;"
    "g-only if have GUI"
    ,
    fpAny ^ (fpNone | fpOne),
    "Schedules a particular macro (second argument) to be executed within "
    "provided interval (first argument)");

  this_InitFuncD(User, fpNone | fpOne, "reap");
  this_InitFuncD(SetVar, fpTwo, "setvar");
  this_InitFuncD(UnsetVar, fpOne, "");
  this_InitFuncD(GetVar, fpOne | fpTwo, "");
  this_InitFuncD(IsVar, fpOne, "");
  this_InitFuncD(GetEnv, fpNone|fpOne, "");
  this_InitFuncD(IsPluginInstalled, fpOne, "");
  this_InitFuncD(StrCmp, fpTwo, "");
  this_InitFuncD(HasGUI, fpNone, "");
  this_InitFuncD(Sel, fpNone, "");
  this_InitFuncD(GetMAC, fpNone | fpOne, "");
  this_InitFuncD(LoadDll, fpOne, "Loads given DLL. The Dll must have "
    "olex2::IOlex2Runnable* GetOlex2Runnable symbol");

  this_InitFuncD(ExtractCell, fpOne,
    "Extracts and reduces cell from a given file");
  this_InitFuncD(ReduceCell, fpSeven, "Reduces the cell given by symbolic "
    "centering and the 6 parameters for side lengths and angles");
  this_InitFuncD(ImportMetadata, fpOne,
    "Reads a CIF and returns a \\n separated list of the values. If a value is"
    " multiline - it will have lines separated by \\r.");
  this_InitFuncD(SuggestSGAndContent, fpNone,
    "Returns a list of proposed space groups for the given file");
  this_InitFuncD(Deposit, fpOne | fpTwo | fpThree,
    "Helpf with depositing file(s) into the DB");

  this_InitMacroD(UpdateModel, "", fpThree, "Updates the refinement model and "
    "creates json file");
  this_InitMacroD(UpdateJSON, "", fpOne, "Updates the JSON for the given file"
    ". Stateless.");

  this_InitFuncD(TranslatePhrase, fpOne, "");
  this_InitFuncD(CurrentLanguageEncoding, fpNone, "");
  this_InitFuncD(IsCurrentLanguage, fpOne, "");
#ifdef _CUSTOM_BUILD_
  CustomCodeBase::Initialise(Library);
#endif

  olxstr macroFile = XApp.GetBaseDir() + "macrox.xld";
  if (TEFile::Exists(macroFile)) {
    TDataFile df;
    df.LoadFromXLFile(macroFile);
    df.Include(0);
    TDataItem* di = df.Root().FindItem("xl_macro");
    if (di != 0) {
      Macros.Load(*di);
    }
  }
  processMacro("onstartup");
  TBasicApp::GetInstance().OnTimer.Add(this, ID_TIMER);
  _ProcessManager = new ProcessManager(_ProcessHandler);
#ifdef __WIN32__
  timer_id = SetTimer(NULL, NULL, 50, &TimerProc);
  main_thread_id = GetCurrentThreadId();
#elif defined(__WXWIDGETS__)
  timer.Start(50);
#endif
}

TOlex2c::~TOlex2c() {
  volatile olx_scope_cs cs(TBasicApp::GetCriticalSection());
  olx_object_ptr<TStopWatch> sw(new TStopWatch(__FUNC__));
#ifdef __WIN32__
  KillTimer(NULL, timer_id);
#elif defined(__WXWIDGETS__)
  timer.Stop();
#endif
  TBasicApp::GetInstance().OnTimer.Clear();
  processMacro("onexit");
  {
    for (size_t i = 0; i < loadedDll.Count(); i++) {
      loadedDll[i]->Finalise();
    }
  }
  delete _ProcessManager;
  TOlxVars::Finalise();
#ifdef _CUSTOM_BUILD_
  CustomCodeBase::Finalise();
#endif
#ifdef _PYTHON
  PythonExt::Finilise();
#endif
  delete sw.release();
  delete OutStream;
  GetInstance_() = NULL;
}

bool TOlex2c::processMacro(const olxstr& cmdLine, const olxstr &location,
  bool quiet)
{
  TMacroData e;
  Macros.ProcessMacro(cmdLine, e);
  if (!quiet) AnalyseError(e);
  return e.IsSuccessful();
}

bool TOlex2c::processMacroEx(const olxstr& cmdLine,
  TMacroData& er, const olxstr &location, bool quiet)
{
  Macros.ProcessMacro(cmdLine, er);
  AnalyseError(er);
  return er.IsSuccessful();
}
//.............................................................................
bool TOlex2c::Dispatch(int MsgId, short MsgSubId, const IOlxObject *Sender,
  const IOlxObject *Data, TActionQueue *)
{
  bool res = true;
  if (MsgId == ID_TIMER) {
    if (XApp.XFile().HasLastLoader()) {
      olxstr sfn = TEFile::ExtractFilePath(XApp.XFile().GetFileName()) <<
        "autochem.stop";
      if (TEFile::Exists(sfn)) {
        olx_sleep(100);
        volatile olx_scope_cs cs(TBasicApp::GetCriticalSection());
        TEFile::DelFile(sfn);
        TerminateSignal = true;
      }
    }
    //tasks ...
    for (size_t i = 0; i < Tasks.Count(); i++) {
      if ((TETime::Now() - Tasks[i].LastCalled) > Tasks[i].Interval) {
        olxstr tmp = Tasks[i].Task;
        if (!Tasks[i].Repeatable) {
          Tasks.Delete(i--);
        }
        else
          Tasks[i].LastCalled = TETime::Now();
        processMacro(tmp, "Scheduled task");
      }
    }
  }
  else if (MsgId == ID_LOG && (MsgSubId == msiExecute)) {
    if (Data != NULL)
      callCallbackFunc(OnLogCBName, TStrList() << Data->ToString());
  }
  else if (MsgId == ID_INFO || MsgId == ID_WARNING || MsgId == ID_ERROR ||
    MsgId == ID_EXCEPTION)
  {
    if (MsgSubId == msiEnter) {
      if (Data != NULL) {
        if (MsgId == ID_INFO)
          conint.SetTextForeground(fgcBlue, true);
        else if (MsgId == ID_WARNING)
          conint.SetTextForeground(fgcRed, false);
        else if (MsgId == ID_ERROR || MsgId == ID_EXCEPTION)
          conint.SetTextForeground(fgcRed, true);
        OutStream->SetSkipPost(Silent && MsgId == ID_INFO);
        res = false;  // propargate to other streams, logs in particular if not Silent
      }
    }
    else if (MsgSubId == msiExit) {
      conint.SetTextForeground(fgcReset);
    }
  }
  else if (MsgId == ID_STRUCTURECHANGED)
    Selection.Clear();
  return res;
}
///////////////////////////////////////////////////////////////////////////////
//.............................................................................
void TOlex2c::macQuit(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  volatile olx_scope_cs cs(TBasicApp::GetCriticalSection());
  TerminateSignal = true;
}
//.............................................................................
void TOlex2c::macShell(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  olxstr cmd = Cmds.Text(' ');
#ifdef __WIN32__
  ShellExecute(NULL, olxT("open"), cmd.u_str(), NULL,
    TEFile::CurrentDir().u_str(), SW_SHOWNORMAL);
#else
  if (cmd.StartsFrom("http") || cmd.StartsFrom("https") || cmd.EndsWith(".htm")
    || cmd.EndsWith(".html") || cmd.EndsWith(".php") || cmd.EndsWith(".asp"))
  {
    Macros.ProcessMacro(olxstr("exec -o getvar(defbrowser) '") << cmd << '\'', Error);
  }
# ifdef __linux__
  else if (cmd.EndsWith(".pdf")) {
    olxstr dskpAttr = olx_getenv("DESKTOP_SESSION");
    if (dskpAttr.Contains("gnome")) {
      Macros.ProcessMacro(
        olxstr("exec -o gnome-open '") << cmd << '\'', Error);
    }
    else if (dskpAttr.Contains("kde")) {
      Macros.ProcessMacro(
        olxstr("exec -o konqueror '") << cmd << '\'', Error);
    }
    else if (dskpAttr.Contains("xfce")) {
      Macros.ProcessMacro(
        olxstr("exec -o thunar '") << cmd << '\'', Error);
    }
    else {
      Macros.ProcessMacro(
        olxstr("exec -o getvar(defbrowser) '") << cmd << '\'', Error);
    }
  }
# endif
  else {
    Macros.ProcessMacro(
      olxstr("exec -o getvar(defexplorer) '") << cmd << '\'', Error);
  }
#endif
}
//.............................................................................
//.............................................................................
//.............................................................................
struct stri {
  TArrayList<uint8_t> data;
  olxstr id;
  stri(const olxstr &id_ = EmptyString())
    : data(100, olx_list_init::zero()),
    id(id_)
  {}
  double match(const stri &s) {
    double diff = 0;
    for (size_t i = 0; i < data.Count(); i++) {
      diff += (double)olx_abs(data[i] - s.data[i]);
    }
    return diff / 255;  // 0..100
  }
  void ToStream(IDataOutputStream &out) const {
    out << id;
    out << (uint8_t)data.Count();
    out.Write(data.GetData(), data.Count()*sizeof(uint8_t));
  }
  void FromStream(IDataInputStream &in) {
    in >> id;
    uint8_t sz;
    in >> sz;
    data.SetCount(sz);
    for (uint8_t i = 0; i < sz; i++) {
      in >> data[i];
    }
  }
};
static void CreateStri(TUnitCell &uc, TArrayList<double> &strd, stri &sd) {
  strd.ForEach(olx_list_init::zero());
  TAsymmUnit &au = uc.GetLattice().GetAsymmUnit();
  double maxv = 0;
  for (size_t j = 0; j < au.AtomCount(); j++) {
    if (au.GetAtom(j).GetType() < 1) continue;
    TArrayList<AnAssociation3<TCAtom*, smatd, vec3d> > res;
    uc.FindInRangeAMC(au.GetAtom(j).ccrd(), 0,
      5, res);
    vec3d f = au.Orthogonalise(au.GetAtom(j).ccrd());
    for (size_t k = 0; k < res.Count(); k++) {
      if (!TNetwork::IsBondAllowed(au.GetAtom(j), *res[k].GetA(), res[k].GetB()))
        continue;
      double d = f.DistanceTo(res[k].GetC());
      int r = (int)olx_round(d * 20);
      if (r < 100 && r > 0) {
        strd[r] += res[k].GetA()->GetChemOccu()*res[k].GetA()->GetType().z;
        if (strd[r] > maxv)
          maxv = strd[r];
      }
    }
  }
  if (maxv > 0) {
    for (size_t j = 0; j < strd.Count(); j++) {
      size_t s = olx_round(255 * strd[j] / maxv);
      if (s > 255) {
        s = 255;
      }
      sd.data[j] = s;
    }
  }
}
class StriCtask : public TaskBase {
  TStrList &files;
  TTypeList<stri> &data;
  TArrayList<double> strd;
  TXFile xf;
public:
  StriCtask(TStrList &files, TTypeList<stri> &data)
    : files(files), data(data), strd(100),
    xf(*(new SObjectProvider()))
  {
    xf.RegisterFileFormat(new TCif, "cif");
  }
  void Run(size_t i) {
    try {
      if (files[i].Contains(".olex"))
        return;
      xf.LoadFromFile(files[i]);
      if (xf.GetAsymmUnit().AtomCount() == 0)
        return;
    }
    catch (const TExceptionBase &e) {
      e.GetException()->PrintStackTrace();
      return;
    }
    olx_critical_section *cs = GetCriticalSection();
    stri *d = new stri(files[i]);
    CreateStri(xf.GetUnitCell(), strd, *d);
    if (cs) cs->enter();
    TBasicApp::NewLogEntry() << files[i];
    data.Add(d);
    if (cs) cs->leave();
  }
  StriCtask *Replicate() {
    return new StriCtask(files, data);
  }
};
void TOlex2c::macStat(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  olxstr db_file_name("c:/tmp/pdb.db");
  TTypeList<stri> data;
  if (TEFile::Exists(db_file_name)) {
    TEFile dbf(db_file_name, "rb");
    uint32_t db_sz;
    dbf >> db_sz;
    data.SetCapacity(db_sz);
    for (size_t i = 0; i < db_sz; i++) {
      data.AddNew().FromStream(dbf);
    }
  }
  if (TEFile::IsDir(Cmds[0])) {
    TStrList files;
    TFileTree ft(Cmds[0]);
    ft.Expand(TFileTree::efReport);
    ft.GetRoot().ListFiles(files, "*.cif");
    data.SetCapacity(data.Count() + files.Count());
    StriCtask task(files, data);
    OlxListTask::Run(task, files.Count(), tLinearTask, 20);

    TEFile dbf(db_file_name, "w+b");
    dbf << (uint32_t)data.Count();
    for (size_t i = 0; i < data.Count(); i++)
      data[i].ToStream(dbf);
    dbf.Close();
  }
  if (Options.GetBoolOption('s')) {
    for (size_t i = 0; i < data.Count(); i++) {
      TPtrList<stri> matches;
      for (size_t j = i + 1; j < data.Count(); j++) {
        if (data[i].match(data[j]) < 0.5) {
          matches << data[j];
        }
      }
      if (!matches.IsEmpty()) {
        TBasicApp::NewLogEntry() << data[i].id;
        for (size_t j = 0; j < matches.Count(); j++)
          TBasicApp::NewLogEntry() << '\t' << matches[j]->id;
      }
    }
  }
  else if (TEFile::Exists(Cmds[0])) {
    TXApp::GetInstance().XFile().LoadFromFile(Cmds[0]);
  }
  TArrayList<double> strd(100);
  stri sd;
  TXFile &xf = TXApp::GetInstance().XFile();
  CreateStri(xf.GetUnitCell(), strd, sd);
  sorted::PrimitiveAssociation<double, stri*> hits;
  for (size_t i = 0; i < data.Count(); i++) {
    hits.Add(data[i].match(sd), &data[i]);
  }
  size_t hsz = olx_min(hits.Count(), 10);
  for (size_t i = 0; i < hsz; i++) {
    size_t idx = i;
    TBasicApp::NewLogEntry() << hits.GetKey(idx) << ": " <<
      hits.GetValue(idx)->id;
    idx = data.Count() - i - 1;
    if (idx < data.Count()) {
      TBasicApp::NewLogEntry() << hits.GetKey(idx) << ": " <<
        hits.GetValue(idx)->id;
    }
  }
}
//.............................................................................
void TOlex2c::macSilent(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  if (Cmds[0].Equalsi("on"))
    Silent = true;
  else if (Cmds[0].Equalsi("off"))
    Silent = false;
  else
    Silent = Cmds[0].ToBool();
}
//.............................................................................
void TOlex2c::macSchedule(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  if (!Cmds[0].IsNumber()) {
    Error.ProcessingError(__OlxSrcInfo,
      "invalid syntax: <interval 'task'> are expected");
    return;
  }
  if (Options.GetBoolOption('g')) {
    return;
  }
  TScheduledTask& task = Tasks.AddNew();
  task.Repeatable = Options.GetBoolOption('r');
  task.Interval = Cmds[0].ToInt();
  task.Task = Cmds[1];
  task.LastCalled = TETime::Now();
}
//.............................................................................
//.............................................................................
//.............................................................................
void TOlex2c::funSel(const TStrObjList& Params, TMacroData &E) {
  olxstr rv;
  for (size_t i = 0; i < Selection.Count(); i++) {
    rv << Selection[i]->GetLabel() << ' ';
  }
  E.SetRetVal(rv);
}
//.............................................................................
void TOlex2c::macSel(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  TLattice& latt = XApp.XFile().GetLattice();
  int flag = 0;
  if (Options.GetBoolOption('a')) {
    flag = 1;
  }
  else if (Options.GetBoolOption('u')) {
    flag = -1;
  }
  else if (Options.GetBoolOption('i')) {
    flag = 2;
  }
  if (Cmds.IsEmpty() || (Cmds.Count() == 1 && Cmds[0].Equalsi("sel"))) {
    if (flag == -1) {
      Selection.Clear();
    }
    else if (flag == 1) {
      Selection.Clear();
      Selection.SetCapacity(latt.GetObjects().atoms.Count());
      for (size_t i = 0; i < latt.GetObjects().atoms.Count(); i++) {
        if (!latt.GetObjects().atoms[i].IsDeleted()) {
          Selection.Add(latt.GetObjects().atoms[i]);
        }
      }
    }
    else if (flag == 2) {
      latt.GetObjects().atoms.ForEach(ACollectionItem::TagSetter(0));
      Selection.ForEach(ACollectionItem::TagSetter(1));
      Selection.Clear();
      for (size_t i = 0; i < latt.GetObjects().atoms.Count(); i++) {
        TSAtom& sa = latt.GetObjects().atoms[i];
        if (sa.GetTag() == 0 && !sa.IsDeleted()) {
          Selection.Add(sa);
        }
      }
    }
    return;
  }
  TSAtomPList atoms;
  if (Cmds.Count() > 1 && Cmds[0].Equalsi("atoms")) {
    size_t wi = Cmds.IndexOf("where");
    olxstr Where(Cmds.Text(' ', wi + 1).LowerCase());
    TSFactoryRegister rf;
    TTSAtom_EvaluatorFactory* satom =
      (TTSAtom_EvaluatorFactory*)rf.FindBinding("atom");
    TExpressionParser SyntaxParser(&rf, Where);
    if (SyntaxParser.Errors().Count() == 0) {
      for (size_t i = 0; i < latt.GetObjects().atoms.Count(); i++) {
        TSAtom& sa = latt.GetObjects().atoms[i];
        if (sa.IsDeleted()) {
          continue;
        }
        satom->provider->SetTSAtom(&sa);
        if (SyntaxParser.Evaluate()) {
          atoms.Add(sa);
        }
      }
    }
    else {
      XApp.NewLogEntry(logError) << SyntaxParser.Errors().Text(NewLineSequence());
    }
  }
  if (Cmds.Count() > 1 && Cmds[0].Equalsi("part")) {
    olx_pset<int> parts;
    Cmds.Delete(0);
    for (size_t i = 0; i < Cmds.Count(); i++) {
      parts.Add(Cmds[i].ToInt());
    }
    for (size_t i = 0; i < latt.GetObjects().atoms.Count(); i++) {
      TSAtom& sa = latt.GetObjects().atoms[i];
      if (!sa.IsDeleted() && parts.Contains(sa.CAtom().GetPart())) {
        atoms.Add(sa);
      }
    }
  }
  else {
    atoms = XApp.FindSAtoms(Cmds);
  }
  // select extra
  if (flag == 1 || flag == 0) {
    latt.GetObjects().atoms.ForEach(ACollectionItem::TagSetter(0));
    atoms.ForEach(ACollectionItem::TagSetter(1));
    Selection.ForEach(ACollectionItem::TagSetter(0));
    for (size_t i = 0; i < atoms.Count(); i++) {
      if (atoms[i]->GetTag() != 0) {
        Selection.Add(atoms[i]);
      }
    }
  }
  else if (flag == 2) {
    Selection.ForEach(ACollectionItem::TagSetter(-1));
    atoms.ForEach(ACollectionItem::IndexTagSetter());
    for (size_t i = 0; i < Selection.Count(); i++) {
      if (Selection[i]->GetTag() >= 0) {
        atoms[Selection[i]->GetTag()] = 0;
        Selection[i] = 0;
      }
    }
    Selection.Pack();
    atoms.Pack();
    Selection.AddAll(atoms);
  }
  else if (flag == -1) {
    Selection.ForEach(ACollectionItem::TagSetter(0));
    atoms.ForEach(ACollectionItem::TagSetter(1));
    for (size_t i = 0; i < Selection.Count(); i++) {
      if (Selection[i]->GetTag() == 1) {
        Selection[i] = 0;
      }
    }
    Selection.Pack();
  }
  //UnifyAtomList(Selection);
}
//.............................................................................
void TOlex2c::macStop(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E) {
  return;
}
//.............................................................................
void TOlex2c::macWaitFor(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  if (Cmds[0].Equalsi("process"))
    _ProcessManager->WaitForLast();
}
//.............................................................................
void TOlex2c::macWait(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  olx_sleep(Cmds[0].ToInt());
}
//.............................................................................
void TOlex2c::macDelIns(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  TIns& Ins = XApp.XFile().GetLastLoader<TIns>();
  if (Cmds[0].IsNumber()) {
    int insIndex = Cmds[0].ToInt();
    Ins.DelIns(insIndex);
    return;
  }
  for (size_t i = 0; i < Ins.InsCount(); i++) {
    if (Ins.InsName(i).Equalsi(Cmds[0])) {
      Ins.DelIns(i--);
      continue;
    }
  }
}
//.............................................................................
void TOlex2c::macExec(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  bool Asyn = !Options.Contains('s'), // synchroniusly
    Cout = !Options.Contains('o'),    // catch output
    quite = Options.Contains('q');

  olxstr dubFile(Options.FindValue('s', EmptyString()));

  olxstr Tmp;
  bool Space;
  for (size_t i = 0; i < Cmds.Count(); i++) {
    Space = (Cmds[i].FirstIndexOf(' ') != InvalidIndex);
    if (Space)  Tmp << '\"';
    Tmp << Cmds[i];
    if (Space) Tmp << '\"';
    Tmp << ' ';
  }
  TBasicApp::NewLogEntry() << "EXEC: " << Tmp;
  short flags = 0;
  if ((Cout && Asyn) || Asyn) {  // the only combination
    if (!Cout)
      flags = quite ? spfQuiet : 0;
    else
      flags = quite ? spfRedirected | spfQuiet : spfRedirected;
  }
  else
    flags = spfSynchronised;
  AProcess *Process = 0;
#ifdef __WIN32__
  Process = new TWinProcess(Tmp, flags);
#else
#ifdef __WXWIDGETS__
  Process = new TWxProcess(Tmp, flags);
#else
  throw TNotImplementedException(__OlxSourceInfo);
#endif
#endif
  if ((Cout && Asyn) || Asyn) {  // the only combination
    if (!Cout) {
      _ProcessManager->OnCreate(*Process);
      if (!Process->Execute()) {
        _ProcessManager->OnTerminate(*Process);
        Error.ProcessingError(__OlxSrcInfo, "failed to launch a new process");
      }
      return;
    }
    else  {
      _ProcessManager->OnCreate(*Process);
      if (!dubFile.IsEmpty()) {
        TEFile* df = new TEFile(dubFile, "wb+");
        Process->SetDubStream(df);
      }
      if (!Process->Execute()) {
        _ProcessManager->OnTerminate(*Process);
        Error.ProcessingError(__OlxSrcInfo, "failed to launch a new process");
      }
    }
    return;
  }
  if (!Process->Execute())
    Error.ProcessingError(__OlxSrcInfo, "failed to launch a new process");
  delete Process;
}
//.............................................................................
void TOlex2c::macPython(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
#ifdef _PYTHON
  olxstr tmp = Cmds.Text(' ');
  tmp.Replace("\\n", "\n");
  if (!tmp.EndsWith('\n'))  tmp << '\n';
  PythonExt::GetInstance()->RunPython(tmp);
#else
  E.ProcessingError(__OlxSrcInfo, "Python is not available");
#endif
}
//.............................................................................
void TOlex2c::macReload(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  if (Cmds[0].Equalsi("macro")) {
    olxstr macroFile(XApp.GetBaseDir() + "macrox.xld");
    if (TEFile::Exists(macroFile)) {
      TDataFile df;
      df.LoadFromXLFile(macroFile);
      df.Include(NULL);
      TDataItem* di = df.Root().FindItem("xl_macro");
      if (di != NULL)  Macros.Load(*di);
    }
  }
}
//.............................................................................
void TOlex2c::macClear(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  return;
}
//.............................................................................
void TOlex2c::macEcho(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  for (size_t i = 0; i < Cmds.Count(); i++)
    TBasicApp::GetLog() << Cmds[i] << ' ';
  TBasicApp::GetLog() << NewLineSequence();
}
//.............................................................................
void TOlex2c::funIsPluginInstalled(const TStrObjList& Params,
  TMacroData &E)
{
  E.SetRetVal(plugins.Contains(Params[0]));
}
//.............................................................................
void TOlex2c::funHasGUI(const TStrObjList& Params, TMacroData &E) {
  E.SetRetVal(false);
}
//.............................................................................
void TOlex2c::funStrCmp(const TStrObjList& Params, TMacroData &E) {
  E.SetRetVal(Params[0] == Params[1]);
}
//.............................................................................
void TOlex2c::funUnsetVar(const TStrObjList& Params, TMacroData &E) {
  TOlxVars::UnsetVar(Params[0]);
}
//.............................................................................
void TOlex2c::funSetVar(const TStrObjList& Params, TMacroData &E) {
  TOlxVars::SetVar(Params[0], Params[1]);
}
//.............................................................................
void TOlex2c::funGetVar(const TStrObjList& Params, TMacroData &E) {
  size_t ind = TOlxVars::VarIndex(Params[0]);
  if (ind == InvalidIndex) {
    if (Params.Count() == 2)
      E.SetRetVal(Params[1]);
    else  {
      E.ProcessingError(__OlxSrcInfo,
        "Could not locate specified variable: '") << Params[0] << '\'';
    }
    return;
  }
  E.SetRetVal(TOlxVars::GetVarStr(ind));
}
//.............................................................................
void TOlex2c::funIsVar(const TStrObjList& Params, TMacroData &E) {
  E.SetRetVal(TOlxVars::IsVar(Params[0]));
}
//.............................................................................
void TOlex2c::funGetEnv(const TStrObjList& Params, TMacroData &E) {
  if (Params.IsEmpty()) {
#if defined(__WIN32__) && defined(_UNICODE) && defined(_MSC_VER)
    if (_wenviron != NULL) {
      for (size_t i = 0; _wenviron[i] != 0; i++) {
        TBasicApp::NewLogEntry() << _wenviron[i];
      }
    }
#else
    extern char **environ;
    if (environ != NULL) {
      for (size_t i = 0; environ[i] != 0; i++) {
        TBasicApp::NewLogEntry() << environ[i];
      }
    }
#endif
  }
  else {
    E.SetRetVal(olx_getenv(Params[0]));
  }
}
//..............................................................................
void TOlex2c::macReap(TStrObjList &Cmds, const TParamList &Options, TMacroData &Error) {
  if (Cmds.IsEmpty()) {
    FileFilter ff;
    ff.AddAll("ins;cif;res;xyz;p4p;crs;pdb");
    olxstr f = TShellUtil::PickFile("Choose file", ff.GetString(), true);
    if (!f.IsEmpty())
      XApp.XFile().LoadFromFile(f);
  }
  else
    XApp.XFile().LoadFromFile(TEFile::ExpandRelativePath(Cmds.Text(' ')));
}
//.............................................................................
void TOlex2c::funUser(const TStrObjList &Cmds, TMacroData &Error) {
  if (Cmds.IsEmpty())
    Error.SetRetVal(TEFile::CurrentDir());
  else
    TEFile::ChangeDir(Cmds[0]);
}
//.............................................................................
void TOlex2c::macInfo(TStrObjList& Cmds, const TParamList& Options, TMacroData& Error) {
  TSAtomPList satoms;
  LocateAtoms(Cmds, satoms, true);
  TCAtomPList atoms(satoms,
    FunctionAccessor::MakeConst(&TSAtom::CAtom));
  TTTable<TStrList> Table(atoms.Count(), 7);
  Table.ColName(0) = "Atom";
  Table.ColName(1) = "Symb";
  Table.ColName(2) = "X";
  Table.ColName(3) = "Y";
  Table.ColName(4) = "Z";
  Table.ColName(5) = "Ueq";
  Table.ColName(6) = "Peak";
  for (size_t i = 0; i < atoms.Count(); i++) {
    Table[i][0] = atoms[i]->GetLabel();
    Table[i][1] = atoms[i]->GetType().symbol;
    Table[i][2] = olxstr::FormatFloat(3, atoms[i]->ccrd()[0]);
    Table[i][3] = olxstr::FormatFloat(3, atoms[i]->ccrd()[1]);
    Table[i][4] = olxstr::FormatFloat(3, atoms[i]->ccrd()[2]);
    Table[i][5] = olxstr::FormatFloat(3, atoms[i]->GetUiso());
    if (atoms[i]->GetType() == iQPeakZ) {
      Table[i][6] = olxstr::FormatFloat(3, atoms[i]->GetQPeak());
    }
    else {
      Table[i][6] = '-';
    }
  }
  TBasicApp::NewLogEntry() << Table.CreateTXTList("Atom information", true, true, ' ');
}
//.............................................................................
void TOlex2c::macName(TStrObjList& Cmds, const TParamList& Options, TMacroData& Error) {
  TSAtomPList atoms;
  TStrObjList toks(Cmds.Text(' ', 0, Cmds.Count() - 1), ' ');
  if (!LocateAtoms(toks, atoms, false)) {
    return;
  }
  if (Cmds[Cmds.Count() - 1].IsNumber()) {
    int start = Cmds[1].ToInt();
    for (size_t i = 0; i < atoms.Count(); i++) {
      atoms[i]->CAtom().SetLabel(atoms[i]->GetType().symbol + start++, false);
    }
  }
  else {
    for (size_t i = 0; i < atoms.Count(); i++) {
      atoms[i]->CAtom().SetLabel(Cmds[1]);
    }
  }
}
//.............................................................................
void TOlex2c::macKill(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  TSAtomPList atoms;
  if (!LocateAtoms(Cmds, atoms, false)) {
    return;
  }
  for (size_t i = 0; i < atoms.Count(); i++) {
    atoms[i]->CAtom().SetDeleted(true);
  }
  XApp.XFile().EndUpdate();
  UnifyAtomList(Selection);
}
//.............................................................................
// cannot stick it anywhere else, eh?
void TOlex2c::funGetMAC(const TStrObjList& Params, TMacroData &E) {
  bool full = (Params.Count() == 1 && Params[0].Equalsi("full"));
  olxstr rv(EmptyString(), 256);
  char bf[16];
  TShellUtil::MACInfo MACsInfo;
  TShellUtil::ListMACAddresses(MACsInfo);
  for (size_t i = 0; i < MACsInfo.Count(); i++) {
    if (full) {
      rv << MACsInfo[i] << '=';
    }
    for (size_t j = 0; j < MACsInfo.GetObject(i).Count(); j++) {
      sprintf(bf, "%02X", MACsInfo.GetObject(i)[j]);
      rv << bf;
      if (j < 5) {
        rv << '-';
      }
    }
    if ((i + 1) < MACsInfo.Count()) {
      rv << ';';
    }
  }
  E.SetRetVal(rv.IsEmpty() ? XLibMacros::NAString() : rv);
}
//.............................................................................
void TOlex2c::funExtractCell(const TStrObjList& Params, TMacroData &E) {
  olx_scope_cs cs_(TBasicApp::GetCriticalSection());
  TBasicCFile *fi = XApp.XFile().FindFormat(TEFile::ExtractFileExt(Params[0]));
  TStrList rv;
  if (fi != NULL) {
    fi = dynamic_cast<TBasicCFile *>(fi->Replicate());
    fi->LoadFromFile(Params[0]);
    TAsymmUnit &au = fi->GetAsymmUnit();
    short latt = olx_abs(au.GetLatt());
    rv.Add(TCLattice::SymbolForLatt(latt));
    for (int i = 0; i < 3; i++) rv.Add(au.GetAxes()[i]);
    for (int i = 0; i < 3; i++) rv.Add(au.GetAngles()[i]);
    TEValueD vol = TUnitCell::CalcVolumeEx(au);
    rv.Add(vol.GetV());
    for (int i = 0; i < 3; i++) {
      rv.Add(au.GetAxisEsds()[i]);
    }
    for (int i = 0; i < 3; i++) {
      rv.Add(au.GetAngleEsds()[i]);
    }
    rv.Add(vol.GetE());
    vec3d ax = au.GetAxes(), ag = au.GetAngles();
    Niggli::reduce(latt, ax, ag);
    for (int i = 0; i < 3; i++) {
      rv.Add(ax[i]);
    }
    for (int i = 0; i < 3; i++) {
      rv.Add(ag[i]);
    }
    rv.Add(TUnitCell::CalcVolume(ax, ag));
  }
  E.SetRetVal(rv.Text(','));
}
//.............................................................................
void TOlex2c::funReduceCell(const TStrObjList& Params, TMacroData &E) {
  olx_scope_cs cs_(TBasicApp::GetCriticalSection());
  TStrList rv;
  vec3d
    ax = vec3d(Params[1].ToDouble(), Params[2].ToDouble(), Params[3].ToDouble()),
    ag = vec3d(Params[4].ToDouble(), Params[5].ToDouble(), Params[6].ToDouble());
  Niggli::reduce(TCLattice::LattForSymbol(Params[0].CharAt(0)), ax, ag);
  for (int i = 0; i < 3; i++) {
    rv.Add(ax[i]);
  }
  for (int i = 0; i < 3; i++) {
    rv.Add(ag[i]);
  }
  rv.Add(TUnitCell::CalcVolume(ax, ag));
  E.SetRetVal(rv.Text(','));
}
//.............................................................................
void TOlex2c::funImportMetadata(const TStrObjList& Params, TMacroData &E) {
  TCif cif;
  cif.LoadFromFile(Params[0]);
  TStrList rv, to_skip("", ';');
  for (size_t i = 0; i < cif.ParamCount(); i++) {
    const olxstr &pn = cif.ParamName(i);

    if ((pn.StartsFrom("_atom_site") && !pn.StartsFrom("_atom_sites")) ||
      pn.StartsFrom("_geom") ||
      pn.StartsFrom("_space_group") ||
      pn.StartsFrom("_refine")
      )
    {
      continue;
    }
    rv.Add(pn) << ',' << cif.GetParamAsString(cif.ParamName(i));
  }
  E.SetRetVal(rv.Text('\r'));
}
//.............................................................................
void TOlex2c::funSuggestSGAndContent(const TStrObjList& Params, TMacroData &E) {
  TMacroData me;
  TStrList rv;
  TPtrList<TSpaceGroup> groups;
  if (processMacroEx("sg", me)) {
    me.SetRetVal(false);
    rv.Strtok(XApp.GetLastSGResult(), ';');
    if (rv.Count() > 1 && processMacroEx("wilson", me)) {
      if (E.HasRetVal()) {
        bool test_v = E.GetRetVal().ToBool();
        for (size_t i = 0; i < rv.Count(); i++) {
          groups.Add(TSymmLib::GetInstance().FindGroupByName(rv[i]));
        }
        rv.Clear();
        for (size_t i = 0; i < groups.Count(); i++) {
          if (groups[i]->IsCentrosymmetric() == test_v) {
            rv.Add(groups[i]->GetName());
          }
        }
        for (size_t i = 0; i < groups.Count(); i++) {
          if (groups[i]->IsCentrosymmetric() != test_v) {
            rv.Add(groups[i]->GetName());
          }
        }
      }
    }
  }
  olxstr content = "xf.GetFormula(unit)";
  processFunction(content, EmptyString(), true);
  rv.Add(content);
  E.SetRetVal(rv.Text(';'));
}
//.............................................................................
void TOlex2c::macUpdateModel(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  TXFile &xf = *dynamic_cast<TXFile *>(XApp.XFile().Replicate());
  xf.LoadFromFile(Cmds[0]);
  TTypeList<olx_pair_t<olxstr, olxstr> > catoms;
  {
    TStrList l = TEFile::ReadLines(Cmds[1]);
    catoms.SetCapacity(l.Count());
    for (size_t i = 0; i < l.Count(); i++) {
      size_t idx = l[i].IndexOf(' ');
      if (idx == InvalidIndex) {
        E.ProcessingError(__OlxSrcInfo, "invalid file format");
        return;
      }
      catoms.AddNew(l[i].SubStringTo(idx), l[i].SubStringFrom(idx + 1));
    }
  }
  TIObjectProvider<TSAtom> &fatoms = xf.GetLattice().GetObjects().atoms;
  if (fatoms.Count() != catoms.Count()) {
    E.ProcessingError(__OlxSrcInfo, "mismatching atom lists");
    TEFile::WriteLines(Cmds[2], TCStrList(xf.ToJSON().obj()));
    return;
  }
  for (size_t i = 0; i < catoms.Count(); i++) {
    TCAtom &a = fatoms[i].CAtom();
    if (a.GetLabel() != catoms[i].GetA()) {
      a.SetLabel(catoms[i].GetA());
    }
    TStrList toks(catoms[i].GetB(), ';');
    bool has_matrix = false;
    for (size_t j = 0; j < toks.Count(); j++) {
      size_t idx = toks[j].IndexOf(':');
      if (idx == InvalidIndex) {
        E.ProcessingError(__OlxSrcInfo, "invalid file format");
        return;
      }
      olxstr pname = toks[j].SubStringTo(idx),
        pval = toks[j].SubStringFrom(idx + 1);
      if (pname == "deleted") {
        a.SetDeleted(true);
        break;
      }
      else if (pname == "occu") {
        a.SetOccu(pval.ToDouble());
      }
      else if (pname == "part") {
        a.SetPart((int8_t)pval.ToInt());
      }
      else if (pname == "matrix") {
        has_matrix = true;
        TStrList mt(pval, ',');
        if (a.GetEllipsoid() == NULL && mt.Count() == 3) {
          TEllipsoid e;
          evecd ee(6);
          ee[0] = mt[0].ToDouble();
          ee[1] = mt[1].ToDouble();
          ee[2] = mt[2].ToDouble();
          xf.GetAsymmUnit().UcartToUcif(ee);
          a.UpdateEllp(ee);
        }
      }
    }
    if (!has_matrix && a.GetEllipsoid() != NULL) {
      a.AssignEllp(NULL);
    }
  }
  xf.GetUnitCell().UpdateEllipsoids();
  xf.EndUpdate();
  xf.SaveToFile(Cmds[0]);
  xf.LoadFromFile(Cmds[0]); // update the atom order!
  TEFile::WriteLines(Cmds[2], TCStrList(xf.ToJSON().obj()));
}
//.............................................................................
void TOlex2c::macUpdateJSON(TStrObjList &Cmds, const TParamList &Options, TMacroData &E) {
  TXFile &xf = *dynamic_cast<TXFile *>(XApp.XFile().Replicate());
  xf.LoadFromFile(Cmds[0]);
  TEFile::WriteLines(TEFile::ChangeFileExt(Cmds[0], "json"),
    TCStrList(xf.ToJSON().obj()));
}
//.............................................................................
void TOlex2c::funDeposit(const TStrObjList &Params, TMacroData &E) {
  volatile olx_scope_cs cs_(TBasicApp::GetCriticalSection());
  static const TStrList
    extr("_diffrn_radiation_wavelength;_diffrn_radiation_type;"
    "_diffrn_ambient_temperature;"
    "_exptl_crystal_size_max;_exptl_crystal_size_mid;_exptl_crystal_size_min;"
    "_exptl_crystal_colour;_exptl_crystal_description;"
    "_chemical_formula_sum;_chemical_formula_weight;"
    "_computing_structure_solution;_computing_structure_refinement;"
    "_refine_ls_R_factor_all;_refine_ls_R_factor_gt;"
    "_refine_ls_wR_factor_ref;_refine_ls_wR_factor_gt;"
    "_refine_ls_goodness_of_fit_ref;_refine_ls_restrained_S_all;"
    "_refine_ls_shift/su_max;_refine_diff_density_min;_refine_diff_density_max;"
    "_shelxl_version_number;"
    "_database_code_depnum_ccdc_archive;"
    "_audit_creation_method;"
    ,
    ';');
  TStrList rv;
  TXFile &xf = XApp.XFile();
  // self-containing CIF
  const olxstr parent_dir = TEFile::ExtractFilePath(Params[0]);
  if (Params.Count() == 1 &&
    (Params[0].EndsWithi(".cif") || Params[0].EndsWithi(".fcf")))
  {
    xf.LoadFromFile(Params[0]);
    TCif &cif = xf.GetLastLoader<TCif>();
    cif_dp::cetStringList *ci = dynamic_cast<cif_dp::cetStringList *>(
      cif.FindEntry("_shelx_res_file"));
    if (ci == 0) {
      ci = dynamic_cast<cif_dp::cetStringList *>(
        cif.FindEntry("_iucr_refine_instructions_details"));
    }
    if (xf.GetRM().GetReflections().IsEmpty()) {
      E.SetRetVal(olxstr("error,") << "incomplete deposition CIF");
      return;
    }
    if (ci != 0) {
      //TEFile::WriteLines(parent_dir + "model.res", TCStrList(ci->lines));
      rv.Add("deposition_type,model,observations,deposition");
    }
    else {
      rv.Add("deposition_type,observations");
    }
    //THklFile::SaveToFile(parent_dir + "model.hkl", xf.GetRM().GetReflections());
  }
  else if (Params.Count() == 2) {
    TStrList files(Params);
    olxstr ref_f;
    if (files[0].EndsWithi(".hkl") || files[0].EndsWithi(".fcf")) {
      ref_f = files[0];
      files.Delete(0);
    }
    else if (files[1].EndsWithi(".hkl") || files[1].EndsWithi(".fcf")) {
      ref_f = files[1];
      files.Delete(1);
    }
    if (files.Count() != 1) {
      E.SetRetVal(olxstr("error,") <<
        "could not locate reflections file (HKL, FCF is expected)");
      return;
    }
    try {
      xf.LoadFromFile(files[0]);
      if (xf.GetAsymmUnit().AtomCount() != 0) {
        rv.Add("deposition_type,model,observations");
      }
      else {
        rv.Add("deposition_type,observations");
      }
    }
    catch (...) {
      E.SetRetVal(olxstr("error,") <<
        "could not read the cell/model file (HKL, FCF is expected)");
      return;
    }
  }
  else if (Params.Count() == 3) {
    olxstr ref_f, mod_f, cif_f;
    for (size_t i = 0; i < Params.Count(); i++) {
      if (Params[i].EndsWithi(".hkl") || Params[i].EndsWithi(".fcf")) {
        ref_f = Params[i];
      }
      else if (Params[i].EndsWithi(".ins") || Params[i].EndsWithi(".res")) {
        mod_f = Params[i];
      }
      else if (Params[i].EndsWithi(".cif")) {
        cif_f = Params[i];
      }
    }
    if (ref_f.IsEmpty() || mod_f.IsEmpty() || cif_f.IsEmpty()) {
      E.SetRetVal(olxstr("error,") <<
        "could not locate one of the required files (INS/RES, HKL/FCF and CIF "
        "are expected)");
      return;
    }
    olxstr path = TEFile::AddPathDelimeter(TEFile::ExtractFilePath(mod_f));
    //if (!mod_f.EndsWithi(".res"))  // if ins is provided
    //  TEFile::Rename(mod_f, path + "model.res");
    //if (!ref_f.EndsWithi(".hkl"))  // if fcf is provided
    //  TEFile::Rename(ref_f, path + "model.hkl");
    xf.LoadFromFile(path + "model.cif");
    if (!processMacro("cifmerge -f=true")) {
      E.SetRetVal(olxstr("error,") << "CifMerge failed");
      return;
    }
    if (xf.GetAsymmUnit().AtomCount() != 0) {
      rv.Add("deposition_type,model,observations,deposition");
    }
    else {
      rv.Add("deposition_type,observations");
    }
  }

  if (xf.HasLastLoader()) {
    if (XApp.CheckFileType<TCif>()) {
      TCif &cif = xf.GetLastLoader<TCif>();
      for (size_t i = 0; i < extr.Count(); i++) {
        cif_dp::IStringCifEntry *e = cif.FindParam<cif_dp::IStringCifEntry>(extr[i]);
        if (e != 0) {
          olxstr v = (*e)[0];
          for (size_t si = 1; si < e->Count(); si++) {
            v << ' ' << (*e)[si];
          }
          rv.Add(extr[i]) << ',' << v;
        }
      }
    }
      {
        TIns ins;
        ins.Adopt(xf, 0);
        ins.SaveForSolution(parent_dir + "model.ins", "TREF", EmptyString());
      }
      TAsymmUnit &au = xf.GetAsymmUnit();
      TEValueD vol = TUnitCell::CalcVolumeEx(au);
      vec3d ax = au.GetAxes(), ag = au.GetAngles();
      rv.Add("cell").stream(',') << xf.GetLastLoaderSG().GetLattice().GetSymbol()
        << olxstr(',').Join(ax) << olxstr(',').Join(ag) << vol.GetV() <<
        olxstr(',').Join(au.GetAxisEsds()) <<
        olxstr(',').Join(au.GetAngleEsds()) << vol.GetE();

      Niggli::reduce(xf.GetLastLoaderSG().GetLattice().GetLatt(), ax, ag);
      rv.GetLastString().stream(',') << olxstr(',').Join(ax) <<
        olxstr(',').Join(ag) << TUnitCell::CalcVolume(ax, ag);
      rv.Add("space_group,") << xf.GetLastLoaderSG().GetName();
      if (!xf.GetRM().GetReflections().IsEmpty()) {
        const RefinementModel::HklStat &s = xf.GetRM().GetMergeStat();
        rv.Add("stat_r_int,") << s.Rint;
        rv.Add("stat_r_sig,") << s.Rsigma;
        rv.Add("stat_MIS,") << s.MeanIOverSigma;
        rv.Add("stat_completness,") << s.Completeness;
        rv.Add("stat_total_count,") << s.TotalReflections;
        rv.Add("stat_unique_count,") << s.UniqueReflections;
        rv.Add("stat_friedel_count,") << xf.GetRM().GetFriedelPairCount();
        rv.Add("stat_inconsistent_count,") << s.InconsistentEquivalents;
        rv.Add("stat_redundancy,") <<
          olx_round((double)s.TotalReflections/s.UniqueReflections, 100);
        rv.Add("ref_gt,") << s.GTRefs;
        rv.Add("ref_gtf,") <<
          olx_round((double)s.GTRefs/s.UniqueReflections, 100);
        rv.Add("ref_max_d,") << s.MaxD;
        rv.Add("ref_min_d,") << s.MinD;
        rv.Add("ref_count,") << s.GetReadReflections();
      }
    }
    E.SetRetVal(rv.Text('\r'));
    xf.Close();
  }
  void TOlex2c::funTranslatePhrase(const TStrObjList& Params, TMacroData &E) {
    E.SetRetVal(TranslateString(Params[0]));
  }
  //..............................................................................
  void TOlex2c::funCurrentLanguageEncoding(const TStrObjList& Params, TMacroData &E) {
    E.SetRetVal(Dictionary.GetCurrentLanguageEncodingStr());
  }
  //..............................................................................
  void TOlex2c::funIsCurrentLanguage(const TStrObjList& Params, TMacroData &E) {
    TStrList toks;
    toks.Strtok(Params[0], ';');
    for (size_t i = 0; i < toks.Count(); i++) {
      if (toks[i] == Dictionary.GetCurrentLanguage()) {
        E.SetRetVal(true);
        return;
      }
    }
    E.SetRetVal(false);
  }
  //..............................................................................
void TOlex2c::funLoadDll(const TStrObjList &Cmds, TMacroData &E) {
  typedef olex2::IOlex2Runnable* (*GOR)();
#ifdef __WXWIDGETS__
  olx_object_ptr<wxDynamicLibrary> dl = new wxDynamicLibrary(Cmds[0].u_str());
  if (!dl->IsLoaded()) {
    E.ProcessingError(__OlxSrcInfo, "could not load the library");
    return;
  }
  GOR  gor = (GOR)dl->GetSymbol(wxT("GetOlex2Runnable"));
  if (gor == 0) {
    E.ProcessingError(__OlxSrcInfo, "could not locate initialisation point");
    return;
  }
  olex2::IOlex2Runnable* runnable = (*gor)();
  if (runnable == 0) {
    E.ProcessingError(__OlxSrcInfo, "NULL runnable");
    return;
  }
  if (!loadedDll.Contains(runnable)) {
    if (!runnable->Initialise(this)) {
      dl->Detach();
    }
    else {
      loadedDll << runnable;
    }
  }
#elif __WIN32__
  HMODULE hm = LoadLibrary(Cmds[0].u_str());
  if (hm == 0) {
    E.ProcessingError(__OlxSrcInfo, "could not load the library");
    return;
  }
  GOR gor = (GOR)GetProcAddress(hm, "GetOlex2Runnable");
  if (gor == 0) {
    E.ProcessingError(__OlxSrcInfo, "could not locate initialisation point");
    return;
  }
  olex2::IOlex2Runnable* runnable = (*gor)();
  if (runnable == 0) {
    E.ProcessingError(__OlxSrcInfo, "NULL runnable");
    return;
  }
  if (!loadedDll.Contains(runnable)) {
    if (!runnable->Initialise(this)) {
      FreeLibrary(hm);
    }
    else {
      loadedDll << runnable;
    }
  }
#endif
}
//.............................................................................
/////////////////////////////////////////////////////////////////////////////
bool TOlex2c::TerminateSignal = false;
////////////////////////////////////////////////////////////////////////////////////////

