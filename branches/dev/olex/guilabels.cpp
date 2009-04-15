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
}
//..............................................................................
void GuiLabel::OnMouseDown(wxMouseEvent& event)  {
  if( !Parent.OnMouseDown(event, this) ) 
    event.Skip();
}
//..............................................................................
void GuiLabel::OnMouseUp(wxMouseEvent& event)  {
  if( !Parent.OnMouseUp(event, this) )
    event.Skip();
}
//..............................................................................
void GuiLabel::OnMouseMotion(wxMouseEvent& event)  {
  if( !Parent.OnMouseMotion(event, this) )
    event.Skip();
}
//..............................................................................
void GuiLabel::OnMouseDblClick(wxMouseEvent& event)  {
  if( !Parent.OnMouseDblClick(event, this) )
    event.Skip();
}
//..............................................................................
////////////////////////////////////////////////////////////////////////////////////
GuiLabels::GuiLabels() {
  active = NULL;
}
//..............................................................................
GuiLabel& GuiLabels::NewLabel(wxWindow* wnd, const olxstr& txt, int x, int y)  {
  GuiLabel* guil = Labels.Add( new GuiLabel(*this, wnd, x, y) );
  guil->SetLabel(txt.u_str());
  guil->Show();
  return *guil;
}
//..............................................................................
bool GuiLabels::OnMouseDblClick(wxMouseEvent& event, GuiLabel* sender)  {
  return false;
}
//..............................................................................
bool GuiLabels::OnMouseDown(wxMouseEvent& event, GuiLabel* sender)  {
  if( sender != NULL )  {
    active = sender;
    sender->SetCursor( wxCursor(wxCURSOR_SIZING) );
    return true;
  }
  return false;
}
//..............................................................................
bool GuiLabels::OnMouseUp(wxMouseEvent& event, GuiLabel* sender)  {
  if( active )  {
    active->SetCursor( wxCursor(wxCURSOR_ARROW) );
    active = NULL;
    return true;
  }
  return false;
}
//..............................................................................
bool GuiLabels::OnMouseMotion(wxMouseEvent& event, GuiLabel* sender)  {
  if( active == NULL )
    return false;
  int dx = event.GetX() - FMouseX;
  int dy = event.GetY() - FMouseY;
  if( (dx|dy) == 0 )  return true;
  wxPoint ps = active->GetPosition();
  active->Move(ps.x+dx, ps.y+dy);
  return false;
}
//..............................................................................
