#include "guilabels.h"

BEGIN_EVENT_TABLE(GuiLabel, wxStaticText)
  EVT_LEFT_DCLICK(GuiLabel::OnMouseDblClick)
  EVT_LEFT_DOWN(GuiLabel::OnMouseDown)
  EVT_LEFT_UP(GuiLabel::OnMouseUp)
  EVT_MOTION(GuiLabel::OnMouseMotion)
END_EVENT_TABLE()

GuiLabel::GuiLabel(GuiLabels& _Parent, wxWindow* wnd, int x, int y) :
  Parent(_Parent),
  wxStaticText(wnd, -1, wxT(""), wxPoint(x,y)) 
{
  FMouseDown = false; 
}
//..............................................................................
void GuiLabel::OnMouseDown(wxMouseEvent& event)  {
  event.Skip();
  FMouseX = event.GetX();
  FMouseY = event.GetY();
  SetCursor( wxCursor(wxCURSOR_SIZING) );
  FMouseDown = true;
}
//..............................................................................
void GuiLabel::OnMouseUp(wxMouseEvent& event)  {
  event.Skip();
  FMouseDown = false;
  SetCursor( wxCursor(wxCURSOR_ARROW) );
}
//..............................................................................
void GuiLabel::OnMouseMotion(wxMouseEvent& event)  {
  if( !FMouseDown )  {
    event.Skip();
    return;
  }
  int dx = event.GetX() - FMouseX;
  int dy = event.GetY() - FMouseY;
  if( (dx|dy) == 0 )  return;
  wxPoint ps = this->GetPosition();
  Move(ps.x+dx, ps.y+dy);
}
//..............................................................................
void GuiLabel::OnMouseDblClick(wxMouseEvent& event)  {
  event.Skip();
  //OnDblClick->Execute(this, NULL);
}
//..............................................................................
////////////////////////////////////////////////////////////////////////////////////
GuiLabel& GuiLabels::NewLabel(wxWindow* wnd, const olxstr& txt, int x, int y)  {
  GuiLabel* guil = Labels.Add( new GuiLabel(*this, wnd, x, y) );
  guil->SetLabel(txt.u_str());
  guil->Show();
  return *guil;
}