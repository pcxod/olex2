/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "imgcellext.h"
#include "bapp.h"
#include "log.h"
#include "wx/dc.h"
#include "wx/artprov.h"

THtmlImageCell::THtmlImageCell(wxWindow *window, wxFSFile *input,
  int w, int h, double scale, int align,
  const wxString& mapname,
  bool width_per, bool height_per) : wxHtmlCell(), AOlxCtrl(window)
{
  m_window = window ? wxStaticCast(window, wxScrolledWindow) : 0;
  m_scale = scale;
  m_showFrame = false;
  m_bitmap = 0;
  m_bmpW = w;
  m_bmpH = h;
  m_mapName = mapname;
  SetCanLiveOnPagebreak(false);
#if wxUSE_GIF && wxUSE_TIMER
  m_gifDecoder = 0;
  m_gifTimer = 0;
  File = input;
  m_physX = m_physY = wxDefaultCoord;
#endif
  if (m_bmpW && m_bmpH) {
    if (input != 0) {
      wxInputStream *s = input->GetStream();
      if (s != 0) {
        bool readImg = true;

#if wxUSE_GIF && wxUSE_TIMER && !wxCHECK_VERSION(2,8,0)
        if ((input->GetLocation().Matches(wxT("*.gif")) ||
          input->GetLocation().Matches(wxT("*.GIF"))) && m_window)
        {
          m_gifDecoder = new wxGIFDecoder(s, true);
          if (m_gifDecoder->ReadGIF() == wxGIF_OK) {
            wxImage img;
            if (m_gifDecoder->ConvertToImage(&img))
              SetImage(img);
            readImg = false;
            if (m_gifDecoder->IsAnimation()) {
              m_gifTimer = new wxGIFTimer(this);
              m_gifTimer->Start(m_gifDecoder->GetDelay(), true);
            }
            else {
              wxDELETE(m_gifDecoder);
            }
          }
          else {
            wxDELETE(m_gifDecoder);
          }
        }
        if (readImg)
#endif // wxUSE_GIF && wxUSE_TIMER
        {
          wxImage image;
          {
            wxLogNull nl;
            image.LoadFile(*s, wxBITMAP_TYPE_ANY);
          }
          if (image.Ok()) {
            SetImage(image);
          }
          else {
            if (mapname.IsEmpty()) {
              TBasicApp::NewLogEntry(logError) << "Invalid image";
            }
            else {
              TBasicApp::NewLogEntry(logError) << "Invalid image with map: " << mapname;
            }
          }
        }
      }
    }
  }
  else {  // input==NULL, use "broken image" bitmap
    if (m_bmpW == wxDefaultCoord && m_bmpH == wxDefaultCoord) {
      m_bmpW = 29;
      m_bmpH = 31;
    }
    else {
      m_showFrame = true;
      if (m_bmpW == wxDefaultCoord) {
        m_bmpW = 31;
      }
      if (m_bmpH == wxDefaultCoord) {
        m_bmpH = 33;
      }
    }
    m_bitmap = new wxBitmap(wxArtProvider::GetBitmap(wxART_MISSING_IMAGE));
  }
  //else: ignore the 0-sized images used sometimes on the Web pages

  m_Width = (int)(scale * (double)m_bmpW);
  m_Height = (int)(scale * (double)m_bmpH);
  WidthInPercent = width_per;
  HeightInPercent = height_per;
  switch (align) {
  case wxHTML_ALIGN_TOP:
    m_Descent = m_Height;
    break;
  case wxHTML_ALIGN_CENTER:
    m_Descent = m_Height / 2;
    break;
  case wxHTML_ALIGN_BOTTOM:
  default:
    m_Descent = 0;
    break;
  }
}
//..............................................................................
void THtmlImageCell::SetImage(const wxImage& img) {
  if (img.Ok()) {
    delete m_bitmap;
    int ww = img.GetWidth();
    int hh = img.GetHeight();
    if (m_bmpW == wxDefaultCoord) {
      if (m_bmpH != wxDefaultCoord) {
        m_bmpW = ww*m_bmpH / hh;
      }
      else {
        m_bmpW = ww;
      }
    }
    if (m_bmpH == wxDefaultCoord) {
      if (m_bmpW != wxDefaultCoord) {
        m_bmpH = hh * m_bmpW / ww;
      }
      else {
        m_bmpH = hh;
      }
    }

    if ((m_bmpW != ww || m_bmpH != hh) && m_bmpW > 0 && m_bmpH > 0) {
      wxImage img2 = img.Scale(m_bmpW, m_bmpH, wxIMAGE_QUALITY_HIGH);
      m_bitmap = new wxBitmap(img2);
    }
    else {
      m_bitmap = new wxBitmap(img);
    }
  }
}
//..............................................................................
#if wxUSE_GIF && wxUSE_TIMER && !wxCHECK_VERSION(2,8,0)
void THtmlImageCell::AdvanceAnimation(wxTimer *timer)  {
  wxImage img;
  m_gifDecoder->GoNextFrame(true);

  if( m_physX == wxDefaultCoord )  {
    m_physX = m_physY = 0;
    for(wxHtmlCell *cell = this; cell; cell = cell->GetParent() )  {
      m_physX += cell->GetPosX();
      m_physY += cell->GetPosY();
    }
  }

  int x, y;
  m_window->CalcScrolledPosition(m_physX, m_physY, &x, &y);
  wxRect rect(x, y, m_Width, m_Height);

  if( m_window->GetClientRect().Intersects(rect) &&
      m_gifDecoder->ConvertToImage(&img) )
  {
    if( (int)m_gifDecoder->GetWidth() != m_Width ||
         (int)m_gifDecoder->GetHeight() != m_Height ||
         m_gifDecoder->GetLeft() != 0 || m_gifDecoder->GetTop() != 0 )
      {
        wxBitmap bmp(img);
        wxMemoryDC dc;
        dc.SelectObject(*m_bitmap);
        dc.DrawBitmap(bmp, m_gifDecoder->GetLeft(), m_gifDecoder->GetTop(), true /* use mask */);
      }
      else
        SetImage(img);
      m_window->Refresh(img.HasMask(), &rect);
  }
  timer->Start(m_gifDecoder->GetDelay(), true);
}
#endif
//..............................................................................
void THtmlImageCell::Layout(int w) {
  wxHtmlCell::Layout(w);
  m_physX = m_physY = wxDefaultCoord;
}
//..............................................................................

THtmlImageCell::~THtmlImageCell()  {
  if (File != 0) {
    delete File;
  }
    delete m_bitmap;
#if wxUSE_GIF && wxUSE_TIMER
  delete m_gifTimer;
  delete m_gifDecoder;
#endif
}
//..............................................................................
void THtmlImageCell::Draw(wxDC& dc, int x, int y) {
  int width = m_bmpW, height = m_bmpH;
  if (WidthInPercent || HeightInPercent) {
    if (WidthInPercent) {
      width = GetParent()->GetWidth() * m_Width / 100;
    }
    if (HeightInPercent) {
      height = GetParent()->GetHeight() * m_Height / 100;
    }
  }
  if (m_showFrame) {
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.SetPen(*wxBLACK_PEN);
    dc.DrawRectangle(x + m_PosX, y + m_PosY, (int)(width*m_scale), (int)(height*m_scale));
    x++, y++;
  }
  if (m_bitmap != 0) {
    // We add in the scaling from the desired bitmap width
    // and height, so we only do the scaling once.
    double imageScaleX = 1.0;
    double imageScaleY = 1.0;
    if (width != m_bitmap->GetWidth())
      imageScaleX = (double)width / (double)m_bitmap->GetWidth();
    if (height != m_bitmap->GetHeight())
      imageScaleY = (double)height / (double)m_bitmap->GetHeight();

    double us_x, us_y;
    dc.GetUserScale(&us_x, &us_y);
    dc.SetUserScale(us_x * m_scale * imageScaleX, us_y * m_scale * imageScaleY);
    int cx = (int)((double)(x + m_PosX) / (m_scale*imageScaleX)),
      cy = (int)((double)(y + m_PosY) / (m_scale*imageScaleY));
    //        dc.DrawBitmap(*m_bitmap, cx+1, cy, true);
    dc.DrawBitmap(*m_bitmap, cx, cy, true);
    dc.SetUserScale(us_x, us_y);
    if (!Text.IsEmpty()) {
      dc.SetTextForeground(*wxBLACK);
      dc.DrawText(Text, x + m_PosX, y + m_PosY);
    }
  }
}
//..............................................................................
wxHtmlLinkInfo *THtmlImageCell::GetLink(int x, int y) const {
  for (int i = Shapes.Count() - 1; i >= 0; i--) {
    if (Shapes[i].IsInside(x, y)) {
      return Shapes[i].link;
    }
  }
  wxHtmlContainerCell *op = GetParent(), *p = op;
  while (p != 0) {
    op = p;
    p = p->GetParent();
  }
  p = op;
  wxHtmlCell *cell = (wxHtmlCell*)p->Find(wxHTML_COND_ISIMAGEMAP, (const void*)(&m_mapName));
  return (cell == 0) ? wxHtmlCell::GetLink(x, y) : cell->GetLink(x, y);
}
//..............................................................................
