#ifndef __olx_ctrl_textedit_H
#define __olx_ctrl_textedit_H
#include "olxctrlbase.h"

namespace ctrl_ext  {

  class TTextEdit: public wxTextCtrl, public AOlxCtrl  {
  protected:
    void ClickEvent(wxMouseEvent& event);
    void ChangeEvent(wxCommandEvent& event);
    void KeyDownEvent(wxKeyEvent& event);
    void CharEvent(wxKeyEvent& event);
    void EnterPressedEvent(wxCommandEvent& event);
    void LeaveEvent(wxFocusEvent& event);
    void EnterEvent(wxFocusEvent& event);
    olxstr Data, OnChangeStr, OnLeaveStr, OnEnterStr, OnReturnStr;
  public:
    TTextEdit(wxWindow *Parent, int style=0) :
      wxTextCtrl(Parent, -1, wxString(), wxDefaultPosition, wxDefaultSize, style),
      AOlxCtrl(this),
      OnChange(Actions.New(evt_change_id)),
      OnLeave(Actions.New(evt_on_mouse_leave_id)),
      OnEnter(Actions.New(evt_on_mouse_enter_id)),
      OnReturn(Actions.New(evt_on_return_id)),
      OnClick(Actions.New(evt_on_click_id)),
      OnChar(Actions.New(evt_on_char_id)),
      OnKeyDown(Actions.New(evt_on_key_down_id)),
      Data(EmptyString()),
      OnChangeStr(EmptyString()),
      OnLeaveStr(EmptyString()),
      OnEnterStr(EmptyString()),
      OnReturnStr(EmptyString())  {}
    virtual ~TTextEdit()  {}

    olxstr GetText() const {  return wxTextCtrl::GetValue().c_str(); }
    void SetText(const olxstr &T)  {  wxTextCtrl::SetValue(T.u_str()); }

    inline bool IsReadOnly() const {   return WI.HasWindowStyle(wxTE_READONLY);  }
    inline void SetReadOnly(bool v)  {  WI.AddWindowStyle(wxTE_READONLY);  }

    DefPropC(olxstr, Data) // data associated with the object
    DefPropC(olxstr, OnChangeStr) // this is passed to the OnChange event
    DefPropC(olxstr, OnLeaveStr) // this is passed to the OnChange event
    DefPropC(olxstr, OnEnterStr) // this is passed when the control becomes focuesd
    DefPropC(olxstr, OnReturnStr) // this is passed when return key is perssed

    TActionQueue &OnChange, &OnReturn, &OnLeave, &OnEnter, 
      &OnClick, &OnChar, &OnKeyDown;

    DECLARE_CLASS(TTextEdit)
    DECLARE_EVENT_TABLE()
  };
}; // end namespace ctrl_ext
#endif
