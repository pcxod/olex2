#include "treeviewext.h"
#include "../xglapp.h"
#include "../mainform.h"

using namespace ctrl_ext;
IMPLEMENT_CLASS(TTreeView, wxGenericTreeCtrl)

BEGIN_EVENT_TABLE(TTreeView, wxGenericTreeCtrl)
  EVT_TREE_ITEM_ACTIVATED(-1, TTreeView::ItemActivateEvent)
  EVT_TREE_SEL_CHANGED(-1, TTreeView::SelectionEvent)
END_EVENT_TABLE()

void TTreeView::ItemActivateEvent(wxTreeEvent& event)  {
  StartEvtProcessing()
    OnDblClick.Execute(this, &TEGC::New<olxstr>(GetOnItemActivateStr()) );
  EndEvtProcessing()
}
//..............................................................................
void TTreeView::SelectionEvent(wxTreeEvent& event) {
  StartEvtProcessing()
    OnSelect.Execute(this, &TEGC::New<olxstr>(GetOnSelectStr()) );
  EndEvtProcessing()
}
//..............................................................................
int TTreeView::ReadStrings(int& index, const wxTreeItemId* thisCaller, const TStrList& strings)  {
  while( (index + 2) <= strings.Count() )  {
    int level = strings[index].LeadingCharCount( '\t' );
    index++;  // now index is on data string
    wxTreeItemId item;
    if( strings[index].Trim('\t').IsEmpty() )
      item = AppendItem(*thisCaller, olxstr(strings[index-1]).Trim('\t').u_str() );
    else
      item = AppendItem(*thisCaller, olxstr(strings[index-1]).Trim('\t').u_str(), -1, -1,
         new TTreeNodeData(new olxstr(strings[index])) );
    index++;  // and now on the next item
    if( index < strings.Count() )  {
      int nextlevel = strings[index].LeadingCharCount('\t');
      if( nextlevel > level )  {
        int slevel = ReadStrings(index, &item, strings);
        if( slevel != level )
          return slevel;
      }
      if( nextlevel < level )
        return  nextlevel;
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
  int index = 0;
  ReadStrings(index, &Root, strings);
  return true;
}
