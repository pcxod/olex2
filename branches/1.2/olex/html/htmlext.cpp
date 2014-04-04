/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "htmlext.h"
#include "htmlmanager.h"
#include "estack.h"
#include "exparse/exptree.h"
#include "htmlswitch.h"
#include "imgcellext.h"
#include "wx/tooltip.h"
#include "wx/clipbrd.h"
#include "../xglapp.h"
#include "olxstate.h"
#include "wxzipfs.h"

BEGIN_EVENT_TABLE(THtml, wxHtmlWindow)
  EVT_LEFT_DCLICK(THtml::OnMouseDblClick)
  EVT_LEFT_DOWN(THtml::OnMouseDown)
  EVT_LEFT_UP(THtml::OnMouseUp)
  EVT_MOTION(THtml::OnMouseMotion)
  EVT_SCROLL(THtml::OnScroll)
  EVT_CHILD_FOCUS(THtml::OnChildFocus)
  EVT_KEY_DOWN(THtml::OnKeyDown)
  EVT_CHAR(THtml::OnChar)
  EVT_SIZE(THtml::OnSizeEvt)
  EVT_TEXT_COPY(-1, THtml::OnClipboard)
END_EVENT_TABLE()
//.............................................................................
THtml::THtml(THtmlManager &manager, wxWindow *Parent,
  const olxstr &pop_name, int flags)
  : wxHtmlWindow(Parent, -1, wxDefaultPosition, wxDefaultSize, flags),
    PopupName(pop_name), WI(this), ObjectsState(*this),
    InFocus(NULL),
    Manager(manager),
    OnLink(Actions.New("ONLINK")),
    OnURL(Actions.New("ONURL")),
    OnDblClick(Actions.New("ONDBLCL")),
    OnKey(Actions.New("ONCHAR")),
    OnSize(Actions.New("ONSIZE"))
{
  Root = new THtmlSwitch(this, NULL);
  Movable = false;
  MouseDown = false;
  ShowTooltips = true;
  PageLoadRequested = false;
  if (stateTooltipsVisible() == InvalidIndex) {
    stateTooltipsVisible() = TStateRegistry::GetInstance().Register("htmlttvis",
      new TStateRegistry::Slot(
        TStateRegistry::NewGetter(*this, &THtml::GetShowTooltips),
        new TStateRegistry::TMacroSetter("html.Tooltips")
      )
    );
  }
}
//.............................................................................
THtml::~THtml()  {
  TFileHandlerManager::Clear();
  delete Root;
  ClearSwitchStates();
}
//.............................................................................
void THtml::OnLinkClicked(const wxHtmlLinkInfo& link)  {
  if (!MouseDown)  return;
  MouseDown = false;
  olxstr Href = link.GetHref();
  size_t ind = Href.FirstIndexOf('%');
  while (ind != InvalidIndex && ((ind+2) < Href.Length()) &&
    olxstr::o_ishexdigit(Href[ind + 1]) &&
    olxstr::o_ishexdigit(Href[ind + 2]))
  {
    int val = Href.SubString(ind+1, 2).RadInt<int>(16);
    Href.Delete(ind, 3);
    Href.Insert(val, ind);
    ind = Href.FirstIndexOf('%');
  }
  if (!OnLink.Execute(this, (IEObject*)&Href)) {
    wxHtmlLinkInfo NewLink(Href.u_str(), link.GetTarget());
    wxHtmlWindow::OnLinkClicked(NewLink);
  }
}
//.............................................................................
wxHtmlOpeningStatus THtml::OnOpeningURL(wxHtmlURLType type, const wxString& url,
  wxString *redirect) const {
  olxstr Url = url;
  if( !OnURL.Execute(this, &Url) )  return wxHTML_OPEN;
  return wxHTML_BLOCK;
}
//.............................................................................
void THtml::SetSwitchState(THtmlSwitch& sw, size_t state)  {
  const size_t ind = SwitchStates.IndexOf( sw.GetName() );
  if( ind == InvalidIndex )
    SwitchStates.Add(sw.GetName(), state);
  else
    SwitchStates.GetObject(ind) = state;
}
//.............................................................................
size_t THtml::GetSwitchState(const olxstr& switchName)  {
  const size_t ind = SwitchStates.IndexOf( switchName );
  return (ind == InvalidIndex) ? UnknownSwitchState : SwitchStates.GetObject(ind);
}
//.............................................................................
void THtml::OnMouseDown(wxMouseEvent& event)  {
  this->SetFocusIgnoringChildren();
  MouseX = event.GetX();
  MouseY = event.GetY();
  MouseDown = true;
  if( Movable )
    SetCursor(wxCursor(wxCURSOR_SIZING));
  event.Skip();
}
//.............................................................................
void THtml::OnMouseUp(wxMouseEvent& event)  {
  if( Movable && MouseDown )
    SetCursor(wxCursor(wxCURSOR_ARROW));
  event.Skip();
}
//.............................................................................
void THtml::OnMouseMotion(wxMouseEvent& event)  {
  if( !Movable || !MouseDown )  {
    event.Skip();
    return;
  }
  int dx = event.GetX() - MouseX;
  int dy = event.GetY() - MouseY;
  if( !dx && !dy )  return;
  wxWindow *parent = GetParent();
  if( parent == NULL || parent->GetParent() == NULL )  return;

  int x=0, y=0;
  parent->GetPosition(&x, &y);
  parent->SetSize(x+dx, y+dy, -1, -1, wxSIZE_USE_EXISTING);
}
//.............................................................................
void THtml::OnMouseDblClick(wxMouseEvent& event)  {
  event.Skip();
  volatile THtmlManager::DestructionLocker dm =
    Manager.LockDestruction(this, this);
  OnDblClick.Execute(this, &OnDblClickData);
}
//.............................................................................
void THtml::OnSizeEvt(wxSizeEvent& event)  {
  event.Skip();
  volatile THtmlManager::DestructionLocker dm =
    Manager.LockDestruction(this, this);
  OnSize.Execute(this, &OnSizeData);
}
//.............................................................................
bool THtml::Dispatch(int MsgId, short MsgSubId, const IEObject* Sender,
  const IEObject* Data, TActionQueue *)
{
  if( MsgId == html_parent_resize )  {
    volatile THtmlManager::DestructionLocker dm =
      Manager.LockDestruction(this, this);
    OnSize.Execute(this, &OnSizeData);
  }
  return true;
}
//.............................................................................
void THtml::OnChildFocus(wxChildFocusEvent& event)  {
  wxWindow *wx_next = event.GetWindow(), 
    *focused = FindFocus();
  /* this happens when the child windows is in visible to API like in
  combo/spinctls */
  if (wx_next != focused)  {
    if (focused->GetParent() == wx_next) {
      focused = wx_next;
    }
    else if(InFocus != NULL) { // this would be odd
      InFocus->SetFocus();
      return;
    }
  }
  AOlxCtrl* prev = NULL, *next = NULL;
  for( size_t i=0; i < Traversables.Count(); i++ )  {
    if( Traversables[i].GetB() == InFocus )
      prev = Traversables[i].GetA();
    if( Traversables[i].GetB() == wx_next )
      next = Traversables[i].GetA();
  }
  bool skip = true;
  if( prev != next || next == NULL || prev == NULL )  {
    skip = DoHandleFocusEvent(prev, next);
    InFocus = wx_next;
  }
  event.Skip();
}
//.............................................................................
void THtml::_FindNext(index_t from, index_t& dest, bool scroll) const {
  if( Traversables.IsEmpty() )  {
    dest = -1;
    return;
  }
  int i = from;
  while( ++i < (int)Traversables.Count() && Traversables[i].GetB() == NULL )
    ;
  dest = ((i >= (int)Traversables.Count() ||
    (Traversables[i].GetB() == NULL)) ? -1 : i);
  if( dest == -1 && scroll )  {
    i = -1;
    while( ++i < from && Traversables[i].GetB() == NULL )
      ;
    dest = ((Traversables[i].GetB() == NULL) ? -1 : i);
  }
}
//.............................................................................
void THtml::_FindPrev(index_t from, index_t& dest, bool scroll) const {
  if( Traversables.IsEmpty() )  {
    dest = -1;
    return;
  }
  if( from < 0 )  from = Traversables.Count();
  index_t i = from;
  while( --i >= 0 && Traversables[i].GetB() == NULL )
    ;
  dest = ((i < 0 || (Traversables[i].GetB() == NULL)) ? -1 : i);
  if( dest == -1 && scroll )  {
    i = Traversables.Count();
    while( --i > from && Traversables[i].GetB() == NULL )
      ;
    dest = ((Traversables[i].GetB() == NULL) ? -1 : i);
  }
}
//.............................................................................
void THtml::GetTraversibleIndeces(index_t& current, index_t& another,
  bool forward) const
{
  wxWindow* w = FindFocus();
  // no focus? find the one at the edge
  if( w == NULL || w == static_cast<const wxWindow*>(this) )  {
    if( forward )
      _FindNext(-1, another, false);
    else
      _FindPrev(Traversables.Count(), another, false);
  }
  else  {
    for( size_t i=0; i < Traversables.Count(); i++ )  {
      if( Traversables[i].GetB() == w ||
          Traversables[i].GetB() == w->GetParent() )
      {
        current = i;
        break;
      }
    }
    if( forward )
      _FindNext(current, another, true);
    else
      _FindPrev(current , another, true);
  }
}
//.............................................................................
bool THtml::DoHandleFocusEvent(AOlxCtrl* prev, AOlxCtrl* next)  {
  // prevent page re-loading and object deletion
  volatile THtmlManager::DestructionLocker dm =
    Manager.LockDestruction(this, this);
  bool rv = false;
  if( prev != NULL )  {
    if( EsdlInstanceOf(*prev, TTextEdit) )  {
      olxstr s = ((TTextEdit*)prev)->OnLeave.data;
      ((TTextEdit*)prev)->OnLeave.Execute(prev, &s);
    }
    else if( EsdlInstanceOf(*prev, TComboBox) )  {
      ((TComboBox*)prev)->HandleOnLeave();
    }
    else
      rv = true;
  }
  if (next != NULL) {
    if (EsdlInstanceOf(*next, TTextEdit)) {
      olxstr s = ((TTextEdit*)next)->OnEnter.data;
      ((TTextEdit*)next)->OnEnter.Execute(next, &s);
      if (!((TTextEdit*)next)->IsReadOnly())
        ((TTextEdit*)next)->SetSelection(-1,-1);
    }
    else if (EsdlInstanceOf(*next, TComboBox)) {
      ((TComboBox*)next)->HandleOnEnter();
      if (!((TTextEdit*)next)->IsReadOnly())
        ((TComboBox*)next)->SetSelection(-1,-1);
    }
    else
      rv = true;
  }
  return rv;
}
//.............................................................................
void THtml::DoNavigate(bool forward)  {
  index_t current=-1, another=-1;
  GetTraversibleIndeces(current, another, forward);
  DoHandleFocusEvent( 
    current == -1 ? NULL : Traversables[current].GetA(),
    another == -1 ? NULL : Traversables[another].GetA());
  if( another != -1 )  {
    InFocus = Traversables[another].GetB();
    InFocus->SetFocus();
    InFocus = FindFocus();
    for( size_t i=0; i < Objects.Count(); i++ )  {
      if( Objects.GetValue(i).GetB() == NULL )  continue;
      if( Objects.GetValue(i).GetB() == InFocus
#ifdef __WIN32__
          || Objects.GetValue(i).GetB() == InFocus->GetParent()
#endif
        )
      {
        FocusedControl = Objects.GetKey(i);
        break;
      }
    }
  }
}
//.............................................................................
void THtml::OnKeyDown(wxKeyEvent& event)  {
  if( event.GetKeyCode() == WXK_TAB )
    DoNavigate( event.GetModifiers() != wxMOD_SHIFT );
  else
    event.Skip();
}
//.............................................................................
void THtml::OnNavigation(wxNavigationKeyEvent& event)  {
  if( event.IsFromTab() )
    DoNavigate( event.GetDirection() );
  else
    event.Skip();
}
//.............................................................................
void THtml::OnChar(wxKeyEvent& event)  {
  //Ctrl+c
  if( event.m_controlDown && event.GetKeyCode() == 'c'-'a'+1 )  {
    CopySelection();
    event.Skip();
    return;
  }
  wxWindow* parent = GetParent();
  if( parent != NULL )  {
    wxDialog* dlg = dynamic_cast<wxDialog*>(parent);
    if( dlg != NULL )
      return;
  }
  TKeyEvent KE(event);
  OnKey.Execute(this, &KE);
}
//.............................................................................
void THtml::UpdateSwitchState(THtmlSwitch &Switch, olxstr &String)  {
  if( !Switch.IsToUpdateSwitch() )  return;
  olxstr Tmp = "<!-- #include ";
  Tmp << Switch.GetName() << ' ';
  for( size_t i=0; i < Switch.FileCount(); i++ )
    Tmp << Switch.GetFile(i) << ';';
  for( size_t i=0; i < Switch.GetParams().Count(); i++ )  {
    Tmp << Switch.GetParams().GetName(i) << '=';
    if( Switch.GetParams().GetValue(i).FirstIndexOf(' ') == InvalidIndex )
      Tmp << Switch.GetParams().GetValue(i);
    else
      Tmp << '\'' << Switch.GetParams().GetValue(i) << '\'';
    Tmp << ';';
  }

  Tmp << Switch.GetFileIndex()+1 << ';' << " -->";
  String = Tmp;
}
//.............................................................................
void THtml::CheckForSwitches(THtmlSwitch &Sender, bool izZip)  {
  TStrPObjList<olxstr,THtmlSwitch*>& Lst = Sender.GetStrings();
  TStrList Toks;
  using namespace exparse::parser_util;
  static const olxstr Tag  = "<!-- #include ",
           Tag1 = "<!-- #includeif ",
           Tag2 = "<!-- #include",
           comment_open = "<!--", comment_close = "-->";
  Olex2App *app=NULL;
  try { app = &Olex2App::GetInstance(); }
  catch(...) {}
  olex2::IOlex2Processor *op = olex2::IOlex2Processor::GetInstance();
  for( size_t i=0; i < Lst.Count(); i++ )  {
    Lst[i].Replace("~popup_name~", PopupName);
    olxstr tmp = olxstr(Lst[i]).TrimWhiteChars();
    // skip comments
    if( tmp.StartsFrom(comment_open) && !tmp.StartsFrom(Tag2) )  {
      //tag_parse_info tpi = skip_tag(Lst, Tag2, Tag3, i, 0);
      if( tmp.EndsWith(comment_close) )  continue;
      bool tag_found = false;
      while( ++i < Lst.Count() )  {
        if( olxstr(Lst[i]).TrimWhiteChars().EndsWith(comment_close) )  {
          tag_found = true;
          break;
        }
      }
      if( tag_found )  continue;
      else  
        break;
    }

    // TRANSLATION START
    if (app != NULL) {
      Lst[i] = app->TranslateString(Lst[i]);
      size_t bs = Lst[i].IndexOf("$+");
      if (bs != InvalidIndex) {
        TStrList blk;
        blk.Add(Lst[i].SubStringFrom(bs+2).TrimWhiteChars(true, false));
        Lst[i].Delete(bs, Lst[i].Length()-bs);
        size_t bi=i;
        while (++bi < Lst.Count()) {
          size_t be = Lst[bi].IndexOf("$-");
          if (be != InvalidIndex) {
            blk.Add(Lst[bi].SubStringTo(be).TrimWhiteChars(true, false));
            Lst[bi].Delete(0, be+2);
            break;
          }
          else
            blk.Add(Lst[bi].TrimWhiteChars(true, false));
        }
        Lst.DeleteRange(i+1, blk.Count()-2);
        Lst.Insert(i+1, olxstr('$') << blk.Text(EmptyString()), NULL);
      }
      if (Lst[i].IndexOf("$") != InvalidIndex && op != NULL) {
        op->processFunction(Lst[i],
          olxstr(Sender.GetCurrentFile()) << '#' << (i+1));
      }
    }
    // TRANSLATION END
    int stm = (Lst[i].StartsFrom(Tag1) ? 1 : 0);
    if( stm == 0 )  stm = (Lst[i].StartsFrom(Tag) ? 2 : 0);
    if( stm != 0 )  {
      tmp = Lst[i].SubStringFrom(stm == 1 ? Tag1.Length() : Tag.Length());
      Toks.Clear();
      Toks.Strtok(tmp, ' '); // extract item name
      if( (stm == 1 && Toks.Count() < 4) ||
          (stm == 2 && Toks.Count() < 3) )  {
        TBasicApp::NewLogEntry(logError) << "Wrong #include[if] syntax: " << tmp;
        continue;
      }
      if( stm == 1 )  {
        tmp = Toks[0];
        if( op != NULL && op->processFunction(tmp) )  {
          if( !tmp.ToBool() )  continue;
        }
        else  continue;
        Toks.Delete(0);
      }
      THtmlSwitch* Sw = &Sender.NewSwitch();
      Lst.GetObject(i) = Sw;
      Sw->SetName(Toks[0]);
      tmp = Toks.Text(' ', 1, Toks.Count()-1);
      Toks.Clear();
      TParamList::StrtokParams(tmp, ';', Toks); // extract arguments
      if( Toks.Count() < 2 )  { // must be at least 2 for filename and status
        TBasicApp::NewLogEntry(logError) <<
          "Wrong defined switch (not enough data)" << Sw->GetName();
        continue;
      }

      for( size_t j=0; j < Toks.Count()-1; j++ )  {
        if( Toks[j].FirstIndexOf('=') == InvalidIndex )  {
          if( izZip && !TZipWrapper::IsZipFile(Toks[j]) )  {
            if( Toks[j].StartsFrom('\\') || Toks[j].StartsFrom('/') )
              tmp = Toks[j].SubStringFrom(1);
            else {
              tmp = TZipWrapper::ComposeFileName(
                Sender.GetFile(Sender.GetFileIndex()), Toks[j]);
            }
          }
          else
            tmp = Toks[j];
          if (op != NULL)
            op->processFunction(tmp);
          Sw->AddFile(tmp);
        }
        else  {
          // check for parameters
          if( Toks[j].IndexOf('#') != InvalidIndex )  {
            tmp = Toks[j];
            for( size_t k=0; k < Sender.GetParams().Count(); k++ )  {
              olxstr tmp1 = olxstr().Allocate(
                  Sender.GetParams().GetName(k).Length()+2) <<
                '#' << Sender.GetParams().GetName(k);
              tmp.Replace(tmp1, Sender.GetParams().GetValue(k));
            }
            Sw->AddParam(tmp);
          }
          else
            Sw->AddParam(Toks[j]);
        }
      }

      size_t switchState = GetSwitchState(Sw->GetName()), index = InvalidIndex;
      if( switchState == UnknownSwitchState )  {
        index_t iv = Toks.GetLastString().RadInt<index_t>();
        if( iv < 0 )
          Sw->SetUpdateSwitch(false);
        index = olx_abs(iv)-1;
      }
      else
        index = switchState;
      Sw->SetFileIndex(index);
    }
  }
}
//.............................................................................
bool THtml::ProcessPageLoadRequest()  {
  if (!PageLoadRequested || IsPageLocked()) return false;
  PageLoadRequested = false;
  bool res = false;
  if( !PageRequested.IsEmpty() )
    res = LoadPage(PageRequested.u_str());
  else
    res = UpdatePage();
  PageRequested.SetLength(0);
  return res;
}
//.............................................................................
bool THtml::LoadPage(const wxString &file)  {
  if (file.IsEmpty())
    return false;

  if (IsPageLocked()) {
    PageLoadRequested = true;
    PageRequested = file;
    return true;
  }
  if (this->WI.GetWidth() < 100) {
    return false;
  }
  olxstr File(file), TestFile(file);
  olxstr Path = TEFile::ExtractFilePath(File);
  TestFile = TEFile::ExtractFileName(File);
  if( Path.Length() > 1 )  {
    Path = TEFile::OSPath(Path);
    if( TEFile::IsAbsolutePath(Path) )
      WebFolder = Path;
  }
  else
    Path = WebFolder;
  if( Path == WebFolder )
    TestFile = WebFolder + TestFile;
  else
    TestFile = WebFolder + Path + TestFile;

  if( !TZipWrapper::IsValidFileName(TestFile) &&
      !TFileHandlerManager::Exists(file) )
  {
    throw TFileDoesNotExistException(__OlxSourceInfo, file);
  }
  Root->Clear();
  Root->ClearFiles();
  Root->AddFile(File);

  Root->SetFileIndex(0);
  Root->UpdateFileIndex();
  FileName = File;
  return UpdatePage(false);
}
//.............................................................................
bool THtml::ItemState(const olxstr &ItemName, short State)  {
  THtmlSwitch * Sw = Root->FindSwitch(ItemName);
  if( Sw == NULL )  {
    TBasicApp::NewLogEntry(logError) << "THtml::ItemState: unresolved: "
      << ItemName;
    return false;
  }
  Sw->SetFileIndex(State-1);
  return true;
}
//.............................................................................
bool THtml::UpdatePage(bool update_indices)  {
  if( IsPageLocked() )  {
    PageLoadRequested = true;
    PageRequested.SetLength(0);
    return true;
  }

  olxstr Path(TEFile::ExtractFilePath(FileName));
  if( TEFile::IsAbsolutePath(FileName) )  {
    Path = TEFile::OSPath(Path);
    WebFolder = Path;
  }
  else
    Path = WebFolder;

  olxstr oldPath(TEFile::CurrentDir());
  TEFile::ChangeDir(WebFolder);
  if (update_indices) { // reload switches
    for (size_t i=0; i < Root->SwitchCount(); i++)
      Root->GetSwitch(i).UpdateFileIndex();
  }

  TStrList Res;
  Root->ToStrings(Res);
  ObjectsState.SaveState();
  Objects.Clear();
  Traversables.Clear();
  InFocus = NULL;
  int xPos = -1, yPos = -1, xWnd=-1, yWnd = -1;
  wxHtmlWindow::GetViewStart(&xPos, &yPos);
#if defined(__WIN32__)
  wxHtmlWindow::Freeze();
#else
  Hide();
#endif
  SetPage(Res.Text(' ').u_str());
  ObjectsState.RestoreState();
  wxHtmlWindow::Scroll(xPos, yPos);
#if defined(__MAC__) || (defined(__linux__) && !wxCHECK_VERSION(2,9,0))
  CreateLayout();
  Show();
  Refresh();
  Update();
#endif
  for( size_t i=0; i < Objects.Count(); i++ )  {
    if( Objects.GetValue(i).B() != NULL )  {
#ifndef __MAC__
      Objects.GetValue(i).B()->Move(16000, 0);
#endif
      Objects.GetValue(i).B()->Show();
    }
  }
#if defined(__WIN32__)
  CreateLayout();
  Thaw();
#elif defined(__linux__) && wxCHECK_VERSION(2,9,0)
  CreateLayout();
  Show();
  Refresh();
  Update();
#endif
  SwitchSources().Clear();
  SwitchSource().SetLength(0);
  TEFile::ChangeDir(oldPath);
  if( !FocusedControl.IsEmpty() )  {
    size_t ind = Objects.IndexOf( FocusedControl );
    if( ind != InvalidIndex )  {
      wxWindow* wnd = Objects.GetValue(ind).B();
      if( EsdlInstanceOf(*wnd, TTextEdit) )
        ((TTextEdit*)wnd)->SetSelection(-1,-1);
      else if( EsdlInstanceOf(*wnd, TComboBox) )  {
        TComboBox* cb = (TComboBox*)wnd;
        wnd = cb;
      }
      else if( EsdlInstanceOf(*wnd, TSpinCtrl) )  {
        TSpinCtrl* sc = (TSpinCtrl*)wnd;
        olxstr sv(sc->GetValue());
        sc->SetSelection((long)sv.Length(), -1);
      }
      wnd->SetFocus();
      InFocus = wnd;
    }
    else
      FocusedControl.SetLength(0);
  }
  return true;
}
//.............................................................................
void THtml::OnScroll(wxScrollEvent& evt)  {  // this is never called at least on GTK
  evt.Skip();
}
//.............................................................................
void THtml::ScrollWindow(int dx, int dy, const wxRect* rect)  {
  wxHtmlWindow::ScrollWindow(dx,dy,rect);
}
//.............................................................................
bool THtml::AddObject(const olxstr& Name, AOlxCtrl *Object, wxWindow* wxWin,
  bool Manage)
{
  Traversables.Add(Association::New(Object,wxWin));
  if( Name.IsEmpty() )  return true;  // an anonymous object
  if( Objects.IndexOf(Name) != InvalidIndex )  return false;
  Objects.Add(Name, Association::Create(Object, wxWin, Manage));
  return true;
}
//.............................................................................
void THtml::OnCellMouseHover(wxHtmlCell *Cell, wxCoord x, wxCoord y)  {
  wxHtmlLinkInfo *Link = Cell->GetLink(x, y);
  if( Link != NULL )  {
    olxstr Href = Link->GetTarget();
    if( Href.IsEmpty() )
      Href = Link->GetHref();

    size_t ind = Href.FirstIndexOf('%');
    while( ind != InvalidIndex && ((ind+2) < Href.Length()) )  {
      if( Href.CharAt(ind+1) == '%' )  {
        Href.Delete(ind, 1);
        ind = Href.FirstIndexOf('%', ind+1);
        continue;
      }
      olxstr nm = Href.SubString(ind+1, 2); 
      if( nm.IsNumber() )  {
        try  {
          int val = nm.RadInt<int>(16);
          Href.Delete(ind, 3);
          Href.Insert((char)val, ind);
        }
        catch(...)  {}
      }
      ind = Href.FirstIndexOf('%', ind+1);
    }
    if( ShowTooltips )  {
      wxToolTip *tt = GetToolTip();
      wxString wxs(Href.Replace("#href", Link->GetHref()).u_str());
      if( tt == NULL || tt->GetTip() != wxs )  {
        SetToolTip( wxs );
      }
    }
  }
  else
    SetToolTip(NULL);
}
//.............................................................................
olxstr THtml::GetObjectValue(const AOlxCtrl *Obj)  {
  if( EsdlInstanceOf(*Obj, TTextEdit) )  {  return ((TTextEdit*)Obj)->GetText();  }
  if( EsdlInstanceOf(*Obj, TCheckBox) )  {  return ((TCheckBox*)Obj)->GetCaption();  }
  if( EsdlInstanceOf(*Obj, TTrackBar) )  {  return ((TTrackBar*)Obj)->GetValue(); }
  if( EsdlInstanceOf(*Obj, TSpinCtrl) )  {  return ((TSpinCtrl*)Obj)->GetValue(); }
  if( EsdlInstanceOf(*Obj, TButton) )    {  return ((TButton*)Obj)->GetCaption();    }
  if( EsdlInstanceOf(*Obj, TComboBox) )  {  return ((TComboBox*)Obj)->GetText();  }
  if( EsdlInstanceOf(*Obj, TListBox) )   {  return ((TListBox*)Obj)->GetValue();  }
  if( EsdlInstanceOf(*Obj, TTreeView) )  {
    TTreeView* T = (TTreeView*)Obj;
    wxTreeItemId ni = T->GetSelection();
    wxTreeItemData* td = T->GetItemData(ni);
    if( td == NULL || !EsdlInstanceOf(*td, TTreeNodeData) )
      return EmptyString();
    TTreeNodeData* olx_td = dynamic_cast<TTreeNodeData*>(td);
    if( olx_td == NULL || olx_td->GetData() == NULL )
      return EmptyString();
    return olx_td->GetData()->ToString();
  }
  if( EsdlInstanceOf(*Obj, TDateCtrl) )  {
    return ((TDateCtrl*)Obj)->GetValue().Format(wxT("%d/%m/%Y"));
  }
  if( EsdlInstanceOf(*Obj, TColorCtrl) )  {
    wxColor c = ((TColorCtrl*)Obj)->GetColour();
    return OLX_RGBA(c.Red(), c.Green(), c.Blue(), c.Alpha());
  }
  return EmptyString();
}
//.............................................................................
void THtml::SetObjectValue(AOlxCtrl *Obj, const olxstr& Value)  {
  if( EsdlInstanceOf(*Obj, TTextEdit) )
    ((TTextEdit*)Obj)->SetText(Value);
  else if( EsdlInstanceOf(*Obj, TCheckBox) )
    ((TCheckBox*)Obj)->SetCaption(Value);
  else if( EsdlInstanceOf(*Obj, TTrackBar) )  {
    const size_t si = Value.IndexOf(',');
    if( si == InvalidIndex )
      ((TTrackBar*)Obj)->SetValue(olx_round(Value.ToDouble()));
    else
      ((TTrackBar*)Obj)->SetRange(olx_round(Value.SubStringTo(si).ToDouble()),
      olx_round(Value.SubStringFrom(si+1).ToDouble()));
  }
  else if( EsdlInstanceOf(*Obj, TSpinCtrl) )  {
    const size_t si = Value.IndexOf(',');
    if( si == InvalidIndex )
      ((TSpinCtrl*)Obj)->SetValue(olx_round(Value.ToDouble()));
    else
      ((TSpinCtrl*)Obj)->SetRange(olx_round(Value.SubStringTo(si).ToDouble()),
      olx_round(Value.SubStringFrom(si+1).ToDouble()));
  }
  else if( EsdlInstanceOf(*Obj, TButton) )
    ((TButton*)Obj)->SetLabel(Value.u_str());
  else if( EsdlInstanceOf(*Obj, TComboBox) )  {
    ((TComboBox*)Obj)->SetText(Value);
    ((TComboBox*)Obj)->Update();
  }
  else if( EsdlInstanceOf(*Obj, TListBox) )  {
    TListBox *L = (TListBox*)Obj;
    int index = L->GetSelection();
    if( index >=0 && index < L->Count() )
      L->SetString(index, Value.u_str());
  }
  else if( EsdlInstanceOf(*Obj, TDateCtrl) )  {
    wxDateTime dt;
    dt.ParseDateTime(Value.u_str());
    ((TDateCtrl*)Obj)->SetValue(dt);
  }
  else if( EsdlInstanceOf(*Obj, TColorCtrl) )  {
    ((TColorCtrl*)Obj)->SetColour(wxColor(Value.u_str()));
  }
  else
    return;
}
//.............................................................................
const olxstr& THtml::GetObjectData(const AOlxCtrl *Obj)  {
  if( EsdlInstanceOf(*Obj, TTextEdit) )  {  return ((TTextEdit*)Obj)->GetData();  }
  if( EsdlInstanceOf(*Obj, TCheckBox) )  {  return ((TCheckBox*)Obj)->GetData();  }
  if( EsdlInstanceOf(*Obj, TTrackBar) )  {  return ((TTrackBar*)Obj)->GetData(); }
  if( EsdlInstanceOf(*Obj, TSpinCtrl) )  {  return ((TSpinCtrl*)Obj)->GetData(); }
  if( EsdlInstanceOf(*Obj, TButton) )    {  return ((TButton*)Obj)->GetData();    }
  if( EsdlInstanceOf(*Obj, TComboBox) )  {  return ((TComboBox*)Obj)->GetData();  }
  if( EsdlInstanceOf(*Obj, TListBox) )  {  return ((TListBox*)Obj)->GetData();  }
  if( EsdlInstanceOf(*Obj, TTreeView) )  {  return ((TTreeView*)Obj)->GetData();  }
  return EmptyString();
}
//.............................................................................
olxstr THtml::GetObjectImage(const AOlxCtrl* Obj)  {
  if( EsdlInstanceOf(*Obj, TBmpButton) )
    return ((TBmpButton*)Obj)->GetSource();
  else if( EsdlInstanceOf(*Obj, THtmlImageCell) )
    return ((THtmlImageCell*)Obj)->GetSource();
  return EmptyString();
}
//.............................................................................
bool THtml::SetObjectImage(AOlxCtrl* Obj, const olxstr& src)  {
  if( src.IsEmpty() )
    return false;
  if( EsdlInstanceOf(*Obj, TBmpButton) || EsdlInstanceOf(*Obj, THtmlImageCell) )  {
    wxFSFile *fsFile = TFileHandlerManager::GetFSFileHandler(src);
    if( fsFile == NULL )  {
      TBasicApp::NewLogEntry(logError) <<
        "Setimage: could not locate specified file: " << src;
      return false;
    }
    wxImage image(*(fsFile->GetStream()), wxBITMAP_TYPE_ANY);
    delete fsFile;
    if ( !image.Ok() )  {
      TBasicApp::NewLogEntry(logError) <<
        "Setimage: could not read specified file: " << src;
      return false;
    }
    if( EsdlInstanceOf(*Obj, TBmpButton) )  {
      ((TBmpButton*)Obj)->SetBitmapLabel(image);
      ((TBmpButton*)Obj)->SetSource(src);
      ((TBmpButton*)Obj)->Refresh(true);
    }
    else if( EsdlInstanceOf(*Obj, THtmlImageCell) )  {
      ((THtmlImageCell*)Obj)->SetImage(image);
      ((THtmlImageCell*)Obj)->SetSource(src);
      ((THtmlImageCell*)Obj)->GetWindow()->Refresh(true);
    }
  }
  else if( EsdlInstanceOf(*Obj, TImgButton) )  {
    ((TImgButton*)Obj)->SetImages(src);
    ((TImgButton*)Obj)->Refresh(true);
  }
  else  {
    TBasicApp::NewLogEntry(logError) << "Setimage: unsupported object type";
    return false;
  }
  return true;
}
//.............................................................................
olxstr THtml::GetObjectItems(const AOlxCtrl* Obj)  {
  if( EsdlInstanceOf(*Obj, TComboBox) )  {
    return ((TComboBox*)Obj)->ItemsToString(';');
  }
  else if( EsdlInstanceOf(*Obj, TListBox) )  {
    return ((TListBox*)Obj)->ItemsToString(';');
  }
  return EmptyString();
}
//.............................................................................
bool THtml::SetObjectItems(AOlxCtrl* Obj, const olxstr& src)  {
  if( src.IsEmpty() )  return false;
  if( EsdlInstanceOf(*Obj, TComboBox) )  {
    TComboBox *cb = ((TComboBox*)Obj);
    cb->Clear();
    cb->AddItems(TStrList(src, ';'));
    if (cb->IsReadOnly() && cb->GetCount() > 0)
      cb->SetValue(cb->GetString(0));
  }
  else if( EsdlInstanceOf(*Obj, TListBox) )  {
    ((TListBox*)Obj)->Clear();
    ((TListBox*)Obj)->AddItems(TStrList(src, ';'));
  }
  else  {
    TBasicApp::NewLogEntry(logError) << "SetItems: unsupported object type";
    return false;
  }
  return true;
}
//.............................................................................
void THtml::SetObjectData(AOlxCtrl *Obj, const olxstr& Data)  {
  if( EsdlInstanceOf(*Obj, TTextEdit) )  {  ((TTextEdit*)Obj)->SetData(Data);  return;  }
  if( EsdlInstanceOf(*Obj, TCheckBox) )  {  ((TCheckBox*)Obj)->SetData(Data);  return;  }
  if( EsdlInstanceOf(*Obj, TTrackBar) )  {  ((TTrackBar*)Obj)->SetData(Data);  return;  }
  if( EsdlInstanceOf(*Obj, TSpinCtrl) )  {  ((TSpinCtrl*)Obj)->SetData(Data);  return;  }
  if( EsdlInstanceOf(*Obj, TButton) )    {  ((TButton*)Obj)->SetData(Data);    return;  }
  if( EsdlInstanceOf(*Obj, TComboBox) )  {  ((TComboBox*)Obj)->SetData(Data);  return;  }
  if( EsdlInstanceOf(*Obj, TListBox) )  {  ((TListBox*)Obj)->SetData(Data);  return;  }
  if( EsdlInstanceOf(*Obj, TTreeView) )  {  ((TTreeView*)Obj)->SetData(Data);  return;  }
}
//.............................................................................
bool THtml::GetObjectState(const AOlxCtrl *Obj, const olxstr& state)  {
  if( EsdlInstanceOf(*Obj, TCheckBox) )
    return ((TCheckBox*)Obj)->IsChecked();
  else if( EsdlInstanceOf(*Obj, TImgButton) )  {
    if( state.Equalsi("enabled") )
      return ((TImgButton*)Obj)->IsEnabled();
    else if( state.Equalsi("down") )
      return ((TImgButton*)Obj)->IsDown();
    else
      return false;
  }
  else if( EsdlInstanceOf(*Obj, TButton) )
    return ((TButton*)Obj)->IsDown();
  else
    return false;
}
//.............................................................................
void THtml::SetObjectState(AOlxCtrl *Obj, bool State, const olxstr& state_name)  {
  if( EsdlInstanceOf(*Obj, TCheckBox) )
    ((TCheckBox*)Obj)->SetChecked(State);
  else if( EsdlInstanceOf(*Obj, TImgButton) )  {
    if( state_name.Equalsi("enabled") )
      ((TImgButton*)Obj)->SetEnabled(State);
    else if( state_name.Equalsi("down") )
      ((TImgButton*)Obj)->SetDown(State);
    ((TImgButton*)Obj)->Refresh(true);
  }
  else if( EsdlInstanceOf(*Obj, TButton) )
    ((TButton*)Obj)->SetDown(State);
}
//.............................................................................
//.............................................................................
//.............................................................................
THtml::TObjectsState::~TObjectsState()  {
  for( size_t i=0; i < Objects.Count(); i++ )
    delete Objects.GetObject(i);
}
//.............................................................................
void THtml::TObjectsState::SaveState()  {
  for( size_t i=0; i < html.ObjectCount(); i++ )  {
    if( !html.IsObjectManageble(i) )  continue;
    size_t ind = Objects.IndexOf(html.GetObjectName(i));
    AOlxCtrl* obj = html.GetObject(i);
    wxWindow* win = html.GetWindow(i);
    TSStrStrList<olxstr,false>* props;
    if( ind == InvalidIndex )  {
      props = new TSStrStrList<olxstr,false>;
      Objects.Add(html.GetObjectName(i), props);
    }
    else  {
      props = Objects.GetObject(ind);
      props->Clear();
    }
    props->Add("type", EsdlClassName(*obj));  // type
    if( EsdlInstanceOf(*obj, TTextEdit))  {
      TTextEdit* te = (TTextEdit*)obj;
      props->Add("val", te->GetText());
      props->Add("data", te->GetData());
    }
    else if( EsdlInstanceOf(*obj, TCheckBox) )  {
      TCheckBox* cb = (TCheckBox*)obj;
      props->Add("val", cb->GetCaption());
      props->Add("checked", cb->IsChecked());
      props->Add("data", cb->GetData());
    }
    else if( EsdlInstanceOf(*obj, TTrackBar) )  {
      TTrackBar* tb = (TTrackBar*)obj;
      props->Add("min", tb->GetMin());
      props->Add("max", tb->GetMax());
      props->Add("val", tb->GetValue());
      props->Add("data", tb->GetData());
    }
    else if( EsdlInstanceOf(*obj, TSpinCtrl) )  {
      TSpinCtrl* sc = (TSpinCtrl*)obj;
      props->Add("min", sc->GetMin());
      props->Add("max", sc->GetMax());
      props->Add("val", sc->GetValue());
      props->Add("data", sc->GetData());
    }
    else if( EsdlInstanceOf(*obj, TButton) )  {
      TButton* bt = (TButton*)obj;
      props->Add("val", bt->GetCaption());
      props->Add("checked", bt->IsDown());
      props->Add("data", bt->GetData());
    }
    else if( EsdlInstanceOf(*obj, TBmpButton) )  {
      TBmpButton* bt = (TBmpButton*)obj;
      props->Add("checked", bt->IsDown());
      props->Add("val", bt->GetSource());
      props->Add("data", bt->GetData());
    }
    else if( EsdlInstanceOf(*obj, TImgButton) )  {
      TImgButton* bt = (TImgButton*)obj;
      props->Add("checked", bt->IsDown());
      props->Add("enabled", bt->IsEnabled());
      props->Add("val", bt->GetSource());
      props->Add("width", bt->GetWidth());
      props->Add("height", bt->GetHeight());
      props->Add("data", bt->GetData());
    }
    else if( EsdlInstanceOf(*obj, TComboBox) )  {
      TComboBox* cb = (TComboBox*)obj;
      props->Add("val", (cb->HasValue() ? cb->GetValue() : EmptyString()));
      props->Add("items", cb->ItemsToString(';'));
      props->Add("data", cb->GetData());
    }
    else if( EsdlInstanceOf(*obj, TListBox) )  {
      TListBox* lb = (TListBox*)obj;
      props->Add("val", lb->GetValue());
      props->Add("items", lb->ItemsToString(';'));
      props->Add("data", lb->GetData());
    }
    else if( EsdlInstanceOf(*obj, TTreeView) )  {
      TTreeView* tv = (TTreeView*)obj;
      props->Add("state", tv->SaveState());
    }
    else if( EsdlInstanceOf(*obj, TLabel) )  {
      TLabel* lb = (TLabel*)obj;
      props->Add("val", lb->GetCaption());
      props->Add("data", lb->GetData());
    }
    else //?
      ;
    // stroring the control colours, it is generic 
    if( win != NULL )  {
      props->Add("fg", win->GetForegroundColour().GetAsString(wxC2S_HTML_SYNTAX));
      props->Add("bg", win->GetBackgroundColour().GetAsString(wxC2S_HTML_SYNTAX));
    }
  }
}
//.............................................................................
void THtml::TObjectsState::RestoreState()  {
  olex2::IOlex2Processor *op = olex2::IOlex2Processor::GetInstance();
  for( size_t i=0; i < html.ObjectCount(); i++ )  {
    if( !html.IsObjectManageble(i) )  continue;
    size_t ind = Objects.IndexOf(html.GetObjectName(i));
    if( ind == InvalidIndex )  continue;
    AOlxCtrl* obj = html.GetObject(i);
    wxWindow* win = html.GetWindow(i);
    TSStrStrList<olxstr,false>& props = *Objects.GetObject(ind);
    if( props["type"] != EsdlClassName(*obj) )  {
      TBasicApp::NewLogEntry(logError) << "Object type changed for: "
        << Objects.GetString(ind);
      continue;
    }
    if( EsdlInstanceOf(*obj, TTextEdit) )  {
      TTextEdit* te = (TTextEdit*)obj;
      te->SetText(props["val"]);
      te->SetData(props["data"]);
    }
    else if( EsdlInstanceOf(*obj, TCheckBox) )  {
      TCheckBox* cb = (TCheckBox*)obj;
      cb->SetCaption(props["val"]);
      cb->SetChecked(props["checked"].ToBool());
      cb->SetData(props["data"]);
    }
    else if( EsdlInstanceOf(*obj, TTrackBar) )  {
      TTrackBar* tb = (TTrackBar*)obj;
      tb->SetRange(olx_round(props["min"].ToDouble()), olx_round(props["max"].ToDouble()));
      tb->SetValue(olx_round(props["val"].ToDouble()));
      tb->SetData(props["data"]);
    }
    else if( EsdlInstanceOf(*obj, TSpinCtrl) )  {
      TSpinCtrl* sc = (TSpinCtrl*)obj;  
      sc->SetRange(olx_round(props["min"].ToDouble()), olx_round(props["max"].ToDouble()));
      sc->SetValue(olx_round(props["val"].ToDouble()));
      sc->SetData(props["data"]);
    }
    else if( EsdlInstanceOf(*obj, TButton) )  {
      TButton* bt = (TButton*)obj;
      bt->SetData(props["data"]);
      bt->SetCaption(props["val"] );
      bt->OnDown.SetEnabled(false);
      bt->OnUp.SetEnabled(false);
      bt->OnClick.SetEnabled(false);
      bt->SetDown(props["checked"].ToBool());
      bt->OnDown.SetEnabled(true);
      bt->OnUp.SetEnabled(true);
      bt->OnClick.SetEnabled(true);
    }
    else if( EsdlInstanceOf(*obj, TBmpButton) )  {
      TBmpButton* bt = (TBmpButton*)obj;
      bt->SetData(props["data"]);
      bt->SetSource(props["val"] );
      bt->OnDown.SetEnabled(false);
      bt->OnUp.SetEnabled(false);
      bt->OnClick.SetEnabled(false);
      bt->SetDown(props["checked"].ToBool());
      bt->OnDown.SetEnabled(true);
      bt->OnUp.SetEnabled(true);
      bt->OnClick.SetEnabled(true);
    }
    else if( EsdlInstanceOf(*obj, TImgButton) )  {
      TImgButton* bt = (TImgButton*)obj;
      bt->SetData(props["data"]);
      bt->SetImages(props["val"], props["width"].ToInt(), props["height"].ToInt());
      bt->SetDown(props["checked"].ToBool());
      bt->SetEnabled(props["enabled"].ToBool());
    }
    else if( EsdlInstanceOf(*obj, TComboBox) )  {
      TComboBox* cb = (TComboBox*)obj;
      if (!cb->IsReadOnly()) {
        TStrList toks(props["items"], ';');
        cb->Clear();
        cb->AddItems(toks);
      }
      cb->SetText(props["val"] );
      cb->SetData(props["data"]);
    }
    else if( EsdlInstanceOf(*obj, TListBox) )  {
      TListBox* lb = (TListBox*)obj;
      TStrList toks(props["items"], ';');
      lb->Clear();
      lb->AddItems(toks);
    }
    else if( EsdlInstanceOf(*obj, TTreeView) )  {
      ((TTreeView*)obj)->RestoreState(props["state"]);
    }
    else if( EsdlInstanceOf(*obj, TLabel) )  {
      TLabel* lb = (TLabel*)obj;
      lb->SetCaption(props["val"]);
    }
    else //?
      ;
    // restoring the control colours, it is generic
    if( win != NULL && false )  {
      olxstr bg(props["bg"]), fg(props["fg"]);
      if (op != NULL) {
        op->processFunction(bg);
        op->processFunction(fg);
      }
      if( EsdlInstanceOf(*win, TComboBox) )  {
        TComboBox* Box = (TComboBox*)win;
        if( !fg.IsEmpty() )  {
          wxColor fgCl = wxColor(fg.u_str());
          Box->SetForegroundColour(fgCl);
        }
        if( !bg.IsEmpty() )  {
          wxColor bgCl = wxColor(bg.u_str());
          Box->SetBackgroundColour(bgCl);
        }
      }
      else  {
        if( !fg.IsEmpty() )  win->SetForegroundColour(wxColor(fg.u_str()));
        if( !bg.IsEmpty() )  win->SetBackgroundColour(wxColor(bg.u_str()));
      }
    }
  }
}
//.............................................................................
void THtml::TObjectsState::SaveToFile(const olxstr& fn)  {
}
//.............................................................................
bool THtml::TObjectsState::LoadFromFile(const olxstr& fn)  {
  return true;
}
//.............................................................................
TSStrStrList<olxstr,false>* THtml::TObjectsState::DefineControl(
  const olxstr& name, const std::type_info& type)
{
  const size_t ind = Objects.IndexOf( name );
  if( ind != InvalidIndex )
    throw TFunctionFailedException(__OlxSourceInfo, "object already exists");
  TSStrStrList<olxstr,false>* props = new TSStrStrList<olxstr,false>;

  props->Add("type", type.name());  // type
  props->Add("data");
  if( type == typeid(TTextEdit) )  {
    props->Add("val");
  }
  else if( type == typeid(TCheckBox) )  {
    props->Add("val");
    props->Add("checked");
  }
  else if( type == typeid(TTrackBar) || type == typeid(TSpinCtrl) )  {
    props->Add("min");
    props->Add("max");
    props->Add("val");
  }
  else if( type == typeid(TButton) )  {
    props->Add("checked");
    props->Add("val");
  }
  else if( type == typeid(TBmpButton) )  {
    props->Add("checked");
    props->Add("val");
  }
  else if( type == typeid(TImgButton) )  {
    props->Add("checked");
    props->Add("enabled");
    props->Add("val");
    props->Add("width");
    props->Add("height");
  }
  else if( type == typeid(TComboBox) )  {
    props->Add("val");
    props->Add("items");
  }
  else if( type == typeid(TListBox) )  {
    props->Add("val");
    props->Add("items");
  }
  else if( type == typeid(TTreeView) )  {
    props->Add("state");
  }
  else if( type == typeid(TLabel) )  {
    props->Add("val");
  }
  else //?
    ;
  props->Add("fg");
  props->Add("bg");

  Objects.Add(name, props);

  return props;
}
//.............................................................................
void THtml::OnClipboard(wxClipboardTextEvent& evt) {
  wxWindow *w = FindFocus();
  if (w == NULL) return;
  bool processed = true;
  wxString text;
  if (EsdlInstanceOf(*w, TTextEdit) || EsdlInstanceOf(*w, wxTextCtrl)) {
    wxTextCtrl *tc = dynamic_cast<wxTextCtrl*>(w);
    if (tc != NULL)
      text = tc->GetStringSelection();
  }
  else if(EsdlInstanceOf(*w, TComboBox))
    text = dynamic_cast<wxComboBox*>(w)->GetStringSelection();
  else
    processed = false;
  if (processed && !text.IsEmpty() && wxTheClipboard->Open()) {
    if (wxTheClipboard->IsSupported(wxDF_TEXT))
      wxTheClipboard->SetData(new wxTextDataObject(text));
    wxTheClipboard->Close();
  }
  evt.Skip(processed);
}
//.............................................................................
