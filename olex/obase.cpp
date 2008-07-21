#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "obase.h"
#include "xglapp.h"
#include "mainform.h"

#include "xatom.h"
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


AMode::AMode(int id) : Id(id)  {
  TModeChange mc(Id, true);
  TGlXApp::GetMainForm()->OnModeChange->Execute(NULL, &mc);
}
//..............................................................................
AMode::~AMode() {
  TModeChange mc(Id, false);
  TGlXApp::GetMainForm()->OnModeChange->Execute(NULL, &mc);
  TGlXApp::GetMainForm()->executeFunction("cursor()");  //r eset the screen cursor
  TGlXApp::GetGXApp()->ClearLabelMarks();  // hide atom marks if any
}
//..............................................................................
//..............................................................................
//..............................................................................
AModeWithLabels::AModeWithLabels(int id) : AMode(id)  {
  LabelsVisible = TGlXApp::GetGXApp()->LabelsVisible();
  LabelsMode = TGlXApp::GetGXApp()->LabelsMode();
}
//..............................................................................
AModeWithLabels::~AModeWithLabels()  {
  TGlXApp::GetGXApp()->LabelsVisible(LabelsVisible);
  TGlXApp::GetGXApp()->LabelsMode(LabelsMode);
}
//..............................................................................
//..............................................................................
//..............................................................................
  TModes* TModes::Instance = NULL;
TModes::TModes() {
  if( Instance != NULL )  throw TFunctionFailedException(__OlxSourceInfo, "singleton");
  Instance = this;
  Modes.Add( "name",   (AModeFactory* const &)(new TModeFactory<TNameMode>(Modes.Count()+1)));
  Modes.Add( "match",  (AModeFactory* const &)(new TModeFactory<TMatchMode>(Modes.Count()+1)));
  Modes.Add( "split",  (AModeFactory* const &)(new TModeFactory<TSplitMode>(Modes.Count()+1)));
  Modes.Add( "himp",   (AModeFactory* const &)(new TModeFactory<THimpMode>(Modes.Count()+1)));
  Modes.Add( "hfix",   (AModeFactory* const &)(new TModeFactory<THfixMode>(Modes.Count()+1)));
  Modes.Add( "part",   (AModeFactory* const &)(new TModeFactory<TPartMode>(Modes.Count()+1)));
  Modes.Add( "occu",   (AModeFactory* const &)(new TModeFactory<TOccuMode>(Modes.Count()+1)));
  Modes.Add( "fixu",   (AModeFactory* const &)(new TModeFactory<TFixUMode>(Modes.Count()+1)));
  Modes.Add( "fixxyz", (AModeFactory* const &)(new TModeFactory<TFixCMode>(Modes.Count()+1)));
  Modes.Add( "grow",   (AModeFactory* const &)(new TModeFactory<TGrowMode>(Modes.Count()+1)));
  Modes.Add( "pack",   (AModeFactory* const &)(new TModeFactory<TPackMode>(Modes.Count()+1)));
  Modes.Add( "move",   (AModeFactory* const &)(new TModeFactory<TMoveMode>(Modes.Count()+1)));
  CurrentMode = NULL;
}
//..............................................................................
AMode* TModes::SetMode(const olxstr& name)  {
  AModeFactory* mf = Modes[name];
  if( CurrentMode != NULL )  delete CurrentMode;
  CurrentMode = NULL;  // mf->New Calls other functions, validating currnet mode...
  CurrentMode = (mf == NULL) ? NULL : mf->New();
  return CurrentMode;
}
//..............................................................................
TModes::~TModes()  {
  for( int i=0; i < Modes.Count(); i++ )
    delete Modes.Object(i);
  if( CurrentMode != NULL )  delete CurrentMode;
  Instance = NULL;
}
//..............................................................................
unsigned short TModes::DecodeMode( const olxstr& mode )  {
  if( Instance == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "uninitialised instance");
  return Instance->Modes.IndexOfComparable(mode) + 1;  // -1 +1 = 0 = mmNone
}
//..............................................................................
bool TModeChange::CheckStatus( const olxstr& mode, const olxstr& modeData )  {
  return CheckStatus(TModes::DecodeMode(mode), modeData);
}
bool TModeChange::CheckStatus( unsigned short mode, const olxstr& modeData ) {
  return TGlXApp::GetMainForm()->CheckMode( mode, modeData );
}

//..............................................................................
//..............................................................................
//..............................................................................
TStateChange::TStateChange(unsigned short state, bool status) {
  FStatus = status;
  State = state;
  TGlXApp::GetMainForm()->SetProgramState(status,  state );
}
//..............................................................................
unsigned short TStateChange::DecodeState( const olxstr& mode )  {
 if( !mode.Comparei("strvis") )
   return prsStrVis;
  else if( !mode.Comparei("hvis") )
    return prsHVis;
  else if( !mode.Comparei("hbvis") )
    return prsHBVis;
  else if( !mode.Comparei("qvis") )
    return prsQVis;
  else if( !mode.Comparei("qbvis") )
    return prsQBVis;
  else if( !mode.Comparei("cellvis") )
    return prsCellVis;
  else if( !mode.Comparei("basisvis") )
    return prsBasisVis;
  else if( !mode.Comparei("htmlvis") )
    return prsHtmlVis;
  else if( !mode.Comparei("htmlttvis") )
    return prsHtmlTTVis;
  else if( !mode.Comparei("bmpvis") )
    return prsBmpVis;
  else if( !mode.Comparei("pluginInstalled") )
    return prsPluginInstalled;
  else if( !mode.Comparei("infoVis") )
    return prsInfoVis;
  else if( !mode.Comparei("helpVis") )
    return prsHelpVis;
  else if( !mode.Comparei("cmdlineVis") )
    return prsCmdlVis;
  else if( !mode.Comparei("gradBG") )
    return prsGradBG;
  return prsNone;
}
//..............................................................................
bool TStateChange::CheckStatus(const olxstr& stateName, const olxstr& stateData) {
  return CheckStatus(TStateChange::DecodeState(stateName), stateData);
}
//..............................................................................
bool TStateChange::CheckStatus(unsigned short state, const olxstr& stateData) {
  return TGlXApp::GetMainForm()->CheckState( state, stateData );
}


