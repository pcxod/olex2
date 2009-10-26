#include "listboxext.h"
#include "frameext.h"
#include "olxvar.h"

using namespace ctrl_ext;
IMPLEMENT_CLASS(TListBox, wxListBox)

BEGIN_EVENT_TABLE(TListBox, wxListBox)
  EVT_LEFT_DCLICK(TListBox::ClickEvent)
  EVT_LISTBOX(-1, TListBox::ItemSelectEvent)
END_EVENT_TABLE()
//..............................................................................
void TListBox::ClickEvent(wxMouseEvent& event)  {
  StartEvtProcessing()
    OnDblClick.Execute(this, &TEGC::New<olxstr>(GetOnDblClickStr()) );
  EndEvtProcessing()
}
//..............................................................................
void TListBox::ItemSelectEvent(wxCommandEvent& event)  {
  if( !Data.IsEmpty() )  TOlxVars::SetVar(Data, GetValue());
  StartEvtProcessing()
    OnSelect.Execute(this, &TEGC::New<olxstr>(GetOnSelectStr()) );
  EndEvtProcessing()
}
//..............................................................................
olxstr TListBox::ItemsToString(const olxstr &sep)  {
  olxstr rv;
  for( size_t i=0; i < GetCount(); i++ )  {
    rv << GetString(i).c_str();
    if( (i+1) < GetCount() )
      rv << sep;
  }
  return rv;
}
//..............................................................................
void TListBox::AddItems(const TStrList &items)  {
  for( size_t i=0; i < items.Count(); i++ )
    AddObject(items[i]);
}
