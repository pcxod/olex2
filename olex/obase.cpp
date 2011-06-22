/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "obase.h"
#include "xglapp.h"
#include "mainform.h"

#include "xatom.h"
#include "xbond.h"
#include "modes/name.h"

#include "glgroup.h"
#include "modes/match.h"

#include "ins.h"
#include "modes/split.h"
#include "modes/himp.h"
#include "modes/part.h"
#include "modes/hfix.h"
#include "modes/occu.h"
#include "modes/fixu.h"
#include "modes/fixc.h"

#include "xgrowline.h"
#include "modes/grow.h"
#include "xgrowpoint.h"
#include "modes/pack.h"
#include "modes/move.h"
#include "modes/fit.h"

 bool TPartMode::HasInstance = false;
 bool TOccuMode::HasInstance = false;
 bool TFixUMode::HasInstance = false;
 bool TFixCMode::HasInstance = false;
 TNameMode* TNameMode::Instance = NULL;

AMode::AMode(size_t id) : Id(id)  {
  TModeChange mc(Id, true);
  TGlXApp::GetMainForm()->OnModeChange.Execute(NULL, &mc);
}
//..............................................................................
AMode::~AMode() {
  TModeChange mc(Id, false);
  TGlXApp::GetMainForm()->OnModeChange.Execute(NULL, &mc);
  TGlXApp::GetMainForm()->executeFunction("cursor()");  //r eset the screen cursor
  TGlXApp::GetGXApp()->ClearLabelMarks();  // hide atom marks if any
}
//..............................................................................
//..............................................................................
//..............................................................................
AModeWithLabels::AModeWithLabels(size_t id) : AMode(id)  {
  LabelsVisible = TGlXApp::GetGXApp()->AreLabelsVisible();
  LabelsMode = TGlXApp::GetGXApp()->GetLabelsMode();
}
//..............................................................................
AModeWithLabels::~AModeWithLabels()  {
  TGlXApp::GetGXApp()->SetLabelsVisible(LabelsVisible);
  TGlXApp::GetGXApp()->SetLabelsMode(LabelsMode);
}
//..............................................................................
//..............................................................................
//..............................................................................
TModes* TModes::Instance = NULL;

TModes::TModes() {
  if( Instance != NULL )  throw TFunctionFailedException(__OlxSourceInfo, "singleton");
  Instance = this;
  Modes.Add( "name",   new TModeFactory<TNameMode>(Modes.Count()+1));
  Modes.Add( "match",  new TModeFactory<TMatchMode>(Modes.Count()+1));
  Modes.Add( "split",  new TModeFactory<TSplitMode>(Modes.Count()+1));
  Modes.Add( "himp",   new TModeFactory<THimpMode>(Modes.Count()+1));
  Modes.Add( "hfix",   new TModeFactory<THfixMode>(Modes.Count()+1));
  Modes.Add( "part",   new TModeFactory<TPartMode>(Modes.Count()+1));
  Modes.Add( "occu",   new TModeFactory<TOccuMode>(Modes.Count()+1));
  Modes.Add( "fixu",   new TModeFactory<TFixUMode>(Modes.Count()+1));
  Modes.Add( "fixxyz", new TModeFactory<TFixCMode>(Modes.Count()+1));
  Modes.Add( "grow",   new TModeFactory<TGrowMode>(Modes.Count()+1));
  Modes.Add( "pack",   new TModeFactory<TPackMode>(Modes.Count()+1));
  Modes.Add( "move",   new TModeFactory<TMoveMode>(Modes.Count()+1));
  Modes.Add( "fit",   new TModeFactory<TFitMode>(Modes.Count()+1));
  CurrentMode = NULL;
}
//..............................................................................
AMode* TModes::SetMode(const olxstr& name)  {
  AModeFactory* mf = Modes[name];
  if( CurrentMode != NULL )  {
    CurrentMode->Finalise();
    delete CurrentMode;
  }
  CurrentMode = NULL;  // mf->New Calls other functions, validating currnet mode...
  CurrentMode = (mf == NULL) ? NULL : mf->New();
  return CurrentMode;
}
//..............................................................................
void TModes::ClearMode(bool finalise)  {
  if( CurrentMode == NULL )  return;
  if( finalise )  CurrentMode->Finalise();
  delete CurrentMode;
  CurrentMode = NULL;
}
//..............................................................................
TModes::~TModes()  {
  for( size_t i=0; i < Modes.Count(); i++ )
    delete Modes.GetObject(i);
  if( CurrentMode != NULL )
    delete CurrentMode;
  Instance = NULL;
}
//..............................................................................
size_t TModes::DecodeMode(const olxstr& mode)  {
  if( Instance == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "uninitialised instance");
  return Instance->Modes.IndexOf(mode) + 1;  // -1 +1 = 0 = mmNone
}
//..............................................................................
bool TModeChange::CheckStatus(const olxstr& mode, const olxstr& modeData)  {
  return CheckStatus(TModes::DecodeMode(mode), modeData);
}
bool TModeChange::CheckStatus(size_t mode, const olxstr& modeData) {
  return TGlXApp::GetMainForm()->CheckMode(mode, modeData);
}

//..............................................................................
//..............................................................................
//..............................................................................
TStateChange::TStateChange(uint32_t state, bool status, const olxstr& data) {
  FStatus = status;
  State = state;
  TGlXApp::GetMainForm()->SetProgramState(status,  state, data);
}
//..............................................................................
olxstr TStateChange::StrRepr(uint32_t State) {
  switch( State )  {
    case prsStrVis:     return "strvis";
    case prsHVis:       return "hvis";
    case prsHBVis:      return "hbvis";
    case prsQVis:       return "qvis";
    case prsQBVis:      return "qbvis";
    case prsCellVis:    return "cellvis";
    case prsBasisVis:   return "basisvis";
    case prsHtmlVis:    return "htmlvis";
    case prsHtmlTTVis:  return "htmlttvis";
    case prsBmpVis:     return "bmpvis";
    case prsInfoVis:    return "infovis";
    case prsHelpVis:    return "helpvis";
    case prsCmdlVis:    return "cmdlinevis";
    case prsGradBG:     return "gradBG";
    case prsLabels:     return "labelsvis";
    case prsGLTT:       return "gltt";
    case prsPluginInstalled:  return "pluginInstalled";
    case prsGridVis:  return "gridvis";
    case prsWBoxVis:  return "wboxvis";
  }
  return "none";
}
//..............................................................................
uint32_t TStateChange::DecodeState(const olxstr& mode)  {
 if( mode.Equalsi("strvis") )
   return prsStrVis;
  else if( mode.Equalsi("hvis") )
    return prsHVis;
  else if( mode.Equalsi("hbvis") )
    return prsHBVis;
  else if( mode.Equalsi("qvis") )
    return prsQVis;
  else if( mode.Equalsi("qbvis") )
    return prsQBVis;
  else if( mode.Equalsi("cellvis") )
    return prsCellVis;
  else if( mode.Equalsi("basisvis") )
    return prsBasisVis;
  else if( mode.Equalsi("htmlvis") )
    return prsHtmlVis;
  else if( mode.Equalsi("htmlttvis") )
    return prsHtmlTTVis;
  else if( mode.Equalsi("bmpvis") )
    return prsBmpVis;
  else if( mode.Equalsi("pluginInstalled") )
    return prsPluginInstalled;
  else if( mode.Equalsi("infovis") )
    return prsInfoVis;
  else if( mode.Equalsi("helpvis") )
    return prsHelpVis;
  else if( mode.Equalsi("cmdlinevis") )
    return prsCmdlVis;
  else if( mode.Equalsi("gradBG") )
    return prsGradBG;
  else if( mode.Equalsi("labelsvis") )
    return prsLabels;
  else if( mode.Equalsi("GLTT") )
    return prsGLTT;
  else if( mode.Equalsi("gridvis") )
    return prsGridVis;
  else if( mode.Equalsi("wboxvis") )
    return prsWBoxVis;
  return prsNone;
}
//..............................................................................
bool TStateChange::CheckStatus(const olxstr& stateName, const olxstr& stateData) {
  return CheckStatus(TStateChange::DecodeState(stateName), stateData);
}
//..............................................................................
bool TStateChange::CheckStatus(uint32_t state, const olxstr& stateData) {
  return TGlXApp::GetMainForm()->CheckState(state, stateData);
}
