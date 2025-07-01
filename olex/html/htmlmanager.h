/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_htmlmanager_H
#define __olx_htmlmanager_H
#include "htmlext.h"

class THtmlManager : public AActionHandler {
  wxWindow *mainWindow;
  TActionQList Actions;
  virtual bool Enter(const IOlxObject *, const IOlxObject *, TActionQueue *);
  virtual bool Exit(const IOlxObject *, const IOlxObject *, TActionQueue *);
  virtual bool Execute(const IOlxObject *, const IOlxObject *, TActionQueue *);
  bool destroyed;
protected:
  // library
  DefMacro(ItemState)
  DefMacro(Update)
  DefMacro(Home)
  DefMacro(Load)
  DefMacro(Dump)
  DefMacro(Tooltips)
  DefMacro(SetFonts)
  DefMacro(SetBorders)
  DefMacro(DefineControl)
  DefMacro(Hide)
  DefMacro(Group)
  DefMacro(LstObj)

  DefFunc(Refresh)
  DefFunc(GetValue)
  DefFunc(GetData)
  DefFunc(GetLabel)
  DefFunc(GetImage)
  DefFunc(GetState)
  DefFunc(GetItems)
  DefFunc(SetValue)
  DefFunc(SetData)
  DefFunc(SetLabel)
  DefFunc(SetImage)
  DefFunc(SetItems)
  DefFunc(SetState)
  DefFunc(SetEnabled)
  DefFunc(SetFG)
  DefFunc(SetBG)
  DefFunc(GetFontName)
  DefFunc(GetBorders)
  DefFunc(SetFocus)
  DefFunc(Select)
  DefFunc(EndModal)
  DefFunc(ShowModal)

  DefFunc(SaveData)
  DefFunc(LoadData)
  DefFunc(GetItemState)
  DefFunc(IsItem)
  DefFunc(IsPopup)
  DefFunc(IsEnabled)
  DefFunc(Width)
  DefFunc(Height)
  DefFunc(ClientWidth)
  DefFunc(ClientHeight)
  DefFunc(ContainerWidth)
  DefFunc(ContainerHeight)
  DefFunc(Call)
  DefFunc(Snippet)
  struct Control {
    THtml *html;
    AOlxCtrl *ctrl;
    wxWindow *wnd;
    olxstr ctrl_name;
    Control(THtml *h, AOlxCtrl *c, wxWindow *w, const olxstr &cn)
      : html(h), ctrl(c), wnd(w), ctrl_name(cn) {}
  };
  // needed = 0 - nothing, 1 - AOlxCtrl, 2 - wxWidow, 3 - both
  Control FindControl(const olxstr &name, TMacroData& me,
    short needed, const char* location);
  bool SetState(const TStrObjList &Params, TMacroData &E);
public:
  THtml *main;
  struct TPopupData  {
    TDialog *Dialog;
    THtml *Html;
  };
  struct DestructionLocker {
    THtmlManager &manager;
    wxWindow *wnd;
    IOlxObject *sender;
    DestructionLocker(THtmlManager &manager, wxWindow *wnd, IOlxObject *sender)
      : manager(manager), wnd(wnd), sender(sender)
    {
      manager.LockWindowDestruction(wnd, sender);
    }
    ~DestructionLocker() {
      manager.UnlockWindowDestruction(wnd, sender);
    }
  };
  THtmlManager(wxWindow *mainWindow);
  ~THtmlManager();
  void Destroy();
  void InitialiseMain(long flags);
  void ProcessPageLoadRequests();
  void ClearPopups();
  DestructionLocker LockDestruction(wxWindow *wnd, IOlxObject *sender) {
    return DestructionLocker(*this, wnd, sender);
  }
  void LockWindowDestruction(wxWindow* wnd, const IOlxObject* caller);
  void UnlockWindowDestruction(wxWindow* wnd, const IOlxObject* caller);
  THtml* FindHtml(const olxstr& name) const;
  TPopupData &NewPopup(TDialog *owner, const olxstr &name, long flags=4);
  TLibrary* ExportLibrary(const olxstr &name="html");

  olxstr_dict<TPopupData*, true> Popups;
  TActionQueue &OnLink;
};

#endif
