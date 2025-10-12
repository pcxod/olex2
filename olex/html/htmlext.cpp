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
#include "eutf8.h"
#include "wxzipfs.h"
//.............................................................................
THtml::THtml(THtmlManager &manager, wxWindow *Parent,
  const olxstr &pop_name, int flags)
  : wxHtmlWindow(Parent, -1, wxDefaultPosition, wxDefaultSize, flags),
  AOlxCtrl(this),
  THtmlPreprocessor(pop_name),
  parentCell(0),
  ObjectsState(*this),
  Manager(manager),
  OnLink(Actions.New("ONLINK")),
  OnURL(Actions.New("ONURL")),
  OnDblClick(Actions.New("ONDBLCL")),
  OnKey(Actions.New("ONCHAR")),
  OnSize(Actions.New("ONSIZE"))
{
  Root = new THtmlSwitch(this, 0);
  Movable = false;
  MouseDown = false;
  ShowTooltips = true;
  PageLoadRequested = false;
  if (stateTooltipsVisible() == InvalidIndex) {
    stateTooltipsVisible() = TStateRegistry::GetInstance().Register("htmlttvis",
      new TStateRegistry::Slot(
        TStateRegistry::NewGetter<THtml>(this, &THtml::GetShowTooltips),
        new TStateRegistry::TMacroSetter("html.Tooltips")
      )
    );
  }
  Bind(wxEVT_LEFT_DCLICK, &THtml::OnMouseDblClick, this);
  Bind(wxEVT_LEFT_DOWN, &THtml::OnMouseDown, this);
  Bind(wxEVT_LEFT_UP, &THtml::OnMouseUp, this);
  Bind(wxEVT_MOTION, &THtml::OnMouseMotion, this);
  Bind(wxEVT_KEY_DOWN, &THtml::OnKeyDown, this);
  Bind(wxEVT_CHAR, &THtml::OnChar, this);
  Bind(wxEVT_SIZE, &THtml::OnSizeEvt, this);
  Bind(wxEVT_TEXT_COPY, &THtml::OnClipboard, this);
}
//.............................................................................
THtml::~THtml() {
  delete Root;
  ClearSwitchStates();
}
//.............................................................................
bool THtml::Destroy() {
  for (size_t i = 0; i < Objects.Count(); i++) {
    Objects.GetValue(i).a->ClearActionQueues();
  }
  return wxHtmlWindow::Destroy();
}
//.............................................................................
void THtml::OnLinkClicked(const wxHtmlLinkInfo& link) {
  if (!MouseDown) {
    return;
  }
  MouseDown = false;
  olxstr Href = link.GetHref();
  size_t ind = Href.FirstIndexOf('%');
  while (ind != InvalidIndex && ((ind + 2) < Href.Length()) &&
    olxstr::o_ishexdigit(Href[ind + 1]) &&
    olxstr::o_ishexdigit(Href[ind + 2]))
  {
    int val = Href.SubString(ind + 1, 2).RadInt<int>(16);
    Href.Delete(ind, 3);
    Href.Insert(val, ind);
    ind = Href.FirstIndexOf('%');
  }
  if (!OnLink.Execute((const AOlxCtrl*)this, (IOlxObject*)&Href)) {
    wxHtmlLinkInfo NewLink(Href.u_str(), link.GetTarget());
    wxHtmlWindow::OnLinkClicked(NewLink);
  }
}
//.............................................................................
wxHtmlOpeningStatus THtml::OnOpeningURL(wxHtmlURLType type, const wxString& url,
  wxString *redirect) const
{
  olxstr Url = url;
  if (!OnURL.Execute((const AOlxCtrl *)this, &Url)) {
    return wxHTML_OPEN;
  }
  return wxHTML_BLOCK;
}
//.............................................................................
void THtml::OnMouseDown(wxMouseEvent& event) {
  this->SetFocusIgnoringChildren();
  MouseX = event.GetX();
  MouseY = event.GetY();
  MouseDown = true;
  if (Movable) {
    SetCursor(wxCursor(wxCURSOR_SIZING));
  }
  event.Skip();
}
//.............................................................................
void THtml::OnMouseUp(wxMouseEvent& event) {
  if (Movable && MouseDown) {
    SetCursor(wxCursor(wxCURSOR_ARROW));
  }
  event.Skip();
}
//.............................................................................
void THtml::OnMouseMotion(wxMouseEvent& event) {
  if (!Movable || !MouseDown) {
    event.Skip();
    return;
  }
  int dx = event.GetX() - MouseX;
  int dy = event.GetY() - MouseY;
  if ((dx | dy) == 0) {
    return;
  }
  wxWindow* parent = GetParent();
  if (parent == 0 || parent->GetParent() == 0) {
    return;
  }
  int x = 0, y = 0;
  parent->GetPosition(&x, &y);
  parent->SetSize(x + dx, y + dy, -1, -1, wxSIZE_USE_EXISTING);
}
//.............................................................................
void THtml::OnMouseDblClick(wxMouseEvent& event) {
  event.Skip();
  volatile THtmlManager::DestructionLocker dm =
    Manager.LockDestruction(this, (AOlxCtrl *)this);
  OnDblClick.Execute((const AOlxCtrl *)this, &OnDblClickData);
}
//.............................................................................
void THtml::OnSizeEvt(wxSizeEvent& event)  {
  event.Skip();
  volatile THtmlManager::DestructionLocker dm =
    Manager.LockDestruction(this, (AOlxCtrl *)this);
  OnSize.Execute((const AOlxCtrl *)this, &OnSizeData);
}
//.............................................................................
bool THtml::Dispatch(int MsgId, short MsgSubId, const IOlxObject* Sender,
  const IOlxObject* Data, TActionQueue *)
{
  if (MsgId == html_parent_resize) {
    volatile THtmlManager::DestructionLocker dm =
      Manager.LockDestruction(this, (AOlxCtrl *)this);
    OnSize.Execute((const AOlxCtrl *)this, &OnSizeData);
  }
  return true;
}
//.............................................................................
void THtml::OnKeyDown(wxKeyEvent& event) {
  if (event.GetModifiers() == wxMOD_CMD && event.GetKeyCode() == 'C') {
    CopySelection();
    event.Skip(false);
    return;
  }
  event.Skip();
}
//.............................................................................
void THtml::OnNavigation(wxNavigationKeyEvent& event) {
  event.Skip();
}
//.............................................................................
void THtml::OnChar(wxKeyEvent& event) {
  TKeyEvent KE(event);
  OnKey.Execute((const AOlxCtrl *)this, &KE);
}
//.............................................................................
bool THtml::ProcessPageLoadRequest()  {
  if (!PageLoadRequested || IsPageLocked()) {
    return false;
  }
  PageLoadRequested = false;
  bool res = false;
  if (!PageRequested.IsEmpty()) {
    res = LoadPage(PageRequested.u_str());
  }
  else {
    res = UpdatePage();
  }
  PageRequested.SetLength(0);
  return res;
}
//.............................................................................
bool THtml::LoadPage(const wxString &file) {
  if (file.IsEmpty()) {
    return false;
  }

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
  if (Path.Length() > 1) {
    Path = TEFile::OSPath(Path);
    if (TEFile::IsAbsolutePath(Path)) {
      WebFolder = Path;
    }
  }
  else {
    Path = WebFolder;
  }
  if (Path == WebFolder) {
    TestFile = WebFolder + TestFile;
  }
  else {
    TestFile = WebFolder + Path + TestFile;
  }

  if (!TZipWrapper::IsValidFileName(TestFile) &&
    !TFileHandlerManager::Exists(file))
  {
    TBasicApp::NewLogEntry(logError) << "File does not exists: '" <<
      file << '\'';
    return false;
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
bool THtml::UpdatePage(bool update_indices) {
  if (IsPageLocked()) {
    PageLoadRequested = true;
    PageRequested.SetLength(0);
    return true;
  }
  TStopWatch sw(__FUNC__);
  LockPageLoad(this);
  Freeze();

  olxstr Path = TEFile::ExtractFilePath(FileName);
  if (TEFile::IsAbsolutePath(FileName)) {
    Path = TEFile::OSPath(Path);
    WebFolder = Path;
  }
  else {
    Path = WebFolder;
  }
  sw.start("Updating indices");
  olxstr oldPath = TEFile::CurrentDir();
  TEFile::ChangeDir(WebFolder);
  if (update_indices) { // reload switches
    for (size_t i = 0; i < Root->SwitchCount(); i++) {
      try {
        Root->GetSwitch(i).UpdateFileIndex();
      }
      catch (const TExceptionBase& e) {
        TBasicApp::NewLogEntry(logException) << e;
      }
    }
  }

  int xPos = -1, yPos = -1, xWnd = -1, yWnd = -1;
  GetViewStart(&xPos, &yPos);
  olxstr FocusedControl;
  {
    wxWindow *fw = wxWindow::FindFocus();
    if (fw != 0 && fw->GetParent() == this) {
      for (size_t i = 0; i < Objects.Count(); i++) {
        if (Objects.GetValue(i).b == fw) {
          FocusedControl = Objects.GetKey(i);
          break;
        }
      }
    }
  }
  sw.start("Saving to string list");
  TStrList Res;
  Root->ToStrings(Res, false);
  sw.start("Saving object states");
  ObjectsState.SaveState();
  Objects.Clear();
  sw.start("Setting the page");
  SetPage(Res.Text(' ').u_str());
  sw.start("Restoring object states");
  ObjectsState.RestoreState();
  sw.start("Loading inner html objects");
  wxWindowList &wil = GetChildren();
  TPtrList<THtml> htmls;
  for (size_t i = 0; i < wil.size(); i++) {
    if (olx_is<THtml>(*wil[i])) {
      THtml * ht = (THtml*)wil[i];
      ht->LoadPage(ht->GetHomePage().u_str());
      htmls << ht;
    }
  }
  CreateLayout();
  Scroll(xPos, yPos);
  if (!olx_is<THtml>(*GetParent())) {
    SwitchSources().Clear();
    SwitchSource().SetLength(0);
  }
  TEFile::ChangeDir(oldPath);
  if (!FocusedControl.IsEmpty()) {
    size_t ind = Objects.IndexOf(FocusedControl);
    if (ind != InvalidIndex) {
      wxWindow* wnd = Objects.GetValue(ind).b;
      if (olx_type<TTextEdit>::check(*wnd)) {
        ((TTextEdit*)wnd)->SetSelection(-1, -1);
      }
      else if (olx_type<TComboBox>::check(*wnd)) {
        TComboBox* cb = (TComboBox*)wnd;
        wnd = cb;
      }
      else if (olx_type<TSpinCtrl>::check(*wnd)) {
        TSpinCtrl* sc = (TSpinCtrl*)wnd;
        olxstr sv(sc->GetValue());
        sc->SetSelection((long)sv.Length(), -1);
      }
      wnd->SetFocus();
    }
    else {
      FocusedControl.SetLength(0);
    }
  }
  sw.start("Updating layout");
  wxClientDC dc(this);
  wxHtmlRenderingInfo r_info;
  this->m_Cell->DrawInvisible(dc, 0, 0, r_info);
  //for (size_t i = 0; i < htmls.Count(); i++) {
  //  if (htmls[i]->GetParentCell()->GetFloatY() != 0) {
  //    wxClientDC dc1(htmls[i]);
  //    wxHtmlRenderingInfo r_info1;
  //    htmls[i]->m_Cell->DrawInvisible(dc1, 0, 0, r_info1);
  //    int bh = htmls[i]->GetBestHeight(htmls[i]->GetParentCell()->GetWidth());
  //    htmls[i]->GetParentCell()->SetHeight(bh+20);
  //    htmls[i]->WI.SetHeight(bh + 20);
  //    htmls[i]->GetParentCell()->GetRootCell()->Layout(
  //      htmls[i]->GetParentCell()->GetWidth()-1);
  //  }
  //}
  //CreateLayout();
  //this->m_Cell->DrawInvisible(dc, 0, 0, r_info);
  sw.start("Finsihing...");
  for (size_t i = 0; i < wil.size(); i++) {
    wil[i]->Show();
  }
  Thaw();
  UnlockPageLoad(this);
  return true;
}
//.............................................................................
void THtml::DoScroll(int x, int y) {
  // disable automatic horizontal scrolling
  wxHtmlWindow::DoScroll(0, y);
}
//.............................................................................
void THtml::ScrollWindow(int dx, int dy, const wxRect* rect)  {
  wxHtmlWindow::ScrollWindow(dx, dy,rect);
} 
//.............................................................................
bool THtml::AddControl(const olxstr& Name, AOlxCtrl *Object, wxWindow* wxWin,
  bool Manage)
{
  // an anonymous object
  if (Name.IsEmpty()) {
    return false;
  }
  size_t idx = Objects.IndexOf(Name);
  if (idx != InvalidIndex) {
    Objects.GetValue(idx).a = Object;
    Objects.GetValue(idx).b = wxWin;
    Objects.GetValue(idx).c = Manage;
    return true;
  }
  else {
    Objects.Add(Name, Association::Create(Object, wxWin, Manage));
    return false;
  }
}
//.............................................................................
void THtml::OnCellMouseHover(wxHtmlCell *Cell, wxCoord x, wxCoord y) {
  wxHtmlLinkInfo *Link = Cell->GetLink(x, y);
  if (Link != NULL) {
    olxstr Href = Link->GetTarget();
    if (Href.IsEmpty()) {
      Href = Link->GetHref();
    }
    size_t ind = Href.FirstIndexOf('%');
    while (ind != InvalidIndex && ((ind + 2) < Href.Length())) {
      if (Href.CharAt(ind + 1) == '%') {
        Href.Delete(ind, 1);
        ind = Href.FirstIndexOf('%', ind + 1);
        continue;
      }
      olxstr nm = Href.SubString(ind + 1, 2);
      if (nm.IsNumber()) {
        try {
          int val = nm.RadInt<int>(16);
          Href.Delete(ind, 3);
          Href.Insert((char)val, ind);
        }
        catch (...) {}
      }
      ind = Href.FirstIndexOf('%', ind + 1);
    }
    if (ShowTooltips) {
      wxToolTip *tt = GetToolTip();
      wxString wxs(Href.Replace("#href", Link->GetHref()).u_str());
      if (tt == 0 || tt->GetTip() != wxs) {
        SetToolTip(wxs.SubString(0, olx_min(wxs.size(), 256)));
      }
    }
  }
  else {
    SetToolTip(NULL);
  }
}
//.............................................................................
olxstr THtml::GetObjectValue(const AOlxCtrl *Obj) {
  const std::type_info &ti = typeid(*Obj);
  if (ti == typeid(TTextEdit)) { return ((TTextEdit*)Obj)->GetText(); }
  if (ti == typeid(TCheckBox)) { return ((TCheckBox*)Obj)->GetCaption(); }
  if (ti == typeid(TTrackBar)) { return ((TTrackBar*)Obj)->GetValue(); }
  if (ti == typeid(TSpinCtrl)) { return ((TSpinCtrl*)Obj)->GetValue(); }
  if (ti == typeid(TButton)) { return ((TButton*)Obj)->GetCaption(); }
  if (ti == typeid(TComboBox)) { return ((TComboBox*)Obj)->GetText(); }
  if (ti == typeid(TChoice)) { return ((TChoice*)Obj)->GetText(); }
  if (ti == typeid(TListBox)) { return ((TListBox*)Obj)->GetValue(); }
  if (ti == typeid(TTreeView)) {
    TTreeView* T = (TTreeView*)Obj;
    wxTreeItemId ni = T->GetSelection();
    if (!ni.IsOk()) {
      return EmptyString();
    }
    wxTreeItemData* td = T->GetItemData(ni);
    if (td == 0 || !olx_type<TTreeNodeData>::check(*td)) {
      return EmptyString();
    }
    TTreeNodeData* olx_td = dynamic_cast<TTreeNodeData*>(td);
    if (olx_td == 0 || olx_td->GetData() == 0) {
      return EmptyString();
    }
    return olx_td->GetData()->ToString();
  }
  if (ti == typeid(TDateCtrl)) {
    return ((TDateCtrl*)Obj)->GetValue().Format(wxT("%Y-%m-%d"));
  }
  if (ti == typeid(TColorCtrl)) {
    wxColor c = ((TColorCtrl*)Obj)->GetColour();
    return OLX_RGBA(c.Red(), c.Green(), c.Blue(), c.Alpha());
  }
  return EmptyString();
}
//.............................................................................
void THtml::SetObjectValue(AOlxCtrl *Obj,
  const olxstr& name, const olxstr& Value)
{
  const std::type_info &ti = typeid(*Obj);
  if (ti == typeid(TTextEdit))
    ((TTextEdit*)Obj)->SetText(Value);
  else if (ti == typeid(TCheckBox))
    ((TCheckBox*)Obj)->SetCaption(Value);
  else if (ti == typeid(TTrackBar)) {
    const size_t si = Value.IndexOf(',');
    if (si == InvalidIndex)
      ((TTrackBar*)Obj)->SetValue(olx_round(Value.ToDouble()));
    else
      ((TTrackBar*)Obj)->SetRange(olx_round(Value.SubStringTo(si).ToDouble()),
      olx_round(Value.SubStringFrom(si + 1).ToDouble()));
  }
  else if (ti == typeid(TSpinCtrl)) {
    const size_t si = Value.IndexOf(',');
    if (si == InvalidIndex)
      ((TSpinCtrl*)Obj)->SetValue(olx_round(Value.ToDouble()));
    else
      ((TSpinCtrl*)Obj)->SetRange(olx_round(Value.SubStringTo(si).ToDouble()),
      olx_round(Value.SubStringFrom(si + 1).ToDouble()));
  }
  else if (ti == typeid(TButton))
    ((TButton*)Obj)->SetLabel(Value.u_str());
  else if (ti == typeid(TComboBox)) {
    ((TComboBox*)Obj)->SetText(Value);
    ((TComboBox*)Obj)->Update();
  }
  else if (ti == typeid(TChoice)) {
    ((TChoice*)Obj)->SetText(Value);
    ((TChoice*)Obj)->Update();
  }
  else if (ti == typeid(TListBox)) {
    TListBox *L = (TListBox*)Obj;
    int index = L->GetSelection();
    if (index >= 0 && index < L->Count())
      L->SetString(index, Value.u_str());
  }
  else if (ti == typeid(TDateCtrl)) {
    wxDateTime dt;
    dt.ParseDateTime(Value.u_str());
    ((TDateCtrl*)Obj)->SetValue(dt);
  }
  else if (ti == typeid(TColorCtrl)) {
    ((TColorCtrl*)Obj)->SetColour(wxColor(Value.u_str()));
  }
  else if (ti == typeid(THtml)) {
    THtml *h = (THtml*)Obj;
    olxstr fn = name + "_content.htm";
    olxcstr cont = TUtf8::Encode(Value);
    TFileHandlerManager::AddMemoryBlock(fn, cont.c_str(), cont.Length(), 0);
    h->LoadPage(fn.u_str());
    //h->SetPage(Value.u_str());
  }
  else
    return;
}
//.............................................................................
const olxstr& THtml::GetObjectData(const AOlxCtrl *Obj)  {
  const std::type_info &ti = typeid(*Obj);
  if (ti == typeid(TTextEdit)) { return ((TTextEdit*)Obj)->GetData(); }
  if (ti == typeid(TCheckBox)) { return ((TCheckBox*)Obj)->GetData(); }
  if (ti == typeid(TTrackBar)) { return ((TTrackBar*)Obj)->GetData(); }
  if (ti == typeid(TSpinCtrl)) { return ((TSpinCtrl*)Obj)->GetData(); }
  if (ti == typeid(TButton)) { return ((TButton*)Obj)->GetData(); }
  if (ti == typeid(TComboBox)) { return ((TComboBox*)Obj)->GetData(); }
  if (ti == typeid(TChoice)) { return ((TChoice*)Obj)->GetData(); }
  if (ti == typeid(TListBox)) { return ((TListBox*)Obj)->GetData(); }
  if (ti == typeid(TTreeView)) { return ((TTreeView*)Obj)->GetData(); }
  return EmptyString();
}
//.............................................................................
olxstr THtml::GetObjectImage(const AOlxCtrl* Obj)  {
  const std::type_info &ti = typeid(*Obj);
  if (ti == typeid(TBmpButton))
    return ((TBmpButton*)Obj)->GetSource();
  else if (ti == typeid(THtmlImageCell))
    return ((THtmlImageCell*)Obj)->GetSource();
  return EmptyString();
}
//.............................................................................
bool THtml::SetObjectImage(AOlxCtrl* Obj, const olxstr& src) {
  if (src.IsEmpty())
    return false;
  const std::type_info &ti = typeid(*Obj);
  if (ti == typeid(TBmpButton) || ti == typeid(THtmlImageCell)) {
    olx_object_ptr<wxFSFile> fsFile = TFileHandlerManager::GetFSFileHandler(src);
    if (fsFile == 0) {
      TBasicApp::NewLogEntry(logError) <<
        "Setimage: could not locate specified file: " << src;
      return false;
    }
    wxImage image(*(fsFile->GetStream()), wxBITMAP_TYPE_ANY);
    if (!image.Ok()) {
      TBasicApp::NewLogEntry(logError) <<
        "Setimage: could not read specified file: " << src;
      return false;
    }
    if (ti == typeid(TBmpButton)) {
      ((TBmpButton*)Obj)->SetBitmapLabel(image);
      ((TBmpButton*)Obj)->SetSource(src);
      ((TBmpButton*)Obj)->Refresh(true);
    }
    else if (ti == typeid(THtmlImageCell)) {
      ((THtmlImageCell*)Obj)->SetImage(image);
      ((THtmlImageCell*)Obj)->SetSource(src);
      ((THtmlImageCell*)Obj)->GetWindowInterface()->GetHTMLWindow()->Refresh(true);
    }
  }
  else if (ti == typeid(TImgButton)) {
    ((TImgButton*)Obj)->SetImages(src);
    ((TImgButton*)Obj)->Refresh(true);
  }
  else {
    TBasicApp::NewLogEntry(logError) << "Setimage: unsupported object type";
    return false;
  }
  return true;
}
//.............................................................................
olxstr THtml::GetObjectItems(const AOlxCtrl* Obj)  {
  const std::type_info &ti = typeid(*Obj);
  if (ti == typeid(TComboBox)) {
    return ((TComboBox*)Obj)->ItemsToString(';');
  }
  if (ti == typeid(TChoice)) {
    return ((TChoice*)Obj)->ItemsToString(';');
  }
  if (ti == typeid(TListBox)) {
    return ((TListBox*)Obj)->ItemsToString(';');
  }
  return EmptyString();
}
//.............................................................................
bool THtml::SetObjectItems(AOlxCtrl* Obj, const olxstr& src)  {
  const std::type_info &ti = typeid(*Obj);
  if (ti == typeid(TComboBox)) {
    TComboBox *cb = ((TComboBox*)Obj);
    cb->Clear();
    cb->AddItems(TStrList(src, ';'));
  }
  else if (ti == typeid(TChoice)) {
    TChoice *cb = ((TChoice*)Obj);
    cb->Clear();
    cb->AddItems(TStrList(src, ';'));
  }
  else if (ti == typeid(TListBox)) {
    ((TListBox*)Obj)->Clear();
    ((TListBox*)Obj)->AddItems(TStrList(src, ';'));
  }
  else {
    TBasicApp::NewLogEntry(logError) << "SetItems: unsupported object type";
    return false;
  }
  return true;
}
//.............................................................................
void THtml::SetObjectData(AOlxCtrl *Obj, const olxstr& Data)  {
  const std::type_info &ti = typeid(*Obj);
  if (ti == typeid(TTextEdit))  { ((TTextEdit*)Obj)->SetData(Data);  return; }
  if (ti == typeid(TCheckBox))  { ((TCheckBox*)Obj)->SetData(Data);  return; }
  if( ti == typeid(TTrackBar) )  {  ((TTrackBar*)Obj)->SetData(Data);  return;  }
  if( ti == typeid(TSpinCtrl) )  {  ((TSpinCtrl*)Obj)->SetData(Data);  return;  }
  if( ti == typeid(TButton) )    {  ((TButton*)Obj)->SetData(Data);    return;  }
  if( ti == typeid(TComboBox) )  {  ((TComboBox*)Obj)->SetData(Data);  return;  }
  if (ti == typeid(TChoice))  { ((TChoice*)Obj)->SetData(Data);  return; }
  if (ti == typeid(TListBox))  { ((TListBox*)Obj)->SetData(Data);  return; }
  if( ti == typeid(TTreeView) )  {  ((TTreeView*)Obj)->SetData(Data);  return;  }
}
//.............................................................................
bool THtml::GetObjectState(const AOlxCtrl *Obj, const olxstr& state)  {
  const std::type_info &ti = typeid(*Obj);
  if (ti == typeid(TCheckBox))
    return ((TCheckBox*)Obj)->IsChecked();
  else if( ti == typeid(TImgButton) )  {
    if( state.Equalsi("enabled") )
      return ((TImgButton*)Obj)->IsEnabled();
    else if( state.Equalsi("down") )
      return ((TImgButton*)Obj)->IsDown();
    else
      return false;
  }
  else if( ti == typeid(TButton) )
    return ((TButton*)Obj)->IsDown();
  else
    return false;
}
//.............................................................................
void THtml::SetObjectState(AOlxCtrl *Obj, bool State, const olxstr& state_name)  {
  const std::type_info &ti = typeid(*Obj);
  if (ti == typeid(TCheckBox))
    ((TCheckBox*)Obj)->SetChecked(State);
  else if (ti == typeid(TImgButton)) {
    if( state_name.Equalsi("enabled") )
      ((TImgButton*)Obj)->SetEnabled(State);
    else if (state_name.Equalsi("down"))
      ((TImgButton*)Obj)->SetDown(State);
    ((TImgButton*)Obj)->Refresh(true);
  }
  else if (ti == typeid(TButton))
    ((TButton*)Obj)->SetDown(State);
}
//.............................................................................
//.............................................................................
//.............................................................................
THtml::TObjectsState::~TObjectsState()  {
  for( size_t i=0; i < Objects.Count(); i++ )
    delete Objects.GetValue(i);
}
//.............................................................................
void THtml::TObjectsState::SaveState() {
  for (size_t i = 0; i < html.ObjectCount(); i++) {
    if (!html.IsObjectManageble(i))  continue;
    size_t ind = Objects.IndexOf(html.GetObjectName(i));
    AOlxCtrl* obj = html.GetObject(i);
    wxWindow* win = html.GetWindow(i);
    olxstr_dict<olxstr, false>* props;
    if (ind == InvalidIndex) {
      props = new olxstr_dict<olxstr, false>;
      Objects.Add(html.GetObjectName(i), props);
    }
    else {
      props = Objects.GetValue(ind);
      props->Clear();
    }
    props->Add("type", EsdlClassName(*obj));  // type
    if (obj->Is<TTextEdit>()) {
      TTextEdit* te = (TTextEdit*)obj;
      props->Add("val", te->GetText());
      props->Add("data", te->GetData());
    }
    else if (obj->Is<TCheckBox>()) {
      TCheckBox* cb = (TCheckBox*)obj;
      props->Add("val", cb->GetCaption());
      props->Add("checked", cb->IsChecked());
      props->Add("data", cb->GetData());
    }
    else if (obj->Is<TTrackBar>()) {
      TTrackBar* tb = (TTrackBar*)obj;
      props->Add("min", tb->GetMin());
      props->Add("max", tb->GetMax());
      props->Add("val", tb->GetValue());
      props->Add("data", tb->GetData());
    }
    else if (obj->Is<TSpinCtrl>()) {
      TSpinCtrl* sc = (TSpinCtrl*)obj;
      props->Add("min", sc->GetMin());
      props->Add("max", sc->GetMax());
      props->Add("val", sc->GetValue());
      props->Add("data", sc->GetData());
    }
    else if (obj->Is<TButton>()) {
      TButton* bt = (TButton*)obj;
      props->Add("val", bt->GetCaption());
      props->Add("checked", bt->IsDown());
      props->Add("data", bt->GetData());
    }
    else if (obj->Is<TBmpButton>()) {
      TBmpButton* bt = (TBmpButton*)obj;
      props->Add("checked", bt->IsDown());
      props->Add("val", bt->GetSource());
      props->Add("data", bt->GetData());
    }
    else if (obj->Is<TImgButton>()) {
      TImgButton* bt = (TImgButton*)obj;
      props->Add("checked", bt->IsDown());
      props->Add("enabled", bt->IsEnabled());
      props->Add("val", bt->GetSource());
      props->Add("width", bt->GetWidth());
      props->Add("height", bt->GetHeight());
      props->Add("data", bt->GetData());
    }
    else if (obj->Is<TComboBox>()) {
      TComboBox* cb = (TComboBox*)obj;
      props->Add("val", cb->GetValue());
      props->Add("items", cb->ItemsToString(';'));
      props->Add("data", cb->GetData());
    }
    else if (obj->Is<TChoice>()) {
      TChoice* c = (TChoice*)obj;
      props->Add("val", c->GetValue());
      props->Add("items", c->ItemsToString(';'));
      props->Add("data", c->GetData());
    }
    else if (obj->Is<TListBox>()) {
      TListBox* lb = (TListBox*)obj;
      props->Add("val", lb->GetValue());
      props->Add("items", lb->ItemsToString(';'));
      props->Add("data", lb->GetData());
    }
    else if (obj->Is<TTreeView>()) {
      TTreeView* tv = (TTreeView*)obj;
      props->Add("state", tv->SaveState());
    }
    else if (obj->Is<TLabel>()) {
      TLabel* lb = (TLabel*)obj;
      props->Add("val", lb->GetCaption());
      props->Add("data", lb->GetData());
    }
    else //?
      ;
    // stroring the control colours, it is generic
    if (win != 0) {
      props->Add("fg", win->GetForegroundColour().GetAsString(wxC2S_HTML_SYNTAX));
      props->Add("bg", win->GetBackgroundColour().GetAsString(wxC2S_HTML_SYNTAX));
    }
  }
}
//.............................................................................
void THtml::TObjectsState::RestoreState() {
  olex2::IOlex2Processor *op = olex2::IOlex2Processor::GetInstance();
  for (size_t i = 0; i < html.ObjectCount(); i++) {
    if (!html.IsObjectManageble(i)) {
      continue;
    }
    size_t ind = Objects.IndexOf(html.GetObjectName(i));
    if (ind == InvalidIndex) {
      continue;
    }
    AOlxCtrl* obj = html.GetObject(i);
    wxWindow* win = html.GetWindow(i);
    olxstr_dict<olxstr, false>& props = *Objects.GetValue(ind);
    if (props.Get("type") != EsdlClassName(*obj)) {
      TBasicApp::NewLogEntry(logError) << "Object type changed for: "
        << Objects.GetKey(ind);
      continue;
    }
    if (obj->Is<TTextEdit>()) {
      TTextEdit* te = (TTextEdit*)obj;
      te->SetText(props["val"]);
      te->SetData(props["data"]);
    }
    else if (obj->Is<TCheckBox>()) {
      TCheckBox* cb = (TCheckBox*)obj;
      cb->SetCaption(props["val"]);
      cb->SetChecked(props["checked"].ToBool());
      cb->SetData(props["data"]);
    }
    else if (obj->Is<TTrackBar>()) {
      TTrackBar* tb = (TTrackBar*)obj;
      tb->SetRange(olx_round(props["min"].ToDouble()), olx_round(props["max"].ToDouble()));
      tb->SetValue(olx_round(props["val"].ToDouble()));
      tb->SetData(props["data"]);
      tb->SetData(props.Get("data"));
    }
    else if (obj->Is<TSpinCtrl>()) {
      TSpinCtrl* sc = (TSpinCtrl*)obj;
      sc->SetRange(olx_round(props["min"].ToDouble()), olx_round(props["max"].ToDouble()));
      sc->SetValue(olx_round(props["val"].ToDouble()));
      sc->SetData(props["data"]);
      sc->SetData(props.Get("data"));
    }
    else if (obj->Is<TButton>()) {
      TButton* bt = (TButton*)obj;
      bt->SetData(props["data"]);
      bt->SetCaption(props["val"]);
      bt->OnDown.SetEnabled(false);
      bt->OnUp.SetEnabled(false);
      bt->OnClick.SetEnabled(false);
      bt->SetDown(props["checked"].ToBool());
      bt->OnDown.SetEnabled(true);
      bt->OnUp.SetEnabled(true);
      bt->OnClick.SetEnabled(true);
    }
    else if (obj->Is<TBmpButton>()) {
      TBmpButton* bt = (TBmpButton*)obj;
      bt->SetData(props["data"]);
      bt->SetSource(props["val"]);
      bt->OnDown.SetEnabled(false);
      bt->OnUp.SetEnabled(false);
      bt->OnClick.SetEnabled(false);
      bt->SetDown(props["checked"].ToBool());
      bt->OnDown.SetEnabled(true);
      bt->OnUp.SetEnabled(true);
      bt->OnClick.SetEnabled(true);
    }
    else if (obj->Is<TImgButton>()) {
      TImgButton* bt = (TImgButton*)obj;
      bt->SetData(props["data"]);
      bt->SetImages(props["val"], props["width"].ToInt(), props["height"].ToInt());
      bt->SetDown(props["checked"].ToBool());
      bt->SetEnabled(props["enabled"].ToBool());
    }
    else if (obj->Is<TComboBox>()) {
      TComboBox* cb = (TComboBox*)obj;
      cb->Clear();
      cb->AddItems(TStrList(props["items"], ';'));
      cb->SetText(props["val"]);
      cb->SetData(props["data"]);
    }
    else if (obj->Is<TChoice>()) {
      TChoice* cb = (TChoice*)obj;
      cb->Clear();
      cb->AddItems(TStrList(props["items"], ';'));
      cb->SetText(props["val"]);
      cb->SetData(props["data"]);
    }
    else if (obj->Is<TListBox>()) {
      TListBox* lb = (TListBox*)obj;
      lb->Clear();
      lb->AddItems(TStrList(props["items"], ';'));
    }
    else if (obj->Is<TTreeView>()) {
      ((TTreeView*)obj)->RestoreState(props["state"]);
    }
    else if (obj->Is<TLabel>()) {
      TLabel* lb = (TLabel*)obj;
      lb->SetCaption(props["val"]);
    }
    else //?
      ;
    // restoring the control colours, it is generic
    if (win != NULL && false) {
      olxstr bg(props["bg"]), fg(props["fg"]);
      if (op != 0) {
        op->processFunction(bg);
        op->processFunction(fg);
      }
      if (!fg.IsEmpty()) {
        win->SetForegroundColour(wxColor(fg.u_str()));
      }
      if (!bg.IsEmpty()) {
        win->SetBackgroundColour(wxColor(bg.u_str()));
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
olxstr_dict<olxstr,false>* THtml::TObjectsState::DefineControl(
  const olxstr& name, const std::type_info& type)
{
  const size_t ind = Objects.IndexOf( name );
  if( ind != InvalidIndex )
    throw TFunctionFailedException(__OlxSourceInfo, "object already exists");
  olxstr_dict<olxstr,false>* props = new olxstr_dict<olxstr,false>;

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
  else if (type == typeid(TChoice))  {
    props->Add("val");
    props->Add("items");
  }
  else if (type == typeid(TListBox))  {
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
  if (w == 0) {
    return;
  }
  bool processed = true;
  wxString text;
  if (olx_type<TTextEdit>::check(*w) || olx_type<wxTextCtrl>::check (*w)) {
    wxTextCtrl *tc = dynamic_cast<wxTextCtrl*>(w);
    if (tc != 0) {
      text = tc->GetStringSelection();
    }
  }
  else if(olx_type<TComboBox>::check(*w))
    text = dynamic_cast<wxComboBox*>(w)->GetStringSelection();
  else
    processed = false;
  if (processed && !text.IsEmpty() && wxTheClipboard->Open()) {
    if (wxTheClipboard->IsSupported(wxDF_UNICODETEXT)) {
      wxTheClipboard->SetData(new wxTextDataObject(text));
    }
    wxTheClipboard->Close();
  }
  evt.Skip(processed);
}
//.............................................................................
void THtml::SetFonts(const olxstr &normal, const olxstr &fixed) {
  NormalFont = normal;
  FixedFont = fixed;
  Root->Clear();
  Objects.Clear();
  wxHtmlWindow::SetFonts(normal.u_str(), fixed.u_str());
}
//.............................................................................
void THtml::CyclicReduce(olxstr_dict<olxstr, true>& values) {
  for (size_t i = 0; i < values.Count(); i++) {
    bool reduce = false;
    if (values.GetValue(i).StartsFrom('#')) {
      TEBitArray used(values.Count());
      used.SetTrue(i);
      size_t ni = values.IndexOf(values.GetValue(i).SubStringFrom(1));
      while (ni != InvalidIndex) {
        if (used[ni]) {
          reduce = true;
          break;
        }
        ni = values.IndexOf(values.GetValue(ni).SubStringFrom(1));
      }
    }
    if (reduce) {
      values.GetValue(i).SetLength(0);
    }
  }
}
//.............................................................................
