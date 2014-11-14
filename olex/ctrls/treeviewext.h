/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_ctrl_treeview_H
#define __olx_ctrl_treeview_H
#include "olxctrlbase.h"
#include "estrlist.h"
#include "bitarray.h"
#include "wx/treectrl.h"
#include "wx/generic/treectlg.h"

namespace ctrl_ext  {

  class TTreeNodeData : public wxTreeItemData, public IOlxObject {
    IOlxObject* Data;
  public:
    TTreeNodeData(IOlxObject* obj) {  Data = obj;  }
    virtual ~TTreeNodeData()  { delete Data;  }
    inline IOlxObject* GetData() const {  return Data;  }
  };

  class TTreeView: public wxTreeCtrl, public AOlxCtrl  {
    olxstr Data;
  protected:
    void SelectionEvent(wxTreeEvent& event);
    void ItemActivateEvent(wxTreeEvent& event);
    void ItemEditEvent(wxTreeEvent& event);
    void OnMouseUp(wxMouseEvent& event);
    void OnContextMenu(wxCommandEvent& event);
    size_t ReadStrings(size_t& index, const wxTreeItemId* thisCaller, const TStrList& strings);
    void ClearData();
    wxMenu* Popup;
    // returns index of the selected item...
    size_t _SaveState(TEBitArray& res, const wxTreeItemId& item, size_t& counter) const;
    void _RestoreState(const TEBitArray& res, const wxTreeItemId& item, size_t& counter, size_t selected);
    wxTreeItemId _FindByLabel(const wxTreeItemId& root, const olxstr& label) const;
    wxTreeItemId _FindByData(const wxTreeItemId& root, const olxstr& data) const;
  public:
    TTreeView(wxWindow* Parent, long flags=(wxTR_HAS_BUTTONS|wxTR_LINES_AT_ROOT)) :
      wxTreeCtrl(Parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, flags),
      AOlxCtrl(this),
      Popup(NULL),
      OnSelect(AOlxCtrl::ActionQueue::New(Actions, evt_on_select_id)),
      OnDblClick(AOlxCtrl::ActionQueue::New(Actions, evt_on_dbl_click_id)),
      OnEdit(AOlxCtrl::ActionQueue::New(Actions, evt_change_id))  {}

    virtual ~TTreeView()  {
      ClearData();
      if( Popup != NULL )  delete Popup;
    }

    DefPropC(olxstr, Data)
    DefPropP(wxMenu*, Popup)

    void SelectByLabel(const olxstr& label);
    void SelectByData(const olxstr& data);

    bool LoadFromStrings(const TStrList& strings);
    olxstr SaveState() const;
    void RestoreState(const olxstr& state);

    AOlxCtrl::ActionQueue &OnDblClick, &OnSelect, &OnEdit;

    DECLARE_CLASS(TTreeView)
    DECLARE_EVENT_TABLE()
  };
}; // end namespace ctrl_ext
#endif
