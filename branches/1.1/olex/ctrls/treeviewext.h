#ifndef __olx_ctrl_treeview_H
#define __olx_ctrl_treeview_H
#include "olxctrlbase.h"
#include "estrlist.h"
#include "bitarray.h"
#include "wx/treectrl.h"
#include "wx/generic/treectlg.h"

namespace ctrl_ext  {

  class TTreeNodeData : public wxTreeItemData, public IEObject {
    IEObject* Data;
  public:
    TTreeNodeData(IEObject* obj) {  Data = obj;  }
    virtual ~TTreeNodeData()  { delete Data;  }
    inline IEObject* GetData() const {  return Data;  }
  };

  class TTreeView: public wxGenericTreeCtrl, public AOlxCtrl  {
    olxstr Data, OnItemActivateStr, OnSelectStr, OnEditStr;
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
      wxGenericTreeCtrl(Parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, flags), 
      AOlxCtrl(this),
      Popup(NULL),
      OnSelect(Actions.New(evt_on_select_id)),
      OnDblClick(Actions.New(evt_on_dbl_click_id)),
      OnEdit(Actions.New(evt_change_id)),
      Data(EmptyString()),
      OnItemActivateStr(EmptyString()),
      OnSelectStr(EmptyString())  {}
    virtual ~TTreeView()  {
      ClearData();
      if( Popup != NULL )  delete Popup;
    }

    DefPropC(olxstr, Data)       // data associated with the object
    DefPropC(olxstr, OnItemActivateStr) // this is passed to the OnDoubleClick event
    DefPropC(olxstr, OnSelectStr) // this is passed to the OnSelect
    DefPropC(olxstr, OnEditStr) // this is passed to the OnEdit
    DefPropP(wxMenu*, Popup)

    void SelectByLabel(const olxstr& label);
    void SelectByData(const olxstr& data);

    bool LoadFromStrings(const TStrList& strings);
    olxstr SaveState() const;
    void RestoreState(const olxstr& state);

    TActionQueue &OnDblClick, &OnSelect, &OnEdit;

    DECLARE_CLASS(TTreeView)
    DECLARE_EVENT_TABLE()
  };
}; // end namespace ctrl_ext
#endif
