/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_ctrl_btn_H
#define __olx_ctrl_btn_H
#include "olxctrlbase.h"

namespace ctrl_ext {

  class AButtonBase: public AActionHandler, public AOlxCtrl {
    bool Down;
  protected:
    void _ClickEvent();
    olxstr Data, DependMode, Hint;
    TActionQueue *ActionQueue;
    virtual wxWindow* GetParent() const = 0;
    void _SetDown(bool v)  {  Down = v;  }
  public:
    AButtonBase(wxWindow* this_wnd) :
      AOlxCtrl(this_wnd),
      OnClick(AOlxCtrl::ActionQueue::New(Actions, evt_on_click_id)),
      OnUp(AOlxCtrl::ActionQueue::New(Actions, evt_on_uncheck_id)),
      OnDown(AOlxCtrl::ActionQueue::New(Actions, evt_on_check_id)),
      Down(false),
      ActionQueue(0)
    {
      SetToDelete(false);
    }
    virtual ~AButtonBase() {
      if (ActionQueue != 0) {
        ActionQueue->Remove(this);
      }
    }

    void SetActionQueue(TActionQueue& q, const olxstr& dependMode);
    bool Execute(const IOlxObject *, const IOlxObject *, TActionQueue *);
    void OnRemove(TActionQueue *) { ActionQueue = 0; }

    DefPropC(olxstr, Data)
    DefPropC(olxstr, Hint)

    bool IsDown() const { return Down; }
    void SetDown(bool v);

    AOlxCtrl::ActionQueue &OnClick, &OnUp, &OnDown;
  };

  class TButton: public wxButton, public AButtonBase {
    void MouseEnterEvent(wxMouseEvent& event);
    void MouseLeaveEvent(wxMouseEvent& event);
    void ClickEvent(wxCommandEvent&);
    void PaintEvent(wxPaintEvent&);
    olxstr_dict<olxstr> drawParams;
  protected:
    virtual wxWindow* GetParent() const {  return wxButton::GetParent();  }
  public:
    TButton(wxWindow* parent, wxWindowID id = -1,
      const wxString& label = wxEmptyString,
      const wxPoint& pos = wxDefaultPosition,
      const wxSize& size = wxDefaultSize, long style = 0);

    void SetCaption(const olxstr &l)  {  wxButton::SetLabel(l.u_str());  }
    olxstr GetCaption() const { return wxButton::GetLabel();  }
    void SetCustomDrawParams(const olxstr &params) {
      drawParams = MapFromToks(params);
    }
  };

  class TBmpButton: public wxBitmapButton, public AButtonBase  {
    void MouseEnterEvent(wxMouseEvent& event);
    void MouseLeaveEvent(wxMouseEvent& event);
    void ClickEvent(wxCommandEvent&)  {  AButtonBase::_ClickEvent();  }
    olxstr Source;
  public:
    virtual wxWindow* GetParent() const { return wxBitmapButton::GetParent(); }
  public:
    TBmpButton(wxWindow* parent, wxWindowID id = -1,
      const wxBitmap& bitmap = wxNullBitmap,
      const wxPoint& pos = wxDefaultPosition,
      const wxSize& size = wxDefaultSize, long style = wxBU_AUTODRAW);

    DefPropC(olxstr, Source)
  };

  class TImgButton : public wxPanel, public AButtonBase  {
  public:
    const static short
      stUp       = 0x0001,
      stDown     = 0x0002,
      stDisabled = 0x0004,
      stHover    = 0x0008;
  private:
    short state, width, height;
    bool MouseIn, ProcessingOnDown;
    wxBitmap bmpDown, bmpUp, bmpDisabled, bmpHover;
    olxstr Source;
  protected:
    virtual wxWindow* GetParent() const {  return wxPanel::GetParent();  }
    const wxBitmap& ChooseBitmap() const;
    wxBitmap BmpFromImage(const wxImage& img, int w, int h) const;
    void ClickEvent(wxCommandEvent&)  {  AButtonBase::_ClickEvent();  }
    void EraseBGEvent(wxEraseEvent&)  {}
    void SetImages(const TTypeList<wxImage>&, short , int w=-1, int h=-1);
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
    // swaps Up and Down images if v is different to Down state...
    void SetDown(bool v);
    const olxstr& GetSource() const {  return Source;  }
    short GetWidth() const {  return width;  }
    short GetHeight() const {  return height;  }
    void Render(wxDC& dc) const;
    void SetImages(const olxstr& src, int w=-1, int h=-1);
    void MouseDownEvent(wxMouseEvent& event);
    void MouseUpEvent(wxMouseEvent& event);
    void MouseMoveEvent(wxMouseEvent& event);
    void MouseEnterEvent(wxMouseEvent& event);
    void MouseLeaveEvent(wxMouseEvent& event);
  };
}; //end namespace ctrl_ext
#endif
