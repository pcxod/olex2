/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_html_widget_H
#define __olx_html_widget_H
#include "wx/wxhtml.h"
#include "eset.h"

class THtmlWidgetCell : public wxHtmlWidgetCell {
  int float_y;
protected:
  wxRect CalcRect() const {
    int absx = 0, absy = 0, stx, sty;
    const wxHtmlCell *c = this;
    while (c != 0) {
      absx += c->GetPosX();
      absy += c->GetPosY();
      c = c->GetParent();
    }
    wxScrolledWindow *scrolwin =
      wxDynamicCast(m_Wnd->GetParent(), wxScrolledWindow);
    scrolwin->GetViewStart(&stx, &sty);
    return wxRect(absx - wxHTML_SCROLL_STEP * stx,
        absy - wxHTML_SCROLL_STEP * sty,
        m_Width, m_Height);
  }
public:
  THtmlWidgetCell(wxWindow* _wnd, int _float_x = 0, int _float_y =0) :
    wxHtmlWidgetCell(_wnd, _float_x),
    float_y(_float_y)
  {}

  ~THtmlWidgetCell() {}

  int GetFloatY() const { return float_y;  }
  // implemented in imagecellext.cpp ...
  void SetHeight(int h);

  void Draw(wxDC& WXUNUSED(dc), int WXUNUSED(x), int WXUNUSED(y),
    int WXUNUSED(view_y1), int WXUNUSED(view_y2),
    wxHtmlRenderingInfo& WXUNUSED(info))
  {
    wxRect csz = m_Wnd->GetRect(),
      nsz = CalcRect();
    if (csz != nsz) {
      m_Wnd->SetSize(nsz);
    }
  }

  void DrawInvisible(wxDC& WXUNUSED(dc), int WXUNUSED(x), int WXUNUSED(y),
    wxHtmlRenderingInfo& WXUNUSED(info))
  {
    wxRect csz = m_Wnd->GetRect(),
      nsz = nsz = CalcRect();
    if (csz != nsz) {
      m_Wnd->SetSize(nsz);
    }
  }
};
#endif
