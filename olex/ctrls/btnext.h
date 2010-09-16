#ifndef __olx_ctrl_btn_H
#define __olx_ctrl_btn_H
#include "olxctrlbase.h"

namespace ctrl_ext  {

  class AButtonBase: public AActionHandler, public AOlxCtrl  {
    bool Down;
  protected:
    void ClickEvent();
    olxstr OnClickStr, Data, OnUpStr, OnDownStr, DependMode, Hint;
    TActionQueue *ActionQueue;
    virtual wxWindow* GetParent()  const  = 0;
  public:
    AButtonBase(wxWindow* this_wnd) :
      AOlxCtrl(this_wnd),
      OnClick(Actions.New(evt_on_click_id)),
      OnUp(Actions.New(evt_on_uncheck_id)),
      OnDown(Actions.New(evt_on_check_id)),
      Down(false),
      ActionQueue(NULL),
      OnClickStr(EmptyString),
      OnUpStr(EmptyString),
      OnDownStr(EmptyString),
      DependMode(EmptyString),
      Hint(EmptyString),
      Data(EmptyString)  {  SetToDelete(false);  }
    virtual ~AButtonBase() {  if( ActionQueue != NULL )  ActionQueue->Remove(this);  }

    void SetActionQueue(TActionQueue& q, const olxstr& dependMode);
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
    void ClickEvent(wxCommandEvent&)  {  AButtonBase::ClickEvent();  }
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
    void ClickEvent(wxCommandEvent&)  {  AButtonBase::ClickEvent();  }
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

  class TImgButton : public wxPanel, public AButtonBase  {
  public:
    const static short
      stUp       = 0x0001,
      stDown     = 0x0002,
      stDisabled = 0x0004,
      stHover    = 0x0008;
  private:
    short state;
    bool MouseIn, ProcessingOnDown;
    wxBitmap bmpDown, bmpUp, bmpDisabled, bmpHover;
  protected:
    virtual wxWindow* GetParent()  const  {  return wxPanel::GetParent();  }
    const wxBitmap& ChooseBitmap() const;
    wxBitmap BmpFromImage(const wxImage& img, int w, int h) const;
    void ClickEvent(wxCommandEvent&)  {  AButtonBase::ClickEvent();  }
  public:
    TImgButton(wxWindow* parent);

    void PaintEvent(wxPaintEvent & evt)  {
      wxPaintDC dc(this);
      Render(dc);
    }
    void Paint()  {
      wxClientDC dc(this);
      Render(dc);
    }
    bool IsEnabled() const {  return state != stDisabled;  }
    void SetEnabled(bool v)  {  state = (v ? stUp : stDisabled);  }
    void Render(wxDC& dc) const;
    void SetImages(const TTypeList<wxImage>& images, short imgState, int w=-1, int h=-1);
    void MouseDownEvent(wxMouseEvent& event);
    void MouseUpEvent(wxMouseEvent& event);
    void MouseMoveEvent(wxMouseEvent& event);
    void MouseEnterEvent(wxMouseEvent& event);
    void MouseLeaveEvent(wxMouseEvent& event);
        
    DECLARE_EVENT_TABLE()
  };
}; //end namespace ctrl_ext
#endif
