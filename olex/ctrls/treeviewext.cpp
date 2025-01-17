/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "treeviewext.h"
#include "frameext.h"
#include "bapp.h"
#include "integration.h"

using namespace ctrl_ext;

//..............................................................................
TTreeView::TTreeView(wxWindow* Parent, wxWindowID id,
const wxPoint& pos, const wxSize& size, long flags)
: wxTreeCtrl(Parent, id, pos, size, flags),
  AOlxCtrl(this),
  OnSelect(AOlxCtrl::ActionQueue::New(Actions, evt_on_select_id)),
  OnDblClick(AOlxCtrl::ActionQueue::New(Actions, evt_on_dbl_click_id)),
  OnEdit(AOlxCtrl::ActionQueue::New(Actions, evt_change_id))
{
  Bind(wxEVT_TREE_ITEM_ACTIVATED, &TTreeView::ItemActivateEvent, this);
  Bind(wxEVT_TREE_SEL_CHANGED, &TTreeView::SelectionEvent, this);
  Bind(wxEVT_TREE_END_LABEL_EDIT, &TTreeView::ItemEditEvent, this);
  Bind(wxEVT_TREE_ITEM_MENU, &TTreeView::ShowContextMenu, this);

  Bind(wxEVT_LEFT_UP, &TTreeView::OnMouseUp, this);
  Bind(wxEVT_RIGHT_UP, &TTreeView::OnMouseUp, this);
}
//..............................................................................
void TTreeView::ItemActivateEvent(wxTreeEvent& event) {
  event.Skip();
  OnDblClick.Execute(this);
}
//..............................................................................
void TTreeView::SelectionEvent(wxTreeEvent& event) {
  event.Skip();
  OnSelect.Execute(this);
}
//..............................................................................
void TTreeView::ItemEditEvent(wxTreeEvent& event) {
  event.Skip();
  olxstr d = olxstr(OnEdit.data).Replace("~label~", event.GetLabel());
  OnEdit.Execute(this, &d);
}
//..............................................................................
void TTreeView::ShowContextMenu(wxCommandEvent& event) {
  if (contextMenu.ok()) {
    PopupMenu(&contextMenu);
  }
}
//..............................................................................
size_t TTreeView::ReadStrings(size_t& index, const wxTreeItemId* thisCaller,
  const TStrList& strings)
{
  while ((index + 2) <= strings.Count()) {
    size_t level = strings[index].LeadingCharCount('\t');
    index++;  // now index is on data string
    wxTreeItemId item;
    if (strings[index].Trim('\t').IsEmpty()) {
      item = AppendItem(*thisCaller, olxstr(strings[index - 1]).Trim('\t').u_str());
    }
    else {
      item = AppendItem(*thisCaller, olxstr(strings[index - 1]).Trim('\t').u_str(), -1, -1,
        new TTreeNodeData(new olxstr(strings[index])));
    }
    index++;  // and now on the next item
    if (index < strings.Count()) {
      size_t nextlevel = strings[index].LeadingCharCount('\t');
      if (nextlevel > level) {
        size_t slevel = ReadStrings(index, &item, strings);
        if (slevel != level) {
          return slevel;
        }
      }
      if (nextlevel < level) {
        return  nextlevel;
      }
    }
  }
  return 0;
}
//..............................................................................
void TTreeView::ClearData()  {
  return;
}
//..............................................................................
bool TTreeView::LoadFromStrings(const TStrList &strings)  {
  ClearData();
  DeleteAllItems();
  wxTreeItemId Root = AddRoot(wxT("Root"));
  size_t index = 0;
  ReadStrings(index, &Root, strings);
  return true;
}
//..............................................................................
void TTreeView::OnMouseUp(wxMouseEvent& me)  {
  me.Skip();
}
//..............................................................................
void TTreeView::OnItemContextMenu(wxCommandEvent& evt) {
  if (evt.GetId() == ID_TREE_ExpandAll) {
    ExpandAllChildren(GetSelection());
  }
  else if (evt.GetId() == ID_TREE_CollapseAll) {
    CollapseAllChildren(GetSelection());
  }
  else if (contextMenu.ok()) {
    const MenuData* md = dynamic_cast<MenuData*>(contextMenu->GetClientObject());
    if (md != 0) {
      olxstr cmd = md->macros.Find(evt.GetId(), EmptyString());
      if (!cmd.IsEmpty()) {
        olex2::IOlex2Processor::GetInstance()->processMacro(cmd);
      }
    }
  }
}
//..............................................................................
size_t TTreeView::_SaveState(TEBitArray& res, const wxTreeItemId& item,
  size_t& counter) const
{
  size_t selected = InvalidIndex;
  if (IsExpanded(item)) {
    res.SetTrue(counter);
  }
  if (IsSelected(item)) {
    selected = counter;
  }
  wxTreeItemIdValue cookie;
  wxTreeItemId ch_id = GetFirstChild(item, cookie);
  while (ch_id.IsOk()) {
    counter++;
    if (HasChildren(ch_id)) {
      size_t sel = _SaveState(res, ch_id, counter);
      if (sel != InvalidIndex) {
        selected = sel;
      }
    }
    else {
      res.SetFalse(counter);
      if (IsSelected(ch_id)) {
        selected = counter;
      }
    }
    ch_id = GetNextChild(item, cookie);
  }
  return selected;
}
//..............................................................................
olxstr TTreeView::SaveState() const {
  wxTreeItemId root = GetRootItem();
  if ((GetWindowStyle()&wxTR_HIDE_ROOT) != 0) {
    wxTreeItemIdValue cookie;
    root = GetFirstChild(root, cookie);
  }
  TEBitArray res(GetChildrenCount(root, true)+1);
  size_t counter = 0;
  size_t selected = _SaveState(res, root, counter);
  olxstr rv = res.ToBase64String();
  return (rv.stream(';') << selected << ';' << GetScrollPos(wxVERTICAL) <<
    GetScrollPos(wxHORIZONTAL));
}
//..............................................................................
void TTreeView::_RestoreState(const TEBitArray& res, const wxTreeItemId& item,
  size_t& counter, size_t selected)
{
  if (res[counter]) {
    Expand(item);
  }
  if (selected == counter) {
    SelectItem(item, true);
  }
  wxTreeItemIdValue cookie;
  wxTreeItemId ch_id = GetFirstChild(item, cookie);
  while (ch_id.IsOk()) {
    counter++;
    if (HasChildren(ch_id)) {
      _RestoreState(res, ch_id, counter, selected);
    }
    else {
      if (counter == selected) {
        SelectItem(ch_id, true);
      }
    }
    ch_id = GetNextChild(item, cookie);
  }
}
//..............................................................................
void TTreeView::RestoreState(const olxstr& state) {
  TStrList toks(state, ';');
  if (toks.Count() != 4) {
    return;
  }
  wxTreeItemId root = GetRootItem();
  if ((GetWindowStyle()&wxTR_HIDE_ROOT) != 0) {
    wxTreeItemIdValue cookie;
    root = GetFirstChild(root, cookie);
  }
  TEBitArray res;
  res.FromBase64String(toks[0]);
  if (res.Count() != GetChildrenCount(root, true) + 1) {
    return;
  }
  size_t counter = 0;
  size_t selected = toks[1].ToSizeT();
  Freeze();
  OnSelect.SetEnabled(false);
  _RestoreState(res, root, counter, selected);
  //SelectItem(GetSelection(), true);
  OnSelect.SetEnabled(true);
  SetScrollPos(wxVERTICAL, toks[2].ToInt(), false);
  Thaw();
  SetScrollPos(wxHORIZONTAL, toks[3].ToInt(), true);
}
//..............................................................................
wxTreeItemId TTreeView::_FindByLabel(const wxTreeItemId& root,
  const olxstr& label) const
{
  if (label == GetItemText(root)) {
    return root;
  }
  if (!HasChildren(root)) {
    return wxTreeItemId();
  }
  wxTreeItemIdValue cookie;
  wxTreeItemId ch_id = GetFirstChild(root, cookie);
  while (ch_id.IsOk()) {
    wxTreeItemId id = _FindByLabel(ch_id, label);
    if (id.IsOk()) {
      return id;
    }
    ch_id = GetNextChild(root, cookie);
  }
  return ch_id;
}
//..............................................................................
wxTreeItemId TTreeView::_FindByData(const wxTreeItemId& root,
  const olxstr& data) const
{
  wxTreeItemData* _dt = GetItemData(root);
  if (_dt != 0 && olx_type<TTreeNodeData>::check(*_dt)) {
    TTreeNodeData* dt = (TTreeNodeData*)_dt;
    if (dt->GetData() != 0 && data == dt->GetData()->ToString()) {
      return root;
    }
  }
  wxTreeItemIdValue cookie;
  wxTreeItemId ch_id = GetFirstChild(root, cookie);
  while (ch_id.IsOk()) {
    wxTreeItemId id = _FindByData(ch_id, data);
    if (id.IsOk()) {
      return id;
    }
    ch_id = GetNextChild(root, cookie);
  }
  return ch_id;
}
//..............................................................................
void TTreeView::SelectByLabel(const olxstr& label) {
  wxTreeItemId item = _FindByLabel(GetRootItem(), label);
  if (item.IsOk()) {
    OnSelect.SetEnabled(false);
    SelectItem(item);
    OnSelect.SetEnabled(true);
  }
}
//..............................................................................
void TTreeView::SelectByData(const olxstr& data) {
  wxTreeItemId item = _FindByData(GetRootItem(), data);
  if (item.IsOk()) {
    OnSelect.SetEnabled(false);
    SelectItem(item);
    OnSelect.SetEnabled(true);
  }
}
//..............................................................................
void TTreeView::SetContextMenu(wxMenu* menu) {
  contextMenu = menu;
  if (menu != 0) {
    wxMenuItemList& items = menu->GetMenuItems();
    for (size_t i = 0; i < items.size(); i++) {
      Bind(wxEVT_MENU, &TTreeView::OnItemContextMenu, this, items[i]->GetId());
    }
  }
}
//..............................................................................
//..............................................................................
//..............................................................................
wxMenu* TTreeView::CreateDefaultContextMenu() {
  wxMenu* menu = new wxMenu;
  menu->Append(ID_TREE_ExpandAll, wxT("Expand all"));
  menu->Append(ID_TREE_CollapseAll, wxT("Collapse all"));
  return menu;
}
//..............................................................................
wxMenu* TTreeView::CreateContextMenu(const olxstr& def) {
  TStrList items(def, "<-");
  if ((items.Count()%2) != 0) {
    TBasicApp::NewLogEntry(logError) << "Wrong number of menu items";
    return 0;
  }
  olx_object_ptr<wxMenu> menu = CreateDefaultContextMenu();
  olx_object_ptr<MenuData> data = new MenuData();
  for (size_t i = 0; i < items.Count(); i+=2) {
    wxMenuItem* mi = menu->Append(ID_TREE_LAST + i, items[i].u_str());
    data->macros(ID_TREE_LAST + i, items[i + 1]);
  }
  menu->SetClientObject(data.release());
  return menu.release();
}
//..............................................................................
