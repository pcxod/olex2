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
#include "widgetcellext.h"
#include "htmlprep.h"

enum {
  html_parent_resize = 2
};

class THtmlManager;

class THtml : public wxHtmlWindow, public AEventsDispatcher, public AOlxCtrl,
  public THtmlPreprocessor
{
private:
  bool Movable, PageLoadRequested, ShowTooltips;
  olxdict<const IOlxObject*, int, TPointerComparator> Locks;
  olxstr PageRequested;
  static size_t &stateTooltipsVisible() {
    static size_t v=InvalidIndex;
    return v;
  }
protected:
  THtmlWidgetCell *parentCell;
  olxstr WebFolder, // the base of all web files
    FileName;
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
  void OnClipboard(wxClipboardTextEvent& event);
  olxstr OnSizeData, OnDblClickData;
  virtual bool Dispatch(int MsgId, short MsgSubId, const IOlxObject* Sender,
    const IOlxObject* Data, TActionQueue *);
  virtual void ScrollWindow(int dx, int dy, const wxRect* rect = NULL);
  virtual void DoScroll(int x_pos, int y_pos);
  // position of where the mous was down
  int MouseX, MouseY;
  bool MouseDown;

  THtmlSwitch* Root;
  olxstr_dict<AnAssociation3<AOlxCtrl*,wxWindow*,bool>, true> Objects;
  TTypeList<TStrList> Groups;
  class TObjectsState  {
    olxstr_dict<olxstr_dict<olxstr,false>*, true> Objects;
    THtml& html;
  public:
    TObjectsState(THtml& htm) : html(htm) { }
    ~TObjectsState();
    olxstr_dict<olxstr,false>* FindProperties(const olxstr& cname) {
      const size_t ind = Objects.IndexOf(cname);
      return (ind == InvalidIndex) ? NULL : Objects.GetValue(ind);
    }
    olxstr_dict<olxstr,false>* DefineControl(const olxstr& name,
      const std::type_info& type);
    void SaveState();
    void RestoreState();
    void SaveToFile(const olxstr& fn);
    bool LoadFromFile(const olxstr& fn);
  };
  TObjectsState ObjectsState;
protected:
  olxstr GetObjectValue(const AOlxCtrl *Object);
  const olxstr& GetObjectData(const AOlxCtrl *Object);
  bool GetObjectState(const AOlxCtrl *Object, const olxstr& state);
  olxstr GetObjectImage(const AOlxCtrl *Object);
  olxstr GetObjectItems(const AOlxCtrl *Object);
  void SetObjectValue(AOlxCtrl *AOlxCtrl,
    const olxstr& name, const olxstr& value);
  void SetObjectData(AOlxCtrl *AOlxCtrl, const olxstr& Data);
  void SetObjectState(AOlxCtrl *AOlxCtrl, bool State,
    const olxstr& state_name);
  bool SetObjectImage(AOlxCtrl *AOlxCtrl, const olxstr& src);
  bool SetObjectItems(AOlxCtrl *AOlxCtrl, const olxstr& src);
public:
  THtml(THtmlManager &manager, wxWindow *Parent,
    const olxstr &pop_name=EmptyString(), int flags=4);
  virtual ~THtml();

  void OnKeyDown(wxKeyEvent &event);
  void OnChar(wxKeyEvent &event);
  void OnNavigation(wxNavigationKeyEvent &event);

  int GetBorders() const {  return wxHtmlWindow::m_Borders;  }
  void SetFonts(const olxstr &normal, const olxstr &fixed);
  void GetFonts(olxstr &normal, olxstr &fixed)  {
    normal = this->NormalFont;
    fixed = this->FixedFont;
  }
  bool GetShowTooltips() const {  return ShowTooltips;  }
  void SetShowTooltips(bool v, const olxstr &html_name=EmptyString());

  bool IsPageLoadRequested() const {  return PageLoadRequested;  }
  void LockPageLoad(const IOlxObject* caller)  {
    volatile olx_scope_cs cs(TBasicApp::GetCriticalSection());
    Locks.Add(caller, 0)++;
  }
  void UnlockPageLoad(const IOlxObject* caller)  {
    volatile olx_scope_cs cs(TBasicApp::GetCriticalSection());
    const size_t pos = Locks.IndexOf(caller);
    if (pos == InvalidIndex) {
      throw TInvalidArgumentException(__OlxSourceInfo, "caller");
    }
    int lc = --Locks.GetValue(pos);
    if( lc < 0 ) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "not matching call to unlock");
    }
    if (lc == 0) {
      Locks.Delete(pos);
    }
  }
  bool IsPageLocked() const {
    volatile olx_scope_cs cs(TBasicApp::GetCriticalSection());
    return !Locks.IsEmpty();
  }

  bool ProcessPageLoadRequest();
  virtual bool Destroy();

  const olxstr& GetHomePage() const  {  return FileName;  }
  void SetHomePage(const olxstr& hp)  {  FileName = hp;  }

  bool LoadPage(const wxString &File);
  bool UpdatePage(bool update_indices=true);
  DefPropC(olxstr, WebFolder);

  THtmlSwitch& GetRoot() const {  return *Root; }
  bool ItemState(const olxstr& ItemName, short State);
  /*
  adds an object to container and return true if an existing control
  has been replaced
  */
  bool AddControl(const olxstr& Name, AOlxCtrl *Obj, wxWindow* wxObj,
    bool Manage = false);
  AOlxCtrl *FindObject(const olxstr& Name)  {
    const size_t ind = Objects.IndexOf(Name);
    return (ind == InvalidIndex) ? NULL : Objects.GetValue(ind).a;
  }
  wxWindow *FindObjectWindow(const olxstr& Name)  {
    const size_t ind = Objects.IndexOf(Name);
    return (ind == InvalidIndex) ? 0 : Objects.GetValue(ind).b;
  }
  size_t ObjectCount() const {  return Objects.Count();  }
  AOlxCtrl* GetObject(size_t i)  {  return Objects.GetValue(i).a;  }
  wxWindow* GetWindow(size_t i)  {  return Objects.GetValue(i).b;  }
  const olxstr& GetObjectName(size_t i) const {  return Objects.GetKey(i);  }
  bool IsObjectManageble(size_t i) const {
    return Objects.GetValue(i).GetC();
  }
  //
  DefPropBIsSet(Movable)

  THtmlManager &Manager;
  TActionQueue& OnURL;
  TActionQueue& OnLink;

  TActionQueue& OnDblClick, &OnSize;
  TActionQueue& OnKey;

  DefPropC(olxstr, OnSizeData)
  DefPropC(olxstr, OnDblClickData)

  THtmlWidgetCell *GetParentCell() {
    return parentCell;
  }

  wxHtmlCell *GetRootCell() {
    return m_Cell;
  }

  void SetParentCell(THtmlWidgetCell *cell) {
    parentCell = cell;
  }

  // global data for the HTML parsing....
  static olxstr &SwitchSource() {
    static olxstr src;
    return src;
  }
  static str_stack &SwitchSources() {
    static str_stack st;
    return st;
  }
  /* removes cycling refences like #onclick = #onclick or
  #onclick = #onchange, #onchange = #onclick, returns 
  */
  static void CyclicReduce(olxstr_dict<olxstr, true> &values);
  // an extention...
  class WordCell : public wxHtmlWordCell  {
  public:
    WordCell(const wxString& word, const wxDC& dc) : wxHtmlWordCell(word, dc)
    {}
    //just this extra function for managed alignment ...
    void SetDescent(int v) {  m_Descent = v;  }
  };

  friend class THtmlManager;
};

#endif
