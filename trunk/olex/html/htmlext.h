/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_htmlext_H
#define __olx_htmlext_H
#include "estrlist.h"
#include "bapp.h"
#include "paramlist.h"
#include "actions.h"
#include "../wininterface.h"
#include "wx/wxhtml.h"
#include "wx/dynarray.h"
#include "library.h"
#include "edict.h"
#include "../ctrls.h"

class THtmlSwitch;

enum {
  html_parent_resize = 2
};

class THtmlManager;

class THtml: public wxHtmlWindow, public AEventsDispatcher  {
private:
  bool Movable, PageLoadRequested, ShowTooltips;
  olxdict<const IEObject*, int, TPointerComparator> Locks;
  olxstr PageRequested;
  wxWindow* InFocus;
  TActionQList Actions;
  olxstr PopupName;
protected:
  olxstr WebFolder, // the base of all web files
    FileName, HomePage;
  olxstr NormalFont, FixedFont;
  void OnLinkClicked(const wxHtmlLinkInfo& link);
  wxHtmlOpeningStatus OnOpeningURL(wxHtmlURLType type, const wxString& url,
    wxString *redirect) const;

  void OnSizeEvt(wxSizeEvent& event);
  void OnMouseDblClick(wxMouseEvent& event);
  void OnMouseDown(wxMouseEvent& event);
  void OnMouseUp(wxMouseEvent& event);
  void OnMouseMotion(wxMouseEvent& event);
  void OnCellMouseHover(wxHtmlCell *Cell, wxCoord x, wxCoord y);
  void OnChildFocus(wxChildFocusEvent& event);
  void DoHandleFocusEvent(AOlxCtrl* prev, AOlxCtrl* next);
  olxstr OnSizeData, OnDblClickData;
  virtual bool Dispatch(int MsgId, short MsgSubId, const IEObject* Sender,
    const IEObject* Data=NULL);
  /* on GTK scrolling makes mess out of the controls so will try to "fix it"
  here
  */
  void OnScroll(wxScrollEvent& evt);
  virtual void ScrollWindow(int dx, int dy, const wxRect* rect = NULL);

  // position of where the mous was down
  int MouseX, MouseY;
  bool MouseDown;

  THtmlSwitch* Root;
  olxdict<olxstr, AnAssociation3<AOlxCtrl*,wxWindow*,bool>,
    olxstrComparator<true> > Objects;
  TTypeList<AnAssociation2<AOlxCtrl*,wxWindow*> > Traversables;
  TSStrPObjList<olxstr,size_t,true> SwitchStates;
  TTypeList<TStrList> Groups;
  olxstr FocusedControl;
  class TObjectsState  {
    TSStrPObjList<olxstr,TSStrStrList<olxstr,false>*, true> Objects;
    THtml& html;
  public:
    TObjectsState(THtml& htm) : html(htm) { }
    ~TObjectsState();
    TSStrStrList<olxstr,false>* FindProperties(const olxstr& cname) {
      const size_t ind = Objects.IndexOf(cname);
      return (ind == InvalidIndex) ? NULL : Objects.GetObject(ind);
    }
    TSStrStrList<olxstr,false>* DefineControl(const olxstr& name,
      const std::type_info& type);
    void SaveState();
    void RestoreState();
    void SaveToFile(const olxstr& fn);
    bool LoadFromFile(const olxstr& fn);
  };
  TObjectsState ObjectsState;
  bool SetState(const TStrObjList &Params, TMacroError &E);
protected:
  size_t GetSwitchState(const olxstr& switchName);
  void ClearSwitchStates()  {  SwitchStates.Clear();  }
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
    DefFunc(Width)
    DefFunc(Height)
    DefFunc(ClientWidth)
    DefFunc(ClientHeight)
    DefFunc(ContainerWidth)
    DefFunc(ContainerHeight)
    DefFunc(Call)

  olxstr GetObjectValue(const AOlxCtrl *Object);
  const olxstr& GetObjectData(const AOlxCtrl *Object);
  bool GetObjectState(const AOlxCtrl *Object, const olxstr& state);
  olxstr GetObjectImage(const AOlxCtrl *Object);
  olxstr GetObjectItems(const AOlxCtrl *Object);
  void SetObjectValue(AOlxCtrl *AOlxCtrl, const olxstr& Value);
  void SetObjectData(AOlxCtrl *AOlxCtrl, const olxstr& Data);
  void SetObjectState(AOlxCtrl *AOlxCtrl, bool State,
    const olxstr& state_name);
  bool SetObjectImage(AOlxCtrl *AOlxCtrl, const olxstr& src);
  bool SetObjectItems(AOlxCtrl *AOlxCtrl, const olxstr& src);
  void _FindNext(index_t from, index_t &dest, bool scroll) const;
  void _FindPrev(index_t from, index_t &dest, bool scroll) const;
  void GetTraversibleIndeces(index_t& current, index_t& another,
    bool forward) const;
  void DoNavigate(bool forward);
  struct Control {
    THtml *html;
    AOlxCtrl *ctrl;
    wxWindow *wnd;
    olxstr ctrl_name;
    Control(THtml *h, AOlxCtrl *c, wxWindow *w, const olxstr &cn)
      : html(h), ctrl(c), wnd(w), ctrl_name(cn) {}
  };
  // needed = 0 - nothing, 1 - AOlxCtrl, 2 - wxWidow, 3 - both
  Control FindControl(const olxstr &name, TMacroError& me,
    short needed, const char* location);
public:
  THtml(THtmlManager &manager, wxWindow *Parent,
    const olxstr &pop_name=EmptyString(), int flags=4);
  virtual ~THtml();

  const olxstr &GetPopupName() const {  return PopupName;  }
  void OnKeyDown(wxKeyEvent &event);  
  void OnChar(wxKeyEvent &event);  
  void OnNavigation(wxNavigationKeyEvent &event);  

  void SetSwitchState(THtmlSwitch &sw, size_t state);

  int GetBorders() const {  return wxHtmlWindow::m_Borders;  }
  void SetFonts(const olxstr &normal, const olxstr &fixed)  {
    this->NormalFont = normal;
    this->FixedFont = fixed;
    wxHtmlWindow::SetFonts(normal.u_str(), fixed.u_str());
  }
  void GetFonts(olxstr &normal, olxstr &fixed)  {
    normal = this->NormalFont;
    fixed = this->FixedFont;
  }

  bool GetShowTooltips() const {  return ShowTooltips;  }
  void SetShowTooltips(bool v, const olxstr &html_name=EmptyString());

  bool IsPageLoadRequested() const {  return PageLoadRequested;  }
  void LockPageLoad(const IEObject* caller)  {
    volatile olx_scope_cs cs(TBasicApp::GetCriticalSection());
    Locks.Add(caller, 0)++;
  }
  void UnlockPageLoad(const IEObject* caller)  {
    volatile olx_scope_cs cs(TBasicApp::GetCriticalSection());
    const size_t pos = Locks.IndexOf(caller);
    if( pos == InvalidIndex )
      throw TInvalidArgumentException(__OlxSourceInfo, "caller");
    int lc = --Locks.GetValue(pos);
    if( lc < 0 ) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "not matching call to unlock");
    }
    if( lc == 0 )
      Locks.Delete(pos);
  }
  bool IsPageLocked() const {
    volatile olx_scope_cs cs(TBasicApp::GetCriticalSection());
    return !Locks.IsEmpty();
  }

  bool ProcessPageLoadRequest();

  const olxstr& GetHomePage() const  {  return HomePage;  }
  void SetHomePage(const olxstr& hp)  {  HomePage = hp;  }

  bool LoadPage(const wxString &File);
  bool UpdatePage();
  DefPropC(olxstr, WebFolder)

  void CheckForSwitches(THtmlSwitch& Sender, bool IsZip);
  void UpdateSwitchState(THtmlSwitch& Switch, olxstr& String);
  THtmlSwitch& GetRoot() const {  return *Root; }
  bool ItemState(const olxstr& ItemName, short State);
  // object operations
  bool AddObject(const olxstr& Name, AOlxCtrl *Obj, wxWindow* wxObj,
    bool Manage = false);
  AOlxCtrl *FindObject(const olxstr& Name)  {
    const size_t ind = Objects.IndexOf(Name);
    return (ind == InvalidIndex) ? NULL : Objects.GetValue(ind).A();
  }
  wxWindow *FindObjectWindow(const olxstr& Name)  {
    const size_t ind = Objects.IndexOf(Name);
    return (ind == InvalidIndex) ? NULL : Objects.GetValue(ind).B();
  }
  size_t ObjectCount() const {  return Objects.Count();  }
  AOlxCtrl* GetObject(size_t i)  {  return Objects.GetValue(i).A();  }
  wxWindow* GetWindow(size_t i)  {  return Objects.GetValue(i).B();  }
  const olxstr& GetObjectName(size_t i) const {  return Objects.GetKey(i);  }
  bool IsObjectManageble(size_t i) const {
    return Objects.GetValue(i).GetC();
  }
  TLibrary* ExportLibrary(const olxstr &name="html");

  //
  DefPropBIsSet(Movable)

  THtmlManager &Manager;
  TActionQueue& OnURL;
  TActionQueue& OnLink;

  TActionQueue& OnDblClick, &OnSize;
  TActionQueue& OnKey;

  DefPropC(olxstr, OnSizeData)
  DefPropC(olxstr, OnDblClickData)

  TWindowInterface WI;
  // global data for the HTML parsing....
  static olxstr SwitchSource;
  static str_stack SwitchSources;
  // an extention...
  class WordCell : public wxHtmlWordCell  {
  public:
    WordCell(const wxString& word, const wxDC& dc) : wxHtmlWordCell(word, dc)
    {}
    //just this extra function for managed alignment ...
    void SetDescent(int v) {  m_Descent = v;  }
  };

  DECLARE_EVENT_TABLE()
};

class THtmlManager : public AActionHandler {
  wxWindow *mainWindow;
  TActionQList Actions;
  virtual bool Enter(const IEObject *, const IEObject *);
  virtual bool Exit(const IEObject *, const IEObject *);
  virtual bool Execute(const IEObject *, const IEObject *);
public:
  THtml *main;
  struct TPopupData  {
    TDialog *Dialog;
    THtml *Html;
  };
  struct DestructionLocker {
    THtmlManager &manager;
    wxWindow *wnd;
    IEObject *sender;
    DestructionLocker(THtmlManager &manager, wxWindow *wnd, IEObject *sender)
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
  void InitialiseMain(long flags);
  void ProcessPageLoadRequests();
  void ClearPopups();
  DestructionLocker LockDestruction(wxWindow *wnd, IEObject *sender) {
    return DestructionLocker(*this, wnd, sender);
  }
  void LockWindowDestruction(wxWindow* wnd, const IEObject* caller);
  void UnlockWindowDestruction(wxWindow* wnd, const IEObject* caller);
  THtml* FindHtml(const olxstr& name) const;
  TPopupData &NewPopup(TDialog *owner, const olxstr &name, long flags=4);
  olxstr_dict<TPopupData*, true> Popups;
  TActionQueue &OnStateChange,
    &OnModeChange,
    &OnLink;

};

#endif
