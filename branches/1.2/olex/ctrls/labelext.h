/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_ctrl_label_H
#define __olx_ctrl_label_H
#include "olxctrlbase.h"

namespace ctrl_ext  {
  class TLabel: public wxStaticText, public AOlxCtrl  {
    TActionQList *FActions;
    void ClickEvent(wxMouseEvent& event);
    olxstr Data;
  public:
    TLabel(wxWindow *Parent, const olxstr &label) :
      AOlxCtrl(this),
      OnClick(AOlxCtrl::ActionQueue::New(Actions, evt_on_click_id)),
      wxStaticText(Parent, wxID_ANY, label.u_str()),
      Data(EmptyString())
    {
      Bind(wxEVT_LEFT_DOWN, &TLabel::ClickEvent, this);
    }

    DefPropC(olxstr, Data)

    AOlxCtrl::ActionQueue &OnClick;

    void SetCaption(const olxstr &T) {  SetLabel(T.u_str()); }
    olxstr GetCaption() const {  return GetLabel(); }
  };
}; // end namespace ctrl_ext
#endif
