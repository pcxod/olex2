/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_ctrl_combo_H
#define __olx_ctrl_combo_H
#include "olxctrlbase.h"
#include "itemlist.h"
#include "estrlist.h"
#include "wx/combo.h"

namespace ctrl_ext {
  class TComboBox : public TItemList<wxComboBox>, public AOlxCtrl {
    void ChangeEvent(wxCommandEvent& event);
    void EnterPressedEvent(wxCommandEvent& event);
    void LeaveEvent(wxFocusEvent& event);
    void EnterEvent(wxFocusEvent& event);
    olxstr Data;
    olxstr StrValue;
    olxstr ItemsSrc; // what SetItemsFrom filled the list from, Clear resets
    int entered_counter;
    bool OnChangeAlways;
  public:
    TComboBox(wxWindow *Parent, wxWindowID id = -1,
      const wxString &value = wxEmptyString,
      const wxPoint& pos = wxDefaultPosition,
      const wxSize& size = wxDefaultSize,
      long style = wxTE_PROCESS_ENTER);
    virtual ~TComboBox();

    void Clear();
    /* fills the list from ';' separated items, skips the refill when the
    list already came from the same string: a reused html control
    */
    void SetItemsFrom(const olxstr &src);

    bool HasValue() const {
      return !(IsReadOnly() && GetSelection() == -1);
    }

    wxString GetValue() const {
      return (HasValue() ? wxComboBox::GetValue() : wxString(wxEmptyString));
    }

    olxstr GetText() const;
    void SetText(const olxstr &text);

    DefPropC(olxstr, Data)

    bool IsReadOnly() const { return WI.HasWindowStyle(wxCB_READONLY); }
    void SetReadOnly(bool v) {
      v ? WI.AddWindowStyle(wxCB_READONLY) : WI.DelWindowStyle(wxCB_READONLY);
    }
    DefPropBIsSet(OnChangeAlways)

    void HandleOnLeave();
    void HandleOnEnter();

    ActionQueue &OnChange, &OnLeave, &OnEnter, &OnReturn;
  };
};  // end namespace ctrl_ext
#endif
