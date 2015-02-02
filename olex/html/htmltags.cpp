/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "htmlmanager.h"
#include "imgcellext.h"
#include "widgetcellext.h"
#include "wx/html/htmlcell.h"
#include "wx/html/m_templ.h"
#include "wxzipfs.h"
#include "integration.h"
#include "utf8file.h"
#include "olxstate.h"

#ifdef _UNICODE
  #define _StrFormat_ wxT("%ls")
#else
  #define _StrFormat_ wxT("%s")
#endif

bool GetBoolAttribute(const wxHtmlTag &tag, const olxstr &attr) {
  if (tag.HasParam(attr.u_str())) {
    olxstr v = tag.GetParam(attr.u_str());
    if (v.IsBool())
      return v.ToBool();
    else
      return true;
  }
  return false;
}

void AdjustSize(wxWindow &w, bool height=true, bool width=false) {
  wxSize sz = w.GetSize(), bsz = w.GetBestSize();
  if (height) {
    if (width) {
      w.SetSize(bsz.x, bsz.y);
    }
    else {
      w.SetSize(sz.x, bsz.y);
    }
  }
  else if (width) {
    w.SetSize(bsz.x, sz.y);
  }
}

// helper tag
TAG_HANDLER_BEGIN(SWITCHINFOS, "SWITCHINFOS")
TAG_HANDLER_PROC(tag)  {
  THtml::SwitchSources().Push(THtml::SwitchSource());
  THtml::SwitchSource() = tag.GetParam(wxT("SRC"));
  return true;
}
TAG_HANDLER_END(SWITCHINFOS)
// helper tag
TAG_HANDLER_BEGIN(SWITCHINFOE, "SWITCHINFOE")
TAG_HANDLER_PROC(tag)
{
  try {
    THtml::SwitchSource() = THtml::SwitchSources().Pop();
  }
  catch (TExceptionBase &e) {
    TBasicApp::NewLogEntry(logException) << e;
  }
  return true;
}
TAG_HANDLER_END(SWITCHINFOE)
// Z-ordered image map tag
TAG_HANDLER_BEGIN(RECT, "ZRECT")
TAG_HANDLER_PROC(tag)  {
  if( tag.HasParam(wxT("COORDS")) )  {
    if( m_WParser->GetContainer()->GetLastChild() != NULL &&
      EsdlInstanceOf(*m_WParser->GetContainer()->GetLastChild(), THtmlImageCell) )
    {
      THtmlImageCell* ic =
        (THtmlImageCell*)m_WParser->GetContainer()->GetLastChild();
      TStrList toks(tag.GetParam(wxT("COORDS")), ',');
      if( toks.Count() == 4 )
        ic->AddRect(
          toks[0].ToInt(),
          toks[1].ToInt(),
          toks[2].ToInt(),
          toks[3].ToInt(),
          tag.GetParam(wxT("HREF")),
          tag.GetParam(wxT("TARGET"))
        );
    }
  }
  return false;
}
TAG_HANDLER_END(RECT)
// Z-ordered image tag
TAG_HANDLER_BEGIN(CIRCLE, "ZCIRCLE")
TAG_HANDLER_PROC(tag)  {
  if( tag.HasParam(wxT("COORDS")) )  {
    if( m_WParser->GetContainer()->GetLastChild() != NULL &&
      EsdlInstanceOf(*m_WParser->GetContainer()->GetLastChild(), THtmlImageCell) )
    {
      THtmlImageCell* ic =
        (THtmlImageCell*)m_WParser->GetContainer()->GetLastChild();
      TStrList toks(tag.GetParam(wxT("COORDS")), ',');
      if( toks.Count() == 3 )
        ic->AddCircle(
          toks[0].ToInt(),
          toks[1].ToInt(),
          toks[2].ToFloat(),
          tag.GetParam(wxT("HREF")),
          tag.GetParam(wxT("TARGET"))
        );
    }
  }
  return false;
}

TAG_HANDLER_END(CIRCLE)
// extended image tag
TAG_HANDLER_BEGIN(IMAGE, "ZIMG")
TAG_HANDLER_PROC(tag)  {
  int ax=-1, ay=-1;
  bool WidthInPercent = false, HeightInPercent = false;
  int fl = 0;
  olex2::IOlex2Processor *op = olex2::IOlex2Processor::GetInstance();
  wxString text = tag.GetParam(wxT("TEXT")),
           mapName = tag.GetParam(wxT("USEMAP"));
  olxstr ObjectName = tag.GetParam(wxT("NAME")),
    Tmp;
  try  {
    Tmp = tag.GetParam(wxT("WIDTH"));
    if( !Tmp.IsEmpty() )  {
      if( Tmp.EndsWith('%') )  {
        ax = (int)Tmp.SubStringTo(Tmp.Length()-1).ToDouble();
        WidthInPercent = true;
      }
      else
        ax = (int)Tmp.ToDouble();
    }
    Tmp = tag.GetParam(wxT("HEIGHT"));
    if( !Tmp.IsEmpty() )  {
      if( Tmp.EndsWith('%') )  {
        ay = (int)Tmp.SubStringTo(Tmp.Length()-1).ToDouble();
        HeightInPercent = true;
      }
      else
        ay = (int)Tmp.ToDouble();
    }
  }
  catch(const TExceptionBase& e)  {
    TBasicApp::NewLogEntry(logException) << e.GetException()->GetFullMessage();
    TBasicApp::NewLogEntry() << "While processing Width/Height for zimg::" <<
      ObjectName;
    TBasicApp::NewLogEntry() << "Offending input: '" << Tmp << '\'';
  }

  if (tag.HasParam(wxT("FLOAT"))) fl = ax;

  if( !text.IsEmpty() )  {
    int textW = 0, textH = 0;
    m_WParser->GetDC()->GetTextExtent( text, &textW, &textH );
    if( textW > ax )  ax = textW;
    if( textH > ay )  ay = textH;
  }

  olxstr src = tag.GetParam(wxT("SRC"));
  if (op != NULL)
    op->processFunction(src, EmptyString(), true);

  if (TZipWrapper::IsZipFile(THtml::SwitchSource()) && !TZipWrapper::IsZipFile(src))
    src = TZipWrapper::ComposeFileName(THtml::SwitchSource(), src);

  wxFSFile *fsFile = TFileHandlerManager::GetFSFileHandler( src );
  if( fsFile == NULL )
    TBasicApp::NewLogEntry(logError) << "Could not locate image: " << src;

  if( (mapName.Length() > 0) && mapName.GetChar(0) == '#')
      mapName = mapName.Mid( 1 );

  THtmlImageCell *cell = new THtmlImageCell(
    m_WParser->GetWindowInterface()->GetHTMLWindow(),
    fsFile,
    ax, ay,
    m_WParser->GetPixelScale(),
    wxHTML_ALIGN_BOTTOM,
    mapName,
    WidthInPercent,
    HeightInPercent);

  cell->SetText( text );
  cell->SetSource( src );
  cell->SetLink(m_WParser->GetLink());
  cell->SetId(tag.GetParam(wxT("id"))); // may be empty
  m_WParser->GetContainer()->InsertCell(cell);
  if( !ObjectName.IsEmpty() )  {
    THtml * html = dynamic_cast<THtml*>(
      m_WParser->GetWindowInterface()->GetHTMLWindow());
    if( !html->AddObject(ObjectName, cell, NULL) ) {
      TBasicApp::NewLogEntry(logError) << "THTML: object already exist: " <<
        ObjectName;
    }
  }
  return false;
}
TAG_HANDLER_END(IMAGE)
// helpe function
olxstr ExpandMacroShortcuts(const olxstr &s, const olxstr_dict<olxstr> &map) {
  olxstr rv = s;
  for( size_t i=0; i < map.Count(); i++ )
    rv.Replace(map.GetKey(i), map.GetValue(i));
  return rv;
}
// input tag
TAG_HANDLER_BEGIN(INPUT, "INPUT")
TAG_HANDLER_PROC(tag)  {
  const olxstr TagName = tag.GetParam(wxT("TYPE"));
  olex2::IOlex2Processor *op = olex2::IOlex2Processor::GetInstance();
  THtml * html = dynamic_cast<THtml*>(
    m_WParser->GetWindowInterface()->GetHTMLWindow());
  olxstr ObjectName, Value, Data, Tmp, Label;
  ObjectName = tag.GetParam(wxT("NAME"));
  const olxstr SrcInfo = olxstr(__OlxSrcInfo) << " for input '" << TagName <<
    '[' << ObjectName << "]'";
  int valign = -1, halign = -1,
    fl=0,
    ax=100, ay=20;
  bool width_set = false, height_set = false;
  AOlxCtrl* CreatedObject = NULL;
  wxWindow* CreatedWindow = NULL;
  olxstr_dict<olxstr> macro_map;
  // init the shortcuts
  {
    olxstr pn = html->GetPopupName();
    macro_map.Add("~name~", pn.IsEmpty() ? ObjectName
      : (pn << '.' << ObjectName));
  }
  try  {
    Tmp = tag.GetParam(wxT("WIDTH"));
    op->processFunction(Tmp, SrcInfo, false);
    if( !Tmp.IsEmpty() )  {
      if( Tmp.EndsWith('%') )
        fl = Tmp.SubStringTo(Tmp.Length()-1).ToFloat();
      else
        ax = (int)Tmp.ToDouble();
      width_set = true;
    }
    Tmp = tag.GetParam(wxT("HEIGHT"));
    op->processFunction(Tmp, SrcInfo, false);
    if( !Tmp.IsEmpty() )  {
      if( Tmp.EndsWith('%') )  {
        ay = 0;
        float _ay = Tmp.SubStringTo(Tmp.Length()-1).ToFloat()/100;
        _ay *= html->GetSize().GetHeight();
        ay = (int)_ay;
      }
      else
        ay = Tmp.ToDouble();
      height_set = true;
    }
  }
  catch(const TExceptionBase& e)  {
    TBasicApp::NewLogEntry(logException) << e.GetException()->GetFullMessage();
    TBasicApp::NewLogEntry() << "While processing Width/Height HTML tags for "
      << TagName << "::" << ObjectName;
    TBasicApp::NewLogEntry() << "Offending input: '" << Tmp << '\'';
  }
  if( ax == 0 )  ax = 30;
  if( ay == 0 )  ay = 20;

  if( tag.HasParam(wxT("FLOAT")) )
    fl = ax;

  {  // parse h alignment
    wxString ha;
    if( tag.HasParam(wxT("ALIGN")) )
      ha = tag.GetParam(wxT("ALIGN"));
    else if( tag.HasParam(wxT("HALIGN")) )
      ha = tag.GetParam(wxT("HALIGN"));
    if( !ha.IsEmpty() )  {
      if( ha.CmpNoCase(wxT("left")) == 0 )
        halign = wxHTML_ALIGN_LEFT;
      else if( ha.CmpNoCase(wxT("center")) == 0 ||
               ha.CmpNoCase(wxT("middle")) == 0 )
        halign = wxHTML_ALIGN_CENTER;
      else if( ha.CmpNoCase(wxT("right")) == 0 )
        halign = wxHTML_ALIGN_RIGHT;
    }
  }
  if( tag.HasParam(wxT("VALIGN")) ){
    wxString va = tag.GetParam(wxT("VALIGN"));
    if( va.CmpNoCase(wxT("top")) == 0 )
      valign = wxHTML_ALIGN_TOP;
    else if( va.CmpNoCase(wxT("center")) == 0 ||
             va.CmpNoCase(wxT("middle")) == 0 )
      valign = wxHTML_ALIGN_CENTER;
    else if( va.CmpNoCase(wxT("bottom")) == 0 )
      valign = wxHTML_ALIGN_BOTTOM;
  }
  Label = tag.GetParam(wxT("LABEL"));

  wxHtmlLinkInfo* LinkInfo = NULL;
  if( !Label.IsEmpty() )  {
    if( Label.StartsFromi("href=") )  {
      Label = Label.SubStringFrom(5);
      const size_t tagInd = Label.IndexOfi("&target=");
      olxstr tag(EmptyString());
      if( tagInd != InvalidIndex )  {
        tag = Label.SubStringFrom(tagInd+8);
        Label.SetLength(tagInd);
      }
      LinkInfo = new wxHtmlLinkInfo(Label.u_str(), tag.u_str() );
    }
  }
  if( !ObjectName.IsEmpty() )  {
    wxWindow* wnd = html->FindObjectWindow(ObjectName);
    if( wnd != NULL )  {
      if( !tag.HasParam(wxT("reuse")) ) {
        TBasicApp::NewLogEntry(logError) << "HTML: duplicated object \'" <<
          ObjectName << '\'';
      }
      else  {
        if( !Label.IsEmpty() )  {
          wxHtmlContainerCell* contC =
            new wxHtmlContainerCell(m_WParser->GetContainer());
          THtml::WordCell* wc =
            new THtml::WordCell(Label.u_str(), *m_WParser->GetDC());
          if( LinkInfo != NULL ) {
            wc->SetLink(*LinkInfo);
            delete LinkInfo;
          }
          wc->SetDescent(0);
          contC->InsertCell( wc );
          contC->InsertCell(new THtmlWidgetCell(wnd, fl));
          if( valign != -1 )  contC->SetAlignVer(valign);
          if( halign != -1 )  contC->SetAlignHor(halign);
        }
        else
          m_WParser->GetContainer()->InsertCell(new THtmlWidgetCell(wnd, fl));
      }
      return false;
    }
  }

  Value = ExpandMacroShortcuts(tag.GetParam(wxT("VALUE")), macro_map);
  op->processFunction(Value, SrcInfo, true);
  Data = tag.GetParam(wxT("DATA"));
/******************* TEXT CONTROL *********************************************/
  if (TagName.Equalsi("text")) {
    int flags = wxWANTS_CHARS;
    if (GetBoolAttribute(tag, "MULTILINE"))
      flags |= wxTE_MULTILINE;
    else
      flags = wxTE_PROCESS_ENTER;
    if (GetBoolAttribute(tag, "PASSWORD"))
      flags |= wxTE_PASSWORD;
    if (GetBoolAttribute(tag, "READONLY"))
      flags |= wxTE_READONLY;
    TTextEdit *Text = new TTextEdit(html, flags);
    Text->SetFont(m_WParser->GetDC()->GetFont());
    CreatedObject = Text;
    CreatedWindow = Text;
    Text->SetText(Value);
    Text->SetSize(ax, ay);
    if ((flags&wxTE_MULTILINE) == 0)
      AdjustSize(*Text);
    Text->SetData(Data);

    if (!Label.IsEmpty()) {
      wxHtmlContainerCell* contC =
        new wxHtmlContainerCell(m_WParser->GetContainer());
      THtml::WordCell* wc =
        new THtml::WordCell(Label.u_str(), *m_WParser->GetDC());
      if (LinkInfo != NULL) wc->SetLink(*LinkInfo);
      wc->SetDescent(0);
      contC->InsertCell(wc);
      contC->InsertCell(new THtmlWidgetCell(Text, fl));
      if (valign != -1)  contC->SetAlignVer(valign);
      if (halign != -1)  contC->SetAlignHor(halign);
    }
    else
      m_WParser->GetContainer()->InsertCell(new THtmlWidgetCell(Text, fl));

    if (tag.HasParam(wxT("ONCHANGE"))) {
      Text->OnChange.data =
        ExpandMacroShortcuts(tag.GetParam(wxT("ONCHANGE")), macro_map);
      Text->OnChange.Add(&html->Manager);
    }
    if (tag.HasParam(wxT("ONLEAVE"))) {
      Text->OnLeave.data =
        ExpandMacroShortcuts(tag.GetParam(wxT("ONLEAVE")), macro_map);
      Text->OnLeave.Add(&html->Manager);
    }
    if (tag.HasParam(wxT("ONENTER"))) {
      Text->OnEnter.data =
        ExpandMacroShortcuts(tag.GetParam(wxT("ONENTER")), macro_map);
      Text->OnEnter.Add(&html->Manager);
    }
    if (tag.HasParam(wxT("ONRETURN"))) {
      Text->OnReturn.data =
        ExpandMacroShortcuts(tag.GetParam(wxT("ONRETURN")), macro_map);
      Text->OnReturn.Add(&html->Manager);
    }
  }
/******************* DATE CONTROL *****************************************/
  if( TagName.Equalsi("date") )  {
    int flags = wxDP_SPIN;
    if (GetBoolAttribute(tag, "dropdown")) {
      flags = wxDP_DROPDOWN;
    }
    TDateCtrl *DT = new TDateCtrl(html, flags);
    DT->SetFont(m_WParser->GetDC()->GetFont());
    CreatedObject = DT;
    CreatedWindow = DT;
    DT->SetSize(ax, ay);
    DT->SetData(Data);
    if (!Value.IsEmpty()) {
      wxDateTime dt;
      dt.ParseDate(Value.u_str());
      if (dt.IsValid())
        DT->SetValue(dt);
      else {
        TBasicApp::NewLogEntry(logError) << (
          olxstr("Invalid format for date and time: ").quote() << Value);
      }
    }
    if (!Label.IsEmpty()) {
      wxHtmlContainerCell* contC =
        new wxHtmlContainerCell(m_WParser->GetContainer());
      THtml::WordCell* wc =
        new THtml::WordCell(Label.u_str(), *m_WParser->GetDC());
      if (LinkInfo != NULL)
        wc->SetLink(*LinkInfo);
      wc->SetDescent(0);
      contC->InsertCell(wc);
      contC->InsertCell(new THtmlWidgetCell(DT, fl));
      if (valign != -1)  contC->SetAlignVer(valign);
      if (halign != -1)  contC->SetAlignHor(halign);
    }
    else {
      m_WParser->GetContainer()->InsertCell(new THtmlWidgetCell(DT, fl));
    }

    if (tag.HasParam(wxT("ONCHANGE"))) {
      DT->OnChange.data =
        ExpandMacroShortcuts(tag.GetParam(wxT("ONCHANGE")), macro_map);
      DT->OnChange.Add(&html->Manager);
    }
  }
/******************* COLOR CONTROL *******************************************/
  if( TagName.Equalsi("color") )  {
    TColorCtrl *CC = new TColorCtrl(html);
    CC->SetFont(m_WParser->GetDC()->GetFont());
    CreatedObject = CC;
    CreatedWindow = CC;
    CC->SetSize(ax, ay);
    CC->SetData(Data);
    CC->SetValue(wxColor(Value.u_str()));
    if( !Label.IsEmpty() )  {
      wxHtmlContainerCell* contC =
        new wxHtmlContainerCell(m_WParser->GetContainer());
      THtml::WordCell* wc =
        new THtml::WordCell(Label.u_str(), *m_WParser->GetDC());
      if( LinkInfo != NULL ) wc->SetLink(*LinkInfo);
      wc->SetDescent(0);
      contC->InsertCell( wc );
      contC->InsertCell(new THtmlWidgetCell(CC, fl));
      if( valign != -1 )  contC->SetAlignVer(valign);
      if( halign != -1 )  contC->SetAlignHor(halign);
    }
    else
      m_WParser->GetContainer()->InsertCell(new THtmlWidgetCell(CC, fl));

    if( tag.HasParam(wxT("ONCHANGE")) )  {
      CC->OnChange.data =
        ExpandMacroShortcuts(tag.GetParam(wxT("ONCHANGE")), macro_map);
      CC->OnChange.Add(&html->Manager);
    }
  }
/******************* LABEL ***************************************************/
  else if( TagName.Equalsi("label") )  {
    TLabel *Text = new TLabel(html, Value);
    Text->SetFont(m_WParser->GetDC()->GetFont());
    CreatedObject = Text;
    CreatedWindow = Text;
    Text->WI.SetWidth(ax);
    Text->WI.SetHeight(ay);
    Text->SetData(Data);
    m_WParser->GetContainer()->InsertCell(new THtmlWidgetCell(Text, fl));
  }
/******************* BUTTON ***************************************************/
  else if (TagName.Equalsi("button")) {
    AButtonBase *Btn;
    long flags = 0;
    if (GetBoolAttribute(tag, "FIT"))  flags |= wxBU_EXACTFIT;
    if (GetBoolAttribute(tag, "FLAT"))  flags |= wxNO_BORDER;
    olxstr buttonImage = tag.GetParam(wxT("IMAGE"));
    if (!buttonImage.IsEmpty()) {
      if (buttonImage.IndexOf(',') != InvalidIndex) {
        TImgButton* ibtn = new TImgButton(html);
        ibtn->SetImages(buttonImage, width_set ? ax : -1, height_set ? ay : -1);
        if( tag.HasParam(wxT("ENABLED")) )
          ibtn->SetEnabled(olxstr(tag.GetParam(wxT("ENABLED"))).ToBool());
        if( tag.HasParam(wxT("DOWN")) )
          ibtn->SetDown(olxstr(tag.GetParam(wxT("DOWN"))).ToBool());
        CreatedWindow = ibtn;
        Btn = ibtn;
      }
      else {
        Btn = new TBmpButton(html, -1, wxNullBitmap,
          wxDefaultPosition, wxDefaultSize, flags );
        ((TBmpButton*)Btn)->SetSource( buttonImage );
        wxFSFile *fsFile = TFileHandlerManager::GetFSFileHandler(buttonImage);
        if (fsFile == NULL) {
          TBasicApp::NewLogEntry(logError) <<
            "THTML: could not locate image for button: " << ObjectName;
        }
        else {
          wxImage image(*(fsFile->GetStream()), wxBITMAP_TYPE_ANY);
          if (!image.Ok()) {
            TBasicApp::NewLogEntry(logError) <<
              "THTML: could not load image for button: " << ObjectName;
          }
          else {
            if ((image.GetWidth() > ax || image.GetHeight() > ay) &&
              GetBoolAttribute(tag, "STRETCH"))
            {
              image = image.Scale(ax, ay);
            }
            else  {
              ax = image.GetWidth();
              ay = image.GetHeight();
            }
            ((TBmpButton*)Btn)->SetBitmapLabel(image);
          }
          delete fsFile;
        }
        if ((flags & wxBU_EXACTFIT) == 0) {
          Btn->WI.SetWidth(ax);
          Btn->WI.SetHeight(ay);
        }
        else {
          ((TBmpButton*)Btn)->Fit();
        }
        ((TBmpButton*)Btn)->SetFont(m_WParser->GetDC()->GetFont());
        CreatedWindow = (TBmpButton*)Btn;
      }
    }
    else {
      Btn = new TButton(html, -1, Value.u_str(),
        wxDefaultPosition, wxDefaultSize, flags);
      ((TButton*)Btn)->SetFont(m_WParser->GetDC()->GetFont());
      if ((flags & wxBU_EXACTFIT) == 0) {
        Btn->WI.SetWidth(ax);
        Btn->WI.SetHeight(ay);
      }
      else {
        ((TButton*)Btn)->Fit();
      }
#ifdef __WXGTK__  // got no idea what happens here, client size does not work?
      wxFont fnt(m_WParser->GetDC()->GetFont());
      fnt.SetPointSize( fnt.GetPointSize()-2);
      ((TButton*)Btn)->SetFont(fnt);
      wxCoord w=0, h=0, desc=0, exlead=0;
      wxString wxs(Value.u_str());
      m_WParser->GetDC()->GetTextExtent(wxs, &w, &h, &desc, &exlead, &fnt);
      int borderx = 12, bordery = 8;
      ((TButton*)Btn)->SetClientSize(w+borderx,h+desc+bordery);
#endif
      CreatedWindow = (TButton*)Btn;
    }
    CreatedObject = Btn;
    Btn->SetData(Data);
    if (tag.HasParam(wxT("ONCLICK"))) {
      Btn->OnClick.data =
        ExpandMacroShortcuts(tag.GetParam(wxT("ONCLICK")), macro_map);
      Btn->OnClick.Add(&html->Manager);
    }
    if (tag.HasParam(wxT("ONDOWN"))) {
      Btn->OnUp.data = ExpandMacroShortcuts(tag.GetParam(wxT("ONUP")), macro_map);
      Btn->OnUp.Add(&html->Manager);
    }
    if (tag.HasParam(wxT("ONDOWN"))) {
      Btn->OnDown.data =
        ExpandMacroShortcuts(tag.GetParam(wxT("ONDOWN")),macro_map);
      Btn->OnDown.Add(&html->Manager);
    }
    if (html->GetShowTooltips()) {
      Btn->SetHint(
        ExpandMacroShortcuts(tag.GetParam(wxT("HINT")),macro_map));
    }
    if (tag.HasParam(wxT("DOWN")))
      Btn->SetDown(tag.GetParam(wxT("DOWN")).CmpNoCase(wxT("true")) == 0);

    olxstr modeDependent = tag.GetParam(wxT("MODEDEPENDENT"));
    if (!modeDependent.IsEmpty())
      Btn->SetActionQueue(TModeRegistry::GetInstance().OnChange, modeDependent);
    m_WParser->GetContainer()->InsertCell(new THtmlWidgetCell(CreatedWindow, fl));
  }
/******************* COMBOBOX *************************************************/
  else if (TagName.Equalsi("combo")) {
    if (GetBoolAttribute(tag, "READONLY")) {
      TChoice *Box = new TChoice(html, wxSize(ax, ay));
      Box->SetFont(m_WParser->GetDC()->GetFont());
      AdjustSize(*Box);
      CreatedObject = Box;
      CreatedWindow = Box;
      if (tag.HasParam(wxT("ITEMS"))) {
        olxstr Items = tag.GetParam(wxT("ITEMS"));
        op->processFunction(Items, SrcInfo, true);
        Box->AddItems(TStrList(Items, ';'));
      }
      else {
        Box->AddObject(Value);
      }
      Box->SetHasDefault(GetBoolAttribute(tag, "SETDEFAULT"));
      Box->SetText(Value);
      if (Box->GetSelection() == -1 && Box->HasDefault() &&
        !Box->IsEmpty())
      {
        Box->SetSelection(0);
      }
      Box->SetData(Data);
      if (tag.HasParam(wxT("ONCHANGE"))) {
        Box->OnChange.data =
          ExpandMacroShortcuts(tag.GetParam(wxT("ONCHANGE")), macro_map);
        Box->OnChange.Add(&html->Manager);
        Box->SetOnChangeAlways(GetBoolAttribute(tag, "ONCHANGEALWAYS"));
      }
      if (tag.HasParam(wxT("ONLEAVE"))) {
        Box->OnLeave.data =
          ExpandMacroShortcuts(tag.GetParam(wxT("ONLEAVE")), macro_map);
        Box->OnLeave.Add(&html->Manager);
      }
      if (tag.HasParam(wxT("ONENTER"))) {
        Box->OnEnter.data =
          ExpandMacroShortcuts(tag.GetParam(wxT("ONENTER")), macro_map);
        Box->OnEnter.Add(&html->Manager);
      }
      if (!Label.IsEmpty()) {
        wxHtmlContainerCell* contC =
          new wxHtmlContainerCell(m_WParser->GetContainer());
        THtml::WordCell* wc =
          new THtml::WordCell(Label.u_str(), *m_WParser->GetDC());
        if (LinkInfo != NULL) wc->SetLink(*LinkInfo);
        wc->SetDescent(0);
        contC->InsertCell(wc);
        contC->InsertCell(new THtmlWidgetCell(Box, fl));
        if (valign != -1) contC->SetAlignVer(valign);
        if (halign != -1) contC->SetAlignHor(halign);
      }
      else
        m_WParser->GetContainer()->InsertCell(new THtmlWidgetCell(Box, fl));
    }
    else {
      TComboBox *Box = new TComboBox(html, false, wxSize(ax, ay));
      Box->SetFont(m_WParser->GetDC()->GetFont());
      AdjustSize(*Box);
      CreatedObject = Box;
      CreatedWindow = Box;
      if (tag.HasParam(wxT("ITEMS"))) {
        olxstr Items = tag.GetParam(wxT("ITEMS"));
        op->processFunction(Items, SrcInfo, true);
        TStrList SL(Items, ';');
        Box->AddItems(SL);
      }
      else {
        Box->AddObject(Value);
      }
      Box->SetText(Value);
      Box->SetData(Data);
      if (tag.HasParam(wxT("ONCHANGE"))) {
        Box->OnChange.data =
          ExpandMacroShortcuts(tag.GetParam(wxT("ONCHANGE")), macro_map);
        Box->OnChange.Add(&html->Manager);
        Box->SetOnChangeAlways(GetBoolAttribute(tag, "ONCHANGEALWAYS"));
      }
      if (tag.HasParam(wxT("ONLEAVE"))) {
        Box->OnLeave.data =
          ExpandMacroShortcuts(tag.GetParam(wxT("ONLEAVE")), macro_map);
        Box->OnLeave.Add(&html->Manager);
      }
      if (tag.HasParam(wxT("ONENTER"))) {
        Box->OnEnter.data =
          ExpandMacroShortcuts(tag.GetParam(wxT("ONENTER")), macro_map);
        Box->OnEnter.Add(&html->Manager);
      }
      if (tag.HasParam(wxT("ONRETURN"))) {
        Box->OnReturn.data =
          ExpandMacroShortcuts(tag.GetParam(wxT("ONRETURN")), macro_map);
        Box->OnReturn.Add(&html->Manager);
      }
      if (!Label.IsEmpty()) {
        wxHtmlContainerCell* contC =
          new wxHtmlContainerCell(m_WParser->GetContainer());
        THtml::WordCell* wc =
          new THtml::WordCell(Label.u_str(), *m_WParser->GetDC());
        if (LinkInfo != NULL) wc->SetLink(*LinkInfo);
        wc->SetDescent(0);
        contC->InsertCell(wc);
        contC->InsertCell(new THtmlWidgetCell(Box, fl));
        if (valign != -1) contC->SetAlignVer(valign);
        if (halign != -1) contC->SetAlignHor(halign);
      }
      else
        m_WParser->GetContainer()->InsertCell(new THtmlWidgetCell(Box, fl));
    }
  }
/******************* SPIN CONTROL *********************************************/
  else if (TagName.Equalsi("spin")) {
    TSpinCtrl *Spin = new TSpinCtrl(html);
    Spin->SetFont(m_WParser->GetDC()->GetFont());
    Spin->WI.SetHeight(ay);
    Spin->WI.SetWidth(ax);
    AdjustSize(*Spin);
    Spin->SetForegroundColour(m_WParser->GetDC()->GetTextForeground());
    int min=0, max = 100;
    if (tag.HasParam( wxT("MIN")))
      tag.ScanParam(wxT("MIN"), wxT("%i"), &min);
    if (tag.HasParam( wxT("MAX")))
      tag.ScanParam(wxT("MAX"), wxT("%i"), &max);
    Spin->SetRange(min, max);
    try  { Spin->SetValue((int)Value.ToDouble()); }
    catch(...) {
      TBasicApp::NewLogEntry() << "Invalid value for spin control: \'" <<
        Value << '\'';
    }
    CreatedObject = Spin;
    CreatedWindow = Spin;
    Spin->SetData(Data);
    if (tag.HasParam(wxT("ONCHANGE"))) {
      Spin->OnChange.data =
        ExpandMacroShortcuts(tag.GetParam(wxT("ONCHANGE")), macro_map);
      Spin->OnChange.Add(&html->Manager);
    }
    if (!Label.IsEmpty()) {
      wxHtmlContainerCell* contC =
        new wxHtmlContainerCell(m_WParser->GetContainer());
      THtml::WordCell* wc =
        new THtml::WordCell(Label.u_str(), *m_WParser->GetDC());
      if( LinkInfo != NULL ) wc->SetLink(*LinkInfo);
      wc->SetDescent(0);
      contC->InsertCell(wc);
      contC->InsertCell(new THtmlWidgetCell(Spin, fl));
      if (valign != -1) contC->SetAlignVer(valign);
      if (halign != -1) contC->SetAlignHor(halign);
    }
    else
      m_WParser->GetContainer()->InsertCell(new THtmlWidgetCell(Spin, fl));
  }
/******************* SLIDER ***************************************************/
  else if (TagName.Equalsi("slider")) {
    TTrackBar *Track = new TTrackBar(html);
    Track->SetFont(m_WParser->GetDC()->GetFont());
    Track->WI.SetWidth(ax);
    Track->WI.SetHeight(ay);
#ifdef __MAC__
    AdjustSize(*Track);
#endif
    CreatedObject = Track;
    CreatedWindow = Track;
    int min=0, max = 100;
    if (tag.HasParam( wxT("MIN")))
      tag.ScanParam(wxT("MIN"), wxT("%i"), &min);
    if (tag.HasParam( wxT("MAX")))
      tag.ScanParam(wxT("MAX"), wxT("%i"), &max);
    if (min < max)
      Track->SetRange(min, max);
    try { Track->SetValue((int)Value.ToDouble()); }
    catch(...) {
      TBasicApp::NewLogEntry() << "Invalid value slider: \'" << Value << '\'';
    }
    Track->SetData(Data);
    if (tag.HasParam(wxT("ONCHANGE"))) {
      Track->OnChange.data =
        ExpandMacroShortcuts(tag.GetParam(wxT("ONCHANGE")), macro_map);
      Track->OnChange.Add(&html->Manager);
    }
    if (tag.HasParam(wxT("ONMOUSEUP"))) {
      Track->OnMouseUp.data =
        ExpandMacroShortcuts(tag.GetParam(wxT("ONMOUSEUP")), macro_map);
      Track->OnMouseUp.Add(&html->Manager);
    }
    if (!Label.IsEmpty()) {
      wxHtmlContainerCell* contC =
        new wxHtmlContainerCell(m_WParser->GetContainer());
      THtml::WordCell* wc =
        new THtml::WordCell(Label.u_str(), *m_WParser->GetDC());
      if( LinkInfo != NULL ) wc->SetLink(*LinkInfo);
      wc->SetDescent(0);
      contC->InsertCell(wc);
      contC->InsertCell(new THtmlWidgetCell(Track, fl));
      if (valign != -1) contC->SetAlignVer(valign);
      if (halign != -1) contC->SetAlignHor(halign);
    }
    else
      m_WParser->GetContainer()->InsertCell(new THtmlWidgetCell(Track, fl));
  }
/******************* CHECKBOX *************************************************/
  else if (TagName.Equalsi("checkbox")) {
    bool label_to_the_right = GetBoolAttribute(tag, "RIGHT");
    TCheckBox *Box = new TCheckBox(html,
      (label_to_the_right ? wxALIGN_RIGHT : 0));
    Box->SetFont(m_WParser->GetDC()->GetFont());
    wxLayoutConstraints* wxa = new wxLayoutConstraints;
    wxa->centreX.Absolute(0);
    Box->SetConstraints(wxa);
    Box->WI.SetWidth(ax);
    Box->WI.SetHeight(ay);
    Box->SetCaption(Value);
    AdjustSize(*Box, true, true);
    CreatedObject = Box;
    CreatedWindow = Box;
    if (tag.HasParam(wxT("CHECKED"))) {
      Tmp = tag.GetParam(wxT("CHECKED"));
      op->processFunction(Tmp, SrcInfo, false);
      if (Tmp.IsEmpty())
        Box->SetChecked(true);
      else if (Tmp.IsBool())
        Box->SetChecked(Tmp.ToBool());
      else {
        TBasicApp::NewLogEntry(logError) <<
          (olxstr("Invalid value for boolean: ").quote() << Tmp);
      }
    }
    Box->SetData(Data);
    // binding events
    if (tag.HasParam(wxT("ONCLICK"))) {
      Box->OnClick.data =
        ExpandMacroShortcuts(tag.GetParam(wxT("ONCLICK")), macro_map);
      Box->OnClick.Add(&html->Manager);
    }
    if (tag.HasParam(wxT("ONCHECK"))) {
      Box->OnCheck.data =
        ExpandMacroShortcuts(tag.GetParam(wxT("ONCHECK")), macro_map);
      Box->OnCheck.Add(&html->Manager);
    }
    if (tag.HasParam(wxT("ONUNCHECK"))) {
      Box->OnUncheck.data =
        ExpandMacroShortcuts(tag.GetParam(wxT("ONUNCHECK")), macro_map);
      Box->OnUncheck.Add(&html->Manager);
    }
    if (tag.HasParam(wxT("MODEDEPENDENT"))) {
      Box->SetActionQueue(TModeRegistry::GetInstance().OnChange,
        tag.GetParam(wxT("MODEDEPENDENT")));
    }
    m_WParser->GetContainer()->InsertCell(new THtmlWidgetCell(Box, fl));
  }
/******************* TREE CONTROL *********************************************/
  else if( TagName.Equalsi("tree") )  {
    long flags = wxTR_HAS_BUTTONS;
    if( tag.HasParam(wxT("NOROOT")) )
      flags |= wxTR_HIDE_ROOT;
    if( tag.HasParam(wxT("EDITABLE")) )
      flags |= wxTR_EDIT_LABELS;

    TTreeView *Tree = new TTreeView(html, flags);
    if( (flags&wxTR_HIDE_ROOT) == 0 && tag.HasParam(wxT("ROOTLABEL")) )
      Tree->SetItemText(Tree->GetRootItem(), tag.GetParam(wxT("ROOTLABEL")));
    olxstr src = tag.GetParam(wxT("SRC"));
    op->processFunction(src, SrcInfo, true);
    IInputStream* ios = TFileHandlerManager::GetInputStream(src);
    Tree->SetFont(m_WParser->GetDC()->GetFont());
    CreatedObject = Tree;
    CreatedWindow = Tree;
    Tree->WI.SetWidth(ax);
    Tree->WI.SetHeight(ay);
    wxMenu* menu = new wxMenu;
    menu->Append(1000, wxT("Expand all"));
    menu->Append(1001, wxT("Collapse all"));
    Tree->SetPopup(menu);
    Tree->SetData( Data );
    if( tag.HasParam(wxT("ONSELECT")) )  {
      Tree->OnSelect.data =
        ExpandMacroShortcuts(tag.GetParam(wxT("ONSELECT")), macro_map);
      Tree->OnSelect.Add(&html->Manager);
    }
    if( tag.HasParam(wxT("ONITEM")) )  {
      Tree->OnDblClick.data =
        ExpandMacroShortcuts(tag.GetParam(wxT("ONITEM")), macro_map);
      Tree->OnDblClick.Add(&html->Manager);
    }
    if( (flags&wxTR_EDIT_LABELS) != 0 && tag.HasParam(wxT("ONEDIT")) )  {
      Tree->OnEdit.data =
        ExpandMacroShortcuts(tag.GetParam(wxT("ONEDIT")), macro_map);
      Tree->OnEdit.Add(&html->Manager);
    }
    m_WParser->GetContainer()->InsertCell(new THtmlWidgetCell(Tree, fl));
    if( ios == NULL )  {  // create test tree
      TBasicApp::NewLogEntry(logError) <<
        "THTML: could not locate tree source: \'" << src <<  '\'';
      wxTreeItemId Root = Tree->AddRoot( wxT("Test data") );
      wxTreeItemId sc1 = Tree->AppendItem(
        Tree->AppendItem(Root, wxT("child")), wxT("subchild"));
        Tree->AppendItem(Tree->AppendItem(sc1, wxT("child1")), wxT("subchild1"));
      wxTreeItemId sc2 = Tree->AppendItem(
        Tree->AppendItem(Root, wxT("child1")), wxT("subchild1"));
        Tree->AppendItem(Tree->AppendItem(sc2, wxT("child1")), wxT("subchild1"));
    }
    else  {
      TStrList list;
#ifdef _UNICODE
      list = TUtf8File::ReadLines(*ios, false);
#else
      list.LoadFromTextStream(*ios);
#endif
      Tree->LoadFromStrings(list);
      delete ios;
      if (tag.HasParam(wxT("EXPANDED"))) {
        Tree->ExpandAll();
      }
      wxString sel = tag.GetParam(wxT("SELECTED"));
      if( sel.IsEmpty() )  {
        sel = tag.GetParam(wxT("VALUE"));
        if( !sel.IsEmpty() )
          Tree->SelectByLabel(sel);
      }
      else
          Tree->SelectByData(sel);
    }
  }
/******************* LIST CONTROL *********************************************/
  else if (TagName.Equalsi("list")) {
    bool srcTag = tag.HasParam(wxT("SRC")),
      itemsTag = tag.HasParam(wxT("ITEMS"));
    TStrList itemsList;
    if (srcTag && itemsTag) {
      TBasicApp::NewLogEntry(logError) <<
        "THTML: list can have only src or items";
    }
    else if (srcTag) {
      olxstr src = tag.GetParam(wxT("SRC"));
      op->processFunction(src, SrcInfo, true);
      IInputStream* ios = TFileHandlerManager::GetInputStream(src);
      if (ios == NULL) {
        TBasicApp::NewLogEntry(logError) <<
          "THTML: could not locate list source: \'" << src <<  '\'';
      }
      else {
#ifdef _UNICODE
        itemsList = TUtf8File::ReadLines(*ios, false);
#else
        itemsList.LoadFromTextStream(*ios);
#endif
        delete ios;
      }
    }
    else if (itemsTag) {
      olxstr items = tag.GetParam(wxT("ITEMS"));
      op->processFunction(items, SrcInfo, true);
      itemsList.Strtok(items, ';');
    }
    TListBox *List = new TListBox(html);
    List->SetFont(m_WParser->GetDC()->GetFont());
    CreatedObject = List;
    CreatedWindow = List;
    List->WI.SetWidth(ax);
    List->WI.SetHeight(ay);
    List->SetData(Data);
    List->AddItems(itemsList);
    // binding events
    if (tag.HasParam(wxT("ONSELECT"))) {
      List->OnSelect.data =
        ExpandMacroShortcuts(tag.GetParam(wxT("ONSELECT")), macro_map);
      List->OnSelect.Add(&html->Manager);
    }
    if (tag.HasParam(wxT("ONDBLCLICK"))) {
      List->OnDblClick.data =
        ExpandMacroShortcuts(tag.GetParam(wxT("ONDBLCLICK")), macro_map);
      List->OnDblClick.Add(&html->Manager);
    }
    // creating cell
    m_WParser->GetContainer()->InsertCell(new THtmlWidgetCell(List, fl));
  }
/******************* END OF CONTROLS ******************************************/
  if( LinkInfo != NULL )  delete LinkInfo;
  if( ObjectName.IsEmpty() )  {  }  // create random name?
  if( CreatedWindow != NULL )  {  // set default colors
    if( m_WParser->GetActualColor().IsOk() )
      CreatedWindow->SetForegroundColour(m_WParser->GetActualColor());
    if( m_WParser->GetContainer()->GetBackgroundColour().IsOk() ) {
      CreatedWindow->SetBackgroundColour(
        m_WParser->GetContainer()->GetBackgroundColour());
    }
  }
  if( CreatedObject != NULL )  {
    bool manage = GetBoolAttribute(tag, "MANAGE");
    if( !html->AddObject(
      ObjectName, CreatedObject, CreatedWindow, manage))
    {
      TBasicApp::NewLogEntry(logError) << "HTML: duplicated object \'" <<
        ObjectName << '\'';
    }
    if( CreatedWindow != NULL && !ObjectName.IsEmpty() )  {
      CreatedWindow->Hide();
      olxstr bgc, fgc;
      if( tag.HasParam(wxT("BGCOLOR")) )  {
        bgc = ExpandMacroShortcuts(tag.GetParam(wxT("BGCOLOR")), macro_map);
        op->processFunction(bgc, SrcInfo, false);
      }
      if( tag.HasParam(wxT("FGCOLOR")) )  {
        fgc = ExpandMacroShortcuts(tag.GetParam(wxT("FGCOLOR")), macro_map);
        op->processFunction(fgc, SrcInfo, false);
      }
      if( !bgc.IsEmpty() )
        CreatedWindow->SetBackgroundColour(wxColor(bgc.u_str()));
      if( !fgc.IsEmpty() )
        CreatedWindow->SetForegroundColour(wxColor(fgc.u_str()));
    }
  }
  return false;
}

TAG_HANDLER_END(INPUT)

TAGS_MODULE_BEGIN(Input)
    TAGS_MODULE_ADD(INPUT)
    TAGS_MODULE_ADD(IMAGE)
    TAGS_MODULE_ADD(RECT)
    TAGS_MODULE_ADD(CIRCLE)
    TAGS_MODULE_ADD(SWITCHINFOS)
    TAGS_MODULE_ADD(SWITCHINFOE)
TAGS_MODULE_END(Input)
