/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_image_cell_ext_H
#define __olx_image_cell_ext_H
#include "estrlist.h"
#include "emath.h"
#include "wx/wxhtml.h"
#include "wx/gifdecod.h"
#include "wx/timer.h"
#include "../ctrls/olxctrlbase.h"

class THtmlImageCell : public wxHtmlCell, public AOlxCtrl {
  struct Rect {
    short left, top, right, bottom;
    Rect(short l, short t, short r, short b) : left(l), top(t), right(r), bottom(b) {}
    inline bool IsInside(short x, short y) const {
      return (x >= left && x <= right && y >= top && y <= bottom);
    }
  };
  struct Circle {
    short x, y;
    float qr;
    Circle(short _x, short _y, float _r) : x(_x), y(_y), qr(_r*_r) {}
    inline bool IsInside(short _x, short _y) const {
      return (olx_sqr(_x - x) + olx_sqr(_y - y) <= qr);
    }
  };
  struct AShapeInfo {
    wxHtmlLinkInfo* link;
    AShapeInfo(wxHtmlLinkInfo* lnk) : link(lnk) {}
    virtual ~AShapeInfo() { delete link; }
    virtual bool IsInside(short x, short y) const = 0;
  };
  template <class Shape>
  struct ShapeInfo : public AShapeInfo {
    Shape shape;
    ShapeInfo(const Shape& _shape, wxHtmlLinkInfo* lnk) : AShapeInfo(lnk), shape(_shape) {}
    virtual bool IsInside(short x, short y) const { return shape.IsInside(x, y); }
  };
  TTypeList<AShapeInfo> Shapes;
  void Draw(wxDC& dc, int x, int y);
public:
  THtmlImageCell(wxWindow *window,
    wxFSFile *input, int w = wxDefaultCoord, int h = wxDefaultCoord,
    double scale = 1.0, int align = wxHTML_ALIGN_BOTTOM,
    const wxString& mapname = wxEmptyString,
    bool WidthInPercent = false,
    bool HeightInPercent = false);
  ~THtmlImageCell();
  void Draw(wxDC& dc, int x, int y, int WXUNUSED(view_y1), int WXUNUSED(view_y2),
    wxHtmlRenderingInfo& WXUNUSED(info)) {
    Draw(dc, x, y);
  }
  virtual wxHtmlLinkInfo *GetLink(int x = 0, int y = 0) const;
  wxScrolledWindow* GetWindow() { return m_window; }
  void SetImage(const wxImage& img);
  void AddRect(short l, short t, short r, short b, const wxString& href, const wxString& target) {
    Shapes.Add(new ShapeInfo<Rect>(Rect(l, t, r, b), new wxHtmlLinkInfo(href, target)));
  }
  void AddCircle(short x, short y, float r, const wxString& href, const wxString& target) {
    Shapes.Add(new ShapeInfo<Circle>(Circle(x, y, r), new wxHtmlLinkInfo(href, target)));
  }
#if wxUSE_GIF && wxUSE_TIMER
  void AdvanceAnimation(wxTimer *timer);
  virtual void Layout(int w);
#endif
  void SetText(const wxString& text) { Text = text; }
  const wxString& GetText() const { return Text; }

  void SetSource(const olxstr& text) { FSource = text; }
  const olxstr& GetSource() const { return FSource; }
private:
  wxBitmap           *m_bitmap;
  wxFSFile           *File;
  wxString           Text;
  olxstr           FSource;
  int                 m_bmpW, m_bmpH;
  bool                m_showFrame : 1;
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

#endif
