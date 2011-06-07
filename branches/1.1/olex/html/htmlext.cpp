#include "htmlext.h"

#include "estack.h"
#include "exparse/exptree.h"
#include "htmlswitch.h"
#include "imgcellext.h"
#include "../mainform.h"

#include "wx/tooltip.h"

#include "../xglapp.h"
#include "../obase.h"
#include "utf8file.h"
#include "wxzipfs.h"

#define this_InitFunc(funcName, argc) \
  (*THtml::Library).RegisterFunction( new TFunction<THtml>(this, &THtml::fun##funcName, #funcName, argc));\
  (LC->GetLibrary()).RegisterFunction( new TFunction<THtml>(this, &THtml::fun##funcName, #funcName, argc))
#define this_InitFuncD(funcName, argc, desc) \
  (*THtml::Library).RegisterFunction( new TFunction<THtml>(this, &THtml::fun##funcName, #funcName, argc, desc));\
  (LC->GetLibrary()).RegisterFunction( new TFunction<THtml>(this, &THtml::fun##funcName, #funcName, argc, desc))

TLibrary* THtml::Library = NULL;
olxstr THtml::SwitchSource;
str_stack THtml::SwitchSources;

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
END_EVENT_TABLE()
//..............................................................................
THtml::THtml(wxWindow *Parent, ALibraryContainer* LC, int flags) :
  wxHtmlWindow(Parent, -1, wxDefaultPosition, wxDefaultSize, flags),
  WI(this), ObjectsState(*this),
  InFocus(NULL),
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
  if( LC && ! THtml::Library )  {
    THtml::Library = LC->GetLibrary().AddLibrary("html");

    InitMacroA( *THtml::Library, THtml, ItemState, ItemState, u-does not update the html, fpAny^(fpNone|fpOne) );
    InitMacro( LC->GetLibrary(), THtml, ItemState, u-does not update the html, fpAny^(fpNone|fpOne) );

    InitMacroA( *THtml::Library, THtml, UpdateHtml, UpdateHtml, , fpNone|fpOne );
    InitMacro( LC->GetLibrary(), THtml, UpdateHtml, , fpNone|fpOne );

    InitMacroA( *THtml::Library, THtml, HtmlHome, Home, , fpNone|fpOne );
    InitMacro( LC->GetLibrary(), THtml, HtmlHome, , fpNone|fpOne );

    InitMacroA( *THtml::Library, THtml, HtmlReload, Reload, , fpNone|fpOne );
    InitMacro( LC->GetLibrary(), THtml, HtmlReload, , fpNone|fpOne );

    InitMacroA( *THtml::Library, THtml, HtmlLoad, Load, , fpOne|fpTwo );
    InitMacro( LC->GetLibrary(), THtml, HtmlLoad, , fpOne|fpTwo );

    InitMacroA( *THtml::Library, THtml, HtmlDump, Dump, , fpOne|fpTwo );
    InitMacro( LC->GetLibrary(), THtml, HtmlDump, , fpOne|fpTwo );

    InitMacroA( *THtml::Library, THtml, Tooltips, Tooltips, , fpNone|fpOne|fpTwo );
    InitMacroA( LC->GetLibrary(), THtml, Tooltips, Htmltt, , fpNone|fpOne|fpTwo );

    InitMacroD( *THtml::Library, THtml, SetFonts, EmptyString(), fpTwo,
      "Sets normal and fixed fonts to display HTML content");

    InitMacroD( *THtml::Library, THtml, SetBorders, EmptyString(), fpOne|fpTwo,
      "Sets borders between HTML content and window edges");
    InitMacroD( *THtml::Library, THtml, DefineControl, 
      "v-value&;i-tems&;c-checked/down&;bg-background color&;fg-foreground color;&;min-min value&;max-max value", 
      fpTwo, "Defines a managed control properties");
    InitMacroD( *THtml::Library, THtml, Hide, EmptyString(), 
      fpOne, "Hides an Html popup window");
    InitMacroD( *THtml::Library, THtml, Group, EmptyString(), 
      fpAny^(fpNone|fpOne), "Creates an exclusive group of buttons");

    this_InitFuncD(GetValue, fpOne, "Returns value of specified object");
    this_InitFuncD(GetData, fpOne, "Returns data associated with specified object");
    this_InitFuncD(GetState, fpOne|fpTwo, "Returns state of the checkbox or a button.\
 For example: echo getstate(button, enabled/down) or echo getstate(checkbox)");
    this_InitFuncD(GetLabel, fpOne,
"Returns labels of specified object. Applicable to labels, buttons and checkboxes");
    this_InitFuncD(GetImage, fpOne, "Returns image source for a button or zimg");
    this_InitFuncD(GetItems, fpOne, "Returns items of a combobox or list");
    this_InitFuncD(SetValue, fpTwo, "Sets value of specified object");
    this_InitFuncD(SetData, fpTwo, "Sets data for specified object");
    this_InitFuncD(SetState, fpTwo|fpThree, "Sets state of a checkbox or a button");
    this_InitFuncD(SetLabel, fpTwo, "Sets labels for a label, button or checkbox");
    this_InitFuncD(SetImage, fpTwo, "Sets image location for a button or a zimg");
    this_InitFuncD(SetItems, fpTwo, "Sets items for comboboxes and lists");
    this_InitFuncD(SaveData, fpOne,
"Saves state, data, label and value of all objects to a file");
    this_InitFuncD(LoadData, fpOne,
"Loads previously saved data of html objects form a file");
    this_InitFuncD(SetFG, fpTwo, "Sets foreground of specified object");
    this_InitFuncD(SetBG, fpTwo, "Sets background of specified object");
    this_InitFuncD(GetFontName, fpNone, "Returns current font name");
    this_InitFuncD(GetBorders, fpNone, "Returns borders width between HTML content and window boundaries");
    this_InitFuncD(SetFocus, fpOne, "Sets input focus to the specified HTML control");
    this_InitFuncD(Select, fpTwo|fpThree, "Selects a treeview item by label (default) or data (third argument should be False)");
    this_InitFuncD(GetItemState, fpOne|fpTwo, "Returns item state of provided switch");
    this_InitFuncD(IsItem, fpOne, "Returns true if specified switch exists");
    this_InitFuncD(IsPopup, fpOne, "Returns true if specified popup window exists and visible");
    this_InitFuncD(EndModal, fpTwo, "Ends a modal popup and sets the return code");
    this_InitFuncD(ShowModal, fpOne, "Shows a previously created popup window as a modal dialog");
    this_InitFuncD(Width, fpOne|fpTwo, "Returns/sets width of an HTML object window (use 'self' to address the window itself)");
    this_InitFuncD(Height, fpOne|fpTwo, "Returns/sets height of an HTML object window (use 'self' to address the window itself)");
    this_InitFuncD(ClientWidth, fpOne|fpTwo, "Returns/sets client width of an HTML window (use 'self' to address the window itself)");
    this_InitFuncD(ClientHeight, fpOne|fpTwo, "Returns/sets client height of an HTML window (use 'self' to address the window itself)");
    this_InitFuncD(ContainerWidth, fpOne|fpTwo, "Returns/sets width of a popup window");
    this_InitFuncD(ContainerHeight, fpOne|fpTwo, "Returns/sets height of a popup window");
  }
}
//..............................................................................
THtml::~THtml()  {
  TFileHandlerManager::Clear();
  delete Root;
  ClearSwitchStates();
}
//..............................................................................
void THtml::OnLinkClicked(const wxHtmlLinkInfo& link)  {
  if( !MouseDown )  return;
  MouseDown = false;
  olxstr Href = link.GetHref();
  size_t ind = Href.FirstIndexOf('%');
  while( ind != InvalidIndex && ((ind+2) < Href.Length()) )  {
    int val = Href.SubString(ind+1, 2).RadInt<int>(16);
    Href.Delete(ind, 3);
    Href.Insert(val, ind);
    ind = Href.FirstIndexOf('%');
  }
  if( !OnLink.Execute(this, (IEObject*)&Href) )  {
    wxHtmlLinkInfo NewLink(Href.u_str(), link.GetTarget());
    wxHtmlWindow::OnLinkClicked(NewLink);
  }
}
//..............................................................................
wxHtmlOpeningStatus THtml::OnOpeningURL(wxHtmlURLType type, const wxString& url, wxString *redirect) const
{
  olxstr Url = url;
  if( !OnURL.Execute(this, &Url) )  return wxHTML_OPEN;
  return wxHTML_BLOCK;
}
//..............................................................................
void THtml::SetSwitchState(THtmlSwitch& sw, size_t state)  {
  const size_t ind = SwitchStates.IndexOf( sw.GetName() );
  if( ind == InvalidIndex )
    SwitchStates.Add(sw.GetName(), state);
  else
    SwitchStates.GetObject(ind) = state;
}
//..............................................................................
size_t THtml::GetSwitchState(const olxstr& switchName)  {
  const size_t ind = SwitchStates.IndexOf( switchName );
  return (ind == InvalidIndex) ? UnknownSwitchState : SwitchStates.GetObject(ind);
}
//..............................................................................
void THtml::OnMouseDown(wxMouseEvent& event)  {
  this->SetFocusIgnoringChildren();
  MouseX = event.GetX();
  MouseY = event.GetY();
  MouseDown = true;
  if( Movable )
    SetCursor( wxCursor(wxCURSOR_SIZING) );
  event.Skip();
}
//..............................................................................
void THtml::OnMouseUp(wxMouseEvent& event)  {
  if( Movable && MouseDown )
    SetCursor(wxCursor(wxCURSOR_ARROW));
  event.Skip();
}
//..............................................................................
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
//..............................................................................
void THtml::OnMouseDblClick(wxMouseEvent& event)  {
  event.Skip();
  StartEvtProcessing()
    OnDblClick.Execute(this, &OnDblClickData);
  EndEvtProcessing()
}
//..............................................................................
void THtml::OnSizeEvt(wxSizeEvent& event)  {
  event.Skip();
  StartEvtProcessing()
    OnSize.Execute(this, &OnSizeData);
  EndEvtProcessing()
}
//..............................................................................
bool THtml::Dispatch(int MsgId, short MsgSubId, const IEObject* Sender, const IEObject* Data)  {
  if( MsgId == html_parent_resize )  {
    TMainFrame::GetMainFrameInstance().LockWindowDestruction(this, this);
    OnSize.Execute(this, &OnSizeData);
    TMainFrame::GetMainFrameInstance().UnlockWindowDestruction(this, this);
  }
  return true;
}
//..............................................................................
void THtml::OnChildFocus(wxChildFocusEvent& event)  {
  wxWindow *wx_next = event.GetWindow(), 
    *focused = FindFocus();
  if( wx_next != focused )  {
    if( InFocus != NULL )
    InFocus->SetFocus();
    return;
  }
  AOlxCtrl* prev = NULL, *next = NULL;
  for( size_t i=0; i < Traversables.Count(); i++ )  {
    if( Traversables[i].GetB() == InFocus )
      prev = Traversables[i].GetA();
    if( Traversables[i].GetB() == wx_next )
      next = Traversables[i].GetA();
  }
  if( prev != next || next == NULL || prev == NULL )  {
    DoHandleFocusEvent(prev, next);
    InFocus = wx_next;
  }
  event.Skip();
}
//..............................................................................
void THtml::_FindNext(index_t from, index_t& dest, bool scroll) const {
  if( Traversables.IsEmpty() )  {
    dest = -1;
    return;
  }
  int i = from;
  while( ++i < (int)Traversables.Count() && Traversables[i].GetB() == NULL )
    ;
  dest = ((i >= (int)Traversables.Count() || (Traversables[i].GetB() == NULL)) ? -1 : i);
  if( dest == -1 && scroll )  {
    i = -1;
    while( ++i < from && Traversables[i].GetB() == NULL )
      ;
    dest = ((Traversables[i].GetB() == NULL) ? -1 : i);
  }
}
//..............................................................................
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
//..............................................................................
void THtml::GetTraversibleIndeces(index_t& current, index_t& another, bool forward) const {
  wxWindow* w = FindFocus();
  if( w == NULL || w == static_cast<const wxWindow*>(this) )  {  // no focus? find the one at the edge
    if( forward )
      _FindNext(-1, another, false);
    else
      _FindPrev(Traversables.Count(), another, false);
  }
  else  {
    for( size_t i=0; i < Traversables.Count(); i++ )  {
      if( Traversables[i].GetB() == w || Traversables[i].GetB() == w->GetParent() )  {
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
//..............................................................................
void THtml::DoHandleFocusEvent(AOlxCtrl* prev, AOlxCtrl* next)  {
  // prevent pae re-loading and object deletion
  TMainFrame::GetMainFrameInstance().LockWindowDestruction(this, this);
  if( prev != NULL )  {
    if( EsdlInstanceOf(*prev, TTextEdit) )  {
      olxstr s = ((TTextEdit*)prev)->GetOnLeaveStr();
      ((TTextEdit*)prev)->OnLeave.Execute(prev, &s);
    }
    else if( EsdlInstanceOf(*prev, TComboBox) )  {
      olxstr s = ((TComboBox*)prev)->GetOnLeaveStr();
      ((TComboBox*)prev)->OnLeave.Execute(prev, &s);
    }
  }
  if( next != NULL )  {
    if( EsdlInstanceOf(*next, TTextEdit) )  {
      olxstr s = ((TTextEdit*)next)->GetOnEnterStr();
      ((TTextEdit*)next)->OnEnter.Execute(next, &s);
      ((TTextEdit*)next)->SetSelection(-1,-1);
    }
    else if( EsdlInstanceOf(*next, TComboBox) )  {
      olxstr s = ((TComboBox*)next)->GetOnEnterStr();
      ((TComboBox*)next)->OnEnter.Execute(next, &s);
      ((TComboBox*)next)->SetSelection(-1,-1);
    }
  }
  TMainFrame::GetMainFrameInstance().UnlockWindowDestruction(this, this);
}
//..............................................................................
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
//..............................................................................
void THtml::OnKeyDown(wxKeyEvent& event)  {
  if( event.GetKeyCode() == WXK_TAB )
    DoNavigate( event.GetModifiers() != wxMOD_SHIFT );
  else
    event.Skip();
}
//..............................................................................
void THtml::OnNavigation(wxNavigationKeyEvent& event)  {
  if( event.IsFromTab() )
    DoNavigate( event.GetDirection() );
  else
    event.Skip();
}
//..............................................................................
void THtml::OnChar(wxKeyEvent& event)  {
  //Ctrl+c
  if( event.m_controlDown && event.GetKeyCode() == 'c'-'a'+1 )  {
    CopySelection();
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
//..............................................................................
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
//..............................................................................
void THtml::CheckForSwitches(THtmlSwitch &Sender, bool izZip)  {
  TStrPObjList<olxstr,THtmlSwitch*>& Lst = Sender.GetStrings();
  TStrList Toks;
  using namespace exparse::parser_util;
  static const olxstr Tag  = "<!-- #include ",
           Tag1 = "<!-- #includeif ",
           Tag2 = "<!-- #include",
           comment_open = "<!--", comment_close = "-->";
  for( size_t i=0; i < Lst.Count(); i++ )  {
    olxstr tmp = olxstr(Lst[i]).TrimWhiteChars();
    if( tmp.StartsFrom(comment_open) && !tmp.StartsFrom(Tag2) )  {  // skip comments
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
    Lst[i] = TGlXApp::GetMainForm()->TranslateString(Lst[i]);
    if( Lst[i].IndexOf("$") != InvalidIndex )
      TGlXApp::GetMainForm()->ProcessFunction(Lst[i], olxstr(Sender.GetCurrentFile()) << '#' << (i+1));
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
        if( TGlXApp::GetMainForm()->ProcessFunction(tmp) )  {
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
        TBasicApp::NewLogEntry(logError) << "Wrong defined switch (not enough data)" << Sw->GetName();
        continue;
      }

      for( size_t j=0; j < Toks.Count()-1; j++ )  {
        if( Toks[j].FirstIndexOf('=') == InvalidIndex )  {
          if( izZip && !TZipWrapper::IsZipFile(Toks[j]) )  {
            if( Toks[j].StartsFrom('\\') || Toks[j].StartsFrom('/') )
              tmp = Toks[j].SubStringFrom(1);
            else
              tmp = TZipWrapper::ComposeFileName(Sender.GetFile(Sender.GetFileIndex()), Toks[j]);
          }
          else
            tmp = Toks[j];

          TGlXApp::GetMainForm()->ProcessFunction(tmp);
          Sw->AddFile(tmp);
        }
        else  {
          // check for parameters
          if( Toks[j].IndexOf('#') != InvalidIndex )  {
            olxstr tmp1;
            tmp = Toks[j];
            for( size_t k=0; k < Sender.GetParams().Count(); k++ )  {
              tmp1 = '#';  
              tmp1 << Sender.GetParams().GetName(k);
              tmp.Replace(tmp1, Sender.GetParams().GetValue(k) );
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
//..............................................................................
bool THtml::ProcessPageLoadRequest()  {
  if( !PageLoadRequested || IsPageLocked() )  return false;
  PageLoadRequested = false;
  bool res = false;
  if( !PageRequested.IsEmpty() )
    res = LoadPage(PageRequested.u_str());
  else
    res = UpdatePage();
  PageRequested.SetLength(0);
  return res;
}
//..............................................................................
bool THtml::ReloadPage()  {
  if( IsPageLocked() )  {
    PageLoadRequested = true;
    PageRequested = FileName;
    return true;
  }
  Objects.Clear();
  Traversables.Clear();
  return LoadPage(FileName.u_str());
}
//..............................................................................
bool THtml::LoadPage(const wxString &file)  {
  if( file.IsEmpty() )
    return false;
  
  if( IsPageLocked() )  {
    PageLoadRequested = true;
    PageRequested = file;
    return true;
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

  if( !TZipWrapper::IsValidFileName(TestFile) && !TFileHandlerManager::Exists(file) )  {
    throw TFileDoesNotExistException(__OlxSourceInfo, file);
  }
  Root->Clear();
  Root->ClearFiles();
  Root->AddFile(File);

  Root->SetFileIndex(0);
  Root->UpdateFileIndex();
  FileName = File;
  return UpdatePage();
}
//..............................................................................
bool THtml::ItemState(const olxstr &ItemName, short State)  {
  THtmlSwitch * Sw = Root->FindSwitch(ItemName);
  if( Sw == NULL )  {
    TBasicApp::NewLogEntry(logError) << "THtml::ItemState: unresolved: " << ItemName;
    return false;
  }
  Sw->SetFileIndex(State-1);
  return true;
}
//..............................................................................
bool THtml::UpdatePage()  {
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

  olxstr oldPath( TEFile::CurrentDir() );

  TEFile::ChangeDir(WebFolder);

  for( size_t i=0; i < Root->SwitchCount(); i++ )  // reload switches
    Root->GetSwitch(i).UpdateFileIndex();

  TStrList Res;
  Root->ToStrings(Res);
  ObjectsState.SaveState();
  Objects.Clear();
  Traversables.Clear();
  InFocus = NULL;
  int xPos = -1, yPos = -1, xWnd=-1, yWnd = -1;
  wxHtmlWindow::GetViewStart(&xPos, &yPos);
#if defined(__WIN32__) || (defined(__linux__) && wxCHECK_VERSION(9,0,0))
  wxHtmlWindow::Freeze();
#else
  Hide();
#endif
  SetPage(Res.Text(' ').u_str());
  ObjectsState.RestoreState();
  wxHtmlWindow::Scroll(xPos, yPos);
#if defined(__WIN32__) || (defined(__linux__) && wxCHECK_VERSION(9,0,0))
  Thaw();
#else
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
//#endif
  SwitchSources.Clear();
  SwitchSource.SetLength(0);
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
#ifdef __WIN32__
        if( cb->GetTextCtrl() != NULL )  {
          cb->GetTextCtrl()->SetInsertionPoint(0);
          wnd = cb->GetTextCtrl();
        }
#endif
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
//..............................................................................
void THtml::OnScroll(wxScrollEvent& evt)  {  // this is never called at least on GTK
  evt.Skip();
#ifdef __WXGTK__
  this->Update();
#endif
}
//..............................................................................
void THtml::ScrollWindow(int dx, int dy, const wxRect* rect)  {
#ifdef __WXGTK__
  if( dx == 0 && dy == 0 )  return;
  Freeze();
  wxHtmlWindow::ScrollWindow(dx,dy,rect);
  Thaw();
  this->Refresh();
  this->Update();
#else
  wxHtmlWindow::ScrollWindow(dx,dy,rect);
#endif
}
//..............................................................................
bool THtml::AddObject(const olxstr& Name, AOlxCtrl *Object, wxWindow* wxWin, bool Manage)  {
#ifdef __WIN32__
  wxWindow* ew = wxWin;
  if( Object != NULL && EsdlInstanceOf(*Object, TComboBox) )  {
    TComboBox* cb = (TComboBox*)Object;
    if( cb->GetTextCtrl() != NULL )
      ew = cb->GetTextCtrl();
  }
  Traversables.Add(Association::New(Object, ew));
#else
  Traversables.Add(Association::New(Object,wxWin));
#endif
  if( Name.IsEmpty() )  return true;  // an anonymous object
  if( Objects.IndexOf(Name) != InvalidIndex )  return false;
  Objects.Add(Name, Association::Create(Object, wxWin, Manage));
  return true;
}
//..............................................................................
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
      wxString wxs(Href.Replace("#href", Link->GetHref()).u_str() );
      if( tt == NULL || tt->GetTip() != wxs )  {
        SetToolTip( wxs );
      }
    }
  }
  else
    SetToolTip(NULL);
}
//..............................................................................
/*
the format is following intemstate popup_name item_name statea stateb ... item_nameb ...
if there are more than 1 state for an item the function does the rotation if
one of the states correspond to current - the next one is selected
*/
void THtml::macItemState(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  THtml *html = NULL;
  if( Cmds.Count() > 2 && !Cmds[1].IsNumber() )  {
    html = TGlXApp::GetMainForm()->FindHtml(Cmds[0]);
    if( html == NULL )  {
      Error.ProcessingError(__OlxSrcInfo, "undefined popup: ").quote() << Cmds[0];
      return;
    }
    Cmds.Delete(0);
  }
  else
    html = this;

  THtmlSwitch& rootSwitch = html->GetRoot();
  TIndexList states;
  TPtrList<THtmlSwitch> Switches;
  olxstr itemName(Cmds[0]);
  bool changed = false;
  for( size_t i=1; i < Cmds.Count(); i++ )  {
    Switches.Clear();
    if( itemName.EndsWith('.') )  {  // special treatment of any particular index
      THtmlSwitch* sw = rootSwitch.FindSwitch(itemName.SubStringTo(itemName.Length()-1));
      if( sw == NULL )
        return;
      for( size_t j=0; j < sw->SwitchCount(); j++ )
        Switches.Add(sw->GetSwitch(j));
      //sw->Expand(Switches);
    }
    else if( itemName.FirstIndexOf('*') == InvalidIndex )  {
      THtmlSwitch* sw = rootSwitch.FindSwitch(itemName);
      if( sw == NULL )  {
        Error.ProcessingError(__OlxSrcInfo, "could not locate specified switch: ").quote() << itemName;
        return;
      }
      Switches.Add(sw);
    }
    else  {
      if( itemName == '*' )  {
        for( size_t j=0; j < rootSwitch.SwitchCount(); j++ )
          Switches.Add(rootSwitch.GetSwitch(j));
      }
      else  {
        size_t sindex = itemName.FirstIndexOf('*');
        // *blabla* syntax
        if( sindex == 0 && itemName.Length() > 2 && itemName.CharAt(itemName.Length()-1) == '*')  {
          rootSwitch.FindSimilar( itemName.SubString(1, itemName.Length()-2), EmptyString(), Switches );
        }
        else  {  // assuming bla*bla syntax
          olxstr from = itemName.SubStringTo(sindex), with;
          if( (sindex+1) < itemName.Length() )
            with = itemName.SubStringFrom(sindex+1);
          rootSwitch.FindSimilar(from, with, Switches);
        }
      }
    }
    states.Clear();
    for( size_t j=i; j < Cmds.Count();  j++,i++ )  {
      // is new switch encountered?
      if( !Cmds[j].IsNumber() )  {  itemName = Cmds[j];  break;  }
      states.Add(Cmds[j].ToInt());
    }
    if( states.Count() == 1 )  {  // simply change the state to the request
      for( size_t j=0; j < Switches.Count(); j++ )  {
        if( Switches[j]->GetFileIndex() != states[0]-1 )
          changed = true;
        Switches[j]->SetFileIndex(states[0]-1);
      }
    }
    else  {
      for( size_t j=0; j < Switches.Count(); j++ )  {
        THtmlSwitch* sw = Switches[j];
        const index_t currentState = sw->GetFileIndex();
        for( size_t k=0; k < states.Count(); k++ )  {
          if( states[k] == (currentState+1) )  {
            if( (k+1) < states.Count() )  {
              if( sw->GetFileIndex() != states[k+1]-1 )
                changed = true;
              sw->SetFileIndex(states[k+1]-1);
            }
            else  {
              if( sw->GetFileIndex() != states[0]-1 )
                changed = true;
              sw->SetFileIndex(states[0]-1);
            }
          }
        }
      }
    }
  }
  if( changed && !Options.Contains('u') )
    html->UpdatePage();
  return;
}
//..............................................................................
void THtml::funGetItemState(const TStrObjList &Params, TMacroError &E)  {
  THtml *html = (Params.Count() == 2) ? TGlXApp::GetMainForm()->FindHtml(Params[0]) : this;
  if( html == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "undefined html window: ").quote() << Params[0];
    return;
  }
  olxstr itemName( (Params.Count() == 1) ? Params[0] : Params[1] );
  THtmlSwitch *sw = html->GetRoot().FindSwitch(itemName);
  if( sw == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "could not locate specified switch: ").quote() << itemName;
    return;
  }
  if( sw->GetFileIndex() == InvalidIndex )
    E.SetRetVal<olxstr>("-1");
  else if( sw->GetFileIndex() == UnknownSwitchState )
    E.SetRetVal<olxstr>("-2");
  else
    E.SetRetVal(sw->GetFileIndex());
}
//..............................................................................
void THtml::funIsPopup(const TStrObjList& Params, TMacroError &E)  {
  THtml *html = TGlXApp::GetMainForm()->FindHtml(Params[0]);
  E.SetRetVal(html != NULL && html->GetParent()->IsShown());
}
//..............................................................................
void THtml::funIsItem(const TStrObjList &Params, TMacroError &E)  {
  THtml *html = (Params.Count() == 2) ? TGlXApp::GetMainForm()->FindHtml(Params[0]) : this;
  if( html == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "undefined html window: ").quote() << Params[0];
    return;
  }
  olxstr itemName((Params.Count() == 1) ? Params[0] : Params[1]);
  E.SetRetVal(html->GetRoot().FindSwitch(itemName) != NULL);
}
//..............................................................................
void THtml::SetShowTooltips(bool v, const olxstr& html_name)  {
  ShowTooltips = v;
  TStateChange sc(prsHtmlTTVis, v, html_name);
  TGlXApp::GetMainForm()->OnStateChange.Execute((AEventsDispatcher*)this, &sc);
}
//..............................................................................
void THtml::macTooltips(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( Cmds.IsEmpty() )  {
    SetShowTooltips( !GetShowTooltips() );
  }
  else if( Cmds.Count() == 1 )  {
    if( Cmds[0].Equalsi("true") || Cmds[0].Equalsi("false") )
      this->SetShowTooltips( Cmds[0].ToBool() );
    else  {
      THtml* html = TGlXApp::GetMainForm()->FindHtml( Cmds[0] );
      if( html == NULL )  return;
      html->SetShowTooltips( !html->GetShowTooltips() );
    }
  }
  else  {
    THtml* html = TGlXApp::GetMainForm()->FindHtml( Cmds[0] );
    if( html != NULL )
      html->SetShowTooltips( Cmds[1].ToBool(), Cmds[0] );
  }
}
//..............................................................................
void THtml::macUpdateHtml(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  THtml *html = (Cmds.Count() == 1) ? TGlXApp::GetMainForm()->FindHtml(Cmds[0]) : this;
  if( html == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "undefined html window: ").quote() << Cmds[0];
    return;
  }
  html->UpdatePage();
}
//..............................................................................
void THtml::macSetFonts(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  this->SetFonts(Cmds[0], Cmds[1]);
}
//..............................................................................
void THtml::macSetBorders(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  THtml *html = (Cmds.Count() == 2) ? TGlXApp::GetMainForm()->FindHtml(Cmds[0]) : this;
  if( html == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "undefined html window: ").quote() << Cmds[0];
    return;
  }
  html->SetBorders(Cmds.GetLastString().ToInt());
}
//..............................................................................
void THtml::macHtmlHome(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  THtml *html = (Cmds.Count() == 1) ? TGlXApp::GetMainForm()->FindHtml(Cmds[0]) : this;
  if( html == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "undefined html window: ").quote() << Cmds[0];
    return;
  }
  html->LoadPage( html->GetHomePage().u_str() );
}
//..............................................................................
void THtml::macHtmlReload(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  THtml *html = (Cmds.Count() == 1) ? TGlXApp::GetMainForm()->FindHtml(Cmds[0]) : this;
  if( html == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "undefined html window: ").quote() << Cmds[0];
    return;
  }
  html->ReloadPage();
}
//..............................................................................
void THtml::macHtmlLoad(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  THtml *html = (Cmds.Count() == 2) ? TGlXApp::GetMainForm()->FindHtml(Cmds[0]) : this;
  if( html == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "undefined html window: ").quote() << Cmds[0];
    return;
  }
  html->LoadPage( Cmds.GetLastString().u_str() );
}
//..............................................................................
void THtml::macHide(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TPopupData *html = TGlXApp::GetMainForm()->FindHtmlEx(Cmds[0]);
  if( html == NULL )  {
    //E.ProcessingError(__OlxSrcInfo, "undefined html window");
    return;
  }
  if( html->Dialog->IsShown() )
    html->Dialog->Show(false);
}
//..............................................................................
void THtml::macHtmlDump(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  THtml *html = (Cmds.Count() == 2) ? TGlXApp::GetMainForm()->FindHtml(Cmds[0]) : this;
  if( html == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "undefined html window: ").quote() << Cmds[0];
    return;
  }
  TStrList SL;
  html->GetRoot().ToStrings(SL);
  TUtf8File::WriteLines(Cmds.GetLastString(), SL);
}
//..............................................................................
void THtml::macDefineControl(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( ObjectsState.FindProperties(Cmds[0]) != NULL )  {
    E.ProcessingError(__OlxSrcInfo, "control ").quote() << Cmds[0] << " already exists";
    return;
  }
  TSStrStrList<olxstr,false>* props = NULL;
  if( Cmds[1].Equalsi("text") )  {
    props = ObjectsState.DefineControl(Cmds[0], typeid(TTextEdit) );
  }
  else if( Cmds[1].Equalsi("label") )  {
    props = ObjectsState.DefineControl(Cmds[0], typeid(TLabel) );
  }
  else if( Cmds[1].Equalsi("button") )  {
    props = ObjectsState.DefineControl(Cmds[0], typeid(TButton) );
    (*props)["image"] = Options.FindValue("i");
  }
  else if( Cmds[1].Equalsi("combo") )  {
    props = ObjectsState.DefineControl(Cmds[0], typeid(TComboBox) );
    (*props)["items"] = Options.FindValue("i");
  }
  else if( Cmds[1].Equalsi("spin") )  {
    props = ObjectsState.DefineControl(Cmds[0], typeid(TSpinCtrl) );
    (*props)["min"] = Options.FindValue("min", "0");
    (*props)["max"] = Options.FindValue("max", "100");
  }
  else if( Cmds[1].Equalsi("slider") )  {
    props = ObjectsState.DefineControl(Cmds[0], typeid(TTrackBar) );
    (*props)["min"] = Options.FindValue("min", "0");
    (*props)["max"] = Options.FindValue("max", "100");
  }
  else if( Cmds[1].Equalsi("checkbox") )  {
    props = ObjectsState.DefineControl(Cmds[0], typeid(TCheckBox) );
    (*props)["checked"] = Options.FindValue("c", "false");
  }
  else if( Cmds[1].Equalsi("tree") )  {
    props = ObjectsState.DefineControl(Cmds[0], typeid(TTreeView) );
  }
  else if( Cmds[1].Equalsi("list") )  {
    props = ObjectsState.DefineControl(Cmds[0], typeid(TListBox) );
    (*props)["items"] = Options.FindValue("i");
  }
  if( props != NULL )  {
    (*props)["bg"] = Options.FindValue("bg");
    (*props)["fg"] = Options.FindValue("fg");
    if( props->IndexOf("data") != InvalidIndex )
      (*props)["data"] = Options.FindValue("data", EmptyString());
    if( props->IndexOf("val") != InvalidIndex )
      (*props)["val"] = Options.FindValue("v");
  }
}
//..............................................................................
//..............................................................................
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
  return EmptyString();
}
void THtml::funGetValue(const TStrObjList &Params, TMacroError &E)  {
  Control c = FindControl(Params[0], E, 0, __OlxSrcInfo);
  if( c.html == NULL )  return;
  if( c.ctrl == NULL )  {
    TSStrStrList<olxstr,false>* props = c.html->ObjectsState.FindProperties(c.ctrl_name);
    if( props == NULL )  {
      E.ProcessingError(__OlxSrcInfo,  "wrong html object name: ").quote() << c.ctrl_name;
      return;
    }
    if( props->IndexOf("val") == InvalidIndex )  {
      E.ProcessingError(__OlxSrcInfo,  "object definition does not have value for: ").quote() << c.ctrl_name;
      return;
    }
    E.SetRetVal((*props)["val"]);
  }
  else
    E.SetRetVal(c.html->GetObjectValue(c.ctrl));
}
//..............................................................................
void THtml::SetObjectValue(AOlxCtrl *Obj, const olxstr& Value)  {
  if( EsdlInstanceOf(*Obj, TTextEdit) )       ((TTextEdit*)Obj)->SetText(Value);
  else if( EsdlInstanceOf(*Obj, TCheckBox) )  ((TCheckBox*)Obj)->SetCaption(Value);
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
  else  return;
}
void THtml::funSetValue(const TStrObjList &Params, TMacroError &E)  {
  Control c = FindControl(Params[0], E, 0, __OlxSrcInfo);
  if( c.html == NULL )  return;
  if( c.ctrl == NULL )  {
    TSStrStrList<olxstr,false>* props = c.html->ObjectsState.FindProperties(c.ctrl_name);
    if( props == NULL )  {
      E.ProcessingError(__OlxSrcInfo,  "wrong html object name: ").quote() << c.ctrl_name;
      return;
    }
    if( props->IndexOf("val") == InvalidIndex )  {
      E.ProcessingError(__OlxSrcInfo,  "object definition does not accept value for: ").quote() << c.ctrl_name;
      return;
    }
    if( (*props)["type"] == EsdlClassName(TTrackBar) || (*props)["type"] == EsdlClassName(TSpinCtrl) )  {
      const size_t si = Params[1].IndexOf(',');
      if( si == InvalidIndex )
        (*props)["val"] = Params[1];
      else  {
        (*props)["min"] = Params[1].SubStringTo(si);
        (*props)["max"] = Params[1].SubStringFrom(si+1);
      }
    }
    else
      (*props)["val"] = Params[1];
  }
  else  {
    c.html->SetObjectValue(c.ctrl, Params[1]);
    c.html->Refresh();
  }
}
//..............................................................................
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
void THtml::funGetData(const TStrObjList &Params, TMacroError &E)  {
  Control c = FindControl(Params[0], E, 0, __OlxSrcInfo);
  if( c.html == NULL )  return;
  if( c.ctrl == NULL )  {
    TSStrStrList<olxstr,false>* props = c.html->ObjectsState.FindProperties(c.ctrl_name);
    if( props == NULL )  {
      E.ProcessingError(__OlxSrcInfo,  "wrong html object name: ").quote() << c.ctrl_name;
      return;
    }
    E.SetRetVal( (*props)["data"] );
  }
  else
    E.SetRetVal(c.html->GetObjectData(c.ctrl));
}
//..............................................................................
void THtml::funGetItems(const TStrObjList &Params, TMacroError &E)  {
  Control c = FindControl(Params[0], E, 1, __OlxSrcInfo);
  if( c.html == NULL || c.ctrl == NULL )  return;
  E.SetRetVal(c.html->GetObjectItems(c.ctrl));
}
//..............................................................................
olxstr THtml::GetObjectImage(const AOlxCtrl* Obj)  {
  if( EsdlInstanceOf(*Obj, TBmpButton) )
    return ((TBmpButton*)Obj)->GetSource();
  else if( EsdlInstanceOf(*Obj, THtmlImageCell) )
    return ((THtmlImageCell*)Obj)->GetSource();
  return EmptyString();
}
//..............................................................................
bool THtml::SetObjectImage(AOlxCtrl* Obj, const olxstr& src)  {
  if( src.IsEmpty() )  return false;

  if( EsdlInstanceOf(*Obj, TBmpButton) || EsdlInstanceOf(*Obj, THtmlImageCell) )  {
    wxFSFile *fsFile = TFileHandlerManager::GetFSFileHandler(src);
    if( fsFile == NULL )  {
      TBasicApp::NewLogEntry(logError) << "Setimage: could not locate specified file: " << src;
      return false;
    }
    wxImage image(*(fsFile->GetStream()), wxBITMAP_TYPE_ANY);
    delete fsFile;
    if ( !image.Ok() )  {
      TBasicApp::NewLogEntry(logError) << "Setimage: could not read specified file: " << src;
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
//..............................................................................
olxstr THtml::GetObjectItems(const AOlxCtrl* Obj)  {
  if( EsdlInstanceOf(*Obj, TComboBox) )  {
    return ((TComboBox*)Obj)->ItemsToString(';');
  }
  else if( EsdlInstanceOf(*Obj, TListBox) )  {
    return ((TListBox*)Obj)->ItemsToString(';');
  }
  return EmptyString();
}
//..............................................................................
bool THtml::SetObjectItems(AOlxCtrl* Obj, const olxstr& src)  {
  if( src.IsEmpty() )  return false;

  if( EsdlInstanceOf(*Obj, TComboBox) )  {
    ((TComboBox*)Obj)->Clear();
    ((TComboBox*)Obj)->AddItems(TStrList(src, ';'));
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
//..............................................................................
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
void THtml::funSetData(const TStrObjList &Params, TMacroError &E)  {
  Control c = FindControl(Params[0], E, 1, __OlxSrcInfo);
  if( c.html == NULL || c.ctrl == NULL )  return;
  c.html->SetObjectData(c.ctrl, Params[1]);
}
//..............................................................................
THtml::Control THtml::FindControl(const olxstr &name, TMacroError& me, short needs,
  const char* location)
{
  const size_t ind = name.IndexOf('.');
  THtml* html = (ind == InvalidIndex) ? this
    : TGlXApp::GetMainForm()->FindHtml(name.SubStringTo(ind));
  olxstr objName = (ind == InvalidIndex) ? name : name.SubStringFrom(ind+1);
  if( html == NULL )  {
    me.ProcessingError(location, "could not locate specified popup: ").quote() <<
      name.SubStringTo(ind);
    return Control(NULL, NULL, NULL, objName);
  }
  if( objName.Equals("self") )  {
    if( needs == 1 )
      me.ProcessingError(__OlxSrcInfo, "wrong control name: ").quote() << objName;
    return Control(html, NULL, this, objName);
  }
  const size_t ci = html->Objects.IndexOf(objName);
  if( ci == InvalidIndex && needs != 0 )
    me.ProcessingError(__OlxSrcInfo, "wrong html object name: ").quote() << objName;
  return Control(html, ci == InvalidIndex ? NULL : html->GetObject(ci),
    ci == InvalidIndex ? NULL : html->GetWindow(ci), objName);
}
//..............................................................................
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
void THtml::funGetState(const TStrObjList &Params, TMacroError &E)  {
  Control c = FindControl(Params[0], E, 0, __OlxSrcInfo);
  if( c.html == NULL )  return;
  if( c.ctrl == NULL )  {
    TSStrStrList<olxstr,false>* props = c.html->ObjectsState.FindProperties(c.ctrl_name);
    if( props == NULL )  {
      E.ProcessingError(__OlxSrcInfo,  "wrong html object name: ").quote() << c.ctrl_name;
      return;
    }
    if( Params.Count() == 1 )
      E.SetRetVal((*props)["checked"]);
    else
      E.SetRetVal((*props)[Params[1]]);
  }
  else
    E.SetRetVal(c.html->GetObjectState(c.ctrl, Params.Count() == 1 ? EmptyString() : Params[1]));
}
//..............................................................................
void THtml::funGetLabel(const TStrObjList &Params, TMacroError &E)  {
  Control c = FindControl(Params[0], E, 1, __OlxSrcInfo);
  if( c.html == NULL || c.ctrl == NULL )  return;
  olxstr rV;
  if( EsdlInstanceOf(*c.ctrl, TButton) )  rV = ((TButton*)c.ctrl)->GetCaption();
  else if( EsdlInstanceOf(*c.ctrl, TLabel) )   rV = ((TLabel*)c.ctrl)->GetCaption();
  else if( EsdlInstanceOf(*c.ctrl, TCheckBox) )   rV = ((TCheckBox*)c.ctrl)->GetCaption();
  else if( EsdlInstanceOf(*c.ctrl, TTreeView) )  {
    TTreeView* T = (TTreeView*)c.ctrl;
    wxTreeItemId ni = T->GetSelection();
    rV = T->GetItemText(ni);
  }
  else  {
    E.ProcessingError(__OlxSrcInfo, "wrong html object type: ").quote()  << EsdlObjectName(*c.ctrl);
    return;
  }
  E.SetRetVal(rV);
}
//..............................................................................
void THtml::funSetLabel(const TStrObjList &Params, TMacroError &E)  {
  Control c = FindControl(Params[0], E, 1, __OlxSrcInfo);
  if( c.html == NULL || c.ctrl == NULL )  return;
  if( EsdlInstanceOf(*c.ctrl, TButton) )  ((TButton*)c.ctrl)->SetCaption(Params[1]);
  else if( EsdlInstanceOf(*c.ctrl, TLabel) )  ((TLabel*)c.ctrl)->SetCaption(Params[1]);
  else if( EsdlInstanceOf(*c.ctrl, TCheckBox) )  ((TCheckBox*)c.ctrl)->SetCaption(Params[1]);
  else if( EsdlInstanceOf(*c.ctrl, TTreeView) )  {
    TTreeView* T = (TTreeView*)c.ctrl;
    wxTreeItemId ni = T->GetSelection();
    T->SetItemText(ni, Params[1].u_str());
  }
  else  {
    E.ProcessingError(__OlxSrcInfo, "wrong html object type: ").quote()  << EsdlObjectName(*c.ctrl);
    return;
  }
}
//..............................................................................
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
//..............................................................................
void THtml::funSetImage(const TStrObjList &Params, TMacroError &E)  {
  Control c = FindControl(Params[0], E, 1, __OlxSrcInfo);
  if( c.html == NULL || c.ctrl == NULL )  return;
  if( !c.html->SetObjectImage(c.ctrl, Params[1]) )
    E.ProcessingError(__OlxSrcInfo, "could not set image for the object: ").quote()  << c.ctrl_name;
}
//..............................................................................
void THtml::funGetImage(const TStrObjList &Params, TMacroError &E)  {
  Control c = FindControl(Params[0], E, 1, __OlxSrcInfo);
  if( c.html == NULL || c.ctrl == NULL )  return;
  E.SetRetVal(c.html->GetObjectImage(c.ctrl));
}
//..............................................................................
void THtml::funSetFocus(const TStrObjList &Params, TMacroError &E)  {
  Control c = FindControl(Params[0], E, 0, __OlxSrcInfo);
  if( c.html == NULL )  return;
  FocusedControl = c.ctrl_name;
  InFocus = c.wnd;
  if( InFocus == NULL )  // not created yet?
    return;
  if( EsdlInstanceOf(*InFocus, TTextEdit) )
    ((TTextEdit*)InFocus)->SetSelection(-1,-1);
  else if( EsdlInstanceOf(*InFocus, TComboBox) )  {
    TComboBox* cb = (TComboBox*)InFocus;
    //cb->GetTextCtrl()->SetSelection(-1, -1);
#ifdef __WIN32__
    cb->GetTextCtrl()->SetInsertionPoint(0);
    InFocus = cb->GetTextCtrl();
#endif		
  }
  InFocus->SetFocus();
}
//..............................................................................
void THtml::funSelect(const TStrObjList &Params, TMacroError &E)  {
  bool by_label = Params.Count() == 3 ? Params[2].ToBool() : true;
  Control c = FindControl(Params[0], E, 1, __OlxSrcInfo);
  if( c.html == NULL || c.ctrl == NULL )  return;
  if( !EsdlInstanceOf(*c.ctrl, TTreeView) )  {
    E.ProcessingError(__OlxSrcInfo, "incompatible object type");
    return;
  }
  TTreeView* tv = (TTreeView*)c.ctrl;
  if( by_label )
    tv->SelectByLabel(Params[1]);
  else
    tv->SelectByData(Params[1]);
}
//..............................................................................
bool THtml::SetState(const TStrObjList &Params, TMacroError &E)  {
  Control c = FindControl(Params[0], E, 0, __OlxSrcInfo);
  if( c.html == NULL )  return false;
  const bool state = Params.GetLastString().ToBool();
  if( c.ctrl == NULL )  {
    TSStrStrList<olxstr,false>* props = c.html->ObjectsState.FindProperties(c.ctrl_name);
    if( props == NULL )  {
      E.ProcessingError(__OlxSrcInfo, "wrong html object name: ").quote() << c.ctrl_name;
      return false;
    }
    if( props->IndexOf("checked") == InvalidIndex )  {
      E.ProcessingError(__OlxSrcInfo, "object definition does have state for: ").quote() << c.ctrl_name;
      return false;
    }
    if( Params.Count() == 2 )
      (*props)["checked"] = state;
    else
      (*props)[Params[1]] = state;
  }
  else
    c.html->SetObjectState(c.ctrl, state, (Params.Count() == 2 ? EmptyString() : Params[1]));
  return true;
}
//..............................................................................
void THtml::funSetState(const TStrObjList &Params, TMacroError &E)  {
  if( !SetState(Params, E) )
    return;
  if( Params.GetLastString().ToBool() )  {
    TStrObjList params(Params);
    params.GetLastString() = FalseString();
    TMacroError e;
    for( size_t i=0; i < Groups.Count(); i++ )  {
      if( Groups[i].IndexOf(Params[0]) == InvalidIndex )  continue;
      const TStrList& group = Groups[i];
      for( size_t j=0; j < group.Count(); j++ )  {
        if( group[j].Equalsi(Params[0]) )  continue;
        params[0] = group[j];
        SetState(params, e);
      }
    }
  }
}
//..............................................................................
void THtml::funSetItems(const TStrObjList &Params, TMacroError &E)  {
  Control c = FindControl(Params[0], E, 1, __OlxSrcInfo);
  if( c.html == NULL || c.ctrl == NULL )  return;
  c.html->SetObjectItems(c.ctrl, Params[1]);
}
//..............................................................................
void THtml::funSaveData(const TStrObjList &Params, TMacroError &E)  {
  ObjectsState.SaveToFile(Params[0]);
}
//..............................................................................
void THtml::funLoadData(const TStrObjList &Params, TMacroError &E)  {
  if( !TEFile::Exists(Params[0]) )  {
    E.ProcessingError(__OlxSrcInfo, "file does not exist: ") << Params[0];
    return;
  }
  if( !ObjectsState.LoadFromFile(Params[0]) )  {
    E.ProcessingError(__OlxSrcInfo, "error occured while processing file");
    return;
  }
}
//..............................................................................
void THtml::funSetFG(const TStrObjList &Params, TMacroError &E)  {
  Control c = FindControl(Params[0], E, 2, __OlxSrcInfo);
  if( c.html == NULL || c.wnd == NULL )  return;
  const wxColor fgc = wxColor(Params[1].u_str());
  c.wnd->SetForegroundColour(fgc);
  if( EsdlInstanceOf(*c.wnd, TComboBox) )  {
    TComboBox* Box = (TComboBox*)c.wnd;
    //Box->SetForegroundColour(fgc);
#ifdef __WIN32__
    if( Box->GetPopupControl() != NULL )
      Box->GetPopupControl()->GetControl()->SetForegroundColour(fgc);
    if( Box->GetTextCtrl() != NULL )
      Box->GetTextCtrl()->SetForegroundColour(fgc);
#endif				
  }
  c.wnd->Refresh();
}
//..............................................................................
void THtml::funSetBG(const TStrObjList &Params, TMacroError &E)  {
  Control c = FindControl(Params[0], E, 2, __OlxSrcInfo);
  if( c.html == NULL || c.wnd == NULL )  return;
  const wxColor bgc(Params[1].u_str());
  c.wnd->SetBackgroundColour(bgc);
  if( EsdlInstanceOf(*c.wnd, TComboBox) )  {
    TComboBox* Box = (TComboBox*)c.wnd;
#ifdef __WIN32__
    if( Box->GetPopupControl() != NULL )
      Box->GetPopupControl()->GetControl()->SetBackgroundColour(bgc);
    if( Box->GetTextCtrl() != NULL )
      Box->GetTextCtrl()->SetBackgroundColour(bgc);
#endif
  }
  //else if( EsdlInstanceOf(*wxw, TTrackBar) )  {
  //  TTrackBar* Bar = (TTrackBar*)wxw;
  //}
  c.wnd->Refresh();
}
//..............................................................................
void THtml::funGetFontName(const TStrObjList &Params, TMacroError &E)  {
  E.SetRetVal<olxstr>(GetParser()->GetFontFace());
}
//..............................................................................
void THtml::funGetBorders(const TStrObjList &Params, TMacroError &E)  {
  E.SetRetVal(GetBorders());
}
//..............................................................................
void THtml::funEndModal(const TStrObjList &Params, TMacroError &E)  {
  TPopupData *pd = TGlXApp::GetMainForm()->FindHtmlEx(Params[0]);
  if( pd == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "undefined html window: ").quote() << Params[0];
    return;
  }
  if( !pd->Dialog->IsModal() )  {
    E.ProcessingError(__OlxSrcInfo, "non-modal html window: ").quote() << Params[0];
    return;
  }
  pd->Dialog->EndModal(Params[1].ToInt());
}
//..............................................................................
void THtml::funShowModal(const TStrObjList &Params, TMacroError &E)  {
  TPopupData *pd = TGlXApp::GetMainForm()->FindHtmlEx(Params[0]);
  if( pd == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "undefined html window: ").quote() << Params[0];
    return;
  }
  E.SetRetVal(pd->Dialog->ShowModal());
}
//..............................................................................
void THtml::funWidth(const TStrObjList &Params, TMacroError &E)  {
  Control c = FindControl(Params[0], E, 2, __OlxSrcInfo);
  if( c.html == NULL || c.wnd == NULL )  return;
  if( Params.Count() == 1 )
    E.SetRetVal(c.wnd->GetSize().GetWidth());
  else
    c.wnd->SetSize(-1, -1, Params[1].ToInt(), -1);
}
//..............................................................................
void THtml::funHeight(const TStrObjList &Params, TMacroError &E)  {
  Control c = FindControl(Params[0], E, 2, __OlxSrcInfo);
  if( c.html == NULL || c.wnd == NULL )  return;
  if( Params.Count() == 1 )
    E.SetRetVal(c.wnd->GetSize().GetHeight());
  else
    c.wnd->SetSize(-1, -1, -1, Params[1].ToInt());
}
//..............................................................................
void THtml::funClientWidth(const TStrObjList &Params, TMacroError &E)  {
  Control c = FindControl(Params[0], E, 2, __OlxSrcInfo);
  if( c.html == NULL || c.wnd == NULL )  return;
  if( Params.Count() == 1 )
    E.SetRetVal(c.wnd->GetClientSize().GetWidth());
  else
    c.wnd->SetClientSize(Params[1].ToInt(), -1);
}
//..............................................................................
void THtml::funClientHeight(const TStrObjList &Params, TMacroError &E)  {
  Control c = FindControl(Params[0], E, 2, __OlxSrcInfo);
  if( c.html == NULL || c.wnd == NULL )  return;
  if( Params.Count() == 1 )
    E.SetRetVal(c.wnd->GetClientSize().GetHeight());
  else
    c.wnd->SetClientSize(-1, Params[1].ToInt());
}
//..............................................................................
void THtml::funContainerWidth(const TStrObjList &Params, TMacroError &E)  {
  TPopupData *pd = TGlXApp::GetMainForm()->FindHtmlEx(Params[0]);
  if( pd == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "undefined html window: ").quote() << Params[0];
    return;
  }
  if( Params.Count() == 1 )
    E.SetRetVal(pd->Dialog->GetClientSize().GetWidth());
  else
    pd->Dialog->SetClientSize(Params[1].ToInt(), -1);
}
//..............................................................................
void THtml::funContainerHeight(const TStrObjList &Params, TMacroError &E)  {
  TPopupData *pd = TGlXApp::GetMainForm()->FindHtmlEx(Params[0]);
  if( pd == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "undefined html window: ").quote() << Params[0];
    return;
  }
  if( Params.Count() == 1 )
    E.SetRetVal(pd->Dialog->GetClientSize().GetHeight());
  else
    pd->Dialog->SetClientSize(-1, Params[1].ToInt());
}
//..............................................................................
//..............................................................................
//..............................................................................
THtml::TObjectsState::~TObjectsState()  {
  for( size_t i=0; i < Objects.Count(); i++ )
    delete Objects.GetObject(i);
}
//..............................................................................
void THtml::TObjectsState::SaveState()  {
  for( size_t i=0; i < html.ObjectCount(); i++ )  {
    if( !html.IsObjectManageble(i) )  continue;
    size_t ind = Objects.IndexOf( html.GetObjectName(i) );
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
    else if( EsdlInstanceOf(*obj, TButton) )    {  
      TButton* bt = (TButton*)obj;
      props->Add("val", bt->GetCaption());
      props->Add("checked", bt->IsDown());
      props->Add("data", bt->GetData());
    }
    else if( EsdlInstanceOf(*obj, TBmpButton) )    {  
      TBmpButton* bt = (TBmpButton*)obj;  
      props->Add("checked", bt->IsDown());
      props->Add("val", bt->GetSource());
      props->Add("data", bt->GetData());
    }
    else if( EsdlInstanceOf(*obj, TImgButton) )    {  
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
      props->Add("val", cb->GetValue());
      props->Add("items", cb->ItemsToString(';'));
      props->Add("data", cb->GetData());
    }
    else if( EsdlInstanceOf(*obj, TListBox) )  {  
      TListBox* lb = (TListBox*)obj;  
      props->Add("val", lb->GetValue() );
      props->Add("items", lb->ItemsToString(';') );
      props->Add("data", lb->GetData() );
    }
    else if( EsdlInstanceOf(*obj, TTreeView) )  {  
      TTreeView* tv = (TTreeView*)obj;  
      props->Add("state", tv->SaveState());
    }
    else if( EsdlInstanceOf(*obj, TLabel) )  {  
      TLabel* lb = (TLabel*)obj;  
      props->Add("val", lb->GetCaption() );
      props->Add("data", lb->GetData() );
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
//..............................................................................
void THtml::TObjectsState::RestoreState()  {
  for( size_t i=0; i < html.ObjectCount(); i++ )  {
    if( !html.IsObjectManageble(i) )  continue;
    size_t ind = Objects.IndexOf( html.GetObjectName(i) );
    if( ind == InvalidIndex )  continue;
    AOlxCtrl* obj = html.GetObject(i);
    wxWindow* win = html.GetWindow(i);
    TSStrStrList<olxstr,false>& props = *Objects.GetObject(ind);
    if( props["type"] != EsdlClassName(*obj) )  {
      TBasicApp::NewLogEntry(logError) << "Object type changed for: " << Objects.GetString(ind);
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
      tb->SetRange(olx_round(props["min"].ToDouble()), olx_round(props["max"].ToDouble()) );
      tb->SetValue(olx_round(props["val"].ToDouble()));
      tb->SetData(props["data"]);
    }
    else if( EsdlInstanceOf(*obj, TSpinCtrl) )  {  
      TSpinCtrl* sc = (TSpinCtrl*)obj;  
      sc->SetRange(olx_round(props["min"].ToDouble()), olx_round(props["max"].ToDouble()));
      sc->SetValue(olx_round(props["val"].ToDouble()));
      sc->SetData(props["data"]);
    }
    else if( EsdlInstanceOf(*obj, TButton) )    {  
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
    else if( EsdlInstanceOf(*obj, TBmpButton) )    {  
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
    else if( EsdlInstanceOf(*obj, TImgButton) )    {  
      TImgButton* bt = (TImgButton*)obj;  
      bt->SetData(props["data"]);
      bt->SetImages(props["val"], props["width"].ToInt(), props["height"].ToInt());
      bt->SetDown(props["checked"].ToBool());
      bt->SetEnabled(props["enabled"].ToBool());
    }
    else if( EsdlInstanceOf(*obj, TComboBox) )  {  
      TComboBox* cb = (TComboBox*)obj;  
      TStrList toks(props["items"], ';');
      cb->Clear();
      cb->AddItems(toks);
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
      TGlXApp::GetMainForm()->ProcessFunction(bg);
      TGlXApp::GetMainForm()->ProcessFunction(fg);
      if( EsdlInstanceOf(*win, TComboBox) )  {
        TComboBox* Box = (TComboBox*)win;
        if( !fg.IsEmpty() )  {
          wxColor fgCl = wxColor(fg.u_str());
          //Box->SetForegroundColour(fgCl);
#ifdef __WIN32__
          if( Box->GetPopupControl() != NULL )
            Box->GetPopupControl()->GetControl()->SetForegroundColour(fgCl);
          if( Box->GetTextCtrl() != NULL )
            Box->GetTextCtrl()->SetForegroundColour(fgCl);
#endif						
        }
        if( !bg.IsEmpty() )  {
          wxColor bgCl = wxColor(bg.u_str());
          //Box->SetBackgroundColour(bgCl);
#ifdef __WIN32__					
          if( Box->GetPopupControl() != NULL )
            Box->GetPopupControl()->GetControl()->SetBackgroundColour(bgCl);
          if( Box->GetTextCtrl() != NULL )
            Box->GetTextCtrl()->SetBackgroundColour(bgCl);
#endif						
        }
      }  
      else  {
        if( !fg.IsEmpty() )  win->SetForegroundColour( wxColor(fg.u_str()) );
        if( !bg.IsEmpty() )  win->SetBackgroundColour( wxColor(bg.u_str()) );
      }
    }
  }
}
//..............................................................................
void THtml::TObjectsState::SaveToFile(const olxstr& fn)  {
}
//..............................................................................
bool THtml::TObjectsState::LoadFromFile(const olxstr& fn)  {
  return true;
}
//..............................................................................
TSStrStrList<olxstr,false>* THtml::TObjectsState::DefineControl(const olxstr& name, const std::type_info& type) {
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
  else if( type == typeid(TButton) )    {  
    props->Add("checked");
    props->Add("val");
  }
  else if( type == typeid(TBmpButton) )    {  
    props->Add("checked");
    props->Add("val");
  }
  else if( type == typeid(TImgButton) )    {  
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
//..............................................................................
void THtml::macGroup(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  bool intersects = false;
  for( size_t i=0; i < Groups.Count(); i++ )  {
    for( size_t j=0; j < Cmds.Count(); j++ )  {
      if( Groups[i].IndexOfi(Cmds[j]) != InvalidIndex )  {
        intersects = true;
        break;
      }
    }
    if( intersects )  break;
  }
  if( intersects )  {
    E.ProcessingError(__OlxSrcInfo, "The group intersects with already existing one");
    return;
  }
  Groups.AddNew(Cmds);
}
//..............................................................................

