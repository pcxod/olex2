//---------------------------------------------------------------------------

#ifndef _xl_ctrlsH
#define _xl_ctrlsH
#include "actions.h"
#include "eobjects.h"
#include "wx/wx.h"
#include "wx/spinctrl.h"
#include "wx/treectrl.h"
#include "wx/generic/treectlg.h"
#include "wx/listbox.h"
#include "wx/combo.h"
#include "wx/odcombo.h"
#include "wx/listctrl.h"
#include "globj.h"
#include "dataitem.h"
#include "wx/html/m_templ.h"
#include "wininterface.h"

#ifndef uiStr  // ansi string to wxString in unicode
  #define uiStr(v)  (wxString((v).u_str()))
  #define uiStrT(v) (wxString(v, *wxConvUI))
#endif

//---------------------------------------------------------------------------
namespace _xl_Controls  {

const short UnknownSwitchState   = -2;

class TKeyEvent: public IEObject  {
  wxKeyEvent* Event;
public:
  TKeyEvent( wxKeyEvent& evt )  {  Event = &evt;  }
  virtual ~TKeyEvent()  {  ;  }

  wxKeyEvent& GetEvent()    const {  return *Event;  }
  void SetEvent(wxKeyEvent& evt)  {  Event = &evt;  }
};

//----------------------------------------------------------------------------//
// TComboBox implementation
//----------------------------------------------------------------------------//
#ifndef __WIN32__
class TComboBox: public wxComboBox, public IEObject  {
#else
class TComboBox: public wxOwnerDrawnComboBox, public IEObject  {
#endif

  struct TDataObj  {
    IEObject* Data;
    bool Delete;
  };
  void ChangeEvent(wxCommandEvent& event);
  void LeaveEvent(wxFocusEvent& event);
  void EnterEvent(wxFocusEvent& event);
  void EnterPressedEvent(wxCommandEvent& event);
  TActionQList *FActions;
  olxstr Data, OnChangeStr, OnLeaveStr, OnEnterStr;
protected:
  void _AddObject( const olxstr &Item, IEObject* Data, bool Delete);
#ifdef __WIN32__
  virtual void OnDrawItem( wxDC& dc, const wxRect& rect, int item, int flags ) const;
  virtual wxCoord OnMeasureItem( size_t item ) const;
  virtual wxCoord OnMeasureItemWidth( size_t item ) const;
  virtual void OnDrawBg(wxDC& dc, const wxRect& rect, int item, int flags) const;
#endif
public:
  TComboBox(wxWindow *Parent, bool ReadOnly=false, const wxSize& sz=wxDefaultSize);
  virtual ~TComboBox();

  void Clear();
  void AddObject( const olxstr &Item, IEObject* Data = NULL);

  olxstr GetItem(int i)  const  {  return GetString(i).c_str();  }
  const IEObject* GetObject(int i);

  /*if a list item is constructed like 'name<-value' the pair is added as single
   item, though, once this item is selected, the value of Text() function will be
   the valu part of the item
  */
  void AddItems(const TStrList& EL);

  olxstr ItemsToString(const olxstr &separator);

  olxstr GetText() const;
  void SetText(const olxstr& T);

  DefPropC(olxstr, OnChangeStr) // this is passed to the OnChange Event
  DefPropC(olxstr, Data)
  DefPropC(olxstr, OnLeaveStr) // this is passed to the OnChange event
  DefPropC(olxstr, OnEnterStr) // this is passed when the control becomes focuesd

  inline bool IsReadOnly()  {   return WI.HasWindowStyle(wxCB_READONLY);  }
  void ReadOnly(bool v)   {  
    if( v ) 
      WI.AddWindowStyle(wxCB_READONLY);  
    else
      WI.DelWindowStyle(wxCB_READONLY);  
  }

  TActionQueue *OnChange, *OnLeave, *OnEnter;
  TWindowInterface WI;

  DECLARE_CLASS(TComboBox)
  DECLARE_EVENT_TABLE()
};
//----------------------------------------------------------------------------//
// TMenu implementation
//----------------------------------------------------------------------------//
class TMenu: public wxMenu, public IEObject  {
public:
  TMenu(const olxstr &Name=EmptyString);
  // the function creates a new clone of wxMenu
  TMenu *Clone()  {  return CopyMenu(this); };
  static TMenu *CopyMenu(wxMenu *menu);
  void Clear(); // empties the menu

  DECLARE_CLASS(TMenu)
};
//----------------------------------------------------------------------------//
// TMenuItem implementation
//----------------------------------------------------------------------------//
const short mtSeparator = wxITEM_SEPARATOR,
            mtNormalItem = wxITEM_NORMAL,
            mtCheckItem = wxITEM_CHECK,
            mtRadioItem = wxITEM_RADIO;
class TMenuItem: public wxMenuItem,  public AActionHandler  {
  TActionQueue *FActionQueue;
  TActionQList *FActions;
  olxstr FDependMode, Command, OnModeChangeCmd;
  short DependentOn;
public:
  TMenuItem(const short type, int id, TMenu* parent=NULL, const olxstr &Name=EmptyString);
  virtual ~TMenuItem();
  void ActionQueue(TActionQueue* q, const olxstr& dependMode, short dependentOn);
  bool Execute(const IEObject *Sender, const IEObject *Data);
  // updates checked status
  void ValidateState(); 
  DefPropC(olxstr, Command)

  TActionQueue *OnModeChange;
  static const short ModeDependent  = 1,
                     StateDependent = 2;
  DECLARE_CLASS(TMenuItem)
};
//----------------------------------------------------------------------------//
// TButton implementation
//----------------------------------------------------------------------------//
class AButtonBase: public AActionHandler  {
  bool FDown;
protected:
  TActionQList *FActions;
  void ClickEvent(wxCommandEvent& event);
  olxstr OnClickStr, Data, OnUpStr, OnDownStr, FDependMode, Hint;
  TActionQueue *FActionQueue;
  virtual wxWindow* GetParent()  const  = 0;
public:
  AButtonBase();
  virtual ~AButtonBase();

  void ActionQueue(TActionQueue* q, const olxstr& dependMode);
  bool Execute(const IEObject *Sender, const IEObject *Data);
  void OnRemove()  {  FActionQueue = NULL;  }

  DefPropC(olxstr, OnClickStr) // passed to OnClick event
  DefPropC(olxstr, Data)       // data associated with object
  DefPropC(olxstr, OnUpStr)   // passed to OnUp event
  DefPropC(olxstr, OnDownStr) // passed to OnDown event
  DefPropC(olxstr, Hint) // passed to OnDown event

  inline bool IsDown()  const {  return FDown;  }
  void SetDown(bool v);

  TActionQueue *OnClick, *OnUp, *OnDown;

  virtual TWindowInterface& GetWI() = 0;
};

class TButton: public wxButton, public AButtonBase {
  void MouseEnterEvent(wxMouseEvent& event);
  void MouseLeaveEvent(wxMouseEvent& event);
  void ClickEvent(wxCommandEvent& event)  {  AButtonBase::ClickEvent(event);  }
protected:
  virtual wxWindow* GetParent()  const  {  return wxButton::GetParent();  }
public:
  TButton(wxWindow* parent, wxWindowID id=-1, const wxString& label = wxEmptyString, const wxPoint& pos = wxDefaultPosition, 
    const wxSize& size = wxDefaultSize, long style = 0);
  virtual ~TButton();

  inline void SetCaption(const olxstr& l)  {  wxButton::SetLabel( uiStr(l) );  }
  inline olxstr GetCaption()         const { return wxButton::GetLabel().c_str();  }

  TWindowInterface WI;
  virtual TWindowInterface& GetWI() {  return WI;  }

  DECLARE_CLASS(TButton)
  DECLARE_EVENT_TABLE()
};

class TBmpButton: public wxBitmapButton, public AButtonBase  {
  void MouseEnterEvent(wxMouseEvent& event);
  void MouseLeaveEvent(wxMouseEvent& event);
  void ClickEvent(wxCommandEvent& event)  {  AButtonBase::ClickEvent(event);  }
  olxstr Source;
public:
  virtual wxWindow* GetParent()  const  {  return wxBitmapButton::GetParent();  }
public:
  TBmpButton(wxWindow* parent, wxWindowID id=-1, const wxBitmap& bitmap=wxNullBitmap, const wxPoint& pos = wxDefaultPosition, 
    const wxSize& size = wxDefaultSize, long style = wxBU_AUTODRAW);
  virtual ~TBmpButton();

  TWindowInterface WI;
  virtual TWindowInterface& GetWI() {  return WI;  }

  DefPropC(olxstr, Source)

  DECLARE_CLASS(TBmpButton)
  DECLARE_EVENT_TABLE()
};
//----------------------------------------------------------------------------//
// TCheckbox implementation
//----------------------------------------------------------------------------//
class TCheckBox: public wxCheckBox, public AActionHandler  {
  TActionQList *FActions;
  void ClickEvent(wxCommandEvent& event);
  void MouseEnterEvent(wxMouseEvent& event);
  olxstr OnCheckStr, OnUncheckStr, OnClickStr, Data, FDependMode;
  TActionQueue *FActionQueue;
public:
  TCheckBox(wxWindow *Parent);
  virtual ~TCheckBox();

  void ActionQueue(TActionQueue* q, const olxstr& dependMode);
  bool Execute(const IEObject *Sender, const IEObject *Data);
  void OnRemove()  {  FActionQueue = NULL;  }

  DefPropC(olxstr, Data)       // data associated with the object
  DefPropC(olxstr, OnUncheckStr)   // this is passed to the OnUncheck event
  DefPropC(olxstr, OnCheckStr)    // this is passed to the OnCheck event
  DefPropC(olxstr, OnClickStr) // this is passed to the OnClick event

  TActionQueue *OnClick, *OnCheck, *OnUncheck;

  inline void SetCaption(const olxstr &T)  {  SetLabel( uiStr(T) ); }
  inline olxstr GetCaption()         const {  return GetLabel().c_str(); }

  inline bool IsChecked()           const {  return wxCheckBox::IsChecked();  }
  inline void SetChecked(bool v)          {  wxCheckBox::SetValue(v);  }

  TWindowInterface WI;
  DECLARE_CLASS(TCheckBox)
  DECLARE_EVENT_TABLE()
};
//----------------------------------------------------------------------------//
// TLabel
//----------------------------------------------------------------------------//
class TLabel: public wxStaticText, public ACollectionItem  {
  TActionQList *FActions;
  void ClickEvent(wxCommandEvent& event);
  olxstr Data, OnClickStr;
public:
  TLabel(wxWindow *Parent);
  virtual ~TLabel();

  DefPropC(olxstr, Data)
  DefPropC(olxstr, OnClickStr)

  TActionQueue *OnClick;

  inline void SetCaption(const olxstr &T) {  SetLabel( uiStr(T) ); }
  inline olxstr GetCaption()        const {  return GetLabel().c_str(); }

  TWindowInterface WI;
  DECLARE_CLASS(TLabel)
  DECLARE_EVENT_TABLE()
};
//----------------------------------------------------------------------------//
// TListBox
//----------------------------------------------------------------------------//
class TListBox: public wxListBox, public IEObject  {
  TActionQList *FActions;
  void ClickEvent(wxMouseEvent& event);
  void ItemSelectEvent(wxCommandEvent& event);
  olxstr Data, OnDblClickStr, OnSelectStr;
public:
  TListBox(wxWindow *Parent);
  virtual ~TListBox();

  DefPropC(olxstr, Data)       // data associated with the object
  DefPropC(olxstr, OnDblClickStr) // this is passed to the OnDoubleClick event
  DefPropC(olxstr, OnSelectStr) // this is passed to the OnSelect

  void AddObject(const olxstr& Name, void *Data=NULL)  {
    wxListBox::Append( uiStr(Name), Data);
  }
  inline void Clear()  {  wxListBox::Clear();  }
  
  void AddItems(const TStrList &items);

  olxstr ItemsToString(const olxstr &separator);

  inline olxstr GetItem(int i)   {  return wxListBox::GetString(i).c_str();  }
  inline void *GetObject(int i)    {   return wxListBox::GetClientData(i);  }
  inline int Count()         const {  return wxListBox::GetCount();  }
  
  olxstr GetValue() const  {
    int index = GetSelection();
    return (index == -1) ? EmptyString : olxstr(wxListBox::GetString(index).c_str());
  }

  TActionQueue *OnDblClick, *OnSelect;

  TWindowInterface WI;
  DECLARE_CLASS(TListBox)
  DECLARE_EVENT_TABLE()
};
//----------------------------------------------------------------------------//
// TTreeView
//----------------------------------------------------------------------------//

class TTreeNodeData : public wxTreeItemData, public IEObject {
  IEObject* Data;
public:
  TTreeNodeData(IEObject* obj)      {  Data = obj;  }
  virtual ~TTreeNodeData()          { delete Data;  }
  inline IEObject* GetData()  const {  return Data;  }
};

class TTreeView: public wxGenericTreeCtrl, public IEObject  {
  olxstr Data, OnItemActivateStr, OnSelectStr;
protected:
  TActionQList *FActions;
  void SelectionEvent(wxTreeEvent& event);
  void ItemActivateEvent(wxTreeEvent& event);

  int ReadStrings(int& index, const wxTreeItemId* thisCaller, TStrList& strings);
  void ClearData();
public:
  TTreeView(wxWindow* Parent);
  virtual ~TTreeView();

  DefPropC(olxstr, Data)       // data associated with the object
  DefPropC(olxstr, OnItemActivateStr) // this is passed to the OnDoubleClick event
  DefPropC(olxstr, OnSelectStr) // this is passed to the OnSelect

  bool LoadFromStrings(TStrList &strings);

  TActionQueue *OnDblClick, *OnSelect;

  TWindowInterface WI;
  DECLARE_CLASS(TTreeView)
  DECLARE_EVENT_TABLE()
};
//----------------------------------------------------------------------------//
// TMainFrame implementation
//----------------------------------------------------------------------------//

struct TWindowInfo  {
  int x, y;
};

class TMainFrame: public wxFrame  {
protected:
  TSStrPObjList<olxstr,TWindowInfo*, false> FWindowPos;
public:
  TMainFrame(const wxString& title, const wxPoint& pos, const wxSize& size, const wxString &ClassName);
  virtual ~TMainFrame();
  void RestorePosition(wxWindow *Window); // restores previously saved position
  void SavePosition(wxWindow *Window);    //saves current position of the window on screen
  olxstr PickFile(const olxstr &Caption, const olxstr &Filter, const olxstr &DefFolder, bool Open);
//  void Maximize();
  virtual void LockWindowDestruction(wxWindow* wnd) = 0;
  virtual void UnlockWindowDestruction(wxWindow* wnd) = 0;
  
  DECLARE_CLASS(TMainFrame)
  TWindowInterface WI;
};

//----------------------------------------------------------------------------//
// TDialog implementation
//----------------------------------------------------------------------------//
class TDialog: public wxDialog {
protected:
  TMainFrame *FParent;
public:
  TDialog(TMainFrame *Parent, const wxString &Title, const wxString &ClassName);
  virtual ~TDialog();

  DECLARE_CLASS(TDialog)
  TWindowInterface WI;
};
//----------------------------------------------------------------------------//
// TTextEdit implementation
//----------------------------------------------------------------------------//
class TTextEdit: public wxTextCtrl, public IEObject  {
private:
  TActionQList *FActions;
protected:
  void ClickEvent(wxMouseEvent& event);
  void ChangeEvent(wxCommandEvent& event);
  void KeyDownEvent(wxKeyEvent& event);
  void KeyUpEvent(wxKeyEvent& event);
  void CharEvent(wxKeyEvent& event);
  void EnterPressedEvent(wxCommandEvent& event);
  void LeaveEvent(wxFocusEvent& event);
  void EnterEvent(wxFocusEvent& event);
  olxstr OnChangeStr, Data, OnLeaveStr, OnEnterStr;
public:
  TTextEdit(wxWindow *Parent, int style=0);
  virtual ~TTextEdit();

  olxstr GetText()          const {  return wxTextCtrl::GetValue().c_str(); }
  void SetText(const olxstr &T)   {  wxTextCtrl::SetValue( uiStr(T) ); }

  inline bool IsReadOnly()    const {   return WI.HasWindowStyle(wxTE_READONLY);  }
  inline void SetReadOnly( bool v)  {  WI.AddWindowStyle(wxTE_READONLY);  }

  DefPropC(olxstr, Data) // data associated with the object
  DefPropC(olxstr, OnChangeStr) // this is passed to the OnChange event
  DefPropC(olxstr, OnLeaveStr) // this is passed to the OnChange event
  DefPropC(olxstr, OnEnterStr) // this is passed when the control becomes focuesd

  TActionQueue *OnClick, *OnChange, *OnLeave, *OnEnter;
  TActionQueue *OnKeyUp, *OnKeyDown, *OnChar;
  TWindowInterface WI;

  DECLARE_CLASS(TTextEdit)
  DECLARE_EVENT_TABLE()
};
//----------------------------------------------------------------------------//
// TSpinCtrl implementation
//----------------------------------------------------------------------------//
class TSpinCtrl: public wxSpinCtrl, public IEObject  {
private:
  TActionQList *FActions;
  int Value;
protected:
  void SpinChangeEvent(wxSpinEvent& event);
  void TextChangeEvent(wxCommandEvent& event);
  void LeaveEvent(wxFocusEvent& event);
  void EnterEvent(wxFocusEvent& event);
  void EnterPressedEvent(wxCommandEvent& event);
  olxstr OnChangeStr, Data;
public:
  TSpinCtrl(wxWindow *Parent);
  virtual ~TSpinCtrl();

  DefPropC(olxstr, OnChangeStr) // this is passed to the OnChange event
  DefPropC(olxstr, Data) // data associated with the object

  inline int GetValue() const {  return wxSpinCtrl::GetValue(); }
  inline void SetValue(int v) { 
    Value = v;
    wxSpinCtrl::SetValue(v); 
  }

  TActionQueue *OnChange;

  TWindowInterface WI;

  DECLARE_CLASS(TSpinCtrl)
  DECLARE_EVENT_TABLE()
};
//----------------------------------------------------------------------------//
// TTracBar implementation
//----------------------------------------------------------------------------//
class TTrackBar: public wxSlider, public IEObject  {
private:
  TActionQList *FActions;
protected:
  void ScrollEvent(wxScrollEvent& event);
  void MouseUpEvent(wxMouseEvent& event);
  olxstr OnChangeStr, OnMouseUpStr, Data;
  int this_Val;  // needed to call events only if value has changed
public:
  TTrackBar(wxWindow *Parent);
  virtual ~TTrackBar();

  DefPropC(olxstr, OnChangeStr) // this is passed to the OnChange event
  DefPropC(olxstr, OnMouseUpStr) // this is passed to the OnChange event
  DefPropC(olxstr, Data) // data associated with the object

  inline int GetValue() const {  return wxSlider::GetValue(); }
  inline void SetValue(int v) { wxSlider::SetValue(v); }
  inline int GetMin()   const {  return wxSlider::GetMin(); }
  inline void SetMin( int v ) { wxSlider::SetRange(this->GetMax(), v); }
  inline int GetMax()   const {  return wxSlider::GetMax(); }
  inline void SetMax( int v ) { wxSlider::SetRange(v, this->GetMin()); }

  TActionQueue *OnChange, *OnMouseUp;

  TWindowInterface WI;

  DECLARE_CLASS(TTrackBar)
  DECLARE_EVENT_TABLE()
};
//----------------------------------------------------------------------------//
// TTimer implementation
//----------------------------------------------------------------------------//
class TTimer: public wxTimer, public IEObject  {
  TActionQList *FActions;
protected:
  void Notify();
  TActionQueue* FOnTimer;
public:
  TTimer();
  virtual ~TTimer();
  TActionQueue* OnTimer();

  DECLARE_CLASS(TTimer)
};
//----------------------------------------------------------------------------//
// THtml implementation
//----------------------------------------------------------------------------//
};  // end of the _xl_Controls namespace
using namespace _xl_Controls;
#endif
