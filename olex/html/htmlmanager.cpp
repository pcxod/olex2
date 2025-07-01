/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "htmlmanager.h"
#include "htmlswitch.h"
#include "fsext.h"
#include "utf8file.h"
#include "olxstate.h"
#include "imgcellext.h"

#define this_InitFuncD(funcName, argc, desc) \
  (Library).Register(\
    new TFunction<THtmlManager>(\
      this, &THtmlManager::fun##funcName, #funcName, argc, desc))

THtmlManager::THtmlManager(wxWindow *mainWindow)
  : mainWindow(mainWindow),
    destroyed(false),
    main(0),
    OnLink(Actions.New("OnLink"))
{
  AActionHandler::SetToDelete(false);
}
//.............................................................................
THtmlManager::~THtmlManager() {
  Destroy();
}
//.............................................................................
void THtmlManager::Destroy() {
  if (destroyed) {
    return;
  }
  destroyed = true;
  if (main != 0) {
    main->OnLink.Remove(this);
    main->Destroy();
  }
  ClearPopups();
}
//.............................................................................
void THtmlManager::InitialiseMain(long flags) {
  main = new THtml(*this, mainWindow, EmptyString(), flags);
  main->OnLink.Add(this);
  main->ShowScrollbars(wxSHOW_SB_ALWAYS, wxSHOW_SB_ALWAYS);
}
//.............................................................................
void THtmlManager::ProcessPageLoadRequests() {
  if (main != 0 && main->IsPageLoadRequested() && !main->IsPageLocked()) {
    main->ProcessPageLoadRequest();
  }
  try {
    for (size_t pi=0; pi < Popups.Count(); pi++) {
      if (Popups.GetValue(pi)->Html->IsPageLoadRequested() &&
        !Popups.GetValue(pi)->Html->IsPageLocked() )
      {
        Popups.GetValue(pi)->Html->ProcessPageLoadRequest();
      }
    }
  }
  catch(const TExceptionBase &e)  {
    TBasicApp::NewLogEntry(logExceptionTrace) << e;
  }
}
//.............................................................................
void THtmlManager::ClearPopups() {
  for (size_t i = 0; i < Popups.Count(); i++) {
    Popups.GetValue(i)->Html->OnLink.Remove(this);
    Popups.GetValue(i)->Html->Destroy();
    Popups.GetValue(i)->Dialog->Destroy();
    delete Popups.GetValue(i);
  }
  Popups.Clear();
}
//.............................................................................
void THtmlManager::LockWindowDestruction(wxWindow* wnd,
  const IOlxObject* caller)
{
  if (wnd == main) {
    main->LockPageLoad(caller);
  }
  else  {
    for (size_t i=0; i < Popups.Count(); i++) {
      if (Popups.GetValue(i)->Html == wnd) {
        Popups.GetValue(i)->Html->LockPageLoad(caller);
        break;
      }
    }
  }
}
//.............................................................................
void THtmlManager::UnlockWindowDestruction(wxWindow* wnd,
  const IOlxObject* caller)
{
  if (wnd == main) {
    main->UnlockPageLoad(caller);
  }
  else {
    for (size_t i = 0; i < Popups.Count(); i++) {
      if (Popups.GetValue(i)->Html == wnd) {
        Popups.GetValue(i)->Html->UnlockPageLoad(caller);
        break;
      }
    }
  }
}
//.............................................................................
THtml* THtmlManager::FindHtml(const olxstr& name) const {
  if (name.IsEmpty()) {
    return main;
  }
  TPopupData *pd = Popups.Find(name, 0);
  if (pd == 0) {
    for (size_t i = 0; i < main->ObjectCount(); i++) {
      if (main->GetObjectName(i).Equals(name) &&
        main->GetObject(i)->Is<THtml>())
      {
        return (THtml *)main->GetObject(i);
      }
    }
    return 0;
  }
  else {
    return pd->Html;
  }
}
//.............................................................................
bool THtmlManager::Enter(const IOlxObject *sender, const IOlxObject *data,
  TActionQueue *)
{
  if (sender != 0) {
    const wxWindow *wx = dynamic_cast<const wxWindow*>(sender);
    THtml *html = dynamic_cast<THtml*>(wx->GetParent());
    if (html != 0) {
      volatile THtmlManager::DestructionLocker dm =
        LockDestruction(html, this);
      OnLink.Enter(sender, data);
      return true;
    }
  }
  OnLink.Enter(sender, data);
  return true;
}
//.............................................................................
bool THtmlManager::Exit(const IOlxObject *sender, const IOlxObject *data,
  TActionQueue *)
{
  if (sender != 0) {
    const wxWindow *wx = dynamic_cast<const wxWindow*>(sender);
    THtml *html = dynamic_cast<THtml*>(wx->GetParent());
    if (html != 0) {
      volatile THtmlManager::DestructionLocker dm =
        LockDestruction(html, this);
      OnLink.Exit(sender, data);
      return true;
    }
  }
  OnLink.Exit(sender, data);
  return true;
}
//.............................................................................
bool THtmlManager::Execute(const IOlxObject *sender, const IOlxObject *data,
  TActionQueue *)
{
  olxstr dt;
  if (data != 0 && data->Is<olxstr>()) {
    dt = *(olxstr *)data;
    size_t ind = dt.FirstIndexOf('%');
    while (ind != InvalidIndex && ((ind + 2) < dt.Length()) &&
      olxstr::o_ishexdigit(dt[ind + 1]) &&
      olxstr::o_ishexdigit(dt[ind + 2]))
    {
      int val = dt.SubString(ind + 1, 2).RadInt<int>(16);
      dt.Delete(ind, 3);
      dt.Insert(val, ind);
      ind = dt.FirstIndexOf('%');
    }
    data = &dt;
  }
  if (sender != 0) {
    const wxWindow *wx = dynamic_cast<const wxWindow*>(sender);
    THtml *html = dynamic_cast<THtml*>(wx->GetParent());
    if (html != 0) {
      volatile THtmlManager::DestructionLocker dm =
        LockDestruction(html, this);
      olxstr cmd = *(const olxstr *)data;
      if (!cmd.IsEmpty()) {
        TBasicApp::PostAction(new olxCommandAction(cmd));
        //OnLink.Execute(sender, data);
      }
      return true;
    }
  }
  OnLink.Execute(sender, data);
  return true;
}
//.............................................................................
THtmlManager::TPopupData &THtmlManager::NewPopup(TDialog *owner,
  const olxstr &name, long flags)
{
  TPopupData *pd = Popups.Find(name, 0);
  if (pd == 0) {
    pd = Popups.Add(name, new TPopupData);
    pd->Dialog = owner;
    pd->Html = new THtml(*this, owner, name, flags);
    pd->Html->OnLink.Add(this);
  }
  return *pd;
}
//.............................................................................
/*
the format is following intemstate popup_name item_name statea stateb ...
item_nameb ... if there are more than 1 state for an item the function does the
rotation if one of the states correspond to current - the next one is selected
*/
void THtmlManager::macItemState(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  THtml *html = 0;
  if (Cmds.Count() > 2 && !Cmds[1].IsNumber()) {
    html = FindHtml(Cmds[0]);
    if (html == 0) {
      Error.ProcessingError(__OlxSrcInfo,
        "undefined popup: ").quote() << Cmds[0];
      return;
    }
    Cmds.Delete(0);
  }
  else {
    html = main;
  }

  THtmlSwitch& rootSwitch = html->GetRoot();
  TIndexList states;
  olxstr itemName(Cmds[0]);
  bool changed = false;
  for (size_t i = 1; i < Cmds.Count(); i++) {
    TPtrList<THtmlSwitch> Switches;
    // special treatment of any particular index
    if (itemName.EndsWith('.')) {
      THtmlSwitch* sw =
        rootSwitch.FindSwitch(itemName.SubStringTo(itemName.Length() - 1));
      if (sw == 0) {
        return;
      }
      for (size_t j = 0; j < sw->SwitchCount(); j++) {
        Switches.Add(sw->GetSwitch(j));
      }
      //sw->Expand(Switches);
    }
    else if (itemName.FirstIndexOf('*') == InvalidIndex) {
      THtmlSwitch* sw = rootSwitch.FindSwitch(itemName);
      if (sw == 0) {
        Error.ProcessingError(__OlxSrcInfo,
          "could not locate specified switch: ").quote() << itemName;
        return;
      }
      Switches.Add(sw);
    }
    else {
      if (itemName == '*') {
        for (size_t j = 0; j < rootSwitch.SwitchCount(); j++) {
          Switches.Add(rootSwitch.GetSwitch(j));
        }
      }
      else {
        size_t sindex = itemName.FirstIndexOf('*');
        // *blabla* syntax
        if (sindex == 0 && itemName.Length() > 2 &&
          itemName.CharAt(itemName.Length() - 1) == '*')
        {
          rootSwitch.FindSimilar(itemName.SubString(
            1, itemName.Length() - 2), EmptyString(), Switches);
        }
        else {  // assuming bla*bla syntax
          olxstr from = itemName.SubStringTo(sindex), with;
          if ((sindex + 1) < itemName.Length()) {
            with = itemName.SubStringFrom(sindex + 1);
          }
          rootSwitch.FindSimilar(from, with, Switches);
        }
      }
    }
    states.Clear();
    for (size_t j = i; j < Cmds.Count(); j++, i++) {
      // is new switch encountered?
      if (!Cmds[j].IsNumber()) {
        itemName = Cmds[j];
        break;
      }
      states.Add(Cmds[j].ToInt());
    }
    if (states.Count() == 1) {  // simply change the state to the request
      for (size_t j = 0; j < Switches.Count(); j++) {
        if (Switches[j]->GetFileIndex() != states[0] - 1) {
          changed = true;
        }
        Switches[j]->SetFileIndex(states[0] - 1);
      }
    }
    else {
      for (size_t j = 0; j < Switches.Count(); j++) {
        THtmlSwitch* sw = Switches[j];
        const index_t currentState = sw->GetFileIndex();
        for (size_t k = 0; k < states.Count(); k++) {
          if (states[k] == (currentState + 1)) {
            if ((k + 1) < states.Count()) {
              if (sw->GetFileIndex() != states[k + 1] - 1) {
                changed = true;
              }
              sw->SetFileIndex(states[k + 1] - 1);
            }
            else {
              if (sw->GetFileIndex() != states[0] - 1)
                changed = true;
              sw->SetFileIndex(states[0] - 1);
            }
          }
        }
      }
    }
  }
  if (changed && !Options.GetBoolOption('u')) {
    html->UpdatePage();
  }
}
//.............................................................................
void THtmlManager::funGetItemState(const TStrObjList &Params, TMacroData &E) {
  size_t di = Params[0].IndexOf('.');
  olxstr hn = di == InvalidIndex ? EmptyString() : Params[0].SubStringTo(di);
  olxstr in = di == InvalidIndex ? Params[0] : Params[0].SubStringFrom(di + 1);
  THtml *html = di == InvalidIndex ? main : FindHtml(hn);
  if (html == 0) {
    E.ProcessingError(__OlxSrcInfo,
      "undefined html window: ").quote() << hn;
    return;
  }
  THtmlSwitch *sw = html->GetRoot().FindSwitch(in);
  if (sw == 0) {
    E.ProcessingError(__OlxSrcInfo,
      "could not locate specified switch: ").quote() << in;
    return;
  }
  if (sw->GetFileIndex() == InvalidIndex) {
    E.SetRetVal<olxstr>("-1");
  }
  else if (sw->GetFileIndex() == THtmlPreprocessor::UnknownSwitchState) {
    E.SetRetVal<olxstr>("-2");
  }
  else {
    E.SetRetVal(sw->GetFileIndex());
  }
}
//.............................................................................
void THtmlManager::funIsPopup(const TStrObjList& Params, TMacroData &E) {
  THtml *html = FindHtml(Params[0]);
  E.SetRetVal(html != 0 && html->GetParent()->IsShown());
}
//.............................................................................
void THtmlManager::funIsEnabled(const TStrObjList& Params, TMacroData &E) {
  Control c = FindControl(Params[0], E, 0, __OlxSrcInfo);
  if (c.ctrl == 0) {
    E.ProcessingError(__OlxSrcInfo, "undefined control: ").quote() << Params[0];
    return;
  }
  E.SetRetVal(c.ctrl->WI.IsEnabled());
}
//.............................................................................
void THtmlManager::funIsItem(const TStrObjList &Params, TMacroData &E) {
  THtml *html = (Params.Count() == 2) ? FindHtml(Params[0]) : main;
  if (html == 0) {
    E.ProcessingError(__OlxSrcInfo,
      "undefined html window: ").quote() << Params[0];
    return;
  }
  olxstr itemName((Params.Count() == 1) ? Params[0] : Params[1]);
  E.SetRetVal(html->GetRoot().FindSwitch(itemName) != 0);
}
//.............................................................................
void THtml::SetShowTooltips(bool v, const olxstr& html_name)  {
  ShowTooltips = v;
  TStateRegistry::GetInstance().SetState(stateTooltipsVisible(), v,
    EmptyString(), true);
}
//.............................................................................
void THtmlManager::macTooltips(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  THtml *html = main;
  olxstr hname;
  if (Cmds.Count() > 0 && !Cmds[0].IsBool()) {
    html = FindHtml(Cmds[0]);
    hname = Cmds[0];
    Cmds.Delete(0);
  }
  if (html == 0) {
    E.ProcessingError(__OlxSrcInfo,
      "undefined html window: ").quote() << hname;
    return;
  }
  if (Cmds.IsEmpty()) {
    html->SetShowTooltips(!html->GetShowTooltips());
  }
  else {
    html->SetShowTooltips(Cmds[0].ToBool());
  }
}
//.............................................................................
void THtmlManager::macUpdate(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  THtml *html = (Cmds.Count() == 1) ? FindHtml(Cmds[0]) : main;
  if (html == 0) {
    if (!Options.GetBoolOption('q')) {
      E.ProcessingError(__OlxSrcInfo,
        "undefined html window: ").quote() << (Cmds.IsEmpty() ? "main" : Cmds[0]);
    }
    return;
  }
  if (html->GetParentCell() != 0 &&
    html->GetParentCell()->GetFloatY() != 0 && false)
  {
    main->Freeze();
    html->LoadPage(html->FileName.u_str());
    wxClientDC dc(main);
    wxHtmlRenderingInfo r_info;
    main->GetRootCell()->DrawInvisible(dc, 0, 0, r_info);
    wxClientDC dc1(html);
    wxHtmlRenderingInfo r_info1;
    html->GetRootCell()->DrawInvisible(dc, 0, 0, r_info1);
    int bh = html->GetBestHeight(html->GetParentCell()->GetWidth());
    if (html->GetParentCell()->GetHeight() != bh + 20) {
      html->GetParentCell()->SetHeight(bh + 20);
      main->CreateLayout();
      main->GetRootCell()->DrawInvisible(dc, 0, 0, r_info);
    }
    main->Thaw();
  }
  else {
    html->LoadPage(html->FileName.u_str());
  }
  
}
//.............................................................................
void THtmlManager::macSetFonts(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  olxstr hn = Cmds.Count() == 2 ? EmptyString() : Cmds[0];
  THtml *html = Cmds.Count() == 3 ? FindHtml(hn) : main;
  if (html == 0) {
    Error.ProcessingError(__OlxSrcInfo,
      "undefined html window: ").quote() << hn;
    return;
  }
  if (Cmds.Count() == 3) {
    html->SetFonts(Cmds[1], Cmds[2]);
  }
  else {
    html->SetFonts(Cmds[0], Cmds[1]);
  }
}
//.............................................................................
void THtmlManager::macSetBorders(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  olxstr hn = Cmds.Count() == 1 ? EmptyString() : Cmds[0];
  THtml *html = Cmds.Count() == 2 ? FindHtml(hn) : main;
  if (html == 0) {
    E.ProcessingError(__OlxSrcInfo,
      "undefined html window: ").quote() << hn;
    return;
  }
  html->SetBorders(Cmds.GetLastString().ToInt());
}
//.............................................................................
void THtmlManager::macHome(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  olxstr hn = Cmds.Count() == 0 ? EmptyString() : Cmds[0];
  THtml *html = (Cmds.Count() == 1) ? FindHtml(Cmds[0]) : main;
  if (html == 0) {
    E.ProcessingError(__OlxSrcInfo,
      "undefined html window: ").quote() << hn;
    return;
  }
  html->LoadPage(html->GetHomePage().u_str());
}
//.............................................................................
void THtmlManager::macLoad(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  olxstr hn = Cmds.Count() == 1 ? EmptyString() : Cmds[0];
  THtml *html = (Cmds.Count() == 2) ? FindHtml(Cmds[0]) : main;
  if (html == 0) {
    E.ProcessingError(__OlxSrcInfo,
      "undefined html window: ").quote() << hn;
    return;
  }
  html->LoadPage(Cmds.GetLastString().u_str());
}
//.............................................................................
void THtmlManager::macHide(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  THtmlManager::TPopupData *html = Popups.Find(Cmds[0], NULL);
  if (html == 0) {
    //E.ProcessingError(__OlxSrcInfo, "undefined html window");
    return;
  }
  if (html->Dialog->IsShown()) {
    if (html->Dialog->IsModal()) {
      E.ProcessingError(__OlxSrcInfo,
        "Use html.EndModal to close a modal dialog!");
      return;
    }
    html->Dialog->Show(false);
  }
}
//.............................................................................
void THtmlManager::macDump(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  olxstr hn = Cmds.Count() == 1 ? EmptyString() : Cmds[0];
  THtml *html = (Cmds.Count() == 2) ? FindHtml(Cmds[0]) : main;
  if (html == 0) {
    E.ProcessingError(__OlxSrcInfo,
      "undefined html window: ").quote() << hn;
    return;
  }
  TStrList SL;
  html->GetRoot().ToStrings(SL, !Options.GetBoolOption('a'));
  TUtf8File::WriteLines(Cmds.GetLastString(), SL);
}
//.............................................................................
void THtmlManager::macDefineControl(TStrObjList &Cmds,
  const TParamList &Options, TMacroData &E)
{
  const size_t ind = Cmds[0].IndexOf('.');
  THtml* html = (ind == InvalidIndex) ? main : FindHtml(Cmds[0].SubStringTo(ind));
  olxstr objName = (ind == InvalidIndex) ? Cmds[0] : Cmds[0].SubStringFrom(ind + 1);
  if (html->ObjectsState.FindProperties(objName) != 0) {
    E.ProcessingError(__OlxSrcInfo, "control ").quote() << objName <<
      " already exists";
    return;
  }
  olxstr_dict<olxstr, false>* props = 0;
  if (Cmds[1].Equalsi("text")) {
    props = html->ObjectsState.DefineControl(objName, typeid(TTextEdit));
  }
  else if (Cmds[1].Equalsi("label")) {
    props = html->ObjectsState.DefineControl(objName, typeid(TLabel));
  }
  else if (Cmds[1].Equalsi("button")) {
    props = html->ObjectsState.DefineControl(objName, typeid(TButton));
    (*props)["image"] = Options.FindValue("i");
  }
  else if (Cmds[1].Equalsi("combo")) {
    props = html->ObjectsState.DefineControl(objName, typeid(TComboBox));
    (*props)["items"] = Options.FindValue("i");
  }
  else if (Cmds[1].Equalsi("choice")) {
    props = html->ObjectsState.DefineControl(objName, typeid(TChoice));
    (*props)["items"] = Options.FindValue("i");
  }
  else if (Cmds[1].Equalsi("spin")) {
    props = html->ObjectsState.DefineControl(objName, typeid(TSpinCtrl));
    (*props)["min"] = Options.FindValue("min", "0");
    (*props)["max"] = Options.FindValue("max", "100");
  }
  else if (Cmds[1].Equalsi("slider")) {
    props = html->ObjectsState.DefineControl(objName, typeid(TTrackBar));
    (*props)["min"] = Options.FindValue("min", "0");
    (*props)["max"] = Options.FindValue("max", "100");
  }
  else if (Cmds[1].Equalsi("checkbox")) {
    props = html->ObjectsState.DefineControl(objName, typeid(TCheckBox));
    (*props)["checked"] = Options.FindValue("c", "false");
  }
  else if (Cmds[1].Equalsi("tree")) {
    props = html->ObjectsState.DefineControl(objName, typeid(TTreeView));
  }
  else if (Cmds[1].Equalsi("list")) {
    props = html->ObjectsState.DefineControl(objName, typeid(TListBox));
    (*props)["items"] = Options.FindValue("i");
  }
  if (props != 0) {
    (*props)["bg"] = Options.FindValue("bg");
    (*props)["fg"] = Options.FindValue("fg");
    if (props->IndexOf("data") != InvalidIndex) {
      (*props)["data"] = Options.FindValue("data", EmptyString());
    }
    if (props->HasKey("val")) {
      (*props)["val"] = Options.FindValue("v");
    }
  }
}
//.............................................................................
void THtmlManager::funGetValue(const TStrObjList &Params, TMacroData &E) {
  Control c = FindControl(Params[0], E, 0, __OlxSrcInfo);
  if (c.html == 0) {
    return;
  }
  if (c.ctrl == 0) {
    olxstr_dict<olxstr, false>* props =
      c.html->ObjectsState.FindProperties(c.ctrl_name);
    if (props == 0) {
      E.ProcessingError(__OlxSrcInfo,
        "wrong html object name: ").quote() << c.ctrl_name;
      return;
    }
    if (props->IndexOf("val") == InvalidIndex) {
      E.ProcessingError(__OlxSrcInfo,
        "object definition does not have value for: ").quote() << c.ctrl_name;
      return;
    }
    E.SetRetVal((*props)["val"]);
  }
  else {
    E.SetRetVal(c.html->GetObjectValue(c.ctrl));
  }
  if (Params.Count() == 2 && E.GetRetVal().IsEmpty()) {
    E.SetRetVal(Params[1]);
  }
}
//.............................................................................
void THtmlManager::funRefresh(const TStrObjList& Params, TMacroData& E) {
  Control c = FindControl(Params[0], E, 0, __OlxSrcInfo);
  if (c.wnd != 0) {
    c.wnd->Refresh();
  }
  else if (c.ctrl != 0) {
    THtmlImageCell* ic = dynamic_cast<THtmlImageCell*>(c.ctrl);
    if (ic != 0 && c.html != 0) {
      try {
        ic->OnFileChange();
        c.html->Refresh();
      }
      catch (const TExceptionBase &e) {
        TBasicApp::NewLogEntry(logException) << e;
      }
    }
  }
}
//.............................................................................
void THtmlManager::funSetValue(const TStrObjList &Params, TMacroData &E) {
  Control c = FindControl(Params[0], E, 0, __OlxSrcInfo);
  if (c.html == 0) {
    return;
  }
  if (c.ctrl == 0) {
    olxstr_dict<olxstr, false>* props =
      c.html->ObjectsState.FindProperties(c.ctrl_name);
    if (props == 0) {
      E.ProcessingError(__OlxSrcInfo,
        "wrong html object name: ").quote() << c.ctrl_name;
      return;
    }
    if (props->IndexOf("val") == InvalidIndex) {
      E.ProcessingError(__OlxSrcInfo,
        "object definition does not accept value for: ").quote() << c.ctrl_name;
      return;
    }
    if ((*props)["type"] == EsdlClassName(TTrackBar) ||
      (*props)["type"] == EsdlClassName(TSpinCtrl))
    {
      const size_t si = Params[1].IndexOf(',');
      if (si == InvalidIndex)
        (*props)["val"] = Params[1];
      else {
        (*props)["min"] = Params[1].SubStringTo(si);
        (*props)["max"] = Params[1].SubStringFrom(si + 1);
      }
    }
    else {
      (*props)["val"] = Params[1];
    }
  }
  else {
    c.html->SetObjectValue(c.ctrl, c.ctrl_name, Params[1]);
    wxWindow *w = dynamic_cast<wxWindow *>(c.ctrl);
    if (w != 0) {
      w->Refresh();
    }
  }
}
//.............................................................................
void THtmlManager::funGetData(const TStrObjList &Params, TMacroData &E) {
  Control c = FindControl(Params[0], E, 0, __OlxSrcInfo);
  if (c.html == 0) {
    return;
  }
  if (c.ctrl == 0) {
    olxstr_dict<olxstr, false>* props =
      c.html->ObjectsState.FindProperties(c.ctrl_name);
    if (props == 0) {
      E.ProcessingError(__OlxSrcInfo,
        "wrong html object name: ").quote() << c.ctrl_name;
      return;
    }
    E.SetRetVal((*props)["data"]);
  }
  else {
    E.SetRetVal(c.html->GetObjectData(c.ctrl));
  }
}
//.............................................................................
void THtmlManager::funGetItems(const TStrObjList &Params, TMacroData &E) {
  Control c = FindControl(Params[0], E, 1, __OlxSrcInfo);
  if (c.html == 0 || c.ctrl == 0) {
    return;
  }
  E.SetRetVal(c.html->GetObjectItems(c.ctrl));
}
//.............................................................................
void THtmlManager::funSetData(const TStrObjList &Params, TMacroData &E) {
  Control c = FindControl(Params[0], E, 1, __OlxSrcInfo);
  if (c.html == 0 || c.ctrl == 0) {
    return;
  }
  c.html->SetObjectData(c.ctrl, Params[1]);
}
//.............................................................................
THtmlManager::Control THtmlManager::FindControl(const olxstr &name,
  TMacroData& me, short needs, const char* location)
{
  const size_t ind = name.IndexOf('.');
  olxstr hn = (ind == InvalidIndex) ? EmptyString() : name.SubStringTo(ind);
  THtml* html = (ind == InvalidIndex) ? main : FindHtml(name.SubStringTo(ind));
  olxstr objName = (ind == InvalidIndex) ? name : name.SubStringFrom(ind + 1);
  if (html == 0) {
    const size_t ci = main->Objects.IndexOf(name);
    if (ci != InvalidIndex) {
      return Control(main, main->GetObject(ci), main->GetWindow(ci), name);
    }
    me.ProcessingError(location,
      "could not locate specified popup/item: ").quote() << (hn << '/' << name);
    return Control(0, 0, 0, objName);
  }
  if (objName.Equals("self")) {
    if (needs == 1) { //AOlxCtrl
      me.ProcessingError(__OlxSrcInfo,
        "wrong control name: ").quote() << objName;
    }
    return Control(html, 0, main, objName);
  }
  const size_t ci = html->Objects.IndexOf(objName);
  if (ci == InvalidIndex && needs != 0) {
    me.ProcessingError(__OlxSrcInfo,
      "wrong html object name: ").quote() << objName;
  }
  AOlxCtrl* ctrl = 0;
  wxWindow* wxw = 0;
  if (ci != InvalidIndex) {
    ctrl = html->GetObject(ci);
    wxw = html->GetWindow(ci);
  }
  return Control(html, ctrl, wxw, objName);
}
//.............................................................................
void THtmlManager::funGetState(const TStrObjList &Params, TMacroData &E) {
  Control c = FindControl(Params[0], E, 0, __OlxSrcInfo);
  if (c.html == 0) {
    return;
  }
  if (c.ctrl == 0) {
    olxstr_dict<olxstr, false>* props =
      c.html->ObjectsState.FindProperties(c.ctrl_name);
    if (props == 0) {
      E.ProcessingError(__OlxSrcInfo,
        "wrong html object name: ").quote() << c.ctrl_name;
      return;
    }
    if (Params.Count() == 1) {
      E.SetRetVal((*props)["checked"]);
    }
    else {
      E.SetRetVal((*props)[Params[1]]);
    }
  }
  else {
    E.SetRetVal(c.html->GetObjectState(
      c.ctrl, Params.Count() == 1 ? EmptyString() : Params[1]));
  }
}
//.............................................................................
void THtmlManager::funGetLabel(const TStrObjList &Params, TMacroData &E) {
  Control c = FindControl(Params[0], E, 1, __OlxSrcInfo);
  if (c.html == 0 || c.ctrl == 0) {
    return;
  }
  olxstr rV;
  if (c.ctrl->Is<TButton>()) {
    rV = ((TButton*)c.ctrl)->GetCaption();
  }
  else if (c.ctrl->Is<TLabel>()) {
    rV = ((TLabel*)c.ctrl)->GetCaption();
  }
  else if (c.ctrl->Is<TCheckBox>()) {
    rV = ((TCheckBox*)c.ctrl)->GetCaption();
  }
  else if (c.ctrl->Is<TTreeView>()) {
    TTreeView* T = (TTreeView*)c.ctrl;
    wxTreeItemId ni = T->GetSelection();
    rV = T->GetItemText(ni);
  }
  else {
    E.ProcessingError(__OlxSrcInfo,
      "wrong html object type: ").quote() << EsdlObjectName(*c.ctrl);
    return;
  }
  E.SetRetVal(rV);
}
//.............................................................................
void THtmlManager::funSetLabel(const TStrObjList &Params, TMacroData &E) {
  Control c = FindControl(Params[0], E, 1, __OlxSrcInfo);
  if (c.html == 0 || c.ctrl == 0) {
    return;
  }
  if (c.ctrl->Is<TButton>()) {
    ((TButton*)c.ctrl)->SetCaption(Params[1]);
  }
  else if (c.ctrl->Is<TLabel>()) {
    ((TLabel*)c.ctrl)->SetCaption(Params[1]);
  }
  else if (c.ctrl->Is<TCheckBox>()) {
    ((TCheckBox*)c.ctrl)->SetCaption(Params[1]);
  }
  else if (c.ctrl->Is<TTreeView>()) {
    TTreeView* T = (TTreeView*)c.ctrl;
    wxTreeItemId ni = T->GetSelection();
    T->SetItemText(ni, Params[1].u_str());
  }
  else {
    E.ProcessingError(__OlxSrcInfo,
      "wrong html object type: ").quote() << EsdlObjectName(*c.ctrl);
    return;
  }
}
//.............................................................................
void THtmlManager::funCall(const TStrObjList &Params, TMacroData &E) {
  const size_t d_cnt = Params[0].CharCount('.');
  olxstr ctrl_name, evt_name;
  if (d_cnt >= 1) {
    const size_t si = Params[0].IndexOf('.');
    evt_name = Params[0].SubStringFrom(si + 1);
    ctrl_name = Params[0].SubStringTo(si);
  }
  else {
    E.ProcessingError(__OlxSrcInfo,
      "Invalid expression: ").quote() << Params[0];
    return;
  }
  Control c = FindControl(ctrl_name, E, 1, __OlxSrcInfo);
  if (c.html == NULL || c.ctrl == NULL)  return;
  volatile THtmlManager::DestructionLocker dm = LockDestruction(c.html, this);
  if (!c.ctrl->ExecuteActionQueue(evt_name))
    E.ProcessingError(__OlxSrcInfo, "Failed to execute: ").quote() << Params[0];
}
//.............................................................................
void THtmlManager::macLstObj(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  THtml* html = Cmds.Count() == 0 ? main : FindHtml(Cmds[1]);
  const olxstr &hn = Cmds.Count() == 0 ? EmptyString() : Cmds[0];
  if (html == 0) {
    E.ProcessingError(__OlxSrcInfo,
      "undefined html window: ").quote() << hn;
    return;
  }
  TStrList all(olx_reserve(html->Objects.Count()));
  for (size_t i = 0; i < html->Objects.Count(); i++)
    all.Add(html->Objects.GetKey(i));
  for (size_t i = 0; i < Popups.Count(); i++) {
    THtml &h = *Popups.GetValue(i)->Html;
    all.SetCapacity(all.Count() + h.Objects.Count());
    for (size_t j = 0; j < h.Objects.Count(); j++) {
      all.Add(Popups.GetKey(i)) << '.' << h.Objects.GetKey(j);
    }
  }
  TBasicApp::NewLogEntry() << all;
}
//.............................................................................
void THtmlManager::funSetImage(const TStrObjList &Params, TMacroData &E) {
  Control c = FindControl(Params[0], E, 1, __OlxSrcInfo);
  if (c.html == 0 || c.ctrl == 0) {
    return;
  }
  if (!c.html->SetObjectImage(c.ctrl, Params[1])) {
    E.ProcessingError(__OlxSrcInfo,
      "could not set image for the object: ").quote() << c.ctrl_name;
  }
}
//.............................................................................
void THtmlManager::funGetImage(const TStrObjList &Params, TMacroData &E) {
  Control c = FindControl(Params[0], E, 1, __OlxSrcInfo);
  if (c.html == 0 || c.ctrl == 0) {
    return;
  }
  E.SetRetVal(c.html->GetObjectImage(c.ctrl));
}
//.............................................................................
void THtmlManager::funSetFocus(const TStrObjList &Params, TMacroData &E) {
  Control c = FindControl(Params[0], E, 0, __OlxSrcInfo);
  if (c.html == 0) {
    return;
  }
  if (c.wnd == 0) { // not created yet?
    return;
  }
  if (olx_type<TTextEdit>::check(*c.wnd)) {
    ((TTextEdit*)c.wnd)->SetSelection(-1, -1);
  }
  else if (olx_type<TComboBox>::check(*c.wnd)) {
    TComboBox* cb = (TComboBox*)c.wnd;
    //cb->GetTextCtrl()->SetSelection(-1, -1);
  }
  c.wnd->SetFocus();
}
//.............................................................................
void THtmlManager::funSelect(const TStrObjList &Params, TMacroData &E) {
  bool by_label = Params.Count() == 3 ? Params[2].ToBool() : true;
  Control c = FindControl(Params[0], E, 1, __OlxSrcInfo);
  if (c.html == 0 || c.ctrl == 0) {
    return;
  }
  if (!olx_type<TTreeView>::check (*c.ctrl)) {
    E.ProcessingError(__OlxSrcInfo, "incompatible object type");
    return;
  }
  TTreeView* tv = (TTreeView*)c.ctrl;
  if (by_label) {
    tv->SelectByLabel(Params[1]);
  }
  else {
    tv->SelectByData(Params[1]);
  }
}
//.............................................................................
bool THtmlManager::SetState(const TStrObjList &Params, TMacroData &E) {
  Control c = FindControl(Params[0], E, 0, __OlxSrcInfo);
  if (c.html == 0) {
    return false;
  }
  const bool state = Params.GetLastString().ToBool();
  if (c.ctrl == 0) {
    olxstr_dict<olxstr, false>* props =
      c.html->ObjectsState.FindProperties(c.ctrl_name);
    if (props == 0) {
      E.ProcessingError(__OlxSrcInfo,
        "wrong html object name: ").quote() << c.ctrl_name;
      return false;
    }
    if (props->IndexOf("checked") == InvalidIndex) {
      E.ProcessingError(__OlxSrcInfo,
        "object definition does have state for: ").quote() << c.ctrl_name;
      return false;
    }
    if (Params.Count() == 2) {
      (*props)["checked"] = state;
    }
    else {
      (*props)[Params[1]] = state;
    }
  }
  else {
    c.html->SetObjectState(
      c.ctrl, state, (Params.Count() == 2 ? EmptyString() : Params[1]));
  }
  return true;
}
//.............................................................................
void THtmlManager::funSetState(const TStrObjList &Params, TMacroData &E)  {
  if( !SetState(Params, E) )
    return;
  //if( Params.GetLastString().ToBool() )  {
  //  TStrObjList params(Params);
  //  params.GetLastString() = FalseString();
  //  TMacroData e;
  //  for( size_t i=0; i < Groups.Count(); i++ )  {
  //    if( Groups[i].IndexOf(Params[0]) == InvalidIndex )  continue;
  //    const TStrList& group = Groups[i];
  //    for( size_t j=0; j < group.Count(); j++ )  {
  //      if( group[j].Equalsi(Params[0]) )  continue;
  //      params[0] = group[j];
  //      SetState(params, e);
  //    }
  //  }
  //}
}
//.............................................................................
void THtmlManager::funSetEnabled(const TStrObjList &Params, TMacroData &E) {
  bool enabled = Params[1].ToBool();
  if (Params[0].Contains('*')) {
    olxstr name = Params[0];
    const size_t ind = name.IndexOf('.');
    olxstr hn = (ind == InvalidIndex) ? EmptyString() : name.SubStringTo(ind);
    THtml* html = (ind == InvalidIndex) ? main : FindHtml(name.SubStringTo(ind));
    olxstr objName = (ind == InvalidIndex) ? name : name.SubStringFrom(ind + 1);
    if (html == 0) {
#ifdef _DEBUG
      TBasicApp::NewLogEntry(logError)  <<
        (olxstr("could not locate specified popup: ").quote() << hn);
#endif
      E.ClearErrorFlag();
      return;
    }
    Wildcard wc(objName);
    for (size_t i = 0; i < html->Objects.Count(); i++) {
      if (wc.DoesMatch(html->Objects.GetKey(i))) {
        html->Objects.GetValue(i).GetA()->WI.SetEnabled(enabled);
      }
    }
    return;
  }
  Control c = FindControl(Params[0], E, 1, __OlxSrcInfo);
  if (c.ctrl == 0) {
#ifdef _DEBUG
    TBasicApp::NewLogEntry(logError) <<
      (olxstr("undefined control: ").quote() << Params[0]);
#endif
    E.ClearErrorFlag();
    return;
  }
  c.ctrl->WI.SetEnabled(enabled);
}
//.............................................................................
void THtmlManager::funSetItems(const TStrObjList &Params, TMacroData &E) {
  Control c = FindControl(Params[0], E, 1, __OlxSrcInfo);
  if (c.html == 0 || c.ctrl == 0) {
    return;
  }
  c.html->SetObjectItems(c.ctrl, Params[1]);
}
//.............................................................................
void THtmlManager::funSaveData(const TStrObjList &Params, TMacroData &E) {
  olxstr hn = Params.Count() == 1 ? EmptyString() : Params[0];
  THtml* html = Params.Count() == 1 ? main : FindHtml(Params[0]);
  if (html == 0) {
    E.ProcessingError(__OlxSrcInfo,
      "undefined html window: ").quote() << hn;
    return;
  }
  olxstr &fn = Params[Params.Count() == 1 ? 0 : 1];
  html->ObjectsState.SaveToFile(fn);
}
//.............................................................................
void THtmlManager::funLoadData(const TStrObjList &Params, TMacroData &E) {
  olxstr hn = Params.Count() == 1 ? EmptyString() : Params[0];
  THtml* html = Params.Count() == 1 ? main : FindHtml(Params[0]);
  if (html == 0) {
    E.ProcessingError(__OlxSrcInfo,
      "undefined html window: ").quote() << hn;
    return;
  }
  olxstr &fn = Params[Params.Count() == 1 ? 0 : 1];
  if (!TEFile::Exists(fn)) {
    E.ProcessingError(__OlxSrcInfo, "file does not exist: ") << fn;
    return;
  }
  if (!html->ObjectsState.LoadFromFile(fn)) {
    E.ProcessingError(__OlxSrcInfo,
      "error occured while loading data from filr");
    return;
  }
}
//.............................................................................
void THtmlManager::funSetFG(const TStrObjList &Params, TMacroData &E) {
  Control c = FindControl(Params[0], E, 2, __OlxSrcInfo);
  if (c.html == 0 || c.wnd == 0) {
    return;
  }
  const wxColor fgc = wxColor(Params[1].u_str());
  c.wnd->SetForegroundColour(fgc);
  if (olx_type<TComboBox>::check(*c.wnd)) {
    TComboBox* Box = (TComboBox*)c.wnd;
    //Box->SetForegroundColour(fgc);
  }
  c.wnd->Refresh();
}
//.............................................................................
void THtmlManager::funSetBG(const TStrObjList &Params, TMacroData &E) {
  Control c = FindControl(Params[0], E, 2, __OlxSrcInfo);
  if (c.html == 0 || c.wnd == 0) {
    return;
  }
  const wxColor bgc(Params[1].u_str());
  c.wnd->SetBackgroundColour(bgc);
  if (olx_type<TComboBox>::check(*c.wnd)) {
    TComboBox* Box = (TComboBox*)c.wnd;
  }
  //else if( EsdlInstanceOf(*wxw, TTrackBar) )  {
  //  TTrackBar* Bar = (TTrackBar*)wxw;
  //}
  c.wnd->Refresh();
}
//.............................................................................
void THtmlManager::funGetFontName(const TStrObjList &Params, TMacroData &E) {
  olxstr hn = Params.IsEmpty() ? EmptyString() : Params[0];
  THtml* html = Params.IsEmpty() ? main : FindHtml(Params[0]);
  if (html == 0) {
    E.ProcessingError(__OlxSrcInfo,
      "undefined html window: ").quote() << hn;
    return;
  }
  E.SetRetVal<olxstr>(html->GetParser()->GetFontFace());
}
//.............................................................................
void THtmlManager::funGetBorders(const TStrObjList &Params, TMacroData &E) {
  olxstr hn = Params.IsEmpty() ? EmptyString() : Params[0];
  THtml* html = Params.IsEmpty() ? main : FindHtml(Params[0]);
  if (html == 0) {
    E.ProcessingError(__OlxSrcInfo,
      "undefined html window: ").quote() << hn;
    return;
  }
  E.SetRetVal(html->GetBorders());
}
//.............................................................................
void THtmlManager::funEndModal(const TStrObjList &Params, TMacroData &E) {
  THtmlManager::TPopupData *pd = Popups.Find(Params[0], 0);
  if (pd == 0) {
    E.ProcessingError(__OlxSrcInfo,
      "undefined html window: ").quote() << Params[0];
    return;
  }
  if (!pd->Dialog->IsModal()) {
    E.ProcessingError(__OlxSrcInfo,
      "non-modal html window: ").quote() << Params[0];
    return;
  }
  pd->Dialog->EndModal(Params[1].ToInt());
#if defined __MAC__ && 0
  pd->Html->OnLink.Remove(this);
  pd->Html->Destroy();
  pd->Dialog->Destroy();
  delete pd;
  Popups.Remove(Params[0]);
#endif
}
//.............................................................................
void THtmlManager::funShowModal(const TStrObjList &Params, TMacroData &E) {
  THtmlManager::TPopupData *pd = Popups.Find(Params[0], 0);
  if (pd == 0) {
    E.ProcessingError(__OlxSrcInfo,
      "undefined html window: ").quote() << Params[0];
    return;
  }
  E.SetRetVal(
    pd->Dialog->ShowModalEx(Params.Count() == 1 ? false : Params[1].ToBool()));
}
//.............................................................................
void THtmlManager::funWidth(const TStrObjList &Params, TMacroData &E) {
  Control c = FindControl(Params[0], E, 2, __OlxSrcInfo);
  if (c.html == 0 || c.wnd == 0) {
    return;
  }
  if (Params.Count() == 1) {
    E.SetRetVal(c.wnd->GetSize().GetWidth());
  }
  else {
    c.wnd->SetSize(-1, -1, Params[1].ToInt(), -1);
  }
}
//.............................................................................
void THtmlManager::funHeight(const TStrObjList &Params, TMacroData &E) {
  Control c = FindControl(Params[0], E, 2, __OlxSrcInfo);
  if (c.html == 0 || c.wnd == 0) {
    return;
  }
  if (Params.Count() == 1) {
    E.SetRetVal(c.wnd->GetSize().GetHeight());
  }
  else {
    c.wnd->SetSize(-1, -1, -1, Params[1].ToInt());
  }
}
//.............................................................................
void THtmlManager::funClientWidth(const TStrObjList &Params, TMacroData &E) {
  Control c = FindControl(Params[0], E, 2, __OlxSrcInfo);
  if (c.html == 0 || c.wnd == 0) {
    if (Params.Count() == 1) {
      E.Reset();
      E.SetRetVal(0);
    }
    return;
  }
  if (Params.Count() == 1) {
    E.SetRetVal(c.wnd->GetClientSize().GetWidth());
  }
  else {
    c.wnd->SetClientSize(Params[1].ToInt(), -1);
  }
}
//.............................................................................
void THtmlManager::funClientHeight(const TStrObjList &Params, TMacroData &E) {
  Control c = FindControl(Params[0], E, 2, __OlxSrcInfo);
  if (c.html == 0 || c.wnd == 0) {
    if (Params.Count() == 1) {
      E.Reset();
      E.SetRetVal(0);
    }
    return;
  }
  if (Params.Count() == 1) {
    E.SetRetVal(c.wnd->GetClientSize().GetHeight());
  }
  else {
    c.wnd->SetClientSize(-1, Params[1].ToInt());
  }
}
//.............................................................................
void THtmlManager::funContainerWidth(const TStrObjList &Params, TMacroData &E) {
  THtmlManager::TPopupData *pd = Popups.Find(Params[0], 0);
  if (pd == 0) {
    E.ProcessingError(__OlxSrcInfo,
      "undefined html window: ").quote() << Params[0];
    return;
  }
  if (Params.Count() == 1) {
    E.SetRetVal(pd->Dialog->GetClientSize().GetWidth());
  }
  else {
    pd->Dialog->SetClientSize(Params[1].ToInt(), -1);
  }
}
//.............................................................................
void THtmlManager::funContainerHeight(const TStrObjList &Params, TMacroData &E) {
  THtmlManager::TPopupData *pd = Popups.Find(Params[0], 0);
  if (pd == 0) {
    E.ProcessingError(__OlxSrcInfo,
      "undefined html window: ").quote() << Params[0];
    return;
  }
  if (Params.Count() == 1) {
    E.SetRetVal(pd->Dialog->GetClientSize().GetHeight());
  }
  else {
    pd->Dialog->SetClientSize(-1, Params[1].ToInt());
  }
}
//.............................................................................
void THtmlManager::macGroup(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  //bool intersects = false;
  //for( size_t i=0; i < Groups.Count(); i++ )  {
  //  for( size_t j=0; j < Cmds.Count(); j++ )  {
  //    if( Groups[i].IndexOfi(Cmds[j]) != InvalidIndex )  {
  //      intersects = true;
  //      break;
  //    }
  //  }
  //  if( intersects )  break;
  //}
  //if( intersects )  {
  //  E.ProcessingError(__OlxSrcInfo, "The group intersects with already existing one");
  //  return;
  //}
  //Groups.AddNew(Cmds);
}
//.............................................................................
void THtmlManager::funSnippet(const TStrObjList &Params,
  TMacroData &E)
{
  olx_object_ptr<IDataInputStream> is = TFileHandlerManager::GetInputStream(Params[0]);
  if (is == 0) {
    TBasicApp::NewLogEntry(logError) <<
      (olxstr("Snippet::File does not exist: ").quote() << Params[0]);
    return;
  }
  TStrList lines;
#ifdef _UNICODE
  lines = TUtf8File::ReadLines(*is, false);
#else
  lines.LoadFromTextStream(*is);
#endif
  olxstr_dict<olxstr, true> values;

  size_t data_start=0;
  for (size_t i=0; i < lines.Count(); i++) {
    if (!lines[i].StartsFrom('#')) {
      data_start = i;
      break;
    }
    size_t idx = lines[i].IndexOf('=');
    if (idx == InvalidIndex) {
      values(lines[i].SubStringFrom(1).TrimWhiteChars(), EmptyString());
    }
    else {
      olxstr name = lines[i].SubStringTo(idx).SubStringFrom(1).TrimWhiteChars();
      values(name, lines[i].SubStringFrom(idx+1));
    }
  }

  for (size_t i=1; i < Params.Count(); i++) {
    size_t idx = Params[i].IndexOf('=');
    olxstr pn, vl;
    if (idx == InvalidIndex) {
      pn = olxstr(Params[i]);
      vl = EmptyString();
    }
    else {
      pn = Params[i].SubStringTo(idx);
      vl = Params[i].SubStringFrom(idx+1);
    }
    pn.TrimWhiteChars();
    idx = values.IndexOf(pn);
    if (idx != InvalidIndex) {
      values.GetValue(idx) = vl;
    }
    else {
      values.Add(pn, vl);
    }
  }
  THtml::CyclicReduce(values);
  for (size_t i=data_start; i < lines.Count(); i++) {
    bool remove=false;
    int replaces=0;
    size_t idx=InvalidIndex;
    while (true) {
      idx = lines[i].FirstIndexOf('#', idx+1);
      if (idx == InvalidIndex) {
        break;
      }
      size_t j=idx+1;
      for (; j < lines[i].Length(); j++) {
        if (!olxstr::o_isalphanumeric(lines[i].CharAt(j))) {
          break;
        }
      }
      if (j-idx > 1) {
        olxstr pn = lines[i].SubString(idx+1, j-idx-1);
        if (values.HasKey(pn)) {
          lines[i].Delete(idx, j-idx);
          lines[i].Insert(values[pn], idx);
          idx=InvalidIndex;
          replaces++;
          remove = false;
        }
        if (replaces == 0) {
          remove = true;
        }
      }
    }
    if (remove) {
      lines.Delete(i--);
      continue;
    }
  }
  E.SetRetVal(lines.Text(NewLineSequence(), data_start));
}
//.............................................................................
TLibrary *THtmlManager::ExportLibrary(const olxstr &name) {
  TLibrary &Library = *(new TLibrary(name));
  InitMacroD(Library, THtmlManager, ItemState,
    "u-does not update the html",
    fpAny^(fpNone|fpOne),
    "Changes state of the HTML switch, accepts masks like '*-picture-*'"
  );
  InitMacroD(Library, THtmlManager, Update,
    "q-quiet if the given window does not exist",
    fpNone|fpOne,
    "Reloads the content of the main or given named HTML window"
  );
  InitMacroD(Library, THtmlManager, Home, EmptyString(), fpNone|fpOne,
    "Reloads the page"
  );
  InitMacroD(Library, THtmlManager, Load, EmptyString(), fpOne|fpTwo,
    "Loads content into main or given HTML page. Example: load name file_name"
  );
  InitMacroD(Library, THtmlManager, Dump,
    "a-do not annotate includes",
    fpOne|fpTwo,
    "Saves content of the main or given page into the file"
  );
  InitMacroD(Library, THtmlManager, Tooltips, EmptyString(),
    fpNone|fpOne|fpTwo,
    "Enables or disables tooltip for HTML. If no arguments is given the state "
    "of tooltops for the main HTML is inverted. If a single boolean argument "
    "is given - the state of the tooltips is set to the given value, if the "
    "argument is not a boolean the state of the tooltips for HTML windows with"
    " given name is inverted. If two arguments are given - the first should be"
    " an HTML window name and the second - a boolean value. This function "
    "executes the htmltt state change event"
  );
  InitMacroD(Library, THtmlManager, SetFonts, EmptyString(), fpTwo|fpThree,
    "Sets normal and fixed fonts to display HTML content"
    " [html normal_face fixed_face]");
  InitMacroD(Library, THtmlManager, SetBorders, EmptyString(), fpOne|fpTwo,
    "Sets borders between HTML content and window edges");
  InitMacroD(Library, THtmlManager, DefineControl,
    "v-value&;"
    "i-tems&;"
    "c-checked/down&;"
    "bg-background color&;"
    "fg-foreground color&;"
    "min-min value&;max-max value",
    fpTwo,
    "Defines a managed control properties"
  );
  InitMacroD(Library, THtmlManager, Hide, EmptyString(), fpOne,
    "Hides an Html popup window");
  InitMacroD(Library, THtmlManager, Group, EmptyString(),
    fpAny^(fpNone|fpOne),
    "Creates an exclusive group of buttons");
  InitMacroD(Library, THtmlManager, LstObj, EmptyString(),
    fpNone|fpOne,
    "Prints the list of available HTML objects");
  this_InitFuncD(Refresh, fpOne,
    "Updates given window");
  this_InitFuncD(GetValue, fpOne | fpTwo,
    "Returns value of specified object. When the value is empty and two params"
    " are provided - the second parameters is returned instead");
  this_InitFuncD(GetData, fpOne,
    "Returns data associated with specified object");
  this_InitFuncD(GetState, fpOne|fpTwo,
    "Returns state of the checkbox or a button. For example: echo "
    "getstate(button, enabled/down) or echo getstate(checkbox)");
  this_InitFuncD(GetLabel, fpOne,
    "Returns labels of specified object. Applicable to labels, buttons and "
    "checkboxes");
  this_InitFuncD(GetImage, fpOne,
    "Returns image source for a button or zimg");
  this_InitFuncD(GetItems, fpOne,
    "Returns items of a combobox or list");
  this_InitFuncD(SetValue, fpTwo,
    "Sets value of specified object");
  this_InitFuncD(SetData, fpTwo,
    "Sets data for specified object");
  this_InitFuncD(SetState, fpTwo|fpThree,
    "Sets state of a checkbox or a button");
  this_InitFuncD(SetLabel, fpTwo,
    "Sets labels for a label, button or checkbox");
  this_InitFuncD(SetImage, fpTwo,
    "Sets image location for a button or a zimg");
  this_InitFuncD(SetItems, fpTwo,
    "Sets items for comboboxes and lists");
  this_InitFuncD(SetEnabled, fpTwo,
    "Enables/disables the control");
  this_InitFuncD(SaveData, fpOne | fpTwo,
    "Saves state, data, label and value of all objects to a file");
  this_InitFuncD(LoadData, fpOne|fpTwo,
    "Loads previously saved data of html objects form a file");
  this_InitFuncD(SetFG, fpTwo,
    "Sets foreground of specified object");
  this_InitFuncD(SetBG, fpTwo,
    "Sets background of specified object");
  this_InitFuncD(GetFontName, fpNone|fpOne, "Returns current font name");
  this_InitFuncD(GetBorders, fpNone|fpOne,
    "Returns borders width between HTML content and window boundaries");
  this_InitFuncD(SetFocus, fpOne,
    "Sets input focus to the specified HTML control");
  this_InitFuncD(Select, fpTwo|fpThree,
    "Selects a treeview item by label (default) or data (third argument "
    "should be False)");
  this_InitFuncD(GetItemState, fpOne,
    "Returns item state of the given switch");
  this_InitFuncD(IsItem, fpOne, "Returns true if specified switch exists");
  this_InitFuncD(IsPopup, fpOne,
    "Returns true if specified popup window exists and visible");
  this_InitFuncD(IsEnabled, fpOne,
    "Returns true if specified control is enabled");
  this_InitFuncD(EndModal, fpTwo,
    "Ends a modal popup and sets the return code");
  this_InitFuncD(ShowModal, fpOne|fpTwo,
    "Shows a previously created popup window as a modal dialog");
  this_InitFuncD(Width, fpOne|fpTwo,
    "Returns/sets width of an HTML object window (use 'self' to address the "
    "window itself)");
  this_InitFuncD(Height, fpOne|fpTwo,
    "Returns/sets height of an HTML object window (use 'self' to address the"
    " window itself)");
  this_InitFuncD(ClientWidth, fpOne|fpTwo,
    "Returns/sets client width of an HTML window (use 'self' to address the "
    "window itself)");
  this_InitFuncD(ClientHeight, fpOne|fpTwo,
    "Returns/sets client height of an HTML window (use 'self' to address the"
    " window itself)");
  this_InitFuncD(ContainerWidth, fpOne|fpTwo,
    "Returns/sets width of a popup window");
  this_InitFuncD(ContainerHeight, fpOne|fpTwo,
    "Returns/sets height of a popup window");
  this_InitFuncD(Call, fpOne,
    "Calls event of specified control, expects [popup.]control.event");
  this_InitFuncD(Snippet, fpAny^fpNone,
    "Loads a file (first arg), replaces #name from name=val for following "
    "params and returns the result.");
  return &Library;
}

//.............................................................................
