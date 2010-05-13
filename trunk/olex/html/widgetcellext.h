#ifndef __olx_html_widget_H
#define __olx_html_widget_H
#include "wx/wxhtml.h"

class THtmlWidgetCell : public wxHtmlWidgetCell  {
  int float_y;
public:
  THtmlWidgetCell(wxWindow* _wnd, int _float_x = 0, int _float_y =0) :
    wxHtmlWidgetCell(_wnd, _float_x),
    float_y(_float_y)  {}
  ~THtmlWidgetCell()  {}
  virtual void Layout(int w)  {
    const int height = m_Wnd->GetParent()->GetClientSize().GetHeight();
    wxHtmlCell *c = this;
    int wi = 0, hi = 0, _h = GetHeight();
    if( GetParent() != NULL && GetParent()->GetHeight() <= _h )
      _h = GetParent()->GetHeight();
    while( c != NULL )  {
      //wi += c->GetPosX();
      hi += c->GetPosY();
      _h = c->GetHeight();
      c = c->GetParent();
      if( c != NULL )  {
        hi += (c->GetHeight() - _h);
        _h = c->GetHeight();
      }
    }
    //const int height = GetParent()->GetHeight();
    if( m_WidthFloat != 0 )
      m_Width = (w * m_WidthFloat) / 100;
    if( float_y != 0 )
      m_Height = ((height-hi) * float_y) / 100;
    if( m_WidthFloat != 0 || float_y != 0 )
      m_Wnd->SetSize(m_Width, m_Height);
  }
};
#endif
