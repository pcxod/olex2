#ifndef __olx_ctrl_btn_H
#define __olx_ctrl_btn_H
#include "olxctrlbase.h"

namespace ctrl_ext  {

  class AButtonBase: public AActionHandler, public AOlxCtrl  {
    bool Down;
  protected:
    void ClickEvent(wxCommandEvent& event);
    olxstr OnClickStr, Data, OnUpStr, OnDownStr, DependMode, Hint;
    TActionQueue *ActionQueue;
    virtual wxWindow* GetParent()  const  = 0;
  public:
    AButtonBase(wxWindow* this_wnd) :
      AOlxCtrl(this_wnd),
      OnClick(Actions.NewQueue(evt_on_click_id)),
      OnUp(Actions.NewQueue(evt_on_uncheck_id)),
      OnDown(Actions.NewQueue(evt_on_check_id)),
      Down(false),
      ActionQueue(NULL),
      OnClickStr(EmptyString),
      OnUpStr(EmptyString),
      OnDownStr(EmptyString),
      DependMode(EmptyString),
      Hint(EmptyString),
      Data(EmptyString)  {  SetToDelete(false); }
    virtual ~AButtonBase() {  if( ActionQueue != NULL )  ActionQueue->Remove(this);  }

    void SetActionQueue(TActionQueue *q, const olxstr &dependMode);
    bool Execute(const IEObject *Sender, const IEObject *Data);
    void OnRemove()  {  ActionQueue = NULL;  }

    DefPropC(olxstr, OnClickStr) // passed to OnClick event
    DefPropC(olxstr, Data)       // data associated with object
    DefPropC(olxstr, OnUpStr)   // passed to OnUp event
    DefPropC(olxstr, OnDownStr) // passed to OnDown event
    DefPropC(olxstr, Hint) // passed to OnDown event

    bool IsDown() const {  return Down;  }
    void SetDown(bool v);

    TActionQueue &OnClick, &OnUp, &OnDown;
  };

  class TButton: public wxButton, public AButtonBase {
    void MouseEnterEvent(wxMouseEvent& event);
    void MouseLeaveEvent(wxMouseEvent& event);
    void ClickEvent(wxCommandEvent& event)  {  AButtonBase::ClickEvent(event);  }
  protected:
    virtual wxWindow* GetParent()  const  {  return wxButton::GetParent();  }
  public:
    TButton(wxWindow* parent, wxWindowID id=-1, const wxString& label = wxEmptyString, const wxPoint& pos = wxDefaultPosition, 
      const wxSize& size = wxDefaultSize, long style = 0) :
        wxButton(parent, id, label, pos, size, style),
        AButtonBase(this)  {}
    virtual ~TButton() {}

    void SetCaption(const olxstr &l)  {  wxButton::SetLabel(l.u_str());  }
    olxstr GetCaption() const { return wxButton::GetLabel().c_str();  }

    DECLARE_CLASS(TButton)
    DECLARE_EVENT_TABLE()
  };

  class TBmpButton: public wxBitmapButton, public AButtonBase  {
    void MouseEnterEvent(wxMouseEvent& event);
    void MouseLeaveEvent(wxMouseEvent& event);
    void ClickEvent(wxCommandEvent& event)  {  AButtonBase::ClickEvent(event);  }
    olxstr Source;
  public:
    virtual wxWindow* GetParent()  const  {  return wxBitmapButton::GetParent();  }
  public:
    TBmpButton(wxWindow* parent, wxWindowID id=-1, const wxBitmap& bitmap=wxNullBitmap, const wxPoint& pos = wxDefaultPosition, 
      const wxSize& size = wxDefaultSize, long style = wxBU_AUTODRAW) :
        wxBitmapButton(parent, -1, bitmap, pos, size, style),
        AButtonBase(this)  {}
    virtual ~TBmpButton() {}

    DefPropC(olxstr, Source)

    DECLARE_CLASS(TBmpButton)
    DECLARE_EVENT_TABLE()
  };

}; //end namespace ctrl_ext
#endif
