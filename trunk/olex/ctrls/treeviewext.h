#ifndef __olx_ctrl_treeview_H
#define __olx_ctrl_treeview_H
#include "olxctrlbase.h"
#include "estrlist.h"
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
    olxstr Data, OnItemActivateStr, OnSelectStr;
  protected:
    void SelectionEvent(wxTreeEvent& event);
    void ItemActivateEvent(wxTreeEvent& event);

    size_t ReadStrings(size_t& index, const wxTreeItemId* thisCaller, const TStrList& strings);
    void ClearData();
  public:
    TTreeView(wxWindow* Parent) :
      wxGenericTreeCtrl(Parent), 
      AOlxCtrl(this),
      OnSelect(Actions.NewQueue(evt_on_select_id)),
      OnDblClick(Actions.NewQueue(evt_on_dbl_click_id)),
      Data(EmptyString),
      OnItemActivateStr(EmptyString),
      OnSelectStr(EmptyString)  {}
    virtual ~TTreeView()  {  ClearData();  }

    DefPropC(olxstr, Data)       // data associated with the object
    DefPropC(olxstr, OnItemActivateStr) // this is passed to the OnDoubleClick event
    DefPropC(olxstr, OnSelectStr) // this is passed to the OnSelect

    bool LoadFromStrings(const TStrList &strings);

    TActionQueue &OnDblClick, &OnSelect;

    DECLARE_CLASS(TTreeView)
    DECLARE_EVENT_TABLE()
  };
}; // end namespace ctrl_ext
#endif
