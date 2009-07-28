#ifndef htmlextH
#define htmlextH

#include "estrlist.h"
#include "paramlist.h"
#include "actions.h"
#include "wininterface.h"
#include "wx/wxhtml.h"
#include "wx/gifdecod.h"
#include "wx/dynarray.h"

#include "library.h"


class THtmlFunc;
class THtmlLink;
class THtmlSwitch;
class THtmlImageCell;

class THtmlImageCell : public wxHtmlCell, public IEObject  {
public:
  THtmlImageCell(wxWindow *window,
                  wxFSFile *input, int w = wxDefaultCoord, int h = wxDefaultCoord,
                  double scale = 1.0, int align = wxHTML_ALIGN_BOTTOM,
                  const wxString& mapname = wxEmptyString, 
                  bool WidthInPercent = false, 
                  bool HeightInPercent = false);
  ~THtmlImageCell();
  void Draw(wxDC& dc, int x, int y, int view_y1, int view_y2,
            wxHtmlRenderingInfo& info);
  virtual wxHtmlLinkInfo *GetLink(int x = 0, int y = 0) const;
  wxScrolledWindow* GetWindow()  {  return m_window;  }
  void SetImage(const wxImage& img);

#if wxUSE_GIF && wxUSE_TIMER
  void AdvanceAnimation(wxTimer *timer);
  virtual void Layout(int w);
#endif
  void SetText(const wxString& text)  {  Text = text;  }
  const wxString& GetText() const     {  return Text;  }

  void SetSource(const olxstr& text) {  FSource = text;  }
  const olxstr& GetSource() const    {  return FSource;  }
private:
  wxBitmap           *m_bitmap;
  wxFSFile           *File;
  wxString           Text;
  olxstr           FSource;
  int                 m_bmpW, m_bmpH;
  bool                m_showFrame:1;
  bool WidthInPercent, HeightInPercent;
  wxScrolledWindow   *m_window;
#if wxUSE_GIF && wxUSE_TIMER
  wxGIFDecoder       *m_gifDecoder;
  wxTimer            *m_gifTimer;
  int                 m_physX, m_physY;
#endif
  double              m_scale;
  wxString            m_mapName;

  DECLARE_NO_COPY_CLASS(THtmlImageCell)
};

#if wxUSE_GIF && wxUSE_TIMER
class wxGIFTimer : public wxTimer  {
public:
  wxGIFTimer(THtmlImageCell *cell) : m_cell(cell) {}
  virtual void Notify()  {
    m_cell->AdvanceAnimation(this);
  }
  private:
    THtmlImageCell *m_cell;
    DECLARE_NO_COPY_CLASS(wxGIFTimer)
};
#endif
class THtmlWordCell : public wxHtmlWordCell  {
public:
  THtmlWordCell(const wxString& word, const wxDC& dc) :
    wxHtmlWordCell(word, dc)  {  }
  // just this extra function for managed alignment ...
  inline void SetDescent(int v) {  m_Descent = v;  }

};

class AHtmlObject: public IEObject  {
protected:
  class THtml *FParentHtml;
public:
  AHtmlObject(THtml *ParentHtml){  FParentHtml = ParentHtml; };
  virtual ~AHtmlObject();
  virtual void ToStrings(TStrList &List)=0;
};

class THtmlFunc: public AHtmlObject  {
protected:
  olxstr FFunc;
  THtmlSwitch *FParent;
public:
  THtmlFunc(THtml *ParentHtml, THtmlSwitch *ParentSwitch);
  virtual ~THtmlFunc();

  const olxstr&  Func() const {  return FFunc; }
  void Func(const olxstr& N) {  FFunc = N; }
  olxstr Evaluate();

  void ToStrings(TStrList &List);
};

class THtmlLink: public AHtmlObject {
protected:
  olxstr FFileName;
  THtmlSwitch *FParent;
protected:
public:
  THtmlLink(THtml *ParentHtml, THtmlSwitch *ParentSwitch);
  virtual ~THtmlLink();

  const olxstr&  FileName() {  return FFileName; }
  void FileName(const olxstr& N) {  FFileName = N; }

  void ToStrings(TStrList &List);
};

class THtmlSwitch: public AHtmlObject {
protected:
  olxstr FName;
  short FFileIndex;  // the file index
  TStrList  FFiles;
  TStrPObjList<olxstr,AHtmlObject*> FStrings;  // represents current content of the switch
  TParamList FParams;   // parameters to be replaced with their values param=ll use #param
  TTypeList<THtmlSwitch> FSwitches; // a list of subitems
  TTypeList<THtmlFunc> FFuncs; // a list of functions
  TTypeList<THtmlLink> FLinks; // a list of links
  THtmlSwitch *FParent;
protected:
  bool FUpdateSwitch;
public:
  THtmlSwitch(THtml *ParentHtml, THtmlSwitch *ParentSwitch);
  virtual ~THtmlSwitch();
  void Clear();

  const olxstr&  Name() const {  return FName; }
  void Name(const olxstr& N) {  FName = N; }
  inline short FileIndex() const {  return FFileIndex; }
  void  FileIndex(short ind);
  void UpdateFileIndex();
  inline int FileCount() const {  return FFiles.Count(); }
  const olxstr &File(int ind) const {  return FFiles[ind]; }
  void ClearFiles()  {  FFiles.Clear(); }
  void AddFile(const olxstr &FN){ FFiles.Add(FN); }
  const olxstr& CurrentFile() const {  return FFileIndex == -1 ? EmptyString : FFiles[FFileIndex];  }

  TStrPObjList<olxstr,AHtmlObject*>& Strings()  {  return FStrings; }
  inline int SwitchCount() const {  return FSwitches.Count(); }
  inline THtmlSwitch& Switch(int ind)  {  return FSwitches[ind]; }
  THtmlSwitch*  FindSwitch(const olxstr &IName);
  int FindSimilar(const olxstr& start, const olxstr& end, TPtrList<THtmlSwitch>& ret);
  void Expand(TPtrList<THtmlSwitch>& ret);
  THtmlSwitch& NewSwitch();

  void AddParam(const olxstr& name, const olxstr& value){  FParams.AddParam(name, value);  };
  void AddParam(const olxstr& nameEqVal)  {  FParams.FromString(nameEqVal, '=');  };
  inline TParamList& Params()  {  return FParams;  }

  THtmlFunc& NewFunc();
  inline int FuncCount() const {  return FFuncs.Count(); }
  inline THtmlFunc& Func(int ind){  return FFuncs[ind]; }

  THtmlLink& NewLink();
  inline int LinkCount() const {  return FLinks.Count(); }
  inline THtmlLink& Link(int ind)  {  return FLinks[ind]; }

  inline bool UpdateSwitch() const {  return FUpdateSwitch; };
  void UpdateSwitch(bool V){  FUpdateSwitch = V; };

  void ToStrings(TStrList &List);
  bool ToFile();
};

class THtml: public wxHtmlWindow, public IEObject  {
private:
  TActionQList *FActions;
  bool FMovable, FPageLoadRequested, ShowTooltips;
  int FLockPageLoad;
  olxstr FPageRequested;
protected:
  olxstr  FWebFolder, FFileName, HomePage;   // the base of all web files
  olxstr NormalFont, FixedFont;
  void OnLinkClicked(const wxHtmlLinkInfo& link);
  wxHtmlOpeningStatus OnOpeningURL(wxHtmlURLType type, const wxString& url, wxString *redirect) const;

  void OnMouseDblClick(wxMouseEvent& event);
  void OnMouseDown(wxMouseEvent& event);
  void OnMouseUp(wxMouseEvent& event);
  void OnMouseMotion(wxMouseEvent& event);
  void OnCellMouseHover(wxHtmlCell *Cell, wxCoord x, wxCoord y);
  void OnChar(wxKeyEvent& event);  
  /* on GTK scrolling makes mess out of the controls so will try to "fix it" here*/
  void OnScroll(wxScrollEvent& evt);
  virtual void ScrollWindow(int dx, int dy, const wxRect* rect = NULL);

  // position of where the mous was down
  int FMouseX, FMouseY;
  bool FMouseDown;

  THtmlSwitch* FRoot;
  TSStrObjList<olxstr,AnAssociation3<IEObject*, wxWindow*, bool>, true> FObjects;
  TSStrPObjList<olxstr,int, true> FSwitchStates;
  olxstr FocusedControl;
  class TObjectsState  {
    TSStrPObjList<olxstr,TSStrStrList<olxstr,false>*, true> Objects;
    THtml& html;
  public:
    TObjectsState(THtml& htm) : html(htm) { }
    ~TObjectsState();
    TSStrStrList<olxstr,false>* FindProperties(const olxstr& cname) {
      int ind = Objects.IndexOf(cname);
      return (ind == -1) ? NULL : Objects.GetObject(ind);
    }
    TSStrStrList<olxstr,false>* DefineControl(const olxstr& name, const std::type_info& type);
    void SaveState();
    void RestoreState();
    void SaveToFile(const olxstr& fn);
    bool LoadFromFile(const olxstr& fn);
  };
  TObjectsState ObjectsState;
protected:
  int GetSwitchState(const olxstr& switchName);
  void ClearSwitchStates();
  // library
  DefMacro(ItemState)
  DefMacro(UpdateHtml)
  DefMacro(HtmlHome)
  DefMacro(HtmlReload)
  DefMacro(HtmlLoad)
  DefMacro(HtmlDump)
  DefMacro(Tooltips)
  DefMacro(SetFonts)
  DefMacro(SetBorders)
  DefMacro(DefineControl)
  DefMacro(Hide)

  DefFunc(GetValue)
  DefFunc(GetData)
  DefFunc(GetLabel)
  DefFunc(GetImage)
  DefFunc(GetState)
  DefFunc(GetItems)
  DefFunc(SetValue)
  DefFunc(SetData)
  DefFunc(SetLabel)
  DefFunc(SetImage)
  DefFunc(SetItems)
  DefFunc(SetState)
  DefFunc(SetFG)
  DefFunc(SetBG)
  DefFunc(GetFontName)
  DefFunc(GetBorders)
  DefFunc(SetFocus)
  DefFunc(EndModal)
  DefFunc(ShowModal)

  DefFunc(SaveData)
  DefFunc(LoadData)
  DefFunc(GetItemState)
  DefFunc(IsItem)
  DefFunc(IsPopup)

  olxstr GetObjectValue(const IEObject *Object);
  const olxstr& GetObjectData(const IEObject *Object);
  bool GetObjectState(const IEObject *Object);
  olxstr GetObjectImage(const IEObject *Object);
  olxstr GetObjectItems(const IEObject *Object);
  void SetObjectValue(IEObject *Object, const olxstr& Value);
  void SetObjectData(IEObject *Object, const olxstr& Data);
  void SetObjectState(IEObject *Object, bool State);
  bool SetObjectImage(IEObject *Object, const olxstr& src);
  bool SetObjectItems(IEObject *Object, const olxstr& src);

  static TLibrary* Library;
public:
  THtml(wxWindow *Parent, ALibraryContainer* LC);
  virtual ~THtml();

  void SetSwitchState(THtmlSwitch& sw, int state);

  inline int GetBorders() const {  return wxHtmlWindow::m_Borders;  }
  void SetFonts(const olxstr& normal, const olxstr& fixed )  {
    this->NormalFont = normal;
    this->FixedFont = fixed;
    wxHtmlWindow::SetFonts( normal.u_str(), fixed.u_str() );
  }
  void GetFonts(olxstr& normal, olxstr& fixed)  {
    normal = this->NormalFont;
    fixed = this->FixedFont;
  }

  inline bool GetShowTooltips()  const {  return ShowTooltips;  }
  void SetShowTooltips(bool v, const olxstr& html_name=EmptyString);

  bool PageLoadRequested()  const  {  return FPageLoadRequested;  }
  inline void IncLockPageLoad()    {  FLockPageLoad++;  }
  inline void DecLockPageLoad()    {  FLockPageLoad--;  }
  inline bool IsPageLocked() const {  return FLockPageLoad != 0;  }
  
  bool ProcessPageLoadRequest();

  const olxstr& GetHomePage() const   {  return HomePage;  }
  void SetHomePage(const olxstr& hp)  {  HomePage = hp;  }

  bool LoadPage(const wxString &File);
  bool ReloadPage();
  bool UpdatePage();
  const olxstr& WebFolder() const {  return FWebFolder; }
  void WebFolder(const olxstr& path)  {  FWebFolder = path;  }

  void CheckForSwitches(THtmlSwitch &Sender, bool IsZip);
  void UpdateSwitchState(THtmlSwitch &Switch, olxstr &String);
  THtmlSwitch* Root(){  return FRoot; }
  bool ItemState(const olxstr &ItemName, short State);
  // object operations
  bool AddObject(const olxstr& Name, IEObject *Obj, wxWindow* wxObj, bool Manage = false);
  IEObject *FindObject(const olxstr& Name)  {
    int ind = FObjects.IndexOf(Name);
    return (ind == -1) ? NULL : FObjects.GetObject(ind).A();
  }
  wxWindow *FindObjectWindow(const olxstr& Name)  {
    int ind = FObjects.IndexOf(Name);
    return (ind == -1) ? NULL : FObjects.GetObject(ind).B();
  }
  inline int ObjectCount()          const {  return FObjects.Count();  }
  inline IEObject* GetObject(int i)       {  return FObjects.GetObject(i).A();  }
  inline wxWindow* GetWindow(int i)       {  return FObjects.GetObject(i).B();  }
  inline const olxstr& GetObjectName(int i) const {  return FObjects.GetString(i);  }
  inline bool IsObjectManageble(int i) const      {  return FObjects.GetObject(i).GetC();  }
  //
  inline bool Movable() const {  return FMovable;  }
  inline void Movable(bool v) {  FMovable = v;  }

  TActionQueue *OnURL;
  TActionQueue *OnLink;

  TActionQueue *OnDblClick;
  TActionQueue *OnKey;
  TActionQueue *OnCmd;

  TWindowInterface WI;
  DECLARE_EVENT_TABLE()
};
#endif
