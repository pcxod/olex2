//----------------------------------------------------------------------------//
// controls implementation
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif
#include "ctrls.h"
#include <wx/filedlg.h>
#include <wx/tooltip.h>
#include <wx/mstream.h>
#include "efile.h"
#include "bapp.h"

#include "wx/image.h"
#include "wx/wfstream.h"
#include "mainform.h"

#include "fsext.h"
#include "htmlext.h"

#include "obase.h"

#include "xglapp.h"
#include "etime.h"
#include "egc.h"
#include "olxvar.h"

#define StartEvtProcessing()   TGlXApp::GetMainForm()->LockWindowDestruction( GetParent() );\
  try  {

#define EndEvtProcessing()  }  catch( const TExceptionBase& exc )  {\
    TGlXApp::GetMainForm()->UnlockWindowDestruction( GetParent() );\
    throw TFunctionFailedException(__OlxSourceInfo, exc.Replicate() );\
  }\
  TGlXApp::GetMainForm()->UnlockWindowDestruction( GetParent() );


#ifndef __WIN32__
IMPLEMENT_CLASS(TComboBox, wxComboBox)
#else
IMPLEMENT_CLASS(TComboBox, wxOwnerDrawnComboBox)
#endif
IMPLEMENT_CLASS(TMenu, wxMenu)
IMPLEMENT_CLASS(TMenuItem, wxMenuItem)
IMPLEMENT_CLASS(TButton, wxButton)
IMPLEMENT_CLASS(TBmpButton, wxBitmapButton)
IMPLEMENT_CLASS(TCheckBox, wxCheckBox)
IMPLEMENT_CLASS(TLabel, wxStaticText)
IMPLEMENT_CLASS(TListBox, wxListBox)
IMPLEMENT_CLASS(TTreeView, wxGenericTreeCtrl)
IMPLEMENT_CLASS(TMainFrame, wxFrame)
IMPLEMENT_CLASS(TDialog, wxDialog)
IMPLEMENT_CLASS(TTextEdit, wxTextCtrl)
IMPLEMENT_CLASS(TSpinCtrl, wxSpinCtrl)
IMPLEMENT_CLASS(TTrackBar, wxSlider)
IMPLEMENT_CLASS(TTimer, wxTimer)

//----------------------------------------------------------------------------//
// TdlgComboBox function bodies
//----------------------------------------------------------------------------//
#ifndef __WIN32__
BEGIN_EVENT_TABLE(TComboBox, wxComboBox)
#else
BEGIN_EVENT_TABLE(TComboBox, wxOwnerDrawnComboBox)
#endif
//  EVT_TEXT(-1, TComboBox::ChangeEvent)
  EVT_COMBOBOX(-1, TComboBox::ChangeEvent)
  EVT_TEXT_ENTER(-1, TComboBox::EnterPressedEvent)
  EVT_KILL_FOCUS(TComboBox::LeaveEvent)
  EVT_SET_FOCUS(TComboBox::EnterEvent)
END_EVENT_TABLE()
//..............................................................................
TComboBox::TComboBox(wxWindow *Parent, bool ReadOnly, const wxSize& sz) :
#ifndef __WIN32__
  wxComboBox(Parent, -1, wxString(), wxDefaultPosition, sz, 0, NULL,
    wxCB_DROPDOWN|(ReadOnly?wxCB_READONLY:0)|wxTE_PROCESS_ENTER), WI(this)
#else
  wxOwnerDrawnComboBox(Parent, -1, wxString(), wxDefaultPosition, sz, 0, NULL,
    wxCB_DROPDOWN|(ReadOnly?wxCB_READONLY:0)|wxTE_PROCESS_ENTER), WI(this)
#endif
{
  FActions = new TActionQList;
  OnChange = &FActions->NewQueue("ONCHANGE");
  OnLeave = &FActions->NewQueue("ONLEAVE");
  OnEnter = &FActions->NewQueue("ONENTER");
}
TComboBox::~TComboBox()  {
  for( unsigned int i=0; i < GetCount(); i++ )  {
    TDataObj* d_o = (TDataObj*)GetClientData(i);
    if( d_o != NULL )  {
      if( d_o->Delete )
        delete (IEObject*)d_o->Data;
      delete d_o;
    }
  }
  delete FActions;
}
//..............................................................................
void TComboBox::SetText(const olxstr& T)  {  
  SetValue( T.u_str() );  
#ifdef __WIN32__
  if( GetTextCtrl() != NULL )
    GetTextCtrl()->SetInsertionPoint(0);
#endif		
}
//..............................................................................
void TComboBox::Clear() {
  for( unsigned int i=0; i < GetCount(); i++ )  {
    TDataObj* d_o = (TDataObj*)GetClientData(i);
    if( d_o != NULL )  {
      if( d_o->Delete )
        delete (IEObject*)d_o->Data;
      delete d_o;
    }
  }
#ifndef __WIN32__
  wxComboBox::Clear();
#else	
  wxOwnerDrawnComboBox::Clear();
#endif	
}
//..............................................................................
void TComboBox::_AddObject( const olxstr &Item, IEObject* Data, bool Delete)  {
  Append( uiStr(Item) );
  if( Data != NULL )  {
    TDataObj* d_o = new TDataObj;
    d_o->Data = Data;
    d_o->Delete = Delete;
    SetClientData( GetCount()-1, d_o );
  }
  else
    SetClientData( GetCount()-1, NULL );
}
//..............................................................................
void TComboBox::AddObject( const olxstr &Item, IEObject* Data)  {
  _AddObject(Item, Data, false);
}
//..............................................................................
void TComboBox::EnterPressedEvent(wxCommandEvent& event)  {
  if( !Data.IsEmpty() )  TOlxVars::SetVar(Data, GetText());
  StartEvtProcessing()
    OnChange->Execute(this, &TEGC::New<olxstr>(GetOnChangeStr()) );
  EndEvtProcessing()
}
//..............................................................................
void TComboBox::ChangeEvent(wxCommandEvent& event)  {
  if( !Data.IsEmpty() )  TOlxVars::SetVar(Data, GetText());
  StartEvtProcessing()
    OnChange->Execute(this, &TEGC::New<olxstr>(GetOnChangeStr()) );
  EndEvtProcessing()
}
//..............................................................................
void TComboBox::LeaveEvent(wxFocusEvent& event)  {
  if( !Data.IsEmpty() )  TOlxVars::SetVar(Data, GetText());
  StartEvtProcessing()
    OnLeave->Execute(this, &TEGC::New<olxstr>(GetOnLeaveStr()));
  EndEvtProcessing()
}
//..............................................................................
void TComboBox::EnterEvent(wxFocusEvent& event)  {
  StartEvtProcessing()
    OnEnter->Execute(this, &TEGC::New<olxstr>(GetOnEnterStr()));
  EndEvtProcessing()
}
//..............................................................................
const IEObject* TComboBox::GetObject(int i)  {
  TDataObj* res = (TDataObj*)GetClientData(i);
  return (res != NULL && !res->Delete) ? res->Data : NULL;
}
//..............................................................................
olxstr TComboBox::GetText() const {
  olxstr cv = GetValue().c_str();
  for( unsigned int i=0; i < GetCount(); i++ )  {
    if( cv == GetString(i).c_str() )  {
      TDataObj* res = (TDataObj*)GetClientData(i);
      return (res != NULL && res->Delete) ? res->Data->ToString() : cv;
    }
  }
  return cv;
}
//..............................................................................
olxstr TComboBox::ItemsToString(const olxstr &sep)  {
  olxstr rv;
  for( unsigned int i=0; i < GetCount(); i++ )  {
    rv << GetString(i).c_str();
    TDataObj* res = (TDataObj*)GetClientData(i);
    if( res != NULL && res->Delete )
      rv << "<-" << res->Data->ToString();
    if( (i+1) < GetCount() )
      rv << sep;
  }
  return rv;
}
//..............................................................................
void TComboBox::AddItems(const TStrList& EL) {
  for( int i=0; i < EL.Count(); i++ )  {
    int ind = EL[i].IndexOf( "<-" );
    if(  ind != -1 )  {
      olxstr tmp = EL[i].SubStringFrom(ind + 2);
      _AddObject( EL[i].SubStringTo(ind), tmp.Replicate(), true);
    }
    else
      _AddObject( EL[i], NULL, false );
  }
}
//..............................................................................
#ifdef __WIN32__
void TComboBox::OnDrawItem( wxDC& dc, const wxRect& rect, int item, int flags ) const {
  wxOwnerDrawnComboBox::OnDrawItem(dc, rect, item, flags);
  return;
}
//..............................................................................
wxCoord TComboBox::OnMeasureItem( size_t item ) const {
  return this->GetCharHeight();
}
//..............................................................................
wxCoord TComboBox::OnMeasureItemWidth( size_t item ) const {
  return wxOwnerDrawnComboBox::OnMeasureItem(item);
}
//..............................................................................
void TComboBox::OnDrawBg(wxDC& dc, const wxRect& rect, int item, int flags) const {
  return;
}
#endif
//..............................................................................
//----------------------------------------------------------------------------//
// TMainFrame implementation
//----------------------------------------------------------------------------//
TMainFrame::TMainFrame(const wxString& title, const wxPoint& pos, const wxSize& size, const wxString &ClassName)
: wxFrame((wxFrame*)NULL, wxID_ANY, title, pos, size, wxDEFAULT_FRAME_STYLE),
//: wxFrame((wxFrame *)NULL, 1, title, pos, size,
//  wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxMAXIMIZE | wxMINIMIZE_BOX | wxMAXIMIZE_BOX | wxCLOSE_BOX, ClassName),
  WI(this)
{
}
TMainFrame::~TMainFrame()  {
  for( int i=0; i < FWindowPos.Count(); i++ )
    delete FWindowPos.GetObject(i);
}
//..............................................................................
/*void TMainFrame::Maximize()
{
  Width(wxSystemSettings::GetMetric(wxSYS_SCREEN_X));
  Height(wxSystemSettings::GetMetric(wxSYS_SCREEN_Y));
  Left(0);
  Top(0);
} */
//..............................................................................
void TMainFrame::RestorePosition(wxWindow *Window)  {  // restores previously saved position
  TWindowInfo * wi;
  olxstr S = Window->GetName().c_str();
  wi = FWindowPos[S];
  if( wi != NULL )
    Window->Move(wi->x, wi->y);
}
//..............................................................................
void TMainFrame::SavePosition(wxWindow *Window)  {  //saves current position of the window on screen
  TWindowInfo * wi;
  olxstr S = Window->GetName().c_str();
  wi = FWindowPos[S];
  if( wi == NULL )  {
    wi = new TWindowInfo;
    FWindowPos.Add(S, wi);
  }
  Window->GetPosition(&(wi->x), &(wi->y));
}
//..............................................................................
olxstr TMainFrame::PortableFilter(const olxstr& filter)  {
#if defined(__WIN32__) || defined(__MAC__)
  return filter;
#else
  olxstr rv;
  TStrList fitems(filter, '|');
  for( int i=0; i < fitems.Count(); i+=2 )  {
    if( i+1 >= fitems.Count() )
      break;
    if( i != 0 )
      rv << '|';
    rv << fitems[i] << '|';
    TStrList masks(fitems[i+1], ';');
    for( int j=0; j < masks.Count(); j++ )  {
      int di = masks[j].LastIndexOf('.');
      if( di == -1 )  {
        rv << masks[j];
        if( j+1 < masks.Count() )
          rv << ';';
        continue;
      }
      rv << masks[j].SubStringTo(di+1);
      for( int k=di+1; k < masks[j].Length(); k++ )
        rv << '[' << olxstr::o_tolower(masks[j].CharAt(k)) << olxstr::o_toupper(masks[j].CharAt(k)) << ']';
      if( j+1 < masks.Count() )
        rv << ';';
    }
  }
  return rv;
#endif
}
//..............................................................................
olxstr TMainFrame::PickFile(const olxstr &Caption, const olxstr &Filter,
                              const olxstr &DefFolder, bool Open)  {
  olxstr FN;
  int Style;
  if( Open )  Style = wxFD_OPEN;
  else        Style = wxFD_SAVE;
  wxFileDialog *dlgFile = new wxFileDialog( this, uiStr(Caption), uiStr(DefFolder), wxString(),
    PortableFilter(Filter).u_str(), Style);
  if( dlgFile->ShowModal() ==  wxID_OK )
    FN = dlgFile->GetPath().c_str();
  delete dlgFile;
  return FN;
}
//----------------------------------------------------------------------------//
// TDialog implementation
//----------------------------------------------------------------------------//
TDialog::TDialog(TMainFrame *Parent, const wxString &Title, const wxString &ClassName)
:wxDialog(Parent, -1,  Title, wxPoint(0, 0), wxSize(425, 274), wxRESIZE_BORDER | wxDEFAULT_DIALOG_STYLE, ClassName), WI(this)
{
  FParent = Parent;
  if( FParent != NULL )
    FParent->RestorePosition(this);
}
TDialog::~TDialog()  {
  if( FParent != NULL )
    FParent->SavePosition(this);
}
//----------------------------------------------------------------------------//
// TMenu implementation
//----------------------------------------------------------------------------//
TMenu::TMenu(const olxstr &Name) : wxMenu( uiStr(Name) )
{ }
//..............................................................................
TMenu *TMenu::CopyMenu(wxMenu *menu)  {
  TMenu *M = new TMenu, *nm;
  wxMenuItem *mi, *nmi;
  int miId;
  wxItemKind miKind;
  wxString miText, miHelpStr;
  for( unsigned int i=0; i < menu->GetMenuItemCount(); i++ )  {
    mi = menu->FindItemByPosition(i);
    if( mi->GetSubMenu() )  {
      nm = CopyMenu(mi->GetSubMenu());
      miId = mi->GetId();
      miKind = mi->GetKind();
      miText = mi->GetText();
      miHelpStr = mi->GetHelp();
      M->Append(miId, miText, nm, miHelpStr);
    }
    else  {
      miId = mi->GetId();
      miKind = mi->GetKind();
      miText = mi->GetText();
      miHelpStr = mi->GetHelp();
      nmi = new wxMenuItem(M, miId, miText, miHelpStr, miKind);
      M->Append(nmi);
    }
  }
  return M;
}
//..............................................................................
void TMenu::Clear()  {
  TPtrList<wxMenuItem> E;
  unsigned int count = GetMenuItemCount();
  E.SetCapacity( count+1 );
  for( unsigned int i=0; i < count; i++ )
    E.Add(FindItemByPosition(i));

  for( unsigned int i=0; i < count; i++ )
    Destroy(E[i]);
}
//----------------------------------------------------------------------------//
// TMenuItem implementation
//----------------------------------------------------------------------------//
//..............................................................................
TMenuItem::TMenuItem(const short type, int id, TMenu* parent, const olxstr& Name) :
  wxMenuItem(parent, id, Name.u_str(), wxString(), static_cast<wxItemKind>(type))
{
  FActions = new TActionQList;
  OnModeChange = &FActions->NewQueue("ONMODECHANGE");
  FActionQueue = NULL;
  SetToDelete(false);  // see TButton for details
  DependentOn = 0;
}
//..............................................................................
TMenuItem::~TMenuItem()  {
//  if( FActionQueue )  FActionQueue->Remove( this );
  delete FActions;
}
//..............................................................................
void TMenuItem::ActionQueue(TActionQueue* q, const olxstr& dependMode, short dependentOn)  {
  int cmdind = dependMode.FirstIndexOf(';');
  if( cmdind != -1 )  {
    FDependMode = dependMode.SubStringTo(cmdind);
    OnModeChangeCmd = dependMode.SubStringFrom(cmdind+1);
  }
  else
    FDependMode = dependMode;
  DependentOn = dependentOn;
  FActionQueue = q;
  FActionQueue->Add( this );
}
//..............................................................................
void TMenuItem::ValidateState()  {
  if( DependentOn != 0 && IsCheckable() && !FDependMode.IsEmpty() )  {  
    if( DependentOn == ModeDependent )
      Check( TModeChange::CheckStatus(FDependMode, OnModeChangeCmd) );
    else if( DependentOn == StateDependent )
      Check( TStateChange::CheckStatus(FDependMode, OnModeChangeCmd) );
  }
}
//..............................................................................
bool TMenuItem::Execute(const IEObject *Sender, const IEObject *Data)  {
  if( Data && EsdlInstanceOf(*Data, TModeChange) )  {
    const TModeChange* mc = (const TModeChange*)Data;
    if( this->IsCheckable() )
      Check( mc->CheckStatus(FDependMode, OnModeChangeCmd) );
  }
  if( Data && EsdlInstanceOf(*Data, TStateChange) )  {
    TStateChange* sc = (TStateChange*)Data;
    if( this->IsCheckable() )
      Check( sc->CheckStatus(FDependMode, OnModeChangeCmd) );
  }
  return true;
}
//..............................................................................
//----------------------------------------------------------------------------//
// TButton implementation
//----------------------------------------------------------------------------//
AButtonBase::AButtonBase()  {
  FActions = new TActionQList;
  OnClick = &FActions->NewQueue("ONCLICK");
  OnUp = &FActions->NewQueue("ONUP");
  OnDown = &FActions->NewQueue("ONDOWN");
  FDown = false;
  FActionQueue = NULL;
  SetToDelete(false); // the objects gets deleted by wxWidgets
}
//..............................................................................
AButtonBase::~AButtonBase()  {
  if( FActionQueue )  FActionQueue->Remove(this);
  delete FActions;
}
//..............................................................................
void AButtonBase::ActionQueue(TActionQueue* q, const olxstr& dependMode)  {
  FActionQueue = q;
  FDependMode = dependMode;
  FActionQueue->Add( this );
}
bool AButtonBase::Execute(const IEObject *Sender, const IEObject *Data)  {
  if( Data && EsdlInstanceOf(*Data, TModeChange) )  {
    const TModeChange* mc = (const TModeChange*)Data;
    SetDown( mc->CheckStatus(FDependMode) );
  }
  return true;
}
//..............................................................................
void AButtonBase::SetDown(bool v )  {
  if( FDown == v )  return;
  StartEvtProcessing()
    if( FDown )  {
      FDown = false;
      if( !GetOnUpStr().IsEmpty() )
        OnUp->Execute(dynamic_cast<IEObject*>(this), &TEGC::New<olxstr>( GetOnUpStr() ));
    }
    else  {
      FDown = true;
      if( !GetOnDownStr().IsEmpty() )
        OnDown->Execute(dynamic_cast<IEObject*>(this), &TEGC::New<olxstr>( GetOnDownStr() ));
    }
  EndEvtProcessing()
}
//..............................................................................
void AButtonBase::ClickEvent(wxCommandEvent& event)  {
  StartEvtProcessing()
    OnClick->Execute(this, &TEGC::New<olxstr>(GetOnClickStr()) );
    if( FDown )  {
      FDown = false;
      if( !GetOnUpStr().IsEmpty() )
        OnUp->Execute(dynamic_cast<IEObject*>(this), &TEGC::New<olxstr>( GetOnUpStr() ));
    }
    else  {
      FDown = true;
      if( !GetOnDownStr().IsEmpty() )
        OnDown->Execute(dynamic_cast<IEObject*>(this), &TEGC::New<olxstr>( GetOnDownStr() ));
    }
  EndEvtProcessing()
}
//..............................................................................
BEGIN_EVENT_TABLE(TButton, wxButton)
  EVT_BUTTON(-1, TButton::ClickEvent)
  EVT_ENTER_WINDOW(TButton::MouseEnterEvent)
  EVT_LEAVE_WINDOW(TButton::MouseLeaveEvent)
END_EVENT_TABLE()

TButton::TButton(wxWindow* P, wxWindowID id, const wxString& label, const wxPoint& pos, 
    const wxSize& size, long style) : 
    wxButton(P, id, label, pos, size, style), WI(this)  {
}
//..............................................................................
TButton::~TButton()  {  ;  }
//..............................................................................
void TButton::MouseEnterEvent(wxMouseEvent& event)  {
  SetCursor( wxCursor(wxCURSOR_HAND) );
  if( !GetHint().IsEmpty() )
    SetToolTip( GetHint().u_str() );
  event.Skip();
}
//..............................................................................
void TButton::MouseLeaveEvent(wxMouseEvent& event)  {  
  
  event.Skip();
}
//..............................................................................
BEGIN_EVENT_TABLE(TBmpButton, wxBitmapButton)
  EVT_BUTTON(-1, TBmpButton::ClickEvent)
  EVT_ENTER_WINDOW(TBmpButton::MouseEnterEvent)
  EVT_LEAVE_WINDOW(TBmpButton::MouseLeaveEvent)
END_EVENT_TABLE()

TBmpButton::TBmpButton(wxWindow* P, wxWindowID id, const wxBitmap& bitmap, const wxPoint& pos, 
    const wxSize& size, long style) : 
    wxBitmapButton(P, -1, bitmap, pos, size, style), WI(this)  {
}
//..............................................................................
TBmpButton::~TBmpButton()  {  ;  }
//..............................................................................
void TBmpButton::MouseEnterEvent(wxMouseEvent& event)  {
  SetCursor( wxCursor(wxCURSOR_HAND) );
  event.Skip();
}
//..............................................................................
void TBmpButton::MouseLeaveEvent(wxMouseEvent& event)  {  
  event.Skip();
}
//..............................................................................
//----------------------------------------------------------------------------//
// TCheckboximplementation
//----------------------------------------------------------------------------//
BEGIN_EVENT_TABLE(TCheckBox, wxCheckBox)
  EVT_CHECKBOX(-1, TCheckBox::ClickEvent)
  EVT_ENTER_WINDOW(TCheckBox::MouseEnterEvent)
END_EVENT_TABLE()

TCheckBox::TCheckBox(wxWindow *P):wxCheckBox(P, -1, wxString()), WI(this)  {
  FActions = new TActionQList;
  OnClick = &FActions->NewQueue("ONCLICK");
  OnCheck = &FActions->NewQueue("ONCHECK");
  OnUncheck = &FActions->NewQueue("ONUNCHECK");
  FActionQueue = NULL;
  SetToDelete(false);  // see TButton for details
}
//..............................................................................
TCheckBox::~TCheckBox()  {
  if( FActionQueue )  FActionQueue->Remove(this);
  delete FActions;
}
//..............................................................................
void TCheckBox::MouseEnterEvent(wxMouseEvent& event)  {
  event.Skip();
  SetCursor( wxCursor(wxCURSOR_HAND) );
}
//..............................................................................
void TCheckBox::ActionQueue(TActionQueue* q, const olxstr& dependMode)  {
  FActionQueue = q;
  FDependMode = dependMode;
  FActionQueue->Add( this );
}
bool TCheckBox::Execute(const IEObject *Sender, const IEObject *Data)  {
  if( Data && EsdlInstanceOf(*Data, TModeChange) )  {
    const TModeChange* mc = (const TModeChange*)Data;
      SetChecked( mc->CheckStatus(FDependMode) );
  }

  StartEvtProcessing()
    OnClick->Execute(this, &TEGC::New<olxstr>(GetOnClickStr()) );
    if( IsChecked() )
      OnCheck->Execute(this, &TEGC::New<olxstr>(GetOnCheckStr()) );
    else
      OnUncheck->Execute(this, &TEGC::New<olxstr>(GetOnUncheckStr()) );
  EndEvtProcessing()

  return true;
}
//..............................................................................
void TCheckBox::ClickEvent(wxCommandEvent& event)  {
  if( !Data.IsEmpty() )  TOlxVars::SetVar(Data, IsChecked());
  StartEvtProcessing()
    OnClick->Execute(this, &TEGC::New<olxstr>(GetOnClickStr()) );
    if( IsChecked() )
      OnCheck->Execute(this, &TEGC::New<olxstr>(GetOnCheckStr()) );
    else
      OnUncheck->Execute(this, &TEGC::New<olxstr>(GetOnUncheckStr()) );
  EndEvtProcessing()
}
//..............................................................................
//----------------------------------------------------------------------------//
// TLabel Implementation
//----------------------------------------------------------------------------//
BEGIN_EVENT_TABLE(TLabel, wxStaticText)
  EVT_COMMAND(-1, wxEVT_LEFT_DOWN, TLabel::ClickEvent)
END_EVENT_TABLE()

TLabel::TLabel(wxWindow *P):wxStaticText(P, -1, wxString()), WI(this)  {
  FActions = new TActionQList;
  OnClick = &FActions->NewQueue("ONCLICK");
}
//..............................................................................
TLabel::~TLabel()  {
  delete FActions;
}
//..............................................................................
void TLabel::ClickEvent(wxCommandEvent& event)  {
  StartEvtProcessing()
    OnClick->Execute(this, &TEGC::New<olxstr>( GetOnClickStr() ));
  EndEvtProcessing()
}
//..............................................................................
//----------------------------------------------------------------------------//
// TTextEdit implementation
//----------------------------------------------------------------------------//
//..............................................................................
BEGIN_EVENT_TABLE(TTextEdit, wxTextCtrl)
  EVT_LEFT_DCLICK(TTextEdit::ClickEvent)
  EVT_TEXT(-1, TTextEdit::ChangeEvent)
  EVT_KEY_UP(TTextEdit::KeyUpEvent)
  EVT_CHAR(TTextEdit::CharEvent)
  EVT_KEY_DOWN(TTextEdit::KeyDownEvent)
  EVT_TEXT_ENTER(-1, TTextEdit::EnterPressedEvent)
  EVT_KILL_FOCUS(TTextEdit::LeaveEvent)
  EVT_SET_FOCUS(TTextEdit::EnterEvent)
END_EVENT_TABLE()
//..............................................................................
TTextEdit::TTextEdit(wxWindow *Parent, int style):
    wxTextCtrl(Parent, -1, wxString(), wxDefaultPosition, wxDefaultSize, style), WI(this)  {
  FActions = new TActionQList;
  OnChange = &FActions->NewQueue("ONCHANGE");
  OnClick = &FActions->NewQueue("ONCLICK");
  OnLeave = &FActions->NewQueue("ONLEAVE");
  OnEnter = &FActions->NewQueue("ONENTER");
  OnKeyUp = &FActions->NewQueue("ONKEYUP");
  OnChar = &FActions->NewQueue("ONCHAR");
  OnKeyDown = &FActions->NewQueue("ONKEYDOWN");
}
//..............................................................................
TTextEdit::~TTextEdit()  {
  delete FActions;
}
//..............................................................................
void TTextEdit::ClickEvent(wxMouseEvent& event)  {
  StartEvtProcessing()
    OnClick->Execute(this);
    event.Skip();
  EndEvtProcessing()
}
//..............................................................................
void TTextEdit::ChangeEvent(wxCommandEvent& event)  {
  if( !Data.IsEmpty() )  TOlxVars::SetVar(Data, GetText());
  StartEvtProcessing()
    OnChange->Execute(this);
  EndEvtProcessing()
}
//..............................................................................
void TTextEdit::LeaveEvent(wxFocusEvent& event)  {
  if( !Data.IsEmpty() )  TOlxVars::SetVar(Data, GetText());
  StartEvtProcessing()
    OnLeave->Execute(this, &TEGC::New<olxstr>(GetOnLeaveStr()));
  EndEvtProcessing()
}
//..............................................................................
void TTextEdit::EnterEvent(wxFocusEvent& event)  {
  StartEvtProcessing()
    OnEnter->Execute(this, &TEGC::New<olxstr>(GetOnEnterStr()));
  EndEvtProcessing()
}
//..............................................................................
void TTextEdit::EnterPressedEvent(wxCommandEvent& event)  {
  if( !Data.IsEmpty() )  TOlxVars::SetVar(Data, GetText());
  StartEvtProcessing()
    OnChange->Execute(this, &TEGC::New<olxstr>(GetOnChangeStr()));
  EndEvtProcessing()
}
//..............................................................................
void TTextEdit::KeyDownEvent(wxKeyEvent& event)  {
  TKeyEvent evt(event);
  event.Skip();
  StartEvtProcessing()
    OnKeyDown->Execute(this, &evt);
  EndEvtProcessing()
}
//..............................................................................
void TTextEdit::KeyUpEvent(wxKeyEvent& event)  {
  TKeyEvent evt(event);
  event.Skip();
  StartEvtProcessing()
    OnKeyUp->Execute(this, &evt);
  EndEvtProcessing()
}
//..............................................................................
void TTextEdit::CharEvent(wxKeyEvent& event)  {
  TKeyEvent evt(event);
  event.Skip();
  StartEvtProcessing()
    OnChar->Execute(this, &evt);
  EndEvtProcessing()
}
//..............................................................................
//----------------------------------------------------------------------------//
// TListBox implementation
//----------------------------------------------------------------------------//
//..............................................................................
BEGIN_EVENT_TABLE(TListBox, wxListBox)
  EVT_LEFT_DCLICK(TListBox::ClickEvent)
  EVT_LISTBOX(-1, TListBox::ItemSelectEvent)
END_EVENT_TABLE()
//..............................................................................
TListBox::TListBox(wxWindow *Parent): wxListBox(Parent, -1), WI(this)  {
  FActions = new TActionQList;
  OnSelect = &FActions->NewQueue("ONSELECT");
  OnDblClick = &FActions->NewQueue("ONCLICK");
}
//..............................................................................
TListBox::~TListBox()  {
  delete FActions;
}
//..............................................................................
void TListBox::ClickEvent(wxMouseEvent& event)  {
  StartEvtProcessing()
    OnDblClick->Execute(this, &TEGC::New<olxstr>(GetOnDblClickStr()) );
  EndEvtProcessing()
}
//..............................................................................
void TListBox::ItemSelectEvent(wxCommandEvent& event)  {
  if( !Data.IsEmpty() )  TOlxVars::SetVar(Data, GetValue());
  StartEvtProcessing()
    OnSelect->Execute(this, &TEGC::New<olxstr>(GetOnSelectStr()) );
  EndEvtProcessing()
}
//..............................................................................
olxstr TListBox::ItemsToString(const olxstr &sep)  {
  olxstr rv;
  for( unsigned int i=0; i < GetCount(); i++ )  {
    rv << GetString(i).c_str();
    if( (i+1) < GetCount() )
      rv << sep;
  }
  return rv;
}
//..............................................................................
void TListBox::AddItems(const TStrList &items)  {
  for( int i=0; i < items.Count(); i++ )
    AddObject(items[i]);
}
//..............................................................................
//----------------------------------------------------------------------------//
// TTextEdit implementation
//----------------------------------------------------------------------------//
//..............................................................................
BEGIN_EVENT_TABLE(TSpinCtrl, wxSpinCtrl)
  EVT_TEXT(-1, TSpinCtrl::TextChangeEvent)
  EVT_SPIN(-1, TSpinCtrl::SpinChangeEvent)
  EVT_TEXT_ENTER(-1, TSpinCtrl::EnterPressedEvent)
  EVT_KILL_FOCUS(TSpinCtrl::LeaveEvent)
  EVT_SET_FOCUS(TSpinCtrl::EnterEvent)
END_EVENT_TABLE()
//..............................................................................
TSpinCtrl::TSpinCtrl(wxWindow *Parent, const wxSize& sz): 
  wxSpinCtrl(Parent, -1, wxEmptyString, wxDefaultPosition, sz), WI(this)  
{
  FActions = new TActionQList;
  OnChange = &FActions->NewQueue("ONCHANGE");
}
//..............................................................................
TSpinCtrl::~TSpinCtrl() {
  delete FActions;
}
//..............................................................................
void TSpinCtrl::SpinChangeEvent(wxSpinEvent& event)  {
  int val = GetValue();
  if( val == Value ) return;
  Value = val;
  StartEvtProcessing()
    OnChange->Execute(this, &TEGC::New<olxstr>(GetOnChangeStr()));
  EndEvtProcessing()
}
//..............................................................................
void TSpinCtrl::TextChangeEvent(wxCommandEvent& event)  {
  int val = GetValue();
  if( val == Value ) return;
  Value = val;
  if( !Data.IsEmpty() )  TOlxVars::SetVar(Data, GetValue());
  StartEvtProcessing()
   OnChange->Execute(this, &TEGC::New<olxstr>(GetOnChangeStr()));
  EndEvtProcessing()
}
void TSpinCtrl::LeaveEvent(wxFocusEvent& event)  {
  int val = GetValue();
  if( val == Value ) return;
  Value = val;
  StartEvtProcessing()
   OnChange->Execute(this, &TEGC::New<olxstr>(GetOnChangeStr()));
  EndEvtProcessing()
}
void TSpinCtrl::EnterEvent(wxFocusEvent& event)  {
}
void TSpinCtrl::EnterPressedEvent(wxCommandEvent& event)  {
  int val = GetValue();
  if( val == Value ) return;
  Value = val;
  StartEvtProcessing()
   OnChange->Execute(this, &TEGC::New<olxstr>(GetOnChangeStr()));
  EndEvtProcessing()
}
//..............................................................................
//----------------------------------------------------------------------------//
// TTreeView implementation
//----------------------------------------------------------------------------//
BEGIN_EVENT_TABLE(TTreeView, wxGenericTreeCtrl)
  EVT_TREE_ITEM_ACTIVATED(-1, TTreeView::ItemActivateEvent)
  EVT_TREE_SEL_CHANGED(-1, TTreeView::SelectionEvent)
END_EVENT_TABLE()

TTreeView::TTreeView(wxWindow* Parent): wxGenericTreeCtrl(Parent), WI(this)  {
  FActions = new TActionQList;
  OnSelect = &FActions->NewQueue("ONSELECT");
  OnDblClick = &FActions->NewQueue("ONCLICK");
}
//..............................................................................
TTreeView::~TTreeView()  {
  delete FActions;
  ClearData();
}
//..............................................................................
void TTreeView::ItemActivateEvent(wxTreeEvent& event)  {
  StartEvtProcessing()
    OnDblClick->Execute(this, &TEGC::New<olxstr>(GetOnItemActivateStr()) );
  EndEvtProcessing()
}
//..............................................................................
void TTreeView::SelectionEvent(wxTreeEvent& event) {
  StartEvtProcessing()
    OnSelect->Execute(this, &TEGC::New<olxstr>(GetOnSelectStr()) );
  EndEvtProcessing()
}
//..............................................................................
int TTreeView::ReadStrings(int& index, const wxTreeItemId* thisCaller, TStrList& strings)  {
  while( (index + 2) <= strings.Count() )  {
    int level = strings[index].LeadingCharCount( '\t' );
    index++;  // now index is on data string
    wxTreeItemId item;
    if( strings[index].Trim('\t').IsEmpty() )
      item = AppendItem( *thisCaller, strings[index-1].Trim('\t').u_str() );
    else
      item = AppendItem( *thisCaller, strings[index-1].Trim('\t').u_str(), -1, -1,
         new TTreeNodeData(new olxstr(strings[index])) );
    index++;  // and now on the next item
    if( index < strings.Count() )  {
      int nextlevel = strings[index].LeadingCharCount('\t');
      if( nextlevel > level )  {
        int slevel = ReadStrings(index, &item, strings);
        if( slevel != level )
          return slevel;
      }
      if( nextlevel < level )  return  nextlevel;
    }
  }
  return 0;
}
//..............................................................................
void TTreeView::ClearData()  {
  return;
}
//..............................................................................
bool TTreeView::LoadFromStrings(TStrList &strings)  {
  ClearData();
  DeleteAllItems();
  wxTreeItemId Root = AddRoot(wxT("Root"));
  int index = 0;
  ReadStrings(index, &Root, strings);
  //this->
  return true;
}
//----------------------------------------------------------------------------//
// TTrackBar implementation
//----------------------------------------------------------------------------//
BEGIN_EVENT_TABLE(TTrackBar, wxSlider)
  EVT_SCROLL(TTrackBar::ScrollEvent)
  EVT_LEFT_UP(TTrackBar::MouseUpEvent)
END_EVENT_TABLE()
//..............................................................................
TTrackBar::TTrackBar(wxWindow *Parent, const wxSize& sz) : 
  wxSlider(Parent, -1, 0, 0, 100, wxDefaultPosition, sz, wxSL_HORIZONTAL|wxSL_AUTOTICKS),
  WI(this)  {
  FActions = new TActionQList;
  OnChange = &FActions->NewQueue("ONCHANGE");
  OnMouseUp = &FActions->NewQueue("ONMOUSEUP");
  SetValue(0);
  this_Val = 0;
}
//..............................................................................
TTrackBar::~TTrackBar()  {  delete FActions;  }
//..............................................................................
void TTrackBar::ScrollEvent(wxScrollEvent& evt)  {
  if( this_Val == GetValue() )  return;
  this_Val = GetValue();
  if( !Data.IsEmpty() )  TOlxVars::SetVar(Data, this_Val);
  StartEvtProcessing()
    OnChange->Execute(this, &TEGC::New<olxstr>(GetOnChangeStr()));
  EndEvtProcessing()
}
//..............................................................................
void TTrackBar::MouseUpEvent(wxMouseEvent& evt)  {
  evt.Skip();
  StartEvtProcessing()
    OnMouseUp->Execute(this, &TEGC::New<olxstr>(GetOnMouseUpStr()));
  EndEvtProcessing()
}
//..............................................................................
//----------------------------------------------------------------------------//
// TTimer implementation
//----------------------------------------------------------------------------//
//..............................................................................
TTimer::TTimer()  {
  FActions = new TActionQList;
  FOnTimer = &FActions->NewQueue("ONTIMER");
}
//..............................................................................
TTimer::~TTimer()
{
  delete FActions;
}
//..............................................................................
void TTimer::Notify()  {
//  static bool running = false;
//  static long StartTime = TETime::EpochTime();
//  if( running )  return;
//  running = true;
  FOnTimer->Execute(this, NULL);
//  running = false;
}
//..............................................................................
TActionQueue* TTimer::OnTimer()  {
  return FOnTimer;
}
//..............................................................................

