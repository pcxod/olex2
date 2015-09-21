/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_ctrl_choice_H
#define __olx_ctrl_choice_H
#include "olxctrlbase.h"
#include "itemlist.h"
#include "estrlist.h"
#include "wx/choice.h"

namespace ctrl_ext {
  class TChoice : public TItemList<wxChoice>, public AOlxCtrl {
    void ChangeEvent(wxCommandEvent& event);
    void EnterEvent(wxFocusEvent& event);
    void LeaveEvent(wxFocusEvent& event);
    olxstr Data;
    olxstr StrValue;
    int entered_counter;
    bool OnChangeAlways, hasDefault;
  public:
    TChoice(wxWindow *Parent, wxWindowID id = -1,
      const wxPoint& pos = wxDefaultPosition,
      const wxSize& size = wxDefaultSize, long style = 0);
    virtual ~TChoice();

    void Clear();

    wxString GetValue() const {
      return (GetSelection() != wxNOT_FOUND ?
        wxChoice::GetString(GetSelection()) : wxString(wxEmptyString));
    }

    olxstr GetText() const;
    void SetText(const olxstr &text);
    void AddItems(const TStrList &items) {
      TItemList<wxChoice>::AddItems(items);
      if (HasDefault() && !IsEmpty()) {
        SetSelection(0);
      }
    }

    DefPropC(olxstr, Data)

    DefPropBIsSet(OnChangeAlways)

    bool HasDefault() const { return hasDefault; }
    void SetHasDefault(bool v) { hasDefault = v; }

    void HandleOnLeave();
    void HandleOnEnter();

    ActionQueue &OnChange, &OnLeave, &OnEnter;
  };
};  // end namespace ctrl_ext
#endif
