#include "htmlext.h"
#include "imgcellext.h"
#include "widgetcellext.h"
#include "../mainform.h"
#include "../xglapp.h"
#include "wx/html/htmlcell.h"
#include "wx/html/m_templ.h"
#include "wxzipfs.h"

#ifdef _UNICODE
  #define _StrFormat_ wxT("%ls")
#else
  #define _StrFormat_ wxT("%s")
#endif

// helper tag
TAG_HANDLER_BEGIN(SWITCHINFOS, "SWITCHINFOS")
TAG_HANDLER_PROC(tag)  {
  THtml::SwitchSources.Push( THtml::SwitchSource );
  THtml::SwitchSource = tag.GetParam(wxT("SRC")).c_str();
  return true;
}
TAG_HANDLER_END(SWITCHINFOS)
// helper tag
TAG_HANDLER_BEGIN(SWITCHINFOE, "SWITCHINFOE")
TAG_HANDLER_PROC(tag)
{
  THtml::SwitchSource = THtml::SwitchSources.Pop();
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
      THtmlImageCell* ic = (THtmlImageCell*)m_WParser->GetContainer()->GetLastChild();
      TStrList toks( tag.GetParam(wxT("COORDS")).c_str(), ',');
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
      THtmlImageCell* ic = (THtmlImageCell*)m_WParser->GetContainer()->GetLastChild();
      TStrList toks( tag.GetParam(wxT("COORDS")).c_str(), ',');
      if( toks.Count() == 3 )
        ic->AddCircle(
          toks[0].ToInt(), 
          toks[1].ToInt(),
          toks[2].ToFloat<float>(),
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
  wxString text = tag.GetParam(wxT("TEXT")),
           mapName = tag.GetParam(wxT("USEMAP"));
  olxstr ObjectName = tag.GetParam(wxT("NAME")).c_str(),
    Tmp;
  try  {
    Tmp = tag.GetParam(wxT("WIDTH")).c_str();
    if( !Tmp.IsEmpty() )  {
      if( Tmp.EndsWith('%') )  {
        ax = (int)Tmp.SubStringTo(Tmp.Length()-1).ToDouble();
        WidthInPercent = true;
      }
      else
        ax = (int)Tmp.ToDouble();
    }
    Tmp = tag.GetParam(wxT("HEIGHT")).c_str();
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
    TBasicApp::GetLog().Exception(e.GetException()->GetFullMessage());
    TBasicApp::GetLog() << (olxstr("While processing Width/Height for zimg::") << ObjectName << '\n');
    TBasicApp::GetLog() << (olxstr("Offending input: '") << Tmp << "'\n");
  }

  if (tag.HasParam(wxT("FLOAT"))) fl = ax;

  if( !text.IsEmpty() )  {
    int textW = 0, textH = 0;
    m_WParser->GetDC()->GetTextExtent( text, &textW, &textH );
    if( textW > ax )  ax = textW;
    if( textH > ay )  ay = textH;
  }

  olxstr src = tag.GetParam(wxT("SRC")).c_str();

  TGlXApp::GetMainForm()->ProcessFunction(src);

  if( TZipWrapper::IsZipFile(THtml::SwitchSource) && !TZipWrapper::IsZipFile(src) )
    src = TZipWrapper::ComposeFileName(THtml::SwitchSource, src);

  wxFSFile *fsFile = TFileHandlerManager::GetFSFileHandler( src );
  if( fsFile == NULL )
    TBasicApp::GetLog().Error( olxstr("Could not locate image: ") << src );

  if( (mapName.Length() > 0) && mapName.GetChar(0) == '#')
      mapName = mapName.Mid( 1 );

  THtmlImageCell *cell = new THtmlImageCell( m_WParser->GetWindowInterface()->GetHTMLWindow(),
                                           fsFile,
                                           ax, ay,
                                           m_WParser->GetPixelScale(),
                                           wxHTML_ALIGN_BOTTOM,
                                           mapName,
                                           WidthInPercent,
                                           HeightInPercent
                                           );

  cell->SetText( text );
  cell->SetSource( src );
  cell->SetLink(m_WParser->GetLink());
  cell->SetId(tag.GetParam(wxT("id"))); // may be empty
  m_WParser->GetContainer()->InsertCell(cell);
  if( !ObjectName.IsEmpty() )  {
    if( !TGlXApp::GetMainForm()->GetHtml()->AddObject(ObjectName, cell, NULL) )
      TBasicApp::GetLog().Error(olxstr("THTML: object already exist: ") << ObjectName);
  }
  return false;
}
TAG_HANDLER_END(IMAGE)
// input tag
TAG_HANDLER_BEGIN(INPUT, "INPUT")
TAG_HANDLER_PROC(tag)  {
  olxstr TagName = tag.GetParam(wxT("TYPE")).c_str();
  olxstr ObjectName, Value, Data, Tmp, Label;
  ObjectName = tag.GetParam(wxT("NAME")).c_str();
  int valign = -1, halign = -1, 
    fl=0,
    ax=100, ay=20;
  bool width_set = false, height_set = false;
  AOlxCtrl* CreatedObject = NULL;
  wxWindow* CreatedWindow = NULL;
  try  {
    Tmp = tag.GetParam(wxT("WIDTH")).c_str();
    TGlXApp::GetMainForm()->ProcessFunction(Tmp);
    if( !Tmp.IsEmpty() )  {
      if( Tmp.EndsWith('%') )  {
        ax = 0;
        float _ax = Tmp.SubStringTo(Tmp.Length()-1).ToFloat<float>()/100;
        _ax *= m_WParser->GetWindowInterface()->GetHTMLWindow()->GetSize().GetWidth();
        ax = (int)_ax;
      }
      else
        ax = (int)Tmp.ToDouble();
      width_set = true;
    }
    Tmp = tag.GetParam(wxT("HEIGHT")).c_str();
    TGlXApp::GetMainForm()->ProcessFunction(Tmp);
    if( !Tmp.IsEmpty() )  {
      if( Tmp.EndsWith('%') )  {
        ay = 0;
        float _ay = Tmp.SubStringTo(Tmp.Length()-1).ToFloat<float>()/100;
        _ay *= m_WParser->GetWindowInterface()->GetHTMLWindow()->GetSize().GetHeight();
        ay = (int)_ay;
      }
      else
        ay = Tmp.ToDouble();
      height_set = true;
    }
  }
  catch(const TExceptionBase& e)  {
    TBasicApp::GetLog().Exception(e.GetException()->GetFullMessage());
    TBasicApp::GetLog() << (olxstr("While processing Width/Height HTML tags for ") <<
      TagName << "::" << ObjectName << '\n');
    TBasicApp::GetLog() << (olxstr("Offending input: '") << Tmp << "'\n");
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
      else if( ha.CmpNoCase(wxT("center")) == 0 || ha.CmpNoCase(wxT("middle")) == 0 )
        halign = wxHTML_ALIGN_CENTER;
      else if( ha.CmpNoCase(wxT("right")) == 0 )
        halign = wxHTML_ALIGN_RIGHT;
    }
  }
  if( tag.HasParam(wxT("VALIGN")) ){
    wxString va = tag.GetParam(wxT("VALIGN"));
    if( va.CmpNoCase(wxT("top")) == 0 )
      valign = wxHTML_ALIGN_TOP;
    else if( va.CmpNoCase(wxT("center")) == 0 || va.CmpNoCase(wxT("middle")) == 0 )
      valign = wxHTML_ALIGN_CENTER;
    else if( va.CmpNoCase(wxT("bottom")) == 0 )
      valign = wxHTML_ALIGN_BOTTOM;
  }
  Label = tag.GetParam(wxT("LABEL")).c_str();

  wxHtmlLinkInfo* LinkInfo = NULL;
  if( !Label.IsEmpty() )  {
    if( Label.StartsFromi("href=") )  {
      Label = Label.SubStringFrom(5);
      const size_t tagInd = Label.IndexOfi("&target=");
      olxstr tag(EmptyString);
      if( tagInd != InvalidIndex )  {
        tag = Label.SubStringFrom(tagInd+8);
        Label.SetLength(tagInd);
      }
      LinkInfo = new wxHtmlLinkInfo(Label.u_str(), tag.u_str() );
    }
  }
  if( !ObjectName.IsEmpty() )  {
    wxWindow* wnd = TGlXApp::GetMainForm()->GetHtml()->FindObjectWindow(ObjectName);
    if( wnd != NULL )  {
      if( !tag.HasParam(wxT("reuse")) )
        TBasicApp::GetLog().Error(olxstr("HTML: duplicated object \'") << ObjectName << '\'');
      else  {
        if( !Label.IsEmpty() )  {
          wxHtmlContainerCell* contC = new wxHtmlContainerCell(m_WParser->GetContainer());
          THtml::WordCell* wc = new THtml::WordCell( Label.u_str(), *m_WParser->GetDC());
          if( LinkInfo != NULL ) {  
            wc->SetLink(*LinkInfo);
            delete LinkInfo;
          }
          wc->SetDescent(0);
          contC->InsertCell( wc );
          contC->InsertCell(new wxHtmlWidgetCell(wnd, fl));
          if( valign != -1 )  contC->SetAlignVer(valign);
          if( halign != -1 )  contC->SetAlignHor(halign);
        }
        else
          m_WParser->GetContainer()->InsertCell(new wxHtmlWidgetCell(wnd, fl));
      }
      return false;
    }
  }

  Value = tag.GetParam(wxT("VALUE")).c_str();
  TGlXApp::GetMainForm()->ProcessFunction(Value);
  Data = tag.GetParam(wxT("DATA")).c_str();
/******************* TEXT CONTROL *********************************************/
  if( TagName.Equalsi("text") )  {
    int flags = wxWANTS_CHARS;
    if( tag.HasParam( wxT("MULTILINE") ) )  flags |= wxTE_MULTILINE;
    if( tag.HasParam( wxT("PASSWORD") ) )     flags |= wxTE_PASSWORD;
    TTextEdit *Text = new TTextEdit(m_WParser->GetWindowInterface()->GetHTMLWindow(), flags);
    Text->SetFont( m_WParser->GetDC()->GetFont() );
    CreatedObject = Text;
    CreatedWindow = Text;
    Text->SetSize(ax, ay);
    Text->SetData( Data );

    Text->SetText(Value);
    if( !Label.IsEmpty() )  {
      wxHtmlContainerCell* contC = new wxHtmlContainerCell(m_WParser->GetContainer());
      THtml::WordCell* wc = new THtml::WordCell(Label.u_str(), *m_WParser->GetDC());
      if( LinkInfo != NULL ) wc->SetLink(*LinkInfo);
      wc->SetDescent(0);
      contC->InsertCell( wc );
      contC->InsertCell(new wxHtmlWidgetCell(Text, fl));
      if( valign != -1 )  contC->SetAlignVer(valign);
      if( halign != -1 )  contC->SetAlignHor(halign);
    }
    else
      m_WParser->GetContainer()->InsertCell(new wxHtmlWidgetCell(Text, fl));

    if( tag.HasParam(wxT("ONCHANGE")) )  {
      Text->SetOnChangeStr( tag.GetParam(wxT("ONCHANGE")).c_str() );
      Text->OnChange.Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( tag.HasParam(wxT("ONLEAVE")) )  {
      Text->SetOnLeaveStr( tag.GetParam(wxT("ONLEAVE")).c_str() );
      Text->OnLeave.Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( tag.HasParam(wxT("ONENTER")) )  {
      Text->SetOnEnterStr( tag.GetParam(wxT("ONENTER")).c_str() );
      Text->OnEnter.Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( tag.HasParam(wxT("ONRETURN")) )  {
      Text->SetOnReturnStr( tag.GetParam(wxT("ONRETURN")).c_str() );
      Text->OnReturn.Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
  }
/******************* LABEL ***************************************************/
  else if( TagName.Equalsi("label") )  {
    TLabel *Text = new TLabel(m_WParser->GetWindowInterface()->GetHTMLWindow(), Value);
    Text->SetFont( m_WParser->GetDC()->GetFont() );
    CreatedObject = Text;
    CreatedWindow = Text;
    Text->WI.SetWidth( ax );
    Text->WI.SetHeight( ay );
    Text->SetData( Data );
    m_WParser->GetContainer()->InsertCell(new wxHtmlWidgetCell(Text, fl));
  }
/******************* BUTTON ***************************************************/
  else if( TagName.Equalsi("button") )  {
    AButtonBase *Btn;
    long flags = 0;
    if( tag.HasParam(wxT("FIT")) )  flags |= wxBU_EXACTFIT;
    if( tag.HasParam(wxT("FLAT")) )  flags |= wxNO_BORDER;
    olxstr buttonImage = tag.GetParam(wxT("IMAGE")).c_str();
    if( !buttonImage.IsEmpty() )  {
      if( buttonImage.IndexOf(',') != InvalidIndex )  {
        TImgButton* ibtn = new TImgButton(m_WParser->GetWindowInterface()->GetHTMLWindow());
        ibtn->SetImages(buttonImage, width_set ? ax : -1, height_set ? ay : -1);
        if( tag.HasParam(wxT("ENABLED")) )
          ibtn->SetEnabled(olxstr(tag.GetParam(wxT("ENABLED")).c_str()).ToBool());
        if( tag.HasParam(wxT("DOWN")) )
          ibtn->SetDown(olxstr(tag.GetParam(wxT("DOWN")).c_str()).ToBool());
        CreatedWindow = ibtn;
        Btn = ibtn;
      }
      else  {
        Btn = new TBmpButton(  m_WParser->GetWindowInterface()->GetHTMLWindow(), -1, wxNullBitmap, 
          wxDefaultPosition, wxDefaultSize, flags );
        ((TBmpButton*)Btn)->SetSource( buttonImage );
        wxFSFile *fsFile = TFileHandlerManager::GetFSFileHandler( buttonImage );
        if( fsFile == NULL )
          TBasicApp::GetLog().Error(olxstr("THTML: could not locate image for button: ") << ObjectName);
        else  {
          wxImage image(*(fsFile->GetStream()), wxBITMAP_TYPE_ANY);
          if ( !image.Ok() )
            TBasicApp::GetLog().Error(olxstr("THTML: could not load image for button: ") << ObjectName);
          else  {
            if( (image.GetWidth() > ax || image.GetHeight() > ay) && tag.HasParam(wxT("STRETCH")) )
              image = image.Scale(ax, ay);
            else  {
              ax = image.GetWidth();
              ay = image.GetHeight();
            }
            ((TBmpButton*)Btn)->SetBitmapLabel( image );
          }
          delete fsFile;
        }
        Btn->WI.SetWidth(ax);
        Btn->WI.SetHeight(ay);
        ((TBmpButton*)Btn)->SetFont( m_WParser->GetDC()->GetFont() );
        CreatedWindow = (TBmpButton*)Btn;
      }
    }
    else  {
      Btn = new TButton( m_WParser->GetWindowInterface()->GetHTMLWindow(), -1, wxEmptyString, 
        wxDefaultPosition, wxDefaultSize, flags );
      ((TButton*)Btn)->SetCaption(Value);
      ((TButton*)Btn)->SetFont( m_WParser->GetDC()->GetFont() );
      if( (flags & wxBU_EXACTFIT) == 0 )  {
        Btn->WI.SetWidth(ax);
        Btn->WI.SetHeight(ay);
      }
#ifdef __WXGTK__  // got no idea what happens here, client size does not work?
      wxFont fnt(m_WParser->GetDC()->GetFont());
      fnt.SetPointSize( fnt.GetPointSize()-2);
      ((TButton*)Btn)->SetFont( fnt );
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
    if( tag.HasParam(wxT("ONCLICK")) )  {
      Btn->SetOnClickStr( tag.GetParam(wxT("ONCLICK")).c_str() );
      Btn->OnClick.Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( tag.HasParam(wxT("ONDOWN")) )  {
      Btn->SetOnUpStr( tag.GetParam(wxT("ONUP")).c_str() );
      Btn->OnUp.Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( tag.HasParam(wxT("ONDOWN")) )  {
      Btn->SetOnDownStr( tag.GetParam(wxT("ONDOWN")).c_str() );
      Btn->OnDown.Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }

    if( tag.HasParam(wxT("DOWN")) )
      Btn->SetDown( tag.GetParam(wxT("DOWN")).CmpNoCase(wxT("true")) == 0 );

    if( tag.HasParam(wxT("HINT")) )
      Btn->SetHint( tag.GetParam(wxT("HINT")).c_str() );

    olxstr modeDependent = tag.GetParam(wxT("MODEDEPENDENT")).c_str();
    if( !modeDependent.IsEmpty() )  {
      Btn->SetActionQueue(TGlXApp::GetMainForm()->OnModeChange, modeDependent);
    }

    if( EsdlInstanceOf(*Btn, TButton) )
      m_WParser->GetContainer()->InsertCell(new wxHtmlWidgetCell((TButton*)Btn, fl));
    else  if( EsdlInstanceOf(*Btn, TBmpButton) )
      m_WParser->GetContainer()->InsertCell(new wxHtmlWidgetCell((TBmpButton*)Btn, fl));
    else  if( EsdlInstanceOf(*Btn, TImgButton) )
      m_WParser->GetContainer()->InsertCell(new wxHtmlWidgetCell((TImgButton*)Btn, fl));
  }
/******************* COMBOBOX *************************************************/
  else if( TagName.Equalsi("combo") )  {
    TComboBox *Box = new TComboBox( m_WParser->GetWindowInterface()->GetHTMLWindow(),
                                    tag.HasParam(wxT("READONLY")),
                                    wxSize(ax, ay) );
    Box->SetFont( m_WParser->GetDC()->GetFont() );

    CreatedObject = Box;
    CreatedWindow = Box;
    Box->WI.SetWidth(ax);
#ifdef __MAC__    
    Box->WI.SetHeight( olx_max(ay, Box->GetCharHeight()+10) );
#else
    Box->WI.SetHeight( ay );
#endif    
    if( tag.HasParam(wxT("ITEMS")) )  {
      olxstr Items = tag.GetParam(wxT("ITEMS")).c_str();
      TGlXApp::GetMainForm()->ProcessFunction(Items);
      TStrList SL(Items, ';');
      if( SL.IsEmpty() )
        Box->AddObject(EmptyString);  // fix the bug in wxWidgets (if Up pressed, crass occurs)
      else
        Box->AddItems(SL);
    }
    else  {  // need to intialise the items - or wxWidgets will crash (pressing Up button)
      Box->AddObject(Value);
      Box->AddObject(EmptyString);  // fix the bug in wxWidgets (if Up pressed, crass occurs)
    }
    Box->SetText(Value);
    Box->SetData(Data);
    if( tag.HasParam(wxT("ONCHANGE")) )  {
      Box->SetOnChangeStr( tag.GetParam(wxT("ONCHANGE")).c_str() );
      Box->OnChange.Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( tag.HasParam(wxT("ONLEAVE")) )  {
      Box->SetOnLeaveStr( tag.GetParam(wxT("ONLEAVE")).c_str() );
      Box->OnLeave.Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( tag.HasParam(wxT("ONENTER")) )  {
      Box->SetOnEnterStr( tag.GetParam(wxT("ONENTER")).c_str() );
      Box->OnEnter.Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( tag.HasParam(wxT("ONRETURN")) )  {
      Box->SetOnReturnStr( tag.GetParam(wxT("ONRETURN")).c_str() );
      Box->OnReturn.Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( !Label.IsEmpty() )  {
      wxHtmlContainerCell* contC = new wxHtmlContainerCell(m_WParser->GetContainer());
      THtml::WordCell* wc = new THtml::WordCell( Label.u_str(), *m_WParser->GetDC());
      if( LinkInfo != NULL ) wc->SetLink(*LinkInfo);
      wc->SetDescent(0);
      contC->InsertCell( wc );
      contC->InsertCell(new wxHtmlWidgetCell(Box, fl));
      if( valign != -1 )  contC->SetAlignVer(valign);
      if( halign != -1 )  contC->SetAlignHor(halign);
    }
    else
      m_WParser->GetContainer()->InsertCell(new wxHtmlWidgetCell(Box, fl));
  }
/******************* SPIN CONTROL *********************************************/
  else if( TagName.Equalsi("spin") )  {
    TSpinCtrl *Spin = new TSpinCtrl( m_WParser->GetWindowInterface()->GetHTMLWindow() );
    Spin->SetFont( m_WParser->GetDC()->GetFont() );
    Spin->SetForegroundColour( m_WParser->GetDC()->GetTextForeground() );

    int min=0, max = 100;
    if( tag.HasParam( wxT("MIN") ) )
      tag.ScanParam(wxT("MIN"), wxT("%i"), &min);
    if( tag.HasParam( wxT("MAX") ) )
      tag.ScanParam(wxT("MAX"), wxT("%i"), &max);
    Spin->SetRange(min, max);
    try  {  Spin->SetValue((int)Value.ToDouble());  }
    catch(...)  {
      TBasicApp::GetLog() << (olxstr("Invalid value spin control: \'") << Value << "\'\n");
    }

    CreatedObject = Spin;
    CreatedWindow = Spin;
    Spin->WI.SetHeight(ay);
    Spin->WI.SetWidth(ax);

    Spin->SetData(Data);
    if( tag.HasParam(wxT("ONCHANGE")) )  {
      Spin->SetOnChangeStr( tag.GetParam(wxT("ONCHANGE")).c_str() );
      Spin->OnChange.Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( !Label.IsEmpty() )  {
      wxHtmlContainerCell* contC = new wxHtmlContainerCell(m_WParser->GetContainer());
      THtml::WordCell* wc = new THtml::WordCell(Label.u_str(), *m_WParser->GetDC());
      if( LinkInfo != NULL ) wc->SetLink(*LinkInfo);
      wc->SetDescent(0);
      contC->InsertCell( wc );
      contC->InsertCell(new wxHtmlWidgetCell(Spin, fl));
      if( valign != -1 )  contC->SetAlignVer(valign);
      if( halign != -1 )  contC->SetAlignHor(halign);
    }
    else
      m_WParser->GetContainer()->InsertCell(new wxHtmlWidgetCell(Spin, fl));
  }
/******************* SLIDER ***************************************************/
  else  if( TagName.Equalsi("slider") )  {
    TTrackBar *Track = new TTrackBar( m_WParser->GetWindowInterface()->GetHTMLWindow() );
    Track->SetFont( m_WParser->GetDC()->GetFont() );

    CreatedObject = Track;
    CreatedWindow = Track;
    int min=0, max = 100;
    if( tag.HasParam( wxT("MIN") ) )
      tag.ScanParam(wxT("MIN"), wxT("%i"), &min);
    if( tag.HasParam( wxT("MAX") ) )
      tag.ScanParam(wxT("MAX"), wxT("%i"), &max);
    Track->WI.SetWidth(ax);
    Track->WI.SetHeight(ay);
    if( min < max )
      Track->SetRange(min, max);
    try  {  Track->SetValue((int)Value.ToDouble());  }
    catch(...)  {
      TBasicApp::GetLog() << (olxstr("Invalid value slider: \'") << Value << "\'\n");
    }

    Track->SetData(Data);
    if( tag.HasParam(wxT("ONCHANGE")) )  {
      Track->SetOnChangeStr(tag.GetParam(wxT("ONCHANGE")).c_str());
      Track->OnChange.Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( tag.HasParam(wxT("ONMOUSEUP")) )  {
      Track->SetOnMouseUpStr(tag.GetParam(wxT("ONMOUSEUP")).c_str());
      Track->OnMouseUp.Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( !Label.IsEmpty() )  {
      wxHtmlContainerCell* contC = new wxHtmlContainerCell(m_WParser->GetContainer());
      THtml::WordCell* wc = new THtml::WordCell(Label.u_str(), *m_WParser->GetDC());
      if( LinkInfo != NULL ) wc->SetLink(*LinkInfo);
      wc->SetDescent(0);
      contC->InsertCell( wc );
      contC->InsertCell(new wxHtmlWidgetCell(Track, fl));
      if( valign != -1 )  contC->SetAlignVer(valign);
      if( halign != -1 )  contC->SetAlignHor(halign);
    }
    else
      m_WParser->GetContainer()->InsertCell(new wxHtmlWidgetCell(Track, fl));
  }
/******************* CHECKBOX *************************************************/
  else if( TagName.Equalsi("checkbox") )  {
    TCheckBox *Box = new TCheckBox( 
      m_WParser->GetWindowInterface()->GetHTMLWindow(), tag.HasParam(wxT("RIGHT")) ? wxALIGN_RIGHT : 0);
    Box->SetFont( m_WParser->GetDC()->GetFont());
    wxLayoutConstraints* wxa = new wxLayoutConstraints;
    wxa->centreX.Absolute(0);
    Box->SetConstraints(wxa);
    CreatedObject = Box;
    CreatedWindow = Box;
    Box->WI.SetWidth(ax);
    Box->WI.SetHeight(ay);

    Box->SetCaption(Value);
    if( tag.HasParam(wxT("CHECKED")) )  {
      Tmp = tag.GetParam(wxT("CHECKED")).c_str();
      if( Tmp.IsEmpty() )
        Box->SetChecked( true );
      else
        Box->SetChecked( Tmp.ToBool() );
    }

    Box->SetData(Data);
    // binding events
    if( tag.HasParam(wxT("ONCLICK")) )  {
      Box->SetOnClickStr( tag.GetParam(wxT("ONCLICK")).c_str() );
      Box->OnClick.Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( tag.HasParam(wxT("ONCHECK")) )  {
      Box->SetOnCheckStr( tag.GetParam(wxT("ONCHECK")).c_str() );
      Box->OnCheck.Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( tag.HasParam(wxT("ONUNCHECK")) )  {
      Box->SetOnUncheckStr( tag.GetParam(wxT("ONUNCHECK")).c_str() );
      Box->OnUncheck.Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( tag.HasParam(wxT("MODEDEPENDENT")) )  {
      Box->SetActionQueue(TGlXApp::GetMainForm()->OnModeChange, tag.GetParam(wxT("MODEDEPENDENT")).c_str());
    }
    m_WParser->GetContainer()->InsertCell(new wxHtmlWidgetCell(Box, fl));
  }
/******************* TREE CONTROL *********************************************/
  else if( TagName.Equalsi("tree") )  {
    long flags = wxTR_HAS_BUTTONS;
    if( tag.HasParam(wxT("NOROOT")) )
      flags |= wxTR_HIDE_ROOT;
    if( tag.HasParam(wxT("EDITABLE")) )
      flags |= wxTR_EDIT_LABELS;

    TTreeView *Tree = new TTreeView(m_WParser->GetWindowInterface()->GetHTMLWindow(), flags);

    if( (flags&wxTR_HIDE_ROOT) == 0 && tag.HasParam(wxT("ROOTLABEL")) )
      Tree->SetItemText(Tree->GetRootItem(), tag.GetParam(wxT("ROOTLABEL")));
    olxstr src = tag.GetParam(wxT("SRC")).c_str();
    TGlXApp::GetMainForm()->ProcessFunction(src);
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
      Tree->SetOnSelectStr( tag.GetParam(wxT("ONSELECT")).c_str() );
      Tree->OnSelect.Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( tag.HasParam(wxT("ONITEM")) )  {
      Tree->SetOnItemActivateStr( tag.GetParam(wxT("ONITEM")).c_str() );
      Tree->OnDblClick.Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( (flags&wxTR_EDIT_LABELS) != 0 && tag.HasParam(wxT("ONEDIT")) )  {
      Tree->SetOnEditStr( tag.GetParam(wxT("ONEDIT")).c_str() );
      Tree->OnEdit.Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    m_WParser->GetContainer()->InsertCell(new wxHtmlWidgetCell(Tree, fl));
    if( ios == NULL )  {  // create test tree
      TBasicApp::GetLog().Error(olxstr("THTML: could not locate tree source: \'") << src <<  '\'');
      wxTreeItemId Root = Tree->AddRoot( wxT("Test data") );
      wxTreeItemId sc1 = Tree->AppendItem( Tree->AppendItem(Root, wxT("child")), wxT("subchild"));
         Tree->AppendItem( Tree->AppendItem(sc1, wxT("child1")), wxT("subchild1"));
      wxTreeItemId sc2 = Tree->AppendItem( Tree->AppendItem(Root, wxT("child1")), wxT("subchild1"));
        sc2 = Tree->AppendItem( Tree->AppendItem(sc2, wxT("child1")), wxT("subchild1"));
    }
    else  {
      TStrList list;
#ifdef _UNICODE
      TUtf8File::ReadLines(*ios, list, false);
#else
      list.LoadFromTextStream(*ios);
#endif
      Tree->LoadFromStrings(list);
      delete ios;
      wxString sel = tag.GetParam(wxT("SELECTED"));
      if( sel.IsEmpty() )  {
        sel = tag.GetParam(wxT("VALUE"));
        if( !sel.IsEmpty() )
          Tree->SelectByLabel(sel.c_str());
      }
      else
          Tree->SelectByData(sel.c_str());
    }
  }
/******************* LIST CONTROL *********************************************/
  else if( TagName.Equalsi("list") )  {
    bool srcTag   = tag.HasParam(wxT("SRC")),
         itemsTag = tag.HasParam(wxT("ITEMS"));
    TStrList itemsList;
    if( srcTag && itemsTag )
      TBasicApp::GetLog().Error( "THTML: list can have only src or items");
    else if( srcTag ) {
      olxstr src = tag.GetParam(wxT("SRC")).c_str();
      TGlXApp::GetMainForm()->ProcessFunction(src);
      IInputStream* ios = TFileHandlerManager::GetInputStream(src);
      if( ios == NULL )
        TBasicApp::GetLog().Error(olxstr("THTML: could not locate list source: \'") << src <<  '\'');
      else  {
#ifdef _UNICODE
      TUtf8File::ReadLines(*ios, itemsList, false);
#else
        itemsList.LoadFromTextStream( *ios );
#endif
        delete ios;
      }
    }
    else if( itemsTag )  {
      olxstr items = tag.GetParam(wxT("ITEMS")).c_str();
      TGlXApp::GetMainForm()->ProcessFunction(items);
      itemsList.Strtok(items, ';');
    }
    TListBox *List = new TListBox( m_WParser->GetWindowInterface()->GetHTMLWindow() );
    List->SetFont( m_WParser->GetDC()->GetFont() );

    CreatedObject = List;
    CreatedWindow = List;
    List->WI.SetWidth(ax);
    List->WI.SetHeight(ay);

    List->SetData( Data );
    List->AddItems( itemsList );
    // binding events
    if( tag.HasParam(wxT("ONSELECT")) )  {
      List->SetOnSelectStr( tag.GetParam(wxT("ONSELECT")).c_str() );
      List->OnSelect.Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( tag.HasParam(wxT("ONDBLCLICK")) )  {
      List->SetOnDblClickStr( tag.GetParam(wxT("ONDBLCLICK")).c_str() );
      List->OnDblClick.Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    // creating cell
    m_WParser->GetContainer()->InsertCell(new wxHtmlWidgetCell(List, fl));
  }
/******************* END OF CONTROLS ******************************************/
  if( LinkInfo != NULL )  delete LinkInfo;
  if( ObjectName.IsEmpty() )  {  }  // create random name?
  if( CreatedWindow != NULL )  {  // set default colors
#ifdef __WIN32__
    if( EsdlInstanceOf(*CreatedWindow, TComboBox) )  {
      TComboBox* Box = (TComboBox*)CreatedWindow;
      if( Box->GetTextCtrl() != NULL )  {
        if( m_WParser->GetContainer()->GetBackgroundColour().IsOk() )
          Box->GetTextCtrl()->SetBackgroundColour( m_WParser->GetContainer()->GetBackgroundColour() );
        if( m_WParser->GetActualColor().IsOk() )
          Box->GetTextCtrl()->SetForegroundColour( m_WParser->GetActualColor() );
      }
      if( Box->GetPopupControl() != NULL && Box->GetPopupControl()->GetControl() != NULL )  {
        if( m_WParser->GetContainer()->GetBackgroundColour().IsOk() )
          Box->GetPopupControl()->GetControl()->SetBackgroundColour( 
            m_WParser->GetContainer()->GetBackgroundColour() );
        if( m_WParser->GetActualColor().IsOk() )  {
          Box->GetPopupControl()->GetControl()->SetForegroundColour( 
            m_WParser->GetActualColor() );
        }
      }
    }
#endif
    if( m_WParser->GetActualColor().IsOk() )
      CreatedWindow->SetForegroundColour( m_WParser->GetActualColor() );
    if( m_WParser->GetContainer()->GetBackgroundColour().IsOk() )
      CreatedWindow->SetBackgroundColour( m_WParser->GetContainer()->GetBackgroundColour() );
  }
  if( CreatedObject != NULL )  {
    if( !TGlXApp::GetMainForm()->GetHtml()->AddObject(ObjectName, CreatedObject, CreatedWindow, tag.HasParam(wxT("MANAGE")) ) )
      TBasicApp::GetLog().Error(olxstr("HTML: duplicated object \'") << ObjectName << '\'');
    if( CreatedWindow != NULL && !ObjectName.IsEmpty() )  {
      CreatedWindow->Hide();
      olxstr bgc, fgc;
      if( tag.HasParam(wxT("BGCOLOR")) )  {
        bgc = tag.GetParam( wxT("BGCOLOR") ).c_str();
        TGlXApp::GetMainForm()->ProcessFunction(bgc);
      }
      if( tag.HasParam(wxT("FGCOLOR")) )  {
        fgc = tag.GetParam( wxT("FGCOLOR") ).c_str();
        TGlXApp::GetMainForm()->ProcessFunction(fgc);
      }

      if( EsdlInstanceOf(*CreatedWindow, TComboBox) )  {
        TComboBox* Box = (TComboBox*)CreatedWindow;
        if( !bgc.IsEmpty() )  {
          wxColor bgCl = wxColor(bgc.u_str());
          Box->SetBackgroundColour( bgCl );
#ifdef __WIN32__          
          if( Box->GetTextCtrl() != NULL )
            Box->GetTextCtrl()->SetBackgroundColour( bgCl );
          if( Box->GetPopupControl() != NULL && Box->GetPopupControl()->GetControl() != NULL )
            Box->GetPopupControl()->GetControl()->SetBackgroundColour( bgCl );
#endif
        }
        if( !fgc.IsEmpty() )  {
          wxColor fgCl = wxColor(bgc.u_str());
          Box->SetForegroundColour( fgCl );
#ifdef __WIN32__          
          if( Box->GetTextCtrl() != NULL )
            Box->GetTextCtrl()->SetForegroundColour( fgCl );
          if( Box->GetPopupControl() != NULL && Box->GetPopupControl()->GetControl() != NULL)
            Box->GetPopupControl()->GetControl()->SetForegroundColour( fgCl );
#endif
        }
      }
      else  {
        if( !bgc.IsEmpty() )
          CreatedWindow->SetBackgroundColour( wxColor(bgc.u_str()) );
        if( !fgc.IsEmpty() )
          CreatedWindow->SetForegroundColour( wxColor(fgc.u_str()) );
      }
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
