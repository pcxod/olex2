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
  enum {
    ID_TREE_ExpandAll = 1000,
    ID_TREE_CollapseAll,
    ID_TREE_LAST, // custom menu ids start here
  };

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
    void ShowContextMenu(wxCommandEvent& event);
    void OnItemContextMenu(wxCommandEvent& event);
    void OnMouseUp(wxMouseEvent& event);
    size_t ReadStrings(size_t& index, const wxTreeItemId* thisCaller,
      const TStrList& strings);
    void ClearData();
    olx_object_ptr<wxMenu> contextMenu;
    // returns index of the selected item...
    size_t _SaveState(TEBitArray& res, const wxTreeItemId& item,
      size_t& counter) const;
    void _RestoreState(const TEBitArray& res, const wxTreeItemId& item,
      size_t& counter, size_t selected);
    wxTreeItemId _FindByLabel(const wxTreeItemId& root,
      const olxstr& label) const;
    wxTreeItemId _FindByData(const wxTreeItemId& root,
      const olxstr& data) const;
    class MenuData : public wxClientData {
    public:
      olx_pdict<int, olxstr> macros;
    };
  public:
    TTreeView(wxWindow* Parent, wxWindowID id = -1,
      const wxPoint& pos = wxDefaultPosition,
      const wxSize& size = wxDefaultSize,
      long flags = (wxTR_HAS_BUTTONS | wxTR_LINES_AT_ROOT));

    virtual ~TTreeView()  {
      ClearData();
    }

    DefPropC(olxstr, Data);
    void SetContextMenu(wxMenu*);

    void SelectByLabel(const olxstr& label);
    void SelectByData(const olxstr& data);

    bool LoadFromStrings(const TStrList& strings);
    olxstr SaveState() const;
    void RestoreState(const olxstr& state);

    AOlxCtrl::ActionQueue &OnDblClick, &OnSelect, &OnEdit;

    static wxMenu* CreateDefaultContextMenu();
    static wxMenu* CreateContextMenu(const olxstr& def);
  };
}; // end namespace ctrl_ext
#endif
