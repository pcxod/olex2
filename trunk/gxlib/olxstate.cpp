/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "olxstate.h"

#include "modes/name.h"
#include "modes/match.h"

#include "ins.h"
#include "modes/split.h"
#include "modes/himp.h"
#include "modes/part.h"
#include "modes/hfix.h"
#include "modes/occu.h"
#include "modes/fixu.h"
#include "modes/fixc.h"

#include "modes/grow.h"
#include "xgrowpoint.h"
#include "modes/pack.h"
#include "modes/move.h"
#include "modes/fit.h"
#include "modes/lock.h"

bool TPartMode::HasInstance = false;
bool TOccuMode::HasInstance = false;
bool TFixUMode::HasInstance = false;
bool TFixCMode::HasInstance = false;
TNameMode* TNameMode::Instance = NULL;

AMode::AMode(size_t id)
  : Id(id), gxapp(TGXApp::GetInstance()),
    olex2(*olex2::IOlex2Processor::GetInstance()),
    ObjectPicker(*this),
    Initialised(false)
{
  TModeChange mc(Id, true);
  TModeRegistry::GetInstance().OnChange.Execute(NULL, &mc);
  gxapp.GetMouseHandler().SetInMode(true);
  gxapp.GetMouseHandler().OnObject.Add(&ObjectPicker);
}
//..............................................................................
bool AMode::ObjectPicker_::Execute(const IOlxObject *sender, const IOlxObject *data,
  TActionQueue *)
{
  const AGDrawObject *o = dynamic_cast<const AGDrawObject *>(data);
  if (o != NULL) {
    mode.OnObject(*const_cast<AGDrawObject *>(o));
    return true;
  }
  return false;
}
//..............................................................................
AMode::~AMode() {
  gxapp.GetMouseHandler().OnObject.Remove(&ObjectPicker);
  gxapp.GetMouseHandler().SetInMode(false);
  TModeChange mc(Id, false);
  TModeRegistry::GetInstance().OnChange.Execute(NULL, &mc);
  //reset the screen cursor
  olex2.processMacro("cursor()");
}
//..............................................................................
void AMode::SetUserCursor(const olxstr &val, const olxstr &name) const {
  olxstr v = val;
  v.Replace('$', "\\$");
  ABasicFunction *cf = olex2.GetLibrary().FindFunction("cursor");
  if (cf != 0) {
    TStrObjList params;
    params.Add("user");
    params.Add(v);
    params.Add(name);
    TMacroData md;
    cf->Run(params, md);
  }
  else {
    TBasicApp::NewLogEntry(logWarning) << "No 'cursor' function available";
  }
}
//..............................................................................
//..............................................................................
//..............................................................................
AModeWithLabels::AModeWithLabels(size_t id) : AMode(id)  {
  LabelsVisible = gxapp.AreLabelsVisible();
  LabelsMode = gxapp.GetLabelsMode();
}
//..............................................................................
AModeWithLabels::~AModeWithLabels()  {
  gxapp.SetLabelsVisible(LabelsVisible);
  gxapp.SetLabelsMode(LabelsMode);
  gxapp.UpdateDuplicateLabels();
}
//..............................................................................
//..............................................................................
//..............................................................................
TModeRegistry::TModeRegistry()
  : OnChange(Actions.New("OnChange"))
{
  if (Instance() != NULL)
    throw TFunctionFailedException(__OlxSourceInfo, "singleton");
  Instance() = this;
  Modes.Add("name",   new TModeFactory<TNameMode>(0));
  Modes.Add("match",  new TModeFactory<TMatchMode>(0));
  Modes.Add("split",  new TModeFactory<TSplitMode>(0));
  Modes.Add("himp",   new TModeFactory<THimpMode>(0));
  Modes.Add("hfix",   new TModeFactory<THfixMode>(0));
  Modes.Add("part",   new TModeFactory<TPartMode>(0));
  Modes.Add("occu",   new TModeFactory<TOccuMode>(0));
  Modes.Add("fixu",   new TModeFactory<TFixUMode>(0));
  Modes.Add("fixxyz", new TModeFactory<TFixCMode>(0));
  Modes.Add("grow",   new TModeFactory<TGrowMode>(0));
  Modes.Add("pack",   new TModeFactory<TPackMode>(0));
  Modes.Add("move",   new TModeFactory<TMoveMode>(0));
  Modes.Add("fit",    new TModeFactory<TFitMode>(0));
  Modes.Add("lock",   new TModeFactory<TLockMode>(0));
  for (size_t i=0; i < Modes.Count(); i++)
    Modes.GetValue(i)->SetId_(i);
  CurrentMode = NULL;
}
//..............................................................................
AMode* TModeRegistry::SetMode(const olxstr& name, const olxstr &args)  {
  AModeFactory* mf = Modes.Find(name, NULL);
  if (CurrentMode != NULL) {
    if (CurrentMode->IsInitialised())
    CurrentMode->Finalise();
    delete CurrentMode;
  }
  CurrentMode = NULL;  // mf->New Calls other functions, validating current mode...
  CurrentMode = (mf == NULL) ? NULL : mf->New();
  if (CurrentMode != NULL || name.Equalsi("off")) {
    olex2::IOlex2Processor *op = olex2::IOlex2Processor::GetInstance();
    olxstr tmp = name;
    if (!args.IsEmpty()) tmp << ' ' << args;
    op->callCallbackFunc(ModeChangeCB(), TStrList() << tmp);
    if (name.Equalsi("off")) {
      olxstr location = __OlxSrcInfo;
      for (size_t i=0; i < OnModeExit.Count(); i++) {
        if (!op->processMacro(OnModeExit[i], location))
          break;
      }
      OnModeExit.Clear();
    }
  }
  return CurrentMode;
}
//..............................................................................
void TModeRegistry::ClearMode(bool finalise)  {
  if( CurrentMode == NULL )  return;
  if( finalise )  CurrentMode->Finalise();
  delete CurrentMode;
  CurrentMode = NULL;
}
//..............................................................................
TModeRegistry::~TModeRegistry()  {
  for( size_t i=0; i < Modes.Count(); i++ )
    delete Modes.GetValue(i);
  if( CurrentMode != NULL )
    delete CurrentMode;
  Instance() = NULL;
}
//..............................................................................
size_t TModeRegistry::DecodeMode(const olxstr& mode)  {
  return GetInstance().Modes.IndexOf(mode);
}
//..............................................................................
TModeRegistry &TModeRegistry::GetInstance() {
  if (Instance() == NULL)
    throw TFunctionFailedException(__OlxSourceInfo, "uninitialised instance");
  return *Instance();
}
//..............................................................................
bool TModeRegistry::CheckMode(size_t mode) {
  TModeRegistry &inst = GetInstance();
  return inst.GetCurrent() == NULL ? false
    : inst.GetCurrent()->GetId() == mode;
}
//..............................................................................
bool TModeRegistry::CheckMode(const olxstr& mode) {
  return CheckMode(DecodeMode(mode));
}
//..............................................................................
//..............................................................................
//..............................................................................
TStateRegistry::TStateRegistry()
 : OnChange(Actions.New("state_change"))
{
  if (Instance() != NULL)
    throw TFunctionFailedException(__OlxSourceInfo, "singleton");
  Instance() = this;
}
//..............................................................................
void TStateRegistry::RepeatAll() {
  olex2::IOlex2Processor *op = olex2::IOlex2Processor::GetInstance();
  for (size_t i = 0; i < slots.Count(); i++) {
    olx_cset<olxstr> &dl = data_cache.Add(i);
    for (size_t j = 0; j <= dl.Count(); j++) {
      TStrList args;
      olxstr data = (j == dl.Count() ? EmptyString() : dl[j]);
      args.Add(slots[i]->name);
      args.Add(slots[i]->Get(data));
      if (j != dl.Count()) {
        args.Add(dl[j]);
      }
      op->callCallbackFunc(StateChangeCB(), args);
      TStateChange sc(i, slots[i]->Get(data), data);
      OnChange.Execute(NULL, &sc);
    }
  }
}
//..............................................................................
void TStateRegistry::SetState(size_t id, bool status, const olxstr &data,
  bool internal_call)
{
  TStateChange sc(id, status, data);
  OnChange.Execute(NULL, &sc);
  olex2::IOlex2Processor *op = olex2::IOlex2Processor::GetInstance();
  TStrList args;
  args.Add(slots[id]->name);
  args.Add(status);
  if (!data.IsEmpty()) {
    args.Add(data);
    data_cache.Add(id).Add(data);
  }
  op->callCallbackFunc(StateChangeCB(), args);
  if (internal_call) return;
  slots[id]->Set(status, data);
}
//..............................................................................
void TStateRegistry::TMacroSetter::operator ()(bool v, const olxstr &data) {
  olxstr c = cmd;
  c << ' ' << v;
  if (!data.IsEmpty()) c << ' ' << data;
  olex2::IOlex2Processor::GetInstance()->processMacro(c, __OlxSrcInfo);
}
//..............................................................................
TStateRegistry &TStateRegistry::GetInstance() {
  if (Instance() == NULL)
    throw TFunctionFailedException(__OlxSourceInfo, "uninitialised instance");
  return *Instance();
}
