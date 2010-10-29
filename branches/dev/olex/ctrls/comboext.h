#ifndef __olx_ctrl_combo_H
#define __olx_ctrl_combo_H
#include "olxctrlbase.h"
#include "estrlist.h"
#ifdef __WIN32__
  #include "wx/odcombo.h"
#else
#include "wx/combo.h"
#endif

namespace ctrl_ext {
#ifndef __WIN32__
  class TComboBox: public wxComboBox, public AOlxCtrl  {
#else
  class TComboBox: public wxOwnerDrawnComboBox, public AOlxCtrl  {
#endif
    struct TDataObj  {
      IEObject* Data;
      bool Delete;
    };
    void ChangeEvent(wxCommandEvent& event);
    void EnterPressedEvent(wxCommandEvent& event);
    olxstr Data, OnChangeStr, OnLeaveStr, OnEnterStr, OnReturnStr;
  protected:
    void _AddObject( const olxstr &Item, IEObject* Data, bool Delete);
#ifdef __WIN32__
    virtual void OnDrawItem( wxDC& dc, const wxRect& rect, int item, int flags ) const;
    virtual wxCoord OnMeasureItem( size_t item ) const;
    virtual wxCoord OnMeasureItemWidth( size_t item ) const;
    virtual void OnDrawBg(wxDC& dc, const wxRect& rect, int item, int flags) const;
#endif
  public:
    TComboBox(wxWindow *Parent, bool ReadOnly=false, const wxSize& sz=wxDefaultSize) :
#ifndef __WIN32__
      wxComboBox(Parent, -1, wxString(), wxDefaultPosition, sz, 0, NULL,
        wxCB_DROPDOWN|(ReadOnly?wxCB_READONLY:0)|wxTE_PROCESS_ENTER),
#else
      wxOwnerDrawnComboBox(Parent, -1, wxString(), wxDefaultPosition, sz, 0, NULL,
        wxCB_DROPDOWN|(ReadOnly?wxCB_READONLY:0)|wxTE_PROCESS_ENTER),
#endif
      AOlxCtrl(this),
      OnChange(Actions.New(evt_change_id)),
      OnLeave(Actions.New(evt_on_mouse_leave_id)),
      OnEnter(Actions.New(evt_on_mouse_enter_id)),
      OnReturn(Actions.New(evt_on_return_id)),
      Data(EmptyString),
      OnChangeStr(EmptyString),
      OnLeaveStr(EmptyString),
      OnEnterStr(EmptyString),
      OnReturnStr(EmptyString) {}
    virtual ~TComboBox();

    void Clear();
    void AddObject( const olxstr &Item, IEObject *Data = NULL);

    olxstr GetItem(int i) const {  return GetString(i).c_str();  }
    const IEObject* GetObject(int i);

    /*if a list item is constructed like 'name<-value' the pair is added as single
    item, though, once this item is selected, the value of Text() function will be
    the valu part of the item */
    void AddItems(const TStrList &items);

    olxstr ItemsToString(const olxstr &separator);

    olxstr GetText() const;
    void SetText(const olxstr &text);

    DefPropC(olxstr, OnChangeStr) // this is passed to the OnChange Event
    DefPropC(olxstr, Data)
    DefPropC(olxstr, OnLeaveStr) // this is passed to the OnChange event
    DefPropC(olxstr, OnEnterStr) // this is passed when the control becomes focused
    DefPropC(olxstr, OnReturnStr) // this is passed when return is pressed

    bool IsReadOnly() const {   return WI.HasWindowStyle(wxCB_READONLY);  }
    void SetReadOnly(bool v)   {  
      v ? WI.AddWindowStyle(wxCB_READONLY) : WI.DelWindowStyle(wxCB_READONLY);  
    }

    TActionQueue &OnChange, &OnLeave, &OnEnter, &OnReturn;

    DECLARE_CLASS(TComboBox)
    DECLARE_EVENT_TABLE()
  };
};  // end namespace ctrl_ext
#endif
